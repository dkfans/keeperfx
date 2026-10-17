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
#include "kfx/renderer/RendererManager.h"
#include "scrcapt.h"
#include "bflib_basics.h"
#include "bflib_datetm.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "globals.h"

#include "gui_topmsg.h"
#include "game_legacy.h"
#include "frontend.h"
#include "config.h"
#include "config_strings.h"

#include <string.h>
#include <ctype.h>
#include "post_inc.h"
/******************************************************************************/

unsigned char screenshot_format = 1;
unsigned char cap_palette[768];
static TbClockMSec movie_record_started_at;

/******************************************************************************/
TbBool take_screenshot(char *fname)
{
    TbBool frame_open = RendererIsFrameOpen();
    if (!frame_open)
    {
        if (!RendererBeginFrame())
        {
            ERRORLOG("Can't lock canvas");
            return false;
        }
    }
    TbBool success = RendererScheduleScreenshot(fname, screenshot_format);
    if (!frame_open)
    {
        RendererEndFrame();
    }
    return success;
}

TbBool cumulative_screen_shot(void)
{
    char fname[255] = "";
    const char *fext;
    switch (screenshot_format)
    {
        case 1:
        fext = "png";
        break;
      case 2:
        fext = "bmp";
        break;
      default:
        ERRORLOG("Screenshot format incorrectly set.");
        return false;
    }
    unsigned long i;
    for (i = 0; i < 10000; i++)
    {
        snprintf(fname, sizeof(fname), "scrshots/scr%05lu.%s", i, fext);
        if (!LbFileExists(fname)) break;
    }
    if (i >= 10000)
    {
        show_onscreen_msg(turns_per_second, "No free filename for screenshot.");
        return false;
    }
    TbBool ret = take_screenshot(fname);
    if (ret)
    {
        show_onscreen_msg(turns_per_second, "File \"%s\" saved.", fname);
    }
    else
    {
        show_onscreen_msg(turns_per_second, "Cannot save \"%s\".", fname);
    }
    return ret;
}

static int movie_message_scale(void)
{
    if (dbc_initialized && dbc_enabled)
        return scale_value_by_horizontal_resolution((MyScreenWidth >= 640) ? 16 : 32);
    return scale_ui_value_lofi(16);
}

static void show_movie_onscreen_msg(TextStringId stridx)
{
    // The message timer only advances while gameplay is drawing.
    if ((game.game_kind != GKind_LocalGame) && (game.game_kind != GKind_MultiGame))
        return;
    const char *text = get_string(stridx);
    if ((text == NULL) || (text[0] == '\0'))
        return;
    show_onscreen_msg(2 * turns_per_second, "%s", text);
}

static void draw_movie_recording_indicator(void)
{
    if ((local_system_flags & GSF_CaptureMovie) == 0)
        return;
    if (!RendererIsFrameOpen())
        return;
    const char *text = get_string(GUIStr_RecMovie);
    if ((text == NULL) || (text[0] == '\0'))
        return;
    TbClockMSec elapsed_ms = LbTimerClock() - movie_record_started_at;
    long secs = (long)(elapsed_ms / 1000);
    if (secs < 0)
        secs = 0;
    char line[255];
    snprintf(line, sizeof(line), "%s  %ld:%02ld", text, secs / 60, secs % 60);
    LbTextSetFont(winfont);
    LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    RendererSetDrawFlags(Lb_TEXT_HALIGN_LEFT);
    LbTextDrawResized(scale_value_by_horizontal_resolution(160), 0, movie_message_scale(), line);
}

TbBool movie_record_start(void)
{
  if ( anim_record() )
  {
      set_flag(local_system_flags, GSF_CaptureMovie);
      movie_record_started_at = LbTimerClock();
      show_movie_onscreen_msg(GUIStr_RecMovie);
      return true;
  }
  show_movie_onscreen_msg(GUIStr_RecMovieFail);
  return false;
}

TbBool movie_record_stop(void)
{
    clear_flag(local_system_flags, GSF_CaptureMovie);
    anim_stop();
    show_movie_onscreen_msg(GUIStr_RecMovieDone);
    return true;
}

TbBool movie_record_frame(void)
{
    TbBool frame_open = RendererIsFrameOpen();
    if (!frame_open)
    {
        if (!RendererBeginFrame())
            return false;
  }
  RendererPaletteGet(cap_palette);
  // anim_record_frame() reads raw pixels straight from lbDisplay.WScreen --
  // only ever valid for backends without a GPU render path (RendererBeginFrame()
  // only locks the CPU framebuffer for those). GL genuinely has no CPU
  // surface to read back here yet (needs its own glReadPixels()-based
  // capture path, not yet built).
  short result = (lbDisplay.WScreen != NULL) ? anim_record_frame(lbDisplay.WScreen, cap_palette) : false;
  if (!frame_open)
    RendererEndFrame();
  return result;
}

/**
 * Captures the screen to make a gameplay movie or screenshot image.
 * @return Returns 0 if no capturing was performed, nonzero otherwise.
 */
TbBool perform_any_screen_capturing(void)
{
    TbBool captured=0;
    if ((local_system_flags & GSF_CaptureSShot) != 0)
    {
      captured |= cumulative_screen_shot();
      clear_flag(local_system_flags, GSF_CaptureSShot);
    }
    if ((local_system_flags & GSF_CaptureMovie) != 0)
    {
      captured |= movie_record_frame();
    }
    // Drawn after the frame is stored, so the warning stays on screen and out of the movie.
    draw_movie_recording_indicator();
    return captured;
}

/******************************************************************************/
