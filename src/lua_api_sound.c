/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_api_sound.c
 *     Lua API for sound management
 * @par Purpose:
 *     Exposes sound management functions to Lua scripts
 * @par Comment:
 *     Allows modders to load custom sounds and control audio at runtime
 * @author   KeeperFX Team
 * @date     31 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "lua_api_sound.h"
#include "sound_manager.h"
#include "bflib_sound.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/**
 * @brief Lua: LoadCustomSound(name, filepath)
 * Loads a custom WAV file and returns its sample ID
 * 
 * @param L Lua state
 * @return Number of return values (1: sample_id)
 * 
 * Example:
 *   local sound_id = LoadCustomSound("maiden_happy", "campgns/mymod/sounds/happy.wav")
 *   if sound_id > 0 then
 *       print("Sound loaded successfully!")
 *   end
 */
int lua_LoadCustomSound(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* filepath = luaL_checkstring(L, 2);
    
    SoundSmplTblID sample_id = sound_manager_load_custom_sound(name, filepath);
    
    lua_pushinteger(L, sample_id);
    return 1;
}

/**
 * @brief Lua: GetCustomSoundId(name)
 * Gets the sample ID of a previously loaded custom sound
 * 
 * @param L Lua state
 * @return Number of return values (1: sample_id)
 * 
 * Example:
 *   local sound_id = GetCustomSoundId("maiden_happy")
 *   if sound_id > 0 then
 *       PlaySound(sound_id, 3, 256)
 *   end
 */
int lua_GetCustomSoundId(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    
    SoundSmplTblID sample_id = sound_manager_get_custom_sound_id(name);
    
    lua_pushinteger(L, sample_id);
    return 1;
}

/**
 * @brief Lua: SetCreatureSound(creature_model, sound_type, custom_sound_name)
 * Overrides a creature's sound with a custom sound
 * 
 * @param L Lua state
 * @return Number of return values (1: success boolean)
 * 
 * Example:
 *   SetCreatureSound("MAIDEN", "Happy", "maiden_happy_custom")
 */
int lua_SetCreatureSound(lua_State* L) {
    const char* creature_model = luaL_checkstring(L, 1);
    const char* sound_type = luaL_checkstring(L, 2);
    const char* custom_sound_name = luaL_checkstring(L, 3);
    
    TbBool success = sound_manager_set_creature_sound(creature_model, sound_type, custom_sound_name);
    
    lua_pushboolean(L, success);
    return 1;
}

/**
 * @brief Lua: IsCustomSoundLoaded(name)
 * Checks if a custom sound has been loaded
 * 
 * @param L Lua state
 * @return Number of return values (1: loaded boolean)
 * 
 * Example:
 *   if IsCustomSoundLoaded("maiden_happy") then
 *       print("Sound is ready to use")
 *   end
 */
int lua_IsCustomSoundLoaded(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    
    TbBool loaded = sound_manager_is_custom_sound_loaded(name);
    
    lua_pushboolean(L, loaded);
    return 1;
}

/**
 * @brief Lua: PlaySound(sample_id, priority, volume)
 * Plays a sound effect
 * 
 * @param L Lua state
 * @return Number of return values (1: emitter_id)
 * 
 * Example:
 *   local emitter_id = PlaySound(100, 3, 256)  -- Play sample 100 at normal priority, full volume
 */
int lua_PlaySound(lua_State* L) {
    SoundSmplTblID sample_id = luaL_checkinteger(L, 1);
    long priority = luaL_optinteger(L, 2, 3);  // Default priority: 3
    SoundVolume volume = luaL_optinteger(L, 3, FULL_LOUDNESS);  // Default volume: 256
    
    SoundEmitterID emitter_id = sound_manager_play_effect(sample_id, priority, volume);
    
    lua_pushinteger(L, emitter_id);
    return 1;
}

/**
 * @brief Lua: StopSound(emitter_id)
 * Stops a playing sound
 * 
 * @param L Lua state
 * @return Number of return values (0)
 * 
 * Example:
 *   local emitter = PlaySound(100, 3, 256)
 *   StopSound(emitter)
 */
int lua_StopSound(lua_State* L) {
    SoundEmitterID emitter_id = luaL_checkinteger(L, 1);
    
    sound_manager_stop_effect(emitter_id);
    
    return 0;
}

/**
 * @brief Lua: PlayMusic(track_or_file)
 * Plays music by track number or file path
 * 
 * @param L Lua state
 * @return Number of return values (1: success boolean)
 * 
 * Example:
 *   PlayMusic(5)  -- Play track 5
 *   PlayMusic("campgns/mymod/music/theme.ogg")  -- Play custom music
 */
int lua_PlayMusic(lua_State* L) {
    TbBool success = false;
    
    if (lua_isnumber(L, 1)) {
        // Track number
        int track = luaL_checkinteger(L, 1);
        success = sound_manager_play_music(track);
    } else {
        // File path
        const char* filepath = luaL_checkstring(L, 1);
        // TODO: Implement play_music_file() wrapper
        // success = sound_manager_play_music_file(filepath);
        success = false;
    }
    
    lua_pushboolean(L, success);
    return 1;
}

/**
 * @brief Lua: StopMusic()
 * Stops music playback
 * 
 * @param L Lua state
 * @return Number of return values (0)
 * 
 * Example:
 *   StopMusic()
 */
int lua_StopMusic(lua_State* L) {
    sound_manager_stop_music();
    return 0;
}

/**
 * @brief Register sound API functions with Lua
 * Called during Lua initialization to expose sound functions
 */
void register_lua_sound_api(lua_State* L) {
    // Custom sound loading
    lua_register(L, "LoadCustomSound", lua_LoadCustomSound);
    lua_register(L, "GetCustomSoundId", lua_GetCustomSoundId);
    lua_register(L, "IsCustomSoundLoaded", lua_IsCustomSoundLoaded);
    
    // Creature sound overrides
    lua_register(L, "SetCreatureSound", lua_SetCreatureSound);
    
    // Sound playback
    lua_register(L, "PlaySound", lua_PlaySound);
    lua_register(L, "StopSound", lua_StopSound);
    
    // Music
    lua_register(L, "PlayMusic", lua_PlayMusic);
    lua_register(L, "StopMusic", lua_StopMusic);
    
    // Sound constants
    lua_pushinteger(L, FULL_LOUDNESS);
    lua_setglobal(L, "SOUND_VOLUME_FULL");
    
    lua_pushinteger(L, FULL_LOUDNESS / 2);
    lua_setglobal(L, "SOUND_VOLUME_HALF");
    
    lua_pushinteger(L, FULL_LOUDNESS / 4);
    lua_setglobal(L, "SOUND_VOLUME_QUIET");
    
    lua_pushinteger(L, 3);
    lua_setglobal(L, "SOUND_PRIORITY_NORMAL");
    
    lua_pushinteger(L, 6);
    lua_setglobal(L, "SOUND_PRIORITY_HIGH");
    
    lua_pushinteger(L, 1);
    lua_setglobal(L, "SOUND_PRIORITY_LOW");
}

#ifdef __cplusplus
}
#endif
/******************************************************************************/
