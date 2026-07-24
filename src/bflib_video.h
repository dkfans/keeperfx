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

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define PALETTE_COLORS 256
#define PALETTE_SIZE (3*PALETTE_COLORS)

#define LOWRES_SCREEN_SIZE          320

#define MAX_SUPPORTED_SCREEN_WIDTH  3840
#define MAX_SUPPORTED_SCREEN_HEIGHT 2160

/******************************************************************************/
#pragma pack(1)

/** Pixel definition - represents value of one point on the graphics screen. */
typedef unsigned char TbPixel;

/** Standard video modes, registered by LbScreenInitialize().
 * These are standard VESA modes, indexed this way in all Bullfrog games.
 */
enum ScreenMode {
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

typedef unsigned short TbScreenMode;
typedef int32_t TbScreenCoord;

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
    Lb_SPRITE_REMAP        = 0x0800,
    Lb_TEXT_UNDERLNSHADOW  = 0x1000,
};

enum TbVideoModeFlags {
    Lb_VF_DEFAULT     = 0x0000, // dummy flag
    Lb_VF_RGBCOLOR    = 0x0001,
    Lb_VF_TRUCOLOR    = 0x0002,
    Lb_VF_PALETTE     = 0x0004,
    Lb_VF_WINDOWED    = 0x0010,
    Lb_VF_BORDERLESS  = 0x0020,
    Lb_VF_DESKTOP     = 0x0040,
    Lb_VF_FILLALL     = 0x0080,
};

struct GraphicsWindow {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    TbPixel *ptr;
};
typedef struct GraphicsWindow TbGraphicsWindow;

struct ScreenModeInfo {
    /** Hardware driver screen width. */
    TbScreenCoord Width;
    /** Hardware driver screen height. */
    TbScreenCoord Height;
    /** Hardware driver color depth. */
    unsigned short BitsPerPixel;
    /** Is the mode currently available for use. */
    int Available;
    /** Video mode flags. */
    uint32_t VideoFlags;
     /** Window position X. */
    int window_pos_x;
     /** Window position Y. */
    int window_pos_y;
    /** SDL window flags. */
    Uint32 sdlFlags;
    /** Text description of the mode. */
    char Desc[23];
};
typedef struct ScreenModeInfo TbScreenModeInfo;

struct DisplayStruct {
        /** Pointer to physical screen buffer, if locked. */
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
        const struct TbSprite *MouseSprite;
        /** Resolution in width of the current video mode.
         *  Note that it's not always "physical" size.
         *  It is the part of screen buffer which is being drawn
         *  on physical screen (WScreen X drawing size). */
        int32_t PhysicalScreenWidth;
        /** Resolution in height of the current video mode.
         *  Note that it's not always "physical" size.
         *  It is the part of screen buffer which is being drawn
         *  on physical screen (WScreen Y drawing size). */
        int32_t PhysicalScreenHeight;
        /** Width of the screen buffer (WScreen X pitch).
         *  Note that only part of this width may be drawn on real screen. */
        int32_t GraphicsScreenWidth;
        /** Height of the screen buffer (WScreen Y pitch).
        *  Note that only part of this height may be drawn on real screen. */
        int32_t GraphicsScreenHeight;
        /** Current graphics window beginning X coordinate. */
        int32_t GraphicsWindowX;
        /** Current graphics window beginning Y coordinate. */
        int32_t GraphicsWindowY;
        /** Current graphics window width (size in X axis). */
        int32_t GraphicsWindowWidth;
        /** Current graphics window height (size in Y axis). */
        int32_t GraphicsWindowHeight;
        /** Current mouse clipping window start X coordinate. */
        int32_t MouseWindowX;
        /** Current mouse clipping window start Y coordinate. */
        int32_t MouseWindowY;
        /** Current mouse clipping window width (in pixels). */
        int32_t MouseWindowWidth;
        /** Current mouse clipping window height (in pixels). */
        int32_t MouseWindowHeight;
        /** Mouse position during button "down" event, X coordinate. */
        int32_t MouseX;
        /** Mouse position during button "down" event, Y coordinate. */
        int32_t MouseY;
        /** Mouse position during move, X coordinate. */
        int32_t MMouseX;
        /** Mouse position during move, Y coordinate. */
        int32_t MMouseY;
        /** Mouse position during button release, X coordinate. */
        int32_t RMouseX;
        /** Mouse position during button release, Y coordinate. */
        int32_t RMouseY;
        ushort DrawFlags;
        short MouseMoveRatio; // was ushort OldVideoMode; but wasn't needed
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
    short WhellPosition;
    ushort WhellMoveUp;
    ushort WhellMoveDown;
    /** Colour index used for drawing shadow. */
    uchar ShadowColour;
};
typedef struct DisplayStructEx TbDisplayStructEx;

struct SSurface;
typedef struct SSurface TSurface;

/******************************************************************************/


#pragma pack()
/******************************************************************************/
extern volatile TbBool lbScreenInitialised;
extern volatile TbBool lbUseSdk;
extern volatile TbBool lbInteruptMouse;
extern volatile TbDisplayStructEx lbDisplayEx;
extern unsigned char lbPalette[PALETTE_SIZE];

#define DEFAULT_UI_SCALE                       128 // is equivilent to size 1 or 100%
#define DEFAULT_ASPECT_RATIO_FACTOR            160 // is equivilent to 16/10 * 100
#define DEFAULT_FIRST_PERSON_HORIZONTAL_FOV     94 // 94 degrees at 16/10 aspect ratio
#define DEFAULT_FIRST_PERSON_VERTICAL_FOV       68 // 68 degrees at 16/10 aspect ratio

enum UIScaleSettings {
    UI_NORMAL_SIZE = DEFAULT_UI_SCALE,
    UI_HALF_SIZE   = DEFAULT_UI_SCALE / 2,
    UI_DOUBLE_SIZE = DEFAULT_UI_SCALE * 2,
};

extern unsigned short units_per_pixel_width;
extern unsigned short units_per_pixel_height;
extern unsigned short units_per_pixel_menu_height;
extern unsigned short units_per_pixel_best;
extern unsigned short units_per_pixel_menu;
extern unsigned short units_per_pixel_landview;
extern unsigned short units_per_pixel_landview_frame;
extern unsigned short units_per_pixel_ui;
extern uint32_t aspect_ratio_factor_HOR_PLUS;
extern uint32_t aspect_ratio_factor_HOR_PLUS_AND_VERT_PLUS;
extern uint32_t first_person_horizontal_fov;
extern uint32_t first_person_vertical_fov;
extern uint32_t landview_frame_movement_scale_x;
extern uint32_t landview_frame_movement_scale_y;

extern unsigned short MyScreenWidth;
extern unsigned short MyScreenHeight;
extern unsigned short pixel_size;
extern unsigned short pixels_per_block;
extern unsigned short units_per_pixel;

extern unsigned short display_id;

extern TbDisplayStruct lbDisplay;
extern SDL_Window *lbWindow;
/******************************************************************************/
TbResult LbScreenInitialize(void);
TbResult LbScreenSetDoubleBuffering(TbBool state);
TbResult LbScreenSetup(TbScreenMode mode, TbScreenCoord width, TbScreenCoord height,
    unsigned char *palette, short buffers_count, TbBool wscreen_vid);
TbResult LbScreenReset(TbBool exiting_application);

TbBool LbScreenIsModeAvailable(TbScreenMode mode, unsigned short display);
TbScreenMode LbRecogniseVideoModeString(const char *desc);
TbScreenMode LbRegisterVideoMode(const char *desc, TbScreenCoord width, TbScreenCoord height,
    unsigned short bpp, uint32_t flags);
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

TbResult LbScreenSwap(void);
TbResult LbScreenClear(TbPixel colour);
TbResult LbScreenWaitVbi(void);
unsigned short LbGetCurrentDisplayIndex();

int32_t LbPaletteFade(unsigned char *pal, int32_t n, enum TbPaletteFadeFlag flg);
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

int32_t scale_value_for_resolution(int32_t base_value);
int32_t scale_value_for_resolution_with_upp(int32_t base_value, int32_t units_per_px);
int32_t scale_value_by_horizontal_resolution(int32_t base_value);
int32_t scale_value_by_vertical_resolution(int32_t base_value);
int32_t scale_ui_value_lofi(int32_t base_value);
int32_t scale_ui_value(int32_t base_value);
int32_t scale_fixed_DK_value(int32_t base_value);
int32_t scale_value_menu(int32_t base_value);
int32_t scale_value_landview(int32_t base_value);
void calculate_landview_upp(int32_t width, int32_t height, int32_t landview_width, int32_t landview_height);
TbBool is_ar_wider_than_original(int32_t width, int32_t height);
TbBool is_menu_ar_wider_than_original(int32_t width, int32_t height);
int32_t calculate_relative_upp(int32_t base_length, int32_t reference_upp, int32_t reference_length);
int32_t resize_ui(int32_t units_per_px, int32_t ui_scale);
void calculate_aspect_ratio_factor(int32_t width, int32_t height);
int32_t scale_fixed_DK_value_by_ar(int32_t base_value, TbBool scale_up, TbBool vert_plus);
int32_t FOV_based_on_aspect_ratio(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
