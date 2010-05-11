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

/******************************************************************************/
long find_from_task_list(long plyr_idx, long srch_tsk)
{
  struct Dungeon *dungeon;
  struct MapTask *task;
  long i;
  dungeon = get_dungeon(plyr_idx);
  for (i=0; i < dungeon->field_AF7; i++)
  {
    task = &dungeon->task_list[i%MAPTASKS_COUNT];
    if (task->field_1 == srch_tsk)
      return i;
  }
  return -1;
}

long find_dig_from_task_list(long a1, long a2)
{
    return _DK_find_dig_from_task_list(a1, a2);
}

long remove_from_task_list(long a1, long a2)
{
    return _DK_remove_from_task_list(a1, a2);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
