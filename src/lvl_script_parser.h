/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_parser.h
 *     Header file for lvl_script_parser.c.
 * @par Purpose:
 *     should only be used by files under lvl_script_*
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LVLSCRIPTPARSER_H
#define DK_LVLSCRIPTPARSER_H

#ifdef __cplusplus
extern "C" {
#endif

int get_script_current_condition();
void set_script_current_condition(int current_condition);

#ifdef __cplusplus
}
#endif
#endif