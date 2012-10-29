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
#include "vidfade.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_video.h"
#include "vidmode.h"
#include "kjm_input.h"
#include "front_simple.h"
#include "player_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
TbBool lbAdvancedFade = true;
/******************************************************************************/
DLLIMPORT long _DK_PaletteFadePlayer(struct PlayerInfo *player);
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
  unsigned char *dst;
  unsigned long i,k;
  unsigned long r,g,b;
  unsigned long rr,rg,rb;
  SYNCMSG("Recomputing fade tables");
  // Intense fade to/from black - slower fade near black
  dst = coltbl->fade_tables;
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
    rr = spal[3*i+0];
    rg = spal[3*i+1];
    rb = spal[3*i+2];
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

void ProperFadePalette(unsigned char *pal, long fade_steps, enum TbPaletteFadeFlag flg)
{
/*    if (flg != Lb_PALETTE_FADE_CLOSED)
    {
        LbPaletteFade(pal, fade_steps, flg);
    } else*/
    if (lbAdvancedFade)
    {
        TbClockMSec last_loop_time;
        last_loop_time = LbTimerClock();
        while (LbPaletteFade(pal, fade_steps, Lb_PALETTE_FADE_OPEN) < fade_steps)
        {
          if (!is_key_pressed(KC_SPACE,KMod_DONTCARE) &&
              !is_key_pressed(KC_ESCAPE,KMod_DONTCARE) &&
              !is_key_pressed(KC_RETURN,KMod_DONTCARE) &&
              !is_mouse_pressed_lrbutton())
          {
            last_loop_time += 25;
            LbSleepUntil(last_loop_time);
          }
        }
    } else
    if (pal != NULL)
    {
        LbPaletteSet(pal);
    } else
    {
        LbMemorySet(palette_buf, 0, sizeof(palette_buf));
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
        TbClockMSec last_loop_time;
        last_loop_time = LbTimerClock();
        while (LbPaletteFade(pal, fade_steps, Lb_PALETTE_FADE_OPEN) < fade_steps)
        {
          last_loop_time += 25;
          LbSleepUntil(last_loop_time);
        }
    } else
    if (pal != NULL)
    {
        LbPaletteSet(pal);
    } else
    {
        LbMemorySet(palette_buf, 0, sizeof(palette_buf));
        LbPaletteSet(palette_buf);
    }
}

long PaletteFadePlayer(struct PlayerInfo *player)
{
  long i,step;
  unsigned char palette[PALETTE_SIZE];
  unsigned char *dst;
  unsigned char *src;
  unsigned long pix;
  //return _DK_PaletteFadePlayer(player);
  // Find the fade step
  if ((player->field_4C1 != 0) && (player->field_4C5 != 0))
  {
    i = 12 * (player->field_4C1-1) + 10 * (player->field_4C5-1);
  } else
  if (player->field_4C5 != 0)
  {
    i = 2 * (5 * (player->field_4C5-1));
  } else
  if (player->field_4C1 != 0)
  {
    i = 4 * (3 * (player->field_4C1-1));
  } else
  { // both are == 0 - no fade
    return 0;
  }
  if (i >= 120)
    i = 120;
  step = 120 - i;
  // Create the new palette
  for (i=0; i < PALETTE_COLORS; i++)
  {
    src = &player->palette[3*i];
    dst = &palette[3*i];
    pix = ((step * (((long)src[0]) - 63)) / 120) + 63;
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
  if (player->field_4C1 > 0)
    player->field_4C1--;
  if ((player->field_4C5 == 0) || (player->instance_num == 18) || (player->instance_num == 17))
  {
  } else
  if ((player->instance_num == 5) || (player->instance_num == 6))
  {
    if (player->field_4C5 <= 12)
      player->field_4C5++;
  } else
  {
    if (player->field_4C5 > 0)
      player->field_4C5--;
  }
  // Set the palette to screen
  LbScreenWaitVbi();
  LbPaletteSet(palette);
  return step;
}

void PaletteApplyPainToPlayer(struct PlayerInfo *player, long intense)
{
    long i;
    i = player->field_4C1 + intense;
    if (i < 1)
        i = 1;
    else
    if (i > 10)
        i = 10;
    player->field_4C1 = i;
}

void PaletteClearPainFromPlayer(struct PlayerInfo *player)
{
    player->field_4C1 = 0;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
