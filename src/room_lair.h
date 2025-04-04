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
#include "creature_control.h"
#include "thing_data.h"
#include "room_data.h"
#include "dungeon_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
long calculate_free_lair_space(struct Dungeon * dungeon);
TbBool creature_model_is_lair_enemy(const short lair_enemy[LAIR_ENEMY_MAX], short crmodel);
struct Room *get_best_new_lair_for_creature(struct Thing *thing);
void count_lair_occupants_on_slab(struct Room *room,MapSlabCoord slb_x, MapSlabCoord slb_y);
void count_lair_occupants(struct Room *room);
struct Thing *find_lair_totem_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
