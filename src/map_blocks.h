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
#pragma pack(1)

struct Thing;
struct Map;

#pragma pack()
/******************************************************************************/
TbBool block_has_diggable_side(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
int block_count_diggable_sides(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
long tag_blocks_for_digging_in_rectangle_around(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
void untag_blocks_for_digging_in_rectangle_around(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
TbBool tag_blocks_for_digging_in_area(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
long untag_blocks_for_digging_in_area(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
void all_players_untag_blocks_for_digging_in_area(MapSlabCoord slb_x, MapSlabCoord slb_y);

#define place_slab_type_on_map(nslab, stl_x, stl_y, owner, a5) place_slab_type_on_map_f(nslab, stl_x, stl_y, owner, a5, __func__)
void place_slab_type_on_map_f(SlabKind nslab, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner, unsigned char a5,const char *func_name);
void mine_out_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
TbBool dig_has_revealed_area(MapSubtlCoord rev_stl_x, MapSubtlCoord rev_stl_y, PlayerNumber plyr_idx);
void dig_out_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
void neutralise_enemy_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber domn_plyr_idx);
void check_map_explored(struct Thing* creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey);
TbBool set_slab_explored(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
void update_floor_and_ceiling_heights_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y,
    MapSubtlCoord *floor_height, MapSubtlCoord *ceiling_height);
TbBool point_in_map_is_solid(const struct Coord3d *pos);
TbBool point_in_map_is_solid_ignoring_door(const struct Coord3d *pos, const struct Thing *doortng);
unsigned short get_point_in_map_solid_flags_ignoring_door(const struct Coord3d *pos, const struct Thing *doortng);
unsigned short get_point_in_map_solid_flags_ignoring_own_door(const struct Coord3d *pos, PlayerNumber plyr_idx);
SlabKind alter_rock_style(SlabKind slbkind, MapSlabCoord tgslb_x, MapSlabCoord tgslb_y, PlayerNumber owner);
void create_dirt_rubble_for_dug_slab(MapSlabCoord slb_x, MapSlabCoord slb_y);
void create_dirt_rubble_for_dug_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_height, PlayerNumber owner);
void place_and_process_pretty_wall_slab(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y);
void place_animating_slab_type_on_map(SlabKind slbkind, char a2, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner);
SlabKind choose_pretty_type(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
void pretty_map_remove_flags_and_update(MapSlabCoord slb_x, MapSlabCoord slb_y);
void fill_in_reinforced_corners(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);

long element_top_face_texture(struct Map *map);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
