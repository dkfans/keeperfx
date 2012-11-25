/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_panel.h
 *     Header file for gui_panel.c.
 * @par Purpose:
 *     Left GUI panel drawing and support functions.
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

#ifndef DK_GUIPANEL_H
#define DK_GUIPANEL_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
void pannel_map_update(long x, long y, long w, long h);
void gui_set_button_flashing(long btn_idx, long gameturns);
short zoom_to_fight(unsigned char a1);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
