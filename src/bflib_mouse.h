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
        int32_t Valid;//bool
        int32_t Width;
        int32_t Height;
        uint32_t Offset;
        unsigned char Buffer[0x1000];
        int32_t X;
        int32_t Y;
        int32_t XOffset;
        int32_t YOffset;
};

struct mouse_info {
        int32_t XMoveRatio;
        int32_t YMoveRatio;
        int32_t XSpriteOffset;
        int32_t YSpriteOffset;
        //Note: debug info says it has 0x100 items, but this is suspicious..
        unsigned char Sprite[0x1000];
};

struct DevInput {
        int32_t Yaw[16];
        int32_t Roll[16];
        int32_t Pitch[16];
        int32_t AnalogueX[16];
        int32_t AnalogueY[16];
        int32_t AnalogueZ[16];
        int32_t AnalogueU[16];
        int32_t AnalogueV[16];
        int32_t AnalogueR[16];
        int32_t DigitalX[16];
        int32_t DigitalY[16];
        int32_t DigitalZ[16];
        int32_t DigitalU[16];
        int32_t DigitalV[16];
        int32_t DigitalR[16];
        int32_t MinXAxis[16];
        int32_t MinYAxis[16];
        int32_t MinZAxis[16];
        int32_t MinUAxis[16];
        int32_t MinVAxis[16];
        int32_t MinRAxis[16];
        int32_t MaxXAxis[16];
        int32_t MaxYAxis[16];
        int32_t MaxZAxis[16];
        int32_t MaxUAxis[16];
        int32_t MaxVAxis[16];
        int32_t MaxRAxis[16];
        int32_t XCentre[16];
        int32_t YCentre[16];
        int32_t ZCentre[16];
        int32_t UCentre[16];
        int32_t VCentre[16];
        int32_t RCentre[16];
        int32_t HatX[16];
        int32_t HatY[16];
        int32_t HatMax[16];
        int32_t Buttons[16];
        int32_t NumberOfButtons[16];
        int32_t ConfigType[16];
        int32_t MenuButtons[16];
        int32_t Type;
        int32_t NumberOfDevices;
        int32_t DeviceType[16];
        unsigned char Init[16];
};

#pragma pack()
/******************************************************************************/
extern volatile TbBool lbMouseGrab; // set to false if user sets altinput command line option
extern volatile TbBool lbMouseGrabbed; // whether the mouse is current grabbed by the game window
/******************************************************************************/
TbResult LbMouseChangeSpriteAndHotspot(const struct TbSprite *mouseSprite, int32_t hot_x, int32_t hot_y);
TbResult LbMouseSetup(struct TbSprite *mouseSprite);
TbResult LbMouseSetPointerHotspot(int32_t hot_x, int32_t hot_y);
TbResult LbMouseSetPosition(int32_t x, int32_t y);
TbResult LbMouseSetPositionInitial(int32_t x, int32_t y);
void LbMoveHostCursorToGameCursor(void);
TbResult LbMoveGameCursorToHostCursor(void);
TbBool IsMouseInsideWindow(void);
TbResult LbMouseChangeSprite(const struct TbSprite *mouseSprite);
TbResult LbMouseSuspend(void);
void GetPointerHotspot(int32_t *hot_x, int32_t *hot_y);
TbResult LbMouseIsInstalled(void);
TbResult LbMouseSetWindow(int32_t x, int32_t y, int32_t width, int32_t height);
TbResult LbMouseChangeMoveRatio(int32_t ratio_x, int32_t ratio_y);

void mouseControl(unsigned int action, struct TbPoint *pos);
TbResult LbMouseOnBeginSwap(void);
TbResult LbMouseOnEndSwap(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
