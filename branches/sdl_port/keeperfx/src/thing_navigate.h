/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_navigate.h
 *     Header file for thing_navigate.c.
 * @par Purpose:
 *     Things movement navigation functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_TNGNAVIGATE_H
#define DK_TNGNAVIGATE_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;


/******************************************************************************/
#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
TbBool setup_person_move_to_position(struct Thing *thing, long stl_x, long stl_y, unsigned char a4);
TbBool creature_can_travel_over_lava(struct Thing *thing);
TbBool creature_can_navigate_to(struct Thing *thing, struct Coord3d *pos, TbBool no_owner);
TbBool creature_can_navigate_to_with_storage(struct Thing *crtng, struct Coord3d *pos, unsigned char storage);
TbBool creature_can_get_to_dungeon(struct Thing *thing, long plyr_idx);
struct Thing *find_hero_door_hero_can_navigate_to(struct Thing *herotng);

long creature_move_to_using_gates(struct Thing *thing, struct Coord3d *pos, short a3, long a4, long a5, unsigned char a6);
short move_to_position(struct Thing *thing);
long creature_turn_to_face(struct Thing *thing, struct Coord3d *pos);
long creature_turn_to_face_backwards(struct Thing *thing, struct Coord3d *pos);
long creature_turn_to_face_angle(struct Thing *thing, long a2);
void creature_set_speed(struct Thing *thing, short speed);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
