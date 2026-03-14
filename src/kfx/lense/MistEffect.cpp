/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file MistEffect.cpp
 *     Mist/fog lens effect implementation.
 * @par Purpose:
 *     Self-contained fog overlay effect with animation.
 * @par Comment:
 *     Mist is an effect of bitmap image moving over original 3D view.
 *     When moving, mist can change directions, but can never go upward.
 *     Mist looks like there was a layer of dirt just behind the eye.
 * @author   Tomasz Lis, KeeperFX Team
 * @date     05 Jan 2009 - 09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "../../pre_inc.h"
#include "MistEffect.h"

#include <string.h>
#include "../../config_lenses.h"
#include "../../lens_api.h"
#include "../../custom_sprites.h"
#include "../../globals.h"
#include "../../vidmode.h"

#include "../../keeperfx.hpp"
#include "../../post_inc.h"

/******************************************************************************/
// INTERNAL MIST RENDERER CLASS
/******************************************************************************/

/**
 * CMistFade - Internal mist animation and rendering class.
 * Handles the animated layered fog effect with scrolling mist textures.
 */
class CMistFade {
public:
    CMistFade();
    ~CMistFade();
    
    void Setup(unsigned char *lens_mem, unsigned char *fade, unsigned char *ghost,
               unsigned char pos_x_step, unsigned char pos_y_step,
               unsigned char sec_x_step, unsigned char sec_y_step);
    void SetAnimation(long counter, long speed);
    void Render(unsigned char *dstbuf, long dstpitch, 
               unsigned char *srcbuf, long srcpitch,
               long width, long height);
    void Animate();
    
private:
    /** Mist data width and height are the same and equal to this dimension */
    unsigned int lens_dim;
    unsigned char *lens_data;
    unsigned char *fade_data;
    unsigned char *ghost_data;
    unsigned char position_offset_x;
    unsigned char position_offset_y;
    unsigned char secondary_offset_x;
    unsigned char secondary_offset_y;
    long animation_counter;
    long animation_speed;
    unsigned char position_x_step;
    unsigned char position_y_step;
    unsigned char secondary_x_step;
    unsigned char secondary_y_step;
};

CMistFade::CMistFade()
{
    Setup(NULL, NULL, NULL, 2, 1, 253, 3);
}

CMistFade::~CMistFade()
{
}

void CMistFade::Setup(unsigned char *lens_mem, unsigned char *fade, unsigned char *ghost,
                     unsigned char pos_x_step, unsigned char pos_y_step,
                     unsigned char sec_x_step, unsigned char sec_y_step)
{
    this->lens_data = lens_mem;
    this->fade_data = fade;
    this->ghost_data = ghost;
    this->lens_dim = 256;
    this->position_offset_x = 0;
    this->position_offset_y = 0;
    this->secondary_offset_x = 50;
    this->secondary_offset_y = 128;
    this->animation_speed = 1024;
    this->animation_counter = 0;
    this->position_x_step = pos_x_step;
    this->position_y_step = pos_y_step;
    this->secondary_x_step = sec_x_step;
    this->secondary_y_step = sec_y_step;
}

void CMistFade::SetAnimation(long a1, long a2)
{
    this->animation_counter = a1;
    this->animation_speed = a2;
}

void CMistFade::Animate()
{
    this->position_offset_x += this->position_x_step;
    this->position_offset_y += this->position_y_step;
    this->secondary_offset_x -= this->secondary_x_step;
    this->animation_counter += this->animation_speed;
    this->secondary_offset_y += this->secondary_y_step;
}

void CMistFade::Render(unsigned char *dstbuf, long dstpitch,
                      unsigned char *srcbuf, long srcpitch,
                      long width, long height)
{
    if ((lens_data == NULL) || (fade_data == NULL))
    {
        ERRORLOG("Can't draw Mist as it's not initialized!");
        return;
    }
    
    // Reference dimensions for resolution-independent scaling
    // The mist pattern will appear identical to 640x480 at any resolution
    static const int REF_WIDTH = 640;
    static const int REF_HEIGHT = 480;
    
    // Fixed-point scale factors (16.16 format)
    // Maps screen coordinates to virtual 640x480 space
    const unsigned int scale_x = (REF_WIDTH << 16) / width;
    const unsigned int scale_y = (REF_HEIGHT << 16) / height;
    
    // Animation offsets (copied to local for performance)
    const int pos_x = this->position_offset_x;
    const int pos_y = this->position_offset_y;
    const int sec_x = this->secondary_offset_x;
    const int sec_y = this->secondary_offset_y;
    
    unsigned char *src = srcbuf;
    unsigned char *dst = dstbuf;
    
    for (long y = 0; y < height; y++)
    {
        // Virtual Y coordinate in 640x480 space
        int virtual_y = (y * scale_y) >> 16;
        
        // Pre-calculate row-constant texture coordinates
        int c2_base = (pos_y + virtual_y) & 0xFF;
        int p1_base = (sec_x + 0x10000 - virtual_y) & 0xFF;
        
        for (long x = 0; x < width; x++)
        {
            // Virtual X coordinate in 640x480 space
            int virtual_x = (x * scale_x) >> 16;
            
            // Primary layer texture coords: (row=c2, col=p2)
            // p2 increments with x, c2 increments with y
            int p2 = (pos_x + virtual_x) & 0xFF;
            int c2 = c2_base;
            
            // Secondary layer texture coords: (row=c1, col=p1)
            // c1 decrements with x, p1 decrements with y
            int c1 = (sec_y + 0x10000 - virtual_x) & 0xFF;
            int p1 = p1_base;
            
            // Sample both layers from 256x256 texture
            long k = lens_data[(c2 << 8) + p2];  // primary
            long i = lens_data[(c1 << 8) + p1];  // secondary
            
            // Combine layers and clamp
            long n = (k + i) >> 3;
            if (n > 32) n = 32;
            else if (n < 0) n = 0;
            
            // Apply fade table and write result
            *dst = this->fade_data[(n << 8) + *src];
            src++;
            dst++;
        }
        
        // Move to next row
        dst += (dstpitch - width);
        src += (srcpitch - width);
    }
}

/******************************************************************************/
// MISTEFFECT PUBLIC INTERFACE
/******************************************************************************/

MistEffect::MistEffect()
    : LensEffect(LensEffectType::Mist, "Mist")
    , m_current_lens(-1)
{
}

MistEffect::~MistEffect()
{
    Cleanup();
}

TbBool MistEffect::Setup(long lens_idx)
{
    SYNCDBG(8, "Setting up mist effect for lens %ld", lens_idx);
    
    struct LensConfig* cfg = &lenses_conf.lenses[lens_idx];
    
    // Check if this lens has a mist effect configured
    if ((cfg->flags & LCF_HasMist) == 0)
    {
        SYNCDBG(8, "Lens %ld does not have mist effect configured", lens_idx);
        return true;  // Not an error - effect just not configured
    }
    
    // Load mist texture using base class fallback loader
    if (!LoadMistTexture(cfg->mist_file))
    {
        WARNLOG("Failed to load mist texture '%s' for lens %ld - effect will be skipped", 
                cfg->mist_file, lens_idx);
        return true;  // Continue without mist effect (graceful degradation)
    }
    
    // Setup the mist renderer
    CMistFade* renderer = new CMistFade();
    renderer->Setup((unsigned char*)eye_lens_memory,
                   &pixmap.fade_tables[(cfg->mist_lightness) * 256],
                   &pixmap.ghost[(cfg->mist_ghost) * 256],
                   (unsigned char)cfg->mist_pos_x_step,
                   (unsigned char)cfg->mist_pos_y_step,
                   (unsigned char)cfg->mist_sec_x_step,
                   (unsigned char)cfg->mist_sec_y_step);
    renderer->SetAnimation(0, 1024);
    
    // Store renderer in user data (we'll manage it through the base class)
    m_user_data = renderer;
    m_current_lens = lens_idx;
    
    SYNCDBG(7, "Mist effect ready");
    return true;
}

void MistEffect::Cleanup()
{
    if (m_current_lens >= 0)
    {
        // Free the mist renderer
        if (m_user_data != NULL)
        {
            delete static_cast<CMistFade*>(m_user_data);
            m_user_data = NULL;
        }
        m_current_lens = -1;
        SYNCDBG(9, "Mist effect cleaned up");
    }
}

TbBool MistEffect::Draw(LensRenderContext* ctx)
{
    if (m_current_lens < 0 || m_user_data == NULL)
    {
        return false;
    }
    
    SYNCDBG(16, "Drawing mist effect");
    
    // Get the mist renderer
    CMistFade* renderer = static_cast<CMistFade*>(m_user_data);
    
    // Mist reads from viewport-aligned source
    unsigned char* viewport_src = ctx->srcbuf + ctx->viewport_x;
    
    // Render mist effect
    renderer->Render(ctx->dstbuf, ctx->dstpitch, viewport_src, ctx->srcpitch,
                    ctx->width, ctx->height);
    renderer->Animate();
    
    ctx->buffer_copied = true;  // Mist writes to dstbuf
    return true;
}

TbBool MistEffect::LoadMistTexture(const char* filename)
{
    // Try to load from asset registry first (ZIP files with mists.json)
    const struct LensMistData* mist = get_lens_mist_data(filename);
    
    if (mist != NULL && mist->data != NULL)
    {
        // Copy from registry (mists are always 256x256)
        memcpy(eye_lens_memory, mist->data, 256 * 256);
        SYNCDBG(7, "Loaded mist '%s' from asset registry", filename);
        return true;
    }
    
    // Fall back to loading raw .raw files from mods/data directories
    // This allows simple file-based mods without requiring ZIP/JSON
    const char* loaded_from = NULL;
    if (LoadAssetWithFallback(filename, (unsigned char*)eye_lens_memory,
                              256 * 256, &loaded_from))
    {
        if (loaded_from != NULL) {
            SYNCDBG(7, "Loaded mist '%s' from mod '%s'", filename, loaded_from);
        } else {
            SYNCDBG(7, "Loaded mist '%s' from base game files", filename);
        }
        return true;
    }
    
    // Neither registry nor file loading worked
    WARNLOG("Failed to load mist '%s' from registry or files", filename);
    return false;
}

/******************************************************************************/
