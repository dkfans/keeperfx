/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_map.h
 *     Header file for skirmish_ai_map.c
 * @par Purpose:
 *     Map analysis routines for Skirmish AI.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef SKIRMISH_AI_MAP_H
#define SKIRMISH_AI_MAP_H

#include "skirmish_ai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAI_MAX_GOLD_AREAS  64 //so quad word bit mask is enough
#define SAI_MAP_WIDTH       85
#define SAI_MAP_HEIGHT      85

struct SAI_GoldArea
{
    int gold_left;
    struct SAI_Rect extents;
    short area_idx;
    short temp; //used for init calculations
    short diamond_sides;
    short player_dists[SAI_MAX_KEEPERS]; //shortest to players of all gold blocks in area
    short heart_dists[SAI_MAX_KEEPERS]; //shortest to dungeon hearts of all gold blocks in area
};

struct SAI_TileAnalysis
{
    short gold_area_idx;
    short open_area_idx; //which is passable without bridge
    short connected_area_idx; //ever passable without teleport
    short player_dists[SAI_MAX_KEEPERS]; //distance to player territory
    short heart_dists[SAI_MAX_KEEPERS]; //distance to dungeon hearts
};

/**
 * Initializes map analysis for the current map state.
 */
void SAI_init_map_analysis(void);

/**
 * Runs the map analysis routines for current state.
 */
void SAI_run_map_analysis(void);

/**
 * Gets a pointer to the gold area array.
 * @param count The number of array elements are stored here.
 * @return
 */
const struct SAI_GoldArea * SAI_get_gold_areas(int * count);

/**
 * Gets a point to the map tile analysis.
 * @param x X coordinate in tiles/slabs.
 * @param y Y coordinate in tiles/slabs.
 * @return
 */
const struct SAI_TileAnalysis * SAI_get_tile_analysis(int x, int y);

/**
 * Counts the number of 'open' (not wall, not rock, not earth) tiles in a rectangle.
 * @param rect
 * @return
 */
int SAI_count_open_tiles_in_rect(struct SAI_Rect rect);

/**
 * Gets position of dungeon heart for a player (convenience method).
 * @param plyr
 * @return
 */
struct SAI_Point SAI_get_dungeon_heart_position(int plyr);


#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_MAP_H
