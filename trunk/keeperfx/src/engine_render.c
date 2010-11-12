/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_render.c
 *     Rendering the 3D view functions.
 * @par Purpose:
 *     Functions for displaying drawlist elements on screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_render.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_render.h"

#include "engine_lenses.h"
#include "engine_camera.h"
#include "engine_arrays.h"
#include "kjm_input.h"
#include "front_simple.h"
#include "vidmode.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_draw_fastview_mapwho(struct Camera *cam, struct JontySpr *spr);
DLLIMPORT void _DK_draw_clipped_line(long a1, long a2, long a3, long a4, unsigned char a5);
DLLIMPORT void _DK_draw_engine_number(struct Number *num);
DLLIMPORT void _DK_draw_engine_room_flagpole(struct RoomFlag *rflg);
DLLIMPORT void _DK_draw_status_sprites(long a1, long a2, struct Thing *thing, long a4);
DLLIMPORT void _DK_draw_iso_only_fastview_mapwho(struct Camera *cam, struct JontySpr *spr);
DLLIMPORT void _DK_draw_engine_room_flag_top(struct RoomFlag *rflg);
DLLIMPORT void _DK_draw_stripey_line(long a1, long a2, long a3, long a4, unsigned char a5);
DLLIMPORT void _DK_draw_map_who(struct RotoSpr *spr);
DLLIMPORT void _DK_draw_element(struct Map *map, long a2, long a3, long a4, long a5, long a6, long a7, unsigned char a8, long *a9);
DLLIMPORT long _DK_convert_world_coord_to_front_view_screen_coord(struct Coord3d *pos, struct Camera *cam, long *x, long *y, long *z);
DLLIMPORT void _DK_display_fast_drawlist(struct Camera *cam);
DLLIMPORT void _DK_draw_frontview_engine(struct Camera *cam);
DLLIMPORT void _DK_display_drawlist(void);
DLLIMPORT void _DK_draw_view(struct Camera *cam, unsigned char a2);
DLLIMPORT void _DK_do_a_plane_of_engine_columns_perspective(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_do_a_plane_of_engine_columns_cluedo(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_do_a_plane_of_engine_columns_isometric(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_rotate_base_axis(struct M33 *matx, short a2, unsigned char a3);
DLLIMPORT void _DK_fill_in_points_perspective(long a1, long a2, struct MinMax *mm);
DLLIMPORT void _DK_fill_in_points_cluedo(long a1, long a2, struct MinMax *mm);
DLLIMPORT void _DK_fill_in_points_isometric(long a1, long a2, struct MinMax *mm);
DLLIMPORT void _DK_find_gamut(void);
DLLIMPORT void _DK_fiddle_gamut(long a1, long a2);
DLLIMPORT void _DK_create_map_volume_box(long a1, long a2, long a3);
DLLIMPORT void _DK_frame_wibble_generate(void);
DLLIMPORT void _DK_setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8);
/******************************************************************************/
unsigned short shield_offset[] = {
 0x0,  0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x118, 0x80,
 0x80, 0x100,  0x80,  0x80, 0x100, 0x100, 0x138,  0x80,  0x80, 0x138,  0x80,  0x80, 0x100,  0x80, 0x80, 0x100,
};
long const x_offs[] =  { 0, 1, 1, 0};
long const y_offs[] =  { 0, 0, 1, 1};
long const x_step1[] = { 0,-1, 0, 1};
long const y_step1[] = { 1, 0,-1, 0};
long const x_step2[] = { 1, 0,-1, 0};
long const y_step2[] = { 0, 1, 0,-1};
long const orient_table_xflip[] =  {0, 0, 1, 1};
long const orient_table_yflip[] =  {0, 1, 1, 0};
long const orient_table_rotate[] = {0, 1, 0, 1};
long const orient_to_mapU1[] = { 0, 0x1F0000, 0x1F0000, 0 };
long const orient_to_mapU2[] = { 0x1F0000, 0x1F0000, 0, 0 };
long const orient_to_mapU3[] = { 0x1F0000, 0, 0, 0x1F0000 };
long const orient_to_mapU4[] = { 0, 0, 0x1F0000, 0x1F0000 };
long const orient_to_mapV1[] = { 0, 0, 0x1F0000, 0x1F0000 };
long const orient_to_mapV2[] = { 0, 0x1F0000, 0x1F0000, 0 };
long const orient_to_mapV3[] = {  0x1F0000, 0x1F0000, 0x0, 0x0 };
long const orient_to_mapV4[] = { 0x1F0000, 0x0, 0x0, 0x1F0000 };
unsigned char i_can_see_levels[] = {15, 20, 25, 30,};
//unsigned char temp_cluedo_mode;
unsigned long render_problems;
long render_prob_kind;
/******************************************************************************/
long compute_cells_away(void)
{
  long xmin,ymin,xmax,ymax;
  long xcell,ycell;
  struct PlayerInfo *player;
  long ncells_a;
  player = get_my_player();
    if ((vert_offset[1]) || (hori_offset[1]))
    {
      xcell = 660/pixel_size - player->engine_window_x/pixel_size - x_init_off;
      ycell = (8 * high_offset[1] >> 8) - 20/pixel_size - player->engine_window_y/pixel_size - y_init_off;
      ymax = (((vert_offset[1] * xcell) >> 1) - ((vert_offset[0] * ycell) >> 1))
         / ((hori_offset[0] * vert_offset[1] - vert_offset[0] * hori_offset[1]) >> 11) >> 2;
      xmax = (((hori_offset[1] * xcell) >> 1) - ((hori_offset[0] * ycell) >> 1))
         / ((vert_offset[0] * hori_offset[1] - hori_offset[0] * vert_offset[1]) >> 11) >> 2;
    } else
    {
      ymax = 0;
      xmax = 0;
    }
    if ((vert_offset[1]) || (hori_offset[1]))
    {
      xcell = 320 / pixel_size - player->engine_window_x/pixel_size - x_init_off;
      ycell = 200 / pixel_size - player->engine_window_y/pixel_size - y_init_off;
      ymin = (((vert_offset[1] * xcell) >> 1) - ((vert_offset[0] * ycell) >> 1))
          / ((hori_offset[0] * vert_offset[1] - vert_offset[0] * hori_offset[1]) >> 11) >> 2;
      xmin = (((hori_offset[1] * xcell) >> 1) - ((hori_offset[0] * ycell) >> 1))
         / ((vert_offset[0] * hori_offset[1] - hori_offset[0] * vert_offset[1]) >> 11) >> 2;
    } else
    {
      ymin = 0;
      xmin = 0;
    }
    xcell = abs(ymax - ymin);
    ycell = abs(xmax - xmin);
    if (ycell >= xcell)
      ncells_a = ycell + (xcell >> 1);
    else
      ncells_a = xcell + (ycell >> 1);
    ncells_a += 2;
    if (ncells_a > max_i_can_see)
      ncells_a = max_i_can_see;
    return ncells_a;
}

void init_coords_and_rotation(struct EngineCoord *origin,struct M33 *matx)
{
  origin->x = 0;
  origin->y = 0;
  origin->z = 0;
  matx->r0[0] = 0x4000u;
  matx->r0[1] = 0;
  matx->r0[2] = 0;
  matx->r1[0] = 0;
  matx->r1[1] = 0x4000u;
  matx->r1[2] = 0;
  matx->r2[0] = 0;
  matx->r2[1] = 0;
  matx->r2[2] = 0x4000u;
}

void update_fade_limits(long ncells_a)
{
  fade_max = (ncells_a << 8);
  fade_scaler = (ncells_a << 8);
  fade_way_out = (ncells_a + 1) << 8;
  fade_min = 768 * ncells_a / 4;
  split_1 = (split1at << 8);
  split_2 = (split2at << 8);
}

void update_normal_shade(struct M33 *matx)
{
  normal_shade_left = matx->r2[0];
  normal_shade_right = -matx->r2[0];
  normal_shade_back = -matx->r2[2];
  normal_shade_front = matx->r2[2];
  if (normal_shade_front < 0)
    normal_shade_front = 0;
  if (normal_shade_back < 0)
    normal_shade_back = 0;
  if (normal_shade_left < 0)
    normal_shade_left = 0;
  if (normal_shade_right < 0)
    normal_shade_right = 0;
}

void update_engine_settings(struct PlayerInfo *player)
{
  engine_player_number = player->id_number;
  player_bit = (1 << engine_player_number);
  switch (settings.field_0)
  {
  case 0:
      split1at = 4;
      split2at = 3;
      break;
  case 1:
      split1at = 3;
      split2at = 2;
      break;
  case 2:
      split1at = 2;
      split2at = 1;
      break;
  case 3:
  default:
      split1at = 0;
      split2at = 0;
      break;
  }
  me_pointed_at = NULL;
  me_distance = 100000000;
  max_i_can_see = i_can_see_levels[settings.view_distance % 4];
  if (lens_mode != 0)
    temp_cluedo_mode = 0;
  else
    temp_cluedo_mode = settings.video_cluedo_mode;
  thing_pointed_at = NULL;
}

TbBool is_free_space_in_poly_pool(int nitems)
{
  return (getpoly+(nitems*sizeof(struct BasicUnk13)) <= poly_pool_end);
}

void rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_parallel_3(epos, matx);
}

void rotate_base_axis(struct M33 *matx, short a2, unsigned char a3)
{
  _DK_rotate_base_axis(matx, a2, a3);
}

void fill_in_points_perspective(long a1, long a2, struct MinMax *mm)
{
  _DK_fill_in_points_perspective(a1, a2, mm);
}

void fill_in_points_cluedo(long a1, long a2, struct MinMax *mm)
{
  _DK_fill_in_points_cluedo(a1, a2, mm);
}

void fill_in_points_isometric(long a1, long a2, struct MinMax *mm)
{
  _DK_fill_in_points_isometric(a1, a2, mm);
}

void frame_wibble_generate(void)
{
  _DK_frame_wibble_generate();
}

void setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8)
{
  _DK_setup_rotate_stuff(a1, a2, a3, a4, a5, a6, a7, a8);
}

void do_perspective_rotation(long x, long y, long z)
{
  struct PlayerInfo *player;
  struct EngineCoord epos;

  player = get_my_player();
    epos.x = -x;
    epos.y = 0;
    epos.z = y;
    rotpers_parallel_3(&epos, &camera_matrix);
    x_init_off = epos.field_0;
    y_init_off = epos.field_4;
    depth_init_off = epos.z;
    epos.x = 65536;
    epos.y = 0;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix);
    hori_offset[0] = epos.field_0 - ((player->engine_window_width/pixel_size) >> 1);
    hori_offset[1] = epos.field_4 - ((player->engine_window_height/pixel_size) >> 1);
    hori_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 0;
    epos.z = -65536;
    rotpers_parallel_3(&epos, &camera_matrix);
    vert_offset[0] = epos.field_0 - ((player->engine_window_width/pixel_size) >> 1);
    vert_offset[1] = epos.field_4 - ((player->engine_window_height/pixel_size) >> 1);
    vert_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 65536;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix);
    high_offset[0] = epos.field_0 - ((player->engine_window_width/pixel_size) >> 1);
    high_offset[1] = epos.field_4 - ((player->engine_window_height/pixel_size) >> 1);
    high_offset[2] = epos.z;
}

void find_gamut(void)
{
  SYNCDBG(19,"Starting");
  _DK_find_gamut();
}

void fiddle_gamut(long a1, long a2)
{
  _DK_fiddle_gamut(a1, a2);
}

void create_map_volume_box(long x, long y, long z)
{
  _DK_create_map_volume_box(x, y, z);
}

void do_a_plane_of_engine_columns_perspective(long a1, long a2, long a3, long a4)
{
  _DK_do_a_plane_of_engine_columns_perspective(a1, a2, a3, a4);
}

void do_a_plane_of_engine_columns_cluedo(long a1, long a2, long a3, long a4)
{
  _DK_do_a_plane_of_engine_columns_cluedo(a1, a2, a3, a4);
}

void do_a_plane_of_engine_columns_isometric(long xcell, long ycell, long a3, long a4)
{
  //TODO: This is where first and last columns are cut off from rendering
  _DK_do_a_plane_of_engine_columns_isometric(xcell, ycell, a3, a4);
}

void create_box_coords(struct EngineCoord *coord, long x, long z, long y)
{
  coord->x = x;
  coord->z = z;
  coord->field_8 = 0;
  coord->y = y;
  rotpers(coord, &camera_matrix);
}

void draw_map_volume_box(long a1, long a2, long a3, long a4, long a5, unsigned char color)
{
  map_volume_box.visible = 1;
  map_volume_box.field_3 = a1 & 0xFFFF0000;
  map_volume_box.field_7 = a2 & 0xFF00;
  map_volume_box.field_B = a3 & 0xFFFF0000;
  map_volume_box.field_13 = a5;
  map_volume_box.field_F = a4 & 0xFFFF0000;
  map_volume_box.color = color;
}

void draw_fastview_mapwho(struct Camera *cam, struct JontySpr *spr)
{
    _DK_draw_fastview_mapwho(cam, spr);
}

void draw_engine_number(struct Number *num)
{
    _DK_draw_engine_number(num);
}

void draw_engine_room_flagpole(struct RoomFlag *rflg)
{
    _DK_draw_engine_room_flagpole(rflg);
}

void draw_status_sprites(long a1, long a2, struct Thing *thing, long a4)
{
    _DK_draw_status_sprites(a1, a2, thing, a4);
}

void draw_iso_only_fastview_mapwho(struct Camera *cam, struct JontySpr *spr)
{
    _DK_draw_iso_only_fastview_mapwho(cam, spr);
}

void draw_engine_room_flag_top(struct RoomFlag *rflg)
{
    _DK_draw_engine_room_flag_top(rflg);
}

void draw_stripey_line(long a1, long a2, long a3, long a4, unsigned char a5)
{
    _DK_draw_stripey_line(a1, a2, a3, a4, a5);
}

void draw_clipped_line(long x1, long y1, long x2, long y2, TbPixel color)
{
    struct PlayerInfo *player;
    if ((x1 >= 0) || (x2 >= 0))
    {
      if ((y1 >= 0) || (y2 >= 0))
      {
        player = get_my_player();
        if ((x1 < player->engine_window_width) || (x2 < player->engine_window_width))
        {
          if ((y1 < player->engine_window_width) || (y2 < player->engine_window_width))
          {
            draw_stripey_line(x1, y1, x2, y2, color);
          }
        }
      }
    }
}

void draw_map_who(struct RotoSpr *spr)
{
    _DK_draw_map_who(spr);
}

void draw_unkn09(struct BasicUnk09 *unk09)
{
    struct XYZ coord_a,coord_b,coord_c,coord_d,coord_e;
    struct PolyPoint point_a,point_b,point_c,point_d,point_e,
        point_f,point_g,point_h,point_i,point_j,point_k,point_l;
    vec_map = block_ptrs[unk09->block];
    switch (unk09->subtype)
    {
    case 0:
        vec_mode = 5;
        draw_gpoly(&unk09->p1,&unk09->p2,&unk09->p3);
        break;
    case 1:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&unk09->p1, &point_a, &unk09->p3);
        draw_gpoly(&point_a, &unk09->p2, &unk09->p3);
        break;
    case 2:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_a.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&unk09->p1, &unk09->p2, &point_a);
        draw_gpoly(&unk09->p1, &point_a, &unk09->p3);
        break;
    case 3:
        vec_mode = 5;
        coord_a.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&unk09->p1, &unk09->p2, &point_a);
        draw_gpoly(&point_a, &unk09->p2, &unk09->p3);
        break;
    case 4:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&unk09->p1, &point_a, &point_c);
        draw_gpoly(&point_a, &unk09->p2, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_c, &point_b, &unk09->p3);
        break;
    case 5:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_44) >> 1;
        coord_b.y = (coord_a.y + unk09->field_48) >> 1;
        coord_b.z = (coord_a.z + unk09->field_4C) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        point_b.field_10 = (point_a.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_50) >> 1;
        coord_c.y = (coord_a.y + unk09->field_54) >> 1;
        coord_c.z = (coord_a.z + unk09->field_58) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        point_c.field_10 = (point_a.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&unk09->p1, &point_b, &unk09->p3);
        draw_gpoly(&point_b, &point_a, &unk09->p3);
        draw_gpoly(&point_a, &point_c, &unk09->p3);
        draw_gpoly(&point_c, &unk09->p2, &unk09->p3);
        break;
    case 6:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_a.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_50) >> 1;
        coord_b.y = (coord_a.y + unk09->field_54) >> 1;
        coord_b.z = (coord_a.z + unk09->field_58) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (point_a.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_5C) >> 1;
        coord_c.y = (coord_a.y + unk09->field_60) >> 1;
        coord_c.z = (coord_a.z + unk09->field_64) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p3.field_C) >> 1;
        point_c.field_10 = (point_a.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&unk09->p1, &unk09->p2, &point_b);
        draw_gpoly(&unk09->p1, &point_b, &point_a);
        draw_gpoly(&unk09->p1, &point_a, &point_c);
        draw_gpoly(&unk09->p1, &point_c, &unk09->p3);
        break;
    case 7:
        vec_mode = 5;
        coord_a.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_5C) >> 1;
        coord_b.y = (coord_a.y + unk09->field_60) >> 1;
        coord_b.z = (coord_a.z + unk09->field_64) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p3.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p3.field_C) >> 1;
        point_b.field_10 = (point_a.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_44) >> 1;
        coord_c.y = (coord_a.y + unk09->field_48) >> 1;
        coord_c.z = (coord_a.z + unk09->field_4C) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (point_a.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&unk09->p2, &unk09->p3, &point_b);
        draw_gpoly(&unk09->p2, &point_b, &point_a);
        draw_gpoly(&unk09->p2, &point_a, &point_c);
        draw_gpoly(&unk09->p2, &point_c, &unk09->p1);
        break;
    case 8:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + unk09->field_44) >> 1;
        coord_d.y = (coord_a.y + unk09->field_48) >> 1;
        coord_d.z = (coord_a.z + unk09->field_4C) >> 1;
        point_d.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_d.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        point_d.field_10 = (point_a.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + unk09->field_50) >> 1;
        coord_e.y = (coord_a.y + unk09->field_54) >> 1;
        coord_e.z = (coord_a.z + unk09->field_58) >> 1;
        point_e.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_e.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        point_e.field_10 = (point_a.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&unk09->p1, &point_d, &point_c);
        draw_gpoly(&point_d, &point_a, &point_c);
        draw_gpoly(&point_a, &point_e, &point_b);
        draw_gpoly(&point_e, &unk09->p2, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_c, &point_b, &unk09->p3);
        break;
    case 9:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_b.x + unk09->field_50) >> 1;
        coord_d.y = (coord_b.y + unk09->field_54) >> 1;
        coord_d.z = (coord_b.z + unk09->field_58) >> 1;
        point_d.field_8 = (point_b.field_8 + unk09->p2.field_8) >> 1;
        point_d.field_C = (point_b.field_C + unk09->p2.field_C) >> 1;
        point_d.field_10 = (point_b.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_b.x + unk09->field_5C) >> 1;
        coord_e.y = (coord_b.y + unk09->field_60) >> 1;
        coord_e.z = (coord_b.z + unk09->field_64) >> 1;
        point_e.field_8 = (point_b.field_8 + unk09->p3.field_8) >> 1;
        point_e.field_C = (point_b.field_C + unk09->p3.field_C) >> 1;
        point_e.field_10 = (point_b.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&unk09->p1, &point_a, &point_c);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_a, &unk09->p2, &point_d);
        draw_gpoly(&point_a, &point_d, &point_b);
        draw_gpoly(&point_c, &point_b, &point_e);
        draw_gpoly(&point_c, &point_e, &unk09->p3);
        break;
    case 10:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_c.x + unk09->field_5C) >> 1;
        coord_d.y = (coord_c.y + unk09->field_60) >> 1;
        coord_d.z = (coord_c.z + unk09->field_64) >> 1;
        point_d.field_8 = (point_c.field_8 + unk09->p3.field_8) >> 1;
        point_d.field_C = (point_c.field_C + unk09->p3.field_C) >> 1;
        point_d.field_10 = (point_c.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_c.x + unk09->field_44) >> 1;
        coord_e.y = (coord_c.y + unk09->field_48) >> 1;
        coord_e.z = (coord_c.z + unk09->field_4C) >> 1;
        point_e.field_8 = (point_c.field_8 + unk09->p1.field_8) >> 1;
        point_e.field_C = (point_c.field_C + unk09->p1.field_C) >> 1;
        point_e.field_10 = (point_c.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&point_a, &unk09->p2, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&unk09->p1, &point_a, &point_e);
        draw_gpoly(&point_e, &point_a, &point_c);
        draw_gpoly(&point_c, &point_b, &point_d);
        draw_gpoly(&point_d, &point_b, &unk09->p3);
        break;
    case 11:
        vec_mode = 5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + unk09->field_44) >> 1;
        coord_d.y = (coord_a.y + unk09->field_48) >> 1;
        coord_d.z = (coord_a.z + unk09->field_4C) >> 1;
        point_d.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_d.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        point_d.field_10 = (point_a.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + unk09->field_50) >> 1;
        coord_e.y = (coord_a.y + unk09->field_54) >> 1;
        coord_e.z = (coord_a.z + unk09->field_58) >> 1;
        point_e.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_e.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        point_e.field_10 = (point_a.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_e, &point_e);
        coord_d.x = (coord_b.x + unk09->field_50) >> 1;
        coord_d.y = (coord_b.y + unk09->field_54) >> 1;
        coord_d.z = (coord_b.z + unk09->field_58) >> 1;
        point_f.field_8 = (point_b.field_8 + unk09->p2.field_8) >> 1;
        point_f.field_C = (point_b.field_C + unk09->p2.field_C) >> 1;
        point_f.field_10 = (point_b.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_d, &point_f);
        coord_e.x = (coord_b.x + unk09->field_5C) >> 1;
        coord_e.y = (coord_b.y + unk09->field_60) >> 1;
        coord_e.z = (coord_b.z + unk09->field_64) >> 1;
        point_g.field_8 = (point_b.field_8 + unk09->p3.field_8) >> 1;
        point_g.field_C = (point_b.field_C + unk09->p3.field_C) >> 1;
        point_g.field_10 = (point_b.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_e, &point_g);
        coord_d.x = (coord_c.x + unk09->field_5C) >> 1;
        coord_d.y = (coord_c.y + unk09->field_60) >> 1;
        coord_d.z = (coord_c.z + unk09->field_64) >> 1;
        point_h.field_8 = (point_c.field_8 + unk09->p3.field_8) >> 1;
        point_h.field_C = (point_c.field_C + unk09->p3.field_C) >> 1;
        point_h.field_10 = (point_c.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_d, &point_h);
        coord_e.x = (coord_c.x + unk09->field_44) >> 1;
        coord_e.y = (coord_c.y + unk09->field_48) >> 1;
        coord_e.z = (coord_c.z + unk09->field_4C) >> 1;
        point_i.field_8 = (point_c.field_8 + unk09->p1.field_8) >> 1;
        point_i.field_C = (point_c.field_C + unk09->p1.field_C) >> 1;
        point_i.field_10 = (point_c.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_e, &point_i);
        coord_d.x = (coord_a.x + coord_c.x) >> 1;
        coord_d.y = (coord_a.y + coord_c.y) >> 1;
        coord_d.z = (coord_a.z + coord_c.z) >> 1;
        point_j.field_8 = (point_a.field_8 + point_c.field_8) >> 1;
        point_j.field_C = (point_a.field_C + point_c.field_C) >> 1;
        point_j.field_10 = (point_a.field_10 + point_c.field_10) >> 1;
        perspective(&coord_d, &point_j);
        coord_e.x = (coord_a.x + coord_b.x) >> 1;
        coord_e.y = (coord_a.y + coord_b.y) >> 1;
        coord_e.z = (coord_a.z + coord_b.z) >> 1;
        point_k.field_8 = (point_a.field_8 + point_b.field_8) >> 1;
        point_k.field_C = (point_a.field_C + point_b.field_C) >> 1;
        point_k.field_10 = (point_a.field_10 + point_b.field_10) >> 1;
        perspective(&coord_e, &point_k);
        coord_d.x = (coord_b.x + coord_c.x) >> 1;
        coord_d.y = (coord_b.y + coord_c.y) >> 1;
        coord_d.z = (coord_b.z + coord_c.z) >> 1;
        point_l.field_8 = (point_b.field_8 + point_c.field_8) >> 1;
        point_l.field_C = (point_b.field_C + point_c.field_C) >> 1;
        point_l.field_10 = (point_b.field_10 + point_c.field_10) >> 1;
        perspective(&coord_d, &point_l);
        draw_gpoly(&unk09->p1, &point_d, &point_i);
        draw_gpoly(&point_d, &point_a, &point_j);
        draw_gpoly(&point_a, &point_e, &point_k);
        draw_gpoly(&point_e, &unk09->p2, &point_f);
        draw_gpoly(&point_d, &point_j, &point_i);
        draw_gpoly(&point_a, &point_k, &point_j);
        draw_gpoly(&point_e, &point_f, &point_k);
        draw_gpoly(&point_i, &point_j, &point_c);
        draw_gpoly(&point_j, &point_k, &point_l);
        draw_gpoly(&point_k, &point_f, &point_b);
        draw_gpoly(&point_j, &point_l, &point_c);
        draw_gpoly(&point_k, &point_b, &point_l);
        draw_gpoly(&point_c, &point_l, &point_h);
        draw_gpoly(&point_l, &point_b, &point_g);
        draw_gpoly(&point_l, &point_g, &point_h);
        draw_gpoly(&point_h, &point_g, &unk09->p3);
        break;
    case 12:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        trig(&unk09->p1, &unk09->p2, &unk09->p3);
        break;
    case 13:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        trig(&unk09->p1, &point_a, &unk09->p3);
        trig(&point_a, &unk09->p2, &unk09->p3);
        break;
    case 14:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_a, &point_a);
        trig(&unk09->p1, &unk09->p2, &point_a);
        trig(&unk09->p1, &point_a, &unk09->p3);
        break;
    case 15:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        trig(&unk09->p1, &unk09->p2, &point_a);
        trig(&point_a, &unk09->p2, &unk09->p3);
        break;
    case 16:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        trig(&unk09->p1, &point_a, &point_c);
        trig(&point_a, &unk09->p2, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&point_c, &point_b, &unk09->p3);
        break;
    case 17:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_44) >> 1;
        coord_b.y = (coord_a.y + unk09->field_48) >> 1;
        coord_b.z = (coord_a.z + unk09->field_4C) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_50) >> 1;
        coord_c.y = (coord_a.y + unk09->field_54) >> 1;
        coord_c.z = (coord_a.z + unk09->field_58) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_c, &point_c);
        trig(&unk09->p1, &point_b, &unk09->p3);
        trig(&point_b, &point_a, &unk09->p3);
        trig(&point_a, &point_c, &unk09->p3);
        trig(&point_c, &unk09->p2, &unk09->p3);
        break;
    case 18:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_50) >> 1;
        coord_b.y = (coord_a.y + unk09->field_54) >> 1;
        coord_b.z = (coord_a.z + unk09->field_58) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_5C) >> 1;
        coord_c.y = (coord_a.y + unk09->field_60) >> 1;
        coord_c.z = (coord_a.z + unk09->field_64) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_c, &point_c);
        trig(&unk09->p1, &unk09->p2, &point_b);
        trig(&unk09->p1, &point_b, &point_a);
        trig(&unk09->p1, &point_a, &point_c);
        trig(&unk09->p1, &point_c, &unk09->p3);
        break;
    case 19:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_5C) >> 1;
        coord_b.y = (coord_a.y + unk09->field_60) >> 1;
        coord_b.z = (coord_a.z + unk09->field_64) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p3.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_44) >> 1;
        coord_c.y = (coord_a.y + unk09->field_48) >> 1;
        coord_c.z = (coord_a.z + unk09->field_4C) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        trig(&unk09->p2, &unk09->p3, &point_b);
        trig(&unk09->p2, &point_b, &point_a);
        trig(&unk09->p2, &point_a, &point_c);
        trig(&unk09->p2, &point_c, &unk09->p1);
        break;
    case 20:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + unk09->field_44) >> 1;
        coord_d.y = (coord_a.y + unk09->field_48) >> 1;
        coord_d.z = (coord_a.z + unk09->field_4C) >> 1;
        point_d.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_d.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + unk09->field_50) >> 1;
        coord_e.y = (coord_a.y + unk09->field_54) >> 1;
        coord_e.z = (coord_a.z + unk09->field_58) >> 1;
        point_e.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_e.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_e, &point_e);
        trig(&unk09->p1, &point_d, &point_c);
        trig(&point_d, &point_a, &point_c);
        trig(&point_a, &point_e, &point_b);
        trig(&point_e, &unk09->p2, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&point_c, &point_b, &unk09->p3);
        break;
    case 21:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_b.x + unk09->field_50) >> 1;
        coord_d.y = (coord_b.y + unk09->field_54) >> 1;
        coord_d.z = (coord_b.z + unk09->field_58) >> 1;
        point_d.field_8 = (point_b.field_8 + unk09->p2.field_8) >> 1;
        point_d.field_C = (point_b.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_b.x + unk09->field_5C) >> 1;
        coord_e.y = (coord_b.y + unk09->field_60) >> 1;
        coord_e.z = (coord_b.z + unk09->field_64) >> 1;
        point_e.field_8 = (point_b.field_8 + unk09->p3.field_8) >> 1;
        point_e.field_C = (point_b.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_e, &point_e);
        trig(&unk09->p1, &point_a, &point_c);
        trig(&point_a, &point_b, &point_c);
        trig(&point_a, &unk09->p2, &point_d);
        trig(&point_a, &point_d, &point_b);
        trig(&point_c, &point_b, &point_e);
        trig(&point_c, &point_e, &unk09->p3);
        break;
    case 22:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_c.x + unk09->field_5C) >> 1;
        coord_d.y = (coord_c.y + unk09->field_60) >> 1;
        coord_d.z = (coord_c.z + unk09->field_64) >> 1;
        point_d.field_8 = (point_c.field_8 + unk09->p3.field_8) >> 1;
        point_d.field_C = (point_c.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_c.x + unk09->field_44) >> 1;
        coord_e.y = (coord_c.y + unk09->field_48) >> 1;
        coord_e.z = (coord_c.z + unk09->field_4C) >> 1;
        point_e.field_8 = (point_c.field_8 + unk09->p1.field_8) >> 1;
        point_e.field_C = (point_c.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_e, &point_e);
        trig(&point_a, &unk09->p2, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&unk09->p1, &point_a, &point_e);
        trig(&point_e, &point_a, &point_c);
        trig(&point_c, &point_b, &point_d);
        trig(&point_d, &point_b, &unk09->p3);
        break;
    case 23:
        vec_mode = 7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + unk09->field_44) >> 1;
        coord_d.y = (coord_a.y + unk09->field_48) >> 1;
        coord_d.z = (coord_a.z + unk09->field_4C) >> 1;
        point_d.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_d.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + unk09->field_50) >> 1;
        coord_e.y = (coord_a.y + unk09->field_54) >> 1;
        coord_e.z = (coord_a.z + unk09->field_58) >> 1;
        point_e.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_e.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_e, &point_e);
        coord_d.x = (coord_b.x + unk09->field_50) >> 1;
        coord_d.y = (coord_b.y + unk09->field_54) >> 1;
        coord_d.z = (coord_b.z + unk09->field_58) >> 1;
        point_f.field_8 = (point_b.field_8 + unk09->p2.field_8) >> 1;
        point_f.field_C = (point_b.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_d, &point_f);
        coord_e.x = (coord_b.x + unk09->field_5C) >> 1;
        coord_e.y = (coord_b.y + unk09->field_60) >> 1;
        coord_e.z = (coord_b.z + unk09->field_64) >> 1;
        point_g.field_8 = (point_b.field_8 + unk09->p3.field_8) >> 1;
        point_g.field_C = (point_b.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_e, &point_g);
        coord_d.x = (coord_c.x + unk09->field_5C) >> 1;
        coord_d.y = (coord_c.y + unk09->field_60) >> 1;
        coord_d.z = (coord_c.z + unk09->field_64) >> 1;
        point_h.field_8 = (point_c.field_8 + unk09->p3.field_8) >> 1;
        point_h.field_C = (point_c.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_d, &point_h);
        coord_e.x = (coord_c.x + unk09->field_44) >> 1;
        coord_e.y = (coord_c.y + unk09->field_48) >> 1;
        coord_e.z = (coord_c.z + unk09->field_4C) >> 1;
        point_i.field_8 = (point_c.field_8 + unk09->p1.field_8) >> 1;
        point_i.field_C = (point_c.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_e, &point_i);
        coord_d.x = (coord_a.x + coord_c.x) >> 1;
        coord_d.y = (coord_a.y + coord_c.y) >> 1;
        coord_d.z = (coord_a.z + coord_c.z) >> 1;
        point_j.field_8 = (point_a.field_8 + point_c.field_8) >> 1;
        point_j.field_C = (point_a.field_C + point_c.field_C) >> 1;
        perspective(&coord_d, &point_j);
        coord_e.x = (coord_a.x + coord_b.x) >> 1;
        coord_e.y = (coord_a.y + coord_b.y) >> 1;
        coord_e.z = (coord_a.z + coord_b.z) >> 1;
        point_k.field_8 = (point_a.field_8 + point_b.field_8) >> 1;
        point_k.field_C = (point_a.field_C + point_b.field_C) >> 1;
        perspective(&coord_e, &point_k);
        coord_d.x = (coord_b.x + coord_c.x) >> 1;
        coord_d.y = (coord_b.y + coord_c.y) >> 1;
        coord_d.z = (coord_b.z + coord_c.z) >> 1;
        point_l.field_8 = (point_b.field_8 + point_c.field_8) >> 1;
        point_l.field_C = (point_b.field_C + point_c.field_C) >> 1;
        perspective(&coord_d, &point_l);
        trig(&unk09->p1, &point_d, &point_i);
        trig(&point_d, &point_a, &point_j);
        trig(&point_a, &point_e, &point_k);
        trig(&point_e, &unk09->p2, &point_f);
        trig(&point_d, &point_j, &point_i);
        trig(&point_a, &point_k, &point_j);
        trig(&point_e, &point_f, &point_k);
        trig(&point_i, &point_j, &point_c);
        trig(&point_j, &point_k, &point_l);
        trig(&point_k, &point_f, &point_b);
        trig(&point_j, &point_l, &point_c);
        trig(&point_k, &point_b, &point_l);
        trig(&point_c, &point_l, &point_h);
        trig(&point_l, &point_b, &point_g);
        trig(&point_l, &point_g, &point_h);
        trig(&point_h, &point_g, &unk09->p3);
        break;
    default:
        render_problems++;
        render_prob_kind = unk09->b.kind;
        break;
    }
}
void display_drawlist(void)
{
    struct PlayerInfo *player;
    struct Camera *cam;
    union {
      struct BasicQ *b;
      struct BasicUnk00 *unk00;
      struct BasicUnk01 *unk01;
      struct BasicUnk02 *unk02;
      struct BasicUnk03 *unk03;
      struct BasicUnk04 *unk04;
      struct BasicUnk05 *unk05;
      struct BasicUnk06 *unk06;
      struct BasicUnk07 *unk07;
      struct RotoSpr *rotSpr;
      struct BasicUnk09 *unk09;
      struct BasicUnk10 *unk10;
      struct JontySpr *jonSpr;
      struct BasicUnk12 *unk12;
      struct BasicUnk13 *unk13;
      struct BasicUnk14 *unk14;
      struct TexturedQuad *txquad;
      struct Number *number;
      struct RoomFlag *roomFlg;
    } item;
    long bucket_num;
    struct PolyPoint point_a,point_b,point_c;
    SYNCDBG(9,"Starting");
    //_DK_display_drawlist(); return;
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_problems = 0;
    thing_pointed_at = 0;
    for (bucket_num = BUCKETS_COUNT-1; bucket_num >= 0; bucket_num--)
    {
      for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
      {
        //JUSTLOG("%d",(int)item.b->kind);
        switch ( item.b->kind )
        {
        case 0:
          vec_mode = 5;
          vec_map = block_ptrs[item.unk00->block];
          draw_gpoly(&item.unk00->p1, &item.unk00->p2, &item.unk00->p3);
          break;
        case 1:
          vec_mode = 7;
          vec_colour = ((item.unk01->p3.field_10 + item.unk01->p2.field_10 + item.unk01->p1.field_10)/3) >> 16;
          vec_map = block_ptrs[item.unk01->block];
          trig(&item.unk01->p1, &item.unk01->p2, &item.unk01->p3);
          break;
        case 2:
          vec_mode = 0;
          vec_colour = item.unk02->colour;
          point_a.field_0 = item.unk02->x1;
          point_a.field_4 = item.unk02->y1;
          point_b.field_0 = item.unk02->x2;
          point_b.field_4 = item.unk02->y2;
          point_c.field_0 = item.unk02->x3;
          point_c.field_4 = item.unk02->y3;
          draw_gpoly(&point_a, &point_b, &point_c);
          break;
        case 3:
          vec_mode = 4;
          vec_colour = item.unk03->colour;
          point_a.field_0 = item.unk03->x1;
          point_a.field_4 = item.unk03->y1;
          point_b.field_0 = item.unk03->x2;
          point_b.field_4 = item.unk03->y2;
          point_c.field_0 = item.unk03->x3;
          point_c.field_4 = item.unk03->y3;
          point_a.field_10 = item.unk03->vf1 << 16;
          point_b.field_10 = item.unk03->vf2 << 16;
          point_c.field_10 = item.unk03->vf3 << 16;
          draw_gpoly(&point_a, &point_b, &point_c);
          break;
        case 4:
          vec_mode = 2;
          point_a.field_0 = item.unk04->x1;
          point_a.field_4 = item.unk04->y1;
          point_b.field_0 = item.unk04->x2;
          point_b.field_4 = item.unk04->y2;
          point_c.field_0 = item.unk04->x3;
          point_c.field_4 = item.unk04->y3;
          point_a.field_8 = item.unk04->uf1 << 16;
          point_a.field_C = item.unk04->vf1 << 16;
          point_b.field_8 = item.unk04->uf2 << 16;
          point_b.field_C = item.unk04->vf2 << 16;
          point_c.field_8 = item.unk04->uf3 << 16;
          point_c.field_C = item.unk04->vf3 << 16;
          trig(&point_a, &point_b, &point_c);
          break;
        case 5:
          vec_mode = 5;
          point_a.field_0 = item.unk05->x1;
          point_a.field_4 = item.unk05->y1;
          point_b.field_0 = item.unk05->x2;
          point_b.field_4 = item.unk05->y2;
          point_c.field_0 = item.unk05->x3;
          point_c.field_4 = item.unk05->y3;
          point_a.field_8 = item.unk05->uf1 << 16;
          point_a.field_C = item.unk05->vf1 << 16;
          point_b.field_8 = item.unk05->uf2 << 16;
          point_b.field_C = item.unk05->vf2 << 16;
          point_c.field_8 = item.unk05->uf3 << 16;
          point_c.field_C = item.unk05->vf3 << 16;
          point_a.field_10 = item.unk05->wf1 << 16;
          point_b.field_10 = item.unk05->wf2 << 16;
          point_c.field_10 = item.unk05->wf3 << 16;
          draw_gpoly(&point_a, &point_b, &point_c);
          break;
        case 6:
          vec_mode = 3;
          point_a.field_0 = item.unk06->x1;
          point_a.field_4 = item.unk06->y1;
          point_b.field_0 = item.unk06->x2;
          point_b.field_4 = item.unk06->y2;
          point_c.field_0 = item.unk06->x3;
          point_c.field_4 = item.unk06->y3;
          point_a.field_8 = item.unk06->uf1 << 16;
          point_a.field_C = item.unk06->vf1 << 16;
          point_b.field_8 = item.unk06->uf2 << 16;
          point_b.field_C = item.unk06->vf2 << 16;
          point_c.field_8 = item.unk06->uf3 << 16;
          point_c.field_C = item.unk06->vf3 << 16;
          trig(&point_a, &point_b, &point_c);
          break;
        case 7:
          vec_mode = 6;
          point_a.field_0 = item.unk07->x1;
          point_a.field_4 = item.unk07->y1;
          point_b.field_0 = item.unk07->x2;
          point_b.field_4 = item.unk07->y2;
          point_c.field_0 = item.unk07->x3;
          point_c.field_4 = item.unk07->y3;
          point_a.field_8 = item.unk07->uf1 << 16;
          point_a.field_C = item.unk07->vf1 << 16;
          point_b.field_8 = item.unk07->uf2 << 16;
          point_b.field_C = item.unk07->vf2 << 16;
          point_c.field_8 = item.unk07->uf3 << 16;
          point_c.field_C = item.unk07->vf3 << 16;
          point_a.field_10 = item.unk07->wf1 << 16;
          point_b.field_10 = item.unk07->wf2 << 16;
          point_c.field_10 = item.unk07->wf3 << 16;
          trig(&point_a, &point_b, &point_c);
          break;
        case 8:
          draw_map_who(item.rotSpr);
          break;
        case 9:
          draw_unkn09(item.unk09);
          break;
        case 10:
          vec_mode = 0;
          vec_colour = item.unk10->field_6;
          draw_gpoly(&item.unk10->p1, &item.unk10->p2, &item.unk10->p3);
          break;
        case 11:
          draw_jonty_mapwho(item.jonSpr);
          break;
        case 12:
          draw_keepsprite_unscaled_in_buffer(item.unk12->field_5C, item.unk12->field_58, item.unk12->field_5E, scratch);
          vec_map = scratch;
          vec_mode = 10;
          vec_colour = item.unk12->p1.field_10;
          trig(&item.unk12->p1, &item.unk12->p2, &item.unk12->p3);
          trig(&item.unk12->p1, &item.unk12->p3, &item.unk12->p4);
          break;
        case 13:
          draw_clipped_line(item.unk13->p.field_0,item.unk13->p.field_4,item.unk13->p.field_8,item.unk13->p.field_C,item.unk13->p.field_10);
          break;
        case 14:
          player = get_my_player();
          cam = player->acamera;
          if (cam != NULL)
          {
            if ((cam->field_6 == 2) || (cam->field_6 == 5))
              draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, cam->field_17/pixel_size);
          }
          break;
        case 16:
          draw_engine_number(item.number);
          break;
        case 17:
          draw_engine_room_flagpole(item.roomFlg);
          break;
        case 18:
          player = get_my_player();
          cam = player->acamera;
          if (cam != NULL)
          {
            if (cam->field_6 == 2)
              draw_jonty_mapwho(item.jonSpr);
          }
          break;
        case 19:
          draw_engine_room_flag_top(item.roomFlg);
          break;
        default:
          render_problems++;
          render_prob_kind = item.b->kind;
          break;
        }
      }
    }
    if (render_problems > 0)
      WARNLOG("Encoured %lu rendering problems; last was with poly kind %ld",render_problems,render_prob_kind);
}

void draw_view(struct Camera *cam, unsigned char a2)
{
    long nlens;
    long x,y,z;
    long xcell,ycell;
    long i;
    long aposc,bposc;
    struct EngineCol *ec;
    struct MinMax *mm;
    SYNCDBG(9,"Starting");
    nlens = cam->field_17 / pixel_size;
    getpoly = poly_pool;
    LbMemorySet(buckets, 0, sizeof(buckets));
    LbMemorySet(poly_pool, 0, sizeof(poly_pool));
    i = lens_mode;
    if ((i < 0) || (i >= PERS_ROUTINES_COUNT))
        i = 0;
    perspective = perspective_routines[i];
    rotpers = rotpers_routines[i];
    update_fade_limits(cells_away);
    init_coords_and_rotation(&object_origin,&camera_matrix);
    rotate_base_axis(&camera_matrix, cam->orient_a, 2);
    update_normal_shade(&camera_matrix);
    rotate_base_axis(&camera_matrix, -cam->orient_b, 1);
    rotate_base_axis(&camera_matrix, -cam->orient_c, 3);
    map_angle = cam->orient_a;
    map_roll = cam->orient_c;
    map_tilt = -cam->orient_b;
    x = cam->mappos.x.val;
    y = cam->mappos.y.val;
    z = cam->mappos.z.val;
    frame_wibble_generate();
    view_alt = z;
    if (lens_mode != 0)
    {
      cells_away = max_i_can_see;
      update_fade_limits(cells_away);
      fade_range = (fade_max - fade_min) >> 8;
      setup_rotate_stuff(x, y, z, fade_max, fade_min, lens, map_angle, map_roll);
    } else
    {
      fade_min = 1000000;
      setup_rotate_stuff(x, y, z, fade_max, fade_min, nlens, map_angle, map_roll);
      do_perspective_rotation(x, y, z);
      cells_away = compute_cells_away();
    }
    xcell = (x >> 8);
    aposc = -(x & 0xFF);
    bposc = (cells_away << 8) + (y & 0xFF);
    ycell = (y >> 8) - (cells_away+1);
    find_gamut();
    fiddle_gamut(xcell, ycell + (cells_away+1));
    apos = aposc;
    bpos = bposc;
    back_ec = &ecs1[0];
    front_ec = &ecs2[0];
    i = 31-cells_away;
    if (i < 0)
        i = 0;
    mm = &minmaxs[i];
    if (lens_mode != 0)
    {
      fill_in_points_perspective(xcell, ycell, mm);
    } else
    if (settings.video_cluedo_mode)
    {
      fill_in_points_cluedo(xcell, ycell, mm);
    } else
    {
      fill_in_points_isometric(xcell, ycell, mm);
    }
    for (i = 2*cells_away-1; i > 0; i--)
    {
        ycell++;
        bposc -= 256;
        mm++;
        ec = front_ec;
        front_ec = back_ec;
        back_ec = ec;
        apos = aposc;
        bpos = bposc;
        if (lens_mode != 0)
        {
          fill_in_points_perspective(xcell, ycell, mm);
          if (mm->min < mm->max)
          {
            apos = aposc;
            bpos = bposc;
            do_a_plane_of_engine_columns_perspective(xcell, ycell, mm->min, mm->max);
          }
        } else
        if ( settings.video_cluedo_mode )
        {
          fill_in_points_cluedo(xcell, ycell, mm);
          if (mm->min < mm->max)
          {
            apos = aposc;
            bpos = bposc;
            do_a_plane_of_engine_columns_cluedo(xcell, ycell, mm->min, mm->max);
          }
        } else
        {
          fill_in_points_isometric(xcell, ycell, mm);
          if (mm->min < mm->max)
          {
            apos = aposc;
            bpos = bposc;
            do_a_plane_of_engine_columns_isometric(xcell, ycell, mm->min, mm->max);
          }
        }
    }
    if (map_volume_box.visible)
      create_map_volume_box(x, y, z);
    display_drawlist();
    map_volume_box.visible = 0;
    SYNCDBG(9,"Finished");
}

void clear_fast_bucket_list(void)
{
    getpoly = poly_pool;
    LbMemorySet(buckets, 0, sizeof(buckets));
}

void draw_texturedquad_block(struct TexturedQuad *txquad)
{
    if (!UseFastBlockDraw)
    {
        struct PolyPoint point_a;
        struct PolyPoint point_b;
        struct PolyPoint point_c;
        struct PolyPoint point_d;
        vec_mode = 5;
        switch (txquad->field_2A)
        {
        case 0:
            vec_map = block_ptrs[578];
            break;
        case 1:
            vec_map = block_ptrs[579];
            break;
        case 3:
        default:
            vec_map = block_ptrs[txquad->field_6];
            break;
        }
        point_a.field_0 = (txquad->field_A >> 8) / pixel_size;
        point_a.field_4 = (txquad->field_E >> 8) / pixel_size;
        point_a.field_8 = orient_to_mapU1[txquad->field_5];
        point_a.field_C = orient_to_mapV1[txquad->field_5];
        point_a.field_10 = txquad->field_1A;
        point_d.field_0 = ((txquad->field_12 + txquad->field_A) >> 8) / pixel_size;
        point_d.field_4 = (txquad->field_E >> 8) / pixel_size;
        point_d.field_8 = orient_to_mapU2[txquad->field_5];
        point_d.field_C = orient_to_mapV2[txquad->field_5];
        point_d.field_10 = txquad->field_1E;
        point_b.field_0 = ((txquad->field_12 + txquad->field_A) >> 8) / pixel_size;
        point_b.field_4 = ((txquad->field_16 + txquad->field_E) >> 8) / pixel_size;
        point_b.field_8 = orient_to_mapU3[txquad->field_5];
        point_b.field_C = orient_to_mapV3[txquad->field_5];
        point_b.field_10 = txquad->field_22;
        point_c.field_0 = (txquad->field_A >> 8) / pixel_size;
        point_c.field_4 = ((txquad->field_16 + txquad->field_E) >> 8) / pixel_size;
        point_c.field_8 = orient_to_mapU4[txquad->field_5];
        point_c.field_C = orient_to_mapV4[txquad->field_5];
        point_c.field_10 = txquad->field_26;
        draw_gpoly(&point_a, &point_d, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
    } else
    {
        struct GtBlock gtb;
        switch (txquad->field_2A)
        {
        case 0:
            gtb.field_0 = block_ptrs[578];
            break;
        case 1:
            gtb.field_0 = block_ptrs[579];
            break;
        case 3:
            gtb.field_0 = block_ptrs[txquad->field_6];
            break;
        default:
            gtb.field_0 = block_ptrs[txquad->field_6];
            goto LABEL_19;
            break;
        }
        gtb.field_C = txquad->field_1A >> 16;
        gtb.field_10 = txquad->field_1E >> 16;
        gtb.field_18 = txquad->field_22 >> 16;
        gtb.field_14 = txquad->field_26 >> 16;
LABEL_19:
        gtb.field_4 = (txquad->field_A >> 8) / pixel_size;
        gtb.field_8 = (txquad->field_E >> 8) / pixel_size;
        gtb.field_1C = orient_table_xflip[txquad->field_5];
        gtb.field_20 = orient_table_yflip[txquad->field_5];
        gtb.field_24 = orient_table_rotate[txquad->field_5];
        gtb.field_28 = (txquad->field_12 >> 8) / pixel_size >> 5;
        gtb.field_2C = (txquad->field_16 >> 8) / pixel_size >> 4;
        gtblock_draw(&gtb);
    }
}

void display_fast_drawlist(struct Camera *cam)
{
    int bucket_num;
    union {
      struct BasicQ *b;
      struct BasicUnk00 *unk00;
      struct BasicUnk01 *unk01;
      struct BasicUnk02 *unk02;
      struct BasicUnk03 *unk03;
      struct BasicUnk04 *unk04;
      struct BasicUnk05 *unk05;
      struct BasicUnk06 *unk06;
      struct BasicUnk07 *unk07;
      struct RotoSpr *rotSpr;
      struct BasicUnk09 *unk09;
      struct BasicUnk10 *unk10;
      struct JontySpr *jonSpr;
      struct BasicUnk12 *unk12;
      struct BasicUnk13 *unk13;
      struct BasicUnk14 *unk14;
      struct TexturedQuad *txquad;
      struct Number *number;
      struct RoomFlag *roomFlg;
    } item;
    //TODO enable rewritten code when gtblock_draw() is ready
    _DK_display_fast_drawlist(cam); return;
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_problems = 0;
    thing_pointed_at = 0;
    for (bucket_num = BUCKETS_COUNT-1; bucket_num >= 0; bucket_num--)
    {
        for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
        {
            switch (item.b->kind)
            {
            case 11:
                draw_fastview_mapwho(cam, item.jonSpr);
                break;
            case 13:
                draw_clipped_line(item.unk13->p.field_0,item.unk13->p.field_4,item.unk13->p.field_8,item.unk13->p.field_C,item.unk13->p.field_10);
                break;
            case 14:
                if (pixel_size == 1)
                    draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, 12288);
                else
                    draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, 4096);
                break;
            case 15:
                draw_texturedquad_block(item.txquad);
                break;
            case 16:
                draw_engine_number(item.number);
                break;
            case 17:
                draw_engine_room_flagpole(item.roomFlg);
                break;
            case 18:
                draw_iso_only_fastview_mapwho(cam, item.jonSpr);
                break;
            case 19:
                draw_engine_room_flag_top(item.roomFlg);
                break;
            default:
                render_problems++;
                render_prob_kind = item.b->kind;
                break;
            }
        }
    } // end for(bucket_num...
    if (render_problems > 0)
      WARNLOG("Encoured %lu rendering problems; last was with poly kind %ld",render_problems,render_prob_kind);
}

long convert_world_coord_to_front_view_screen_coord(struct Coord3d *pos, struct Camera *cam, long *x, long *y, long *z)
{
  return _DK_convert_world_coord_to_front_view_screen_coord(pos, cam, x, y, z);
}

void create_line_element(long a1, long a2, long a3, long a4, long bckt_idx, TbPixel color)
{
  struct BasicUnk13 *poly;
  if (bckt_idx >= BUCKETS_COUNT)
    bckt_idx = BUCKETS_COUNT-1;
  else
  if (bckt_idx < 0)
    bckt_idx = 0;
  poly = (struct BasicUnk13 *)getpoly;
  getpoly += sizeof(struct BasicUnk13);
  poly->b.next = buckets[bckt_idx];
  poly->b.kind = 13;
  buckets[bckt_idx] = (struct BasicQ *)poly;
  if (pixel_size > 0)
  {
    poly->p.field_0 = a1 / pixel_size;
    poly->p.field_4 = a2 / pixel_size;
    poly->p.field_8 = a3 / pixel_size;
    poly->p.field_C = a4 / pixel_size;
  }
  poly->p.field_10 = color;
}

void create_line_segment(struct EngineCoord *start, struct EngineCoord *end, TbPixel color)
{
  struct BasicUnk13 *poly;
  long bckt_idx;
  if (!is_free_space_in_poly_pool(1))
    return;
  // Get bucket index
  bckt_idx = (start->z+end->z)/2 / 16 - 2;
  if (bckt_idx >= BUCKETS_COUNT)
    bckt_idx = BUCKETS_COUNT-1;
  else
  if (bckt_idx < 0)
    bckt_idx = 0;
  // Add to bucket
  poly = (struct BasicUnk13 *)getpoly;
  getpoly += sizeof(struct BasicUnk13);
  poly->b.next = buckets[bckt_idx];
  poly->b.kind = 13;
  buckets[bckt_idx] = (struct BasicQ *)poly;
  // Fill parameters
  if (pixel_size > 0)
  {
    poly->p.field_0 = start->field_0;
    poly->p.field_4 = start->field_4;
    poly->p.field_8 = end->field_0;
    poly->p.field_C = end->field_4;
  }
  poly->p.field_10 = color;
}

void add_unkn11_to_polypool(struct Thing *thing, long scr_x, long scr_y, long a4, long bckt_idx)
{
  struct JontySpr *poly;
  if (bckt_idx >= BUCKETS_COUNT)
    bckt_idx = BUCKETS_COUNT-1;
  else
  if (bckt_idx < 0)
    bckt_idx = 0;
  poly = (struct JontySpr *)getpoly;
  getpoly += sizeof(struct JontySpr);
  poly->b.next = buckets[bckt_idx];
  poly->b.kind = 11;
  buckets[bckt_idx] = (struct BasicQ *)poly;
  poly->thing = thing;
  if (pixel_size > 0)
  {
    poly->scr_x = scr_x / pixel_size;
    poly->scr_y = scr_y / pixel_size;
  }
  poly->field_14 = a4;
}

void add_unkn18_to_polypool(struct Thing *thing, long scr_x, long scr_y, long a4, long bckt_idx)
{
  struct JontySpr *poly;
  if (bckt_idx >= BUCKETS_COUNT)
    bckt_idx = BUCKETS_COUNT-1;
  else
  if (bckt_idx < 0)
    bckt_idx = 0;
  poly = (struct JontySpr *)getpoly;
  getpoly += sizeof(struct JontySpr);
  poly->b.next = buckets[bckt_idx];
  poly->b.kind = 18;
  buckets[bckt_idx] = (struct BasicQ *)poly;
  poly->thing = thing;
  if (pixel_size > 0)
  {
    poly->scr_x = scr_x / pixel_size;
    poly->scr_y = scr_y / pixel_size;
  }
  poly->field_14 = a4;
}

void create_status_box_element(struct Thing *thing, long a2, long a3, long a4, long bckt_idx)
{
  struct BasicUnk14 *poly;
  if (bckt_idx >= BUCKETS_COUNT)
    bckt_idx = BUCKETS_COUNT-1;
  else
  if (bckt_idx < 0)
    bckt_idx = 0;
  poly = (struct BasicUnk14 *)getpoly;
  getpoly += sizeof(struct BasicUnk14);
  poly->b.next = buckets[bckt_idx];
  poly->b.kind = 14;
  buckets[bckt_idx] = (struct BasicQ *)poly;
  poly->thing = thing;
  if (pixel_size > 0)
  {
    poly->field_C = a2 / pixel_size;
    poly->field_10 = a3 / pixel_size;
  }
  poly->field_14 = a4;
}

void create_fast_view_status_box(struct Thing *thing, long x, long y)
{
  create_status_box_element(thing, x, y - (shield_offset[thing->model]+thing->field_58) / 12, y, 1);
}

void add_textruredquad_to_polypool(long x, long y, long texture_idx, long a7, long a8, long lightness, long a9, long bckt_idx)
{
    struct TexturedQuad *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct TexturedQuad *)getpoly;
    getpoly += sizeof(struct TexturedQuad);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = 15;
    buckets[bckt_idx] = (struct BasicQ *)poly;

    poly->field_6 = texture_idx;
    poly->field_A = x;
    poly->field_E = y;
    poly->field_12 = a7;
    poly->field_16 = a7;
    poly->field_5 = a8;
    poly->field_1A = lightness;
    poly->field_1E = lightness;
    poly->field_22 = lightness;
    poly->field_26 = lightness;
    poly->field_2A = a9;
}

void add_lgttextrdquad_to_polypool(long x, long y, long texture_idx, long a6, long a7, long a8, long lg0, long lg1, long lg2, long lg3, long bckt_idx)
{
    struct TexturedQuad *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct TexturedQuad *)getpoly;
    getpoly += sizeof(struct TexturedQuad);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = 15;
    buckets[bckt_idx] = (struct BasicQ *)poly;

    poly->field_6 = texture_idx;
    poly->field_A = x;
    poly->field_E = y;
    poly->field_12 = a6;
    poly->field_16 = a7;
    poly->field_5 = a8;
    poly->field_1A = lg0;
    poly->field_1E = lg1;
    poly->field_22 = lg2;
    poly->field_26 = lg3;
    poly->field_2A = 3;
}

void add_unkn16_to_polypool(long x, long y, long lvl, long bckt_idx)
{
  struct Number *poly;
  if (bckt_idx >= BUCKETS_COUNT)
    bckt_idx = BUCKETS_COUNT-1;
  else
  if (bckt_idx < 0)
    bckt_idx = 0;
  poly = (struct Number *)getpoly;
  getpoly += sizeof(struct Number);
  poly->b.next = buckets[bckt_idx];
  poly->b.kind = 16;
  buckets[bckt_idx] = (struct BasicQ *)poly;
  if (pixel_size > 0)
  {
    poly->x = x / pixel_size;
    poly->y = y / pixel_size;
  }
  poly->lvl = lvl;
}

void add_unkn17_to_polypool(long x, long y, long lvl, long bckt_idx)
{
  struct RoomFlag *poly;
  if (bckt_idx >= BUCKETS_COUNT)
    bckt_idx = BUCKETS_COUNT-1;
  else
  if (bckt_idx < 0)
    bckt_idx = 0;
  poly = (struct RoomFlag *)getpoly;
  getpoly += sizeof(struct RoomFlag);
  poly->b.next = buckets[bckt_idx];
  poly->b.kind = 17;
  buckets[bckt_idx] = (struct BasicQ *)poly;
  if (pixel_size > 0)
  {
    poly->x = x / pixel_size;
    poly->y = y / pixel_size;
  }
  poly->lvl = lvl;
}

void add_unkn19_to_polypool(long x, long y, long lvl, long bckt_idx)
{
  struct RoomFlag *poly;
  if (bckt_idx >= BUCKETS_COUNT)
    bckt_idx = BUCKETS_COUNT-1;
  else
  if (bckt_idx < 0)
    bckt_idx = 0;
  poly = (struct RoomFlag *)getpoly;
  getpoly += sizeof(struct RoomFlag);
  poly->b.next = buckets[bckt_idx];
  poly->b.kind = 19;
  buckets[bckt_idx] = (struct BasicQ *)poly;
  if (pixel_size > 0)
  {
    poly->x = x / pixel_size;
    poly->y = y / pixel_size;
  }
  poly->lvl = lvl;
}

void prepare_lightness_intensity_array(long stl_x, long stl_y, long *arrp, long base_lightness)
{
    long *rndmis;
    long rndi,nval;
    long i;
    i = 4 * stl_x + 17 * stl_y;
    rndmis = &randomisors[i & 0xFF];
    for (i=0; i < 9; i++)
    {
      if ((base_lightness <= 256) || (base_lightness > 15872))
      {
          nval = base_lightness;
      } else
      {
          rndi = *rndmis;
          rndmis++;
          nval = 32 * (rndi & 0x3F) + base_lightness - 256;
      }
      *arrp = nval << 8;
      arrp++;
    }
}

void draw_element(struct Map *map, long lightness, long stl_x, long stl_y, long pos_x, long pos_y, long a7, unsigned char a8, long *a9)
{
    struct PlayerInfo *myplyr;
    TbBool sibrevealed[3][3];
    struct Column *col;
    struct UnkStruc5 *unkstrcp;
    struct Map *mapblk;
    long lightness_arr[4][9];
    long bckt_idx;
    long cube_itm,delta_y;
    long tc; // top cube index
    long x,y;
    long i;
  //_DK_draw_element(map, lightness, stl_x, stl_y, pos_x, pos_y, a7, a8, a9); return;
    myplyr = get_my_player();
    cube_itm = (a8 + 2) & 3;
    delta_y = (a7 << 7) / 256;
    bckt_idx = myplyr->engine_window_height - (pos_y >> 8) + 64;
    // Check if there's enough place to draw
    if (!is_free_space_in_poly_pool(8))
      return;

    // Prepare light intensity array

    for (y=0; y < 3; y++)
        for (x=0; x < 3; x++)
        {
            sibrevealed[y][x] = subtile_revealed(stl_x+x-1, stl_y+y-1, myplyr->id_number);
        }

    i = 0;
    if (sibrevealed[0][1] && sibrevealed[1][0] && sibrevealed[1][1] && sibrevealed[0][0])
        i = lightness;
    prepare_lightness_intensity_array(stl_x,stl_y,lightness_arr[(-a8) & 3],i);

    i = 0;
    if (sibrevealed[0][1] && sibrevealed[0][2] && sibrevealed[1][2] && sibrevealed[1][1])
        i = get_subtile_lightness(stl_x+1,stl_y);
    prepare_lightness_intensity_array(stl_x+1,stl_y,lightness_arr[(1-a8) & 3],i);

    i = 0;
    if (sibrevealed[1][0] && sibrevealed[1][1] && sibrevealed[2][0] && sibrevealed[2][1])
        i = get_subtile_lightness(stl_x,stl_y+1);
    prepare_lightness_intensity_array(stl_x,stl_y+1,lightness_arr[(-1-a8) & 3],i);

    i = 0;
    if (sibrevealed[2][2] && sibrevealed[1][2] && sibrevealed[1][1] && sibrevealed[2][1])
        i = get_subtile_lightness(stl_x+1,stl_y+1);
    prepare_lightness_intensity_array(stl_x+1,stl_y+1,lightness_arr[(-2-a8) & 3],i);

    // Get column to be drawn on the current subtile

    if (map_block_revealed_bit(map, player_bit))
      i = get_mapblk_column_index(map);
    else
      i = game.field_149E77;
    col = get_column(i);
    mapblk = get_map_block_at(stl_x, stl_y);

    // Draw the columns base block

    if (*a9 > pos_y)
    {
      if ((col->baseblock != 0) && (col->cubes[0] == 0))
      {
          *a9 = pos_y;
          if ((mapblk->flags & 0x04) != 0)
          {
              add_textruredquad_to_polypool(pos_x, pos_y, col->baseblock, a7, 0,
                  2097152, 0, bckt_idx);
          } else
          {
              add_lgttextrdquad_to_polypool(pos_x, pos_y, col->baseblock, a7, a7, 0,
                  lightness_arr[0][0], lightness_arr[1][0], lightness_arr[2][0], lightness_arr[3][0], bckt_idx);
          }
      }
    }

    // Draw the columns cubes

    y = a7 + pos_y;
    unkstrcp = 0;
    for (tc=0; tc < COLUMN_STACK_HEIGHT; tc++)
    {
      if (col->cubes[tc] == 0)
        break;
      y -= delta_y;
      unkstrcp = &game.struc_D8C7[col->cubes[tc]];
      if (*a9 > y)
      {
        *a9 = y;
        add_lgttextrdquad_to_polypool(pos_x, y, unkstrcp->texture_0[cube_itm], a7, delta_y, 0,
            lightness_arr[3][tc+1], lightness_arr[2][tc+1], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
      }
    }

    if (unkstrcp != NULL)
    {
      i = y - a7;
      if (*a9 > i)
      {
        *a9 = i;
        if ((mapblk->flags & 0x80) != 0)
        {
          add_textruredquad_to_polypool(pos_x, i, unkstrcp->field_8[0], a7, a8,
              2097152, 1, bckt_idx);
        } else
        if ((mapblk->flags & 0x04) != 0)
        {
          add_textruredquad_to_polypool(pos_x, i, unkstrcp->field_8[0], a7, a8,
              2097152, 0, bckt_idx);
        } else
        {
          add_lgttextrdquad_to_polypool(pos_x, i, unkstrcp->field_8[0], a7, a7, a8,
              lightness_arr[0][tc], lightness_arr[1][tc], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
        }
      }
    }

    // If there are still some solid cubes higher than tc
    if (((col->bitfileds & 0x0E) != 0) && (col->solidmask > (1 << tc)))
    {
        // Find any top cube separated by empty space
        for (;tc < COLUMN_STACK_HEIGHT; tc++)
        {
            if (col->cubes[tc] != 0)
              break;
            y -= delta_y;
        }

        for (;tc < COLUMN_STACK_HEIGHT; tc++)
        {
            if (col->cubes[tc] == 0)
              break;
            y -= delta_y;
            unkstrcp = &game.struc_D8C7[col->cubes[tc]];
            if (*a9 > y)
            {
              add_lgttextrdquad_to_polypool(pos_x, y, unkstrcp->texture_0[cube_itm], a7, delta_y, 0,
                  lightness_arr[3][tc+1], lightness_arr[2][tc+1], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
            }
        }
        if (unkstrcp != NULL)
        {
          i = y - a7;
          if (*a9 > i)
          {
            add_lgttextrdquad_to_polypool(pos_x, i, unkstrcp->field_8[0], a7, a7, a8,
                lightness_arr[0][tc], lightness_arr[1][tc], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
          }
        }
    }

}

void update_frontview_pointed_block(unsigned long laaa, unsigned char qdrant, long w, long h, long qx, long qy)
{
  TbGraphicsWindow ewnd;
  unsigned long mask;
  struct Map *map;
  long pos_x,pos_y;
  long slb_x,slb_y;
  long point_a,point_b,delta;
  long i;
  SYNCDBG(16,"Starting");
  store_engine_window(&ewnd,1);
  point_a = (((GetMouseX() - ewnd.x) << 8) - qx) << 8;
  point_b = (((GetMouseY() - ewnd.y) << 8) - qy) << 8;
  delta = (laaa << 7) / 256 << 8;
  for (i=0; i < 8; i++)
  {
    pos_x = (point_a / laaa) * x_step2[qdrant] + (point_b / laaa) * x_step1[qdrant] + (w << 8);
    pos_y = (point_a / laaa) * y_step2[qdrant] + (point_b / laaa) * y_step1[qdrant] + (h << 8);
    slb_x = (pos_x >> 8) + x_offs[qdrant];
    slb_y = (pos_y >> 8) + y_offs[qdrant];
    map = get_map_block_at(slb_x, slb_y);
    if (!map_block_invalid(map))
    {
      if (i == 0)
      {
        floor_pointed_at_x = slb_x;
        floor_pointed_at_y = slb_y;
        block_pointed_at_x = slb_x;
        block_pointed_at_y = slb_y;
        pointed_at_frac_x = pos_x & 0xFF;
        pointed_at_frac_y = pos_y & 0xFF;
        me_pointed_at = map;
      } else
      {
        mask = 0;
        mask = game.columns[map->data & 0x7FF].solidmask;
        if ( (1 << (i-1)) & mask )
        {
          pointed_at_frac_x = pos_x & 0xFF;
          pointed_at_frac_y = pos_y & 0xFF;
          block_pointed_at_x = slb_x;
          block_pointed_at_y = slb_y;
          me_pointed_at = map;
        }
        if (((temp_cluedo_mode)  && (i == 2))
         || ((!temp_cluedo_mode) && (i == 5)))
        {
          top_pointed_at_frac_x = pos_x & 0xFF;
          top_pointed_at_frac_y = pos_y & 0xFF;
          top_pointed_at_x = slb_x;
          top_pointed_at_y = slb_y;
        }
      }
    }
    point_b += delta;
  }
}

void create_frontview_map_volume_box(struct Camera *cam, unsigned char stl_width)
{
  struct Coord3d pos;
  long coord_x,coord_y,coord_z;
  unsigned char orient;
  long i;
  long slb_width,depth;
  long vstart,vend;
  long delta[4];

  pos.y.val = map_volume_box.field_7;
  pos.x.val = map_volume_box.field_3;
  pos.z.val = 1280;
  orient = ((unsigned int)(cam->orient_a + 256) >> 9) & 0x03;
  convert_world_coord_to_front_view_screen_coord(&pos, cam, &coord_x, &coord_y, &coord_z);
  depth = (5 - map_volume_box.field_13) * ((long)stl_width << 7) / 256;
  slb_width = 3 * (long)stl_width;
  switch ( orient )
  {
  case 1:
      coord_y -= slb_width;
      coord_z += slb_width;
      break;
  case 2:
      coord_x -= slb_width;
      coord_y -= slb_width;
      coord_z += slb_width;
      break;
  case 3:
      coord_x -= slb_width;
      break;
  }
  vstart = 0;
  coord_z -= (stl_width >> 1);
  vend = stl_width;
  delta[0] = 0;
  delta[1] = slb_width;
  delta[2] = depth;
  delta[3] = slb_width + depth;
  // Draw a horizonal line element for every subtile
  for (i=3; i > 0; i--)
  {
    if (!is_free_space_in_poly_pool(4))
      break;
    create_line_element(coord_x + vstart,    coord_y + delta[0],  coord_x + vend,      coord_y + delta[0], coord_z,             map_volume_box.color);
    create_line_element(coord_x + vstart,    coord_y + delta[1],  coord_x + vend,      coord_y + delta[1], coord_z - slb_width, map_volume_box.color);
    create_line_element(coord_x + vstart,    coord_y + delta[2],  coord_x + vend,      coord_y + delta[2], coord_z,             map_volume_box.color);
    create_line_element(coord_x + vstart,    coord_y + delta[3],  coord_x + vend,      coord_y + delta[3], coord_z - slb_width, map_volume_box.color);
    vend += stl_width;
    vstart += stl_width;
  }
  // Now the rectangles at left and right
  for (i=3; i > 0; i--)
  {
    if (!is_free_space_in_poly_pool(4))
      break;
    create_line_element(coord_x,             coord_y + delta[0],  coord_x,             coord_y + delta[1], coord_z - delta[0], map_volume_box.color);
    create_line_element(coord_x + slb_width, coord_y + delta[0],  coord_x + slb_width, coord_y + delta[1], coord_z - delta[0], map_volume_box.color);
    create_line_element(coord_x,             coord_y + delta[2],  coord_x,             coord_y + delta[3], coord_z - delta[0], map_volume_box.color);
    create_line_element(coord_x + slb_width, coord_y + delta[2],  coord_x + slb_width, coord_y + delta[3], coord_z - delta[0], map_volume_box.color);
    delta[0] += stl_width;
    delta[2] += stl_width;
    delta[3] += stl_width;
    delta[1] += stl_width;
  }
}

void draw_frontview_thing_on_element(struct Thing *thing, struct Map *map, struct Camera *cam)
{
  long cx,cy,cz;
  if ((thing->field_4F & 0x01) != 0)
    return;
  switch ( (thing->field_50 >> 2) )
  {
  case 2:
          convert_world_coord_to_front_view_screen_coord(&thing->mappos,cam,&cx,&cy,&cz);
          if (is_free_space_in_poly_pool(1))
          {
            add_unkn11_to_polypool(thing, cx, cy, cy, cz-3);
            if ((thing->class_id == TCls_Creature) && is_free_space_in_poly_pool(1))
            {
              create_fast_view_status_box(thing, cx, cy);
            }
          }
          break;
  case 4:
          convert_world_coord_to_front_view_screen_coord(&thing->mappos,cam,&cx,&cy,&cz);
          if (is_free_space_in_poly_pool(1))
          {
            add_unkn16_to_polypool(cx, cy, thing->long_13, 1);
          }
          break;
  case 5:
          convert_world_coord_to_front_view_screen_coord(&thing->mappos,cam,&cx,&cy,&cz);
          if (is_free_space_in_poly_pool(1))
          {
            if (game.play_gameturn - thing->long_15 != 1)
            {
              thing->field_19 = 0;
            } else
            if (thing->field_19 < 40)
            {
              thing->field_19++;
            }
            thing->long_15 = game.play_gameturn;
            if (thing->field_19 == 40)
            {
              add_unkn17_to_polypool(cx, cy, thing->long_13, cz-3);
              if (is_free_space_in_poly_pool(1))
              {
                add_unkn19_to_polypool(cx, cy, thing->long_13, 1);
              }
            }
          }
          break;
  case 6:
          convert_world_coord_to_front_view_screen_coord(&thing->mappos,cam,&cx,&cy,&cz);
          if (is_free_space_in_poly_pool(1))
          {
            add_unkn18_to_polypool(thing, cx, cy, cy, cz-3);
          }
          break;
  default:
          break;
  }
}

void draw_frontview_things_on_element(struct Map *map, struct Camera *cam)
{
  struct Thing *thing;
  long i;
  unsigned long k;
  k = 0;
  i = get_mapwho_thing_index(map);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->field_2;
    draw_frontview_thing_on_element(thing, map, cam);
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void draw_frontview_engine(struct Camera *cam)
{
  struct PlayerInfo *player;
  TbGraphicsWindow grwnd;
  TbGraphicsWindow ewnd;
  unsigned char qdrant;
  struct Map *map;
  long px,py,qx,qy;
  long w,h;
  long pos_x,pos_y;
  long stl_x,stl_y;
  long lim_x,lim_y;
  long cam_x,cam_y;
  long long laaa,lbbb;
  long i;
  SYNCDBG(9,"Starting");
  player = get_my_player();
  UseFastBlockDraw = (cam->field_17 == 65536);
  if (cam->field_17 > 65536)
    cam->field_17 = 65536;
  LbScreenStoreGraphicsWindow(&grwnd);
  store_engine_window(&ewnd,pixel_size);
  LbScreenSetGraphicsWindow(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
  gtblock_set_clipping_window(lbDisplay.GraphicsWindowPtr, ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
  setup_vecs(lbDisplay.GraphicsWindowPtr, NULL, lbDisplay.GraphicsScreenWidth, ewnd.width, ewnd.height);
  engine_player_number = player->id_number;
  player_bit = (1 << player->id_number);
  clear_fast_bucket_list();
  store_engine_window(&ewnd,1);
  setup_engine_window(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
  qdrant = ((unsigned int)(cam->orient_a + 256) >> 9) & 0x03;
  laaa = (32 * cam->field_17) / 256;
  w = (ewnd.width << 16) / laaa >> 1;
  h = (ewnd.height << 16) / laaa >> 1;
  cam_x = cam->mappos.x.val;
  cam_y = cam->mappos.y.val;
  switch (qdrant)
  {
  case 0:
      px = ((cam_x - w) >> 8);
      py = ((cam_y - h) >> 8);
      lbbb = cam_x - (px << 8);
      qx = (ewnd.width << 7)  - ((laaa * lbbb) >> 8);
      lbbb = cam_y - (py << 8);
      qy = (ewnd.height << 7) - ((laaa * lbbb) >> 8);
      break;
  case 1:
      px = ((cam_x + h) >> 8);
      py = ((cam_y - w) >> 8);
      lbbb = cam_y - (py << 8);
      qx = (ewnd.width << 7)  - ((laaa * lbbb) >> 8);
      lbbb = (px << 8) - cam_x;
      qy = (ewnd.height << 7) - ((laaa * lbbb) >> 8);
      px--;
      break;
  case 2:
      px = ((cam_x + w) >> 8) + 1;
      py = ((cam_y + h) >> 8);
      lbbb = (px << 8) - cam_x;
      qx = (ewnd.width << 7)  - ((laaa * lbbb) >> 8);
      lbbb = (py << 8) - cam_y;
      qy = (ewnd.height << 7) - ((laaa * lbbb) >> 8);
      px--;
      py--;
      break;
  case 3:
      px = ((cam_x - h) >> 8);
      py = ((cam_y + w) >> 8) + 1;
      lbbb = (py << 8) - cam_y;
      qx = (ewnd.width << 7)  - ((laaa * lbbb) >> 8);
      lbbb = cam_x - (px << 8);
      qy = (ewnd.height << 7) - ((laaa * lbbb) >> 8);
      py--;
      break;
  default:
      ERRORLOG("Illegal quadrant, %d.",qdrant);
      LbScreenLoadGraphicsWindow(&grwnd);
      return;
  }

  update_frontview_pointed_block(laaa, qdrant, px, py, qx, qy);
  if (map_volume_box.visible)
      create_frontview_map_volume_box(cam, (laaa >> 8) & 0xFF);
  map_volume_box.visible = 0;

  h = (8 * (laaa + 32 * ewnd.height) - qy) / laaa;
  w = (8 * (laaa + 32 * ewnd.height) - qy) / laaa;
  qy += laaa * h;
  px += x_step1[qdrant] * w;
  stl_x = x_step1[qdrant] * w + px;
  stl_y = y_step1[qdrant] * h + py;
  py += y_step1[qdrant] * h;
  lim_x = ewnd.width << 8;
  lim_y = -laaa;
  SYNCDBG(19,"Range (%ld,%ld) to (%ld,%ld), quadrant %d",px,py,qx,qy,(int)qdrant);
  for (pos_x=qx; pos_x < lim_x; pos_x += laaa)
  {
    i = (ewnd.height << 8);
    // Initialize the stl_? which will be swept by second loop
    if (x_step1[qdrant] != 0)
      stl_x = px;
    else
      stl_y = py;
    for (pos_y=qy; pos_y > lim_y; pos_y -= laaa)
    {
        map = get_map_block_at(stl_x, stl_y);
        if (!map_block_invalid(map))
        {
            if (get_mapblk_column_index(map) > 0)
            {
                draw_element(map, game.field_46157[get_subtile_number(stl_x,stl_y)], stl_x, stl_y, pos_x, pos_y, laaa, qdrant, &i);
            }
            if ( subtile_revealed(stl_x, stl_y, player->id_number) )
            {
                draw_frontview_things_on_element(map, cam);
            }
        }
        stl_x -= x_step1[qdrant];
        stl_y -= y_step1[qdrant];
    }
    stl_x += x_step2[qdrant];
    stl_y += y_step2[qdrant];
  }

  display_fast_drawlist(cam);
  LbScreenLoadGraphicsWindow(&grwnd);
  SYNCDBG(9,"Finished");
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
