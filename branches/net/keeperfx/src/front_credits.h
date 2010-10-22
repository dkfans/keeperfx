/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_credits.h
 *     Header file for front_credits.c.
 * @par Purpose:
 *     Credits displaying routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONT_CREDITS_H
#define DK_FRONT_CREDITS_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
DLLIMPORT extern long _DK_credits_scroll_speed;
#define credits_scroll_speed _DK_credits_scroll_speed
DLLIMPORT extern long _DK_credits_offset;
#define credits_offset _DK_credits_offset
DLLIMPORT extern int _DK_credits_end;
#define credits_end _DK_credits_end
/******************************************************************************/
void frontcredits_draw(void);
void frontcredits_input(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
