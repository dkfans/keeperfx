/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_base.h
 *     Header file for lua_base.c.
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
#ifndef LUABASE_H
#define LUABASE_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_basics.h"
#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

TbBool CheckLua(lua_State *L, int result,const char* func);
TbBool open_lua_script(LevelNumber lvnum);
void close_lua_script();

const char* get_lua_serialized_data(size_t *len);
void set_lua_serialized_data(const char* data, size_t len);

TbBool execute_lua_code_from_console(const char* code);
TbBool execute_lua_code_from_script(const char* code);

const char* lua_get_serialised_data(size_t *len);
void lua_set_serialised_data(const char *data, size_t len);
void cleanup_serialized_data();

void lua_set_random_seed(unsigned int seed);

extern struct lua_State *Lvl_script;

#ifdef __cplusplus
}
#endif

#endif