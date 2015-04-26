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

#define SCREEN_MODES_COUNT 20

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
volatile TbBool lbScreenInitialized = false;

// Whether we want to always use current desktop resolution and scale game to fill screen.
// TODO make this into config tool and cfg file.
volatile TbBool lbUseDesktopResolution = false;

/** The depth of the surface in bits, or bytes per pixel expected by the engine.
 * can be 8 or 4
 * On any try of entering different video BPP, this mode will be emulated. 
 * see https://wiki.libsdl.org/SDL_CreateRGBSurface#Remarks */
volatile unsigned short lbPalettedSurfaceColorDepth = 8;

/** Informs if the application window is active (focused on screen). */
extern volatile TbBool lbAppActive;

/** Name of the video driver to be used. Must be set before LbScreenInitialize().
 * Under Win32 and with SDL2.0, the only choice is 'windows'. */
char lbVideoDriver[16];

/** Colour palette buffer, to be used inside lbDisplay. */
unsigned char lbPalette[PALETTE_SIZE];

/** Driver-specific colour palette buffer. */
SDL_Color lbPaletteColors[PALETTE_COLORS];

char lbDrawAreaTitle[128] = "Bullfrog Shell";
volatile TbBool lbInteruptMouse;
volatile unsigned long lbIconIndex = 0;
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
    SYNCDBG(12, "Starting");
    if (!lbScreenInitialized)
        return Lb_FAIL;

    lbDisplay.WScreen = (unsigned char *)lbPalettedSurface->pixels;
    lbDisplay.GraphicsScreenWidth = lbPalettedSurface->pitch;
    lbDisplay.GraphicsWindowPtr = &lbDisplay.WScreen[lbDisplay.GraphicsWindowX +
        lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsWindowY];

    return Lb_SUCCESS;
}

TbResult LbScreenUnlock(void)
{
    SYNCDBG(12, "Starting");
    if (!lbScreenInitialized)
        return Lb_FAIL;
    lbDisplay.WScreen = NULL;
    lbDisplay.GraphicsWindowPtr = NULL;

    return Lb_SUCCESS;
}

TbResult LbScreenRender(void)
{
    assert((lbScreenWindow != NULL));
    assert((lbPalettedSurface != NULL));
    assert((lbGameRenderer != NULL));
    assert((lbDrawTexture == NULL));

    TbResult ret;
    SYNCDBG(12, "Starting");

    ret = LbMouseOnBeginSwap();
    // Flip the image displayed on Screen Surface
    if (ret == Lb_SUCCESS && lbScreenInitialized )
    {
        // Converting 256 color to True color
        lbDrawTexture = SDL_CreateTextureFromSurface(lbGameRenderer, lbPalettedSurface);
        if (lbDrawTexture == NULL)
        {
            ERRORLOG("Failed Converting surface to texture: %s", SDL_GetError());
            return Lb_FAIL;
        }

        SDL_RenderCopy(lbGameRenderer, lbDrawTexture, NULL, NULL);
        SDL_RenderPresent(lbGameRenderer);

        SDL_DestroyTexture(lbDrawTexture);
        lbDrawTexture = NULL;
    }
    LbMouseOnEndSwap();
    return ret;
}

TbResult LbScreenClear(TbPixel colour)
{
    SYNCDBG(12,"Starting");
    if ((!lbScreenInitialized) || (lbPalettedSurface == NULL))
      return Lb_FAIL;
    if (SDL_FillRect(lbPalettedSurface, NULL, colour) < 0) {
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
    if (lbPalettedSurface != NULL) {
        return lbPalettedSurface->format->BitsPerPixel;
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

TbResult LbPaletteFadeStep(unsigned char *from_pal,unsigned char *to_pal,long fade_steps)
{
    int i,c1,c2;
    unsigned char palette[PALETTE_SIZE];
    for (i=0; i < 3*PALETTE_COLORS; i+=3)
    {
        c1 =   to_pal[i+0];
        c2 = from_pal[i+0];
        palette[i+0] = fade_count * (c1 - c2) / fade_steps + c2;
        c1 =   to_pal[i+1];
        c2 = from_pal[i+1];
        palette[i+1] = fade_count * (c1 - c2) / fade_steps + c2;
        c1 =   to_pal[i+2];
        c2 = from_pal[i+2];
        palette[i+2] = fade_count * (c1 - c2) / fade_steps + c2;
    }
    TbResult ret;
    LbScreenWaitVbi();
    ret = LbPaletteSet(palette);
    LbScreenRender();
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
  // TODO: HeM No longer needed in SDL 2.0, clean this up
  return true;
}

TbResult LbScreenFindVideoModes(void)
{
  int i,avail_num;
  avail_num = 0;
  lbScreenModeInfo[0].Available = false;
  for (i=1; i < lbScreenModeInfoNum; i++)
  {
      if (LbHwCheckIsModeAvailable(i)) {
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

// TODO (related number 01) do we need all these wired reolution?
static void LbRegisterStandardVideoModes(void)
{
    lbScreenModeInfoNum = 0;
    LbRegisterVideoMode("INVALID",    0,    0,  Lb_VF_DEFAULT);
    LbRegisterVideoMode("320x200",  320,  200, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("320x240",  320,  240, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("512x384",  512,  384, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("640x400",  640,  400, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("640x480",  640,  480, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("800x600",  800,  600, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("1024x768",1024,  768, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("1280x1024",1280,1024, Lb_VF_RGBCOLOR);
    LbRegisterVideoMode("1600x1200",1600,1200, Lb_VF_RGBCOLOR);
}

TbResult LbScreenInitialize(void)
{
    char buf[32];

    // Clear global variables
    lbScreenInitialized = false;
    lbScreenWindow = NULL;
    lbAppActive = true;
    LbMouseChangeMoveRatio(DEFAULT_MOUSE_MOVE_RATIO, DEFAULT_MOUSE_MOVE_RATIO);

    // Register default video modes
    if (lbScreenModeInfoNum == 0) {
        LbRegisterStandardVideoModes();
    }

    // SDL environment variables
    if (lbVideoDriver[0] != '\0') {
        sprintf(buf,"SDL_VIDEODRIVER=%s",lbVideoDriver);
        putenv(buf);
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

static LPCTSTR MsResourceMapping(int index)
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

static TbResult LbScreenActivationUpdate(void)
{
    SDL_Event ev;
    ev.type = SDL_WINDOWEVENT;
    ev.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
    //ev.active.gain = ((SDL_GetAppState() & ev.active.state) != 0);
    SDL_PushEvent(&ev);
    return Lb_SUCCESS;
}

/** Updates icon of the application.
 *  Icon index is stored in lbIconIndex global variable; this function maps
 *  the index into OS-specific resource and applies it to engine process.
 *
 * @return If icon was updated, Lb_SUCCESS is returned.
 */
TbResult LbScreenUpdateIcon(void)
{
    HICON hIcon = NULL;
    HINSTANCE lbhInstance;
    SDL_SysWMinfo wmInfo;

    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(lbScreenWindow, &wmInfo) != SDL_TRUE)
    {
        WARNLOG("Couldn't get SDL window info, therefore cannot set icon");
        return Lb_FAIL;
    }

    lbhInstance = GetModuleHandle(NULL);

    hIcon = LoadIcon(lbhInstance, MsResourceMapping(lbIconIndex));

    if (!hIcon)
    {
        ERRORLOG("failed loading icon");
    }

    SendMessage(wmInfo.info.win.window, WM_SETICON, ICON_BIG,  (LPARAM)hIcon);
    SendMessage(wmInfo.info.win.window, WM_SETICON, ICON_SMALL,(LPARAM)hIcon);
    return Lb_SUCCESS;
}

TbResult LbScreenSetup(TbScreenMode modeIndex, unsigned char *palette, short buffers_count, TbBool wscreen_vid)
{
    long hot_x, hot_y;
    struct TbSprite *msspr;
    TbScreenModeInfo *mdinfo;
    unsigned long sdlFlags;

    msspr = NULL;
    LbExeReferenceNumber();
    if (lbDisplay.MouseSprite != NULL)
    {
        msspr = lbDisplay.MouseSprite;
        GetPointerHotspot(&hot_x, &hot_y);
    }

    LbScreenReset(false);

    assert((lbPalettedSurface == NULL));
    //assert((lbGameRenderer == NULL));
    assert((lbDrawTexture == NULL));

    mdinfo = LbScreenGetModeInfo(modeIndex);
    if ( !LbScreenIsModeAvailable(modeIndex) )
    {
        ERRORLOG("%s resolution %dx%d (modeIndex %d) not available",
            (mdinfo->VideoFlags & Lb_VF_WINDOWED) ? "Windowed" : "Full screen",
            (int)mdinfo->Width,(int)mdinfo->Height,(int)modeIndex);
        return Lb_FAIL;
    }

    // Working around SDL bug: use different size of window and renderer to keep movie full screen.
    int isMovieMode = false;
    isMovieMode = (mdinfo->Width == 320) && (mdinfo->Height == 200);

    // SDL video modeIndex flags
    sdlFlags = SDL_WINDOW_RESIZABLE;
    if ((mdinfo->VideoFlags & Lb_VF_WINDOWED) == 0) 
    {
        
        sdlFlags |= lbUseDesktopResolution ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
    }

    // TODO HeM break following into smaller methods.
    // Set SDL video mode (also creates window if we do not have one already).
    if (lbScreenWindow == NULL)
    {
        lbScreenWindow = SDL_CreateWindow("Dungeon Keeper FX",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            mdinfo->Width,
            mdinfo->Height,
            sdlFlags);    

        if (lbScreenWindow == NULL) 
        {
            ERRORLOG("Failed to initialize mode %d: %s", (int)modeIndex, SDL_GetError());
            return Lb_FAIL;
        }
    }
    else
    {
        if (!isMovieMode)
        {
            // Reisze window when it already exists.
            SDL_SetWindowSize(lbScreenWindow, mdinfo->Width, mdinfo->Height);
            SDL_SetWindowFullscreen(lbScreenWindow, sdlFlags);

            // Do not reset window location since it is disturbing.
            // SDL_SetWindowPosition(lbScreenWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    }


    // Working around a possible SDL bug: providing no flags 
    // gives priority to available SDL_RENDERER_ACCELERATED renders, which cause memory leak
    // on SDL_DestoryRenderer(), TODO report this when I have an account.
    if (lbGameRenderer == NULL)
    {
        lbGameRenderer = SDL_CreateRenderer(lbScreenWindow, -1, 0);
    }

    if (lbGameRenderer == NULL)
    {
        ERRORLOG("Error creating lbGameRenderer %d: %s", (int)modeIndex, SDL_GetError());
        return Lb_FAIL;
    }

    // We now leaves the scaling work to renderer and it will be taken care automatically.
    // 'linear' make the scaled rendering look smoother.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    // Set the logical screen size as video mode specified.
    SDL_RenderSetLogicalSize(lbGameRenderer, mdinfo->Width, mdinfo->Height);

    lbPalettedSurface = SDL_CreateRGBSurface(0/*obselete flag*/, mdinfo->Width, mdinfo->Height, lbPalettedSurfaceColorDepth, 0, 0, 0, 0);
    if (lbPalettedSurface == NULL)
    {
        ERRORLOG("Can't create paletted surface: %s", SDL_GetError());
        LbScreenReset(false);
        return Lb_FAIL;
    }

    LbScreenUpdateIcon();

    lbDisplay.DrawFlags = 0;
    lbDisplay.DrawColour = 0; 
    lbDisplay.PhysicalScreenWidth = mdinfo->Width;
    lbDisplay.PhysicalScreenHeight = mdinfo->Height;
    lbDisplay.ScreenMode = modeIndex;
    lbDisplay.PhysicalScreen = NULL;

    // The graphics screen size should be really taken after screen is locked, but it seem just getting in now will work too
    lbDisplay.GraphicsScreenWidth = lbPalettedSurface->pitch;
    lbDisplay.GraphicsScreenHeight = mdinfo->Height;
    lbDisplay.WScreen = NULL;
    lbDisplay.GraphicsWindowPtr = NULL;

    init_lbDisplayEx_values();
    lbDisplayEx.mainPanelWidth = 0;
    lbDisplayEx.cameraMoveRatioX = 0;
    lbDisplayEx.cameraMoveRatioY = 0;
    lbDisplayEx.isMouseOverButton = 0;
    lbDisplayEx.isDragMovingCamera = false;
    lbDisplayEx.isDragRotatingCamera = false;
    lbDisplayEx.skipLButtonRelease = false;
    lbDisplayEx.skipRButtonRelease = false;

    SYNCLOG("Mode %d x %d setup succeeded", (int)mdinfo->Width, (int)mdinfo->Height);

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
        LbMouseSetPosition(lbDisplay.PhysicalScreenWidth / 2, lbDisplay.PhysicalScreenHeight / 2);
        if (msspr != NULL)
        {
            LbMouseChangeSpriteAndHotspot(msspr, hot_x, hot_y);
        }
    }
    LbInputRestate();
    lbScreenInitialized = true;
    LbScreenActivationUpdate();
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
    unsigned char * bufColors;
    SDL_Color * destColors;
    const unsigned char * srcColors;
    unsigned long i;
    TbResult ret;
    SYNCDBG(12, "Starting");
    if ((!lbScreenInitialized) || (lbPalettedSurface == NULL))
        return Lb_FAIL;
    //destColors = (SDL_Color *) malloc(sizeof(SDL_Color) * PALETTE_COLORS);
    destColors = lbPaletteColors;
    srcColors = palette;
    bufColors = lbPalette;
    if ((destColors == NULL) || (srcColors == NULL))
        return Lb_FAIL;
    ret = Lb_SUCCESS;
    for (i = 0; i < PALETTE_COLORS; i++) {
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

    // TODO: HeM Not sure this works:
    SDL_Palette *sdl_palette = SDL_AllocPalette(PALETTE_COLORS);
    SDL_SetPaletteColors(sdl_palette, lbPaletteColors, 0, PALETTE_COLORS);
    if (SDL_SetSurfacePalette(lbPalettedSurface, sdl_palette) != 0) {
        SYNCDBG(8, "SDL SetPalette failed.");
        ret = Lb_FAIL;
    }
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
    if ((!lbScreenInitialized) || (lbPalettedSurface == NULL))
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
    colours_num = lbPalettedSurface->format->palette->ncolors;
    if (colours_num > PALETTE_COLORS) {
        colours_num = PALETTE_COLORS;
    } else
    if (colours_num < PALETTE_COLORS) {
        memset(palette,0,PALETTE_SIZE);
    }
    srcColors = lbPalettedSurface->format->palette->colors;
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

TbResult LbScreenHardwareConfig(const char *driver, short engine_bpp)
{
    if (driver != NULL)
    {
        if (strlen(driver) > sizeof(lbVideoDriver)-1)
            return Lb_FAIL;
        strcpy(lbVideoDriver,driver);
    }
    if (engine_bpp != 0)
        lbPalettedSurfaceColorDepth = engine_bpp;
    return Lb_SUCCESS;
}

TbScreenModeInfo *LbScreenGetModeInfo(TbScreenMode mode)
{
    if (mode < lbScreenModeInfoNum)
    {
        return &lbScreenModeInfo[mode];
    }

    return &lbScreenModeInfo[0];
}

TbBool LbScreenIsLocked(void)
{
    return (lbDisplay.WScreen != NULL);
}

TbResult LbScreenReset(TbBool resetMainWindow)
{
    LbMouseChangeSprite(NULL);

    SDL_FreeSurface(lbPalettedSurface);
    lbPalettedSurface = NULL;    

    SDL_DestroyTexture(lbDrawTexture);
    lbDrawTexture = NULL;

    // Commenting following part to workaround SDL bug: when using GPU acceleration 
    // will cause memory leak at SDL_DestroyRenderer. So instead of destroying everything
    // we simply resize the window. 

    //SDL_DestroyRenderer(lbGameRenderer);
    //lbGameRenderer = NULL;

    // Only reset main windows at 'alt + r' event
    //if (resetMainWindow)
    //{
    //    SDL_DestroyWindow(lbScreenWindow);
    //    lbScreenWindow = NULL;
    //}

    // Mark as not initialized
    lbScreenInitialized = false;
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
  TbScreenModeInfo *mdinfo;
  static TbBool setup = false;
  if (!setup)
  {
    // TODO: HeM this is checking 'if nothing is supported at all', not needed either, clean up.
    if (LbScreenFindVideoModes() != Lb_SUCCESS)
      return false;
    setup = true;
  }
  mdinfo = LbScreenGetModeInfo(mode);
  return mdinfo->Available;
}

TbScreenMode LbRecogniseVideoModeString(const char *desc)
{
    int mode;
    for (mode=0; mode < lbScreenModeInfoNum; mode++)
    {
      if (strcasecmp(lbScreenModeInfo[mode].Desc,desc) == 0)
        return (TbScreenMode)mode;
    }
    return Lb_SCREEN_MODE_INVALID;
}

TbScreenMode LbRegisterVideoMode(const char *describe, TbScreenCoord width, TbScreenCoord height, unsigned long flags)
{
    TbScreenModeInfo *mdinfo;
    TbScreenMode modeIndex;
    modeIndex = LbRecogniseVideoModeString(describe);

    // Found mode with same description already registered.
    if (modeIndex != Lb_SCREEN_MODE_INVALID)
    {
        mdinfo = &lbScreenModeInfo[modeIndex];

        // Check if parameters really match.
        if ((mdinfo->Width == width) && (mdinfo->Height == height))
        {
            // modeIndex is already registered
            return modeIndex;
        }
        else
        {
            // modeIndex with same name but different params is registered
#ifdef __DEBUG
            LbWarnLog("%s: modeIndex with same name but different params is registered, cannot register %dx%dx%d\n", __func__, (int)width, (int)height, (int)bpp);
#endif
            return Lb_SCREEN_MODE_INVALID;
        }
    }

    if (lbScreenModeInfoNum >= SCREEN_MODES_COUNT)
    {
        // No free modeIndex slots
        return Lb_SCREEN_MODE_INVALID;
    }

    // Insert new modeIndex to array
    modeIndex = lbScreenModeInfoNum;
    lbScreenModeInfoNum++;
    mdinfo = &lbScreenModeInfo[modeIndex];

    // Fill the modeIndex content
    memset(mdinfo, 0, sizeof(TbScreenModeInfo));
    mdinfo->Width = width;
    mdinfo->Height = height;
    mdinfo->BitsPerPixel = 32;
    mdinfo->Available = false;
    mdinfo->VideoFlags = flags;
    strncpy(mdinfo->Desc, describe, sizeof(mdinfo->Desc));

    return modeIndex;
}

TbScreenMode LbRegisterVideoModeString(const char *desc)
{
    // TODO HeM remove the last %d(bpp) in pattern when config window is updated.
    // TODO mefistotelis remove the 'x' or 'w' part for each resolution, instead make a unified setting.

    int width, height, bpp;
    unsigned long flags;
    int ret;
    {
        width = 0; height = 0; flags = Lb_VF_DEFAULT;
        ret = sscanf(desc, " %d x %d x %d", &width, &height, &bpp);
    }

    // if matches pattern  %d x %d x %d
    if (ret != 3)
    {
        // pattern not matched - maybe it's windowed mode
        width = 0; height = 0; flags = Lb_VF_DEFAULT;
        ret = sscanf(desc, " %d x %d w %d", &width, &height, &bpp);
        flags |= Lb_VF_WINDOWED;
    }

    // if matches pattern  %d x %d w %d
    if (ret != 3)
    {
        // Cannot recognize parameters in mode
#ifdef __DEBUG
        LbWarnLog("%s: Cannot recognize parameters in mode, got %dx%dx%d\n",__func__, width, height, bpp);
#endif
        return Lb_SCREEN_MODE_INVALID;
    }

    flags |= Lb_VF_RGBCOLOR;

    return LbRegisterVideoMode(desc, width, height, flags);
}

TbPixel LbPaletteFindColour(const unsigned char *pal, unsigned char r, unsigned char g, unsigned char b)
{
    int min_delta;
    const unsigned char *c;
    int i;
    // Compute minimal square difference in color; return exact match if found
    min_delta = 999999;
    c = pal;
    for (i = 0; i < 256; i++)
    {
        int dr,dg,db;
        dr = (r - c[0]) * (r - c[0]);
        dg = (g - c[1]) * (g - c[1]);
        db = (b - c[2]) * (b - c[2]);
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
    unsigned char tmcol[256];
    unsigned char *o;
    int n;
    n = 0;
    o = tmcol;
    c = pal;
    for (i = 0; i < 256; i++)
    {
        int dr,dg,db;
        dr = (r - c[0]) * (r - c[0]);
        dg = (g - c[1]) * (g - c[1]);
        db = (b - c[2]) * (b - c[2]);
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
        int dr,dg,db;
        c = &pal[3 * tmcol[i]];
        dr = abs(r - c[0]);
        dg = abs(g - c[1]);
        db = abs(b - c[2]);
        if (min_delta > dr+dg+db) {
            min_delta = dr+dg+db;
        }
    }
    // Gather all the colors with minimal linear difference
    // Note that we may re-use tmcol array, because (i <= m)
    int m;
    m = 0;
    o = tmcol;
    for (i = 0; i < n; i++)
    {
        int dr,dg,db;
        c = &pal[3 * tmcol[i]];
        dr = abs(r - c[0]);
        dg = abs(g - c[1]);
        db = abs(b - c[2]);
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
        int dr,dg,db;
        c = &pal[3 * tmcol[i]];
        dr = (c[0] * c[0]);
        dg = (c[1] * c[1]);
        db = (c[2] * c[2]);
        if (min_delta > db+2*(dg+dr))
        {
          min_delta = db+2*(dg+dr);
          o = &tmcol[i];
        }
    }
    return *o;
}

// initialize fields in lbDisplayEx which should be refreshed every turn.
void init_lbDisplayEx_values()
{
    lbDisplayEx.isPowerHandNothingTodoLeftClick = 0;
    lbDisplayEx.isPowerHandNothingTodoRightClick = 0;
    lbDisplayEx.cameraMoveX = 0;
    lbDisplayEx.cameraMoveY = 0;
    lbDisplayEx.cameraRotateAngle = 0;
    lbDisplayEx.wheelUp = 0;
    lbDisplayEx.wheelDown = 0;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
