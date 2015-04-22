/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_video.h
 *     Header file for bflib_video.c.
 * @par Purpose:
 *     Video support library for 8-bit graphics.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Feb 2008 - 26 Jun 2010
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

#define PALETTE_COLORS 256
#define PALETTE_SIZE (3*PALETTE_COLORS)

#define MAX_SUPPORTED_SCREEN_WIDTH  3840
#define MAX_SUPPORTED_SCREEN_HEIGHT 2160

#pragma pack(1)

/** Pixel definition - represents value of one point on the graphics screen. */
typedef unsigned char TbPixel;

/** Standard video mode indexes, registered by LbScreenInitialize(). */
// TODO (related number 01) do we need all these wired reolution? What about chaning them to 
// useful value like 1920 1080?
enum ScreenMode {
    Lb_SCREEN_MODE_INVALID      = 0x00,
    Lb_SCREEN_MODE_320_200      = 0x01, // Used to play movies
    Lb_SCREEN_MODE_320_240      = 0x02,
    Lb_SCREEN_MODE_512_384      = 0x03,
    Lb_SCREEN_MODE_640_400      = 0x04,
    Lb_SCREEN_MODE_640_480      = 0x05,
    Lb_SCREEN_MODE_800_600      = 0x06,
    Lb_SCREEN_MODE_1024_768     = 0x07,
    Lb_SCREEN_MODE_1200_1024    = 0x08,
    Lb_SCREEN_MODE_1600_1200    = 0x09,
};

typedef unsigned short TbScreenMode;
typedef long TbScreenCoord;

enum TbPaletteFadeFlag {
    Lb_PALETTE_FADE_OPEN   = 0,
    Lb_PALETTE_FADE_CLOSED = 1,
};

enum TbDrawFlags {
    Lb_SPRITE_FLIP_HORIZ   = 0x0001,
    Lb_SPRITE_FLIP_VERTIC  = 0x0002,
    Lb_SPRITE_TRANSPAR4    = 0x0004,
    Lb_SPRITE_TRANSPAR8    = 0x0008,
    Lb_SPRITE_OUTLINE      = 0x0010,
    Lb_TEXT_HALIGN_LEFT    = 0x0020,
    Lb_TEXT_ONE_COLOR      = 0x0040,
    Lb_TEXT_HALIGN_RIGHT   = 0x0080,
    Lb_TEXT_HALIGN_CENTER  = 0x0100,
    Lb_TEXT_HALIGN_JUSTIFY = 0x0200,
    Lb_TEXT_UNDERLINE      = 0x0400,
    Lb_TEXT_UNDERLNSHADOW  = 0x0800,
};

enum TbVideoModeFlags {
    Lb_VF_DEFAULT     = 0x0000, // dummy flag
    Lb_VF_RGBCOLOR    = 0x0001,
    Lb_VF_WINDOWED    = 0x0010,
};

struct GraphicsWindow {
    long x;
    long y;
    long width;
    long height;
    TbPixel *ptr;
};
typedef struct GraphicsWindow TbGraphicsWindow;

struct ScreenModeInfo {
    /** Hardware driver screen width. */
    TbScreenCoord Width;

    /** Hardware driver screen height. */
    TbScreenCoord Height;

    /** Hardware driver color depth. Not in use since we fixed this to 32, remove when possible.*/
    unsigned short BitsPerPixel;

    /** Is the mode currently available for use. */
    int Available;

    /** Video mode flags. Can be Lb_VF_DEFAULT, Lb_VF_PALETTE, Lb_VF_TRUCOLOR, Lb_VF_RGBCOLOR.
    Not in use anymore since we fixed video mode to 32bit RGB color, remove when possible.;*/
    unsigned long VideoFlags;

    /** Text description of the mode. */
    char Desc[23];
};
typedef struct ScreenModeInfo TbScreenModeInfo;

// Do NOT modify imported structures
struct DisplayStruct {
        /** Pointer to physical screen buffer, not used. */
        uchar *PhysicalScreen;

        /** Pointer to graphics screen buffer, if locked. */
        uchar *WScreen;

        /** Pointer to glass map, used for 8-bit video transparency. */
        uchar *GlassMap;

        /** Pointer to fade table, used for 8-bit video fading. */
        uchar *FadeTable;

        /** Pointer to graphics window buffer, if locked. */
        uchar *GraphicsWindowPtr;

        /** Sprite used as mouse cursor. */
        struct TbSprite *MouseSprite;

        /** Resolution in width of the current video mode.
         *  Note that it's not always "physical" size.
         *  It is the part of screen buffer which is being drawn
         *  on physical screen (WScreen X pixel number). */
        long PhysicalScreenWidth;

        /** Resolution in height of the current video mode.
         *  Note that it's not always "physical" size.
         *  It is the part of screen buffer which is being drawn
         *  on physical screen (WScreen Y pixel number). */
        long PhysicalScreenHeight;

        /** Width of the screen buffer (WScreen X pitch).
         *  Note that only part of this width may be drawn on real screen. */
        long GraphicsScreenWidth;

        /** Height of the screen buffer (WScreen Y pitch).
        *  Note that only part of this height may be drawn on real screen. */
        long GraphicsScreenHeight;

        /** Current drawing area beginning X coordinate. */
        long GraphicsWindowX;

        /** Current drawing area beginning Y coordinate. */
        long GraphicsWindowY;

        /** Current drawing area width (size in X axis). */
        long GraphicsWindowWidth;

        /** Current drawing area height (size in Y axis). */
        long GraphicsWindowHeight;

        /** Current mouse clipping window start X coordinate. */
        long MouseWindowX;

        /** Current mouse clipping window start Y coordinate. */
        long MouseWindowY;

        /** Current mouse clipping window width (in pixels). */
        long MouseWindowWidth;

        /** Current mouse clipping window height (in pixels). */
        long MouseWindowHeight;

        /** Mouse position during button "down" event, X coordinate. */
        long MouseX;

        /** Mouse position during button "down" event, Y coordinate. */
        long MouseY;

        /** Mouse position during move, X coordinate. */
        long MMouseX;

        /** Mouse position during move, Y coordinate. */
        long MMouseY;

        /** Mouse position during button release, X coordinate. */
        long RMouseX;

        /** Mouse position during button release, Y coordinate. */
        long RMouseY;

        ushort DrawFlags;
        short MouseMoveRatio; // was ushort OldVideoMode; but wasn't needed

        // Actual Screen Mode of the lbDrawTexture, can be same as mode in setting, or 320*200 for playing movie.
        ushort ScreenMode;

        /** VESA set-up flag, used only with VBE video modes. */
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
        /** Selected drawing colour index. */
        uchar DrawColour;
        /** Currently active colour palette.
         *  LbPaletteGet() should be used to retrieve a copy of the palette. */
        uchar *Palette;
};
typedef struct DisplayStruct TbDisplayStruct;

/** Extensions to DisplayStruct - will be later integrated into it. */
struct DisplayStructEx {
    /** Colour index used for drawing shadow. */
    uchar ShadowColour;

    // Power hand have no creature to pick up, and no slab to dig.
    uchar isPowerHandNothingTodoLeftClick;

    // Power hand have no creature to drop slap.
    uchar isPowerHandNothingTodoRightClick;

    // Convert ratio from mouse move distance to camera move distance while dragging.
    double cameraMoveRatioX, cameraMoveRatioY;

    // Distance of camera to move.
    double cameraMoveX, cameraMoveY;

    // Angle to rotate the camera.
    long cameraRotateAngle;

    // Wheel event
    uchar wheelUp, wheelDown;
};
typedef struct DisplayStructEx TbDisplayStructEx;

struct SSurface;
typedef struct SSurface TSurface;

#pragma pack()
/******************************************************************************/

DLLIMPORT extern TbDisplayStruct _DK_lbDisplay;
#define lbDisplay _DK_lbDisplay
DLLIMPORT extern unsigned short _DK_MyScreenWidth;
#define MyScreenWidth _DK_MyScreenWidth
DLLIMPORT extern unsigned short _DK_MyScreenHeight;
#define MyScreenHeight _DK_MyScreenHeight
DLLIMPORT extern unsigned short _DK_pixel_size;
#define pixel_size _DK_pixel_size
DLLIMPORT unsigned short _DK_pixels_per_block;
#define pixels_per_block _DK_pixels_per_block
DLLIMPORT unsigned short _DK_units_per_pixel;
#define units_per_pixel _DK_units_per_pixel
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
DLLIMPORT TbGraphicsWindow _DK_lbTextJustifyWindow;
#define lbTextJustifyWindow _DK_lbTextJustifyWindow
DLLIMPORT TbGraphicsWindow _DK_lbTextClipWindow;
#define lbTextClipWindow _DK_lbTextClipWindow

extern volatile TbBool lbScreenInitialized;
extern volatile TbBool lbUseSdk;
extern volatile TbBool lbInteruptMouse;
extern volatile TbDisplayStructEx lbDisplayEx;
extern unsigned char lbPalette[PALETTE_SIZE];
/******************************************************************************/
TbResult LbScreenHardwareConfig(const char *driver, short engine_bpp);
TbResult LbScreenInitialize(void);

TbResult LbScreenSetup(TbScreenMode mode, unsigned char *palette, short buffers_count, TbBool wscreen_vid);
TbResult LbScreenReset(TbBool resetMainWindow);

TbResult LbScreenFindVideoModes(void);
TbBool LbScreenIsModeAvailable(TbScreenMode mode);
TbScreenMode LbRecogniseVideoModeString(const char *desc);
TbScreenMode LbRegisterVideoMode(const char *desc, TbScreenCoord width, TbScreenCoord height, unsigned long flags);
TbScreenMode LbRegisterVideoModeString(const char *desc);
TbScreenModeInfo *LbScreenGetModeInfo(TbScreenMode mode);

TbScreenMode LbScreenActiveMode(void);
TbScreenCoord LbScreenWidth(void);
TbScreenCoord LbScreenHeight(void);
unsigned short LbGraphicsScreenBPP(void);
TbScreenCoord LbGraphicsScreenWidth(void);
TbScreenCoord LbGraphicsScreenHeight(void);

TbResult LbScreenLock(void);
TbResult LbScreenUnlock(void);
TbBool LbScreenIsLocked(void);

TbResult LbScreenRender(void);
TbResult LbScreenClear(TbPixel colour);
TbResult LbScreenWaitVbi(void);

long LbPaletteFade(unsigned char *pal, long n, enum TbPaletteFadeFlag flg);
TbResult LbPaletteStopOpenFade(void);
TbResult LbPaletteSet(unsigned char *palette);
TbResult LbPaletteGet(unsigned char *palette);
TbPixel LbPaletteFindColour(const unsigned char *pal, unsigned char r, unsigned char g, unsigned char b);
TbResult LbPaletteDataFillBlack(unsigned char *palette);
TbResult LbPaletteDataFillWhite(unsigned char *palette);

TbResult LbScreenStoreGraphicsWindow(TbGraphicsWindow *grwnd);
TbResult LbScreenLoadGraphicsWindow(TbGraphicsWindow *grwnd);
TbResult LbScreenSetGraphicsWindow(TbScreenCoord x, TbScreenCoord y,
    TbScreenCoord width, TbScreenCoord height);

TbResult LbSetTitle(const char *title);
TbResult LbSetIcon(unsigned short nicon);

void init_lbDisplayEx_values();
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
