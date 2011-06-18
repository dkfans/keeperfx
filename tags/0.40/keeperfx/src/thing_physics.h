/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_physics.h
 *     Header file for thing_physics.c.
 * @par Purpose:
 *     Implementation of physics functions used for things.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 02 Mar 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_TNGPHYSICS_H
#define DK_TNGPHYSICS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;
struct Dungeon;

/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
void slide_thing_against_wall_at(struct Thing *thing, struct Coord3d *pos, long a3);
void bounce_thing_off_wall_at(struct Thing *thing, struct Coord3d *pos, long a3);
void remove_relevant_forces_from_thing_after_slide(struct Thing *thing, struct Coord3d *pos, long a3);
TbBool positions_equivalent(struct Coord3d *pos_a, struct Coord3d *pos_b);
long creature_cannot_move_directly_to(struct Thing *thing, struct Coord3d *pos);
void creature_set_speed(struct Thing *thing, long speed);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
