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
#include "bflib_planar.h"

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_math.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void LbSetRect(struct TbRect *rect, long xLeft, long yTop, long xRight, long yBottom)
{
    if (rect == NULL)
        return;
    rect->left = xLeft;
    rect->top = yTop;
    rect->right = xRight;
    rect->bottom = yBottom;
}

/**
 * Returns symmetrical difference between angles, ranged -LbFPMath_PI to LbFPMath_PI.
 * @param angle_a
 * @param angle_b
 */
long get_angle_symmetric_difference(long angle_a, long angle_b)
{
    long diff = (angle_a & LbFPMath_AngleMask) - (angle_b & LbFPMath_AngleMask);
    if (diff > LbFPMath_PI)
        diff = (2*LbFPMath_PI - diff);
    else
    if (diff < -LbFPMath_PI)
        diff = (2*LbFPMath_PI + diff);
    return diff;
}

/**
 * Returns unsigned difference between angles, ranged 0 to LbFPMath_PI.
 * Information about sign of the angle is not provided.
 * @param angle_a
 * @param angle_b
 */
long get_angle_difference(long angle_a, long angle_b)
{
    long diff = abs((angle_a & LbFPMath_AngleMask) - (angle_b & LbFPMath_AngleMask));
    if (diff > LbFPMath_PI)
        diff = (2*LbFPMath_PI - diff);
    return diff;
}

long get_angle_sign(long angle_a, long angle_b)
{
    long diff = (angle_b & LbFPMath_AngleMask) - (angle_a & LbFPMath_AngleMask);
    if (diff == 0)
        return 0;
    if (abs(diff) > LbFPMath_PI)
    {
      if (diff >= 0)
          diff -= 2*LbFPMath_PI;
      else
          diff += 2*LbFPMath_PI;
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
long distance_with_angle_to_coord_x(long distance, long angle)
{
    long long val = (long long)distance * LbSinL(angle);
    return val >> 16;
}

/**
 * Gives Y coordinate of a planar position shift by given distance into given direction.
 * @param distance Specifies the distance to move.
 * @param angle Specifies the movement direction.
 */
long distance_with_angle_to_coord_y(long distance, long angle)
{
    long long val = (long long)distance * LbCosL(angle);
    return (-(val >> 8)) >> 8;
}

long get_distance_xy(long x1, long y1, long x2, long y2)
{
    long dx = abs(x1 - x2);
    long dy = abs(y1 - y2);
    return LbDiagonalLength(dx, dy);
}

/**
 * Gives X coordinate of a 3D position shift by given distance into given direction.
 * @param distance Specifies the distance to move.
 * @param angle_a Specifies the movement rotation a.
 * @param angle_b Specifies the movement rotation b.
 */
long distance3d_with_angles_to_coord_x(long distance, long angle_a, long angle_b)
{
    long long val = (LbSinL(angle_a)>> 8)
          * (distance * LbCosL(angle_b) >> 8);
    return val >> 16;
}

/**
 * Gives Y coordinate of a 3D position shift by given distance into given direction.
 * @param distance Specifies the distance to move.
 * @param angle_a Specifies the movement rotation a.
 * @param angle_b Specifies the movement rotation b.
 */
long distance3d_with_angles_to_coord_y(long distance, long angle_a, long angle_b)
{
    long long val = (LbCosL(angle_a) >> 8)
        * (distance * LbCosL(angle_b) >> 8);
    return (-(val >> 8)) >> 8;
}

/**
 * Gives new X coordinate after shifting planar position by given distance into given direction.
 * @param pos_x The source coordinate to be shifted.
 * @param distance Specifies the distance to move.
 * @param angle Specifies the movement direction.
 */
long move_coord_with_angle_x(long pos_x, long distance, long angle)
{
    long long val = (long long)distance * LbSinL(angle);
    return pos_x + (val >> 16);
}

/**
 * Gives new Y coordinate after shifting planar position by given distance into given direction.
 * @param pos_y The source coordinate to be shifted.
 * @param distance Specifies the distance to move.
 * @param angle Specifies the movement direction.
 */
long move_coord_with_angle_y(long pos_y, long distance, long angle)
{
    long long val = (long long)distance * LbCosL(angle);
    return pos_y + ((-(val >> 8)) >> 8);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
