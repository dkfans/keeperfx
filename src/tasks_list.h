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

struct MapTask { // sizeof = 3
  unsigned char kind;
  unsigned short coords;
};

#pragma pack()
/******************************************************************************/
extern struct MapTask bad_map_task;
#define INVALID_MAP_TASK (&bad_map_task)
/******************************************************************************/
struct MapTask *get_task_list_entry(long plyr_idx, long task_idx);
struct MapTask *get_dungeon_task_list_entry(struct Dungeon *dungeon, long task_idx);
void add_task_list_entry(PlayerNumber plyr_idx, unsigned char kind, SubtlCodedCoords stl_num);
TbBool task_list_entry_invalid(struct MapTask *task);

long find_from_task_list(PlayerNumber plyr_idx, SubtlCodedCoords srch_tsk);
long find_from_task_list_by_slab(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
long find_from_task_list_by_subtile(PlayerNumber plyr_idx, MapSlabCoord stl_x, MapSlabCoord stl_y);
long find_dig_from_task_list(PlayerNumber plyr_idx, SubtlCodedCoords srch_tsk);
long remove_from_task_list(long a1, long a2);
long find_next_dig_in_dungeon_task_list(struct Dungeon *dungeon, long last_dig);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
