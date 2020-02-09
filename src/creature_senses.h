/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_senses.h
 *     Header file for creature_senses.c.
 * @par Purpose:
 *     Creature senses checks and handling.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     27 Nov 2011 - 22 Jan 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRSENSES_H
#define DK_CRTRSENSES_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;

#pragma pack()
/******************************************************************************/
TbBool jonty_creature_can_see_thing_including_lava_check(const struct Thing *creatng, const struct Thing *thing);
TbBool sibling_line_of_sight_ignoring_door(const struct Coord3d *prevpos,
    const struct Coord3d *nextpos, const struct Thing *doortng);
#define sibling_line_of_sight(prevpos, nextpos) sibling_line_of_sight_ignoring_door(prevpos, nextpos, INVALID_THING)

TbBool line_of_sight_3d(const struct Coord3d *frpos, const struct Coord3d *topos);
TbBool line_of_sight_2d(const struct Coord3d *pos1, const struct Coord3d *pos2);
TbBool line_of_sight_3d_ignoring_specific_door(const struct Coord3d *frpos, const struct Coord3d *topos, const struct Thing *doortng);
TbBool nowibble_line_of_sight_3d(const struct Coord3d *frpos, const struct Coord3d *topos);

long get_explore_sight_distance_in_slabs(const struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
