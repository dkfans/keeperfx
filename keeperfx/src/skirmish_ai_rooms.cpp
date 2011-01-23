/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_rooms.cpp
 *     Dungeon/room layout routines for Skirmish AI.
 * @par Purpose:
 *     Controls where a Skirmish AI computer player digs out rooms and paths.
 *     Focus is on the spatial understanding; the actual action issuing and
 *     strategic motivation is found in other modules.
 * @par Comment:
 *     None.
 * @author   Keeper FX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "skirmish_ai_rooms.h"
#include "skirmish_ai.h"
#include "skirmish_ai_map.h"

#include "keeperfx.hpp"

#include <assert.h>

//start of C++ dependencies
#include <set> //again I can't do without set for pathfinding
//end of C++ dependencies

using namespace std; //we try to be transparent to C


#define SAI_MAX_ROOMS   1000

struct PlayerRooms {
    int my_idx; //player

    //layout data
    struct SAI_Room array[SAI_MAX_ROOMS];
    int count;
    int last_used;
    int markers[SAI_MAP_WIDTH * SAI_MAP_HEIGHT];

    //settings
    int path_safety_dist;
    int room_safety_dist;
    enum SAI_WallBreakOptions wall_break_mode;
} static player_rooms[SAI_MAX_KEEPERS];


static struct PlayerRooms * rooms_for_player(int plyr_idx)
{
    assert(plyr_idx >= 0);
    assert(plyr_idx < SAI_MAX_KEEPERS);
    return &player_rooms[plyr_idx];
}

static void find_room_extents(const struct Room * room, struct SAI_Rect * rect)
{
    int i;
    signed char x, y;

    assert(room->slabs_list != 0);

    i = room->slabs_list;
    rect->l = rect->r = slb_num_decode_x(i);
    rect->t = rect->b = slb_num_decode_y(i);
    i = get_next_slab_number_in_room(i);

    while (i != 0) {
        x = slb_num_decode_x(i);
        y = slb_num_decode_y(i);
        rect->l = min(rect->l, x);
        rect->r = max(rect->r, x);
        rect->t = min(rect->t, y);
        rect->b = max(rect->b, y);
        i = get_next_slab_number_in_room(i);
    }
}

static int obtain_unused_room_id(struct PlayerRooms * rooms)
{
    int i;

    if (rooms->count >= SAI_MAX_ROOMS) {
        ERRORLOG("Out of rooms for AI player %i", rooms->my_idx);
        return -1;
    }

    i = rooms->last_used; //probably taken, but next iteration we find an empty one
    do {
        if (rooms->array[i].state == SAI_ROOM_UNUSED) {
            rooms->last_used = i;
            rooms->count += 1;
            rooms->array[i].id = i;
            return i;
        }
        i = (i + 1) % SAI_MAX_ROOMS;
    } while (i != rooms->last_used);

    ERRORLOG("Bad room number");
    return -1;
}

static int any_rooms_marked_on_rect(const struct PlayerRooms * rooms,
    struct SAI_Rect * rect)
{
    int x, y;

    for (y = rect->t; y <= rect->b; ++y) {
        for (x = rect->l; x <= rect->r; ++x) {
            if (rooms->markers[y * SAI_MAP_WIDTH + x] >= 0) {
                return 1;
            }
        }
    }

    return 0;
}

static int room_misses_slabs(const struct SAI_Room * plan, const struct Room * room)
{
    int w, h;
    w = plan->rect.r - plan->rect.l + 1;
    h = plan->rect.b - plan->rect.t + 1;

    return room->slabs_count < w * h;
}

static int room_too_small(const struct SAI_Room * room)
{
    int w, h;
    w = room->rect.r - room->rect.l + 1;
    h = room->rect.b - room->rect.t + 1;

    return w < 3 || h < 3; //for now: possibly context dependent later
}

static void mark_room(struct PlayerRooms * rooms, const struct SAI_Room * room)
{
    int x, y;

    for (y = room->rect.t; y <= room->rect.b; ++y) {
        for (x = room->rect.l; x <= room->rect.r; ++x) {
            rooms->markers[y * SAI_MAP_WIDTH + x] = room->id;
        }
    }
}

static void eval_players_room(int plyr, struct Room * room)
{
    int id;
    struct SAI_Room * r;
    struct PlayerRooms * rooms;

    rooms = rooms_for_player(plyr);

    id = obtain_unused_room_id(rooms);
    if (id < 0) {
        return;
    }

    r = &rooms->array[id];
    r->kind = (enum RoomKinds) room->kind;
    find_room_extents(room, &r->rect);

    if (any_rooms_marked_on_rect(rooms, &r->rect)) {
        r->state = SAI_ROOM_BAD; //don't want to handle overlap due to added complexity
        //don't mark on map; other rooms must keep precedence
    }
    else if (room_misses_slabs(r, room) || room_too_small(r)) {
        r->state = SAI_ROOM_INCOMPLETE;
        mark_room(rooms, r);
    }
    else {
        r->state = SAI_ROOM_ACTIVE;
        mark_room(rooms, r);
    }
}

static void clear_player_rooms(int plyr_idx)
{
    int i;
    struct PlayerRooms * rooms;

    rooms = rooms_for_player(plyr_idx);

    rooms->my_idx = plyr_idx;
    rooms->count = 0;
    memset(&rooms->array, 0, sizeof(rooms->array));
    for (i = 0; i < SAI_MAP_WIDTH * SAI_MAP_HEIGHT; ++i) {
        rooms->markers[i] = -1;
    }
}

static void evaluate_rooms_of_player(int plyr_idx)
{
    int i, j;
    struct PlayerInfo * plyr;
    struct Dungeon * dungeon;
    struct Room * room;
    struct PlayerRooms * rooms;

    rooms = rooms_for_player(plyr_idx);
    plyr = get_player(plyr_idx);
    dungeon = get_players_dungeon(plyr);


    for (i = 0; i < ROOM_TYPES_COUNT; ++i) {
        for (j = dungeon->room_kind[i]; j != 0; j = room->next_of_owner) {
            room = room_get(i);
            if (!room_is_invalid(room)) {
                eval_players_room(plyr_idx, room);

                if (rooms->count >= SAI_MAX_ROOMS) {
                    break;
                }
            }
        }

        if (rooms->count >= SAI_MAX_ROOMS) {
            break;
        }
    }
}

static void evaluate_empty_space_in_player_reach(int plyr)
{

}

static int extend_layout(struct PlayerRooms * rooms)
{
    return 0;
}

static int look_for_room_of_size(struct PlayerRooms * rooms, enum RoomKinds kind,
    int min_size, int max_size, SAI_RoomState state)
{
    int i;
    int w, h;
    int size;
    SAI_Room * room;

    for (i = 0; i < SAI_MAX_ROOMS; ++i) {
        room = &rooms->array[i];
        if (room->state != state || room->kind != RoK_NONE) {
            continue;
        }

        w = room->rect.r - room->rect.l + 1;
        h = room->rect.b - room->rect.t + 1;
        size = w * h;

        if (size >= min_size && size <= max_size) {
            return i;
        }
    }

    return -1;
}

void SAI_init_room_layout_for_player(int plyr)
{
    clear_player_rooms(plyr);
    evaluate_rooms_of_player(plyr);
    evaluate_empty_space_in_player_reach(plyr);
}

void SAI_set_room_layout_safety(int plyr, int path_safety_dist,
    int room_safety_dist, enum SAI_WallBreakOptions wall_break)
{
    struct PlayerRooms * rooms;

    rooms = rooms_for_player(plyr);
    rooms->path_safety_dist = path_safety_dist;
    rooms->room_safety_dist = room_safety_dist;
    rooms->wall_break_mode = wall_break;
}

int SAI_request_room(int plyr, enum RoomKinds kind, int min_size, int max_size)
{
    int id;
    struct PlayerRooms * rooms;

    rooms = rooms_for_player(plyr);
    id = -1;

    id = max(id, look_for_room_of_size(rooms, kind,
        min_size, max_size, SAI_ROOM_EMPTY));

    while (id < 0) {
        id = max(id, look_for_room_of_size(rooms, kind,
            min_size, max_size, SAI_ROOM_UNDUG));

        if (id < 0 && !extend_layout(rooms)) {
            break;
        }
    }

    return id;
}

int SAI_request_path_to_room(int plyr, int id, struct SAI_Point ** coords)
{
    return 0;
}

int SAI_request_path_to_gold_area(int plyr, int gold_area_idx,
    struct SAI_Point ** coords)
{
    //TODO AI: implement
    return 0;
}

int SAI_request_path_to_position(int plyr, struct SAI_Point pos,
    struct SAI_Point ** coords)
{
    //TODO AI: implement when required
    return 0;
}

const struct SAI_Room * SAI_get_room(int plyr, int id)
{
    struct PlayerRooms * rooms;
    if (id < 0 || id >= SAI_MAX_ROOMS) {
        ERRORLOG("Invalid AI room ID %i of player %i requested", id, plyr);
        //rather than assert because callers can handle NULL, although it is an
        //unnatural condition
        return NULL;
    }

    rooms = rooms_for_player(plyr);
    if (rooms->array[id].state == SAI_ROOM_UNUSED) {
        return NULL;
    }

    return &rooms->array[id];
}

void SAI_set_room_kind(int plyr, int room_id, enum RoomKinds kind)
{
    struct PlayerRooms * rooms;
    struct SAI_Room * room;

    assert(room_id >= 0 && room_id < SAI_MAX_ROOMS);
    rooms = rooms_for_player(plyr);
    room = &rooms->array[room_id];
    assert(room->state == SAI_ROOM_EMPTY || room->state == SAI_ROOM_UNDUG);
    assert(room->kind == RoK_NONE);

    room->kind = kind;
}
