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

#include "dungeon_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_find_dig_from_task_list(long a1, long a2);
DLLIMPORT long _DK_remove_from_task_list(long a1, long a2);

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

long find_from_task_list(long plyr_idx, long srch_tsk)
{
  struct Dungeon *dungeon;
  struct MapTask *task;
  long i,imax;
  dungeon = get_dungeon(plyr_idx);
  imax = dungeon->field_AF7;
  if (imax > MAPTASKS_COUNT)
      imax = MAPTASKS_COUNT;
  for (i=0; i < imax; i++)
  {
    task = &dungeon->task_list[i];
    if (task->field_1 == srch_tsk)
      return i;
  }
  return -1;
}

long find_dig_from_task_list(long plyr_idx, long srch_tsk)
{
    //return _DK_find_dig_from_task_list(a1, a2);
    struct Dungeon *dungeon;
    struct MapTask *task;
    long i,max;
    dungeon = get_dungeon(plyr_idx);
    max = dungeon->field_AF7;
    if (max > MAPTASKS_COUNT)
        max = MAPTASKS_COUNT;
    for (i=0; i < max; i++)
    {
      task = &dungeon->task_list[i];
      if ((task->field_1 == srch_tsk) && (task->field_0 != 3))
        return i;
    }
    return -1;
}

long find_next_dig_in_dungeon_task_list(struct Dungeon *dungeon, long last_dig)
{
    struct MapTask *task;
    long i,max;
    max = dungeon->field_AF7;
    if (max > MAPTASKS_COUNT)
        max = MAPTASKS_COUNT;
    for (i=last_dig+1; i < max; i++)
    {
      task = &dungeon->task_list[i];
      if ((task->field_0 != 0) && (task->field_0 != 3))
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

long remove_from_task_list(long a1, long a2)
{
    return _DK_remove_from_task_list(a1, a2);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
