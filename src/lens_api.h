/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_api.h
 *     Header file for lens_api.c.
 * @par Purpose:
 *     Eye lenses support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LENS_API_H
#define DK_LENS_API_H

#include "globals.h"
#include "bflib_video.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

/******************************************************************************/
DLLIMPORT unsigned long *_DK_eye_lens_memory;
#define eye_lens_memory _DK_eye_lens_memory
DLLIMPORT TbPixel *_DK_eye_lens_spare_screen_memory;
#define eye_lens_spare_screen_memory _DK_eye_lens_spare_screen_memory

#pragma pack()
/******************************************************************************/
extern unsigned int eye_lens_width;
extern unsigned int eye_lens_height;
/******************************************************************************/
void initialise_eye_lenses(void);
void setup_eye_lens(long nlens);
void reinitialise_eye_lens(long nlens);
void reset_eye_lenses(void);
void draw_lens_effect(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, long width, long height, long effect);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
