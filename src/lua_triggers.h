/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_base.h
 *     Header file for lua_base.c.
 * @par Purpose:
 *     Console commands
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Sim
 * @date     07 Jul 2020 - 07 Jul 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef LUATRIGGERS_H
#define LUATRIGGERS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

void lua_chatmsg(PlayerNumber plyr_idx, char *msg);
void lua_game_start();
void lua_game_tick();
void lua_on_power_cast(PlayerNumber plyr_idx, PowerKind pwkind,unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing);
void lua_on_special_box_activate(PlayerNumber plyr_idx, struct Thing *cratetng);
#ifdef __cplusplus
}
#endif

#endif