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
/**
 * Checks if there's space for given amount of points.
 * Quite useless, as we never know if a new point will be inserted or existing will be reused.
 * @param n Amount of point required.
 * @return Returns whether there is space for given amount of points or not.
 */
TbBool has_free_points(long n)
{
    if (count_Points + n >= POINTS_COUNT)
        return false;
    return true;
}

long get_ix_points()
{
    return ix_Points;
}

long get_free_points()
{
    return free_Points;
}

AridPointId point_new(void)
{
    AridPointId i;
    if (free_Points == -1)
    {
        i = ix_Points;
        if ((i < 0) || (i >= POINTS_COUNT))
        {
            WARNLOG("ix_Points overflow; %d allocated, id %d outranged",(int)count_Points,(int)ix_Points);
            erstat_inc(ESE_NoFreePathPts);
            return -1;
        }
        ix_Points++;
    } else
    {
        i = free_Points;
        if ((i < 0) || (i >= POINTS_COUNT))
        {
            ERRORDBG(13,"free_Points overflow; %d allocated, id %d outranged",(int)count_Points,(int)free_Points);
            erstat_inc(ESE_NoFreePathPts);
            return -1;
        }
        free_Points = ari_Points[i].x;
    }
    ari_Points[i].y = 0;
    count_Points++;
    return i;
}

void point_dispose(AridPointId pt_id)
{
    AridPointId last_pt_id = free_Points;
    ari_Points[pt_id].y = 0x8000;
    free_Points = pt_id;
    ari_Points[pt_id].x = last_pt_id;
    count_Points--;
}

TbBool point_set(AridPointId pt_id, long x, long y)
{
    if ((pt_id < 0) || (pt_id >= POINTS_COUNT))
    {
        return false;
    }
    ari_Points[pt_id].x = x;
    ari_Points[pt_id].y = y;
    return true;
}

struct Point *point_get(AridPointId pt_id)
{
    if ((pt_id < 0) || (pt_id >= POINTS_COUNT))
    {
        return INVALID_POINT;
    }
    return &ari_Points[pt_id];
}

TbBool point_is_invalid(const struct Point *pt)
{
    return (pt < &ari_Points[0]) || (pt > &ari_Points[POINTS_COUNT-1]) || (pt == INVALID_POINT) || (pt == NULL);
}

TbBool point_equals(AridPointId pt_idx, long pt_x, long pt_y)
{
    if ((pt_idx < 0) || (pt_idx >= POINTS_COUNT))
        return false;
    long tip_x = ari_Points[pt_idx].x;
    long tip_y = ari_Points[pt_idx].y;
    if ((tip_x != pt_x) || (tip_y != pt_y))
        return false;
    return true;
}

AridPointId allocated_point_search(long pt_x, long pt_y)
{
    if (pt_y == 0x8000) {
        return -1;
    }
    for (AridPointId pt_idx = 0; pt_idx < POINTS_COUNT; pt_idx++)
    {
        long tip_x = ari_Points[pt_idx].x;
        long tip_y = ari_Points[pt_idx].y;
        if ((tip_x == pt_x) && (tip_y == pt_y)) {
            return pt_idx;
        }
    }
    return -1;
}

AridPointId point_set_new_or_reuse(long pt_x, long pt_y)
{
    AridPointId pt_idx = allocated_point_search(pt_x, pt_y);
    if (pt_idx >= 0) {
        return pt_idx;
    }
    pt_idx = point_new();
    if (pt_idx < 0) {
        return -1;
    }
    point_set(pt_idx, pt_x, pt_y);
    return pt_idx;
}

void triangulation_initxy_points(long startx, long starty, long endx, long endy)
{
    for (long i = 0; i < POINTS_COUNT; i++)
    {
        struct Point* pt = &ari_Points[i];
        pt->y = 0x8000;
    }
    ari_Points[0].x = startx;
    ari_Points[0].y = starty;
    ari_Points[1].x = endx;
    ari_Points[1].y = starty;
    ari_Points[2].x = endx;
    ari_Points[2].y = endy;
    ari_Points[3].x = startx;
    ari_Points[3].y = endy;
    ix_Points = 4;
    count_Points = 4;
    free_Points = -1;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
