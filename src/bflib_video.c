/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_video.c
 *     Video support library for 8-bit graphics.
 * @par Purpose:
 *     Allows displaying on graphics device - graphic canvas setup and locking
 *     functions.
 * @par Comment:
 *     None yet.
 * @author   Tomasz Lis
 * @date     11 Feb 2008 - 28 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_video.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbScreenModeInfo lbScreenModeInfo[]={
    {   0,   0, 0,0,   0x0,"MODE_INVALID"},
    { 320, 200, 8,0,   0x0,"MODE_320_200_8"},
    { 320, 200,16,0,   0x0,"MODE_320_200_16"},
    { 320, 200,24,0,   0x0,"MODE_320_200_24"},
    { 320, 240, 8,0,   0x0,"MODE_320_240_8"},
    { 320, 240,16,0,   0x0,"MODE_320_240_16"},
    { 320, 240,24,0,   0x0,"MODE_320_240_24"},
    { 512, 384, 8,0,   0x0,"MODE_512_384_8"},
    { 512, 384,16,0,   0x0,"MODE_512_384_16"},
    { 512, 384,24,0,0x0100,"MODE_512_384_24"},
    { 640, 400, 8,0,   0x0,"MODE_640_400_8"},
    { 640, 400,16,0,   0x0,"MODE_640_400_16"},
    { 640, 400,24,0,0x0101,"MODE_640_400_24"},
    { 640, 480, 8,0,   0x0,"MODE_640_480_8"},
    { 640, 480,16,0,   0x0,"MODE_640_480_16"},
    { 640, 480,24,0,0x0103,"MODE_640_480_24"},
    { 800, 600, 8,0,   0x0,"MODE_800_600_8"},
    { 800, 600,16,0,   0x0,"MODE_800_600_16"},
    { 800, 600,24,0,0x0105,"MODE_800_600_24"},
    {1024, 768, 8,0,   0x0,"MODE_1024_768_8"},
    {1024, 768,16,0,   0x0,"MODE_1024_768_16"},
    {1024, 768,24,0,0x0107,"MODE_1024_768_24"},
    {1280,1024, 8,0,   0x0,"MODE_1280_1024_8"},
    {1280,1024,16,0,   0x0,"MODE_1280_1024_16"},
    {1280,1024,24,0,   0x0,"MODE_1280_1024_24"},
    {1600,1200, 8,0,   0x0,"MODE_1600_1200_8"},
    {1600,1200,16,0,   0x0,"MODE_1600_1200_16"},
    {1600,1200,24,0,   0x0,"MODE_1600_1200_24"},
    {   0,   0, 0,0,   0x0,"MODE_INVALID"},
};

/******************************************************************************/
DLLIMPORT int _DK_LbPaletteFindColour(unsigned char *pal, unsigned char r, unsigned char g, unsigned char b);
DLLIMPORT int __stdcall _DK_LbScreenReset(void);
DLLIMPORT int _DK_LbScreenSetGraphicsWindow(int x, int y, uint width, uint height);
DLLIMPORT int _DK_LbScreenIsModeAvailable(enum TbScreenMode mode);
DLLIMPORT int __stdcall _DK_LbScreenLock(void);
DLLIMPORT int __stdcall _DK_LbScreenUnlock(void);
DLLIMPORT int __stdcall _DK_LbScreenSwap(void);
DLLIMPORT int __cdecl _DK_LbScreenClear(TbPixel colour);
DLLIMPORT int __stdcall _DK_LbWindowsControl(void);
DLLIMPORT int __cdecl _DK_LbPaletteFade(unsigned char *pal, long n, enum TbPaletteFadeFlag flg);
DLLIMPORT void __cdecl _DK_LbScreenWaitVbi(void);
DLLIMPORT int __cdecl _DK_LbScreenSetup(enum TbScreenMode mode, unsigned int width,
               unsigned int height, unsigned char *palette, int flag1, int flag2);
DLLIMPORT int __cdecl _DK_LbPaletteSet(unsigned char *palette);
DLLIMPORT int __cdecl _DK_LbPaletteGet(unsigned char *palette);
DLLIMPORT void __cdecl _DK_copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags);
DLLIMPORT int __stdcall _DK_LbIsActive(void);
/******************************************************************************/
DLLIMPORT extern int _DK_icon_index;
volatile int lbUserQuit = 0;
/******************************************************************************/
void *LbExeReferenceNumber(void)
{
  return NULL;
}

TbResult LbScreenLock(void)
{
  return _DK_LbScreenLock();
}

TbResult LbScreenUnlock(void)
{
  return _DK_LbScreenUnlock();
}

TbResult LbScreenSwap(void)
{
  return _DK_LbScreenSwap();
}

TbResult LbScreenClear(TbPixel colour)
{
  return _DK_LbScreenClear(colour);
}

TbBool LbWindowsControl(void)
{
  return (lbUserQuit < 1);
}

void LbPaletteFadeStep(unsigned char *from_pal,unsigned char *to_pal,long n)
{
  int i;
  unsigned char palette[PALETTE_SIZE];
  for (i=0; i < PALETTE_COLORS; i++)
  {
    palette[3*i+0] = from_pal[3*i+0] + fade_count * (short)(to_pal[3*i+0] - from_pal[3*i+0]) / n;
    palette[3*i+1] = from_pal[3*i+1] + fade_count * (short)(to_pal[3*i+1] - from_pal[3*i+1]) / n;
    palette[3*i+2] = from_pal[3*i+2] + fade_count * (short)(to_pal[3*i+2] - from_pal[3*i+2]) / n;
  }
  LbScreenWaitVbi();
  LbPaletteSet(palette);
}

TbResult LbPaletteStopOpenFade(void)
{
    fade_started = 0;
    return Lb_SUCCESS;
}

long LbPaletteFade(unsigned char *pal, long n, enum TbPaletteFadeFlag flg)
{
  if (flg == Lb_PALETTE_FADE_CLOSED)
  {
    LbPaletteGet(from_pal);
    if (pal == NULL)
    {
      pal = to_pal;
      memset(to_pal, 0, PALETTE_SIZE);
    }
    fade_count = 0;
    do
    {
      LbPaletteFadeStep(from_pal,pal,n);
      fade_count++;
    }
    while (fade_count <= n);
    fade_started = 0;
    return fade_count;
  }
  if (fade_started)
  {
    fade_count++;
    if (fade_count == n)
      fade_started = 0;
    if (pal == NULL)
      pal = to_pal;
  } else
  {
    fade_count = 0;
    fade_started = 1;
    LbPaletteGet(from_pal);
    if (pal == NULL)
    {
      memset(to_pal, 0, PALETTE_SIZE);
      pal = to_pal;
    }
  }
  LbPaletteFadeStep(from_pal,pal,n);
  return fade_count;
}

TbResult LbScreenWaitVbi(void)
{
  _DK_LbScreenWaitVbi();
}

TbResult LbScreenFindVideoModes(void)
{
  /*if (lpDDC == NULL)
    return Lb_FAIL;
  lpDDC->find_video_modes(); */
  return Lb_SUCCESS;
}

TbResult LbScreenSetup(enum TbScreenMode mode, unsigned int width, unsigned int height,
    unsigned char *palette, short buffers_count, TbBool wscreen_vid)
{
  return _DK_LbScreenSetup(mode,width,height,palette,buffers_count,wscreen_vid);
}

TbResult LbPaletteSet(unsigned char *palette)
{
  return _DK_LbPaletteSet(palette);
}

TbResult LbPaletteGet(unsigned char *palette)
{
  return _DK_LbPaletteGet(palette);
}

TbResult LbSetTitle(const char *title)
{
  return Lb_SUCCESS;
}

void LbSetIcon(unsigned short nicon)
{
  _DK_icon_index=nicon;
}

struct TbScreenModeInfo *LbScreenGetModeInfo(unsigned short mode)
{
  int maxmode=sizeof(lbScreenModeInfo)/sizeof(struct TbScreenModeInfo);
  if (mode < maxmode)
    return &lbScreenModeInfo[mode];
  return &lbScreenModeInfo[0];
}

TbBool LbScreenIsLocked(void)
{
    return (lbDisplay.WScreen > NULL);
}

TbResult LbScreenReset(void)
{
  return _DK_LbScreenReset();
}

TbBool LbIsActive(void)
{
  return _DK_LbIsActive();
/*  if (lpDDC != NULL)
    return lpDDC->numfield_1C;
  return -1;*/
}

/*
 * Stores the current graphics window coords into TbGraphicsWindow structure.
 * Intended to use with LbScreenLoadGraphicsWindow() when changing the window
 * temporarly.
 */
TbResult LbScreenStoreGraphicsWindow(struct TbGraphicsWindow *grwnd)
{
  grwnd->x = lbDisplay.GraphicsWindowX;
  grwnd->y = lbDisplay.GraphicsWindowY;
  grwnd->width = lbDisplay.GraphicsWindowWidth;
  grwnd->height = lbDisplay.GraphicsWindowHeight;
  return Lb_SUCCESS;
}

/*
 * Sets the current graphics window coords from those in TbGraphicsWindow structure.
 * Use it only with TbGraphicsWindow which was filled using function
 * LbScreenStoreGraphicsWindow(), because the values are not checked for sanity!
 * To set values from other sources, use LbScreenSetGraphicsWindow() instead.
 */
TbResult LbScreenLoadGraphicsWindow(struct TbGraphicsWindow *grwnd)
{
  lbDisplay.GraphicsWindowX = grwnd->x;
  lbDisplay.GraphicsWindowY = grwnd->y;
  lbDisplay.GraphicsWindowWidth = grwnd->width;
  lbDisplay.GraphicsWindowHeight = grwnd->height;
  lbDisplay.GraphicsWindowPtr = lbDisplay.WScreen
      + lbDisplay.GraphicsScreenWidth*lbDisplay.GraphicsWindowY + lbDisplay.GraphicsWindowX;
  return Lb_SUCCESS;
}

TbResult LbScreenSetGraphicsWindow(long x, long y, long width, long height)
{
  long x2,y2;
  long i;
  x2 = x + width;
  y2 = y + height;
  if (x2 < x)
  {
    i = (x^x2);
    x = x^i;
    x2 = x^i^i;
  }
  if (y2 < y)
  {
    i = (y^y2);
    y = y^i;
    y2 = y^i^i;
  }
  if (x < 0)
    x = 0;
  if (x2 < 0)
    x2 = 0;
  if (y < 0)
    y = 0;
  if (y2 < 0)
    y2 = 0;
  if (x > lbDisplay.GraphicsScreenWidth)
    x = lbDisplay.GraphicsScreenWidth;
  if (x2 > lbDisplay.GraphicsScreenWidth)
    x2 = lbDisplay.GraphicsScreenWidth;
  if (y > lbDisplay.GraphicsScreenHeight)
    y = lbDisplay.GraphicsScreenHeight;
  if (y2 > lbDisplay.GraphicsScreenHeight)
    y2 = lbDisplay.GraphicsScreenHeight;
  lbDisplay.GraphicsWindowX = x;
  lbDisplay.GraphicsWindowY = y;
  lbDisplay.GraphicsWindowWidth = x2 - x;
  lbDisplay.GraphicsWindowHeight = y2 - y;
  lbDisplay.GraphicsWindowPtr = lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth*y + x;
  return Lb_SUCCESS;
}

TbBool LbScreenIsModeAvailable(enum TbScreenMode mode)
{
  return _DK_LbScreenIsModeAvailable(mode);
}

enum TbScreenMode LbRecogniseVideoModeString(char *str)
{
  int maxmode=sizeof(lbScreenModeInfo)/sizeof(struct TbScreenModeInfo);
  int mode;
  for (mode=0; mode<maxmode; mode++)
  {
    if (stricmp(lbScreenModeInfo[mode].Desc,str) == 0)
      return (enum TbScreenMode)mode;
  }
  return Lb_SCREEN_MODE_INVALID;
}

TbPixel LbPaletteFindColour(unsigned char *pal, unsigned char r, unsigned char g, unsigned char b)
{
  return _DK_LbPaletteFindColour(pal, r, g, b);
}

void copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags)
{
  _DK_copy_to_screen(srcbuf, width, height, flags);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
