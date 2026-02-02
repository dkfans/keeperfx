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
#include "map_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

enum MapCoordClipFlags {
    MapCoord_None     = 0x00,
    MapCoord_ClipX    = 0x01,
    MapCoord_ClipY    = 0x02,
    MapCoord_ClipZ    = 0x04,
};

struct Map {
      unsigned char flags; // flags in enum SlabAttrFlags
      unsigned char filled_subtiles;
      unsigned char wibble_value;
      ColumnIndex col_idx;
      ThingIndex mapwho;
      PlayerBitFlags revealed;
};

#define INVALID_MAP_BLOCK (&bad_map_block)
#define MOVE_VELOCITY_LIMIT 256
#define STL_PER_SLB 3
#define COORD_PER_STL 256
#define COORD_PER_SLB (COORD_PER_STL*STL_PER_SLB)
#define FILLED_COLUMN_HEIGHT 1280
#define DEFAULT_MAP_SIZE 85

#pragma pack()
/******************************************************************************/
extern struct Map bad_map_block;
extern MapSubtlCoord map_subtiles_z;
extern NavColour *IanMap;
extern long nav_map_initialised;
/******************************************************************************/
/** Convert subtile to slab. */
#define subtile_slab(stl) ((stl)/STL_PER_SLB)
/** Converts slab to a subtile. Second parameter selects a specific subtile. */
#define slab_subtile(slb,subnum) ((MapSubtlCoord)(slb)*STL_PER_SLB+(MapSubtlCoord)(subnum))
/** Converts slab to its central subtile. */
#define slab_subtile_center(slb) ((MapSubtlCoord)(slb)*STL_PER_SLB+(MapSubtlCoord)1)
/******************************************************************************/
/** Converts coord value to number of subtiles. */
#define coord_subtile(coord) ((coord)/COORD_PER_STL)
/** Converts coord value to number of slabs. */
#define coord_slab(coord) ((coord)/(COORD_PER_SLB))
/** Converts subtile to coord value. */
#define subtile_coord(stl,spos) ((stl)*COORD_PER_STL+(spos))
/** Converts slab to coord value. */
#define slab_coord(slb) ((slb) * (COORD_PER_SLB))
#define subtile_coord_center(stl) ((stl)*COORD_PER_STL+COORD_PER_STL/2)
#define navmap_tile_number(stl_x,stl_y) ((stl_y)*game.navigation_map_size_x+(stl_x))
/******************************************************************************/
struct Map *get_map_block_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Map *get_map_block_at_pos(SubtlCodedCoords stl_num);
TbBool map_block_invalid(const struct Map *mapblk);

void reveal_map_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
TbBool subtile_revealed(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
TbBool subtile_revealed_directly(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
#define thing_revealed(thing, plyr_idx) subtile_revealed(thing->mappos.x.stl.num, thing->mappos.y.stl.num, plyr_idx)
void reveal_map_block(struct Map *mapblk, PlayerNumber plyr_idx);
void conceal_map_block(struct Map *mapblk, PlayerNumber plyr_idx);
TbBool slabs_reveal_slab_and_corners(MapSlabCoord slab_x, MapSlabCoord slab_y, MaxCoordFilterParam param);
TbBool slabs_change_owner(MapSlabCoord slab_x, MapSlabCoord slab_y, MaxCoordFilterParam param);
TbBool slabs_change_type(MapSlabCoord slab_x, MapSlabCoord slab_y, MaxCoordFilterParam param);
TbBool slabs_change_texture(MapSlabCoord slb_x, MapSlabCoord slb_y, MaxCoordFilterParam param);
TbBool map_block_revealed(const struct Map *mapblk, PlayerNumber plyr_idx);
TbBool map_block_revealed_directly(const struct Map* mapblk, PlayerNumber plyr_idx);

TbBool valid_dig_position(PlayerNumber plyr_idx, long stl_x, long stl_y);
long get_ceiling_height(const struct Coord3d *pos);
ThingIndex get_mapwho_thing_index(const struct Map *mapblk);
void set_mapwho_thing_index(struct Map *map, ThingIndex thing_idx);
long get_mapblk_column_index(const struct Map *map);
void set_mapblk_column_index(struct Map *map, long column_idx);
long get_mapblk_filled_subtiles(const struct Map *mapblk);
void set_mapblk_filled_subtiles(struct Map *map, long height);
long get_mapblk_wibble_value(const struct Map *mapblk);
void set_mapblk_wibble_value(struct Map *mapblk, long wib);

NavColour get_navigation_map(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void set_navigation_map(MapSubtlCoord stl_x, MapSubtlCoord stl_y, NavColour navcolour);
unsigned long get_navigation_map_floor_height(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

TbBool set_coords_with_clip(struct Coord3d *pos, MapCoord cor_x, MapCoord cor_y, MapCoord cor_z);
TbBool subtile_has_slab(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_coords_invalid(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool set_coords_to_subtile_center(struct Coord3d *pos, MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_z);
TbBool set_coords_to_subtile_start(struct Coord3d *pos, long x, long y, long z);
TbBool set_coords_to_subtile_end(struct Coord3d *pos, long x, long y, long z);
TbBool set_coords_to_slab_center(struct Coord3d *pos, MapSubtlCoord slb_x, MapSubtlCoord slb_y);
TbBool set_coords_to_cylindric_shift(struct Coord3d *pos, const struct Coord3d *source, long radius, long angle, long z);
TbBool set_coords_add_velocity(struct Coord3d *pos, const struct Coord3d *source, const struct CoordDelta3d *velocity, unsigned short flags);

SubtlCodedCoords get_subtile_number(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
SubtlCodedCoords get_subtile_number_at_slab_center(long slb_x, long slb_y);
MapSubtlCoord stl_num_decode_x(SubtlCodedCoords stl_num);
MapSubtlCoord stl_num_decode_y(SubtlCodedCoords stl_num);
MapSubtlCoord stl_slab_center_subtile(MapSubtlCoord stl_v);
MapSubtlCoord stl_slab_starting_subtile(MapSubtlCoord stl_v);
MapSubtlCoord stl_slab_ending_subtile(MapSubtlCoord stl_v);

TbBool map_pos_is_lava(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool lava_at_position(const struct Coord3d *pos);
TbBool subtile_is_room(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_is_player_room(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_is_sellable_room(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_is_sellable_door_or_trap(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_is_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
#define can_dig_here(stl_x, stl_y, plyr_idx, enemy_wall_diggable) subtile_is_diggable_for_player(plyr_idx, stl_x, stl_y, enemy_wall_diggable)
TbBool subtile_is_diggable_for_player(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool enemy_wall_diggable);

void clear_dig_for_map_rect(long plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y);
void clear_slab_dig(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx);

void player_reveal_map_area(PlayerNumber plyr_idx, MapSubtlCoord x, MapSubtlCoord y, MapSubtlDelta w, MapSubtlDelta h);
void player_conceal_map_area(PlayerNumber plyr_idx, MapSubtlCoord x, MapSubtlCoord y, MapSubtlDelta w, MapSubtlDelta h, TbBool all);
void reveal_map_rect(PlayerNumber plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y);
void reveal_map_area(PlayerNumber plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y);
void conceal_map_area(PlayerNumber plyr_idx,MapSubtlCoord start_x,MapSubtlCoord end_x,MapSubtlCoord start_y,MapSubtlCoord end_y, TbBool all);
void clear_mapwho(void);
void clear_mapmap(void);

void set_map_size(MapSlabCoord slb_x,MapSlabCoord slb_y);
void init_map_size(LevelNumber lvnum);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
