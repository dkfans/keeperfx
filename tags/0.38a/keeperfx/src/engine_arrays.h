/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_arrays.h
 *     Header file for engine_arrays.c.
 * @par Purpose:
 *     Helper arrays for the engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     02 Apr 2010 - 06 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_ENGNARR_H
#define DK_ENGNARR_H

#include "bflib_basics.h"
#include "globals.h"

#define TD_ISO_POINTS        982

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif


#ifdef __cplusplus
#pragma pack()
#endif

/******************************************************************************/
DLLIMPORT long _DK_randomisors[512];
#define randomisors _DK_randomisors

/******************************************************************************/
unsigned long convert_td_iso(unsigned long n);

void init_iso_3d_conversion_tables(void);
void setup_3d(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
