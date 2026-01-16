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

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
uint32_t *eye_lens_memory;
TbPixel *eye_lens_spare_screen_memory;
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
    player->main_palette = palette;
    player->lens_palette = palette;
}

void reset_eye_lenses(void)
{
    SYNCDBG(7,"Starting");
    free_mist();
    clear_lens_palette();
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
        SYNCDBG(9,"Mist config entered");
        char* fname = prepare_file_path(FGrp_StdData, lenscfg->mist_file);
        LbFileLoadAt(fname, eye_lens_memory);
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
        set_lens_palette(lenscfg->palette);
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
