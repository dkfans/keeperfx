/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_visibility.c
 *     What a keeper can see (keeper_visibility.h). Moved down from kfx_script's api_seat_view.c.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "keeper_visibility.h"

#include "config_terrain.h"
#include "game_legacy.h"
#include "map_blocks.h"
#include "map_data.h"
#include "player_data.h"
#include "slab_data.h"
#include "thing_data.h"
#include "thing_doors.h"
#include "thing_list.h"
#include "thing_traps.h"
#include "creature_states.h"
#include "daimon_engine.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

TbBool keeper_slab_revealed(PlayerNumber plyr_idx, DaiSlabCoord slb_x, DaiSlabCoord slb_y)
{
    return subtile_revealed(DaiSlabSubtileCenter(slb_x), DaiSlabSubtileCenter(slb_y), plyr_idx);
}

TbBool keeper_slab_open_behind_seen_wall(PlayerNumber plyr_idx, DaiSlabCoord x, DaiSlabCoord y, TbBool own_walls_only)
{
    // Every test only reads, so the cheap ones (the owner, the wall's face) go before the revealed ones, which loop
    // over the players when allies share vision: most slabs of the map have no wall beside them at all.
    static const int8_t dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
    for (int k = 0; k < 4; k++) {
        const DaiSlabCoord wx = x + dx[k], wy = y + dy[k];
        if ((wx < 0) || (wy < 0) || (wx >= game.map_tiles_x) || (wy >= game.map_tiles_y)) continue;
        if (own_walls_only && (slabmap_owner(get_slabmap_block(wx, wy)) != plyr_idx)) continue;
        if (!wall_slab_shows_face_toward(wx, wy, x, y)) continue;
        if (!keeper_slab_revealed(plyr_idx, wx, wy)) continue;
        return !keeper_slab_revealed(plyr_idx, x, y);
    }
    return false;
}

int64_t keeper_open_behind_walls(PlayerNumber plyr_idx, int64_t *out_xy, int64_t max)
{
    // Only a slab beside one of the keeper's own fortified walls (not a room's) can pass the test, so those walls are
    // marked first in one pass and the test runs only beside them: it was most of a computer keeper's turn (12 section
    // 13.3). A map larger than the mask is tested slab by slab.
    const DaiSlabCoord w = game.map_tiles_x, h = game.map_tiles_y;
    unsigned char wall[MAX_TILES_X * MAX_TILES_Y];
    const TbBool masked = (w > 0) && (h > 0) && (w <= MAX_TILES_X) && (h <= MAX_TILES_Y);
    if (masked) {
        for (DaiSlabCoord y = 0; y < h; y++) {
            for (DaiSlabCoord x = 0; x < w; x++) {
                const struct SlabMap *slb = &game.slabmap[y * w + x];
                const struct SlabConfigStats *slabst = (slabmap_owner(slb) == plyr_idx) ? get_slab_stats(slb) : NULL;
                wall[y * w + x] = (slabst != NULL) && (slabst->category == SlbAtCtg_FortifiedWall) && (slabst->slb_id == 0);
            }
        }
    }
    int64_t n = 0;
    for (DaiSlabCoord y = 0; y < h; y++) {
        for (DaiSlabCoord x = 0; x < w; x++) {
            if (masked && !((y > 0) && wall[(y - 1) * w + x]) && !((x + 1 < w) && wall[y * w + x + 1]) && !((y + 1 < h) && wall[(y + 1) * w + x])
                && !((x > 0) && wall[y * w + x - 1])) continue;
            if (!keeper_slab_open_behind_seen_wall(plyr_idx, x, y, true)) continue;
            if ((out_xy != NULL) && (n < max)) { out_xy[2 * n] = x; out_xy[2 * n + 1] = y; }
            n++;
        }
    }
    return n;
}

TbBool keeper_creature_visible_to(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (thing->owner == plyr_idx) {
        return true;
    }
    if (!thing_revealed(thing, plyr_idx)) {
        return false;
    }
    return !(players_are_enemies(plyr_idx, thing->owner) && creature_is_invisible(thing));
}

TbBool keeper_trap_visible_to(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (thing->owner == plyr_idx) {
        return true;
    }
    if (!thing_revealed(thing, plyr_idx)) {
        return false;
    }
    return (thing->trap.revealed != 0) || !get_trap_model_stats(thing->model)->hidden;
}

TbBool keeper_door_visible_to(struct Thing *thing, PlayerNumber plyr_idx)
{
    if (thing->owner == plyr_idx) {
        return true;
    }
    return thing_revealed(thing, plyr_idx) && !door_is_hidden_to_player(thing, plyr_idx);
}

#ifdef __cplusplus
}
#endif
