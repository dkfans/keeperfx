/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_list.c
 *     Rooms array maintain functions.
 * @par Purpose:
 *     Functions to maintain and browse list of rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     01 Feb 2012 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_list.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_stats.h"
#include "config_terrain.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void clear_rooms(void)
{
  int i;
  for (i=0; i < ROOMS_COUNT; i++)
  {
    memset(&game.rooms[i], 0, sizeof(struct Room));
  }
}

/**
 * Counts amount of rooms of specific type owned by specific player.
 * @param plyr_idx The player number. Only specific player number is accepted.
 * @param rkind Room kind to count. Only specific kind is accepted.
 */
long count_player_rooms_of_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long i;
    unsigned long k;
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return 0;
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // No Per-room code - we only want count
        SYNCDBG(19,"Player %d has %s at (%d,%d)",(int)plyr_idx, room_code_name(room->kind), (int)room->central_stl_x, (int)room->central_stl_y);
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return k;
}

/**
 * Calculates amount of buildable rooms in possession of a player.
 * @param plyr_idx
 */
long calculate_player_num_rooms_built(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    long rkind;
    long count;
    count = 0;
    player = get_player(plyr_idx);
    for (rkind=1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        if (!room_never_buildable(rkind))
        {
            count += count_player_rooms_of_type(player->id_number, rkind);
        }
    }
    return count;
}

long count_player_rooms_entrances(PlayerNumber plyr_idx)
{
    struct Room *room;
    long i;
    unsigned long k;
    long count;
    count = 0;
    i = game.entrance_room_id;
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_kind;
        // Per-room code
        if ((plyr_idx < 0) || (room->owner == plyr_idx))
            count++;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return count;
}

struct Room *get_player_room_of_kind_nearest_to(PlayerNumber plyr_idx, RoomKind rkind,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *retdist)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long i;
    unsigned long k;
    dungeon = get_dungeon(plyr_idx);
    struct Room *nearest_room;
    long nearest_dist;
    nearest_dist = LONG_MAX;
    nearest_room = INVALID_ROOM;
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
      room = room_get(i);
      if (room_is_invalid(room))
      {
          ERRORLOG("Jump to invalid room detected");
          break;
      }
      i = room->next_of_owner;
      // Per-room code
      long dist;
      dist = abs(room->central_stl_y - stl_y) + abs(room->central_stl_x - stl_x);
      if (dist < nearest_dist)
      {
          nearest_dist = dist;
          nearest_room = room;
      }
      // Per-room code ends
      k++;
      if (k > ROOMS_COUNT)
      {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
      }
    }
    if (retdist != NULL)
        *retdist = nearest_dist;
    return nearest_room;
}

struct Room *get_player_room_any_kind_nearest_to(PlayerNumber plyr_idx,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *retdist)
{
    struct Room *nearest_room;
    long nearest_dist;
    nearest_dist = LONG_MAX;
    nearest_room = INVALID_ROOM;
    RoomKind rkind;
    for (rkind = 1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        struct Room *room;
        long dist;
        room = get_player_room_of_kind_nearest_to(plyr_idx, rkind, stl_x, stl_y, &dist);
        if (!room_is_invalid(room) && (dist < nearest_dist)) {
            nearest_dist = dist;
            nearest_room = room;
        }
    }
    if (retdist != NULL)
        *retdist = nearest_dist;
    return nearest_room;
}
/******************************************************************************/
