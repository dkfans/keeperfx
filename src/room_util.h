/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_util.h
 *     Header file for room_util.c.
 * @par Purpose:
 *     Generic utility and maintain functions for rooms.
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
#ifndef DK_ROOM_UTIL_H
#define DK_ROOM_UTIL_H

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "thing_data.h"
#include "dungeon_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
void process_rooms(void);

TbBool delete_room_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, TbBool is_destroyed);
TbBool replace_slab_from_script(MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char slabkind);
void change_slab_owner_from_script(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx);
TbBool check_and_asimilate_thing_by_room(struct Thing *thing);
EventIndex update_cannot_find_room_of_role_wth_spare_capacity_event(PlayerNumber plyr_idx, struct Thing *creatng, RoomRole rrole);
void query_room(struct Room *room);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
