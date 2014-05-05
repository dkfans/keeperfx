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
 * @date     20 Mar 2009 - 20 May 2011
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
#include "bflib_math.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_render.h"
#include "bflib_heapmgr.h"

#include "engine_lenses.h"
#include "engine_camera.h"
#include "engine_arrays.h"
#include "engine_textures.h"
#include "engine_redraw.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "creature_states_mood.h"
#include "creature_states_gardn.h"
#include "creature_states_lair.h"
#include "thing_stats.h"
#include "game_lghtshdw.h"
#include "game_heap.h"
#include "kjm_input.h"
#include "front_simple.h"
#include "frontend.h"
#include "vidmode.h"
#include "config_settings.h"
#include "config_creature.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_draw_fastview_mapwho(struct Camera *cam, struct JontySpr *outbuf);
DLLIMPORT void _DK_draw_clipped_line(long pos_x, long pos_z, long start_y, long end_y, unsigned char scale);
DLLIMPORT void _DK_draw_engine_room_flagpole(struct RoomFlag *rflg);
DLLIMPORT void _DK_draw_status_sprites(long pos_x, long pos_z, struct Thing *thing, long end_y);
DLLIMPORT void _DK_draw_engine_room_flag_top(struct RoomFlag *rflg);
DLLIMPORT void _DK_draw_stripey_line(long pos_x, long pos_z, long start_y, long end_y, unsigned char scale);
DLLIMPORT void _DK_draw_jonty_mapwho(struct JontySpr *jspr);
DLLIMPORT void _DK_draw_keepsprite_unscaled_in_buffer(unsigned short pos_x, short pos_z, unsigned char start_y, unsigned char *end_y);
DLLIMPORT long _DK_convert_world_coord_to_front_view_screen_coord(struct Coord3d *pos, struct Camera *cam, long *x, long *y, long *z);
DLLIMPORT void _DK_draw_frontview_engine(struct Camera *cam);
DLLIMPORT void _DK_draw_view(struct Camera *cam, unsigned char pos_z);
DLLIMPORT void _DK_do_a_plane_of_engine_columns_cluedo(long pos_x, long pos_z, long start_y, long end_y);
DLLIMPORT void _DK_do_a_plane_of_engine_columns_isometric(long pos_x, long pos_z, long start_y, long end_y);
DLLIMPORT void _DK_rotate_base_axis(struct M33 *matx, short pos_z, unsigned char start_y);
DLLIMPORT void _DK_fill_in_points_perspective(long pos_x, long pos_z, struct MinMax *mm);
DLLIMPORT void _DK_fill_in_points_cluedo(long pos_x, long pos_z, struct MinMax *mm);
DLLIMPORT void _DK_fill_in_points_isometric(long pos_x, long pos_z, struct MinMax *mm);
DLLIMPORT void _DK_find_gamut(void);
DLLIMPORT void _DK_frame_wibble_generate(void);
DLLIMPORT void _DK_setup_rotate_stuff(long pos_x, long pos_z, long start_y, long end_y, long scale, long a6, long a7, long a8);
DLLIMPORT void _DK_process_keeper_sprite(short x, short y, unsigned short start_y, short end_y, unsigned char scale, long a6);
DLLIMPORT void _DK_do_a_trig_gourad_tr(struct EngineCoord *ep1, struct EngineCoord *ep2, struct EngineCoord *ep3, short plane_end, long scale);
DLLIMPORT void _DK_do_a_trig_gourad_bl(struct EngineCoord *ep1, struct EngineCoord *ep2, struct EngineCoord *ep3, short plane_end, long scale);
DLLIMPORT void _DK_do_map_who(short stl_x);
DLLIMPORT void _DK_fiddle_half_gamut(long y, long pos_y, long floor_x, long floor_y);
DLLIMPORT long _DK_heap_manage_keepersprite(unsigned short kspr_n);
DLLIMPORT void _DK_draw_single_keepersprite_xflip(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_n, long scale);
DLLIMPORT long _DK_load_single_frame(unsigned short kspr_idx);
DLLIMPORT long _DK_do_a_plane_of_engine_columns_sub5(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3);
DLLIMPORT void _DK_do_a_gpoly_gourad_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short plane_end, int a5);
DLLIMPORT void _DK_do_a_gpoly_unlit_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short plane_end);
DLLIMPORT void _DK_do_a_gpoly_unlit_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short plane_end);
DLLIMPORT void _DK_do_a_gpoly_gourad_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short plane_end, int a5);
/******************************************************************************/
unsigned short shield_offset[] = {
 0x0,  0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x118, 0x80,
 0x80, 0x100,  0x80,  0x80, 0x100, 0x100, 0x138,  0x80,  0x80, 0x138,  0x80,  0x80, 0x100,  0x80, 0x80, 0x100,
};
struct SideOri sideoris[] = {
    { 0,  1,  2,  3},
    { 0,  0,  3,  2},
    { 1,128,  2,  1},
    { 0,  3,128,  2},
    { 3,  0,  1,  0},
    { 3,  2,  1,  0},
    {128, 3,  0,  1},
    { 2,  0,  1,  2},
    { 3,  0,  0,  1},
    { 0,  3,  2,128},
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
long const orient_to_mapU1[] = { 0x00, 0x1F0000, 0x1F0000, 0x00 };
long const orient_to_mapU2[] = { 0x1F0000, 0x1F0000, 0x00, 0x00 };
long const orient_to_mapU3[] = { 0x1F0000, 0x00, 0x00, 0x1F0000 };
long const orient_to_mapU4[] = { 0x00, 0x00, 0x1F0000, 0x1F0000 };
long const orient_to_mapV1[] = { 0x00, 0x00, 0x1F0000, 0x1F0000 };
long const orient_to_mapV2[] = { 0x00, 0x1F0000, 0x1F0000, 0x00 };
long const orient_to_mapV3[] = { 0x1F0000, 0x1F0000, 0x00, 0x00 };
long const orient_to_mapV4[] = { 0x1F0000, 0x00, 0x00, 0x1F0000 };
//unsigned char temp_cluedo_mode;
unsigned long render_problems;
long render_prob_kind;
long sp_x,sp_y,sp_dx,sp_dy;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void get_floor_pointed_at(long x, long y, long *floor_x, long *floor_y)
{
    long long ofs_x,ofs_y;
    long long sor_hp,sor_hn,sor_vp,sor_vn;
    long long der_hp,der_hn,der_vp,der_vn;
    if ( (vert_offset[1] == 0) && (hori_offset[1] == 0) )
    {
        *floor_x = 0;
        *floor_y = 0;
        return;
    }
    ofs_x = (long long)x - (long long)x_init_off;
    ofs_y = (long long)y - (long long)y_init_off;
    sor_vp = (((long long)vert_offset[1] * ofs_x) / 2LL);
    sor_vn = (((long long)vert_offset[0] * ofs_y) / 2LL);
    der_vp = ((long long)hori_offset[0] * (long long)vert_offset[1]) / 8LL;
    der_vn = ((long long)vert_offset[0] * (long long)hori_offset[1]) / 8LL;
    sor_hp = (((long long)hori_offset[1] * ofs_x) / 2LL);
    sor_hn = (((long long)hori_offset[0] * ofs_y) / 2LL);
    der_hp = ((long long)vert_offset[0] * (long long)hori_offset[1]) / 8LL;
    der_hn = ((long long)hori_offset[0] * (long long)vert_offset[1]) / 8LL;
    *floor_y = ((sor_vp-sor_vn) / ((der_vp-der_vn)>>8)) >> 2;
    *floor_x = ((sor_hp-sor_hn) / ((der_hp-der_hn)>>8)) >> 2;
}

long compute_cells_away(void)
{
    long half_width,half_height;
    long xmin,ymin,xmax,ymax;
    long xcell,ycell;
    struct PlayerInfo *player;
    long ncells_a;
    player = get_my_player();
    half_width = (player->engine_window_width >> 1);
    half_height = (player->engine_window_height >> 1);
    xcell = ((half_width<<1) + (half_width>>4))/pixel_size - player->engine_window_x/pixel_size;
    ycell = ((8 * high_offset[1]) >> 8) - (half_width>>4)/pixel_size - player->engine_window_y/pixel_size;
    get_floor_pointed_at(xcell, ycell, &xmax, &ymax);
    xcell = (half_width)/pixel_size - player->engine_window_x/pixel_size;
    ycell = (half_height)/pixel_size - player->engine_window_y/pixel_size;
    get_floor_pointed_at(xcell, ycell, &xmin, &ymin);
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
    fade_min = (768 * ncells_a) / 4;
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
    max_i_can_see = get_creature_can_see_subtiles();
    if (lens_mode != 0)
      temp_cluedo_mode = 0;
    else
      temp_cluedo_mode = settings.video_cluedo_mode;
    thing_pointed_at = NULL;
}

/**
 * Sets the reserved amount of poly pool entries.
 * Entries which are reserved won't be filled by standard rendering items, even if the queue is full.
 * @param nitems
 */
void poly_pool_end_reserve(int nitems)
{
    poly_pool_end = &poly_pool[sizeof(poly_pool)-(nitems*sizeof(struct BasicUnk13)-1)];
}

TbBool is_free_space_in_poly_pool(int nitems)
{
    return (getpoly+(nitems*sizeof(struct BasicUnk13)) <= poly_pool_end);
}

void rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx, long zoom)
{
    long factor_w,factor_h;
    long inp_x,inp_y,inp_z;
    long long out_x,out_y;
    inp_x = epos->x;
    inp_y = epos->y;
    inp_z = epos->z;
    out_x = (inp_z * matx->r0[2] + (inp_y + matx->r0[0]) * (inp_x + matx->r0[1]) - matx->r0[3] - inp_x * inp_y) >> 14;
    epos->x = out_x;
    out_y = (inp_z * matx->r1[2] + (inp_y + matx->r1[0]) * (inp_x + matx->r1[1]) - matx->r1[3] - inp_x * inp_y) >> 14;
    epos->y = out_y;
    epos->z = (inp_z * matx->r2[2] + (inp_y + matx->r2[0]) * (inp_x + matx->r2[1]) - matx->r2[3] - inp_x * inp_y) >> 14;
    factor_w = (long)view_width_over_2 + (zoom * out_x >> 16);
    epos->view_width = factor_w;
    factor_h = (long)view_height_over_2 - (zoom * out_y >> 16);
    epos->view_height = factor_h;
    if (factor_w < 0)
    {
        epos->field_8 |= 0x0008;
    } else
    if (vec_window_width <= factor_w)
    {
        epos->field_8 |= 0x0010;
    }
    if (factor_h < 0)
    {
        epos->field_8 |= 0x0020;
    } else
    if (factor_h >= vec_window_height)
    {
        epos->field_8 |= 0x0040;
    }
    epos->field_8 |= 0x0400;
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

void setup_rotate_stuff(long x, long y, long z, long fade_max, long fade_min, long zoom, long map_angle, long map_roll)
{
  _DK_setup_rotate_stuff(x, y, z, fade_max, fade_min, zoom, map_angle, map_roll);
}

void create_box_coords(struct EngineCoord *coord, long x, long z, long y)
{
    coord->x = x;
    coord->z = z;
    coord->field_8 = 0;
    coord->y = y;
    rotpers(coord, &camera_matrix);
}

void do_perspective_rotation(long x, long y, long z)
{
    struct PlayerInfo *player;
    struct EngineCoord epos;
    long zoom;
    long engine_w,engine_h;
    player = get_my_player();
    zoom = camera_zoom / pixel_size;
    engine_w = player->engine_window_width/pixel_size;
    engine_h = player->engine_window_height/pixel_size;
    epos.x = -x;
    epos.y = 0;
    epos.z = y;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    x_init_off = epos.view_width;
    y_init_off = epos.view_height;
    depth_init_off = epos.z;
    epos.x = 65536;
    epos.y = 0;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    hori_offset[0] = epos.view_width - (engine_w >> 1);
    hori_offset[1] = epos.view_height - (engine_h >> 1);
    hori_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 0;
    epos.z = -65536;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    vert_offset[0] = epos.view_width - (engine_w >> 1);
    vert_offset[1] = epos.view_height - (engine_h >> 1);
    vert_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 65536;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    high_offset[0] = epos.view_width - (engine_w >> 1);
    high_offset[1] = epos.view_height - (engine_h >> 1);
    high_offset[2] = epos.z;
}

void find_gamut(void)
{
    SYNCDBG(19,"Starting");
    _DK_find_gamut();
}

void fiddle_half_gamut(long a1, long a2, long a3, long a4)
{
    _DK_fiddle_half_gamut(a1, a2, a3, a4);
}

void fiddle_gamut_find_limits(long *floor_x, long *floor_y, long ewwidth, long ewheight, long ewzoom)
{
    long len_01,len_02,len_13,len_23;
    long tmp_y,tmp_x;
    long i;
    get_floor_pointed_at(ewwidth + ewzoom, -ewzoom, &floor_y[2], &floor_x[2]);
    get_floor_pointed_at(ewwidth + ewzoom, ewheight + ewzoom, &floor_y[1], &floor_x[1]);
    get_floor_pointed_at(-ewzoom, ewheight + ewzoom, &floor_y[0], &floor_x[0]);
    get_floor_pointed_at(-ewzoom, -ewzoom, &floor_y[3], &floor_x[3]);
    // Get the value with lowest X coord into [0]
    for (i=1; i < 4; i++)
    {
        tmp_y = floor_y[i];
        if (floor_y[0] > tmp_y)
        {
          tmp_x = floor_x[i];
          floor_x[i] = floor_x[0];
          floor_x[0] = tmp_x;
          floor_y[i] = floor_y[0];
          floor_y[0] = tmp_y;
        }
    }
    // Get the value with highest X coord into [3]
    for (i=0; i < 3; i++)
    {
        tmp_y = floor_y[i];
        if (floor_y[3] < tmp_y)
        {
          tmp_x = floor_x[i];
          floor_x[i] = floor_x[3];
          floor_x[3] = tmp_x;
          floor_y[i] = floor_y[3];
          floor_y[3] = tmp_y;
        }
    }
    // Between values with medicore X, place the lowest Y first
    if (floor_x[1] > floor_x[2])
    {
        tmp_x = floor_x[1];
        tmp_y = floor_y[1];
        floor_x[1] = floor_x[2];
        floor_x[2] = tmp_x;
        floor_y[1] = floor_y[2];
        floor_y[2] = tmp_y;
    }

    // Lengths of X vectors
    len_01 = abs(floor_y[1] - floor_y[0]);
    len_13 = abs(floor_y[3] - floor_y[1]);
    len_02 = abs(floor_y[2] - floor_y[0]);
    len_23 = abs(floor_y[3] - floor_y[2]);
    // Update points according to both coordinates
    if ( (floor_x[1] > floor_x[0]) && (len_01 < len_13) )
    {
        tmp_x = floor_x[1];
        floor_y[1] = floor_y[0];
        floor_x[1] = floor_x[0];
        floor_x[0] = tmp_x;
    }
    if ( (floor_x[1] > floor_x[3]) && (len_13 < len_01) )
    {
        tmp_x = floor_x[1];
        floor_y[1] = floor_y[3];
        floor_x[1] = floor_x[3];
        floor_x[3] = tmp_x;
    }
    if ( (floor_x[2] < floor_x[0]) && (len_02 < len_23) )
    {
        tmp_x = floor_x[2];
        floor_y[2] = floor_y[0];
        floor_x[2] = floor_x[0];
        floor_x[0] = tmp_x;
    }
    if ( (floor_x[2] < floor_x[3]) && (len_23 < len_02) )
    {
        tmp_x = floor_x[2];
        floor_x[2] = floor_x[3];
        floor_x[3] = tmp_x;
        floor_y[2] = floor_y[3];
    }
}

void fiddle_gamut_set_base(long *floor_x, long *floor_y, long pos_x, long pos_y)
{
    floor_x[0] -= pos_x;
    floor_x[1] -= pos_x;
    floor_y[0] += 32 - pos_y;
    floor_x[2] -= pos_x;
    floor_y[1] += 32 - pos_y;
    floor_y[2] += 32 - pos_y;
    floor_x[3] -= pos_x;
    floor_y[3] += 32 - pos_y;
}

void fiddle_gamut_set_minmaxes(long *floor_x, long *floor_y, long max_tiles)
{
    struct MinMax *mm;
    long mlimit,bormul,bormuh,borinc;
    short bordec;
    long midx;
    midx = 0;
    mlimit = floor_y[0];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        mm->min = 0;
        mm->max = 0;
    }
    if (floor_y[1] <= floor_y[0])
        borinc = floor_x[0];
    else
        borinc = ((floor_x[1] - floor_x[0]) << 16) / (floor_y[1] - floor_y[0]);

    bormul = (floor_x[0] << 16);
    if (floor_y[0] < 0)
        bormul -= floor_y[0] * borinc;

    mlimit = floor_y[1];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16);
        if (bordec < -max_tiles)
            mm->min = -max_tiles;
        else
            mm->min = bordec;
        bormul += borinc;
    }

    bormul = floor_x[1] << 16;
    if (floor_y[1] < floor_y[3])
      borinc = ((floor_x[3] - floor_x[1]) << 16) / (floor_y[3] - floor_y[1]);

    mlimit = floor_y[3];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    if (midx < 0) {
        bormul -= midx * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16);
        if (bordec < -max_tiles)
            mm->min = -max_tiles;
        else
            mm->min = bordec;
        bormul += borinc;
    }
    midx = floor_y[0];
    if (floor_y[2] > floor_y[0])
        borinc = ((floor_x[2] - floor_x[0]) << 16) / (floor_y[2] - floor_y[0]);
    mlimit = floor_y[2];
    if (mlimit > MINMAX_LENGTH-1)
        mlimit = MINMAX_LENGTH-1;
    bormuh = (floor_x[0] << 16);
    if (midx < 0) {
        bormuh -= floor_y[0] * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormuh >> 16) + 1;
        if (bordec > max_tiles)
            mm->max = max_tiles;
        else
            mm->max = bordec;
        bormuh += borinc;
    }

    bormul = floor_x[2] << 16;
    if (floor_y[2] < floor_y[3])
      borinc = ((floor_x[3] - floor_x[2]) << 16) / (floor_y[3] - floor_y[2]);
    mlimit = floor_y[3];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    if ( midx < 0 ) {
        bormul -= midx * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16) + 1;
        if (bordec > max_tiles)
            mm->max = max_tiles;
        else
            mm->max = bordec;
        bormul += borinc;
    }
    for (; midx <= MINMAX_LENGTH-1; midx++)
    {
        mm = &minmaxs[midx];
        mm->min = 0;
        mm->max = 0;
    }
}

/** Prepares limits for tiles to be rendered.
 *
 * @param pos_x
 * @param pos_y
 */
void fiddle_gamut(long pos_x, long pos_y)
{
    struct PlayerInfo *player;
    long ewwidth,ewheight,ewzoom;
    long floor_x[4];
    long floor_y[4];
    player = get_my_player();
    switch (player->view_mode)
    {
    case PVM_CreatureView:
        fiddle_half_gamut(pos_x, pos_y, 1, cells_away);
        fiddle_half_gamut(pos_x, pos_y, -1, cells_away + 2);
        break;
    case PVM_IsometricView:
        // Retrieve coordinates on limiting map points
        ewwidth = player->engine_window_width / pixel_size;
        ewheight = player->engine_window_height / pixel_size - ((8 * high_offset[1]) >> 8);
        ewzoom = (768 * (camera_zoom/pixel_size)) >> 17;
        fiddle_gamut_find_limits(floor_x, floor_y, ewwidth, ewheight, ewzoom);
        // Place the area at proper base coords
        fiddle_gamut_set_base(floor_x, floor_y, pos_x, pos_y);
        fiddle_gamut_set_minmaxes(floor_x, floor_y, 30);
        break;
    }
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
    poly->b.kind = QK_ClippedLine;
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
    poly->b.kind = QK_ClippedLine;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    // Fill parameters
    if (pixel_size > 0)
    {
        poly->p.field_0 = start->view_width;
        poly->p.field_4 = start->view_height;
        poly->p.field_8 = end->view_width;
        poly->p.field_C = end->view_height;
    }
    poly->p.field_10 = color;
}

void create_line_const_xz(long pos_x, long pos_z, long start_y, long end_y)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_y;
    create_box_coords(&start, pos_x, start_y, pos_z);
    for (pos_y = start_y+256; pos_y <= end_y; pos_y+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

void create_line_const_xy(long pos_x, long pos_y, long start_z, long end_z)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_z;
    create_box_coords(&start, pos_x, pos_y, start_z);
    for (pos_z = start_z+256; pos_z <= end_z; pos_z+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

void create_line_const_yz(long pos_y, long pos_z, long start_x, long end_x)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_x;
    create_box_coords(&start, start_x, pos_y, pos_z);
    for (pos_x = start_x+256; pos_x <= end_x; pos_x+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

void create_map_volume_box(long x, long y, long z)
{
    long box_xs,box_xe;
    long box_ys,box_ye;
    long box_zs,box_ze;
    long i;

    box_xs = map_volume_box.field_3 - x;
    box_ys = y - map_volume_box.field_7;
    box_ye = y - map_volume_box.field_F;
    box_xe = map_volume_box.field_B - x;

    if ( temp_cluedo_mode )
        box_ze = 512 - z;
    else
        box_ze = 1280 - z;

    box_zs = (map_volume_box.field_13 << 8) - z;
    if ( box_zs >= box_ze )
      box_zs = box_ze;

    if ( box_xe < box_xs )
    {
        i = map_volume_box.field_3;
        box_xs = map_volume_box.field_B - x;
        box_xe = map_volume_box.field_3 - x;
        map_volume_box.field_3 = map_volume_box.field_B;
        map_volume_box.field_B = i;
    }

    if ( box_ye < box_ys )
    {
        i = map_volume_box.field_7;
        box_ys = y - map_volume_box.field_F;
        box_ye = y - map_volume_box.field_7;
        map_volume_box.field_7 = map_volume_box.field_F;
        map_volume_box.field_F = i;
    }

    // Draw top rectangle
    create_line_const_yz(box_ye, box_zs, box_xs, box_xe);
    create_line_const_yz(box_ys, box_zs, box_xs, box_xe);
    create_line_const_xz(box_xs, box_zs, box_ys, box_ye);
    create_line_const_xz(box_xe, box_zs, box_ys, box_ye);
    // Vertical lines which connect the rectangles
    create_line_const_xy(box_xs, box_ys, box_zs, box_ze);
    create_line_const_xy(box_xe, box_ys, box_zs, box_ze);
    create_line_const_xy(box_xe, box_ye, box_zs, box_ze);
    create_line_const_xy(box_xs, box_ye, box_zs, box_ze);
    // Bottom rectangle
    create_line_const_yz(box_ye, box_ze, box_xs, box_xe);
    create_line_const_yz(box_ys, box_ze, box_xs, box_xe);
    create_line_const_xz(box_xs, box_ze, box_ys, box_ye);
    create_line_const_xz(box_xe, box_ze, box_ys, box_ye);
}

void do_a_trig_gourad_tr(struct EngineCoord *ep1, struct EngineCoord *ep2, struct EngineCoord *ep3, short a4, long a5)
{
    _DK_do_a_trig_gourad_tr(ep1, ep2, ep3, a4, a5);
}

void do_a_trig_gourad_bl(struct EngineCoord *ep1, struct EngineCoord *ep2, struct EngineCoord *ep3, short a4, long a5)
{
    _DK_do_a_trig_gourad_bl(ep1, ep2, ep3, a4, a5);
}

void do_map_who(short a1)
{
    _DK_do_map_who(a1);
}

void do_a_plane_of_engine_columns_perspective(long stl_x, long stl_y, long plane_start, long plane_end)
{
    struct Column *blank_colmn;
    struct Column *colmn;
    struct Map *mapblk;
    struct Map *sib_mapblk;
    struct Column *sib_colmn;
    unsigned short textr_idx,height_bit;
    unsigned long center_block_idx;
    long fepos,bepos,ecpos;
    long clip_start,clip_end;
    struct CubeAttribs *texturing;
    unsigned short *cubenum_ptr;
    long i,n;
    if ((stl_y <= 0) || (stl_y >= 255))
        return;
    clip_start = plane_start;
    if (stl_x + plane_start < 1)
        clip_start = 1 - stl_x;
    clip_end = plane_end;
    if (stl_x + plane_end > 255)
        clip_end = 255 - stl_x;
    struct EngineCol *bec;
    struct EngineCol *fec;
    bec = &back_ec[clip_start + 31];
    fec = &front_ec[clip_start + 31];
    blank_colmn = get_column(game.unrevealed_column_idx);
    center_block_idx = clip_start + stl_x + (stl_y << 8);
    for (i = clip_end-clip_start; i > 0; i--)
    {
        mapblk = get_map_block_at_pos(center_block_idx);
        colmn = blank_colmn;
        if (map_block_revealed_bit(mapblk, player_bit) )
        {
            n = get_mapwho_thing_index(mapblk);
            if (n != 0)
                do_map_who(n);
            colmn = get_map_column(mapblk);
        }
        // Retrieve solidmasks for surrounding area
        unsigned short solidmsk_center,solidmsk_top,solidmsk_bottom,solidmsk_left,solidmsk_right;
        solidmsk_center = colmn->solidmask;
        solidmsk_top = blank_colmn->solidmask;
        solidmsk_right = blank_colmn->solidmask;
        solidmsk_bottom = blank_colmn->solidmask;
        solidmsk_left = blank_colmn->solidmask;
        sib_mapblk = get_map_block_at_pos(center_block_idx-256);
        if (map_block_revealed_bit(sib_mapblk, player_bit) ) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_top = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx+256);
        if (map_block_revealed_bit(sib_mapblk, player_bit) ) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_bottom = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx-1);
        if (map_block_revealed_bit(sib_mapblk, player_bit) ) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_left = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx+1);
        if (map_block_revealed_bit(sib_mapblk, player_bit) ) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_right = sib_colmn->solidmask;
        }
        bepos = 0;
        fepos = 0;
        cubenum_ptr = &colmn->cubes[0];
        height_bit = 1;
        while (height_bit <= solidmsk_center)
        {
            texturing = &game.cubes_data[*cubenum_ptr];
            if ((solidmsk_center & height_bit) != 0)
            {
              if ((solidmsk_top & height_bit) == 0)
              {
                  textr_idx = texturing->texture_id[sideoris[0].field_0];
                  do_a_trig_gourad_tr(&bec[1].cors[bepos+1], &bec[0].cors[bepos+1], &bec[0].cors[bepos],   textr_idx, normal_shade_back);
                  do_a_trig_gourad_bl(&bec[0].cors[bepos],   &bec[1].cors[bepos],   &bec[1].cors[bepos+1], textr_idx, normal_shade_back);
              }
              if ((solidmsk_bottom & height_bit) == 0)
              {
                  textr_idx = texturing->texture_id[sideoris[0].field_2];
                  do_a_trig_gourad_tr(&fec[0].cors[fepos+1], &fec[1].cors[fepos+1], &fec[1].cors[fepos],   textr_idx, normal_shade_front);
                  do_a_trig_gourad_bl(&fec[1].cors[fepos],   &fec[0].cors[fepos],   &fec[0].cors[fepos+1], textr_idx, normal_shade_front);
              }
              if ((solidmsk_left & height_bit) == 0)
              {
                  textr_idx = texturing->texture_id[sideoris[0].field_3];
                  do_a_trig_gourad_tr(&bec[0].cors[bepos+1], &fec[0].cors[fepos+1], &fec[0].cors[fepos],   textr_idx, normal_shade_left);
                  do_a_trig_gourad_bl(&fec[0].cors[fepos],   &bec[0].cors[bepos],   &bec[0].cors[bepos+1], textr_idx, normal_shade_left);
              }
              if ((solidmsk_right & height_bit) == 0)
              {
                  textr_idx = texturing->texture_id[sideoris[0].field_1];
                  do_a_trig_gourad_tr(&fec[1].cors[fepos+1], &bec[1].cors[bepos+1], &bec[1].cors[bepos],   textr_idx, normal_shade_right);
                  do_a_trig_gourad_bl(&bec[1].cors[bepos],   &fec[1].cors[fepos],   &fec[1].cors[fepos+1], textr_idx, normal_shade_right);
              }
            }
            bepos++; fepos++;
            cubenum_ptr++;
            height_bit = height_bit << 1;
        }

        ecpos = floor_height[solidmsk_center];
        if (ecpos > 0)
        {
            cubenum_ptr = &colmn->cubes[ecpos-1];
            texturing = &game.cubes_data[*cubenum_ptr];
            textr_idx = texturing->texture_id[4];
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);
        } else
        {
            ecpos = 0;
            textr_idx = colmn->baseblock;
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);
        }
        // For tiles which have solid columns at top, draw them
        ecpos = lintel_top_height[solidmsk_center];
        if (ecpos > 0)
        {
            cubenum_ptr = &colmn->cubes[ecpos-1];
            texturing = &game.cubes_data[*cubenum_ptr];
            textr_idx = texturing->texture_id[4];
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);

            ecpos =  lintel_bottom_height[solidmsk_center];
            textr_idx = texturing->texture_id[5];
            do_a_trig_gourad_tr(&fec[0].cors[ecpos], &fec[1].cors[ecpos], &bec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&bec[1].cors[ecpos], &bec[0].cors[ecpos], &fec[0].cors[ecpos], textr_idx, -1);
        }
        // Draw the universal ceiling on top of the columns
        ecpos = 8;
        {
            textr_idx = floor_to_ceiling_map[colmn->baseblock];
            do_a_trig_gourad_tr(&fec[0].cors[ecpos], &fec[1].cors[ecpos], &bec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&bec[1].cors[ecpos], &bec[0].cors[ecpos], &fec[0].cors[ecpos], textr_idx, -1);
        }
        bec++;
        fec++;
        center_block_idx++;
    }
}

long do_a_plane_of_engine_columns_sub5(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3)
{
    return _DK_do_a_plane_of_engine_columns_sub5(ec1, ec2, ec3);
}

void do_a_gpoly_gourad_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short a4, int a5)
{
    _DK_do_a_gpoly_gourad_tr(ec1, ec2, ec3, a4, a5); return;
}

void do_a_gpoly_unlit_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short a4)
{
    _DK_do_a_gpoly_unlit_tr(ec1, ec2, ec3, a4); return;
}

void do_a_gpoly_unlit_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short a4)
{
    _DK_do_a_gpoly_unlit_bl(ec1, ec2, ec3, a4); return;
}

void do_a_gpoly_gourad_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short a4, int a5)
{
    _DK_do_a_gpoly_gourad_bl(ec1, ec2, ec3, a4, a5); return;
}

void do_a_plane_of_engine_columns_cluedo(long stl_x, long stl_y, long plane_start, long plane_end)
{
    //_DK_do_a_plane_of_engine_columns_cluedo(stl_x, stl_y, plane_start, plane_end); return;

    if ((stl_y < 1) || (stl_y > 254)) {
        return;
    }
    long xaval, xbval;
    xaval = plane_start;
    if (stl_x + plane_start < 1) {
        xaval = 1 - stl_x;
    }
    xbval = plane_end;
    if (stl_x + plane_end > 255) {
        xbval = 255 - stl_x;
    }
    int xidx, xdelta;
    xdelta = xbval - xaval;
    const struct Column *unrev_colmn;
    unrev_colmn = get_column(game.unrevealed_column_idx);
    for (xidx=0; xidx < xdelta; xidx++)
    {
        struct Map *cur_mapblk;
        cur_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y);
        // Get solidmasks of sibling columns
        unsigned short solidmsk_cur_raw, solidmsk_cur, solidmsk_back, solidmsk_front, solidmsk_left, solidmsk_right;
        solidmsk_cur_raw = unrev_colmn->solidmask;
        solidmsk_cur = unrev_colmn->solidmask & 3;
        solidmsk_back = unrev_colmn->solidmask & 3;
        solidmsk_right = unrev_colmn->solidmask & 3;
        solidmsk_front = unrev_colmn->solidmask & 3;
        solidmsk_left = unrev_colmn->solidmask & 3;
        // Get column to be drawn
        const struct Column *cur_colmn;
        cur_colmn = unrev_colmn;
        if (map_block_revealed_bit(cur_mapblk, player_bit))
        {
            long i;
            i = get_mapwho_thing_index(cur_mapblk);
            if (i > 0) {
              do_map_who(i);
            }
            cur_colmn = get_map_column(cur_mapblk);
            solidmsk_cur_raw = cur_colmn->solidmask;
            solidmsk_cur = solidmsk_cur_raw;
            if (solidmsk_cur >= (1<<3))
            {
                if (((cur_mapblk->flags & (0x40|0x02)) == 0) && ((cur_colmn->bitfields & 0xE) == 0)) {
                    solidmsk_cur &= 3;
                }
            }
        }
        struct Map *sib_mapblk;
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y - 1);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_back = colmn->solidmask;
            if (solidmsk_back >= (1<<3))
            {
                if (((sib_mapblk->flags & (0x40|0x02)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_back &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y + 1);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_front = colmn->solidmask;
            if (solidmsk_front >= (1<<3))
            {
                if (((sib_mapblk->flags & (0x40|0x02)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_front &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx - 1, stl_y);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_left = colmn->solidmask;
            if (solidmsk_left >= (1<<3))
            {
                if (((sib_mapblk->flags & (0x40|0x02)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_left &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx + 1, stl_y);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_right = colmn->solidmask;
            if (solidmsk_right >= (1<<3))
            {
                if (((sib_mapblk->flags & (0x40|0x02)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_right &= 3;
                }
            }
        }

        struct EngineCol *bec;
        struct EngineCol *fec;
        bec = &back_ec[xaval + 31 + xidx];
        fec = &front_ec[xaval + 31 + xidx];
        unsigned short mask;
        int ncor;
        for (mask=1,ncor=0; mask <= solidmsk_cur; mask*=2,ncor++)
        {
            unsigned short textr_id;
            struct CubeAttribs *cubed;
            cubed = &game.cubes_data[cur_colmn->cubes[ncor]];
            if ((mask & solidmsk_cur) == 0)
            {
                continue;
            }
            if ((mask & solidmsk_back) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_0];
                do_a_gpoly_gourad_tr(&bec[1].cors[ncor+1], &bec[0].cors[ncor+1], &bec[0].cors[ncor],   textr_id, normal_shade_back);
                do_a_gpoly_gourad_bl(&bec[0].cors[ncor],   &bec[1].cors[ncor],   &bec[1].cors[ncor+1], textr_id, normal_shade_back);
            }
            if ((solidmsk_front & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_2];
                do_a_gpoly_gourad_tr(&fec[0].cors[ncor+1], &fec[1].cors[ncor+1], &fec[1].cors[ncor],   textr_id, normal_shade_front);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor],   &fec[0].cors[ncor],   &fec[0].cors[ncor+1], textr_id, normal_shade_front);
            }
            if ((solidmsk_left & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_3];
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor+1], &fec[0].cors[ncor+1], &fec[0].cors[ncor],   textr_id, normal_shade_left);
                do_a_gpoly_gourad_bl(&fec[0].cors[ncor],   &bec[0].cors[ncor],   &bec[0].cors[ncor+1], textr_id, normal_shade_left);
            }
            if ((solidmsk_right & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_1];
                do_a_gpoly_gourad_tr(&fec[1].cors[ncor+1], &bec[1].cors[ncor+1], &bec[1].cors[ncor],   textr_id, normal_shade_right);
                do_a_gpoly_gourad_bl(&bec[1].cors[ncor],   &fec[1].cors[ncor],   &fec[1].cors[ncor+1], textr_id, normal_shade_right);
            }
        }

        ncor = floor_height[solidmsk_cur];
        if ((ncor > 0) && (ncor <= COLUMN_STACK_HEIGHT))
        {
            int ncor_raw;
            ncor_raw = floor_height[solidmsk_cur_raw];
            if ((cur_mapblk->flags & (0x80|0x04)) == 0)
            {
                if ((ncor_raw > 0) && (ncor_raw <= COLUMN_STACK_HEIGHT))
                {
                    struct CubeAttribs * cubed;
                    cubed = &game.cubes_data[cur_colmn->cubes[ncor_raw-1]];
                    do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], cubed->texture_id[4], -1);
                    do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], cubed->texture_id[4], -1);
                }
            } else
            if ((cur_mapblk->flags & 0x01) != 0)
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], 579);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], 579);
            } else
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], 578);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], 578);
            }
        } else
        {
            if ((cur_mapblk->flags & 0x04) == 0)
            {
                do_a_gpoly_gourad_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], cur_colmn->baseblock, -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], cur_colmn->baseblock, -1);
            } else
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], 578);
                do_a_gpoly_unlit_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], 578);
            }
        }
        ncor = lintel_top_height[solidmsk_cur];
        if ((ncor > 0) && (ncor <= COLUMN_STACK_HEIGHT))
        {
            struct CubeAttribs * cubed;
            cubed = &game.cubes_data[cur_colmn->cubes[ncor-1]];
            do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], cubed->texture_id[4], -1);
            do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], cubed->texture_id[4], -1);
        }
    }
}

void do_a_plane_of_engine_columns_isometric(long stl_x, long stl_y, long plane_start, long plane_end)
{
    //_DK_do_a_plane_of_engine_columns_isometric(stl_x, stl_y, plane_start, plane_end); return;
    if ((stl_y < 1) || (stl_y > 254)) {
        return;
    }
    long xaval, xbval;
    TbBool xaclip, xbclip;
    xaval = plane_start;
    xaclip = 0;
    xbclip = 0;
    if (stl_x + plane_start < 1) {
        xaclip = 1;
        xaval = 1 - stl_x;
    }
    xbval = plane_end;
    if (stl_x + plane_end > map_subtiles_x) {
        xbclip = 1;
        xbval = map_subtiles_x - stl_x;
    }
    int xidx, xdelta;
    xdelta = xbval - xaval;
    const struct Column *unrev_colmn;
    unrev_colmn = get_column(game.unrevealed_column_idx);
    for (xidx=0; xidx < xdelta; xidx++)
    {
        struct Map *cur_mapblk;
        cur_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y);
        // Get column to be drawn
        const struct Column *cur_colmn;
        cur_colmn = unrev_colmn;
        if (map_block_revealed_bit(cur_mapblk, player_bit))
        {
            long i;
            i = get_mapwho_thing_index(cur_mapblk);
            if (i > 0) {
              do_map_who(i);
            }
            cur_colmn = get_map_column(cur_mapblk);
        }
        // Get solidmasks of sibling columns
        unsigned short solidmsk_cur, solidmsk_back, solidmsk_front, solidmsk_left, solidmsk_right;
        solidmsk_cur = cur_colmn->solidmask;
        solidmsk_back = unrev_colmn->solidmask;
        solidmsk_right = unrev_colmn->solidmask;
        solidmsk_front = unrev_colmn->solidmask;
        solidmsk_left = unrev_colmn->solidmask;
        struct Map *sib_mapblk;
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y - 1);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_back = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y + 1);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_front = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx - 1, stl_y);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_left = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx + 1, stl_y);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_right = colmn->solidmask;
        }
        if ( xaclip || xbclip || (stl_y <= 1) || (stl_y >= 254))
        {
            if (xaclip && (xidx == 0)) {
                solidmsk_left = 0;
            }
            if (xbclip && (xdelta - xidx == 1)) {
                solidmsk_right = 0;
            }
            if (stl_y <= 1) {
                solidmsk_back = 0;
            }
            if (stl_y >= 254) {
                solidmsk_front = 0;
            }
        }

        struct EngineCol *bec;
        struct EngineCol *fec;
        bec = &back_ec[xaval + 31 + xidx];
        fec = &front_ec[xaval + 31 + xidx];
        unsigned short mask;
        int ncor;
        for (mask=1,ncor=0; mask <= solidmsk_cur; mask*=2,ncor++)
        {
            unsigned short textr_id;
            struct CubeAttribs *cubed;
            cubed = &game.cubes_data[cur_colmn->cubes[ncor]];
            if ((mask & solidmsk_cur) == 0)
            {
                continue;
            }
            if ((mask & solidmsk_back) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_0];
                do_a_gpoly_gourad_tr(&bec[1].cors[ncor+1], &bec[0].cors[ncor+1], &bec[0].cors[ncor],   textr_id, normal_shade_back);
                do_a_gpoly_gourad_bl(&bec[0].cors[ncor],   &bec[1].cors[ncor],   &bec[1].cors[ncor+1], textr_id, normal_shade_back);
            }
            if ((solidmsk_front & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_2];
                do_a_gpoly_gourad_tr(&fec[0].cors[ncor+1], &fec[1].cors[ncor+1], &fec[1].cors[ncor],   textr_id, normal_shade_front);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor],   &fec[0].cors[ncor],   &fec[0].cors[ncor+1], textr_id, normal_shade_front);
            }
            if ((solidmsk_left & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_3];
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor+1], &fec[0].cors[ncor+1], &fec[0].cors[ncor],   textr_id, normal_shade_left);
                do_a_gpoly_gourad_bl(&fec[0].cors[ncor],   &bec[0].cors[ncor],   &bec[0].cors[ncor+1], textr_id, normal_shade_left);
            }
            if ((solidmsk_right & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_1];
                do_a_gpoly_gourad_tr(&fec[1].cors[ncor+1], &bec[1].cors[ncor+1], &bec[1].cors[ncor],   textr_id, normal_shade_right);
                do_a_gpoly_gourad_bl(&bec[1].cors[ncor],   &fec[1].cors[ncor],   &fec[1].cors[ncor+1], textr_id, normal_shade_right);
            }
        }

        ncor = floor_height[solidmsk_cur];
        if (ncor > 0)
        {
            if ((cur_mapblk->flags & (0x80|0x04)) == 0)
            {
                struct CubeAttribs * cubed;
                cubed = &game.cubes_data[*(short *)((char *)&cur_colmn->baseblock + 2 * ncor + 1)];
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], cubed->texture_id[4], -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], cubed->texture_id[4], -1);
            } else
            if ((cur_mapblk->flags & 0x01) != 0)
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], 579);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], 579);
            } else
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], 578);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], 578);
            }
        } else
        {
            if ((cur_mapblk->flags & 0x04) == 0)
            {
                do_a_gpoly_gourad_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], cur_colmn->baseblock, -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], cur_colmn->baseblock, -1);
            } else
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], 578);
                do_a_gpoly_unlit_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], 578);
            }
        }
        ncor = lintel_top_height[solidmsk_cur];
        if (ncor > 0)
        {
            struct CubeAttribs * cubed;
            cubed = &game.cubes_data[*(short *)((char *)&cur_colmn->baseblock + 2 * ncor + 1)];
            do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], cubed->texture_id[4], -1);
            do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], cubed->texture_id[4], -1);
        }
    }
}

void draw_map_volume_box(long cor1_x, long cor1_y, long cor2_x, long cor2_y, long a5, unsigned char color)
{
    map_volume_box.visible = 1;
    map_volume_box.field_3 = cor1_x & 0xFFFF00;
    map_volume_box.field_7 = cor1_y & 0xFF00;
    map_volume_box.field_B = cor2_x & 0xFFFF00;
    map_volume_box.field_13 = a5;
    map_volume_box.field_F = cor2_y & 0xFFFF00;
    map_volume_box.color = color;
}

void draw_fastview_mapwho(struct Camera *cam, struct JontySpr *spr)
{
    _DK_draw_fastview_mapwho(cam, spr);
}

void draw_engine_number(struct Number *num)
{
    struct PlayerInfo *player;
    unsigned short flg_mem;
    struct TbSprite *spr;
    long val,ndigits;
    long w,h,pos_x;
    flg_mem = lbDisplay.DrawFlags;
    player = get_my_player();
    lbDisplay.DrawFlags &= ~Lb_SPRITE_ONECOLOUR1;
    spr = &button_sprite[71];
    w = spr->SWidth;
    h = spr->SHeight;
    if ((player->acamera->field_6 == 2) || (player->acamera->field_6 == 5))
    {
        // Count digits to be displayed
        ndigits=0;
        for (val = num->lvl; val > 0; val /= 10)
            ndigits++;
        if (ndigits > 0)
        {
            // Show the digits
            pos_x = w*(ndigits-1)/2 + num->x;
            for (val = num->lvl; val > 0; val /= 10)
            {
                spr = &button_sprite[(val%10) + 71];
                LbSpriteDrawScaled(pos_x, num->y - h, spr, w, h);
                pos_x -= w;
            }
        }
    }
    lbDisplay.DrawFlags = flg_mem;
}

void draw_engine_room_flagpole(struct RoomFlag *rflg)
{
    _DK_draw_engine_room_flagpole(rflg);
}

/**
 * Selects index of a sprite used to show creature health flower.
 * @param thing
 */
unsigned short choose_health_sprite(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    long health, maxhealth;
    int color_idx;
    health = thing->health;
    maxhealth = compute_creature_max_health(crstat->health,cctrl->explevel);
    color_idx = (thing->owner % 5);
    if (is_neutral_thing(thing)) {
        color_idx = game.play_gameturn & 3;
    }
    if ((maxhealth <= 0) || (health <= 0))
    {
        return 88 + (8*color_idx);
    } else
    if (health >= maxhealth)
    {
        return 88 + (8*color_idx) - 7;
    } else
    {
        return 88 + (8*color_idx) - (8 * health / maxhealth);
    }
}

void draw_status_sprites(long scrpos_x, long scrpos_y, struct Thing *thing, long a4)
{
    struct CreatureControl *cctrl;
    struct PlayerInfo *myplyr;
    struct Camera *mycam;
    unsigned short flg_mem;
    //_DK_draw_status_sprites(a1, a2, thing, a4);
    myplyr = get_my_player();

    CrtrExpLevel exp;
    short health_spridx,v31h;
    signed short anger_spridx;
    int h_add;

    flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = 0;
    anger_spridx = 0;
    h_add = 0;
    health_spridx = 0;
    v31h = 0;
    cctrl = creature_control_get_from_thing(thing);
    if ((game.flags_cd & 0x80) != 0)
    {
      if ( myplyr->thing_under_hand != thing->index )
      {
        cctrl->field_43 = game.play_gameturn;
        return;
      }
      cctrl->field_47 = 40;
    }
    exp = min(cctrl->explevel,9);
    mycam = myplyr->acamera;
    if ((mycam->field_6 == 2) || (mycam->field_6 == 5))
    {
      health_spridx = choose_health_sprite(thing);
      if (is_my_player_number(thing->owner))
      {
        lbDisplay.DrawFlags |= 0x04;
        cctrl = creature_control_get_from_thing(thing);
        if (cctrl->field_43 - game.play_gameturn != -1)
        {
            cctrl->field_47 = 0;
        } else
        if (cctrl->field_47 < 40)
        {
            cctrl->field_47++;
        }
        cctrl->field_43 = game.play_gameturn;
        if (cctrl->field_47 == 40)
        {
            struct StateInfo *stati;
            stati = get_creature_state_with_task_completion(thing);
            if ( !stati->field_23 )
            {
                if (anger_is_creature_livid(thing))
                {
                    stati = &states[CrSt_CreatureLeavingDungeon];
                } else
                if (creature_is_called_to_arms(thing))
                {
                    stati = &states[CrSt_ArriveAtCallToArms];
                } else
                if (creature_is_at_alarm(thing))
                {
                    stati = &states[CrSt_ArriveAtAlarm];
                } else
                if ( anger_is_creature_angry(thing) )
                {
                    stati = &states[CrSt_PersonSulkAtLair];
                } else
                if (hunger_is_creature_hungry(thing))
                {
                    stati = &states[CrSt_CreatureArrivedAtGarden];
                } else
                if (creature_requires_healing(thing))
                {
                    stati = &states[CrSt_CreatureSleep];
                } else
                if (cctrl->field_48)
                {
                    stati = &states[CrSt_CreatureWantsSalary];
                } else
                {
                    stati = get_creature_state_with_task_completion(thing);
                }
                if ((*(short *)&stati->field_26 == 1) || (thing_pointed_at == thing))
                  v31h = stati->field_24;
                switch ( anger_get_creature_anger_type(thing) )
                {
                case 1:
                    anger_spridx = 52;
                    break;
                case 2:
                    anger_spridx = 59;
                    break;
                case 3:
                    anger_spridx = 54;
                    break;
                case 4:
                    anger_spridx = 55;
                    break;
                default:
                    break;
                }
            }
        }
      }
    }
    int w, h;
    struct TbSprite *spr;
    if ( v31h || anger_spridx )
    {
        spr = &button_sprite[70];
        h = (pixel_size * a4 * spr->SHeight) >> 13;
        w = (pixel_size * a4 * spr->SWidth) >> 13;
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
    } else
    {
      h = 0;
    }
    lbDisplay.DrawFlags &= ~0x08;
    lbDisplay.DrawFlags &= ~0x04;
    if (((game.play_gameturn & 4) == 0) && (anger_spridx > 0))
    {
        spr = &button_sprite[anger_spridx];
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
        spr = &button_sprite[v31h];
        h_add = spr->SHeight;
    } else
    if ( v31h )
    {
        spr = &button_sprite[v31h];
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
        h_add = spr->SHeight;
    }
    if ((thing->word_17 > 0) && (health_spridx > 0) && ((game.play_gameturn & 1) != 0))
    {
        spr = &button_sprite[health_spridx];
        h = pixel_size * spr->SHeight;
        w = pixel_size * spr->SWidth;
        if (is_neutral_thing(thing))
        {
            LbSpriteDrawOneColour(scrpos_x - w / pixel_size / 2, scrpos_y - h / pixel_size - h_add, spr, player_flash_colours[game.play_gameturn & 3]);
        } else
        {
            LbSpriteDrawOneColour(scrpos_x - w / pixel_size / 2, scrpos_y - h / pixel_size - h_add, spr, player_flash_colours[thing->owner]);
        }
    }
    else
    {
      if ( (myplyr->thing_under_hand == thing->index)
        || ((myplyr->id_number != thing->owner) && !creature_is_invisible(thing))
        || (cctrl->combat_flags != 0)
        || (thing->word_17 > 0)
        || (mycam->field_6 == 3) )
      {
          spr = &button_sprite[health_spridx];
          w = pixel_size * spr->SWidth;
          h = pixel_size * spr->SHeight;
          LbSpriteDraw(scrpos_x - w / pixel_size / 2, scrpos_y - h / pixel_size - h_add, spr);
          spr = &button_sprite[184 + exp];
          LbSpriteDraw(scrpos_x - w / pixel_size / 2, scrpos_y - h / pixel_size - h_add, spr);
      }
    }
    lbDisplay.DrawFlags = flg_mem;
}

void draw_iso_only_fastview_mapwho(struct Camera *cam, struct JontySpr *spr)
{
    if (cam->field_6 == 5)
      draw_fastview_mapwho(cam, spr);
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
    // empty
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
        vec_mode = VM_Unknown5;
        draw_gpoly(&unk09->p1,&unk09->p2,&unk09->p3);
        break;
    case 1:
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown5;
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
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        trig(&unk09->p1, &unk09->p2, &unk09->p3);
        break;
    case 13:
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
        vec_mode = VM_Unknown7;
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
      struct KeeperSpr *keepSpr;
      struct BasicUnk13 *unk13;
      struct BasicUnk14 *unk14;
      struct TexturedQuad *txquad;
      struct Number *number;
      struct RoomFlag *roomFlg;
    } item;
    long bucket_num;
    struct PolyPoint point_a,point_b,point_c;
    SYNCDBG(9,"Starting");
    // Color rendering array pointers used by draw_keepersprite()
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_alpha = (unsigned char *)&alpha_sprite_table;
    render_problems = 0;
    thing_pointed_at = 0;
    for (bucket_num = BUCKETS_COUNT-1; bucket_num > 0; bucket_num--)
    {
      for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
      {
        //JUSTLOG("%d",(int)item.b->kind);
        switch ( item.b->kind )
        {
        case QK_PolyTriangle:
          vec_mode = VM_Unknown5;
          vec_map = block_ptrs[item.unk00->block];
          draw_gpoly(&item.unk00->p1, &item.unk00->p2, &item.unk00->p3);
          break;
        case QK_PolyTriangleSimp:
          vec_mode = VM_Unknown7;
          vec_colour = ((item.unk01->p3.field_10 + item.unk01->p2.field_10 + item.unk01->p1.field_10)/3) >> 16;
          vec_map = block_ptrs[item.unk01->block];
          trig(&item.unk01->p1, &item.unk01->p2, &item.unk01->p3);
          break;
        case QK_PolyMode0:
          vec_mode = VM_Unknown0;
          vec_colour = item.unk02->colour;
          point_a.field_0 = item.unk02->x1;
          point_a.field_4 = item.unk02->y1;
          point_b.field_0 = item.unk02->x2;
          point_b.field_4 = item.unk02->y2;
          point_c.field_0 = item.unk02->x3;
          point_c.field_4 = item.unk02->y3;
          draw_gpoly(&point_a, &point_b, &point_c);
          break;
        case QK_PolyMode4:
          vec_mode = VM_Unknown4;
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
        case QK_TrigMode2:
          vec_mode = VM_Unknown2;
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
        case QK_PolyMode5:
          vec_mode = VM_Unknown5;
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
        case QK_TrigMode3:
          vec_mode = VM_Unknown3;
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
        case QK_TrigMode6:
          vec_mode = VM_Unknown6;
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
        case QK_RotableSprite:
          draw_map_who(item.rotSpr);
          break;
        case QK_Unknown9:
          draw_unkn09(item.unk09);
          break;
        case QK_Unknown10:
          vec_mode = VM_Unknown0;
          vec_colour = item.unk10->field_6;
          draw_gpoly(&item.unk10->p1, &item.unk10->p2, &item.unk10->p3);
          break;
        case QK_JontySprite:
          draw_jonty_mapwho(item.jonSpr);
          break;
        case QK_KeeperSprite:
          draw_keepsprite_unscaled_in_buffer(item.keepSpr->field_5C, item.keepSpr->field_58, item.keepSpr->field_5E, scratch);
          vec_map = scratch;
          vec_mode = VM_Unknown10;
          vec_colour = item.keepSpr->p1.field_10;
          trig(&item.keepSpr->p1, &item.keepSpr->p2, &item.keepSpr->p3);
          trig(&item.keepSpr->p1, &item.keepSpr->p3, &item.keepSpr->p4);
          break;
        case QK_ClippedLine:
          draw_clipped_line(item.unk13->p.field_0,item.unk13->p.field_4,item.unk13->p.field_8,item.unk13->p.field_C,item.unk13->p.field_10);
          break;
        case QK_StatusSprites:
          player = get_my_player();
          cam = player->acamera;
          if (cam != NULL)
          {
              if ((cam->field_6 == 2) || (cam->field_6 == 5)) {
                  draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, camera_zoom/pixel_size);
              }
          }
          break;
        case QK_IntegerValue:
          draw_engine_number(item.number);
          break;
        case QK_RoomFlagPole:
          draw_engine_room_flagpole(item.roomFlg);
          break;
        case QK_JontyISOSprite:
          player = get_my_player();
          cam = player->acamera;
          if (cam != NULL)
          {
            if (cam->field_6 == 2)
              draw_jonty_mapwho(item.jonSpr);
          }
          break;
        case QK_RoomFlagTop:
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

void prepare_draw_plane_of_engine_columns(long aposc, long bposc, long xcell, long ycell, struct MinMax *mm)
{
    apos = aposc;
    bpos = bposc;
    back_ec = &ecs1[0];
    front_ec = &ecs2[0];
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
}

/**
 * Draws single plane of engine columns.
 *
 * @param aposc
 * @param bposc
 * @param xcell
 * @param ycell
 */
void draw_plane_of_engine_columns(long aposc, long bposc, long xcell, long ycell, struct MinMax *mm)
{
    struct EngineCol *ec;
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

/**
 * Draws rectangular area of engine columns.
 * @param aposc
 * @param bposc
 * @param xcell
 * @param ycell
 */
void draw_view_map_plane(long aposc, long bposc, long xcell, long ycell)
{
    struct MinMax *mm;
    long i;
    i = 31-cells_away;
    if (i < 0)
        i = 0;
    mm = &minmaxs[i];
    prepare_draw_plane_of_engine_columns(aposc, bposc, xcell, ycell, mm);
    for (i = 2*cells_away-1; i > 0; i--)
    {
        ycell++;
        bposc -= (map_subtiles_y+1);
        mm++;
        draw_plane_of_engine_columns(aposc, bposc, xcell, ycell, mm);
    }
}

void draw_view(struct Camera *cam, unsigned char a2)
{
    long zoom_mem;
    long x,y,z;
    long xcell,ycell;
    long i;
    long aposc,bposc;
    SYNCDBG(9,"Starting");
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    zoom_mem = cam->zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    cam->zoom = camera_zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    getpoly = poly_pool;
    LbMemorySet(buckets, 0, sizeof(buckets));
    LbMemorySet(poly_pool, 0, sizeof(poly_pool));
    if (map_volume_box.visible) {
        poly_pool_end_reserve(14);
    } else {
        poly_pool_end_reserve(4);
    }
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
        setup_rotate_stuff(x, y, z, fade_max, fade_min, camera_zoom/pixel_size, map_angle, map_roll);
        do_perspective_rotation(x, y, z);
        cells_away = compute_cells_away();
    }
    xcell = (x >> 8);
    aposc = -(x & 0xFF);
    bposc = (cells_away << 8) + (y & 0xFF);
    ycell = (y >> 8) - (cells_away+1);
    find_gamut();
    fiddle_gamut(xcell, ycell + (cells_away+1));
    draw_view_map_plane(aposc, bposc, xcell, ycell);
    if (map_volume_box.visible) {
        poly_pool_end_reserve(0);
        create_map_volume_box(x, y, z);
    }
    cam->zoom = zoom_mem;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
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
        vec_mode = VM_Unknown5;
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
            gtb.field_C = txquad->field_1A >> 16;
            gtb.field_10 = txquad->field_1E >> 16;
            gtb.field_18 = txquad->field_22 >> 16;
            gtb.field_14 = txquad->field_26 >> 16;
            break;
        case 1:
            gtb.field_0 = block_ptrs[579];
            gtb.field_C = txquad->field_1A >> 16;
            gtb.field_10 = txquad->field_1E >> 16;
            gtb.field_18 = txquad->field_22 >> 16;
            gtb.field_14 = txquad->field_26 >> 16;
            break;
        case 3:
            gtb.field_0 = block_ptrs[txquad->field_6];
            gtb.field_C = txquad->field_1A >> 16;
            gtb.field_10 = txquad->field_1E >> 16;
            gtb.field_18 = txquad->field_22 >> 16;
            gtb.field_14 = txquad->field_26 >> 16;
            break;
        default:
            gtb.field_0 = block_ptrs[txquad->field_6];
            break;
        }
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
      struct KeeperSpr *unk12;
      struct BasicUnk13 *unk13;
      struct BasicUnk14 *unk14;
      struct TexturedQuad *txquad;
      struct Number *number;
      struct RoomFlag *roomFlg;
    } item;
    // Color rendering array pointers used by draw_keepersprite()
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_alpha = (unsigned char *)&alpha_sprite_table;
    render_problems = 0;
    thing_pointed_at = 0;
    for (bucket_num = BUCKETS_COUNT-1; bucket_num >= 0; bucket_num--)
    {
        for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
        {
            switch (item.b->kind)
            {
            case QK_JontySprite:
                draw_fastview_mapwho(cam, item.jonSpr);
                break;
            case QK_ClippedLine:
                draw_clipped_line(item.unk13->p.field_0,item.unk13->p.field_4,item.unk13->p.field_8,item.unk13->p.field_C,item.unk13->p.field_10);
                break;
            case QK_StatusSprites:
                if (pixel_size == 1)
                    draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, 12288);
                else
                    draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, 4096);
                break;
            case QK_TextureQuad:
                draw_texturedquad_block(item.txquad);
                break;
            case QK_IntegerValue:
                draw_engine_number(item.number);
                break;
            case QK_RoomFlagPole:
                draw_engine_room_flagpole(item.roomFlg);
                break;
            case QK_JontyISOSprite:
                draw_iso_only_fastview_mapwho(cam, item.jonSpr);
                break;
            case QK_RoomFlagTop:
                draw_engine_room_flag_top(item.roomFlg);
                break;
            default:
                render_problems++;
                render_prob_kind = item.b->kind;
                break;
            }
        }
    } // end for(bucket_num...
    if (render_problems > 0) {
        WARNLOG("Encoured %lu rendering problems; last was with poly kind %ld",render_problems,render_prob_kind);
    }
}

long convert_world_coord_to_front_view_screen_coord(struct Coord3d *pos, struct Camera *cam, long *x, long *y, long *z)
{
    return _DK_convert_world_coord_to_front_view_screen_coord(pos, cam, x, y, z);
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
    poly->b.kind = QK_JontySprite;
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
    poly->b.kind = QK_JontyISOSprite;
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
    if (bckt_idx >= BUCKETS_COUNT) {
      bckt_idx = BUCKETS_COUNT-1;
    } else
    if (bckt_idx < 0) {
      bckt_idx = 0;
    }
    poly = (struct BasicUnk14 *)getpoly;
    getpoly += sizeof(struct BasicUnk14);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_StatusSprites;
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
    poly->b.kind = QK_TextureQuad;
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
    poly->b.kind = QK_TextureQuad;
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
    poly->b.kind = QK_IntegerValue;
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
    poly->b.kind = QK_RoomFlagPole;
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
    poly->b.kind = QK_RoomFlagTop;
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
    long i,n;
    n = 4 * stl_x + 17 * stl_y;
    for (i=0; i < 9; i++)
    {
        long rndi,nval;
        if ((base_lightness <= 256) || (base_lightness > 15872))
        {
            nval = base_lightness;
        } else
        {
            rndi = randomisors[n&RANDOMISORS_MASK];
            n++;
            nval = 32 * (rndi & 0x3F) + base_lightness - 256;
        }
        *arrp = nval << 8;
        arrp++;
    }
}

void draw_element(struct Map *map, long lightness, long stl_x, long stl_y, long pos_x, long pos_y, long a7, unsigned char a8, long *ymax)
{
    struct PlayerInfo *myplyr;
    TbBool sibrevealed[3][3];
    struct Column *col;
    struct CubeAttribs *unkstrcp;
    struct Map *mapblk;
    long lightness_arr[4][9];
    long bckt_idx;
    long cube_itm,delta_y;
    long tc; // top cube index
    long x,y;
    long i;
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
        i = get_subtile_lightness(&game.lish,stl_x+1,stl_y);
    prepare_lightness_intensity_array(stl_x+1,stl_y,lightness_arr[(1-a8) & 3],i);

    i = 0;
    if (sibrevealed[1][0] && sibrevealed[1][1] && sibrevealed[2][0] && sibrevealed[2][1])
        i = get_subtile_lightness(&game.lish,stl_x,stl_y+1);
    prepare_lightness_intensity_array(stl_x,stl_y+1,lightness_arr[(-1-a8) & 3],i);

    i = 0;
    if (sibrevealed[2][2] && sibrevealed[1][2] && sibrevealed[1][1] && sibrevealed[2][1])
        i = get_subtile_lightness(&game.lish,stl_x+1,stl_y+1);
    prepare_lightness_intensity_array(stl_x+1,stl_y+1,lightness_arr[(-2-a8) & 3],i);

    // Get column to be drawn on the current subtile

    if (map_block_revealed_bit(map, player_bit))
      i = get_mapblk_column_index(map);
    else
      i = game.unrevealed_column_idx;
    col = get_column(i);
    mapblk = get_map_block_at(stl_x, stl_y);

    // Draw the columns base block

    if (*ymax > pos_y)
    {
      if ((col->baseblock != 0) && (col->cubes[0] == 0))
      {
          *ymax = pos_y;
          if ((mapblk->flags & MapFlg_Unkn04) != 0)
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
    unkstrcp = NULL;
    for (tc=0; tc < COLUMN_STACK_HEIGHT; tc++)
    {
      if (col->cubes[tc] == 0)
        break;
      y -= delta_y;
      unkstrcp = &game.cubes_data[col->cubes[tc]];
      if (*ymax > y)
      {
        *ymax = y;
        add_lgttextrdquad_to_polypool(pos_x, y, unkstrcp->texture_id[cube_itm], a7, delta_y, 0,
            lightness_arr[3][tc+1], lightness_arr[2][tc+1], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
      }
    }

    if (unkstrcp != NULL)
    {
      i = y - a7;
      if (*ymax > i)
      {
        *ymax = i;
        if ((mapblk->flags & MapFlg_Unkn80) != 0)
        {
          add_textruredquad_to_polypool(pos_x, i, unkstrcp->texture_id[4], a7, a8,
              2097152, 1, bckt_idx);
        } else
        if ((mapblk->flags & MapFlg_Unkn04) != 0)
        {
          add_textruredquad_to_polypool(pos_x, i, unkstrcp->texture_id[4], a7, a8,
              2097152, 0, bckt_idx);
        } else
        {
          add_lgttextrdquad_to_polypool(pos_x, i, unkstrcp->texture_id[4], a7, a7, a8,
              lightness_arr[0][tc], lightness_arr[1][tc], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
        }
      }
    }

    // If there are still some solid cubes higher than tc
    if ((get_column_ceiling_filled_subtiles(col) != 0) && (col->solidmask > (1 << tc)))
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
            unkstrcp = &game.cubes_data[col->cubes[tc]];
            if (*ymax > y)
            {
              add_lgttextrdquad_to_polypool(pos_x, y, unkstrcp->texture_id[cube_itm], a7, delta_y, 0,
                  lightness_arr[3][tc+1], lightness_arr[2][tc+1], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
            }
        }
        if (unkstrcp != NULL)
        {
          i = y - a7;
          if (*ymax > i)
          {
            add_lgttextrdquad_to_polypool(pos_x, i, unkstrcp->texture_id[4], a7, a7, a8,
                lightness_arr[0][tc], lightness_arr[1][tc], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
          }
        }
    }

}

unsigned short get_thing_shade(struct Thing *thing)
{
    MapSubtlCoord stl_x,stl_y;
    long lgh[2][2]; // the dimensions are lgh[y][x]
    long shval;
    long fract_x,fract_y;
    stl_x = thing->mappos.x.stl.num;
    stl_y = thing->mappos.y.stl.num;
    fract_x = thing->mappos.x.stl.pos;
    fract_y = thing->mappos.y.stl.pos;
    lgh[0][0] = get_subtile_lightness(&game.lish,stl_x,  stl_y);
    lgh[0][1] = get_subtile_lightness(&game.lish,stl_x+1,stl_y);
    lgh[1][0] = get_subtile_lightness(&game.lish,stl_x,  stl_y+1);
    lgh[1][1] = get_subtile_lightness(&game.lish,stl_x+1,stl_y+1);
    shval = (fract_x
        * (lgh[0][1] + (fract_y * (lgh[1][1] - lgh[0][1]) >> 8)
        - (lgh[0][0] + (fract_y * (lgh[1][0] - lgh[0][0]) >> 8))) >> 8)
        + (lgh[0][0] + (fract_y * (lgh[1][0] - lgh[0][0]) >> 8));
    if (shval < MINIMUM_LIGHTNESS)
    {
        shval += (MINIMUM_LIGHTNESS>>2);
        if (shval > MINIMUM_LIGHTNESS)
            shval = MINIMUM_LIGHTNESS;
    } else
    {
        // Max lightness value - make sure it won't exceed our limits
        if (shval > 64*256+255)
            shval = 64*256+255;
    }
    return shval;
}

void lock_keepersprite(unsigned short kspr_idx)
{
    int frame_num,frame_count;
    struct KeeperSprite *kspr_arr;
    kspr_arr = &creature_table[kspr_idx];
    if (kspr_arr->rotable) {
        frame_count = 5 * kspr_arr->frames;
    } else {
        frame_count = kspr_arr->frames;
    }
    for (frame_num=0; frame_num < frame_count; frame_num++)
    {
        struct HeapMgrHandle *hmhndl;
        hmhndl = heap_handle[kspr_idx+frame_num];
        if (hmhndl != NULL) {
            hmhndl->flags |= 0x02;
        }
    }
}

void unlock_keepersprite(unsigned short kspr_idx)
{
    int frame_num,frame_count;
    struct KeeperSprite *kspr_arr;
    kspr_arr = &creature_table[kspr_idx];
    if (kspr_arr->rotable) {
        frame_count = 5 * kspr_arr->frames;
    } else {
        frame_count = kspr_arr->frames;
    }
    for (frame_num=0; frame_num < frame_count; frame_num++)
    {
        struct HeapMgrHandle *hmhndl;
        hmhndl = heap_handle[kspr_idx+frame_num];
        if (hmhndl != NULL) {
            hmhndl->flags &= ~0x02;
        }
    }
}

long load_single_frame(unsigned short kspr_idx)
{
    long nlength;
    struct HeapMgrHandle *nitem;
    int i;
    //return _DK_load_single_frame(kspr_idx);
    nlength = creature_table[kspr_idx+1].foffset - creature_table[kspr_idx].foffset;
    for (i=0; i < 100; i++)
    {
        nitem = heapmgr_add_item(graphics_heap, nlength);
        if (nitem != NULL) {
            break;
        }
        long idfreed;
        idfreed = heapmgr_free_oldest(graphics_heap);
        if (idfreed < 0)
        {
            // If can't free anything more, try to defrag existing items
            heapmgr_complete_defrag(graphics_heap);
            nitem = heapmgr_add_item(graphics_heap, nlength);
            if (nitem != NULL) {
                break;
            }
            // Cannot free and cannot defrag - we can't do anything more
            ERRORLOG("Unable to find freeable item");
            return 0;
        }
        heap_handle[idfreed] = NULL;
        keepsprite[idfreed] = NULL;
    }
    if (nitem == NULL) {
        ERRORLOG("Unable to make room for new item on heap");
        return 0;
    }
    if (!read_heap_item(nitem, creature_table[kspr_idx].foffset, nlength))
    {
        ERRORLOG("Load Fail On KeepSprite %d",(int)kspr_idx);
        return 0;
    }
    keepsprite[kspr_idx] = (unsigned char **)&nitem->buf;
    heap_handle[kspr_idx] = nitem;
    nitem->idx = kspr_idx;
    return 1;
}

long load_keepersprite_if_needed(unsigned short kspr_idx)
{
    int frame_num,frame_count;
    struct KeeperSprite *kspr_arr;
    kspr_arr = &creature_table[kspr_idx];
    if (kspr_arr->rotable) {
        frame_count = 5 * kspr_arr->frames;
    } else {
        frame_count = kspr_arr->frames;
    }
    for (frame_num=0; frame_num < frame_count; frame_num++)
    {
        struct HeapMgrHandle **hmhndl;
        hmhndl = &heap_handle[kspr_idx+frame_num];
        if ((*hmhndl) != NULL)
        {
            heapmgr_make_newest(graphics_heap, *hmhndl);
        } else
        {
            if ( !load_single_frame(kspr_idx+frame_num) )
            {
                return 0;
            }
            (*hmhndl)->flags |= 0x02;
        }
    }
    return 1;
}

long heap_manage_keepersprite(unsigned short kspr_idx)
{
    //return _DK_heap_manage_keepersprite(kspr_idx);
    long result;
    lock_keepersprite(kspr_idx);
    result = load_keepersprite_if_needed(kspr_idx);
    unlock_keepersprite(kspr_idx);
    return result;
}

void draw_keepersprite(long x, long y, long w, long h, long kspr_idx)
{
    struct TbSprite sprite;
    long cut_w,cut_h;
    TbSpriteData *kspr_item;
    if ((kspr_idx < 0) || (kspr_idx >= KEEPSPRITE_LENGTH)) {
        WARNDBG(9,"Invalid KeeperSprite %ld at (%ld,%ld) size (%ld,%ld) alpha %d",kspr_idx,x,y,w,h,(int)EngineSpriteDrawUsingAlpha);
        return;
    }
    SYNCDBG(17,"Drawing %ld at (%ld,%ld) size (%ld,%ld) alpha %d",kspr_idx,x,y,w,h,(int)EngineSpriteDrawUsingAlpha);
    cut_w = w;
    cut_h = h - water_source_cutoff;
    if (cut_h <= 0) {
        return;
    }
    kspr_item = keepsprite[kspr_idx];
    sprite.SWidth = cut_w;
    sprite.SHeight = cut_h;
    if (kspr_item != NULL) {
        sprite.Data = *kspr_item;
    } else {
        sprite.Data = NULL;
    }
    if (sprite.Data == NULL) {
        WARNDBG(9,"Unallocated KeeperSprite %ld can't be drawn at (%ld,%ld)",kspr_idx,x,y);
        return;
    }
    if ( EngineSpriteDrawUsingAlpha ) {
        DrawAlphaSpriteUsingScalingData(x, y, &sprite);
    } else {
        LbSpriteDrawUsingScalingData(x, y, &sprite);
    }
    SYNCDBG(18,"Finished");
}

void set_thing_pointed_at(struct Thing *thing)
{
    if (thing_pointed_at == NULL) {
        thing_pointed_at = thing;
    }
}

void draw_single_keepersprite_omni_xflip(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long x,y;
    long src_dy,src_dx;
    src_dy = (long)kspr->field_7;
    src_dx = (long)kspr->field_6;
    x = src_dx - (long)kspr->field_A - (long)kspr->field_4;
    y = kspr->field_B;
    if ( EngineSpriteDrawUsingAlpha )
    {
        sp_x = kspos_x;
        sp_y = kspos_y;
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        SetAlphaScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    } else
    {
        sp_x = kspos_x;
        sp_y = kspos_y;
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    }
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= sp_x) && (pointer_x <= sp_dx + sp_x) )
      {
          if ( (pointer_y >= sp_y) && (pointer_y <= sp_dy + sp_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(x, y, kspr->field_4, kspr->field_5, kspr_idx);
}

void draw_single_keepersprite_omni(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long x,y;
    long src_dy,src_dx;
    src_dy = (long)kspr->field_7;
    src_dx = (long)kspr->field_6;
    x = kspr->field_A;
    y = kspr->field_B;
    if ( EngineSpriteDrawUsingAlpha )
    {
        sp_x = kspos_x;
        sp_y = kspos_y;
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        SetAlphaScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    } else
    {
        sp_x = kspos_x;
        sp_y = kspos_y;
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    }
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= sp_x) && (pointer_x <= sp_dx + sp_x) )
      {
          if ( (pointer_y >= sp_y) && (pointer_y <= sp_dy + sp_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(x, y, kspr->field_4, kspr->field_5, kspr_idx);
}

void draw_single_keepersprite_xflip(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long x,y;
    long src_dy,src_dx;
    SYNCDBG(18,"Starting");
    //_DK_draw_single_keepersprite_xflip(kspos_x, kspos_y, kspr, kspr_idx, scale); return;
    src_dy = (long)kspr->field_5;
    src_dx = (long)kspr->field_4;
    x = (long)kspr->field_6 - (long)kspr->field_A - src_dx;
    y = kspr->field_B;
    if ( EngineSpriteDrawUsingAlpha )
    {
        sp_x = kspos_x + ((scale * x) >> 5);
        sp_y = kspos_y + ((scale * y) >> 5);
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        SetAlphaScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    } else
    {
        sp_x = kspos_x + ((scale * x) >> 5);
        sp_y = kspos_y + ((scale * y) >> 5);
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    }
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= sp_x) && (pointer_x <= sp_dx + sp_x) )
      {
          if ( (pointer_y >= sp_y) && (pointer_y <= sp_dy + sp_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(0, 0, kspr->field_4, kspr->field_5, kspr_idx);
    SYNCDBG(18,"Finished");
}

void draw_single_keepersprite(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long x,y;
    long src_dy,src_dx;
    SYNCDBG(18,"Starting");
    src_dy = (long)kspr->field_5;
    src_dx = (long)kspr->field_4;
    x = kspr->field_A;
    y = kspr->field_B;
    if ( EngineSpriteDrawUsingAlpha )
    {
        sp_x = kspos_x + ((scale * x) >> 5);
        sp_y = kspos_y + ((scale * y) >> 5);
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        SetAlphaScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    } else
    {
        sp_x = kspos_x + ((scale * x) >> 5);
        sp_y = kspos_y + ((scale * y) >> 5);
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    }
    if ( thing_being_displayed_is_creature )
    {
        if ( (pointer_x >= x) && (pointer_x <= sp_dx + x) )
        {
            if ( (pointer_y >= y) && (pointer_y <= sp_dy + y) )
            {
                set_thing_pointed_at(thing_being_displayed);
            }
        }
    }
    draw_keepersprite(0, 0, kspr->field_4, kspr->field_5, kspr_idx);
    SYNCDBG(18,"Finished");
}

void process_keeper_sprite(short x, short y, unsigned short kspr_base, short kspr_frame, unsigned char sprgroup, long scale)
{
    struct KeeperSprite *creature_sprites;
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct KeeperSprite *kspr;
    long kspr_idx,draw_idx;
    short dim_ow,dim_oh,dim_th,dim_tw;
    long scaled_x,scaled_y;
    TbBool needs_xflip;
    long long lltemp;
    long sprite_group,sprite_delta,cutoff;
    SYNCDBG(17,"At (%d,%d) opts %d %d %d %d",(int)x,(int)y,(int)kspr_base,(int)kspr_frame,(int)sprgroup,(int)scale);
    //_DK_process_keeper_sprite(x, y, a3, a4, sprgroup, scale); return;
    player = get_my_player();

    if ( ((kspr_frame & 0x7FF) <= 1151) || ((kspr_frame & 0x7FF) >= 1919) )
        needs_xflip = 0;
    else
        needs_xflip = 1;
    if ( needs_xflip )
      lbDisplay.DrawFlags |= Lb_SPRITE_ONECOLOUR1;
    else
      lbDisplay.DrawFlags &= ~Lb_SPRITE_ONECOLOUR1;
    sprite_group = sprgroup;
    lltemp = 4 - ((((long)kspr_frame + 128) & 0x7FF) >> 8);
    sprite_delta = llabs(lltemp);
    kspr_idx = keepersprite_index(kspr_base);
    global_scaler = scale;
    creature_sprites = keepersprite_array(kspr_base);
    scaled_x = ((scale * (long)creature_sprites->field_C) >> 5) + (long)x;
    scaled_y = ((scale * (long)creature_sprites->field_E) >> 5) + (long)y;
    SYNCDBG(17,"Scaled (%d,%d)",(int)scaled_x,(int)scaled_y);
    if (thing_is_invalid(thing_being_displayed))
    {
        water_y_offset = 0;
        water_source_cutoff = 0;
    } else
    if ( (thing_being_displayed->movement_flags & (TMvF_IsOnWater|TMvF_IsOnLava|TMvF_Unknown04)) == 0)
    {
        water_y_offset = 0;
        water_source_cutoff = 0;
    } else
    {
        cutoff = 6;
        if ( (thing_being_displayed->movement_flags & TMvF_Unknown04) != 0 )
        {
            get_keepsprite_unscaled_dimensions(thing_being_displayed->field_44, thing_being_displayed->field_52, thing_being_displayed->field_48, &dim_ow, &dim_oh, &dim_tw, &dim_th);
            cctrl = creature_control_get_from_thing(thing_being_displayed);
            lltemp = dim_oh * (48 - (long)cctrl->word_9A);
            cutoff = ((((lltemp >> 24) & 0x1F) + (long)lltemp) >> 5) / 2;
        }
        if (player->view_mode == 1)
        {
            water_source_cutoff = cutoff;
            water_y_offset = (2 * scale * cutoff) >> 5;
        } else
        {
            water_source_cutoff = 2 * cutoff;
            water_y_offset = (scale * cutoff) >> 5;
        }
    }
    scaled_y += water_y_offset;
    if (creature_sprites->rotable == 0)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &creature_sprites[sprite_group];
        draw_idx = sprite_group + kspr_idx;
        if ( needs_xflip )
        {
            draw_single_keepersprite_omni_xflip(scaled_x, scaled_y, kspr, draw_idx, scale);
        } else
        {
            draw_single_keepersprite_omni(scaled_x, scaled_y, kspr, draw_idx, scale);
        }
    } else
    if (creature_sprites->rotable == 2)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &creature_sprites[sprite_group + sprite_delta * (long)creature_sprites->frames];
        draw_idx = sprite_group + sprite_delta * (long)kspr->frames + kspr_idx;
        if ( needs_xflip )
        {
            draw_single_keepersprite_xflip(scaled_x, scaled_y, kspr, draw_idx, scale);
        } else
        {
            draw_single_keepersprite(scaled_x, scaled_y, kspr, draw_idx, scale);
        }
    }
}

void process_keeper_speedup_sprite(struct JontySpr *jspr, long angle, long scale)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    long transp2;
    unsigned short graph_id2;
    unsigned long nframe2;
    long add_x,add_y;
    thing = jspr->thing;
    player = get_my_player();
    switch (thing->model)
    {
    case 28:
        graph_id2 = 112;
        if (player->view_type == PVT_DungeonTop)
        {
          add_x = scale >> 3;
          add_y = (scale >> 2) - scale;
        } else
        {
          add_x = scale * LbSinL(angle) >> 20;
          add_y = -(LbCosL(angle) * (scale + (scale >> 1))) >> 16;
        }
        transp2 = scale / 2;
        break;
    case 2:
        graph_id2 = 113;
        if (player->view_type == PVT_DungeonTop)
        {
            add_x = 0;
            add_y = 3 * scale >> 3;
        } else
        {
            add_x = (scale * LbSinL(angle)) >> 20;
            add_y = (scale * LbCosL(angle)) >> 20;
        }
        transp2 = 2 * scale / 3;
        break;
    default:
        graph_id2 = 113;
        if (player->view_type == PVT_DungeonTop)
        {
            add_x = (scale >> 2) / 3;
            add_y = (scale >> 1) / 3;
        } else
        {
            add_x = scale * LbSinL(angle) >> 20;
            add_y = (-(LbCosL(angle) * (scale + (scale >> 1))) >> 16) / 3;
        }
        transp2 = scale / 3;
        break;
    }
    EngineSpriteDrawUsingAlpha = 0;
    nframe2 = (thing->index + game.play_gameturn) % keepersprite_frames(graph_id2);
    process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->field_44, angle, thing->field_48, scale);
    EngineSpriteDrawUsingAlpha = 1;
    process_keeper_sprite(jspr->scr_x+add_x, jspr->scr_y+add_y, graph_id2, angle, nframe2, transp2);
}

void prepare_jonty_remap_and_scale(long *scale, const struct JontySpr *jspr)
{
    long i;
    struct Thing *thing;
    long shade,shade_factor,fade;
    thing = jspr->thing;
    if (lens_mode == 0)
    {
        fade = 65536;
        if ((thing->field_4F & 0x02) == 0)
            i = get_thing_shade(thing);
        else
            i = MINIMUM_LIGHTNESS;
        shade = i;
    } else
    if (jspr->field_14 <= lfade_min)
    {
        fade = jspr->field_14;
        if ((thing->field_4F & 0x02) == 0)
            i = get_thing_shade(thing);
        else
            i = MINIMUM_LIGHTNESS;
        shade = i;
    } else
    if (jspr->field_14 < lfade_max)
    {
        fade = jspr->field_14;
        if ((thing->field_4F & 0x02) == 0)
            i = get_thing_shade(thing);
        else
            i = MINIMUM_LIGHTNESS;
        shade = i * (long long)(lfade_max - fade) / fade_mmm;
    } else
    {
        fade = jspr->field_14;
        shade = 0;
    }
    shade_factor = shade >> 8;
    *scale = (thelens * (long)thing->field_46) / fade;
    if ((thing->field_4F & 0xC) != 0)
    {
        lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
        shade_factor = thing->field_51;
        lbSpriteReMapPtr = &pixmap.ghost[256 * shade_factor];
    } else
    if (shade_factor == 32)
    {
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
    } else
    {
        lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
        lbSpriteReMapPtr = &pixmap.fade_tables[256 * shade_factor];
    }
}

void draw_jonty_mapwho(struct JontySpr *jspr)
{
    unsigned short flg_mem;
    unsigned char alpha_mem;
    struct PlayerInfo *player;
    struct Thing *thing;
    long angle,scale;
    //_DK_draw_jonty_mapwho(jspr); return;
    flg_mem = lbDisplay.DrawFlags;
    alpha_mem = EngineSpriteDrawUsingAlpha;
    thing = jspr->thing;
    player = get_my_player();
    if (keepersprite_rotable(thing->field_44))
      angle = thing->field_52 - spr_map_angle;
    else
      angle = thing->field_52;
    prepare_jonty_remap_and_scale(&scale, jspr);
    EngineSpriteDrawUsingAlpha = 0;
    switch (thing->field_4F & 0x30)
    {
    case 0x10:
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
        break;
    case 0x20:
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
        break;
    case 0x30:
        EngineSpriteDrawUsingAlpha = 1;
        break;
    }

    if ((thing->class_id == TCls_Creature)
     || (thing->class_id == TCls_Object)
     || (thing->class_id == TCls_DeadCreature))
    {
        if ((player->thing_under_hand == thing->index) && (game.play_gameturn & 2))
        {
          if (player->acamera->field_6 == 2)
          {
              lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
              lbSpriteReMapPtr = white_pal;
          }
        } else
        if ((thing->field_4F & 0x80) != 0)
        {
            lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
            lbSpriteReMapPtr = red_pal;
            thing->field_4F &= ~0x80u;
        }
        thing_being_displayed_is_creature = 1;
        thing_being_displayed = thing;
    } else
    {
        thing_being_displayed_is_creature = 0;
        thing_being_displayed = NULL;
    }

    if (thing->field_44 >= CREATURE_FRAMELIST_LENGTH)
    {
        ERRORLOG("Invalid graphic Id %d from model %d, class %d", (int)thing->field_44, (int)thing->model, (int)thing->class_id);
    } else
    {
        switch (thing->class_id)
        {
        case TCls_Object:
            if ((thing->model == 2) || (thing->model == 4) || (thing->model == 28))
            {
                process_keeper_speedup_sprite(jspr, angle, scale);
                break;
            }
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->field_44, angle, thing->field_48, scale);
            break;
        case TCls_Trap:
            if ((thing->model != 1) && (player->id_number != thing->owner) && (thing->byte_18 == 0))
            {
                break;
            }
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->field_44, angle, thing->field_48, scale);
            break;
        default:
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->field_44, angle, thing->field_48, scale);
            break;
        }
    }
    lbDisplay.DrawFlags = flg_mem;
    EngineSpriteDrawUsingAlpha = alpha_mem;
}

/** Fills solid area of the sprite in target buffer with color 255.
 *
 * @param sprdata Sprite data.
 * @param outbuf Output buffer to be filled.
 * @param lines_max Max lines to be written into output buffer.
 * @param scanln Length of scanline (length of line in output buffer).
 */
void sprite_to_sbuff(const TbSpriteData sprdata, unsigned char *outbuf, int lines_max, int scanln)
{
    unsigned char *out_lnstart;
    unsigned char *out;
    int cval;
    const unsigned char *sprd;
    int i;
    sprd = sprdata;
    out = outbuf;
    out_lnstart = outbuf;
    cval = 0;
    while (lines_max > 0)
    {
      while ( 1 )
      {
          // Skip transparent area
          while ( 1 )
          {
              cval = *(char *)sprd;
              sprd++;
              if (cval >= 0)
                break;
              cval = -cval;
              out += cval;
          }
          if (cval == 0)
            break;
          sprd += cval;
          // Fill area per-byte until we get 32bit-aligned position
          while ((unsigned int)out & 3)
          {
              *out = 0xFF;
              out++;
              cval--;
              if (cval <= 0)
                  break;
          }
          // Now fill the area faster - by writing 32-bit values
          for (i = (cval >> 2); i > 0; i--)
          {
              *(unsigned long *)out = 0xFFFFFFFF;
              out += 4;
          }
          // Fill the last unaligned bytes
          cval &= 3;
          while (cval > 0)
          {
            *out = 0xFF;
            out++;
            cval--;
          }
      }
      out_lnstart += scanln;
      out = out_lnstart;
      lines_max--;
    }
}

/** Fills solid area of the sprite in target buffer with color 255, x-flipping the sprite.
 *
 * @param sprdata Sprite data.
 * @param outbuf Output buffer to be filled.
 * @param lines_max Max lines to be written into output buffer.
 * @param scanln Length of scanline (length of line in output buffer).
 */
void sprite_to_sbuff_xflip(const TbSpriteData sprdata, unsigned char *outbuf, int lines_max, int scanln)
{
    unsigned char *out_lnstart;
    unsigned char *out;
    int cval;
    const unsigned char *sprd;
    int i;
    sprd = sprdata;
    out = outbuf;
    out_lnstart = outbuf;
    cval = 0;
    while (lines_max > 0)
    {
      while ( 1 )
      {
          // Skip transparent area
          while ( 1 )
          {
              cval = *(char *)sprd;
              sprd++;
              if (cval >= 0)
                  break;
              cval = -cval;
              out -= cval;
          }
          if (cval == 0)
            break;
          sprd += cval;
          out++;
          // Fill area per-byte until we get 32bit-aligned position
          while ((unsigned long)out & 3)
          {
              out--;
              *out = 0xFF;
              cval--;
              if (cval <= 0)
                  break;
          }
          // Now fill the area faster - by writing 32-bit values
          for (i = (cval >> 2); i > 0; i--)
          {
              out -= 4;
              *(unsigned long *)out = 0xFFFFFFFF;
          }
          out--;
          // Fill the last unaligned bytes
          cval &= 3;
          while (cval > 0)
          {
              *out = 0xFF;
              out--;
              cval--;
          }
      }
      out_lnstart += scanln;
      out = out_lnstart;
      lines_max--;
    }
}

void draw_keepsprite_unscaled_in_buffer(unsigned short kspr_n, short angle, unsigned char a3, unsigned char *outbuf)
{
    struct KeeperSprite *kspr_arr;
    unsigned long kspr_idx;
    struct KeeperSprite *kspr;
    unsigned int keepsprite_id;
    unsigned char *tmpbuf;
    int skip_w,skip_h;
    int fill_w,fill_h;
    TbBool flip_range;
    short quarter;
    int i;
    //_DK_draw_keepsprite_unscaled_in_buffer(a1, a2, a3, a4);
    if ( ((angle & 0x7FF) <= 1151) || ((angle & 0x7FF) >= 1919) )
        flip_range = false;
    else
        flip_range = true;
    i = ((angle + 128) & 0x7FF);
    quarter = abs(4 - (i >> 8)); // i is restricted by "&" so (i>>8) is 0..7
    kspr_idx = keepersprite_index(kspr_n);
    kspr_arr = keepersprite_array(kspr_n);
    if (kspr_arr->rotable == 0)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        keepsprite_id = a3 + kspr_idx;
        kspr = &kspr_arr[a3];
        fill_w = kspr->field_6;
        fill_h = kspr->field_7;
        if ( flip_range )
        {
            tmpbuf = outbuf;
            skip_w = kspr->field_6 - kspr->field_A;
            skip_h = kspr->field_B;
            for (i = fill_h; i > 0; i--)
            {
                LbMemorySet(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff_xflip(*keepsprite[keepsprite_id], &outbuf[256 * skip_h + skip_w], kspr->field_5, 256);
        } else
        {
            tmpbuf = outbuf;
            skip_w = kspr->field_A;
            skip_h = kspr->field_B;
            for (i = fill_h; i > 0; i--)
            {
                LbMemorySet(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff(*keepsprite[keepsprite_id], &outbuf[256 * skip_h + skip_w], kspr->field_5, 256);
        }
    } else
    if (kspr_arr->rotable == 2)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &kspr_arr[a3 + quarter * kspr_arr->frames];
        fill_w = kspr->field_4;
        fill_h = kspr->field_5;
        keepsprite_id = a3 + quarter * kspr->frames + kspr_idx;
        if ( flip_range )
        {
            tmpbuf = outbuf;
            for (i = fill_h; i > 0; i--)
            {
                LbMemorySet(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff_xflip(*keepsprite[keepsprite_id], &outbuf[kspr->field_4], kspr->field_5, 256);
        } else
        {
            tmpbuf = outbuf;
            for (i = fill_h; i > 0; i--)
            {
                LbMemorySet(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff(*keepsprite[keepsprite_id], &outbuf[0], kspr->field_5, 256);
        }
    }
}

void update_frontview_pointed_block(unsigned long laaa, unsigned char qdrant, long w, long h, long qx, long qy)
{
    TbGraphicsWindow ewnd;
    struct Column *colmn;
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
            colmn = get_map_column(map);
            mask = colmn->solidmask;
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
    pos.z.val = 5*256;
    orient = ((unsigned int)(cam->orient_a + 256) >> 9) & 0x03;
    convert_world_coord_to_front_view_screen_coord(&pos, cam, &coord_x, &coord_y, &coord_z);
    depth = (5 - map_volume_box.field_13) * ((long)stl_width << 7) / 256;
    slb_width = STL_PER_SLB * (long)stl_width;
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
            add_unkn16_to_polypool(cx, cy, thing->creature.gold_carried, 1);
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
              add_unkn17_to_polypool(cx, cy, thing->creature.gold_carried, cz-3);
              if (is_free_space_in_poly_pool(1))
              {
                  add_unkn19_to_polypool(cx, cy, thing->creature.gold_carried, 1);
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
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
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
    long zoom_mem;
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
    long long zoom,lbbb;
    long i;
    SYNCDBG(9,"Starting");
    player = get_my_player();
    if (cam->zoom > 65536)
        cam->zoom = 65536;
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    zoom_mem = cam->zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    cam->zoom = camera_zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    UseFastBlockDraw = (camera_zoom == 65536);
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
    zoom = camera_zoom >> 3;
    w = (ewnd.width << 16) / zoom >> 1;
    h = (ewnd.height << 16) / zoom >> 1;
    cam_x = cam->mappos.x.val;
    cam_y = cam->mappos.y.val;
    switch (qdrant)
    {
    case 0:
        px = ((cam_x - w) >> 8);
        py = ((cam_y - h) >> 8);
        lbbb = cam_x - (px << 8);
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = cam_y - (py << 8);
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        break;
    case 1:
        px = ((cam_x + h) >> 8);
        py = ((cam_y - w) >> 8);
        lbbb = cam_y - (py << 8);
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = (px << 8) - cam_x;
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        px--;
        break;
    case 2:
        px = ((cam_x + w) >> 8) + 1;
        py = ((cam_y + h) >> 8);
        lbbb = (px << 8) - cam_x;
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = (py << 8) - cam_y;
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        px--;
        py--;
        break;
    case 3:
        px = ((cam_x - h) >> 8);
        py = ((cam_y + w) >> 8) + 1;
        lbbb = (py << 8) - cam_y;
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = cam_x - (px << 8);
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        py--;
        break;
    default:
        ERRORLOG("Illegal quadrant, %d.",qdrant);
        LbScreenLoadGraphicsWindow(&grwnd);
        return;
    }

    update_frontview_pointed_block(zoom, qdrant, px, py, qx, qy);
    if (map_volume_box.visible)
        create_frontview_map_volume_box(cam, (zoom >> 8) & 0xFF);
    map_volume_box.visible = 0;

    h = (8 * (zoom + 32 * ewnd.height) - qy) / zoom;
    w = (8 * (zoom + 32 * ewnd.height) - qy) / zoom;
    qy += zoom * h;
    px += x_step1[qdrant] * w;
    stl_x = x_step1[qdrant] * w + px;
    stl_y = y_step1[qdrant] * h + py;
    py += y_step1[qdrant] * h;
    lim_x = ewnd.width << 8;
    lim_y = -zoom;
    SYNCDBG(19,"Range (%ld,%ld) to (%ld,%ld), quadrant %d",px,py,qx,qy,(int)qdrant);
    for (pos_x=qx; pos_x < lim_x; pos_x += zoom)
    {
        i = (ewnd.height << 8);
        // Initialize the stl_? which will be swept by second loop
        if (x_step1[qdrant] != 0)
          stl_x = px;
        else
          stl_y = py;
        for (pos_y=qy; pos_y > lim_y; pos_y -= zoom)
        {
            map = get_map_block_at(stl_x, stl_y);
            if (!map_block_invalid(map))
            {
                if (get_mapblk_column_index(map) > 0)
                {
                    draw_element(map, game.lish.subtile_lightness[get_subtile_number(stl_x,stl_y)], stl_x, stl_y, pos_x, pos_y, zoom, qdrant, &i);
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
    cam->zoom = zoom_mem;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    SYNCDBG(9,"Finished");
}
/******************************************************************************/
