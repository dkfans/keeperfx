/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_api_lens.h
 *     Header file for lua_api_lens.c.
 * @par Purpose:
 *     LUA API for dynamic lens effect creation.
 * @par Comment:
 *     Allows LUA scripts to create custom lens effects with callbacks.
 * @author   KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef LUA_API_LENS_H
#define LUA_API_LENS_H

#include <lua.h>
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

/**
 * Register lens API functions with LUA.
 * Called during LUA initialization.
 */
void Lens_register(lua_State *L);

/**
 * Cleanup lens LUA state.
 * Called during shutdown.
 */
void Lens_cleanup_lua(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
