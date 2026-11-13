#include "pre_inc.h"
#include "daimon/DaiSave.h"

#include <algorithm>
#include <cinttypes>
#include <cstring>

#include "daimon/DaiLog.h"
#include "daimon/DaiModeAssistant.h"
#include "daimon/DaiModeLieutenant.h"
#include "daimon/DaiModeOpponent.h"
#include "daimon/IDaiJobKind.h"
#include "keeper_ctl.h"
#include "post_inc.h"

namespace {
constexpr uint32_t MAGIC = DaiFourCC('D', 'A', 'I', 'M');
constexpr uint32_t SEC_KEEPER = DaiFourCC('K', 'E', 'E', 'P');
constexpr uint32_t SEC_KNOW = DaiFourCC('K', 'N', 'O', 'W');
constexpr uint32_t SEC_INBOX = DaiFourCC('I', 'N', 'B', 'X');
constexpr uint32_t SEC_GOALS = DaiFourCC('G', 'O', 'A', 'L');
constexpr uint32_t SEC_REQUESTS = DaiFourCC('R', 'E', 'Q', 'S');
constexpr uint32_t SEC_INCOME = DaiFourCC('I', 'N', 'C', 'M');
constexpr uint32_t SEC_DIRECTOR = DaiFourCC('D', 'I', 'R', 'C');
constexpr uint32_t SEC_SEARCHES = DaiFourCC('S', 'R', 'C', 'H');
constexpr uint32_t SEC_JOBS = DaiFourCC('J', 'O', 'B', 'S');
constexpr uint32_t SEC_LEDGER = DaiFourCC('L', 'E', 'D', 'G');
constexpr uint32_t SEC_ARBITER = DaiFourCC('A', 'R', 'B', 'I');
constexpr uint32_t SEC_MANAGERS = DaiFourCC('M', 'G', 'R', 'S');
/** The resolved difficulty (05 section 7, P3.4): its id and values by key; a save without it plays normal. */
constexpr uint32_t SEC_DIFFICULTY = DaiFourCC('D', 'I', 'F', 'F');
/** How the take was chosen, as the screens show it (P3.7): "random" picked the personality. Absent: not random. */
constexpr uint32_t SEC_SETUP = DaiFourCC('S', 'E', 'T', 'P');
constexpr uint32_t SEC_THREAT = DaiFourCC('T', 'H', 'R', 'T');
constexpr uint32_t SEC_OBJECTIVES = DaiFourCC('O', 'B', 'J', 'S');
constexpr uint32_t SEC_RIVALS = DaiFourCC('R', 'I', 'V', 'L');
constexpr uint32_t SEC_QUEUE = DaiFourCC('K', 'C', 'T', 'L');
constexpr uint32_t SEC_PIPE = DaiFourCC('P', 'I', 'P', 'E');
/** Whose side each vein is on (plan 13 section 31). Absent (an older save): worked out again at once. */
constexpr uint32_t SEC_SIDES = DaiFourCC('S', 'I', 'D', 'E');
/** The perimeter (plan 13 section 32). Absent (an older save): worked out again at once. */
constexpr uint32_t SEC_PERIMETER = DaiFourCC('P', 'E', 'R', 'I');
constexpr uint32_t SEC_LEASES = DaiFourCC('L', 'E', 'A', 'S');
constexpr uint32_t SEC_MODE = DaiFourCC('M', 'O', 'D', 'E');
/** Each section's format version this build writes and reads; a section of another version is skipped (its class
 *  starts afresh). All 1 at major 2 (14 F1): bump a section's version when its layout changes. Inside MGRS each
 *  manager carries its own (IDaiManager::SaveVersion), inside JOBS each job its kind's (IDaiJobKind::ParamsVersion),
 *  so one manager's or kind's change loses only its own state. A section's history is in its class. */
constexpr uint16_t SEC_VERSION = 1;

uint16_t section_version(uint32_t id)
{
    (void)id;
    return SEC_VERSION;
}
/** Players, slots and model numbers: the game's own ranges, kept loose (the game checks them again). */
constexpr int64_t MAX_SLOT = 64, MAX_MODEL = 255;
/** The largest controller queue image taken (a full queue of steps is a few kilobytes). */
constexpr size_t MAX_QUEUE_BYTES = 1 << 16;
/** The pipeline's limits (12 section 13.5): outputs waiting, operations and lines in one, a verb's image. */
constexpr int64_t MAX_PENDING = 64, MAX_OPS = 256, MAX_VERB_BYTES = 1 << 12, MAX_QUESTIONS = 1 << 14;

void write_bytes(DaiBlobWriter &w, const std::vector<uint8_t> &b)
{
    w.I64((int64_t)b.size());
    w.Bytes(b.data(), b.size());
}

void write_section(DaiBlobWriter &w, uint32_t id, const DaiBlobWriter &body)
{
    w.BeginSection(id, section_version(id));
    w.Bytes(body.Data().data(), body.Size());
    w.EndSection();
}

void read_why(DaiBlobReader &r, char *why, size_t size)
{
    const std::string s = r.Str();
    if (s.size() >= size) r.Fail("text too long");
    snprintf(why, size, "%s", s.c_str());
}
}

/******************************************************************************/
/* Writing */

std::vector<uint8_t> DaiSave::Write(const std::vector<const DaiKeeper *> &keepers, const IDaiActor &actor)
{
    DaiBlobWriter w;
    w.U32(MAGIC);
    w.U16(DAI_SAVE_MAJOR);
    w.U16(DAI_SAVE_MINOR);
    w.I64((int64_t)keepers.size());
    for (const DaiKeeper *k : keepers) {
        DaiBlobWriter body;
        write_keeper(body, *k, actor);
        write_section(w, SEC_KEEPER, body);
    }
    return w.Data();
}

void DaiSave::write_keeper(DaiBlobWriter &w, const DaiKeeper &k, const IDaiActor &actor)
{
    w.I64(k.m_player);
    w.I64(k.m_slot);
    w.I64((int64_t)k.m_mode->GetMode());
    w.I64((int64_t)k.m_why);
    w.I64(k.m_prev_model);
    w.U64(k.m_taken_turn);
    w.U64(k.m_last_turn);
    w.Bool(k.m_defeated);
    w.U64(k.m_rng.State());
    w.I64(k.m_last_result);
    w.I64((int64_t)k.m_ended.size());
    for (int64_t n : k.m_ended) w.I64(n);
    // The personality as it was resolved: a later edit of daimon.toml does not change a saved keeper.
    w.Str(k.m_personality.GetId().c_str());
    w.U64(k.m_personality.SourceHash());
    const auto fields = DaiPersonality::Fields();
    w.I64((int64_t)fields.size() + k.m_personality.Favours());
    for (const DaiPersonality::Field &f : fields) {
        w.Str(f.module);
        w.Str(f.key);
        w.I64(k.m_personality.*(f.member));
    }
    // The kinds it favours, as fields of a "favour" module (P9): a save without them keeps today's file's.
    for (int64_t i = 0; i < k.m_personality.Favours(); i++) {
        w.Str("favour");
        w.Str(k.m_personality.FavourAt(i).code);
        w.I64(k.m_personality.FavourAt(i).weight);
    }

    // Each part in its own section (write_section: the id and its version).
    auto section = [&](uint32_t id, auto fill) {
        DaiBlobWriter body;
        fill(body);
        write_section(w, id, body);
    };
    section(SEC_MODE, [&](DaiBlobWriter &b) { k.m_mode->Save(b); });     // the mode's own state (14 F2)
    DaiBlobWriter s;
    section(SEC_KNOW, [&](DaiBlobWriter &b) { k.m_knowledge.Save(b); });
    section(SEC_INBOX, [&](DaiBlobWriter &b) { k.m_inbox.Save(b); });
    section(SEC_GOALS, [&](DaiBlobWriter &b) { write_goals(b, k.m_goals); });
    section(SEC_REQUESTS, [&](DaiBlobWriter &b) { k.m_requests.Save(b); });
    section(SEC_INCOME, [&](DaiBlobWriter &b) { k.m_income.Save(b); });
    section(SEC_DIRECTOR, [&](DaiBlobWriter &b) { k.m_director.Save(b); });
    s = DaiBlobWriter();
    s.Str(k.m_difficulty.GetId().c_str());
    s.I64((int64_t)DaiDifficulty::Fields().size());
    for (const DaiDifficulty::Field &f : DaiDifficulty::Fields()) {
        s.Str(f.key);
        s.I64(k.m_difficulty.*(f.member));
    }
    s.U64(k.m_difficulty.Cheats());
    write_section(w, SEC_DIFFICULTY, s);
    s = DaiBlobWriter();
    s.Bool(k.m_random_personality);
    write_section(w, SEC_SETUP, s);
    section(SEC_THREAT, [&](DaiBlobWriter &b) { k.m_threat.Save(b); });
    section(SEC_OBJECTIVES, [&](DaiBlobWriter &b) { k.m_objectives.Save(b); });
    section(SEC_RIVALS, [&](DaiBlobWriter &b) { k.m_rivals.Save(b); });
    section(SEC_SIDES, [&](DaiBlobWriter &b) { k.m_gold_sides.Save(b); });
    section(SEC_PERIMETER, [&](DaiBlobWriter &b) { k.m_perimeter.Save(b); });
    section(SEC_LEASES, [&](DaiBlobWriter &b) { k.m_leases.Save(b); });
    section(SEC_SEARCHES, [&](DaiBlobWriter &b) { k.m_searches.Save(b); });
    section(SEC_JOBS, [&](DaiBlobWriter &b) { write_jobs(b, k.m_jobs); });
    section(SEC_LEDGER, [&](DaiBlobWriter &b) { write_ledger(b, k.m_ledger); });
    section(SEC_ARBITER, [&](DaiBlobWriter &b) { k.m_arbiter.Save(b); });
    s = DaiBlobWriter();
    s.I64((int64_t)k.m_managers.size());
    for (const auto &m : k.m_managers) {
        DaiBlobWriter ms;
        m->Save(ms);
        s.Str(m->GetName());
        s.U16(m->SaveVersion());
        write_bytes(s, ms.Data());
    }
    write_section(w, SEC_MANAGERS, s);
    section(SEC_PIPE, [&](DaiBlobWriter &b) { write_pipeline(b, k); });
    std::vector<uint8_t> queue;
    if (actor.SaveQueue(k.m_slot, &queue)) {
        s = DaiBlobWriter();
        write_bytes(s, queue);
        write_section(w, SEC_QUEUE, s);
    } else {
        DaiLog::Warn(k.m_player, "the slot's gesture in progress cannot be saved; a load drops it");
    }
}

void DaiSave::write_goals(DaiBlobWriter &w, const DaiGoals &g)
{
    w.U32(g.m_goals.NextId());
    w.I64(g.m_goals.Count());
    for (int64_t i = 0; i < g.m_goals.Capacity(); i++) {
        const DaiGoal &o = g.m_goals.SlotAt(i);
        if (o.id == 0) continue;
        w.I64(i);
        w.U32(o.id);
        w.I64((int64_t)o.kind);
        w.I64(o.weight);
        w.U64(o.deadline);
        w.I64((int64_t)o.status);
        w.I64((int64_t)o.source);
        w.I64(o.p.count);
        w.I64(o.p.key);
        w.I64(o.p.at.x0); w.I64(o.p.at.y0); w.I64(o.p.at.x1); w.I64(o.p.at.y1);
        w.I64(o.p.aim);
        w.U64(o.set_turn);
        w.U64(o.status_turn);
        w.Str(o.why);
    }
}

void DaiSave::write_jobs(DaiBlobWriter &w, const DaiJobPool &jobs)
{
    const auto kinds = DaiJobKinds();
    w.U32(jobs.m_jobs.NextId());
    w.I64(jobs.m_jobs.Count());
    for (int64_t i = 0; i < jobs.m_jobs.Capacity(); i++) {
        const DaiJob &j = jobs.m_jobs.SlotAt(i);
        if (j.id == 0) continue;
        const IDaiJobKind *kind = (j.kind < kinds.size()) ? kinds[j.kind] : nullptr;
        w.I64(i);
        w.U32(j.id);
        w.Str((kind != nullptr) ? kind->GetName() : "");   // by name: kind numbers may change between builds
        w.U16((kind != nullptr) ? kind->ParamsVersion() : 0);
        w.I64((int64_t)j.domain);
        w.U64(j.key);
        w.U32(j.goal_id);
        w.U32(j.request_id);
        w.I64(j.utility);
        w.U64(j.created);
        w.U64(j.state_turn);
        w.U64(j.last_progress_turn);
        w.U64(j.stall_window);
        w.U64(j.retry_after);
        w.I64(j.last_progress);
        w.I64((int64_t)j.state);
        w.I64(j.verb_id);
        w.I64(j.gestures);
        w.I64(j.needs.gold);
        w.Bool(j.needs.hand);
        w.Ref(j.needs.thing);
        w.Bool(j.end_seen);
        w.Str(j.why);
        DaiBlobWriter p;
        if (kind != nullptr) kind->SaveParams(j, p);
        write_bytes(w, p.Data());
    }
}

void DaiSave::write_ledger(DaiBlobWriter &w, const DaiLedger &l)
{
    w.U32(l.m_claims.NextId());
    w.I64(l.m_claims.Count());
    for (int64_t i = 0; i < l.m_claims.Capacity(); i++) {
        const DaiClaim &c = l.m_claims.SlotAt(i);
        if (c.id == 0) continue;
        w.I64(i);
        w.U32(c.id);
        w.I64((int64_t)c.kind);
        w.I64((int64_t)c.owner);
        w.U32(c.job_id);
        w.I64(c.amount);
        w.Ref(c.thing);
        w.I64(c.area.x0);
        w.I64(c.area.y0);
        w.I64(c.area.x1);
        w.I64(c.area.y1);
        w.Bool(c.discretionary);
        w.U64(c.expires);
    }
}

/** The outputs planned and not yet applied (not their log lines), with the provisional ids' map, so a load applies them
 *  at the same turns as a game that went on (12 section 13.5). A gesture the controller's verb format does not keep (set_assist, which
 *  Daimon does not make) is left out, and the job waiting on it ends by the stall rule. */
void DaiSave::write_pipeline(DaiBlobWriter &w, const DaiKeeper &k)
{
    w.I64(k.m_next_verb);
    w.U64(k.m_last_lands);
    w.U64(k.m_last_submit_lands);
    w.I64((int64_t)k.m_outstanding.size());
    for (const auto &o : k.m_outstanding) { w.I64(o.first); w.U64(o.second); }
    w.I64((int64_t)k.m_verb_map.size());
    for (const DaiVerbMapping &m : k.m_verb_map) { w.I64(m.provisional); w.I64(m.real); }
    w.I64((int64_t)k.m_refused.size());
    for (const DaiActResult &r : k.m_refused) { w.I64(r.verb_id); w.Str(r.error); }
    // A settled keeper (the facade settles before a save): its planning turns have all run, and their outputs wait.
    std::vector<const DaiPlanOutput *> pending;
    for (const DaiPlanOutput &o : k.m_pending) pending.push_back(&o);
    for (const auto &job : k.m_inflight) pending.push_back(&job->out);
    w.I64((int64_t)pending.size());
    for (const DaiPlanOutput *po : pending) {
        const DaiPlanOutput &o = *po;
        w.U64(o.planned);
        w.U64(o.lands);
        std::vector<std::vector<uint8_t>> verbs;
        std::vector<const DaiOutputOp *> ops;
        for (const DaiOutputOp &op : o.ops) {
            std::vector<uint8_t> b;
            if (op.kind == DaiOpKind::Submit) {
                b.resize(kctl_verb_serialise(&op.verb, nullptr, 0));
                if (b.empty() || (kctl_verb_serialise(&op.verb, b.data(), b.size()) != b.size())) continue;
            }
            ops.push_back(&op);
            verbs.push_back(std::move(b));
        }
        w.I64((int64_t)ops.size());
        for (size_t i = 0; i < ops.size(); i++) {
            w.I64((int64_t)ops[i]->kind);
            w.I64(ops[i]->verb_id);
            write_bytes(w, verbs[i]);
        }
        w.I64((int64_t)o.veins.size());
        for (const DaiVeinEdit &e : o.veins) {
            w.U64(e.id); w.I64((int64_t)e.status); w.I64(e.ex); w.I64(e.ey); w.I64(e.route_cost); w.U64(e.retry_after);
        }
        w.I64((int64_t)o.watch.size());
        for (const DaiSlabXY &x : o.watch) { w.I64(x.x); w.I64(x.y); }
        w.I64((int64_t)o.questions.size());
        for (const DaiQuestion &q : o.questions) {
            w.I64((int64_t)q.kind); w.I64(q.power); w.I64(q.x); w.I64(q.y); w.Ref(q.target);
        }
    }
}

bool DaiSave::read_pipeline(DaiBlobReader &r, DaiKeeper &k)
{
    k.m_next_verb = r.Range(0, INT64_MAX);
    k.m_last_lands = (DaiTurn)r.U64();
    k.m_last_submit_lands = (DaiTurn)r.U64();
    const int64_t nout = r.Count(MAX_OPS);
    for (int64_t i = 0; (i < nout) && r.Ok(); i++) {
        const int64_t id = r.Range(1, INT64_MAX);
        k.m_outstanding.emplace_back(id, (DaiTurn)r.U64());
    }
    const int64_t nmap = r.Count((int64_t)DAI_VERB_MAP_MAX);
    for (int64_t i = 0; (i < nmap) && r.Ok(); i++) {
        DaiVerbMapping m;
        m.provisional = r.Range(1, INT64_MAX);
        m.real = r.Range(1, INT64_MAX);
        k.m_verb_map.push_back(m);
    }
    const int64_t nref = r.Count(MAX_OPS);
    for (int64_t i = 0; (i < nref) && r.Ok(); i++) {
        DaiActResult res;
        res.verb_id = r.Range(1, INT64_MAX);
        res.failed = true;
        read_why(r, res.error, sizeof(res.error));
        k.m_refused.push_back(res);
    }
    const int64_t npend = r.Count(MAX_PENDING);
    for (int64_t i = 0; (i < npend) && r.Ok(); i++) {
        DaiPlanOutput o;
        o.planned = (DaiTurn)r.U64();
        o.lands = (DaiTurn)r.U64();
        const int64_t nops = r.Count(MAX_OPS);
        for (int64_t j = 0; (j < nops) && r.Ok(); j++) {
            DaiOutputOp op;
            op.kind = (DaiOpKind)r.Range(0, (int64_t)DaiOpKind::CancelAll);
            op.verb_id = r.Range(0, INT64_MAX);
            std::vector<uint8_t> b((size_t)r.Count(MAX_VERB_BYTES));
            r.Bytes(b.data(), b.size());
            if (!r.Ok()) break;
            if (op.kind == DaiOpKind::Submit) {
                if (!kctl_verb_deserialise(&op.verb, b.data(), b.size())) return false;
            } else if (!b.empty()) {
                return false;
            }
            o.ops.push_back(op);
        }
        const int64_t nveins = r.Count(DAI_MAX_VEINS);
        for (int64_t j = 0; (j < nveins) && r.Ok(); j++) {
            DaiVeinEdit e;
            e.id = (uint32_t)r.Range(1, UINT32_MAX);
            e.status = (DaiVeinStatus)r.Range(0, (int64_t)DaiVeinStatus::Count - 1);
            e.ex = (DaiSlabCoord)r.Range(0, INT32_MAX);
            e.ey = (DaiSlabCoord)r.Range(0, INT32_MAX);
            e.route_cost = (int32_t)r.Range(-1, INT32_MAX);
            e.retry_after = (DaiTurn)r.U64();
            o.veins.push_back(e);
        }
        const int64_t nwatch = r.Count(DAI_MAX_WATCH);
        for (int64_t j = 0; (j < nwatch) && r.Ok(); j++) {
            DaiSlabXY x;
            x.x = (DaiSlabCoord)r.Range(0, INT32_MAX);
            x.y = (DaiSlabCoord)r.Range(0, INT32_MAX);
            o.watch.push_back(x);
        }
        const int64_t nq = r.Count(MAX_QUESTIONS);
        for (int64_t j = 0; (j < nq) && r.Ok(); j++) {
            DaiQuestion q;
            q.kind = (DaiQuestion::Kind)r.Range(0, (int64_t)DaiQuestion::Kind::Takes);
            q.power = (DaiPowerKind)r.Range(0, INT32_MAX);
            q.x = (DaiSubtlCoord)r.Range(0, INT32_MAX);
            q.y = (DaiSubtlCoord)r.Range(0, INT32_MAX);
            q.target = r.Ref();
            o.questions.push_back(q);
        }
        // Not landed: its vein edits are still the planning side's view.
        k.m_unlanded.emplace_back(o.lands, o.veins);
        k.m_pending.push_back(std::move(o));
    }
    return r.Ok();
}

void DaiSave::ReconcileAfterHostLost(DaiKeeper &k, DaiTurn now)
{
    k.m_jobs.ForEach([&](DaiJob &j) {
        if (j.state != DaiJobState::Running) return;
        j.verb_id = 0;
        k.m_jobs.SetState(j, DaiJobState::Ready, "HOST_LOST", now);
    });
    k.m_ledger.ReleaseAll();
    k.m_last_result = 0;      // the slot's controller starts again: its verb ids from 1
    // The gestures the old host had planned and not submitted go with its controller (12 section 13.5).
    k.m_pending.clear();
    k.m_inflight.clear();
    k.m_unlanded.clear();
    k.m_last_lands = 0;
    k.m_last_submit_lands = 0;
    k.m_outstanding.clear();
    k.m_verb_map.clear();
    k.m_refused.clear();
    k.m_last_turn = now;
}

/******************************************************************************/
/* Reading */

bool DaiSave::Read(const uint8_t *data, size_t len, const DaimonConfig &cfg, IDaiWorldReader &world, IDaiActor &actor,
                   std::vector<DaiLoadedKeeper> *out, std::string *why)
{
    out->clear();
    DaiBlobReader r(data, len);
    if (r.U32() != MAGIC) {
        *why = "not Daimon's data";
        return false;
    }
    const uint16_t major = r.U16();
    const uint16_t minor = r.U16();
    if (r.Ok() && (major != DAI_SAVE_MAJOR)) {
        *why = "format " + std::to_string(major) + "." + std::to_string(minor) + ", this version reads " +
            std::to_string(DAI_SAVE_MAJOR) + ".x";
        return false;
    }
    const int64_t n = r.Count(DAI_MAX_PLAYERS);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        uint32_t id = 0;
        uint16_t version = 0;
        size_t length = 0;
        if (!r.NextSection(&id, &version, &length)) {
            r.Fail("a keeper is missing");
            break;
        }
        DaiBlobReader body = r.Sub(length);
        if ((id != SEC_KEEPER) || (version != SEC_VERSION)) {
            r.Fail("not a keeper record");
            break;
        }
        DaiLoadedKeeper k;
        if (!read_keeper(body, cfg, world, actor, &k, why)) {
            out->clear();
            return false;
        }
        for (const DaiLoadedKeeper &o : *out) {
            if ((o.keeper->GetPlayer() == k.keeper->GetPlayer()) || (o.keeper->GetSlot() == k.keeper->GetSlot())) {
                *why = "two keepers share a player or a slot";
                out->clear();
                return false;
            }
        }
        out->push_back(std::move(k));
    }
    if (!r.Ok() || !r.AtEnd()) {
        *why = r.Ok() ? "data after the last keeper" : r.Why();
        out->clear();
        return false;
    }
    return true;
}

bool DaiSave::read_keeper(DaiBlobReader &r, const DaimonConfig &cfg, IDaiWorldReader &world, IDaiActor &actor,
                          DaiLoadedKeeper *out, std::string *why)
{
    const PlayerNumber player = (PlayerNumber)r.Range(0, DAI_MAX_PLAYERS - 1);
    const NetUserId slot = (NetUserId)r.Range(0, MAX_SLOT);    // 0: the solo human's, an assistant's only
    const int64_t mode = r.I64();
    const bool assistant = (mode == (int64_t)DaimonMode::Assistant);
    const bool lieutenant = (mode == (int64_t)DaimonMode::Lieutenant);
    const int64_t take_why = r.Range((int64_t)DaimonTakeReason::Skirmish, (int64_t)DaimonTakeReason::LocalSeat);
    const int64_t prev_model = r.Range(0, MAX_MODEL);
    const DaiTurn taken = (DaiTurn)r.U64();
    const DaiTurn last_turn = (DaiTurn)r.U64();
    const bool defeated = r.Bool();
    const uint64_t rng = r.U64();
    const int64_t last_result = r.Range(0, INT64_MAX);
    std::array<int64_t, (size_t)DaiJobState::Cancelled + 1> ended{};
    const int64_t nended = r.Count((int64_t)ended.size());
    for (int64_t i = 0; i < nended; i++) ended[(size_t)i] = r.Range(0, INT64_MAX);
    const std::string pid = r.Str();
    const uint64_t phash = r.U64();
    if (!r.Ok()) {
        *why = std::string("keeper record: ") + r.Why();
        return false;
    }
    if ((mode != (int64_t)DaimonMode::Opponent) && !assistant && !lieutenant) {
        *why = "a keeper in a mode this version does not build";
        return false;
    }
    if (!assistant && (slot < 1)) {
        *why = "an opponent on the human's slot";
        return false;
    }
    // The personality: today's file for anything the save does not have, then the saved values, clamped.
    DaiPersonality p;
    std::string perr;
    if (!DaiPersonality::Resolve(cfg, pid.c_str(), &p, &perr)) DaiPersonality::Resolve(cfg, "default", &p, &perr);
    if (p.SourceHash() != phash)    // 05 section 4.3: the saved values play on; say so
        DaiLog::Info(player, "personality %s: daimon.toml differs from the save's; the saved values play on", pid.c_str());
    const int64_t nfields = r.Count(1024);
    for (int64_t i = 0; (i < nfields) && r.Ok(); i++) {
        const std::string module = r.Str(), key = r.Str();
        const int64_t value = r.I64();
        if (module == "favour") {
            p.SetFavour(key.c_str(), std::clamp<int64_t>(value, 0, 4 * DAI_FIX_ONE));
            continue;
        }
        for (const DaiPersonality::Field &f : DaiPersonality::Fields()) {
            if ((module != f.module) || (key != f.key)) continue;
            p.*(f.member) = (value < f.min) ? f.min : (value > f.max) ? f.max : value;
        }
    }
    p.SetIdentity(pid, phash);
    if (!r.Ok()) {
        *why = std::string("personality: ") + r.Why();
        return false;
    }
    // The mode as a new one: its own state comes with the MODE section below.
    std::unique_ptr<IDaiMode> mode_of = assistant ? std::unique_ptr<IDaiMode>(std::make_unique<DaiModeAssistant>(0))
                                      : lieutenant ? std::unique_ptr<IDaiMode>(std::make_unique<DaiModeLieutenant>(0))
                                                  : std::unique_ptr<IDaiMode>(std::make_unique<DaiModeOpponent>());
    auto k = std::make_unique<DaiKeeper>(player, slot, std::move(mode_of), p, prev_model,
        (DaimonTakeReason)take_why, taken, 1, world, actor);
    k->m_last_turn = last_turn;
    k->m_defeated = defeated;
    k->m_rng.SetState(rng);
    k->m_last_result = last_result;
    k->m_ended = ended;
    // The keeper's own sections; unknown ones (a later minor) are skipped, missing ones leave a new keeper's state.
    uint32_t id = 0;
    uint16_t version = 0;
    size_t length = 0;
    bool ok = true;
    bool pipeline = false;
    while (ok && r.NextSection(&id, &version, &length)) {
        DaiBlobReader s = r.Sub(length);
        if (version != section_version(id)) continue;
        const char *what = "";
        switch (id) {
        case SEC_MODE: what = "mode"; ok = k->m_mode->Load(s); break;
        case SEC_KNOW: what = "knowledge"; ok = k->m_knowledge.Load(s); break;
        case SEC_INBOX: what = "inbox"; ok = k->m_inbox.Load(s); break;
        case SEC_GOALS: what = "goals"; ok = read_goals(s, k->m_goals); break;
        case SEC_REQUESTS: what = "requests"; ok = k->m_requests.Load(s); break;
        case SEC_INCOME: what = "income"; ok = k->m_income.Load(s); break;
        case SEC_DIRECTOR: what = "director"; ok = k->m_director.Load(s); break;
        case SEC_SETUP: what = "setup"; k->m_random_personality = s.Bool(); ok = s.Ok(); break;
        case SEC_THREAT: what = "threat model"; ok = k->m_threat.Load(s); break;
        case SEC_OBJECTIVES: what = "objectives"; ok = k->m_objectives.Load(s); break;
        case SEC_RIVALS: what = "rivals"; ok = k->m_rivals.Load(s); break;
        case SEC_SIDES: what = "gold sides"; ok = k->m_gold_sides.Load(s); break;
        case SEC_PERIMETER: what = "perimeter"; ok = k->m_perimeter.Load(s); break;
        case SEC_LEASES: what = "leases"; ok = k->m_leases.Load(s); k->m_human_kept_stale = true; break;
        case SEC_DIFFICULTY: {
            what = "difficulty";
            // Today's built-in values for anything the save does not have, then the saved ones, clamped.
            const std::string did = s.Str();
            DaiDifficulty d;
            std::string derr;
            if (!DaiDifficulty::Resolve(cfg, did.c_str(), &d, &derr)) DaiDifficulty::Resolve(cfg, "normal", &d, &derr);
            d.SetId(did);
            const int64_t nf = s.Count(64);
            for (int64_t i = 0; (i < nf) && s.Ok(); i++) {
                const std::string key = s.Str();
                const int64_t value = s.I64();
                for (const DaiDifficulty::Field &f : DaiDifficulty::Fields())
                    if (key == f.key) d.*(f.member) = std::clamp(value, f.min, f.max);
            }
            d.SetCheats(s.U64());
            ok = s.Ok();
            if (ok) k->m_difficulty = d;
            break;
        }
        case SEC_SEARCHES: what = "searches"; ok = k->m_searches.Load(s); break;
        case SEC_JOBS: what = "jobs"; ok = read_jobs(s, k->m_jobs); break;
        case SEC_LEDGER: what = "claims"; ok = read_ledger(s, k->m_ledger); break;
        case SEC_ARBITER: what = "arbiter"; ok = k->m_arbiter.Load(s); break;
        case SEC_MANAGERS: {
            what = "managers";
            const int64_t nm = s.Count(64);
            for (int64_t i = 0; (i < nm) && s.Ok() && ok; i++) {
                const std::string name = s.Str();
                const uint16_t mver = s.U16();
                const int64_t mlen = s.Count((int64_t)s.Left());
                DaiBlobReader ms = s.Sub((size_t)mlen);
                for (auto &m : k->m_managers) {
                    if (name != m->GetName()) continue;
                    if (mver != m->SaveVersion()) {     // another layout: this manager starts afresh, the others are read
                        DaiLog::Info(player, "%s: saved state of another version (%d, now %d): starts afresh", name.c_str(), (int)mver,
                            (int)m->SaveVersion());
                        continue;
                    }
                    ok = m->Load(ms) && ms.AtEnd();
                }
            }
            ok = ok && s.Ok();
            break;
        }
        case SEC_PIPE: what = "pipeline"; ok = read_pipeline(s, *k); pipeline = ok; break;
        case SEC_QUEUE: {
            what = "controller queue";
            const int64_t qlen = s.Count((int64_t)MAX_QUEUE_BYTES);
            out->queue.resize((size_t)qlen);
            s.Bytes(out->queue.data(), (size_t)qlen);
            out->has_queue = s.Ok();
            ok = s.Ok();
            break;
        }
        default:
            continue;
        }
        if (ok && !s.AtEnd()) ok = false;
        if (!ok) *why = std::string(what) + ": " + (s.Ok() ? "malformed" : s.Why());
    }
    if (ok && !r.Ok()) {
        ok = false;
        *why = std::string("keeper sections: ") + r.Why();
    }
    if (!ok) return false;
    // A claim whose job did not come back (a kind this version does not know) is dropped with it.
    uint32_t stale[DAI_MAX_CLAIMS];
    int64_t nstale = 0;
    k->m_ledger.ForEach([&](const DaiClaim &c) {
        if ((c.owner == DaiClaimOwner::Job) && (k->m_jobs.Get(c.job_id) == nullptr)) stale[nstale++] = c.id;
    });
    for (int64_t i = 0; i < nstale; i++) k->m_ledger.m_claims.Remove(stale[i]);
    if (!pipeline) {
        // A save from before the pipeline: its jobs hold the controller's own ids, each its own provisional id.
        k->m_next_verb = k->m_last_result;
        k->m_jobs.ForEach([&](const DaiJob &j) {
            if (j.verb_id == 0) return;
            k->m_verb_map.push_back(DaiVerbMapping{ j.verb_id, j.verb_id });
            k->m_next_verb = std::max(k->m_next_verb, j.verb_id);
        });
    }
    // The planning copy as the last planning turn left it: the main knowledge and that turn's vein edits.
    k->m_plan_know = k->m_knowledge;
    for (const DaiPlanOutput &o : k->m_pending)
        for (const DaiVeinEdit &e : o.veins) k->m_plan_know.ApplyVeinEdit(e);
    out->keeper = std::move(k);
    return true;
}

bool DaiSave::read_goals(DaiBlobReader &r, DaiGoals &g)
{
    g.Clear();
    g.m_goals.SetNextId(r.U32());
    const int64_t n = r.Count(DAI_MAX_GOALS);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        const int64_t slot = r.Range(0, DAI_MAX_GOALS - 1);
        DaiGoal o;
        o.id = r.U32();
        o.kind = (DaiGoalKind)r.Range(0, (int64_t)DaiGoalKind::Count - 1);
        o.weight = r.I64();
        o.deadline = (DaiTurn)r.U64();
        o.status = (DaiGoalStatus)r.Range(0, (int64_t)DaiGoalStatus::Blocked);
        o.source = (DaiGoalSource)r.Range(0, (int64_t)DaiGoalSource::Script);
        o.p.count = r.I64();
        o.p.key = r.I64();
        o.p.at.x0 = (DaiSlabCoord)r.Range(-1, 1 << 16); o.p.at.y0 = (DaiSlabCoord)r.Range(-1, 1 << 16);
        o.p.at.x1 = (DaiSlabCoord)r.Range(-1, 1 << 16); o.p.at.y1 = (DaiSlabCoord)r.Range(-1, 1 << 16);
        o.p.aim = r.Range(-1, 8);
        o.set_turn = (DaiTurn)r.U64();
        o.status_turn = (DaiTurn)r.U64();
        read_why(r, o.why, sizeof(o.why));
        if (r.Ok() && !g.m_goals.RestoreAt(slot, o)) r.Fail("goal slot or id");
    }
    if (!r.Ok()) g.Clear();
    return r.Ok();
}

bool DaiSave::read_jobs(DaiBlobReader &r, DaiJobPool &jobs)
{
    const auto kinds = DaiJobKinds();
    jobs.Clear();
    jobs.m_jobs.SetNextId(r.U32());
    const int64_t n = r.Count(DAI_MAX_JOBS);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        const int64_t slot = r.Range(0, DAI_MAX_JOBS - 1);
        DaiJob j;
        j.id = r.U32();
        const std::string kind_name = r.Str();
        const uint16_t params_version = r.U16();
        j.domain = (DaiDomain)r.Range(0, (int64_t)DaiDomain::Count - 1);
        j.key = r.U64();
        j.goal_id = r.U32();
        j.request_id = r.U32();
        j.utility = r.I64();
        j.created = (DaiTurn)r.U64();
        j.state_turn = (DaiTurn)r.U64();
        j.last_progress_turn = (DaiTurn)r.U64();
        j.stall_window = (DaiTurn)r.U64();
        j.retry_after = (DaiTurn)r.U64();
        j.last_progress = r.I64();
        j.state = (DaiJobState)r.Range(0, (int64_t)DaiJobState::Cancelled);
        j.verb_id = r.Range(0, INT64_MAX);
        j.gestures = r.Range(0, INT64_MAX);
        j.needs.gold = r.Range(0, INT64_MAX);
        j.needs.hand = r.Bool();
        j.needs.thing = r.Ref();
        j.end_seen = r.Bool();
        read_why(r, j.why, sizeof(j.why));
        const int64_t plen = r.Count((int64_t)r.Left());
        DaiBlobReader params = r.Sub((size_t)plen);
        if (!r.Ok()) break;
        const IDaiJobKind *kind = nullptr;
        for (DaiJobKindId k = 1; k < kinds.size(); k++) {
            if ((kinds[k] != nullptr) && (kind_name == kinds[k]->GetName())) {
                kind = kinds[k];
                j.kind = k;
            }
        }
        if ((kind == nullptr) || (params_version != kind->ParamsVersion())) continue;   // dropped, with its claims
        if (!kind->LoadParams(j, params) || !params.AtEnd()) r.Fail("job parameters");
        if (r.Ok() && !jobs.m_jobs.RestoreAt(slot, j)) r.Fail("job slot or id");
    }
    if (!r.Ok()) jobs.Clear();
    return r.Ok();
}

bool DaiSave::read_ledger(DaiBlobReader &r, DaiLedger &l)
{
    l.ReleaseAll();
    l.m_claims.SetNextId(r.U32());
    const int64_t n = r.Count(DAI_MAX_CLAIMS);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        const int64_t slot = r.Range(0, DAI_MAX_CLAIMS - 1);
        DaiClaim c;
        c.id = r.U32();
        c.kind = (DaiClaimKind)r.Range(0, (int64_t)DaiClaimKind::ImpCapacity);
        c.owner = (DaiClaimOwner)r.Range(0, (int64_t)DaiClaimOwner::Agent);
        c.job_id = r.U32();
        c.amount = r.Range(0, INT64_MAX);
        c.thing = r.Ref();
        c.area.x0 = (DaiSlabCoord)r.I64();
        c.area.y0 = (DaiSlabCoord)r.I64();
        c.area.x1 = (DaiSlabCoord)r.I64();
        c.area.y1 = (DaiSlabCoord)r.I64();
        c.discretionary = r.Bool();
        c.expires = (DaiTurn)r.U64();
        if (r.Ok() && !l.m_claims.RestoreAt(slot, c)) r.Fail("claim slot or id");
    }
    if (!r.Ok()) l.ReleaseAll();
    return r.Ok();
}
