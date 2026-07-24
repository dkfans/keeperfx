/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file tasks_list.h
 *     Header file for tasks_list.c.
 * @par Purpose:
 *     Tasks list support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_TASKSLIST_H
#define DK_TASKSLIST_H

#include "globals.h"
#include "bflib_basics.h"

#define MAPTASKS_COUNT        300

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Dungeon;

struct MapTask {
  unsigned char kind;
  SubtlCodedCoords coords;
};

#pragma pack()
/******************************************************************************/
extern struct MapTask bad_map_task;
#define INVALID_MAP_TASK (&bad_map_task)
/******************************************************************************/
struct MapTask *get_task_list_entry(int32_t plyr_idx, int32_t task_idx);
struct MapTask *get_dungeon_task_list_entry(struct Dungeon *dungeon, int32_t task_idx);
void add_task_list_entry(PlayerNumber plyr_idx, unsigned char kind, SubtlCodedCoords stl_num);

int32_t find_from_task_list(PlayerNumber plyr_idx, SubtlCodedCoords srch_tsk);
int32_t find_from_task_list_by_slab(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
int32_t find_from_task_list_by_subtile(PlayerNumber plyr_idx, MapSlabCoord stl_x, MapSlabCoord stl_y);
int32_t find_dig_from_task_list(PlayerNumber plyr_idx, SubtlCodedCoords srch_tsk);
int32_t remove_from_task_list(int32_t a1, int32_t a2);
int32_t find_next_dig_in_dungeon_task_list(struct Dungeon *dungeon, int32_t last_dig);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
