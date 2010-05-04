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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif


#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
long find_from_task_list(long plyr_idx, long srch_tsk);
long find_dig_from_task_list(long a1, long a2);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
