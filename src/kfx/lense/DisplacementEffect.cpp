/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file DisplacementEffect.cpp
 *     Displacement/warping lens effect implementation.
 * @par Purpose:
 *     Self-contained image warping/distortion effect.
 * @par Comment:
 *     Displacement effects warp the image by reading pixels from different
 *     locations, creating wave, ripple, or fisheye-like distortions.
 *     Resolution-independent via pre-computed lookup tables.
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
#include "DisplacementEffect.h"

#include <math.h>
#include <stdlib.h>
#include "../../config_lenses.h"
#include "../../vidmode.h"
#include "../../lens_api.h"

#include "../../keeperfx.hpp"
#include "../../post_inc.h"

// Reference dimensions for resolution-independent scaling
static const int REF_WIDTH = 640;
static const int REF_HEIGHT = 480;

/******************************************************************************/

DisplacementEffect::DisplacementEffect()
    : LensEffect(LensEffectType::Displacement, "Displacement")
    , m_current_lens(-1)
    , m_algorithm(DisplaceAlgo_Sinusoidal)
    , m_magnitude(0)
    , m_period(0)
    , m_lookup_table(nullptr)
    , m_table_width(0)
    , m_table_height(0)
{
}

DisplacementEffect::~DisplacementEffect()
{
    Cleanup();
}

void DisplacementEffect::FreeLookupTable()
{
    if (m_lookup_table != nullptr)
    {
        free(m_lookup_table);
        m_lookup_table = nullptr;
    }
    m_table_width = 0;
    m_table_height = 0;
}

/**
 * Build pre-computed lookup table for current resolution.
 * Computes displacement in virtual 640x480 space, then maps to actual coords.
 */
void DisplacementEffect::BuildLookupTable(long width, long height)
{
    // Free existing table if any
    FreeLookupTable();
    
    // Allocate lookup table
    size_t table_size = width * height * sizeof(DisplaceLookupEntry);
    m_lookup_table = (DisplaceLookupEntry*)malloc(table_size);
    if (m_lookup_table == nullptr)
    {
        ERRORLOG("Failed to allocate displacement lookup table (%" PRIuSIZE " bytes)", SZCAST(table_size));
        return;
    }
    
    m_table_width = width;
    m_table_height = height;
    
    // Fixed-point scale factors (16.16 format)
    const unsigned int scale_x = (REF_WIDTH << 16) / width;
    const unsigned int scale_y = (REF_HEIGHT << 16) / height;
    const unsigned int inv_scale_x = (width << 16) / REF_WIDTH;
    const unsigned int inv_scale_y = (height << 16) / REF_HEIGHT;
    
    // Pre-compute constants
    const double ref_center_x = REF_WIDTH * 0.5;
    const double ref_center_y = REF_HEIGHT * 0.5;
    const double flmag = m_magnitude;
    const double flperiod = m_period;
    const double flmag_sq = m_magnitude * (double)m_magnitude;
    const double fldivs = sqrt(ref_center_y * ref_center_y + ref_center_x * ref_center_x + flmag_sq);
    
    DisplaceLookupEntry* entry = m_lookup_table;
    
    for (long y = 0; y < height; y++)
    {
        int virtual_y = (y * scale_y) >> 16;
        double flpos_y = virtual_y - ref_center_y;
        
        for (long x = 0; x < width; x++)
        {
            int virtual_x = (x * scale_x) >> 16;
            double flpos_x = virtual_x - ref_center_x;
            
            long src_virtual_x, src_virtual_y;
            
            switch (m_algorithm)
            {
            case DisplaceAlgo_Linear:
                src_virtual_x = (virtual_x + (REF_WIDTH >> 1)) / 2;
                src_virtual_y = (virtual_y + (REF_HEIGHT >> 1)) / 2;
                break;
                
            case DisplaceAlgo_Sinusoidal:
                src_virtual_x = (long)(sin(flpos_y / REF_WIDTH * flperiod) * flmag + flpos_x + ref_center_x);
                src_virtual_y = (long)(sin(flpos_x / REF_HEIGHT * flperiod) * flmag + flpos_y + ref_center_y);
                break;
                
            case DisplaceAlgo_Radial:
                {
                    double fldist = sqrt(flpos_x * flpos_x + flpos_y * flpos_y + flmag_sq) / fldivs;
                    src_virtual_x = (long)(fldist * flpos_x + ref_center_x);
                    src_virtual_y = (long)(fldist * flpos_y + ref_center_y);
                    
                    if ((m_period & 1) == 0 && src_virtual_x < 0) src_virtual_x = 0;
                    if ((m_period & 2) == 0 && src_virtual_y < 0) src_virtual_y = 0;
                }
                break;
                
            default:
                src_virtual_x = virtual_x;
                src_virtual_y = virtual_y;
                break;
            }
            
            // Clamp to valid virtual range
            if (src_virtual_x >= REF_WIDTH)  src_virtual_x = REF_WIDTH - 1;
            if (src_virtual_x < 0)           src_virtual_x = 0;
            if (src_virtual_y >= REF_HEIGHT) src_virtual_y = REF_HEIGHT - 1;
            if (src_virtual_y < 0)           src_virtual_y = 0;
            
            // Map to actual resolution
            long actual_src_x = (src_virtual_x * inv_scale_x) >> 16;
            long actual_src_y = (src_virtual_y * inv_scale_y) >> 16;
            
            if (actual_src_x >= width)  actual_src_x = width - 1;
            if (actual_src_y >= height) actual_src_y = height - 1;
            
            entry->src_x = (short)actual_src_x;
            entry->src_y = (short)actual_src_y;
            entry++;
        }
    }
    
    SYNCDBG(7, "Built displacement lookup table %ldx%ld", width, height);
}

TbBool DisplacementEffect::Setup(long lens_idx)
{
    SYNCDBG(8, "Setting up displacement effect for lens %ld", lens_idx);
    
    struct LensConfig* cfg = &lenses_conf.lenses[lens_idx];
    
    m_algorithm = (DisplacementAlgorithm)cfg->displace_kind;
    m_magnitude = cfg->displace_magnitude;
    m_period = cfg->displace_period;
    
    // Algorithm 3 (compound) is handled by FlyeyeEffect
    if (m_algorithm == DisplaceAlgo_Compound)
    {
        SYNCDBG(7, "Displacement algorithm 3 (compound) handled by FlyeyeEffect");
        return false;
    }
    
    // Note: Lookup table built on first Draw() when we know the resolution
    FreeLookupTable();
    
    m_current_lens = lens_idx;
    SYNCDBG(7, "Displacement effect ready (algo=%d, mag=%d, period=%d)",
           m_algorithm, m_magnitude, m_period);
    return true;
}

void DisplacementEffect::Cleanup()
{
    FreeLookupTable();
    m_current_lens = -1;
}

TbBool DisplacementEffect::Draw(LensRenderContext* ctx)
{
    if (m_current_lens < 0)
    {
        return false;
    }
    
    // Build lookup table if needed (resolution may have changed)
    if (m_lookup_table == nullptr || 
        m_table_width != ctx->width || 
        m_table_height != ctx->height)
    {
        BuildLookupTable(ctx->width, ctx->height);
        if (m_lookup_table == nullptr)
        {
            return false;
        }
    }
    
    // Fast lookup-based rendering
    unsigned char* viewport_src = ctx->srcbuf + ctx->viewport_x;
    unsigned char* dst = ctx->dstbuf;
    DisplaceLookupEntry* entry = m_lookup_table;
    
    for (long y = 0; y < ctx->height; y++)
    {
        for (long x = 0; x < ctx->width; x++)
        {
            dst[x] = viewport_src[entry->src_y * ctx->srcpitch + entry->src_x];
            entry++;
        }
        dst += ctx->dstpitch;
    }
    
    ctx->buffer_copied = true;
    return true;
}
