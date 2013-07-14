/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_findcache.h
 *     Header file for ariadne_findcache.c.
 * @par Purpose:
 *     FindCache support functions for Ariadne pathfinding.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 05 Aug 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_FINDCACHE_H
#define DK_ARIADNE_FINDCACHE_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)


#pragma pack()
/******************************************************************************/
long triangle_find_cache_get(long pos_x, long pos_y);
void triangle_find_cache_put(long pos_x, long pos_y, long ntri);

void triangulation_init_cache(long tri_idx);

long triangle_find8(long pt_x, long pt_y);
TbBool point_find(long pt_x, long pt_y, long *out_tri_idx, long *out_cor_idx);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
