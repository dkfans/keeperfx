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
#include "globals.h"

#include "gui_topmsg.h"
#include "game_legacy.h"
#include "frontend.h"

#include <string.h>
/******************************************************************************/

short screenshot_format=1;
unsigned char cap_palette[768];

/******************************************************************************/
long prepare_hsi_screenshot(unsigned char *buf,unsigned char *palette)
{
    long i;
    long pos = 0;
    int w = MyScreenWidth / pixel_size;
    int h = MyScreenHeight / pixel_size;

    write_int8_buf(buf + pos, 'm');
    pos++;
    write_int8_buf(buf + pos, 'h');
    pos++;
    write_int8_buf(buf + pos, 'w');
    pos++;
    write_int8_buf(buf + pos, 'a');
    pos++;
    write_int8_buf(buf + pos, 'n');
    pos++;
    write_int8_buf(buf + pos, 'h');
    pos++;
    // pos=6
    write_int16_be_buf(buf + pos, 4);
    pos += 2;
    write_int16_be_buf(buf + pos, w);
    pos += 2;
    write_int16_be_buf(buf + pos, h);
    pos += 2;
    write_int16_be_buf(buf + pos, 256);
    pos += 2;
    // pos=14
    write_int16_be_buf(buf + pos, 256);
    pos += 2;
    write_int16_be_buf(buf + pos, 256);
    pos += 2;
    write_int16_be_buf(buf + pos, 256);
    pos += 2;
    // pos=20
    for (i = 0; i < 6; i++)
    {
        write_int16_be_buf(buf + pos, 0);
        pos += 2;
  }
  for (i=0; i<768; i+=3)
  {
    write_int8_buf(buf+pos,4*palette[i+0]);pos++;
    write_int8_buf(buf+pos,4*palette[i+1]);pos++;
    write_int8_buf(buf+pos,4*palette[i+2]);pos++;
  }
  short lock_mem = LbScreenIsLocked();
  if (!lock_mem)
  {
    if (LbScreenLock() != Lb_SUCCESS)
    {
      ERRORLOG("Can't lock canvas");
      LbMemoryFree(buf);
      return 0;
    }
  }
  for (i=0; i<h; i++)
  {
    memcpy(buf+pos, lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth*i, w);
    pos += w;
  }
  if (!lock_mem)
    LbScreenUnlock();
  return pos;
}

long prepare_bmp_screenshot(unsigned char *buf,unsigned char *palette)
{
    long i;
    long j;
    long pos = 0;
    int width = MyScreenWidth / pixel_size;
    int height = MyScreenHeight / pixel_size;
    write_int8_buf(buf + pos, 'B');
    pos++;
    write_int8_buf(buf + pos, 'M');
    pos++;
    int padding_size = 4 - (width & 3);
    long data_len = (width + padding_size) * height;
    long pal_len = 256 * 4;
    write_int32_le_buf(buf + pos, data_len + pal_len + 0x36);
    pos += 4;
    write_int32_le_buf(buf + pos, 0);
    pos += 4;
    write_int32_le_buf(buf + pos, pal_len + 0x36);
    pos += 4;
    write_int32_le_buf(buf + pos, 40);
    pos += 4;
    write_int32_le_buf(buf + pos, width);
    pos += 4;
    write_int32_le_buf(buf + pos, height);
    pos += 4;
    write_int16_le_buf(buf + pos, 1);
    pos += 2;
    write_int16_le_buf(buf + pos, 8);
    pos += 2;
    write_int32_le_buf(buf + pos, 0);
    pos += 4;
    write_int32_le_buf(buf + pos, 0);
    pos += 4;
    write_int32_le_buf(buf + pos, 0);
    pos += 4;
    write_int32_le_buf(buf + pos, 0);
    pos += 4;
    write_int32_le_buf(buf + pos, 0);
    pos += 4;
    write_int32_le_buf(buf + pos, 0);
    pos += 4;
    for (i = 0; i < 768; i += 3)
    {
        unsigned int cval = 4 * (unsigned int)palette[i + 2];
        if (cval > 255)
            cval = 255;
        write_int8_buf(buf + pos, cval);
        pos++;
        cval = 4 * (unsigned int)palette[i + 1];
        if (cval > 255)
            cval = 255;
        write_int8_buf(buf + pos, cval);
        pos++;
        cval = 4 * (unsigned int)palette[i + 0];
        if (cval > 255)
            cval = 255;
        write_int8_buf(buf + pos, cval);
        pos++;
        write_int8_buf(buf + pos, 0);
        pos++;
  }
  short lock_mem = LbScreenIsLocked();
  if (!lock_mem)
  {
    if (LbScreenLock() != Lb_SUCCESS)
    {
      ERRORLOG("Can't lock canvas");
      LbMemoryFree(buf);
      return 0;
    }
  }
  for (i=0; i<height; i++)
  {
    memcpy(buf+pos, lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth*(height-i-1), width);
    pos += width;
    if ((padding_size&3) > 0)
      for (j=0; j < padding_size; j++)
      {
        write_int8_buf(buf+pos,0);pos++;
      }
  }
  if (!lock_mem)
    LbScreenUnlock();
  return pos;
}

TbBool cumulative_screen_shot(void)
{
  //_DK_cumulative_screen_shot();return;
  static long frame_number=0;
  char fname[255];
  const char *fext;
  switch (screenshot_format)
  {
  case 1:
    fext="raw";
    break;
  case 2:
    fext="bmp";
    break;
  default:
    ERRORLOG("Screenshot format incorrectly set.");
    return false;
  }
  long i;
  long ssize;
  for (i=frame_number; i<10000; i++)
  {
    sprintf(fname, "scrshots/scr%05ld.%s", i, fext);
    if (!LbFileExists(fname)) break;
  }
  frame_number = i;
  if (frame_number >= 10000)
  {
    show_onscreen_msg(game.num_fps, "No free filename for screenshot.");
    return false;
  }
  sprintf(fname, "scrshots/scr%05ld.%s", frame_number, fext);

  int w = MyScreenWidth / pixel_size;
  int h = MyScreenHeight / pixel_size;

  unsigned char* buf = LbMemoryAlloc((w + 3) * h + 2048);
  if (buf == NULL)
  {
    ERRORLOG("Can't allocate buffer");
    return false;
  }
  LbPaletteGet(cap_palette);
  switch (screenshot_format)
  {
  case 1:
    ssize=prepare_hsi_screenshot(buf,cap_palette);
    break;
  case 2:
    ssize=prepare_bmp_screenshot(buf,cap_palette);
    break;
  default:
    ssize=0;
    break;
  }
  if (ssize>0)
    ssize = LbFileSaveAt(fname, buf, ssize);
  LbMemoryFree(buf);
  if (ssize>0)
    show_onscreen_msg(game.num_fps, "File \"%s\" saved.", fname);
  else
    show_onscreen_msg(game.num_fps, "Cannot save \"%s\".", fname);
  frame_number++;
  return (ssize>0);
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
