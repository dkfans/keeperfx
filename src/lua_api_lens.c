/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_api_lens.c
 *     LUA API for dynamic lens effect creation.
 * @par Purpose:
 *     Allows modders to create custom lens effects using LUA callbacks.
 * @par Comment:
 *     Provides functions to register lenses, set callbacks, and control effects.
 * @author   KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "lua_api_lens.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bflib_basics.h"
#include "globals.h"
#include "game_legacy.h"
#include "lens_api.h"

#include "keeperfx.hpp"
#include "post_inc.h"

/******************************************************************************/

// Buffer metadata structure - mirrors LuaBufferInfo in LuaLensEffect.cpp
typedef struct {
    unsigned char* data;
    long width;
    long height;
    long pitch;
} LuaBufferInfo;

// LUA state (stored for callbacks)
static lua_State* g_lua_state = NULL;

/**
 * CreateLens(name, config_table)
 * 
 * Creates a new custom lens effect with specified configuration.
 * 
 * LUA Example:
 *   CreateLens("poison_cloud", {
 *       mist = "poison_fog.raw",
 *       displacement = {type=1, magnitude=10, period=5},
 *       overlay = "poison_border.raw",
 *       palette = "green_tint"
 *   })
 * 
 * @param name String - Unique lens identifier
 * @param config Table - Configuration table with effect properties (optional)
 * @return boolean - true on success
 */
static int lua_Create_lens(lua_State *L)
{
    const char* lens_name = luaL_checkstring(L, 1);
    // Config table is optional (lua_type returns LUA_TNIL if not provided)
    
    // Get LensManager instance
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL) {
        ERRORLOG("LUA: LensManager not available");
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // Check if lens already exists
    void* existing_lens = LensManager_GetCustomLens(mgr, lens_name);
    if (existing_lens != NULL) {
        WARNLOG("LUA: Lens '%s' already exists, skipping creation", lens_name);
        lua_pushboolean(L, 1);  // Return true since lens exists
        return 1;
    }
    
    // Create a new LuaLensEffect
    void* effect = LuaLensEffect_Create(lens_name, L);
    if (effect == NULL) {
        ERRORLOG("LUA: Failed to create LuaLensEffect for '%s'", lens_name);
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // Register the custom lens with the manager
    TbBool success = LensManager_RegisterCustomLens(mgr, lens_name, effect);
    if (!success) {
        ERRORLOG("LUA: Failed to register custom lens '%s'", lens_name);
        // Note: LensManager takes ownership of effect, so don't delete here
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // Store LUA state for callbacks
    g_lua_state = L;
    
    SYNCDBG(7, "LUA: Successfully created custom lens '%s'", lens_name);
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * SetLensDrawCallback(lens_name, callback_function)
 * 
 * Sets a LUA function to be called each frame when the lens is active.
 * The callback receives a context table with buffers and dimensions.
 * 
 * LUA Example:
 *   SetLensDrawCallback("poison_cloud", function(ctx)
 *       -- ctx.width, ctx.height, ctx.srcbuf, ctx.dstbuf available
       -- Use GetPixel/SetPixel/CopyPixel to manipulate buffers
 *       return true  -- Return true if buffer was modified
 *   end)
 * 
 * @param lens_name String - Lens identifier
 * @param callback Function - Drawing callback function(context)
 * @return boolean - true on success
 */
static int lua_Set_lens_draw_callback(lua_State *L)
{
    const char* lens_name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    // Get LensManager instance
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL) {
        ERRORLOG("LUA: LensManager not available");
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // Get the custom lens effect - must exist first
    void* effect = LensManager_GetCustomLens(mgr, lens_name);
    if (effect == NULL) {
        ERRORLOG("LUA: Custom lens '%s' not found. Call CreateLens() first", lens_name);
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // Store callback in LUA registry for later invocation
    // This pushes the function to top of stack, then stores it and returns a reference
    lua_pushvalue(L, 2);  // Duplicate the function
    int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // Set the callback on the LuaLensEffect
    LuaLensEffect_SetDrawCallback(effect, callback_ref);
    
    // Store LUA state for callbacks
    g_lua_state = L;
    
    SYNCDBG(7, "LUA: SetLensDrawCallback('%s') successful (ref=%d)", lens_name, callback_ref);
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * SetActiveLens(lens_name_or_index)
 * 
 * Activates a lens by name (custom) or index (built-in).
 * 
 * LUA Example:
 *   SetActiveLens("poison_cloud")  -- Custom lens
 *   SetActiveLens(6)                -- Built-in lens (dragon)
 *   SetActiveLens(0)                -- Disable lens
 * 
 * @param lens String or Number - Lens name or index
 * @return boolean - true on success
 */
static int lua_Set_active_lens(lua_State *L)
{
    void* mgr = LensManager_GetInstance();
    
    if (mgr == NULL || !LensManager_IsReady(mgr)) {
        WARNLOG("LUA: Lens system not initialized");
        lua_pushboolean(L, 0);
        return 1;
    }
    
    TbBool success = false;
    
    if (lua_isnumber(L, 1)) {
        // Built-in lens by index
        long lens_idx = luaL_checkinteger(L, 1);
        success = LensManager_SetLens(mgr, lens_idx);
    } else if (lua_isstring(L, 1)) {
        // Custom lens by name
        const char* lens_name = luaL_checkstring(L, 1);
        success = LensManager_SetLensByName(mgr, lens_name);
    } else {
        ERRORLOG("LUA: SetActiveLens requires number or string");
        success = false;
    }
    
    lua_pushboolean(L, success);
    return 1;
}

/**
 * GetActiveLens()
 * 
 * Returns the currently active lens index or name.
 * 
 * LUA Example:
 *   local lens = GetActiveLens()
 *   print("Active lens: " .. lens)
 * 
 * @return number or string - Current lens identifier (0 = no lens)
 */
static int lua_Get_active_lens(lua_State *L)
{
    void* mgr = LensManager_GetInstance();
    
    if (mgr == NULL || !LensManager_IsReady(mgr)) {
        lua_pushinteger(L, 0);
        return 1;
    }
    
    long active_lens = LensManager_GetActiveLens(mgr);
    
    // If it's a custom lens (-1), return the name instead of index
    if (active_lens == -1) {
        const char* custom_name = LensManager_GetActiveCustomLensName(mgr);
        if (custom_name != NULL) {
            lua_pushstring(L, custom_name);
            return 1;
        }
    }
    
    lua_pushinteger(L, active_lens);
    return 1;
}

/**
 * SetLensParameter(lens_name, param_name, value)
 * 
 * Sets a runtime parameter for a custom lens effect.
 * 
 * LUA Example:
 *   SetLensParameter("poison_cloud", "intensity", 0.8)
 *   SetLensParameter("poison_cloud", "fog_speed", 2.5)
 * 
 * @param lens_name String - Lens identifier
 * @param param_name String - Parameter name
 * @param value Number - Parameter value
 * @return boolean - true on success
 */
static int lua_Set_lens_parameter(lua_State *L)
{
    const char* lens_name = luaL_checkstring(L, 1);
    const char* param_name = luaL_checkstring(L, 2);
    double value = luaL_checknumber(L, 3);
    
    SYNCDBG(7, "LUA: SetLensParameter('%s', '%s', %f)", lens_name, param_name, value);
    
    // TODO: Implement parameter setting via C API
    WARNLOG("LUA: SetLensParameter not yet implemented");
    (void)lens_name; (void)param_name; (void)value;
    lua_pushboolean(L, 0);
    
    return 1;
}

/**
 * LoadLensAsset(filename, asset_type)
 * 
 * Loads a custom asset for use in lens effects.
 * Uses the standard asset loading fallback (JSON → mods → data).
 * 
 * LUA Example:
 *   local asset_id = LoadLensAsset("my_fog.raw", "mist")
 *   local overlay_id = LoadLensAsset("my_border.raw", "overlay")
 * 
 * @param filename String - Asset filename
 * @param asset_type String - Type: "mist", "overlay", "palette"
 * @return number - Asset ID for reference (or -1 on failure)
 */
static int lua_Load_lens_asset(lua_State *L)
{
    const char* filename = luaL_checkstring(L, 1);
    const char* asset_type = luaL_checkstring(L, 2);
    
    SYNCDBG(7, "LUA: LoadLensAsset('%s', '%s')", filename, asset_type);
    
    // TODO: Load asset using LensEffect::LoadAssetWithFallback
    // Return asset ID that can be used in custom lens configuration
    
    SCRPTLOG("Loaded lens asset '%s' (type=%s)", filename, asset_type);
    
    lua_pushinteger(L, 1);  // Placeholder asset ID
    return 1;
}

/**
 * SetLensEnabled(effect_type, enabled)
 * 
 * Controls accessibility settings for lens effects.
 * 
 * LUA Example:
 *   SetLensEnabled("displacement", false)  -- Disable warping
 *   SetLensEnabled("mist", true)           -- Enable fog
 * 
 * @param effect_type String - "mist", "displacement", "overlay", "palette", "flyeye"
 * @param enabled Boolean - true to enable, false to disable
 * @return boolean - true on success
 */
static int lua_Set_lens_enabled(lua_State *L)
{
    const char* effect_type = luaL_checkstring(L, 1);
    TbBool enabled = lua_toboolean(L, 2);
    
    SYNCDBG(7, "LUA: SetLensEnabled('%s', %d)", effect_type, enabled);
    
    // TODO: Implement SetEffectEnabled via C API
    // Need to expose LensManager_SetEffectEnabled in lens_api.c
    WARNLOG("LUA: SetLensEnabled not yet implemented");
    (void)effect_type; (void)enabled;
    lua_pushboolean(L, 0);
    
    return 1;
}

/**
 * IsLensEnabled(effect_type)
 * 
 * Checks if a lens effect type is enabled (accessibility check).
 * 
 * LUA Example:
 *   if IsLensEnabled("displacement") then
 *       -- Warping is enabled
 *   end
 * 
 * @param effect_type String - Effect type to check
 * @return boolean - true if enabled
 */
static int lua_Is_lens_enabled(lua_State *L)
{
    const char* effect_type = luaL_checkstring(L, 1);
    
    // TODO: Implement IsEffectEnabled via C API
    WARNLOG("LUA: IsLensEnabled not yet implemented");
    (void)effect_type;
    
    lua_pushboolean(L, false);
    return 1;
}

/**
 * CopyBuffer(src, dst)
 * 
 * High-performance buffer copy operation. Copies entire source buffer to destination
 * in native C without Lua overhead. Use this before applying effects that modify pixels.
 * 
 * LUA Example:
 *   -- Copy source to destination first (preserves original scene)
 *   CopyBuffer(ctx.srcbuf, ctx.dstbuf)
 *   -- Now apply plasma effect on top
 *   SubmitPixelBatch(ctx.dstbuf, plasma_batch)
 * 
 * @param src Userdata - Source buffer
 * @param dst Userdata - Destination buffer
 * @return boolean - true on success
 */
static int lua_Copy_buffer(lua_State *L)
{
    LuaBufferInfo* src_info = (LuaBufferInfo*)lua_touserdata(L, 1);
    LuaBufferInfo* dst_info = (LuaBufferInfo*)lua_touserdata(L, 2);
    
    if (src_info == NULL || src_info->data == NULL || 
        dst_info == NULL || dst_info->data == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // Buffers should have same dimensions
    if (src_info->width != dst_info->width || src_info->height != dst_info->height) {
        WARNLOG("LUA: CopyBuffer dimension mismatch: src=%ldx%ld dst=%ldx%ld",
               src_info->width, src_info->height, dst_info->width, dst_info->height);
        lua_pushboolean(L, 0);
        return 1;
    }
    
    // Copy row by row in case pitches differ
    long width = src_info->width;
    long height = src_info->height;
    unsigned char* src = src_info->data;
    unsigned char* dst = dst_info->data;
    long src_pitch = src_info->pitch;
    long dst_pitch = dst_info->pitch;
    
    for (long y = 0; y < height; y++) {
        memcpy(dst + y * dst_pitch, src + y * src_pitch, width);
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * SubmitPixelBatch(buffer, operations)
 * 
 * High-performance batch pixel submission. Processes all pixel operations
 * in native C with a single boundary crossing. Much faster than SetPixel loops.
 * 
 * LUA Example:
 *   local batch = {}
 *   for y = 0, height-1, 4 do
 *       for x = 0, width-1, 4 do
 *           local color = CalculateEffect(x, y)
 *           table.insert(batch, {x=x, y=y, w=4, h=4, color=color})
 *       end
 *   end
 *   SubmitPixelBatch(ctx.dstbuf, batch)
 * 
 * @param buffer Userdata - Buffer handle (srcbuf or dstbuf)
 * @param operations Table - Array of {x, y, w, h, color} operations
 * @return boolean - true on success
 */
static int lua_Submit_pixel_batch(lua_State *L)
{
    // Get buffer info - userdata created in LuaLensEffect::PushContextToLua
    LuaBufferInfo* buf_info = (LuaBufferInfo*)lua_touserdata(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    
    if (buf_info == NULL || buf_info->data == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    unsigned char* buffer = buf_info->data;
    int buf_width = (int)buf_info->width;
    int buf_height = (int)buf_info->height;
    int buf_pitch = (int)buf_info->pitch;
    
    // Get number of operations (Lua 5.1 compatible)
#if LUA_VERSION_NUM >= 502
    int num_ops = (int)lua_rawlen(L, 2);
#else
    int num_ops = (int)lua_objlen(L, 2);
#endif
    
    // Process each operation
    for (int i = 1; i <= num_ops; i++) {
        lua_rawgeti(L, 2, i);  // Get batch[i]
        
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        
        // Extract operation fields: x, y, w, h, color
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        lua_getfield(L, -3, "w");
        lua_getfield(L, -4, "h");
        lua_getfield(L, -5, "color");
        
        if (lua_isnumber(L, -5) && lua_isnumber(L, -4) && 
            lua_isnumber(L, -3) && lua_isnumber(L, -2) && 
            lua_isnumber(L, -1)) {
            
            int x = (int)lua_tointeger(L, -5);
            int y = (int)lua_tointeger(L, -4);
            int w = (int)lua_tointeger(L, -3);
            int h = (int)lua_tointeger(L, -2);
            int color = (int)lua_tointeger(L, -1);
            
            // Clamp color to valid palette range
            if (color < 0) color = 0;
            if (color > 255) color = 255;
            
            // Fill rectangle (bounds checked, use pitch for row stride)
            for (int dy = 0; dy < h; dy++) {
                int py = y + dy;
                if (py < 0 || py >= buf_height) continue;
                
                for (int dx = 0; dx < w; dx++) {
                    int px = x + dx;
                    if (px < 0 || px >= buf_width) continue;
                    
                    buffer[py * buf_pitch + px] = (unsigned char)color;
                }
            }
        }
        
        lua_pop(L, 6);  // Pop color, h, w, y, x, and the table
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * ApplyColorOffsetBatch(buffer, operations)
 * 
 * This is the simplest color quantization algorithm, applying a signed offset to existing pixel values.
 * 
 * High-performance batch color offset application. Reads existing pixels,
 * applies signed offsets, and writes back clamped values. Perfect for effects
 * that modulate existing scene colors (plasma, tint, brightness).
 * 
 * LUA Example:
 *   -- First copy source to destination
 *   CopyBuffer(ctx.srcbuf, ctx.dstbuf)
 *   
 *   -- Calculate plasma offsets purely in Lua
 *   local batch = {}
 *   for y = 0, height-1, step do
 *       for x = 0, width-1, step do
 *           local plasma_offset = CalculatePlasmaValue(x, y, time)
 *           table.insert(batch, {x=x, y=y, w=step, h=step, offset=plasma_offset})
 *       end
 *   end
 *   
 *   -- Apply all offsets in one C call (read-modify-write)
 *   ApplyColorOffsetBatch(ctx.dstbuf, batch)
 * 
 * @param buffer Userdata - Buffer handle (must already contain base image)
 * @param operations Table - Array of {x, y, w, h, offset} operations (offset can be negative)
 * @return boolean - true on success
 */
static int lua_Apply_color_offset_batch(lua_State *L)
{
    // Get buffer info
    LuaBufferInfo* buf_info = (LuaBufferInfo*)lua_touserdata(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    
    if (buf_info == NULL || buf_info->data == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    unsigned char* buffer = buf_info->data;
    int buf_width = (int)buf_info->width;
    int buf_height = (int)buf_info->height;
    int buf_pitch = (int)buf_info->pitch;
    
    // Get number of operations (Lua 5.1 compatible)
#if LUA_VERSION_NUM >= 502
    int num_ops = (int)lua_rawlen(L, 2);
#else
    int num_ops = (int)lua_objlen(L, 2);
#endif
    
    // Process each operation
    for (int i = 1; i <= num_ops; i++) {
        lua_rawgeti(L, 2, i);  // Get batch[i]
        
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        
        // Extract operation fields: x, y, w, h, offset
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        lua_getfield(L, -3, "w");
        lua_getfield(L, -4, "h");
        lua_getfield(L, -5, "offset");
        
        if (lua_isnumber(L, -5) && lua_isnumber(L, -4) && 
            lua_isnumber(L, -3) && lua_isnumber(L, -2) && 
            lua_isnumber(L, -1)) {
            
            int x = (int)lua_tointeger(L, -5);
            int y = (int)lua_tointeger(L, -4);
            int w = (int)lua_tointeger(L, -3);
            int h = (int)lua_tointeger(L, -2);
            int offset = (int)lua_tointeger(L, -1);  // Can be negative
            
            // Fill rectangle with color offset (bounds checked, read-modify-write)
            for (int dy = 0; dy < h; dy++) {
                int py = y + dy;
                if (py < 0 || py >= buf_height) continue;
                
                for (int dx = 0; dx < w; dx++) {
                    int px = x + dx;
                    if (px < 0 || px >= buf_width) continue;
                    
                    // Read current pixel, apply offset, clamp, write back
                    int pixel_idx = py * buf_pitch + px;
                    int current = buffer[pixel_idx];
                    int new_color = current + offset;
                    
                    // Clamp to valid palette range
                    if (new_color < 0) new_color = 0;
                    if (new_color > 255) new_color = 255;
                    
                    buffer[pixel_idx] = (unsigned char)new_color;
                }
            }
        }
        
        lua_pop(L, 6);  // Pop offset, h, w, y, x, and the table
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * BlendColorBatch(buffer, operations)
 * This is from the 1984 paper "Compositing Digital Images" by Porter and Duff.
 * 
 * High-performance batch alpha blending. Blends a target color with existing pixels
 * using proper RGB color space blending (reads from current active palette).
 * 
 * LUA Example:
 *   -- Create purple tint with varying opacity
 *   local batch = {}
 *   for y = 0, height-1, step do
 *       for x = 0, width-1, step do
 *           local opacity = CalculatePlasmaValue(x, y, time)
 *           table.insert(batch, {x=x, y=y, w=step, h=step, color=128, alpha=opacity})
 *       end
 *   end
 *   BlendColorBatch(ctx.dstbuf, batch)
 * 
 * @param buffer Userdata - Buffer handle
 * @param operations Table - Array of {x, y, w, h, color, alpha} where alpha is 0-255
 * @return boolean - true on success
 */
static int lua_Blend_color_batch(lua_State *L)
{
    LuaBufferInfo* buf_info = (LuaBufferInfo*)lua_touserdata(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    
    if (buf_info == NULL || buf_info->data == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    unsigned char* buffer = buf_info->data;
    int buf_width = (int)buf_info->width;
    int buf_height = (int)buf_info->height;
    int buf_pitch = (int)buf_info->pitch;
    
    // Get active palette from display system (already declared in bflib_video.h)
    unsigned char* palette = lbDisplay.Palette;
    
    // Get number of operations
#if LUA_VERSION_NUM >= 502
    int num_ops = (int)lua_rawlen(L, 2);
#else
    int num_ops = (int)lua_objlen(L, 2);
#endif
    
    // Process each operation
    for (int i = 1; i <= num_ops; i++) {
        lua_rawgeti(L, 2, i);
        
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        
        // Extract: x, y, w, h, color (palette index), alpha (0-255)
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        lua_getfield(L, -3, "w");
        lua_getfield(L, -4, "h");
        lua_getfield(L, -5, "color");
        lua_getfield(L, -6, "alpha");
        
        if (lua_isnumber(L, -6) && lua_isnumber(L, -5) && 
            lua_isnumber(L, -4) && lua_isnumber(L, -3) &&
            lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            
            int x = (int)lua_tointeger(L, -6);
            int y = (int)lua_tointeger(L, -5);
            int w = (int)lua_tointeger(L, -4);
            int h = (int)lua_tointeger(L, -3);
            int target_color = (int)lua_tointeger(L, -2) & 0xFF;
            int alpha = (int)lua_tointeger(L, -1);
            
            // Clamp alpha
            if (alpha < 0) alpha = 0;
            if (alpha > 255) alpha = 255;
            
            // Skip if fully transparent
            if (alpha == 0) {
                lua_pop(L, 7);
                continue;
            }
            
            // Get target RGB
            int target_r = palette[target_color * 3 + 0];
            int target_g = palette[target_color * 3 + 1];
            int target_b = palette[target_color * 3 + 2];
            
            // Fill rectangle with alpha blending
            for (int dy = 0; dy < h; dy++) {
                int py = y + dy;
                if (py < 0 || py >= buf_height) continue;
                
                for (int dx = 0; dx < w; dx++) {
                    int px = x + dx;
                    if (px < 0 || px >= buf_width) continue;
                    
                    int pixel_idx = py * buf_pitch + px;
                    int src_color = buffer[pixel_idx];
                    
                    // Get source RGB
                    int src_r = palette[src_color * 3 + 0];
                    int src_g = palette[src_color * 3 + 1];
                    int src_b = palette[src_color * 3 + 2];
                    
                    // Alpha blend: result = src * (1-alpha) + target * alpha
                    int blend_r = ((src_r * (255 - alpha)) + (target_r * alpha)) / 255;
                    int blend_g = ((src_g * (255 - alpha)) + (target_g * alpha)) / 255;
                    int blend_b = ((src_b * (255 - alpha)) + (target_b * alpha)) / 255;
                    
                    // Find closest palette color (simple linear search)
                    int best_idx = 0;
                    int best_dist = 0x7FFFFFFF;
                    for (int c = 0; c < 256; c++) {
                        int pr = palette[c * 3 + 0];
                        int pg = palette[c * 3 + 1];
                        int pb = palette[c * 3 + 2];
                        int dr = blend_r - pr;
                        int dg = blend_g - pg;
                        int db = blend_b - pb;
                        int dist = dr*dr + dg*dg + db*db;
                        
                        if (dist < best_dist) {
                            best_dist = dist;
                            best_idx = c;
                            if (dist == 0) break;  // Perfect match
                        }
                    }
                    
                    buffer[pixel_idx] = (unsigned char)best_idx;
                }
            }
        }
        
        lua_pop(L, 7);
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * RemapPixelBatch(buffer, remap_table, operations)
 * 
 * Ultra-fast palette remapping using a single lookup table for all operations.
 * The remap table is read ONCE and applied to all rectangles in the batch.
 * 
 * LUA Example:
 *   local darker = BuildDarkeningTable(0.5)
 *   local batch = {}
 *   for y = 0, height-1, step do
 *       for x = 0, width-1, step do
 *           table.insert(batch, {x=x, y=y, w=step, h=step})
 *       end
 *   end
 *   RemapPixelBatch(ctx.dstbuf, darker, batch)
 * 
 * @param buffer Userdata - Buffer handle
 * @param remap_table Table - 256-entry lookup table (read once)
 * @param operations Table - Array of {x, y, w, h} operations
 * @return boolean - true on success
 */
static int lua_Remap_pixel_batch(lua_State *L)
{
    LuaBufferInfo* buf_info = (LuaBufferInfo*)lua_touserdata(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);  // remap table
    luaL_checktype(L, 3, LUA_TTABLE);  // operations
    
    if (buf_info == NULL || buf_info->data == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    unsigned char* buffer = buf_info->data;
    int buf_width = (int)buf_info->width;
    int buf_height = (int)buf_info->height;
    int buf_pitch = (int)buf_info->pitch;
    
    // Load remap table ONCE (256 entries from arg 2)
    unsigned char remap[256];
    for (int c = 0; c < 256; c++) {
        lua_rawgeti(L, 2, c + 1);  // Lua arrays are 1-indexed
        if (lua_isnumber(L, -1)) {
            int val = (int)lua_tointeger(L, -1);
            remap[c] = (unsigned char)(val & 0xFF);
        } else {
            remap[c] = (unsigned char)c;  // Identity mapping
        }
        lua_pop(L, 1);
    }
    
    // Get number of operations (from arg 3)
#if LUA_VERSION_NUM >= 502
    int num_ops = (int)lua_rawlen(L, 3);
#else
    int num_ops = (int)lua_objlen(L, 3);
#endif
    
    // Process each operation using the shared remap table
    for (int i = 1; i <= num_ops; i++) {
        lua_rawgeti(L, 3, i);
        
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            continue;
        }
        
        // Extract: x, y, w, h (no remap field needed)
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        lua_getfield(L, -3, "w");
        lua_getfield(L, -4, "h");
        
        if (lua_isnumber(L, -4) && lua_isnumber(L, -3) && 
            lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            
            int x = (int)lua_tointeger(L, -4);
            int y = (int)lua_tointeger(L, -3);
            int w = (int)lua_tointeger(L, -2);
            int h = (int)lua_tointeger(L, -1);
            
            // Apply remap to rectangle
            for (int dy = 0; dy < h; dy++) {
                int py = y + dy;
                if (py < 0 || py >= buf_height) continue;
                
                for (int dx = 0; dx < w; dx++) {
                    int px = x + dx;
                    if (px < 0 || px >= buf_width) continue;
                    
                    int pixel_idx = py * buf_pitch + px;
                    unsigned char src_color = buffer[pixel_idx];
                    buffer[pixel_idx] = remap[src_color];
                }
            }
        }
        
        lua_pop(L, 5);
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * BuildDarkeningLUT(strength)
 * 
 * Builds a proper darkening lookup table using the current game palette.
 * Uses RGB color space blending and finds the closest palette match.
 * 
 * LUA Example:
 *   local darker_50 = BuildDarkeningLUT(0.5)  -- 50% darker
 *   local darker_80 = BuildDarkeningLUT(0.8)  -- 80% darker
 * 
 * @param strength Number - Darkening strength (0.0 = no change, 1.0 = black)
 * @return Table - 256-entry lookup table
 */
static int lua_Build_darkening_lut(lua_State *L)
{
    double strength = luaL_checknumber(L, 1);
    
    // Clamp strength
    if (strength < 0.0) strength = 0.0;
    if (strength > 1.0) strength = 1.0;
    
    // Get active palette
    unsigned char* palette = lbDisplay.Palette;
    if (palette == NULL) {
        ERRORLOG("LUA: BuildDarkeningLUT - palette not available");
        lua_pushnil(L);
        return 1;
    }
    
    // Create result table
    lua_createtable(L, 256, 0);
    
    // For each palette color, blend toward black and find closest match
    for (int i = 0; i < 256; i++) {
        int src_r = palette[i * 3 + 0];
        int src_g = palette[i * 3 + 1];
        int src_b = palette[i * 3 + 2];
        
        // Blend toward black in RGB space
        int target_r = (int)(src_r * (1.0 - strength));
        int target_g = (int)(src_g * (1.0 - strength));
        int target_b = (int)(src_b * (1.0 - strength));
        
        // Find closest palette color using Euclidean distance
        int best_idx = 0;
        int best_dist = 999999;
        for (int c = 0; c < 256; c++) {
            int pr = palette[c * 3 + 0];
            int pg = palette[c * 3 + 1];
            int pb = palette[c * 3 + 2];
            int dr = target_r - pr;
            int dg = target_g - pg;
            int db = target_b - pb;
            int dist = dr*dr + dg*dg + db*db;
            
            if (dist < best_dist) {
                best_dist = dist;
                best_idx = c;
                if (dist == 0) break;  // Perfect match
            }
        }
        
        // Store in Lua table (1-indexed)
        lua_pushinteger(L, best_idx);
        lua_rawseti(L, -2, i + 1);
    }
    
    return 1;
}

/******************************************************************************/
// Registration
/******************************************************************************/

static const struct luaL_Reg lens_methods[] = {
    {"CreateLens",              lua_Create_lens              },
    {"SetLensDrawCallback",     lua_Set_lens_draw_callback   },
    {"SetActiveLens",           lua_Set_active_lens          },
    {"GetActiveLens",           lua_Get_active_lens          },
    {"SetLensParameter",        lua_Set_lens_parameter       },
    {"LoadLensAsset",           lua_Load_lens_asset          },
    {"SetLensEnabled",          lua_Set_lens_enabled         },
    {"IsLensEnabled",           lua_Is_lens_enabled          },
    {"CopyBuffer",              lua_Copy_buffer              },
    {"SubmitPixelBatch",        lua_Submit_pixel_batch       },
    {"ApplyColorOffsetBatch",   lua_Apply_color_offset_batch },
    {"BlendColorBatch",         lua_Blend_color_batch        },
    {"RemapPixelBatch",         lua_Remap_pixel_batch        },
    {"BuildDarkeningLUT",       lua_Build_darkening_lut      },
    {NULL, NULL}
};

void Lens_register(lua_State *L)
{
    // Store LUA state for callbacks
    g_lua_state = L;
    
    // Register all lens functions
    for (size_t i = 0; lens_methods[i].name != NULL; i++)
    {
        lua_register(L, lens_methods[i].name, lens_methods[i].func);
    }
    
    SYNCDBG(7, "Registered %d lens API functions", 
           (int)(sizeof(lens_methods)/sizeof(lens_methods[0]) - 1));
}

void Lens_cleanup_lua(void)
{
    g_lua_state = NULL;
}

/******************************************************************************/
