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
#include "pre_inc.h"
#include "ariadne_tringls.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "ariadne_points.h"
#include "ariadne_edge.h"
#include "ariadne.h"
#include "gui_topmsg.h"
#include "post_inc.h"

#define EDGELEN_BITS 6

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

int32_t free_Triangles = 0;
struct Triangle Triangles[TRIANLGLES_COUNT];
int32_t count_Triangles = 0;
int32_t ix_Triangles = 0;

/******************************************************************************/
struct Triangle bad_triangle;
const int32_t MOD3[] = {0, 1, 2, 0, 1, 2};
/******************************************************************************/
int32_t tri_new(void)
{
    int32_t i;
    if (free_Triangles == -1)
    {
        i = ix_Triangles;
        if ((i < 0) || (i >= TRIANLGLES_COUNT))
        {
            ERRORLOG("ix_Triangles overflow, got %d",i);
            erstat_inc(ESE_NoFreeTriangls);
            return -1;
        }
        if (i > (TRIANLGLES_COUNT * 999 / 1000))
        {
            WARNLOG("TRIANGLES near limit: %d", i);
        }
        ix_Triangles++;
    } else
    {
        i = free_Triangles;
        if ((i < 0) || (i >= TRIANLGLES_COUNT))
        {
            ERRORLOG("free_Triangles overflow, got %d",i);
            erstat_inc(ESE_NoFreeTriangls);
            return -1;
        }
        free_Triangles = Triangles[i].tags[0];
    }
    Triangles[i].tree_alt = 0;
    count_Triangles++;
    return i;
}

void tri_dispose(int32_t tri_idx)
{
    int32_t pfree_idx = free_Triangles;
    free_Triangles = tri_idx;
    Triangles[tri_idx].tags[0] = pfree_idx;
    Triangles[tri_idx].tree_alt = NAV_COL_UNSET;
    count_Triangles--;
}

int32_t get_triangle_region_id(int32_t tri_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return -1;
    return (Triangles[tri_id].region_and_edgelen >> EDGELEN_BITS);
}

TbBool set_triangle_region_id(int32_t tri_id, int32_t reg_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return false;
    Triangles[tri_id].region_and_edgelen &= ((1 << EDGELEN_BITS) - 1);
    Triangles[tri_id].region_and_edgelen |= (reg_id << EDGELEN_BITS);
    return true;
}

int32_t get_triangle_edgelen(int32_t tri_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return 0;
    return (Triangles[tri_id].region_and_edgelen & ((1 << EDGELEN_BITS) - 1));
}

TbBool set_triangle_edgelen(int32_t tri_id, int32_t edgelen)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return false;
    Triangles[tri_id].region_and_edgelen &= ~((1 << EDGELEN_BITS) - 1);
    Triangles[tri_id].region_and_edgelen |= edgelen;
    return true;
}

NavColour get_triangle_tree_alt(int32_t tri_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return -1;
    return Triangles[tri_id].tree_alt;
}

struct Triangle *get_triangle(int32_t tri_id)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return INVALID_TRIANGLE;
    return &Triangles[tri_id];
}

TbBool triangle_is_invalid(const struct Triangle *tri)
{
    return (tri < &Triangles[0]) || (tri > &Triangles[TRIANLGLES_COUNT-1]) || (tri == INVALID_TRIANGLE) || (tri == NULL);
}

struct Point *get_triangle_point(int32_t tri_id, int32_t pt_cor)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return INVALID_POINT;
    if ((pt_cor < 0) || (pt_cor >= 3))
        return INVALID_POINT;
    return point_get(Triangles[tri_id].points[pt_cor]);
}

TbBool triangle_tip_equals(int32_t tri_id, int32_t pt_cor, int32_t pt_x, int32_t pt_y)
{
    if ((tri_id < 0) || (tri_id >= TRIANLGLES_COUNT))
        return false;
    if ((pt_cor < 0) || (pt_cor >= 3))
        return false;
    int32_t pt_id = Triangles[tri_id].points[pt_cor];
    return point_equals(pt_id, pt_x, pt_y);
}

int32_t link_find(int32_t ntri, int32_t val)
{
    if ((ntri < 0) || (ntri >= TRIANLGLES_COUNT))
    {
        return -1;
    }
    for (int32_t i = 0; i < 3; i++)
    {
        if (Triangles[ntri].tags[i] == val)
        {
            return i;
        }
    }
    return -1;
}

TbBool outer_locked(int32_t ntri, int32_t ncor)
{
    int32_t n = Triangles[ntri].tags[ncor];
    int32_t shft = link_find(n, ntri);
    if (shft < 0)
    {
        ERRORLOG("border edge");
        return true;
    }
    return ( (Triangles[n].navigation_flags & (1 << (shft + 3)) ) != 0);
}

int32_t point_loop(int32_t pt_tri, int32_t pt_cor)
{
    int32_t ntri = pt_tri;
    int32_t ncor = pt_cor;
    if (ntri < 0)
        return -1;
    int32_t k = 0;
    int32_t n;
    do
    {
        n = Triangles[ntri].tags[ncor];
        int32_t i = link_find(n, ntri);
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

void edgelen_set(int32_t tri_id)
{
    NAVIDBG(19,"Starting");
    static const uint32_t EdgeLenBits[][4] = {
        {1, 1, 2, 3},
        {1, 1, 2, 3},
        {2, 2, 2, 3},
        {3, 3, 3, 3}
    };
    struct Triangle* tri = &Triangles[tri_id];
    int pt2idx = tri->points[2];
    int pt0idx = tri->points[0];
    int delta_x = abs(ari_Points[pt2idx].x - ari_Points[pt0idx].x);
    int delta_y = abs(ari_Points[pt2idx].y - ari_Points[pt0idx].y);
    if (delta_x > 3)
        delta_x = 3;
    if (delta_y > 3)
        delta_y = 3;
    uint32_t edge_len = (EdgeLenBits[delta_y][delta_x] << 4);
    int pt1idx = tri->points[1];
    delta_x = abs(ari_Points[pt1idx].x - ari_Points[pt2idx].x);
    delta_y = abs(ari_Points[pt1idx].y - ari_Points[pt2idx].y);
    if (delta_x > 3)
        delta_x = 3;
    if (delta_y > 3)
        delta_y = 3;
    edge_len |= (EdgeLenBits[delta_y][delta_x] << 2);
    delta_x = abs(ari_Points[pt0idx].x - ari_Points[pt1idx].x);
    delta_y = abs(ari_Points[pt0idx].y - ari_Points[pt1idx].y);
    if (delta_x > 3)
        delta_x = 3;
    if (delta_y > 3)
        delta_y = 3;
    edge_len |= (EdgeLenBits[delta_y][delta_x] << 0);
    set_triangle_edgelen(tri_id, edge_len);
}

int32_t edge_rotateAC(int32_t tri1_id, int32_t cor1_id)
{
    int32_t tri2_id = Triangles[tri1_id].tags[cor1_id];
    if (tri2_id == -1) {
        return false;
    }
    int cor2_id = link_find(tri2_id, tri1_id);
    if ( cor2_id == -1 ) {
        ERRORLOG("lB not found");
    }

    int cor1b_id = MOD3[cor1_id + 1];
    int cor2b_id = MOD3[cor2_id + 1];
    int cor1c_id = MOD3[cor1_id + 2];
    int cor2c_id = MOD3[cor2_id + 2];

    int32_t tri3_id = Triangles[tri1_id].tags[cor1b_id];
    int32_t tri4_id = Triangles[tri2_id].tags[cor2b_id];

    {
        unsigned short tri1_fld = Triangles[tri1_id].navigation_flags;
        unsigned short tri2_fld = Triangles[tri2_id].navigation_flags;
        if ( ((1 << (cor1_id + 3)) & tri1_fld) || ((1 << (cor2_id + 3)) & tri2_fld) ) {
            return false;
        }
        Triangles[tri1_id].navigation_flags &= ~(1 << (cor1b_id + 3));
        Triangles[tri2_id].navigation_flags &= ~(1 << (cor2b_id + 3));
        Triangles[tri1_id].navigation_flags &= ~(1 << (cor1_id + 3));
        Triangles[tri1_id].navigation_flags |= ((((1 << (cor2b_id + 3)) & tri2_fld) != 0) << (cor1_id + 3));
        Triangles[tri2_id].navigation_flags &= ~(1 << (cor2_id + 3));
        Triangles[tri2_id].navigation_flags |= ((((1 << (cor1b_id + 3)) & tri1_fld) != 0) << (cor2_id + 3));
    }

    int pt1_id = Triangles[tri1_id].points[cor1_id];
    int pt2_id = Triangles[tri1_id].points[cor1c_id];
    int32_t diff_ax = ari_Points[pt1_id].x - ari_Points[pt2_id].x;
    int32_t diff_ay = ari_Points[pt1_id].y - ari_Points[pt2_id].y;
    int pt3_id = Triangles[tri2_id].points[cor2c_id];
    int32_t diff_bx = ari_Points[pt3_id].x - ari_Points[pt2_id].x;
    int32_t diff_by = ari_Points[pt3_id].y - ari_Points[pt2_id].y;
    if (LbCompareMultiplications(diff_ay, diff_bx, diff_ax, diff_by) <= 0) {
        return false;
    }
    int pt4_id = Triangles[tri2_id].points[cor2_id];
    int32_t diff_cx = ari_Points[pt4_id].x - ari_Points[pt2_id].x;
    int32_t diff_cy = ari_Points[pt4_id].y - ari_Points[pt2_id].y;
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
        int tmcor_id = link_find(tri3_id, tri1_id);
        if (tmcor_id == -1) {
            ERRORLOG("A not found");
        }
        Triangles[tri3_id].tags[tmcor_id] = tri2_id;
    }
    if (tri4_id != -1)
    {
        int tmcor_id = link_find(tri4_id, tri2_id);
        if (tmcor_id == -1) {
            ERRORLOG("B not found");
        }
        Triangles[tri4_id].tags[tmcor_id] = tri1_id;
    }
    unsigned short tri1_fld = Triangles[tri1_id].navigation_flags;
    unsigned short tri2_fld = Triangles[tri2_id].navigation_flags;
    Triangles[tri1_id].navigation_flags &= ~(1 << cor1_id);
    Triangles[tri1_id].navigation_flags |= ((((1 << cor2b_id) & tri2_fld) != 0) << cor1_id);
    Triangles[tri1_id].navigation_flags &= ~(1 << cor1b_id);
    Triangles[tri1_id].navigation_flags |= ((((1 << cor1_id) & tri1_fld) != 0) << cor1b_id);
    Triangles[tri2_id].navigation_flags &= ~(1 << cor2_id);
    Triangles[tri2_id].navigation_flags |= ((((1 << cor1b_id) & tri1_fld) != 0) << cor2_id);
    Triangles[tri2_id].navigation_flags &= ~(1 << cor2b_id);
    Triangles[tri2_id].navigation_flags |= ((((1 << cor2_id) & tri2_fld) != 0) << cor2b_id);
    edgelen_set(tri1_id);
    edgelen_set(tri2_id);
    return true;
}
int32_t reduce_point(int32_t *pt_tri, int32_t *pt_cor)
{
    int32_t k = 0;
    int32_t ntri = *pt_tri;
    int32_t ncor = *pt_cor;
    int32_t first_tri = *pt_tri;
    int32_t ctri;
    do
    {
        ctri = Triangles[ntri].tags[ncor];
        if (ctri < 0)
            return -1;
        if (!edge_rotateAC(ntri, ncor))
        {
            int32_t i = link_find(ctri, ntri);
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

int32_t triangle_find_first_used(void)
{
    for (int32_t tri_idx = 0; tri_idx < ix_Triangles; tri_idx++)
    {
        struct Triangle* tri = &Triangles[tri_idx];
        if (tri->tree_alt != NAV_COL_UNSET) {
            return tri_idx;
        }
    }
    ERRORLOG("not found!!");
    return -1;
}

void triangulation_init_triangles(int32_t pt_id1, int32_t pt_id2, int32_t pt_id3, int32_t pt_id4)
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
    Triangles[0].tree_alt = NAVMAP_FLOORHEIGHT_MAX;
    Triangles[1].tags[0] = 0;
    Triangles[1].tree_alt = NAVMAP_FLOORHEIGHT_MAX;
    Triangles[1].tags[1] = -1;
    Triangles[1].tags[2] = -1;
    Triangles[0].navigation_flags = 7;
    Triangles[0].region_and_edgelen = 0;
    Triangles[1].region_and_edgelen = 0;
    Triangles[1].navigation_flags = 7;
}

char triangle_divide_areas_s8differ(int32_t ntri, int32_t ncorA, int32_t ncorB, int32_t pt_x, int32_t pt_y)
{
    struct Point* pt = get_triangle_point(ntri, ncorA);
    int32_t tipA_x = (pt->x << 8);
    int32_t tipA_y = (pt->y << 8);
    pt = get_triangle_point(ntri,ncorB);
    int32_t tipB_x = (pt->x << 8);
    int32_t tipB_y = (pt->y << 8);
    return LbCompareMultiplications(tipA_x-pt_x, tipB_y-pt_y, tipA_y-pt_y, tipB_x-pt_x);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
