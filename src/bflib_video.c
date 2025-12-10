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
 *     This is SDL-based implementation of the video routines.
 * @author   Tomasz Lis
 * @date     11 Feb 2008 - 10 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_video.h"

#include "bflib_mouse.h"
#include "bflib_render.h"
#include "bflib_sprfnt.h"
#include "bflib_vidsurface.h"

#include "keeperfx.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <math.h>
#include "post_inc.h"

#define SCREEN_MODES_COUNT 40

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Global variables
/** List of registered video modes. */
TbScreenModeInfo lbScreenModeInfo[SCREEN_MODES_COUNT];
/** Count of used entries in registered video modes list. */
long lbScreenModeInfoNum = 0;

/** Informs if Video Screen subsystem initialization was done. */
volatile TbBool lbScreenInitialised = false;
/** Bytes per pixel expected by the engine.
 * On any try of entering different video BPP, this mode will be emulated. */
volatile unsigned short lbEngineBPP = 8;
/** Informs if the application window is active (focused on screen). */
extern volatile TbBool lbAppActive;
/** True if we have two surfaces. */
volatile TbBool lbHasSecondSurface;
/** True if we request the double buffering to be on in next mode switch. */
TbBool lbDoubleBufferingRequested;
/** Name of the video driver to be used. Must be set before LbScreenInitialize().
 * Under Win32 and with SDL, choises are windib or directx. */
/** Colour palette buffer, to be used inside lbDisplay. */
unsigned char lbPalette[PALETTE_SIZE];
/** Driver-specific colour palette buffer. */
SDL_Color lbPaletteColors[PALETTE_COLORS];

char lbDrawAreaTitle[128] = "Bullfrog Shell";
volatile TbBool lbInteruptMouse;
volatile unsigned long lbIconIndex = 0;
SDL_Window *lbWindow = NULL;

TbDisplayStruct lbDisplay;


unsigned short MyScreenWidth;
unsigned short MyScreenHeight;
unsigned short pixel_size;
unsigned short pixels_per_block;
unsigned short units_per_pixel;

/**
  * The id number of the current display that the game renders to, defaults to 0.
  * id 0 is the first (or only) screen, id 1 is the second screen, etc.
  *
  * The display number can be set in cfg file (as DISPLAY_NUMBER), display number = display id + 1.
  * Screen number 1 is the first (or only screen), etc.
  */
unsigned short display_id = 0;

static unsigned char fade_started;
static unsigned char from_pal[PALETTE_SIZE];
static unsigned char to_pal[PALETTE_SIZE];
static long fade_count;

/******************************************************************************/
void *LbExeReferenceNumber(void)
{
  return NULL;
}

/** Locks the graphics screen.
 *  This function gives access to the WScreen pointer, which contains buffer
 *  of size GraphicsScreenWidth x GraphicsScreenHeight.
 *  It also allows accessing GraphicsWindowPtr buffer, of size
 *  GraphicsWindowWidth x GraphicsWindowHeight, but with pitch (scanline length)
 *   same as graphics screen (which is GraphicsScreenWidth).
 *
 * @return Lb_SUCCESS if the lock was successful.
 * @see LbScreenUnlock()
 */
TbResult LbScreenLock(void)
{
    SYNCDBG(12,"Starting");
    if (!lbScreenInitialised)
        return Lb_FAIL;

    if (SDL_LockSurface(lbDrawSurface) < 0) {
        lbDisplay.GraphicsWindowPtr = NULL;
        lbDisplay.WScreen = NULL;
        return Lb_FAIL;
    }

    lbDisplay.WScreen = (unsigned char *) lbDrawSurface->pixels;
    lbDisplay.GraphicsScreenWidth = lbDrawSurface->pitch;
    lbDisplay.GraphicsWindowPtr = &lbDisplay.WScreen[lbDisplay.GraphicsWindowX +
        lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsWindowY];
    return Lb_SUCCESS;
}

TbResult LbScreenUnlock(void)
{
    SYNCDBG(12,"Starting");
    if (!lbScreenInitialised)
        return Lb_FAIL;
    lbDisplay.WScreen = NULL;
    lbDisplay.GraphicsWindowPtr = NULL;
    SDL_UnlockSurface(lbDrawSurface);
    return Lb_SUCCESS;
}

TbResult LbScreenSwap(void)
{
    int blresult;
    SYNCDBG(12,"Starting");
    TbResult ret = LbMouseOnBeginSwap();
    // Put the data from Draw Surface onto Screen Surface
    if ((ret == Lb_SUCCESS) && (lbHasSecondSurface)) {
        // Update pointer to window surface on every frame
        // to avoid problems with alt tab
        lbScreenSurface = SDL_GetWindowSurface(lbWindow);
        blresult = SDL_BlitSurface(lbDrawSurface, NULL, lbScreenSurface, NULL);
        if (blresult < 0) {
            ERRORLOG("Blit failed: %s",SDL_GetError());
            ret = Lb_FAIL;
        }
    }
    // Flip the image displayed on Screen Surface
    if (ret == Lb_SUCCESS) {
        // calls SDL_UpdateRect for entire screen if not double buffered
        blresult = SDL_UpdateWindowSurface(lbWindow);
        if (blresult < 0) {
            // In some cases this situation seems to be quite common
            ERRORDBG(11,"Flip failed: %s",SDL_GetError());
            ret = Lb_FAIL;
        }
    }
    LbMouseOnEndSwap();
    return ret;
}

TbResult LbScreenClear(TbPixel colour)
{
    SYNCDBG(12,"Starting");
    if ((!lbScreenInitialised) || (lbDrawSurface == NULL))
      return Lb_FAIL;
    if (SDL_FillRect(lbDrawSurface, NULL, colour) < 0) {
        ERRORLOG("Error while clearing screen.");
        return Lb_FAIL;
    }
  return Lb_SUCCESS;
}

/** Returns the currently active screen mode.
 *
 * @return Screen mode index.
 */
TbScreenMode LbScreenActiveMode(void)
{
    return lbDisplay.ScreenMode;
}

/** Color depth for the Graphics Screen.
 *  Gives BPP of the graphics canvas buffer. This value
 *  may differ from BPP used by Video Driver.
 *
 * @return Graphics canvas Bits Per Pixel, in bits.
 */
unsigned short LbGraphicsScreenBPP(void)
{
    if (lbDrawSurface != NULL) {
        return lbDrawSurface->format->BitsPerPixel;
    }
    // On error, return 0
    return 0;
    // Old way - returns video BPP, not graphics BPP
    // TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(lbDisplay.ScreenMode);
    // return mdinfo->BitsPerPixel;
}

TbScreenCoord LbGraphicsScreenWidth(void)
{
    return lbDisplay.GraphicsScreenWidth;
}

TbScreenCoord LbGraphicsScreenHeight(void)
{
    return lbDisplay.GraphicsScreenHeight;
}

/** Resolution in width of the current video mode.
 *  Note that it's not always "physical" size,
 *  and it definitely can't be used as pitch/scanline
 *  (size of data for one line) in the graphics buffer.
 *
 *  But it is the width that will be visible on screen.
 *
 * @return
 */
TbScreenCoord LbScreenWidth(void)
{
    return lbDisplay.PhysicalScreenWidth;
}

TbScreenCoord LbScreenHeight(void)
{
    return lbDisplay.PhysicalScreenHeight;
}

TbResult LbPaletteFadeStep(unsigned char *from_palette,unsigned char *to_palette,long fade_steps)
{
    unsigned char palette[PALETTE_SIZE];
    for (int i = 0; i < 3 * PALETTE_COLORS; i += 3)
    {
        int target_color_component = to_palette[i + 0];
        int source_color_component = from_palette[i + 0];
        palette[i+0] = fade_count * (target_color_component - source_color_component) / fade_steps + source_color_component;
        target_color_component =   to_palette[i+1];
        source_color_component = from_palette[i+1];
        palette[i+1] = fade_count * (target_color_component - source_color_component) / fade_steps + source_color_component;
        target_color_component =   to_palette[i+2];
        source_color_component = from_palette[i+2];
        palette[i+2] = fade_count * (target_color_component - source_color_component) / fade_steps + source_color_component;
    }
    LbScreenWaitVbi();
    TbResult ret = LbPaletteSet(palette);
    if (lbHasSecondSurface)
        LbScreenSwap();
    return ret;
}

TbResult LbPaletteStopOpenFade(void)
{
    fade_started = 0;
    return Lb_SUCCESS;
}

long LbPaletteFade(unsigned char *pal, long fade_steps, enum TbPaletteFadeFlag flg)
{
    long errors_num = 0;
    if (flg == Lb_PALETTE_FADE_CLOSED)
    {
        // Finish the fading fast
        LbPaletteGet(from_pal);
        if (pal == NULL)
        {
          pal = to_pal;
          LbPaletteDataFillBlack(to_pal);
        }
        fade_count = 0;
        do
        {
            if (LbPaletteFadeStep(from_pal,pal,fade_steps) == Lb_FAIL)
                errors_num++;
            fade_count++;
        }
        while (fade_count <= fade_steps);
        fade_started = false;
        return fade_count;
    }
    if (fade_started)
    {
        fade_count++;
        if (fade_count >= fade_steps)
          fade_started = false;
        if (pal == NULL)
          pal = to_pal;
    } else
    {
        fade_count = 0;
        fade_started = true;
        LbPaletteGet(from_pal);
        if (pal == NULL)
        {
            LbPaletteDataFillBlack(to_pal);
            pal = to_pal;
        }
    }
    if (LbPaletteFadeStep(from_pal,pal,fade_steps) == Lb_FAIL)
        errors_num++;
    if (errors_num > 0) {
#ifdef __DEBUG
        LbWarnLog("%s: There were errors while fading\n",__func__);
#endif
    }
    return fade_count;
}

/** Wait for vertical blanking interval.
 *
 * @return Lb_SUCCESS if wait successful.
 */
TbResult LbScreenWaitVbi(void)
{
  return Lb_SUCCESS;
}

/** Get the display id that the game is currently rendering to, or the default if there is no game window. Uses SDL2. */
unsigned short LbGetCurrentDisplayIndex()
{
    unsigned short current_display_id = display_id; // default to the already set display_id
    if (lbWindow != NULL)
    {
        int ret = SDL_GetWindowDisplayIndex(lbWindow); // Get the screen that the game is currently running on (if it is running)
        if (ret >= 0)
        {
            current_display_id = (unsigned short)ret; // update the record of the current display that keeperfx is rendering to
        }
        else
        {
            ERRORLOG("SDL_GetWindowDisplayIndex failed: %s", SDL_GetError());
        }
    }
    return current_display_id;
}

/** Check if a given mode is available on the current display, and set its Available field to TRUE if it is. */
static TbBool LbHwCheckIsModeAvailable(TbScreenMode mode, unsigned short display)
{
    TbScreenModeInfo* mdinfo = LbScreenGetModeInfo(mode);
    mdinfo->Available = false;
    mdinfo->window_pos_x = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    mdinfo->window_pos_y = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    // if this is window mode
    if (mdinfo->VideoFlags & Lb_VF_WINDOWED)
    {
        if (mdinfo->VideoFlags & Lb_VF_BORDERLESS)
        {
            // bordeless window - do nothing
        }
        else
        {
            // normal bordered window - do nothing
        }
    }
    // else if this is fill all screens mode
    else if (mdinfo->VideoFlags & Lb_VF_FILLALL)
    {
        // We actually need to setup this mode now, as it is flexible to the user's setup
        int top = 0;
        int left = 0;
        int bottom = 0;
        int right = 0;
        //int screenArray[columnCount*rowCount] = {1, 0, 2};
        mdinfo->Width = mdinfo->Height = mdinfo->window_pos_x = mdinfo->window_pos_y = 0;
        // Get the total number of displays
        int numDisplays = SDL_GetNumVideoDisplays();
        if (numDisplays <= 0)
        {
            ERRORLOG("SDL_GetNumVideoDisplays failed: %s", SDL_GetError());
            return false; // for some reason we can't get the number of displays!
        }
        for (int d = 0; d < numDisplays; d++)
        {
            SDL_Rect rect = {0, 0, 0, 0};
            if (SDL_GetDisplayBounds(d, &rect) != 0)
            {
                ERRORLOG("SDL_GetDisplayBounds failed: %s", SDL_GetError());
                return false; // for some reason we can't get the current display bounds!
            }
            left = min(left, rect.x);
            top = min(top, rect.y);
            right = max(right, rect.x + rect.w);
            bottom = max(bottom, rect.y + rect.h);
        }
        mdinfo->Width = abs(left - right);
        mdinfo->Height = abs(top - bottom);
        if ((mdinfo->Width == 0) || (mdinfo->Height == 0))
        {
            ERRORLOG("no valid screens in FILLALL mode");
            return false;
        }
        mdinfo->window_pos_x = left;
        mdinfo->window_pos_y = top;
    }
    // else if this is the real or fake desktop fullscreen mode
    else if (mdinfo->VideoFlags & Lb_VF_DESKTOP)
    {
        // We actually need to setup this mode now, as it is flexible to the user's setup
        // Get current desktop display width and height (after DPI scaling)
        SDL_DisplayMode desktop;
        if (SDL_GetDesktopDisplayMode(display, &desktop) != 0)
        {
            ERRORLOG("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
            return false; // for some reason we can't get the current desktop resolution!
        }
        // update the mode's width and height to the desktop resolution of the current monitor
        mdinfo->Width = desktop.w;
        mdinfo->Height = desktop.h;
    }
    // else if this is a specific fullscreen mode
    else
    {
        // See if the desired fullscreen mode is a valid mode for the current display
        SDL_DisplayMode desired = {SDL_PIXELFORMAT_UNKNOWN, (int)mdinfo->Width, (int)mdinfo->Height, 0, 0}; // maybe there is a better/more accurate way to describe the display mode...
        SDL_DisplayMode closest = desired;
        if (SDL_GetClosestDisplayMode(display, &desired, &closest) == NULL)
        {
            return false; // all available fullscreen modes are too small for the desired mode to fit
        }
        if ((desired.w != closest.w) && (desired.h != closest.h))
        {
            return false; // desired fullscreen mode is not available (but a "close" match is)
        }
    }
    // desired screen mode must be valid if we get here
    mdinfo->Available = true; // redundant?
    return true;
}

/**
 * Registers standard VESA video modes.
 *
 * The modes have to registered to an empty list of modes, to make index of each mode
 * match the mode index in VESA standard.
 * Non-standard modes may be register later, getting mode numbers higher than standard modes.
 */
static void LbRegisterStandardVideoModes(void)
{
    lbScreenModeInfoNum = 0;
    LbRegisterVideoMode("INVALID",       0,    0,  0, Lb_VF_DEFAULT);
    LbRegisterVideoMode("320x200x8",   320,  200,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("320x200x16",  320,  200, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("320x200x24",  320,  200, 24, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("320x240x8",   320,  240,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("320x240x16",  320,  240, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("320x240x24",  320,  240, 24, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("512x384x8",   512,  384,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("512x384x16",  512,  384, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("512x384x24",  512,  384, 24, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("640x400x8",   640,  400,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("640x400x16",  640,  400, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("640x400x24",  640,  400, 24, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("640x480x8",   640,  480,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("640x480x16",  640,  480, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("640x480x24",  640,  480, 24, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("800x600x8",   800,  600,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("800x600x16",  800,  600, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("800x600x24",  800,  600, 24, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("1024x768x8", 1024,  768,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("1024x768x16",1024,  768, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("1024x768x24",1024,  768, 24, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("1280x1024x8", 1280,1024,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("1280x1024x16",1280,1024, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("1280x1024x24",1280,1024, 24, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("1600x1200x8", 1600,1200,  8, Lb_VF_PALETTE);
    LbRegisterVideoMode("1600x1200x16",1600,1200, 16, Lb_VF_TRUCOLOR);
    LbRegisterVideoMode("1600x1200x24",1600,1200, 24, Lb_VF_RGBCOLOR);
}

/**
 * Registers special video modes that allow us to work in a modern environment (Uses SDL2).
 *
 * These are added to the end of list of VESA standard modes (see LbRegisterStandardVideoModes), as "custom modes".
 */
static void LbRegisterModernVideoModes(void)
{
    LbRegisterVideoMode("DESKTOP",      0, 0, 32, Lb_VF_RGBCOLOR|Lb_VF_BORDERLESS|Lb_VF_DESKTOP); // borderless fullscreen window mode
    LbRegisterVideoMode("DESKTOP_FULL", 0, 0, 32, Lb_VF_RGBCOLOR|Lb_VF_DESKTOP); // normal fullscreen mode (at desktop resolution)
    //LbRegisterVideoMode("WINDOW",       0, 0, 32, Lb_VF_RGBCOLOR|Lb_VF_WINDOWED); // normal bordered window at any resolution, remebers previous set size (defaults to 640x480?)
    //LbRegisterVideoMode("BORDERLESS",   0, 0, 32, Lb_VF_RGBCOLOR|Lb_VF_BORDERLESS|Lb_VF_WINDOWED); // borderless window at any resolution
    LbRegisterVideoMode("ALL",          0, 0, 32, Lb_VF_RGBCOLOR|Lb_VF_FILLALL); // span all displays with a borderless window
}

TbResult LbScreenInitialize(void)
{
    // Clear global variables
    lbScreenInitialised = false;
    lbScreenSurface = NULL;
    lbDrawSurface = NULL;
    lbHasSecondSurface = false;
    lbDoubleBufferingRequested = false;
    lbAppActive = true;
    LbMouseChangeMoveRatio(256, 256);
    // Register default video modes
    if (lbScreenModeInfoNum == 0) {
        LbRegisterStandardVideoModes();
        LbRegisterModernVideoModes(); // register modern and flexible custom modes
    }
    // Initialize SDL library
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0) {
        ERRORLOG("SDL init: %s",SDL_GetError());
        return Lb_FAIL;
    }
    // Setup the atexit() call to un-initialize
    atexit(SDL_Quit);
    return Lb_SUCCESS;
}

/** Set up the window, render surface, etc. Called when we want to change the screen setup. Uses SDL2. */
TbResult LbScreenSetup(TbScreenMode mode, TbScreenCoord width, TbScreenCoord height,
    unsigned char *palette, short buffers_count, TbBool wscreen_vid)
{
    int32_t hot_x;
    int32_t hot_y;
    const struct TbSprite* msspr = NULL;
    LbExeReferenceNumber();
    if (lbDisplay.MouseSprite != NULL)
    {
        msspr = lbDisplay.MouseSprite;
        GetPointerHotspot(&hot_x,&hot_y);
    }
    SDL_Surface* prevScreenSurf = lbScreenSurface;
    LbMouseChangeSprite(NULL);

    if (lbHasSecondSurface) {
        SDL_FreeSurface(lbDrawSurface);
    }
    lbDrawSurface = NULL;
    lbScreenInitialised = false;

    if (prevScreenSurf != NULL) {
    }

    TbScreenModeInfo* mdinfo = LbScreenGetModeInfo(mode); // The desired mode has already been checked
    // Note:
    // We set the window's fullscreen state (either Window, Fullscreen, or Fake Fullscreen)
    // We set the DisplayMode for real fullscreen mode
    // We set the window size, position, and border for a window
    // The FILL ALL mode is a borderless window
    if (lbWindow != NULL)
    {
        Uint32 current_fullscreen_flags = SDL_GetWindowFlags(lbWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP; // SDL_WINDOW_FULLSCREEN is already flagged in SDL_WINDOW_FULLSCREEN_DESKTOP
        Uint32 new_fullscreen_flags = mdinfo->sdlFlags & SDL_WINDOW_FULLSCREEN_DESKTOP;
        // If the new mode is a real fullscreen mode, then set the new mode
        if (new_fullscreen_flags == SDL_WINDOW_FULLSCREEN)
        {
            SDL_DisplayMode dm = { SDL_PIXELFORMAT_UNKNOWN, (int)mdinfo->Width, (int)mdinfo->Height, 0, 0}; // this works in a modern setting (we get WxH at 32 bpp), but I'm not sure if this provides true 8-bit color mode (e.g. if we request 320x200x8 mode)
            if (SDL_SetWindowDisplayMode(lbWindow, &dm) != 0) // set display mode for fullscreen
            {
                ERRORLOG("SDL_SetWindowDisplayMode failed for mode %d (%s): %s", (int)mode, mdinfo->Desc, SDL_GetError());
                return Lb_FAIL;
            }
            // If we change to a fullscreen mode that is a higher res than the previous fullscreen mode (after having already changed
            // to a normal window/fake fullscreen window at some point in the past), then the result is a small window in the
            // top left of the screen, or potentially the buffer does not fill the whole mode's width/height (I don't know these things).
            // The above seems to be this SDL issue: https://github.com/libsdl-org/SDL/issues/3869
            // said issue was supposedly fixed in https://github.com/libsdl-org/SDL/pull/4392
            // but that is either not the case, or said pull has been reverted (I cannot find evidence of it in the sdl2 codebase).
            // The issue is fixed by running the following line (after SDL_SetWindowDisplayMode above):
            SDL_SetWindowSize(lbWindow, mdinfo->Width, mdinfo->Height);
        }
        // If mode has changed between fullscreen/windowed/fake fullscreen, set the new mode
        if (current_fullscreen_flags != new_fullscreen_flags)
        {
            if (SDL_SetWindowFullscreen(lbWindow, new_fullscreen_flags) != 0)
            {
                ERRORLOG("SDL_SetWindowFullscreen failed for mode %d (%s): %s", (int)mode, mdinfo->Desc, SDL_GetError());
                return Lb_FAIL;
            }
        }
        SDL_SetWindowBordered(lbWindow, (mdinfo->sdlFlags & SDL_WINDOW_BORDERLESS) ? SDL_FALSE : SDL_TRUE);
        // if the new mode is windowed mode (including the special FILL ALL mode)
        if (new_fullscreen_flags == 0)
        {
            SDL_SetWindowSize(lbWindow, mdinfo->Width, mdinfo->Height);
            SDL_SetWindowPosition(lbWindow, mdinfo->window_pos_x, mdinfo->window_pos_y);
        }
    }
    // If the game window doesn't yet exists
    if (lbWindow == NULL) {
        lbWindow = SDL_CreateWindow(lbDrawAreaTitle, mdinfo->window_pos_x, mdinfo->window_pos_y, mdinfo->Width, mdinfo->Height, mdinfo->sdlFlags);
    }
    if (lbWindow == NULL) {
        ERRORLOG("SDL_CreateWindow failed for mode %d (%s): %s", (int)mode, mdinfo->Desc, SDL_GetError());
        return Lb_FAIL;
    }
    lbScreenSurface = lbDrawSurface = SDL_GetWindowSurface( lbWindow );
    if (lbScreenSurface == NULL) {
        ERRORLOG("Failed to initialize mode %d (%s): %s", (int)mode, mdinfo->Desc, SDL_GetError());
        return Lb_FAIL;
    }

    // Create secondary surface if necessary, that is if BPP != lbEngineBPP.
    if (mdinfo->BitsPerPixel != lbEngineBPP)
    {
        lbDrawSurface = SDL_CreateRGBSurface(0, mdinfo->Width, mdinfo->Height, lbEngineBPP, 0, 0, 0, 0);
        if (lbDrawSurface == NULL) {
            ERRORLOG("Can't create secondary surface for mode %d (%s): %s", (int)mode, mdinfo->Desc, SDL_GetError());
            LbScreenReset(false);
            return Lb_FAIL;
        }
        lbHasSecondSurface = true;
    }

    lbDisplay.DrawFlags = 0;
    lbDisplay.DrawColour = 0;
    lbDisplayEx.ShadowColour = 0;
    lbDisplay.PhysicalScreenWidth = mdinfo->Width;
    lbDisplay.PhysicalScreenHeight = mdinfo->Height;
    lbDisplay.ScreenMode = mode;
    lbDisplay.PhysicalScreen = NULL;
    // The graphics screen size should be really taken after screen is locked, but it seem just getting in now will work too
    lbDisplay.GraphicsScreenWidth = lbDrawSurface->pitch;
    lbDisplay.GraphicsScreenHeight = mdinfo->Height;
    lbDisplay.WScreen = NULL;
    lbDisplay.GraphicsWindowPtr = NULL;
    lbScreenInitialised = true;
    SYNCLOG("Mode %dx%dx%d setup succeeded",(int)lbScreenSurface->w,(int)lbScreenSurface->h,(int)lbScreenSurface->format->BitsPerPixel);
    if (palette != NULL)
    {
        LbPaletteSet(palette);
    }
    LbScreenSetGraphicsWindow(0, 0, mdinfo->Width, mdinfo->Height);
    LbTextSetWindow(0, 0, mdinfo->Width, mdinfo->Height);
    SYNCDBG(8,"Done filling display properties struct");
    if ( LbMouseIsInstalled() )
    {
        LbMouseSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
        if (msspr != NULL)
        {
          LbMouseChangeSpriteAndHotspot(msspr, hot_x, hot_y);
        }
        if (!IsMouseInsideWindow())
        {
            SDL_WarpMouseInWindow(lbWindow, mdinfo->Width / 2, mdinfo->Height / 2);
        }
    }

    reset_bflib_render();

    redetect_screen_refresh_rate_for_draw();

    SYNCDBG(8,"Finished");
    return Lb_SUCCESS;
}

/** Clears the 8-bit video palette with black colour.
 * Only writes values to given palette bufer - does no screen operations.
 *
 * @param palette Pointer to the palette colors data.
 * @return Lb_SUCCESS, or error code.
 */
TbResult LbPaletteDataFillBlack(unsigned char *palette)
{
    memset(palette, 0, PALETTE_SIZE);
    return Lb_SUCCESS;
}

/** Clears the 8-bit video palette with white colour.
 *
 * @param palette Pointer to the palette colors data.
 * @return Lb_SUCCESS, or error code.
 */
TbResult LbPaletteDataFillWhite(unsigned char *palette)
{
    memset(palette, 0x3F, PALETTE_SIZE);
    return Lb_SUCCESS;
}

/** Sets the 8-bit video palette.
 *
 * @param palette Pointer to the palette colors data.
 * @return Lb_SUCCESS, or error code.
 */
TbResult LbPaletteSet(unsigned char *palette)
{
    SYNCDBG(12,"Starting");
    if ((!lbScreenInitialised) || (lbDrawSurface == NULL))
      return Lb_FAIL;
    //destColors = (SDL_Color *) malloc(sizeof(SDL_Color) * PALETTE_COLORS);
    SDL_Color* destColors = lbPaletteColors;
    const unsigned char* srcColors = palette;
    unsigned char* bufColors = lbPalette;
    if ((destColors == NULL) || (srcColors == NULL))
      return Lb_FAIL;
    TbResult ret = Lb_SUCCESS;
    for (unsigned long i = 0; i < PALETTE_COLORS; i++)
    {
        // note that bufColors and srcColors could be the same pointer
        bufColors[0] = srcColors[0] & 0x3F;
        bufColors[1] = srcColors[1] & 0x3F;
        bufColors[2] = srcColors[2] & 0x3F;
        destColors[i].r = (bufColors[0] << 2);
        destColors[i].g = (bufColors[1] << 2);
        destColors[i].b = (bufColors[2] << 2);
        srcColors += 3;
        bufColors += 3;
    }
    //if (SDL_SetPalette(lbDrawSurface, SDL_LOGPAL | SDL_PHYSPAL,
    SDL_SetPaletteColors(lbDrawSurface->format->palette, lbPaletteColors, 0, PALETTE_COLORS);
    //free(destColors);
    lbDisplay.Palette = lbPalette;
    return ret;
}

/** Retrieves the 8-bit video palette.
 *
 * @param palette Pointer to target palette colors buffer.
 * @return Lb_SUCCESS, or error code.
 */
TbResult LbPaletteGet(unsigned char *palette)
{
    SYNCDBG(12,"Starting");
    if ((!lbScreenInitialised) || (lbDrawSurface == NULL))
      return Lb_FAIL;
    if (lbDisplay.Palette == NULL)
        return Lb_FAIL;
    memcpy(palette,lbDisplay.Palette,PALETTE_SIZE);
/*  // Getting the palette in SDL way may sometimes lead to problems.
    // Instead, we will remember palette which was set the last time.
    //
    const SDL_Color * srcColors;
    unsigned char * destColors;
    unsigned long i;
    unsigned long colours_num;
    colours_num = lbDrawSurface->format->palette->ncolors;
    if (colours_num > PALETTE_COLORS) {
        colours_num = PALETTE_COLORS;
    } else
    if (colours_num < PALETTE_COLORS) {
        memset(palette,0,PALETTE_SIZE);
    }
    srcColors = lbDrawSurface->format->palette->colors;
    destColors = palette;
    for (i = 0; i < colours_num; i++) {
        destColors[0] = (srcColors[i].r >> 2);
        destColors[1] = (srcColors[i].g >> 2);
        destColors[2] = (srcColors[i].b >> 2);
        destColors += 3;
    }*/
    return Lb_SUCCESS;
}

TbResult LbSetTitle(const char *title)
{
    snprintf(lbDrawAreaTitle, sizeof(lbDrawAreaTitle), "%s", title);
    return Lb_SUCCESS;
}

TbResult LbSetIcon(unsigned short nicon)
{
    lbIconIndex = nicon;
    return Lb_SUCCESS;
}

TbScreenModeInfo *LbScreenGetModeInfo(TbScreenMode mode)
{
    if (mode < lbScreenModeInfoNum)
        return &lbScreenModeInfo[mode];
    return &lbScreenModeInfo[0];
}

TbBool LbScreenIsLocked(void)
{
    return (lbDisplay.WScreen != NULL);
}

TbResult LbScreenReset(TbBool exiting_application)
{
    if (!lbScreenInitialised)
      return Lb_FAIL;
    LbMouseChangeSprite(NULL);
    if (lbHasSecondSurface) {
        SDL_FreeSurface(lbDrawSurface);
    }
    //do not free screen surface, it is freed automatically on SDL_Quit or next call to set video mode
    lbHasSecondSurface = false;
    lbDrawSurface = NULL;
    lbScreenSurface = NULL;
    // Mark as not initialized
    lbScreenInitialised = false;
    if (exiting_application)
    {
        // we get here when we are actually closing the application
        finish_bflib_render();
    }
    return Lb_SUCCESS;
}

/**
 * Stores the current graphics window coords into TbGraphicsWindow structure.
 * Intended to use with LbScreenLoadGraphicsWindow() when changing the window
 * temporary.
 */
TbResult LbScreenStoreGraphicsWindow(TbGraphicsWindow *grwnd)
{
  grwnd->x = lbDisplay.GraphicsWindowX;
  grwnd->y = lbDisplay.GraphicsWindowY;
  grwnd->width = lbDisplay.GraphicsWindowWidth;
  grwnd->height = lbDisplay.GraphicsWindowHeight;
  grwnd->ptr = NULL;
  return Lb_SUCCESS;
}

/**
 * Sets the current graphics window coords from those in TbGraphicsWindow structure.
 * Use it only with TbGraphicsWindow which was filled using function
 * LbScreenStoreGraphicsWindow(), because the values are not checked for sanity!
 * To set values from other sources, use LbScreenSetGraphicsWindow() instead.
 */
TbResult LbScreenLoadGraphicsWindow(TbGraphicsWindow *grwnd)
{
  lbDisplay.GraphicsWindowX = grwnd->x;
  lbDisplay.GraphicsWindowY = grwnd->y;
  lbDisplay.GraphicsWindowWidth = grwnd->width;
  lbDisplay.GraphicsWindowHeight = grwnd->height;
  if (lbDisplay.WScreen != NULL)
  {
      lbDisplay.GraphicsWindowPtr = lbDisplay.WScreen
        + lbDisplay.GraphicsScreenWidth*lbDisplay.GraphicsWindowY + lbDisplay.GraphicsWindowX;
  } else
  {
      lbDisplay.GraphicsWindowPtr = NULL;
  }
  return Lb_SUCCESS;
}

TbResult LbScreenSetGraphicsWindow(long x, long y, long width, long height)
{
    long i;
    long right_edge = x + width;
    long bottom_edge = y + height;
    if (right_edge < x)  //Alarm! Voodoo magic detected!
    {
        i = (x ^ right_edge);
        x = x ^ i;
        right_edge = x ^ i ^ i;
  }
  if (bottom_edge < y)
  {
    i = (y^bottom_edge);
    y = y^i;
    bottom_edge = y^i^i;
  }
  if (x < 0)
    x = 0;
  if (right_edge < 0)
    right_edge = 0;
  if (y < 0)
    y = 0;
  if (bottom_edge < 0)
    bottom_edge = 0;
  if (x > lbDisplay.GraphicsScreenWidth)
    x = lbDisplay.GraphicsScreenWidth;
  if (right_edge > lbDisplay.GraphicsScreenWidth)
    right_edge = lbDisplay.GraphicsScreenWidth;
  if (y > lbDisplay.GraphicsScreenHeight)
    y = lbDisplay.GraphicsScreenHeight;
  if (bottom_edge > lbDisplay.GraphicsScreenHeight)
    bottom_edge = lbDisplay.GraphicsScreenHeight;
  lbDisplay.GraphicsWindowX = x;
  lbDisplay.GraphicsWindowY = y;
  lbDisplay.GraphicsWindowWidth = right_edge - x;
  lbDisplay.GraphicsWindowHeight = bottom_edge - y;
  if (lbDisplay.WScreen != NULL)
  {
    lbDisplay.GraphicsWindowPtr = lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth*y + x;
  } else
  {
    lbDisplay.GraphicsWindowPtr = NULL;
  }
  return Lb_SUCCESS;
}

TbBool LbScreenIsModeAvailable(TbScreenMode mode, unsigned short display)
{
  if (mode == Lb_SCREEN_MODE_INVALID)
  {
    return false;
  }
  if (!LbHwCheckIsModeAvailable(mode, display))
  {
    TbScreenModeInfo* mdinfo = LbScreenGetModeInfo(mode);
    ERRORLOG("%s resolution %dx%d (mode %d) not available",
            (mdinfo->VideoFlags&Lb_VF_WINDOWED)?"Windowed":"Full screen",
            (int)mdinfo->Width,(int)mdinfo->Height,(int)mode);
    return false;
  }
  return true;
}

/** Allows to change recommended state of double buffering function.
 *  Double buffering is a technique where two graphics surfaces are used,
 *  and every screen redraw (flip) switches the primary and secondary surface.
 *  This may produce smoother motion on some platforms, but it forces
 *  the screen to be redrawn completely after each switch - if only
 *  changed areas were to be updated, they would have to be updated on both
 *  surfaces.
 *
 * @param state Recommended state of Double Buffering in next video mode switch.
 * @return Lb_SUCCESS if the request has been noted and stored.
 */
TbResult LbScreenSetDoubleBuffering(TbBool state)
{
    lbDoubleBufferingRequested = state;
    return Lb_SUCCESS;
}

TbScreenMode LbRecogniseVideoModeString(const char *desc)
{
    for (int mode = 0; mode < lbScreenModeInfoNum; mode++)
    {
      if (strcasecmp(lbScreenModeInfo[mode].Desc,desc) == 0)
        return (TbScreenMode)mode;
    }
    return Lb_SCREEN_MODE_INVALID;
}

TbScreenMode LbRegisterVideoMode(const char *desc, TbScreenCoord width, TbScreenCoord height,
    unsigned short bpp, unsigned long flags)
{
    TbScreenModeInfo *mdinfo;
    TbScreenMode mode = LbRecogniseVideoModeString(desc);
    if (mode != Lb_SCREEN_MODE_INVALID)
    {
        mdinfo = &lbScreenModeInfo[mode];
        if ((mdinfo->Width == width) && (mdinfo->Height == height) && (mdinfo->BitsPerPixel == bpp))
        {
            // Mode is already registered
            return mode;
        }
        // Mode with same name but different params is registered
#ifdef __DEBUG
        LbWarnLog("%s: Mode with same name but different params is registered, cannot register %dx%dx%d\n",__func__, (int)width, (int)height, (int)bpp);
#endif
        return Lb_SCREEN_MODE_INVALID;
    }
    if ((size_t) lbScreenModeInfoNum >= sizeof(lbScreenModeInfo)/sizeof(lbScreenModeInfo[0]))
    {
        // No free mode slots
        return Lb_SCREEN_MODE_INVALID;
    }
    // Insert new mode to array
    mode = lbScreenModeInfoNum;
    lbScreenModeInfoNum++;
    mdinfo = &lbScreenModeInfo[mode];
    // Fill the mode content
    memset(mdinfo, 0, sizeof(TbScreenModeInfo));
    mdinfo->Width = width;
    mdinfo->Height = height;
    mdinfo->BitsPerPixel = bpp;
    mdinfo->Available = false;
    mdinfo->VideoFlags = flags;
    // SDL flags
    mdinfo->sdlFlags = 0; // default to an normal window
    if ((mdinfo->VideoFlags & Lb_VF_WINDOWED))
    {
        if (mdinfo->VideoFlags & Lb_VF_BORDERLESS)
        {
            mdinfo->sdlFlags |= SDL_WINDOW_BORDERLESS; // borderless window
        }
        /* else
        {
            sdlFlags |= SDL_WINDOW_RESIZABLE; // todo (allow window to be freely scaled) - needs window resize function triggered by SDL_WINDOWEVENT_SIZE_CHANGED
        } */
    }
    else
    {
        if (mdinfo->VideoFlags & Lb_VF_FILLALL)
        {
            mdinfo->sdlFlags |= SDL_WINDOW_BORDERLESS; // fill all displays with fake fullscreen
        }
        else if (mdinfo->VideoFlags & Lb_VF_BORDERLESS)
        {
            mdinfo->sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP; // fake fullscreen
        }
        else
        {
            mdinfo->sdlFlags |= SDL_WINDOW_FULLSCREEN; // real fullscreen
        }
    }
    snprintf(mdinfo->Desc, sizeof(mdinfo->Desc), "%s", desc);
    return mode;
}

/**
 * Parses video mode description string and registers new mode.
 *
 * @param desc Mode description string, in form of WIDTHxHEIGHTxBPP or WIDTHxHEIGHTwBPP.
 * @return Mode number assigned to the newly created video mode.
 */
TbScreenMode LbRegisterVideoModeString(const char *desc)
{
    int width;
    int height;
    int bpp;
    unsigned long flags;
    int ret = 0;
    // check for the special "span all displays" mode
    if (strncasecmp(desc, "ALL", 3) == 0)
    {
        return LbRecogniseVideoModeString("ALL");
    }
    // check for the special real fullscreen mode
    if (strncasecmp(desc, "DESKTOP_FULL", 12) == 0)
    {
        return LbRecogniseVideoModeString("DESKTOP_FULL");
    }
    // check for the special fake fullscreen mode
    if (strncasecmp(desc, "DESKTOP", 7) == 0)
    {
        return LbRecogniseVideoModeString("DESKTOP");
    }
    // modern patterns not matched - maybe it's fullscreen mode
    width = 0; height = 0; bpp = 0; flags = Lb_VF_DEFAULT;
    ret = sscanf(desc," %d x %d x %d", &width, &height, &bpp);
    if (ret != 3)
    {
        // pattern not matched - maybe it's windowed mode
        width = 0; height = 0; bpp = 0; flags = Lb_VF_DEFAULT;
        ret = sscanf(desc," %d x %d w %d", &width, &height, &bpp);
        flags |= Lb_VF_WINDOWED;
    }
    if (ret != 3)
    {
        // Cannot recognize parameters in mode
#ifdef __DEBUG
        LbWarnLog("%s: Cannot recognize parameters in mode, got %dx%dx%d\n",__func__, width, height, bpp);
#endif
        return Lb_SCREEN_MODE_INVALID;
    }
    if (bpp < 9) {
        flags |= Lb_VF_PALETTE;
    } else
    if ((bpp == 24) || (bpp = 32)) {
        flags |= Lb_VF_RGBCOLOR;
    } else
    {
        flags |= Lb_VF_TRUCOLOR;
    }
    return LbRegisterVideoMode(desc, width, height, bpp, flags);
}

TbPixel LbPaletteFindColour(const unsigned char *pal, unsigned char r, unsigned char g, unsigned char b)
{
    int i;
    // Compute minimal square difference in color; return exact match if found
    int min_delta = 999999;
    const unsigned char* c = pal;
    for (i = 0; i < 256; i++)
    {
        int dr = (r - c[0]) * (r - c[0]);
        int dg = (g - c[1]) * (g - c[1]);
        int db = (b - c[2]) * (b - c[2]);
        if (min_delta > dr+dg+db)
        {
            min_delta = dr+dg+db;
            if (min_delta == 0) {
                return i;
            }
        }
        c += 3;
    }
    // Gather all the colors with minimal square difference
    int n = 0;
    unsigned char tmcol[256];
    unsigned char* o = tmcol;
    c = pal;
    for (i = 0; i < 256; i++)
    {
        int dr = (r - c[0]) * (r - c[0]);
        int dg = (g - c[1]) * (g - c[1]);
        int db = (b - c[2]) * (b - c[2]);
        if (min_delta == dr+dg+db)
        {
            n += 1;
            *o = i;
            o++;
        }
        c += 3;
    }
    // If there's only one left on list - return it
    if (n == 1) {
        return tmcol[0];
    }
    // Get minimal linear difference out of remaining colors
    min_delta = 999999;
    for (i = 0; i < n; i++)
    {
        c = &pal[3 * tmcol[i]];
        int dr = abs(r - c[0]);
        int dg = abs(g - c[1]);
        int db = abs(b - c[2]);
        if (min_delta > dr+dg+db) {
            min_delta = dr+dg+db;
        }
    }
    // Gather all the colors with minimal linear difference
    // Note that we may re-use tmcol array, because (i <= m)
    int m = 0;
    o = tmcol;
    for (i = 0; i < n; i++)
    {
        c = &pal[3 * tmcol[i]];
        int dr = abs(r - c[0]);
        int dg = abs(g - c[1]);
        int db = abs(b - c[2]);
        if (min_delta == dr+dg+db)
        {
            m += 1;
            *o = tmcol[i];
            o++;
        }
    }
    // If there's only one left on list - return it
    if (m == 1) {
        return tmcol[0];
    }
    // It's hard to select best color out of the left ones - use darker one with wages
    min_delta = 999999;
    o = &tmcol[0];
    for (i = 0; i < m; i++)
    {
        c = &pal[3 * tmcol[i]];
        int dr = (c[0] * c[0]);
        int dg = (c[1] * c[1]);
        int db = (c[2] * c[2]);
        if (min_delta > db+2*(dg+dr))
        {
          min_delta = db+2*(dg+dr);
          o = &tmcol[i];
        }
    }
    return *o;
}

/**
 * Takes a fixed value designed for 640x400 resolution and scales it to the game's current resolution
 * Returns a new integer value, calculated from base_value relative to ("current units_per_pixel" / "reference units_per_pixel")
 *
 * "reference units_per_pixel" is = 16, as this is the value of units_per_pixel at 640x400 or 640x480 mode
 *
 * If 640x400 mode   (units_per_pixel = 16) then this function returns (base_value * 1)
 * If 320x200 mode   (units_per_pixel =  8) then this function returns (base_value * 0.5)
 * If 1920x1080 mode (units_per_pixel = 48) then this function returns (base_value * 3)
 *
 * (!base_value should always be divisible by 2, for compatibility with 320x200 resolution!)
 *
 * @param base_value The fixed value from original DK 640x400 mode that needs to be scaled with the game's current resolution
 */
long scale_value_for_resolution(long base_value)
{
    // return value is equivalent to: round(base_value * units_per_pixel /16)
    long value = ((((units_per_pixel * base_value) >> 3) + (((units_per_pixel * base_value) >> 3) & 1)) >> 1);
    return max(1,value);
}

/**
 * Duplicates scale_value_for_resolution() except that this function:
 * is passed a units_per_px parameter, rather than using the global units_per_pixel
 *
 * @param base_value The fixed value from original DK 640x400 mode that needs to be scaled with the game's current resolution
 * @param units_per_px The current units_per_px value for the current resolution
 */
long scale_value_for_resolution_with_upp(long base_value, long units_per_px)
{
    long value = ((((units_per_px * base_value) >> 3) + (((units_per_px * base_value) >> 3) & 1)) >> 1);
    // return value is equivalent to: round(base_value * units_per_px /16)
    return max(1,value);
}

/**
 * Duplicates scale_value_for_resolution() except that this function:
 * uses units_per_pixel_width rather than using units_per_pixel
 *
 * @param base_value The fixed value from original DK 640x400 mode that needs to be scaled with the game's current horizontal resolution
 */
long scale_value_by_horizontal_resolution(long base_value)
{
    // return value is equivalent to: round(base_value * units_per_pixel_width /16)
    long value = ((((units_per_pixel_width * base_value) >> 3) + (((units_per_pixel_width * base_value) >> 3) & 1)) >> 1);
    return value;
}

/**
 * Duplicates scale_value_for_resolution() except that this function:
 * uses units_per_pixel_height rather than using units_per_pixel
 *
 * @param base_value The fixed value from original DK 640x400 mode that needs to be scaled with the game's current vertical resolution
 */
long scale_value_by_vertical_resolution(long base_value)
{
    // return value is equivalent to: round(base_value * units_per_pixel_height /16)
    long value = ((((units_per_pixel_height * base_value) >> 3) + (((units_per_pixel_height * base_value) >> 3) & 1)) >> 1);
    return value;
}

/**
 * Takes a fixed value tuned for original DK at 640x400 and scales it for the game's current resolution and UI scale.
 * Uses units_per_pixel_ui (which is 16 at 640x400)
 *
 * @param base_value The fixed value tuned for original DK 640x400 mode
 */
long scale_ui_value(long base_value)
{
    // return value is equivalent to: round(base_value * units_per_pixel_ui /16)
    long value = ((((units_per_pixel_ui * base_value) >> 3) + (((units_per_pixel_ui * base_value) >> 3) & 1)) >> 1);
    return value; // can return zero
}

/**
 * Sames as scale_ui_value, but if "lofi" detected, then scale is doubled
 *
 * @param base_value The fixed value tuned for original DK 640x400 mode
 */
long scale_ui_value_lofi(long base_value)
{
    TbBool lofi_mode = ((LbGraphicsScreenHeight() < 400) ? true : false);
    long value;
    if (lofi_mode)
    {
        value = scale_ui_value(base_value * 2);
    }
    else
    {
        value = scale_ui_value(base_value);
    }
    return value; // can return zero
}

/**
 * Takes a fixed value tuned for original DK at 640x400 and scales it for the game's current resolution.
 * If the screen is wider than 16:10 the height is used; if the screen is narrower than 16:10 the width is used.
 * Uses units_per_pixel_best (which is 16 at 640x400)
 *
 * @param base_value The fixed value tuned for original DK 640x400 mode
 */
long scale_fixed_DK_value(long base_value)
{
    // return value is equivalent to: round(base_value * units_per_pixel_best /16)
    long value = ((((units_per_pixel_best * base_value) >> 3) + (((units_per_pixel_best * base_value) >> 3) & 1)) >> 1);
    return value;
}

// TODO: The menu is currently always scaled so that the graphics FILL the screen on wider ARs than 4/3.
// make a config setting to choose FIT or FILL for the menu background/map background (etc)
// (background image only really, buttons should always FIT the screen, but they currently FILL (so ultrawide is borked)).

/**
 * Takes a fixed value tuned for original DK main menu at 640x480 and scales it to FIT the game's current resolution.
 * If the screen is wider than 16:10 the height is used; if the screen is narrower than 16:10 the width is used.
 * Uses units_per_pixel_menu (which is 16 at 640x480)
 *
 * @param base_value The fixed value tuned for original DK menu in 640x480 mode
 */
long scale_value_menu(long base_value)
{
    // return value is equivalent to: round(base_value * units_per_pixel_menu /16)
    long value = ((((units_per_pixel_menu * base_value) >> 3) + (((units_per_pixel_menu * base_value) >> 3) & 1)) >> 1);
    return value;
}

/** Scale the size and position of the landview background and banners. */
long scale_value_landview(long base_value)
{
    // return value is equivalent to: round(base_value * units_per_pixel_landview /16)
    long value = ((((units_per_pixel_landview * base_value) >> 3) + (((units_per_pixel_landview * base_value) >> 3) & 1)) >> 1);
    return value;
}

/**
 * Calculate units_per_pixel_landview (DK 640x480 was upp = 16) based on the current window size and the relative aspect ratio compared to 640x480).
 * The aim is for the landview background image to be twice the size of the game window, but wider and taller aspect ratios inhibit this.
 * For example 1920x1080p is wider than 640x480, so the height is used. Which would lead to a upp of 36, and a background of 2880x2160 (which is twice as tall as the game window).

 * Calculate units_per_pixel_landview_frame based on the current window size and the landview background (scaled) size.
 * This is used to make the window frame on the landview the correct size.
 * The aim is to be half-way between these the window size and the background size (where the landview background is up to 2x larger than the game window).
 *
 * Also calculates landview_frame_movement_scale_x, and landview_frame_movement_scale_y) for the landview window frame.
 *
 * @param width the current window width
 * @param height the current window height
 * @param landview_width the current landview background image width (passed LANDVIEW_MAP_WIDTH)
 * @param landview_height the current landview background image height (passed LANDVIEW_MAP_HEIGHT)
 */
void calculate_landview_upp(long width, long height, long landview_width, long landview_height)
{
    // horizontal, and vertical, aspect ratios for the current game window
    long h_ar = 1024 * width / height;
    long v_ar = 1024 * height / width;
    if (is_menu_ar_wider_than_original(width, height))  // Get FIT upp
    {
        // **HOR+ land view**

        // Is the game window more than twice as wide as 4:3? i.e. is it wider than 24:9?
        if  (1024 * 1024 / h_ar < 384)
        {
            units_per_pixel_landview = (((width / 2 * 1024 / 40 / 1024) + 1) / 2) * 2;

            // setup the landview frame upp and movement speed
            landview_frame_movement_scale_x = 1024;
            landview_frame_movement_scale_y = 1024;
            units_per_pixel_landview_frame = (((width * 1024 * 2 / 3 / 40 / 1024) + 1) / 2) * 2;
            return;
        }
        units_per_pixel_landview = (((height * 1024 / 30 / 1024) + 1) / 2) * 2;

        // setup window frame movement speed (in land view)
        long temp_width = 480 * h_ar;
        landview_frame_movement_scale_x = (1024 * (1024 * 640 - (temp_width - (1024 * 640))) / 640) / (h_ar / (640 / 480));
        landview_frame_movement_scale_y = 1024;

        // calculate the window frame units per pixel value
        long landview_frame_width_ideal = width + (((((scale_value_landview(landview_width) - width) / 2)) + 1) / 2) * 2;
        units_per_pixel_landview_frame = (((landview_frame_width_ideal * 1024 * 2 / 3 / 40 / 1024) + 1) / 2) * 2;
    }
    else
    {
        // **VERT+ land view**

        // Is the game window more than twice as tall as 4:3? i.e. is it taller than 4:6?
        if  (1024 * 1024 / v_ar < 682)
        {
            // Make the landview background (approximately) the same size as the height of the game window
            units_per_pixel_landview = (((height / 2 * 1024 / 30 / 1024) + 1) / 2) * 2;

            // setup the landview frame upp and movement speed
            landview_frame_movement_scale_x = 1024;
            landview_frame_movement_scale_y = 1024;
            units_per_pixel_landview_frame = (((height * 1024 * 2 / 3 / 30 / 1024) + 1) / 2) * 2;
            return;
        }
        units_per_pixel_landview = (((width * 1024 / 40 / 1024) + 1) / 2) * 2;

        // setup window frame movement speed (in land view)
        long temp_height = 640 * v_ar;
        landview_frame_movement_scale_x = 1024;
        landview_frame_movement_scale_y = (1024 * (1024 * 480 - (temp_height - (1024 * 480))) / 480) / (1024 * v_ar / (1024 * 480 / 640));

        // calculate the window frame units per pixel value
        long landview_frame_height_ideal = height + (((scale_value_landview(landview_height) - height) / 2 + 1) / 2) * 2;
        units_per_pixel_landview_frame = (((landview_frame_height_ideal * 1024 * 2 / 3 / 30 / 1024) + 1) / 2) * 2;
    }
}

/**
 * Determine whether the current window aspect ratio is wider than the original (16/10)
 *
 * @param width current window width
 * @param height current window height
 */
TbBool is_ar_wider_than_original(long width, long height)
{
    long original_aspect_ratio = (320 << 8) / 200;
    long current_aspect_ratio = (width << 8) / height;
    return (current_aspect_ratio > original_aspect_ratio);
}
/**
 * Determine whether the current window aspect ratio is wider than the original main menu (4/3)
 *
 * @param width current window width
 * @param height current window height
 */
TbBool is_menu_ar_wider_than_original(long width, long height)
{
    long original_aspect_ratio = (640 << 8) / 480;
    long current_aspect_ratio = (width << 8) / height;
    return (current_aspect_ratio > original_aspect_ratio);
}

/**
 * Calculate a units_per_px value relative to a given 640x400 base length,
*  a current reference length, and a current reference units_per_pixel
 *
 * @param base_length a given length/size for DK 640x400 mode
 * @param reference_upp a reference units_per_pixel value, that is relative to the current window resolution
 * @param reference_length a reference length/size to put in a ratio relative to the give base_length
 */
long calculate_relative_upp(long base_length, long reference_upp, long reference_length)
{
    long value = ((((base_length * reference_upp) << 2) / reference_length) >> 2); // bitshifts to round up
    return max(1,value);
}

/**
 * Scale UI relative to the base DEFAULT_UI_SCALE
 *
 * @param units_per_px the current units_per_pixel value
 * @param ui_scale the relative scale to multiply units_per_px by
 */
long resize_ui(long units_per_px, long ui_scale)
{
    long value = (units_per_px * ui_scale / DEFAULT_UI_SCALE);
    return max(1,value);
}

void calculate_aspect_ratio_factor(long width, long height)
{
    aspect_ratio_factor_HOR_PLUS = aspect_ratio_factor_HOR_PLUS_AND_VERT_PLUS = 100 * width / height;
    if (!is_ar_wider_than_original(width, height))
    {
        aspect_ratio_factor_HOR_PLUS = 160;
        aspect_ratio_factor_HOR_PLUS_AND_VERT_PLUS = 256 * height / width;
    }
}

long scale_fixed_DK_value_by_ar(long base_value, TbBool scale_up, TbBool vert_plus)
{
    long aspect_ratio_factor = vert_plus ? aspect_ratio_factor_HOR_PLUS_AND_VERT_PLUS : aspect_ratio_factor_HOR_PLUS;
    long multiplier = scale_up ? aspect_ratio_factor : DEFAULT_ASPECT_RATIO_FACTOR;
    long divisor = scale_up ? DEFAULT_ASPECT_RATIO_FACTOR : aspect_ratio_factor;

    long value = multiplier * base_value / divisor;
    return value;
}

long convert_vertical_FOV_to_horizontal(long vert_fov)
{
    double horizontal_fov = (2.0 * atan(tan((vert_fov * M_PI /180.0) / 2.0) * 16.0 / 10.0 )) * 180.0 / M_PI;
    long value = lround(horizontal_fov);
    return value;
}

long FOV_based_on_aspect_ratio(void)
{
    long value = scale_fixed_DK_value_by_ar(convert_vertical_FOV_to_horizontal(first_person_vertical_fov), false, false);
    return value;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
