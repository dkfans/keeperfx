/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file LensManager.cpp
 *     Lens effect manager implementation.
 * @par Purpose:
 *     Centralized manager for all lens effects.
 * @par Comment:
 *     Refactyled from original lens code to support multiple simultaneous effects and
 *     a clean API for effect implementations.
 * @author   Peter Lockett, KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "../../pre_inc.h"
#include "LensManager.h"

#include "MistEffect.h"
#include "DisplacementEffect.h"
#include "FlyeyeEffect.h"
#include "OverlayEffect.h"
#include "PaletteEffect.h"
#include "LuaLensEffect.h"

#include "../../globals.h"
#include "../../config_lenses.h"
#include "../../config_keeperfx.h"
#include "../../lens_api.h"
#include "../../vidmode.h"
#include "../../game_legacy.h"

#include "../../keeperfx.hpp"
#include "../../post_inc.h"

/******************************************************************************/

LensManager* LensManager::s_instance = nullptr;

LensManager* LensManager::GetInstance()
{
    if (s_instance == nullptr) {
        s_instance = new LensManager();
    }
    return s_instance;
}

LensManager::LensManager()
    : m_initialized(false)
    , m_active_lens(0)
    , m_applied_lens(0)
    , m_lens_memory(nullptr)
    , m_spare_screen_memory(nullptr)
    , m_buffer_width(0)
    , m_buffer_height(0)
{
    // Initialize accessibility config to all enabled by default
    m_config.enable_mist_effect = true;
    m_config.enable_displacement_effect = true;
    m_config.enable_overlay_effect = true;
    m_config.enable_palette_effect = true;
    m_config.enable_flyeye_effect = true;
}

LensManager::~LensManager()
{
    Reset();
}

TbBool LensManager::Init()
{
    SYNCDBG(7, "Initializing lens manager");
    
    if (m_initialized) {
        SYNCDBG(8, "Already initialized, resetting first");
        Reset();
    }
    
    // Check if lens feature is enabled via game flags
    if ((features_enabled & Ft_EyeLens) == 0) {
        SYNCDBG(7, "Eye lens feature disabled in game features");
        return false;
    }
    
    // Allocate shared buffers
    if (!AllocateBuffers()) {
        ERRORLOG("Failed to allocate lens buffers");
        return false;
    }
    
    // Register all effect implementations
    RegisterEffect(new MistEffect());
    RegisterEffect(new DisplacementEffect());
    RegisterEffect(new FlyeyeEffect());
    RegisterEffect(new OverlayEffect());
    RegisterEffect(new PaletteEffect());
    
    // Load accessibility configuration from keeper.cfg
    LoadAccessibilityConfig();
    
    m_initialized = true;
    set_flag(game.mode_flags, MFlg_EyeLensReady);
    
    SYNCDBG(7, "Lens manager initialized with %d effects", (int)m_effects.size());
    return true;
}

void LensManager::Reset()
{
    SYNCDBG(7, "Resetting lens manager");
    
    // Cleanup all effects (releases per-effect resources)
    CleanupAllEffects();
    
    // Free effect objects (they will be re-created by Init())
    FreeAllEffects();
    
    // Free buffers
    FreeBuffers();
    
    // Reset state
    m_active_lens = 0;
    m_applied_lens = 0;
    m_active_custom_lens.clear();
    m_initialized = false;
    clear_flag(game.mode_flags, MFlg_EyeLensReady);
    
    SYNCDBG(9, "Lens manager reset complete");
}

TbBool LensManager::SetLens(long lens_idx)
{
    if (!m_initialized) {
        WARNLOG("Cannot set lens - manager not initialized");
        return false;
    }
    
    SYNCDBG(7, "Setting lens to %ld", lens_idx);
    
    // lens_idx == 0 means "remove lens"
    if (lens_idx == 0) {
        CleanupAllEffects();
        m_active_lens = 0;
        m_applied_lens = 0;
        m_active_custom_lens.clear();  // Also clear custom lens
        game.active_lens_type = 0;
        game.applied_lens_type = 0;
        return true;
    }
    
    // Validate lens index
    if (lens_idx < 0 || lens_idx >= LENS_ITEMS_MAX) {
        ERRORLOG("Invalid lens index %ld", lens_idx);
        return false;
    }
    
    // Get lens configuration
    struct LensConfig* cfg = &lenses_conf.lenses[lens_idx];
    
    // Setup effects based on configuration and accessibility settings
    TbBool success = true;
    
    for (LensEffect* effect : m_effects) {
        if (!effect->IsEnabled()) {
            SYNCDBG(8, "Effect '%s' disabled in accessibility config", effect->GetName());
            continue;
        }
        
        // Check if this lens uses this effect type
        TbBool uses_effect = false;
        switch (effect->GetType()) {
            case LensEffectType::Mist:
                uses_effect = (cfg->flags & LCF_HasMist) != 0;
                break;
            case LensEffectType::Displacement:
                // Displacement used for algorithms 0-2, algorithm 3 is handled by Flyeye
                uses_effect = ((cfg->flags & LCF_HasDisplace) != 0 && cfg->displace_kind != 3);
                break;
            case LensEffectType::Overlay:
                uses_effect = (cfg->flags & LCF_HasOverlay) != 0;
                break;
            case LensEffectType::Palette:
                uses_effect = (cfg->flags & LCF_HasPalette) != 0;
                break;
            case LensEffectType::Flyeye:
                // Flyeye is used when displacement kind is 3 (compound eye)
                uses_effect = ((cfg->flags & LCF_HasDisplace) != 0 && cfg->displace_kind == 3);
                break;
            case LensEffectType::Custom:
                 // Lua/custom effects would need their own detection logic
                uses_effect = false;
                break;
        }
        
        if (uses_effect) {
            if (!effect->Setup(lens_idx)) {
                WARNLOG("Effect '%s' setup failed for lens %ld", effect->GetName(), lens_idx);
                success = false;
            }
        }
    }
    
    if (success) {
        m_active_lens = lens_idx;
        m_applied_lens = lens_idx;
        m_active_custom_lens.clear();  // Clear custom lens name when using standard lens
        game.active_lens_type = lens_idx;
        game.applied_lens_type = lens_idx;
    }
    
    return success;
}

void LensManager::Draw(unsigned char* srcbuf, unsigned char* dstbuf, 
                      long srcpitch, long dstpitch, 
                      long width, long height, long viewport_x)
{
    SYNCDBG(0, "LensManager::Draw() called: m_initialized=%d, m_applied_lens=%ld, m_active_custom_lens='%s'",
           m_initialized, m_applied_lens, m_active_custom_lens.c_str());
    
    // Setup render context
    LensRenderContext ctx;
    ctx.srcbuf = srcbuf;
    ctx.dstbuf = dstbuf;
    ctx.srcpitch = srcpitch;
    ctx.dstpitch = dstpitch;
    ctx.width = width;
    ctx.height = height;
    ctx.viewport_x = viewport_x;
    ctx.buffer_copied = false;
    
    // If not initialized or no lens active, just copy the buffer
    if (!m_initialized || (m_applied_lens == 0 && m_active_custom_lens.empty())) {
        unsigned char* viewport_src = srcbuf + viewport_x;
        CopyBuffer(dstbuf, dstpitch, viewport_src, srcpitch, width, height);
        return;
    }
    
    // Check if a custom lens is active
    if (!m_active_custom_lens.empty()) {
        LensEffect* custom_effect = GetCustomLens(m_active_custom_lens.c_str());
        if (custom_effect != nullptr && custom_effect->IsEnabled()) {
            if (custom_effect->Draw(&ctx)) {
                return;  // Custom lens rendered successfully
            }
        }
        // Custom lens failed, fall through to standard effects or fallback
    }
    
    // Apply standard effects in order
    TbBool rendered = false;
    for (LensEffect* effect : m_effects) {
        if (effect->IsEnabled()) {
            if (effect->Draw(&ctx)) {
                rendered = true;
            }
        }
    }
    
    // If no effects rendered (all failed or none applicable), copy as fallback
    if (!rendered) {
        unsigned char* viewport_src = srcbuf + viewport_x;
        CopyBuffer(dstbuf, dstpitch, viewport_src, srcpitch, width, height);
    }
}

void LensManager::LoadAccessibilityConfig()
{
    // TODO: Load from keeper.cfg
    // For now, use default values (all enabled)
    // This will be integrated with the config system
    SYNCDBG(7, "Loading accessibility config (using defaults for now)");
    
    // Update effect enable states based on config
    SetEffectEnabled(LensEffectType::Mist, m_config.enable_mist_effect);
    SetEffectEnabled(LensEffectType::Displacement, m_config.enable_displacement_effect);
    SetEffectEnabled(LensEffectType::Overlay, m_config.enable_overlay_effect);
    SetEffectEnabled(LensEffectType::Palette, m_config.enable_palette_effect);
    SetEffectEnabled(LensEffectType::Flyeye, m_config.enable_flyeye_effect);
}

void LensManager::SetEffectEnabled(LensEffectType type, TbBool enabled)
{
    LensEffect* effect = GetEffect(type);
    if (effect != nullptr) {
        effect->SetEnabled(enabled);
        SYNCDBG(8, "Effect '%s' %s", effect->GetName(), enabled ? "enabled" : "disabled");
    }
}

TbBool LensManager::IsEffectEnabled(LensEffectType type) const
{
    for (const LensEffect* effect : m_effects) {
        if (effect->GetType() == type) {
            return effect->IsEnabled();
        }
    }
    return false;
}

TbBool LensManager::RegisterCustomLens(const char* name, LensEffect* effect)
{
    if (name == nullptr || effect == nullptr) {
        WARNLOG("Cannot register custom lens: null name or effect");
        return false;
    }
    
    std::string lens_name(name);
    
    // Check if this name is already registered
    auto it = m_custom_lenses.find(lens_name);
    if (it != m_custom_lenses.end()) {
        WARNLOG("Custom lens '%s' is already registered", name);
        return false;
    }
    
    // Store the custom lens by name
    // IMPORTANT: Do NOT add to m_effects list - custom lenses are only invoked
    // when explicitly activated via SetLensByName(), not during standard lens processing
    m_custom_lenses[lens_name] = effect;
    
    SYNCDBG(7, "Registered custom lens '%s'", name);
    return true;
}

LensEffect* LensManager::GetCustomLens(const char* name)
{
    if (name == nullptr) {
        return nullptr;
    }
    
    std::string lens_name(name);
    auto it = m_custom_lenses.find(lens_name);
    
    if (it != m_custom_lenses.end()) {
        return it->second;
    }
    
    return nullptr;
}

TbBool LensManager::SetLensByName(const char* name)
{
    SYNCDBG(7, "SetLensByName called with name='%s'", name);
    
    LensEffect* effect = GetCustomLens(name);
    if (effect == nullptr) {
        WARNLOG("Custom lens '%s' not found", name);
        return false;
    }
    
    // Clean up current effects
    CleanupAllEffects();
    
    // Setup the custom lens (use -1 for custom lenses with no config index)
    TbBool result = effect->Setup(-1);
    if (!result) {
        ERRORLOG("Failed to setup custom lens '%s'", name);
        return false;
    }
    m_active_lens = -1;  // Custom lenses use -1 as index
    m_applied_lens = -1;
    m_active_custom_lens = name;  // Track which custom lens is active
    
    // CRITICAL: Update game state to indicate custom lens is active
    // Use 255 as sentinel value for custom lenses (vs 0=none, 1-5=standard)
    game.active_lens_type = 255;
    game.applied_lens_type = 255;
    
    SYNCDBG(0, "Activated custom lens '%s', set game.applied_lens_type=255", name);
    return true;
}

const char* LensManager::GetActiveCustomLensName() const
{
    if (m_active_custom_lens.empty()) {
        return nullptr;
    }
    return m_active_custom_lens.c_str();
}

void LensManager::RegisterEffect(LensEffect* effect)
{
    if (effect != nullptr) {
        m_effects.push_back(effect);
        SYNCDBG(9, "Registered effect '%s'", effect->GetName());
    }
}

LensEffect* LensManager::GetEffect(LensEffectType type)
{
    for (LensEffect* effect : m_effects) {
        if (effect->GetType() == type) {
            return effect;
        }
    }
    return nullptr;
}

void LensManager::CleanupAllEffects()
{
    // Only cleanup standard lens effects, not custom lenses
    // Custom lenses maintain persistent state (callbacks, config) across activations
    for (LensEffect* effect : m_effects) {
        // Check if this effect is a custom lens by seeing if it's in the map
        bool is_custom = false;
        for (const auto& pair : m_custom_lenses) {
            if (pair.second == effect) {
                is_custom = true;
                break;
            }
        }
        
        if (!is_custom) {
            effect->Cleanup();
        }
    }
}

void LensManager::FreeAllEffects()
{
    // Delete all standard effect objects
    for (LensEffect* effect : m_effects) {
        delete effect;
    }
    m_effects.clear();
    
    // Delete all custom lens objects
    for (const auto& pair : m_custom_lenses) {
        delete pair.second;
    }
    m_custom_lenses.clear();
}

TbBool LensManager::AllocateBuffers()
{
    m_buffer_width = lbDisplay.GraphicsScreenWidth;
    m_buffer_height = lbDisplay.GraphicsScreenHeight;
    
    unsigned long buffer_size = m_buffer_width * m_buffer_height + 2;
    
    // Ensure minimum size for 256x256 mist textures
    if (buffer_size < 256 * 256) {
        buffer_size = 256 * 256 + 2;
    }
    
    m_lens_memory = (uint32_t*)calloc(buffer_size, sizeof(uint32_t));
    m_spare_screen_memory = (unsigned char*)calloc(buffer_size, sizeof(unsigned char));
    
    if (m_lens_memory == nullptr || m_spare_screen_memory == nullptr) {
        ERRORLOG("Failed to allocate lens buffers (%lu bytes)", buffer_size * sizeof(uint32_t));
        FreeBuffers();
        return false;
    }
    
    // Update global pointers for C code compatibility
    eye_lens_memory = m_lens_memory;
    eye_lens_spare_screen_memory = m_spare_screen_memory;
    eye_lens_width = m_buffer_width;
    eye_lens_height = m_buffer_height;
    
    SYNCDBG(9, "Allocated lens buffers: %ldx%ld, size=%lu", m_buffer_width, m_buffer_height, buffer_size);
    return true;
}

void LensManager::FreeBuffers()
{
    if (m_lens_memory != nullptr) {
        free(m_lens_memory);
        m_lens_memory = nullptr;
    }
    
    if (m_spare_screen_memory != nullptr) {
        free(m_spare_screen_memory);
        m_spare_screen_memory = nullptr;
    }
    
    // Clear global pointers for C code compatibility
    eye_lens_memory = nullptr;
    eye_lens_spare_screen_memory = nullptr;
    eye_lens_width = 0;
    eye_lens_height = 0;
    
    m_buffer_width = 0;
    m_buffer_height = 0;
}

/******************************************************************************/
// C API WRAPPERS (for C code compatibility)
/******************************************************************************/

extern "C" {

void* LensManager_GetInstance(void)
{
    return LensManager::GetInstance();
}

TbBool LensManager_Init(void* mgr)
{
    if (mgr == nullptr) return false;
    return static_cast<LensManager*>(mgr)->Init();
}

void LensManager_Reset(void* mgr)
{
    if (mgr != nullptr) {
        static_cast<LensManager*>(mgr)->Reset();
    }
}

TbBool LensManager_SetLens(void* mgr, long lens_idx)
{
    if (mgr == nullptr) return false;
    return static_cast<LensManager*>(mgr)->SetLens(lens_idx);
}

long LensManager_GetActiveLens(void* mgr)
{
    if (mgr == nullptr) return 0;
    return static_cast<LensManager*>(mgr)->GetActiveLens();
}
const char* LensManager_GetActiveCustomLensName(void* mgr)
{
    if (mgr == NULL) return NULL;
    return static_cast<LensManager*>(mgr)->GetActiveCustomLensName();
}
TbBool LensManager_IsReady(void* mgr)
{
    if (mgr == nullptr) return false;
    return static_cast<LensManager*>(mgr)->IsReady();
}

void LensManager_Draw(void* mgr, unsigned char* srcbuf, unsigned char* dstbuf,
                      long srcpitch, long dstpitch, long width, long height, long viewport_x)
{
    if (mgr != nullptr) {
        static_cast<LensManager*>(mgr)->Draw(srcbuf, dstbuf, srcpitch, dstpitch,
                                             width, height, viewport_x);
    }
}

void LensManager_CopyBuffer(unsigned char* dstbuf, long dstpitch,
                           unsigned char* srcbuf, long srcpitch,
                           long width, long height)
{
    LensManager::CopyBuffer(dstbuf, dstpitch, srcbuf, srcpitch, width, height);
}

TbBool LensManager_RegisterCustomLens(void* mgr, const char* name, void* effect)
{
    if (mgr == nullptr || name == nullptr || effect == nullptr) return false;
    return static_cast<LensManager*>(mgr)->RegisterCustomLens(name, static_cast<LensEffect*>(effect));
}

void* LensManager_GetCustomLens(void* mgr, const char* name)
{
    if (mgr == nullptr || name == nullptr) return nullptr;
    return static_cast<LensManager*>(mgr)->GetCustomLens(name);
}

TbBool LensManager_SetLensByName(void* mgr, const char* name)
{
    if (mgr == nullptr || name == nullptr) return false;
    return static_cast<LensManager*>(mgr)->SetLensByName(name);
}

void* LuaLensEffect_Create(const char* name, void* lua_state)
{
    if (name == nullptr || lua_state == nullptr) return nullptr;
    return new LuaLensEffect(name, static_cast<lua_State*>(lua_state));
}

void LuaLensEffect_SetDrawCallback(void* effect, int callback_ref)
{
    if (effect == nullptr) {
        ERRORLOG("C WRAPPER: effect is NULL!");
        return;
    }
    static_cast<LuaLensEffect*>(effect)->SetDrawCallback(callback_ref);
}

} // extern "C"

/******************************************************************************/
// HELPER FUNCTIONS
/******************************************************************************/

void LensManager::CopyBuffer(unsigned char* dstbuf, long dstpitch,
                            unsigned char* srcbuf, long srcpitch,
                            long width, long height)
{
    unsigned char* dst = dstbuf;
    unsigned char* src = srcbuf;
    for (long i = 0; i < height; i++)
    {
        memcpy(dst, src, width * sizeof(TbPixel));
        dst += dstpitch;
        src += srcpitch;
    }
}

/******************************************************************************/

