/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_vidsurface.h
 *     Header file for bflib_vidsurface.c.
 * @par Purpose:
 *     Graphics surfaces support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2010 - 30 Sep 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_VIDSURFACE_H
#define BFLIB_VIDSURFACE_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct SDL_Surface;
struct TbRect;

struct SSurface {
    struct SDL_Surface * surf_data;
    unsigned long locks_count;
    long pitch;
};
/******************************************************************************/
extern struct SDL_Surface * lbScreenSurface;
extern struct SDL_Surface * lbDrawSurface;
/******************************************************************************/
void LbScreenSurfaceInit(struct SSurface *surf);
TbResult LbScreenSurfaceCreate(struct SSurface *surf, unsigned long w, unsigned long h);
TbResult LbScreenSurfaceRelease(struct SSurface *surf);
TbResult LbScreenSurfaceBlit(struct SSurface *surf, unsigned long x, unsigned long y,
    struct TbRect *rect, unsigned long blflags);
void *LbScreenSurfaceLock(struct SSurface *surf);
TbResult LbScreenSurfaceUnlock(struct SSurface *surf);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
