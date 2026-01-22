/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_inputctrl.h
 *     Header file for bflib_inputctrl.h.
 * @par Purpose:
 *     Input devices control and polling.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     16 Mar 2009 - 12 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_INPUTCTRL_H
#define BFLIB_INPUTCTRL_H

#include "bflib_basics.h"

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum MouseGrabEvents {
    MG_InitMouse         = 0x001,
    MG_OnPauseEnter      = 0x002,
    MG_OnPauseLeave      = 0x004,
    MG_OnFocusLost       = 0x010,
    MG_OnFocusGained     = 0x020,
    MG_OnPossessionEnter = 0x100,
    MG_OnPossessionLeave = 0x200,
    MG_InPossessionMode  = 0x400,
};
/******************************************************************************/
extern volatile int lbUserQuit;
extern volatile TbBool lbMouseGrab;
extern volatile TbBool lbMouseGrabbed;
extern volatile TbBool lbAppActive;

extern float movement_accum_x;
extern float movement_accum_y;
/******************************************************************************/
TbBool LbWindowsControl(void);
TbBool LbIsActive(void);
TbBool LbIsMouseActive(void);
void LbGrabMouseCheck(long grab_event);
void LbGrabMouseInit(void);
void LbSetMouseGrab(TbBool grab_mouse);
void controller_rumble(long ms);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
