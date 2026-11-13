/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_visibility.h
 *     What a keeper can see (the fog): which slabs it has revealed, which things on them it is shown, and the open
 *     ground its walls give away. Shared by the External seat's view, the keeper-facts tracker and Daimon's world
 *     reader (docs/plans/ai-plans/v2/01-phase0-foundations.md P0.4), so all three see exactly what a human in that
 *     seat would. Read-only on the simulation.
 */
/******************************************************************************/
#ifndef DK_KEEPER_VISIBILITY_H
#define DK_KEEPER_VISIBILITY_H

#include "globals.h"
#include "daimon_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Thing;

/** The keeper has revealed the slab (its centre subtile). */
TbBool keeper_slab_revealed(PlayerNumber plyr_idx, DaiSlabCoord slb_x, DaiSlabCoord slb_y);
/** An unseen slab that a seen fortified wall beside it is built facing (wall_slab_shows_face_toward): not plain earth or
 *  rock but open ground, a room, a door or another keeper's wall. A keeper sees that on the wall without seeing the
 *  slab, e.g. heroes' tunnellers digging around the dungeon. `own_walls_only`: only the keeper's own walls count. */
TbBool keeper_slab_open_behind_seen_wall(PlayerNumber plyr_idx, DaiSlabCoord x, DaiSlabCoord y, TbBool own_walls_only);
/** Every slab keeper_slab_open_behind_seen_wall() finds with own walls only, as x, y pairs (at most `max` written).
 *  Returns how many there are. */
int64_t keeper_open_behind_walls(PlayerNumber plyr_idx, int64_t *out_xy, int64_t max);
/** A creature the keeper is shown: its own, or one on a revealed subtile that is not an invisible enemy (invisible
 *  allies stay visible, as in engine_render.c's enemy-and-visible test). */
TbBool keeper_creature_visible_to(const struct Thing *thing, PlayerNumber plyr_idx);
/** Hidden traps stay hidden from other keepers until revealed; a trap on an unrevealed tile is unseen either way. */
TbBool keeper_trap_visible_to(const struct Thing *thing, PlayerNumber plyr_idx);
/** A door the keeper is shown: its own, or a revealed one not hidden from it (a secret door). */
TbBool keeper_door_visible_to(struct Thing *thing, PlayerNumber plyr_idx);

#ifdef __cplusplus
}
#endif

#endif
