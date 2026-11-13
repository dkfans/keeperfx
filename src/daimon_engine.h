/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_engine.h
 *     Engine helpers Daimon's keeper controller and packets use that KeeperFX does not have otherwise
 *     (daimon_engine.c).
 */
/******************************************************************************/
#ifndef DK_DAIMON_ENGINE_H
#define DK_DAIMON_ENGINE_H

#include "globals.h"
#include "bflib_basics.h"
#include "daimon_base_types.h"
#include "creature_jobs.h"
#include "roomspace.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayerInfo;
struct Thing;

/** A subtile in one packet parameter: x in the low 16 bits, y in the high 16. */
static inline int32_t packet_pack_xy(int64_t x, int64_t y) { return (int32_t)((uint32_t)(x & 0xFFFF) | ((uint32_t)(y & 0xFFFF) << 16)); }
static inline int64_t packet_unpack_x(int32_t v) { return (int64_t)((uint32_t)v & 0xFFFF); }
static inline int64_t packet_unpack_y(int32_t v) { return (int64_t)(((uint32_t)v >> 16) & 0xFFFF); }

/** The seat a user's packets act for. */
struct PlayerInfo *get_packet_seat(NetUserId user);
/** Whether the game refuses cheat packets (a network game). */
TbBool game_refuses_cheats(void);
/** A box roomspace from a drag's start and end slabs (roomspace.c, not in its header: its own types). */
struct RoomSpace create_box_roomspace_from_drag(struct RoomSpace roomspace, MapSlabCoord start_x, MapSlabCoord start_y, MapSlabCoord end_x, MapSlabCoord end_y);
/** The slabs of a roomspace the keeper may sell (roomspace.c, not in its header). */
struct RoomSpace check_roomspace_for_sellable_slabs(struct RoomSpace roomspace, PlayerNumber plyr_idx);
/** Whether a keeper may place a door at a subtile. */
TbBool door_placement_allowed(PlayerNumber plyr_idx, DaiSubtlCoord stl_x, DaiSubtlCoord stl_y);
/** Drops the first thing a user holds at a subtile (power_hand.c, not in its header). */
short dump_first_held_thing_on_map(PlayerNumber plyr_idx, NetUserId user, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool update_hand);
/** The job a creature dropped at a subtile will take. */
CreatureJob predict_hand_drop_job(struct Thing *creatng, PlayerNumber plyr_idx, DaiSubtlCoord stl_x, DaiSubtlCoord stl_y);

#ifdef __cplusplus
}
#endif
#endif
