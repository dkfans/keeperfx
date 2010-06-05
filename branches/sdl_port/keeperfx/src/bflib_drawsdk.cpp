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

HRESULT lbDDRval = DD_OK;
volatile int lbWait = 0;
volatile long backLockCount = 0;
/******************************************************************************/
// Methods

TDDrawSdk::TDDrawSdk(void) : TDDrawBaseClass()
{
  /*lpDDInterface = NULL;
  this->lpDDSurface3 = NULL;
  this->lpDDSurface2 = NULL;
  this->lpDDSurface1 = NULL;
  this->lpDDPalette = NULL;
  this->vidMode = Lb_SCREEN_MODE_INVALID;
  this->resWidth = 0;
  this->resHeight = 0;
  this->window_created = 0;
  hThread = NULL;*/
	screenSurface = NULL;
	drawSurface = NULL;
	flags = 0;
	hasSecondSurface = false;
}

TDDrawSdk::~TDDrawSdk(void)
{
  /*if (lpDDInterface != NULL)
  {
    release_palettes();
    release_surfaces();
    lpDDInterface->SetCooperativeLevel(hWindow, DDSCL_NORMAL);
    lpDDInterface->Release();
    lpDDInterface = NULL;
  }*/
	 remove_sdk_window();
}

bool TDDrawSdk::setup_window(void)
{
  /*DWORD nThreadId;
  SYNCDBG(12,"Starting");
  if (hThread != NULL)
  {
    return true;
  }
  hThread = CreateThread(NULL, 0, sdk_window_thread, this, 0, &nThreadId);
  if (hThread == NULL)
  {
    return false;
  }
  while ( !this->window_created )
  {
    SleepEx(100, 0);
  }*/
  return true;
}

long TDDrawSdk::WindowProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
  /*TbScreenModeInfo *mdinfo;
  TDDrawSdk *sdk;
  struct tagPOINT mouse_pos;
  switch (message)
  {
  case WM_DESTROY:
      if (window_created)
      {
          window_created = 0;
          PostQuitMessage(0);
          // Sometimes PostQuitMessage isn't able to send it properly, so:
          PostMessage(hWindow, WM_QUIT, 0, 0);
      } else
      {
          PostQuitMessage(0);
      }
      return DefWindowProcA(hWnd, message, wParam, lParam);

  case WM_SETCURSOR:
      if (this->active)
      {
        SetCursor(0);
        return 1;
      }
      return DefWindowProcA(hWnd, message, wParam, lParam);

  case WM_ACTIVATEAPP:
      this->active = (wParam) && (GetForegroundWindow() == hWnd);
      return 0;

  case WM_CLOSE:
      lbUserQuit = 1;
      return 0;

  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
      KeyboardProc(0, 0, lParam);
      return 0;

  case WM_MOUSEMOVE:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_LBUTTONDBLCLK:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_RBUTTONDBLCLK:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_MBUTTONDBLCLK:
      mouse_pos.x = (unsigned long)(lParam) & 0xffff;
      mouse_pos.y = (unsigned long)(lParam >> 16) & 0xffff;
      mouseControl(message, &mouse_pos);
      return DefWindowProcA(hWnd, message, wParam, lParam);

  case WM_USER+100:
      sdk = (TDDrawSdk *)lParam;
      if ( !sdk->setup_direct_draw() )
        lbDDRval = DDERR_GENERIC;
      lbWait = 0;
      return 0;

  case WM_USER+101:
      sdk = (TDDrawSdk *)lParam;
      sdk->reset_direct_draw();
      lbWait = 0;
      return 0;

  case WM_USER+102:
      mdinfo = TDDrawSdk::get_mode_info(*(long *)wParam);
      if ( !setup_surfaces(mdinfo->Width,mdinfo->Height,mdinfo->BitsPerPixel) )
          lbDDRval = DDERR_GENERIC;
      lbWait = 0;
      return 0;

  case WM_QUIT:
      DestroyWindow(hWindow);
      return 0;

  default:
      return DefWindowProcA(hWnd, message, wParam, lParam);
  }*/
  return 0;
}

void TDDrawSdk::find_video_modes(void)
{
  /*if ((flags & DMF_DoneSetup) == 0)
  {
    WARNLOG("Direct Draw not set up.");
    return;
  }
  lpDDInterface->EnumDisplayModes(0, NULL, NULL, );
  if ((flags & DMF_LoresForceAvailable) != 0)
    lbScreenModeInfo[Lb_SCREEN_MODE_320_200_8].Available = 1;*/
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

  /*if (!LbScreenIsModeAvailable(mode)) { //TODO: implement properly first
    ERRORLOG("screen mode %d not available",(int)mode);
    return false;
  }*/

  // Set mostly obsolete flags.

  flags &= ~DMF_DoneSetup;
  flags &= ~DMF_SurfacesSetup;
  flags &= ~DMF_Unknown0010;
  flags &= ~DMF_Unknown0020;
  flags |= DMF_PaletteSetup;
  flags |= DMF_ControlDisplayMode;
  flags |= DMF_Unknown0100;
  flags |= DMF_Unknown0200;
  flags &= ~DMF_LoresForceAvailable;
  flags &= ~DMF_Unknown0800;
  flags &= ~DMF_Unknown1000;

  //setup SDL something...

  flags |= DMF_DoneSetup;

  //release_surfaces();
  bool fullscreen = !mode->windowed;
  //bool wscreen = is_wscreen_in_video();

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
  SetIcon();

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

  flags |= DMF_SurfacesSetup;

  // Update DK display struct.
  //TODO: actually I've commented it out because LbScreenSetup already does this AFAIK

  /*lbDisplay.DrawFlags = 0;
  lbDisplay.DrawColour = 0;
  lbDisplay.GraphicsScreenWidth = screenSurface->pitch; //TODO: note this
  lbDisplay.GraphicsScreenHeight = resHeight;
  lbDisplay.PhysicalScreenWidth = resWidth;
  lbDisplay.PhysicalScreenHeight = resHeight;
  lbDisplay.ScreenMode = ;
  lbDisplay.WScreen = NULL;

  // Set graphics window...
  LbScreenSetGraphicsWindow(0, 0, resWidth, resHeight); //already done by LbScreenSetup */

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

  backLockCount++;
  lbDisplay.WScreen = (unsigned char *) drawSurface->pixels;
  lbDisplay.GraphicsScreenWidth = drawSurface->pitch;
  lbDisplay.GraphicsWindowPtr = &lbDisplay.WScreen[lbDisplay.GraphicsWindowX +
                                                   lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsWindowY];

  return true;
}

bool TDDrawSdk::unlock_screen(void)
{
  SYNCDBG(12,"Starting");

  if (backLockCount <= 0) {
      WARNLOG("Unmatching call");
  }

  backLockCount--;
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

  //TODO: study original logic, see if it's needed...

  /*HRESULT locRet;
  static RECT srcRect;
  static RECT destRect;

  if ((flags & DMF_SurfacesSetup) == 0)
    return false;
  if ((flags & DMF_DoubleBuffering) != 0)
  {
    if ((flags & DMF_WScreenInVideo) == 0)
    {
      if ((flags & DMF_LoresEmulation) != 0)
      {
        srcRect.left = 0;
        srcRect.top = 0;
        srcRect.right = 320;
        srcRect.bottom = 200;
        locRet = lpDDSurface2->Blt(0, lpDDSurface1, &srcRect, DDBLT_WAIT, 0);
      } else
      {
        locRet = lpDDSurface2->BltFast(0, 0, lpDDSurface1, 0, DDBLTFAST_WAIT);
      }
      if (locRet == DDERR_SURFACELOST)
      {
        WARNLOG("DDraw surface lost - restoring.");
        restore_surfaces();
        return false;
      }
      if (locRet != DD_OK)
      {
        ERRORLOG("Swap screen failed in Blt*().");
        return false;
      }
    }
    locRet = lpDDSurface3->Flip(0, 1);
    if (locRet == DDERR_SURFACEBUSY)
    {
      ERRORLOG("Surface busy in Flip().");
      return false;
    }
    if (locRet == DDERR_SURFACELOST)
    {
      WARNLOG("DDraw surface lost - restoring.");
      restore_surfaces();
      return false;
    }
    if (locRet != DD_OK)
    {
      ERRORLOG("Swap screen failed in Flip().");
      return false;
    }
  } else
  {
    if ((flags & DMF_WScreenInVideo) == 0)
    {
      if ((flags & DMF_LoresEmulation) != 0)
      {
        srcRect.left = 0;
        srcRect.top = 0;
        srcRect.right = 320;
        srcRect.bottom = 200;
        destRect.left = 0;
        destRect.top = 40;
        destRect.right = 640;
        destRect.bottom = 440;
        locRet = lpDDSurface3->Blt(&destRect, lpDDSurface1, &srcRect, DDBLT_WAIT, 0);
      } else
      {
        locRet = lpDDSurface3->BltFast(0, 0, lpDDSurface1, 0, DDBLTFAST_WAIT);
      }
      if (locRet == DDERR_SURFACELOST)
      {
        WARNLOG("DDraw surface lost - restoring.");
        restore_surfaces();
        return false;
      }
      if (locRet != DD_OK)
      {
        ERRORLOG("Swap screen failed in Blt*().");
        return false;
      }
    }
  }*/

  return true;
}

bool TDDrawSdk::reset_screen(void)
{
  if (hasSecondSurface) {
	  SDL_FreeSurface(drawSurface);
  }

  //do not free screen surface, it is freed automatically on SDL_Quit or next call to set video mode

  hasSecondSurface = NULL;
  drawSurface = NULL;
  screenSurface = NULL;

  return true;
}

bool TDDrawSdk::restore_surfaces(void)
{
  /*if ((flags & DMF_SurfacesSetup) == 0)
    return false;
  if (lpDDSurface3 != NULL)
  {
    if (lpDDSurface3->IsLost() == DDERR_SURFACELOST)
    {
      ddResult = lpDDSurface3->Restore();
      if (lpDDPalette != NULL)
        lpDDSurface3->SetPalette(lpDDPalette);
    }
  }
  if (lpDDSurface1 != NULL)
  {
    if (lpDDSurface1->IsLost() == DDERR_SURFACELOST)
    {
      ddResult = lpDDSurface1->Restore();
      if (lpDDPalette != NULL)
        lpDDSurface1->SetPalette(lpDDPalette);
    }
  }
  return (ddResult == DD_OK);*/ return true;
}

void TDDrawSdk::wait_vbi(void)
{
  /*BOOL bIsInVB;
  if (!this->active)
  {
    return;
  }
  bIsInVB = false;
  while (!bIsInVB)
  {
    lpDDInterface->GetVerticalBlankStatus(&bIsInVB);
  }*/
}

bool TDDrawSdk::swap_box(struct tagPOINT coord,struct tagRECT &rect)
{
  //SDL perhaps doesn't even let us touch primary surface, so swap entire screen for now (hope this works)
  swap_screen();

  /*LPDIRECTDRAWSURFACE lpDDSurf;
  HRESULT locRet;
  lpDDSurf = wscreen_surface();
  if (lpDDSurf == NULL)
  {
    return false;
  }
  locRet = lpDDSurface3->BltFast(coord.x, coord.y, lpDDSurf, &rect, DDBLTFAST_WAIT);
  if (locRet == DDERR_SURFACELOST)
  {
    restore_surfaces();
    return false;
  }
  if (ddResult != DD_OK)
  {
    ERRORLOG("Swap Box screen failed.");
    return false;
  }*/
  return true;
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

void TDDrawSdk::LoresEmulation(bool nstate)
{
  /*static bool lre_set = false;
  static long w;
  static long h;
  static TbDisplayStruct backup;
  if (lre_set == nstate)
    return;
  if ((lre_set) || (this->resWidth > 320) && (this->resHeight > 200))
  {
    // Don't allow to change if screen is locked
    if (lbDisplay.WScreen != NULL)
    {
      WARNLOG("Cannot switch lowres emulation while screen is locked!");
      return;
    }
    // Update the flag
    if (nstate)
      flags |= DMF_LoresEmulation;
    else
      flags ^= flags & DMF_LoresEmulation;
    // Do the save or restore
    if (nstate)
    {
      memcpy(&backup, &lbDisplay, 0x76u);
      w = this->resWidth;
      h = this->resHeight;
      this->resWidth = 320;
      this->resHeight = 200;
      lbDisplay.PhysicalScreenWidth = 320;
      lbDisplay.PhysicalScreenHeight = this->resHeight;
      lbDisplay.GraphicsScreenWidth = this->resWidth;
      lbDisplay.GraphicsScreenHeight = this->resHeight;
      LbScreenSetGraphicsWindow(0, 0, this->resWidth, this->resHeight);
      LbMouseSetWindow(0, 0, this->resWidth, this->resHeight);
      lbInteruptMouse = 0;
    } else
    {
      this->resWidth = w;
      this->resHeight = h;
      memcpy(&lbDisplay, &backup, sizeof(lbDisplay));
      lbInteruptMouse = 1;
    }
    lre_set = nstate;
  }*/
}

/**
 * Sends a DD message call to Direct Draw thread.
 */
void TDDrawSdk::SendDDMsg(int message, void *param)
{
  /*Sleep(1);
  lbDDRval = DD_OK;
  lbWait = 1;
  PostMessage(hWindow, message, (WPARAM)param, (LPARAM)this);*/
}

/**
 * Waits until DD call finishes and returns its result.
 */
HRESULT TDDrawSdk::ResultDDMsg(void)
{
  /*while (lbWait)
    Sleep(1);
  return lbDDRval;*/ return 0;
}

//TODO: add screen mode checking code overall
HRESULT CALLBACK TDDrawSdk::screen_mode_callback(LPDDSURFACEDESC lpDDSurf, LPVOID lpContext)
{
  /*TbScreenModeInfo *mdinfo;
  mdinfo = &lbScreenModeInfo[1];
  while (mdinfo->Width > 0)
  {
    if ( (mdinfo->Width == lpDDSurf->dwWidth) && (mdinfo->Height == lpDDSurf->dwHeight)
         && (mdinfo->BitsPerPixel == lpDDSurf->ddpfPixelFormat.dwRGBBitCount) )
    {
      mdinfo->Available = 1;
      return DDENUMRET_OK;
    }
    mdinfo++;
  }*/
  return DDENUMRET_OK;
}

bool TDDrawSdk::setup_direct_draw(void)
{
  /*DDCAPS ddEmulCaps;
  this->flags &= ~DMF_DoneSetup;
  if (this->lpDDInterface == 0)
  {
    ddResult = DirectDrawCreate(0, &lpDDInterface, NULL);
    if (ddResult != DD_OK)
    {
      ERRORLOG("Could not create direct draw object.");
      return false;
    }
    ddEmulCaps.dwSize = sizeof(ddEmulCaps);
    ddDriverCaps.dwSize = sizeof(ddDriverCaps);
    lpDDInterface->GetCaps(&ddDriverCaps, &ddEmulCaps);
  }
  ddResult = lpDDInterface->SetCooperativeLevel(hWindow, DDSCL_ALLOWMODEX|DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE);
  lpDDInterface->SetDisplayMode(640, 480, 8);
  if (ddResult != DD_OK)
  {
    ERRORLOG("Could not set cooperative level.");
    return false;
  }
  flags |= DMF_DoneSetup;*/
  return true;
}

bool TDDrawSdk::reset_direct_draw(void)
{
  /*if ((flags & DMF_DoneSetup) == 0)
  {
    return false;
  }
  if (lpDDInterface != NULL)
  {
    if ((this->ddSurfaceCaps.dwCaps & 0x200000) != 0)
    {
      release_surfaces();
      lpDDInterface->RestoreDisplayMode();
      lpDDInterface->SetCooperativeLevel(hWindow, DDSCL_NORMAL);
      release_palettes();
      lpDDInterface->Release();
      lpDDInterface = NULL;
    } else
    {
      lpDDInterface->SetCooperativeLevel(hWindow, DDSCL_NORMAL);
    }
  }
  flags &= ~DMF_DoneSetup;*/
  return true;
}

bool TDDrawSdk::setup_dds_double_video(void)
{
  /*union {
  DDSURFACEDESC ddSurfDesc;
  DDBLTFX ddBltFx;
  DDSCAPS attchSCaps;
  };
  // Create primary surface
  memset(&ddSurfDesc, 0, sizeof(ddSurfDesc));
  ddSurfDesc.dwSize = sizeof(ddSurfDesc);
  ddSurfDesc.dwFlags = 33;
  ddSurfDesc.dwBackBufferCount = 1;
  ddSurfDesc.ddsCaps.dwCaps = 536;
  ddResult = lpDDInterface->CreateSurface(&ddSurfDesc, &lpDDSurface3, NULL);
  if (ddResult != DD_OK)
  {
    ERRORLOG("Could not create primary surface");
    return false;
  }
  lpDDSurface3->GetCaps(&this->ddSurfaceCaps);
  if (lpDDPalette != NULL)
  {
    ddResult = lpDDSurface3->SetPalette(lpDDPalette);
  }
  // Back buffer
  memset(&ddBltFx, 0, sizeof(ddBltFx));
  ddBltFx.dwFillColor = 0;
  ddBltFx.dwSize = sizeof(ddBltFx);
  lpDDSurface3->Blt(NULL, 0, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddBltFx);
  // Attach surface caps
  attchSCaps.dwCaps = DDSCAPS_BACKBUFFER;
  ddResult = lpDDSurface3->GetAttachedSurface(&attchSCaps, &lpDDSurface2);
  if (ddResult != DD_OK)
  {
    ERRORLOG("Could not create back buffer");
    release_surfaces();
    return false;
  }*/
  return true;
}

bool TDDrawSdk::setup_dds_single_video(void)
{
  /*union {
  DDSURFACEDESC ddSurfDesc;
  DDBLTFX ddBltFx;
  };
  memset(&ddSurfDesc, 0, sizeof(ddSurfDesc));
  ddSurfDesc.dwSize = sizeof(ddSurfDesc);
  ddSurfDesc.dwFlags = 1;
  ddSurfDesc.ddsCaps.dwCaps = 512;
  ddResult = lpDDInterface->CreateSurface(&ddSurfDesc, &lpDDSurface3, NULL);
  if (ddResult != DD_OK)
  {
    ERRORLOG("Could not create surface");
    return false;
  }
  lpDDSurface3->GetCaps(&this->ddSurfaceCaps);
  if (lpDDPalette != NULL)
  {
    ddResult = lpDDSurface3->SetPalette(lpDDPalette);
  }
  memset(&ddBltFx, 0, sizeof(ddBltFx));
  ddBltFx.dwFillColor = 0;
  ddBltFx.dwSize = sizeof(ddBltFx);
  lpDDSurface3->Blt(NULL, 0, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddBltFx);
  return (ddResult < 1);*/ return true;
}

bool TDDrawSdk::setup_dds_system(void)
{
  /*DDSURFACEDESC ddSurfDesc;
  memset(&ddSurfDesc, 0, sizeof(ddSurfDesc));
  ddSurfDesc.dwSize = sizeof(ddSurfDesc);
  ddSurfDesc.dwHeight = this->resHeight;
  ddSurfDesc.dwWidth = this->resWidth;
  ddSurfDesc.dwFlags = 7;
  ddSurfDesc.ddsCaps.dwCaps = 2112;
  ddResult = lpDDInterface->CreateSurface(&ddSurfDesc, &lpDDSurface1, NULL);
  return (ddResult == DD_OK);*/ return true;
}

bool TDDrawSdk::setup_surfaces(short w, short h, short bpp)
{
  /*if ((flags & DMF_DoneSetup) == 0)
  {
    return false;
  }
  lpDDInterface->WaitForVerticalBlank(1,NULL);
  release_surfaces();
  if ((flags & DMF_ControlDisplayMode) != 0)
  {
    ddResult = lpDDInterface->SetCooperativeLevel(hWindow, DDSCL_ALLOWMODEX|DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE);
    ddResult = lpDDInterface->SetDisplayMode(w, h, bpp);
    if (ddResult != DD_OK)
    {
      ERRORLOG("Could not set screen res");
      return false;
    }
  }
  this->resWidth = w;
  this->resHeight = h;
  if (is_double_buffering_video())
  {
    if (!setup_dds_double_video())
      set_double_buffering_video(false);
  }
  if (!is_double_buffering_video())
  {
    if (!setup_dds_single_video())
    {
      return false;
    }
  }
  if (!is_wscreen_in_video())
  {
    if (!setup_dds_system())
      set_wscreen_in_video(true);
  }
  flags |= DMF_SurfacesSetup;*/
  return true;
}

bool TDDrawSdk::release_surfaces(void)
{
  /*LPDIRECTDRAWSURFACE lpDDSurLocal;
  if (!this->active)
    return false;
  flags &= ~DMF_SurfacesSetup;
  if ((flags & DMF_DoneSetup) != 0)
  {
    lpDDSurLocal = lpDDSurface3;
    if (lpDDSurLocal != NULL)
    {
      lpDDSurface3 = NULL;
      while (lpDDSurLocal->Release() != DD_OK)
        Sleep(1);
    }
    lpDDSurLocal = lpDDSurface1;
    if (lpDDSurLocal != NULL)
    {
      lpDDSurface1 = NULL;
      while (lpDDSurLocal->Release() != DD_OK)
        Sleep(1);
    }
  }*/
  return true;
}

bool TDDrawSdk::release_palettes(void)
{
  /*LPDIRECTDRAWPALETTE lpDDPalLocal;
  if ((flags & DMF_DoneSetup) == 0)
    return false;
  lpDDPalLocal = lpDDPalette;
  if (lpDDPalette != NULL)
  {
    lpDDPalette = NULL;
    while (lpDDPalLocal->Release() != DD_OK)
    { Sleep(1); }
  }*/
  return true;
}

LPDIRECTDRAWSURFACE TDDrawSdk::wscreen_surface(void)
{
  /*if ((flags & DMF_DoneSetup) == 0)
  {
    return NULL;
  } else
  if ((flags & DMF_SurfacesSetup) == 0)
  {
    return NULL;
  } else
  if ((flags & DMF_WScreenInVideo) == 0)
  {
    return lpDDSurface1;
  } else
  if ((flags & DMF_DoubleBuffering) == 0)
  {
    return lpDDSurface3;
  } else
  {
    return lpDDSurface2;
  }*/ return NULL;
}

DWORD CALLBACK TDDrawSdk::sdk_window_thread(LPVOID lpParam)
{
  /*struct TDDrawSdk *pthis;
  struct tagMSG tMsg;
  BOOL gmRet;
  pthis = (struct TDDrawSdk *)lpParam;
  //SYNCDBG(0,"Starting Sdk Thread.");
  if (!pthis->create_sdk_window())
  {
    return 0;
  }
  pthis->window_created = 1;
  while ((gmRet = GetMessage(&tMsg, pthis->hWindow, 0, 0)) != 0)
  {
    if (gmRet == -1)
    {
        SYNCDBG(0,"Error received from GetMessage().");
        break;
    }
    //pthis->WindowProc(tMsg.hwnd, tMsg.message, tMsg.wParam, tMsg.lParam);
    //TranslateMessage(&tMsg);
    DispatchMessage(&tMsg);
  }
  //SYNCDBG(0,"Ending Sdk Thread.");*/
  return 0;
}

bool TDDrawSdk::create_sdk_window(void)
{
 /* int w,h;
  w = GetSystemMetrics(SM_CXSCREEN);
  h = GetSystemMetrics(SM_CYSCREEN);
  hWindow = CreateWindowEx(WS_EX_APPWINDOW,appName,appTitle,
      WS_SYSMENU|CW_USEDEFAULT|WS_VISIBLE,0,0,w,h,NULL,NULL,lbhInstance,NULL);
  if (hWindow == NULL)
  {
    return false;
  }
  SetIcon();
  UpdateWindow(hWindow);
  ShowWindow(hWindow, 1);
  SetFocus(hWindow);*/
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

void TDDrawSdk::SetIcon(void)
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
  //hIcon = LoadIcon(lbhInstance, resource_mapping(lbIconIndex));
  SendMessage(wmInfo.window, WM_SETICON, ICON_SMALL,(LPARAM)hIcon);
}

bool TDDrawSdk::remove_sdk_window(void)
{
  /*DWORD ret;
  if (this->window_created)
  {
    SYNCDBG(0,"closing down Sdk Window.");
    PostMessage(hWindow, WM_DESTROY, 0, 0);
    ret = WaitForSingleObject(hThread, 4000); // originally was 10000
    if ((ret == WAIT_FAILED) || (ret == WAIT_TIMEOUT))
    {
      SYNCDBG(0,"Timed out waiting for Sdk thread to terminate");
      TerminateThread(hThread, 0);
    }
    CloseHandle(hThread);
    hThread = NULL;
    this->window_created = 0;
    hWindow = NULL;
    return true;
  }*/
  return false;
}

bool TDDrawSdk::is_mode_possible(TbScreenMode * mode)
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

TbScreenMode TDDrawSdk::get_mode_info_by_str(char *str)
{
  /*int maxmode=sizeof(lbScreenModeInfo)/sizeof(TbScreenModeInfo);
  int mode;
  for (mode=0; mode<maxmode; mode++)
  {
    if (stricmp(lbScreenModeInfo[mode].Desc,str) == 0)
      return (TbScreenMode)mode;
  }
  return Lb_SCREEN_MODE_INVALID;*/
	TbScreenMode null = { -1, -1, -1 };
	return null;
}

/******************************************************************************/
