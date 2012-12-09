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

long get_angle_difference(long angle_a, long angle_b)
{
    long diff;
    diff = abs((angle_a & 0x7FF) - (angle_b & 0x7FF));
    if (diff > 1024)
        diff = (2048 - diff);
    return diff;
}

long get_angle_sign(long angle_a, long angle_b)
{
    long diff;
    diff = (angle_b & 0x7FF) - (angle_a & 0x7FF);
    if (diff == 0)
        return 0;
    if (abs(diff) > 1024)
    {
      if (diff >= 0)
          diff -= 2048;
      else
          diff += 2048;
    }
    if (diff == 0)
        return 0;
    return diff / abs(diff);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
