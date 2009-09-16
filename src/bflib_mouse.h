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

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct TbSprite;
class LbSemaphore;

struct MouseStateHandler { // sizeof = 4224
  LbSemaphore *semaphore;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned long field_14;
  unsigned long field_18;
  unsigned long field_1C;
  unsigned long field_20;
  unsigned long field_24;
  unsigned char field_28[12];
  unsigned long field_34;
  unsigned long field_38;
  unsigned long field_3C;
  unsigned char field_40[4108];
  unsigned long field_104C;
  unsigned long field_1050;
  unsigned char field_1054[16];
  unsigned long field_1064;
  unsigned long field_1068;
  unsigned long field_106C;
  unsigned long field_1070;
  unsigned long field_1074;
  unsigned long sema_rel;
  unsigned long field_107C;
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

/******************************************************************************/
DLLIMPORT struct MouseStateHandler _DK_winMouseHandler;
#define winMouseHandler _DK_winMouseHandler
DLLIMPORT int volatile _DK_lbMouseInstalled;
#define lbMouseInstalled _DK_lbMouseInstalled

#pragma pack()
/******************************************************************************/
int LbMouseChangeSpriteAndHotspot(struct TbSprite *mouseSprite, int hot_x, int hot_y);
int LbMouseSetup(struct TbSprite *MouseSprite);
int LbMouseSetPointerHotspot(int hot_x, int hot_y);
int LbMouseSetPosition(int x, int y);
short LbMouseChangeSprite(struct TbSprite *MouseSprite);
int LbMouseSuspend(void);
/*
int __fastcall LbMouseReset();
int __fastcall LbMousePlace(void);
int __fastcall LbMouseRemove(void);

int __fastcall screen_place(void);
int __fastcall screen_remove(unsigned long force);
bool __fastcall adjust_point(long *x, long *y);
char __fastcall mouse_in_rect(short x1, short x2, short y1, short y2);
bool mouse_setup_range(void);
void __fastcall LbProcessMouseMove(struct SDL_MouseMotionEvent *motion);
void __fastcall LbProcessMouseClick(struct SDL_MouseButtonEvent *button);
*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
