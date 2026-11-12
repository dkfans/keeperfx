#include "pre_inc.h"
#include "daimon/DaiExcavation.h"
#include "daimon/DaiGeom.h"

#include <algorithm>
#include <cinttypes>

#include "daimon/DaiDirector.h"
#include "daimon/DaiForecast.h"
#include "daimon/DaiLog.h"
#include "post_inc.h"

namespace {
/** Unmark jobs for the perimeter's kept slabs: apart from the hazards' keys (a slab index). */
constexpr uint64_t KEPT_UNMARK_KEY = 1ULL << 40;
/** Turns of imp work a vein slab takes to mine (03 section 2.1: ~180 for gold). */
constexpr int64_t MINE_TURNS_A_SLAB = 180;
/** Gold a gem slab gives a hit, and turns between hits (03 section 2.1). */
constexpr int64_t GEM_GOLD_A_HIT = 8, TURNS_A_HIT = 9;

/** Turns of walking from every slab to the nearest one the keeper's imps stand on (4 sides, through anything): the
 *  straight-line part of a route estimate. */
std::vector<int32_t> distance_from_reach(const DaiKnowledge &k)
{
    const DaiSlabCoord w = k.Width(), h = k.Height();
    std::vector<int32_t> d((size_t)w * (size_t)h, INT32_MAX);
    std::vector<int32_t> queue;
    for (DaiSlabCoord y = 0; y < h; y++)
        for (DaiSlabCoord x = 0; x < w; x++)
            if (k.Reach().Reached(x, y)) { d[(size_t)y * w + x] = 0; queue.push_back((int32_t)(y * w + x)); }
    for (size_t head = 0; head < queue.size(); head++) {
        const int32_t at = queue[head];
        const DaiSlabCoord x = at % w, y = at / w;
        const int32_t next[4] = { (y > 0) ? at - (int32_t)w : -1, (x + 1 < w) ? at + 1 : -1, (y + 1 < h) ? at + (int32_t)w : -1,
                                  (x > 0) ? at - 1 : -1 };
        for (int32_t nb : next) {
            if ((nb < 0) || (d[(size_t)nb] != INT32_MAX)) continue;
            d[(size_t)nb] = d[(size_t)at] + 1;
            queue.push_back(nb);
        }
    }
    return d;
}

bool live(const DaiJob *j) { return (j != nullptr) && !DaiJobEnded(j->state); }

/** Imps a vein being worked takes: more veins than the imps can work leaves every one half done. */
constexpr int64_t IMPS_A_VEIN = 4;

bool has_treasury(const DaiKnowledge &k)
{
    const RoomKind t = k.RoomKindOf(DaiRoom::Treasury);
    for (const DaiRoomLook &r : k.Rooms()) if ((t != 0) && (r.kind == t)) return true;
    return false;
}
}

int64_t DaiExcavation::VeinScore(const DaiTurnCtx &c, const DaiVein &v, int64_t route_turns, DaiSlabXY entry, DaiSlabXY haul_to,
                                 int64_t imps_available)
{
    const int64_t imps = std::max<int64_t>(imps_available, 1);
    const int64_t route = std::max<int64_t>(route_turns, 0);
    // Haul (03 section 8.2): the round trips the gold needs, an imp's load at a time, shared by the imps.
    const int64_t carry = std::max<int64_t>(c.know.Own().imp_gold_hold, 1);
    const int64_t round_trip = 2 * (std::abs(entry.x - haul_to.x) + std::abs(entry.y - haul_to.y)) * c.cfg.dig_floor;
    auto score = [&](int64_t gold, int64_t work_turns) {
        if (gold <= 0) return (int64_t)0;
        const int64_t haul_turns = ((gold + carry - 1) / carry) * round_trip / imps;
        return (gold * 1024) / (route + work_turns / imps + haul_turns + 1);
    };
    // Its gold, and its gems over the horizon: each paid for in the imps' hitting and carrying. A gem gives a hit's gold
    // every few turns, slower than gold comes out of a slab, so gems win only where gold is far or gone. Scored apart,
    // the better taken: gold beside a gem is not made worth less by it.
    const int64_t gold = score(v.gold_left, ((int64_t)v.gold_slabs + v.dense_slabs) * MINE_TURNS_A_SLAB);
    const int64_t horizon = std::max<int64_t>(c.personality.excavation_horizon_turns, 0);
    const int64_t gems = score((int64_t)v.gem_slabs * GEM_GOLD_A_HIT * horizon / TURNS_A_HIT, (int64_t)v.gem_slabs * horizon);
    // Gold about as near a rival as the keeper: first, before the rival takes it (plan 13).
    const int64_t best = std::max(gold, gems);
    if (DaiForecast::VeinSide(c.know, c.personality, v, c.forecast.sides) == DaiVeinSide::Contested)
        return FixToInt(FixMul(FixFromInt(best), c.personality.excavation_contest_vein_weight));
    return best;
}

/** Dig requests: a dig_route job through the slab asked for, its route searched by the job itself as a claim route --
 *  water bridged, done once the keeper's land reaches it (07 P5.12: a breach's staging point must be the keeper's, and
 *  imps claim nothing across water; a bridge is also land to drop an imp on, by ground never seen). Creep
 *  requests: a creep job, imps brought to another keeper's slab by the keeper's land (`amount` of them). */
void DaiExcavation::services(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    // A request withdrawn (its asker gave up): its job is dropped, and with it the job's claims.
    std::vector<std::pair<DaiJobKindId, uint64_t>> gone;
    c.jobs.ForEach([&](const DaiJob &j) {
        if ((j.domain != DaiDomain::Excavation) || ((j.key & SERVICE_KEY) == 0) || DaiJobEnded(j.state) || (j.request_id == 0)) return;
        bool there = false;
        c.requests.ForEach([&](const DaiRequest &r) { there = there || (r.id == j.request_id); });
        if (!there) gone.emplace_back(j.kind, j.key);
    });
    for (const auto &g : gone) if (DaiJob *j = w.FindJob(g.first, g.second)) w.Cancel(*j, "WITHDRAWN");
    std::vector<DaiRequest> asked;
    c.requests.ForEachService(DaiDomain::Excavation, [&](const DaiRequest &r) { if (!r.served) asked.push_back(r); });
    for (const DaiRequest &r : asked) {
        const bool known = (r.service == DaiService::Dig) || (r.service == DaiService::Creep) || (r.service == DaiService::Reinforce);
        if (!known || (r.at.x < 0) || (r.at.y < 0) || (r.at.x >= c.know.Width()) || (r.at.y >= c.know.Height())) {
            w.MarkRefused(r.id);
            continue;
        }
        DaiJob *j = w.AddJob((r.service == DaiService::Dig) ? DAI_JOB_DIG_ROUTE : DAI_JOB_CREEP, SERVICE_KEY + r.id, FixFromInt(3));
        if (j == nullptr) return;           // the pool is full: next turn
        if (r.service == DaiService::Dig) {
            DaiDigRouteParams p;
            p.goals.push_back(r.at);
            p.through = true;               // the asked slab dug, not stood by
            p.claim = true;
            j->p = p;
        } else {
            DaiCreepParams p;
            p.at = r.at;
            p.imps = std::clamp<int64_t>(r.amount, 1, EXTSEAT_MAX_HAND_SET);
            p.reinforce = (r.service == DaiService::Reinforce);
            j->p = p;
        }
        j->request_id = r.id;
        w.MarkServed(r.id);
    }
}

/** The marks within reach of a hot hazard taken off (P5.13): a mark there sends imps to the trap. */
void DaiExcavation::hazards(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const int64_t reach = c.cfg.defence_hazard_reach;
    for (const DaiHazard &h : c.threat.Hazards()) {
        if (!c.threat.Hot(h)) continue;
        const DaiRect a{ (DaiSlabCoord)std::max<int64_t>(h.x - reach, 0), (DaiSlabCoord)std::max<int64_t>(h.y - reach, 0),
                         (DaiSlabCoord)std::min<int64_t>(h.x + reach, k.Width() - 1), (DaiSlabCoord)std::min<int64_t>(h.y + reach, k.Height() - 1) };
        bool marked = false;
        for (DaiSlabCoord y = a.y0; (y <= a.y1) && !marked; y++)
            for (DaiSlabCoord x = a.x0; x <= a.x1; x++) marked = marked || k.SlabAt(x, y).marked;
        const int64_t key = (int64_t)h.y * k.Width() + h.x;
        if (!marked || (w.FindJob(DAI_JOB_UNMARK_AREA, key) != nullptr)) continue;
        DaiJob *j = w.AddJob(DAI_JOB_UNMARK_AREA, key, FixFromInt(2));
        if (j != nullptr) j->p = DaiAreaParams{ a };
    }
}

void DaiExcavation::kept_marks(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    for (const DaiSlabXY &s : k.KeptSlabs()) {
        if (!k.SlabAt(s.x, s.y).marked) continue;
        const int64_t key = (int64_t)(KEPT_UNMARK_KEY + (uint64_t)s.y * (uint64_t)k.Width() + (uint64_t)s.x);
        if (w.FindJob(DAI_JOB_UNMARK_AREA, key) != nullptr) continue;
        DaiJob *j = w.AddJob(DAI_JOB_UNMARK_AREA, key, FixFromInt(3));
        if (j != nullptr) j->p = DaiAreaParams{ DaiRect{ s.x, s.y, s.x, s.y } };
    }
}

void DaiExcavation::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    services(c, w);
    probe(c, w);
    hazards(c, w);
    kept_marks(c, w);
    // No treasury yet: gold mined now would not be banked, and the imps are wanted for the treasury (Economy's first
    // job). New veins and expansions wait; work under way goes on.
    if ((c.now >= m_next_rescore) && (m_probe == 0) && has_treasury(c.know)) {
        m_next_rescore = c.now + (DaiTurn)c.cfg.excavation_rescore_interval;
        rescore(c, w);
        expand(c, w);
    }
    // Claim watch: an open route to a target the imps have not claimed within the window.
    for (ClaimWatch &cw : m_watch) {
        if (cw.key < 0) continue;
        const DaiGoal *g = c.goals.Find(DaiGoalKind::ExpandTo, cw.key);
        const DaiSlabCoord x = (DaiSlabCoord)(cw.key % std::max<DaiSlabCoord>(c.know.Width(), 1));
        const DaiSlabCoord y = (DaiSlabCoord)(cw.key / std::max<DaiSlabCoord>(c.know.Width(), 1));
        if ((g == nullptr) || (c.know.SlabAt(x, y).owner == c.player)) {
            if (g != nullptr) w.NoteGoal(g->id, DaiGoalStatus::Met, "");
            cw = ClaimWatch();
        } else if (c.now >= cw.since + (DaiTurn)c.cfg.excavation_claim_window) {
            w.NoteGoal(g->id, DaiGoalStatus::Blocked, "NOT_CLAIMED");
            cw = ClaimWatch();
        }
    }
}

void DaiExcavation::rescore(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiGoal *mine = c.goals.Find(DaiGoalKind::Mine);
    if ((mine == nullptr) || (mine->weight <= 0)) return;
    const DaiKnowledge &k = c.know;
    // Veins worked now (a live route or mining job) and how many imps each may count on.
    int64_t worked = 0;
    k.ForEachVein([&](const DaiVein &v) {
        if (live(w.FindJob(DAI_JOB_DIG_ROUTE, v.id)) || live(w.FindJob(DAI_JOB_MINE_VEIN, v.id))) worked++;
    });
    const int64_t room = std::min<int64_t>(c.cfg.excavation_max_veins, std::max<int64_t>(c.know.Own().imps / IMPS_A_VEIN, 1));
    if ((worked >= room) || (m_probe != 0)) return;
    const int64_t imps_available = std::max<int64_t>(k.Own().imps / (worked + 1), 1);
    // Where the gold goes: the treasury, or the heart while there is none.
    const RoomKind tkind = k.RoomKindOf(DaiRoom::Treasury);
    DaiSlabXY haul_to{ DaiSubtileSlab(k.Own().heart_x), DaiSubtileSlab(k.Own().heart_y) };
    for (const DaiRoomLook &r : k.Rooms()) if ((tkind != 0) && (r.kind == tkind)) { haul_to = DaiSlabXY{ r.cx, r.cy }; break; }
    const std::vector<int32_t> d = distance_from_reach(k);
    struct Pick { uint32_t vein = 0; int64_t score = -1; DaiSlabXY entry; bool face = false; };
    Pick best;
    // Mine's count: in contest, veins this many slabs from the heart at most (0: anywhere; 05 section 6.2).
    const DaiSlabCoord hx = DaiSubtileSlab(k.Own().heart_x), hy = DaiSubtileSlab(k.Own().heart_y);
    k.ForEachVein([&](const DaiVein &v) {
        if ((v.status == DaiVeinStatus::Exhausted) || (v.Slabs() == 0)) return;
        if ((mine->p.count > 0) && (std::max(std::abs(v.cx - hx), std::abs(v.cy - hy)) > mine->p.count)) return;
        if ((v.status == DaiVeinStatus::Unreachable) && (c.now < v.retry_after)) return;
        if (live(w.FindJob(DAI_JOB_DIG_ROUTE, v.id)) || live(w.FindJob(DAI_JOB_MINE_VEIN, v.id))) return;
        // Its slab nearest the imps: the entry, and the straight-line route to it through what would be dug.
        DaiSlabXY entry{ v.cx, v.cy };
        int32_t near = INT32_MAX;
        for (DaiSlabCoord y = v.box.y0; y <= v.box.y1; y++) {
            for (DaiSlabCoord x = v.box.x0; x <= v.box.x1; x++) {
                const DaiVein *at = k.VeinAt(x, y);
                const int32_t dd = d[(size_t)y * k.Width() + x];
                if ((at != nullptr) && (at->id == v.id) && (dd < near)) { near = dd; entry = DaiSlabXY{ x, y }; }
            }
        }
        if (near == INT32_MAX) return;
        const int64_t route = (int64_t)std::max(near - 1, 0) * c.cfg.dig_earth;
        const int64_t s = VeinScore(c, v, route, entry, haul_to, imps_available);
        if (s > best.score) best = Pick{ v.id, s, entry, near <= 1 };
    });
    if (best.vein == 0) return;
    DaiVein *v = w.Vein(best.vein);
    if (v == nullptr) return;
    v->ex = best.entry.x;
    v->ey = best.entry.y;
    if (best.face) {
        DaiJob *j = w.AddJob(DAI_JOB_MINE_VEIN, best.vein, mine->weight);
        if (j == nullptr) return;
        DaiMineVeinParams p;
        p.vein = best.vein;
        p.entry = best.entry;
        j->p = p;
        v->status = DaiVeinStatus::Mining;
    } else {
        m_probe = best.vein;            // a route is searched first (probe): no job until one is found
        v->status = DaiVeinStatus::Routing;
    }
    DaiLog::Debug(c.player, "vein %u: %s from (%d,%d), score %" PRId64, best.vein, best.face ? "mining" : "routing",
                  (int)best.entry.x, (int)best.entry.y, best.score);
}

/** The vein slabs (the route's goals). */
static std::vector<DaiSlabXY> vein_slabs(const DaiKnowledge &k, uint32_t vein)
{
    std::vector<DaiSlabXY> out;
    const DaiVein *v = k.Vein(vein);
    if (v == nullptr) return out;
    for (DaiSlabCoord y = v->box.y0; y <= v->box.y1; y++)
        for (DaiSlabCoord x = v->box.x0; x <= v->box.x1; x++)
            if ((k.VeinAt(x, y) != nullptr) && (k.VeinAt(x, y)->id == vein)) out.push_back(DaiSlabXY{ x, y });
    return out;
}

/** The route probe: a search for the chosen vein, stepped within the turn's budget. Found: a dig_route job with the
 *  path; none: the vein is Unreachable for a while -- no job is made to fail. */
void DaiExcavation::probe(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if (m_probe == 0) return;
    DaiVein *v = w.Vein(m_probe);
    if (v == nullptr) {                 // the vein went (mined out, or the table changed)
        c.searches.Release(PROBE_KEY);
        m_probe = 0;
        return;
    }
    bool fresh = false;
    DaiDigSearch *s = c.searches.Get(PROBE_KEY, c.cfg.budgets_max_searches, &fresh);
    if (s == nullptr) return;           // every search is taken: next turn
    const std::vector<DaiSlabXY> goals = vein_slabs(c.know, m_probe);
    if (fresh) s->StartFromReach(c.know, c.player, goals, DaiDigCosts::OwnRoute(c.cfg, c.personality, c.know));
    const DaiResult r = s->Step(c.know, c.budget);
    if (r == DaiResult::Wait) return;
    const uint32_t vein = m_probe;
    m_probe = 0;
    if (r != DaiResult::Done) {
        c.searches.Release(PROBE_KEY);
        v->status = DaiVeinStatus::Unreachable;
        v->retry_after = c.now + (DaiTurn)c.cfg.excavation_unreachable_retry;
        DaiLog::Debug(c.player, "vein %u: no route", vein);
        return;
    }
    DaiDigRouteParams p;
    p.goals = goals;
    p.path.assign(s->Path().begin(), s->Path().end());
    p.path_cost = s->Cost();
    c.searches.Release(PROBE_KEY);
    v->route_cost = (int32_t)std::min<int64_t>(p.path_cost, INT32_MAX);
    const DaiGoal *mine = c.goals.Find(DaiGoalKind::Mine);
    DaiJob *j = w.AddJob(DAI_JOB_DIG_ROUTE, vein, (mine != nullptr) ? mine->weight : DAI_FIX_ONE);
    if (j == nullptr) {
        v->status = DaiVeinStatus::Untouched;
        return;
    }
    j->p = p;
}

/** A claim route for each ExpandTo goal that has none and is not met or waiting on a claim. */
void DaiExcavation::expand(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    // A route whose goal is gone (the room another keeper's now: level 52's portals) ends -- its dig would find no way.
    std::vector<uint64_t> gone;
    c.jobs.ForEach([&](const DaiJob &j) {
        if ((j.kind == DAI_JOB_DIG_ROUTE) && ((j.key & EXPAND_KEY) != 0) && live(&j)
            && (c.goals.Find(DaiGoalKind::ExpandTo, (int64_t)(j.key & ~EXPAND_KEY)) == nullptr)) gone.push_back(j.key);
    });
    for (uint64_t key : gone) if (DaiJob *j = w.FindJob(DAI_JOB_DIG_ROUTE, key)) w.Cancel(*j, "GOAL_GONE");
    // One expansion at a time: each is a long claim route, and two halve the imps on both.
    bool busy = false;
    c.goals.ForEach([&](const DaiGoal &g) {
        if ((g.kind == DaiGoalKind::ExpandTo) && live(w.FindJob(DAI_JOB_DIG_ROUTE, EXPAND_KEY | (uint64_t)g.p.key))) busy = true;
    });
    for (const ClaimWatch &cw : m_watch) busy = busy || (cw.key >= 0);
    if (busy) return;
    c.goals.ForEach([&](const DaiGoal &g) {
        if (busy) return;
        if ((g.kind != DaiGoalKind::ExpandTo) || (g.status == DaiGoalStatus::Met) || (g.p.key < 0)) return;
        if ((g.status == DaiGoalStatus::Blocked) && (c.now < g.status_turn + (DaiTurn)c.cfg.excavation_claim_window)) return;
        const uint64_t key = EXPAND_KEY | (uint64_t)g.p.key;
        if (live(w.FindJob(DAI_JOB_DIG_ROUTE, key))) return;
        for (const ClaimWatch &cw : m_watch) if (cw.key == g.p.key) return;
        const DaiSlabCoord x = (DaiSlabCoord)(g.p.key % std::max<DaiSlabCoord>(k.Width(), 1));
        const DaiSlabCoord y = (DaiSlabCoord)(g.p.key / std::max<DaiSlabCoord>(k.Width(), 1));
        DaiSlabXY slabs[64];
        int64_t n = 0;
        if (g.p.count == DAI_EXPAND_POINT) {
            // A script's DAIMON_DIG: its slab, done once the keeper's ground reaches it.
            const DaiSlabInfo s = k.SlabAt(x, y);
            if ((s.owner == c.player) && (s.terrain == DaiTerrain::Floor)) {
                w.NoteGoal(g.id, DaiGoalStatus::Met, "");
                return;
            }
            slabs[0] = DaiSlabXY{ x, y };
            n = 1;
        } else {
            n = k.RoomCluster(x, y, slabs, 64);
        }
        if (n == 0) return;
        DaiJob *j = w.AddJob(DAI_JOB_DIG_ROUTE, key, g.weight);
        if (j == nullptr) return;
        DaiDigRouteParams p;
        p.claim = true;
        p.goals.assign(slabs, slabs + n);
        j->p = p;
        j->goal_id = g.id;
        w.NoteGoal(g.id, DaiGoalStatus::Progress, "");
        busy = true;
    });
}

void DaiExcavation::OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w)
{
    if ((j.key & SERVICE_KEY) != 0) {
        if ((j.state != DaiJobState::Done) && (j.request_id != 0)) w.MarkRefused(j.request_id);
        if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
        return;
    }
    if ((j.key & EXPAND_KEY) != 0) {
        const int64_t key = (int64_t)(j.key & ~EXPAND_KEY);
        const DaiGoal *g = c.goals.Find(DaiGoalKind::ExpandTo, key);
        if ((j.state == DaiJobState::Done) && (g != nullptr) && (g->p.count == DAI_EXPAND_POINT)) {
            w.NoteGoal(g->id, DaiGoalStatus::Met, "");
        } else if (j.state == DaiJobState::Done) {
            for (ClaimWatch &cw : m_watch) {
                if (cw.key >= 0) continue;
                cw = ClaimWatch{ key, c.now };
                break;
            }
        } else if (g != nullptr) {
            w.NoteGoal(g->id, DaiGoalStatus::Blocked, (j.why[0] != '\0') ? j.why : "NO_ROUTE");
        }
        if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
        return;
    }
    const uint32_t vein = (uint32_t)j.key;
    DaiVein *v = w.Vein(vein);
    if (j.kind == DAI_JOB_DIG_ROUTE) {
        if ((j.state == DaiJobState::Done) && (v != nullptr)) {
            // The route is open: mine from where it ends.
            DaiMineVeinParams p;
            p.vein = vein;
            const DaiDigRouteParams *r = std::get_if<DaiDigRouteParams>(&j.p);
            p.entry = ((r != nullptr) && !r->path.empty()) ? r->path.back() : DaiSlabXY{ v->ex, v->ey };
            DaiJob *m = w.AddJob(DAI_JOB_MINE_VEIN, vein, j.utility);
            if (m != nullptr) m->p = p;
            v->status = DaiVeinStatus::Mining;
            if (const DaiDigRouteParams *rp = std::get_if<DaiDigRouteParams>(&j.p)) v->route_cost = (int32_t)std::min<int64_t>(rp->path_cost, INT32_MAX);
        } else if (v != nullptr) {
            // No way there (or it stalled): not again for a while.
            v->status = DaiVeinStatus::Unreachable;
            v->retry_after = c.now + (DaiTurn)c.cfg.excavation_unreachable_retry;
        }
    } else if ((j.kind == DAI_JOB_MINE_VEIN) && (v != nullptr)) {
        if (j.state == DaiJobState::Done) {
            v->status = DaiVeinStatus::Exhausted;
        } else if (j.state == DaiJobState::Blocked) {
            // Mined nothing for a stall window (no imp gets to it): not chosen again for a while.
            v->status = DaiVeinStatus::Unreachable;
            v->retry_after = c.now + (DaiTurn)c.cfg.excavation_unreachable_retry;
        } else {
            v->status = DaiVeinStatus::Untouched;
        }
    }
    if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
}

void DaiExcavation::Reset()
{
    m_next_rescore = 0;
    m_probe = 0;
    m_watch.fill(ClaimWatch());
}

void DaiExcavation::Save(DaiBlobWriter &w) const
{
    w.U64(m_next_rescore);
    w.U32(m_probe);
    for (const ClaimWatch &cw : m_watch) { w.I64(cw.key); w.U64(cw.since); }
}

bool DaiExcavation::Load(DaiBlobReader &r)
{
    m_next_rescore = (DaiTurn)r.U64();
    m_probe = r.U32();
    for (ClaimWatch &cw : m_watch) { cw.key = r.Range(-1, 256 * 256); cw.since = (DaiTurn)r.U64(); }
    return r.Ok();
}
