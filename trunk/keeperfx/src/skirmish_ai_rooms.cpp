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

#include "config_terrain.h"
#include "keeperfx.hpp"

#include <assert.h>

//start of C++ dependencies
#include <set> //again I can't do without set for pathfinding
//end of C++ dependencies

using namespace std; //we try to be transparent to C


#define PATH_COST_BASE_UNIT     100

struct PlayerRooms {
    int my_idx; //player

    //layout data
    struct SAI_Room array[SAI_MAX_ROOMS];
    struct SAI_Room * state_lists[SAI_ROOM_STATE_COUNT];
    int markers[SAI_MAP_WIDTH * SAI_MAP_HEIGHT];

    //settings
    enum SAI_WallBreakOptions wall_break_mode;
} static player_rooms[SAI_MAX_KEEPERS];

typedef int (*PathFunc)(struct SAI_Point pos, void * args);
typedef bool (*PathNodeCompare)(const struct PathNode * lhs, const struct PathNode * rhs);

struct PathNode
{
    struct PathNode * parent;
    int cost_sum;
    int heuristic;
    struct SAI_Point pos;
    int should_dig;
};

typedef std::set<PathNode *, PathNodeCompare> PathNodeSet;

struct Pathfinder
{
    PathFunc goal_func;
    PathFunc cost_func;
    PathFunc heuristic_func;
    PathFunc should_dig_func;
    void * args;
    PathNodeSet open;
    PathNodeSet closed;

    //no choice but have C++ constructor for this context
    Pathfinder(PathNodeCompare open_cmp, PathNodeCompare closed_cmp) :
        open(open_cmp), closed(closed_cmp) { }
};

struct RoomPathArgs
{
    struct PlayerRooms * rooms;
    struct SAI_Room * room;
    struct SAI_Point search_pos;
};


static struct PlayerRooms * rooms_for_player(int plyr_idx)
{
    assert(plyr_idx >= 0);
    assert(plyr_idx < SAI_MAX_KEEPERS);
    return &player_rooms[plyr_idx];
}

static void set_room_state(struct PlayerRooms * rooms, struct SAI_Room * room,
    enum SAI_RoomState new_state)
{
    AIDBG(5, "Starting");
    assert(new_state >= 0);
    assert(new_state < SAI_ROOM_STATE_COUNT);

    //remove from old state list
    if (rooms->state_lists[room->state] == room) {
        rooms->state_lists[room->state] = room->next_of_state;
    }
    if (room->next_of_state) {
        room->next_of_state->prev_of_state = room->prev_of_state;
    }
    if (room->prev_of_state) {
        room->prev_of_state->next_of_state = room->next_of_state;
    }

    //add to new state list
    room->state = new_state;
    room->prev_of_state = NULL;
    room->next_of_state = rooms->state_lists[room->state];
    rooms->state_lists[room->state] = room;

    if (room->next_of_state) {
        room->next_of_state->prev_of_state = room;
    }
}

static void find_room_extents(const struct Room * room, struct SAI_Rect * rect)
{
    int i;
    signed char x, y;

    AIDBG(5, "Starting");
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

    assert(rect->l >= 0);
    assert(rect->t >= 0);
    assert(rect->r < SAI_MAP_WIDTH);
    assert(rect->b < SAI_MAP_HEIGHT);
}

static struct SAI_Room * obtain_unused_room(struct PlayerRooms * rooms)
{
    struct SAI_Room * room;

    if (rooms->state_lists[SAI_ROOM_UNUSED] == NULL) {
        ERRORLOG("Out of rooms for AI player %i", rooms->my_idx);
        return NULL;
    }

    room = rooms->state_lists[SAI_ROOM_UNUSED];

    return room;
}

static int any_rooms_marked_on_rect(const struct PlayerRooms * rooms,
    struct SAI_Rect rect)
{
    int x, y;

    AIDBG(11, "Starting");

    for (y = rect.t; y <= rect.b; ++y) {
        for (x = rect.l; x <= rect.r; ++x) {
            if (rooms->markers[y * SAI_MAP_WIDTH + x] >= 0) {
                return 1;
            }
        }
    }

    return 0;
}

static int plannable_tile(int x, int y)
{
    struct SlabMap * slab;

    slab = get_slabmap_block(x, y);
    switch (slab->kind) {
    case SlbT_CLAIMED:
    case SlbT_PATH:
    case SlbT_EARTH:
    case SlbT_TORCHDIRT:
    case SlbT_WALLDRAPE:
    case SlbT_WALLTORCH:
    case SlbT_WALLWTWINS:
    case SlbT_WALLWWOMAN:
    case SlbT_WALLPAIRSHR:
    case SlbT_GOLD:
        return 1;
    default:
        return 0;
    }
}

static int pathable_tile(int x, int y, int allow_bridge)
{
    struct SlabMap * slab;

    slab = get_slabmap_block(x, y);
    switch (slab->kind) {
    case SlbT_ROCK:
        return 0;
    case SlbT_WATER:
    case SlbT_LAVA:
        return allow_bridge;
    default:
        return 1;
    }
}

static int cant_layout_room_in_rect(int plyr, struct SAI_Rect rect)
{
    int x, y;

    for (y = rect.t; y <= rect.b; ++y) {
        for (x = rect.l; x <= rect.r; ++x) {
            //TODO AI: check owner for some of these
            if (!plannable_tile(x, y)) {
                return 1;
            }
        }
    }

    return 0;
}

static int room_misses_slabs(const struct SAI_Room * plan, const struct Room * room)
{
    return room->slabs_count < SAI_rect_width(plan->rect) * SAI_rect_height(plan->rect);
}

static int room_too_small(const struct SAI_Room * room)
{
    return  SAI_rect_width(room->rect) < 3 ||
            SAI_rect_height(room->rect) < 3; //for now: possibly context dependent later
}

static void mark_room(struct PlayerRooms * rooms, const struct SAI_Room * room)
{
    int x, y;

    AIDBG(5, "Starting for room with rect %i, %i, %i, %i",
        room->rect.l, room->rect.t, room->rect.r, room->rect.b);

    for (y = room->rect.t; y <= room->rect.b; ++y) {
        for (x = room->rect.l; x <= room->rect.r; ++x) {
            rooms->markers[y * SAI_MAP_WIDTH + x] = room->id;
        }
    }
}

static void eval_players_room(int plyr, struct Room * room)
{
    struct SAI_Room * r;
    struct PlayerRooms * rooms;

    if (room->slabs_list == 0) {
        return; //for some reason this occurs
    }

    AIDBG(4, "Starting");
    rooms = rooms_for_player(plyr);

    r = obtain_unused_room(rooms);
    if (r == NULL) {
        return;
    }

    r->kind = (enum RoomKinds) room->kind;
    find_room_extents(room, &r->rect);

    if (any_rooms_marked_on_rect(rooms, r->rect)) {
        //don't want to handle overlap due to added complexity
        set_room_state(rooms, r, SAI_ROOM_BAD);
        //don't mark on map; other rooms must keep precedence
    }
    else if (room_misses_slabs(r, room) || room_too_small(r)) {
        set_room_state(rooms, r, SAI_ROOM_INCOMPLETE);
        mark_room(rooms, r);
    }
    else {
        set_room_state(rooms, r, SAI_ROOM_ACTIVE);
        mark_room(rooms, r);
    }
}

static void clear_player_rooms(int plyr_idx)
{
    int i;
    struct PlayerRooms * rooms;

    AIDBG(3, "Starting");
    rooms = rooms_for_player(plyr_idx);

    rooms->my_idx = plyr_idx;
    memset(&rooms->array, 0, sizeof(rooms->array));
    memset(&rooms->state_lists, 0, sizeof(rooms->state_lists));

    for (i = 0; i < SAI_MAX_ROOMS; ++i) {
        rooms->array[i].id = i;
        rooms->array[i].state = SAI_ROOM_UNUSED;
        rooms->array[i].next_of_state = &rooms->array[i+1];
        rooms->array[i].prev_of_state = &rooms->array[i-1];
    }

    rooms->array[0].prev_of_state = NULL;
    rooms->array[SAI_MAX_ROOMS - 1].next_of_state = NULL;
    rooms->state_lists[SAI_ROOM_UNUSED] = &rooms->array[0];

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

    AIDBG(3, "Starting");
    rooms = rooms_for_player(plyr_idx);
    plyr = get_player(plyr_idx);
    dungeon = get_players_dungeon(plyr);

    for (i = 0; i < ROOM_TYPES_COUNT; ++i) {
        for (j = dungeon->room_kind[i]; j != 0; j = room->next_of_owner) {
            room = room_get(i);
            if (!room_is_invalid(room)) {
                eval_players_room(plyr_idx, room);

                if (rooms->state_lists[SAI_ROOM_UNUSED] == NULL) {
                    break;
                }
            }
        }

        if (rooms->state_lists[SAI_ROOM_UNUSED] == NULL) {
            break;
        }
    }
}

static void evaluate_empty_space_in_player_reach(int plyr)
{

}

static int try_extend_in_direction(struct PlayerRooms * rooms,
    const struct SAI_Room * src_room, int dir_x, int dir_y)
{
    int x, y, r;
    int x2, y2;
    struct SAI_Rect rect;
    struct SAI_Room * room;

    AIDBG(4, "Starting");
    room = obtain_unused_room(rooms);
    if (room == NULL) {
        return 0;
    }

    if (dir_x == 0) {
        x = (src_room->rect.l + src_room->rect.r) / 2;
    } else if (dir_x < 0) {
        x = src_room->rect.l - 1;
    }
    else {
        x = src_room->rect.r + 1;
    }

    if (dir_y == 0) {
        y = (src_room->rect.t + src_room->rect.b) / 2;
    }
    else if (dir_y < 0) {
        y = src_room->rect.t - 1;
    }
    else {
        y = src_room->rect.b + 1;
    }

    for (r = 7; r >= 5; --r) { //+2 radius for walls, so this is 5x5 to 3x3 in practice
        x2 = x + dir_x * (r - 1);
        y2 = y + dir_y * (r - 1);

        if (x == x2) {
            rect.l = x - r / 2;
            rect.r = rect.l + r - 1;
        }
        else {
            rect.l = min(x, x2);
            rect.r = max(x, x2);
        }

        if (y == y2) {
            rect.t = y - r / 2;
            rect.b = rect.t + r - 1;
        }
        else {
            rect.t = min(y, y2);
            rect.b = max(y, y2);
        }

        rect.l = max((int) rect.l, 0);
        rect.t = max((int) rect.t, 0);
        rect.r = min((int) rect.r, SAI_MAP_WIDTH - 1);
        rect.b = min((int) rect.b, SAI_MAP_HEIGHT - 1);

        AIDBG(11, "Rect after adjustment is %i, %i, %i, %i", rect.l, rect.t, rect.r, rect.b);

        if (rect.r - rect.l < 4 || rect.b - rect.t < 4) {
            AIDBG(11, "Room to small after r = %i", r);
            continue;
        }

        if (any_rooms_marked_on_rect(rooms, rect)) {
            AIDBG(11, "Rooms already in vicinity at r = %i", r);
            continue;
        }

        if (cant_layout_room_in_rect(rooms->my_idx, rect)) {
            AIDBG(11, "Terrain is bad for r = %i", r);
            continue;
        }

        //remove walls from rect as final adjustment
        rect.l += 1;
        rect.t += 1;
        rect.r -= 1;
        rect.b -= 1;

        memcpy(&room->rect, &rect, sizeof(rect));
        set_room_state(rooms, room, SAI_ROOM_UNDUG);
        mark_room(rooms, room);

        AIDBG(6, "Managed to extend plan in direction (%i, %i) with radius %i",
            dir_x, dir_y, r);
        return 1;
    }

    return 0;
}

static int extend_layout(struct PlayerRooms * rooms)
{
    int i;
    int change;
    struct SAI_Room * room;

    change = 0;

    //looks for rooms close to existing ones (preferable - efficient packing)
    for (i = 0; i < SAI_MAX_ROOMS && rooms->state_lists[SAI_ROOM_UNUSED]; ++i) {
        room = &rooms->array[i];

        if (    room->state != SAI_ROOM_UNUSED &&
                room->state != SAI_ROOM_BAD) {
            //TODO AI: shuffle direction to make it appear random? right now, west will
            //always be preferred
            change = try_extend_in_direction(rooms, room, -1, 0) || change;
            change = try_extend_in_direction(rooms, room, 0, 1) || change;
            change = try_extend_in_direction(rooms, room, 1, 0) || change;
            change = try_extend_in_direction(rooms, room, 0, -1) || change;
        }
    }

    if (!change && rooms->state_lists[SAI_ROOM_UNUSED]) {
        //TODO AI: look for farther rooms
    }

    return change;
}

static int look_for_room_of_size(struct PlayerRooms * rooms, enum RoomKinds kind,
    int min_size, int max_size, SAI_RoomState state)
{
    int size;
    SAI_Room * room;

    AIDBG(5, "Starting");

    for (room = rooms->state_lists[state]; room != NULL; room = room->next_of_state) {
        if (room->kind != RoK_NONE) {
            AIDBG(8, "Room already had kind %i assigned", room->kind);
            continue;
        }

        size = SAI_rect_width(room->rect) * SAI_rect_height(room->rect);

        if (size >= min_size && size <= max_size) {
            AIDBG(8, "Room picked with size %i", size);
            return room->id;
        }

        AIDBG(8, "Size %i not in desired range [%i, %i]", size, min_size, max_size);
    }

    return -1;
}

static bool path_node_position_compare(const struct PathNode * lhs,
    const struct PathNode * rhs)
{
    if (lhs->pos.x == rhs->pos.x) {
        return lhs->pos.y < rhs->pos.y;
    }

    return lhs->pos.x < rhs->pos.x;
}

static bool path_node_score_compare(const struct PathNode * lhs,
    const struct PathNode * rhs)
{
    int lhs_score, rhs_score;
    lhs_score = lhs->cost_sum + lhs->heuristic;
    rhs_score = rhs->cost_sum + rhs->heuristic;

    if (lhs_score == rhs_score) {
        return path_node_position_compare(lhs, rhs);
    }

    return lhs_score < rhs_score;
}

static void try_insert_node(struct SAI_Point pos, struct PathNode * parent,
    struct Pathfinder * pf)
{
    struct PathNode node;
    struct PathNode * heap_node;
    int cost;

    if (pos.x < 0 || pos.y < 0 || pos.x >= SAI_MAP_WIDTH || pos.y >= SAI_MAP_HEIGHT) {
        return;
    }

    cost = pf->cost_func(pos, pf->args);
    if (cost < 0) {
        return;
    }

    node.pos = pos;
    if (pf->closed.find(&node) != pf->closed.end()) { //only compares pos AFAIK
        return;
    }

    node.heuristic = pf->heuristic_func(pos, pf->args);
    node.parent = parent;
    node.should_dig = pf->should_dig_func(pos, pf->args);

    if (parent) {
        node.cost_sum = parent->cost_sum + cost;
    }
    else {
        node.cost_sum = 0;
    }

    //create copy of node on heap
    heap_node = (struct PathNode *) malloc(sizeof(*heap_node));
    memcpy(heap_node, &node, sizeof(*heap_node));

    //insert heap copy
    pf->open.insert(heap_node);
    pf->closed.insert(heap_node);

    AIDBG(15, "Inserted node for position %i, %i", pos.x, pos.y);
}

static int find_path(struct SAI_Point start, struct SAI_Point ** coords,
    PathFunc goal_func, PathFunc cost_func, PathFunc heuristic_func,
    PathFunc should_dig_func, void * args)
{
    struct SAI_Point pos;
    struct PathNode * node, * goal;
    PathNodeSet::iterator it;
    int i;
    int path_len;
    struct Pathfinder pf(path_node_score_compare, path_node_position_compare);

    AIDBG(5, "Starting");

    *coords = NULL;

    pf.args = args;
    pf.cost_func = cost_func;
    pf.goal_func = goal_func;
    pf.heuristic_func = heuristic_func;
    pf.should_dig_func = should_dig_func;

    try_insert_node(start, NULL, &pf);

    path_len = 0;
    while (!pf.open.empty()) {
        node = *pf.open.begin();
        pf.open.erase(pf.open.begin());

        if (goal_func(node->pos, args)) {
            path_len = 1;
            break;
        }

        pos = node->pos;
        pos.x += 1;
        try_insert_node(pos, node, &pf);
        pos = node->pos;
        pos.x -= 1;
        try_insert_node(pos, node, &pf);
        pos = node->pos;
        pos.y += 1;
        try_insert_node(pos, node, &pf);
        pos = node->pos;
        pos.y -= 1;
        try_insert_node(pos, node, &pf);
    }

    if (path_len == 1) {
        path_len = 0;
        for (goal = node; node->parent != NULL; node = node->parent) {
            if (node->should_dig) {
                path_len += 1;
            }
        }

        *coords = (struct SAI_Point *) malloc(sizeof(**coords) * path_len);
        i = 0;
        for (node = goal; node->parent != NULL; node = node->parent) {
            if (node->should_dig) {
                (*coords)[i++] = node->pos;
            }
        }

        AIDBG(6, "Path consisting of %i tiles found", path_len);
    }
    else {
        AIDBG(6, "No path found");
    }

    AIDBG(6, "%i nodes inspected", pf.closed.size());
    for (it = pf.closed.begin(); it != pf.closed.end(); ++it) {
        free(*it);
    }

    return path_len;
}

static int room_path_should_dig(struct SAI_Point pos, struct RoomPathArgs * args)
{
    return 1; //TODO AI: implement
}

static int room_path_goal(struct SAI_Point pos, struct RoomPathArgs * args)
{
    return SAI_point_in_rect(pos, args->room->rect);
}

static int room_path_cost(struct SAI_Point pos, struct RoomPathArgs * args)
{
    int cost;
    int i;
    int dist, proximity;
    const struct SAI_TileAnalysis * tile;

    if (!pathable_tile(pos.x, pos.y, is_room_available(args->rooms->my_idx, RoK_BRIDGE))) {
        return -1;
    }

    cost = PATH_COST_BASE_UNIT;

    //TODO: check ownership, if we own tile, it is reasonable to give only lowest cost

    if (args->rooms->wall_break_mode != SAI_WB_PREFER) {
        //find distance of closest enemy
        dist = SAI_MAP_WIDTH + SAI_MAP_HEIGHT;
        tile = SAI_get_tile_analysis(pos.x, pos.y);
        for (i = 0; i < SAI_MAX_KEEPERS; ++i) {
            if (i != args->rooms->my_idx && tile->player_dists[i] > 0) {
                dist = min(dist, (int) tile->player_dists[i]);
            }
        }

        proximity = SAI_MAP_WIDTH + SAI_MAP_HEIGHT - dist;
        if (args->rooms->wall_break_mode == SAI_WB_AVOID) {
            cost += 3 * proximity / 2;
        }
        else {
            cost += proximity / 2;
        }
    }

    AIDBG(16, "Returning cost %i", cost);
    return cost;
}

static int room_path_heuristic(struct SAI_Point pos, struct RoomPathArgs * args)
{
    //manhattan heuristic
    return (abs((int) pos.x - args->search_pos.x) +
        abs((int) pos.y - args->search_pos.y)) * PATH_COST_BASE_UNIT;
}

void SAI_init_room_layout_for_player(int plyr)
{
    AIDBG(3, "Starting");

    clear_player_rooms(plyr);
    evaluate_rooms_of_player(plyr);
    evaluate_empty_space_in_player_reach(plyr);
}

void SAI_set_room_layout_safety(int plyr, enum SAI_WallBreakOptions wall_break)
{
    struct PlayerRooms * rooms;

    rooms = rooms_for_player(plyr);
    rooms->wall_break_mode = wall_break;
}

int SAI_request_room(int plyr, enum RoomKinds kind, int min_size, int max_size)
{
    int id;
    int res;
    struct PlayerRooms * rooms;

    AIDBG(4, "Starting for room kind %i, %i <= size <= %i", (int) kind, min_size,
        max_size);

    rooms = rooms_for_player(plyr);
    id = -1;

    res = look_for_room_of_size(rooms, kind, min_size, max_size, SAI_ROOM_EMPTY);
    id = max(id, res);

    while (id < 0) {
        res = look_for_room_of_size(rooms, kind, min_size, max_size, SAI_ROOM_UNDUG);
        id = max(id, res);

        if (id < 0 && !extend_layout(rooms)) {
            break;
        }
    }

    if (id >= 0) {
        rooms->array[id].kind = kind;
    }

    return id;
}

int SAI_find_path_to_room(int plyr, int room_id, struct SAI_Point ** coords)
{
    struct PlayerRooms * rooms;
    struct RoomPathArgs args;
    struct SAI_Point start;

    assert(room_id >= 0);
    assert(room_id < SAI_MAX_ROOMS);
    rooms = rooms_for_player(plyr);

    args.rooms = rooms;
    args.room = &rooms->array[room_id];
    args.search_pos = SAI_rect_center(args.room->rect);
    start = SAI_get_dungeon_heart_position(plyr);

    AIDBG(5, "Will try to find path from %i, %i to room at %i, %i",
        start.x, start.y, args.room->rect.l, args.room->rect.t);
    return find_path(start, coords, (PathFunc) room_path_goal, (PathFunc) room_path_cost,
        (PathFunc) room_path_heuristic, (PathFunc) room_path_should_dig, &args);
}

int SAI_find_path_to_gold_area(int plyr, int gold_area_idx,
    struct SAI_Point ** coords)
{
    //TODO AI: implement
    *coords = NULL;
    return 0;
}

int SAI_find_path_to_position(int plyr, struct SAI_Point pos,
    struct SAI_Point ** coords)
{
    //TODO AI: implement when required
    *coords = NULL;
    return 0;
}

struct SAI_Room * SAI_get_room(int plyr, int id)
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

    assert(room_id >= 0);
    assert(room_id < SAI_MAX_ROOMS);
    rooms = rooms_for_player(plyr);
    room = &rooms->array[room_id];
    assert(room->state == SAI_ROOM_EMPTY || room->state == SAI_ROOM_UNDUG);
    assert(room->kind == RoK_NONE);

    room->kind = kind;
}

struct SAI_Room * SAI_get_rooms_of_state(int plyr, enum SAI_RoomState state)
{
    struct PlayerRooms * rooms;

    assert(state >= 0);
    assert(state < SAI_ROOM_STATE_COUNT);
    rooms = rooms_for_player(plyr);

    return rooms->state_lists[state];
}

void SAI_set_room_state(int plyr, int room_id, enum SAI_RoomState new_state)
{
    struct PlayerRooms * rooms;
    assert(room_id >= 0);
    assert(room_id < SAI_MAX_ROOMS);

    rooms = rooms_for_player(plyr);
    set_room_state(rooms, &rooms->array[room_id], new_state);
}

int SAI_rect_width(struct SAI_Rect rect)
{
    int w;
    w = rect.r - rect.l + 1;
    assert(w >= 1);
    return w;
}

int SAI_rect_height(struct SAI_Rect rect)
{
    int h;
    h = rect.b - rect.t + 1;
    assert(h >= 1);
    return h;
}

struct SAI_Point SAI_rect_center(struct SAI_Rect rect)
{
    struct SAI_Point center;
    center.x = ((int) rect.l + rect.r) / 2;
    center.y = ((int) rect.t + rect.b) / 2;
    return center;
}

int SAI_point_in_rect(struct SAI_Point p, struct SAI_Rect rect)
{
    return p.x >= rect.l && p.x <= rect.r && p.y >= rect.t && p.y <= rect.b;
}

