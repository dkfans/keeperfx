/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_base.h
 *     Header file for lua_utils.c.
 * @par Purpose:
 *     Console commands
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef LUAUTILS_H
#define LUAUTILS_H

#include "globals.h"
#include "bflib_basics.h"
#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

bool try_get_from_methods(lua_State *L, int obj_index, const char *key);
bool try_get_c_method(lua_State *L, const char *key, const luaL_Reg *methods);

#ifdef __cplusplus
}
#endif

#endif