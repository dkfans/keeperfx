/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_easter.h
 *     Header file for front_easter.c.
 * @par Purpose:
 *     Easter Eggs displaying routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     01 Jan 2012 - 23 Jun 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONT_EASTER_H
#define DK_FRONT_EASTER_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_keybrd.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct KeycodeString {
    TbKeyCode keys[LINEMSG_SIZE];
    long length;
};

#pragma pack()
/******************************************************************************/
void frontbirthday_draw(void);
void input_eastegg(void);
void draw_eastegg(void);
const char *get_team_birthday(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
