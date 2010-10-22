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
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#define SCREEN_MODES_COUNT 40

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT int _DK_LbPaletteFindColour(unsigned char *pal, unsigned char r, unsigned char g, unsigned char b);
DLLIMPORT void _DK_copy_to_screen(unsigned char *srcbuf, unsigned long width, unsigned long height, unsigned int flags);
/******************************************************************************/
// Global variables
/** List of registered video modes. */
TbScreenModeInfo lbScreenModeInfo[SCREEN_MODES_COUNT];
/** Count of used entries in registered video modes list. */
long lbScreenModeInfoNum = 0;

/** Informs if Video Screen subsystem initialization was done. */
volatile TbBool lbScreenInitialised = false;
volatile TbBool lbUseSdk = true;
/** Informs if the application window is active (focused on screen). */
extern volatile TbBool lbAppActive;
/** True if we have two surfaces. */
volatile TbBool lbHasSecondSurface;
/** True if we request the double buffering to be on in next mode switch. */
TbBool lbDoubleBufferingRequested;
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
    TbResult ret;
    SYNCDBG(12,"Starting");
    ret = LbMouseOnBeginSwap();
    // Put the data from Draw Surface onto Screen Surface
    if ((ret == Lb_SUCCESS) && (lbHasSecondSurface)) {
        if (SDL_BlitSurface(lbDrawSurface, NULL, lbScreenSurface, NULL) == -1) {
            ERRORLOG("Blit failed: %s",SDL_GetError());
            ret = Lb_FAIL;
        }
    }
    // Flip the image displayed on Screen Surface
    if (ret == Lb_SUCCESS) {
        if (SDL_Flip(lbScreenSurface) < 0) { //calls SDL_UpdateRect for entire screen if not double buffered
            ERRORLOG("Flip failed: %s",SDL_GetError());
            ret = Lb_FAIL;
        }
    }
    LbMouseOnEndSwap();
    return ret;
}

TbResult LbScreenClear(TbPixel colour)
{
    SYNCDBG(12,"Starting");
    if (!lbScreenInitialised)
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
    TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(lbDisplay.ScreenMode);
    // For DDraw, screen buffer BPP equals video mode BPP
    return mdinfo->BitsPerPixel;
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

void LbPaletteFadeStep(unsigned char *from_pal,unsigned char *to_pal,long fade_steps)
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
    LbScreenWaitVbi();
    LbPaletteSet(palette);
}

TbResult LbPaletteStopOpenFade(void)
{
    fade_started = 0;
    return Lb_SUCCESS;
}

long LbPaletteFade(unsigned char *pal, long fade_steps, enum TbPaletteFadeFlag flg)
{
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
          LbPaletteFadeStep(from_pal,pal,fade_steps);
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
    LbPaletteFadeStep(from_pal,pal,fade_steps);
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
  TbScreenModeInfo *mdinfo;
  unsigned long sdlFlags;
  int closestBPP;
  mdinfo = LbScreenGetModeInfo(mode);
  sdlFlags = 0;
  if (mdinfo->BitsPerPixel == 8) {
      sdlFlags |= SDL_HWPALETTE | SDL_DOUBLEBUF;
  }
  if ((mdinfo->VideoFlags & Lb_VF_WINDOWED) == 0) {
      sdlFlags |= SDL_FULLSCREEN;
  }

  closestBPP = SDL_VideoModeOK(mdinfo->Width, mdinfo->Height, mdinfo->BitsPerPixel, sdlFlags);
  return (closestBPP == mdinfo->BitsPerPixel);
}

TbResult LbScreenFindVideoModes(void)
{
  int i,avail_num;
  avail_num = 0;
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

static void LbRegisterStandardVideoModes(void)
{
    lbScreenModeInfoNum = 0;
    LbRegisterVideoMode("INVALID",       0,    0,  0, Lb_VF_DEFAULT);
    LbRegisterVideoMode("320x200x8",   320,  200,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("320x200x16",  320,  200, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("320x200x24",  320,  200, 24, Lb_VF_DEFAULT);
    LbRegisterVideoMode("320x240x8",   320,  240,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("320x240x16",  320,  240, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("320x240x24",  320,  240, 24, Lb_VF_DEFAULT);
    LbRegisterVideoMode("512x384x8",   512,  384,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("512x384x16",  512,  384, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("512x384x24",  512,  384, 24, Lb_VF_0100);
    LbRegisterVideoMode("640x400x8",   640,  400,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("640x400x16",  640,  400, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("640x400x24",  640,  400, 24, Lb_VF_0100|Lb_VF_0001);
    LbRegisterVideoMode("640x480x8",   640,  480,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("640x480x16",  640,  480, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("640x480x24",  640,  480, 24, Lb_VF_0100|Lb_VF_0001|Lb_VF_0002);
    LbRegisterVideoMode("800x600x8",   800,  600,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("800x600x16",  800,  600, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("800x600x24",  800,  600, 24, Lb_VF_0100|Lb_VF_0001|Lb_VF_0004);
    LbRegisterVideoMode("1024x768x8", 1024,  768,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("1024x768x16",1024,  768, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("1024x768x24",1024,  768, 24, Lb_VF_0100|Lb_VF_0001|Lb_VF_0002|Lb_VF_0004);
    LbRegisterVideoMode("1280x1024x8", 1280,1024,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("1280x1024x16",1280,1024, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("1280x1024x24",1280,1024, 24, Lb_VF_DEFAULT);
    LbRegisterVideoMode("1600x1200x8", 1600,1200,  8, Lb_VF_DEFAULT);
    LbRegisterVideoMode("1600x1200x16",1600,1200, 16, Lb_VF_DEFAULT);
    LbRegisterVideoMode("1600x1200x24",1600,1200, 24, Lb_VF_DEFAULT);
}

TbResult LbScreenInitialize(void)
{
    putenv("SDL_VIDEODRIVER=directx");// under Win32, windib or directx
    // Clear global variables
    lbScreenInitialised = false;
    lbScreenSurface = NULL;
    lbDrawSurface = NULL;
    lbHasSecondSurface = false;
    lbDoubleBufferingRequested = false;
    lbAppActive = true;
    // Initialize SDL library
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
       ERRORLOG("SDL init: %s",SDL_GetError());
       return Lb_FAIL;
    }
    // Setup the atexit() call to un-initialize
    atexit(SDL_Quit);
    // Register default video modes
    if (lbScreenModeInfoNum == 0)
    {
        LbRegisterStandardVideoModes();
    }
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
    ev.type = SDL_ACTIVEEVENT;
    ev.active.state = SDL_APPACTIVE;
    ev.active.gain = ((SDL_GetAppState() & ev.active.state) != 0);
    SDL_PushEvent(&ev);
    return Lb_SUCCESS;
}

TbResult LbScreenUpdateIcon(void)
{
    //TODO replace with portable version
/*
    Uint32          colorkey;
    SDL_Surface     *image;
    image = SDL_LoadBMP("keeperfx_icon.bmp");
    colorkey = SDL_MapRGB(image->format, 255, 0, 255);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey);
    SDL_WM_SetIcon(image,NULL);
 */
    HICON hIcon;
    HINSTANCE lbhInstance;
    SDL_SysWMinfo wmInfo;

    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWMInfo(&wmInfo) < 0) {
        WARNLOG("Couldn't get SDL window info, therefore cannot set icon");
        return Lb_FAIL;
    }

    lbhInstance = GetModuleHandle(NULL);
    hIcon = LoadIcon(lbhInstance, MsResourceMapping(lbIconIndex));
    SendMessage(wmInfo.window, WM_SETICON, ICON_BIG,  (LPARAM)hIcon);
    SendMessage(wmInfo.window, WM_SETICON, ICON_SMALL,(LPARAM)hIcon);
    return Lb_SUCCESS;
}

TbResult LbScreenSetup(TbScreenMode mode, TbScreenCoord width, TbScreenCoord height,
    unsigned char *palette, short buffers_count, TbBool wscreen_vid)
{
    SDL_Surface * prevScreenSurf;
    long hot_x,hot_y;
    struct TbSprite *msspr;
    TbScreenModeInfo *mdinfo;
    unsigned long sdlFlags;

    msspr = NULL;
    LbExeReferenceNumber();
    if (lbDisplay.MouseSprite != NULL)
    {
        msspr = lbDisplay.MouseSprite;
        GetPointerHotspot(&hot_x,&hot_y);
    }
    prevScreenSurf = lbScreenSurface;
    LbMouseChangeSprite(NULL);
    if (lbHasSecondSurface) {
        SDL_FreeSurface(lbDrawSurface);
    }
    lbDrawSurface = NULL;

    if (prevScreenSurf != NULL) {
    }

    if ( !LbScreenIsModeAvailable(mode) )
    {
        ERRORLOG("Screen mode %d not available",(int)mode);
        return Lb_FAIL;
    }
    mdinfo = LbScreenGetModeInfo(mode);

    // SDL video mode flags
    sdlFlags = 0;
    sdlFlags |= SDL_SWSURFACE;
    if (mdinfo->BitsPerPixel == 8) {
        sdlFlags |= SDL_HWPALETTE;
    }
    if (lbDoubleBufferingRequested) {
        sdlFlags |= SDL_DOUBLEBUF;
    }
    if ((mdinfo->VideoFlags & Lb_VF_WINDOWED) == 0) {
        sdlFlags |= SDL_FULLSCREEN;
    }

    // Set SDL video mode (also creates window).
    lbScreenSurface = lbDrawSurface = SDL_SetVideoMode(mdinfo->Width, mdinfo->Height, mdinfo->BitsPerPixel, sdlFlags);

    if (lbScreenSurface == NULL) {
        ERRORLOG("Failed to initialize SDL video mode %d.",(int)mode);
        return Lb_FAIL;
    }

    SDL_WM_SetCaption(lbDrawAreaTitle, lbDrawAreaTitle);
    LbScreenUpdateIcon();

    // Create secondary surface if necessary. Right now, only if BPP != 8.
    //TODO: utilize this for rendering in different resolution later
    if (mdinfo->BitsPerPixel != 8)
    {
        lbDrawSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, mdinfo->Width, mdinfo->Height, 8, 0, 0, 0, 0);
        if (lbDrawSurface == NULL) {
            ERRORLOG("Can't create secondary surface");
            LbScreenReset();
            return Lb_FAIL;
        }
        lbHasSecondSurface = true;
    }

    lbDisplay.DrawFlags = 0;
    lbDisplay.DrawColour = 0;
    lbDisplay.PhysicalScreenWidth = mdinfo->Width;
    lbDisplay.PhysicalScreenHeight = mdinfo->Height;
    lbDisplay.ScreenMode = mode;
    lbDisplay.PhysicalScreen = NULL;
    // The graphics screen size will be later updated to screen pitch by LbScreenLock()
    lbDisplay.GraphicsScreenWidth = mdinfo->Width;
    lbDisplay.GraphicsScreenHeight = mdinfo->Height;
    lbDisplay.WScreen = NULL;
    lbDisplay.GraphicsWindowPtr = NULL;

    SYNCLOG("Mode %dx%dx%d setup succeeded",(int)lbScreenSurface->w,(int)lbScreenSurface->h,(int)lbScreenSurface->format->BitsPerPixel);
    if (palette != NULL)
        LbPaletteSet(palette);
    LbScreenSetGraphicsWindow(0, 0, mdinfo->Width, mdinfo->Height);
    LbTextSetWindow(0, 0, mdinfo->Width, mdinfo->Height);
    SYNCDBG(8,"Done filling display properties struct");
    if ( LbMouseIsInstalled() )
    {
        LbMouseSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
        LbMouseSetPosition(lbDisplay.PhysicalScreenWidth / 2, lbDisplay.PhysicalScreenHeight / 2);
        if (msspr != NULL)
          LbMouseChangeSpriteAndHotspot(msspr, hot_x, hot_y);
    }
/*    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWMInfo(&info))
    {
        UpdateWindow(info.window);
        ShowWindow(info.window, SW_RESTORE);
        SetFocus(info.window);
    }*/
    //SDL_WM_GrabInput(SDL_GRAB_ON);
    lbScreenInitialised = true;
    LbScreenActivationUpdate();
    SYNCDBG(8,"Finished");
    return Lb_SUCCESS;
}

/** Clears the 8-bit video palette with black colour.
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
    SYNCDBG(12,"Starting");
    if ((!lbScreenInitialised) || (lbDrawSurface == NULL))
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
    //if (SDL_SetPalette(lbDrawSurface, SDL_LOGPAL | SDL_PHYSPAL,
    if (SDL_SetColors(lbDrawSurface,
        lbPaletteColors, 0, PALETTE_COLORS) != 1) {
        SYNCDBG(8,"SDL SetPalette failed.");
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
    return (lbDisplay.WScreen > NULL);
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
    if (LbScreenFindVideoModes() != Lb_SUCCESS)
      return false;
    setup = true;
  }
  mdinfo = LbScreenGetModeInfo(mode);
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
    return ((lbScreenSurface->flags & SDL_DOUBLEBUF) != 0);
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

TbScreenMode LbRegisterVideoMode(const char *desc, TbScreenCoord width, TbScreenCoord height,
    unsigned short bpp, unsigned long flags)
{
    TbScreenModeInfo *mdinfo;
    TbScreenMode mode;
    mode = LbRecogniseVideoModeString(desc);
    if (mode != Lb_SCREEN_MODE_INVALID)
    {
        mdinfo = &lbScreenModeInfo[mode];
        if ((mdinfo->Width == width) && (mdinfo->Height == height) && (mdinfo->BitsPerPixel == bpp))
        {
            // Mode is already registered
            return mode;
        }
        // Mode with same name but different params is registered
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

TbScreenMode LbRegisterVideoModeString(const char *desc)
{
    int width, height;
    int bpp;
    unsigned long flags;
    int ret;
    width = 0; height = 0; bpp = 0; flags = 0;
    ret = sscanf(desc," %d x %d x %d", &width, &height, &bpp);
    if (ret != 3)
    {
        // Cannot recognize parameters in mode
        return Lb_SCREEN_MODE_INVALID;
    }
    return LbRegisterVideoMode(desc, width, height, bpp, flags);
}

TbPixel LbPaletteFindColour(unsigned char *pal, unsigned char r, unsigned char g, unsigned char b)
{
  return _DK_LbPaletteFindColour(pal, r, g, b);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
