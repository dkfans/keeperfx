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
#include "room_data.h"
#include "thing_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
TbBool add_body_to_graveyard(struct Thing *corpse, struct Room *room);
void reposition_all_bodies_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
TbBool rectreate_repositioned_body_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
int check_bodies_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void count_and_reposition_bodies_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos);
void count_bodies_in_room(struct Room *room);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
