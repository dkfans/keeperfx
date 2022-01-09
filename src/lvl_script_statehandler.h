/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_statehandler.h
 *     Header file for lvl_script.c.
 * @par Purpose:
 *     handles what the script is currently doing used by multiple files under lvl_script_*
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LVLSCRIPTSTATEHANDLER_H
#define DK_LVLSCRIPTSTATEHANDLER_H

int get_script_current_condition();
void set_script_current_condition(int current_condition);

#endif