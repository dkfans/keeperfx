/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_lair.h
 *     Header file for room_lair.c.
 * @par Purpose:
 *     Lair room and creature lairs maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 05 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_LAIR_H
#define DK_ROOM_LAIR_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Room;
struct Thing;
struct Dungeon;

#pragma pack()
/******************************************************************************/
long calculate_free_lair_space(struct Dungeon * dungeon);
struct Room *get_best_new_lair_for_creature(struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
