/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_statehandler.c
 *     handles what the script is currently doing used by multiple files under lvl_script_*
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "lvl_script_statehandler.h"


#ifdef __cplusplus
extern "C" {
#endif

static int script_current_condition = 0;

int get_script_current_condition()
{
    return script_current_condition;
}

void set_script_current_condition(int current_condition)
{
    script_current_condition = current_condition;
}

#ifdef __cplusplus
}
#endif