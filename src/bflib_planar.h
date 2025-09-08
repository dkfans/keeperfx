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
#include "globals.h"

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

/**
 * This distance is "the number of moves needed by a king to move from one tile to another on a chess board".
 *
 * This is known as Chebyshev distance (see https://en.wikipedia.org/wiki/Chebyshev_distance for details).
 */
#define chessboard_distance(x1,y1,x2,y2) (max(abs(x1 - x2), abs(y1 - y2)))

/**
 * This distance is "the number of moves needed by a king to move from one cube to another on a 3d chess board".
 *
 * This is known as Chebyshev distance (see https://en.wikipedia.org/wiki/Chebyshev_distance and https://en.wikipedia.org/wiki/Three-dimensional_chess for details).
 */
#define chessboard_3d_distance(x1,y1,z1,x2,y2,z2) (max(max(abs(x1 - x2), abs(y1 - y2)),abs(z1 - z2)))

/**
 * This distance is "the number of moves needed to move from one tile on a grid to another tile on a grid; where each move must be directly up, down, left or right (Like D&D)".
 *
 * This is known as Manhattan distance (see https://simple.wikipedia.org/wiki/Manhattan_distance and https://en.wikipedia.org/wiki/Taxicab_geometry for details).
 */
#define grid_distance(x1,y1,x2,y2) (abs(x1 - x2) + abs(y1 - y2))
/******************************************************************************/
void LbSetRect(struct TbRect *rect, long xLeft, long yTop, long xRight, long yBottom);

long get_angle_symmetric_difference(long angle_a, long angle_b);
long get_angle_difference(long angle_a, long angle_b);
long get_angle_sign(long angle_a, long angle_b);

long distance_with_angle_to_coord_x(long distance, long angle);
long distance_with_angle_to_coord_y(long distance, long angle);

long get_distance_xy(long x1, long x2, long y1, long y2);
MapCoordDelta get_chessboard_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);
MapCoordDelta get_chessboard_3d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2);

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
