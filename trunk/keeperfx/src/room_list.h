/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_list.h
 *     Header file for room_list.c.
 * @par Purpose:
 *     Rooms array maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     01 Feb 2012 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_LIST_H
#define DK_ROOM_LIST_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Room;
struct Thing;

DLLIMPORT struct Room *_DK_start_rooms;
#define start_rooms _DK_start_rooms
DLLIMPORT struct Room *_DK_end_rooms;
#define end_rooms _DK_end_rooms

#pragma pack()
/******************************************************************************/
void clear_rooms(void);

long count_player_rooms_of_type(PlayerNumber plyr_idx, RoomKind rkind);
long count_player_rooms_entrances(PlayerNumber plyr_idx);
long calculate_player_num_rooms_built(PlayerNumber plyr_idx);

struct Room *get_player_room_of_kind_nearest_to(PlayerNumber plyr_idx, RoomKind rkind,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *retdist);
struct Room *get_player_room_any_kind_nearest_to(PlayerNumber plyr_idx,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *retdist);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
