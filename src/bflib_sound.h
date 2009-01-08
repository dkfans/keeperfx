/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sound.h
 *     Header file for bflib_sound.c.
 * @par Purpose:
 *     Sound and music related routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     16 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_SOUND_H
#define BFLIB_SOUND_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

// Type definitions
struct SoundEmitter {
    unsigned char flags;
    unsigned char field_1;
    unsigned char field_2[2];
    short pos_x;
    short pos_y;
    short pos_z;
    unsigned char field_A[10];
    unsigned char field_14;
    unsigned char field_15;
};

/******************************************************************************/
// Exported variables

#pragma pack()
/******************************************************************************/
// Exported functions

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
