/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file LensManager.h
 *     Lens effect manager class.
 * @par Purpose:
 *     Centralized manager for all lens effects with clean API.
 * @par Comment:
 *     Manages effect lifecycle, configuration, and rendering.
 * @author   Peter Lockett, KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KFX_LENSMANAGER_H
#define KFX_LENSMANAGER_H

#include "../../bflib_basics.h"
#include "LensEffect.h"
#include <vector>
#include <map>
#include <string>

/*****************************************************************************/

/**
 * Configuration for effect accessibility toggles.
 * These can be set via keeper.cfg to disable effects that may cause
 * motion sickness or other accessibility issues.
 */
struct LensAccessibilityConfig {
    TbBool enable_mist_effect;          // Fog/haze overlays
    TbBool enable_displacement_effect;   // Image warping/distortion
    TbBool enable_overlay_effect;        // Graphic overlays
    TbBool enable_palette_effect;        // Color palette changes
    TbBool enable_flyeye_effect;         // Fisheye distortion
};

/**
 * Lens Manager - centralized management for all lens effects.
 * 
 * Responsibilities:
 * - Effect lifecycle management (init, setup, cleanup)
 * - State management (current lens, buffers, configuration)
 * - Rendering coordination (effect pipeline)
 * - Accessibility configuration integration
 * 
 * Usage:
 *   LensManager* mgr = LensManager::GetInstance();
 *   mgr->Init();
 *   mgr->SetLens(lens_idx);
 *   mgr->Draw(srcbuf, dstbuf, ...);
 *   mgr->Reset();
 */
class LensManager {
public:
    // Singleton access
    static LensManager* GetInstance();
    
    // Core lifecycle
    TbBool Init();
    void Reset();
    
    // Lens control
    TbBool SetLens(long lens_idx);
    long GetActiveLens() const { return m_active_lens; }
    long GetAppliedLens() const { return m_applied_lens; }
    
    // Rendering (always succeeds - handles fallback internally)
    void Draw(unsigned char* srcbuf, unsigned char* dstbuf, 
             long srcpitch, long dstpitch, 
             long width, long height, long viewport_x);
    
    // Configuration
    void LoadAccessibilityConfig();
    void SetEffectEnabled(LensEffectType type, TbBool enabled);
    TbBool IsEffectEnabled(LensEffectType type) const;
    
    // Custom lens support (for LUA-created lenses)
    TbBool RegisterCustomLens(const char* name, LensEffect* effect);
    LensEffect* GetCustomLens(const char* name);
    TbBool SetLensByName(const char* name);
    const char* GetActiveCustomLensName() const;
    
    // State query
    TbBool IsReady() const { return m_initialized; }
    
    // Helper: Copy buffer with pitch
    static void CopyBuffer(unsigned char *dst, long dstpitch,
                          unsigned char *src, long srcpitch,
                          long width, long height);
    
private:
    LensManager();
    ~LensManager();
    
    // Prevent copying
    LensManager(const LensManager&) = delete;
    LensManager& operator=(const LensManager&) = delete;
    
    // Internal management
    void RegisterEffect(LensEffect* effect);
    LensEffect* GetEffect(LensEffectType type);
    void CleanupAllEffects();
    void FreeAllEffects();
    TbBool AllocateBuffers();
    void FreeBuffers();
    
    // Singleton instance
    static LensManager* s_instance;
    
    // State
    TbBool m_initialized;
    long m_active_lens;   // Requested lens
    long m_applied_lens;  // Currently applied lens
    
    // Effects registry
    std::vector<LensEffect*> m_effects;
    
    // Custom lenses (created dynamically via LUA)
    std::map<std::string, LensEffect*> m_custom_lenses;
    std::string m_active_custom_lens;  // Name of currently active custom lens
    
    // Buffers (managed internally)
    uint32_t* m_lens_memory;
    unsigned char* m_spare_screen_memory;
    long m_buffer_width;
    long m_buffer_height;
    
    // Configuration
    LensAccessibilityConfig m_config;
};

/******************************************************************************/
#endif
