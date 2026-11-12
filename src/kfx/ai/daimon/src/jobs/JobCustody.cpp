// custody_hand (08 section 17): one trip of the hand for Custody -- a prisoner of the keeper's carried to one of its
// rooms (a torture chamber's rack, a prison, the portal with the dismiss intent), or a chicken from a hatchery dropped
// on a prisoner to feed it. The prisoner is checked against the custody look, not the keeper's own creatures: it is
// another keeper's (or a hero's). The controller refuses a drop that would free it (WOULD_RELEASE_PRISONER) or a full
// room (ROOM_FULL); a refusal ends the job and Custody plans again. A sacrifice drops a prisoner, a creature of the
// keeper's or an imp on its temple pool with the sacrifice intent (the controller refuses the pool without it, and a
// drop of anything but a creature on it).
#include "pre_inc.h"
#include "JobKinds.h"

#include <cstring>

#include "post_inc.h"

namespace {
const DaiPrisonerLook *prisoner(const DaiKnowledge &k, const DaiThingRef &r)
{
    const DaiCustodyLook &cu = k.Custody();
    for (int64_t i = 0; i < cu.prisoners; i++) if (cu.prisoner[i].ref == r) return &cu.prisoner[i];
    return nullptr;
}

/** A sacrifice's victim: a prisoner, one of its creatures, or one of its imps; its subtile. */
bool victim_at(const DaiKnowledge &k, const DaiThingRef &r, DaiSubtlCoord *x, DaiSubtlCoord *y)
{
    if (const DaiPrisonerLook *p = prisoner(k, r)) { *x = p->x; *y = p->y; return true; }
    if (const DaiCreatureLook *c = k.Creature(r)) { *x = c->x; *y = c->y; return true; }
    for (const DaiImpLook &i : k.OwnImps()) if (i.ref == r) { *x = i.x; *y = i.y; return true; }
    return false;
}

bool food_there(const DaiKnowledge &k, const DaiThingRef &r)
{
    for (const DaiFoodLook &f : k.Food()) if (f.ref == r) return true;
    return false;
}


class JobCustodyHand : public IDaiJobKind {
public:
    const char *GetName() const override { return "custody_hand"; }
    DaiDomain GetDomain() const override { return DaiDomain::Custody; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiCustodyHandParams *p = std::get_if<DaiCustodyHandParams>(&j.p);
        if ((p == nullptr) || (p->trip >= DaiCustodyTrip::Count)) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        if (j.state != DaiJobState::Ready) return j.state;
        // Gone from custody (converted, dead, escaped), or its chicken eaten, or the pool gone: nothing to carry.
        DaiSubtlCoord vx = 0, vy = 0;
        const bool there = (p->trip == DaiCustodyTrip::Sacrifice) ? (victim_at(c.know, p->prisoner, &vx, &vy) && c.know.Custody().pool)
                                                                   : (prisoner(c.know, p->prisoner) != nullptr);
        if (!there || ((p->trip == DaiCustodyTrip::Feed) && !food_there(c.know, p->food))) {
            snprintf(why, why_len, "GONE");
            return DaiJobState::Failed;
        }
        j.needs = DaiJobNeeds();
        j.needs.hand = true;
        j.needs.thing = (p->trip == DaiCustodyTrip::Feed) ? p->food : p->prisoner;
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiCustodyHandParams *p = std::get_if<DaiCustodyHandParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_PickUpAndDrop;
        verb->thing_count = 1;
        if (p->trip == DaiCustodyTrip::Sacrifice) {
            DaiSubtlCoord vx = 0, vy = 0;
            if (!victim_at(c.know, p->prisoner, &vx, &vy) || !c.know.Custody().pool) {
                snprintf(why, why_len, "GONE");
                return DaiResult::Fail;
            }
            verb->thing_ids[0] = p->prisoner.idx;
            verb->has_pos = true;
            verb->stl_x = c.know.Custody().pool_x;
            verb->stl_y = c.know.Custody().pool_y;
            verb->sacrifice = true;
            return DaiResult::Ok;
        }
        const DaiPrisonerLook *pr = prisoner(c.know, p->prisoner);
        if (pr == nullptr) {
            snprintf(why, why_len, "GONE");
            return DaiResult::Fail;
        }
        if (p->trip == DaiCustodyTrip::Feed) {
            verb->thing_ids[0] = p->food.idx;
            verb->has_pos = true;
            verb->stl_x = pr->x;
            verb->stl_y = pr->y;
        } else {
            verb->thing_ids[0] = p->prisoner.idx;
            verb->has_room = true;
            verb->room_id = p->room;
            verb->dismiss = (p->trip == DaiCustodyTrip::Dismiss);
        }
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return 0; }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override { (void)c; (void)j; return false; }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Done;     // one trip
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiCustodyHandParams *p = std::get_if<DaiCustodyHandParams>(&j.p);
        const DaiCustodyHandParams v = (p != nullptr) ? *p : DaiCustodyHandParams();
        w.Ref(v.prisoner);
        w.I64((int64_t)v.trip);
        w.I64(v.room);
        w.Ref(v.food);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiCustodyHandParams p;
        p.prisoner = r.Ref();
        p.trip = (DaiCustodyTrip)r.Range(0, (int64_t)DaiCustodyTrip::Count - 1);
        p.room = r.Range(0, INT32_MAX);
        p.food = r.Ref();
        j.p = p;
        return r.Ok();
    }
};

const JobCustodyHand s_custody_hand;
}

const IDaiJobKind &JobCustodyHandKind() { return s_custody_hand; }
