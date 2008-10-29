/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 11 Feb 2008

// Purpose:
//    Header file for bflib_video.c.

// Comment:
//   Just a header file - #defines, typedefs, function prototypes etc.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#ifndef BFLIB_VIDEO_H
#define BFLIB_VIDEO_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define SCREEN_BUFFER_SIZE (640*480+16384)

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
          bool Available;
          long VideoMode;
          char Desc[23];
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

DLLIMPORT struct TbDisplayStruct _DK_lbDisplay;

#pragma pack()


extern unsigned char *palette;
extern unsigned char *lbVesaData;
extern bool screen_initialised;
extern void *back_buffer;
extern char redraw_screen_flag;
extern bool lbScreenDirectAccessActive;
extern unsigned short lbVesaPage;
/******************************************************************************/
bool __fastcall LbVesaGetGran(TbScreenMode mode);
int __fastcall LbVesaSetPage(short npage);

int __cdecl LbScreenWaitVbi();
bool __fastcall LbScreenIsModeAvailable(TbScreenMode mode);
bool __fastcall LbScreenSetDoubleBuffering(bool newstate);
int __fastcall LbScreenSetup(TbScreenMode mode, unsigned int width,
               unsigned int height, unsigned char *palette);
bool __fastcall LbScreenClearGraphicsWindow(unsigned char colour);
bool __fastcall LbScreenClear(unsigned char colour);
int __fastcall LbScreenSwap();
int __cdecl LbScreenSwapBoxClear(int source_screen, int SourceX, int SourceY,
  int DestinationX, int DestinationY, unsigned int width, unsigned int height,
  unsigned char colour);
int __cdecl LbScreenSwapClear(unsigned char colour);
int __cdecl LbScreenSwapBox(int source_screen, int SourceX, int SourceY,
  int DestinationX, int DestinationY, unsigned int width, unsigned int height);
bool __fastcall LbScreenReset();
bool __fastcall LbScreenLock();
bool __fastcall LbScreenUnlock();
int __fastcall LbScreenSetGraphicsWindow(int x, int y, uint width, uint height);
bool LbPaletteSet(unsigned char *palette);
bool LbPaletteGet(unsigned char *palette);

int __cdecl LbScreenDrawHVLineDirect(int X1, int Y1, int X2, int Y2);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
