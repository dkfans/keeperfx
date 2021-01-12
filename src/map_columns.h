/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_columns.h
 *     Header file for map_columns.c.
 * @par Purpose:
 *     Column and columns array data management functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 Oct 2009 - 09 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_MAPCOLUMN_H
#define DK_MAPCOLUMN_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define COLUMNS_COUNT        2048
#define COLUMN_STACK_HEIGHT     8
/******************************************************************************/
#pragma pack(1)

struct Map;

struct Column { // sizeof=0x18
    short use;
    unsigned char bitfields;
    unsigned short solidmask;
    unsigned short baseblock;
    unsigned char orient;
    unsigned short cubes[COLUMN_STACK_HEIGHT];
};

struct Columns {
    struct Column *lookup[COLUMNS_COUNT];
    struct Column *end;
};


#pragma pack()
/******************************************************************************/
#define INVALID_COLUMN game.columns.lookup[0]
/******************************************************************************/
struct Column *get_column_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Column *get_map_column(const struct Map *map);
struct Column *get_column(long idx);
TbBool column_invalid(const struct Column *col);

void make_solidmask(struct Column *col);
void clear_columns(void);
void init_columns(void);
long find_column(struct Column *col);
long create_column(struct Column *col);
unsigned short find_column_height(struct Column *col);
void init_whole_blocks(void);
void init_top_texture_to_cube_table(void);

long get_column_floor_filled_subtiles(const struct Column *col);
long get_map_floor_filled_subtiles(const struct Map *mapblk);
long get_floor_filled_subtiles_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void set_column_floor_filled_subtiles(struct Column *col, MapSubtlCoord n);
void set_map_floor_filled_subtiles(struct Map *mapblk, MapSubtlCoord n);
long get_column_ceiling_filled_subtiles(const struct Column *col);
long get_map_ceiling_filled_subtiles(const struct Map *mapblk);
long get_ceiling_filled_subtiles_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void set_column_ceiling_filled_subtiles(struct Column *col, MapSubtlCoord n);
void set_map_ceiling_filled_subtiles(struct Map *mapblk, MapSubtlCoord n);
TbBool map_pos_solid_at_ceiling(MapSubtlCoord stl_x, MapSubtlCoord stl_y);


long get_top_cube_at_pos(long mpos);
long get_top_cube_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *cube_pos);
long get_map_floor_height(const struct Map *mapblk);
long get_floor_height(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
long get_floor_height_at(const struct Coord3d *pos);
long get_map_ceiling_height(const struct Map *mapblk);
long get_ceiling_height_at(const struct Coord3d *pos);
long get_ceiling_height_at_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

TbBool cube_is_water(long cube_id);
TbBool cube_is_lava(long cube_id);
TbBool cube_is_sacrificial(long cube_id);

TbBool subtile_has_water_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_has_lava_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_has_sacrificial_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
