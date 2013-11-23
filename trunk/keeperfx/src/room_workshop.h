/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_workshop.h
 *     Header file for room_workshop.c.
 * @par Purpose:
 *     Workshop room maintain functions.
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
#ifndef DK_ROOM_WORKSHOP_H
#define DK_ROOM_WORKSHOP_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Count of possible manufacture types */
#define MANUFCTR_TYPES_COUNT  11
/** Maximal count of manufactured boxes of specific kind. */
#define MANUFACTURED_ITEMS_LIMIT 199

/******************************************************************************/
#pragma pack(1)

struct Room;
struct Thing;
struct Dungeon;

#pragma pack()

/**
 * Manufacture types data.
 * Originally was named TrapData, but stores both traps and doors; now no longer matches original.
 */
struct ManufactureData {
      long work_state; //< Work state used to place the manufactured item on map
      ThingClass tngclass; //< Thing class created when manufactured design is placed
      ThingModel tngmodel; //< Thing model created when manufactured design is placed
      short field_8;
      short parchment_spridx;
};
/******************************************************************************/
//DLLIMPORT extern struct TrapData _DK_trap_data[MANUFCTR_TYPES_COUNT];
/******************************************************************************/
struct ManufactureData *get_manufacture_data(int manufctr_idx);
int get_manufacture_data_index_for_thing(ThingClass tngclass, ThingModel tngmodel);
struct ManufactureData *get_manufacture_data_for_thing(ThingClass tngclass, ThingModel tngmodel);

TbBool add_workshop_object_to_workshop(struct Room *room,struct Thing *cratetng);
TbBool remove_workshop_object_from_workshop(struct Room *room,struct Thing *boxtng);
TbBool add_workshop_item(long plyr_idx, long wrkitm_class, long wrkitm_kind);
TbBool check_workshop_item_limit_reached(long plyr_idx, long wrkitm_class, ThingModel tngmodel);
long remove_workshop_item(long owner, long tngclass, long tngmodel);
long remove_workshop_object_from_player(PlayerNumber owner, long model);
TbBool get_next_manufacture(struct Dungeon *dungeon);
short process_player_manufacturing(PlayerNumber plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
