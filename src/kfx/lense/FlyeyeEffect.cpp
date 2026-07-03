/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file FlyeyeEffect.cpp
 *     Flyeye/compound eye lens effect implementation.
 * @par Purpose:
 *     Self-contained fish eye/hexagonal tiling effect.
 * @par Comment:
 *     Creates a compound eye view by rendering hexagonal tiles with radial distortion.
 *     Resolution-independent via proper hex polygon rasterization in reference space.
 * @author   Tomasz Lis, KeeperFX Team
 * @date     11 Mar 2010 - 09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "../../pre_inc.h"
#include "FlyeyeEffect.h"

#include <cmath>
#include <stdlib.h>
#include <string.h>
#include "../../config_lenses.h"
#include "../../lens_api.h"

#include "../../keeperfx.hpp"
#include "../../post_inc.h"

/******************************************************************************/

// Reference dimensions - effect computed at 640x480, then scaled
static const int REF_WIDTH = 640;
static const int REF_HEIGHT = 480;
static const int REF_MAXSIZE = 640;

// Maximum strips per scanline
#define MAX_STRIPS_PER_LINE 26

/**
 * Strip entry for scanline - defines start X and source offset.
 */
struct FlyeyeStrip {
    short start_x;
    short source_off_x;
    short source_off_y;
};

/**
 * Scanline data for reference-space rasterization.
 */
struct FlyeyeScanline {
    int num_strips;
    FlyeyeStrip strips[MAX_STRIPS_PER_LINE];
};

// Temporary storage for reference-space scanlines
static FlyeyeScanline* g_ref_scanlines = nullptr;

/******************************************************************************/

/**
 * Add a strip to a scanline, maintaining sorted order by start_x.
 * Direct port of CHex::AddScan from original.
 */
static void AddScan(FlyeyeScanline* scan, int strip_len, int len_limit, 
                    short source_off_x, short source_off_y)
{
    if (strip_len < 0) strip_len = 0;
    if (strip_len >= REF_WIDTH || strip_len >= len_limit) return;
    if (scan->num_strips >= MAX_STRIPS_PER_LINE) return;
    
    // Find insertion point (sorted by start_x)
    int insert_idx = 0;
    for (int i = 0; i < scan->num_strips; i++)
    {
        int cur_len = scan->strips[i].start_x;
        if (strip_len == cur_len)
        {
            // Update existing
            scan->strips[i].source_off_x = source_off_x;
            scan->strips[i].source_off_y = source_off_y;
            return;
        }
        if (cur_len > strip_len)
            break;
        insert_idx++;
    }
    
    // Shift to make room
    for (int i = scan->num_strips; i > insert_idx; i--)
    {
        scan->strips[i] = scan->strips[i - 1];
    }
    
    // Insert
    scan->strips[insert_idx].start_x = strip_len;
    scan->strips[insert_idx].source_off_x = source_off_x;
    scan->strips[insert_idx].source_off_y = source_off_y;
    scan->num_strips++;
}

/**
 * Rasterize one hexagon into reference-space scanlines.
 * Direct port of CHex class from original lens_flyeye.cpp.
 */
static void RasterizeHexRef(int hex_x, int hex_y)
{
    // Compute hex center in reference coordinate system
    int mwidth = 50 * hex_x;
    int mheight = 60 * hex_y;
    if ((hex_x & 1) != 0)
        mheight += 30;
    
    // Distortion parameters for reference resolution
    double ldpar1 = REF_MAXSIZE * 0.0175;
    double ldpar2 = REF_MAXSIZE * 0.0025;
    
    // 6 hex vertices (relative to screen center)
    long arrA[6], arrB[6];
    arrA[0] = mwidth - 35;  arrB[0] = mheight + 30;
    arrA[1] = mwidth - 15;  arrB[1] = mheight;
    arrA[2] = mwidth + 15;  arrB[2] = mheight;
    arrA[3] = mwidth + 35;  arrB[3] = mheight + 30;
    arrA[4] = mwidth + 15;  arrB[4] = mheight + 60;
    arrA[5] = mwidth - 15;  arrB[5] = mheight + 60;
    
    // Apply radial distortion to vertices
    double ref_center_x = REF_WIDTH * 0.5;
    double ref_center_y = REF_HEIGHT * 0.5;
    
    for (int i = 0; i < 6; i++)
    {
        double varA = arrA[i];
        double varB = arrB[i];
        double len = sqrt(varA * varA + varB * varB) * 0.0025 + 1.0;
        arrA[i] = (long)(ref_center_x + (varA / len) * ldpar2);
        arrB[i] = (long)(ref_center_y + (varB / len) * ldpar2);
    }
    
    // Source offset for this hex
    short source_strip_w = (short)(-hex_x * ldpar1);
    short source_strip_h = (short)(-hex_y * ldpar1);
    
    // Find topmost vertex
    int min_idx = 0;
    for (int i = 1; i < 6; i++)
    {
        if (arrB[i] < arrB[min_idx])
            min_idx = i;
    }
    
    // Rasterize using original BlitHex algorithm
    long scan_num = arrB[min_idx];
    int first_idx = (min_idx + 1) % 6;
    int last_idx = (min_idx + 5) % 6;
    int deltaV1 = 0, deltaV2 = 0;
    int posV1 = 0, posV2 = 0;
    int counter1 = 0, counter2 = 0;
    
    while (1)
    {
        // Advance left edge (counterclockwise)
        int i = first_idx;
        while (counter1 == 0)
        {
            first_idx = (first_idx + 5) % 6;
            if (first_idx == i)
                return;
            posV1 = arrA[first_idx] << 16;
            int n = (first_idx + 5) % 6;
            counter1 = arrB[n] - arrB[first_idx];
            if (counter1 > 0)
            {
                deltaV1 = ((arrA[n] << 16) - posV1) / counter1;
            }
        }
        
        // Advance right edge (clockwise)
        i = last_idx;
        while (counter2 == 0)
        {
            last_idx = (last_idx + 1) % 6;
            if (last_idx == i)
                return;
            int n = (last_idx + 1) % 6;
            counter2 = arrB[n] - arrB[last_idx];
            posV2 = arrA[last_idx] << 16;
            if (counter2 > 0)
            {
                deltaV2 = ((arrA[n] << 16) - posV2) / counter2;
            }
        }
        
        if (counter1 <= 0 || counter2 <= 0)
            return;
        
        // Add strip to scanline
        if (scan_num >= 0 && scan_num < REF_HEIGHT)
        {
            AddScan(&g_ref_scanlines[scan_num], posV1 >> 16, posV2 >> 16,
                    source_strip_w, source_strip_h);
        }
        
        counter1--;
        counter2--;
        posV1 += deltaV1;
        posV2 += deltaV2;
        scan_num++;
    }
}

/******************************************************************************/

FlyeyeEffect::FlyeyeEffect()
    : LensEffect(LensEffectType::Flyeye, "Flyeye")
    , m_current_lens(-1)
    , m_lookup_table(nullptr)
    , m_table_width(0)
    , m_table_height(0)
{
}

FlyeyeEffect::~FlyeyeEffect()
{
    Cleanup();
}

void FlyeyeEffect::FreeLookupTable()
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
 * Build lookup table:
 * 1. Rasterize hexes in reference 640x480 space
 * 2. Convert reference scanlines to screen-resolution lookup table
 */
void FlyeyeEffect::BuildLookupTable(long width, long height)
{
    FreeLookupTable();
    
    // Allocate reference scanlines
    g_ref_scanlines = (FlyeyeScanline*)malloc(REF_HEIGHT * sizeof(FlyeyeScanline));
    if (g_ref_scanlines == nullptr)
    {
        ERRORLOG("Failed to allocate flyeye reference scanlines");
        return;
    }
    
    // Initialize reference scanlines
    for (int y = 0; y < REF_HEIGHT; y++)
    {
        g_ref_scanlines[y].num_strips = 0;
    }
    
    // Rasterize all hexagons in reference space
    for (int hex_y = -12; hex_y <= 12; hex_y++)
    {
        for (int hex_x = -12; hex_x <= 12; hex_x++)
        {
            RasterizeHexRef(hex_x, hex_y);
        }
    }
    
    // Allocate lookup table for actual resolution
    size_t table_size = width * height * sizeof(FlyeyeLookupEntry);
    m_lookup_table = (FlyeyeLookupEntry*)malloc(table_size);
    if (m_lookup_table == nullptr)
    {
        ERRORLOG("Failed to allocate flyeye lookup table (%" PRIuSIZE " bytes)", SZCAST(table_size));
        free(g_ref_scanlines);
        g_ref_scanlines = nullptr;
        return;
    }
    
    m_table_width = width;
    m_table_height = height;
    
    // Scale factors
    double scale_x = (double)width / REF_WIDTH;
    double scale_y = (double)height / REF_HEIGHT;
    
    // Convert reference scanlines to screen-resolution lookup table
    FlyeyeLookupEntry* entry = m_lookup_table;
    
    for (long y = 0; y < height; y++)
    {
        // Map screen Y to reference Y
        int ref_y = (int)(y / scale_y);
        if (ref_y >= REF_HEIGHT) ref_y = REF_HEIGHT - 1;
        
        FlyeyeScanline* scan = &g_ref_scanlines[ref_y];
        
        for (long x = 0; x < width; x++)
        {
            // Map screen X to reference X
            int ref_x = (int)(x / scale_x);
            if (ref_x >= REF_WIDTH) ref_x = REF_WIDTH - 1;
            
            // Find which strip this reference X falls into
            short cur_off_x = 0;
            short cur_off_y = 0;
            
            for (int i = 0; i < scan->num_strips; i++)
            {
                if (ref_x >= scan->strips[i].start_x)
                {
                    cur_off_x = scan->strips[i].source_off_x;
                    cur_off_y = scan->strips[i].source_off_y;
                }
                else
                {
                    break;
                }
            }
            
            // Compute source in reference space, then scale to screen
            int ref_src_x = ref_x + cur_off_x;
            int ref_src_y = ref_y + cur_off_y;
            
            // Clamp reference coords
            if (ref_src_x < 0) ref_src_x = 0;
            if (ref_src_x >= REF_WIDTH) ref_src_x = REF_WIDTH - 1;
            if (ref_src_y < 0) ref_src_y = 0;
            if (ref_src_y >= REF_HEIGHT) ref_src_y = REF_HEIGHT - 1;
            
            // Scale to actual screen coordinates
            long src_x = (long)(ref_src_x * scale_x);
            long src_y = (long)(ref_src_y * scale_y);
            
            if (src_x >= width) src_x = width - 1;
            if (src_y >= height) src_y = height - 1;
            
            entry->src_x = (short)src_x;
            entry->src_y = (short)src_y;
            entry++;
        }
    }
    
    free(g_ref_scanlines);
    g_ref_scanlines = nullptr;
    
    SYNCDBG(7, "Built flyeye lookup table %ldx%ld", width, height);
}

TbBool FlyeyeEffect::Setup(long lens_idx)
{
    SYNCDBG(8, "Setting up flyeye effect for lens %ld", lens_idx);
    
    FreeLookupTable();
    m_current_lens = lens_idx;
    
    SYNCDBG(7, "Flyeye effect ready");
    return true;
}

void FlyeyeEffect::Cleanup()
{
    FreeLookupTable();
    m_current_lens = -1;
}

TbBool FlyeyeEffect::Draw(LensRenderContext* ctx)
{
    if (m_current_lens < 0)
    {
        return false;
    }
    
    // Build lookup table if needed
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
    FlyeyeLookupEntry* entry = m_lookup_table;
    
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