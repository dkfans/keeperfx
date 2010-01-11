/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file scrcapt.h
 *     Header file for scrcapt.c.
 * @par Purpose:
 *     Screen capturing functions.
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
#ifndef DK_SCRCAPT_H
#define DK_SCRCAPT_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
extern short screenshot_format;

/******************************************************************************/
TbBool perform_any_screen_capturing(void);
TbBool cumulative_screen_shot(void);

TbBool movie_record_start(void);
TbBool movie_record_stop(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
