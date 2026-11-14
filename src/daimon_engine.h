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
#include "slab_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayerInfo;
struct Thing;
struct Room;
struct SlabMap;

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
/** Upstream's own (magic_powers.c, not in its header): whether Sight of Evil shows a subtile to a keeper. */
TbBool power_sight_explored(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
/** A creature's fighting strength, as the keepers' force estimates count it. */
int64_t creature_force_score(const struct Thing *thing);
/** What a prisoner or a tortured creature would become if it died now (0: nothing). */
DaiThingModel prison_death_kind(const struct Thing *thing, const struct Room *room);
DaiThingModel torture_death_kind(const struct Thing *thing, const struct Room *room);
/** Upstream's own (slab_data.c, not in its header): the kind a room slab's centre has. */
SlabKind find_core_slab_type(MapSlabCoord slb_x, MapSlabCoord slb_y);
/** A slab map entry's kind. */
SlabKind slabmap_kind(const struct SlabMap *slb);
/** One side of a room slab as the room's efficiency scores it. */
int64_t room_slab_side_score(SlabCodedCoords slab_num, SlabCodedCoords round_slab_num, int64_t synergy_slab_num, TbBool *continues_room);
/** Whether the keeper's Call to Arms is cast and paid for. */
TbBool player_call_to_arms_is_charged(PlayerNumber plyr_idx);
/** Whether a wall slab shows its face toward a neighbouring slab. */
TbBool wall_slab_shows_face_toward(DaiSlabCoord wall_x, DaiSlabCoord wall_y, DaiSlabCoord nb_x, DaiSlabCoord nb_y);
/** The job a creature dropped at a subtile will take. */
CreatureJob predict_hand_drop_job(struct Thing *creatng, PlayerNumber plyr_idx, DaiSubtlCoord stl_x, DaiSubtlCoord stl_y);

#ifdef __cplusplus
}
#endif
#endif
