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
 * @author   Tomasz Lis
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
  lpDDInterface = NULL;
  this->lpDDSurface3 = NULL;
  this->lpDDSurface2 = NULL;
  this->lpDDSurface1 = NULL;
  this->lpDDPalette = NULL;
  this->vidMode = Lb_SCREEN_MODE_INVALID;
  this->resWidth = 0;
  this->resHeight = 0;
  this->field_180 = 0;
  this->window_created = 0;
  this->need_restore_palettes = false;
  hThread = NULL;
  flags = 0;
}

TDDrawSdk::~TDDrawSdk(void)
{
  if (lpDDInterface != NULL)
  {
    release_palettes();
    release_surfaces();
    lpDDInterface->SetCooperativeLevel(hWindow, DDSCL_NORMAL);
    lpDDInterface->Release();
    lpDDInterface = NULL;
  }
  remove_sdk_window();
}

bool TDDrawSdk::setup_window(void)
{
  DWORD nThreadId;
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
  }
  return true;
}

long TDDrawSdk::WindowProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
  TbScreenModeInfo *mdinfo;
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

  case WM_PALETTECHANGED:
      if ((HWND)wParam != hWnd)
      {
          this->need_restore_palettes = true;
      }
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
  }
  return 0;
}

void TDDrawSdk::find_video_modes(void)
{
  if ((flags & DMF_DoneSetup) == 0)
  {
    WARNLOG("Direct Draw not set up.");
    return;
  }
  lpDDInterface->EnumDisplayModes(0, NULL, NULL, screen_mode_callback);
  if ((flags & DMF_LoresForceAvailable) != 0)
    lbScreenModeInfo[Lb_SCREEN_MODE_320_200_8].Available = 1;
}

bool TDDrawSdk::get_palette(void *palette,unsigned long base,unsigned long numEntries)
{
  PALETTEENTRY ddEntries[256];
  unsigned char *palptr;
  PALETTEENTRY *ddpptr;
  HRESULT locRet;
  long i;
  SYNCDBG(12,"Starting");
  if (lpDDPalette == NULL)
  {
    return false;
  }
  if ((flags & DMF_PaletteSetup) == 0)
  {
    return false;
  }
  // Get palette from DDraw
  locRet = lpDDPalette->GetEntries(0, base, numEntries, ddEntries);
  if (locRet == DDERR_SURFACELOST)
  {
    WARNLOG("DDraw surface lost - restoring.");
    restore_surfaces();
    locRet = lpDDPalette->GetEntries(0, base, numEntries, ddEntries);
  }
  if (locRet != DD_OK)
  {
    return false;
  }
  // Convert the palette to the library format
  palptr = (unsigned char *)palette;
  ddpptr = ddEntries;
  for (i=numEntries; i > 0; i--)
  {
    palptr[0] = (ddpptr->peRed >> 2);
    palptr[1] = (ddpptr->peGreen >> 2);
    palptr[2] = (ddpptr->peBlue >> 2);
    ddpptr++;
    palptr+=3;
  }
  return true;
}

bool TDDrawSdk::set_palette(void *palette,unsigned long base,unsigned long numEntries)
{
  PALETTEENTRY ddEntries[256];
  unsigned char *palptr;
  PALETTEENTRY *ddpptr;
  HRESULT locRet;
  long i;
  SYNCDBG(12,"Starting");
  if (!this->active)
  {
    return false;
  }
  if ((flags & DMF_PaletteSetup) == 0)
  {
    return false;
  }
  if (lpDDInterface == NULL)
  {
    ERRORLOG("DirectDraw not set up");
    return false;
  }
  // If not initiialized yet, then all entries must be set
  if (lpDDPalette == NULL)
  {
    if ((base != 0) || (numEntries != 256))
    {
      ERRORLOG("Partial setting of palette is not supported this time");
      return false;
    }
  }
  // Convert the palette to the library format
  palptr = (unsigned char *)palette;
  ddpptr = ddEntries;
  for (i=numEntries; i > 0; i--)
  {
    ddpptr->peRed  = palptr[0] << 2;
    ddpptr->peGreen = palptr[1] << 2;
    ddpptr->peBlue = palptr[2] << 2;
    ddpptr++;
    palptr+=3;
  }
  if (lpDDSurface3 == NULL)
  {
    return true;
  }
  if (lpDDPalette == NULL)
  {
    locRet = lpDDInterface->CreatePalette(DDPCAPS_ALLOW256|DDPCAPS_8BIT, ddEntries, &lpDDPalette, NULL);
    if (locRet != DD_OK)
    {
      ERRORLOG("Cannot create palette");
      return false;
    }
    this->need_restore_palettes = false;
    locRet = lpDDSurface3->SetPalette(lpDDPalette);
    if (locRet != DD_OK)
    {
      ERRORLOG("Cannot set the newly created palette");
      this->need_restore_palettes = true;
      return false;
    }
    return true;
  }
  locRet = lpDDPalette->SetEntries(0, base, numEntries, ddEntries);
  if (locRet == DDERR_SURFACELOST)
  {
    WARNLOG("DDraw surface lost - restoring.");
    restore_surfaces();
    locRet = lpDDPalette->SetEntries(0, base, numEntries, ddEntries);
  }
  if (locRet != DD_OK)
  {
    ERRORLOG("Cannot set palette entries");
    return false;
  }
  return true;
}

bool TDDrawSdk::setup_screen(TbScreenMode mode)
{
  TbScreenModeInfo *mdinfo;
  DDSURFACEDESC ddSurfDesc;
  SYNCDBG(12,"Starting");
  reset_screen();
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
  if ( !setup_window() )
  {
    ERRORLOG("Could not set up the SDK window.");
    return false;
  }
  SendDDMsg(WM_USER+100, 0);
  if (ResultDDMsg() != DD_OK)
    return false;

  if ( !LbScreenIsModeAvailable(mode) )
  {
    ERRORLOG("screen mode %d not available",(int)mode);
    return false;
  }
  SendDDMsg(WM_USER+102, &mode);
  if (ResultDDMsg() != DD_OK)
    return false;
  mdinfo = TDDrawSdk::get_mode_info(mode);
  if ((mdinfo->Width == 320) && ((flags & DMF_DoubleBuffering) == 0))
  {
    ERRORLOG("Could not setup double buffering for ModeX");
    return false;
  }
  this->vidMode = mode;
  this->field_180 = mdinfo->Width;
  if (((flags & DMF_WScreenInVideo) != 0) && (mdinfo->Width != 320))
  {
    memset(&ddSurfDesc, 0, sizeof(ddSurfDesc));
    ddSurfDesc.dwSize = sizeof(ddSurfDesc);
    if ((flags & DMF_DoubleBuffering) != 0)
    {
      ddResult = lpDDSurface2->Lock(NULL, &ddSurfDesc, DDLOCK_WAIT, NULL);
      if (ddResult != DD_OK)
      {
        ERRORLOG("Could not lock back surface");
        return false;
      }
      lpDDSurface2->Unlock(NULL);
    } else
    {
      ddResult = lpDDSurface3->Lock(NULL, &ddSurfDesc, DDLOCK_WAIT, NULL);
      if (ddResult != DD_OK)
      {
        ERRORLOG("Could not lock primary surface");
        return false;
      }
      lpDDSurface3->Unlock(NULL);
    }
    SYNCLOG("Changing Pitch from %d to %d", this->field_180, ddSurfDesc.lPitch);
    this->field_180 = ddSurfDesc.lPitch;
  } else
  {
    SYNCDBG(1,"Pitch stays at %d", this->field_180);
  }
  lbDisplay.DrawFlags = 0;
  lbDisplay.DrawColour = 0;
  lbDisplay.GraphicsScreenWidth = this->field_180;
  lbDisplay.GraphicsScreenHeight = this->resHeight;
  lbDisplay.PhysicalScreenWidth = this->resWidth;
  lbDisplay.PhysicalScreenHeight = this->resHeight;
  lbDisplay.ScreenMode = mode;
  lbDisplay.WScreen = NULL;
  LbScreenSetGraphicsWindow(0, 0, this->resWidth, this->resHeight);
  return true;
}

bool TDDrawSdk::lock_screen(void)
{
  LPDIRECTDRAWSURFACE lpDDSurf;
  DDSURFACEDESC ddSurfDesc;
  SYNCDBG(12,"Starting");
  lpDDSurf = wscreen_surface();
  if (lpDDSurf == NULL)
  {
    //ERRORLOG("no valid wscreen.");
    return false;
  }
  memset(&ddSurfDesc, 0, sizeof(ddSurfDesc));
  ddSurfDesc.dwSize = sizeof(ddSurfDesc);
  while (backLockCount)
  {  Sleep(1); }
  ddResult = lpDDSurf->Lock(NULL, &ddSurfDesc, DDLOCK_WAIT, NULL);
  if (ddResult == DDERR_SURFACELOST)
  {
    WARNLOG("DDraw surface lost - restoring.");
    restore_surfaces();
    lbDisplay.GraphicsWindowPtr = NULL;
    lbDisplay.WScreen = NULL;
    return false;
  }
  if (ddResult != DD_OK)
  {
    lbDisplay.GraphicsWindowPtr = NULL;
    lbDisplay.WScreen = NULL;
    return false;
  }
  lbDisplay.WScreen = (unsigned char *)ddSurfDesc.lpSurface;
  backLockCount++;
  lbDisplay.GraphicsScreenWidth = ddSurfDesc.lPitch;
  lbDisplay.GraphicsWindowPtr = &((unsigned char *)ddSurfDesc.lpSurface)[lbDisplay.GraphicsWindowX + lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsWindowY];
  return true;
}

bool TDDrawSdk::unlock_screen(void)
{
  LPDIRECTDRAWSURFACE lpDDSurf;
  HRESULT locRet;
  SYNCDBG(12,"Starting");
  lpDDSurf = wscreen_surface();
  if (lpDDSurf == NULL)
  {
    ERRORLOG("no valid wscreen.");
    return false;
  }
  locRet = lpDDSurf->Unlock(NULL);
  switch (locRet)
  {
  case DDERR_NOTLOCKED:
      WARNLOG("Trying to unlock surface which is not locked");
      lbDisplay.WScreen = NULL;
      lbDisplay.GraphicsWindowPtr = NULL;
      break;
  case DDERR_GENERIC:
      WARNLOG("DDraw Generic Error while unlocking");
      break;
  case DD_OK:
      lbDisplay.WScreen = NULL;
      lbDisplay.GraphicsWindowPtr = NULL;
      backLockCount = 0;
      break;
  case DDERR_SURFACELOST:
      WARNLOG("DDraw surface lost - restoring.");
      restore_surfaces();
      backLockCount--;
      break;
  default:
      break;
  }
  return true;
}

bool TDDrawSdk::clear_screen(unsigned long color)
{
  LPDIRECTDRAWSURFACE lpDDSurf;
  HRESULT locRet;
  DDBLTFX ddBltFx;
  SYNCDBG(12,"Starting");
  lpDDSurf = wscreen_surface();
  if (lpDDSurf == NULL)
  {
    ERRORLOG("No valid wscreen.");
    return false;
  }
  memset(&ddBltFx, 0, sizeof(ddBltFx));
  ddBltFx.dwFillColor = color;
  ddBltFx.dwSize = sizeof(ddBltFx);
  locRet = lpDDSurf->Blt(NULL, 0, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddBltFx);
  if (locRet == DDERR_SURFACELOST)
  {
    WARNLOG("DDraw surface lost - restoring.");
    restore_surfaces();
    return false;
  }
  if (locRet != DD_OK)
  {
    ERRORLOG("Clear screen failed.");
    return false;
  }
  return true;
}

bool TDDrawSdk::clear_window(long x,long y,unsigned long w,unsigned long h,unsigned long color)
{
  // Note that it's a copy of clear_screen() - those lazy programmers...
  LPDIRECTDRAWSURFACE lpDDSurf;
  HRESULT locRet;
  DDBLTFX ddBltFx;
  SYNCDBG(12,"Starting");
  lpDDSurf = wscreen_surface();
  if (lpDDSurf == NULL)
  {
    ERRORLOG("no valid wscreen.");
    return false;
  }
  memset(&ddBltFx, 0, sizeof(ddBltFx));
  ddBltFx.dwFillColor = color;
  ddBltFx.dwSize = sizeof(ddBltFx);
  // Screw the rectangle coords, we will clear whole screen!
  locRet = lpDDSurf->Blt(NULL, 0, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddBltFx);
  if (locRet == DDERR_SURFACELOST)
  {
    WARNLOG("DDraw surface lost - restoring.");
    restore_surfaces();
    return false;
  }
  if (locRet != DD_OK)
  {
    ERRORLOG("Clear screen failed.");
    return false;
  }
  return true;
}

bool TDDrawSdk::swap_screen(void)
{
  HRESULT locRet;
  static RECT srcRect;
  static RECT destRect;
  SYNCDBG(12,"Starting");
  if ((flags & DMF_SurfacesSetup) == 0)
      return false;
  if (this->need_restore_palettes)
      restore_palettes();
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
  }
  return true;
}

bool TDDrawSdk::reset_screen(void)
{
  if ((flags & DMF_DoneSetup) == 0)
  {
    return false;
  }
  release_palettes();
  release_surfaces();
  SendDDMsg(WM_USER+101, 0);
  if (ResultDDMsg() != DD_OK)
  {
    return false;
  }
  return true;
}

bool TDDrawSdk::restore_surfaces(void)
{
  if ((flags & DMF_SurfacesSetup) == 0)
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
  return (ddResult == DD_OK);
}

bool TDDrawSdk::restore_palettes(void)
{
  bool result;
  if ((flags & DMF_SurfacesSetup) == 0)
    return false;
  result = false;
  if (lpDDSurface3 != NULL)
  {
      if (lpDDPalette != NULL)
      {
          lpDDSurface3->SetPalette(lpDDPalette);
          result = true;
      }
  }
  if (lpDDSurface1 != NULL)
  {
      if (lpDDPalette != NULL)
      {
          lpDDSurface1->SetPalette(lpDDPalette);
          result = true;
      }
  }
  this->need_restore_palettes = false;
  return result;
}

void TDDrawSdk::wait_vbi(void)
{
  BOOL bIsInVB;
  if (!this->active)
  {
    return;
  }
  bIsInVB = false;
  while (!bIsInVB)
  {
    lpDDInterface->GetVerticalBlankStatus(&bIsInVB);
  }
}

bool TDDrawSdk::swap_box(struct tagPOINT coord,struct tagRECT &rect)
{
  LPDIRECTDRAWSURFACE lpDDSurf;
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
  }
  return true;
}

bool TDDrawSdk::create_surface(struct SSurface *surf,unsigned long w,unsigned long h)
{
  DDSURFACEDESC ddSurfDesc;
  DDCOLORKEY ddColorKey;
  surf->field_C = w;
  surf->locks_count = 0;
  surf->field_14 = 0;
  surf->field_10 = h;
  memset(&ddSurfDesc, 0, sizeof(ddSurfDesc));
  ddSurfDesc.dwHeight = h;
  ddSurfDesc.dwWidth = w;
  ddSurfDesc.dwSize = sizeof(ddSurfDesc);
  ddSurfDesc.dwFlags = 0x07;
  ddSurfDesc.ddsCaps.dwCaps = 0x0840;
  ddResult = lpDDInterface->CreateSurface(&ddSurfDesc, &surf->lpDDSurf, NULL);
  if (ddResult != DD_OK)
  {
    surf->lpDDSurf = NULL;
    return false;
  }
  ddColorKey.dwColorSpaceLowValue = 255;
  ddColorKey.dwColorSpaceHighValue = 255;
  surf->lpDDSurf->SetColorKey(8, &ddColorKey);
  return true;
}

bool TDDrawSdk::release_surface(struct SSurface *surf)
{
  if (surf->lpDDSurf == NULL)
  {
    return false;
  }
  surf->lpDDSurf->Release();
  surf->lpDDSurf = NULL;
  return true;
}

bool TDDrawSdk::blt_surface(struct SSurface *surf,unsigned long x,unsigned long y,tagRECT *rect,unsigned long blflags)
{
  LPDIRECTDRAWSURFACE secSurf;
  RECT reverseRect;
  DWORD dwTrans;
  secSurf = wscreen_surface();
  if (secSurf == NULL)
  {
    //ERRORLOG("no valid wscreen.");
    return false;
  }
  if ((blflags & 0x02) != 0)
    secSurf = lpDDSurface3;
  dwTrans = 0;
  if ((blflags & 0x04) != 0)
      dwTrans |= DDBLTFAST_SRCCOLORKEY;
  if ((blflags & 0x10) != 0)
      dwTrans |= DDBLTFAST_WAIT;
  if ((blflags & 0x08) != 0)
  {
    secSurf->BltFast(x, y, surf->lpDDSurf, rect, dwTrans);
  } else
  {
    reverseRect.left = x;
    reverseRect.top = y;
    reverseRect.right = x + rect->right - rect->left;
    reverseRect.bottom = y + rect->bottom - rect->top;
    surf->lpDDSurf->BltFast(rect->left, rect->top, secSurf, &reverseRect, dwTrans);
  }
  return true;
}

void *TDDrawSdk::lock_surface(struct SSurface *surf)
{
  DDSURFACEDESC ddSurfDesc;
  HRESULT locRes;
  if (surf->lpDDSurf == NULL)
  {
    return NULL;
  }
  memset(&ddSurfDesc, 0, sizeof(ddSurfDesc));
  ddSurfDesc.dwSize = sizeof(ddSurfDesc);
  locRes = surf->lpDDSurf->Lock(NULL, &ddSurfDesc, DDLOCK_WAIT, NULL);
  if (locRes != DD_OK)
  {
    return NULL;
  }
  surf->locks_count++;
  surf->field_14 = ddSurfDesc.lPitch;
  return ddSurfDesc.lpSurface;
}

bool TDDrawSdk::unlock_surface(struct SSurface *surf)
{
  if (surf->locks_count == 0)
  {
    return true;
  }
  if (surf->lpDDSurf == NULL)
  {
    return false;
  }
  surf->lpDDSurf->Unlock(NULL);
  surf->locks_count = 0;
  return true;
}

void TDDrawSdk::LoresEmulation(bool nstate)
{
  static bool lre_set = false;
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
  }
}

/**
 * Sends a DD message call to Direct Draw thread.
 */
void TDDrawSdk::SendDDMsg(int message, void *param)
{
  Sleep(1);
  lbDDRval = DD_OK;
  lbWait = 1;
  PostMessage(hWindow, message, (WPARAM)param, (LPARAM)this);
}

/**
 * Waits until DD call finishes and returns its result.
 */
HRESULT TDDrawSdk::ResultDDMsg(void)
{
  while (lbWait)
    Sleep(1);
  return lbDDRval;
}

HRESULT CALLBACK TDDrawSdk::screen_mode_callback(LPDDSURFACEDESC lpDDSurf, LPVOID lpContext)
{
  TbScreenModeInfo *mdinfo;
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
  }
  return DDENUMRET_OK;
}

bool TDDrawSdk::setup_direct_draw(void)
{
  DDCAPS ddEmulCaps;
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
  flags |= DMF_DoneSetup;
  return true;
}

bool TDDrawSdk::reset_direct_draw(void)
{
  if ((flags & DMF_DoneSetup) == 0)
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
  flags &= ~DMF_DoneSetup;
  return true;
}

bool TDDrawSdk::setup_dds_double_video(void)
{
  union {
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
      this->need_restore_palettes = false;
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
  }
  return true;
}

bool TDDrawSdk::setup_dds_single_video(void)
{
  union {
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
      this->need_restore_palettes = false;
      ddResult = lpDDSurface3->SetPalette(lpDDPalette);
  }
  memset(&ddBltFx, 0, sizeof(ddBltFx));
  ddBltFx.dwFillColor = 0;
  ddBltFx.dwSize = sizeof(ddBltFx);
  lpDDSurface3->Blt(NULL, 0, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddBltFx);
  return (ddResult < 1);
}

bool TDDrawSdk::setup_dds_system(void)
{
  DDSURFACEDESC ddSurfDesc;
  memset(&ddSurfDesc, 0, sizeof(ddSurfDesc));
  ddSurfDesc.dwSize = sizeof(ddSurfDesc);
  ddSurfDesc.dwHeight = this->resHeight;
  ddSurfDesc.dwWidth = this->resWidth;
  ddSurfDesc.dwFlags = 7;
  ddSurfDesc.ddsCaps.dwCaps = 2112;
  ddResult = lpDDInterface->CreateSurface(&ddSurfDesc, &lpDDSurface1, NULL);
  return (ddResult == DD_OK);
}

bool TDDrawSdk::setup_surfaces(short w, short h, short bpp)
{
  if ((flags & DMF_DoneSetup) == 0)
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
  flags |= DMF_SurfacesSetup;
  return true;
}

bool TDDrawSdk::release_surfaces(void)
{
  LPDIRECTDRAWSURFACE lpDDSurLocal;
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
  }
  return true;
}

bool TDDrawSdk::release_palettes(void)
{
  LPDIRECTDRAWPALETTE lpDDPalLocal;
  if ((flags & DMF_DoneSetup) == 0)
    return false;
  lpDDPalLocal = lpDDPalette;
  if (lpDDPalette != NULL)
  {
    lpDDPalette = NULL;
    while (lpDDPalLocal->Release() != DD_OK)
    { Sleep(1); }
  }
  return true;
}

LPDIRECTDRAWSURFACE TDDrawSdk::wscreen_surface(void)
{
  if ((flags & DMF_DoneSetup) == 0)
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
  }
}

DWORD CALLBACK TDDrawSdk::sdk_window_thread(LPVOID lpParam)
{
  struct TDDrawSdk *pthis;
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
  //SYNCDBG(0,"Ending Sdk Thread.");
  return 0;
}

bool TDDrawSdk::create_sdk_window(void)
{
  int w,h;
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
  SetFocus(hWindow);
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
  }
}

void TDDrawSdk::SetIcon(void)
{
  HICON hIcon;
  if (hWindow == NULL)
  {
    WARNLOG("Cannot set - no valid window handle.");
    return;
  }
  hIcon = LoadIcon(lbhInstance, resource_mapping(lbIconIndex));
  SendMessage(hWindow, WM_SETICON, ICON_BIG,  (LPARAM)hIcon);
  hIcon = LoadIcon(lbhInstance, resource_mapping(lbIconIndex));
  SendMessage(hWindow, WM_SETICON, ICON_SMALL,(LPARAM)hIcon);
}

bool TDDrawSdk::remove_sdk_window(void)
{
  DWORD ret;
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
  }
  return false;
}

TbScreenModeInfo *TDDrawSdk::get_mode_info(unsigned short mode)
{
  int maxmode=sizeof(lbScreenModeInfo)/sizeof(TbScreenModeInfo);
  if (mode < maxmode)
    return &lbScreenModeInfo[mode];
  return &lbScreenModeInfo[0];
}

TbScreenMode TDDrawSdk::get_mode_info_by_str(char *str)
{
  int maxmode=sizeof(lbScreenModeInfo)/sizeof(TbScreenModeInfo);
  int mode;
  for (mode=0; mode<maxmode; mode++)
  {
    if (stricmp(lbScreenModeInfo[mode].Desc,str) == 0)
      return (TbScreenMode)mode;
  }
  return Lb_SCREEN_MODE_INVALID;
}

/******************************************************************************/
