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

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
TbBool block_has_diggable_side(long plyr_idx, long slb_x, long slb_y);

void mine_out_block(long a1, long a2, long a3);
unsigned char dig_has_revealed_area(long a1, long a2, unsigned char a3);
void dig_out_block(long a1, long a2, long a3);
void check_map_explored(struct Thing *thing, long a2, long a3);
long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey);
TbBool set_slab_explored(long plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);

long element_top_face_texture(struct Map *map);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
