/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_fmvids.c
 *     Full Motion Videos (Smacker,FLIC) decode & play library.
 * @par Purpose:
 *     Routines to create and decode videos.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     27 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_fmvids.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_sndlib.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_inputctrl.h"
#include "bflib_fileio.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Constants and defines
#define FLI_PREFIX 0xF100u
#define FLI_COLOR256 4
#define FLI_SS2      7
#define FLI_COLOR   11
#define FLI_LC      12
#define FLI_BLACK   13
#define FLI_BRUN    15
#define FLI_COPY    16
#define FLI_PSTAMP  18

/******************************************************************************/
// Global variables
SmackDrawCallback smack_draw_callback = NULL;
unsigned char smk_palette[768];
/******************************************************************************/
void copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags);
/******************************************************************************/
// Functions
typedef char (WINAPI *FARPROCP_C)(void *);
typedef unsigned long (WINAPI *FARPROCP_U)(void *);
typedef void (WINAPI *FARPROCP_V)(void *);
typedef void (WINAPI *FARPROCPU_V)(void *,unsigned long);
typedef void (WINAPI *FARPROCU_V)(unsigned long);
typedef struct SmackTag * (WINAPI *FARSMACKOPEN)(const char *,unsigned int,int);
typedef void (WINAPI *FARSMACKSUMMARY)(struct SmackTag *,struct SmackSumTag *);
typedef void (WINAPI *FARSMACKTOBUF)(struct SmackTag *,unsigned long,unsigned long,
    unsigned long,unsigned long,const void *,unsigned long);

char SmackSoundUseMSS(void* dig_driver)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackSoundUseMSS@4");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackSoundUseMSS function; skipped."); return 0; }
    return ((FARPROCP_C)(void *)proc)(dig_driver);
}

struct SmackTag *SmackOpen(const char *name,unsigned int flags,int extrabuf)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackOpen@12");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackOpen function; skipped."); return 0; }
    return ((FARSMACKOPEN)(void *)proc)(name,flags,extrabuf);
}

void SmackSummary(struct SmackTag *smk,struct SmackSumTag *sum)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackSummary@8");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackSummary function; skipped."); return; }
    ((FARSMACKSUMMARY)(void *)proc)(smk,sum);
}

unsigned long SmackWait(struct SmackTag *smk)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackWait@4");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackWait function; skipped."); return 0; }
    return ((FARPROCP_U)proc)(smk);
}

void SmackClose(struct SmackTag *smk)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackClose@4");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackClose function; skipped."); return; }
    ((FARPROCP_V)(void *)proc)(smk);
}

unsigned long SmackDoFrame(struct SmackTag *smk)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackDoFrame@4");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackDoFrame function; skipped."); return 0; }
    return ((FARPROCP_U)proc)(smk);
}

void SmackNextFrame(struct SmackTag *smk)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackNextFrame@4");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackNextFrame function; skipped."); return; }
    ((FARPROCP_V)(void *)proc)(smk);
}

void SmackToBuffer(struct SmackTag *smk,unsigned long left,unsigned long top,
    unsigned long Pitch,unsigned long destheight,const void *buf,unsigned long Flags)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackToBuffer@28");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackToBuffer function; skipped."); return; }
    ((FARSMACKTOBUF)(void *)proc)(smk,left,top,Pitch,destheight,buf,Flags);
}

void SmackGoto(struct SmackTag *smk,unsigned long frame)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackGoto@8");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackGoto function; skipped."); return; }
    ((FARPROCPU_V)(void *)proc)(smk,frame);
}

void SmackSimulate(unsigned long sim)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackSimulate@4");
    if (proc==NULL)
    { ERRORLOG("Can't get address of SmackSimulate function; skipped."); return; }
    ((FARPROCU_V)(void *)proc)(sim);
}

void copy_to_screen_px_ar_scale(unsigned char *src_buf, unsigned char *dst_buf, int src_width, int src_height, int flags)
{
    // Compute scaling ratio -> Output co-ordinates and output size
    int scanline = lbDisplay.GraphicsScreenWidth;
    int nlines = lbDisplay.GraphicsScreenHeight;
    int spw = 0;
    int sph = 0;
    int dst_width = 0;
    int dst_height = 0;

    if ((flags & SMK_FullscreenStretch) != 0 && !((flags & SMK_FullscreenFit) != 0)) // Use full screen resolution and fill the whole canvas by "stretching"
    {
        dst_width = scanline;
        dst_height = nlines;
    }
    else // Otherwise, calculate the correct output size
    {
        int in_width = src_width;
        int in_height = src_height;
        float units_per_px = 0;
        float relative_ar_difference = (in_width * 1.0 / in_height * 1.0) / (scanline * 1.0 / nlines * 1.0); // relative aspect ratio difference between the source frame and destination frame
        float comparison_ratio = 1; // when keeping aspect ratio, instead of stretching, this is inverted depending on if we want to crop or fit
        if (((flags & SMK_FullscreenStretch) != 0) && ((flags & SMK_FullscreenFit) != 0)) // stretch source from 320x200(16:10) to 320x240 (4:3) (i.e. vertical x 1.2) - "preserve *original* aspect ratio mode"
        {
            if (src_width == 320 && src_height == 200) {// make sure the source is 320x200
                in_height = (int)(in_height * 1.2);
            }
        }
        
        if ((flags & SMK_FullscreenCrop) != 0 && !((flags & SMK_FullscreenFit) != 0)) // fill screen (will crop)
        {
            comparison_ratio = relative_ar_difference;
        }
        else // fit to full screen, preserve aspect ratio (pillar/letter boxed)
        {
            comparison_ratio = 1.0 / relative_ar_difference;
        }
        if (comparison_ratio <= 1.0) // take either the destination width or height, depending on if the destination is wider or narrower than the source (same aspect ratio is treated the same as wider), and also if we want to crop or fit
        {
            units_per_px = (scanline>nlines?scanline:nlines)/((in_width>in_height?in_width:in_height)/16.0);
        }
        else
        {
            units_per_px = (scanline>nlines?nlines:scanline)/((in_width>in_height?in_height:in_width)/16.0);
        }
        if ((flags & SMK_FullscreenCrop) != 0 && ((flags & SMK_FullscreenFit) != 0)) // Find the highest integer scale possible
        {
            if ((flags & SMK_FullscreenStretch) != 0) { //4:3 stretch mode (crop off to the nearest 5x/6x scale
                if (src_width == 320 && src_height == 200) {// make sure the source is 320x200
                    units_per_px = (max(5, (int)(units_per_px / 16.0 / 5.0) * 5) * 16); // make sure the multiple is integer divisible by 5. Use 5x as a minimum, otherwise there will be no video (resolutions smaller than 1600x1200 will have a cropped image from a buffer of that size).
                }
            }
            units_per_px = ((int)(units_per_px / 16.0) * 16); // scale to the nearest integer multiple of the source resolution.
        }
        // Starting point coords and width for the destination buffer (based on desired aspect ratio)
        spw = (int)((scanline - in_width * units_per_px / 16.0) / 2.0);
        sph = (int)((nlines - in_height * units_per_px / 16.0) / 2.0);
        dst_width = (int)(in_width * units_per_px / 16.0);
        dst_height = (int)(in_height * units_per_px / 16.0);
    }

    unsigned char* dst;
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

}

void copy_to_screen_pxquad(unsigned char *srcbuf, unsigned char *dstbuf, long width, long dst_shift)
{
    unsigned long i;
    unsigned long k;
    unsigned long n;
    unsigned long w;
    unsigned long s;
    unsigned long *dst;
    unsigned long *src;
    src = (unsigned long *)srcbuf;
    dst = (unsigned long *)dstbuf;
    w = ((unsigned long)width) >> 2;
    s = dst_shift >> 2;
    do
    {
        i = (*src) & 0xFF;
        k = (*src >> 8) & 0xFF;
        n = (k << 24) + (k << 16) + (i << 8) + i;
        dst[0] = n;
        dst[s] = n;
        i = (*src >> 16) & 0xFF;
        k = (*src >> 24) & 0xFF;
        n = (k << 24) + (k << 16) + (i << 8) + i;
        dst[1] = n;
        dst[s+1] = n;
        src++;
        dst += 2;
        w--;
    }
    while (w > 0);
}

void copy_to_screen_pxdblh(unsigned char *srcbuf, unsigned char *dstbuf, long width, long dst_shift)
{
    unsigned long n;
    unsigned long w;
    unsigned long s;
    unsigned long *dst;
    unsigned long *src;
    src = (unsigned long *)srcbuf;
    dst = (unsigned long *)dstbuf;
    w = ((unsigned long)width) >> 2;
    s = dst_shift >> 2;
    do
    {
        n = *src;
        dst[0] = n;
        dst[s] = n;
        src++;
        dst++;
        w--;
    }
    while (w > 0);
}

void copy_to_screen_pxdblw(unsigned char *srcbuf, unsigned char *dstbuf, long width)
{
    unsigned long i;
    unsigned long k;
    unsigned long n;
    unsigned long w;
    unsigned long *dst;
    unsigned long *src;
    src = (unsigned long *)srcbuf;
    dst = (unsigned long *)dstbuf;
    w = ((unsigned long)width) >> 2;
    do
    {
        i = (*src) & 0xFF;
        k = (*src >> 8) & 0xFF;
        n = (k << 24) + (k << 16) + (i << 8) + i;
        dst[0] = n;
        i = (*src >> 16) & 0xFF;
        k = (*src >> 24) & 0xFF;
        n = (k << 24) + (k << 16) + (i << 8) + i;
        dst[1] = n;
        src++;
        dst += 2;
        w--;
    }
    while (w > 0);
}

void copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags)
{
    unsigned char *dstbuf;
    long buf_center;
    long w;
    long h;
    if ( ((flags & SMK_PixelDoubleLine) != 0) || ((flags & SMK_InterlaceLine) != 0) )
    {
        buf_center = lbDisplay.GraphicsScreenWidth * ((LbScreenHeight() - 2 * height) >> 1);
    } else
    {
        buf_center = lbDisplay.GraphicsScreenWidth * ((LbScreenHeight() - height) >> 1);
    }
    w = width;
    if ((flags & SMK_PixelDoubleWidth) != 0)
      w = 2 * width;
    dstbuf = &lbDisplay.WScreen[buf_center + ((LbScreenWidth() - w) >> 1)];
    if ((flags & SMK_PixelDoubleLine) != 0)
    {
      if ((flags & SMK_PixelDoubleWidth) != 0)
      {
          for (h=height; h > 0; h--)
          {
              copy_to_screen_pxquad(srcbuf, dstbuf, width, lbDisplay.GraphicsScreenWidth);
              dstbuf += 2 * lbDisplay.GraphicsScreenWidth;
              srcbuf += width;
          }
      } else
      {
          for (h=height; h > 0; h--)
          {
              copy_to_screen_pxdblh(srcbuf, dstbuf, width, lbDisplay.GraphicsScreenWidth);
              dstbuf += 2 * lbDisplay.GraphicsScreenWidth;
              srcbuf += width;
          }
      }
    } else
    {
      if ((flags & SMK_PixelDoubleWidth) != 0)
      {
        if ((flags & SMK_InterlaceLine) != 0)
        {
            for (h=height; h > 0; h--)
            {
                copy_to_screen_pxdblw(srcbuf, dstbuf, width);
                dstbuf += 2 * lbDisplay.GraphicsScreenWidth;
                srcbuf += width;
            }
        } else
        {
            for (h=height; h > 0; h--)
            {
                copy_to_screen_pxdblw(srcbuf, dstbuf, width);
              dstbuf += lbDisplay.GraphicsScreenWidth;
              srcbuf += width;
            }
        }
      }
      else
      {
        if ((flags & SMK_InterlaceLine) != 0)
        {
            for (h=height; h > 0; h--)
            {
              memcpy(dstbuf, srcbuf, width);
              dstbuf += 2 * lbDisplay.GraphicsScreenWidth;
              srcbuf += width;
            }
        } else
        {
            for (h=height; h > 0; h--)
            {
              memcpy(dstbuf, srcbuf, width);
              dstbuf += lbDisplay.GraphicsScreenWidth;
              srcbuf += width;
            }
        }
      }
    }
    if (smack_draw_callback != NULL) {
      smack_draw_callback(lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    }
}

void copy_to_screen_scaled(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags)
{
    unsigned char *dstbuf;
    dstbuf = &lbDisplay.WScreen[0];
    copy_to_screen_px_ar_scale(srcbuf, dstbuf, width, height, flags);
    if (smack_draw_callback != NULL) {
      smack_draw_callback(lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    }
}

short play_smk_via_buffer(char *fname, int smkflags, int plyflags)
{
    SYNCDBG(7,"Starting");
    void *snd_driver=GetSoundDriver();
    if ( snd_driver )
      SmackSoundUseMSS(snd_driver);
    else
      plyflags |= 0x01;
    int opnflags = -((plyflags & 0x01) < 1);
    struct SmackTag *smktag = SmackOpen(fname, opnflags & 0xFE000, -1);
    if ( !smktag )
      return 0;
    unsigned long nframe = 1;
    unsigned char *buf = (unsigned char *)LbMemoryAlloc(smktag->Width*smktag->Height);
    if (buf == NULL)
    {
      SmackClose(smktag);
      return 0;
    }
    SmackToBuffer(smktag, 0, 0, smktag->Width, smktag->Height, buf, 0);
    while ( (plyflags & 0x0400) || (smktag->Frames >= nframe) )
    {
        short reset_pal = 0;
        int idx;
        if ( smktag->NewPalette )
        {
          reset_pal = 1;
          for (idx=0;idx<768;idx++)
          {
            unsigned char chr;
            chr = smktag->Palette[idx];
            smk_palette[idx] = chr>>2;
          }
        }
        SmackDoFrame(smktag);
        if (LbScreenLock() == Lb_SUCCESS)
        {
          if ( (plyflags & SMK_FullscreenFit) != 0 || (plyflags & SMK_FullscreenStretch) != 0 || (plyflags & SMK_FullscreenCrop) != 0 ) // new scaling mode
          {
              copy_to_screen_scaled(buf, smktag->Width, smktag->Height, plyflags);
          }
          else
          {
              copy_to_screen(buf, smktag->Width, smktag->Height, plyflags);
          }
          LbScreenUnlock();
          //LbDoMultitasking();
          if ( reset_pal )
          {
            LbScreenWaitVbi();
            LbPaletteSet(smk_palette);
          }
          LbScreenSwap();
        }
        SmackNextFrame(smktag);

        do {
          if (!LbWindowsControl())
          {
              SmackClose(smktag);
              return 2;
          }
          if (((plyflags & SMK_NoStopOnUserInput) == 0) && (lbKeyOn[KC_ESCAPE]
              || lbKeyOn[KC_RETURN] || lbKeyOn[KC_SPACE] || lbDisplay.LeftButton) )
          {
              SmackClose(smktag);
              LbMemoryFree(buf);
              return 2;
          }
        } while ( SmackWait(smktag) );
        ++nframe;
    }
    LbMemoryFree(buf);
    SmackClose(smktag);
    return 1;
}

/**
 * Plays Smacker file more directly.
 * @return Returns 0 on error, 1 if file was played, 2 if the play was interrupted.
 */
short play_smk_direct(char *fname, int smkflags, int plyflags)
{
    SYNCDBG(7,"Starting");

    void *snd_driver=GetSoundDriver();
    if ( snd_driver )
      SmackSoundUseMSS(snd_driver);
    else
      plyflags |= 0x01;
    int opnflags = -((plyflags & 0x01) < 1);
    struct SmackTag *smktag = SmackOpen(fname, opnflags & 0xFE000, -1);
    if ( !smktag )
      return 0;
    unsigned long nframe = 1;
    while ( (plyflags & 0x0400) || (smktag->Frames-1 >= nframe) )
    {
        short reset_pal = 0;
        int idx;
        if ( smktag->NewPalette )
        {
          reset_pal = 1;
          for (idx=0;idx<768;idx++)
          {
            unsigned char chr;
            chr = smktag->Palette[idx];
            smk_palette[idx] = chr>>2;
          }
        }
        if (LbScreenLock() == Lb_SUCCESS)
        {
          int left = 0;
          if ( smktag->Width < lbDisplay.PhysicalScreenWidth )
            left = (lbDisplay.PhysicalScreenWidth-smktag->Width) >> 1;
          int top = 0;
          if ( smktag->Height < lbDisplay.PhysicalScreenHeight )
            top = (lbDisplay.PhysicalScreenHeight-smktag->Height) >> 1;
          SmackToBuffer(smktag,left,top,lbDisplay.GraphicsScreenWidth,
              lbDisplay.GraphicsScreenHeight,lbDisplay.WScreen,0);
          SmackDoFrame(smktag);
          LbScreenUnlock();
          //LbDoMultitasking();
          if ( reset_pal )
          {
            LbScreenWaitVbi();
            LbPaletteSet(smk_palette);
          }
          LbScreenSwap();
        }
        SmackNextFrame(smktag);

        do {
          if (!LbWindowsControl())
          {
              SmackClose(smktag);
              return 2;
          }
          if (((plyflags & SMK_NoStopOnUserInput) == 0) &&
              (lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_RETURN] || lbKeyOn[KC_SPACE]
                || lbDisplay.LeftButton) )
          {
              SmackClose(smktag);
              return 2;
          }
        } while ( SmackWait(smktag) );
        ++nframe;
    }
    if ((plyflags & SMK_WriteStatusFile) != 0)
    {
        struct SmackSumTag smksum;
        SmackSummary(smktag, &smksum);
        FILE *ssp = fopen("smacksum.txt", "w");
        if ( ssp )
        {
            fprintf(ssp, "TotalTime         = %ld\n", smksum.TotalTime);
            fprintf(ssp, "MS100PerFrame     = %ld\n", smksum.MS100PerFrame);
            fprintf(ssp, "TotalOpenTime     = %ld\n", smksum.TotalOpenTime);
            fprintf(ssp, "TotalFrames       = %ld\n", smksum.TotalFrames);
            fprintf(ssp, "SkippedFrames     = %ld\n", smksum.SkippedFrames);
            fprintf(ssp, "TotalBlitTime     = %ld\n", smksum.TotalBlitTime);
            fprintf(ssp, "TotalReadTime     = %ld\n", smksum.TotalReadTime);
            fprintf(ssp, "TotalDecompTime   = %ld\n", smksum.TotalDecompTime);
            fprintf(ssp, "TotalBackReadTime = %ld\n", smksum.TotalBackReadTime);
            fprintf(ssp, "TotalReadSpeed    = %ld\n", smksum.TotalReadSpeed);
            fprintf(ssp, "SlowestFrameTime  = %ld\n", smksum.SlowestFrameTime);
            fprintf(ssp, "Slowest2FrameTime = %ld\n", smksum.Slowest2FrameTime);
            fprintf(ssp, "SlowestFrameNum   = %ld\n", smksum.SlowestFrameNum);
            fprintf(ssp, "Slowest2FrameNum  = %ld\n", smksum.Slowest2FrameNum);
            fprintf(ssp, "AverageFrameSize  = %ld\n", smksum.AverageFrameSize);
            fprintf(ssp, "Highest1SecRate   = %ld\n", smksum.Highest1SecRate);
            fprintf(ssp, "Highest1SecFrame  = %ld\n", smksum.Highest1SecFrame);
            fprintf(ssp, "HighestMemAmount  = %ld\n", smksum.HighestMemAmount);
            fprintf(ssp, "TotalExtraMemory  = %ld\n", smksum.TotalExtraMemory);
            fprintf(ssp, "HighestExtraUsed  = %ld\n", smksum.HighestExtraUsed);
            fclose(ssp);
        }
        FILE *svp = fopen(fname, "rb");
        FILE *sgp = fopen("smkgraph.raw", "wb");
        if ( (sgp) && (svp) )
        {
          int idx;
          fseek(svp, 104, 0);
          for (idx=0;idx<smksum.TotalFrames;idx++)
          {
            unsigned long rawval;
            unsigned short val;
            fread(&rawval, 4u, 1u, svp);
            if ( rawval < 65535 )
              val = rawval-12000;
            else
              val = 32767;
            fwrite(&val, 2u, 1u, sgp);
          }
        }
        if ( sgp )
          fclose(sgp);
        if ( svp )
          fclose(svp);
    }
    SmackClose(smktag);
    return 1;
}

short play_smk_(char *fname, int smkflags, int plyflags)
{
    short result;
    lbDisplay.LeftButton = 0;
    if ( (smack_draw_callback != NULL) || ((plyflags & SMK_PixelDoubleWidth) != 0)
        || ((plyflags & SMK_InterlaceLine) != 0) || ((plyflags & SMK_PixelDoubleLine) != 0)
        || (LbScreenIsDoubleBufferred()) )
      result = play_smk_via_buffer(fname, smkflags, plyflags);
    else
      result = play_smk_direct(fname, smkflags, plyflags);
    return result;
}

/**
 * Writes the data into FLI animation.
 * @return Returns false on error, true on success.
 */
short anim_write_data(void *buf, long size)
{
    if (LbFileWrite(animation.outfhndl,buf,size) == size)
    {
      return true;
    }
    return false;
}

/**
 * Stores data into FLI buffer.
 * @return Returns false on error, true on success.
 */
short anim_store_data(void *buf, long size)
{
    memcpy(animation.field_C, buf, size);
    animation.field_C += size;
    return true;
}

/**
 * Reads the data from FLI animation.
 * @return Returns false on error, true on success.
 */
short anim_read_data(void *buf, long size)
{
    if (buf == NULL)
    {
        LbFileSeek(animation.inpfhndl,size,Lb_FILE_SEEK_CURRENT);
        return true;
    } else
    if (LbFileRead(animation.inpfhndl,buf,size) == size)
    {
        return true;
    }
    return false;
}

long anim_make_FLI_COPY(unsigned char *screenbuf)
{
    int scrpoints = animation.header.height * animation.header.width;
    memcpy(animation.field_C, screenbuf, scrpoints);
    animation.field_C += scrpoints;
    return scrpoints;
}

long anim_make_FLI_BLACK(unsigned char *screenbuf)
{
    return 0;
}

long anim_make_FLI_COLOUR256(unsigned char *palette)
{
    if (LbMemoryCompare(animation.palette, palette, 768) == 0) {
        return 0;
    }
    unsigned short *change_count;
    unsigned char *kept_count;
    short colridx;
    short change_chunk_len;
    short kept_chunk_len;
    change_chunk_len = 0;
    kept_chunk_len = 0;
    change_count = (unsigned short *)animation.field_C;
    kept_count = NULL;
    animation.field_C += 2;
    for (colridx = 0; colridx < 256; colridx++)
    {
        unsigned char *anipal;
        unsigned char *srcpal;
        anipal = &animation.palette[3 * colridx];
        srcpal = &palette[3 * colridx];

        if (LbMemoryCompare(anipal, srcpal, 3) == 0)
        {
            change_chunk_len = 0;
            kept_chunk_len++;
        } else
        {
            if (!change_chunk_len)
            {
                *animation.field_C = kept_chunk_len;
                kept_chunk_len = 0;
                animation.field_C++;
                kept_count = (unsigned char *)animation.field_C;
                animation.field_C++;
            }
            ++change_chunk_len;
            *animation.field_C = 4 * srcpal[0];
            animation.field_C++;
            *animation.field_C = 4 * srcpal[1];
            animation.field_C++;
            *animation.field_C = 4 * srcpal[2];
            animation.field_C++;
            ++(*kept_count);
        }
        if (change_chunk_len == 1) {
            ++(*change_count);
        }
    }
    return 1;
}

/**
 * Compress data into FLI's BRUN block (8-bit Run-Length compression).
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_BRUN(unsigned char *screenbuf)
{
    unsigned char *blk_begin = animation.field_C;
    short w;
    short h;
    short k;
    short count;
    unsigned char *sbuf = screenbuf;
    for ( h = animation.header.height; h>0; h-- )
    {
      animation.field_C++;
      for (w=animation.header.width; w>0; )
      {
        count = 0;
        // Counting size of RLE block
        for ( k=1; w>1; k++ )
        {
          if (sbuf[k] != sbuf[0]) break;
          if (count == 127) break;
          w--;
          count++;
        }
        // If RLE block would be valid
        if ( count>0 )
        {
          if ( count < 127 )
            { count++; w--; }
          *animation.field_C = (char)count;
          animation.field_C++;
          *animation.field_C = sbuf[0];
          animation.field_C++;
          sbuf += count;
        } else
        {
          if ( w > 1 )
          {
            count=0;
            // Find the next block of at least 4 same pixels
            for ( k = 0; w>0; k++ )
            {
              if ( (sbuf[k+1]==sbuf[k]) && (sbuf[k+2]==sbuf[k]) && (sbuf[k+3]==sbuf[k]) )
                break;
              if ( count == -127 )
                break;
              count--;
              w--;
            }
          } else
          { count=-1; w--; }
          if ( count!=0 )
          {
            *animation.field_C = (char)count;
            animation.field_C++;
            memcpy(animation.field_C, sbuf, -count);
            sbuf -= count;
            animation.field_C -= count;
          }
        }
      }
    }
    // Make the block size even
    if ((int)animation.field_C & 1)
    {
      *animation.field_C='\0';
      animation.field_C++;
    }
    return (animation.field_C - blk_begin);
}

/**
 * Compress data into FLI's SS2 block.
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_SS2(unsigned char *curdat, unsigned char *prvdat)
{
    unsigned char *blk_begin;
    blk_begin=animation.field_C;
    unsigned char *cbuf;
    unsigned char *pbuf;
    unsigned char *cbf;
    unsigned char *pbf;
    short h;
    short w;
    short k;
    short nsame;
    short ndiff;
    short wend;
    short wendt;
    cbuf = curdat;
    pbuf = prvdat;
    unsigned short *lines_count;
    unsigned short *pckt_count;
    lines_count = (unsigned short *)animation.field_C;
    animation.field_C += 2;
    pckt_count = (unsigned short *)animation.field_C;

    wend = 0;
    for (h=animation.header.height; h>0; h--)
    {
      cbf = cbuf;
      pbf = pbuf;
      if (wend == 0)
      {
          pckt_count = (unsigned short *)animation.field_C;
          animation.field_C += 2;
          (*lines_count)++;
      }
      for (w=animation.header.width;w>0;)
      {
        for ( k=0; w>0; k++)
        {
          if ( *(unsigned short *)(pbf+2*(long)k) != *(unsigned short *)(cbf+2*(long)k) )
            break;
          w -= 2;
        }
        if (2*(long)k == animation.header.width)
        {
          wend--;
          cbf += LbGraphicsScreenWidth();
          pbf += LbGraphicsScreenWidth();
          continue;
        }
        if ( w > 0 )
        {
          if (wend != 0)
          {
            (*pckt_count) = wend;
            pckt_count = (unsigned short *)animation.field_C;
            animation.field_C += 2;
          }
          wendt = 2*k;
          wend = wendt;
          while (wend > 255)
          {
            *(unsigned char *)animation.field_C = 255;
            animation.field_C++;
            *(unsigned char *)animation.field_C = 0;
            animation.field_C++;
            wend -= 255;
            (*pckt_count)++;
          }
          cbf += wendt;
          pbf += wendt;

          for (nsame=0; nsame<127; nsame++)
          {
            if (w <= 2) break;
            if ((*(unsigned short *)(pbf+2*nsame+0) == *(unsigned short *)(cbf+2*nsame+0)) &&
                (*(unsigned short *)(pbf+2*nsame+2) == *(unsigned short *)(cbf+2*nsame+2)))
                break;
            if ( *(unsigned short *)(cbf+2*nsame+2) != *(unsigned short *)(cbf) )
              break;
            w -= 2;
          }
          if (nsame > 0)
          {
            if (nsame < 127)
            {
              nsame++;
              w -= 2;
            }
            *(unsigned char *)animation.field_C = wend;
            animation.field_C++;
            *(unsigned char *)animation.field_C = -nsame;
            animation.field_C++;
            *(unsigned short *)animation.field_C = *(unsigned short *)cbf;
            animation.field_C+=2;
            pbf += 2*nsame;
            cbf += 2*nsame;
            wend = 0;
            (*pckt_count)++;
          } else
          {
            if (w == 2)
            {
              ndiff = 1;
              w -= 2;
            } else
            {
              for (ndiff=0; ndiff<127; ndiff++)
              {
                if (w <= 0) break;
                if ( *(unsigned short *)(pbf+2*ndiff) == *(unsigned short *)(cbf+2*ndiff) )
                  break;
                if ((*(unsigned short *)(cbf+2*(ndiff+1)) == *(unsigned short *)(cbf+2*ndiff)) &&
                   (*(unsigned short *)(cbf+2*(ndiff+2)) == *(unsigned short *)(cbf+2*ndiff)) )
                  break;
                w -= 2;
              }
            }
            if (ndiff>0)
            {
              *(unsigned char *)animation.field_C = wend;
              animation.field_C++;
              *(unsigned char *)animation.field_C = ndiff;
              animation.field_C++;
              memcpy(animation.field_C, cbf, 2*(long)ndiff);
              animation.field_C += 2*(long)ndiff;
              pbf += 2*(long)ndiff;
              cbf += 2*(long)ndiff;
              wend = 0;
              (*pckt_count)++;
            }
          }
        }
      }
        cbuf += LbGraphicsScreenWidth();
        pbuf += LbGraphicsScreenWidth();
    }

    if (animation.header.height+wend == 0)
    {
      (*lines_count) = 1;
      (*pckt_count) = 1;
      *(unsigned char *)animation.field_C = 0;
      animation.field_C++;
      *(unsigned char *)animation.field_C = 0;
      animation.field_C++;
    } else
    if (wend != 0)
    {
        animation.field_C -= 2;
        (*lines_count)--;
    }
    // Make the data size even
    animation.field_C = (unsigned char *)(((unsigned int)animation.field_C + 1) & 0xFFFFFFFE);
    return animation.field_C - blk_begin;
}

/**
 * Compress data into FLI's LC block.
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_LC(unsigned char *curdat, unsigned char *prvdat)
{
    unsigned char *blk_begin;
    blk_begin=animation.field_C;
    unsigned char *cbuf;
    unsigned char *pbuf;
    unsigned char *cbf;
    unsigned char *pbf;
    unsigned char *outptr;
    short h;
    short w;
    short hend;
    short wend;
    short hdim;
    short wendt;
    short k;
    short nsame;
    short ndiff;
    int blksize;

    cbuf = curdat;
    pbuf = prvdat;
    for (hend = animation.header.height; hend>0;  hend--)
    {
      wend = 0;
      for (w = animation.header.width; w>0; w--)
      {
        if (cbuf[wend] != pbuf[wend]) break;
        ++wend;
      }
      if ( wend != animation.header.width )
        break;
      cbuf += LbGraphicsScreenWidth();
      pbuf += LbGraphicsScreenWidth();
    }

    if (hend != 0)
    {
      hend = animation.header.height - hend;
      blksize = animation.header.width * (long)(animation.header.height-1);
      cbuf = curdat+blksize;
      pbuf = prvdat+blksize;
      for (h=animation.header.height; h>0; h--)
      {
        wend = 0;
        for (w=animation.header.width; w>0; w--)
        {
          if (cbuf[wend] != pbuf[wend]) break;
          wend++;
        }
        if ( wend != animation.header.width )
          break;
        cbuf -= LbGraphicsScreenWidth();
        pbuf -= LbGraphicsScreenWidth();
      }
      hdim = h - hend;
      blksize = animation.header.width * (long)hend;
      cbuf = curdat+blksize;
      pbuf = prvdat+blksize;
      *(unsigned short *)animation.field_C = hend;
      animation.field_C += 2;
      *(unsigned short *)animation.field_C = hdim;
      animation.field_C += 2;

      for (h = hdim; h>0; h--)
      {
          cbf = cbuf;
          pbf = pbuf;
          outptr = animation.field_C++;
          for (w=animation.header.width; w>0; )
          {
            for ( wend=0; w>0; wend++)
            {
              if ( cbf[wend] != pbf[wend]) break;
              w--;
            }
            wendt = wend;
            if (animation.header.width == wend) continue;
            if ( w <= 0 ) break;
            while ( wend > 255 )
            {
              *(unsigned char *)animation.field_C = 255;
              animation.field_C++;
              *(unsigned char *)animation.field_C = 0;
              animation.field_C++;
              wend -= 255;
              (*(unsigned char *)outptr)++;
            }
            cbf += wendt;
            pbf += wendt;
            k = 0;
            nsame = 0;
            while ( w > 1 )
            {
              if ( nsame == -127 ) break;
              if ((cbf[k+0] == pbf[k+0]) && (cbf[k+1] == pbf[k+1]) && (cbf[k+2] == pbf[k+2]))
                  break;
              if (cbf[k+1] != cbf[0]) break;
              w--;
              k++;
              nsame--;
            }
            if ( nsame )
            {
              if ( nsame != -127 )
              { nsame--; w--; }
              *(unsigned char *)animation.field_C = wend;
              animation.field_C++;
              *(unsigned char *)animation.field_C = nsame;
              animation.field_C++;
              *(unsigned char *)animation.field_C = cbf[0];
              cbf -= nsame;
              pbf -= nsame;
              animation.field_C++;
              (*(unsigned char *)outptr)++;
            } else
            {
              if ( w == 1 )
              {
                ndiff = nsame + 1;
                w--;
              } else
              {
                k = 0;
                ndiff = 0;
                while (w != 0)
                {
                  if ( ndiff == 127 ) break;
                  if ((cbf[k+0] == pbf[k+0]) && (cbf[k+1] == pbf[k+1]) && (cbf[k+2] == pbf[k+2]))
                      break;
                  if ((cbf[k+1] == cbf[k+0]) && (cbf[k+2] == cbf[k+0]) && (cbf[k+3] == cbf[k+0]))
                      break;
                  w--;
                  k++;
                  ndiff++;
                }
              }
              if (ndiff != 0)
              {
                *(unsigned char *)animation.field_C = wend;
                animation.field_C++;
                *(unsigned char *)animation.field_C = ndiff;
                animation.field_C++;
                memcpy(animation.field_C, cbf, ndiff);
                animation.field_C += ndiff;
                cbf += ndiff;
                pbf += ndiff;
                (*(unsigned char *)outptr)++;
              }
            }
          }
          cbuf += LbGraphicsScreenWidth();
          pbuf += LbGraphicsScreenWidth();
      }
    } else
    {
      *(short *)animation.field_C = 0;
      animation.field_C += 2;
      *(short *)animation.field_C = 1;
      animation.field_C += 2;
      *(char *)animation.field_C = 0;
      animation.field_C++;
    }
    // Make the data size even
    animation.field_C = (unsigned char *)(((unsigned int)animation.field_C + 1) & 0xFFFFFFFE);
    return animation.field_C - blk_begin;
}

/*
 * Returns size of the FLI movie frame buffer, for given width
 * and height of animation. The buffer of returned size is big enough
 * to store one frame of any kind (any compression).
 */
long anim_buffer_size(int width,int height,int bpp)
{
    int n;
    n=(bpp>>3);
    if (bpp%8) n++;
    return abs(width)*abs(height)*n + 32767;
}

/*
 * Returns size of the FLI movie frame buffer, for given width
 * and height of animation. The buffer of returned size is big enough
 * to store one frame of any kind (any compression).
 */
short anim_format_matches(int width,int height,int bpp)
{
    if (width != animation.header.width)
      return false;
    if (height != animation.header.height)
      return false;
    if (bpp != animation.header.depth)
      return false;
    return true;
}

short anim_stop(void)
{
    SYNCLOG("Finishing movie recording.");
    if ( ((animation.field_0 & 0x01)==0) || (animation.outfhndl==0))
    {
      ERRORLOG("Can't stop recording movie");
      return false;
    }
    LbFileSeek(animation.outfhndl, 0, Lb_FILE_SEEK_BEGINNING);
    animation.header.frames--;
    LbFileWrite(animation.outfhndl, &animation.header, sizeof(struct AnimFLIHeader));
    if ( LbFileClose(animation.outfhndl) == -1 )
    {
        ERRORLOG("Can't close movie file");
        return false;
    }
    animation.outfhndl = 0;
    LbMemoryFree(animation.chunkdata);
    animation.chunkdata=NULL;
    animation.field_0 = 0;
    return true;
}

short anim_open(char *fname, int arg1, short arg2, int width, int height, int bpp, unsigned int flags)
{
  if ( flags & animation.field_0 )
  {
      ERRORLOG("Cannot record movie");
    return false;
  }
  if (flags & 0x01)
  {
      SYNCLOG("Starting to record new movie, \"%s\".",fname);
      LbMemorySet(&animation, 0, sizeof(struct Animation));
      animation.field_0 |= flags;
      animation.videobuf = LbMemoryAlloc(2 * height*width);
      if (animation.videobuf==NULL)
      {
          ERRORLOG("Cannot allocate video buffer.");
        return false;
      }
      long max_chunk_size = anim_buffer_size(width,height,bpp);
      animation.chunkdata = LbMemoryAlloc(max_chunk_size);
      if (animation.chunkdata==NULL)
      {
          ERRORLOG("Cannot allocate chunk buffer.");
        return false;
      }
      animation.outfhndl = LbFileOpen(fname, Lb_FILE_MODE_NEW);
      if (animation.outfhndl == -1)
      {
          ERRORLOG("Can't open movie file.");
        return false;
      }
      animation.header.dsize = 128;
      animation.header.magic = 0xAF12;
      animation.header.depth = bpp;
      animation.header.flags = 3;
      animation.header.speed = 57;
      animation.header.created = 0;
      animation.header.frames = 0;
      animation.header.width = width;
      animation.header.updated = 0;
      animation.header.aspectx = 6;
      animation.header.height = height;
      animation.header.reserved2 = 0;
      animation.header.creator = 0x464C4942;//'BILF'
      animation.header.aspecty = 5;
      animation.header.updater = 0x464C4942;
      LbMemorySet(animation.header.reserved3, 0, sizeof(animation.header.reserved3));
      animation.header.oframe1 = 0;
      animation.header.oframe2 = 0;
      LbMemorySet(animation.header.reserved4, 0, sizeof(animation.header.reserved4));
      animation.field_18 = arg2;
      if ( !anim_write_data(&animation.header, sizeof(struct AnimFLIHeader)) )
      {
          ERRORLOG("Movie write error.");
        LbFileClose(animation.outfhndl);
        return false;
      }
      animation.field_31C = 0;
      animation.field_320 = height*width + 1024;
      LbMemorySet(animation.palette, -1, sizeof(animation.palette));
  }
  if (flags & 0x02)
  {
      SYNCLOG("Resuming movie recording, \"%s\".",fname);
      animation.field_0 |= flags;
      animation.inpfhndl = LbFileOpen(fname, 2);
      if ( animation.inpfhndl == -1 )
        return false;
      // Reading header
      if (!anim_read_data(&animation.header, sizeof(struct AnimFLIHeader)))
      {
          ERRORLOG("Movie header read error.");
        LbFileClose(animation.inpfhndl);
        return false;
      }
      // Now we can allocate chunk buffer
      long max_chunk_size = anim_buffer_size(animation.header.width,animation.header.height,animation.header.depth);
      animation.chunkdata = LbMemoryAlloc(max_chunk_size);
      if (animation.chunkdata==NULL)
        return false;
      if (!anim_read_data(&animation.chunk, sizeof(struct AnimFLIChunk)))
      {
          ERRORLOG("Movie chunk read error.");
        LbFileClose(animation.inpfhndl);
        return false;
      }
      if (animation.chunk.ctype == FLI_PREFIX)
      {
        if (!anim_read_data(animation.chunkdata, animation.chunk.csize-sizeof(struct AnimFLIChunk)))
        {
            ERRORLOG("Movie data read error.");
            LbFileClose(animation.inpfhndl);
            return false;
        }
      } else
      {
        LbFileSeek(animation.inpfhndl, -sizeof(struct AnimFLIChunk), Lb_FILE_SEEK_CURRENT);
      }
      animation.field_31C = 0;
  }
  return true;
}

TbBool anim_make_next_frame(unsigned char *screenbuf, unsigned char *palette)
{
    SYNCDBG(7,"Starting");
    unsigned long max_chunk_size;
    unsigned char *dataptr;
    long brun_size;
    long lc_size;
    long ss2_size;
    int width = animation.header.width;
    int height = animation.header.height;
    animation.field_C = animation.chunkdata;
    max_chunk_size = anim_buffer_size(width,height,animation.header.depth);
    LbMemorySet(animation.chunkdata, 0, max_chunk_size);
    animation.prefix.ctype = 0xF1FAu;
    animation.prefix.nchunks = 0;
    animation.prefix.csize = 0;
    LbMemorySet(animation.prefix.reserved, 0, sizeof(animation.prefix.reserved));
    struct AnimFLIPrefix *prefx = (struct AnimFLIPrefix *)animation.field_C;
    anim_store_data(&animation.prefix, sizeof(struct AnimFLIPrefix));
    animation.subchunk.ctype = 0;
    animation.subchunk.csize = 0;
    struct AnimFLIChunk *subchnk = (struct AnimFLIChunk *)animation.field_C;
    anim_store_data(&animation.subchunk, sizeof(struct AnimFLIChunk));
    if ( animation.field_31C == 0 )
    {
        animation.header.oframe1 = animation.header.dsize;
    } else
    if ( animation.field_31C == 1 )
    {
        animation.header.oframe2 = animation.header.dsize;
    }
    if ( anim_make_FLI_COLOUR256(palette) )
    {
        prefx->nchunks++;
        subchnk->ctype = 4;
        subchnk->csize = animation.field_C-(unsigned char *)subchnk;
        animation.subchunk.ctype = 0;
        animation.subchunk.csize = 0;
        subchnk = (struct AnimFLIChunk *)animation.field_C;
        anim_store_data(&animation.subchunk, sizeof(struct AnimFLIChunk));
    }
    int scrpoints = animation.header.height * (long)animation.header.width;
    if (animation.field_31C == 0)
    {
        if ( anim_make_FLI_BRUN(screenbuf) )
        {
          prefx->nchunks++;
          subchnk->ctype = FLI_BRUN;
        } else
        {
          anim_make_FLI_COPY(screenbuf);
          prefx->nchunks++;
          subchnk->ctype = FLI_COPY;
        }
    } else
    {
      // Determining the best compression method
      dataptr = animation.field_C;
      brun_size = anim_make_FLI_BRUN(screenbuf);
      LbMemorySet(dataptr, 0, brun_size);
      animation.field_C = dataptr;
      ss2_size = anim_make_FLI_SS2(screenbuf, animation.videobuf);
      LbMemorySet(dataptr, 0, ss2_size);
      animation.field_C = dataptr;
      lc_size = anim_make_FLI_LC(screenbuf, animation.videobuf);
      if ((lc_size < ss2_size) && (lc_size < brun_size))
      {
          // Store the LC compressed data
          prefx->nchunks++;
          subchnk->ctype = FLI_LC;
      } else
      if (ss2_size < brun_size)
      {
          // Clear the LC compressed data
          LbMemorySet(dataptr, 0, lc_size);
          animation.field_C = dataptr;
          // Compress with SS2 method
          anim_make_FLI_SS2(screenbuf, animation.videobuf);
          prefx->nchunks++;
          subchnk->ctype = FLI_SS2;
      } else
      if ( brun_size < scrpoints+16 )
      {
          // Clear the LC compressed data
            LbMemorySet(dataptr, 0, lc_size);
          animation.field_C = dataptr;
          // Compress with BRUN method
          anim_make_FLI_BRUN(screenbuf);
          prefx->nchunks++;
          subchnk->ctype = FLI_BRUN;
      } else
      {
          // Clear the LC compressed data
            LbMemorySet(dataptr, 0, lc_size);
          animation.field_C = dataptr;
          // Store uncompressed frame data
          anim_make_FLI_COPY(screenbuf);
          prefx->nchunks++;
          subchnk->ctype = FLI_COPY;
      }
    }
    subchnk->csize = animation.field_C-(unsigned char *)subchnk;
    prefx->csize = animation.field_C - animation.chunkdata;
    if ( !anim_write_data(animation.chunkdata, animation.field_C-animation.chunkdata) )
    {
    //LbSyncLog("Finished frame w/error.\n");
      return false;
    }
    memcpy(animation.videobuf, screenbuf, height*width);
    memcpy(animation.palette, palette, sizeof(animation.palette));
    animation.header.frames++;
    animation.field_31C++;
    animation.header.dsize += animation.field_C-animation.chunkdata;
    //LbSyncLog("Finished frame ok.\n");
    return true;
}

TbBool anim_record_frame(unsigned char *screenbuf, unsigned char *palette)
{
    if ((animation.field_0 & 0x01)==0)
      return false;
    if (!anim_format_matches(MyScreenWidth/pixel_size,MyScreenHeight/pixel_size,LbGraphicsScreenBPP()))
      return false;
    return anim_make_next_frame(screenbuf, palette);
}

short anim_record(void)
{
    SYNCDBG(7,"Starting");
    static char finalname[255];
    if (LbGraphicsScreenBPP() != 8)
    {
      ERRORLOG("Cannot record movie in non-8bit screen mode");
      return 0;
    }
    int idx;
    for (idx=0; idx < 10000; idx++)
    {
        sprintf(finalname, "%s/game%04d.flc","scrshots",idx);
        if (LbFileExists(finalname))
          continue;
        return anim_open(finalname, 0, 0, MyScreenWidth/pixel_size,MyScreenHeight/pixel_size,8, 1);
    }
    ERRORLOG("No free file name for recorded movie");
    return 0;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
