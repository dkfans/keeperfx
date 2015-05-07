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
#include "bflib_planar.h"

#include "engine_lenses.h"
#include "engine_render.h"
#include "vidmode.h"
#include "map_blocks.h"
#include "dungeon_data.h"
#include "player_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_view_set_camera_rotation_inertia(struct Camera *cam, long delta, long ilimit);
DLLIMPORT void _DK_init_player_cameras(struct PlayerInfo *player);
/******************************************************************************/
long camera_zoom;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
MapCoordDelta get_3d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
  long dist_x;
  long dist_y;
  long dist_z;
  dist_y = abs(pos2->y.val - (long)pos1->y.val);
  dist_x = abs(pos2->x.val - (long)pos1->x.val);
  if (dist_y <= dist_x)
    dist_y = dist_x;
  dist_z = abs(pos2->z.val - (long)pos1->z.val);
  if (dist_y <= dist_z)
    dist_y = dist_z;
  return dist_y;
}

MapCoordDelta get_2d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
  long dist_x,dist_y;
  dist_y = abs((long)pos1->y.val - (long)pos2->y.val);
  dist_x = abs((long)pos1->x.val - (long)pos2->x.val);
  if (dist_y <= dist_x)
    return dist_x;
  return dist_y;
}

MapCoordDelta get_2d_box_distance_xy(long pos1_x, long pos1_y, long pos2_x, long pos2_y)
{
    long dist_x,dist_y;
    dist_x = abs((long)pos1_x - (long)pos2_x);
    dist_y = abs((long)pos1_y - (long)pos2_y);
    if (dist_y <= dist_x)
      return dist_x;
    return dist_y;
}

void angles_to_vector(short angle_xy, short angle_yz, long dist, struct ComponentVector *cvect)
{
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

long get_angle_xy_to_vec(const struct CoordDelta3d *vec)
{
    return LbArcTanAngle(vec->x.val, vec->y.val) & LbFPMath_AngleMask;
}

long get_angle_yz_to_vec(const struct CoordDelta3d *vec)
{
    long dist;
    dist = LbDiagonalLength(abs(vec->x.val), abs(vec->y.val));
    return LbArcTanAngle(vec->z.val, dist) & LbFPMath_AngleMask;
}

long get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    return LbArcTanAngle((long)pos2->x.val - (long)pos1->x.val, (long)pos2->y.val - (long)pos1->y.val) & LbFPMath_AngleMask;
}

long get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist;
    dist = get_2d_distance(pos1, pos2);
    return LbArcTanAngle(pos2->z.val - pos1->z.val, dist) & LbFPMath_AngleMask;
}

MapCoordDelta get_2d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist_x,dist_y;
    dist_x = (long)pos1->x.val - (long)pos2->x.val;
    dist_y = (long)pos1->y.val - (long)pos2->y.val;
    return LbDiagonalLength(abs(dist_x), abs(dist_y));
}

MapCoordDelta get_2d_distance_squared(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    long dist_x,dist_y;
    dist_x = (long)pos1->x.val - (long)pos2->x.val;
    dist_y = (long)pos1->y.val - (long)pos2->y.val;
    return dist_x * dist_x + dist_y * dist_y;
}

void project_point_to_wall_on_angle(const struct Coord3d *pos1, struct Coord3d *pos2, long angle_xy, long angle_z, long distance, long num_steps)
{
    long dx,dy,dz;
    long n;
    struct Coord3d pos;
    dx = distance_with_angle_to_coord_x(distance,angle_xy);
    dy = distance_with_angle_to_coord_y(distance,angle_xy);
    dz = distance_with_angle_to_coord_z(distance,angle_z);
    pos.x.val = pos1->x.val;
    pos.y.val = pos1->y.val;
    pos.z.val = pos1->z.val;
    // Do num_steps until a solid wall is reached
    for (n=num_steps; n > 0; n--)
    {
        if (point_in_map_is_solid(&pos))
            break;
        pos.x.val += dx;
        pos.y.val += dy;
        pos.z.val += dz;
    }
    pos2->x.val = pos.x.val;
    pos2->y.val = pos.y.val;
    pos2->z.val = pos.z.val;
}

void view_zoom_camera_in(struct Camera *cam, long limit_max, long limit_min)
{
    long new_zoom,old_zoom;
    old_zoom = get_camera_zoom(cam);
    switch (cam->viewType)
    {
    case PVM_IsometricView:
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
    case PVM_ParchmentView:
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
    case PVM_FrontView:
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
    switch (cam->viewType)
    {
    case PVM_IsometricView:
    case PVM_FrontView:
        cam->zoom = new_zoom;
        break;
    case PVM_ParchmentView:
        cam->mappos.z.val = new_zoom;
        break;
    }
}

void view_zoom_camera_out(struct Camera *cam, long limit_max, long limit_min)
{
    long new_zoom,old_zoom;
    old_zoom = get_camera_zoom(cam);
    switch (cam->viewType)
    {
    case PVM_IsometricView:
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
    case PVM_ParchmentView:
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
    case PVM_FrontView:
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
    switch (cam->viewType)
    {
    case PVM_IsometricView:
    case PVM_FrontView:
        return cam->zoom;
    case PVM_ParchmentView:
        return cam->mappos.z.val;
    default:
        return 0;
    }
}

/** Scales camera zoom for current screen resolution.
 *
 * @param zoom_lvl Unscaled zoom level.
 * @return Zoom level scaled with use of current units_per_pixel value.
 */
unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl)
{
    unsigned long size_wide,size_narr;
    size_narr = ((pixel_size*units_per_pixel_min)<<7)/10;
    size_wide = (pixel_size*units_per_pixel) << 3;
    // Currently, the side menu isn't scaled. We have to take that into account. Side menu takes approx 0.22 of the screen.
    // Note that this is temporary - it would be better to scale the side menu. Not to mention making larger rendering arrays.
    if (units_per_pixel+units_per_pixel_min > 35) // this means resolution over 800x600
        size_wide += size_wide>>3;
    if (units_per_pixel+units_per_pixel_min > 55) // this means resolution over 1200x1024
        size_wide += size_wide>>4;
    return  ((zoom_lvl*size_wide) >> 8) + ((zoom_lvl*size_narr) >> 8);
}

void view_set_camera_y_inertia(struct Camera *cam, long deltaDelta, long ilimit, int interiaOn)
{
    long abslimit;
    abslimit = abs(ilimit);
    cam->verticalPosDelta += deltaDelta;

    if (cam->verticalPosDelta < -abslimit)
    {
        cam->verticalPosDelta = -abslimit;
    }
    else if (cam->verticalPosDelta > abslimit)
    {
        cam->verticalPosDelta = abslimit;
    }
    cam->verticalInteriaOn = interiaOn;
}

void view_set_camera_x_inertia(struct Camera *cam, long deltaDelta, long ilimit, int interiaOn)
{
    long abslimit;
    abslimit = abs(ilimit);
    cam->horizontalPosDelta += deltaDelta;

    if (cam->horizontalPosDelta < -abslimit)
    {
        cam->horizontalPosDelta = -abslimit;
    }
    else if (cam->horizontalPosDelta > abslimit)
    {
        cam->horizontalPosDelta = abslimit;
    }
    cam->horizontalInteriaOn = interiaOn;
}

void view_set_camera_rotation_inertia(struct Camera *cam, long deltaDelta, long ilimit, int interiaOn)
{
    //_DK_view_set_camera_rotation_inertia(cam, delta, ilimit);
    int limit_val;
    limit_val = abs(ilimit);
    cam->rotationDelta += deltaDelta;

    if (cam->rotationDelta < -limit_val)
    {
        cam->rotationDelta = -limit_val;
    }
    else if (cam->rotationDelta > limit_val)
    {
        cam->rotationDelta = limit_val;
    }
    cam->rotationInteriaOn = interiaOn;
}

void init_player_cameras(struct PlayerInfo *player)
{
    //_DK_init_player_cameras(player);
    struct Thing *heartng;
    heartng = get_player_soul_container(player->id_number);
    struct Camera *cam;

    cam = &player->cameras[CamIV_Isometric];
    cam->mappos.x.val = heartng->mappos.x.val;
    cam->mappos.y.val = heartng->mappos.y.val;
    cam->mappos.z.val = 0;
    cam->viewType = PVM_IsometricView;
    cam->orient_a = 256;
    cam->orient_b = -266;
    cam->orient_c = 0;
    cam->field_13 = 188;
    cam->zoom = 10000;

    cam = &player->cameras[CamIV_FirstPerson];
    cam->mappos.x.val = 0;
    cam->mappos.y.val = 0;
    cam->mappos.z.val = 256;
    cam->viewType = PVM_CreatureView;
    cam->orient_a = 512;
    cam->orient_b = 0;
    cam->orient_c = 0;
    cam->field_13 = 188;


    cam = &player->cameras[CamIV_Parchment];
    cam->mappos.x.val = 0;
    cam->mappos.y.val = 0;
    cam->mappos.z.val = 32;
    cam->viewType = PVM_ParchmentView;
    cam->field_13 = 188;

    cam = &player->cameras[CamIV_FrontView];
    cam->mappos.x.val = heartng->mappos.x.val;
    cam->mappos.y.val = heartng->mappos.y.val;
    cam->viewType = PVM_FrontView;
    cam->mappos.z.val = 32;
    cam->field_13 = 188;
    cam->zoom = 65536;
}
/******************************************************************************/
