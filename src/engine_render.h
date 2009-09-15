/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_render.h
 *     Header file for engine_render.c.
 * @par Purpose:
 *     Rendering the 3D view functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_ENGNREND_H
#define DK_ENGNREND_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct EngineCoord { // sizeof = 28
  unsigned long field_0;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  long x;
  long y;
  long z;
};

struct M33 { // sizeof = 48
  unsigned long r0[4];
  unsigned long r1[4];
  unsigned long r2[4];
};

struct EngineCol {
//TODO!!
};

struct BasicQ
{
//TODO!
};

struct PolyPoint { // sizeof = 8
  long field_0;
  long field_4;
//TODO!
};

struct MapVolumeBox { // sizeof = 24
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2;
  unsigned long field_3;
  unsigned long field_7;
  unsigned long field_B;
  unsigned long field_F;
  unsigned long field_13;
  unsigned char field_17;
};

/******************************************************************************/
DLLIMPORT unsigned char *_DK_getpoly;
#define getpoly _DK_getpoly
DLLIMPORT unsigned char _DK_poly_pool[0x40000];
#define poly_pool _DK_poly_pool
DLLIMPORT unsigned char *_DK_poly_pool_end;
#define poly_pool_end _DK_poly_pool_end
DLLIMPORT struct BasicQ *_DK_buckets[];
#define buckets _DK_buckets
DLLIMPORT Offset _DK_vert_offset[3];
#define vert_offset _DK_vert_offset
DLLIMPORT Offset _DK_hori_offset[3];
#define hori_offset _DK_hori_offset
DLLIMPORT Offset _DK_high_offset[3];
#define high_offset _DK_high_offset
DLLIMPORT long _DK_x_init_off;
#define x_init_off _DK_x_init_off
DLLIMPORT long _DK_y_init_off;
#define y_init_off _DK_y_init_off
DLLIMPORT long _DK_floor_pointed_at_x;
#define floor_pointed_at_x _DK_floor_pointed_at_x
DLLIMPORT long _DK_floor_pointed_at_y;
#define floor_pointed_at_y _DK_floor_pointed_at_y
DLLIMPORT long _DK_cells_away;
#define cells_away _DK_cells_away
DLLIMPORT long _DK_fade_max;
#define fade_max _DK_fade_max
DLLIMPORT long _DK_fade_scaler;
#define fade_scaler _DK_fade_scaler
DLLIMPORT long _DK_fade_way_out;
#define fade_way_out _DK_fade_way_out
DLLIMPORT struct MapVolumeBox _DK_map_volume_box;
#define map_volume_box _DK_map_volume_box
DLLIMPORT long _DK_map_angle;
#define map_angle _DK_map_angle
DLLIMPORT long _DK_map_roll;
#define map_roll _DK_map_roll
DLLIMPORT long _DK_map_tilt;
#define map_tilt _DK_map_tilt
DLLIMPORT long _DK_view_alt;
#define view_alt _DK_view_alt
DLLIMPORT long _DK_fade_min;
#define fade_min _DK_fade_min
DLLIMPORT long _DK_split_1;
#define split_1 _DK_split_1
DLLIMPORT long _DK_split_2;
#define split_2 _DK_split_2
DLLIMPORT long _DK_fade_range;
#define fade_range _DK_fade_range
DLLIMPORT long _DK_depth_init_off;
#define depth_init_off _DK_depth_init_off
DLLIMPORT int _DK_normal_shade_left;
#define normal_shade_left _DK_normal_shade_left
DLLIMPORT int _DK_normal_shade_right;
#define normal_shade_right _DK_normal_shade_right
DLLIMPORT long _DK_apos;
#define apos _DK_apos
DLLIMPORT long _DK_bpos;
#define bpos _DK_bpos
DLLIMPORT long _DK_ScrCenterX;
#define ScrCenterX _DK_ScrCenterX
DLLIMPORT long _DK_ScrWidth;
#define ScrWidth _DK_ScrWidth
DLLIMPORT long _DK_ScrHeight;
#define ScrHeight _DK_ScrHeight
DLLIMPORT long _DK_ScrCenterY;
#define ScrCenterY _DK_ScrCenterY
DLLIMPORT long _DK_split1at;
#define split1at _DK_split1at
DLLIMPORT long _DK_split2at;
#define split2at _DK_split2at
DLLIMPORT long _DK_max_i_can_see;
#define max_i_can_see _DK_max_i_can_see
DLLIMPORT long _DK_view_height_over_2;
#define view_height_over_2 _DK_view_height_over_2
DLLIMPORT long _DK_view_width_over_2;
#define view_width_over_2 _DK_view_width_over_2
DLLIMPORT int _DK_normal_shade_front;
#define normal_shade_front _DK_normal_shade_front
DLLIMPORT int _DK_normal_shade_back;
#define normal_shade_back _DK_normal_shade_back
DLLIMPORT unsigned char _DK_temp_cluedo_mode;
#define temp_cluedo_mode _DK_temp_cluedo_mode
DLLIMPORT long _DK_me_distance;
#define me_distance _DK_me_distance
DLLIMPORT short _DK_mx;
#define mx _DK_mx
DLLIMPORT short _DK_my;
#define my _DK_my
DLLIMPORT short _DK_mz;
#define mz _DK_mz
DLLIMPORT unsigned char _DK_engine_player_number;
#define engine_player_number _DK_engine_player_number
DLLIMPORT unsigned char _DK_player_bit;
#define player_bit _DK_player_bit
DLLIMPORT long _DK_gpoly_pro_enable_mode_ofs;
#define gpoly_pro_enable_mode_ofs _DK_gpoly_pro_enable_mode_ofs

#pragma pack()
/******************************************************************************/
void do_a_plane_of_engine_columns_perspective(long a1, long a2, long a3, long a4);
void do_a_plane_of_engine_columns_cluedo(long a1, long a2, long a3, long a4);
void do_a_plane_of_engine_columns_isometric(long a1, long a2, long a3, long a4);
void find_gamut(void);
void fiddle_gamut(long a1, long a2);
void create_map_volume_box(long a1, long a2, long a3);
void rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx);
void rotate_base_axis(struct M33 *matx, short a2, unsigned char a3);
void fill_in_points_perspective(long a1, long a2, struct MinMax *mm);
void fill_in_points_cluedo(long a1, long a2, struct MinMax *mm);
void fill_in_points_isometric(long a1, long a2, struct MinMax *mm);
void frame_wibble_generate(void);
void setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8);

void update_engine_settings(struct PlayerInfo *player);
void gpoly_enable_pentium_pro(TbBool state);
void display_drawlist(void);
void draw_view(struct Camera *cam, unsigned char a2);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
