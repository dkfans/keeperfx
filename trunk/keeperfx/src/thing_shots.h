/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_shots.h
 *     Header file for thing_shots.c.
 * @par Purpose:
 *     Shots support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THING_SHOTS_H
#define DK_THING_SHOTS_H

#include "bflib_basics.h"
#include "globals.h"
#include "room_workshop.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;



#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
/******************************************************************************/
struct Thing *create_shot(struct Coord3d *pos, unsigned short model, unsigned short owner);
long update_shot(struct Thing *thing);

TbBool shot_is_slappable(const struct Thing *thing, long plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
