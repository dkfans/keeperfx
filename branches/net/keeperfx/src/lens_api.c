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
#include "lens_api.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config_lenses.h"
#include "lens_mist.h"
#include "lens_flyeye.h"
#include "vidmode.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_init_lens(unsigned long *lens_mem, int width, int height, int scanln, int nlens);
DLLIMPORT void _DK_draw_lens(unsigned char *dstbuf, unsigned char *srcbuf, unsigned long *lens_mem, int width, int height, int scanln);
DLLIMPORT void _DK_reset_eye_lenses(void);
DLLIMPORT void _DK_initialise_eye_lenses(void);
DLLIMPORT void _DK_setup_eye_lens(long nlens);
/******************************************************************************/
void init_lens(unsigned long *lens_mem, int width, int height, int scanln, int nlens);
void draw_lens(unsigned char *dstbuf, unsigned char *srcbuf, unsigned long *lens_mem, int width, int height, int scanln);
/******************************************************************************/

void init_lens(unsigned long *lens_mem, int width, int height, int scanln, int nlens)
{
  _DK_init_lens(lens_mem, width, height, scanln, nlens);
}

TbBool clear_lens_palette(void)
{
    struct PlayerInfo *player;
    struct LensConfig *lenscfg;
    player = get_my_player();
    // Get lens config and check if it has palette entry
    lenscfg = get_lens_config(game.numfield_1B);
    if ((lenscfg->flags & LCF_HasPalette) != 0)
    {
        // If there is a palette entry, then clear it
        player->field_7 = NULL;
        return true;
    }
    return false;
}

void set_lens_palette(unsigned char *palette)
{
    struct PlayerInfo *player;
    player = get_my_player();
    player->palette = palette;
    player->field_7 = palette;
}

void reset_eye_lenses(void)
{
    free_mist();
    clear_lens_palette();
    if (eye_lens_memory != NULL)
    {
        LbMemoryFree(eye_lens_memory);
        eye_lens_memory = NULL;
    }
    if (eye_lens_spare_screen_memory != NULL)
    {
        LbMemoryFree(eye_lens_spare_screen_memory);
        eye_lens_spare_screen_memory = NULL;
    }
    set_flag_byte(&game.flags_cd, MFlg_EyeLensReady, false);
    game.numfield_1A = 0;
    game.numfield_1B = 0;
}

void initialise_eye_lenses(void)
{
  unsigned long screen_size;
  SYNCDBG(7,"Starting");
  if ((eye_lens_memory != NULL) || (eye_lens_spare_screen_memory != NULL))
  {
    //ERRORLOG("EyeLens Memory already allocated");
    reset_eye_lenses();
  }
  if ((features_enabled & Ft_EyeLens) == 0)
  {
    set_flag_byte(&game.flags_cd,MFlg_EyeLensReady,false);
    return;
  }

  eye_lens_height = lbDisplay.GraphicsScreenHeight;
  eye_lens_width = lbDisplay.GraphicsScreenWidth;
  screen_size = eye_lens_width * eye_lens_height + 2;
  if (screen_size < 256*256) screen_size = 256*256 + 2;
  eye_lens_memory = (unsigned long *)LbMemoryAlloc(screen_size*sizeof(unsigned long));
  eye_lens_spare_screen_memory = (unsigned char *)LbMemoryAlloc(screen_size*sizeof(TbPixel));
  if ((eye_lens_memory == NULL) || (eye_lens_spare_screen_memory == NULL))
  {
    reset_eye_lenses();
    ERRORLOG("Cannot allocate EyeLens memory");
    return;
  }
  SYNCDBG(9,"Buffer dimensions (%d,%d)",eye_lens_width,eye_lens_height);
  set_flag_byte(&game.flags_cd,MFlg_EyeLensReady,true);
}

void setup_eye_lens(long nlens)
{
  //_DK_setup_eye_lens(nlens);return;
  struct PlayerInfo *player;
  struct LensConfig *lenscfg;
  char *fname;
  if ((game.flags_cd & MFlg_EyeLensReady) == 0)
  {
    WARNLOG("Can't setup lens - not initialized");
    return;
  }
  SYNCDBG(7,"Starting for lens %ld",nlens);
  player = get_my_player();
  if (clear_lens_palette())
      game.numfield_1A = 0;
  if (nlens == 0)
  {
      game.numfield_1A = 0;
      game.numfield_1B = 0;
      return;
  }
  if (game.numfield_1A == nlens)
  {
    game.numfield_1B = nlens;
    return;
  }
  lenscfg = get_lens_config(nlens);
  if ((lenscfg->flags & LCF_HasMist) != 0)
  {
      SYNCDBG(9,"Mist config entered");
      fname = prepare_file_path(FGrp_StdData,lenscfg->mist_file);
      LbFileLoadAt(fname, eye_lens_memory);
      setup_mist((unsigned char *)eye_lens_memory,
          &pixmap.fade_tables[(lenscfg->mist_lightness)*256],
          &pixmap.ghost[(lenscfg->mist_ghost)*256]);
  }
  if ((lenscfg->flags & LCF_HasDisplace) != 0)
  {
      SYNCDBG(9,"Displace config entered");
      switch (lenscfg->displace_kind)
      {
      case 1:
      case 2:
          init_lens(eye_lens_memory, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size,
                  lbDisplay.GraphicsScreenWidth, nlens);
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
  game.numfield_1B = nlens;
  game.numfield_1A = nlens;
}

void reinitialise_eye_lens(long nlens)
{
  initialise_eye_lenses();
  if ((game.flags_cd & MFlg_EyeLensReady) && (nlens>0))
  {
      game.numfield_1B = 0;
      setup_eye_lens(nlens);
  }
  SYNCDBG(18,"Finished");
}

void draw_lens(unsigned char *dstbuf, unsigned char *srcbuf, unsigned long *lens_mem, int width, int height, int scanln)
{
  _DK_draw_lens(dstbuf, srcbuf, lens_mem, width, height, scanln);
}

void draw_copy(unsigned char *dstbuf, long dstwidth, unsigned char *srcbuf, long srcwidth, long width, long height)
{
    long i;
    unsigned char *dst;
    unsigned char *src;
    dst = dstbuf;
    src = srcbuf;
    for (i=0; i < height; i++)
    {
        LbMemoryCopy(dst,src,width*sizeof(TbPixel));
        dst += dstwidth;
        src += srcwidth;
    }
}

void draw_lens_effect(unsigned char *dstbuf, long dstwidth, unsigned char *srcbuf, long srcwidth, long width, long height, long effect)
{
    struct LensConfig *lenscfg;
    long copied = 0;
    if ((effect < 1) || (effect > lenses_conf.lenses_count))
    {
        if (effect != 0)
            ERRORLOG("Invalid lens effect %d",effect);
        effect = 0;
    }
    lenscfg = &lenses_conf.lenses[effect];
    if ((lenscfg->flags & LCF_HasMist) != 0)
    {
        draw_mist(dstbuf, dstwidth, srcbuf, srcwidth, width, height);
        copied = true;
    }
    if ((lenscfg->flags & LCF_HasDisplace) != 0)
    {
        switch (lenscfg->displace_kind)
        {
        case 1:
        case 2:
            draw_lens(dstbuf, srcbuf, eye_lens_memory,
                  MyScreenWidth/pixel_size, height, dstwidth);
            copied = true;
            break;
        case 3:
            flyeye_blitsec(srcbuf, dstbuf, MyScreenWidth/pixel_size,
                dstwidth, 1, height);
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
        draw_copy(dstbuf, dstwidth, srcbuf, srcwidth, width, height);
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
