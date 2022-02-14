/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_entrance.h
 *     Header file for room_entrance.c.
 * @par Purpose:
 *     Entrance maintain functions.
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
#ifndef DK_ROOM_ENTRANCE_H
#define DK_ROOM_ENTRANCE_H

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
void process_entrance_generation(void);
struct Thing *create_creature_at_entrance(struct Room * room, ThingModel crtr_kind);

TbBool remove_creature_from_generate_pool(ThingModel crtr_kind);
TbBool creature_will_generate_for_dungeon(const struct Dungeon * dungeon, ThingModel crtr_kind);
/******************************************************************************/
TbBool update_creature_pool_state(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
