#include "pre_inc.h"
#include "daimon/DaiKeeper.h"
#include "daimon/DaiFortify.h"

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <cinttypes>

#include "daimon/DaiConstruction.h"
#include "daimon/DaiCreatures.h"
#include "daimon/DaiEconomy.h"
#include "daimon/DaiExcavation.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiOutput.h"
#include "daimon/DaiCustody.h"
#include "daimon/DaiDefence.h"
#include "daimon/DaiMagic.h"
#include "daimon/DaiOffence.h"
#include "daimon/DaiTactics.h"
#include "daimon/DaiWorkforce.h"
#include "daimon/IDaiJobKind.h"
#include "post_inc.h"

/** Facts one turn of a keeper takes from the reader (the tracker's own limit). */
constexpr int64_t KEEPER_FACTS_MAX_PER_TURN = 512;
/** Gesture results one turn takes (the controller remembers EXTSEAT_RESULT_RING). */
constexpr int64_t RESULTS_MAX_PER_TURN = 16;

DaiKeeper::DaiKeeper(PlayerNumber plyr, NetUserId slot, std::unique_ptr<IDaiMode> mode, const DaiPersonality &personality,
                     int64_t prev_model, DaimonTakeReason why, DaiTurn now, uint64_t seed, IDaiWorldReader &world,
                     IDaiActor &actor)
    : m_player(plyr), m_slot(slot), m_mode(std::move(mode)), m_personality(personality), m_prev_model(prev_model),
      m_why(why), m_taken_turn(now), m_rng(seed), m_world(&world), m_actor(&actor)
{
    m_jobs.SetPlayer(plyr);
    // Registry order (02 section 4.8): Economy, Workforce, Excavation, Construction, Creatures, then the later
    // phases'.
    m_managers.push_back(std::make_unique<DaiEconomy>());
    m_managers.push_back(std::make_unique<DaiWorkforce>());
    m_managers.push_back(std::make_unique<DaiExcavation>());
    m_managers.push_back(std::make_unique<DaiConstruction>());
    m_managers.push_back(std::make_unique<DaiCreatures>());
    m_managers.push_back(std::make_unique<DaiCustody>());
    m_managers.push_back(std::make_unique<DaiDefence>());   // before Tactics: its objectives are served the same turn
    m_managers.push_back(std::make_unique<DaiOffence>());   // before DaiMagic: its casts are served the same turn
    m_managers.push_back(std::make_unique<DaiMagic>());
    m_managers.push_back(std::make_unique<DaiTactics>());
}

const IDaiManager *DaiKeeper::Manager(DaiDomain d) const
{
    for (const auto &m : m_managers) if (m->GetDomain() == d) return m.get();
    return nullptr;
}

void DaiKeeper::Release()
{
    Settle();               // nothing of the keeper's may still run on a worker
    m_ledger.ReleaseAll();
    m_requests.Clear();
    m_objectives.Clear();
    m_searches.ReleaseAll();
    m_jobs.ForEach([&](DaiJob &j) { if (!DaiJobEnded(j.state)) m_jobs.SetState(j, DaiJobState::Cancelled, "RELEASED", m_last_turn); });
    m_actor->CancelAll(m_slot);
    // What was planned and not applied goes too: nothing of the keeper's reaches the controller after this.
    m_pending.clear();
    m_inflight.clear();
    m_unlanded.clear();
    m_last_submit_lands = 0;
    m_outstanding.clear();
    m_questions.clear();
    m_verb_map.clear();
    m_refused.clear();
    for (auto &m : m_managers) m->Reset();
    m_arbiter.Reset();
}

void DaiKeeper::SetDefeated()
{
    m_defeated = true;
    Release();
}

DaiTurn DaiPlanDelay(int64_t turn_rate, int64_t base_rate)
{
    if ((turn_rate <= 0) || (base_rate <= 0)) return 1;
    return std::max<DaiTurn>(1, (DaiTurn)((turn_rate + base_rate - 1) / base_rate));
}

void DaiKeeper::Turn(DaiTurn now, const DaiBudget &budgets, const DaimonGlobalConfig &global, int64_t turn_rate, DaiWorkers *workers)
{
    if (m_defeated) return;
    if (m_inflight.empty()) m_workers = workers;    // a planning turn already posted is waited for where it went
    auto job = std::make_shared<DaiPlanJob>();
    DaiSnapshot &s = job->snap;
    s.clock.Start();
    m_last_turn = now;
    // This keeper's config and budgets: the global ones scaled by its difficulty.
    s.cfg = global;
    s.budget = budgets;
    m_difficulty.Apply(&s.cfg, &s.budget);
    collect(now);
    apply(now);
    s.clock.Mark(DaiProfStage::Apply);
    gather(now, turn_rate, s);
    m_inflight.push_back(job);
    if ((m_workers != nullptr) && (m_workers->Count() > 0)) {
        job->seq = m_workers->Post(m_player, [this, job] { plan(*job); });
    } else {
        plan(*job);
    }
}

/** The planning turns whose outputs land by `upto`, oldest first: waited for, their outputs into the pending list. A
 *  planning turn that threw: everything posted for the keeper is waited for (none still runs on it), then it throws. */
void DaiKeeper::collect(DaiTurn upto)
{
    while (!m_inflight.empty() && (m_inflight.front()->snap.lands <= upto)) {
        const std::shared_ptr<DaiPlanJob> job = m_inflight.front();
        if (job->seq > 0) {
            const std::string err = m_workers->WaitFor(m_player, job->seq);
            if (!err.empty()) {
                m_workers->Wait(m_player);
                m_inflight.clear();
                throw std::runtime_error(err);
            }
        }
        m_pending.push_back(std::move(job->out));
        m_inflight.pop_front();
    }
}

std::string DaiKeeper::Settle()
{
    if (m_inflight.empty()) return "";
    if ((m_workers != nullptr) && (m_inflight.back()->seq > 0)) {
        const std::string err = m_workers->Wait(m_player);
        if (!err.empty()) {
            m_inflight.clear();
            return err;
        }
    }
    for (const std::shared_ptr<DaiPlanJob> &job : m_inflight) m_pending.push_back(std::move(job->out));
    m_inflight.clear();
    return "";
}

/** The outputs whose turn has come, in planning order (12 section 13.5): log lines, vein edits and watched slabs into
 *  the main knowledge, then the gestures and cancels to the controller under its own ids. */
void DaiKeeper::apply(DaiTurn now)
{
    while (!m_pending.empty() && (m_pending.front().lands <= now)) {
        const DaiPlanOutput o = std::move(m_pending.front());
        m_pending.pop_front();
        for (const DaiLogLine &l : o.lines) DaiLog::Write(l);
        for (const DaiVeinEdit &e : o.veins) m_knowledge.ApplyVeinEdit(e);
        for (const DaiSlabXY &w : o.watch) m_knowledge.MarkDirty(DaiRect{ w.x, w.y, w.x, w.y });
        if (!o.status.empty()) m_status = o.status;
        m_questions.insert(m_questions.end(), o.questions.begin(), o.questions.end());
        for (const DaiOutputOp &op : o.ops) {
            switch (op.kind) {
            case DaiOpKind::Submit: {
                int64_t real = 0;
                const char *err = m_actor->Submit(m_slot, m_player, op.verb, &real);
                if (err != nullptr) {
                    // Refused: the job hears it with the next gather's results, as a failed gesture.
                    DaiActResult r;
                    r.verb_id = op.verb_id;
                    r.failed = true;
                    snprintf(r.error, sizeof(r.error), "%s", err);
                    m_refused.push_back(r);
                    break;
                }
                if (m_verb_map.size() >= DAI_VERB_MAP_MAX) m_verb_map.erase(m_verb_map.begin());
                m_verb_map.push_back(DaiVerbMapping{ op.verb_id, real });
                break;
            }
            case DaiOpKind::Cancel:
                cancel_provisional(op.verb_id);
                break;
            case DaiOpKind::CancelAll:
                m_actor->CancelAll(m_slot);
                break;
            }
        }
    }
}

/** What the planning turn is given (its snapshot): the facts, the main knowledge read and derived, the controller's new
 *  results under the provisional ids, whether it is idle, the powers' table and the answers to the questions asked, D
 *  and the landing turn; then the copy of the knowledge it plans on. */
void DaiKeeper::gather(DaiTurn now, int64_t turn_rate, DaiSnapshot &s)
{
    s.now = now;
    s.facts.resize((size_t)KEEPER_FACTS_MAX_PER_TURN);
    s.facts.resize((size_t)m_world->Facts(m_player, now, s.facts.data(), KEEPER_FACTS_MAX_PER_TURN));
    s.clock.Mark(DaiProfStage::Facts);
    m_world->SetSeesAll(m_player, (m_difficulty.Cheats() & DAI_CHEAT_SEES_ALL) != 0);
    m_world->SetKnowsPool(m_player, (m_difficulty.Cheats() & DAI_CHEAT_KNOWS_POOL) != 0);
    m_knowledge.Read(*m_world, m_player, s.facts.data(), (int64_t)s.facts.size(), now, s.budget);
    s.clock.Mark(DaiProfStage::Read);
    m_knowledge.Derive(m_player, s.budget);
    s.clock.Mark(DaiProfStage::Derive);
    DaiActResult res[RESULTS_MAX_PER_TURN];
    const int64_t nres = m_actor->Results(m_slot, res, RESULTS_MAX_PER_TURN, m_last_result);
    for (int64_t i = 0; i < nres; i++) {
        if (res[i].verb_id > m_last_result) m_last_result = res[i].verb_id;
        for (size_t k = 0; k < m_verb_map.size(); k++) {
            if (m_verb_map[k].real != res[i].verb_id) continue;
            DaiActResult r = res[i];
            r.verb_id = m_verb_map[k].provisional;
            s.results.push_back(r);
            m_verb_map.erase(m_verb_map.begin() + (std::ptrdiff_t)k);
            break;
        }
    }
    s.results.insert(s.results.end(), m_refused.begin(), m_refused.end());
    m_refused.clear();
    std::sort(s.results.begin(), s.results.end(), [](const DaiActResult &x, const DaiActResult &y) { return x.verb_id < y.verb_id; });
    s.idle = m_actor->Idle(m_slot);
    // The powers' table, and the answers to the questions the outputs applied this turn asked.
    s.world.Gather(*m_world, m_player, m_questions);
    m_questions.clear();
    // D from the speed the game runs at, and the landing turn: never before the last output's.
    const DaiTurn delay = DaiPlanDelay(turn_rate, s.cfg.threads_base_turns_per_second);
    if (delay != m_last_delay) DaiLog::Info(m_player, "planning delay %" PRIu64 " turns (the game at %" PRId64 " turns a second)", (uint64_t)delay, turn_rate);
    m_last_delay = delay;
    s.lands = std::max<DaiTurn>(now + m_last_delay, m_last_lands);
    m_last_lands = s.lands;
    s.scanned = m_knowledge.LastScanned() + m_knowledge.LastFactReads();
    // The slabs the human unmarked (an assistant's, P7.12): kept off as the perimeter's are.
    if (m_human_kept_stale && (m_knowledge.Width() > 0)) {
        m_knowledge.SetHumanKept(m_leases.Untagged());
        m_human_kept_stale = false;
    }
    s.know = m_knowledge;
    s.leases = m_leases;
    s.clock.Mark(DaiProfStage::Copy);
}

/** One planning turn on its snapshot (any thread): everything a decision does, its gestures and lines into an output
 *  applied at the snapshot's landing turn. It touches the planning side and its snapshot only. */
void DaiKeeper::plan(DaiPlanJob &job)
{
    DaiSnapshot &s = job.snap;
    DaiPlanOutput &out = job.out;
    const DaiTurn now = s.now;
    DaimonGlobalConfig &cfg = s.cfg;
    DaiBudget &b = s.budget;
    out.planned = now;
    out.lands = s.lands;
    struct Capture {
        explicit Capture(DaiPlanOutput &o) { DaiLog::BeginCapture(&o.lines, o.planned); }
        ~Capture() { DaiLog::EndCapture(); }
    } capture(out);
    s.clock.Start();                    // a worker's clock: the time it waited for this turn is not the keeper's
    // Its outputs not landed by this gather: their vein edits are still what it decided, and its last gesture keeps
    // the controller busy until it lands.
    while (!m_unlanded.empty() && (m_unlanded.front().first <= now)) m_unlanded.pop_front();
    for (const auto &u : m_unlanded)
        for (const DaiVeinEdit &e : u.second) s.know.ApplyVeinEdit(e);
    // Its gestures that have their results now are done; one far past any stall window is given up on.
    for (const DaiActResult &r : s.results)
        m_outstanding.erase(std::remove_if(m_outstanding.begin(), m_outstanding.end(),
            [&](const std::pair<int64_t, DaiTurn> &o) { return o.first == r.verb_id; }), m_outstanding.end());
    m_outstanding.erase(std::remove_if(m_outstanding.begin(), m_outstanding.end(),
        [&](const std::pair<int64_t, DaiTurn> &o) { return now > o.second + (DaiTurn)cfg.jobs_stall_window; }), m_outstanding.end());
    // Ready for a gesture: at D 1, the controller idle at the gather and its last gesture landed. At D > 1 (the game
    // fast-forwarded) up to D gestures may be out, queued behind the running one -- at one a planning turn, as many as
    // the turns a gesture takes to land, so the keeper gestures per game turn about as it does at normal speed.
    const DaiTurn delay = std::max<DaiTurn>(1, s.lands - now);
    const bool idle = (delay <= 1) ? (s.idle && (m_last_submit_lands <= now))
                                   : ((int64_t)m_outstanding.size() < std::min<int64_t>((int64_t)delay, EXTSEAT_MAX_QUEUED_VERBS));
    DaiOutput act(out, idle, &m_next_verb);
    const int64_t gestures_before = m_arbiter.Gestures();
    const KeeperFact *facts = s.facts.data();
    const int64_t n = (int64_t)s.facts.size();
    m_inbox.Expire(now);
    m_inbox.Feed(facts, n, now, cfg);
    m_ledger.Expire(now);
    m_requests.Expire(now);
    m_objectives.Expire(now);
    DaiForecast::UpdateEma(m_income, s.know, now);
    // The perimeter: worked out when due, into this turn's knowledge (the dig costs, the layout, Fortify read it).
    const DaiTurn perimeter_before = m_perimeter.Turn();
    const int64_t gaps_before = (int64_t)m_perimeter.Gaps().size();
    m_perimeter.Update(s.know, m_personality, now);
    if (const char *why = m_perimeter.MaybeOpen(s.know, m_personality, cfg, m_forecast, m_rivals))
        DaiLog::Info(m_player, "perimeter: the gates open (%s)", why);
    m_perimeter.Apply(s.know);
    if ((m_perimeter.Turn() != perimeter_before) && (m_perimeter.Found() || (gaps_before > 0))) {
        char gates[128] = "";
        size_t len = 0;
        for (const DaiPerimeter::Gap &g : m_perimeter.Gaps())
            if (len < sizeof(gates) - 24)
                len += (size_t)snprintf(gates + len, sizeof(gates) - len, " (%d,%d)x%d", (int)g.gate.x, (int)g.gate.y, (int)g.slabs.size());
        DaiLog::Info(m_player, "perimeter: %d gaps, %" PRId64 " slabs, %" PRId64 " enclosed (radius %" PRId64 "); gates%s",
            (int)m_perimeter.Gaps().size(), m_perimeter.CutSlabs(), m_perimeter.Enclosed(), m_perimeter.Radius(), gates);
    }
    const DaiTurn sides_before = m_gold_sides.Turn();
    m_gold_sides.Update(s.know, m_personality, now);
    if (m_gold_sides.Turn() != sides_before) {
        // The gold by whose it is: the keeper's alone, shared (its share of it), a rival's, no heart's.
        int64_t mine = 0, shared = 0, share = 0, theirs = 0, nobody = 0, unlisted = 0;
        s.know.ForEachVein([&](const DaiVein &v) {
            const DaiGoldSides::Vein *t = m_gold_sides.Find(v.id);
            if (t == nullptr) unlisted += v.gold_left;
            else if (t->sharers == 0) nobody += v.gold_left;
            else if (t->share <= 0) theirs += v.gold_left;
            else if (t->sharers == 1) mine += v.gold_left;
            else { shared += v.gold_left; share += v.gold_left * t->share / DAI_FIX_ONE; }
        });
        DaiLog::Info(m_player, "gold sides: %s; own %" PRId64 ", shared %" PRId64 " (its share %" PRId64 "), rivals' %" PRId64
            ", no heart's %" PRId64 "%s", m_gold_sides.Split() ? "by route from the hearts" : "no rival heart seen", mine, shared, share,
            theirs, nobody, (unlisted > 0) ? ", some unlisted" : "");
    }
    DaiForecast::Compute(s.know, m_ledger, m_personality, cfg, m_income, now, &m_forecast, &m_gold_sides);
    // Open ground behind one of its walls: a breach, so the routes enemies would take are searched again.
    for (int64_t i = 0; i < n; i++) if (facts[i].kind == KF_WallCue) m_threat.RequestReroute();
    s.clock.Mark(DaiProfStage::Feed);
    m_threat.Update(s.know, m_player, cfg, now);
    s.clock.Mark(DaiProfStage::Threat);
    {
        std::array<int64_t, DAI_MAX_PLAYERS> aims;
        aims.fill(-1);
        m_goals.ForEach([&](const DaiGoal &g) {
            if ((g.kind == DaiGoalKind::Attack) && (g.p.aim >= 0) && (g.p.key >= 1) && (g.p.key <= DAI_MAX_PLAYERS)) aims[(size_t)(g.p.key - 1)] = g.p.aim;
        });
        m_rivals.SetAims(aims);
    }
    m_rivals.Update(s.know, m_player, cfg, m_personality, facts, n, now);
    s.clock.Mark(DaiProfStage::Rivals);

    // By name (14 K13): a member added to DaiTurnCtx cannot shift the others.
    const DaiTurnCtx c{ .player = m_player, .now = now, .know = s.know, .inbox = m_inbox, .goals = m_goals, .requests = m_requests,
                        .jobs = m_jobs, .ledger = m_ledger, .personality = m_personality, .cfg = cfg, .mode = *m_mode, .rng = m_rng,
                        .budget = b, .searches = m_searches, .forecast = m_forecast, .difficulty = m_difficulty,
                        .objectives = m_objectives, .threat = m_threat, .rivals = m_rivals, .world = s.world, .leases = &s.leases };
    take_results(c, s.results);
    track_jobs(c, act);
    s.clock.Mark(DaiProfStage::Results);
    m_director.Update(c, m_goals);
    s.clock.Mark(DaiProfStage::Director);
    run_managers(c, cfg, s.know, s.clock);
    m_arbiter.Step(c, m_jobs, m_ledger, act, m_slot, DaiJobKinds());
    s.clock.Mark(DaiProfStage::Arbiter);
    // A search whose job no longer runs is given back.
    m_searches.Sweep([&](uint32_t id) { const DaiJob *j = m_jobs.Get(id); return (j != nullptr) && !DaiJobEnded(j->state); });
    m_jobs.Reap();
    if (m_mode->GetMode() == DaimonMode::Assistant) out.status = status_line();
    m_budget_left = b;
    out.watch = watch_slabs(c);
    out.veins = s.know.TakeVeinEdits();
    out.questions = s.world.TakeAsked();
    if (act.Submitted()) m_last_submit_lands = s.lands;
    for (const DaiOutputOp &op : out.ops)
        if (op.kind == DaiOpKind::Submit) m_outstanding.emplace_back(op.verb_id, now);
    m_unlanded.emplace_back(s.lands, out.veins);
    s.clock.Mark(DaiProfStage::Watch);
    m_prof.End(s.clock, s.scanned, n, m_arbiter.Gestures() - gestures_before);
    m_plan_turn = now;
    m_plan_know = std::move(s.know);
    if ((now > 0) && ((now % DAI_SUMMARY_TURNS) == 0)) log_summary(now, cfg);
}

/** Every DAI_SUMMARY_TURNS turns (planning side): the status line and what the gold, the defences, the rivals and the
 *  fights look like, at the normal log level; the profile's window starts again. */
void DaiKeeper::log_summary(DaiTurn now, const DaimonGlobalConfig &cfg)
{
    char buf[256];
    Summary(buf, sizeof(buf));
    DaiLog::Info(m_player, "%s", buf);
    // The gold outlook (plan 13) and what it buys: the training.
    int64_t training = 0, fighters = 0;
    for (const DaiCreatureLook &cr : m_plan_know.Creatures()) {
        if (m_plan_know.Kind(cr.model).digger) continue;
        fighters++;
        for (const DaiRoomLook &r : m_plan_know.Rooms()) if ((r.kind == cr.in_room) && ((r.roles & DAI_ROLE_TRAIN) != 0)) { training++; break; }
    }
    const DaiGoal *train = m_goals.Find(DaiGoalKind::Train);
    DaiLog::Info(m_player, "gold: spare %" PRId64 " (bank %" PRId64 ", wages %" PRId64 "), side %" PRId64 " of %" PRId64 ", horizon %" PRId64
        "%s, runway %" PRId64 "; train goal %" PRId64 "/1024 to %" PRId64 ", %" PRId64 " of %" PRId64 " training", m_forecast.spare,
        m_forecast.bank_target, m_forecast.wages_next, m_forecast.side_gold, m_forecast.map_gold, m_forecast.horizon_turns,
        m_forecast.gem_held ? ", gem" : "", m_forecast.runway_paydays, (train != nullptr) ? (int64_t)train->weight : 0,
        (train != nullptr) ? train->p.count : 0, training, fighters);
    if (m_perimeter.Found()) {
        // Each gap's slabs as they stand: '.' earth, 'g' gold, 'W' its fortified wall, 'w' another's, 'f' its floor,
        // 'F' another's, 'D' its door, 'd' another's, '#' rock, '?' not seen, '~' liquid, 'r' a room.
        char gaps[160] = "";
        size_t len = 0;
        for (const DaiPerimeter::Gap &g : m_perimeter.Gaps()) {
            if (len + g.slabs.size() + 16 >= sizeof(gaps)) break;
            len += (size_t)snprintf(gaps + len, sizeof(gaps) - len, " (%d,%d) ", (int)g.gate.x, (int)g.gate.y);
            for (const DaiSlabXY &sl : g.slabs) {
                const DaiSlabInfo i = m_plan_know.SlabAt(sl.x, sl.y);
                const bool mine = (i.owner == m_player);
                char ch = '?';
                switch (i.terrain) {
                case DaiTerrain::Dirt: ch = '.'; break;
                case DaiTerrain::Gold: case DaiTerrain::DenseGold: ch = 'g'; break;
                case DaiTerrain::Wall: ch = mine ? 'W' : 'w'; break;
                case DaiTerrain::Floor: ch = mine ? 'f' : 'F'; break;
                case DaiTerrain::Door: ch = mine ? 'D' : 'd'; break;
                case DaiTerrain::Rock: case DaiTerrain::Gem: ch = '#'; break;
                case DaiTerrain::Water: case DaiTerrain::Lava: case DaiTerrain::Abyss: ch = '~'; break;
                case DaiTerrain::Room: ch = 'r'; break;
                default: break;
                }
                gaps[len++] = ch;
                gaps[len] = '\0';
            }
        }
        DaiLog::Info(m_player, "perimeter: %" PRId64 " of %" PRId64 " slabs closed;%s", m_perimeter.Closed(m_plan_know, m_player),
            m_perimeter.CutSlabs(), gaps);
    }
    int64_t own_chokes = 0;
    for (const DaiChokepoint &cp : m_threat.Chokepoints()) own_chokes += cp.own;
    const DaiChokepoint *top = m_threat.Chokepoints().empty() ? nullptr : &m_threat.Chokepoints()[0];
    DaiLog::Info(m_player, "threat: %d routes (set %" PRId64 " of turn %" PRIu64 "), %d chokepoints (%" PRId64 " own), top %" PRId64
        " at (%d,%d)", (int)m_threat.Routes().size(), m_threat.Generation(), (uint64_t)m_threat.RoutesTurn(),
        (int)m_threat.Chokepoints().size(), own_chokes, (top != nullptr) ? top->score : 0, (top != nullptr) ? (int)top->x : -1,
        (top != nullptr) ? (int)top->y : -1);
    const DaiRivalTarget *best = m_rivals.Best();
    int64_t routed = 0;
    for (const DaiRivalTarget &rt : m_rivals.Targets()) routed += (rt.route_cost > 0) ? 1 : 0;
    DaiLog::Info(m_player, "rivals: %d targets (%" PRId64 " with a route); best %s p%d at (%d,%d), score %" PRId64 "/1024, prize %" PRId64
        ", defence %" PRId64 ", route %" PRId64, (int)m_rivals.Targets().size(), routed, (best != nullptr) ? DaiTargetKindName(best->kind) : "none",
        (best != nullptr) ? (int)best->rival : -1, (best != nullptr) ? (int)best->x : -1, (best != nullptr) ? (int)best->y : -1,
        (best != nullptr) ? (int64_t)best->score : 0, (best != nullptr) ? best->prize : 0, (best != nullptr) ? best->defence : 0,
        (best != nullptr) ? best->route_cost : 0);
    if (best != nullptr) {
        const DaiReadiness r = DaiRivals::Readiness(m_plan_know, m_personality, cfg, m_forecast, *best);
        char urgent[48] = "";
        if (r.urgency > 0)
            snprintf(urgent, sizeof(urgent), "; urgency %" PRId64 "%% (level %" PRId64 ".%02" PRId64 ", runway %" PRId64 ")",
                (int64_t)r.urgency * 100 / DAI_FIX_ONE, r.level_wanted / 100, r.level_wanted % 100, r.runway_wanted);
        DaiLog::Info(m_player, "readiness: force %" PRId64 " of %" PRId64 " needed%s, mean level %" PRId64 ".%02" PRId64 "%s, runway %" PRId64 "%s%s%s",
            r.sendable, r.needed, r.force_ok ? "" : " (short)", r.mean_level / 100, r.mean_level % 100, r.level_ok ? "" : " (short)",
            m_forecast.runway_paydays, r.runway_ok ? "" : " (short)", r.overwhelming ? "; overwhelming" : "", urgent);
    }
    if (const DaiOffence *o = static_cast<const DaiOffence *>(Manager(DaiDomain::Offence))) {
        char line[256];
        size_t len = 0;
        for (PlayerNumber r = 0; (r < DAI_MAX_PLAYERS) && (len < sizeof(line) - 48); r++) {
            const DaiAllianceLook &a = m_plan_know.Alliance(r);
            if (!a.keeper) continue;
            len += (size_t)snprintf(line + len, sizeof(line) - len, " p%d value %" PRId64 "/1024 force %" PRId64 " hate %" PRId64 " war %" PRIu64 "%s%s;",
                (int)r, (int64_t)o->AllyValue(r), m_plan_know.KnownForce(r, now), m_rivals.Hate(r, now, cfg), (uint64_t)o->LastWar(r),
                a.ours ? " ours" : "", a.theirs ? " theirs" : "");
        }
        if (len > 0) DaiLog::Info(m_player, "diplomacy:%s", line);
    }
    if (const DaiTactics *t = static_cast<const DaiTactics *>(Manager(DaiDomain::Tactics))) {
        int64_t battles = 0;
        for (const DaiEngagement &e : t->Engagements()) battles += e.battle;
        DaiLog::Info(m_player, "tactics: %d engagements (%" PRId64 " battles), %" PRId64 " objectives, free force %" PRId64,
            (int)t->Engagements().size(), battles, m_objectives.Count(), t->FreeForce());
    }
    m_prof.NewWindow();
}

void DaiKeeper::Summary(char *buf, size_t len) const
{
    const DaiOwnLook &own = m_plan_know.Own();
    const DaiProfCounts &w = m_prof.Window();
    const int64_t avg_us = (w.turns > 0) ? w.ns_total / w.turns / 1000 : 0;
    snprintf(buf, len, "turn %" PRIu64 ": gold %" PRId64 ", imps %" PRId64 ", creatures %" PRId64 "; %" PRId64 " jobs, %" PRId64
        " claims; %" PRId64 " gestures in %" PRId64 " turns (total %" PRId64 "; jobs done %" PRId64 ", failed %" PRId64
        ", blocked %" PRId64 "); turn %" PRId64 " us avg, %" PRId64 " us max", (uint64_t)m_plan_turn, own.gold, own.imps,
        own.creatures, m_jobs.Count(), m_ledger.Count(), w.gestures, w.turns, m_arbiter.Gestures(), JobsEnded(DaiJobState::Done),
        JobsEnded(DaiJobState::Failed), JobsEnded(DaiJobState::Blocked), avg_us, w.ns_max / 1000);
}

/** What the controller did with the keeper's gestures: each result releases its job's claims (the gesture's gold is
 *  spent or was never needed) and moves the job on as its kind says. */
void DaiKeeper::take_results(const DaiTurnCtx &c, const std::vector<DaiActResult> &results)
{
    const auto kinds = DaiJobKinds();
    for (const DaiActResult &r : results) {
        DaiJob *j = m_jobs.FindByVerb(r.verb_id);
        if ((j == nullptr) || (j->kind >= kinds.size()) || (kinds[j->kind] == nullptr)) continue;
        m_ledger.ReleaseJob(j->id);
        j->verb_id = 0;
        const DaiJobState s = kinds[j->kind]->OnResult(*j, r);
        if ((s != j->state) || r.failed) m_jobs.SetState(*j, s, r.failed ? r.error : "", c.now);
    }
}

/** The slabs live jobs wait on, from the knowledge as the planning turn saw it: re-read in the next gather. */
std::vector<DaiSlabXY> DaiKeeper::watch_slabs(const DaiTurnCtx &c) const
{
    const auto kinds = DaiJobKinds();
    std::vector<DaiSlabXY> slabs((size_t)DAI_MAX_WATCH);
    int64_t n = 0;
    m_jobs.ForEach([&](const DaiJob &j) {
        if (DaiJobEnded(j.state) || (j.kind >= kinds.size()) || (kinds[j.kind] == nullptr) || (n >= DAI_MAX_WATCH)) return;
        n += kinds[j.kind]->Watch(c, j, slabs.data() + n, DAI_MAX_WATCH - n);
    });
    slabs.resize((size_t)n);
    return slabs;
}

/** Progress, "achieved", and the stall rule. */
void DaiKeeper::track_jobs(const DaiTurnCtx &c, IDaiActor &act)
{
    const auto kinds = DaiJobKinds();
    m_jobs.ForEach([&](DaiJob &j) {
        if (!DaiJobLive(j.state) || (j.kind >= kinds.size()) || (kinds[j.kind] == nullptr)) return;
        const IDaiJobKind &kind = *kinds[j.kind];
        char why[32] = {};
        const DaiJobState s = kind.Tick(c, j, why, sizeof(why));
        if (s != j.state) {
            if (DaiJobEnded(s)) {
                if (j.verb_id != 0) act.Cancel(m_slot, j.verb_id);
                m_ledger.ReleaseJob(j.id);
            }
            m_jobs.SetState(j, s, why, c.now);
            if (!DaiJobLive(s)) return;
        }
        if (kind.Achieved(c, j)) {
            if (j.verb_id != 0) act.Cancel(m_slot, j.verb_id);
            m_ledger.ReleaseJob(j.id);
            m_jobs.SetState(j, DaiJobState::Done, "", c.now);
            return;
        }
        m_jobs.NoteProgress(j, kind.Progress(c, j), c.now);
    });
    int64_t cancel[DAI_MAX_JOBS];
    const int64_t ncancel = m_jobs.CheckStalls(c.now, m_ledger, cancel, DAI_MAX_JOBS,
        [](const void *mode, const DaiJob &j) { return static_cast<const IDaiMode *>(mode)->HoldsJob(j); }, m_mode.get());
    for (int64_t i = 0; i < ncancel; i++) act.Cancel(m_slot, cancel[i]);
}

/** Each manager the mode allows, in registry order: first the ends of its jobs it has not seen, then its plan. */
void DaiKeeper::run_managers(const DaiTurnCtx &c, const DaimonGlobalConfig &cfg, DaiKnowledge &know, DaiStageClock &clock)
{
    std::array<bool, (size_t)DaiDomain::Count> managed{};
    for (auto &m : m_managers) {
        const DaiDomain d = m->GetDomain();
        if (!m_mode->DomainActive(d)) continue;
        managed[(size_t)d] = true;
        DaiManagerWrites w(d, m_jobs, m_inbox, m_goals, m_requests, c.now, (DaiTurn)cfg.jobs_stall_window, &know,
                           &m_objectives);
        uint32_t ended[DAI_MAX_JOBS];
        int64_t nended = 0;
        m_jobs.ForEach([&](const DaiJob &j) { if ((j.domain == d) && DaiJobEnded(j.state) && !j.end_seen) ended[nended++] = j.id; });
        for (int64_t i = 0; i < nended; i++) {
            DaiJob *j = m_jobs.Get(ended[i]);
            if (j == nullptr) continue;
            j->end_seen = true;
            m_ended[(size_t)j->state]++;
            if ((j->state == DaiJobState::Failed) || (j->state == DaiJobState::Blocked)) {
                if (j->kind < m_failed_kind.size()) m_failed_kind[j->kind]++;
                const IDaiJobKind *kind = (j->kind < DaiJobKinds().size()) ? DaiJobKinds()[j->kind] : nullptr;
                DaiLog::Info(m_player, "j%u %s %s: %s (key %llx, %llu turns after it was made)", j->id,
                             (kind != nullptr) ? kind->GetName() : "?", DaiJobStateName(j->state), j->why,
                             (unsigned long long)j->key, (unsigned long long)(c.now - j->created));
            }
            m->OnJobEnd(c, *j, w);
        }
        m->Plan(c, w);
        clock.Mark((DaiProfStage)((uint8_t)DaiProfStage::Manager + (uint8_t)d));
    }
    // Ends nobody manages (a domain the mode switched off) are seen here, so they can be reaped.
    m_jobs.ForEach([&](DaiJob &j) {
        if (DaiJobEnded(j.state) && !j.end_seen && ((j.domain >= DaiDomain::Count) || !managed[(size_t)j.domain])) {
            j.end_seen = true;
            m_ended[(size_t)j.state]++;
        }
    });
}

bool DaiKeeper::Dump(const char *what, void (*line)(const char *)) const
{
    char buf[256];
    const std::string view = (what != nullptr) ? what : "status";
    if (view == "jobs") {
        m_jobs.ForEach([&](const DaiJob &j) {
            snprintf(buf, sizeof(buf), "p%d j%u %s: %s, utility %" PRId64 ", gold %" PRId64 ", progress %" PRId64
                " (turn %" PRIu64 ")%s%s", (int)m_player, j.id, DaiJobKindName(j.kind),
                DaiJobStateName(j.state), j.utility, j.needs.gold, j.last_progress, (uint64_t)j.last_progress_turn,
                (j.why[0] != '\0') ? ": " : "", j.why);
            line(buf);
        });
        if (m_jobs.Count() == 0) { snprintf(buf, sizeof(buf), "p%d: no jobs", (int)m_player); line(buf); }
        return true;
    }
    if (view == "claims") {
        m_ledger.ForEach([&](const DaiClaim &cl) {
            snprintf(buf, sizeof(buf), "p%d c%u kind %d, job %u, gold %" PRId64 ", thing %d, expires %" PRIu64, (int)m_player,
                cl.id, (int)cl.kind, cl.job_id, cl.amount, (int)cl.thing.idx, (uint64_t)cl.expires);
            line(buf);
        });
        if (m_ledger.Count() == 0) { snprintf(buf, sizeof(buf), "p%d: no claims", (int)m_player); line(buf); }
        return true;
    }
    if (view == "inbox") {
        for (int64_t i = 0; i < m_inbox.Count(); i++) {
            const DaiInboxEntry &e = m_inbox.Get(i);
            snprintf(buf, sizeof(buf), "p%d inbox %" PRId64 ": kind %d sub %" PRId64 " at (%d,%d), turn %" PRIu64 ", expires %" PRIu64 "%s",
                (int)m_player, i, (int)e.kind, e.sub, (int)e.x, (int)e.y, (uint64_t)e.turn, (uint64_t)e.expires, e.handled ? ", handled" : "");
            line(buf);
        }
        if (m_inbox.Count() == 0) { snprintf(buf, sizeof(buf), "p%d: the inbox is empty", (int)m_player); line(buf); }
        return true;
    }
    if (view == "posture") {
        snprintf(buf, sizeof(buf), "p%d posture %s since turn %" PRIu64 "%s%s", (int)m_player, DaiPostureName(m_director.Posture()),
            (uint64_t)m_director.PostureSince(), (m_director.PostureWhy()[0] != '\0') ? ": " : "", m_director.PostureWhy());
        line(buf);
        std::string scores = "p" + std::to_string((int)m_player) + " scores:";
        for (size_t i = 0; i < (size_t)DaiPosture::Count; i++)
            scores += std::string(" ") + DaiPostureName((DaiPosture)i) + " " + std::to_string(m_director.PostureScore((DaiPosture)i));
        line(scores.c_str());
        return true;
    }
    if (view == "goals") {
        m_goals.ForEach([&](const DaiGoal &g) {
            snprintf(buf, sizeof(buf), "p%d g%u %s: count %" PRId64 ", weight %" PRId64 ", %s since turn %" PRIu64 "%s%s",
                (int)m_player, g.id, DaiGoalKindName(g.kind), g.p.count, g.weight, DaiGoalStatusName(g.status),
                (uint64_t)g.status_turn, (g.why[0] != '\0') ? ": " : "", g.why);
            line(buf);
        });
        if (m_goals.Count() == 0) { snprintf(buf, sizeof(buf), "p%d: no goals", (int)m_player); line(buf); }
        return true;
    }
    if (view == "objectives") {
        m_objectives.ForEach([&](const DaiObjective &o) {
            const DaiObjectiveReport &r = o.report;
            char sub[24] = "";
            if (o.sub != DaiObjectiveSub::None) snprintf(sub, sizeof(sub), " %s", DaiObjectiveSubName(o.sub));
            snprintf(buf, sizeof(buf), "p%d o%u %s%s from %s: %s, area (%d,%d)-(%d,%d), target %d, rival %d, stakes %" PRId64
                "/1024, gold %" PRId64 "/kturn, posted turn %" PRIu64 ", until %" PRIu64, (int)m_player, o.id,
                DaiObjectiveKindName(o.kind), sub, DaiDomainName(o.owner), DaiObjectiveStateName(r.state), (int)o.area.x0,
                (int)o.area.y0, (int)o.area.x1, (int)o.area.y1, (int)o.target.idx, (int)o.rival, (int64_t)o.stakes,
                o.gold_per_kturn, (uint64_t)o.posted, (uint64_t)o.until);
            line(buf);
            snprintf(buf, sizeof(buf), "p%d o%u report turn %" PRIu64 ": force %" PRId64 " against %" PRId64 " (levels %" PRId64
                "), lost %" PRId64 ", killed %" PRId64 ", gold spent %" PRId64 ", target health %" PRId64, (int)m_player, o.id,
                (uint64_t)r.updated, r.own_force, r.enemy_force, r.enemy_levels, r.losses, r.kills, r.gold_spent, r.target_health);
            line(buf);
        });
        if (m_objectives.Count() == 0) { snprintf(buf, sizeof(buf), "p%d: no objectives", (int)m_player); line(buf); }
        return true;
    }
    if (view == "requests") {
        m_requests.ForEach([&](const DaiRequest &r) {
            if (r.kind == DaiRequestKind::Service) {
                snprintf(buf, sizeof(buf), "p%d r%u from %s: %s to %s at (%d,%d), amount %" PRId64 ", key %" PRId64 ", posted turn %" PRIu64
                    "%s%s", (int)m_player, r.id, DaiDomainName(r.from), DaiServiceName(r.service), DaiDomainName(r.to), (int)r.at.x,
                    (int)r.at.y, r.amount, r.key, (uint64_t)r.posted, r.refused ? ", refused" : r.served ? ", served" : "",
                    (r.why[0] != '\0') ? r.why : "");
                line(buf);
                return;
            }
            snprintf(buf, sizeof(buf), "p%d r%u from %s: %s %s key %" PRId64 " %+" PRId64 "/1024, posted turn %" PRIu64
                ", expires %" PRIu64 "%s%s", (int)m_player, r.id, DaiDomainName(r.from), DaiRequestKindName(r.kind),
                DaiGoalKindName(r.goal), r.key, r.weight_delta, (uint64_t)r.posted, (uint64_t)r.expires,
                (r.why[0] != '\0') ? ": " : "", r.why);
            line(buf);
        });
        if (m_requests.Count() == 0) { snprintf(buf, sizeof(buf), "p%d: no requests", (int)m_player); line(buf); }
        return true;
    }
    if (view == "know") {
        know_lines(line);
        return true;
    }
    if (view != "status") return false;
    const std::string cheats = m_difficulty.CheatsText();
    snprintf(buf, sizeof(buf), "p%d: %s, slot %" PRId64 ", personality %s%s, difficulty %s%s%s%s, taken at turn %" PRIu64 " (reason %" PRId64
        "), built-in model %" PRId64 " to go back to%s", (int)m_player, m_mode->GetName(), (int64_t)m_slot,
        m_personality.GetId().c_str(), m_random_personality ? " (random)" : "", m_difficulty.GetId().c_str(), cheats.empty() ? "" : " (cheats: ", cheats.c_str(),
        cheats.empty() ? "" : ")", (uint64_t)m_taken_turn, (int64_t)m_why, m_prev_model, m_defeated ? ", defeated" : "");
    line(buf);
    const DaiOwnLook &own = m_plan_know.Own();
    snprintf(buf, sizeof(buf), "p%d: knows gold %" PRId64 ", imps %" PRId64 ", creatures %" PRId64 "; map %dx%d, %" PRId64
        " full scans", (int)m_player, own.gold, own.imps, own.creatures, (int)m_plan_know.Width(), (int)m_plan_know.Height(),
        m_plan_know.ScanCycles());
    line(buf);
    snprintf(buf, sizeof(buf), "p%d: %" PRId64 " jobs, %" PRId64 " claims (gold %" PRId64 "), inbox %" PRId64 "; %" PRId64
        " gestures; jobs done %" PRId64 ", failed %" PRId64 ", blocked %" PRId64, (int)m_player, m_jobs.Count(), m_ledger.Count(),
        m_ledger.GoldClaimed(), m_inbox.Count(), m_arbiter.Gestures(), JobsEnded(DaiJobState::Done), JobsEnded(DaiJobState::Failed),
        JobsEnded(DaiJobState::Blocked));
    line(buf);
    return true;
}

/** What the keeper knows: its own dungeon, its powers, the map by terrain class, its rivals. */
void DaiKeeper::know_lines(void (*line)(const char *)) const
{
    char buf[256];
    const DaiOwnLook &own = m_plan_know.Own();
    char heart[64];
    if (own.has_heart)
        snprintf(heart, sizeof(heart), "heart at (%d,%d), health %" PRId64, (int)own.heart_x, (int)own.heart_y, own.heart_health);
    else
        snprintf(heart, sizeof(heart), "no heart");
    snprintf(buf, sizeof(buf), "p%d own: gold %" PRId64 ", imps %" PRId64 ", creatures %" PRId64 ", wages %" PRId64 " in %" PRId64
        " turns; %s; %" PRId64 " rooms", (int)m_player, own.gold, own.imps, own.creatures, own.wage_bill, own.turns_to_payday,
        heart, (int64_t)m_plan_know.Rooms().size());
    line(buf);
    for (size_t d = 0; d < (size_t)DaiPower::Count; d++) {
        const DaiPowerLook &pw = m_plan_know.Power((DaiPower)d);
        snprintf(buf, sizeof(buf), "p%d power %s: %s, price %" PRId64, (int)m_player, DaiPowerCode((DaiPower)d),
            pw.available ? "available" : "not available", pw.price);
        line(buf);
    }
    std::array<int64_t, (size_t)DaiTerrain::Count> terrain{};
    int64_t own_slabs = 0;
    for (DaiSlabCoord y = 0; y < m_plan_know.Height(); y++) {
        for (DaiSlabCoord x = 0; x < m_plan_know.Width(); x++) {
            const DaiSlabInfo s = m_plan_know.SlabAt(x, y);
            terrain[(size_t)s.terrain]++;
            own_slabs += (s.owner == m_player);
        }
    }
    snprintf(buf, sizeof(buf), "p%d map %dx%d, %" PRId64 " full scans: unknown %" PRId64 ", rock %" PRId64 ", earth %" PRId64
        ", gold %" PRId64 ", gems %" PRId64 ", floor %" PRId64 ", rooms %" PRId64 ", liquid %" PRId64 ", walls %" PRId64 "; %" PRId64
        " of them the keeper's", (int)m_player, (int)m_plan_know.Width(), (int)m_plan_know.Height(), m_plan_know.ScanCycles(),
        terrain[(size_t)DaiTerrain::Unknown], terrain[(size_t)DaiTerrain::Rock], terrain[(size_t)DaiTerrain::Dirt],
        terrain[(size_t)DaiTerrain::Gold] + terrain[(size_t)DaiTerrain::DenseGold], terrain[(size_t)DaiTerrain::Gem],
        terrain[(size_t)DaiTerrain::Floor], terrain[(size_t)DaiTerrain::Room],
        terrain[(size_t)DaiTerrain::Water] + terrain[(size_t)DaiTerrain::Lava] + terrain[(size_t)DaiTerrain::Abyss],
        terrain[(size_t)DaiTerrain::Wall], own_slabs);
    line(buf);
    for (PlayerNumber r = 0; r < (PlayerNumber)DAI_MAX_PLAYERS; r++) {
        if (r == m_player) continue;
        const DaiRivalSighting &s = m_plan_know.Sighting(r);
        if (!s.heart_seen && (s.creatures_last_cycle == 0) && (s.creatures_this_cycle == 0)) continue;
        snprintf(buf, sizeof(buf), "p%d rival p%d: heart %s, %" PRId64 " creatures seen in the last full scan, force %" PRId64
            " now", (int)m_player, (int)r, s.heart_seen ? "seen" : "not seen", s.creatures_last_cycle,
            m_plan_know.KnownForce(r, m_last_turn));
        line(buf);
    }
}

/** What the assistant is doing, for the Options screen: up to three job kinds by name and its gestures so far (plan's
 *  side: the jobs and the arbiter are the planning thread's). */
std::string DaiKeeper::status_line() const
{
    std::string doing;
    int64_t shown = 0;
    m_jobs.ForEach([&](const DaiJob &j) {
        if ((shown >= 3) || (DaiJobKindOf(j.kind) == nullptr)) return;
        std::string name = DaiJobKindName(j.kind);
        std::replace(name.begin(), name.end(), '_', ' ');
        if (doing.find(name) != std::string::npos) return;
        doing += (doing.empty() ? "" : ", ") + name;
        shown++;
    });
    char buf[192];
    snprintf(buf, sizeof(buf), "%s; %" PRId64 " gesture(s) so far", doing.empty() ? "waiting for work" : doing.c_str(),
        m_arbiter.Gestures());
    return buf;
}

void DaiKeeper::cancel_provisional(int64_t provisional)
{
    for (const DaiVerbMapping &m : m_verb_map) if (m.provisional == provisional) m_actor->Cancel(m_slot, m.real);
    for (DaiPlanOutput &o : m_pending) {
        o.ops.erase(std::remove_if(o.ops.begin(), o.ops.end(), [provisional](const DaiOutputOp &op) {
            return (op.kind == DaiOpKind::Submit) && (op.verb_id == provisional);
        }), o.ops.end());
    }
}

void DaiKeeper::NoteTouches(const KeeperTouch *t, int64_t n, DaiTurn now)
{
    const bool agent = (m_mode->GetMode() == DaimonMode::Lieutenant);
    for (int64_t i = 0; i < n; i++) {
        m_leases.Note(t[i], now);
        if ((t[i].kind == KTch_DigTag) || (t[i].kind == KTch_DigUntag)) m_human_kept_stale = true;
        if ((t[i].kind != KTch_Creature) || (t[i].thing <= 0)) continue;
        // The jobs that hold it give it up.
        std::vector<uint32_t> jobs;
        m_ledger.ForEach([&](const DaiClaim &c) {
            if ((c.kind == DaiClaimKind::Thing) && (c.owner == DaiClaimOwner::Job) && (c.thing.idx == (DaiThingIndex)t[i].thing)) jobs.push_back(c.job_id);
        });
        for (uint32_t id : jobs) {
            DaiJob *j = m_jobs.Get(id);
            if ((j == nullptr) || DaiJobEnded(j->state)) continue;
            if (j->verb_id != 0) cancel_provisional(j->verb_id);   // the job's id is the planning side's
            m_jobs.SetState(*j, DaiJobState::Failed, agent ? "PREEMPTED_BY_AGENT" : "PREEMPTED_BY_PLAYER", now);
            m_ledger.ReleaseJob(id);
            DaiLog::Info(m_player, "j%u gave up creature %d: the %s touched it", id, (int)t[i].thing, agent ? "agent" : "human");
        }
    }
    m_leases.Expire(now, agent ? daimon_config.global.lieutenant_agent_lease_turns : daimon_config.global.assistant_lease_turns);
}
