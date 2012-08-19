/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_tringls.c
 *     Triangles array for Ariadne system support functions.
 * @par Purpose:
 *     Functions to manage list of Triangles.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 22 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "ariadne_tringls.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "ariadne_points.h"

#define EDGELEN_BITS 6

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_edge_rotateAC(long a1, long a2);
DLLIMPORT void _DK_triangulation_initxy(long a1, long a2, long a3, long a4);
/******************************************************************************/
DLLIMPORT long _DK_tri_initialised;
#define tri_initialised _DK_tri_initialised
/******************************************************************************/
struct Triangle bad_triangle;
const long MOD3[] = {0, 1, 2, 0, 1, 2};
/******************************************************************************/
long get_triangle_region_id(long tri_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return -1;
    return (Triangles[tri_id].field_E >> EDGELEN_BITS);
}

TbBool set_triangle_region_id(long tri_id, long reg_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return false;
    Triangles[tri_id].field_E &= ((1 << EDGELEN_BITS) - 1);
    Triangles[tri_id].field_E |= (reg_id << EDGELEN_BITS);
    return true;
}

long get_triangle_edgelen(long tri_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return 0;
    return (Triangles[tri_id].field_E & ((1 << EDGELEN_BITS) - 1));
}

TbBool set_triangle_edgelen(long tri_id, long edgelen)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return false;
    Triangles[tri_id].field_E &= ~((1 << EDGELEN_BITS) - 1);
    Triangles[tri_id].field_E |= edgelen;
    return true;
}

long get_triangle_tree_alt(long tri_id)
{
    long tree_alt;
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return -1;
    tree_alt = Triangles[tri_id].tree_alt;
    if (tree_alt == 255)
        return -1;
    return tree_alt;
}

struct Triangle *get_triangle(long tri_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return INVALID_TRIANGLE;
    return &Triangles[tri_id];
}

struct Point *get_triangle_point(long tri_id, long pt_cor)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return INVALID_POINT;
    if ((pt_cor < 0) || (pt_cor >= 3))
        return INVALID_POINT;
    return point_get(Triangles[tri_id].points[pt_cor]);
}

TbBool triangle_tip_equals(long tri_id, long pt_cor, long pt_x, long pt_y)
{
    long pt_id;
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return false;
    if ((pt_cor < 0) || (pt_cor >= 3))
        return false;
    pt_id = Triangles[tri_id].points[pt_cor];
    return point_equals(pt_id, pt_x, pt_y);
}

long link_find(long ntri, long val)
{
    long i;
    if ((ntri < 0) || (ntri >= TRIANLGLES_COUNT))
    {
        return -1;
    }
    for (i=0; i < 3; i++)
    {
        if (Triangles[ntri].field_6[i] == val)
        {
            return i;
        }
    }
    return -1;
}

TbBool outer_locked(long ntri, long ncor)
{
    long shft,n;
    n = Triangles[ntri].field_6[ncor];
    shft = link_find(n, ntri);
    if (shft < 0)
    {
        ERRORLOG("border edge");
        return true;
    }
    return ( (Triangles[n].field_D & (1 << (shft + 3)) ) != 0);
}

long point_loop(long pt_tri, long pt_cor)
{
    long ntri,ncor;
    long i,k,n;
    ntri = pt_tri;
    ncor = pt_cor;
    if (ntri < 0)
        return -1;
    k = 0;
    do
    {
      n = Triangles[ntri].field_6[ncor];
      i = link_find(n, ntri);
      if (i < 0)
          return -1;
      ncor = MOD3[i+1];
      ntri = n;
      k++;
    } while (n != pt_tri);
    return k;
}

long edge_rotateAC(long a1, long a2)
{
    //Note: uses LbCompareMultiplications()
    return _DK_edge_rotateAC(a1, a2);
}

long reduce_point(long *pt_tri, long *pt_cor)
{
    long first_tri,ntri,ncor;
    long k,i,ctri;
    k = 0;
    ntri = *pt_tri;
    ncor = *pt_cor;
    first_tri = *pt_tri;
    do
    {
        ctri = Triangles[ntri].field_6[ncor];
        if (ctri < 0)
            return -1;
        if (!edge_rotateAC(ntri, ncor))
        {
            i = link_find(ctri, ntri);
            if (i < 0)
                return -1;
            ncor = MOD3[i+1];
            ntri = ctri;
            k++;
        }
    }
    while (ctri != first_tri);
    *pt_tri = ntri;
    *pt_cor = ncor;
    return k;
}

void triangulation_initxy(long a1, long a2, long a3, long a4)
{
    _DK_triangulation_initxy(a1, a2, a3, a4);
}

void triangulation_init(void)
{
    if (!tri_initialised)
    {
        tri_initialised = 1;
        triangulation_initxy(-256, -256, 512, 512);
    }
}

char triangle_divide_areas_s8differ(long ntri, long ncorA, long ncorB, long pt_x, long pt_y)
{
    long tipA_x,tipA_y;
    long tipB_x,tipB_y;
    struct Point *pt;

    pt = get_triangle_point(ntri,ncorA);
    tipA_x = (pt->x << 8);
    tipA_y = (pt->y << 8);
    pt = get_triangle_point(ntri,ncorB);
    tipB_x = (pt->x << 8);
    tipB_y = (pt->y << 8);
    return LbCompareMultiplications(tipA_x-pt_x, tipB_y-pt_y, tipA_y-pt_y, tipB_x-pt_x);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
