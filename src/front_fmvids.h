/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_fmvids.h
 *     Header file for front_fmvids.c.
 * @par Purpose:
 *     Full Motion Videos displaying routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 10 Jun 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONT_FMVIDS_H
#define DK_FRONT_FMVIDS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/******************************************************************************/
TbBool intro(void);
TbBool intro_replay(void);
TbBool campaign_intro(void);
TbBool campaign_outro(void);
TbBool drag_video(void);
TbBool moon_video(void);
void demo(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
