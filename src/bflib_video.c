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
#include "bflib_video.h"

#include "bflib_mouse.h"
#include "bflib_vidsurface.h"
#include "bflib_sprfnt.h"
#include "bflib_inputctrl.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

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
        int c1 = to_palette[i + 0];
        int c2 = from_palette[i + 0];
        palette[i+0] = fade_count * (c1 - c2) / fade_steps + c2;
        c1 =   to_palette[i+1];
        c2 = from_palette[i+1];
        palette[i+1] = fade_count * (c1 - c2) / fade_steps + c2;
        c1 =   to_palette[i+2];
        c2 = from_palette[i+2];
        palette[i+2] = fade_count * (c1 - c2) / fade_steps + c2;
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

static TbBool LbHwCheckIsModeAvailable(TbScreenMode mode)
{
    TbScreenModeInfo* mdinfo = LbScreenGetModeInfo(mode);
    unsigned long sdlFlags = 0;
  if ((mdinfo->VideoFlags & Lb_VF_WINDOWED) == 0) {
      sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  }
  return true;
}

TbResult LbScreenFindVideoModes(void)
{
    int avail_num = 0;
    lbScreenModeInfo[0].Available = false;
    for (int i = 1; i < lbScreenModeInfoNum; i++)
    {
        if (LbHwCheckIsModeAvailable(i))
        {
            lbScreenModeInfo[i].Available = true;
            avail_num++;
        } else {
          lbScreenModeInfo[i].Available = false;
      }
  }
  if (avail_num > 0)
      return Lb_SUCCESS;
  return Lb_FAIL;
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
    }
    // Initialize SDL library
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE|SDL_INIT_AUDIO) < 0) {
        ERRORLOG("SDL init: %s",SDL_GetError());
        return Lb_FAIL;
    }
    // Setup the atexit() call to un-initialize
    atexit(SDL_Quit);
    return Lb_SUCCESS;
}

// this function is unused
LPCTSTR MsResourceMapping(int index)
{
  switch (index)
  {
  case 1:
      return "A";
      //return MAKEINTRESOURCE(110); -- may work for other resource compilers
  default:
      return NULL;
  }
}

TbResult LbScreenSetup(TbScreenMode mode, TbScreenCoord width, TbScreenCoord height,
    unsigned char *palette, short buffers_count, TbBool wscreen_vid)
{
    long hot_x;
    long hot_y;

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

    TbScreenModeInfo* mdinfo = LbScreenGetModeInfo(mode);
    if ( !LbScreenIsModeAvailable(mode) )
    {
        ERRORLOG("%s resolution %dx%d (mode %d) not available",
            (mdinfo->VideoFlags&Lb_VF_WINDOWED)?"Windowed":"Full screen",
            (int)mdinfo->Width,(int)mdinfo->Height,(int)mode);
        return Lb_FAIL;
    }

    // SDL video mode flags
    unsigned long sdlFlags = 0;
    sdlFlags |= SDL_SWSURFACE;
    if ((mdinfo->VideoFlags & Lb_VF_WINDOWED) == 0) {
        sdlFlags |= SDL_WINDOW_FULLSCREEN;
    }
    if (lbWindow != NULL) {
        // We only need to create a new window if we now have a different resolution/mode to the existing window, so check new/old resolution and mode...
        int cw, ch, cflags;
        cflags = SDL_GetWindowFlags(lbWindow);
        SDL_GetWindowSize(lbWindow, &cw, &ch);
        TbBool sameResolution = ((mdinfo->Width == cw) && (mdinfo->Height == ch));
        TbBool sameWindowMode = ((cflags & sdlFlags) != 0);
        TbBool stillInWindowedMode = ((sdlFlags & 1) == 0) && ((cflags & 1) == 0); // it is hard to detect if windowed mode (flag = 0) is still the same (i.e. no change of mode, still in windowed mode)
        if (stillInWindowedMode) {
            sameWindowMode = (sameWindowMode || stillInWindowedMode);
        }
        int fullscreenMode = (((sdlFlags & SDL_WINDOW_FULLSCREEN) != 0) ? SDL_WINDOW_FULLSCREEN : 0);
        if (!sameWindowMode && (fullscreenMode == 0))
        {
            SDL_DestroyWindow(lbWindow); // destroy window on transition from fullscreen to window, as it is quicker than using SDL_SetWindowFullscreen
            lbWindow = NULL;
        } 
        else
        {
            if (!sameResolution)
            {
                if (fullscreenMode == SDL_WINDOW_FULLSCREEN)
                {
                    SDL_DisplayMode dm = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0}; // maybe there is a better/more accurate way to describe the display mode...
                    dm.w=mdinfo->Width;
                    dm.h=mdinfo->Height;
                    SDL_SetWindowDisplayMode(lbWindow, &dm); // set display mode for fullscreen
                }
                SDL_SetWindowSize(lbWindow, mdinfo->Width, mdinfo->Height); // we want to set window size for both windowed mode, and fullscreen
            }
            if (!sameWindowMode)
            {
                SDL_SetWindowFullscreen(lbWindow, fullscreenMode); // change to/from fullscreen if requested
            }
        }
    }
    if (lbWindow == NULL) { // Only create a new window if we don't have a valid one already
        lbWindow = SDL_CreateWindow(lbDrawAreaTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mdinfo->Width, mdinfo->Height, sdlFlags);
    }
    if (lbWindow == NULL) {
        ERRORLOG("SDL_CreateWindow: %s", SDL_GetError());
        return Lb_FAIL;
    }

    lbScreenSurface = lbDrawSurface = SDL_GetWindowSurface( lbWindow );
    if (lbScreenSurface == NULL) {
        ERRORLOG("Failed to initialize mode %d: %s",(int)mode,SDL_GetError());
        return Lb_FAIL;
    }

    // Create secondary surface if necessary, that is if BPP != lbEngineBPP.
    if (mdinfo->BitsPerPixel != lbEngineBPP)
    {
        lbDrawSurface = SDL_CreateRGBSurface(0, mdinfo->Width, mdinfo->Height, lbEngineBPP, 0, 0, 0, 0);
        if (lbDrawSurface == NULL) {
            ERRORLOG("Can't create secondary surface: %s",SDL_GetError());
            LbScreenReset();
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
          LbMouseChangeSpriteAndHotspot(msspr, hot_x, hot_y);
    }
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
  strncpy(lbDrawAreaTitle, title, sizeof(lbDrawAreaTitle)-1);
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

TbResult LbScreenReset(void)
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
    long x2 = x + width;
    long y2 = y + height;
    if (x2 < x)
    {
        i = (x ^ x2);
        x = x ^ i;
        x2 = x ^ i ^ i;
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
  if (lbDisplay.WScreen != NULL)
  {
    lbDisplay.GraphicsWindowPtr = lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth*y + x;
  } else
  {
    lbDisplay.GraphicsWindowPtr = NULL;
  }
  return Lb_SUCCESS;
}

TbBool LbScreenIsModeAvailable(TbScreenMode mode)
{
  static TbBool setup = false;
  if (!setup)
  {
    if (LbScreenFindVideoModes() != Lb_SUCCESS)
      return false;
    setup = true;
  }
  TbScreenModeInfo* mdinfo = LbScreenGetModeInfo(mode);
  return mdinfo->Available;
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

/** Retrieves actual state of the Double Buffering function.
 *  Note that if the function was requested, it still doesn't necessarily
 *  mean it was activated.
 *
 * @return True if the function is currently active, false otherwise.
 */
TbBool LbScreenIsDoubleBufferred(void)
{
    return lbHasSecondSurface;
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
    if (lbScreenModeInfoNum >= sizeof(lbScreenModeInfo)/sizeof(lbScreenModeInfo[0]))
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
    strncpy(mdinfo->Desc,desc,sizeof(mdinfo->Desc));
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
    int ret;
    {
        width = 0; height = 0; bpp = 0; flags = Lb_VF_DEFAULT;
        ret = sscanf(desc," %d x %d x %d", &width, &height, &bpp);
    }
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
    return max(1,value);
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
    return max(1,value);
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
 * Uses units_per_pixel_best (which is 16 at 640x400)
 *
 * @param base_value The fixed value tuned for original DK 640x400 mode
 */
long scale_fixed_DK_value(long base_value)
{
    // return value is equivalent to: round(base_value * units_per_pixel_best /16)
    long value = ((((units_per_pixel_best * base_value) >> 3) + (((units_per_pixel_best * base_value) >> 3) & 1)) >> 1);
    return max(1,value);
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
/******************************************************************************/
#ifdef __cplusplus
}
#endif
