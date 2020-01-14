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
#include <cmath>
#include "engine_render.h"
#include "lens_api.h"
#include "bflib_vidraw.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
class CHex {
 public:
    CHex(long, long);
    void BlitHex(void);
    void DrawOutline(void);
    static void AddScan(struct CScan *scan, long a2, long a3, long a4, long a5);
    static void BlitScan(struct CScan *scan, long h);
 private:
    long arrA[6];
    long arrB[6];
    long source_strip_w;
    long source_strip_h;
};
/******************************************************************************/
static unsigned char *lens_Source;
static long lens_SourcePitch;
static unsigned char *lens_Screen;
static long lens_ScreenPitch;
struct CScan *ScanBuffer;
/******************************************************************************/
CHex::CHex(long width, long height)
{
    long maxsize;
    long mwidth;
    long mheight;
    long double ldpar1;
    long double ldpar2;
    long double varA;
    long double varB;
    long double len;
    long i;
    if (ScrHeight > ScrWidth)
        maxsize = ScrHeight;
    else
        maxsize = ScrWidth;
    mwidth = 50 * width;
    mheight = 60 * height;
    ldpar1 = (long double)maxsize * 0.0175L;
    ldpar2 = (long double)maxsize * 0.0025L;
    if ((width & 1) != 0)
        mheight += 30;
    this->arrA[0] = mwidth - 35;
    this->arrB[0] = mheight + 30;
    this->arrA[1] = mwidth - 15;
    this->arrB[1] = mheight;
    this->arrA[2] = mwidth + 15;
    this->arrB[2] = mheight;
    this->arrA[3] = mwidth + 35;
    this->arrA[4] = mwidth + 15;
    this->arrB[4] = mheight + 60;
    this->arrB[3] = mheight + 30;
    this->arrA[5] = mwidth - 15;
    this->arrB[5] = mheight + 60;
    for (i=0; i < 6; i++)
    {
      varA = this->arrA[i];
      varB = this->arrB[i];
      len = sqrt(varA * varA + varB * varB) * 0.0025L + 1.0L;
      this->arrA[i] = (signed long long)((long double)ScrCenterX + varA / len * ldpar2);
      this->arrB[i] = (signed long long)((long double)ScrCenterY + varB / len * ldpar2);
    }
    this->source_strip_w = ((long double)-width * ldpar1);
    this->source_strip_h = (ldpar1 * (long double)-height);
}

void CHex::DrawOutline(void)
{
    long i;
    long k;
    for (i=0; i < 6; i++)
    {
        k = (i + 1) % 6;
        LbDrawLine(this->arrA[i], this->arrB[i], this->arrA[k], this->arrB[k], 0xFFu);
    }
}

void CHex::AddScan(struct CScan *scan, long strip_len, long len_limit, long nstrip_w, long nstrip_h)
{
    long stlen;
    long clen;
    long i;
    long n;

    stlen = strip_len;
    if (strip_len < 0)
      stlen = 0;
    if ( (stlen >= ScrWidth) || (stlen >= len_limit) )
        return;
    if (scan->strips_num >= CSCAN_STRIPS)
        return;
    n = 0;
    // Find index for new strip
    for (i = 0; ; i++)
    {
      if (i >= scan->strips_num)
      {
          // If it's last, just fill it and finish
          scan->strip_len[scan->strips_num] = stlen;
          scan->strip_w[scan->strips_num] = nstrip_w;
          scan->strip_h[scan->strips_num] = nstrip_h;
          scan->strips_num++;
          return;
      }
      clen = scan->strip_len[n];
      if (stlen == clen)
      {
        scan->strip_w[i] = nstrip_w;
        scan->strip_h[i] = nstrip_h;
        return;
      }
      if (clen > stlen)
        break;
      n++;
    }
    // Move elements to make space for new scan
    n = scan->strips_num;
    while (n > i)
    {
        scan->strip_len[n] = scan->strip_len[n-1];
        scan->strip_w[n] = scan->strip_w[n-1];
        scan->strip_h[n] = scan->strip_h[n-1];
        n--;
    }
    // Insert scan data
    scan->strip_len[i] = stlen;
    scan->strip_w[i] = nstrip_w;
    scan->strip_h[i] = nstrip_h;
    scan->strips_num++;
}

void CHex::BlitHex(void)
{
    int min_idx;
    int counter1;
    int counter2;
    int first_idx;
    int last_idx;
    int deltaV1;
    int deltaV2;
    int posV1;
    int posV2;
    long scan_num;
    int i;
    int n;
    min_idx = 0;
    for (i = 1; i < 6; i++)
    {
        if (this->arrB[i] < this->arrB[min_idx])
            min_idx = i;
    }
    scan_num = this->arrB[min_idx];
    first_idx = (min_idx + 1) % 6;
    last_idx = (min_idx + 5) % 6;
    deltaV1=0;deltaV2=0;
    posV1=0;posV2=0;
    counter2 = 0;
    counter1 = 0;
    while ( 1 )
    {
        i = first_idx;
        while (counter1 == 0)
        {
            first_idx = (first_idx + 5) % 6;
            if (first_idx == i)
              return;
            posV1 = this->arrA[first_idx] << 16;
            n = (first_idx + 5) % 6;
            counter1 = this->arrB[n] - this->arrB[first_idx];
            if (counter1 > 0) {
                deltaV1 = ((this->arrA[n] << 16) - posV1) / counter1;
            }
        }
        i = last_idx;
        while (counter2 == 0)
        {
            last_idx = (last_idx + 1) % 6;
            if (last_idx == i)
              return;
            n = (last_idx + 1) % 6;
            counter2 = this->arrB[n] - this->arrB[last_idx];
            posV2 = this->arrA[last_idx] << 16;
            if (counter2 > 0) {
                deltaV2 = ((this->arrA[n] << 16) - posV2) / counter2;
            }
        }
        if ( (counter1 <= 0) || (counter2 <= 0) )
          return;
        if ( (scan_num >= 0) && (scan_num < ScrHeight) )
        {
            SYNCDBG(19,"ScanBuffer %d pos %d,%d from %d,%d",scan_num, posV1, posV2, (int)this->source_strip_w, (int)this->source_strip_h);
            AddScan(&ScanBuffer[scan_num], posV1 >> 16, posV2 >> 16, this->source_strip_w, this->source_strip_h);
        }
        counter1--;
        counter2--;
        posV1 += deltaV1;
        posV2 += deltaV2;
        scan_num++;
    }
}

void flyeye_setup(long width, long height)
{
    long i;
    long x;
    long y;
    ScrCenterX = (width >> 1);
    ScrWidth = width;
    ScrCenterY = (height >> 1);
    ScrHeight = height;
    ScanBuffer = (struct CScan *)eye_lens_memory;
    for (i=0; i < ScrHeight; i++)
    {
        ScanBuffer[i].strips_num = 0;
    }
    for (y=-12; y <= 12; y++) {
        for (x=-12; x <= 12; x++) {
            class CHex chx(x, y);
            chx.BlitHex();
        }
    }
}

/**
 * Draws displacement on image line, based on given CScan data.
 * @param scan
 * @param h
 */
void CHex::BlitScan(struct CScan *scan, long h)
{
  unsigned char *dst;
  unsigned char *src;
  long shift_w;
  long shift_h;
  long w;
  long end_w;
  long i;
  SYNCDBG(16,"Starting line %d",h);
  for(i=0; i < scan->strips_num; i++)
  {
      w = scan->strip_len[i];
      if (i == scan->strips_num - 1)
          end_w = ScrWidth;
      else
          end_w = scan->strip_len[i+1];
      shift_w = (long)scan->strip_w[i] + w;
      shift_h = (long)scan->strip_h[i] + h;
      dst = &lens_Screen[h * lens_ScreenPitch + w];
      src = &lens_Source[shift_h * lens_SourcePitch + shift_w];
      //some debug code
      //if ((shift_w > lens_ScreenPitch) || (shift_h > 1022) || (end_w - w < 0) || (shift_w + end_w - w > lens_ScreenPitch)) {
      //    ERRORLOG("POS(%d,%d) DST(%d,%d) SRC(%d,%d) LEN %d",w,h,(dst-lens_Screen)%lens_ScreenPitch,(dst-lens_Screen)/lens_ScreenPitch,(src-lens_Source)%lens_SourcePitch,(src-lens_Source)/lens_SourcePitch,end_w - w);
      //    break;
      //}
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
    long h;
    SYNCDBG(16,"Starting");
    lens_Source = srcbuf;
    lens_SourcePitch = srcpitch;
    lens_Screen = dstbuf;
    lens_ScreenPitch = dstpitch;
    // Draw lines
    for (h=start_h; h < end_h; h++)
    {
        CHex::BlitScan(&ScanBuffer[h], h);
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
