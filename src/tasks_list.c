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
#include "bflib_planar.h"

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
    // Find free task index
    int task_idx;
    struct MapTask  *mtask;
    for (task_idx=0; task_idx < dungeon->highest_task_number; task_idx++)
    {
        mtask = &dungeon->task_list[task_idx];
        if (mtask->kind == 0)
          break;
    }
    if (task_idx == dungeon->highest_task_number)
    {
        if (task_idx >= MAPTASKS_COUNT)
            return;
        dungeon->highest_task_number++;
    }
    // Fill the task
    MapSubtlCoord taskstl_x = stl_slab_center_subtile(stl_num_decode_x(stl_num));
    MapSubtlCoord taskstl_y = stl_slab_center_subtile(stl_num_decode_y(stl_num));
    mtask = &dungeon->task_list[task_idx];
    mtask->kind = kind;
    mtask->coords = get_subtile_number(taskstl_x, taskstl_y);
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
        if (mtask->coords == srch_tsk)
            return i;
  }
  return -1;
}

long find_from_task_list_by_slab(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SubtlCodedCoords srch_tsk = get_subtile_number_at_slab_center(slb_x, slb_y);
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long imax = dungeon->highest_task_number;
    if (imax > MAPTASKS_COUNT)
        imax = MAPTASKS_COUNT;
    for (long i = 0; i < imax; i++)
    {
        struct MapTask* mtask = &dungeon->task_list[i];
        if (mtask->coords == srch_tsk)
            return i;
  }
  return -1;
}

long find_from_task_list_by_subtile(PlayerNumber plyr_idx, MapSlabCoord stl_x, MapSlabCoord stl_y)
{
    SubtlCodedCoords srch_tsk = get_subtile_number(stl_slab_center_subtile(stl_x), stl_slab_center_subtile(stl_y));
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long imax = dungeon->highest_task_number;
    if (imax > MAPTASKS_COUNT)
        imax = MAPTASKS_COUNT;
    for (long i = 0; i < imax; i++)
    {
        struct MapTask* mtask = &dungeon->task_list[i];
        if (mtask->coords == srch_tsk)
            return i;
  }
  return -1;
}

long find_dig_from_task_list(PlayerNumber plyr_idx, SubtlCodedCoords srch_tsk)
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

long find_next_dig_in_dungeon_task_list(struct Dungeon *dungeon, long last_dig)
{
    int32_t mtasks_num = dungeon->highest_task_number;
    if (mtasks_num > MAPTASKS_COUNT)
        mtasks_num = MAPTASKS_COUNT;

    if (last_dig >= mtasks_num)
        return -1;

    const struct Coord3d* heart_pos = dungeon_get_essential_pos(dungeon->owner);
    int32_t heart_x = heart_pos->x.stl.num;
    int32_t heart_y = heart_pos->y.stl.num;
    int32_t task_order_stride = (MAX_SUBTILES_X + 2) * (MAX_SUBTILES_Y + 2);
    int32_t last_order = -1;
    if (last_dig >= 0) {
        SubtlCodedCoords last_coords = dungeon->task_list[last_dig].coords;
        int32_t last_task_x = stl_num_decode_x(last_coords);
        int32_t last_task_y = stl_num_decode_y(last_coords);
        int32_t last_task_dist = chessboard_distance(heart_x, heart_y, last_task_x, last_task_y);
        last_order = last_task_dist * task_order_stride + last_coords;
    }

    // Task slots can differ between multiplayer peers, so scan from the dungeon heart outwards.
    int32_t best_task_idx = -1;
    int32_t best_order = INT_MAX;
    for (int32_t i = 0; i < mtasks_num; i++) {
        struct MapTask* mtask = &dungeon->task_list[i];
        if (mtask->kind == SDDigTask_None)
            continue;
        int32_t task_x = stl_num_decode_x(mtask->coords);
        int32_t task_y = stl_num_decode_y(mtask->coords);
        int32_t task_dist = chessboard_distance(heart_x, heart_y, task_x, task_y);
        int32_t task_order = task_dist * task_order_stride + mtask->coords;
        if ((task_order <= last_order) || (task_order >= best_order))
            continue;
        best_task_idx = i;
        best_order = task_order;
    }
    return best_task_idx;
}

long remove_from_task_list(long plyr_idx, long stack_pos)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if ((stack_pos < 0) || (dungeon->highest_task_number <= stack_pos)) {
      ERRORLOG("Invalid stack pos");
      return 0;
    }
    struct MapTask* mtask = &dungeon->task_list[stack_pos];
    mtask->kind = 0;
    mtask->coords = 0;
    dungeon->task_count--;
    if (dungeon->highest_task_number - stack_pos == 1)
    {
        long i;
        for (i = stack_pos; i >= 0; i--)
        {
            mtask = &dungeon->task_list[i];
            if (mtask->kind != 0)
              break;
        }
        dungeon->highest_task_number = i + 1;
    }
    return 1;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
