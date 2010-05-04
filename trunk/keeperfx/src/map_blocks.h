/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_blocks.h
 *     Header file for map_blocks.c.
 * @par Purpose:
 *     Map blocks support functions.
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
#ifndef DK_MAP_BLOCKS_H
#define DK_MAP_BLOCKS_H

#include "globals.h"

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
long block_has_diggable_side(long a1, long a2, long a3);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
