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
#ifndef LUABASE_H
#define LUABASE_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_basics.h"
#include "../lib/lua/include/lua.h"

#ifdef __cplusplus
extern "C" {
#endif

TbBool CheckLua(lua_State *L, int r);
TbBool open_lua_script(LevelNumber lvnum);
void close_lua_script();

extern struct lua_State *Lvl_script;

#ifdef __cplusplus
}
#endif

#endif