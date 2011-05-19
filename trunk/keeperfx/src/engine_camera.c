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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_get_angle_xy_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
DLLIMPORT long _DK_get_angle_yz_to(const struct Coord3d *pos1, const struct Coord3d *pos2);
DLLIMPORT long _DK_get_2d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
DLLIMPORT void _DK_project_point_to_wall_on_angle(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4, long a5, long a6);
DLLIMPORT void _DK_angles_to_vector(short angle_xy, short angle_yz, long dist, struct ComponentVector *cvect);
/******************************************************************************/
long get_3d_box_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
  long dx;
  long dy;
  long dz;
  dy = abs(pos2->y.val - pos1->y.val);
  dx = abs(pos2->x.val - pos1->x.val);
  if (dy <= dx)
    dy = dx;
  dz = abs(pos2->z.val - pos1->z.val);
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
  _DK_angles_to_vector(angle_xy, angle_yz, dist, cvect); return;
  // TODO: fix and enable
  long sin_yz,cos_yz,sin_xy,cos_xy;
  long long mag,factor;
  cos_yz = LbCosL(angle_yz) >> 2;
  sin_yz = LbSinL(angle_yz) >> 2;
  cos_xy = LbCosL(angle_xy) >> 2;
  sin_xy = LbSinL(angle_xy) >> 2;
  mag = (dist << 14) - cos_yz;
  factor = sin_xy * mag;
  cvect->x = (factor >> 14) >> 14;
  factor = cos_xy * mag;
  cvect->y = -(factor >> 14) >> 14;
  factor = dist * sin_yz;
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
    unsigned long dist_x,dist_y;
    //return _DK_get_2d_distance(pos1, pos2);
    dist_x = abs((long)pos1->x.val - (long)pos2->x.val);
    dist_y = abs((long)pos1->y.val - (long)pos2->y.val);
    return LbProportion(dist_x, dist_y);
}

void project_point_to_wall_on_angle(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4, long a5, long a6)
{
  _DK_project_point_to_wall_on_angle(pos1, pos2, a3, a4, a5, a6);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
