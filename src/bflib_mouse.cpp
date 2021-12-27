/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mouse.cpp
 *     Mouse related routines.
 * @par Purpose:
 *     Pointer position, movement and cursor support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 26 Oct 2010
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
#include <SDL2/SDL.h>

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_mshandler.hpp"
#include "bflib_inputctrl.h"

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
volatile TbBool lbMouseGrab = true;
volatile TbBool lbMouseGrabbed = true;
volatile TbDisplayStructEx lbDisplayEx;
/******************************************************************************/
TbResult LbMouseChangeSpriteAndHotspot(const struct TbSprite *pointerSprite, long hot_x, long hot_y)
{
#if (BFDEBUG_LEVEL > 18)
  if (pointerSprite == NULL)
    SYNCLOG("Setting to %s","NONE");
  else
    SYNCLOG("Setting to %dx%d, data at %p",(int)pointerSprite->SWidth,(int)pointerSprite->SHeight,pointerSprite);
#endif
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMousePointerAndOffset(pointerSprite, hot_x, hot_y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetup(struct TbSprite *pointerSprite)
{
  TbResult ret;
  if (lbMouseInstalled)
    LbMouseSuspend();
  pointerHandler.Install();
  lbMouseOffline = true;
  lbMouseInstalled = true;
  LbMouseSetWindow(0,0,LbGraphicsScreenWidth(),LbGraphicsScreenHeight());
  LbGrabMouseInit();
  ret = Lb_SUCCESS;
  if (LbMouseChangeSprite(pointerSprite) != Lb_SUCCESS)
    ret = Lb_FAIL;
  lbMouseInstalled = (ret == Lb_SUCCESS);
  lbMouseOffline = false;
  return ret;
}

TbResult LbMouseSetPointerHotspot(long hot_x, long hot_y)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetPointerOffset(hot_x, hot_y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetPositionInitial(long x, long y)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMousePosition(x, y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetPosition(long x, long y)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!lbMouseGrabbed)
  {
    if (IsMouseInsideWindow())
    {
      // in altinput mode
      // first move the game cursor to the position of the HostOS cursor, and then move the HostOS cursor to the given x and y location.
      // this keeps Host OS cursor and game cursor in sync (same position)
      if (!LbMoveGameCursorToHostCursor())
        return Lb_FAIL;
      SDL_WarpMouseInWindow(SDL_GetKeyboardFocus(), x, y);
    }
  }
  else if (!pointerHandler.SetMousePosition(x, y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

void LbMoveHostCursorToGameCursor(void)
{
    int game_cursor_x = lbDisplay.MMouseX;
    int game_cursor_y = lbDisplay.MMouseY;
    int host_cursor_x, host_cursor_y;
    SDL_GetMouseState(&host_cursor_x, &host_cursor_y);
    if ((host_cursor_x != game_cursor_x) || (host_cursor_y != game_cursor_y))
    {
        LbMouseSetPosition(game_cursor_x, game_cursor_y);
    }
}

TbResult LbMoveGameCursorToHostCursor(void)
{
    int game_cursor_x = lbDisplay.MMouseX;
    int game_cursor_y = lbDisplay.MMouseY;
    int host_cursor_x, host_cursor_y;
    SDL_GetMouseState(&host_cursor_x, &host_cursor_y);
    if (((host_cursor_x != game_cursor_x) || (host_cursor_y != game_cursor_y)) && LbIsActive())
    {
        if (!pointerHandler.SetMousePosition(host_cursor_x, host_cursor_y))
        {
            return Lb_FAIL;
        }
    }
    return Lb_SUCCESS;
}

TbBool IsMouseInsideWindow(void)
{
    SDL_Window *window = SDL_GetMouseFocus();
    TbBool isMouseInsideWindow = ((window != NULL) ? true : false); // if window == NULL then the mouse must be outside the kfx window
    if (!LbIsMouseActive() && !lbMouseGrabbed)
    {
        isMouseInsideWindow = false; // LbIsMouseActive() == false when mouse cursor outside window
    }
    return isMouseInsideWindow;
}

TbResult LbMouseChangeSprite(struct TbSprite *pointerSprite)
{
#if (BFDEBUG_LEVEL > 18)
  if (pointerSprite == NULL)
    SYNCLOG("Setting to %s","NONE");
  else
    SYNCLOG("Setting to %dx%d, data at %p",(int)pointerSprite->SWidth,(int)pointerSprite->SHeight,pointerSprite);
#endif
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMousePointer(pointerSprite))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

void GetPointerHotspot(long *hot_x, long *hot_y)
{
  struct TbPoint *hotspot;
  hotspot = pointerHandler.GetPointerOffset();
  if (hotspot == NULL)
    return;
  *hot_x = hotspot->x;
  *hot_y = hotspot->y;
}

TbResult LbMouseIsInstalled(void)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!pointerHandler.IsInstalled())
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetWindow(long x, long y, long width, long height)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseWindow(x, y, width, height))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseOnMove(struct TbPoint shift)
{
  if ((!lbMouseInstalled) || (lbMouseOffline))
    return Lb_FAIL;
  if (!pointerHandler.SetMousePosition(lbDisplay.MMouseX+shift.x, lbDisplay.MMouseY+shift.y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSuspend(void)
{
  if (!lbMouseInstalled)
    return Lb_FAIL;
  if (!pointerHandler.Release())
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseOnBeginSwap(void)
{
    if (!pointerHandler.PointerBeginSwap())
        return Lb_FAIL;
    return Lb_SUCCESS;
}

TbResult LbMouseOnEndSwap(void)
{
    if (!pointerHandler.PointerEndSwap())
        return Lb_FAIL;
    return Lb_SUCCESS;
}

void mouseControl(unsigned int action, struct TbPoint *pos)
{
    struct TbPoint dstPos;
    dstPos.x = pos->x;
    dstPos.y = pos->y;
    switch ( action )
    {
    case MActn_MOUSEMOVE:
        LbMouseOnMove(dstPos);
        break;
    case MActn_LBUTTONDOWN:
        lbDisplay.MLeftButton = 1;
        if ( !lbDisplay.LeftButton )
        {
            LbMouseOnMove(dstPos);
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.RLeftButton = 0;
            lbDisplay.LeftButton = 1;
        }
        break;
    case MActn_LBUTTONUP:
        lbDisplay.MLeftButton = 0;
        if ( !lbDisplay.RLeftButton )
        {
            LbMouseOnMove(dstPos);
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RLeftButton = 1;
        }
        break;
    case MActn_RBUTTONDOWN:
        lbDisplay.MRightButton = 1;
        if ( !lbDisplay.RightButton )
        {
            LbMouseOnMove(dstPos);
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.RRightButton = 0;
            lbDisplay.RightButton = 1;
        }
        break;
    case MActn_RBUTTONUP:
        lbDisplay.MRightButton = 0;
        if ( !lbDisplay.RRightButton )
        {
            LbMouseOnMove(dstPos);
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RRightButton = 1;
        }
        break;
    case MActn_MBUTTONDOWN:
        lbDisplay.MMiddleButton = 1;
        if ( !lbDisplay.MiddleButton )
        {
            LbMouseOnMove(dstPos);
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.MiddleButton = 1;
            lbDisplay.RMiddleButton = 0;
        }
        break;
    case MActn_MBUTTONUP:
        lbDisplay.MMiddleButton = 0;
        if ( !lbDisplay.RMiddleButton )
        {
            LbMouseOnMove(dstPos);
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RMiddleButton = 1;
            lbDisplay.MiddleButton = 0; // lbDisplay.MiddleButton is not handled as well as lbDisplay.LeftButton and lbDisplay.RightButton, so reset it here
        }
        break;
    case MActn_WHEELMOVEUP:
        lbDisplayEx.WhellPosition--;
        lbDisplayEx.WhellMoveUp++;
        lbDisplayEx.WhellMoveDown = 0;
        break;
    case MActn_WHEELMOVEDOWN:
        lbDisplayEx.WhellPosition++;
        lbDisplayEx.WhellMoveUp = 0;
        lbDisplayEx.WhellMoveDown++;
        break;
    default:
        break;
    }
}

/**
 * Changes mouse movement ratio.
 * Note that this function can be run even before mouse setup. Still, the factor
 *  will be reset during the installation - so use it after LbMouseSetup().
 *
 * @param ratio_x Movement ratio in X direction; 256 means unchanged ratio from OS.
 * @param ratio_y Movement ratio in Y direction; 256 means unchanged ratio from OS.
 * @return Lb_SUCCESS if the ratio values were of correct range and have been set.
 */
TbResult LbMouseChangeMoveRatio(long ratio_x, long ratio_y)
{
    if ((ratio_x < -8192) || (ratio_x > 8192) || (ratio_x == 0))
        return Lb_FAIL;
    if ((ratio_y < -8192) || (ratio_y > 8192) || (ratio_y == 0))
        return Lb_FAIL;
    SYNCLOG("New ratio %ldx%ld",ratio_x, ratio_y);
    // Currently we don't have two ratio factors, so let's store an average
    lbDisplay.MouseMoveRatio = (ratio_x + ratio_y)/2;
    //TODO INPUT Separate mouse ratios in X and Y direction when lbDisplay from DLL will no longer be used.
    //minfo.XMoveRatio = ratio_x;
    //minfo.YMoveRatio = ratio_y;
    return Lb_SUCCESS;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
