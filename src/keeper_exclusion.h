/** @file keeper_exclusion.h
 *  What a keeper's human is doing right now (docs/plans/ai-plans/v2/09 section 5.2a, Daimon P7.10): the creature they
 *  possess and the group it leads, the things in their hand, the thing under their cursor. A gesture another author
 *  makes on the keeper's behalf (its Daimon assistant) never touches these, nor casts or drops within a radius of the
 *  possessed creature: the assistant plans around them (DaiKnowledge's live exclusion) and the self-contained gesture
 *  packets refuse them when applied (game_commands.c), whoever planned the packet.
 */
#ifndef KFX_KEEPER_EXCLUSION_H
#define KFX_KEEPER_EXCLUSION_H

#include "globals.h"
#include "daimon_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Thing;

/** The creature the keeper possesses (or rides along with), 0 none. */
DaiThingIndex keeper_possessed_thing(PlayerNumber plyr_idx);
/** The thing is the keeper's human's now: possessed, a member of the group its possessed creature leads, in its hand,
 *  open in its info panel, or under its cursor. */
TbBool keeper_thing_excluded(PlayerNumber plyr_idx, const struct Thing *thing);
/** The subtile is within `radius` subtiles (chessboard) of the creature the keeper possesses. */
TbBool keeper_area_excluded(PlayerNumber plyr_idx, DaiSubtlCoord stl_x, DaiSubtlCoord stl_y, int64_t radius);

#ifdef __cplusplus
}
#endif

#endif
