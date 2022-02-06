/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_commands.h
 *     Header file for lvl_script_commands.c.
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
#ifndef DK_LVLSCRIPTCOMMANDS_H
#define DK_LVLSCRIPTCOMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvl_script_lib.h"

void script_add_command(const struct CommandDesc *cmd_desc, const struct ScriptLine *scline);


#ifdef __cplusplus
}
#endif
#endif
