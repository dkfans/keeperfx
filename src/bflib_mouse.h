/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 12 Feb 2008

// Purpose:
//    Header file for bflib_mouse.c.

// Comment:
//   Just a header file - #defines, typedefs, function prototypes etc.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#ifndef BFLIB_MOUSE_H
#define BFLIB_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#pragma pack(1)

struct TbSprite;

struct mouse_buffer {
        bool Valid;
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

/*
extern struct DevInput joy;
*/
/******************************************************************************/
int LbMouseChangeSpriteAndHotspot(struct TbSprite *spr, int a, int b);
int LbMouseSetup(struct TbSprite *MouseSprite);
int LbMouseSetPointerHotspot(int x, int y);
int LbMouseSetPosition(int x, int y);

int __fastcall LbMouseReset();
int __fastcall LbMousePlace(void);
int __fastcall LbMouseRemove(void);
int __fastcall LbMouseSuspend(void);

int __fastcall screen_place(void);
int __fastcall screen_remove(unsigned long force);
bool __fastcall adjust_point(long *x, long *y);
char __fastcall mouse_in_rect(short x1, short x2, short y1, short y2);
bool mouse_setup_range(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
