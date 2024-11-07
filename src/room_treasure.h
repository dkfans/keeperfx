/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_treasure.h
 *     Header file for room_treasure.c.
 * @par Purpose:
 *     Hatchery room maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_TREASURE_H
#define DK_ROOM_TREASURE_H

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "thing_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
void count_gold_slabs_wth_effcncy(struct Room *room);
void count_gold_slabs_full(struct Room *room);
struct Thing *find_gold_hoarde_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *treasure_room_eats_gold_piles(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y, struct Thing *hoardtng);
void count_gold_hoardes_in_room(struct Room *room);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
