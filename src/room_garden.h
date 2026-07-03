/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_garden.h
 *     Header file for room_garden.c.
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
#ifndef DK_ROOM_GARDEN_H
#define DK_ROOM_GARDEN_H

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
TbBool remove_food_from_food_room_if_possible(struct Thing *thing);
short room_grow_food(struct Room *room);
TbBool recreate_repositioned_food_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
void reposition_all_food_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
int check_food_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void count_and_reposition_food_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
void count_food_in_room(struct Room *room);
TbBool room_create_new_food_at(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
