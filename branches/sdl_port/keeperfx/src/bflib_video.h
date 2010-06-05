/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_video.h
 *     Header file for bflib_video.cpp.
 * @par Purpose:
 *     Video support library for 8-bit graphics.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Feb 2008 - 28 Nov 2009
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

#ifdef __cplusplus
#pragma pack(1)
#endif

typedef unsigned char TbPixel;

struct ScreenMode {
	int width;
	int height;
	int bpp;
	TbBool windowed;
};

typedef struct ScreenMode TbScreenMode;

enum TbPaletteFadeFlag {
    Lb_PALETTE_FADE_OPEN   = 0,
    Lb_PALETTE_FADE_CLOSED = 1,
};

enum TbDrawFlags {
    Lb_TEXT_HALIGN_LEFT    = 0x0020,
    Lb_TEXT_HALIGN_RIGHT   = 0x0080,
    Lb_TEXT_HALIGN_CENTER  = 0x0100,
    Lb_TEXT_HALIGN_JUSTIFY = 0x0200,
};

struct TbGraphicsWindow {
    long x;
    long y;
    long width;
    long height;
};

struct ScreenModeInfo {
          unsigned short Width;
          unsigned short Height;
          unsigned short BitsPerPixel;
          int Available;//bool
          long VideoMode;
          char Desc[23];
};
typedef struct ScreenModeInfo TbScreenModeInfo;

struct DisplayStruct {
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

typedef struct DisplayStruct TbDisplayStruct;
struct SSurface;
typedef struct SSurface TSurface;

#ifdef __cplusplus
#pragma pack()
#endif

DLLIMPORT extern TbDisplayStruct _DK_lbDisplay;
#define lbDisplay _DK_lbDisplay
DLLIMPORT extern unsigned short _DK_MyScreenWidth;
#define MyScreenWidth _DK_MyScreenWidth
DLLIMPORT extern unsigned short _DK_MyScreenHeight;
#define MyScreenHeight _DK_MyScreenHeight
DLLIMPORT extern unsigned short _DK_pixel_size;
#define pixel_size _DK_pixel_size
DLLIMPORT extern int _DK_lbUseSdk;
//#define lbUseSdk _DK_lbUseSdk

DLLIMPORT unsigned char _DK_fade_started;
#define fade_started _DK_fade_started
DLLIMPORT unsigned char _DK_from_pal[PALETTE_SIZE];
#define from_pal _DK_from_pal
DLLIMPORT unsigned char _DK_to_pal[PALETTE_SIZE];
#define to_pal _DK_to_pal
DLLIMPORT long _DK_fade_count;
#define fade_count _DK_fade_count
DLLIMPORT struct TbGraphicsWindow _DK_lbTextJustifyWindow;
#define lbTextJustifyWindow _DK_lbTextJustifyWindow
DLLIMPORT struct TbGraphicsWindow _DK_lbTextClipWindow;
#define lbTextClipWindow _DK_lbTextClipWindow
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
extern volatile int lbUserQuit;
extern volatile TbBool lbScreenInitialised;
extern volatile TbBool lbUseSdk;
/******************************************************************************/
TbResult LbScreenLock(void);
TbResult LbScreenUnlock(void);
TbBool LbScreenIsLocked(void);
TbResult LbScreenSwap(void);
TbResult LbScreenClear(TbPixel colour);
TbBool LbWindowsControl(void);
long LbPaletteFade(unsigned char *pal, long n, enum TbPaletteFadeFlag flg);
TbResult LbPaletteStopOpenFade(void);
TbResult LbScreenWaitVbi(void);
TbResult LbScreenSetup(TbScreenMode * mode, unsigned char *palette, short buffers_count, TbBool wscreen_vid);
TbResult LbPaletteSet(unsigned char *palette);
TbResult LbPaletteGet(unsigned char *palette);
void LbSetIcon(unsigned short nicon);
TbResult LbScreenReset(void);
TbResult LbScreenStoreGraphicsWindow(struct TbGraphicsWindow *grwnd);
TbResult LbScreenLoadGraphicsWindow(struct TbGraphicsWindow *grwnd);
void copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags);
TbScreenModeInfo *LbScreenGetModeInfo(unsigned short mode);
TbBool LbRecogniseVideoModeString(char * str, int * w, int * h, int * bpp, TbBool * windowed);
TbResult LbScreenSetGraphicsWindow(long x, long y, long width, long height);
TbBool LbScreenIsModeAvailable(TbScreenMode * mode);
TbBool LbIsActive(void);
TbPixel LbPaletteFindColour(unsigned char *pal, unsigned char r, unsigned char g, unsigned char b);
TbResult LbScreenFindVideoModes(void);
TbResult LbSetTitle(const char *title);
/*
bool __fastcall LbVesaGetGran(TbScreenMode mode);
int __fastcall LbVesaSetPage(short npage);

bool __fastcall LbScreenSetDoubleBuffering(bool newstate);
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
