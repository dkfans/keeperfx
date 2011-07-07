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
#pragma pack(1)

struct Map {
      unsigned char flags;
      unsigned long data;
};

#define INVALID_MAP_BLOCK (&bad_map_block)

#pragma pack()
/******************************************************************************/
extern struct Map bad_map_block;
extern const long map_to_slab[];
extern int map_subtiles_x;
extern int map_subtiles_y;
extern int map_subtiles_z;
extern int map_tiles_x;
extern int map_tiles_y;
extern long navigation_map_size_x;
extern long navigation_map_size_y;
extern unsigned char *IanMap;
extern long nav_map_initialised;
/******************************************************************************/
#define coord_subtile(coord) ((coord)/256)
#define subtile_coord(stl,spos) ((stl)*256+(spos))
#define subtile_coord_center(stl) ((stl)*256+128)
#define navmap_tile_number(stl_x,stl_y) ((stl_y)*navigation_map_size_x+(stl_x))
/******************************************************************************/
struct Map *get_map_block_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Map *get_map_block_at_pos(long stl_num);
unsigned long get_navigation_map(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void set_navigation_map(MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long navcolour);
TbBool map_block_invalid(const struct Map *map);
void reveal_map_subtile(long stl_x, long stl_y, long plyr_idx);
TbBool subtile_revealed(long stl_x, long stl_y, long plyr_idx);
void reveal_map_block(struct Map *map, long plyr_idx);
TbBool map_block_revealed(const struct Map *map, long plyr_idx);
TbBool map_block_revealed_bit(const struct Map *map, long plyr_bit);
TbBool valid_dig_position(long plyr_idx, long stl_x, long stl_y);
long get_ceiling_height(const struct Coord3d *pos);
long get_mapwho_thing_index(const struct Map *map);
void set_mapwho_thing_index(struct Map *map, long thing_idx);
long get_mapblk_column_index(const struct Map *map);
void set_mapblk_column_index(struct Map *map, long column_idx);
long get_subtile_lightness(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

TbBool subtile_has_slab(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_coords_invalid(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool set_coords_to_subtile_center(struct Coord3d *pos, long x, long y, long z);
TbBool set_coords_to_subtile_start(struct Coord3d *pos, long x, long y, long z);
TbBool set_coords_to_subtile_end(struct Coord3d *pos, long x, long y, long z);
TbBool set_coords_to_slab_center(struct Coord3d *pos, long slb_x, long slb_y);
MapCoord get_subtile_center_pos(MapSubtlCoord stl_v);
SubtlCodedCoords get_subtile_number(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
SubtlCodedCoords get_subtile_number_at_slab_center(long slb_x, long slb_y);
MapSubtlCoord stl_num_decode_x(SubtlCodedCoords stl_num);
MapSubtlCoord stl_num_decode_y(SubtlCodedCoords stl_num);
MapSubtlCoord slab_center_subtile(MapSubtlCoord stl_v);
MapSubtlCoord slab_starting_subtile(MapSubtlCoord stl_v);
MapSubtlCoord slab_ending_subtile(MapSubtlCoord stl_v);

TbBool map_pos_is_lava(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_is_sellable_room(long plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

void clear_dig_for_map_rect(long plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y);
void reveal_map_rect(long plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y);
void reveal_map_area(long plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y);
void clear_mapwho(void);
void clear_mapmap(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
