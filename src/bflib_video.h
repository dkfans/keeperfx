/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_video.h
 *     Header file for bflib_video.c.
 * @par Purpose:
 *     Video support library.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_VIDEO_H
#define BFLIB_VIDEO_H

#include "bflib_basics.h"

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define PALETTE_SIZE 768
#define PALETTE_COLORS 256

#pragma pack(1)

typedef unsigned char TbPixel;

enum TbScreenMode {
        Lb_SCREEN_MODE_INVALID      = 0x00,
        Lb_SCREEN_MODE_320_200_8    = 0x01,
        Lb_SCREEN_MODE_320_200_16   = 0x02,
        Lb_SCREEN_MODE_320_200_24   = 0x03,
        Lb_SCREEN_MODE_512_384_16   = 0x08,
        Lb_SCREEN_MODE_512_384_24   = 0x09,
        Lb_SCREEN_MODE_640_400_8    = 0x0A,
        Lb_SCREEN_MODE_640_400_16   = 0x0B,
        Lb_SCREEN_MODE_320_240_8    = 0x04,
        Lb_SCREEN_MODE_320_240_16   = 0x05,
        Lb_SCREEN_MODE_320_240_24   = 0x06,
        Lb_SCREEN_MODE_512_384_8    = 0x07,
        Lb_SCREEN_MODE_640_400_24   = 0x0C,
        Lb_SCREEN_MODE_640_480_8    = 0x0D,
        Lb_SCREEN_MODE_640_480_16   = 0x0E,
        Lb_SCREEN_MODE_640_480_24   = 0x0F,
        Lb_SCREEN_MODE_800_600_8    = 0x10,
        Lb_SCREEN_MODE_800_600_16   = 0x11,
        Lb_SCREEN_MODE_800_600_24   = 0x12,
        Lb_SCREEN_MODE_1024_768_8   = 0x13,
        Lb_SCREEN_MODE_1024_768_16  = 0x14,
        Lb_SCREEN_MODE_1024_768_24  = 0x15,
        Lb_SCREEN_MODE_1200_1024_8  = 0x16,
        Lb_SCREEN_MODE_1200_1024_16 = 0x17,
        Lb_SCREEN_MODE_1200_1024_24 = 0x18,
        Lb_SCREEN_MODE_1600_1200_8  = 0x19,
        Lb_SCREEN_MODE_1600_1200_16 = 0x1A,
        Lb_SCREEN_MODE_1600_1200_24 = 0x1B,
};

enum TbPaletteFadeFlag {
        Lb_PALETTE_FADE_OPEN   = 0,
        Lb_PALETTE_FADE_CLOSED = 1,
};

struct TbScreenModeInfo {
          unsigned short Width;
          unsigned short Height;
          unsigned short BitsPerPixel;
          long Available;//bool
          long VideoMode;
          char Desc[23];
};

struct TbGraphicsWindow {
    long x;
    long y;
    long width;
    long height;
};

struct TbDisplayStruct {
        uchar *PhysicalScreen;
        uchar *WScreen;
        uchar *GlassMap;
        uchar *FadeTable;
        uchar *GraphicsWindowPtr;
        struct TbSprite *MouseSprite;
        long PhysicalScreenWidth;
        long PhysicalScreenHeight;
        long GraphicsScreenWidth;
        long GraphicsScreenHeight;
        long GraphicsWindowX;
        long GraphicsWindowY;
        long GraphicsWindowWidth;
        long GraphicsWindowHeight;
        long MouseWindowX;
        long MouseWindowY;
        long MouseWindowWidth;
        long MouseWindowHeight;
        long MouseX;
        long MouseY;
        long MMouseX;
        long MMouseY;
        long RMouseX;
        long RMouseY;
        ushort DrawFlags;
        ushort OldVideoMode;
        ushort ScreenMode;
        uchar VesaIsSetUp;
        uchar LeftButton;
        uchar RightButton;
        uchar MiddleButton;
        uchar MLeftButton;
        uchar MRightButton;
        uchar MMiddleButton;
        uchar RLeftButton;
        uchar RMiddleButton;
        uchar RRightButton;
        uchar FadeStep;
        uchar DrawColour;
        uchar *Palette;
};

DLLIMPORT extern struct TbDisplayStruct _DK_lbDisplay;
#define lbDisplay _DK_lbDisplay
DLLIMPORT extern unsigned short _DK_MyScreenWidth;
#define MyScreenWidth _DK_MyScreenWidth
DLLIMPORT extern unsigned short _DK_MyScreenHeight;
#define MyScreenHeight _DK_MyScreenHeight
DLLIMPORT extern unsigned short _DK_pixel_size;
#define pixel_size _DK_pixel_size

DLLIMPORT unsigned char _DK_fade_started;
#define fade_started _DK_fade_started
DLLIMPORT unsigned char _DK_from_pal[PALETTE_SIZE];
#define from_pal _DK_from_pal
DLLIMPORT unsigned char _DK_to_pal[PALETTE_SIZE];
#define to_pal _DK_to_pal
DLLIMPORT long _DK_fade_count;
#define fade_count _DK_fade_count

#pragma pack()
/******************************************************************************/
/*
extern unsigned char *palette;
extern struct TbDisplayStruct lbDisplay;
extern unsigned char *lbVesaData;
extern bool screen_initialised;
extern void *back_buffer;
extern char redraw_screen_flag;
extern bool lbScreenDirectAccessActive;
extern unsigned short lbVesaPage;
*/
/******************************************************************************/
short LbScreenLock(void);
short LbScreenUnlock(void);
short LbScreenIsLocked(void);
short LbScreenSwap(void);
short LbScreenClear(TbPixel colour);
short LbWindowsControl(void);
long LbPaletteFade(unsigned char *pal, long n, enum TbPaletteFadeFlag flg);
short LbPaletteStopOpenFade(void);
void LbScreenWaitVbi(void);
int LbScreenSetup(enum TbScreenMode mode, unsigned int width,
               unsigned int height, TbPixel *palette, int flag1, int flag2);
int LbPaletteSet(unsigned char *palette);
int LbPaletteGet(unsigned char *palette);
void LbSetIcon(unsigned short nicon);
short LbScreenReset(void);
short LbScreenStoreGraphicsWindow(struct TbGraphicsWindow *grwnd);
short LbScreenLoadGraphicsWindow(struct TbGraphicsWindow *grwnd);
void copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags);
struct TbScreenModeInfo *LbScreenGetModeInfo(unsigned short mode);
enum TbScreenMode LbRecogniseVideoModeString(char *str);
short LbScreenSetGraphicsWindow(long x, long y, long width, long height);
short LbScreenIsModeAvailable(enum TbScreenMode mode);
short LbIsActive(void);
TbPixel LbPaletteFindColour(unsigned char *pal, unsigned char r, unsigned char g, unsigned char b);
/*
bool __fastcall LbVesaGetGran(TbScreenMode mode);
int __fastcall LbVesaSetPage(short npage);

bool __fastcall LbScreenSetDoubleBuffering(bool newstate);
int __fastcall LbScreenSetup(TbScreenMode mode, unsigned int width,
               unsigned int height, unsigned char *palette);
bool __fastcall LbScreenClearGraphicsWindow(unsigned char colour);
bool __fastcall LbScreenClear(unsigned char colour);
int __cdecl LbScreenSwapBoxClear(int source_screen, int SourceX, int SourceY,
  int DestinationX, int DestinationY, unsigned int width, unsigned int height,
  unsigned char colour);
int __cdecl LbScreenSwapClear(unsigned char colour);
int __cdecl LbScreenSwapBox(int source_screen, int SourceX, int SourceY,
  int DestinationX, int DestinationY, unsigned int width, unsigned int height);

int __cdecl LbScreenDrawHVLineDirect(int X1, int Y1, int X2, int Y2);
*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
