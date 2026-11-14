// create_imp (01 section 8.5): cast Create Imp (or, P9, the tunneller summon) on the keeper's own claimed floor nearest
// its heart. Done when the digger count rises past where it was when the job was made; the cast itself leaves the job
// Running until then.
#include "pre_inc.h"
#include "daimon/DaiGeom.h"
#include "JobKinds.h"

#include <cstring>

#include "post_inc.h"

namespace {
/** How far from the heart (slabs) the cast looks for the keeper's own floor. */
constexpr DaiSlabCoord IMP_SPOT_RADIUS = 12;

bool imp_spot(const DaiTurnCtx &c, DaiSubtlCoord *sx, DaiSubtlCoord *sy)
{
    const DaiOwnLook &own = c.know.Own();
    const DaiSlabCoord hx = own.heart_x / DAI_STL_PER_SLB, hy = own.heart_y / DAI_STL_PER_SLB;
    // Rings outwards, each in a fixed order: the same knowledge gives the same spot (principle 6).
    for (DaiSlabCoord r = 1; r <= IMP_SPOT_RADIUS; r++) {
        for (DaiSlabCoord dy = -r; dy <= r; dy++) {
            for (DaiSlabCoord dx = -r; dx <= r; dx++) {
                if ((dx != -r) && (dx != r) && (dy != -r) && (dy != r)) continue;
                const DaiSlabInfo s = c.know.SlabAt(hx + dx, hy + dy);
                if ((s.terrain != DaiTerrain::Floor) || (s.owner != c.player)) continue;
                *sx = (DaiSubtlCoord)((hx + dx) * DAI_STL_PER_SLB + 1);
                *sy = (DaiSubtlCoord)((hy + dy) * DAI_STL_PER_SLB + 1);
                return true;
            }
        }
    }
    return false;
}

class JobCreateImp : public IDaiJobKind {
public:
    const char *GetName() const override { return "create_imp"; }
    DaiDomain GetDomain() const override { return DaiDomain::Workforce; }
    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        if (!c.know.Own().has_heart) {
            snprintf(why, why_len, "NO_HEART");
            return DaiResult::Fail;
        }
        DaiSubtlCoord sx = 0, sy = 0;
        if (!imp_spot(c, &sx, &sy)) return DaiResult::Wait; // the knowledge has not seen the heart's floor yet
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_CastPower;
        const DaiCreateImpParams *p = std::get_if<DaiCreateImpParams>(&j.p);
        snprintf(verb->name, sizeof(verb->name), "%s", DaiPowerCode((p != nullptr) ? p->power : DaiPower::CreateImp));
        verb->has_pos = true;
        verb->stl_x = sx;
        verb->stl_y = sy;
        return DaiResult::Ok;
    }
    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        (void)j;
        return c.know.Own().imps;
    }
    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiCreateImpParams *p = std::get_if<DaiCreateImpParams>(&j.p);
        return (p != nullptr) && (c.know.Own().imps > p->imps_at_start);
    }
    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Running; // the imp shows up in the count (Achieved)
    }
    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiCreateImpParams *p = std::get_if<DaiCreateImpParams>(&j.p);
        w.I64((p != nullptr) ? p->imps_at_start : 0);
        w.I64((int64_t)((p != nullptr) ? p->power : DaiPower::CreateImp));
    }
    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiCreateImpParams p;
        p.imps_at_start = r.Range(0, INT64_MAX);
        p.power = (DaiPower)r.Range(0, (int64_t)DaiPower::Count - 1);
        if ((p.power != DaiPower::CreateImp) && (p.power != DaiPower::Tunneller)) r.Fail("bad digger power");
        j.p = p;
        return r.Ok();
    }
};

const JobCreateImp s_create_imp;
}

const IDaiJobKind &JobCreateImpKind()
{
    return s_create_imp;
}
