/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_api_sound.h
 *     Header for Lua sound API
 * @par Purpose:
 *     Declarations for sound-related Lua functions
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     31 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef LUA_API_SOUND_H
#define LUA_API_SOUND_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration for Lua state
struct lua_State;
typedef struct lua_State lua_State;

/******************************************************************************/
// Lua API function declarations

/**
 * @brief Register all sound-related Lua functions
 * Call this during Lua initialization
 */
void register_lua_sound_api(lua_State* L);

// Individual Lua function wrappers (called by Lua)
int lua_LoadCustomSound(lua_State* L);
int lua_GetCustomSoundId(lua_State* L);
int lua_SetCreatureSound(lua_State* L);
int lua_IsCustomSoundLoaded(lua_State* L);
int lua_PlaySound(lua_State* L);
int lua_StopSound(lua_State* L);
int lua_PlayMusic(lua_State* L);
int lua_StopMusic(lua_State* L);

/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif // LUA_API_SOUND_H
/******************************************************************************/
