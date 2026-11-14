/** @file keeper_ctl_world.h
 *     What the keeper controller's planning, and the hand reflexes' decisions (keeper_reflexes.h), ask about the map.
 * @par Purpose:
 *     Answered by whoever drives the slot (kctl_set_world): the External seat with the game's own answers, Daimon from
 *     its knowledge. Here, in kfx_ai, so the reflex decisions can take it (docs/plans/ai-plans/v2/06-phase4-defence.md
 *     section 8, P4.2); the controller in kfx_net includes it.
 */
#ifndef DK_KEEPER_CTL_WORLD_H
#define DK_KEEPER_CTL_WORLD_H

#include "globals.h"
#include "daimon_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** The game's own answers (the default world: navigation_points_connected, creature_can_navigate_to, the controller's
 *  drop-spot search) refresh navigation caches as a side effect, which is harmless for the External seat in a local game
 *  but not for Daimon, whose writes would exist on one machine only (docs/plans/ai-plans/v2/02-library-architecture.md
 *  principle 8): Daimon answers from its own knowledge. A NULL member falls back to the default world's answer.
 *  Positions are subtiles. */
struct KeeperCtlWorld {
    void *ctx;
    /** Whether a creature walking could get from one spot to the other. */
    TbBool (*connected)(void *ctx, DaiSubtlCoord ax, DaiSubtlCoord ay, DaiSubtlCoord bx, DaiSubtlCoord by);
    /** Whether this creature could walk to the spot (move_creature, attack). */
    TbBool (*can_reach)(void *ctx, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y);
    /** Whether a target thing is known and visible to the slot's keeper (Phase 4-5 orders; unused so far). */
    TbBool (*target_valid)(void *ctx, DaiThingIndex target);
    /** A spot within `radius` subtiles of (x, y) where the keeper's hand may drop `creature` (0: any of its creatures),
     *  nearest ring first; with reach_x >= 0, only one a walker could go from to (reach_x, reach_y). NULL with the spot
     *  in *out_x, *out_y, or why there is none (the hand reflexes, Phase 4). */
    const char *(*find_drop_spot)(void *ctx, PlayerNumber plyr, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y, int64_t radius,
                                  DaiSubtlCoord reach_x, DaiSubtlCoord reach_y, DaiSubtlCoord *out_x, DaiSubtlCoord *out_y);
};

#ifdef __cplusplus
}
#endif
#endif
