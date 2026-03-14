/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file LuaLensEffect.h
 *     LUA-scriptable lens effect.
 * @par Purpose:
 *     Effect implementation that calls LUA callback functions.
 * @par Comment:
 *     Allows custom lens effects to be created entirely in LUA scripts.
 * @author   Peter Lockett, KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KFX_LUALENSEFFECT_H
#define KFX_LUALENSEFFECT_H

#include "LensEffect.h"
#include <string>
#include <map>

extern "C" {
    #include <lua.h>
}

/******************************************************************************/

/**
 * Custom lens configuration (set from LUA).
 */
struct LuaLensConfig {
    std::string mist_file;
    std::string overlay_file;
    std::string palette_name;
    
    int displacement_type;
    int displacement_magnitude;
    int displacement_period;
    
    std::map<std::string, double> custom_params;  // User-defined parameters
};

/**
 * LUA-scriptable lens effect.
 * 
 * This effect bridges the C++ lens system with LUA scripts, allowing
 * modders to create fully custom lens effects using LUA callbacks.
 * 
 * Features:
 * - Custom draw callbacks in LUA
 * - Asset loading from LUA
 * - Dynamic parameters
 * - Full access to render context
 */
class LuaLensEffect : public LensEffect {
public:
    LuaLensEffect(const char* lens_name, lua_State* L);
    virtual ~LuaLensEffect();
    
    virtual TbBool Setup(long lens_idx) override;
    virtual void Cleanup() override;
    virtual TbBool Draw(LensRenderContext* ctx) override;
    
    // LUA integration
    void SetDrawCallback(int lua_ref);
    void SetConfig(const LuaLensConfig& config);
    void SetParameter(const std::string& name, double value);
    double GetParameter(const std::string& name) const;
    
    const std::string& GetLensName() const { return m_lens_name; }
    
private:
    // LUA callback invocation
    TbBool InvokeLuaCallback(LensRenderContext* ctx);
    void PushContextToLua(LensRenderContext* ctx);
    
    // Safe buffer access helpers for LUA
    static int LuaGetPixel(lua_State* L);      // GetPixel(buffer, x, y)
    static int LuaSetPixel(lua_State* L);      // SetPixel(buffer, x, y, color)
    static int LuaCopyPixel(lua_State* L);     // CopyPixel(src, dst, x, y)
    static void RegisterBufferFunctions(lua_State* L);
    
    std::string m_lens_name;
    lua_State* m_lua_state;
    int m_draw_callback_ref;  // LUA registry reference to callback
    
    LuaLensConfig m_config;
    long m_current_lens;
};

/******************************************************************************/
#endif
