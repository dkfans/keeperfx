// dig_route (03 section 7.3): a route to a target, searched, then dug, bridged and walked in order. The path runs from
// where the imps stand (or, for a claim route, from the keeper's land) to the goal slab; the slabs between are the
// work (with `through`, the goal slab too: it is dug, not stood by). Dig runs are marked at most marks_ahead slabs past the last one dug; a bridge waits for the slab before it to
// be the keeper's land (imps claim it on their own) and for the gold. A planned slab that cannot be passed any more,
// or a rest grown dearer than replan_ratio times its plan, starts a new search.
#include "pre_inc.h"
#include "JobKinds.h"

#include "JobPath.h"
#include "daimon/DaiDigSearch.h"
#include "daimon/DaiGeom.h"
#include "daimon/DaiLog.h"

#include <algorithm>
#include <cinttypes>
#include "post_inc.h"

namespace {
/** Turns a job with nothing to do now waits before it is tried again. */
constexpr DaiTurn WAIT_TURNS = 20;
/** Fetches a route takes at most (a water crossing takes several: each shows a few slabs). */
constexpr int64_t MAX_DROPS = 32;

DaiDigCosts costs_for(const DaiTurnCtx &c, bool claim)
{
    return claim ? DaiDigCosts::ClaimRoute(c.cfg, c.personality, c.know) : DaiDigCosts::OwnRoute(c.cfg, c.personality, c.know);
}

/** One past the path's last work slab: the goal's index, or (through) the path's size. */
int64_t work_end(const DaiDigRouteParams &p)
{
    return (int64_t)p.path.size() - (p.through ? 0 : 1);
}

/** The first route slab (between the source and the goal, or through it) not finished yet; work_end if none. */
int64_t first_pending(const DaiTurnCtx &c, const DaiDigRouteParams &p)
{
    const int64_t end = work_end(p);
    for (int64_t i = 1; i < end; i++) {
        const DaiSlabXY &s = p.path[(size_t)i];
        const JobStep st = JobStepAt(c.know, c.player, s.x, s.y, p.claim);
        if ((st == JobStep::Dig) || (st == JobStep::Blocked)) return i;
        if ((st == JobStep::Bridge) && !JobBridged(c.know, c.player, s.x, s.y)) return i;
    }
    return (end > 0) ? end : 0;
}

/** The dig run a gesture would mark from `from`: dig slabs not marked yet, within marks_ahead of `from`. */
/** A path's identity (FNV-1a over its slabs): a re-plan that finds the same path again is not a new way. */
uint64_t path_hash(const std::vector<DaiSlabXY> &path)
{
    uint64_t h = 1469598103934665603ULL;
    for (const DaiSlabXY &s : path) {
        h = (h ^ (uint64_t)(uint32_t)s.x) * 1099511628211ULL;
        h = (h ^ (uint64_t)(uint32_t)s.y) * 1099511628211ULL;
    }
    return h;
}

int64_t dig_run(const DaiTurnCtx &c, const DaiDigRouteParams &p, int64_t from, DaiRect *rect)
{
    const int64_t end = work_end(p);
    const int64_t window_end = std::min<int64_t>(from + c.cfg.excavation_marks_ahead, end);
    auto pending_dig = [&](int64_t i) {
        if ((i >= window_end) || (i >= end)) return false;
        const DaiSlabXY &s = p.path[(size_t)i];
        return JobStepAt(c.know, c.player, s.x, s.y, p.claim) == JobStep::Dig;
    };
    for (int64_t i = from; i < window_end; i++) {
        if (!pending_dig(i)) break;     // the dig run ends at the first slab that is not digging (a bridge, a walk)
        const DaiSlabXY &s = p.path[(size_t)i];
        if (c.know.SlabAt(s.x, s.y).marked) continue;
        return JobRun(p.path, i, window_end - i, [&](int64_t k) {
            const DaiSlabXY &t = p.path[(size_t)k];
            return pending_dig(k) && !c.know.SlabAt(t.x, t.y).marked;
        }, rect);
    }
    return 0;
}

class JobDigRoute : public IDaiJobKind {
public:
    const char *GetName() const override { return "dig_route"; }
    DaiTurn MinStallWindow() const override { return DAI_DIG_STALL_WINDOW; }
    DaiDomain GetDomain() const override { return DaiDomain::Excavation; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        DaiDigRouteParams *p = std::get_if<DaiDigRouteParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        j.needs = DaiJobNeeds();
        if (p->path.empty()) {
            bool fresh = false;
            DaiDigSearch *s = c.searches.Get(j.id, c.cfg.budgets_max_searches, &fresh);
            if (s == nullptr) return j.state;               // every search is taken: wait for one
            if (fresh) {
                if (p->claim) s->StartFromOwnLand(c.know, c.player, p->goals, costs_for(c, true));
                else s->StartFromReach(c.know, c.player, p->goals, costs_for(c, false));
            }
            const DaiResult r = s->Step(c.know, c.budget);
            if (r == DaiResult::Wait) return j.state;
            c.searches.Release(j.id);
            if (r != DaiResult::Done) {
                snprintf(why, why_len, "NO_ROUTE");
                if (!p->goals.empty()) {
                    const DaiSlabInfo g = c.know.SlabAt(p->goals[0].x, p->goals[0].y);
                    DaiLog::Info(c.player, "j%u dig_route: no way to (%d,%d) (terrain %d, owner %d, revealed %d)", j.id, (int)p->goals[0].x,
                        (int)p->goals[0].y, (int)g.terrain, (int)g.owner, (int)g.revealed);
                }
                return DaiJobState::Failed;
            }
            p->path.assign(s->Path().begin(), s->Path().end());
            p->path_cost = s->Cost();
            const uint64_t hash = path_hash(p->path);
            if ((p->replans > 0) && (hash != p->path_hash)) p->new_ways++;
            p->path_hash = hash;
        }
        // How the plan stands: what is finished, whether the rest can still be passed, and what it costs now against
        // what those same slabs cost when the path was planned.
        const DaiDigCosts costs = costs_for(c, p->claim);
        const int64_t last = (int64_t)p->path.size() - 1, end = work_end(*p);
        const bool planned = (p->plan_costs.size() == p->path.size());
        if (!planned) p->plan_costs.assign(p->path.size(), 0);
        int64_t done = 0, rest_now = 0, planned_rest = 0, gold = 0;
        bool blocked = false;
        for (int64_t i = 1; i <= last; i++) {
            const DaiSlabXY &s = p->path[(size_t)i];
            const int32_t cost = DaiDigSearch::EnterCost(c.know, c.player, costs, s.x, s.y, i == last);
            if (!planned) p->plan_costs[(size_t)i] = cost;
            const JobStep st = JobStepAt(c.know, c.player, s.x, s.y, p->claim);
            const bool finished = (i < end) && ((st == JobStep::Walk) || ((st == JobStep::Bridge) && JobBridged(c.know, c.player, s.x, s.y)));
            if (finished) {
                done++;
                const auto same = [&](const DaiSlabXY &f) { return (f.x == s.x) && (f.y == s.y); };
                if (std::find_if(p->finished.begin(), p->finished.end(), same) == p->finished.end()) p->finished.push_back(s);
                continue;
            }
            if ((cost >= DAI_DIG_NEVER) && !JobBridged(c.know, c.player, s.x, s.y)) blocked = true;
            else if (i < end) { rest_now += cost; planned_rest += p->plan_costs[(size_t)i]; }
            if (i < end) gold += c.know.GoldLeft(s.x, s.y);
        }
        (void)done;
        p->done = (int64_t)p->finished.size();
        if ((p->gold_seen >= 0) && (gold < p->gold_seen)) p->gold_mined += p->gold_seen - gold;
        p->gold_seen = gold;
        if (blocked || ((planned_rest > 0) && (rest_now * DAI_FIX_ONE > planned_rest * c.cfg.excavation_replan_ratio))) {
            p->plan_costs.clear();
            p->path.clear();
            p->replans++;
            p->gold_seen = -1;
            return j.state;
        }
        // The next gesture's gold: a bridge run that can start now.
        const int64_t at = first_pending(c, *p);
        // A fetch: the next slab marked and left undug -- an imp dropped on the keeper's land beside it takes the work
        // where it lands (level 52: a breach's mark across the water was never taken while 130 mining marks stood nearer
        // home), and shows the ground about it (the water beyond a bridge was never seen: the mark on it never taken,
        // and the route planned through it again on every try).
        if (p->drop_gestures >= 0) {
            const bool dropped = ((int64_t)j.gestures > p->drop_gestures);
            if (dropped || (c.now >= p->drop_turn + JOB_DROP_TURNS)) {
                DaiLog::Info(c.player, "j%u dig_route fetch %" PRId64 " %s", j.id, p->drops, dropped ? "dropped" : "not dropped");
                p->drop_gestures = -1;
            }
        }
        if ((p->drop_gestures < 0) && (p->drops < MAX_DROPS) && (at > 0) && (at < end)
            && (JobStepAt(c.know, c.player, p->path[(size_t)at].x, p->path[(size_t)at].y, p->claim) == JobStep::Dig)) {
            const DaiSlabXY &a = p->path[(size_t)at];
            const DaiSlabInfo sa = c.know.SlabAt(a.x, a.y);
            const DaiTurn quiet = std::max<DaiTurn>(std::max<DaiTurn>(j.last_progress_turn, j.created), p->drop_turn);
            DaiSlabXY site;
            if (sa.marked && (c.now >= quiet + JOB_DROP_AFTER) && JobDropSite(c.know, c.player, &p->path[(size_t)at - 1], a, &site)) {
                if (const DaiImpLook *imp = JobDropImp(c.know, site)) {
                    p->drop_imp = imp->ref;
                    p->drop_at = site;
                    p->drop_turn = c.now;
                    p->drop_gestures = (int64_t)j.gestures;
                    p->drops++;
                    DaiLog::Info(c.player, "j%u dig_route fetch %" PRId64 ": imp %d to (%d,%d) for (%d,%d)%s", j.id, p->drops, (int)imp->ref.idx,
                        (int)site.x, (int)site.y, (int)a.x, (int)a.y, sa.revealed ? "" : ", never seen");
                }
            }
        }
        if (p->drop_gestures >= 0) {
            j.needs.hand = true;
            j.needs.thing = p->drop_imp;
            return j.state;
        }
        if ((at > 0) && (at < end) && (JobStepAt(c.know, c.player, p->path[(size_t)at].x, p->path[(size_t)at].y, p->claim) == JobStep::Bridge)) {
            DaiRect r;
            const int64_t n = JobBridgeRun(c.know, c.player, p->path, at, p->claim, false, &r);
            j.needs.gold = n * c.know.Build(DaiRoom::Bridge).price;
            if (n == 0) j.retry_after = c.now + WAIT_TURNS;     // the claim wait
        } else if ((at > 0) && (at < end)) {
            DaiRect r;
            if (JobMarksFull(c) || (dig_run(c, *p, at, &r) == 0)) j.retry_after = c.now + WAIT_TURNS;   // the imps are at it
            else JobMarkRepeated(p->marks, r, j, c.now);
        }
        return j.state;
    }

    int64_t Watch(const DaiTurnCtx &c, const DaiJob &j, DaiSlabXY *out, int64_t max) const override
    {
        // The next slab to finish and the one before it (a bank to be claimed), and the goal.
        const DaiDigRouteParams *p = std::get_if<DaiDigRouteParams>(&j.p);
        if ((p == nullptr) || (p->path.size() < 2)) return 0;
        const int64_t at = first_pending(c, *p);
        int64_t n = 0;
        if ((n < max) && (at > 0)) out[n++] = p->path[(size_t)at - 1];
        if ((n < max) && (at < work_end(*p)) && (at < (int64_t)p->path.size() - 1)) out[n++] = p->path[(size_t)at];
        if (n < max) out[n++] = p->path.back();
        return n;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiDigRouteParams *p = std::get_if<DaiDigRouteParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        if (p->path.size() < 2) return DaiResult::Wait;        // searching
        const int64_t at = first_pending(c, *p);
        if (p->drop_gestures >= 0) {
            memset(verb, 0, sizeof(*verb));
            verb->kind = ESV_PickUpAndDrop;
            verb->thing_count = 1;
            verb->thing_ids[0] = p->drop_imp.idx;
            verb->has_pos = true;
            verb->stl_x = DaiSlabSubtileCenter(p->drop_at.x);
            verb->stl_y = DaiSlabSubtileCenter(p->drop_at.y);
            return DaiResult::Ok;
        }
        if ((at <= 0) || (at >= work_end(*p))) return DaiResult::Wait;  // the route is open: Achieved follows the reach map
        DaiRect r;
        switch (JobStepAt(c.know, c.player, p->path[(size_t)at].x, p->path[(size_t)at].y, p->claim)) {
        case JobStep::Dig:
            if (JobMarksFull(c) || (dig_run(c, *p, at, &r) == 0)) return DaiResult::Wait;
            JobMarkDigVerb(verb, r);
            return DaiResult::Ok;
        case JobStep::Bridge:
            if (JobBridgeRun(c.know, c.player, p->path, at, p->claim, false, &r) == 0) return DaiResult::Wait;
            JobBuildRoomVerb(verb, DaiRoom::Bridge, r);
            return DaiResult::Ok;
        default:
            return DaiResult::Wait;
        }
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        // Slabs finished, re-plans (a new way, not a stall), the gold mined from the path, and for a claim route the
        // path's slabs the keeper's land has claimed since (the claim wait is progress too).
        const DaiDigRouteParams *p = std::get_if<DaiDigRouteParams>(&j.p);
        if (p == nullptr) return 0;
        int64_t claimed = 0;
        if (p->claim)
            for (const DaiSlabXY &s : p->path) claimed += JobOwnLand(c.know, c.player, s.x, s.y) ? 1 : 0;
        return (int64_t)p->finished.size() + p->new_ways + claimed + p->gold_mined;
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiDigRouteParams *p = std::get_if<DaiDigRouteParams>(&j.p);
        if ((p == nullptr) || (p->path.size() < 2)) return false;
        const int64_t last = (int64_t)p->path.size() - 1;
        if (first_pending(c, *p) < work_end(*p)) return false;
        const DaiSlabXY &goal = p->path[(size_t)last];
        // Own route: the imps can stand by the goal. Claim route: the keeper's land has reached it -- the imps claimed
        // the way, so the target can be claimed or built on now. An open way not yet claimed is not done: what asked
        // for the route would ask again (a land link made every turn).
        if (!p->claim) return c.know.Reach().NextToReached(goal.x, goal.y) || c.know.Reach().Reached(goal.x, goal.y);
        const DaiSlabXY &before = p->path[(size_t)last - 1];
        return JobOwnLand(c.know, c.player, before.x, before.y) || JobOwnLand(c.know, c.player, goal.x, goal.y);
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j; (void)r;
        return DaiJobState::Ready;      // more runs to come; a refusal is tried again (the stall rule ends a hopeless one)
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiDigRouteParams *p = std::get_if<DaiDigRouteParams>(&j.p);
        const DaiDigRouteParams none;
        const DaiDigRouteParams &q = (p != nullptr) ? *p : none;
        w.I64((int64_t)q.goals.size());
        for (const DaiSlabXY &g : q.goals) { w.I64(g.x); w.I64(g.y); }
        w.Bool(q.claim);
        w.Bool(q.through);
        w.I64((int64_t)q.path.size());
        for (const DaiSlabXY &s : q.path) { w.I64(s.x); w.I64(s.y); }
        w.I64(q.path_cost);
        w.I64((int64_t)q.plan_costs.size());
        for (int32_t pc : q.plan_costs) w.I64(pc);
        w.I64((int64_t)q.finished.size());
        for (const DaiSlabXY &f : q.finished) { w.I64(f.x); w.I64(f.y); }
        w.I64(q.replans);
        w.I64(q.new_ways);
        w.U64(q.path_hash);
        w.I64(q.gold_seen);
        w.I64(q.gold_mined);
        JobSaveMarks(w, q.marks);
        w.Ref(q.drop_imp);
        w.I64(q.drop_at.x);
        w.I64(q.drop_at.y);
        w.U64(q.drop_turn);
        w.I64(q.drop_gestures);
        w.I64(q.drops);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiDigRouteParams p;
        const int64_t ng = r.Count(1024);
        for (int64_t i = 0; (i < ng) && r.Ok(); i++) p.goals.push_back(DaiSlabXY{ (DaiSlabCoord)r.Range(0, 255), (DaiSlabCoord)r.Range(0, 255) });
        p.claim = r.Bool();
        p.through = r.Bool();
        const int64_t np = r.Count(256 * 256);
        for (int64_t i = 0; (i < np) && r.Ok(); i++) p.path.push_back(DaiSlabXY{ (DaiSlabCoord)r.Range(0, 255), (DaiSlabCoord)r.Range(0, 255) });
        p.path_cost = r.Range(0, INT64_MAX);
        const int64_t nc = r.Count((int64_t)p.path.size());
        for (int64_t i = 0; (i < nc) && r.Ok(); i++) p.plan_costs.push_back((int32_t)r.Range(0, INT32_MAX));
        const int64_t nf = r.Count(256 * 256);
        for (int64_t i = 0; (i < nf) && r.Ok(); i++) p.finished.push_back(DaiSlabXY{ (DaiSlabCoord)r.Range(0, 255), (DaiSlabCoord)r.Range(0, 255) });
        p.done = (int64_t)p.finished.size();
        p.replans = r.Range(0, INT64_MAX);
        p.new_ways = r.Range(0, p.replans);
        p.path_hash = r.U64();
        p.gold_seen = r.Range(-1, INT64_MAX);
        p.gold_mined = r.Range(0, INT64_MAX);
        JobLoadMarks(r, &p.marks);
        p.drop_imp = r.Ref();
        p.drop_at.x = (DaiSlabCoord)r.Range(0, 255);
        p.drop_at.y = (DaiSlabCoord)r.Range(0, 255);
        p.drop_turn = (DaiTurn)r.U64();
        p.drop_gestures = r.Range(-1, INT64_MAX);
        p.drops = r.Range(0, MAX_DROPS);
        j.p = p;
        return r.Ok();
    }
};

const JobDigRoute s_dig_route;
}

const IDaiJobKind &JobDigRouteKind()
{
    return s_dig_route;
}
