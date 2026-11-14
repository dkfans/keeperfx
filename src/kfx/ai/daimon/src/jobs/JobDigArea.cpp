// dig_area (03 section 8.1.1; Phase 2's place_room): marks every slab of a rectangle that is still to dig, like a dig
// run without a route. Done when nothing in it is left to dig. unmark_area (P5.13): the marks taken off a rectangle.
#include "pre_inc.h"
#include "JobKinds.h"

#include "JobPath.h"
#include "post_inc.h"

namespace {
constexpr DaiTurn WAIT_TURNS = 20;

/** Slabs of the rectangle still to dig, and how many of those are not marked. */
int64_t to_dig(const DaiKnowledge &k, const DaiRect &a, int64_t *unmarked)
{
    int64_t n = 0;
    *unmarked = 0;
    for (DaiSlabCoord y = a.y0; y <= a.y1; y++) {
        for (DaiSlabCoord x = a.x0; x <= a.x1; x++) {
            const DaiSlabInfo s = k.SlabAt(x, y);
            if (!JobDiggable(s)) continue;
            n++;
            if (!s.marked) (*unmarked)++;
        }
    }
    return n;
}

bool area_ok(const DaiKnowledge &k, const DaiRect &a)
{
    return (a.x0 >= 0) && (a.y0 >= 0) && (a.x1 >= a.x0) && (a.y1 >= a.y0) && (a.x1 < k.Width()) && (a.y1 < k.Height())
        && (JobRectSlabs(a) <= EXTSEAT_MAX_DIG_SLABS);
}

class JobDigArea : public IDaiJobKind {
public:
    const char *GetName() const override { return "dig_area"; }
    DaiTurn MinStallWindow() const override { return DAI_DIG_STALL_WINDOW; }
    DaiDomain GetDomain() const override { return DaiDomain::Excavation; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if ((p == nullptr) || !area_ok(c.know, p->area)) {
            snprintf(why, why_len, "BAD_AREA");
            return DaiJobState::Failed;
        }
        int64_t unmarked = 0;
        if ((to_dig(c.know, p->area, &unmarked) > 0) && ((unmarked == 0) || JobMarksFull(c))) j.retry_after = c.now + WAIT_TURNS;
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if ((p == nullptr) || !area_ok(c.know, p->area)) {
            snprintf(why, why_len, "BAD_AREA");
            return DaiResult::BadInput;
        }
        int64_t unmarked = 0;
        if (to_dig(c.know, p->area, &unmarked) == 0) return DaiResult::Done;
        if ((unmarked == 0) || JobMarksFull(c)) return DaiResult::Wait;
        JobMarkDigVerb(verb, p->area);
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if ((p == nullptr) || !area_ok(c.know, p->area)) return 0;
        int64_t unmarked = 0;
        return JobRectSlabs(p->area) - to_dig(c.know, p->area, &unmarked);
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        int64_t unmarked = 0;
        return (p != nullptr) && area_ok(c.know, p->area) && (to_dig(c.know, p->area, &unmarked) == 0);
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j; (void)r;
        return DaiJobState::Ready;
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        const DaiRect a = (p != nullptr) ? p->area : DaiRect();
        w.I64(a.x0); w.I64(a.y0); w.I64(a.x1); w.I64(a.y1);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiAreaParams p;
        p.area.x0 = (DaiSlabCoord)r.Range(0, 255); p.area.y0 = (DaiSlabCoord)r.Range(0, 255);
        p.area.x1 = (DaiSlabCoord)r.Range(0, 255); p.area.y1 = (DaiSlabCoord)r.Range(0, 255);
        j.p = p;
        return r.Ok();
    }
};

const JobDigArea s_dig_area;

/** unmark_area (P5.13): the dig marks taken off a rectangle's slabs -- those by a hot hazard, which lure imps to it.
 *  Done when none of its slabs is marked. */
class JobUnmarkArea : public IDaiJobKind {
public:
    const char *GetName() const override { return "unmark_area"; }
    DaiDomain GetDomain() const override { return DaiDomain::Excavation; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if ((p == nullptr) || !area_ok(c.know, p->area)) {
            snprintf(why, why_len, "BAD_AREA");
            return DaiJobState::Failed;
        }
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if ((p == nullptr) || !area_ok(c.know, p->area)) {
            snprintf(why, why_len, "BAD_AREA");
            return DaiResult::BadInput;
        }
        if (Achieved(c, j)) return DaiResult::Done;
        JobMarkDigVerb(verb, p->area);
        verb->kind = ESV_UnmarkDig;
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { return Achieved(c, j) ? 1 : 0; }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if ((p == nullptr) || !area_ok(c.know, p->area)) return false;
        for (DaiSlabCoord y = p->area.y0; y <= p->area.y1; y++)
            for (DaiSlabCoord x = p->area.x0; x <= p->area.x1; x++) if (c.know.SlabAt(x, y).marked) return false;
        return true;
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j; (void)r;
        return DaiJobState::Ready;
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override { s_dig_area.SaveParams(j, w); }
    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override { return s_dig_area.LoadParams(j, r); }
};

const JobUnmarkArea s_unmark_area;
}

const IDaiJobKind &JobDigAreaKind()
{
    return s_dig_area;
}

const IDaiJobKind &JobUnmarkAreaKind()
{
    return s_unmark_area;
}
