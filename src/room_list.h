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
struct CompoundRoomFilterParam;

typedef struct CompoundRoomFilterParam * MaxRoomFilterParam;

/** Definition of a simple callback type which can only return true/false and has no memory of previous checks. */
typedef TbBool (*Room_Bool_Filter)(const struct Room *);
/** Definition of a callback type used for selecting best match through all the rooms by maximizing a value. */
typedef long (*Room_Maximizer_Filter)(const struct Room *, MaxRoomFilterParam, long);
/** Definition of a simple callback type which can only return true/false and can modify the room. */
typedef TbBool (*Room_Bool_Modifier)(struct Room *);

struct CompoundRoomFilterParam {
     long plyr_idx;
     long kind_id;
     union {
     long num1;
     void *ptr1;
     };
     union {
     long num2;
     void *ptr2;
     };
     union {
     long num3;
     void *ptr3;
     };
};

/******************************************************************************/

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

struct Room *find_any_navigable_room_for_thing_closer_than(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags, long max_distance);

struct Room *find_nearest_room_for_thing(struct Thing *thing, PlayerNumber plyr_idx, RoomKind rkind, unsigned char nav_flags);
struct Room *find_nearest_room_for_thing_excluding_two_types(struct Thing *thing, PlayerNumber owner, RoomKind skip_rkind1, RoomKind skip_rkind2, unsigned char nav_flags);
struct Room *find_nearest_room_for_thing_with_used_capacity(struct Thing *thing, PlayerNumber plyr_idx, RoomKind rkind, unsigned char a4, long a5);
struct Room *find_nearest_room_to_vandalise(struct Thing *thing, PlayerNumber owner, unsigned char nav_flags);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
