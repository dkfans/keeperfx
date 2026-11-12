// creep (07 sections 2.1, 4.4): another keeper's slab -- its claimed path, or a room's (a whole room flips, a portal
// too) -- beside the keeper's own land is converted by the keeper's imps on their own: the engine lists a convert task
// for every enemy slab its land touches. The job only brings imps there: up to `imps` of them, the idle first, dropped
// on the keeper's own ground by the slab a hand-load at a time, again once the last drop is LIFT_GAP turns old and
// fewer than that stand near (the defenders kill them, or they wander to other work). Done when the slab is the
// keeper's; failed when no slab of its own land touches it any more (the foothold lost). Reinforce (plan 13 section
// 32): the same for earth to fortify -- one imp dropped on the foothold slab itself (a dropped imp reinforces the earth
// beside where it lands), done when the earth is the keeper's fortified wall.
#include "pre_inc.h"
#include "JobKinds.h"

#include <algorithm>
#include <cstring>

#include "daimon/DaiDropSpot.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
/** Turns after a drop before imps are brought again. */
constexpr DaiTurn LIFT_GAP = 300;
/** An imp this many slabs or fewer from the slab is there. */
constexpr int64_t NEAR = 2;
/** A drop lands within this many subtiles of the foothold's middle. */
constexpr int64_t DROP_RADIUS = 3;

/** A slab of the keeper's own land (floor or room) beside `at`, 4 ways; false if none. */
bool foothold(const DaiKnowledge &k, PlayerNumber p, DaiSlabXY at, DaiSlabXY *out)
{
    const int *const dx = DAI_N4_DX, *const dy = DAI_N4_DY;
    for (int d = 0; d < 4; d++) {
        const DaiSlabCoord x = (DaiSlabCoord)(at.x + dx[d]), y = (DaiSlabCoord)(at.y + dy[d]);
        const DaiSlabInfo s = k.SlabAt(x, y);
        if ((s.owner == p) && ((s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Room))) { *out = DaiSlabXY{ x, y }; return true; }
    }
    return false;
}

class JobCreep : public IDaiJobKind {
public:
    const char *GetName() const override { return "creep"; }
    DaiDomain GetDomain() const override { return DaiDomain::Excavation; }
    DaiTurn MinStallWindow() const override { return DAI_DIG_STALL_WINDOW; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        DaiCreepParams *p = std::get_if<DaiCreepParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        const DaiKnowledge &k = c.know;
        DaiSlabXY fh;
        if (!foothold(k, c.player, p->at, &fh)) {
            snprintf(why, why_len, "NO_FOOTHOLD");
            return DaiJobState::Failed;
        }
        // After a drop the job runs until the next is due.
        if ((j.state == DaiJobState::Running) && (c.now >= p->lifted + LIFT_GAP)) return DaiJobState::Ready;
        if (j.state != DaiJobState::Ready) return j.state;
        j.needs = DaiJobNeeds();
        p->things.clear();
        std::vector<const DaiImpLook *> far;
        int64_t near = 0;
        for (const DaiImpLook &i : k.OwnImps()) {
            if (DaiChess(DaiSubtileSlab(i.x), DaiSubtileSlab(i.y), p->at.x, p->at.y) <= NEAR) near++;
            else if (!i.training && i.pickable) far.push_back(&i);       // not a trainee (P9)
        }
        const int64_t want = std::min<int64_t>(p->imps - near, EXTSEAT_MAX_HAND_SET);
        if ((want <= 0) || far.empty()) {
            j.retry_after = c.now + LIFT_GAP / 4;
            return j.state;
        }
        std::stable_sort(far.begin(), far.end(), [](const DaiImpLook *a, const DaiImpLook *b) {
            return (a->working != b->working) ? !a->working : (a->ref.idx < b->ref.idx);
        });
        DaiSubtlCoord x = 0, y = 0;
        if (DaiFindDropSpot(k, c.player, DaiSlabSubtileCenter(fh.x), DaiSlabSubtileCenter(fh.y), p->reinforce ? 1 : DROP_RADIUS, -1, -1, &x, &y) != nullptr) {
            j.retry_after = c.now + LIFT_GAP / 4;
            return j.state;
        }
        for (const DaiImpLook *i : far) if ((int64_t)p->things.size() < want) p->things.push_back(i->ref);
        p->x = x;
        p->y = y;
        p->lifted = c.now;
        j.needs.hand = true;
        j.needs.thing = p->things[0];
        return j.state;
    }

    int64_t Watch(const DaiTurnCtx &c, const DaiJob &j, DaiSlabXY *out, int64_t max) const override
    {
        (void)c;
        const DaiCreepParams *p = std::get_if<DaiCreepParams>(&j.p);
        if ((p == nullptr) || (max < 1)) return 0;
        out[0] = p->at;
        return 1;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        (void)c; (void)why; (void)why_len;
        const DaiCreepParams *p = std::get_if<DaiCreepParams>(&j.p);
        if ((p == nullptr) || p->things.empty()) return DaiResult::Wait;
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_PickUpAndDrop;
        verb->thing_count = std::min<int64_t>((int64_t)p->things.size(), EXTSEAT_MAX_HAND_SET);
        for (int64_t i = 0; i < verb->thing_count; i++) verb->thing_ids[i] = p->things[(size_t)i].idx;
        verb->has_pos = true;
        verb->stl_x = p->x;
        verb->stl_y = p->y;
        return DaiResult::Ok;
    }

    /** The keeper's slabs around the target: they grow as the imps convert. */
    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiCreepParams *p = std::get_if<DaiCreepParams>(&j.p);
        if (p == nullptr) return 0;
        int64_t n = 0;
        for (DaiSlabCoord y = p->at.y - NEAR; y <= p->at.y + NEAR; y++)
            for (DaiSlabCoord x = p->at.x - NEAR; x <= p->at.x + NEAR; x++) n += (c.know.SlabAt(x, y).owner == c.player) ? 1 : 0;
        return n;
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiCreepParams *p = std::get_if<DaiCreepParams>(&j.p);
        return (p != nullptr) && (c.know.SlabAt(p->at.x, p->at.y).owner == c.player);
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Ready : DaiJobState::Running;    // a refused drop is tried again; the stall rule ends a hopeless job
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiCreepParams *p = std::get_if<DaiCreepParams>(&j.p);
        const DaiCreepParams v = (p != nullptr) ? *p : DaiCreepParams();
        w.I64(v.at.x); w.I64(v.at.y);
        w.I64(v.imps);
        w.U64(v.lifted);
        w.I64((int64_t)v.things.size());
        for (const DaiThingRef &r : v.things) { w.Ref(r); }
        w.I64(v.x); w.I64(v.y);
        w.Bool(v.reinforce);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiCreepParams p;
        p.at.x = (DaiSlabCoord)r.Range(-1, 255); p.at.y = (DaiSlabCoord)r.Range(-1, 255);
        p.imps = r.Range(1, EXTSEAT_MAX_HAND_SET);
        p.lifted = (DaiTurn)r.U64();
        const int64_t n = r.Count(EXTSEAT_MAX_HAND_SET);
        for (int64_t i = 0; (i < n) && r.Ok(); i++) {
            DaiThingRef t;
            t = r.Ref();
            p.things.push_back(t);
        }
        p.x = (DaiSubtlCoord)r.Range(0, 1 << 16); p.y = (DaiSubtlCoord)r.Range(0, 1 << 16);
        p.reinforce = r.Bool();
        j.p = p;
        return r.Ok();
    }
};

const JobCreep s_creep;
}

const IDaiJobKind &JobCreepKind()
{
    return s_creep;
}
