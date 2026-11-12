#include "pre_inc.h"
#include "daimon/DaiCustody.h"

#include <algorithm>
#include <array>
#include <cinttypes>
#include <climits>
#include <cstdlib>

#include "daimon/DaiFix.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
/** A heal or a speed asked for a victim is not asked again for this long (the cast comes, or the gold did not). */
constexpr DaiTurn HEAL_AGAIN = 400, SPEED_AGAIN = 600;

bool rack_plan(DaiCustodyPlan p)
{
    return (p == DaiCustodyPlan::Convert) || (p == DaiCustodyPlan::Interrogate) || (p == DaiCustodyPlan::Ghost);
}

bool kept_alive(DaiCustodyPlan p)
{
    return (p == DaiCustodyPlan::Convert) || (p == DaiCustodyPlan::Interrogate);
}

const DaiPrisonerLook *look_of(const DaiKnowledge &k, const DaiThingRef &r)
{
    const DaiCustodyLook &cu = k.Custody();
    for (int64_t i = 0; i < cu.prisoners; i++) if (cu.prisoner[i].ref == r) return &cu.prisoner[i];
    return nullptr;
}

/** The keeper's first room with these roles and a free place (`free`), else its first with them; nullptr if none. */
const DaiRoomLook *room_with(const DaiKnowledge &k, uint64_t roles, bool free)
{
    const DaiRoomLook *any = nullptr;
    for (const DaiRoomLook &r : k.Rooms()) {
        if ((r.roles & roles) == 0) continue;
        if (!free || (r.used < r.capacity)) return &r;
        if (any == nullptr) any = &r;
    }
    return free ? nullptr : any;
}

int64_t pct(int64_t v, int64_t p) { return v * p / 100; }

/** One of its creatures being lured by a rival's scavenger room: praying stops it (08 section 7a.3). */
bool scavenged(const DaiKnowledge &k)
{
    for (const DaiCreatureLook &cr : k.Creatures()) if (cr.scavenged) return true;
    return false;
}

/** Its creatures a temple would help: angry for no reason the keeper can mend, diseased, or chickens (08 section 7.1). */
int64_t prayers(const DaiKnowledge &k)
{
    int64_t n = 0;
    for (const DaiCreatureLook &cr : k.Creatures())
        n += (((cr.anger != DaiAnger::Calm) && (cr.anger_reason == DaiAngerReason::Other)) || cr.diseased || cr.chickened) ? 1 : 0;
    return n;
}

/** How short the gold is (plan 13, DaiFix 0..1): in full when the income ends now, half at extras_horizon turns, none
 *  from twice that, with a gem held or the gold not known. */
DaiFix scarcity(const DaiTurnCtx &c)
{
    const DaiForecastOut &f = c.forecast;
    if (!f.gold_known || f.gem_held || (f.horizon_turns == DAI_NEVER)) return 0;
    const int64_t ref = 2 * std::max<int64_t>(c.personality.economy_extras_horizon, 1);
    return (DaiFix)std::clamp<int64_t>((ref - f.horizon_turns) * DAI_FIX_ONE / ref, 0, DAI_FIX_ONE);
}

/** A creature of kind `m`'s upkeep as force (plan 13): its pay at the keeper's force per gold, by the scarcity and
 *  w_upkeep. */
int64_t upkeep(const DaiTurnCtx &c, DaiThingModel m, DaiFix scarce, DaiFix fpg)
{
    if ((m <= 0) || (scarce <= 0)) return 0;
    const DaiFix force = FixMul(FixFromInt(c.know.Kind(m).pay), fpg);
    return FixToInt(FixMul(FixMul(force, scarce), c.personality.custody_w_upkeep));
}
}

bool DaiCustody::PrisonersOverBodies(const DaiTurnCtx &c)
{
    const DaiKnowledge &k = c.know;
    const DaiCustodyLook &cu = k.Custody();
    if ((c.personality.custody_vampires == 0) || (cu.vampire_model <= 0) || (cu.bodies_per_vampire <= 0)
        || (room_with(k, DAI_ROLE_GRAVEYARD, true) == nullptr)) return true;
    const DaiKindLook &vk = k.Kind(cu.vampire_model);
    if (!vk.known) return true;
    int64_t vampire = vk.score[0];
    if (vk.rebirth > 0) vampire += FixToInt(FixMul(FixFromInt(vampire), c.cfg.abilities_rebirth_value));
    // Its upkeep on a gold-poor map (plan 13: a vampire's pay is 750).
    vampire = std::max<int64_t>(vampire - upkeep(c, cu.vampire_model, scarcity(c), ForcePerGold(k)), 0);
    const int64_t body = FixToInt(FixMul(FixFromInt(vampire), c.personality.custody_w_body)) / cu.bodies_per_vampire;
    // What its prisoners would become, from those it holds (the kinds the rules make of a starved or racked one).
    DaiThingModel skeleton = 0, ghost = 0;
    for (int64_t i = 0; i < cu.prisoners; i++) {
        if (skeleton == 0) skeleton = cu.prisoner[i].skeleton;
        if (ghost == 0) ghost = cu.prisoner[i].ghost;
    }
    int64_t sum = 0, n = 0;
    for (const DaiThingLook &e : k.Enemies()) {
        if ((e.battle == 0) || (e.model <= 0) || k.Alliance(e.owner).ours) continue;
        DaiPrisonerLook p;
        p.model = e.model;
        p.level = std::max<int64_t>(e.level, 1);
        p.owner = e.owner;
        p.force = e.force;
        const int64_t lv = std::clamp<int64_t>(p.level - 1, 0, DaiKindLook::LEVELS - 1);
        p.skeleton = skeleton;
        p.skeleton_force = (skeleton > 0) ? k.Kind(skeleton).score[lv] : 0;
        p.ghost = ghost;
        p.ghost_force = (ghost > 0) ? k.Kind(ghost).score[lv] : 0;
        const DaiCustodyValues v = Values(c, p, true);
        sum += v.v[(size_t)v.Best()];
        n++;
    }
    // Nobody fought now: as before, prisoners.
    return (n == 0) || (sum >= body * n);
}

DaiCustodyPlan DaiCustodyValues::Best() const
{
    // Ties: the first in the plans' order (convert, interrogate, skeleton, ghost, hold). Nothing worth anything: dismissed.
    DaiCustodyPlan best = DaiCustodyPlan::Dismiss;
    int64_t top = 0;
    for (DaiCustodyPlan p : { DaiCustodyPlan::Convert, DaiCustodyPlan::Interrogate, DaiCustodyPlan::Skeleton, DaiCustodyPlan::Ghost,
                              DaiCustodyPlan::Hold }) {
        if (v[(size_t)p] > top) { top = v[(size_t)p]; best = p; }
    }
    return best;
}

int64_t DaiCustody::ForcePerGold(const DaiKnowledge &k)
{
    int64_t force = 0, pay = 0;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        force += cr.force;
        pay += k.Kind(cr.model).pay;
    }
    return (pay > 0) ? force * DAI_FIX_ONE / pay : 0;
}

DaiThingModel DaiCustody::MostPaidKind(const DaiKnowledge &k)
{
    std::array<int64_t, DAI_MAX_KINDS> wages{};
    for (const DaiCreatureLook &cr : k.Creatures())
        if ((cr.model > 0) && (cr.model < DAI_MAX_KINDS)) wages[(size_t)cr.model] += k.Kind(cr.model).pay;
    DaiThingModel best = 0;
    for (size_t m = 1; m < wages.size(); m++) if ((wages[m] > 0) && ((best == 0) || (wages[m] > wages[(size_t)best]))) best = (DaiThingModel)m;
    return best;
}

DaiCustodyValues DaiCustody::Values(const DaiTurnCtx &c, const DaiPrisonerLook &p, bool prison_room_free)
{
    const DaiKnowledge &k = c.know;
    const DaiCustodyLook &cu = k.Custody();
    const DaiPersonality &per = c.personality;
    DaiCustodyValues out;
    auto put = [&](DaiCustodyPlan plan, int64_t v, int64_t w) { out.v[(size_t)plan] = FixToInt(FixMul(FixFromInt(v), w)); };
    // A victim of the kind it pays most for, on a rack: every creature of that kind costs torture_payday % of its pay
    // -- a pay day's saving, as force by the keeper's own force per gold of pay.
    int64_t discount = 0;
    if ((per.custody_pay_discount_torture != 0) && (p.model == MostPaidKind(k)) && (cu.torture_payday < 100)) {
        int64_t wage = 0;
        for (const DaiCreatureLook &cr : k.Creatures()) if (cr.model == p.model) wage += k.Kind(cr.model).pay;
        discount = FixToInt(FixMul(FixFromInt(pct(wage, 100 - cu.torture_payday)), ForcePerGold(k)));
    }
    // Each plan's creature pays its upkeep on a gold-poor map (plan 13): a ghost or a skeleton little, a converted kind
    // its own pay.
    const DaiFix scarce = scarcity(c), fpg = ForcePerGold(k);
    auto net = [&](int64_t v, DaiThingModel m) { return std::max<int64_t>(v - upkeep(c, m, scarce, fpg), 0); };
    put(DaiCustodyPlan::Convert, net(pct(p.force, cu.convert_chance) + discount, p.model), per.custody_w_convert);
    // An interrogation shows the ground about the heart of a keeper Daimon has not found.
    const bool unknown_heart = (p.owner >= 0) && k.RivalKeeper(p.owner) && !k.Sighting(p.owner).heart_seen;
    put(DaiCustodyPlan::Interrogate, unknown_heart ? c.cfg.custody_interrogate_value + discount : 0, per.custody_w_interrogate);
    put(DaiCustodyPlan::Skeleton, (p.skeleton > 0) ? net(pct(p.skeleton_force, cu.skeleton_chance), p.skeleton) : 0, per.custody_w_skeleton);
    put(DaiCustodyPlan::Ghost, (p.ghost > 0) ? net(pct(p.ghost_force, cu.ghost_chance) + discount, p.ghost) : 0, per.custody_w_ghost);
    put(DaiCustodyPlan::Hold, prison_room_free ? FixToInt(FixMul(FixFromInt(p.force), c.cfg.custody_hold_share)) : 0, per.custody_w_hold);
    return out;
}

DaiSacrificeChoice DaiCustody::SacrificePlan(const DaiTurnCtx &c, const std::vector<DaiPrisonerRecord> &prisoners, DaiFix imps_per_kturn)
{
    const DaiKnowledge &k = c.know;
    const DaiCustodyLook &cu = k.Custody();
    const int64_t who = c.personality.custody_sacrifices;      // 0 none, 1 prisoners, 2 any
    if ((who == 0) || k.Recipes().empty()) return DaiSacrificeChoice();
    std::vector<DaiSacrificeCandidate> cands;
    // Prisoners: what their own plan is worth (one worth nothing: free).
    for (const DaiPrisonerRecord &r : prisoners) {
        const DaiPrisonerLook *p = look_of(k, r.ref);
        if ((p == nullptr) || (p->at == DaiHeldAt::Moving) || (p->at == DaiHeldAt::Hand)) continue;
        cands.push_back(DaiSacrificeCandidate{ r.ref, p->model, p->level, (r.plan == DaiCustodyPlan::Dismiss) ? 0 : r.value });
    }
    int64_t fighters = 0, angry = 0;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        fighters++;
        if (cr.anger != DaiAnger::Calm) angry++;
        if ((who < 2) || (cr.held_by >= 0) || (cr.battle != 0) || cr.sacrificing || k.Kind(cr.model).digger) continue;
        cands.push_back(DaiSacrificeCandidate{ cr.ref, cr.model, cr.level, cr.force });
    }
    // Imps above the personality's least: for cheaper imps. Not one trained to imp_train_level or training (speed at 3,
    // teleport at 10: worth far more than the step off the next imp, P9).
    const int64_t fpg = ForcePerGold(k);
    if ((who >= 2) && ((int64_t)k.OwnImps().size() > c.personality.workforce_imps_min)) {
        for (const DaiImpLook &i : k.OwnImps()) {
            if (i.sacrificing || i.training || (i.level >= c.personality.workforce_imp_train_level)) continue;
            cands.push_back(DaiSacrificeCandidate{ i.ref, i.tunneller ? i.model : 0, 1, i.force });
        }
    }
    // The imps' kind: the first digger kind (cheaper imps' recipes name it).
    DaiThingModel imp_kind = 0;
    for (DaiThingModel m = 1; (m <= k.KindCount()) && (imp_kind == 0); m++) if (k.Kind(m).known && k.Kind(m).digger) imp_kind = m;
    for (DaiSacrificeCandidate &cd : cands) if (cd.model == 0) cd.model = imp_kind;
    DaiSacrificeWorth worth;
    worth.creature = [&k](DaiThingModel kind, int64_t lv) {
        const DaiKindLook &kl = k.Kind(kind);
        return kl.known ? kl.score[std::max<int64_t>(0, std::min<int64_t>(lv, DaiKindLook::LEVELS - 1))] : 0;
    };
    worth.research = (k.Research().count > 0) ? c.cfg.custody_research_value : 0;
    worth.manufacture = (k.Stock().making_kind != 0) ? c.cfg.custody_manufacture_value : 0;
    worth.all_happy = c.cfg.custody_happy_value * angry;
    worth.bless_per_creature = c.cfg.custody_bless_value;
    worth.creatures = fighters;
    // Cheaper imps: the step off every imp to come (those KeepImps still wants, or as many as the rate imps were made
    // at lately brings over imp_horizon turns), less the one given up bought again. P9, level 52: a keeper buying back
    // some 60 imps a trap killed counted only the one it was short and never sacrificed one. The price counts the
    // imps less those sacrificed and never goes below none: given up and bought again, an imp leaves that count two
    // lower, and an imp to come is cheaper only while its count is above none, so sacrifices banked below none are
    // worth only the imps to come beyond them (P9, screen h: some 300 a game, with count_imps then counting each
    // imp bought back as one more to come).
    const DaiGoal *keep = c.goals.Find(DaiGoalKind::KeepImps);
    const int64_t short_of = std::max<int64_t>(0, ((keep != nullptr) ? keep->p.count : 0) - (int64_t)k.OwnImps().size());
    const int64_t to_come = (imps_per_kturn * c.cfg.custody_imp_horizon + 500 * DAI_FIX_ONE) / (1000 * DAI_FIX_ONE);
    const int64_t step = c.cfg.custody_imp_price_step, price = k.Power(DaiPower::CreateImp).price;
    const int64_t counted = (k.Own().imps - k.Own().tunnellers) - cu.imps_sacrificed;
    const int64_t rebuy = price - step * (std::max<int64_t>(0, counted) - std::max<int64_t>(0, counted - 2));
    const int64_t cheaper = std::max<int64_t>(0, std::max(short_of, to_come) - std::max<int64_t>(0, 1 - counted));
    worth.cheaper_imps = FixToInt(FixMul(FixFromInt(step * cheaper - rebuy), fpg));
    DaiSacrificeCounts counts;
    // What sinks now counts already: planned from the counts alone, the next drop repeats one (and two of a kind can
    // complete a punishment).
    for (size_t m = 0; m < counts.n.size(); m++) {
        counts.n[m] = cu.offered[m] + cu.sinking[m];
        counts.levels[m] = cu.offered_levels[m] + cu.sinking_levels[m];
    }
    return DaiSacrificePlanner::Plan(k.Recipes(), counts, cands, worth);
}

const DaiPrisonerRecord *DaiCustody::Record(const DaiThingRef &r) const
{
    for (const DaiPrisonerRecord &p : m_prisoners) if (p.ref == r) return &p;
    return nullptr;
}

void DaiCustody::Reset()
{
    m_prisoners.clear();
    m_next_plan = 0;
    m_replan = false;
    m_fought = false;
    m_converted = m_interrogated = m_dismissed = 0;
    m_sacrificed = 0;
    m_sacrifice_wanted = false;
    m_bodies = false;
    m_vampires = 0;
    m_jb_who = DaiThingRef{};
    m_jb_at = DaiSlabXY{ -1, -1 };
    m_jb_rival = -1;
    m_jb_since = m_jb_cool = 0;
    m_jb_request = m_jb_objective = 0;
    m_jb_creep = false;
    m_freed = 0;
    m_punished = 0;
    m_sacrifice_retry = 0;
    m_imps_seen.clear();
    m_imps_baseline = false;
    m_imps_new = 0;
    m_imp_sample = 0;
    m_imp_rate = 0;
    m_imp_samples = 0;
}

DaiFix DaiCustody::NextImpRate(DaiFix rate, bool first, int64_t made, int64_t dt)
{
    if (dt <= 0) return rate;
    const DaiFix sample = (std::max<int64_t>(made, 0) * 1000 * DAI_FIX_ONE) / dt;
    if (first) return sample;
    return rate + FixMul(sample - rate, (dt * DAI_FIX_ONE) / (dt + IMP_TAU));
}

/** New own imps since the last look (made, or come over): the rate cheaper imps are worth. */
void DaiCustody::count_imps(const DaiTurnCtx &c)
{
    std::vector<DaiThingRef> now;
    for (const DaiImpLook &i : c.know.OwnImps()) {
        now.push_back(i.ref);
        if (m_imps_baseline && (std::find(m_imps_seen.begin(), m_imps_seen.end(), i.ref) == m_imps_seen.end())) m_imps_new++;
    }
    // An imp bought back for one sacrificed is not one more to come: counted, each sacrifice made the next worth more.
    const int64_t sacrificed = c.know.Custody().imps_sacrificed;
    if (m_imps_baseline) m_imps_new -= std::max<int64_t>(0, sacrificed - m_imps_sacrificed_seen);
    m_imps_sacrificed_seen = sacrificed;
    m_imps_seen.swap(now);
    m_imps_baseline = true;
    if ((m_imp_sample == 0) || (c.now < m_imp_sample)) {
        m_imp_sample = (c.now > 0) ? c.now : 1;
        m_imps_new = 0;
        return;
    }
    const int64_t dt = (int64_t)(c.now - m_imp_sample);
    if (dt < IMP_SAMPLE) return;
    m_imp_rate = NextImpRate(m_imp_rate, m_imp_samples == 0, m_imps_new, dt);
    m_imp_samples++;
    m_imp_sample = c.now;
    m_imps_new = 0;
}

void DaiCustody::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    count_imps(c);
    refresh(c, w);
    if (m_replan || (c.now >= m_next_plan)) {
        plan_all(c);
        m_next_plan = c.now + (DaiTurn)c.cfg.custody_plan_interval;
        m_replan = false;
    }
    tendency(c, w);
    rooms(c, w);
    hand(c, w);
    casts(c, w);
    jailbreak(c, w);
}

void DaiCustody::end_jailbreak(const DaiTurnCtx &c, DaiManagerWrites &w, const char *why)
{
    if (m_jb_request != 0) w.WithdrawRequest(m_jb_request);
    if (m_jb_objective != 0) w.DropObjective(m_jb_objective);
    DaiLog::Info(c.player, "jailbreak at (%d,%d) on p%d ended: %s", (int)m_jb_at.x, (int)m_jb_at.y, (int)m_jb_rival, why);
    m_jb_who = DaiThingRef{};
    m_jb_at = DaiSlabXY{ -1, -1 };
    m_jb_rival = -1;
    m_jb_request = m_jb_objective = 0;
    m_jb_creep = false;
    m_jb_since = 0;
}

/** A creature of its own in another keeper's prison: a slab of its own beside that prison (08 section 10). */
void DaiCustody::jailbreak(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    auto own_land = [&](DaiSlabCoord x, DaiSlabCoord y) {
        const DaiSlabInfo s = k.SlabAt(x, y);
        return (s.owner == c.player) && ((s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Room) || (s.terrain == DaiTerrain::Door));
    };
    const int *const dx = DAI_N4_DX, *const dy = DAI_N4_DY;
    if (m_jb_at.x >= 0) {
        const DaiCreatureLook *who = k.Creature(m_jb_who);
        if ((who == nullptr) || (who->held_by != m_jb_rival)) {
            if ((who != nullptr) && (who->held_by < 0)) m_freed++;
            end_jailbreak(c, w, (who == nullptr) ? "GONE" : "OUT");
            return;
        }
        bool refused = false;
        c.requests.ForEach([&](const DaiRequest &q) { refused = refused || ((q.id == m_jb_request) && q.refused); });
        // A creep refused (an imp it could not lift): its imps convert another's floor beside its land anyway -- wait.
        if (refused && m_jb_creep) {
            w.WithdrawRequest(m_jb_request);
            m_jb_request = 0;
            refused = false;
        }
        if (refused || (c.now >= m_jb_since + (DaiTurn)c.cfg.custody_jailbreak_timeout)) {
            end_jailbreak(c, w, refused ? "REFUSED" : "TIMED_OUT");
            m_jb_cool = c.now + (DaiTurn)c.cfg.custody_jailbreak_retry;
            return;
        }
        // Its land beside the slab: from digging to creeping onto it (until it is its own; then the engine's roll).
        bool beside = false;
        for (int d = 0; d < 4; d++) beside = beside || own_land((DaiSlabCoord)(m_jb_at.x + dx[d]), (DaiSlabCoord)(m_jb_at.y + dy[d]));
        if (beside && !m_jb_creep && (k.SlabAt(m_jb_at.x, m_jb_at.y).owner != c.player)) {
            if (m_jb_request != 0) w.WithdrawRequest(m_jb_request);
            DaiRequest q;
            q.kind = DaiRequestKind::Service;
            q.to = DaiDomain::Excavation;
            q.service = DaiService::Creep;
            q.key = (int64_t)m_jb_at.y * k.Width() + m_jb_at.x;
            q.at = m_jb_at;
            q.amount = c.cfg.offence_creep_imps;
            snprintf(q.why, sizeof(q.why), "JAILBREAK");
            m_jb_request = w.PostRequest(q);
            m_jb_creep = true;
        }
        return;
    }
    if ((c.personality.custody_rescue_value <= 0) || (c.now < m_jb_cool)) return;
    // The creature most worth it, held by another keeper.
    const DaiCreatureLook *best = nullptr;
    for (const DaiCreatureLook &cr : k.Creatures()) {
        if ((cr.held_by < 0) || (cr.held_by == c.player)) continue;
        if (FixToInt(FixMul(FixFromInt(cr.force), c.personality.custody_rescue_value)) < c.cfg.custody_jailbreak_min_force) continue;
        if ((best == nullptr) || (cr.force > best->force)) best = &cr;
    }
    if (best == nullptr) return;
    // Its prison's slabs (the room it stands in, as far as seen) and the floor beside them nearest its own land.
    const PlayerNumber rival = best->held_by;
    const DaiSlabXY start{ DaiSubtileSlab(best->x), DaiSubtileSlab(best->y) };
    const DaiSlabInfo at = k.SlabAt(start.x, start.y);
    if ((at.terrain != DaiTerrain::Room) || (at.owner != rival)) return;
    std::vector<DaiSlabXY> room{ start }, open{ start };
    std::vector<uint8_t> seen((size_t)k.Width() * k.Height(), 0);
    seen[(size_t)start.y * k.Width() + start.x] = 1;
    while (!open.empty() && (room.size() < 400)) {
        const DaiSlabXY s = open.back();
        open.pop_back();
        for (int d = 0; d < 4; d++) {
            const DaiSlabCoord x = (DaiSlabCoord)(s.x + dx[d]), y = (DaiSlabCoord)(s.y + dy[d]);
            if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height()) || seen[(size_t)y * k.Width() + x]) continue;
            const DaiSlabInfo n = k.SlabAt(x, y);
            if ((n.terrain != DaiTerrain::Room) || (n.owner != rival) || (n.room != at.room)) continue;
            seen[(size_t)y * k.Width() + x] = 1;
            room.push_back(DaiSlabXY{ x, y });
            open.push_back(DaiSlabXY{ x, y });
        }
    }
    // The floor beside it to claim (S), and the slab beside S to dig to (T: earth or nobody's floor -- a claim route
    // cannot end on another keeper's floor; S itself when its own land is beside it already), T nearest its land.
    auto free_ground = [&](DaiSlabCoord x, DaiSlabCoord y) {
        const DaiSlabInfo t = k.SlabAt(x, y);
        return (t.owner < 0) && ((t.terrain == DaiTerrain::Dirt) || (t.terrain == DaiTerrain::Floor) || (t.terrain == DaiTerrain::Gold));
    };
    DaiSlabXY pick{ -1, -1 }, dig{ -1, -1 }, stage{ -1, -1 };
    int64_t pick_d = INT64_MAX;
    for (const DaiSlabXY &s : room) {
        for (int d = 0; d < 4; d++) {
            const DaiSlabCoord x = (DaiSlabCoord)(s.x + dx[d]), y = (DaiSlabCoord)(s.y + dy[d]);
            if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height()) || seen[(size_t)y * k.Width() + x]) continue;
            if (k.SlabAt(x, y).terrain != DaiTerrain::Floor) continue;
            for (int e = 0; e < 4; e++) {
                const DaiSlabCoord tx = (DaiSlabCoord)(x + dx[e]), ty = (DaiSlabCoord)(y + dy[e]);
                const bool own = own_land(tx, ty);
                if (!own && !free_ground(tx, ty)) continue;
                // The nearest of its own land to T (chessboard slabs); T its own: there already.
                for (DaiSlabCoord oy = 0; oy < k.Height(); oy++) {
                    for (DaiSlabCoord ox = 0; ox < k.Width(); ox++) {
                        if (!own_land(ox, oy)) continue;
                        const int64_t dd = std::max<int64_t>(std::abs(ox - tx), std::abs(oy - ty));
                        if (dd < pick_d) {
                            pick_d = dd;
                            pick = DaiSlabXY{ x, y };
                            dig = own ? DaiSlabXY{ -1, -1 } : DaiSlabXY{ tx, ty };
                            stage = DaiSlabXY{ ox, oy };
                        }
                    }
                }
            }
        }
    }
    if (pick.x < 0) { m_jb_cool = c.now + (DaiTurn)c.cfg.custody_jailbreak_retry; return; }
    m_jb_who = best->ref;
    m_jb_at = pick;
    m_jb_rival = rival;
    m_jb_since = c.now;
    m_jb_creep = false;
    DaiObjective o;
    o.kind = DaiObjectiveKind::Raid;
    o.sub = DaiObjectiveSub::Jailbreak;
    o.key = KEY_JAIL;
    o.area = DaiRect{ (DaiSlabCoord)std::max<int64_t>(pick.x - 1, 0), (DaiSlabCoord)std::max<int64_t>(pick.y - 1, 0),
                      (DaiSlabCoord)std::min<int64_t>(pick.x + 1, k.Width() - 1), (DaiSlabCoord)std::min<int64_t>(pick.y + 1, k.Height() - 1) };
    o.rival = rival;
    o.staging = stage;
    o.stakes = 2 * DAI_FIX_ONE;
    o.home_guard_share = c.personality.offence_home_guard_share;
    o.until = c.now + (DaiTurn)c.cfg.custody_jailbreak_timeout;
    m_jb_objective = w.PostObjective(o);
    // A dig to T first (the creep follows once its land is beside S); T its own already: the creep at once (next turn).
    if (dig.x >= 0) {
        DaiRequest q;
        q.kind = DaiRequestKind::Service;
        q.to = DaiDomain::Excavation;
        q.service = DaiService::Dig;
        q.key = (int64_t)dig.y * k.Width() + dig.x;
        q.at = dig;
        snprintf(q.why, sizeof(q.why), "JAILBREAK");
        m_jb_request = w.PostRequest(q);
    }
    DaiLog::Info(c.player, "jailbreak: creature %d in p%d's prison, its land to (%d,%d) by (%d,%d) (%" PRId64 " slabs from its own)", (int)best->ref.idx,
        (int)rival, (int)pick.x, (int)pick.y, (int)dig.x, (int)dig.y, pick_d);
}

/** The records follow the custody look: a prisoner it no longer lists is gone, a new one waits for a plan. Custody
 *  facts make every prisoner be planned again at once. */
void DaiCustody::refresh(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiCustodyLook &cu = c.know.Custody();
    std::vector<DaiPrisonerRecord> kept;
    for (int64_t i = 0; i < cu.prisoners; i++) {
        const DaiPrisonerRecord *old = Record(cu.prisoner[i].ref);
        if (old != nullptr) {
            kept.push_back(*old);
            continue;
        }
        DaiPrisonerRecord r;
        r.ref = cu.prisoner[i].ref;
        r.since = c.now;
        kept.push_back(r);
        m_replan = true;
    }
    m_prisoners = std::move(kept);
    const uint32_t mask = (1u << KF_Custody) | (1u << KF_Sacrifice) | (1u << KF_VampireProgress);
    for (int64_t i = c.inbox.Next(mask, 0); i >= 0; i = c.inbox.Next(mask, i + 1)) {
        const DaiInboxEntry &e = c.inbox.Get(i);
        if (e.kind == KF_VampireProgress) {
            m_vampires++;
            w.HandleInbox(i);
            continue;
        }
        if (e.kind == KF_Sacrifice) {
            m_sacrificed++;
            bool punished = false;
            for (const DaiRecipeLook &rc : c.know.Recipes())
                if ((rc.index == e.args[0]) && (DaiSacrificePlanner::Worth(rc, 0, DaiSacrificeWorth()) == INT64_MIN)) punished = true;
            if (punished) m_punished++;
            DaiLog::Info(c.player, "recipe %" PRId64 " fired%s", e.args[0], punished ? ": a punishment" : "");
            m_replan = true;
            w.HandleInbox(i);
            continue;
        }
        if (e.sub == KFC_Converted) m_converted++;
        if (e.sub == KFC_Interrogated) m_interrogated++;
        m_replan = true;
        w.HandleInbox(i);
    }
    if (!c.know.Battles().empty()) m_fought = true;
    if (c.know.Custody().bodies_lying >= c.cfg.custody_graveyard_bodies) m_bodies = true;
}

void DaiCustody::plan_all(const DaiTurnCtx &c)
{
    const bool prison_free = room_with(c.know, DAI_ROLE_PRISON, true) != nullptr;
    for (DaiPrisonerRecord &r : m_prisoners) {
        const DaiPrisonerLook *p = look_of(c.know, r.ref);
        if (p == nullptr) continue;
        const DaiCustodyValues v = Values(c, *p, prison_free || (p->at == DaiHeldAt::Prison));
        const DaiCustodyPlan best = v.Best();
        const int64_t value = v.v[(size_t)best];
        if (best != r.plan)
            DaiLog::Info(c.player, "prisoner %d (kind %d L%" PRId64 " of p%d): %s, worth %" PRId64, (int)r.ref.idx, (int)p->model,
                p->level, (int)p->owner, DaiCustodyPlanName(best), value);
        r.plan = best;
        r.value = value;
    }
    m_sacrifice_wanted = SacrificePlan(c, m_prisoners, m_imp_rate).any;
}

/** The imprison tendency (08 section 6): on while the personality takes prisoners and a prison has prison_spare places
 *  free -- a knocked-out enemy with nowhere to go wakes and fights again by its imps. With a graveyard standing (room in
 *  it, the personality's vampires), its fights leave prisoners only while they are worth more than bodies (P9): the
 *  enemies it fights now, each as the prisoner it would be (its best plan, Values), against w_body of a vampire over the
 *  bodies one takes (a vampire reborn at its lair counted rebirth_value more). */
void DaiCustody::tendency(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    int64_t spare = 0;
    bool prison = false;
    for (const DaiRoomLook &r : k.Rooms()) {
        if ((r.roles & DAI_ROLE_PRISON) == 0) continue;
        prison = true;
        spare += std::max<int64_t>(0, r.capacity - r.used);
    }
    bool want = (c.personality.custody_take_prisoners != 0) && prison && (spare >= c.cfg.custody_prison_spare);
    if (want) want = PrisonersOverBodies(c);
    if ((k.Own().imprison_tendency == want) || (w.FindJob(DAI_JOB_SET_TENDENCY, KEY_TENDENCY) != nullptr)) return;
    DaiJob *j = w.AddJob(DAI_JOB_SET_TENDENCY, KEY_TENDENCY, FixFromInt(1));
    if (j == nullptr) return;
    DaiTendencyParams t;
    t.which = DaiTendency::Imprison;
    t.on = want;
    j->p = t;
}

/** A prison once it has fought (its personality taking prisoners), a torture chamber once a prisoner's plan wants a
 *  rack, a temple once a sacrifice would be worth it or two of its creatures want to pray, a graveyard once bodies lay
 *  about (its personality wanting vampires): asked of the director (08 section 9). */
void DaiCustody::rooms(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    bool rack = false;
    for (const DaiPrisonerRecord &r : m_prisoners) rack = rack || rack_plan(r.plan);
    const struct { DaiRoom room; uint64_t role; bool want; } asks[] = {
        { DaiRoom::Prison, DAI_ROLE_PRISON, (c.personality.custody_take_prisoners != 0) && m_fought },
        { DaiRoom::Torture, DAI_ROLE_TORTURE, rack },
        { DaiRoom::Temple, DAI_ROLE_SACRIFICE, m_sacrifice_wanted || (prayers(k) >= 2) || scavenged(k) },
        { DaiRoom::Graveyard, DAI_ROLE_GRAVEYARD, (c.personality.custody_vampires != 0) && m_bodies },
    };
    for (const auto &a : asks) {
        const bool want = a.want && (room_with(k, a.role, false) == nullptr) && k.Build(a.room).available;
        const DaiRequest *asked = c.requests.Find(DaiDomain::Custody, DaiGoalKind::Build, (int64_t)a.room);
        if (want && (asked == nullptr)) {
            DaiRequest q;
            q.goal = DaiGoalKind::Build;
            q.key = (int64_t)a.room;
            q.count_min = std::max<int64_t>(c.personality.ProgrammeSlabs(a.room), 9);
            q.weight_delta = DAI_FIX_ONE / 4;
            snprintf(q.why, sizeof(q.why), "CUSTODY");
            w.PostRequest(q);
        } else if (!want && (asked != nullptr)) {
            w.WithdrawRequest(asked->id);
        }
    }
}

/** One hand trip at a time: the best victim to a free rack; one whose plan no longer wants the rack back to prison;
 *  one worth nothing to the portal; a hungry one being kept fed a chicken. */
void DaiCustody::hand(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    if (w.FindJob(DAI_JOB_CUSTODY_HAND, KEY_HAND) != nullptr) return;
    const DaiRoomLook *rack = (k.Custody().devices_free > 0) ? room_with(k, DAI_ROLE_TORTURE, false) : nullptr;
    const DaiRoomLook *prison = room_with(k, DAI_ROLE_PRISON, true);
    const DaiRoomLook *portal = room_with(k, DAI_ROLE_PORTAL, false);
    const DaiPrisonerRecord *pick = nullptr;
    DaiCustodyTrip trip = DaiCustodyTrip::Count;
    const DaiRoomLook *to = nullptr;
    auto consider = [&](const DaiPrisonerRecord &r, DaiCustodyTrip t, const DaiRoomLook *room) {
        if ((pick == nullptr) || (r.value > pick->value)) { pick = &r; trip = t; to = room; }
    };
    for (const DaiPrisonerRecord &r : m_prisoners) {
        const DaiPrisonerLook *p = look_of(k, r.ref);
        if ((p == nullptr) || (p->at == DaiHeldAt::Moving) || (p->at == DaiHeldAt::Hand)) continue;
        if (rack_plan(r.plan) && (p->at != DaiHeldAt::Torture) && (rack != nullptr)) consider(r, DaiCustodyTrip::Torture, rack);
        else if (!rack_plan(r.plan) && (r.plan != DaiCustodyPlan::Dismiss) && (p->at == DaiHeldAt::Torture) && (prison != nullptr))
            consider(r, DaiCustodyTrip::Prison, prison);
    }
    if ((pick == nullptr) && k.Custody().pool && (c.now >= m_sacrifice_retry)) {
        DaiCustodyHandParams sp;
        if (sacrifice(c, &sp)) {
            DaiJob *j = w.AddJob(DAI_JOB_CUSTODY_HAND, KEY_HAND, FixFromInt(2));
            if (j != nullptr) j->p = sp;
            return;
        }
    }
    if (pick == nullptr) {
        for (const DaiPrisonerRecord &r : m_prisoners) {
            const DaiPrisonerLook *p = look_of(k, r.ref);
            if ((p == nullptr) || (p->at == DaiHeldAt::Moving) || (p->at == DaiHeldAt::Hand) || (r.plan != DaiCustodyPlan::Dismiss) || (portal == nullptr)) continue;
            pick = &r;
            trip = DaiCustodyTrip::Dismiss;
            to = portal;
            break;
        }
    }
    DaiCustodyHandParams hp;
    if (pick != nullptr) {
        hp.prisoner = pick->ref;
        hp.trip = trip;
        hp.room = to->index;
        if (trip == DaiCustodyTrip::Dismiss) m_dismissed++;
    } else {
        // Feeding: a held prisoner, or a victim waiting for a rack, hungry; never one meant to starve.
        for (const DaiPrisonerRecord &r : m_prisoners) {
            const DaiPrisonerLook *p = look_of(k, r.ref);
            if ((p == nullptr) || !p->hungry || (p->at != DaiHeldAt::Prison) || k.Food().empty()) continue;
            if ((r.plan != DaiCustodyPlan::Hold) && !kept_alive(r.plan)) continue;
            const DaiFoodLook *best = nullptr;
            int64_t best_d = 0;
            for (const DaiFoodLook &f : k.Food()) {
                const int64_t d = std::max<int64_t>(std::abs(f.x - p->x), std::abs(f.y - p->y));
                if ((best == nullptr) || (d < best_d)) { best = &f; best_d = d; }
            }
            hp.prisoner = r.ref;
            hp.trip = DaiCustodyTrip::Feed;
            hp.food = best->ref;
            pick = &r;
            break;
        }
    }
    if (pick == nullptr) return;
    DaiJob *j = w.AddJob(DAI_JOB_CUSTODY_HAND, KEY_HAND, FixFromInt(2));
    if (j != nullptr) j->p = hp;
}

/** The next sacrifice, if one is worth making: a hand trip to the pool. */
bool DaiCustody::sacrifice(const DaiTurnCtx &c, DaiCustodyHandParams *hp)
{
    const DaiSacrificeChoice ch = SacrificePlan(c, m_prisoners, m_imp_rate);
    if (!ch.any) return false;
    hp->prisoner = ch.victim.ref;
    hp->trip = DaiCustodyTrip::Sacrifice;
    const std::span<const DaiRecipeLook> recipes = c.know.Recipes();
    const int64_t slot = ((ch.recipe >= 0) && (ch.recipe < (int64_t)recipes.size())) ? recipes[(size_t)ch.recipe].index : -1;
    DaiLog::Info(c.player, "sacrifice: %d (kind %d L%" PRId64 ") for recipe %" PRId64 ", worth %" PRId64 " (%" PRId64 " drops)", (int)ch.victim.ref.idx,
        (int)ch.victim.model, ch.victim.level, slot, ch.value, ch.drops);
    return true;
}

/** A victim being kept alive healed below heal_victim_below % of its health; once past its kind's break time, sped up
 *  (its torture points come twice as fast) -- each only with the gold spare after the forecast's reserve. */
void DaiCustody::casts(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    for (DaiPrisonerRecord &r : m_prisoners) {
        const DaiPrisonerLook *p = look_of(k, r.ref);
        if ((p == nullptr) || !kept_alive(r.plan) || (p->at == DaiHeldAt::Hand) || (p->at == DaiHeldAt::Moving)) continue;
        if (w.FindJob(DAI_JOB_MAGIC_CAST, KEY_CAST + (uint64_t)r.ref.idx) != nullptr) continue;
        DaiPower power = DaiPower::Count;
        const bool heal_due = (r.healed == 0) || (c.now >= r.healed + HEAL_AGAIN), speed_due = (r.sped == 0) || (c.now >= r.sped + SPEED_AGAIN);
        if ((p->max_health > 0) && (p->health * 100 < p->max_health * c.cfg.custody_heal_victim_below) && heal_due)
            power = DaiPower::Heal;
        else if ((p->at == DaiHeldAt::Torture) && (p->torture_turns > p->break_time) && speed_due)
            power = DaiPower::Speed;
        if (power == DaiPower::Count) continue;
        const DaiPowerLook &pw = k.Power(power);
        if (!pw.available || (pw.price > c.forecast.spendable)) continue;
        DaiJob *j = w.AddJob(DAI_JOB_MAGIC_CAST, KEY_CAST + (uint64_t)r.ref.idx, FixFromInt(2));
        if (j == nullptr) return;
        DaiCastParams cp;
        cp.power = power;
        cp.level = 0;
        cp.target = r.ref;
        cp.price = pw.price;
        j->p = cp;
        (power == DaiPower::Heal ? r.healed : r.sped) = c.now;
        return;
    }
}

void DaiCustody::OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w)
{
    if (j.kind == DAI_JOB_CUSTODY_HAND) m_replan = true;     // where it went changes what it is worth
    // A sacrifice refused (the victim would not be picked up): not the same trip again every turn.
    const DaiCustodyHandParams *hp = std::get_if<DaiCustodyHandParams>(&j.p);
    if ((j.kind == DAI_JOB_CUSTODY_HAND) && (j.state == DaiJobState::Failed) && (hp != nullptr) && (hp->trip == DaiCustodyTrip::Sacrifice))
        m_sacrifice_retry = c.now + c.cfg.jobs_stall_window / 4;
    if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
}

void DaiCustody::Save(DaiBlobWriter &w) const
{
    w.I64((int64_t)m_prisoners.size());
    for (const DaiPrisonerRecord &r : m_prisoners) {
        w.Ref(r.ref);
        w.I64((int64_t)r.plan);
        w.I64(r.value);
        w.U64(r.since);
        w.U64(r.healed);
        w.U64(r.sped);
    }
    w.U64(m_next_plan);
    w.Bool(m_replan);
    w.Bool(m_fought);
    w.I64(m_converted);
    w.I64(m_interrogated);
    w.I64(m_dismissed);
    w.I64(m_sacrificed);
    w.Bool(m_sacrifice_wanted);
    w.Bool(m_bodies);
    w.I64(m_vampires);
    w.Ref(m_jb_who);
    w.I64(m_jb_at.x);
    w.I64(m_jb_at.y);
    w.I64(m_jb_rival);
    w.U64(m_jb_since);
    w.U64(m_jb_cool);
    w.U32(m_jb_request);
    w.U32(m_jb_objective);
    w.Bool(m_jb_creep);
    w.I64(m_freed);
    w.I64(m_punished);
    w.U64(m_sacrifice_retry);
    w.I64(m_imps_new);
    w.U64(m_imp_sample);
    w.I64(m_imp_rate);
    w.I64(m_imp_samples);
}

bool DaiCustody::Load(DaiBlobReader &r)
{
    Reset();
    const int64_t n = r.Count(DaiCustodyLook::MAX_PRISONERS);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        DaiPrisonerRecord p;
        p.ref = r.Ref();
        p.plan = (DaiCustodyPlan)r.Range(0, (int64_t)DaiCustodyPlan::Count - 1);
        p.value = r.Range(INT64_MIN, INT64_MAX);
        p.since = (DaiTurn)r.U64();
        p.healed = (DaiTurn)r.U64();
        p.sped = (DaiTurn)r.U64();
        m_prisoners.push_back(p);
    }
    m_next_plan = (DaiTurn)r.U64();
    m_replan = r.Bool();
    m_fought = r.Bool();
    m_converted = r.Range(0, INT64_MAX);
    m_interrogated = r.Range(0, INT64_MAX);
    m_dismissed = r.Range(0, INT64_MAX);
    m_sacrificed = r.Range(0, INT64_MAX);
    m_sacrifice_wanted = r.Bool();
    m_bodies = r.Bool();
    m_vampires = r.Range(0, INT64_MAX);
    m_jb_who = r.Ref();
    m_jb_at.x = (DaiSlabCoord)r.Range(-1, 255);
    m_jb_at.y = (DaiSlabCoord)r.Range(-1, 255);
    m_jb_rival = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
    m_jb_since = (DaiTurn)r.U64();
    m_jb_cool = (DaiTurn)r.U64();
    m_jb_request = r.U32();
    m_jb_objective = r.U32();
    m_jb_creep = r.Bool();
    m_freed = r.Range(0, INT64_MAX);
    m_punished = r.Range(0, INT64_MAX);
    m_sacrifice_retry = (DaiTurn)r.U64();
    m_imps_new = r.Range(0, INT64_MAX / 4);
    m_imp_sample = (DaiTurn)r.U64();
    m_imp_rate = r.Range(0, INT64_MAX / 4);
    m_imp_samples = r.Range(0, INT64_MAX);
    if (!r.Ok()) Reset();
    return r.Ok();
}
