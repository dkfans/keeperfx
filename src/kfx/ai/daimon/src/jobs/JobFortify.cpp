// fortify_room (P9, the owner's idea): an imp dropped on a room's edge beside its earth, as a human does to fortify a
// room. Dropped with nothing nearer to do, an imp fortifies the wall beside it (check_out_unreinforced_drop_place: the
// side it looks at first is the side of the slab it lands on) and goes on with the walls near that until none is
// left. A fortified wall scores a room side 2 to earth's 1 (room_side_score.c). One trip: done once the drop went, or
// when the earth there is no longer earth (fortified meanwhile, or dug).
#include "pre_inc.h"
#include "JobKinds.h"

#include <climits>
#include <cstring>

#include "post_inc.h"

namespace {
const DaiImpLook *imp_of(const DaiKnowledge &k, const DaiThingRef &r)
{
    for (const DaiImpLook &i : k.OwnImps()) if (i.ref == r) return &i;
    return nullptr;
}

class JobFortifyRoom : public IDaiJobKind {
public:
    const char *GetName() const override { return "fortify_room"; }
    DaiDomain GetDomain() const override { return DaiDomain::Economy; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiFortifyParams *p = std::get_if<DaiFortifyParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        if (j.state != DaiJobState::Ready) return j.state;
        const DaiImpLook *imp = imp_of(c.know, p->imp);
        if (imp == nullptr) {
            snprintf(why, why_len, "GONE");
            return DaiJobState::Failed;
        }
        if (Achieved(c, j)) return DaiJobState::Done;
        j.needs = DaiJobNeeds();
        // An imp the hand cannot take this turn: wait for it, not a refusal.
        if (!imp->pickable) {
            j.retry_after = c.now + 8;
            return j.state;
        }
        j.needs.hand = true;
        j.needs.thing = p->imp;
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiFortifyParams *p = std::get_if<DaiFortifyParams>(&j.p);
        if ((p == nullptr) || (p->imp.idx == 0)) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        if (Achieved(c, j)) return DaiResult::Done;
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_PickUpAndDrop;
        verb->thing_count = 1;
        verb->thing_ids[0] = p->imp.idx;
        verb->has_pos = true;
        verb->stl_x = p->stl_x;
        verb->stl_y = p->stl_y;
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { return Achieved(c, j) ? 1 : 0; }

    /** The earth beside the drop is earth no more. */
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiFortifyParams *p = std::get_if<DaiFortifyParams>(&j.p);
        return (p != nullptr) && (c.know.SlabAt(p->wall_x, p->wall_y).terrain != DaiTerrain::Dirt);
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Done;     // one trip
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiFortifyParams *p = std::get_if<DaiFortifyParams>(&j.p);
        const DaiFortifyParams v = (p != nullptr) ? *p : DaiFortifyParams();
        w.Ref(v.imp);
        w.I64(v.stl_x);
        w.I64(v.stl_y);
        w.I64(v.wall_x);
        w.I64(v.wall_y);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiFortifyParams p;
        p.imp = r.Ref();
        p.stl_x = (DaiSubtlCoord)r.Range(0, 1023);
        p.stl_y = (DaiSubtlCoord)r.Range(0, 1023);
        p.wall_x = (DaiSlabCoord)r.Range(0, 255);
        p.wall_y = (DaiSlabCoord)r.Range(0, 255);
        j.p = p;
        return r.Ok();
    }
};

const JobFortifyRoom s_fortify_room;
}

const IDaiJobKind &JobFortifyRoomKind()
{
    return s_fortify_room;
}
