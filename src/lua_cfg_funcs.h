/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_cfg_funcs.h
 *     Header file for lua_cfg_funcs.c.
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
#ifndef LUACFGFUNCS_H
#define LUACFGFUNCS_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_FUNCS_MAX       256
#define LUA_FUNCNAME_LENGHT 256

struct LuaFuncsConf{
    char lua_funcs[LUA_FUNCS_MAX][LUA_FUNCNAME_LENGHT];
};


FuncIdx get_function_idx(const char *func_name,const struct NamedCommand * Cfuncs);

#ifdef __cplusplus
}
#endif

#endif