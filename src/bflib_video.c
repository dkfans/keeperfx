/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_video.c
 *     Video support library.
 * @par Purpose:
 *     Allows displaying on graphics device - graphic canvas setup and drawing
 *     functions.
 * @par Comment:
 *     None yet.
 * @author   Tomasz Lis
 * @date     11 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_video.h"

DLLIMPORT extern int _DK_icon_index;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbScreenModeInfo lbScreenModeInfo[]={
    {   0,   0, 0,0,   0x0,"MODE_INVALID     "},
    { 320, 200, 8,0,   0x0,"MODE_320_200_8   "},
    { 320, 200,16,0,   0x0,"MODE_320_200_16  "},
    { 320, 200,24,0,   0x0,"MODE_320_200_24  "},
    { 320, 240, 8,0,   0x0,"MODE_320_240_8   "},
    { 320, 240,16,0,   0x0,"MODE_320_240_16  "},
    { 320, 240,24,0,   0x0,"MODE_320_240_24  "},
    { 512, 384, 8,0,   0x0,"MODE_512_384_8   "},
    { 512, 384,16,0,   0x0,"MODE_512_384_16  "},
    { 512, 384,24,0,0x0100,"MODE_512_384_24  "},
    { 640, 400, 8,0,   0x0,"MODE_640_400_8   "},
    { 640, 400,16,0,   0x0,"MODE_640_400_16  "},
    { 640, 400,24,0,0x0101,"MODE_640_400_24  "},
    { 640, 480, 8,0,   0x0,"MODE_640_480_8   "},
    { 640, 480,16,0,   0x0,"MODE_640_480_16  "},
    { 640, 480,24,0,0x0103,"MODE_640_480_24  "},
    { 800, 600, 8,0,   0x0,"MODE_800_600_8   "},
    { 800, 600,16,0,   0x0,"MODE_800_600_16  "},
    { 800, 600,24,0,0x0105,"MODE_800_600_24  "},
    {1024, 768, 8,0,   0x0,"MODE_1024_768_8  "},
    {1024, 768,16,0,   0x0,"MODE_1024_768_16 "},
    {1024, 768,24,0,0x0107,"MODE_1024_768_24 "}, 
    {1280,1024, 8,0,   0x0,"MODE_1280_1024_8 "},
    {1280,1024,16,0,   0x0,"MODE_1280_1024_16"},
    {1280,1024,24,0,   0x0,"MODE_1280_1024_24"}, 
    {1600,1200, 8,0,   0x0,"MODE_1600_1200_8 "},
    {1600,1200,16,0,   0x0,"MODE_1600_1200_16"},
    {1600,1200,24,0,   0x0,"MODE_1600_1200_24"}, 
    {   0,   0, 0,0,   0x0,"MODE_INVALID     "},
};

/******************************************************************************/
DLLIMPORT int __stdcall _DK_LbScreenLock(void);
DLLIMPORT int __stdcall _DK_LbScreenUnlock(void);
DLLIMPORT int __stdcall _DK_LbScreenSwap(void);
DLLIMPORT int __cdecl _DK_LbScreenClear(TbPixel colour);
DLLIMPORT int __stdcall _DK_LbWindowsControl(void);
DLLIMPORT int __cdecl _DK_LbPaletteFade(unsigned char *pal, long n, TbPaletteFadeFlag flg);
DLLIMPORT int __cdecl _DK_LbMouseChangeSprite(long);
DLLIMPORT void __cdecl _DK_LbScreenWaitVbi(void);
DLLIMPORT int __cdecl _DK_LbScreenSetup(TbScreenMode mode, unsigned int width,
               unsigned int height, unsigned char *palette, int flag1, int flag2);
DLLIMPORT int __cdecl _DK_LbPaletteSet(unsigned char *palette);
DLLIMPORT int __cdecl _DK_LbPaletteGet(unsigned char *palette);
DLLIMPORT void __cdecl _DK_copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags);
/******************************************************************************/
short LbScreenLock(void)
{
  return _DK_LbScreenLock();
}

short LbScreenUnlock(void)
{
  return _DK_LbScreenUnlock();
}

short LbScreenSwap(void)
{
  return _DK_LbScreenSwap();
}

short LbScreenClear(TbPixel colour)
{
  return _DK_LbScreenClear(colour);
}

short LbWindowsControl(void)
{
  return _DK_LbWindowsControl();
}

short LbPaletteFade(unsigned char *pal, long n, TbPaletteFadeFlag flg)
{
  return _DK_LbPaletteFade(pal, n, flg);
}

short LbMouseChangeSprite(long spr_idx)
{
  return _DK_LbMouseChangeSprite(spr_idx);
}

void LbScreenWaitVbi(void)
{
  _DK_LbScreenWaitVbi();
}

int LbScreenSetup(TbScreenMode mode, unsigned int width,
               unsigned int height, unsigned char *palette, int flag1, int flag2)
{
  return _DK_LbScreenSetup(mode,width,height,palette,flag1,flag2);
}

int LbPaletteSet(unsigned char *palette)
{
  return _DK_LbPaletteSet(palette);
}

int LbPaletteGet(unsigned char *palette)
{
  return _DK_LbPaletteGet(palette);
}

void LbSetIcon(unsigned short nicon)
{
  _DK_icon_index=nicon;
}

struct TbScreenModeInfo *LbScreenGetModeInfo(unsigned short mode)
{
  int maxmode=sizeof(lbScreenModeInfo)/sizeof(struct TbScreenModeInfo);
  if ((mode>=0)&&(mode<maxmode))
    return &lbScreenModeInfo[mode];
  return &lbScreenModeInfo[0];
}

short LbScreenIsLocked(void)
{
    return (lbDisplay.WScreen > NULL);
}

void copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags)
{
  _DK_copy_to_screen(srcbuf, width, height, flags);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
