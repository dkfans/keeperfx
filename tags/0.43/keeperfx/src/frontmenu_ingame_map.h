/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_map.h
 *     Header file for frontmenu_ingame_map.c.
 * @par Purpose:
 *     Map on in-game GUI panel drawing and support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 23 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_FRONTMENU_INGAMEMAP_H
#define DK_FRONTMENU_INGAMEMAP_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_clicked_on_small_map;
#define clicked_on_small_map _DK_clicked_on_small_map
DLLIMPORT unsigned char _DK_grabbed_small_map;
#define grabbed_small_map _DK_grabbed_small_map
/******************************************************************************/
void pannel_map_update(long x, long y, long w, long h);
void pannel_map_draw(long x, long y, long zoom);

void do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5);
short do_left_map_drag(long begin_x, long begin_y, long curr_x, long curr_y, long zoom);
short do_left_map_click(long begin_x, long begin_y, long curr_x, long curr_y, long zoom);
short do_right_map_click(long start_x, long start_y, long curr_x, long curr_y, long zoom);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
