#include "pre_inc.h"
#include "daimon/DaiGeom.h"
#include "JobPath.h"

#include <cstdlib>
#include <cstring>

#include "post_inc.h"

bool JobDiggable(const DaiSlabInfo &s)
{
    switch (s.terrain) {
    case DaiTerrain::Unknown: case DaiTerrain::Dirt: case DaiTerrain::Gold: case DaiTerrain::DenseGold: return true;
    default: return false;
    }
}

bool JobDiggableBy(const DaiSlabInfo &s, PlayerNumber p)
{
    return JobDiggable(s) || ((s.terrain == DaiTerrain::Wall) && (s.owner == p));
}

bool JobDropSite(const DaiKnowledge &k, PlayerNumber p, const DaiSlabXY *before, const DaiSlabXY &a, DaiSlabXY *out)
{
    if ((before != nullptr) && JobOwnLand(k, p, before->x, before->y)) { *out = *before; return true; }
    int64_t best = JOB_DROP_REACH + 1;
    for (DaiSlabCoord y = (DaiSlabCoord)(a.y - JOB_DROP_REACH); y <= a.y + JOB_DROP_REACH; y++) {
        for (DaiSlabCoord x = (DaiSlabCoord)(a.x - JOB_DROP_REACH); x <= a.x + JOB_DROP_REACH; x++) {
            if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height()) || !JobOwnLand(k, p, x, y)) continue;
            const int64_t d = std::max<int64_t>(std::abs(x - a.x), std::abs(y - a.y));
            if (d < best) { best = d; *out = DaiSlabXY{ x, y }; }
        }
    }
    return best <= JOB_DROP_REACH;
}

const DaiImpLook *JobDropImp(const DaiKnowledge &k, const DaiSlabXY &at)
{
    const DaiImpLook *best = nullptr;
    int64_t best_d = 0;
    for (const DaiImpLook &i : k.OwnImps()) {
        // Not one training (Creatures sent it for speed or teleport, P9), nor one the hand cannot take now.
        if (i.training || !i.pickable) continue;
        // One carrying gold walks it to the treasury before any work it is dropped by (level 52: a gold seam on a breach's
        // way never dug, every imp fetched to it a laden one).
        const int64_t d = ((i.gold > 0) ? 2000 : 0) + (i.working ? 1000 : 0)
                        + std::max<int64_t>(std::abs(DaiSubtileSlab(i.x) - at.x), std::abs(DaiSubtileSlab(i.y) - at.y));
        if ((best == nullptr) || (d < best_d)) { best = &i; best_d = d; }
    }
    return best;
}

bool JobOwnLand(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y)
{
    const DaiSlabInfo s = k.SlabAt(x, y);
    if (s.owner != p) return false;
    return (s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Room) || (s.terrain == DaiTerrain::Door);
}

bool JobBridged(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y)
{
    const DaiSlabInfo s = k.SlabAt(x, y);
    const RoomKind bridge = k.RoomKindOf(DaiRoom::Bridge);
    return (s.terrain == DaiTerrain::Room) && (s.owner == p) && (bridge != 0) && (s.room == bridge);
}

JobStep JobStepAt(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, bool claim)
{
    const DaiSlabInfo s = k.SlabAt(x, y);
    if (JobDiggable(s)) return JobStep::Dig;
    switch (s.terrain) {
    case DaiTerrain::Lava: case DaiTerrain::Abyss: return JobStep::Bridge;
    case DaiTerrain::Water: return claim ? JobStep::Bridge : JobStep::Walk;
    case DaiTerrain::Floor: return JobStep::Walk;
    case DaiTerrain::Room: return ((s.owner == p) || (s.owner < 0)) ? JobStep::Walk : JobStep::Blocked;
    case DaiTerrain::Door: return (s.owner == p) ? JobStep::Walk : JobStep::Blocked;
    case DaiTerrain::Wall: return (s.owner == p) ? JobStep::Dig : JobStep::Blocked;
    default: return JobStep::Blocked;
    }
}

void JobMarkDigVerb(KeeperVerb *v, const DaiRect &r)
{
    memset(v, 0, sizeof(*v));
    v->kind = ESV_MarkDig;
    v->has_rect = true;
    v->slab_x0 = r.x0;
    v->slab_y0 = r.y0;
    v->slab_x1 = r.x1;
    v->slab_y1 = r.y1;
}

void JobBuildRoomVerb(KeeperVerb *v, DaiRoom room, const DaiRect &r)
{
    JobMarkDigVerb(v, r);
    v->kind = ESV_BuildRoom;
    snprintf(v->name, sizeof(v->name), "%s", DaiRoomCode(room));
}

int64_t JobBridgeRun(const DaiKnowledge &k, PlayerNumber p, std::span<const DaiSlabXY> path, int64_t from, bool claim,
                     bool first_from_any_side, DaiRect *rect)
{
    if ((from < 0) || (from >= (int64_t)path.size())) return 0;
    const DaiSlabXY &at = path[(size_t)from];
    bool land = false;
    if (from > 0) {
        land = JobOwnLand(k, p, path[(size_t)from - 1].x, path[(size_t)from - 1].y);
    } else if (first_from_any_side) {
        land = JobOwnLand(k, p, at.x, at.y - 1) || JobOwnLand(k, p, at.x + 1, at.y) || JobOwnLand(k, p, at.x, at.y + 1)
            || JobOwnLand(k, p, at.x - 1, at.y);
    }
    if (!land) return 0;                // the claim wait: imps have not claimed the bank yet
    return JobRun(path, from, JOB_BRIDGE_RUN_MAX, [&](int64_t i) {
        const DaiSlabXY &s = path[(size_t)i];
        return !JobBridged(k, p, s.x, s.y) && (JobStepAt(k, p, s.x, s.y, claim) == JobStep::Bridge);
    }, rect);
}
