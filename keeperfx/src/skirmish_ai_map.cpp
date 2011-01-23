/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_map.c
 *     Skirmish AI map analysis routines.
 * @par Purpose:
 *     Player independent map analysis functions and data structures to store
 *     results of said analysis.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "skirmish_ai_map.h"

#include "dungeon_data.h"
#include "slab_data.h"
#include "thing_data.h"

#include <assert.h>

//C++ dependencies start here
#include <set>
//end of C++ dependencies

using namespace std; //we try to be transparent to C


struct MapNode
{
    short x;
    short y;
    short dist;
};

typedef bool (*NodeCompareFunc)(const MapNode * lhs, const MapNode * rhs);
typedef std::set<MapNode *, NodeCompareFunc> NodeSet;


struct
{
    struct SAI_GoldArea gold[SAI_MAX_GOLD_AREAS];
    int num_gold_areas;
    struct SAI_TileAnalysis tiles[SAI_MAP_WIDTH * SAI_MAP_HEIGHT];
} static map_analysis;


static struct SAI_TileAnalysis null_tile = { 9999, 9999, 9999 };


static struct SAI_TileAnalysis * get_tile(int x, int y)
{
    if (x < 0 || y < 0 || x >= SAI_MAP_WIDTH || y >= SAI_MAP_HEIGHT) {
        return &null_tile;
    }

    return &map_analysis.tiles[y * SAI_MAP_WIDTH + x];
}

/**
 * Traces an area of gold and gems across vertical/horizontal/diagonal edges.
 * @param idx
 * @param x
 * @param y
 * @return Counts how "important" the trace is (for prioritizing what gold areas to
 * save in gold area array later).
 */
static int gold_trace(short idx, short x, short y)
{
    struct SAI_TileAnalysis * tile;
    struct SlabMap * slab;
    int sum;

    tile = get_tile(x, y);
    if (tile->gold_area_idx >= 0) {
        return 0;
    }

    slab = get_slabmap_block(x, y);
    if (slab->kind != SlbT_GOLD && slab->kind != SlbT_GEMS) {
        return 0;
    }

    tile->gold_area_idx = idx;
    sum = slab->kind == SlbT_GOLD? 1 : 25;
    sum += gold_trace(idx, x - 1, y);
    sum += gold_trace(idx, x - 1, y - 1);
    sum += gold_trace(idx, x, y - 1);
    sum += gold_trace(idx, x + 1, y - 1);
    sum += gold_trace(idx, x + 1, y);
    sum += gold_trace(idx, x + 1, y + 1);
    sum += gold_trace(idx, x, y + 1);
    sum += gold_trace(idx, x - 1, y + 1);

    return sum;
}

/**
 * Traces a connected area (that can in any way be passed except through teleport).
 * For many game functions, the AI can ignore everything that is not in the same
 * connected area as its dungeon heart.
 * @param idx
 * @param x
 * @param y
 */
static void connect_trace(short idx, short x, short y)
{
    struct SAI_TileAnalysis * tile;
    struct SlabMap * slab;

    tile = get_tile(x, y);
    if (tile->connected_area_idx >= 0) {
        return;
    }

    slab = get_slabmap_block(x, y);
    if (slab->kind == SlbT_ROCK || slab->kind == SlbT_GEMS) {
        return;
    }

    tile->connected_area_idx = idx;
    connect_trace(idx, x - 1, y);
    connect_trace(idx, x, y - 1);
    connect_trace(idx, x + 1, y);
    connect_trace(idx, x, y + 1);
}

/**
 * Traces an "open area", which is an area that can be traversed without bridges.
 * @param idx
 * @param x
 * @param y
 */
static void open_trace(short idx, short x, short y)
{
    struct SAI_TileAnalysis * tile;
    struct SlabMap * slab;

    tile = get_tile(x, y);
    if (tile->open_area_idx >= 0) {
        return;
    }

    slab = get_slabmap_block(x, y);
    if (slab->kind == SlbT_ROCK || slab->kind == SlbT_GEMS ||
            slab->kind == SlbT_WATER || slab->kind == SlbT_LAVA) {
        return;
    }

    tile->open_area_idx = idx;
    open_trace(idx, x - 1, y);
    open_trace(idx, x, y - 1);
    open_trace(idx, x + 1, y);
    open_trace(idx, x, y + 1);
}

static bool search_compare(const MapNode * lhs, const MapNode * rhs)
{
    return lhs->dist < rhs->dist;
}

static void try_add_player_trace_node(int reset_on_friendly, NodeSet * open,
    short plyr_idx, short x, short y, short dist)
{
    struct SAI_TileAnalysis * tile;
    struct SlabMap * slab;
    MapNode * node;

    tile = get_tile(x, y);
    if (reset_on_friendly) {
        if (tile->player_dists[plyr_idx] <= dist) {
            return;
        }
    }
    else {
        if (tile->heart_dists[plyr_idx] <= dist) {
            return;
        }
    }

    slab = get_slabmap_block(x, y);
    if (slab->kind == SlbT_ROCK) {
        return;
    }

    if (reset_on_friendly && slabmap_owner(slab) == plyr_idx) {
        dist = 0;
    }

    if (reset_on_friendly) {
        tile->player_dists[plyr_idx] = dist;
    }
    else {
        tile->heart_dists[plyr_idx] = dist;
    }

    node = (struct MapNode *) malloc(sizeof(*node));
    node->x = x;
    node->y = y;
    node->dist = dist;

    open->insert(node);
}

/**
 * Has two modes of operation: reset_on_friendly records distance of a player's reach.
 * This is done by setting node distance to 0 whenever friendly territory is processed.
 * !reset_on_friendly records distance to dungeon heart.
 * @param reset_on_friendly Operation mode.
 * @param plyr_idx
 * @param x Should be heart pos for first node.
 * @param y -/-
 */
static void player_trace(int reset_on_friendly, short plyr_idx, short x, short y)
{
    NodeSet open(search_compare);
    MapNode * node;

    //insert start node
    try_add_player_trace_node(reset_on_friendly, &open, plyr_idx, x, y, 0);

    //BFS loop
    while (!open.empty()) {
        node = *open.begin();
        open.erase(open.begin());

        try_add_player_trace_node(reset_on_friendly, &open,
            plyr_idx, node->x - 1, node->y, node->dist + 1);
        try_add_player_trace_node(reset_on_friendly, &open,
            plyr_idx, node->x, node->y - 1, node->dist + 1);
        try_add_player_trace_node(reset_on_friendly, &open,
            plyr_idx, node->x + 1, node->y, node->dist + 1);
        try_add_player_trace_node(reset_on_friendly, &open,
            plyr_idx, node->x, node->y + 1, node->dist + 1);

        free(node);
    }
}

static int gold_area_comparator(const struct SAI_GoldArea * a,
    const struct SAI_GoldArea * b)
{
    int diff;
    diff = b->temp - a->temp;
    if (diff != 0) {
        return diff;
    }

    return b - a; //ok since they are in same array
}

void SAI_init_map_analysis(void)
{
    int x, y, i;
    int found;
    struct SlabMap * slab;
    struct SAI_TileAnalysis * tile;
    short gold_area_count;
    short open_area_count;
    short connected_area_count;
    struct SAI_GoldArea * gold_areas;

    AIDBG(3, "Starting");

    memset(&map_analysis, 0, sizeof(map_analysis));

    gold_area_count = open_area_count = connected_area_count = 0;
    gold_areas = (struct SAI_GoldArea *) malloc(sizeof(*gold_areas) *
        SAI_MAP_WIDTH * SAI_MAP_HEIGHT);

    //init pass
    for (y = 0; y < SAI_MAP_HEIGHT; ++y) {
        for (x = 0; x < SAI_MAP_WIDTH; ++x) {
            tile = get_tile(x, y);
            tile->gold_area_idx = -1;
            tile->open_area_idx = -1;
            tile->connected_area_idx = -1;
        }
    }

    //feature trace pass
    for (y = 0; y < SAI_MAP_HEIGHT; ++y) {
        for (x = 0; x < SAI_MAP_WIDTH; ++x) {
            tile = get_tile(x, y);
            slab = get_slabmap_block(x, y);

            if ((slab->kind == SlbT_GOLD || slab->kind == SlbT_GEMS) &&
                    tile->gold_area_idx < 0) {
                gold_areas[gold_area_count].area_idx = gold_area_count;
                gold_areas[gold_area_count].temp = gold_trace(gold_area_count, x, y);
                gold_area_count += 1;
            }

            if (slab->kind != SlbT_ROCK && slab->kind != SlbT_GEMS &&
                    tile->connected_area_idx < 0) {
                connect_trace(connected_area_count++, x, y);
            }

            if (slab->kind != SlbT_ROCK && slab->kind != SlbT_GEMS &&
                    slab->kind != SlbT_WATER && slab->kind != SlbT_LAVA &&
                    tile->open_area_idx < 0) {
                open_trace(open_area_count++, x, y);
            }
        }
    }

    //filter gold areas
    qsort(gold_areas, gold_area_count, sizeof(*gold_areas),
        (int (*)(const void *, const void *)) gold_area_comparator);
    map_analysis.num_gold_areas = min(SAI_MAX_GOLD_AREAS, (int) gold_area_count);
    memcpy(map_analysis.gold, gold_areas,
        sizeof(*gold_areas) * map_analysis.num_gold_areas);
    free(gold_areas);

    //adjustment pass
    for (y = 0; y < SAI_MAP_HEIGHT; ++y) {
        for (x = 0; x < SAI_MAP_WIDTH; ++x) {
            tile = get_tile(x, y);

            //convert gold area indices
            found = 0;
            for (i = 0; i < map_analysis.num_gold_areas; ++i) {
                if (tile->gold_area_idx == map_analysis.gold[i].area_idx) {
                    tile->gold_area_idx = i;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                tile->gold_area_idx = -1;
            }
        }
    }

    //fix gold area indices
    for (i = 0; i < map_analysis.num_gold_areas; ++i) {
        map_analysis.gold[i].area_idx = i;
    }
}

void SAI_run_map_analysis(void)
{
    int i, j;
    struct PlayerInfo * plyr;
    struct Thing * heart;
    struct Dungeon * dungeon;

    AIDBG(3, "Starting");

    //clear state that will be modified
    for (i = 0; i < SAI_MAP_WIDTH * SAI_MAP_HEIGHT; ++i) {
        for (j = 0; j < SAI_MAX_KEEPERS; ++j) {
            //distances to player/dungeon hearts set to max so they can be minimized
            map_analysis.tiles[i].player_dists[j] = SAI_MAP_WIDTH * SAI_MAP_HEIGHT;
            map_analysis.tiles[i].heart_dists[j] = SAI_MAP_WIDTH * SAI_MAP_HEIGHT;
        }
    }
    for (i = 0; i < map_analysis.num_gold_areas; ++i) {
        for (j = 0; j < SAI_MAX_KEEPERS; ++j) {
            //distances to player/dungeon hearts set to max so they can be minimized
            map_analysis.gold[i].player_dists[j] = SAI_MAP_WIDTH * SAI_MAP_HEIGHT;
            map_analysis.gold[i].heart_dists[j] = SAI_MAP_WIDTH * SAI_MAP_HEIGHT;
        }
    }

    //calculate distances to players
    for (i = 0; i < SAI_MAX_KEEPERS; ++i) {
        plyr = get_player(i);
        if (!player_exists(plyr)) {
            continue;
        }

        dungeon = get_players_dungeon(plyr);
        heart = thing_get(dungeon->dnheart_idx);
        if (thing_is_invalid(heart)) {
            continue;
        }

        player_trace(0, i, heart->mappos.x.stl.num / 3, heart->mappos.y.stl.num / 3);
        player_trace(1, i, heart->mappos.x.stl.num / 3, heart->mappos.y.stl.num / 3);
    }
}

const struct SAI_GoldArea * SAI_get_gold_areas(int * count)
{
    *count = map_analysis.num_gold_areas;
    return map_analysis.gold;
}

const struct SAI_TileAnalysis * SAI_get_tile_analysis(int x, int y)
{
    if (x < 0 || y < 0 || x >= SAI_MAP_WIDTH || y >= SAI_MAP_HEIGHT) {
        return NULL;
    }

    return &map_analysis.tiles[y * SAI_MAP_WIDTH + x];
}

