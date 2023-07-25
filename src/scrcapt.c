/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file scrcapt.c
 *     Screen capturing functions.
 * @par Purpose:
 *     Functions to read display buffer and store it in various formats.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "scrcapt.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_bufrw.h"
#include "bflib_memory.h"
#include "bflib_dernc.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidsurface.h"
#include "globals.h"

#include "gui_topmsg.h"
#include "game_legacy.h"
#include "frontend.h"
#include "config.h"

#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <ctype.h>
#include "post_inc.h"
/******************************************************************************/

unsigned char screenshot_format = 0;
unsigned char cap_palette[768];

/******************************************************************************/
TbBool take_screenshot(char *fname)
{
    TbBool lock_mem = LbScreenIsLocked();
    if (!lock_mem)
    {
        if (LbScreenLock() != Lb_SUCCESS)
        {
            ERRORLOG("Can't lock canvas");
            return false;
        }
    }
    TbBool success;
    switch (screenshot_format)
    {
        case 0:
        {
            success = (IMG_SavePNG(lbDrawSurface, fname) == 0);
            break;
        }
        case 1:
        {
            success = (SDL_SaveBMP(lbDrawSurface, fname) == 0);
            break;
        }
        default:
        {
            success = false;
            break;
        }
    }
    if (!success)
    {
        ERRORLOG("Unable to save to file %s: %s", fname, SDL_GetError());
    }
    if (!lock_mem)
    {
        LbScreenUnlock();
    }
    return success;
}

TbBool cumulative_screen_shot(void)
{
    if (screenshot_format > 1)
    {
        ERRORLOG("Screenshot format incorrectly set.");
        return false;
    }
    static long frame_number = 0;
    char fname[255];
    size_t len = strlen(scrshot_type[screenshot_format].name);
    char *fext = malloc(len);
    long i;
    for (i = 0; i < len; i++) 
    {
        fext[i] = tolower(scrshot_type[screenshot_format].name[i]);
    }
    for (i = frame_number; i < 10000; i++)
    {
        sprintf(fname, "scrshots/scr%05ld.%s", i, fext);
        if (!LbFileExists(fname)) break;
    }
    frame_number = i;
    if (frame_number >= 10000)
    {
        show_onscreen_msg(game.num_fps, "No free filename for screenshot.");
        free(fext);
        return false;
    }
    sprintf(fname, "scrshots/scr%05ld.%s", frame_number, fext);
    free(fext);
    TbBool ret = take_screenshot(fname);
    if (ret)
    {
        show_onscreen_msg(game.num_fps, "File \"%s\" saved.", fname);
    }
    else
    {
        show_onscreen_msg(game.num_fps, "Cannot save \"%s\".", fname);
    }
    frame_number++;
    return ret;
}

TbBool movie_record_start(void)
{
  if ( anim_record() )
  {
      set_flag_byte(&game.system_flags,GSF_CaptureMovie,true);
      return true;
  }
  return false;
}

TbBool movie_record_stop(void)
{
    set_flag_byte(&game.system_flags,GSF_CaptureMovie,false);
    anim_stop();
    return true;
}

TbBool movie_record_frame(void)
{
    short lock_mem = LbScreenIsLocked();
    if (!lock_mem)
    {
        if (LbScreenLock() != Lb_SUCCESS)
            return false;
  }
  LbPaletteGet(cap_palette);
  short result = anim_record_frame(lbDisplay.WScreen, cap_palette);
  if (!lock_mem)
    LbScreenUnlock();
  return result;
}

/**
 * Captures the screen to make a gameplay movie or screenshot image.
 * @return Returns 0 if no capturing was performed, nonzero otherwise.
 */
TbBool perform_any_screen_capturing(void)
{
    TbBool captured=0;
    if ((game.system_flags & GSF_CaptureSShot) != 0)
    {
      captured |= cumulative_screen_shot();
      set_flag_byte(&game.system_flags,GSF_CaptureSShot,false);
    }
    if ((game.system_flags & GSF_CaptureMovie) != 0)
    {
      captured |= movie_record_frame();
    }
    // Draw a text with bitmap font
    if (captured) {
        //Set font; if winfont isn't loaded, it should be NULL, so text will just be invisible
        LbTextSetFont(winfont);
        LbTextDraw(600*units_per_pixel/16, 4*units_per_pixel/16, "REC");
    }
    return captured;
}

/******************************************************************************/
