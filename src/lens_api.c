/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_api.c
 *     Eye lenses support functions.
 * @par Purpose:
 *     Functions to support and draw creature eye lens effects.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "lens_api.h"

#include <math.h>
#include <string.h>
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config_lenses.h"
#include "lens_mist.h"
#include "lens_flyeye.h"
#include "vidmode.h"
#include "game_legacy.h"
#include "config_keeperfx.h"
#include "custom_sprites.h"
#include "config_mods.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define RAW_OVERLAY_SIZE 256  // RAW overlay files are 256x256 pixels (matching mist texture format)

// Cache structure for lens overlay data (separate from save files)
struct LensOverlayCache {
    unsigned char *data;
    int width;
    int height;
};

// Overlay cache for each lens (indexed by lens number)
static struct LensOverlayCache overlay_cache[LENS_ITEMS_MAX];

uint32_t *eye_lens_memory;
TbPixel *eye_lens_spare_screen_memory;
/******************************************************************************/
/**
 * Try to load a file from mod data directories with fallback to base game.
 * This is a helper function to reduce code duplication when loading files
 * that support mod overrides.
 * 
 * @param fname_base The base filename (without path) to load
 * @param fgroup The file group (e.g. FGrp_StdData)
 * @param buffer Pre-allocated buffer to load data into
 * @param expected_size Expected file size in bytes
 * @param loaded_from Optional output parameter to store which mod loaded the file (NULL if base game)
 * @return true if file was loaded successfully, false otherwise
 */
static TbBool try_load_file_from_mods_with_fallback(const char* fname_base, short fgroup, 
                                                      unsigned char* buffer, size_t expected_size,
                                                      const char** loaded_from)
{
    // Try loading from all loaded mods' data directories first (same order as mod loading)
    for (int i = 0; i < mods_conf.after_base_cnt; i++) {
        const struct ModConfigItem* mod_item = &mods_conf.after_base_item[i];
        // Only check mods that have a directory (mod_dir flag)
        if (mod_item->state.mod_dir) {
            char mod_dir[256];
            snprintf(mod_dir, sizeof(mod_dir), "%s/%s", MODS_DIR_NAME, mod_item->name);
            char* fname_mod = prepare_file_path_mod(mod_dir, fgroup, fname_base);
            
            SYNCDBG(7, "CONFIG_DEBUG: Checking mod file: '%s'", fname_mod);
            
            // Check if file exists first to avoid error messages
            if (LbFileExists(fname_mod)) {
                // Check file size before loading to prevent buffer overflows
                long file_size = LbFileLengthRnc(fname_mod);
                SYNCDBG(7, "CONFIG_DEBUG: File exists (size: %ld bytes, expected: %lu bytes)", file_size, (unsigned long)expected_size);
                
                // Only load if file size matches expected size (exact match required for safety)
                if (file_size == expected_size) {
                    long loaded = LbFileLoadAt(fname_mod, buffer);
                    SYNCDBG(7, "CONFIG_DEBUG: Loaded %ld bytes", loaded);
                    if (loaded == expected_size) {
                        if (loaded_from != NULL) {
                            *loaded_from = mod_item->name;
                        }
                        return true;
                    }
                } else {
                    WARNLOG("File '%s' has wrong size: %ld bytes (expected %lu bytes)", fname_mod, file_size, (unsigned long)expected_size);
                }
            }
        }
    }
    
    // If not found in mods, try base game directory
    char* fname_base_path = prepare_file_path(fgroup, fname_base);
    if (LbFileExists(fname_base_path)) {
        // Check file size before loading to prevent buffer overflows
        long file_size = LbFileLengthRnc(fname_base_path);
        
        // Only load if file size matches expected size (exact match required for safety)
        if (file_size == expected_size) {
            long loaded = LbFileLoadAt(fname_base_path, buffer);
            if (loaded == expected_size) {
                if (loaded_from != NULL) {
                    *loaded_from = NULL;  // NULL indicates base game
                }
                return true;
            }
        }
    }
    
    return false;
}
/******************************************************************************/
void init_lens(uint32_t *lens_mem, int width, int height, int pitch, int nlens, int mag, int period);
/******************************************************************************/

void init_lens(uint32_t *lens_mem, int width, int height, int pitch, int nlens, int mag, int period)
{
    long w;
    long h;
    long shift_w;
    long shift_h;
    uint32_t *mem;
    double flwidth;
    double flheight;
    double center_w;
    double center_h;
    double flpos_w;
    double flpos_h;
    double flmag;
    switch (nlens)
    {
    case 0:
        mem = lens_mem;
        for (h=0; h < height; h++)
        {
            for (w = 0; w < width; w++)
            {
                *mem = ((h+(height>>1))/2) * pitch + ((w+(width>>1))/2);
                mem++;
            }
        }
        break;
    case 1:
    {
        flmag = mag;
        double flperiod = period;
        flwidth = width;
        flheight = height;
        center_h = flheight * 0.5;
        center_w = flwidth * 0.5;
        flpos_h = -center_h;
        mem = lens_mem;
        for (h=0; h < height; h++)
        {
            flpos_w = -center_w;
            for (w = 0; w < width; w++)
            {
              shift_w = (long)(sin(flpos_h / flwidth  * flperiod) * flmag + flpos_w + center_w);
              shift_h = (long)(sin(flpos_w / flheight * flperiod) * flmag + flpos_h + center_h);
              if (shift_w >= width)
                  shift_w = width - 1;
              if (shift_w < 0)
                  shift_w = 0;
              if (shift_h >= height)
                shift_h = height - 1;
              if (shift_h < 0)
                  shift_h = 0;
              *mem = shift_w + shift_h * pitch;
              flpos_w += 1.0;
              mem++;
            }
            flpos_h += 1.0;
        }
        break;
    }
    case 2:
    {
        {
            flmag = mag * (double)mag;
            flwidth = width;
            flheight = height;
            center_h = flheight * 0.5;
            center_w = flwidth * 0.5;
            double fldivs = sqrt(center_h * center_h + center_w * center_w + flmag);
            flpos_h = -center_h;
            mem = lens_mem;
            for (h = 0; h < height; h++)
            {
                flpos_w = -center_w;
                for (w = 0; w < width; w++)
                {
                    double fldist = sqrt(flpos_w * flpos_w + flpos_h * flpos_h + flmag) / fldivs;
                    shift_w = (long)(fldist * flpos_w + center_w);
                    shift_h = (long)(fldist * flpos_h + center_h);
                    if (shift_w >= width)
                        shift_w = width - 1;
                    if ((shift_w < 0) || ((period & 1) == 0))
                        shift_w = 0;
                    if (shift_h >= height)
                        shift_h = height - 1;
                    if ((shift_h < 0) || ((period & 2) == 0))
                        shift_h = 0;
                    *mem = shift_w + shift_h * pitch;
                    flpos_w += 1.0;
                    mem++;
                }
                flpos_h += 1.0;
            }
            break;
        }
    }
    }
}

static TbBool load_overlay_image(long lens_idx)
{
    if (lens_idx < 0 || lens_idx >= LENS_ITEMS_MAX) {
        ERRORLOG("Invalid lens index %ld", lens_idx);
        return false;
    }
    
    struct LensOverlayCache* cache = &overlay_cache[lens_idx];
    if (cache->data != NULL) {
        // Already loaded
        return true;
    }
    
    struct LensConfig* lenscfg = &lenses_conf.lenses[lens_idx];
    if (lenscfg->overlay_file[0] == '\0') {
        WARNLOG("Empty overlay name");
        return false;
    }
    
    // Check if this is a direct file path (contains .raw or other file extension)
    const char* ext = strrchr(lenscfg->overlay_file, '.');
    if (ext != NULL && strcasecmp(ext, ".raw") == 0) {
        // Load directly from file in /data directory
        char* fname = prepare_file_path(FGrp_StdData, lenscfg->overlay_file);
        
        // RAW overlay files are 256x256 8-bit indexed (same as mist files)
        cache->width = 256;
        cache->height = 256;
        size_t size = cache->width * cache->height;  // 1 byte per pixel
        
        cache->data = (unsigned char*)malloc(size);
        if (cache->data == NULL) {
            ERRORLOG("Failed to allocate memory for overlay image");
            return false;
        }
        
        if (LbFileLoadAt(fname, cache->data) != size) {
            WARNLOG("Failed to load overlay file '%s' from /data directory", lenscfg->overlay_file);
            free(cache->data);
            cache->data = NULL;
            cache->width = 0;
            cache->height = 0;
            return false;
        }
        
        SYNCDBG(7, "Loaded overlay '%s' (%dx%d) from file", lenscfg->overlay_file, cache->width, cache->height);
        return true;
    }
    
    // Look up overlay by name in the custom sprites registry
    const struct LensOverlayData* overlay = get_lens_overlay_data(lenscfg->overlay_file);
    
    // If not found in registry, try loading from /data directory as RAW file fallback
    if (overlay == NULL) {
        // Try loading RAW file from mod data directories, then base game /data
        char fname_raw[256];
        snprintf(fname_raw, sizeof(fname_raw), "%s.raw", lenscfg->overlay_file);
        
        // RAW overlay files are 256x256 8-bit indexed (1 byte per pixel, same as mist files)
        cache->width = 256;
        cache->height = 256;
        size_t size = cache->width * cache->height;  // 65536 bytes
        
        cache->data = (unsigned char*)malloc(size);
        if (cache->data == NULL) {
            ERRORLOG("Failed to allocate memory for overlay image");
            return false;
        }
        
        // Try loading from mods with fallback to base game
        const char* loaded_from = NULL;
        if (try_load_file_from_mods_with_fallback(fname_raw, FGrp_StdData, cache->data, size, &loaded_from)) {
            if (loaded_from != NULL) {
                SYNCDBG(7, "Loaded overlay '%s' (%dx%d) from mod '%s' data directory", fname_raw, cache->width, cache->height, loaded_from);
            } else {
                SYNCDBG(7, "Loaded overlay '%s' (%dx%d) from base game data directory", fname_raw, cache->width, cache->height);
            }
            return true;
        }
        
        // Not found anywhere
        WARNLOG("Lens overlay '%s' not found. Make sure it's defined in a lenses.json file in a .zip or provide a .raw file in /data", lenscfg->overlay_file);
        free(cache->data);
        cache->data = NULL;
        cache->width = 0;
        cache->height = 0;
        return false;
    }
    
    // Found in registry - load it
    if (overlay->data == NULL || overlay->width <= 0 || overlay->height <= 0) {
        WARNLOG("Invalid lens overlay data for '%s'", lenscfg->overlay_file);
        return false;
    }
    
    size_t size = overlay->width * overlay->height;
    cache->data = (unsigned char*)malloc(size);
    if (cache->data == NULL) {
        ERRORLOG("Failed to allocate memory for overlay image");
        return false;
    }
    
    memcpy(cache->data, overlay->data, size);
    cache->width = overlay->width;
    cache->height = overlay->height;
    
    SYNCDBG(7, "Loaded overlay '%s' (%dx%d) from registry", lenscfg->overlay_file, cache->width, cache->height);
    return true;
}

static void free_overlay_image(long lens_idx)
{
    if (lens_idx < 0 || lens_idx >= LENS_ITEMS_MAX) {
        return;
    }
    
    struct LensOverlayCache* cache = &overlay_cache[lens_idx];
    if (cache->data != NULL) {
        free(cache->data);
        cache->data = NULL;
        cache->width = 0;
        cache->height = 0;
    }
}

TbBool clear_lens_palette(void)
{
    SYNCDBG(7,"Staring");
    struct PlayerInfo* player = get_my_player();
    // Get lens config and check if it has palette entry
    struct LensConfig* lenscfg = get_lens_config(game.applied_lens_type);
    if ((lenscfg->flags & LCF_HasPalette) != 0)
    {
        // If there is a palette entry, then clear it
        player->lens_palette = NULL;
        player->main_palette = engine_palette;
        SYNCDBG(9,"Clear done");
        return true;
    }
    SYNCDBG(9,"Clear not needed");
    return false;
}

static void set_lens_palette(unsigned char *palette)
{
    struct PlayerInfo* player = get_my_player();
    SYNCDBG(9,"CONFIG_DEBUG: set_lens_palette called, palette[0-4]: %02x %02x %02x %02x %02x",
        palette[0], palette[1], palette[2], palette[3], palette[4]);
    player->main_palette = palette;
    player->lens_palette = palette;
}

void reset_eye_lenses(void)
{
    SYNCDBG(7,"Starting");
    free_mist();
    clear_lens_palette();
    // Free any loaded overlay images from cache
    for (int i = 0; i < LENS_ITEMS_MAX; i++)
    {
        free_overlay_image(i);
    }
    if (eye_lens_memory != NULL)
    {
        free(eye_lens_memory);
        eye_lens_memory = NULL;
    }
    if (eye_lens_spare_screen_memory != NULL)
    {
        free(eye_lens_spare_screen_memory);
        eye_lens_spare_screen_memory = NULL;
    }
    clear_flag(game.mode_flags, MFlg_EyeLensReady);
    game.active_lens_type = 0;
    game.applied_lens_type = 0;
    SYNCDBG(9,"Done");
}

void initialise_eye_lenses(void)
{
  SYNCDBG(7,"Starting");
  if ((eye_lens_memory != NULL) || (eye_lens_spare_screen_memory != NULL))
  {
    //ERRORLOG("EyeLens Memory already allocated");
    reset_eye_lenses();
  }
  if ((features_enabled & Ft_EyeLens) == 0)
  {
    clear_flag(game.mode_flags, MFlg_EyeLensReady);
    return;
  }

  eye_lens_height = lbDisplay.GraphicsScreenHeight;
  eye_lens_width = lbDisplay.GraphicsScreenWidth;
  unsigned long screen_size = eye_lens_width * eye_lens_height + 2;
  if (screen_size < 256*256) screen_size = 256*256 + 2;
  eye_lens_memory = (uint32_t *)calloc(screen_size, sizeof(uint32_t));
  eye_lens_spare_screen_memory = (unsigned char *)calloc(screen_size, sizeof(TbPixel));
  if ((eye_lens_memory == NULL) || (eye_lens_spare_screen_memory == NULL))
  {
    reset_eye_lenses();
    ERRORLOG("Cannot allocate EyeLens memory");
    return;
  }
  SYNCDBG(9,"Buffer dimensions (%d,%d)",eye_lens_width,eye_lens_height);
  set_flag(game.mode_flags, MFlg_EyeLensReady);
}

void setup_eye_lens(long nlens)
{
    if ((game.mode_flags & MFlg_EyeLensReady) == 0)
    {
        WARNLOG("Can't setup lens - not initialized");
        return;
    }
    SYNCDBG(7,"Starting for lens %ld",nlens);
    if (clear_lens_palette()) {
        game.active_lens_type = 0;
    }
    if (nlens == 0)
    {
        game.active_lens_type = 0;
        game.applied_lens_type = 0;
        return;
    }
    if (game.active_lens_type == nlens)
    {
        game.applied_lens_type = nlens;
        return;
    }
    struct LensConfig* lenscfg = get_lens_config(nlens);
    if ((lenscfg->flags & LCF_HasMist) != 0)
    {        
        // Try to load from registry first (ZIP files with mists.json)
        const struct LensMistData* mist = get_lens_mist_data(lenscfg->mist_file);
        
        if (mist != NULL && mist->data != NULL)
        {
            // Load from registry
            memcpy(eye_lens_memory, mist->data, 256 * 256);
            SYNCDBG(7, "Loaded mist '%s' from registry", lenscfg->mist_file);
        }
        else
        {
            // Fall back to loading from mods with fallback to base game /data directory
            const char* loaded_from = NULL;
            if (try_load_file_from_mods_with_fallback(lenscfg->mist_file, FGrp_StdData, 
                                                        (unsigned char*)eye_lens_memory, 256 * 256, &loaded_from))
            {
                if (loaded_from != NULL) {
                    SYNCDBG(7, "Loaded mist '%s' from mod '%s' data directory", lenscfg->mist_file, loaded_from);
                } else {
                    SYNCDBG(7, "Loaded mist '%s' from base game data directory", lenscfg->mist_file);
                }
            }
            else
            {
                WARNLOG("Failed to load mist file '%s'", lenscfg->mist_file);
            }
        }
        
        setup_mist((unsigned char *)eye_lens_memory,
            &pixmap.fade_tables[(lenscfg->mist_lightness)*256],
            &pixmap.ghost[(lenscfg->mist_ghost)*256]);
    }
    if ((lenscfg->flags & LCF_HasDisplace) != 0)
    {
        SYNCDBG(9,"Displace config %d entered",(int)lenscfg->displace_kind);
        switch (lenscfg->displace_kind)
        {
        case 1:
            init_lens(eye_lens_memory, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size,
                eye_lens_width, 1, lenscfg->displace_magnitude, lenscfg->displace_period);
            break;
        case 2:
            init_lens(eye_lens_memory, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size,
                eye_lens_width, 2, lenscfg->displace_magnitude, lenscfg->displace_period);
            break;
        case 3:
            flyeye_setup(MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
            break;
        }
    }
    if ((lenscfg->flags & LCF_HasPalette) != 0)
    {
        SYNCDBG(9,"Palette config entered");
        WARNLOG("CONFIG_DEBUG: setup_eye_lens %ld calling set_lens_palette, flags=0x%02x", nlens, lenscfg->flags);
        set_lens_palette(lenscfg->palette);
    }
    if ((lenscfg->flags & LCF_HasOverlay) != 0)
    {
        SYNCDBG(7, "Overlay config entered for lens %ld, name='%s'", nlens, lenscfg->overlay_file);
        if (!load_overlay_image(nlens)) {
            WARNLOG("Failed to load overlay for lens %ld", nlens);
        } else {
            SYNCDBG(7, "Successfully loaded overlay %dx%d", overlay_cache[nlens].width, overlay_cache[nlens].height);
        }
    }
    game.applied_lens_type = nlens;
    game.active_lens_type = nlens;
}

void reinitialise_eye_lens(long nlens)
{
  initialise_eye_lenses();
  if ((game.mode_flags & MFlg_EyeLensReady) && (nlens>0))
  {
      game.applied_lens_type = 0;
      setup_eye_lens(nlens);
  }
  SYNCDBG(18,"Finished");
}

static void draw_displacement_lens(unsigned char *dstbuf, unsigned char *srcbuf, uint32_t *lens_mem, int width, int height, int dstpitch)
{
    SYNCDBG(16,"Starting");
    unsigned char* dst = dstbuf;
    uint32_t* mem = lens_mem;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            long pos_map = *mem;
            dst[w] = srcbuf[pos_map];
            mem++;
        }
        dst += dstpitch;
    }
}

void draw_copy(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, long width, long height)
{
    unsigned char* dst = dstbuf;
    unsigned char* src = srcbuf;
    for (long i = 0; i < height; i++)
    {
        memcpy(dst,src,width*sizeof(TbPixel));
        dst += dstpitch;
        src += srcpitch;
    }
}

static void draw_overlay(unsigned char *dstbuf, long dstpitch, long width, long height, long lens_idx)
{
    if (lens_idx < 0 || lens_idx >= LENS_ITEMS_MAX) {
        ERRORLOG("Invalid lens index %ld", lens_idx);
        return;
    }
    
    struct LensOverlayCache* cache = &overlay_cache[lens_idx];
    struct LensConfig* lenscfg = &lenses_conf.lenses[lens_idx];
    
    if (cache->data == NULL) {
        WARNLOG("Overlay data is NULL, cannot draw");
        return;
    }
    
    // Validate dimensions
    if (width <= 0 || height <= 0 || cache->width <= 0 || cache->height <= 0) {
        WARNLOG("Invalid dimensions for overlay rendering: screen=%ldx%ld, overlay=%dx%d", width, height, cache->width, cache->height);
        return;
    }
    
    SYNCDBG(8, "Drawing overlay: screen=%ldx%ld, overlay=%dx%d, alpha=%d", width, height, cache->width, cache->height, lenscfg->overlay_alpha);
    
    // Calculate scale factors to stretch/fit overlay to fill entire viewport
    float scale_x = (float)cache->width / width;
    float scale_y = (float)cache->height / height;
    
    // Determine dithering threshold based on alpha (0-255)
    // alpha=255 (opaque): draw all pixels
    // alpha=128 (50%): draw checkerboard pattern
    // alpha=0 (transparent): draw nothing
    int alpha = lenscfg->overlay_alpha;
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    
    // Draw the overlay with alpha transparency using dithering
    unsigned char* dst = dstbuf;
    for (long y = 0; y < height; y++)
    {
        int src_y = (int)(y * scale_y);
        if (src_y >= cache->height) src_y = cache->height - 1;
        
        unsigned char* src_row = cache->data + (src_y * cache->width);
        
        for (long x = 0; x < width; x++)
        {
            int src_x = (int)(x * scale_x);
            if (src_x >= cache->width) src_x = cache->width - 1;
            
            unsigned char overlay_pixel = src_row[src_x];
            
            // Palette index 255 is used as transparency marker - skip it completely
            if (overlay_pixel != 255)
            {
                // Apply alpha dithering using checkerboard pattern
                if (alpha >= 255)
                {
                    // Fully opaque - always draw
                    dst[x] = overlay_pixel;
                }
                else if (alpha > 0)
                {
                    // Partial transparency - use checkerboard dithering
                    // Dither pattern alternates based on x+y coordinate
                    int dither = ((x + y) & 1) ? 255 : 0;
                    if (alpha > dither)
                    {
                        dst[x] = overlay_pixel;
                    }
                }
                // If alpha == 0, skip pixel (fully transparent)
            }
        }
        dst += dstpitch;
    }
}

void draw_lens_effect(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, long width, long height, long effect)
{
    long copied = 0;
    if ((effect < 1) || (effect > lenses_conf.lenses_count))
    {
        if (effect != 0)
            ERRORLOG("Invalid lens effect %ld",effect);
        effect = 0;
    }
    struct LensConfig* lenscfg = &lenses_conf.lenses[effect];
    if ((lenscfg->flags & LCF_HasMist) != 0)
    {
        draw_mist(dstbuf, dstpitch, srcbuf, srcpitch, width, height);
        copied = true;
    }
    if ((lenscfg->flags & LCF_HasDisplace) != 0)
    {
        switch (lenscfg->displace_kind)
        {
        case 1:
        case 2:
            draw_displacement_lens(dstbuf, srcbuf, eye_lens_memory,
                width, height, dstpitch);
            copied = true;
            break;
        case 3:
            flyeye_blitsec(srcbuf, srcpitch, dstbuf, dstpitch, 1, height);
            copied = true;
            break;
        }
    }
    if ((lenscfg->flags & LCF_HasPalette) != 0)
    {
        // Nothing to do - palette is just set and don't have to be drawn
    }
    // Draw overlay effect if present
    if ((lenscfg->flags & LCF_HasOverlay) != 0)
    {
        // First, copy the source buffer to destination if not already done
        if (!copied)
        {
            draw_copy(dstbuf, dstpitch, srcbuf, srcpitch, width, height);
        }
        // Load overlay if not already loaded
        if (load_overlay_image(effect)) {
            // Now draw the overlay on top of the game scene
            SYNCDBG(8, "Calling draw_overlay (flags=%d, lens_idx=%ld)", lenscfg->flags, effect);
            draw_overlay(dstbuf, dstpitch, width, height, effect);
        }
        copied = true;
    }
    // If we haven't copied the buffer to screen yet, do so now
    if (!copied)
    {
        draw_copy(dstbuf, dstpitch, srcbuf, srcpitch, width, height);
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
