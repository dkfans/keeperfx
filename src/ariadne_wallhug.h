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
extern const short slab_around[];
extern const unsigned short small_around_pos[4];
#define AROUND_EIGHT_LENGTH 8
extern struct Around const my_around_eight[];
#define AROUND_NINE_LENGTH 9
extern struct Around const my_around_nine[];
extern short const around_map[];
#define AROUND_MAP_LENGTH 9
/******************************************************************************/
int small_around_index_in_direction(long srcpos_x, long srcpos_y, long dstpos_x, long dstpos_y);

long slab_wall_hug_route(struct Thing *thing, struct Coord3d *pos, long a3);
signed char get_starting_angle_and_side_of_hug(struct Thing *creatng, struct Coord3d *pos, long *a3, unsigned char *a4, long a5, unsigned char a6);
long get_angle_of_wall_hug(struct Thing *creatng, long a2, long a3, unsigned char a4);
void set_hugging_pos_using_blocked_flags(struct Coord3d *dstpos, struct Thing *creatng, unsigned short block_flags, int nav_radius);
TbBool navigation_push_towards_target(struct Navigation *navi, struct Thing *creatng, const struct Coord3d *pos, MoveSpeed speed, MoveSpeed nav_radius, unsigned char a3);
long check_forward_for_prospective_hugs(struct Thing *creatng, struct Coord3d *pos, long a3, long a4, long a5, long a6, unsigned char a7);
TbBool find_approach_position_to_subtile(const struct Coord3d *srcpos, MapSubtlCoord stl_x, MapSubtlCoord stl_y, MoveSpeed spacing, struct Coord3d *aproachpos);
long creature_cannot_move_directly_to_with_collide(struct Thing *creatng, struct Coord3d *pos, long a3, unsigned char a4);
unsigned short get_hugging_blocked_flags(struct Thing *creatng, struct Coord3d *pos, long a3, unsigned char a4);
long get_next_position_and_angle_required_to_tunnel_creature_to(struct Thing *creatng, struct Coord3d *pos, unsigned char a3);
TbBool terrain_toxic_for_creature_at_position(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
long dig_to_position(PlayerNumber plyr_idx, MapSubtlCoord basestl_x, MapSubtlCoord basestl_y, int direction_around, TbBool revside);
TbBool slab_good_for_computer_dig_path(const struct SlabMap *slb);
short get_hug_side_options(MapSubtlCoord stl1_x, MapSubtlCoord stl1_y, MapSubtlCoord stl2_x, MapSubtlCoord stl2_y, unsigned short a6, PlayerNumber plyr_idx,
    MapSubtlCoord *ostla_x, MapSubtlCoord *ostla_y, MapSubtlCoord *ostlb_x, MapSubtlCoord *ostlb_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
