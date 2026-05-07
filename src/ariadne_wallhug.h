/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_wallhug.h
 *     Header file for ariadne_wallhug.c.
 * @par Purpose:
 *     Simple wallhug pathfinding functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 10 Jan 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_WALLHUG_H
#define DK_ARIADNE_WALLHUG_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Coord3d;
struct Navigation;
struct SlabMap;

enum WallHugSideState {
    WaHSS_Initial = 0,
    WaHSS_HitWall,
    WaHSS_Completed
};


#pragma pack()
/******************************************************************************/

/******************************************************************************/
long slab_wall_hug_route(struct Thing *thing, struct Coord3d *pos, long max_val);
long get_next_position_and_angle_required_to_tunnel_creature_to(struct Thing *creatng, struct Coord3d *pos, PlayerBitFlags crt_owner_flags);
SubtlCodedCoords dig_to_position(PlayerNumber plyr_idx, MapSubtlCoord basestl_x, MapSubtlCoord basestl_y, SmallAroundIndex direction_around, TbBool revside);
TbBool slab_good_for_computer_dig_path(const struct SlabMap *slb);
short get_hug_side_options(MapSubtlCoord stl1_x, MapSubtlCoord stl1_y, MapSubtlCoord stl2_x, MapSubtlCoord stl2_y, SmallAroundIndex direction, PlayerNumber plyr_idx,
    MapSubtlCoord *ostla_x, MapSubtlCoord *ostla_y, MapSubtlCoord *ostlb_x, MapSubtlCoord *ostlb_y);
/******************************************************************************/
#define CHECK_SLAB_OWNER (flag_is_set(crt_owner_flags, to_flag(slabmap_owner(slb)))) // return TRUE if the slab's owner is stored in crt_owner_flags
#define IGNORE_SLAB_OWNER_CHECK 0 // crt_owner_flags can be set to 0 to nullify the check for the slab's owner
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
