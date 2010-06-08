/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_drawsdk.cpp
 *     Graphics drawing support sdk class.
 * @par Purpose:
 *     A link between game engine and the DirectDraw library.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     16 Nov 2008 - 21 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_drawsdk.hpp"

#include <string.h>
#include <stdio.h>
#include <SDL_syswm.h>

#include "bflib_basics.h"
#include "bflib_mouse.h"
#include "bflib_keybrd.h"

/******************************************************************************/
// Global variables
//For conversion to old screen mode index... Can perhaps be removed.
struct ScreenModeInfo lbScreenModeInfo[]={

    {   0,   0, 0,0,   0x0,"MODE_INVALID"},
    { 320, 200, 8,0,   0x0,"MODE_320_200_8"},
    { 320, 200,16,0,   0x0,"MODE_320_200_16"},
    { 320, 200,24,0,   0x0,"MODE_320_200_24"},
    { 320, 240, 8,0,   0x0,"MODE_320_240_8"},
    { 320, 240,16,0,   0x0,"MODE_320_240_16"},
    { 320, 240,24,0,   0x0,"MODE_320_240_24"},
    { 512, 384, 8,0,   0x0,"MODE_512_384_8"},
    { 512, 384,16,0,   0x0,"MODE_512_384_16"},
    { 512, 384,24,0,0x0100,"MODE_512_384_24"},
    { 640, 400, 8,0,   0x0,"MODE_640_400_8"},
    { 640, 400,16,0,   0x0,"MODE_640_400_16"},
    { 640, 400,24,0,0x0101,"MODE_640_400_24"},
    { 640, 480, 8,0,   0x0,"MODE_640_480_8"},
    { 640, 480,16,0,   0x0,"MODE_640_480_16"},
    { 640, 480,24,0,0x0103,"MODE_640_480_24"},
    { 800, 600, 8,0,   0x0,"MODE_800_600_8"},
    { 800, 600,16,0,   0x0,"MODE_800_600_16"},
    { 800, 600,24,0,0x0105,"MODE_800_600_24"},
    {1024, 768, 8,0,   0x0,"MODE_1024_768_8"},
    {1024, 768,16,0,   0x0,"MODE_1024_768_16"},
    {1024, 768,24,0,0x0107,"MODE_1024_768_24"},
    {1280,1024, 8,0,   0x0,"MODE_1280_1024_8"},
    {1280,1024,16,0,   0x0,"MODE_1280_1024_16"},
    {1280,1024,24,0,   0x0,"MODE_1280_1024_24"},
    {1600,1200, 8,0,   0x0,"MODE_1600_1200_8"},
    {1600,1200,16,0,   0x0,"MODE_1600_1200_16"},
    {1600,1200,24,0,   0x0,"MODE_1600_1200_24"},
    {   0,   0, 0,0,   0x0,"MODE_INVALID"},
};
typedef struct ScreenModeInfo TbScreenModeInfo;

/******************************************************************************/
// Methods

TDDrawSdk::TDDrawSdk(void) : TDDrawBaseClass(),
		screenSurface(NULL), drawSurface(NULL), lockCount(0), hasSecondSurface(false)
{
}

TDDrawSdk::~TDDrawSdk(void)
{
}

bool TDDrawSdk::get_palette(void *palette,unsigned long base,unsigned long numEntries)
{
  SYNCDBG(12,"Starting");

  char * destColors = (char *) palette;
  const SDL_Color * const srcColors = drawSurface->format->palette->colors;
  unsigned long i;
  for (i = 0; i < numEntries; ++i) {
      destColors[0] = srcColors[base+i].r >> 2;
      destColors[1] = srcColors[base+i].g >> 2;
      destColors[2] = srcColors[base+i].b >> 2;
      destColors += 3;
  }

  return true;
}

bool TDDrawSdk::set_palette(void *palette,unsigned long base,unsigned long numEntries)
{
  SYNCDBG(12,"Starting");

  SDL_Color * const destColors = (SDL_Color *) malloc(sizeof(*destColors) * numEntries);
  const char * srcColors = (const char *) palette;
  unsigned long i;
  for (i = 0; i < numEntries; ++i) {
      destColors[i].r = srcColors[0] << 2;
      destColors[i].g = srcColors[1] << 2;
      destColors[i].b = srcColors[2] << 2;
      srcColors += 3;
  }

  SDL_SetPalette(drawSurface, SDL_LOGPAL | SDL_PHYSPAL, destColors, base, numEntries);
  free(destColors);

  return true;
}

bool TDDrawSdk::setup_screen(TbScreenMode * mode)
{
  SYNCDBG(12,"Starting");

  //TODO: improve error handling in this function, and cleaning up when resetting video mode...

  bool fullscreen = !mode->windowed;

  // SDL video mode flags.

  unsigned long sdlFlags = 0;
  if (mode->bpp == 8) {
    sdlFlags |= SDL_DOUBLEBUF;
    sdlFlags |= SDL_HWPALETTE;
  }
  if (fullscreen) {
    sdlFlags |= SDL_FULLSCREEN;
  }

  // Set SDL video mode (also creates window).

  screenSurface = drawSurface = SDL_SetVideoMode(mode->width, mode->height, mode->bpp, sdlFlags);

  if (screenSurface == NULL) {
      ERRORLOG("Failed to initialize SDL video mode.");
      return false;
  }

  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_SetCaption(lbDrawAreaTitle, lbDrawAreaTitle);
  setIcon();

  // Create secondary surface if necessary. Right now, only if BPP != 8.
  //TODO: utilize this for rendering in different resolution later
  if (mode->bpp != 8) {
	  drawSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, mode->width, mode->height, 8, 0, 0, 0, 0);

	  if (drawSurface == NULL) {
		  screenSurface = NULL;
		  ERRORLOG("Can't create secondary surface");
		  return false;
	  }

	  hasSecondSurface = true;
  }

  return true;
}

bool TDDrawSdk::lock_screen(void)
{
  SYNCDBG(12,"Starting");

  if (SDL_LockSurface(drawSurface) < 0) {
      lbDisplay.GraphicsWindowPtr = NULL;
      lbDisplay.WScreen = NULL;
      return false;
  }

  lockCount++;
  lbDisplay.WScreen = (unsigned char *) drawSurface->pixels;
  lbDisplay.GraphicsScreenWidth = drawSurface->pitch;
  lbDisplay.GraphicsWindowPtr = &lbDisplay.WScreen[lbDisplay.GraphicsWindowX +
                                                   lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsWindowY];

  return true;
}

bool TDDrawSdk::unlock_screen(void)
{
  SYNCDBG(12,"Starting");

  if (lockCount <= 0) {
      WARNLOG("Unmatching call");
  }

  lockCount--;
  lbDisplay.WScreen = NULL;
  lbDisplay.GraphicsWindowPtr = NULL;

  SDL_UnlockSurface(drawSurface);

  return true;
}

bool TDDrawSdk::clear_screen(unsigned long color)
{
  SYNCDBG(12,"Starting");

  if (SDL_FillRect(drawSurface, NULL, color) < 0) {
      ERRORLOG("Error while clearing screen.");
      return false;
  }

  return true;
}

bool TDDrawSdk::clear_window(long x,long y,unsigned long w,unsigned long h,unsigned long color)
{
  SYNCDBG(12,"Starting");

  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;

  if (SDL_FillRect(drawSurface, &rect, color) < 0) {
      ERRORLOG("Error when clearing window.");
      return false;
  }

  return true;
}

bool TDDrawSdk::swap_screen(void)
{
	SYNCDBG(12,"Starting");

	if (hasSecondSurface) {
		if (SDL_BlitSurface(drawSurface, NULL, screenSurface, NULL) == -1) {
			ERRORLOG("SDL_BlitSurface failed.");
			return false;
		}
	}

	if (SDL_Flip(screenSurface) < 0) { //calls SDL_UpdateRect for entire screen if not double buffered
		ERRORLOG("SDL_Flip failed.");
		return false;
	}

  return true;
}

bool TDDrawSdk::reset_screen(void)
{
  if (hasSecondSurface) {
	  SDL_FreeSurface(drawSurface);
  }

  //do not free screen surface, it is freed automatically on SDL_Quit or next call to set video mode

  hasSecondSurface = false;
  drawSurface = NULL;
  screenSurface = NULL;

  return true;
}

bool TDDrawSdk::swap_box(struct tagPOINT coord,struct tagRECT &rect)
{
  //SDL perhaps doesn't even let us touch primary surface, so swap entire screen for now (hope this works)
  return swap_screen();;
}

bool TDDrawSdk::create_surface(struct SSurface *surf,unsigned long w,unsigned long h)
{
  SDL_PixelFormat * const format = drawSurface->format;

  surf->surf = SDL_CreateRGBSurface(SDL_SRCCOLORKEY | SDL_HWSURFACE, w, h, format->BitsPerPixel,
      format->Rmask, format->Gmask, format->Bmask, format->Amask);

  if (surf->surf == NULL) {
      ERRORLOG("Failed to create surface.");
      return false;
  }

  //moved color key control to blt_surface()

  return true;
}

bool TDDrawSdk::release_surface(struct SSurface *surf)
{
  if (surf->surf == NULL) {
    return false;
  }

  SDL_FreeSurface(surf->surf);
  surf->surf = NULL;

  return true;
}

bool TDDrawSdk::blt_surface(struct SSurface *surf, unsigned long x, unsigned long y,
    tagRECT *rect, unsigned long blflags)
{
  // Convert to SDL rectangles:

  SDL_Rect srcRect;
  SDL_Rect destRect;

  srcRect.x = rect->left;
  srcRect.y = rect->top;
  srcRect.w = rect->right - rect->left;
  srcRect.h = rect->bottom - rect->top;

  destRect.x = x;
  destRect.y = y;
  destRect.w = srcRect.w;
  destRect.h = srcRect.h;

  // Set blit parameters:

  if ((blflags & 0x02) != 0) {
    //TODO: see how/if to handle this, I interpret this as "blit directly to primary rather than back"
    //secSurf = lpDDSurface3;
	//I think it can simply be deleted as not even the mouse pointer code is using it and there's no way
	//to access front buffer in SDL
  }
  if ((blflags & 0x04) != 0) {
	  //enable color key
      SDL_SetColorKey(surf->surf, SDL_SRCCOLORKEY, 255);
  }
  else {
	  //disable color key
      SDL_SetColorKey(surf->surf, 0, 255);
  }
  if ((blflags & 0x10) != 0) {
      //TODO: see if this can/should be handled
	  //probably it can just be deleted
      //dwTrans |= DDBLTFAST_WAIT;
  }

  // Blit:

  //unfortunately we must fool SDL because it has a per-surface palette for 8 bit surfaces, DK does not
  //set the palette of any off-screen surfaces, so temporarily change palette
  SDL_Palette * paletteBackup = NULL;
  if (surf->surf->format->BitsPerPixel == 8) {
	  paletteBackup = surf->surf->format->palette;
	  surf->surf->format->palette = drawSurface->format->palette;
  }

  //the blit
  if ((blflags & 0x08) != 0) {
	//surface to screen
    SDL_BlitSurface(surf->surf, &srcRect, drawSurface, &destRect);
  }
  else {
	//screen to surface
    SDL_BlitSurface(drawSurface, &destRect, surf->surf, &srcRect);
  }

  //restore palette
  if (surf->surf->format->BitsPerPixel == 8) {
    surf->surf->format->palette = paletteBackup;
  }

  return true;
}

void *TDDrawSdk::lock_surface(struct SSurface *surf)
{
  if (surf->surf == NULL) {
    return NULL;
  }

  if (SDL_LockSurface(surf->surf) < 0) {
      ERRORLOG("Failed to lock surface");
      return NULL;
  }

  surf->locks_count++;
  surf->pitch = surf->surf->pitch;
  return surf->surf->pixels;
}

bool TDDrawSdk::unlock_surface(struct SSurface *surf)
{
  if (surf->locks_count == 0)
  {
    return true;
  }

  surf->locks_count = 0;

  if (surf->surf == NULL)
  {
    return false;
  }

  SDL_UnlockSurface(surf->surf);

  return true;
}

LPCTSTR TDDrawSdk::resource_mapping(int index)
{
  switch (index)
  {
  case 1:
      return "A";
      //return MAKEINTRESOURCE(110); -- may work for other resource compilers
  default:
      return NULL;
  } return NULL;
}

void TDDrawSdk::setIcon(void)
{
  //TODO: replace with portable version

  HICON hIcon;
  SDL_SysWMinfo wmInfo;

  SDL_VERSION(&wmInfo.version);
  if (SDL_GetWMInfo(&wmInfo) < 0) {
	  WARNLOG("Couldn't get SDL window info, therefore cannot set icon");
	  return;
  }

  hIcon = LoadIcon(lbhInstance, resource_mapping(lbIconIndex));
  SendMessage(wmInfo.window, WM_SETICON, ICON_BIG,  (LPARAM)hIcon);
  SendMessage(wmInfo.window, WM_SETICON, ICON_SMALL,(LPARAM)hIcon);
}

bool TDDrawSdk::isModePossible(TbScreenMode * mode)
{
	unsigned long sdlFlags = 0;
	if (mode->bpp == 8) {
		sdlFlags |= SDL_HWPALETTE | SDL_DOUBLEBUF;
	}
	if (!mode->windowed) {
		sdlFlags |= SDL_FULLSCREEN;
	}

	return SDL_VideoModeOK(mode->width, mode->height, mode->bpp, sdlFlags) == mode->bpp;
}

/******************************************************************************/
