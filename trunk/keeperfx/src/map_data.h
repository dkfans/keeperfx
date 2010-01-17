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

#define INVALID_MAP_BLOCK (&bad_map_block)

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
extern struct Map bad_map_block;
extern const long map_to_slab[];
/******************************************************************************/
struct Map *get_map_block_at(long stl_x, long stl_y);
struct Map *get_map_block_at_pos(long stl_num);
unsigned long get_map_flags(long stl_x, long stl_y);
TbBool map_block_invalid(const struct Map *map);
void reveal_map_subtile(long stl_x, long stl_y, long plyr_idx);
TbBool subtile_revealed(long stl_x, long stl_y, long plyr_idx);
TbBool map_block_revealed(const struct Map *map, long plyr_idx);
TbBool map_block_revealed_bit(const struct Map *map, long plyr_bit);
long get_ceiling_height(const struct Coord3d *pos);
long get_mapwho_thing_index(const struct Map *map);
void set_mapwho_thing_index(struct Map *map, long thing_idx);

TbBool subtile_has_slab(long stl_x, long stl_y);
TbBool subtile_coords_invalid(long stl_x, long stl_y);
TbBool set_coords_to_subtile_center(struct Coord3d *pos, long x, long y, long z);
TbBool set_coords_to_subtile_start(struct Coord3d *pos, long x, long y, long z);
TbBool set_coords_to_subtile_end(struct Coord3d *pos, long x, long y, long z);
TbBool set_coords_to_slab_center(struct Coord3d *pos, long slb_x, long slb_y);
unsigned long get_subtile_number(long stl_x, long stl_y);
unsigned long get_subtile_number_at_slab_center(long slb_x, long slb_y);
long stl_num_decode_x(unsigned long stl_num);
long stl_num_decode_y(unsigned long stl_num);
long slab_center_subtile(long stl_v);
long slab_starting_subtile(long stl_v);
long slab_ending_subtile(long stl_v);

TbBool map_pos_is_lava(long stl_x, long stl_y);

void clear_dig_for_map_rect(long plyr_idx,long start_x,long end_x,long start_y,long end_y);
void reveal_map_rect(long plyr_idx,long start_x,long end_x,long start_y,long end_y);
void reveal_map_area(long plyr_idx,long start_x,long end_x,long start_y,long end_y);
void clear_mapwho(void);
void clear_mapmap(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
