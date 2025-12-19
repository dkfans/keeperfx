/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidfade.c
 *     Video fading routines.
 * @par Purpose:
 *     Helper functions for fading of video screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     16 Jul 2010 - 05 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "vidfade.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_video.h"

#include "vidmode.h"
#include "kjm_input.h"
#include "front_simple.h"
#include "player_data.h"
#include "player_instances.h"
#include "keeperfx.hpp"
#include "config_keeperfx.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static TbBool lbAdvancedFade = true;
static int lbFadeDelay = 25;

unsigned char fade_palette_in;
unsigned char frontend_palette[768];
TbRGBColorTable colours;
/******************************************************************************/
void fade_in(void)
{
    ProperFadePalette(frontend_palette, 8, Lb_PALETTE_FADE_OPEN);
}

void fade_out(void)
{
    ProperFadePalette(NULL, 8, Lb_PALETTE_FADE_CLOSED);
    LbScreenClear(0);
}

void compute_fade_tables(struct TbColorTables *coltbl,unsigned char *spal,unsigned char *dpal)
{
    unsigned long i;
    unsigned long k;
    unsigned long r;
    unsigned long g;
    unsigned long b;
    SYNCMSG("Recomputing fade tables");
    // Intense fade to/from black - slower fade near black
    unsigned char* dst = coltbl->fade_tables;
    for (i=0; i < 32; i++)
    {
      for (k=0; k < 256; k++)
      {
        r = spal[3*k+0];
        g = spal[3*k+1];
        b = spal[3*k+2];
        *dst = LbPaletteFindColour(dpal, i * r >> 5, i * g >> 5, i * b >> 5);
        dst++;
      }
    }
    // Intense fade to/from black - faster fade part
    for (i=32; i < 192; i+=3)
    {
      for (k=0; k < 256; k++)
      {
        r = spal[3*k+0];
        g = spal[3*k+1];
        b = spal[3*k+2];
        *dst = LbPaletteFindColour(dpal, i * r >> 5, i * g >> 5, i * b >> 5);
        dst++;
      }
    }
    // Other fadings - between all the colors
    dst = coltbl->ghost;
    for (i=0; i < 256; i++)
    {
      // Reference colors
      unsigned long rr = spal[3 * i + 0];
      unsigned long rg = spal[3 * i + 1];
      unsigned long rb = spal[3 * i + 2];
      // Creating fades
      for (k=0; k < 256; k++)
      {
        r = dpal[3*k+0];
        g = dpal[3*k+1];
        b = dpal[3*k+2];
        *dst = LbPaletteFindColour(dpal, (rr+2*r) / 3, (rg+2*g) / 3, (rb+2*b) / 3);
        dst++;
      }
    }
}

void compute_alpha_table(unsigned char *alphtbl, unsigned char *spal, unsigned char *dpal, char dred, char dgreen, char dblue)
{
    int blendR = 0;
    int blendG = 0;
    int blendB = 0;
    // Every color alpha-blended with given values for 8 steps of intensity
    for (int nrow = 0; nrow < 8; nrow++)
    {
        for (int n = 0; n < 256; n++)
        {
            unsigned char* baseCol = &spal[3 * n];
            int valR = blendR + baseCol[0];
            if (valR >= 63)
              valR = 63;
            else if (valR < 0)
              valR = 0;
            int valG = blendG + baseCol[1];
            if (valG >= 63)
              valG = 63;
            else if (valG < 0)
              valG = 0;
            int valB = blendB + baseCol[2];
            if (valB >= 63)
              valB = 63;
            else if (valB < 0)
              valB = 0;

            TbPixel c = LbPaletteFindColour(dpal, valR, valG, valB);
            alphtbl[nrow*256 + n] = c;
        }
        blendR += dred;
        blendG += dgreen;
        blendB += dblue;
    }
}

void compute_alpha_tables(struct TbAlphaTables *alphtbls,unsigned char *spal,unsigned char *dpal)
{
    SYNCMSG("Recomputing alpha tables");
    {
        for (int n = 0; n < 256; n++)
        {
            alphtbls->black[n] = 144;
        }
    }
    // Every color alpha-blended with shade of white
    compute_alpha_table(alphtbls->white,  spal, dpal, 4, 4, 4);
    // Every color alpha-blended with yellow
    compute_alpha_table(alphtbls->yellow, spal, dpal, 6, 4, 0);
    // Every color alpha-blended with red
    compute_alpha_table(alphtbls->red,    spal, dpal, 6, 1, 1);
    // Every color alpha-blended with blue
    compute_alpha_table(alphtbls->blue,   spal, dpal, 2, 2, 6);
    // Every color alpha-blended with green
    compute_alpha_table(alphtbls->green,  spal, dpal, 2, 6, 2);
    // Every color alpha-blended with purple
    compute_alpha_table(alphtbls->purple, spal, dpal, 3, 0, 3);
    // Every color alpha-blended with black
    compute_alpha_table(alphtbls->black,  spal, dpal,-2,-2,-2);
    // Every color alpha-blended with orange
    compute_alpha_table(alphtbls->orange, spal, dpal, 6, 3, 1);
}

void compute_rgb2idx_table(TbRGBColorTable ctab,unsigned char *spal)
{
    SYNCMSG("Recomputing rgb-to-index tables");
    int scaler = (1 << 6) / COLOUR_TABLE_DIMENSION;
    for (int valR = 0; valR < COLOUR_TABLE_DIMENSION; valR++)
    {
        for (int valG = 0; valG < COLOUR_TABLE_DIMENSION; valG++)
        {
            for (int valB = 0; valB < COLOUR_TABLE_DIMENSION; valB++)
            {
                TbPixel c = LbPaletteFindColour(spal, scaler * valR + (scaler-1),
                    scaler * valG + (scaler-1), scaler * valB + (scaler-1));
                ctab[valR][valG][valB] = c;
            }
        }
    }
}

/**
 * Gets colours from source palette, adds given shifts to every colour and encodes it to index in destination palette.
 * @param ocol Output colours buffer.
 * @param spal Source palette, from which initial colors are taken.
 * @param dpal Destination palette, in which the output colors are coded.
 * @param shiftR Color intensity shift value, red.
 * @param shiftG Color intensity shift value, green.
 * @param shiftB Color intensity shift value, blue.
 */
void compute_shifted_palette_table(TbPixel *ocol, const unsigned char *spal, const unsigned char *dpal, int shiftR, int shiftG, int shiftB)
{
    SYNCMSG("Recomputing palette table");
    for (int i = 0; i < 256; i++)
    {
        int valR = (int)spal[3 * i + 0] + shiftR;
        if (valR >= 63) valR = 63;
        if (valR <   0) valR = 0;
        int valG = (int)spal[3 * i + 1] + shiftG;
        if (valG >= 63) valG = 63;
        if (valG <   0) valG = 0;
        int valB = (int)spal[3 * i + 2] + shiftB;
        if (valB >= 63) valB = 63;
        if (valB <   0) valB = 0;
        ocol[i] = LbPaletteFindColour(dpal, valR, valG, valB);
    }
}

void ProperFadePalette(unsigned char *pal, long fade_steps, enum TbPaletteFadeFlag flg)
{
/*    if (flg != Lb_PALETTE_FADE_CLOSED)
    {
        LbPaletteFade(pal, fade_steps, flg);
    } else*/
    if (lbAdvancedFade)
    {
        TbClockMSec latest_loop_time = LbTimerClock();
        while (LbPaletteFade(pal, fade_steps, Lb_PALETTE_FADE_OPEN) < fade_steps)
        {
          if (!is_key_pressed(KC_SPACE,KMod_DONTCARE) &&
              !is_key_pressed(KC_ESCAPE,KMod_DONTCARE) &&
              !is_key_pressed(KC_RETURN,KMod_DONTCARE) &&
              !is_mouse_pressed_lrbutton())
          {
            latest_loop_time += lbFadeDelay;
            LbSleepUntil(latest_loop_time);
          }
        }
    } else
    if (pal != NULL)
    {
        LbPaletteSet(pal);
    } else
    {
        LbPaletteDataFillBlack(palette_buf);
        LbPaletteSet(palette_buf);
    }
}

void ProperForcedFadePalette(unsigned char *pal, long fade_steps, enum TbPaletteFadeFlag flg)
{
    if (flg == Lb_PALETTE_FADE_OPEN)
    {
        LbPaletteFade(pal, fade_steps, flg);
        return;
    }
    if (lbAdvancedFade)
    {
        TbClockMSec latest_loop_time = LbTimerClock();
        while (LbPaletteFade(pal, fade_steps, Lb_PALETTE_FADE_OPEN) < fade_steps)
        {
          latest_loop_time += lbFadeDelay;

          if (flag_is_set(start_params.startup_flags, (SFlg_Legal|SFlg_FX))) {
              LbSleepUntil(latest_loop_time);
          }
        }
    } else
    if (pal != NULL)
    {
        LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        LbPaletteSet(palette_buf);
    }
}

long PaletteFadePlayer(struct PlayerInfo *player)
{
    if (player->main_palette != engine_palette && player->main_palette != player->lens_palette) {
        ERRORLOG("main_palette (%p) is invalid (engine: %p, lens: %p), resetting", player->main_palette, engine_palette, player->lens_palette);
        player->main_palette = engine_palette;
    }
    long i;
    unsigned char palette[PALETTE_SIZE];
    // Find the fade step
    if ((player->palette_fade_step_pain != 0) && (player->palette_fade_step_possession != 0))
    {
        i = 12 * (player->palette_fade_step_pain - 1) + 10 * (player->palette_fade_step_possession - 1);
  } else
  if (player->palette_fade_step_possession != 0)
  {
    i = 2 * (5 * (player->palette_fade_step_possession-1));
  } else
  if (player->palette_fade_step_pain != 0)
  {
    i = 4 * (3 * (player->palette_fade_step_pain-1));
  } else
  { // both are == 0 - no fade
    return 0;
  }
  if (i >= 120)
    i = 120;
  long step = 120 - i;
  // Create the new palette
  for (i=0; i < PALETTE_COLORS; i++)
  {
      unsigned char* src = &player->main_palette[3 * i];
      unsigned char* dst = &palette[3 * i];
      unsigned long pix = ((step * (((long)src[0]) - 63)) / 120) + 63;
      if (pix > 63)
          pix = 63;
      dst[0] = pix;
      pix = (step * ((long)src[1])) / 120;
      if (pix > 63)
          pix = 63;
      dst[1] = pix;
      pix = (step * ((long)src[2])) / 120;
      if (pix > 63)
          pix = 63;
      dst[2] = pix;
  }
  // Update the fade step
  if (player->palette_fade_step_pain > 0)
    player->palette_fade_step_pain--;
  if ((player->palette_fade_step_possession == 0) || (player->instance_num == PI_UnusedSlot18) || (player->instance_num == PI_UnusedSlot17))
  {
  } else
  if ((player->instance_num == PI_DirctCtrl) || (player->instance_num == PI_PsngrCtrl))
  {
    if (player->palette_fade_step_possession <= 12)
      player->palette_fade_step_possession++;
  } else
  {
    if (player->palette_fade_step_possession > 0)
      player->palette_fade_step_possession--;
  }
  // Set the palette to screen
  LbScreenWaitVbi();
  LbPaletteSet(palette);
  return step;
}

void PaletteApplyPainToPlayer(struct PlayerInfo *player, long intense)
{
    long i = player->palette_fade_step_pain + intense;
    if (i < 1)
        i = 1;
    else
    if (i > 10)
        i = 10;
    player->palette_fade_step_pain = i;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
