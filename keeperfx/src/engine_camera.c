/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_camera.c
 *     Camera move, maintain and support functions.
 * @par Purpose:
 *     Defines and maintains cameras.
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
#include "engine_camera.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"

#include "engine_lenses.h"
#include "engine_render.h"
#include "vidmode.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
DLLIMPORT long _DK_get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
DLLIMPORT long _DK_get_2d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
DLLIMPORT void _DK_project_point_to_wall_on_angle(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4, long a5, long a6);
DLLIMPORT void _DK_angles_to_vector(short angle_xy, short angle_yz, long dist, struct ComponentVector *cvect);
DLLIMPORT void _DK_view_zoom_camera_in(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_set_camera_zoom(struct Camera *cam, long val);
DLLIMPORT void _DK_view_zoom_camera_out(struct Camera *cam, long a2, long a3);
DLLIMPORT long _DK_get_camera_zoom(struct Camera *camera);
/******************************************************************************/
long camera_zoom;
/******************************************************************************/
long get_3d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
  long dx;
  long dy;
  long dz;
  dy = abs(pos2->y.val - (long)pos1->y.val);
  dx = abs(pos2->x.val - (long)pos1->x.val);
  if (dy <= dx)
    dy = dx;
  dz = abs(pos2->z.val - (long)pos1->z.val);
  if (dy <= dz)
    dy = dz;
  return dy;
}

long get_2d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
  long dx,dy;
  dy = abs(pos1->y.val - (long)pos2->y.val);
  dx = abs(pos1->x.val - (long)pos2->x.val);
  if (dy <= dx)
    return dx;
  return dy;
}
void angles_to_vector(short angle_xy, short angle_yz, long dist, struct ComponentVector *cvect)
{
    //_DK_angles_to_vector(angle_xy, angle_yz, dist, cvect); return;
    long long sin_yz,cos_yz,sin_xy,cos_xy;
    long long lldist,mag,factor;
    cos_yz = LbCosL(angle_yz) >> 2;
    sin_yz = LbSinL(angle_yz) >> 2;
    cos_xy = LbCosL(angle_xy) >> 2;
    sin_xy = LbSinL(angle_xy) >> 2;
    lldist = dist;
    mag = (lldist << 14) - cos_yz;
    factor = sin_xy * mag;
    cvect->x = (factor >> 14) >> 14;
    factor = cos_xy * mag;
    cvect->y = -(factor >> 14) >> 14;
    factor = lldist * sin_yz;
    cvect->z = (factor >> 14);
}

long get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
  return _DK_get_angle_xy_to(pos1, pos2);
}

long get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
  return _DK_get_angle_yz_to(pos1, pos2);
}

long get_2d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist_x,dist_y;
    //return _DK_get_2d_distance(pos1, pos2);
    dist_x = (long)pos1->x.val - (long)pos2->x.val;
    dist_y = (long)pos1->y.val - (long)pos2->y.val;
    return LbDiagonalLength(abs(dist_x), abs(dist_y));
}

void project_point_to_wall_on_angle(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4, long a5, long a6)
{
  _DK_project_point_to_wall_on_angle(pos1, pos2, a3, a4, a5, a6);
}

void view_zoom_camera_in(struct Camera *cam, long limit_max, long limit_min)
{
    long new_zoom,old_zoom;
    //_DK_view_zoom_camera_in(cam, a2, a3);
    old_zoom = get_camera_zoom(cam);
    switch (cam->field_6)
    {
    case 2:
        new_zoom = (100 * old_zoom) / 85;
        if (new_zoom == old_zoom)
            new_zoom++;
        if (new_zoom < limit_min) {
            new_zoom = limit_min;
        } else
        if (new_zoom > limit_max) {
            new_zoom = limit_max;
        }
        break;
    case 3:
        new_zoom = (5 * old_zoom) / 4;
        if (new_zoom == old_zoom)
            new_zoom++;
        if (new_zoom < 16) {
            new_zoom = 16;
        } else
        if (new_zoom > 1024) {
            new_zoom = 1024;
        }
        break;
    case 5:
        new_zoom = (100 * old_zoom) / 85;
        if (new_zoom == old_zoom)
            new_zoom++;
        if (new_zoom < 16384) {
            new_zoom = 16384;
        } else
        if (new_zoom > 65536) {
            new_zoom = 65536;
        }
        break;
    default:
        new_zoom = old_zoom;
    }
    set_camera_zoom(cam, new_zoom);
}

void set_camera_zoom(struct Camera *cam, long new_zoom)
{
    if (cam == NULL)
      return;
    //_DK_set_camera_zoom(cam, val);
    switch (cam->field_6)
    {
    case 2:
    case 5:
        cam->zoom = new_zoom;
        break;
    case 3:
        cam->mappos.z.val = new_zoom;
        break;
    }
}

void view_zoom_camera_out(struct Camera *cam, long limit_max, long limit_min)
{
    long new_zoom,old_zoom;
    //_DK_view_zoom_camera_out(cam, a2, a3);
    old_zoom = get_camera_zoom(cam);
    switch (cam->field_6)
    {
    case 2:
        new_zoom = (85 * old_zoom) / 100;
        if (new_zoom == old_zoom)
            new_zoom--;
        if (new_zoom < limit_min) {
            new_zoom = limit_min;
        } else
        if (new_zoom > limit_max) {
            new_zoom = limit_max;
        }
        break;
    case 3:
        new_zoom = (4 * old_zoom) / 5;
        if (new_zoom == old_zoom)
            new_zoom--;
        if (new_zoom < 16) {
            new_zoom = 16;
        } else
        if (new_zoom > 1024) {
            new_zoom = 1024;
        }
        break;
    case 5:
        new_zoom = (85 * old_zoom) / 100;
        if (new_zoom == old_zoom)
            new_zoom--;
        if (new_zoom < 16384) {
            new_zoom = 16384;
        } else
        if (new_zoom > 65536) {
            new_zoom = 65536;
        }
        break;
    default:
        new_zoom = old_zoom;
    }
    set_camera_zoom(cam, new_zoom);
}

/**
 * Conducts clipping to zoom level of given camera, based on current screen mode.
 */
void update_camera_zoom_bounds(struct Camera *cam,unsigned long zoom_max,unsigned long zoom_min)
{
    SYNCDBG(7,"Starting");
    long zoom_val;
    zoom_val = get_camera_zoom(cam);
    if (zoom_val < zoom_min)
    {
      zoom_val = zoom_min;
    } else
    if (zoom_val > zoom_max)
    {
      zoom_val = zoom_max;
    }
    set_camera_zoom(cam, zoom_val);
}

long get_camera_zoom(struct Camera *cam)
{
    if (cam == NULL)
      return 0;
    //return _DK_get_camera_zoom(cam);
    switch (cam->field_6)
    {
    case 2:
    case 5:
        return cam->zoom;
    case 3:
        return cam->mappos.z.val;
    default:
        return 0;
    }
}

unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl)
{
    unsigned long amp,sizer,square;
    sizer = pixel_size*(long)units_per_pixel;
    amp = ((zoom_lvl*sizer) >> 4);
    square = LbSqrL(sizer);
    return  (amp >> 1) + ((amp*square) >> 3);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
