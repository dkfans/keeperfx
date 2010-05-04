/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_flyeye.c
 *     lens_flyeye support functions.
 * @par Purpose:
 *     Functions to lens_flyeye.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "lens_flyeye.h"

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_flyeye_setup(long width, long height);
DLLIMPORT void _DK_flyeye_blitsec(unsigned char *srcbuf, unsigned char *dstbuf, long srcwidth, long dstwidth, long n, long height);
/******************************************************************************/
void flyeye_setup(long width, long height)
{
  _DK_flyeye_setup(width, height);
}

void flyeye_blitsec(unsigned char *srcbuf, unsigned char *dstbuf, long srcwidth, long dstwidth, long n, long height)
{
  _DK_flyeye_blitsec(srcbuf, dstbuf, srcwidth, dstwidth, n, height);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
