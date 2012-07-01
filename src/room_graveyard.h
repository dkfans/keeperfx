/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_graveyard.h
 *     Header file for room_graveyard.c.
 * @par Purpose:
 *     Workshop room maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     01 Feb 2012 - 01 Jul 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_GRAVEYARD_H
#define DK_ROOM_GRAVEYARD_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Room;
struct Thing;

#pragma pack()
/******************************************************************************/
TbBool add_body_to_graveyard(struct Thing *corpse, struct Room *room);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
