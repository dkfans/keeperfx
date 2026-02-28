/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_api.c
 *     Eye lenses C API wrapper.
 * @par Purpose:
 *     C API wrapper around the C++ LensManager system.
 * @par Comment:
 *     All lens effect implementations are now in kfx/lenses/.
 *     This file only provides C function wrappers for existing C code.
 * @author   Tomasz Lis, KeeperFX Team
 * @date     11 Mar 2010 - 09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "lens_api.h"

#include "globals.h"
#include "bflib_basics.h"
#include "vidmode.h"
#include "game_legacy.h"
#include "bflib_vidraw.h"
#include "config_keeperfx.h"

#include "keeperfx.hpp"

#include "post_inc.h"

/******************************************************************************/
// C API WRAPPERS
/******************************************************************************/

/**
 * Initialize the lens system.
 * Sets up the LensManager and allocates required resources.
 */
void initialise_eye_lenses(void)
{
    SYNCDBG(7, "Starting");
    
    // Check if lens feature is enabled
    if ((features_enabled & Ft_EyeLens) == 0)
    {
        SYNCDBG(7, "Eye lens feature disabled");
        clear_flag(game.mode_flags, MFlg_EyeLensReady);
        return;
    }
    
    // Get LensManager instance and initialize
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL)
    {
        ERRORLOG("Failed to get LensManager instance");
        clear_flag(game.mode_flags, MFlg_EyeLensReady);
        return;
    }
    
    if (!LensManager_Init(mgr))
    {
        ERRORLOG("Failed to initialize LensManager");
        clear_flag(game.mode_flags, MFlg_EyeLensReady);
        return;
    }
    
    // Note: eye_lens_width, eye_lens_height, eye_lens_memory, and eye_lens_spare_screen_memory
    // are set automatically by LensManager::AllocateBuffers()
    
    // Mark system as ready
    set_flag(game.mode_flags, MFlg_EyeLensReady);
    
    SYNCDBG(9, "Lens system initialized");
}

/**
 * Setup a specific lens effect.
 * 
 * @param nlens Lens index (0 = no lens, >0 = specific lens type)
 */
void setup_eye_lens(long nlens)
{
    // Check if lens system is initialized
    if ((game.mode_flags & MFlg_EyeLensReady) == 0)
    {
        WARNLOG("Can't setup lens - system not initialized");
        return;
    }
    
    SYNCDBG(7, "Setting up lens %ld", nlens);
    
    // Get LensManager and set the lens
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL)
    {
        ERRORLOG("Failed to get LensManager instance");
        return;
    }
    
    if (!LensManager_SetLens(mgr, nlens))
    {
        WARNLOG("Failed to set lens %ld", nlens);
        return;
    }
    
    // Note: LensManager updates game.active_lens_type and game.applied_lens_type internally
    SYNCDBG(8, "Lens %ld setup complete", nlens);
}

/**
 * Reinitialize with a specific lens.
 * Clears and reinitializes the entire system with the given lens active.
 * 
 * @param nlens Lens index to activate after reinit
 */
void reinitialise_eye_lens(long nlens)
{
    SYNCDBG(7, "Reinitializing with lens %ld", nlens);
    
    // Reset and reinitialize
    reset_eye_lenses();
    initialise_eye_lenses();
    
    // Apply the requested lens
    if ((game.mode_flags & MFlg_EyeLensReady) != 0)
    {
        setup_eye_lens(nlens);
    }
}

/**
 * Reset the lens system.
 * Clears all lens effects and frees resources.
 */
void reset_eye_lenses(void)
{
    SYNCDBG(7, "Starting");
    
    // Get LensManager and reset
    void* mgr = LensManager_GetInstance();
    if (mgr != NULL)
    {
        LensManager_Reset(mgr);
    }
    
    // Note: LensManager owns eye_lens_memory and eye_lens_spare_screen_memory
    // They are cleared automatically by LensManager::FreeBuffers()
    
    // Clear game state (LensManager already cleared active_lens_type/applied_lens_type)
    clear_flag(game.mode_flags, MFlg_EyeLensReady);
    
    SYNCDBG(9, "Done");
}

/**
 * Draw the active lens effect.
 * 
 * @param dstbuf Destination buffer (viewport area on screen)
 * @param dstpitch Destination pitch
 * @param srcbuf Source buffer (full screen width, unclipped)
 * @param srcpitch Source pitch (full screen width)
 * @param width Viewport width
 * @param height Viewport height
 * @param viewport_x X offset of viewport in source buffer
 * @param effect Lens effect index (0 = no effect)
 */
void draw_lens_effect(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, 
                     long width, long height, long viewport_x, long effect)
{
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL) {
        ERRORLOG("LensManager not available");
        return;
    }
    
    // If effect doesn't match active lens, set it now (Manager updates game state internally)
    long active_lens = LensManager_GetActiveLens(mgr);
    
    // Special handling for custom lenses (effect=255): don't override them with standard lenses
    // Custom lenses return active_lens=-1, but effect will be 255 if custom lens is active
    if (effect == 255) {
        // Custom lens should be active - don't change it
        SYNCDBG(9, "Custom lens active (effect=255), skipping SetLens");
    } else if (active_lens == -1) {
        // Custom lens is active but caller wants standard lens - switch to it
        SYNCDBG(8, "Switching from custom lens to standard lens %ld", effect);
        if (!LensManager_SetLens(mgr, effect)) {
            WARNLOG("Failed to set lens %ld during draw", effect);
        }
    } else if (active_lens != effect) {
        // Standard lens mismatch - update to requested lens
        SYNCDBG(8, "Switching lens from %ld to %ld", active_lens, effect);
        if (!LensManager_SetLens(mgr, effect)) {
            WARNLOG("Failed to set lens %ld during draw", effect);
        }
    }
    
    // Draw via LensManager (handles all fallbacks internally)
    LensManager_Draw(mgr, srcbuf, dstbuf, srcpitch, dstpitch, width, height, viewport_x);
}

/**
 * Check if the lens system is ready for use.
 * This is the proper way to check lens readiness from C code.
 */
TbBool lens_is_ready(void)
{
    // Check feature flag
    if ((features_enabled & Ft_EyeLens) == 0)
        return false;
    
    // Check mode flag
    if ((game.mode_flags & MFlg_EyeLensReady) == 0)
        return false;
    
    // Check manager is initialized
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL)
        return false;
    
    if (!LensManager_IsReady(mgr))
        return false;
    
    // Check if a lens is actually applied
    if (game.applied_lens_type == 0)
        return false;
    
    return true;
}

/******************************************************************************/
// BUFFER ACCESSORS (for external code needing render target)
/******************************************************************************/

/**
 * Get the lens render target buffer.
 * This is the off-screen buffer where the 3D view should be rendered
 * before lens effects are applied.
 * 
 * @return Pointer to render target buffer, or NULL if lens system not ready
 */
unsigned char* lens_get_render_target(void)
{
    if ((game.mode_flags & MFlg_EyeLensReady) == 0)
        return NULL;
    
    return eye_lens_spare_screen_memory;
}

/**
 * Get the render target width.
 * 
 * @return Width in pixels, or 0 if lens system not ready
 */
unsigned int lens_get_render_target_width(void)
{
    if ((game.mode_flags & MFlg_EyeLensReady) == 0)
        return 0;
    
    return eye_lens_width;
}

/**
 * Get the render target height.
 * 
 * @return Height in pixels, or 0 if lens system not ready
 */
unsigned int lens_get_render_target_height(void)
{
    if ((game.mode_flags & MFlg_EyeLensReady) == 0)
        return 0;
    
    return eye_lens_height;
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif
