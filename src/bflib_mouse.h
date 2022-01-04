/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_bufrw.c
 *     Header file for bflib_mouse.c.
 * @par Purpose:
 *     Mouse related routines.
 * @par Comment:
 *   Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 10 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_MOUSE_H
#define BFLIB_MOUSE_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct TbSprite;
struct TbPoint;

enum TbMouseAction {
    MActn_NONE = 0,
    MActn_MOUSEMOVE,
    MActn_LBUTTONDOWN,
    MActn_LBUTTONUP,
    MActn_RBUTTONDOWN,
    MActn_RBUTTONUP,
    MActn_MBUTTONDOWN,
    MActn_MBUTTONUP,
    MActn_WHEELMOVEUP,
    MActn_WHEELMOVEDOWN,
};

struct mouse_buffer {
        long Valid;//bool
        long Width;
        long Height;
        unsigned long Offset;
        unsigned char Buffer[0x1000];
        long X;
        long Y;
        long XOffset;
        long YOffset;
};

struct mouse_info {
        long XMoveRatio;
        long YMoveRatio;
        long XSpriteOffset;
        long YSpriteOffset;
        //Note: debug info says it has 0x100 items, but this is suspicious..
        unsigned char Sprite[0x1000];
};

struct DevInput {
        long Yaw[16];
        long Roll[16];
        long Pitch[16];
        long AnalogueX[16];
        long AnalogueY[16];
        long AnalogueZ[16];
        long AnalogueU[16];
        long AnalogueV[16];
        long AnalogueR[16];
        long DigitalX[16];
        long DigitalY[16];
        long DigitalZ[16];
        long DigitalU[16];
        long DigitalV[16];
        long DigitalR[16];
        long MinXAxis[16];
        long MinYAxis[16];
        long MinZAxis[16];
        long MinUAxis[16];
        long MinVAxis[16];
        long MinRAxis[16];
        long MaxXAxis[16];
        long MaxYAxis[16];
        long MaxZAxis[16];
        long MaxUAxis[16];
        long MaxVAxis[16];
        long MaxRAxis[16];
        long XCentre[16];
        long YCentre[16];
        long ZCentre[16];
        long UCentre[16];
        long VCentre[16];
        long RCentre[16];
        long HatX[16];
        long HatY[16];
        long HatMax[16];
        long Buttons[16];
        long NumberOfButtons[16];
        long ConfigType[16];
        long MenuButtons[16];
        long Type;
        long NumberOfDevices;
        long DeviceType[16];
        unsigned char Init[16];
};

#pragma pack()
/******************************************************************************/
extern volatile TbBool lbMouseGrab; // set to false if user sets altinput command line option
extern volatile TbBool lbMouseGrabbed; // whether the mouse is current grabbed by the game window
/******************************************************************************/
TbResult LbMouseChangeSpriteAndHotspot(const struct TbSprite *mouseSprite, long hot_x, long hot_y);
TbResult LbMouseSetup(struct TbSprite *mouseSprite);
TbResult LbMouseSetPointerHotspot(long hot_x, long hot_y);
TbResult LbMouseSetPosition(long x, long y);
TbResult LbMouseSetPositionInitial(long x, long y);
void LbMoveHostCursorToGameCursor(void);
TbResult LbMoveGameCursorToHostCursor(void);
TbBool IsMouseInsideWindow(void);
TbResult LbMouseChangeSprite(struct TbSprite *mouseSprite);
TbResult LbMouseSuspend(void);
void GetPointerHotspot(long *hot_x, long *hot_y);
TbResult LbMouseIsInstalled(void);
TbResult LbMouseSetWindow(long x, long y, long width, long height);
TbResult LbMouseChangeMoveRatio(long ratio_x, long ratio_y);

void mouseControl(unsigned int action, struct TbPoint *pos);
TbResult LbMouseOnBeginSwap(void);
TbResult LbMouseOnEndSwap(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
