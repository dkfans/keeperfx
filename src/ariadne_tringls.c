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
#include "ariadne_edge.h"
#include "ariadne.h"
#include "gui_topmsg.h"

#define EDGELEN_BITS 6

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#if USE_ORIGINAL_TRIANGLES_DATA
DLLIMPORT long _DK_free_Triangles;
#define free_Triangles _DK_free_Triangles
#else
long free_Triangles = 0;
struct Triangle Triangles[TRIANLGLES_COUNT];
long count_Triangles = 0;
long ix_Triangles = 0;
#endif

/******************************************************************************/
struct Triangle bad_triangle;
const long MOD3[] = {0, 1, 2, 0, 1, 2};
/******************************************************************************/
long tri_new(void)
{
    long i;
    if (free_Triangles == -1)
    {
        i = ix_Triangles;
        if ((i < 0) || (i >= TRIANLGLES_COUNT))
        {
            ERRORLOG("ix_Triangles overflow, got %ld",i);
            erstat_inc(ESE_NoFreeTriangls);
            return -1;
        }
        ix_Triangles++;
    } else
    {
        i = free_Triangles;
        if ((i < 0) || (i >= TRIANLGLES_COUNT))
        {
            ERRORLOG("free_Triangles overflow, got %ld",i);
            erstat_inc(ESE_NoFreeTriangls);
            return -1;
        }
        free_Triangles = Triangles[i].tags[0];
    }
    Triangles[i].tree_alt = 0;
    count_Triangles++;
    return i;
}

void tri_dispose(long tri_idx)
{
    long pfree_idx;
    pfree_idx = free_Triangles;
    free_Triangles = tri_idx;
    Triangles[tri_idx].tags[0] = pfree_idx;
    Triangles[tri_idx].tree_alt = 255;
    count_Triangles--;
}

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

TbBool triangle_is_invalid(const struct Triangle *tri)
{
    return (tri < &Triangles[0]) || (tri > &Triangles[TRIANLGLES_COUNT-1]) || (tri == INVALID_TRIANGLE) || (tri == NULL);
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
        if (Triangles[ntri].tags[i] == val)
        {
            return i;
        }
    }
    return -1;
}

TbBool outer_locked(long ntri, long ncor)
{
    long shft,n;
    n = Triangles[ntri].tags[ncor];
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
        n = Triangles[ntri].tags[ncor];
        i = link_find(n, ntri);
        if (i < 0)
            return -1;
        ncor = MOD3[i+1];
        ntri = n;
        k++;
        if (k > TRIANLGLES_COUNT)
            return -1;
    } while (n != pt_tri);
    return k;
}

void edgelen_set(long tri_id)
{
    NAVIDBG(19,"Starting");
    //_DK_edgelen_set(tri_id); return;
    static const unsigned long EdgeLenBits[][4] = {
        {1, 1, 2, 3},
        {1, 1, 2, 3},
        {2, 2, 2, 3},
        {3, 3, 3, 3}
    };
    struct Triangle *tri;
    tri = &Triangles[tri_id];
    int pt0idx;
    int pt2idx;
    pt2idx = tri->points[2];
    pt0idx = tri->points[0];
    int delta_x, delta_y;
    delta_x = abs(Points[pt2idx].x - Points[pt0idx].x);
    delta_y = abs(Points[pt2idx].y - Points[pt0idx].y);
    if (delta_x > 3)
        delta_x = 3;
    if (delta_y > 3)
        delta_y = 3;
    unsigned long edge_len;
    edge_len = (EdgeLenBits[delta_y][delta_x] << 4);
    int pt1idx;
    pt1idx = tri->points[1];
    delta_x = abs(Points[pt1idx].x - Points[pt2idx].x);
    delta_y = abs(Points[pt1idx].y - Points[pt2idx].y);
    if (delta_x > 3)
        delta_x = 3;
    if (delta_y > 3)
        delta_y = 3;
    edge_len |= (EdgeLenBits[delta_y][delta_x] << 2);
    delta_x = abs(Points[pt0idx].x - Points[pt1idx].x);
    delta_y = abs(Points[pt0idx].y - Points[pt1idx].y);
    if (delta_x > 3)
        delta_x = 3;
    if (delta_y > 3)
        delta_y = 3;
    edge_len |= (EdgeLenBits[delta_y][delta_x] << 0);
    set_triangle_edgelen(tri_id, edge_len);
}

long edge_rotateAC(long tri1_id, long cor1_id)
{
    //return _DK_edge_rotateAC(tri1_id, cor1_id);
    long tri2_id;
    tri2_id = Triangles[tri1_id].tags[cor1_id];
    if (tri2_id == -1) {
        return false;
    }
    int cor2_id;
    cor2_id = link_find(tri2_id, tri1_id);
    if ( cor2_id == -1 ) {
        ERRORLOG("lB not found");
    }

    int cor1b_id, cor2b_id, cor1c_id, cor2c_id;
    cor1b_id = MOD3[cor1_id+1];
    cor2b_id = MOD3[cor2_id+1];
    cor1c_id = MOD3[cor1_id+2];
    cor2c_id = MOD3[cor2_id+2];

    long tri3_id, tri4_id;
    tri3_id = Triangles[tri1_id].tags[cor1b_id];
    tri4_id = Triangles[tri2_id].tags[cor2b_id];

    {
        unsigned short tri2_fld, tri1_fld;
        tri1_fld = Triangles[tri1_id].field_D;
        tri2_fld = Triangles[tri2_id].field_D;
        if ( ((1 << (cor1_id + 3)) & tri1_fld) || ((1 << (cor2_id + 3)) & tri2_fld) ) {
            return false;
        }
        Triangles[tri1_id].field_D &= ~(1 << (cor1b_id + 3));
        Triangles[tri2_id].field_D &= ~(1 << (cor2b_id + 3));
        Triangles[tri1_id].field_D &= ~(1 << (cor1_id + 3));
        Triangles[tri1_id].field_D |= ((((1 << (cor2b_id + 3)) & tri2_fld) != 0) << (cor1_id + 3));
        Triangles[tri2_id].field_D &= ~(1 << (cor2_id + 3));
        Triangles[tri2_id].field_D |= ((((1 << (cor1b_id + 3)) & tri1_fld) != 0) << (cor2_id + 3));
    }

    int pt1_id, pt2_id, pt3_id, pt4_id;
    pt1_id = Triangles[tri1_id].points[cor1_id];
    pt2_id = Triangles[tri1_id].points[cor1c_id];
    long diff_ax, diff_ay;
    diff_ax = Points[pt1_id].x - Points[pt2_id].x;
    diff_ay = Points[pt1_id].y - Points[pt2_id].y;
    pt3_id = Triangles[tri2_id].points[cor2c_id];
    long diff_bx, diff_by;
    diff_bx = Points[pt3_id].x - Points[pt2_id].x;
    diff_by = Points[pt3_id].y - Points[pt2_id].y;
    if (LbCompareMultiplications(diff_ay, diff_bx, diff_ax, diff_by) <= 0) {
        return false;
    }
    pt4_id = Triangles[tri2_id].points[cor2_id];
    long diff_cx, diff_cy;
    diff_cx = Points[pt4_id].x - Points[pt2_id].x;
    diff_cy = Points[pt4_id].y - Points[pt2_id].y;
    if (LbCompareMultiplications(diff_cy, diff_bx, diff_cx, diff_by) >= 0) {
        return false;
    }
    Triangles[tri1_id].points[cor1b_id] = pt3_id;
    Triangles[tri2_id].points[cor2b_id] = pt2_id;
    Triangles[tri1_id].tags[cor1_id] = tri4_id;
    Triangles[tri1_id].tags[cor1b_id] = tri2_id;
    Triangles[tri2_id].tags[cor2_id] = tri3_id;
    Triangles[tri2_id].tags[cor2b_id] = tri1_id;
    if (tri3_id != -1)
    {
        int tmcor_id;
        tmcor_id = link_find(tri3_id, tri1_id);
        if (tmcor_id == -1) {
            ERRORLOG("A not found");
        }
        Triangles[tri3_id].tags[tmcor_id] = tri2_id;
    }
    if (tri4_id != -1)
    {
        int tmcor_id;
        tmcor_id = link_find(tri4_id, tri2_id);
        if (tmcor_id == -1) {
            ERRORLOG("B not found");
        }
        Triangles[tri4_id].tags[tmcor_id] = tri1_id;
    }
    unsigned short tri2_fld, tri1_fld;
    tri1_fld = Triangles[tri1_id].field_D;
    tri2_fld = Triangles[tri2_id].field_D;
    Triangles[tri1_id].field_D &= ~(1 << cor1_id);
    Triangles[tri1_id].field_D |= ((((1 << cor2b_id) & tri2_fld) != 0) << cor1_id);
    Triangles[tri1_id].field_D &= ~(1 << cor1b_id);
    Triangles[tri1_id].field_D |= ((((1 << cor1_id) & tri1_fld) != 0) << cor1b_id);
    Triangles[tri2_id].field_D &= ~(1 << cor2_id);
    Triangles[tri2_id].field_D |= ((((1 << cor1b_id) & tri1_fld) != 0) << cor2_id);
    Triangles[tri2_id].field_D &= ~(1 << cor2b_id);
    Triangles[tri2_id].field_D |= ((((1 << cor2_id) & tri2_fld) != 0) << cor2b_id);
    edgelen_set(tri1_id);
    edgelen_set(tri2_id);
    return true;
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
        ctri = Triangles[ntri].tags[ncor];
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

long triangle_find_first_used(void)
{
    long tri_idx;
    for (tri_idx = 0; tri_idx < ix_Triangles; tri_idx++)
    {
        struct Triangle *tri;
        tri = &Triangles[tri_idx];
        if (tri->tree_alt != 255) {
            return tri_idx;
        }
    }
    ERRORLOG("not found!!");
    return -1;
}

void triangulation_init_triangles(long pt_id1, long pt_id2, long pt_id3, long pt_id4)
{
    free_Triangles = -1;
    ix_Triangles = 2;
    count_Triangles = 2;
    Triangles[0].points[1] = 1;
    Triangles[1].points[0] = 1;
    Triangles[1].points[2] = 2;
    Triangles[0].tags[0] = 1;
    Triangles[0].points[0] = 3;
    Triangles[0].points[2] = 0;
    Triangles[1].points[1] = 3;
    Triangles[0].tags[1] = -1;
    Triangles[0].tags[2] = -1;
    Triangles[0].tree_alt = 15;
    Triangles[1].tags[0] = 0;
    Triangles[1].tree_alt = 15;
    Triangles[1].tags[1] = -1;
    Triangles[1].tags[2] = -1;
    Triangles[0].field_D = 7;
    Triangles[0].field_E = 0;
    Triangles[1].field_E = 0;
    Triangles[1].field_D = 7;
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
