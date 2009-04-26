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

#include "engine_lenses.h"
#include "engine_camera.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
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
unsigned char i_can_see_levels[] = {15, 20, 25, 30,};

/******************************************************************************/
long compute_cells_away(void)
{
  long xmin,ymin,xmax,ymax;
  long xcell,ycell;
  struct PlayerInfo *player;
  long ncells_a;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
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
      ycell = 200 / pixel_size - ymin - y_init_off;
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
  engine_player_number = player->field_2B;
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

  player = &(game.players[my_player_number%PLAYERS_COUNT]);
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
  static const char *func_name="find_gamut";
#if (BFDEBUG_LEVEL > 19)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_find_gamut();
}

void fiddle_gamut(long a1, long a2)
{
  _DK_fiddle_gamut(a1, a2);
}

void create_map_volume_box(long a1, long a2, long a3)
{
  _DK_create_map_volume_box(a1, a2, a3);
}

void do_a_plane_of_engine_columns_perspective(long a1, long a2, long a3, long a4)
{
  _DK_do_a_plane_of_engine_columns_perspective(a1, a2, a3, a4);
}

void do_a_plane_of_engine_columns_cluedo(long a1, long a2, long a3, long a4)
{
  _DK_do_a_plane_of_engine_columns_cluedo(a1, a2, a3, a4);
}

void do_a_plane_of_engine_columns_isometric(long a1, long a2, long a3, long a4)
{
  _DK_do_a_plane_of_engine_columns_isometric(a1, a2, a3, a4);
}

void display_drawlist(void)
{
  static const char *func_name="display_drawlist";
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_display_drawlist();
}

void draw_view(struct Camera *cam, unsigned char a2)
{
  static const char *func_name="draw_view";
  long nlens;
  long x,y,z;
  long xcell,ycell;
  long i;
  long aposc,bposc;
  struct EngineCol *ec;
  struct MinMax *mm;
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_draw_view(cam, a2); return;
  nlens = cam->field_17 / pixel_size;
  getpoly = poly_pool;
  memset(buckets, 0, 0xB00u);
  perspective = perspective_routines[lens_mode%PERS_ROUTINES_COUNT];
  rotpers = rotpers_routines[lens_mode%PERS_ROUTINES_COUNT];
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
  aposc = -(unsigned char)x;
  bposc = (cells_away << 8) + (y & 0xFF);
  ycell = (y >> 8) - (cells_away+1);
  find_gamut();
  fiddle_gamut(xcell, ycell + (cells_away+1));
  apos = aposc;
  bpos = bposc;
  back_ec = &ecs1[0];
  front_ec = &ecs2[0];
  mm = &minmaxs[31-cells_away];
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
  if (map_volume_box.field_0)
    create_map_volume_box(x, y, z);
  display_drawlist();
  map_volume_box.field_0 = 0;
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}



/******************************************************************************/
#ifdef __cplusplus
}
#endif
