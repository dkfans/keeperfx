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
    WaHSS_Val0 = 0,
    WaHSS_Val1,
    WaHSS_Val2
};


#pragma pack()
/******************************************************************************/

/******************************************************************************/
long slab_wall_hug_route(struct Thing *thing, struct Coord3d *pos, long a3);
long get_next_position_and_angle_required_to_tunnel_creature_to(struct Thing *creatng, struct Coord3d *pos, unsigned char a3);
TbBool terrain_toxic_for_creature_at_position(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
long dig_to_position(PlayerNumber plyr_idx, MapSubtlCoord basestl_x, MapSubtlCoord basestl_y, int direction_around, TbBool revside);
TbBool slab_good_for_computer_dig_path(const struct SlabMap *slb);
short get_hug_side_options(MapSubtlCoord stl1_x, MapSubtlCoord stl1_y, MapSubtlCoord stl2_x, MapSubtlCoord stl2_y, unsigned short direction, PlayerNumber plyr_idx,
    MapSubtlCoord *ostla_x, MapSubtlCoord *ostla_y, MapSubtlCoord *ostlb_x, MapSubtlCoord *ostlb_y);
/******************************************************************************/
#define CREATURE_OWNER_FLAG (1 << creatng->owner) // set creature owner flag in crt_owner_bit
#define CHECK_SLAB_OWNER (((1 << slabmap_owner(slb)) & crt_owner_bit) != 0) // return TRUE if the creature and slab are owned by the same player, unless...
#define IGNORE_SLAB_OWNER_CHECK 0 // if crt_owner_bit == 0, CHECK_SLAB_OWNER will always return false
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
