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
#include "thing_navigate.h"
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
    for (int i = 0; i < ROOMS_COUNT; i++)
    {
        memset(&game.rooms[i], 0, sizeof(struct Room));
  }
}

/**
 * Counts amount of rooms of specific type owned by specific player.
 * @param plyr_idx The player number. Only specific player number is accepted.
 * @param rkind Room kind to count. Only specific kind is accepted.
 * @note also named count_rooms_of_type()
 */
long count_player_rooms_of_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    struct DungeonAdd* dungeonadd = get_dungeonadd(plyr_idx);
    if (dungeonadd_invalid(dungeonadd))
        return 0;
    long i = dungeonadd->room_kind[rkind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // No Per-room code - we only want count
        SYNCDBG(19,"Player %d has %s at (%d,%d)",(int)plyr_idx, room_code_name(room->kind), (int)room->central_stl_x, (int)room->central_stl_y);
        if (room->owner != plyr_idx) {
            ERRORDBG(3,"Player %d has bad room in %s list; it's really %s index %d owned by player %d",(int)plyr_idx, room_code_name(rkind), room_code_name(room->kind), (int)room->index, (int)room->owner);
            break;
        }
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
 * Calculates amount of rooms in possession of a player.
 * Only rooms which are possible to build are really counted.
 * @param plyr_idx
 */
long calculate_player_num_rooms_built(PlayerNumber plyr_idx)
{
    long count = 0;
    struct PlayerInfo* player = get_player(plyr_idx);
    for (long rkind = 1; rkind < slab_conf.room_types_count; rkind++)
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
    long count = 0;
    long i = game.entrance_room_id;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(plyr_idx);
    long nearest_dist = LONG_MAX;
    struct Room* nearest_room = INVALID_ROOM;
    long i = dungeonadd->room_kind[rkind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
      }
      i = room->next_of_owner;
      // Per-room code
      long dist = abs(room->central_stl_y - stl_y) + abs(room->central_stl_x - stl_x);
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
    long nearest_dist = LONG_MAX;
    struct Room* nearest_room = INVALID_ROOM;
    for (RoomKind rkind = 1; rkind < slab_conf.room_types_count; rkind++)
    {
        long dist;
        struct Room* room = get_player_room_of_kind_nearest_to(plyr_idx, rkind, stl_x, stl_y, &dist);
        if (!room_is_invalid(room) && (dist < nearest_dist)) {
            nearest_dist = dist;
            nearest_room = room;
        }
    }
    if (retdist != NULL)
        *retdist = nearest_dist;
    return nearest_room;
}

struct Room * find_next_navigable_room_for_thing_with_capacity_and_closer_than(struct Thing *thing, int prev_room_idx, unsigned char nav_flags, long used, long *neardistance)
{
    unsigned long k = 0;
    int i = prev_room_idx;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        // Compute simplified distance - without use of mul or div
        long distance = abs(thing->mappos.x.stl.num - (int)room->central_stl_x) + abs(thing->mappos.y.stl.num - (int)room->central_stl_y);
        if ((*neardistance > distance) && (room->used_capacity >= used))
        {
            struct Coord3d pos;
            if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos))
            {
                TbBool is_connected;
                switch (thing->class_id)
                {
                case TCls_Creature:
                    is_connected = creature_can_navigate_to(thing, &pos, nav_flags);
                    break;
                default:
                    is_connected = navigation_points_connected(&thing->mappos, &pos);
                    break;
                }
                if (is_connected)
                {
                    *neardistance = distance;
                    return room;
                }
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return INVALID_ROOM;
}

struct Room * find_nearest_navigable_room_for_thing_with_capacity_and_closer_than(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags, long used, long *neardistance)
{
    SYNCDBG(18,"Searching for %s navigable by %s index %d",room_role_code_name(rrole),thing_model_name(thing),(int)thing->index);
    struct DungeonAdd* dungeonadd = get_dungeonadd(owner);
    struct Room* nearoom = INVALID_ROOM;
    long distance = *neardistance;
    for (RoomKind rkind = 0; rkind < slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            int i = dungeonadd->room_kind[rkind];
            while (i != 0)
            {
                struct Room* room = find_next_navigable_room_for_thing_with_capacity_and_closer_than(thing, i, nav_flags, used, &distance);
                if (room_is_invalid(room)) {
                    break;
                }
                // Found closer room
                i = room->next_of_owner;
                nearoom = room;
            }
            *neardistance = distance;
        }
    }
    return nearoom;
}

/**
 * Returns nearest room of given kind and owner to which the thing can navigate.
 *
 * @param thing The thing to navigate into room.
 * @param owner Owner of the rooms to be checked.
 * @param rkind Room kind to be returned.
 * @param nav_flags Navigation flags, for checking if creature can reach the room.
 * @return Nearest room of given kind and owner, or invalid room if none found.
 */
struct Room *find_nearest_room_of_role_for_thing(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags)
{
    long neardistance = LONG_MAX;
    struct Room* nearoom = find_nearest_navigable_room_for_thing_with_capacity_and_closer_than(thing, owner, rrole, nav_flags, 0, &neardistance);
    return nearoom;
}

/**
 * Returns any room of given kind and owner to which the thing can navigate.
 *
 * @param thing The thing to navigate into room.
 * @param owner Owner of the rooms to be checked.
 * @param rkind Room kind to be returned.
 * @param nav_flags Navigation flags, for checking if creature can reach the room.
 * @return Nearest room of given kind and owner, or invalid room if none found.
 */
struct Room *find_any_navigable_room_for_thing_closer_than(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags, long max_distance)
{
    struct DungeonAdd* dungeonadd = get_dungeonadd(owner);
    SYNCDBG(18,"Searching for %s navigable by %s index %d",room_role_code_name(rrole),thing_model_name(thing),(int)thing->index);
    long neardistance = max_distance;
    struct Room* nearoom = INVALID_ROOM;

    for (RoomKind rkind = 0; rkind < slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            nearoom = find_next_navigable_room_for_thing_with_capacity_and_closer_than(thing, dungeonadd->room_kind[rkind], nav_flags, 0, &neardistance);
            if(nearoom != INVALID_ROOM)
            {
                return nearoom;
            }
        }
    }
    return INVALID_ROOM;
}

/**
 * Returns nearest room of given kind and owner to which the thing can navigate, which has used capacity not lower than given amount.
 *
 * @param thing The thing to navigate into room.
 * @param owner Owner of the rooms to be checked.
 * @param rkind Room kind to be returned.
 * @param nav_flags Navigation flags, for checking if creature can reach the room.
 * @param used Used capacity required in the room to be returned.
 * @return Nearest room of given kind and owner, or invalid room if none found.
 */
struct Room *find_nearest_room_of_role_for_thing_with_used_capacity(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags, long used)
{
    long neardistance = LONG_MAX;
    struct Room* nearoom = find_nearest_navigable_room_for_thing_with_capacity_and_closer_than(thing, owner, rrole, nav_flags, used, &neardistance);
    return nearoom;
}
/******************************************************************************/

/**
* Returns nearest room of given owner to which the thing can navigate, and which can be destroyed.
*
* @param thing The thing to navigate into room.
* @param owner Owner of the rooms to be checked.
* @param nav_flags Navigation flags, for checking if creature can reach the room.
* @return Nearest room of given kind and owner, or invalid room if none found.
*/
struct Room *find_nearest_room_to_vandalise(struct Thing *thing, PlayerNumber owner, unsigned char nav_flags)
{
    long neardistance = LONG_MAX;
    struct Room* nearoom = INVALID_ROOM;
    for (RoomKind rkind = 1; rkind < slab_conf.room_types_count; rkind++)
    {
		if (room_cannot_vandalise(rkind)) {
			continue;
		}
        long distance = neardistance;
        struct Room* room = find_nearest_navigable_room_for_thing_with_capacity_and_closer_than(thing, owner, rkind, nav_flags, 0, &distance);
        if (neardistance > distance)
		{
			neardistance = distance;
			nearoom = room;
		}
	}
	return nearoom;
}
