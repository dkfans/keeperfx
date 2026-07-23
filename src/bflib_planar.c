/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_planar.c
 *     Basic planar integer geometry.
 * @par Purpose:
 *     Simple geometry transformations unification.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     24 Jan 2009 - 08 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_planar.h"

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_math.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void LbSetRect(struct TbRect *rect, int32_t xLeft, int32_t yTop, int32_t xRight, int32_t yBottom)
{
    if (rect == NULL)
        return;
    rect->left = xLeft;
    rect->top = yTop;
    rect->right = xRight;
    rect->bottom = yBottom;
}

/**
 * Returns unsigned difference between angles, ranged 0 to DEGREES_180.
 * Information about sign of the angle is not provided.
 * @param angle_a
 * @param angle_b
 */
int32_t get_angle_difference(int32_t angle_a, int32_t angle_b)
{
    int32_t diff = abs((angle_a & ANGLE_MASK) - (angle_b & ANGLE_MASK));
    if (diff > DEGREES_180)
        diff = (DEGREES_360 - diff);
    return diff;
}

int32_t get_angle_sign(int32_t angle_a, int32_t angle_b)
{
    int32_t diff = (angle_b & ANGLE_MASK) - (angle_a & ANGLE_MASK);
    if (diff == 0)
        return 0;
    if (abs(diff) > DEGREES_180)
    {
      if (diff >= 0)
          diff -= DEGREES_360;
      else
          diff += DEGREES_360;
    }
    if (diff == 0)
        return 0;
    return diff / abs(diff);
}

/**
 * Gives X coordinate of a planar position shift by given distance into given direction.
 * @param distance Specifies the distance to move.
 * @param angle Specifies the movement direction.
 */
int32_t distance_with_angle_to_coord_x(int32_t distance, int32_t angle)
{
    int64_t val = (int64_t)distance * LbSinL(angle);
    return val >> 16;
}

/**
 * Gives Y coordinate of a planar position shift by given distance into given direction.
 * @param distance Specifies the distance to move.
 * @param angle Specifies the movement direction.
 */
int32_t distance_with_angle_to_coord_y(int32_t distance, int32_t angle)
{
    int64_t val = (int64_t)distance * LbCosL(angle);
    return (-(val >> 8)) >> 8;
}

int32_t get_distance_xy(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t dx = abs(x1 - x2);
    int32_t dy = abs(y1 - y2);
    return LbDiagonalLength(dx, dy);
}

/**
 * This distance is "the number of moves needed by a king to move from one tile to another on a chess board".
 *
 * This is known as Chebyshev distance (see https://en.wikipedia.org/wiki/Chebyshev_distance for details).
 */
MapCoordDelta get_chessboard_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    return chessboard_distance(pos1->x.val, pos1->y.val, pos2->x.val, pos2->y.val);
}

/**
 * This distance is "the number of moves needed by a king to move from one cube to another on a 3d chess board".
 *
 * This is known as Chebyshev distance (see https://en.wikipedia.org/wiki/Chebyshev_distance and https://en.wikipedia.org/wiki/Three-dimensional_chess for details).
 */
MapCoordDelta get_chessboard_3d_distance(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
    return chessboard_3d_distance(pos1->x.val, pos1->y.val, pos1->z.val, pos2->x.val, pos2->y.val, pos2->z.val);
}

/**
 * Gives X coordinate of a 3D position shift by given distance into given direction.
 * @param distance Specifies the distance to move.
 * @param angle_a Specifies the movement rotation a.
 * @param angle_b Specifies the movement rotation b.
 */
int32_t distance3d_with_angles_to_coord_x(int32_t distance, int32_t angle_a, int32_t angle_b)
{
    int64_t val = (LbSinL(angle_a)>> 8)
          * (distance * LbCosL(angle_b) >> 8);
    return val >> 16;
}

/**
 * Gives Y coordinate of a 3D position shift by given distance into given direction.
 * @param distance Specifies the distance to move.
 * @param angle_a Specifies the movement rotation a.
 * @param angle_b Specifies the movement rotation b.
 */
int32_t distance3d_with_angles_to_coord_y(int32_t distance, int32_t angle_a, int32_t angle_b)
{
    int64_t val = (LbCosL(angle_a) >> 8)
        * (distance * LbCosL(angle_b) >> 8);
    return (-(val >> 8)) >> 8;
}

/**
 * Gives new X coordinate after shifting planar position by given distance into given direction.
 * @param pos_x The source coordinate to be shifted.
 * @param distance Specifies the distance to move.
 * @param angle Specifies the movement direction.
 */
int32_t move_coord_with_angle_x(int32_t pos_x, int32_t distance, int32_t angle)
{
    int64_t val = (int64_t)distance * LbSinL(angle);
    return pos_x + (val >> 16);
}

/**
 * Gives new Y coordinate after shifting planar position by given distance into given direction.
 * @param pos_y The source coordinate to be shifted.
 * @param distance Specifies the distance to move.
 * @param angle Specifies the movement direction.
 */
int32_t move_coord_with_angle_y(int32_t pos_y, int32_t distance, int32_t angle)
{
    int64_t val = (int64_t)distance * LbCosL(angle);
    return pos_y + ((-(val >> 8)) >> 8);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
