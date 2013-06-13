/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_points.c
 *     ariadne_points support functions.
 * @par Purpose:
 *     Functions to ariadne_points.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 22 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "ariadne_points.h"

#include "globals.h"
#include "bflib_basics.h"
#include "gui_topmsg.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
DLLIMPORT long _DK_count_Points;
#define count_Points _DK_count_Points
DLLIMPORT long _DK_ix_Points;
#define ix_Points _DK_ix_Points
DLLIMPORT long _DK_free_Points;
#define free_Points _DK_free_Points
/******************************************************************************/
long point_new(void)
{
  long i;
  if (free_Points == -1)
  {
      i = ix_Points;
      if ((i < 0) || (i >= POINTS_COUNT))
      {
          ERRORLOG("ix_Points overflow");
          erstat_inc(ESE_NoFreePathPts);
          return -1;
      }
      ix_Points++;
  } else
  {
      i = free_Points;
      if ((i < 0) || (i >= POINTS_COUNT))
      {
          ERRORLOG("free_Points overflow");
          erstat_inc(ESE_NoFreePathPts);
          return -1;
      }
      free_Points = Points[i].x;
  }
  Points[i].y = 0;
  count_Points++;
  return i;
}

void point_dispose(long pt_id)
{
    long last_pt_id;
    last_pt_id = free_Points;
    Points[pt_id].y = 0x8000;
    free_Points = pt_id;
    Points[pt_id].x = last_pt_id;
    count_Points--;
}

TbBool point_set(long pt_id, long x, long y)
{
    if ((pt_id < 0) || (pt_id >= POINTS_COUNT))
    {
        return false;
    }
    Points[pt_id].x = x;
    Points[pt_id].y = y;
    return true;
}

struct Point *point_get(long pt_id)
{
    if ((pt_id < 0) || (pt_id >= POINTS_COUNT))
    {
        return INVALID_POINT;
    }
    return &Points[pt_id];
}

TbBool point_equals(long pt_idx, long pt_x, long pt_y)
{
    long tip_x,tip_y;
    if ((pt_idx < 0) || (pt_idx >= POINTS_COUNT))
        return false;
    tip_x = Points[pt_idx].x;
    tip_y = Points[pt_idx].y;
    if ((tip_x != pt_x) || (tip_y != pt_y))
        return false;
    return true;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
