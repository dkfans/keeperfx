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
#include "config_lenses.h"
#include "player_data.h"

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
        return;
    }
    
    // Get LensManager instance and initialize
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL)
    {
        ERRORLOG("Failed to get LensManager instance");
        return;
    }
    
    if (!LensManager_Init(mgr))
    {
        ERRORLOG("Failed to initialize LensManager");
        return;
    }
    
    // Note: eye_lens_width, eye_lens_height, eye_lens_memory, and eye_lens_spare_screen_memory
    // are set automatically by LensManager::AllocateBuffers()
    
    SYNCDBG(9, "Lens system initialized");
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
    reset_lens_failures();
    
    // Note: LensManager owns eye_lens_memory and eye_lens_spare_screen_memory
    // They are cleared automatically by LensManager::FreeBuffers()
    
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
 */
void draw_lens_effect(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, 
                     long width, long height, long viewport_x)
{
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL) {
        ERRORLOG("LensManager not available");
        return;
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
    
    // Check manager is initialized
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL)
        return false;
    
    if (!LensManager_IsReady(mgr))
        return false;
    
    // Check if a lens is actually applied
    if (LensManager_GetActiveLens(mgr) == 0)
        return false;
    
    return true;
}

static void set_user_lens(struct UserState *ustate, int lens, const char *custom_lens)
{
    ustate->lens_type = lens;
    snprintf(ustate->custom_lens, sizeof(ustate->custom_lens), "%s", (custom_lens != NULL) ? custom_lens : "");
}

void set_player_lens(PlayerNumber plyr_idx, int lens)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        struct UserState *ustate = get_user_state(user);
        if (ustate->player_id == plyr_idx)
            set_user_lens(ustate, lens, NULL);
    }
}

void set_all_users_lens(int lens, const char *custom_lens)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
        set_user_lens(get_user_state(user), lens, custom_lens);
}

static short failed_lens = -1;
static char failed_custom_lens[COMMAND_WORD_LEN] = "";
static TbBool lens_init_failed = false;

void reset_lens_failures(void)
{
    failed_lens = -1;
    failed_custom_lens[0] = '\0';
    lens_init_failed = false;
}

// user state -> local display
void update_local_lens(void)
{
    const struct UserState *ustate = get_user_state(get_displayed_user());
    if (user_state_invalid(ustate) || ((features_enabled & Ft_EyeLens) == 0))
        return;
    void* mgr = LensManager_GetInstance();
    if (mgr == NULL)
        return;
    const TbBool custom = ustate->custom_lens[0] != '\0';
    if (!LensManager_IsReady(mgr))
    {
        if (lens_init_failed || (!custom && ustate->lens_type == 0))
            return;
        initialise_eye_lenses();
        if (!LensManager_IsReady(mgr))
        {
            lens_init_failed = true;
            return;
        }
    }
    const char *active_custom = LensManager_GetActiveCustomLensName(mgr);
    if (custom)
    {
        if ((active_custom != NULL) && (strcmp(active_custom, ustate->custom_lens) == 0))
            return;
        if (strcmp(failed_custom_lens, ustate->custom_lens) == 0)
            return;
        if (LensManager_SetLensByName(mgr, ustate->custom_lens))
            failed_custom_lens[0] = '\0';
        else
            snprintf(failed_custom_lens, sizeof(failed_custom_lens), "%s", ustate->custom_lens);
        return;
    }
    failed_custom_lens[0] = '\0';
    if ((active_custom == NULL) && (LensManager_GetActiveLens(mgr) == ustate->lens_type))
        return;
    if (ustate->lens_type == failed_lens)
        return;
    if (LensManager_SetLens(mgr, ustate->lens_type))
        failed_lens = -1;
    else
        failed_lens = ustate->lens_type;
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
    if (!LensManager_IsReady(LensManager_GetInstance()))
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
    if (!LensManager_IsReady(LensManager_GetInstance()))
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
    if (!LensManager_IsReady(LensManager_GetInstance()))
        return 0;
    
    return eye_lens_height;
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif
