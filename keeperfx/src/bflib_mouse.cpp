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

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "bflib_datetm.h"
#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_mshandler.hpp"
#include "bflib_keybrd.h"

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

// Whether we want to relative mouse mode, when this is on, mouse will be trapped in game window.
volatile TbBool lbUseRelativeMouseMode = false;

// Whether we want to enable mouse dragging without ctrl pressed.
// TODO make this into option page.
volatile TbBool lbUseDirectMouseDragging = false;

volatile TbDisplayStructEx lbDisplayEx;
/******************************************************************************/
TbResult LbMouseChangeSpriteAndHotspot(struct TbSprite *pointerSprite, long hot_x, long hot_y)
{
#if (BFDEBUG_LEVEL > 18)
  if (pointerSprite == NULL)
    SYNCLOG("Setting to %s","NONE");
  else
    SYNCLOG("Setting to %dx%d, data at %p",(int)pointerSprite->SWidth,(int)pointerSprite->SHeight,pointerSprite);
#endif
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseSpriteAndOffset(pointerSprite, hot_x, hot_y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetup(struct TbSprite *pointerSprite)
{
  TbResult ret;
  long x,y;
  if (lbMouseSpriteInstalled)
  {
      LbMouseSuspend();
  }
  y = (lbDisplay.MouseWindowHeight + lbDisplay.MouseWindowY) / 2;
  x = (lbDisplay.MouseWindowWidth + lbDisplay.MouseWindowX) / 2;
  pointerHandler.Install();
  lbMouseOffline = true;
  lbMouseSpriteInstalled = true;
  LbMouseSetWindow(0, 0, LbScreenWidth(), LbScreenHeight());

  ret = Lb_SUCCESS;
  
  if ((LbMouseSetPosition(x, y) != Lb_SUCCESS) ||
      (LbMouseChangeSprite(pointerSprite) != Lb_SUCCESS))
  {
      ret = Lb_FAIL;
  }

  lbMouseSpriteInstalled = (ret == Lb_SUCCESS);
  lbMouseOffline = false;
  return ret;
}

TbResult LbMouseSetPointerHotspot(long hot_x, long hot_y)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseSpriteOffset(hot_x, hot_y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetPosition(long x, long y)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMousePosition(x, y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseChangeSprite(struct TbSprite *pointerSprite)
{
#if (BFDEBUG_LEVEL > 18)
  if (pointerSprite == NULL)
    SYNCLOG("Setting to %s","NONE");
  else
    SYNCLOG("Setting to %dx%d, data at %p",(int)pointerSprite->SWidth,(int)pointerSprite->SHeight,pointerSprite);
#endif
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseSprite(pointerSprite))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

void GetPointerHotspot(long *hot_x, long *hot_y)
{
  struct TbPoint *hotspot;
  hotspot = pointerHandler.GetMouseSpriteOffset();
  if (hotspot == NULL)
    return;
  *hot_x = hotspot->x;
  *hot_y = hotspot->y;
}

TbResult LbMouseIsInstalled(void)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.IsInstalled())
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetWindow(long x, long y, long width, long height)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseWindow(x, y, width, height))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseOnMove(struct TbPoint dstPos)
{
    if ((!lbMouseSpriteInstalled) || (lbMouseOffline))
    {
        return Lb_FAIL;
    }

    if (!pointerHandler.SetMousePosition(dstPos.x, dstPos.y))
    {
        return Lb_FAIL;
    }

   // SDL_WarpMouseInWindow(lbScreenWindow, lbDisplay.MMouseX, lbDisplay.MMouseY);
    return Lb_SUCCESS;
}

TbResult LbMouseSuspend(void)
{
  if (!lbMouseSpriteInstalled)
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

// Returns X position of mouse cursor on screen.
long GetMouseX(void)
{
    long result;
    result = lbDisplay.MMouseX * (long)pixel_size;
    return result;
}

// Returns Y position of mouse cursor on screen.
long GetMouseY(void)
{
    long result;
    result = lbDisplay.MMouseY * (long)pixel_size;
    return result;
}

/** Converts mouse coordinates into relative and scaled coordinates.
*
* @param pos Pointer to the structure with source point, and where result is placed.
*/
TbPoint _scaleMouseMove(struct TbPoint posDelta)
{
    TbPoint scaledMouseMove;

    if (!lbUseRelativeMouseMode)
    {
        ERRORLOG("Mouse ratio is only valid in captured mouse mode.");
        return scaledMouseMove;
    }

    // Scale coordinate
    scaledMouseMove.x = (long)((float)(posDelta.x)* (float)lbDisplay.MouseMoveRatio / DEFAULT_MOUSE_MOVE_RATIO);
    scaledMouseMove.y = (long)((float)(posDelta.y)* (float)lbDisplay.MouseMoveRatio / DEFAULT_MOUSE_MOVE_RATIO);

    return scaledMouseMove;
}

// Get information of mouse move destination point and mouse move delta.
void _get_mouse_state(TbPoint *positionDelta, TbPoint *destination)
{
    // TODO HeM error handling.
    int x, y;
    struct TbPoint mouseDelta;
    struct TbPoint scaledMouseDelta;

    if (lbUseRelativeMouseMode)
    {
        SDL_GetRelativeMouseState(&x, &y);

        mouseDelta.x = x;
        mouseDelta.y = y;

        // Scale the Delta to Sprite position.
        scaledMouseDelta = _scaleMouseMove(mouseDelta);

        positionDelta->x = scaledMouseDelta.x;
        positionDelta->y = scaledMouseDelta.y;

        destination->x = lbDisplay.MMouseX + positionDelta->x;
        destination->y = lbDisplay.MMouseY + positionDelta->y;
    }
    else
    {
        SDL_GetMouseState(&x, &y);

        positionDelta->x = x - lbDisplay.MMouseX;
        positionDelta->y = y - lbDisplay.MMouseY;

        destination->x = x;
        destination->y = y;
    }
}

#pragma region RotateHelpers
double _get_cosin(TbPoint vectorA, TbPoint vectorB)
{
    double x1 = vectorA.x, 
           y1 = vectorA.y, 
           x2 = vectorB.x, 
           y2 = vectorB.y;

    double productValue = x1*x2 + y1*y2;
    double valvA = sqrt(x1*x1 + y1*y1);
    double valvB = sqrt(x2*x2 + y2*y2);
    double result = productValue / (valvA*valvB);
    return result;
}

// Get the degree from current vector to x axis
double _get_vector_degree(TbPoint vectorA)
{
    const float pi = 3.14159;

    TbPoint vectorXAxis;

    vectorXAxis.x = 1;
    vectorXAxis.y = 0;

    double cos = _get_cosin(vectorXAxis, vectorA);
    double angle = acos(cos) / pi * 180;
    if (vectorA.y > 0)
    {
        angle = 360 - angle;
    }
    return angle;
}

TbPoint _locate_rotate_center(void)
{
    TbPoint rotationCenter;

    rotationCenter.x = lbDisplayEx.mainPanelWidth + (lbDisplay.PhysicalScreenWidth - lbDisplayEx.mainPanelWidth) / 2;
    rotationCenter.y = lbDisplay.PhysicalScreenHeight / 2;

    return rotationCenter;
}

// Get the rotate angle to rotate camera to destination point, in degree.
double _calculate_rotate_angle(TbPoint dstPos)
{
    TbPoint fromVector;
    TbPoint toVector;
    TbPoint rotationCenter = _locate_rotate_center();

    fromVector.x = lbDisplay.MMouseX - rotationCenter.x;
    fromVector.y = (lbDisplay.MMouseY - rotationCenter.y)*VISUALSIZE_RATIO_H_TO_V;

    toVector.x = dstPos.x - rotationCenter.x;
    toVector.y = (dstPos.y - rotationCenter.y)*VISUALSIZE_RATIO_H_TO_V;

    double angleFrom = _get_vector_degree(fromVector);
    double angleTo = _get_vector_degree(toVector);

    double rotateAngle = (angleTo - angleFrom);

    // Dealing with spacial cases that user drag across x axis
    if (rotateAngle > 200)
    {
        rotateAngle = rotateAngle - 360;
    }
    else if (rotateAngle < -200)
    {
        rotateAngle = rotateAngle + 360;
    }

    return rotateAngle;
}
#pragma endregion

void mouseControl(unsigned int action)
{
    struct TbPoint mousePosDelta;
    struct TbPoint dstPos;

    bool isCtrlDown = lbInkeyFlags & KMod_CONTROL;

    static unsigned long leftButtonPressedTime = 0;
    static unsigned long leftButtonHoldTime = 0;
    static unsigned long rightButtonPressedTime = 0;
    static unsigned long rightButtonHoldTime = 0; 

    // Until we are sure user wants to do dragging, the moving distance is reserved.
    static int reservedMoveX = 0;
    static int reservedMoveY = 0;
    static int reservedRotate = 0;

    // Both thresholds are lower bound.
    const int dragTimeThresholdSmall = 100;
    const int dragTimeThresholdLarge = 150;

    _get_mouse_state(&mousePosDelta, &dstPos);

    switch (action)
    {
    case MActn_MOUSEMOVE:
#pragma region LeftDragging
        // Drag to move camera when ctrl is pressed, or there is nothing else to do.
        if (lbDisplayEx.isDragMovingCamera)
        {
            lbDisplayEx.cameraMoveX += mousePosDelta.x * lbDisplayEx.cameraMoveRatioX;
            lbDisplayEx.cameraMoveY += mousePosDelta.y * lbDisplayEx.cameraMoveRatioY;                
        }
        else if (lbUseDirectMouseDragging && lbDisplay.MLeftButton && lbDisplayEx.isPowerHandNothingTodoLeftClick)
        {
            leftButtonHoldTime = LbTimerClock() - leftButtonPressedTime;

            if ((leftButtonHoldTime > dragTimeThresholdLarge) ||
                ((leftButtonHoldTime > dragTimeThresholdSmall) && ((abs(reservedMoveX) + abs(reservedMoveY)) > 1000)))
            {
                // SYNCLOG("left hold time %d", leftButtonHoldTime);
                // SYNCLOG("left move speed %d", (abs(reservedMoveX) + abs(reservedMoveY)));
                
                // Once entered dragging mode, it should not be disrupted.
                lbDisplayEx.isDragMovingCamera = true;

                // Skip next button release event to prevent unexpected behavior(dig or catching creatures).
                lbDisplayEx.skipLButtonRelease = true;

                // Apply reserved move.
                lbDisplayEx.cameraMoveX += reservedMoveX;
                lbDisplayEx.cameraMoveY += reservedMoveY;
                reservedMoveX = 0;
                reservedMoveY = 0;

                // New move this turn.
                lbDisplayEx.cameraMoveX += mousePosDelta.x * lbDisplayEx.cameraMoveRatioX;
                lbDisplayEx.cameraMoveY += mousePosDelta.y * lbDisplayEx.cameraMoveRatioY;
            }
            else
            {
                // Cache move distance when we are not sure user want drag or click.
                reservedMoveX += mousePosDelta.x * lbDisplayEx.cameraMoveRatioX;
                reservedMoveY += mousePosDelta.y * lbDisplayEx.cameraMoveRatioY;
            }
        }
#pragma endregion

#pragma region RightDragging
        // Right drag to rotate camera when ctrl is pressed, or there is nothing else to do.
        if (((lbDisplayEx.isDragRotatingCamera) ||
            (lbUseDirectMouseDragging && lbDisplay.MRightButton && lbDisplayEx.isPowerHandNothingTodoRightClick)
            ))
        {

            // Amplify with angle convert ratio.
            double rotateParam = _calculate_rotate_angle(dstPos) * PARAM_DEGREE_CONVERT_RATIO;

            if (lbDisplayEx.isDragRotatingCamera)
            {
                lbDisplayEx.cameraRotateAngle += rotateParam;
            }
            else
            {
                rightButtonHoldTime = LbTimerClock() - rightButtonPressedTime;

                if ((rightButtonHoldTime > dragTimeThresholdLarge) ||
                    ((rightButtonHoldTime > dragTimeThresholdSmall) && ((abs(reservedMoveX) + abs(reservedMoveY)) > 1200) && (abs(_calculate_rotate_angle(dstPos)) > 35)))
                {
                    // SYNCLOG("right hold time %d", rightButtonHoldTime);
                    // SYNCLOG("right rotate speed %d", reservedRotate);

                    // Once entered dragging mode, it should not be disrupted.
                    lbDisplayEx.isDragRotatingCamera = true;

                    // Skip next button release event to prevent unexpected behavior(slap or dismiss).
                    lbDisplayEx.skipRButtonRelease = true;

                    // Use reserved angle.
                    lbDisplayEx.cameraRotateAngle += reservedRotate;
                    reservedRotate = 0;                
                    reservedMoveX = 0;
                    reservedMoveY = 0;

                    // New delta this turn.
                    lbDisplayEx.cameraRotateAngle += rotateParam;
                }
                else
                {
                    // Cache rotate param when we are not sure user want drag or click.
                    reservedRotate += rotateParam;

                    // Cache move distance because it is more useful when deciding whether enter drag mode.
                    reservedMoveX += mousePosDelta.x * lbDisplayEx.cameraMoveRatioX;
                    reservedMoveY += mousePosDelta.y * lbDisplayEx.cameraMoveRatioY;
                }
            }
        }
#pragma endregion

        // Normal mouse move
        LbMouseOnMove(dstPos);

        break;
    case MActn_LBUTTONDOWN:
        lbDisplay.MLeftButton = 1;
        LbMouseOnMove(dstPos);
        if (!lbDisplay.LeftButton && !lbDisplayEx.isDragMovingCamera)
        {
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.RLeftButton = 0;

            leftButtonPressedTime = LbTimerClock();

            // Do not start dragging when mouse is hovering over a button.
            if (isCtrlDown && !lbDisplayEx.isMouseOverButton)
            {
                lbDisplayEx.isDragMovingCamera = true;

                // Skip next button release event to prevent unexpected behavior(dig or catching creatures).
                lbDisplayEx.skipLButtonRelease = true;
            }
            else
            { 
                lbDisplay.LeftButton = 1;
            }
        }
        break;
    case MActn_LBUTTONUP:
        lbDisplay.MLeftButton = 0;
        lbDisplayEx.isDragMovingCamera = false;
        leftButtonPressedTime = 0;
        leftButtonHoldTime = 0;
        reservedMoveX = 0;
        reservedMoveY = 0;

        LbMouseOnMove(dstPos);
        if (!lbDisplay.RLeftButton)
        {
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RLeftButton = 1;
        }
        break;
    case MActn_RBUTTONDOWN:
        lbDisplay.MRightButton = 1;
        LbMouseOnMove(dstPos);
        if (!lbDisplay.RightButton && !lbDisplayEx.isDragRotatingCamera)
        {
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.RRightButton = 0;

            rightButtonPressedTime = LbTimerClock();

            // Do not start dragging when mouse is hovering over a button.
            if (isCtrlDown && !lbDisplayEx.isMouseOverButton)
            {
                lbDisplayEx.isDragRotatingCamera = true;

                // Skip next button release event to prevent unexpected behavior(slap or dismiss).
                lbDisplayEx.skipRButtonRelease = true;
            }
            else
            {
                lbDisplay.RightButton = 1;
            }
        }
        break;
    case MActn_RBUTTONUP:
        lbDisplay.MRightButton = 0;
        lbDisplayEx.isDragRotatingCamera = false;
        rightButtonPressedTime = 0;
        rightButtonHoldTime = 0;    
        reservedRotate = 0;

        LbMouseOnMove(dstPos);
        if ( !lbDisplay.RRightButton )
        {
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RRightButton = 1;
        }
        break;
    case MActn_WHEELUP:
        lbDisplayEx.wheelUp = true;
        break;
    case MActn_WHEELDOWN:
        lbDisplayEx.wheelDown = true;
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
    SYNCLOG("New ratio %ldx%ld", ratio_x , ratio_y);
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
