/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_simple.c
 *     Simple frontend screens support.
 * @par Purpose:
 *     Displays simple bitmap screens, like loading, no CD or startup screens.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2009 - 23 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "front_simple.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_video.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_filelst.h"

#include "config.h"
#include "kjm_input.h"
#include "scrcapt.h"
#include "gui_draw.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct RawBitmap bitmaps_640[] = {
  {"Empty Image",     640, 480, 8, FGrp_Main,     NULL,               NULL},
  {"Loading Image",   640, 480, 8, FGrp_FxData,   "loading_640.raw",  "loading_640.pal",},
  {"NoCD Image",      320, 200, 8, FGrp_StdData,  "nocd.raw",         "nocd.pal",},
  {"DK Legal Splash", 640, 480, 8, FGrp_StdData,  "legal.raw",        "legal.pal",},
  {"KeeperFX Splash", 640, 480, 8, FGrp_FxData,   "startup_fx.raw",   "startup_fx.pal",},
};

struct RawBitmap bitmaps_320[] = {
  {"Empty Image",     320, 200, 8, FGrp_Main,     NULL,               NULL},
  {"Loading Image",   320, 200, 8, FGrp_StdData,  "loading.raw",      "loading.pal",},
  {"NoCD Image",      320, 200, 8, FGrp_StdData,  "nocd.raw",         "nocd.pal",},
  {"DK Legal Splash", 640, 480, 8, FGrp_StdData,  "legal.raw",        "legal.pal",},
  {"KeeperFX Splash", 640, 480, 8, FGrp_FxData,   "startup_fx.raw",   "startup_fx.pal",},
};

struct ActiveBitmap astd_bmp;
struct ActiveBitmap nocd_bmp;
/******************************************************************************/
//DLLIMPORT void __cdecl _DK_display_loading_screen(void);
//DLLIMPORT void __cdecl _DK_wait_for_cd_to_be_available(void);
/******************************************************************************/
unsigned char palette_buf[PALETTE_SIZE];
/******************************************************************************/
/*
 * Copies the given RAW image at center of screen buffer.
 * @return Returns true on success.
 */
short copy_raw8_image_buffer(unsigned char *dst_buf,const int scanline,const int nlines,const int spx,const int spy,const unsigned char *src_buf,const int src_width,const int src_height,const int m)
{
  int w,h,i,k;
  unsigned char *dst;
  const unsigned char *src;
  w=0;
  h=0;
  // Clearing top of the canvas
  if (spy>0)
  {
    for (h=0; h<spy; h++)
    {
      dst = dst_buf + (h)*scanline;
      memset(dst,0,scanline);
    }
    // Clearing bottom of the canvas
    // (Note: it must be done before drawing, to make sure we won't overwrite last line)
    for (h=nlines-spy; h<nlines; h++)
    {
      dst = dst_buf + (h)*scanline;
      memset(dst,0,scanline);
    }
  }
  // Now drawing
  for (h=0; h<src_height; h++)
  {
    src = src_buf + h*src_width;
    for (k=0; k<m; k++)
    {
      if (spy+m*h+k<0) continue;
      if (spy+m*h+k>=nlines) break;
      dst = dst_buf + (spy+m*h+k)*scanline + spx;
      for (w=0; w<src_width; w++)
      {
        for (i=0;i<m;i++)
        {
            dst[m*w+i] = src[w];
        }
      }
    }
  }
  return true;
}

/*
 * Copies the given RAW image at center of screen buffer and swaps video
 * buffers to make the image visible.
 * @return Returns true on success.
 */
short copy_raw8_image_to_screen_center(const unsigned char *buf,const int img_width,const int img_height)
{
  TbScreenMode * mode = getActiveScreenMode();
  int w,h,m;
  int spx,spy;

  w=0;
  h=0;
  for (m=0;m<5;m++)
  {
    w+=img_width;
    h+=img_height;
    if (w > mode->width) break;
    if (h > mode->height) break;
  }
  // The image width can't be larger than video resolution
  if (m<1)
  {
    if (w > mode->width)
    {
      SYNCMSG("The %dx%d image does not fit on %dx%d screen, skipped.", img_width, img_height,
    		  mode->width, mode->height);
      return false;
    }
    m=1;
  }
  // Locking screen
  if (LbScreenLock() != Lb_SUCCESS)
    return false;
  // Starting point coords
  spx = (mode->width - m*img_width)>>1;
  spy = (mode->height - m*img_height)>>1;
  copy_raw8_image_buffer(lbDisplay.WScreen, mode->width, mode->height,
      spx,spy,buf,img_width,img_height,m);
  perform_any_screen_capturing();
  LbScreenUnlock();
  LbScreenSwap();
  return true;
}

TbBool show_rawimage_screen(unsigned char *raw,unsigned char *pal,int width,int height,TbClockMSec tmdelay)
{
      if (height > lbDisplay.PhysicalScreenHeight)
        height = lbDisplay.PhysicalScreenHeight;
    LbPaletteSet(pal);
    TbClockMSec end_time;
    end_time = LbTimerClock() + tmdelay;
    TbClockMSec tmdelta;
    tmdelta = tmdelay / 100;
    if (tmdelta > 100)
        tmdelta = 100;
    if (tmdelta < 10)
        tmdelta = 10;
    while (LbTimerClock() < end_time)
    {
        LbWindowsControl();
        copy_raw8_image_to_screen_center(raw, width, height);

        poll_sdl_events(false);
        if (is_key_pressed(KC_SPACE, KM_DONTCARE)
         || is_key_pressed(KC_ESCAPE, KM_DONTCARE)
         || is_key_pressed(KC_RETURN, KM_DONTCARE)
         || is_mouse_pressed_lrbutton())
        {
            clear_key_pressed(KC_SPACE);
            clear_key_pressed(KC_ESCAPE);
            clear_key_pressed(KC_RETURN);
            clear_mouse_pressed_lrbutton();
            break;
        }
        LbSleepFor(tmdelta);
    }
    return true;
}

/*
 * Resets bitmap screen structure to zero without freeing.
 * @return Returns true on success.
 */
short clear_bitmap_screen(struct ActiveBitmap *actv_bmp)
{
  LbMemorySet(actv_bmp, 0, sizeof(struct ActiveBitmap));
  return true;
}

/*
 * Frees memory used by bitmap screen and zeroes the data.
 * @return Returns true on success.
 */
short free_bitmap_screen(struct ActiveBitmap *actv_bmp)
{
  LbMemoryFree(actv_bmp->raw_data);
  LbMemoryFree(actv_bmp->pal_data);
  return clear_bitmap_screen(actv_bmp);
}

/*
 * Initializes bitmap screen. Loads all files and sets variables.
 * @return Returns true on success.
 */
short init_bitmap_screen(struct ActiveBitmap *actv_bmp,int stype)
{
	TbScreenMode * mode = getActiveScreenMode();
  struct RawBitmap *rbmp;
  unsigned char *buf;
  long ldsize;

  // Set startup parameters
  if (mode->width >= 640)
    rbmp = &bitmaps_640[stype];
  else
    rbmp = &bitmaps_320[stype];
  clear_bitmap_screen(actv_bmp);
  actv_bmp->name = rbmp->name;
  actv_bmp->width = rbmp->width;
  actv_bmp->height = rbmp->height;
  actv_bmp->bpp = rbmp->bpp;
  actv_bmp->start_tm = LbTimerClock();
  // Load PAL
  ldsize = PALETTE_SIZE;
  buf = load_data_file_to_buffer(&ldsize, rbmp->fgroup, rbmp->pal_fname);
  if (buf == NULL)
  {
    ERRORLOG("Couldn't load palette file for %s screen",rbmp->name);
    clear_bitmap_screen(actv_bmp);
    return false;
  }
  actv_bmp->pal_data = (unsigned char *)buf;
  // Load RAW
  ldsize = actv_bmp->width*actv_bmp->height*((actv_bmp->bpp >> 3) + ((actv_bmp->bpp%8)>0));
  buf = load_data_file_to_buffer(&ldsize, rbmp->fgroup, rbmp->raw_fname);
  if (buf == NULL)
  {
    ERRORLOG("Couldn't load raw bitmap file for %s screen",rbmp->name);
    LbMemoryFree(actv_bmp->pal_data);
    clear_bitmap_screen(actv_bmp);
    return false;
  }
  actv_bmp->raw_data = (TbPixel *)buf;
  return true;
}

/*
 * Draws active bitmap on screen.
 * @return Returns true on success.
 */
short draw_bitmap_screen(struct ActiveBitmap *actv_bmp)
{
  if (actv_bmp->pal_data == NULL)
    return false;
  LbPaletteSet(actv_bmp->pal_data);
  if (actv_bmp->raw_data == NULL)
    return false;
  copy_raw8_image_to_screen_center(actv_bmp->raw_data,actv_bmp->width,actv_bmp->height);
  return true;
}

/*
 * Draws active bitmap on screen, without setting palette.
 * @return Returns true on success.
 */
short redraw_bitmap_screen(struct ActiveBitmap *actv_bmp)
{
  if (actv_bmp->raw_data == NULL)
    return false;
  copy_raw8_image_to_screen_center(actv_bmp->raw_data,actv_bmp->width,actv_bmp->height);
  return true;
}

/*
 * Shows active bitmap screen for specific time.
 * @return Returns true on success.
 */
short show_bitmap_screen(struct ActiveBitmap *actv_bmp,TbClockMSec tmdelay)
{
  if (actv_bmp->pal_data == NULL)
    return false;
  if (actv_bmp->raw_data == NULL)
    return false;
  show_rawimage_screen(actv_bmp->raw_data,actv_bmp->pal_data,actv_bmp->width,actv_bmp->height,tmdelay);
  return true;
}

/*
 * Clears the screen and its palette.
 * @return Returns true on success.
 */
short draw_clear_screen(void)
{
  LbMemorySet(palette_buf, 0, PALETTE_SIZE);
  LbPaletteSet(palette_buf);
  LbScreenClear(0);
  LbScreenSwap();
  return true;
}

/*
 * Initializes bitmap screen on static struct. Loads all files and sets variables.
 * @return Returns true on success.
 */
short init_actv_bitmap_screen(int stype)
{
  return init_bitmap_screen(&astd_bmp,stype);
}

/*
 * Frees static active bitmap struct.
 */
short free_actv_bitmap_screen(void)
{
  return free_bitmap_screen(&astd_bmp);
}

/*
 * Draws active bitmap on screen using static struct.
 * @return Returns true on success.
 */
short draw_actv_bitmap_screen(void)
{
  return draw_bitmap_screen(&astd_bmp);
}

/*
 * Shows active bitmap screen from static struct for specific time.
 * @return Returns true on success.
 */
short show_actv_bitmap_screen(TbClockMSec tmdelay)
{
  return show_bitmap_screen(&astd_bmp,tmdelay);
}

/*
 * Displays the loading screen.
 * Will work properly only on any resolutions.
 * @return Returns true on success.
 */
short display_loading_screen(void)
{
  short done;
  draw_clear_screen();
  if (!wait_for_cd_to_be_available())
    return false;
  done = init_bitmap_screen(&astd_bmp,RBmp_WaitLoading);
  if (done)
  {
    redraw_bitmap_screen(&astd_bmp);
    LbPaletteStopOpenFade();
    ProperForcedFadePalette(astd_bmp.pal_data, 8, Lb_PALETTE_FADE_CLOSED);
  }
  if (done)
    free_bitmap_screen(&astd_bmp);
  return done;
}

TbBool wait_for_cd_to_be_available(void)
{
  char ffullpath[2048];
//  _DK_wait_for_cd_to_be_available(); return;
  short was_locked = LbScreenIsLocked();
  prepare_file_path_buf(ffullpath,FGrp_LoData,"dkwind00.dat");
  if ( LbFileExists(ffullpath) )
    return true;
  if ( was_locked )
    LbScreenUnlock();
  SYNCMSG("CD not found in drive, waiting");
  if (!init_bitmap_screen(&nocd_bmp,RBmp_WaitNoCD))
  {
      ERRORLOG("Unable to display CD wait monit");
      return false;
  }
  draw_bitmap_screen(&nocd_bmp);
  unsigned long counter;
  unsigned int i;
  counter=0;
  while ( !exit_keeper )
  {
      if ( LbFileExists(ffullpath) )
        break;
      for (i=0; i < 10; i++)
      {
        redraw_bitmap_screen(&nocd_bmp);
        while ( (!LbIsActive()) && (!exit_keeper) && (!quit_game) )
        {
          if (!LbWindowsControl())
          {
            exit_keeper = 1;
            break;
          }
        }
        if (is_key_pressed(KC_Q,KM_DONTCARE) || is_key_pressed(KC_X,KM_DONTCARE))
        {
          ERRORLOG("User requested quit, giving up");
          clear_key_pressed(KC_Q);
          clear_key_pressed(KC_X);
          exit_keeper = 1;
          break;
        }
        LbSleepFor(100);
      }
      // One 'counter' cycle lasts approx. 1 second.
      counter++;
      if (counter>300)
      {
          ERRORLOG("Wait time too long, giving up");
          exit_keeper = 1;
      }
  }
  SYNCMSG("Finished waiting for CD after %lu seconds",counter);
  free_bitmap_screen(&nocd_bmp);
  if ( was_locked )
    LbScreenLock();
  return (!exit_keeper);
}

TbBool display_centered_message(long showTime, char *text)
{
  TbBool finish;
  TbClockMSec tmEnd;
  long tmDelta;
  TbBool was_locked;
  tmEnd = LbTimerClock()+showTime;
  tmDelta = showTime/10;
  if (tmDelta < 10) tmDelta = 10;
  if (tmDelta > 250) tmDelta = 100;
  was_locked = LbScreenIsLocked();
  if ( was_locked )
    LbScreenUnlock();

  finish = false;
  while ( !finish )
  {
      // Redraw screen
      if (LbScreenLock() == Lb_SUCCESS)
      {
        draw_text_box(text);
        LbScreenUnlock();
      }
      LbScreenSwap();
      // Check if the window is active
      while (!LbIsActive())
      {
        if (!LbWindowsControl())
        {
          finish = true;
          break;
        }
        if (exit_keeper)
        {
          finish = true;
          break;
        }
      }
      // Process inputs
      update_mouse();
      update_key_modifiers();
      if (is_key_pressed(KC_Q,KM_DONTCARE) || is_key_pressed(KC_X,KM_DONTCARE))
      {
        ERRORLOG("User requested quit, giving up");
        clear_key_pressed(KC_Q);
        clear_key_pressed(KC_X);
        exit_keeper = 1;
        finish = true;
        break;
      }
      if (is_key_pressed(KC_ESCAPE,KM_DONTCARE) || is_key_pressed(KC_RETURN,KM_DONTCARE) || is_key_pressed(KC_SPACE,KM_DONTCARE))
      {
        clear_key_pressed(KC_ESCAPE);
        clear_key_pressed(KC_RETURN);
        clear_key_pressed(KC_SPACE);
        finish = true;
        break;
      }
      if (left_button_clicked || right_button_clicked)
      {
        left_button_clicked = 0;
        right_button_clicked = 0;
        finish = true;
        break;
      }
      // Make delay and check if we should end
      LbSleepFor(tmDelta);
      if (LbTimerClock() > tmEnd)
      {
        finish = true;
      }
  }
  if ( was_locked )
    LbScreenLock();
  return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
