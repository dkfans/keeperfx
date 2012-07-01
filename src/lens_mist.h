/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_mist.h
 *     Header file for lens_mist.cpp.
 * @par Purpose:
 *     Mist lens effect functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LENSMIST_H
#define DK_LENSMIST_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void setup_mist(unsigned char *lens_mem, unsigned char *fade, unsigned char *ghost);
TbBool draw_mist(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, long width, long height);
void free_mist(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
