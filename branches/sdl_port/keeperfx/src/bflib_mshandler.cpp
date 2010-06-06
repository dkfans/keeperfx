/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mshandler.cpp
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
#include "bflib_mshandler.hpp"

#include <SDL.h>
#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"

#include "keeperfx.hpp"
/******************************************************************************/
// Global variables
int volatile lbMouseInstalled = false;
int volatile lbMouseOffline = false;
class MouseStateHandler winMouseHandler;

/******************************************************************************/
/**
 * Adjusts point coordinates.
  @param x,y Coorditates to adjust.
 * @return Returns true if the coordinates have changed.
 */
int adjust_point(long *x, long *y)
{
  return false;
}

MouseStateHandler::MouseStateHandler(void)
{
  Release();
}

MouseStateHandler::~MouseStateHandler(void)
{
}

bool MouseStateHandler::Install(void)
{
  this->installed = true;
  return true;
}

bool MouseStateHandler::IsInstalled(void)
{
  return true;
}

bool MouseStateHandler::Release(void)
{
  lbMouseInstalled = false;
  lbDisplay.MouseSprite = NULL;
  this->installed = false;
  mssprite = NULL;
  pointer.Release();
  mspos.x = 0;
  mspos.y = 0;
  hotspot.x = 0;
  hotspot.y = 0;
  return true;
}

struct tagPOINT *MouseStateHandler::GetPosition(void)
{
  return &mspos;
}

bool MouseStateHandler::SetMousePosition(long x, long y)
{
	SDL_WarpMouse(x, y);
	//rely on SDL_MOUSEMOVE to inform about moved mouse
	return true;
}

void MouseStateHandler::updatePosition(int x, int y)
{
	lbDisplay.MMouseX = x;
	lbDisplay.MMouseY = y;
	mspos.x = x;
	mspos.y = y;
	pointer.NewMousePos(x, y);
}

bool MouseStateHandler::SetPosition(long x, long y)
{
	//TODO: perhaps clip again and do SDL_WarpMouse(), see if necessary
	return SetMousePosition(x, y);
}

bool MouseStateHandler::SetMouseWindow(long x, long y,long width, long height)
{
  lbDisplay.MouseWindowX = x;
  lbDisplay.MouseWindowY = y;
  lbDisplay.MouseWindowWidth = width;
  lbDisplay.MouseWindowHeight = height;
  adjust_point(&lbDisplay.MMouseX, &lbDisplay.MMouseY);
  adjust_point(&lbDisplay.MouseX, &lbDisplay.MouseY);
  return true;
}

struct TbSprite *MouseStateHandler::GetPointer(void)
{
  return mssprite;
}

bool MouseStateHandler::SetPointer(struct TbSprite *spr, struct tagPOINT *point)
{
  mssprite = spr;
  hotspot.y = 0;
  hotspot.x = 0;
  if ((spr != NULL) && (spr->SWidth != 0) && (spr->SHeight != 0))
  {
    if (point != NULL)
    {
      hotspot.x = point->x;
      hotspot.y = point->y;
    }
    pointer.Initialise(spr, &mspos, &hotspot);
    if ((mssprite != NULL))
    {
      pointer.OnMove(mspos.x, mspos.y);
    }
  } else
  {
    pointer.Release();
    mssprite = NULL;
  }
  return true;
}

bool MouseStateHandler::SetMousePointerAndOffset(struct TbSprite *mouseSprite, long x, long y)
{
  struct tagPOINT point;
  if (mouseSprite == lbDisplay.MouseSprite)
    return true;
  if (mouseSprite != NULL)
    if ( (mouseSprite->SWidth > 64) || (mouseSprite->SHeight > 64) )
    {
      WARNLOG("Mouse pointer too large");
      return false;
    }
  lbDisplay.MouseSprite = mouseSprite;
  point.x = x;
  point.y = y;
  this->SetPointer(mouseSprite, &point);
  return true;
}

bool MouseStateHandler::SetMousePointer(struct TbSprite *mouseSprite)
{
  if (mouseSprite == lbDisplay.MouseSprite)
    return true;
  if (mouseSprite != NULL)
    if ( (mouseSprite->SWidth > 64) || (mouseSprite->SHeight > 64) )
    {
      WARNLOG("Mouse pointer too large");
      return false;
    }
  lbDisplay.MouseSprite = mouseSprite;
  this->SetPointer(mouseSprite, NULL);
  return true;
}

bool MouseStateHandler::SetPointerOffset(long x, long y)
{
	pointer.SetHotspot(x, y);
	return true;
}

struct tagPOINT *MouseStateHandler::GetPointerOffset(void)
{
  return &hotspot;
}

bool MouseStateHandler::PointerBeginSwap(void)
{
  if ((lbMouseOffline))
    return true;
  if ((mssprite != NULL))
  {
    swap = 1;
    pointer.OnBeginSwap();
  }
  return true;
}

bool MouseStateHandler::PointerEndSwap(void)
{
  if ((mssprite != NULL))
  {
    if (swap)
    {
      swap = false;
      pointer.OnEndSwap();
    }
  }
  return true;
}
/******************************************************************************/
