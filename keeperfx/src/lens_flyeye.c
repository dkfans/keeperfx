/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_flyeye.c
 *     lens_flyeye support functions.
 * @par Purpose:
 *     Functions to lens_flyeye.
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
#include "lens_flyeye.h"

#include "globals.h"
#include "bflib_basics.h"
#include "engine_render.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_flyeye_setup(long width, long height);
DLLIMPORT void _DK_flyeye_blitsec(unsigned char *srcbuf, unsigned char *dstbuf, long srcwidth, long dstwidth, long n, long height);

/******************************************************************************/
static unsigned char *lens_Source;
static long lens_SourcePitch;
static unsigned char *lens_Screen;
static long lens_ScreenPitch;
/******************************************************************************/
void flyeye_setup(long width, long height)
{
  _DK_flyeye_setup(width, height);
}

/**
 * Draws displacement on image line, based on given CScan data.
 * @param scan
 * @param h
 */
void BlitScan(struct CScan *scan, long h)
{
  unsigned char *dst;
  unsigned char *src;
  long w,end_w;
  long i;
  for(i=0; i < scan->strips_num; i++)
  {
      w = scan->strip_len[i];
      if (i == scan->strips_num - 1)
          end_w = ScrWidth;
      else
          end_w = scan->strip_len[i+1];
      dst = &lens_Screen[h * lens_ScreenPitch + w];
      src = &lens_Source[(scan->strip_y[i] + h) * lens_SourcePitch + (scan->strip_x[i] + w)];
      //JUSTLOG("DST(%d,%d) SRC(%d,%d) LEN %d",(dst-lens_Screen)%lens_ScreenPitch,(dst-lens_Screen)/lens_ScreenPitch,(src-lens_Source)%lens_SourcePitch,(src-lens_Source)/lens_SourcePitch,end_w - w);
      memcpy(dst, src, end_w - w);
  }
}

/** Draws displacement on source image, using ScanBuffer for shift data.
 *
 * @param srcbuf Source image buffer.
 * @param srcpitch Source image line pitch (scanline length).
 * @param dstbuf Destination buffer to store transformed image.
 * @param dstpitch Destination buffer line pitch (scanline length).
 * @param start_h Starting line of the displacement.
 * @param end_h Line beyond end of the displacement.
 */
void flyeye_blitsec(unsigned char *srcbuf, long srcpitch, unsigned char *dstbuf, long dstpitch, long start_h, long end_h)
{
    //_DK_flyeye_blitsec(srcbuf, dstbuf, srcpitch, dstpitch, start_h, end_h); return;
    long h;
    lens_Source = srcbuf;
    lens_SourcePitch = srcpitch;
    lens_Screen = dstbuf;
    lens_ScreenPitch = dstpitch;
    // Draw lines
    for (h=start_h; h < end_h; h++)
    {
        BlitScan(&ScanBuffer[h], h);
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
