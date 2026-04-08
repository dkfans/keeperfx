/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_joyst.h
 *     Header file for bflib_joyst.c.
 * @par Purpose:
 *     Joystick related routines - reading joystick input.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_JOYST_H
#define BFLIB_JOYST_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

enum ControllerButtons {
    CBtn_NONE           = 0x0,
    CBtn_A              = 0x1,
    CBtn_B              = 0x2,
    CBtn_X              = 0x4,
    CBtn_Y              = 0x8,
    CBtn_BACK           = 0x10,
    CBtn_START          = 0x20,
    CBtn_LEFTSTICK      = 0x40,
    CBtn_RIGHTSTICK     = 0x80,
    CBtn_LEFTSHOULDER   = 0x100,
    CBtn_RIGHTSHOULDER  = 0x200,
    CBtn_DPAD_UP        = 0x400,
    CBtn_DPAD_DOWN      = 0x800,
    CBtn_DPAD_LEFT      = 0x1000,
    CBtn_DPAD_RIGHT     = 0x2000,
    CBtn_MISC1          = 0x4000,
    CBtn_PADDLE1        = 0x8000,
    CBtn_PADDLE2        = 0x10000,
    CBtn_PADDLE3        = 0x20000,
    CBtn_PADDLE4        = 0x40000,
    CBtn_TOUCHPAD       = 0x80000,
    CBtn_L2             = 0x100000,
    CBtn_R2             = 0x200000,
    CBtn_LS_UP          = 0x400000,
    CBtn_LS_DOWN        = 0x800000,
    CBtn_LS_LEFT        = 0x1000000,
    CBtn_LS_RIGHT       = 0x2000000,
    CBtn_RS_UP          = 0x4000000,
    CBtn_RS_DOWN        = 0x8000000,
    CBtn_RS_LEFT        = 0x10000000,
    CBtn_RS_RIGHT       = 0x20000000,
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
