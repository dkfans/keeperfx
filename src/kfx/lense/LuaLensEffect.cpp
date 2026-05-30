/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file LuaLensEffect.cpp
 *     LUA-scriptable lens effect implementation.
 * @par Purpose:
 *     Effect that calls LUA callback functions for custom rendering.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "../../pre_inc.h"
#include "LuaLensEffect.h"

extern "C" {
    #include <lauxlib.h>
}

#include "../../keeperfx.hpp"
#include "../../post_inc.h"

/******************************************************************************/
// LUA BUFFER ACCESS API
/******************************************************************************/

/**
 * Buffer metadata structure stored as userdata in LUA.
 */
struct LuaBufferInfo {
    unsigned char* data;
    long width;
    long height;
    long pitch;
};

/**
 * LUA API: GetPixel(buffer, x, y) -> color
 * Safely reads a pixel from a buffer with bounds checking.
 */
int LuaLensEffect::LuaGetPixel(lua_State* L)
{
    // Get buffer info
    LuaBufferInfo* buf = (LuaBufferInfo*)lua_touserdata(L, 1);
    if (buf == NULL || buf->data == NULL) {
        return luaL_argerror(L, 1, "Invalid buffer");
    }
    
    // Get coordinates
    long x = (long)luaL_checkinteger(L, 2);
    long y = (long)luaL_checkinteger(L, 3);
    
    // Bounds check
    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        lua_pushinteger(L, 0);  // Return black for out-of-bounds
        return 1;
    }
    
    // Read pixel
    unsigned char pixel = buf->data[y * buf->pitch + x];
    lua_pushinteger(L, pixel);
    return 1;
}

/**
 * LUA API: SetPixel(buffer, x, y, color)
 * Safely writes a pixel to a buffer with bounds checking.
 */
int LuaLensEffect::LuaSetPixel(lua_State* L)
{
    // Get buffer info
    LuaBufferInfo* buf = (LuaBufferInfo*)lua_touserdata(L, 1);
    if (buf == NULL || buf->data == NULL) {
        return luaL_argerror(L, 1, "Invalid buffer");
    }
    
    // Get coordinates and color
    long x = (long)luaL_checkinteger(L, 2);
    long y = (long)luaL_checkinteger(L, 3);
    unsigned char color = (unsigned char)luaL_checkinteger(L, 4);
    
    // Bounds check
    if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        return 0;  // Silently ignore out-of-bounds writes
    }
    
    // Write pixel
    buf->data[y * buf->pitch + x] = color;
    return 0;
}

/**
 * LUA API: CopyPixel(src, dst, x, y)
 * Safely copies a pixel from source to destination buffer.
 */
int LuaLensEffect::LuaCopyPixel(lua_State* L)
{
    // Get buffer info
    LuaBufferInfo* src = (LuaBufferInfo*)lua_touserdata(L, 1);
    LuaBufferInfo* dst = (LuaBufferInfo*)lua_touserdata(L, 2);
    
    if (src == NULL || src->data == NULL || dst == NULL || dst->data == NULL) {
        return luaL_argerror(L, 1, "Invalid buffer");
    }
    
    // Get coordinates
    long x = (long)luaL_checkinteger(L, 3);
    long y = (long)luaL_checkinteger(L, 4);
    
    // Bounds check both buffers
    if (x < 0 || x >= src->width || y < 0 || y >= src->height ||
        x >= dst->width || y >= dst->height) {
        return 0;
    }
    
    // Copy pixel
    dst->data[y * dst->pitch + x] = src->data[y * src->pitch + x];
    return 0;
}

/**
 * Register buffer manipulation functions in LUA global namespace.
 */
void LuaLensEffect::RegisterBufferFunctions(lua_State* L)
{
    lua_register(L, "GetPixel", LuaGetPixel);
    lua_register(L, "SetPixel", LuaSetPixel);
    lua_register(L, "CopyPixel", LuaCopyPixel);
}

/******************************************************************************/
// LUALENSEFFECT IMPLEMENTATION
/******************************************************************************/

LuaLensEffect::LuaLensEffect(const char* lens_name, lua_State* L)
    : LensEffect(LensEffectType::Custom, lens_name)
    , m_lens_name(lens_name)
    , m_lua_state(L)
    , m_draw_callback_ref(LUA_NOREF)
    , m_current_lens(-1)
{
    // Register buffer access functions in LUA global namespace
    if (m_lua_state != NULL) {
        RegisterBufferFunctions(m_lua_state);
    }
    
    SYNCDBG(8, "Created LUA lens effect '%s'", lens_name);
}

LuaLensEffect::~LuaLensEffect()
{
    Cleanup();
}

TbBool LuaLensEffect::Setup(long lens_idx)
{
    SYNCDBG(8, "Setting up LUA lens effect '%s' for lens %ld", m_lens_name.c_str(), lens_idx);
    
    // Load assets specified in configuration
    if (!m_config.mist_file.empty()) {
        // TODO: Load mist asset
        SYNCDBG(7, "LUA lens '%s' uses mist: %s", m_lens_name.c_str(), m_config.mist_file.c_str());
    }
    
    if (!m_config.overlay_file.empty()) {
        // TODO: Load overlay asset
        SYNCDBG(7, "LUA lens '%s' uses overlay: %s", m_lens_name.c_str(), m_config.overlay_file.c_str());
    }
    
    if (m_config.displacement_type > 0) {
        // TODO: Setup displacement map
        SYNCDBG(7, "LUA lens '%s' uses displacement: type=%d mag=%d period=%d", 
               m_lens_name.c_str(), m_config.displacement_type, 
               m_config.displacement_magnitude, m_config.displacement_period);
    }
    
    m_current_lens = lens_idx;
    SYNCDBG(7, "LUA lens effect '%s' ready", m_lens_name.c_str());
    return true;
}

void LuaLensEffect::Cleanup()
{
    // Clean up callback reference (works for both config and custom lenses)
    if (m_draw_callback_ref != LUA_NOREF && m_lua_state != NULL) {
        luaL_unref(m_lua_state, LUA_REGISTRYINDEX, m_draw_callback_ref);
        m_draw_callback_ref = LUA_NOREF;
    }
    
    m_current_lens = 0;  // Reset to no lens
    SYNCDBG(9, "LUA lens effect '%s' cleaned up", m_lens_name.c_str());
}

TbBool LuaLensEffect::Draw(LensRenderContext* ctx)
{
    // Check if lens is active (0 = no lens, anything else = active)
    if (m_current_lens == 0) {
        return false;
    }
    
    if (m_draw_callback_ref == LUA_NOREF || m_lua_state == NULL) {
        SYNCDBG(8, "LUA lens '%s' has no draw callback", m_lens_name.c_str());
        return false;
    }
    
    SYNCDBG(7, "Invoking LUA lens effect '%s'", m_lens_name.c_str());
    
    // Invoke LUA callback
    return InvokeLuaCallback(ctx);
}

void LuaLensEffect::SetDrawCallback(int lua_ref)
{
    // Release old reference if any
    if (m_draw_callback_ref != LUA_NOREF && m_lua_state != NULL) {
        luaL_unref(m_lua_state, LUA_REGISTRYINDEX, m_draw_callback_ref);
    }
    
    m_draw_callback_ref = lua_ref;
    SYNCDBG(7, "LUA lens '%s' draw callback set (ref=%d)", m_lens_name.c_str(), lua_ref);
}

void LuaLensEffect::SetConfig(const LuaLensConfig& config)
{
    m_config = config;
    SYNCDBG(8, "LUA lens '%s' configuration updated", m_lens_name.c_str());
}

void LuaLensEffect::SetParameter(const std::string& name, double value)
{
    m_config.custom_params[name] = value;
    SYNCDBG(9, "LUA lens '%s' parameter '%s' = %f", m_lens_name.c_str(), name.c_str(), value);
}

double LuaLensEffect::GetParameter(const std::string& name) const
{
    auto it = m_config.custom_params.find(name);
    if (it != m_config.custom_params.end()) {
        return it->second;
    }
    return 0.0;
}

TbBool LuaLensEffect::InvokeLuaCallback(LensRenderContext* ctx)
{
    if (m_lua_state == NULL) {
        ERRORLOG("LUA state is NULL");
        return false;
    }
    
    // Get callback function from registry
    lua_rawgeti(m_lua_state, LUA_REGISTRYINDEX, m_draw_callback_ref);
    
    if (!lua_isfunction(m_lua_state, -1)) {
        ERRORLOG("LUA lens '%s' callback is not a function", m_lens_name.c_str());
        lua_pop(m_lua_state, 1);
        return false;
    }
    
    // Push context table as argument
    PushContextToLua(ctx);
    
    // Call callback(context)
    if (lua_pcall(m_lua_state, 1, 1, 0) != 0) {
        const char* error = lua_tostring(m_lua_state, -1);
        ERRORLOG("LUA lens '%s' callback error: %s", m_lens_name.c_str(), error);
        lua_pop(m_lua_state, 1);
        return false;
    }
    
    // Get return value (true/false indicating if buffer was modified)
    TbBool modified = lua_toboolean(m_lua_state, -1);
    lua_pop(m_lua_state, 1);
    
    if (modified) {
        ctx->buffer_copied = true;
    }
    
    return modified;
}

void LuaLensEffect::PushContextToLua(LensRenderContext* ctx)
{
    // Create context table
    lua_newtable(m_lua_state);
    
    // ctx.width
    lua_pushstring(m_lua_state, "width");
    lua_pushinteger(m_lua_state, ctx->width);
    lua_settable(m_lua_state, -3);
    
    // ctx.height
    lua_pushstring(m_lua_state, "height");
    lua_pushinteger(m_lua_state, ctx->height);
    lua_settable(m_lua_state, -3);
    
    // ctx.viewport_x
    lua_pushstring(m_lua_state, "viewport_x");
    lua_pushinteger(m_lua_state, ctx->viewport_x);
    lua_settable(m_lua_state, -3);
    
    // ctx.srcbuf - as light userdata with metadata
    lua_pushstring(m_lua_state, "srcbuf");
    LuaBufferInfo* src_info = (LuaBufferInfo*)lua_newuserdata(m_lua_state, sizeof(LuaBufferInfo));
    src_info->data = ctx->srcbuf + ctx->viewport_x;  // Viewport-aligned
    src_info->width = ctx->width;
    src_info->height = ctx->height;
    src_info->pitch = ctx->srcpitch;
    lua_settable(m_lua_state, -3);
    
    // ctx.dstbuf - as light userdata with metadata
    lua_pushstring(m_lua_state, "dstbuf");
    LuaBufferInfo* dst_info = (LuaBufferInfo*)lua_newuserdata(m_lua_state, sizeof(LuaBufferInfo));
    dst_info->data = ctx->dstbuf;
    dst_info->width = ctx->width;
    dst_info->height = ctx->height;
    dst_info->pitch = ctx->dstpitch;
    lua_settable(m_lua_state, -3);
    
    // ctx.lens_name

    lua_pushstring(m_lua_state, "lens_name");
    lua_pushstring(m_lua_state, m_lens_name.c_str());
    lua_settable(m_lua_state, -3);
    
    // Add custom parameters to context
    lua_pushstring(m_lua_state, "params");
    lua_newtable(m_lua_state);
    for (const auto& pair : m_config.custom_params) {
        lua_pushstring(m_lua_state, pair.first.c_str());
        lua_pushnumber(m_lua_state, pair.second);
        lua_settable(m_lua_state, -3);
    }
    lua_settable(m_lua_state, -3);
}

/******************************************************************************/
