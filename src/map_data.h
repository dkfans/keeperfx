/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_data.h
 *     Header file for map_data.c.
 * @par Purpose:
 *     Map array data management functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     15 May 2009 - 12 Apr 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_MAPDATA_H
#define DK_MAPDATA_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Map {
      unsigned char flags;
      unsigned long data;
};

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
extern struct Map bad_map_block;
extern const long map_to_slab[];
/******************************************************************************/
struct Map *get_map_block(long stl_x, long stl_y);
TbBool map_block_invalid(struct Map *map);

unsigned long get_subtile_number(long stl_x, long stl_y);
long stl_num_decode_x(unsigned long stl_num);
long stl_num_decode_y(unsigned long stl_num);
long slab_center_subtile(long stl_v);
long slab_starting_subtile(long stl_v);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
