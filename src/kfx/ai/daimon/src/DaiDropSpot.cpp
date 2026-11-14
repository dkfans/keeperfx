#include "pre_inc.h"
#include "daimon/DaiDropSpot.h"
#include "daimon/DaiGeom.h"

#include <algorithm>

#include "post_inc.h"

namespace {
/** The roles of the keeper's rooms of kind `kind` (0 if it has none). */
uint64_t roles_of(const DaiKnowledge &k, RoomKind kind)
{
    for (const DaiRoomLook &r : k.Rooms()) if (r.kind == kind) return r.roles;
    return 0;
}

bool drop_ok(const DaiKnowledge &k, PlayerNumber p, DaiSubtlCoord x, DaiSubtlCoord y)
{
    const DaiSlabCoord sx = DaiSubtileSlab(x), sy = DaiSubtileSlab(y);
    if ((sx < 0) || (sy < 0) || (sx >= k.Width()) || (sy >= k.Height())) return false;
    const DaiSlabInfo s = k.SlabAt(sx, sy);
    if (s.owner != p) return false;
    if ((s.terrain != DaiTerrain::Floor) && (s.terrain != DaiTerrain::Room)) return false;
    if ((s.terrain == DaiTerrain::Room) && ((roles_of(k, s.room) & DAI_NO_DROP_ROLES) != 0)) return false;
    return k.Reach().Reached(sx, sy);
}
}

const char *DaiFindDropSpot(const DaiKnowledge &k, PlayerNumber p, DaiSubtlCoord x, DaiSubtlCoord y, int64_t radius,
                            DaiSubtlCoord reach_x, DaiSubtlCoord reach_y, DaiSubtlCoord *out_x, DaiSubtlCoord *out_y)
{
    radius = std::clamp<int64_t>(radius, 0, 45);
    // The reach map joins every slab it has: a reach point off it joins nothing.
    if ((reach_x >= 0) && !k.Reach().Reached(DaiSubtileSlab(reach_x), DaiSubtileSlab(reach_y))) return "NO_DROP_SPOT";
    for (int64_t r = 0; r <= radius; r++) {
        for (int64_t dy = -r; dy <= r; dy++) {
            for (int64_t dx = -r; dx <= r; dx++) {
                if ((dx > -r) && (dx < r) && (dy > -r) && (dy < r)) continue;
                const DaiSubtlCoord cx = x + dx, cy = y + dy;
                if ((cx < 0) || (cy < 0) || !drop_ok(k, p, cx, cy)) continue;
                *out_x = cx;
                *out_y = cy;
                return nullptr;
            }
        }
    }
    return "NO_DROP_SPOT";
}
