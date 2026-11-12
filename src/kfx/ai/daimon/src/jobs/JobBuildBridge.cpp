// build_bridge (03 section 8.2, 7.5): a bridge on its own -- the slabs in order from the keeper's land, built as
// straight lines, each line starting from a slab that is the keeper's (the bank, or the bridge's last slab), as the
// engine allows. Used when a target needs only a bridge, or ground across water is wanted as the keeper's own.
#include "pre_inc.h"
#include "JobKinds.h"

#include "JobPath.h"
#include "post_inc.h"

namespace {
constexpr DaiTurn WAIT_TURNS = 20;

int64_t first_unbuilt(const DaiTurnCtx &c, const DaiBuildBridgeParams &p)
{
    for (int64_t i = 0; i < (int64_t)p.slabs.size(); i++)
        if (!JobBridged(c.know, c.player, p.slabs[(size_t)i].x, p.slabs[(size_t)i].y)) return i;
    return (int64_t)p.slabs.size();
}

class JobBuildBridge : public IDaiJobKind {
public:
    const char *GetName() const override { return "build_bridge"; }
    DaiTurn MinStallWindow() const override { return DAI_DIG_STALL_WINDOW; }
    DaiDomain GetDomain() const override { return DaiDomain::Excavation; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiBuildBridgeParams *p = std::get_if<DaiBuildBridgeParams>(&j.p);
        if ((p == nullptr) || p->slabs.empty()) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        j.needs = DaiJobNeeds();
        const int64_t at = first_unbuilt(c, *p);
        if (at >= (int64_t)p->slabs.size()) return j.state;
        const DaiSlabXY &s = p->slabs[(size_t)at];
        const DaiTerrain t = c.know.SlabAt(s.x, s.y).terrain;
        if ((t != DaiTerrain::Water) && (t != DaiTerrain::Lava) && (t != DaiTerrain::Abyss)) {
            snprintf(why, why_len, "NOT_LIQUID");
            return DaiJobState::Failed;
        }
        DaiRect r;
        const int64_t n = JobBridgeRun(c.know, c.player, p->slabs, at, true, true, &r);
        j.needs.gold = n * c.know.Build(DaiRoom::Bridge).price;
        if (n == 0) j.retry_after = c.now + WAIT_TURNS;
        return j.state;
    }

    int64_t Watch(const DaiTurnCtx &c, const DaiJob &j, DaiSlabXY *out, int64_t max) const override
    {
        // The next slab and its bank (the slab before it, or the first slab's four sides).
        const DaiBuildBridgeParams *p = std::get_if<DaiBuildBridgeParams>(&j.p);
        if ((p == nullptr) || p->slabs.empty()) return 0;
        const int64_t at = first_unbuilt(c, *p);
        if (at >= (int64_t)p->slabs.size()) return 0;
        const DaiSlabXY s = p->slabs[(size_t)at];
        int64_t n = 0;
        if (n < max) out[n++] = s;
        if (at > 0) {
            if (n < max) out[n++] = p->slabs[(size_t)at - 1];
        } else {
            const DaiSlabXY sides[4] = { { s.x, s.y - 1 }, { s.x + 1, s.y }, { s.x, s.y + 1 }, { s.x - 1, s.y } };
            for (const DaiSlabXY &b : sides) if (n < max) out[n++] = b;
        }
        return n;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiBuildBridgeParams *p = std::get_if<DaiBuildBridgeParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        if (!c.know.Build(DaiRoom::Bridge).available) return DaiResult::Wait;
        const int64_t at = first_unbuilt(c, *p);
        if (at >= (int64_t)p->slabs.size()) return DaiResult::Done;
        DaiRect r;
        if (JobBridgeRun(c.know, c.player, p->slabs, at, true, true, &r) == 0) return DaiResult::Wait;
        JobBuildRoomVerb(verb, DaiRoom::Bridge, r);
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiBuildBridgeParams *p = std::get_if<DaiBuildBridgeParams>(&j.p);
        if (p == nullptr) return 0;
        int64_t n = 0;
        for (const DaiSlabXY &s : p->slabs) n += JobBridged(c.know, c.player, s.x, s.y) ? 1 : 0;
        return n;
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiBuildBridgeParams *p = std::get_if<DaiBuildBridgeParams>(&j.p);
        return (p != nullptr) && !p->slabs.empty() && (first_unbuilt(c, *p) >= (int64_t)p->slabs.size());
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j; (void)r;
        return DaiJobState::Ready;
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiBuildBridgeParams *p = std::get_if<DaiBuildBridgeParams>(&j.p);
        const int64_t n = (p != nullptr) ? (int64_t)p->slabs.size() : 0;
        w.I64(n);
        for (int64_t i = 0; i < n; i++) { w.I64(p->slabs[(size_t)i].x); w.I64(p->slabs[(size_t)i].y); }
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiBuildBridgeParams p;
        const int64_t n = r.Count(256 * 256);
        for (int64_t i = 0; (i < n) && r.Ok(); i++) p.slabs.push_back(DaiSlabXY{ (DaiSlabCoord)r.Range(0, 255), (DaiSlabCoord)r.Range(0, 255) });
        j.p = p;
        return r.Ok();
    }
};

const JobBuildBridge s_build_bridge;
}

const IDaiJobKind &JobBuildBridgeKind()
{
    return s_build_bridge;
}
