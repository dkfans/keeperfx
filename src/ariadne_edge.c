/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_edge.c
 *     Ariadne system edge support functions.
 * @par Purpose:
 *     Functions to maintain edge-related structures.
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
#include "ariadne_edge.h"

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct EdgePoint _DK_EdgePoints[EDGE_POINTS_COUNT];
#define EdgePoints _DK_EdgePoints

/******************************************************************************/
void edge_points_clean(void)
{
    ix_EdgePoints = 0;
}

long edge_point_add(long pt_x, long pt_y)
{
    long ept_id = ix_EdgePoints;
    if (ept_id >= EDGE_POINTS_COUNT)
        return -1;
    EdgePoints[ept_id].field_0 = pt_x;
    EdgePoints[ept_id].field_4 = pt_y;
    ix_EdgePoints = ept_id+1;
    return ept_id;
}

struct EdgePoint *edge_point_get(long ept_id)
{
    if ((ept_id < 0) || (ept_id >= EDGE_POINTS_COUNT))
        return &EdgePoints[0];
    return &EdgePoints[ept_id];
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
