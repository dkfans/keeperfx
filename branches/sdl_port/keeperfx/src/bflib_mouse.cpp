/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mouse.c
 *     Mouse related routines.
 * @par Purpose:
 *     Mouse and mouse cursor support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 18 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_mouse.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
//#include <SDL/SDL.h>
#include <windows.h>

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_mshandler.hpp"

//#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/*

struct mouse_buffer mbuffer;
struct mouse_info minfo;
char ptr[4096];
unsigned int redraw_active=0;
unsigned int mouse_initialised=0;
short volatile mouse_mickey_x;
short volatile mouse_mickey_y;
long volatile mouse_dx;
long volatile mouse_dy;
unsigned long mouse_pos_change_saved;
struct DevInput joy;
*/
/******************************************************************************/
DLLIMPORT int _DK_LbMouseChangeSpriteAndHotspot(struct TbSprite *spr, int a, int b);
DLLIMPORT int _DK_LbMouseSetup(struct TbSprite *MouseSprite);
DLLIMPORT int _DK_LbMouseSetPointerHotspot(int x, int y);
DLLIMPORT int _DK_LbMouseSetPosition(int x, int y);
DLLIMPORT int _DK_LbMouseChangeSprite(struct TbSprite *MouseSprite);
DLLIMPORT int _DK_LbMouseSuspend(void);
DLLIMPORT int _DK_LbMouseIsInstalled(void);
DLLIMPORT int _DK_LbMouseSetWindow(int x, int y, int width, int height);
DLLIMPORT long _DK_LbMouseOnMove(struct tagPOINT pos);
DLLIMPORT long _DK_LbMouseOnBeginSwap(void);
DLLIMPORT void _DK_LbMouseOnEndSwap(void);
/******************************************************************************/
TbResult __stdcall LbMouseChangeSpriteAndHotspot(struct TbSprite *mouseSprite, long hot_x, long hot_y)
{
#if (BFDEBUG_LEVEL > 18)
  if (mouseSprite == NULL)
    SYNCLOG("Setting to %s","NONE");
  else
    SYNCLOG("Setting to %dx%d, data at %p",(int)mouseSprite->SWidth,(int)mouseSprite->SHeight,mouseSprite);
#endif
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!winMouseHandler.SetMousePointerAndOffset(mouseSprite, hot_x, hot_y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult __stdcall LbMouseSetup(struct TbSprite *mouseSprite)
{
  TbResult ret;
  long x,y;
  if (lbMouseInstalled)
    LbMouseSuspend();
  y = (lbDisplay.MouseWindowHeight + lbDisplay.MouseWindowY) / 2;
  x = (lbDisplay.MouseWindowWidth + lbDisplay.MouseWindowX) / 2;
  winMouseHandler.Install();
  lbMouseOffline = true;
  lbMouseInstalled = true;
  LbMouseSetWindow(0,0,lbDisplay.PhysicalScreenWidth,lbDisplay.PhysicalScreenHeight);
  ret = Lb_SUCCESS;
  if (LbMouseSetPosition(x,y) != Lb_SUCCESS)
    ret = Lb_FAIL;
  if (LbMouseChangeSprite(mouseSprite) != Lb_SUCCESS)
    ret = Lb_FAIL;
  lbMouseInstalled = (ret == Lb_SUCCESS);
  lbMouseOffline = false;
  return ret;
}

TbResult __stdcall LbMouseSetPointerHotspot(long hot_x, long hot_y)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!winMouseHandler.SetPointerOffset(hot_x, hot_y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult __stdcall LbMouseSetPosition(long x, long y)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!winMouseHandler.SetMousePosition(x, y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult __stdcall LbMouseChangeSprite(struct TbSprite *mouseSprite)
{
#if (BFDEBUG_LEVEL > 18)
  if (mouseSprite == NULL)
    SYNCLOG("Setting to %s","NONE");
  else
    SYNCLOG("Setting to %dx%d, data at %p",(int)mouseSprite->SWidth,(int)mouseSprite->SHeight,mouseSprite);
#endif
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!winMouseHandler.SetMousePointer(mouseSprite))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

void GetPointerHotspot(long *hot_x, long *hot_y)
{
  struct tagPOINT *hotspot;
  hotspot = winMouseHandler.GetPointerOffset();
  if (hotspot == NULL)
    return;
  *hot_x = hotspot->x;
  *hot_y = hotspot->y;
}

TbResult __stdcall LbMouseIsInstalled(void)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!winMouseHandler.IsInstalled())
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult __stdcall LbMouseSetWindow(long x, long y, long width, long height)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!winMouseHandler.SetMouseWindow(x, y, width, height))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult __stdcall LbMouseOnMove(struct tagPOINT shift)
{
  if ((!lbMouseInstalled) || (lbMouseOffline))
    return Lb_FAIL;
  if (!winMouseHandler.SetMousePosition(lbDisplay.MMouseX+shift.x, lbDisplay.MMouseY+shift.y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

/**
 * Converts mouse coordinates into relative shift coordinates.
 */
void MouseToScreen(struct tagPOINT *pos)
{
  static long mx = 0;
  static long my = 0;
  struct tagRECT clip;
  struct tagPOINT orig;
  if ( lbUseSdk )
  {
    if ( GetClipCursor(&clip) )
    {
      orig.x = pos->x;
      orig.y = pos->y;
      pos->x -= mx;
      pos->y -= my;
      mx = orig.x;
      my = orig.y;
      if ((mx < clip.left + 50) || (mx > clip.right - 50)
       || (my < clip.top + 50) || (my > clip.bottom - 50))
      {
        mx = (clip.right-clip.left)/2 + clip.left;
        my = (clip.bottom-clip.top)/2 + clip.top;
        SetCursorPos(mx, my);
      }
    }
  } else
  {
    pos->x -= lbDisplay.MMouseX;
    pos->y -= lbDisplay.MMouseY;
  }
}

TbResult __stdcall LbMouseSuspend(void)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!winMouseHandler.Release())
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult __stdcall LbMouseOnBeginSwap(void)
{
  if (!winMouseHandler.PointerBeginSwap())
    return Lb_FAIL;
  return Lb_SUCCESS;
}

void __stdcall LbMouseOnEndSwap(void)
{
  winMouseHandler.PointerEndSwap();
}

void __stdcall mouseControl(unsigned int action, struct tagPOINT *pos)
{
  //MouseToScreen calls were commented out because they are unnecessary; SDL can perform the
  //necessary calculations of relative mouse coordinate much better.

  struct tagPOINT dstPos;
  dstPos.x = pos->x;
  dstPos.y = pos->y;
  switch ( action )
  {
    case 512:
      //MouseToScreen(&dstPos);
      LbMouseOnMove(dstPos);
      break;
    case 513:
    case 515:
      lbDisplay.MLeftButton = 1;
      if ( !lbDisplay.LeftButton )
      {
        //MouseToScreen(&dstPos);
        LbMouseOnMove(dstPos);
        lbDisplay.MouseX = lbDisplay.MMouseX;
        lbDisplay.MouseY = lbDisplay.MMouseY;
        lbDisplay.RLeftButton = 0;
        lbDisplay.LeftButton = 1;
      }
      break;
    case 514:
      lbDisplay.MLeftButton = 0;
      if ( !lbDisplay.RLeftButton )
      {
        //MouseToScreen(&dstPos);
        LbMouseOnMove(dstPos);
        lbDisplay.RMouseX = lbDisplay.MMouseX;
        lbDisplay.RMouseY = lbDisplay.MMouseY;
        lbDisplay.RLeftButton = 1;
      }
      break;
    case 516:
    case 518:
      lbDisplay.MRightButton = 1;
      if ( !lbDisplay.RightButton )
      {
        //MouseToScreen(&dstPos);
        LbMouseOnMove(dstPos);
        lbDisplay.MouseX = lbDisplay.MMouseX;
        lbDisplay.MouseY = lbDisplay.MMouseY;
        lbDisplay.RRightButton = 0;
        lbDisplay.RightButton = 1;
      }
      break;
    case 517:
      lbDisplay.MRightButton = 0;
      if ( !lbDisplay.RRightButton )
      {
        //MouseToScreen(&dstPos);
        LbMouseOnMove(dstPos);
        lbDisplay.RMouseX = lbDisplay.MMouseX;
        lbDisplay.RMouseY = lbDisplay.MMouseY;
        lbDisplay.RRightButton = 1;
      }
      break;
    case 519:
    case 521:
      lbDisplay.MMiddleButton = 1;
      if ( !lbDisplay.MiddleButton )
      {
        //MouseToScreen(&dstPos);
        LbMouseOnMove(dstPos);
        lbDisplay.MouseX = lbDisplay.MMouseX;
        lbDisplay.MouseY = lbDisplay.MMouseY;
        lbDisplay.MiddleButton = 1;
        lbDisplay.RMiddleButton = 0;
      }
      break;
    case 520:
      lbDisplay.MMiddleButton = 0;
      if ( !lbDisplay.RMiddleButton )
      {
        //MouseToScreen(&dstPos);
        LbMouseOnMove(dstPos);
        lbDisplay.RMouseX = lbDisplay.MMouseX;
        lbDisplay.RMouseY = lbDisplay.MMouseY;
        lbDisplay.RMiddleButton = 1;
      }
      break;
    default:
      break;
  }
}

/*
int __fastcall LbMouseChangeMoveRatio(int x, int y)
{
  if ( !lbMouseInstalled )
    return -1;
  if ( (x<1) || (x>63) )
    return -1;
  if ( (y<1) || (y>63) )
    return -1;
  minfo.XMoveRatio = x;
  minfo.YMoveRatio = y;
  return 1;
}

void __fastcall LbProcessMouseClick(SDL_MouseButtonEvent *button)
{
  switch (button->state)
  {
  case SDL_PRESSED:
    switch (button->button)
    {
    case SDL_BUTTON_LEFT:
        lbDisplay.MLeftButton=1;
        if (lbDisplay.LeftButton==0)
        {
          lbDisplay.LeftButton=1;
          lbDisplay.MouseX=lbDisplay.MMouseX;
          lbDisplay.MouseY=lbDisplay.MMouseY;
          lbDisplay.RLeftButton=0;
        }
       break;
    case SDL_BUTTON_MIDDLE:
        lbDisplay.MMiddleButton=1;
        if (lbDisplay.MiddleButton==0)
        {
          lbDisplay.MiddleButton=1;
          lbDisplay.MouseX=lbDisplay.MMouseX;
          lbDisplay.MouseY=lbDisplay.MMouseY;
          lbDisplay.RMiddleButton=0;
        }
       break;
    case SDL_BUTTON_RIGHT:
        lbDisplay.MRightButton=1;
        if (lbDisplay.RightButton==0)
        {
          lbDisplay.RightButton=1;
          lbDisplay.MouseX=lbDisplay.MMouseX;
          lbDisplay.MouseY=lbDisplay.MMouseY;
          lbDisplay.RRightButton=0;
        }
       break;
    };break;
  case SDL_RELEASED:
    switch (button->button)
    {
    case SDL_BUTTON_LEFT:
        lbDisplay.MLeftButton=0;
        if (lbDisplay.RLeftButton==0)
        {
          lbDisplay.RLeftButton=1;
          lbDisplay.RMouseX=lbDisplay.MMouseX;
          lbDisplay.RMouseY=lbDisplay.MMouseY;
        }
       break;
    case SDL_BUTTON_MIDDLE:
        lbDisplay.MMiddleButton=0;
        if (lbDisplay.RMiddleButton==0)
        {
          lbDisplay.RMiddleButton=1;
          lbDisplay.RMouseX=lbDisplay.MMouseX;
          lbDisplay.RMouseY=lbDisplay.MMouseY;
        }
       break;
    case SDL_BUTTON_RIGHT:
        lbDisplay.MRightButton=0;
        if (lbDisplay.RRightButton==0)
        {
          lbDisplay.RRightButton=1;
          lbDisplay.RMouseX=lbDisplay.MMouseX;
          lbDisplay.RMouseY=lbDisplay.MMouseY;
        }
       break;
    };break;
  }
  //MouseHandlerMove(button->x,button->y);
}


//Adjusts point coordinates; returns true if the coordinates have changed.
bool __fastcall adjust_point(long *x, long *y)
{
  bool result = false;
  if ( *x >= lbDisplay.MouseWindowX )
  {
    if ( lbDisplay.MouseWindowX + lbDisplay.MouseWindowWidth <= *x )
    {
      *x = lbDisplay.MouseWindowX + lbDisplay.MouseWindowWidth - 1;
      result = true;
    }
  } else
  {
    *x = lbDisplay.MouseWindowX;
    result = true;
  }
  if ( *y >= lbDisplay.MouseWindowY )
  {
    if ( lbDisplay.MouseWindowY + lbDisplay.MouseWindowHeight <= *y )
    {
      *y = lbDisplay.MouseWindowY + lbDisplay.MouseWindowHeight - 1;
      result = true;
    }
  } else
  {
    *y = lbDisplay.MouseWindowY;
    result = true;
  }
  return result;
}

//Returns if the current mouse position is inside of given rectangle
char __fastcall mouse_in_rect(short x1, short x2, short y1, short y2)
{
  return (x1<=lbDisplay.MMouseX) && (x2>lbDisplay.MMouseX) &&
         (y1<=lbDisplay.MMouseY) && (y2>lbDisplay.MMouseY);
}
*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
