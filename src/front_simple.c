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
#include "bflib_inputctrl.h"
#include "bflib_datetm.h"
#include "bflib_video.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_filelst.h"

#include "config.h"
#include "kjm_input.h"
#include "scrcapt.h"
#include "gui_draw.h"
#include "vidfade.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef SPRITE_FORMAT_V2
struct RawBitmap bitmaps_1280[] = {
  {"Empty Image",    1280, 960, 8, FGrp_Main,     NULL,               NULL},
  {"Loading Image",  1280, 960, 8, FGrp_StdData,  "loading-128.raw",  "loading-128.pal",},
  {"NoCD Image",      320, 200, 8, FGrp_StdData,  "nocd-32.raw",      "nocd-32.pal",},
  {"DK Legal Splash",1280, 960, 8, FGrp_StdData,  "legal-128.raw",    "legal-128.pal",},
  {"KeeperFX Splash",1280, 960, 8, FGrp_StdData,  "startfx-128.raw",  "startfx-128.pal",},
};

struct RawBitmap bitmaps_640[] = {
  {"Empty Image",     640, 480, 8, FGrp_Main,     NULL,               NULL},
  {"Loading Image",   640, 480, 8, FGrp_StdData,  "loading-64.raw",   "loading-64.pal",},
  {"NoCD Image",      320, 200, 8, FGrp_StdData,  "nocd-32.raw",      "nocd-32.pal",},
  {"DK Legal Splash", 640, 480, 8, FGrp_StdData,  "legal-64.raw",     "legal-64.pal",},
  {"KeeperFX Splash", 640, 480, 8, FGrp_StdData,  "startfx-64.raw",   "startfx-64.pal",},
};

struct RawBitmap bitmaps_320[] = {
  {"Empty Image",     320, 200, 8, FGrp_Main,     NULL,               NULL},
  {"Loading Image",   320, 200, 8, FGrp_StdData,  "loading-32.raw",   "loading-32.pal",},
  {"NoCD Image",      320, 200, 8, FGrp_StdData,  "nocd-32.raw",      "nocd-32.pal",},
  {"DK Legal Splash", 320, 200, 8, FGrp_StdData,  "legal-32.raw",     "legal-32.pal",},
  {"KeeperFX Splash", 320, 200, 8, FGrp_StdData,  "startfx-32.raw",   "startfx-32.pal",},
};
#else
struct RawBitmap bitmaps_1280[] = {
  {"Empty Image",     640, 480, 8, FGrp_Main,     NULL,               NULL},
  {"Loading Image",   640, 480, 8, FGrp_StdData,  "loading64.raw",    "loading64.pal",},
  {"NoCD Image",      320, 200, 8, FGrp_StdData,  "nocd.raw",         "nocd.pal",},
  {"DK Legal Splash", 640, 480, 8, FGrp_StdData,  "legal64.raw",      "legal64.pal",},
  {"KeeperFX Splash", 640, 480, 8, FGrp_StdData,  "startfx64.raw",    "startfx64.pal",},
};

struct RawBitmap bitmaps_640[] = {
  {"Empty Image",     640, 480, 8, FGrp_Main,     NULL,               NULL},
  {"Loading Image",   640, 480, 8, FGrp_StdData,  "loading64.raw",    "loading64.pal",},
  {"NoCD Image",      320, 200, 8, FGrp_StdData,  "nocd.raw",         "nocd.pal",},
  {"DK Legal Splash", 640, 480, 8, FGrp_StdData,  "legal64.raw",      "legal64.pal",},
  {"KeeperFX Splash", 640, 480, 8, FGrp_StdData,  "startfx64.raw",    "startfx64.pal",},
};

struct RawBitmap bitmaps_320[] = {
  {"Empty Image",     320, 200, 8, FGrp_Main,     NULL,               NULL},
  {"Loading Image",   320, 200, 8, FGrp_StdData,  "loading32.raw",    "loading32.pal",},
  {"NoCD Image",      320, 200, 8, FGrp_StdData,  "nocd.raw",         "nocd.pal",},
  {"DK Legal Splash", 320, 200, 8, FGrp_StdData,  "legal32.raw",      "legal32.pal",},
  {"KeeperFX Splash", 320, 200, 8, FGrp_StdData,  "startfx32.raw",    "startfx32.pal",},
};
#endif
struct ActiveBitmap astd_bmp;
struct ActiveBitmap nocd_bmp;
/******************************************************************************/
unsigned char palette_buf[PALETTE_SIZE];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/** Copies the given RAW image at given point of screen buffer.
 *
 * @param dst_buf Destination screen buffer.
 * @param scanline Amount of bytes making up one line in screen buffer.
 * @param nlines Amount of lines in screen buffer.
 * @param dst_width Destination image width.
 * @param dst_height Destination image height.
 * @param spw Starting position in screen buffer.
 * @param sph Starting position in screen buffer.
 * @param src_buf Source image buffer.
 * @param src_width Source image width.
 * @param src_height Source image height.
 *     Factor of 2 would mean every pixel is repeated in both dimensions and drawn 2*2 times.
 * @return Gives true on success.
 */
TbBool copy_raw8_image_buffer(unsigned char *dst_buf,const int scanline,const int nlines,const int dst_width,const int dst_height,
    const int spw,const int sph,const unsigned char *src_buf,const int src_width,const int src_height)
{
    unsigned char* dst;
    SYNCDBG(18, "Starting; screen buf %d,%d screen size %d,%d dst pos %d,%d src %d,%d", (int)scanline, (int)nlines, (int)dst_width, (int)dst_height, (int)spw, (int)sph, (int)src_width, (int)src_height);
    // Source pixel coords
    int sw = 0;
    int sh = 0;
    // Clearing top of the canvas
    for (sh = 0; sh < sph; sh++)
    {
        dst = dst_buf + (sh)*scanline;
        LbMemorySet(dst, 0, scanline);
  }
  // Clearing bottom of the canvas
  // (Note: it must be done before drawing, to make sure we won't overwrite last line)
  for (sh=sph+dst_height; sh<nlines; sh++)
  {
      dst = dst_buf + (sh)*scanline;
      LbMemorySet(dst, 0, scanline);
  }
  // Now drawing
  int dhstart = sph;
  for (sh=0; sh<src_height; sh++)
  {
      int dhend = sph + (dst_height * (sh + 1) / src_height);
      const unsigned char* src = src_buf + sh * src_width;
      // make for(k=0;k<dhend-dhstart;k++) but restrict k to draw area
      int mhmin = max(0, -dhstart);
      int mhmax = min(dhend - dhstart, nlines - dhstart);
      for (int k = mhmin; k < mhmax; k++)
      {
          dst = dst_buf + (dhstart+k)*scanline;
          int dwstart = spw;
          if (dwstart > 0) {
              LbMemorySet(dst, 0, dwstart);
          }
          for (sw=0; sw<src_width; sw++)
          {
              int dwend = spw + (dst_width * (sw + 1) / src_width);
              // make for(i=0;i<dwend-dwstart;i++) but restrict i to draw area
              int mwmin = max(0, -dwstart);
              int mwmax = min(dwend - dwstart, scanline - dwstart);
              for (int i = mwmin; i < mwmax; i++)
              {
                  dst[dwstart+i] = src[sw];
              }
              dwstart = dwend;
          }
          if (dwstart < scanline) {
              LbMemorySet(dst+dwstart, 0, scanline-dwstart);
          }
      }
      dhstart = dhend;
  }
  return true;
}

/**
 * Copies the given RAW image at center of screen buffer and swaps video
 * buffers to make the image visible.
 * @return Returns true on success.
 */
TbBool copy_raw8_image_to_screen_center(const unsigned char *buf,const int img_width,const int img_height)
{
    // Only 8bpp supported for now
    if (LbGraphicsScreenBPP() != 8)
        return false;
    // Compute scaling ratio
    int units_per_px;
    {
        int width = LbScreenWidth();
        int height = LbScreenHeight();
        units_per_px = (width>height?width:height)/((img_width>img_height?img_width:img_height)/16);
    }
    SYNCDBG(18,"Starting; src %d,%d scale %d",(int)img_width,(int)img_height,(int)units_per_px);
    // Locking screen
    if (LbScreenLock() != Lb_SUCCESS)
      return false;
    // Starting point coords
    int spx = (LbScreenWidth() - img_width * units_per_px / 16) >> 1;
    int spy = (LbScreenHeight() - img_height * units_per_px / 16) >> 1;
    copy_raw8_image_buffer(lbDisplay.WScreen,LbGraphicsScreenWidth(),LbGraphicsScreenHeight(),
        img_width*units_per_px/16,img_height*units_per_px/16,spx,spy,buf,img_width,img_height);
    perform_any_screen_capturing();
    LbScreenUnlock();
    LbScreenSwap();
    return true;
}

TbBool show_rawimage_screen(unsigned char *raw,unsigned char *pal,int width,int height,TbClockMSec tmdelay)
{
    LbPaletteSet(pal);
    TbClockMSec end_time = LbTimerClock() + tmdelay;
    TbClockMSec tmdelta = tmdelay / 100;
    if (tmdelta > 100)
        tmdelta = 100;
    if (tmdelta < 10)
        tmdelta = 10;
    while (LbTimerClock() < end_time)
    {
        LbWindowsControl();
        copy_raw8_image_to_screen_center(raw, width, height);
        if (is_key_pressed(KC_SPACE, KMod_DONTCARE)
         || is_key_pressed(KC_ESCAPE, KMod_DONTCARE)
         || is_key_pressed(KC_RETURN, KMod_DONTCARE)
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

/**
 * Resets bitmap screen structure to zero without freeing.
 * @return Returns true on success.
 */
short clear_bitmap_screen(struct ActiveBitmap *actv_bmp)
{
  LbMemorySet(actv_bmp, 0, sizeof(struct ActiveBitmap));
  return true;
}

/**
 * Frees memory used by bitmap screen and zeroes the data.
 * @return Returns true on success.
 */
short free_bitmap_screen(struct ActiveBitmap *actv_bmp)
{
  LbMemoryFree(actv_bmp->raw_data);
  LbMemoryFree(actv_bmp->pal_data);
  return clear_bitmap_screen(actv_bmp);
}

/**
 * Initializes bitmap screen. Loads all files and sets variables.
 * @return Returns true on success.
 */
TbBool init_bitmap_screen(struct ActiveBitmap *actv_bmp,int stype)
{
  struct RawBitmap *rbmp;
  // Set startup parameters
  if (LbGraphicsScreenWidth() >= 1280)
    rbmp = &bitmaps_1280[stype];
  else
  if (LbGraphicsScreenWidth() >= 640)
    rbmp = &bitmaps_640[stype];
  else
    rbmp = &bitmaps_320[stype];
  clear_bitmap_screen(actv_bmp);
  actv_bmp->name = rbmp->name;
  actv_bmp->width = rbmp->width;
  actv_bmp->height = rbmp->height;
  actv_bmp->bpp = rbmp->bpp;
  actv_bmp->start_tm = LbTimerClock();
  SYNCDBG(18,"Starting; src %d,%d bpp %d",(int)actv_bmp->width,(int)actv_bmp->height,(int)actv_bmp->bpp);
  // Load PAL
  long ldsize = PALETTE_SIZE;
  unsigned char* buf = load_data_file_to_buffer(&ldsize, rbmp->fgroup, rbmp->pal_fname);
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

/** Draws active bitmap on screen.
 *
 * @param actv_bmp The active bitmap structure to be drawn.
 * @return Returns true on success.
 */
TbBool draw_bitmap_screen(struct ActiveBitmap *actv_bmp)
{
    if (actv_bmp->pal_data == NULL)
      return false;
    LbPaletteSet(actv_bmp->pal_data);
    if (actv_bmp->raw_data == NULL)
      return false;
    copy_raw8_image_to_screen_center(actv_bmp->raw_data,actv_bmp->width,actv_bmp->height);
    return true;
}

/** Draws active bitmap on screen, without setting palette.
 *
 * @param actv_bmp The active bitmap structure to be re-drawn.
 * @return Returns true on success.
 */
short redraw_bitmap_screen(struct ActiveBitmap *actv_bmp)
{
    if (actv_bmp->raw_data == NULL)
      return false;
    copy_raw8_image_to_screen_center(actv_bmp->raw_data,actv_bmp->width,actv_bmp->height);
    return true;
}

/**
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

/**
 * Clears the screen and its palette.
 * @return Returns true on success.
 */
TbBool draw_clear_screen(void)
{
    LbPaletteDataFillBlack(palette_buf);
    LbPaletteSet(palette_buf);
    LbScreenClear(0);
    LbScreenSwap();
    return true;
}

/** Initializes bitmap screen on static struct.
 *  Loads all files and sets variables.
 *
 * @param stype Bitmap screen type selector.
 * @return Returns true on success.
 */
TbBool init_actv_bitmap_screen(int stype)
{
    return init_bitmap_screen(&astd_bmp,stype);
}

/**
 * Frees static active bitmap struct.
 */
TbBool free_actv_bitmap_screen(void)
{
  return free_bitmap_screen(&astd_bmp);
}

/**
 * Draws active bitmap on screen using static struct.
 * @return Returns true on success.
 */
TbBool draw_actv_bitmap_screen(void)
{
  return draw_bitmap_screen(&astd_bmp);
}

/**
 * Shows active bitmap screen from static struct for specific time.
 * @return Returns true on success.
 */
TbBool show_actv_bitmap_screen(TbClockMSec tmdelay)
{
  return show_bitmap_screen(&astd_bmp,tmdelay);
}

/**
 * Displays the loading screen.
 * Will work properly only on any resolutions.
 * @return Returns true on success.
 */
TbBool display_loading_screen(void)
{
    draw_clear_screen();
    if (!wait_for_cd_to_be_available())
      return false;
    TbBool done = init_bitmap_screen(&astd_bmp, RBmp_WaitLoading);
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
  short was_locked = LbScreenIsLocked();
  prepare_file_path_buf(ffullpath,FGrp_LoData,"lndflag_ens.dat");
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
  unsigned long counter = 0;
  while ( !exit_keeper )
  {
      if ( LbFileExists(ffullpath) )
        break;
      for (unsigned int i = 0; i < 10; i++)
      {
        redraw_bitmap_screen(&nocd_bmp);
        do
        {
            if (!LbWindowsControl())
                exit_keeper = 1;
            if ((exit_keeper) || (quit_game))
              break;
        } while (!LbIsActive());
        if (is_key_pressed(KC_Q,KMod_DONTCARE) || is_key_pressed(KC_X,KMod_DONTCARE))
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

/** Displays centered message; for logging errors.
 *  Deprecated - will be removed sooner or later; there are now menus for displaying such messages, both in menu and in game.
 *
 * @param showTime
 * @param text
 * @return
 */
TbBool display_centered_message(long showTime, char *text)
{
    TbClockMSec tmEnd = LbTimerClock() + showTime;
    long tmDelta = showTime / 10;
    if (tmDelta < 10)
        tmDelta = 10;
    if (tmDelta > 250)
        tmDelta = 100;
    TbBool was_locked = LbScreenIsLocked();
    if (was_locked)
        LbScreenUnlock();
    TbBool finish = false;
    while (!finish)
    {
        // Redraw screen
        if (LbScreenLock() == Lb_SUCCESS)
        {
            draw_text_box(text);
            LbScreenUnlock();
        }
        LbScreenSwap();
        // Check if the window is active
        do
        {
            if (!LbWindowsControl())
                exit_keeper = 1;
            if ((exit_keeper) || (quit_game))
            {
                finish = true;
                break;
            }
        } while (!LbIsActive());
        // Process inputs
        update_mouse();
        update_key_modifiers();
        if (is_key_pressed(KC_Q, KMod_DONTCARE) || is_key_pressed(KC_X, KMod_DONTCARE))
        {
            ERRORLOG("User requested quit, giving up");
            clear_key_pressed(KC_Q);
            clear_key_pressed(KC_X);
            exit_keeper = 1;
            finish = true;
            break;
        }
        if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE) || is_key_pressed(KC_RETURN, KMod_DONTCARE) || is_key_pressed(KC_SPACE, KMod_DONTCARE))
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
