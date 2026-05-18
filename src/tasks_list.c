/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file tasks_list.c
 *     Tasks list support functions.
 * @par Purpose:
 *     Functions to manage and use list of tasks.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "tasks_list.h"

#include "globals.h"
#include "bflib_basics.h"

#include "spdigger_stack.h"
#include "map_data.h"
#include "dungeon_data.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct MapTask bad_map_task;
/******************************************************************************/
struct MapTask *get_dungeon_task_list_entry(struct Dungeon *dungeon, long task_idx)
{
    if ((task_idx < 0) || (task_idx >= MAPTASKS_COUNT))
        return INVALID_MAP_TASK;
    return &dungeon->task_list[task_idx];
}

struct MapTask *get_task_list_entry(long plyr_idx, long task_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return INVALID_MAP_TASK;
    if ((task_idx < 0) || (task_idx >= MAPTASKS_COUNT))
        return INVALID_MAP_TASK;
    return &dungeon->task_list[task_idx];
}

void add_task_list_entry(PlayerNumber plyr_idx, unsigned char kind, SubtlCodedCoords stl_num)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        return;
    }
    MapSubtlCoord taskstl_x = stl_slab_center_subtile(stl_num_decode_x(stl_num));
    MapSubtlCoord taskstl_y = stl_slab_center_subtile(stl_num_decode_y(stl_num));
    SubtlCodedCoords coords = get_subtile_number(taskstl_x, taskstl_y);
    int32_t task_idx = dungeon->highest_task_number;
    if (task_idx >= MAPTASKS_COUNT) {
        for (task_idx = 0; task_idx < MAPTASKS_COUNT; task_idx++) {
            if (dungeon->task_list[task_idx].kind == SDDigTask_None)
                break;
        }
        if (task_idx >= MAPTASKS_COUNT)
            return;
    } else {
        dungeon->highest_task_number++;
    }
    dungeon->task_list[task_idx].kind = kind;
    dungeon->task_list[task_idx].coords = coords;
    dungeon->task_count++;
}

long find_from_task_list(PlayerNumber plyr_idx, SubtlCodedCoords srch_tsk)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long imax = dungeon->highest_task_number;
    if (imax > MAPTASKS_COUNT)
        imax = MAPTASKS_COUNT;
    for (long i = 0; i < imax; i++)
    {
        struct MapTask* mtask = &dungeon->task_list[i];
        if ((mtask->kind != SDDigTask_None) && (mtask->coords == srch_tsk))
            return i;
  }
  return -1;
}

long find_from_task_list_by_slab(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    return find_from_task_list(plyr_idx, get_subtile_number_at_slab_center(slb_x, slb_y));
}

long find_from_task_list_by_subtile(PlayerNumber plyr_idx, MapSlabCoord stl_x, MapSlabCoord stl_y)
{
    return find_from_task_list(plyr_idx, get_subtile_number(stl_slab_center_subtile(stl_x), stl_slab_center_subtile(stl_y)));
}

long find_next_dig_in_dungeon_task_list(struct Dungeon *dungeon, long last_dig)
{
    long mtasks_num = dungeon->highest_task_number;
    if (mtasks_num > MAPTASKS_COUNT)
        mtasks_num = MAPTASKS_COUNT;
    for (long i = last_dig + 1; i < mtasks_num; i++)
    {
        struct MapTask* mtask = &dungeon->task_list[i];
        if ((mtask->kind != SDDigTask_None))
            return i;
    }
    return -1;
}

long remove_from_task_list(long plyr_idx, long stack_pos)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if ((stack_pos < 0) || (dungeon->highest_task_number <= stack_pos)) {
      ERRORLOG("Invalid stack pos");
      return 0;
    }
    dungeon->highest_task_number--;
    dungeon->task_list[stack_pos] = dungeon->task_list[dungeon->highest_task_number];
    dungeon->task_count--;
    dungeon->task_list[dungeon->highest_task_number].kind = SDDigTask_None;
    dungeon->task_list[dungeon->highest_task_number].coords = 0;
    return 1;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
