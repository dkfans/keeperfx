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
#include "tasks_list.h"

#include "globals.h"
#include "bflib_basics.h"

#include "spdigger_stack.h"
#include "map_data.h"
#include "dungeon_data.h"

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
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return INVALID_MAP_TASK;
    if ((task_idx < 0) || (task_idx >= MAPTASKS_COUNT))
        return INVALID_MAP_TASK;
    return &dungeon->task_list[task_idx];
}

void add_task_list_entry(PlayerNumber plyr_idx, unsigned char kind, SubtlCodedCoords stl_num)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        return;
    }
    // Find free task index
    int task_idx;
    struct MapTask  *mtask;
    for (task_idx=0; task_idx < dungeon->field_AF7; task_idx++)
    {
        mtask = &dungeon->task_list[task_idx];
        if (mtask->kind == 0)
          break;
    }
    if (task_idx == dungeon->field_AF7)
    {
        if (task_idx >= MAPTASKS_COUNT)
            return;
        dungeon->field_AF7++;
    }
    // Fill the task
    MapSubtlCoord taskstl_x, taskstl_y;
    taskstl_x = stl_slab_center_subtile(stl_num_decode_x(stl_num));
    taskstl_y = stl_slab_center_subtile(stl_num_decode_y(stl_num));
    mtask = &dungeon->task_list[task_idx];
    mtask->kind = kind;
    mtask->coords = get_subtile_number(taskstl_x, taskstl_y);
    dungeon->field_E8F++;
}

long find_from_task_list(PlayerNumber plyr_idx, SubtlCodedCoords srch_tsk)
{
  struct Dungeon *dungeon;
  struct MapTask *mtask;
  long i,imax;
  dungeon = get_dungeon(plyr_idx);
  imax = dungeon->field_AF7;
  if (imax > MAPTASKS_COUNT)
      imax = MAPTASKS_COUNT;
  for (i=0; i < imax; i++)
  {
      mtask = &dungeon->task_list[i];
      if (mtask->coords == srch_tsk)
          return i;
  }
  return -1;
}

long find_from_task_list_by_slab(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
  struct Dungeon *dungeon;
  struct MapTask *mtask;
  long i,imax;
  SubtlCodedCoords srch_tsk;
  srch_tsk = get_subtile_number_at_slab_center(slb_x, slb_y);
  dungeon = get_dungeon(plyr_idx);
  imax = dungeon->field_AF7;
  if (imax > MAPTASKS_COUNT)
      imax = MAPTASKS_COUNT;
  for (i=0; i < imax; i++)
  {
      mtask = &dungeon->task_list[i];
      if (mtask->coords == srch_tsk)
          return i;
  }
  return -1;
}

long find_from_task_list_by_subtile(PlayerNumber plyr_idx, MapSlabCoord stl_x, MapSlabCoord stl_y)
{
  struct Dungeon *dungeon;
  struct MapTask *mtask;
  long i,imax;
  SubtlCodedCoords srch_tsk;
  srch_tsk = get_subtile_number(stl_slab_center_subtile(stl_x), stl_slab_center_subtile(stl_y));
  dungeon = get_dungeon(plyr_idx);
  imax = dungeon->field_AF7;
  if (imax > MAPTASKS_COUNT)
      imax = MAPTASKS_COUNT;
  for (i=0; i < imax; i++)
  {
      mtask = &dungeon->task_list[i];
      if (mtask->coords == srch_tsk)
          return i;
  }
  return -1;
}

long find_dig_from_task_list(PlayerNumber plyr_idx, SubtlCodedCoords srch_tsk)
{
    struct Dungeon *dungeon;
    struct MapTask *mtask;
    long i,max;
    dungeon = get_dungeon(plyr_idx);
    max = dungeon->field_AF7;
    if (max > MAPTASKS_COUNT)
        max = MAPTASKS_COUNT;
    for (i=0; i < max; i++)
    {
      mtask = &dungeon->task_list[i];
      if ((mtask->coords == srch_tsk) && (mtask->kind != SDDigTask_Unknown3))
        return i;
    }
    return -1;
}

long find_next_dig_in_dungeon_task_list(struct Dungeon *dungeon, long last_dig)
{
    struct MapTask *mtask;
    long i,mtasks_num;
    mtasks_num = dungeon->field_AF7;
    if (mtasks_num > MAPTASKS_COUNT)
        mtasks_num = MAPTASKS_COUNT;
    for (i=last_dig+1; i < mtasks_num; i++)
    {
      mtask = &dungeon->task_list[i];
      if ((mtask->kind != SDDigTask_None) && (mtask->kind != SDDigTask_Unknown3))
        return i;
    }
    return -1;
}

TbBool task_list_entry_invalid(struct MapTask *task)
{
    if ((task == INVALID_MAP_TASK) || (task == NULL))
        return true;
    return false;
}

long remove_from_task_list(long plyr_idx, long stack_pos)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if ((stack_pos < 0) || (dungeon->field_AF7 <= stack_pos)) {
      ERRORLOG("Invalid stack pos");
      return 0;
    }
    struct MapTask *mtask;
    mtask = &dungeon->task_list[stack_pos];
    mtask->kind = 0;
    mtask->coords = 0;
    dungeon->field_E8F--;
    long i;
    if (dungeon->field_AF7 - stack_pos == 1)
    {
        for (i=stack_pos; i >= 0; i--)
        {
            mtask = &dungeon->task_list[i];
            if (mtask->kind != 0)
              break;
        }
        dungeon->field_AF7 = i + 1;
    }
    return 1;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
