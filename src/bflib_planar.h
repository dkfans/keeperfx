/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_planar.h
 *     Header file for bflib_planar.c.
 * @par Purpose:
 *     Basic planar integer geometry.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     24 Jan 2009 - 08 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_PLANAR_H
#define BFLIB_PLANAR_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbRect {
  long left;
  long top;
  long right;
  long bottom;
};

struct TbPoint {
  long x;
  long y;
};
/******************************************************************************/
void LbSetRect(struct TbRect *rect, long xLeft, long yTop, long xRight, long yBottom);

long get_angle_symmetric_difference(long angle_a, long angle_b);
long get_angle_difference(long angle_a, long angle_b);
long get_angle_sign(long angle_a, long angle_b);

long distance_with_angle_to_coord_x(long distance, long angle);
long distance_with_angle_to_coord_y(long distance, long angle);

long get_distance_xy(long x1, long x2, long y1, long y2);

long distance3d_with_angles_to_coord_x(long distance, long angle_a, long angle_b);
long distance3d_with_angles_to_coord_y(long distance, long angle_a, long angle_b);
#define distance_with_angle_to_coord_z(distance, angle) distance_with_angle_to_coord_x(distance, angle)
long move_coord_with_angle_x(long pos_x, long distance, long angle);
long move_coord_with_angle_y(long pos_y, long distance, long angle);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
