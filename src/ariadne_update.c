/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne.c
 *     Dungeon routing and path finding system.
 * @par Purpose:
 *     Defines functions for finding creature routes and navigating
 *     through the dungeon.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Jan 2010 - 20 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "ariadne.h"
#include "ariadne_tringls.h"
#include "ariadne_findcache.h"
#include "ariadne_points.h"
#include "ariadne_regions.h"
#include "ariadne_edge.h"
#include "ariadne_navitree.h"

#include "game_legacy.h"


#include "post_inc.h"



#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define navmap_tile_number(stl_x,stl_y) ((stl_y)*game.navigation_map_size_x+(stl_x))


static TbBool tri_initialised;

static NavColour *fringe_map;
static long fringe_y1;
static long fringe_y2;
static long fringe_x1;
static long fringe_x2;
static long fringe_y[MAX_SUBTILES_Y];

/******************************************************************************/


/**
 * Changes path level to 3 or 4.
 *
 * @param pt_tri
 * @param pt_cor
 * @return Returns resulted level (3 or 4), or non-positive value on error.
 */
static long make_3or4point(int32_t *pt_tri, int32_t *pt_cor)
{
    long initial_loop_result;
    long final_loop_result;
    long n;
    do
    {
        initial_loop_result = point_loop(*pt_tri, *pt_cor);
        if (initial_loop_result == 3)
          return 3;
        n = reduce_point(pt_tri, pt_cor);
        if (n == 3)
          return 3;
        if (n < 0)
          return -1;
        final_loop_result = point_loop(*pt_tri, *pt_cor);
        if (final_loop_result == n)
        {
            if ((n == 3) || (n == 4))
              break;
        }
        if ((final_loop_result != n) || (final_loop_result >= initial_loop_result))
        {
            ERRORLOG("bad state, l0:%02ld n:%02ld l1:%02ld", initial_loop_result, n, final_loop_result);
            return -1;
        }
    } while (n > 4);
    return n;
}

static long delete_quad_point(long tri1_id, long cor1_id)
{
    struct Triangle *tri1;
    tri1 = &Triangles[tri1_id];
    long del_pt_id;
    del_pt_id = tri1->points[cor1_id];
    long cor2_id;
    long cor3_id;
    long cor4_id;
    long tri2_id;
    long tri3_id;
    long tri4_id;

    tri2_id = tri1->tags[cor1_id];
    cor2_id = link_find(tri2_id, tri1_id);
    cor2_id = MOD3[cor2_id+1];
    struct Triangle *tri2;
    tri2 = &Triangles[tri2_id];

    tri3_id = tri2->tags[cor2_id];
    cor3_id = link_find(tri3_id, tri2_id);
    cor3_id = MOD3[cor3_id+1];
    struct Triangle *tri3;
    tri3 = &Triangles[tri3_id];

    tri4_id = tri3->tags[cor3_id];
    cor4_id = link_find(tri4_id, tri3_id);
    cor4_id = MOD3[cor4_id+1];
    struct Triangle *tri4;
    tri4 = &Triangles[tri4_id];

    if (tri4->tags[cor4_id] != tri1_id) {
        return false;
    }

    int nreg;
    long tri5_id;
    long tri6_id;
    long cor5_id;
    long cor6_id;

    int ptA_cor;
    int ptB_cor;
    int ptC_cor;
    int ptD_cor;
    ptA_cor = tri1->points[MOD3[cor1_id+1]];
    ptB_cor = tri3->points[MOD3[cor3_id+1]];

    int diff_ax;
    int diff_ay;
    diff_ax = ari_Points[ptA_cor].x - ari_Points[ptB_cor].x;
    diff_ay = ari_Points[ptA_cor].y - ari_Points[ptB_cor].y;
    ptC_cor = tri3->points[MOD3[cor3_id+2]];
    int diff_bx;
    int diff_by;
    diff_bx = ari_Points[ptC_cor].x - ari_Points[ptB_cor].x;
    diff_by = ari_Points[ptC_cor].y - ari_Points[ptB_cor].y;

    ptD_cor = tri1->points[MOD3[cor1_id + 2]];
    int diff_cx;
    int diff_cy;
    diff_cx = ari_Points[ptD_cor].x - ari_Points[ptB_cor].x;
    diff_cy = ari_Points[ptD_cor].y - ari_Points[ptB_cor].y;

    if ((LbCompareMultiplications(diff_ay, diff_bx, diff_ax, diff_by) >= 0) ||
        (LbCompareMultiplications(diff_ay, diff_cx, diff_ax, diff_cy) <= 0))
    {
        tri5_id = tri2->tags[MOD3[cor2_id+1]];
        cor5_id = link_find(tri5_id, tri2_id);
        struct Triangle *tri5;
        tri5 = &Triangles[tri5_id];
        tri1->points[cor1_id] = tri2->points[MOD3[cor2_id+1]];
        tri1->tags[cor1_id] = tri5_id;
        tri5->tags[cor5_id] = tri1_id;

        tri6_id = tri4->tags[MOD3[cor4_id+1]];
        cor6_id = link_find(tri6_id, tri4_id);
        struct Triangle *tri6;
        tri6 = &Triangles[tri6_id];
        tri3->points[cor3_id] = tri4->points[MOD3[cor4_id+1]];
        tri3->tags[cor3_id] = tri6_id;
        tri6->tags[cor6_id] = tri3_id;

        tri1->tags[MOD3[cor1_id+2]] = tri3_id;
        tri3->tags[MOD3[cor3_id+2]] = tri1_id;

        nreg = get_triangle_region_id(tri2_id);
        if (nreg > 0) {
            region_unset(tri2_id, nreg);
        }
        nreg = get_triangle_region_id(tri4_id);
        if (nreg > 0) {
            region_unset(tri4_id, nreg);
        }
        tri_dispose(tri2_id);
        tri_dispose(tri4_id);
        edgelen_set(tri1_id);
        edgelen_set(tri3_id);
    }
    else
    {
        tri5_id = tri3->tags[MOD3[cor3_id+1]];
        cor5_id = link_find(tri5_id, tri3_id);
        struct Triangle *tri5;
        tri5 = &Triangles[tri5_id];
        tri2->points[cor2_id] = ptB_cor;
        tri2->tags[cor2_id] = tri5_id;
        tri5->tags[cor5_id] = tri2_id;

        tri6_id = tri1->tags[MOD3[cor1_id+1]];
        cor6_id = link_find(tri6_id, tri1_id);
        struct Triangle *tri6;
        tri6 = &Triangles[tri6_id];
        tri4->points[cor4_id] = tri1->points[MOD3[cor1_id+1]];
        tri4->tags[cor4_id] = tri6_id;
        tri6->tags[cor6_id] = tri4_id;

        tri2->tags[MOD3[cor2_id+2]] = tri4_id;
        tri4->tags[MOD3[cor4_id+2]] = tri2_id;

        nreg = get_triangle_region_id(tri3_id);
        if (nreg > 0) {
            region_unset(tri3_id, nreg);
        }
        nreg = get_triangle_region_id(tri1_id);
        if (nreg > 0) {
            region_unset(tri1_id, nreg);
        }
        tri_dispose(tri3_id);
        tri_dispose(tri1_id);
        edgelen_set(tri2_id);
        edgelen_set(tri4_id);
    }
    point_dispose(del_pt_id);
    return 1;
}

static long delete_triangle_point(long tri1_id, long cor1_id)
{
    struct Triangle *tri1;
    tri1 = &Triangles[tri1_id];
    long del_pt_id;
    del_pt_id = tri1->points[cor1_id];

    int tri2_id;
    tri2_id = tri1->tags[cor1_id];
    if (tri2_id == -1) {
        return false;
    }
    int cor2_id;
    cor2_id = link_find(tri2_id, tri1_id);
    cor2_id = MOD3[cor2_id + 1];
    struct Triangle *tri2;
    tri2 = &Triangles[tri2_id];

    long tri3_id;
    tri3_id = tri2->tags[cor2_id];
    if (tri3_id == -1) {
      return false;
    }
    int cor3_id;
    cor3_id = link_find(tri3_id, tri2_id);
    cor3_id = MOD3[cor3_id+1];
    struct Triangle *tri3;
    tri3 = &Triangles[tri3_id];

    if (tri3->tags[cor3_id] != tri1_id) {
      return false;
    }
    int cor4_id;
    int cor5_id;
    cor4_id = tri2->tags[MOD3[cor2_id+1]];
    cor5_id = tri3->tags[MOD3[cor3_id+1]];
    int tri4_id;
    int tri5_id;
    tri4_id = link_find(cor4_id, tri2_id);
    tri5_id = link_find(cor5_id, tri3_id);
    tri1->points[cor1_id] = tri2->points[MOD3[cor2_id+1]];
    tri1->tags[cor1_id] = cor4_id;
    tri1->tags[MOD3[cor1_id+2]] = cor5_id;
    Triangles[cor4_id].tags[tri4_id] = tri1_id;
    Triangles[cor5_id].tags[tri5_id] = tri1_id;
    int nreg;
    nreg = get_triangle_region_id(tri2_id);
    if (nreg > 0)
    {
        region_unset(tri2_id, nreg);
    }
    nreg = get_triangle_region_id(tri3_id);
    if (nreg > 0)
    {
        region_unset(tri3_id, nreg);
    }
    tri_dispose(tri2_id);
    tri_dispose(tri3_id);
    edgelen_set(tri1_id);
    point_dispose(del_pt_id);
    return true;
}

static TbBool delete_point(long pt_tri, long pt_cor)
{
    long n;
    int32_t ntri;
    int32_t ncor;
    ntri = pt_tri;
    ncor = pt_cor;
    n = make_3or4point(&ntri, &ncor);
    if (n <= 0)
    {
        ERRORLOG("make_3or4point failure");
        return false;
    }
    if (n == 4)
    {
        if (!delete_quad_point(ntri, ncor))
        {
          ERRORLOG("variant 4 fails");
          return false;
        }
    } else
    {
        if (!delete_triangle_point(ntri, ncor))
        {
          ERRORLOG("variant 3 fails");
          return false;
        }
    }
    return true;
}


static long edge_find(long stlstart_x, long stlstart_y, long stlend_x, long stlend_y, int32_t *edge_tri, int32_t *edge_cor)
{
    //Note: uses LbCompareMultiplications()
    struct Triangle *tri;
    struct Point *pt;
    int32_t dst_tri_idx;
    int32_t dst_cor_idx;
    long tri_idx;
    long cor_idx;
    long tri_id2;
    long delta_x;
    long delta_y;
    long len_x;
    long len_y;
    long i;
    NAVIDBG(19,"Starting");
    if (!point_find(stlstart_x, stlstart_y, &dst_tri_idx, &dst_cor_idx))
    {
        return 0;
    }
    tri_idx = dst_tri_idx;
    cor_idx = dst_cor_idx;
    len_y = stlend_y - stlstart_y;
    len_x = stlend_x - stlstart_x;
    unsigned long k;
    k = 0;
    do
    {
        pt = get_triangle_point(tri_idx, MOD3[cor_idx+1]);
        delta_y = (pt->y - stlstart_y);
        delta_x = (pt->x - stlstart_x);
        if (LbCompareMultiplications(len_y, delta_x, len_x, delta_y) == 0)
        {
            if (LbNumberSignsSame(delta_x, len_x) && LbNumberSignsSame(delta_y, len_y))
            {
                *edge_tri = tri_idx;
                *edge_cor = cor_idx;
                return 1;
            }
        }
        tri = get_triangle(tri_idx);
        tri_id2 = tri->tags[cor_idx];
        if (tri_id2 == -1)
          break;
        i = link_find(tri_id2, tri_idx);
        if (i < 0) {
            ERRORLOG("no tri link");
            break;
        }
        tri_idx = tri_id2;
        cor_idx = MOD3[i+1];
        k++;
        if (k > TRIANLGLES_COUNT) {
            ERRORLOG("Infinite loop detected");
            break;
        }
    }
    while (tri_idx != dst_tri_idx);
    return 0;
}

#define edge_lock(fin_x, fin_y, bgn_x, bgn_y) edge_lock_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
static TbBool edge_lock_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name)
{
    long pt_x;
    long pt_y;
    unsigned long k;
    pt_x = ptstart_x;
    pt_y = ptstart_y;
    k = 0;
    while ((pt_x != ptend_x) || (pt_y != ptend_y))
    {
        int32_t tri_id;
        int32_t cor_id;
        if (!edge_find(pt_x, pt_y, ptend_x, ptend_y, &tri_id, &cor_id))
        {
            ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)pt_x, (int)pt_y, (int)ptend_x, (int)ptend_y);
            return false;
        }
        struct Triangle *tri;
        tri = get_triangle(tri_id);
        tri->navigation_flags |= 1 << (cor_id+3);
        struct Point *pt;
        pt = get_triangle_point(tri_id,MOD3[cor_id+1]);
        if (point_is_invalid(pt)) {
            ERRORLOG("Invalid point on edge");
            return false;
        }
        pt_x = pt->x;
        pt_y = pt->y;
        k++;
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected");
            return false;
        }
    }
    return true;
}

#define edge_unlock_record_and_regions(fin_x, fin_y, bgn_x, bgn_y) edge_unlock_record_and_regions_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
static TbBool edge_unlock_record_and_regions_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name)
{
    long pt_x;
    long pt_y;
    unsigned long k;
    long nerr;
    pt_x = ptstart_x;
    pt_y = ptstart_y;
    k = 0;
    nerr = 0;
    while (pt_x != ptend_x || pt_y != ptend_y)
    {
        int32_t tri_id;
        int32_t cor_id;
        if (!edge_find(pt_x, pt_y, ptend_x, ptend_y, &tri_id, &cor_id))
        {
            ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)pt_x, (int)pt_y, (int)ptend_x, (int)ptend_y);
            return false;
        }
        if (edge_point_add(pt_x, pt_y) < 0) {
            nerr++;
        }
        region_unlock(tri_id);
        struct Triangle *tri;
        tri = get_triangle(tri_id);
        tri->navigation_flags &= ~(1 << (cor_id+3));
        struct Point *pt;
        pt = get_triangle_point(tri_id,MOD3[cor_id+1]);
        if (point_is_invalid(pt)) {
            ERRORLOG("Invalid point on edge");
            return false;
        }
        pt_x = pt->x;
        pt_y = pt->y;
        k++;
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected");
            return false;
        }
    }
    if (nerr != 0)
    {
        ERRORMSG("%s: overflow for %ld edge points",func_name,nerr);
        //ERRORLOG("edge_setlock_record:overflow");
        return true; //TODO PATHFINDING make sure we should return true
    }
    return true;
}

static TbBool border_lock(long start_x, long start_y, long end_x, long end_y)
{
    TbBool lock_successful;
    NAVIDBG(19,"Starting");
    lock_successful = true;
    lock_successful &= edge_lock(start_x, start_y, start_x, end_y);
    lock_successful &= edge_lock(start_x, end_y, end_x, end_y);
    lock_successful &= edge_lock(end_x, end_y, end_x, start_y);
    lock_successful &= edge_lock(end_x, start_y, start_x, start_y);
    return lock_successful;
}

static void border_internal_points_delete(long start_x, long start_y, long end_x, long end_y)
{
    int32_t edge_tri;
    int32_t edge_cor;
    long ntri;
    long ncor;
    unsigned long k;
    struct Point *pt;
    long i;
    long n;
    NAVIDBG(19,"Starting");
    if (!edge_find(start_x, start_y, end_x, start_y, &edge_tri, &edge_cor))
    {
        ERRORLOG("Top not found");
        return;
    }

    ntri = Triangles[edge_tri].tags[edge_cor];
    ncor = link_find(ntri, edge_tri);
    if (ncor < 0)
    {
        ERRORLOG("Cannot find edge link");
        return;
    }
    k = 0;
    while ( 1 )
    {
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected");
            break;
        }
        region_set(ntri, 0);
        pt = get_triangle_point(ntri,MOD3[ncor+2]);
        if ((pt->x != start_x) && (pt->x != end_x)
         && (pt->y != start_y) && (pt->y != end_y))
        {
            if (!delete_point(ntri, MOD3[ncor+2]))
                break;
            ntri = Triangles[edge_tri].tags[edge_cor];
            ncor = link_find(ntri, edge_tri);
            if (ncor < 0)
            {
                ERRORLOG("Cannot re-find edge link");
                break;
            }
            k++;
            continue;
        }
        ncor = MOD3[ncor+1];
        while (!outer_locked(ntri, ncor))
        {
            if (k >= TRIANLGLES_COUNT)
            {
                // Message will be displayed in big loop, so not here
                break;
            }
            n = Triangles[ntri].tags[ncor];
            i = link_find(n, ntri);
            if (i < 0)
            {
                ERRORLOG("Cannot find triangle link");
                return;
            }
            ntri = n;
            ncor = MOD3[i+1];
            region_set(ntri, 0);
            k++;
        }
        k++;
        if (Triangles[ntri].tags[ncor] == edge_tri)
            break;
    }
}
static long fringe_scan(int32_t *outfri_x, int32_t *outfri_y, int32_t *outlen_x, int32_t *outlen_y)
{
    long loc_x;
    long sub_y;
    long sub_x;
    int dist_x;
    sub_y = fringe_y2;
    sub_x = 0;
    dist_x = 0;
    for (loc_x = fringe_x1; loc_x < fringe_x2; )
    {
        if (fringe_y[loc_x] < sub_y)
        {
          sub_y = fringe_y[loc_x];
          sub_x = loc_x;
          for (loc_x++; loc_x < fringe_x2; loc_x++)
          {
              if (sub_y != fringe_y[loc_x])
                break;
          }
          dist_x = loc_x - sub_x;
        } else
        {
          loc_x++;
        }
    }
    if (sub_y == fringe_y2) {
        return 0;
    }
    *outfri_x = sub_x;
    *outfri_y = sub_y;
    *outlen_x = dist_x;
    *outlen_y = fringe_y2 - sub_y;
    return 1;
}

static long fringe_get_rectangle(int32_t *outfri_x1, int32_t *outfri_y1, int32_t *outfri_x2, int32_t *outfri_y2, NavColour *oval)
{
    NAVIDBG(19,"Starting");
    int32_t fri_x;
    int32_t fri_y;
    int32_t len_x;
    int32_t len_y;
    len_x = 0;
    len_y = 0;
    if (!fringe_scan(&fri_x,&fri_y,&len_x,&len_y)) {
        return 0;
    }
    NavColour *fri_map;
    fri_map = &fringe_map[get_subtile_number(fri_x,fri_y)];
    // Find dx and dy
    long dx;
    long dy;
    for (dx = 1; dx < len_x; dx++)
    {
        if (fri_map[dx] != fri_map[0]) {
            break;
        }
    }
    for (dy = 1; dy < len_y; dy++)
    {
        // Our data is 0-terminated, so we can use string functions to compare
        if (memcmp(&fri_map[(game.map_subtiles_x + 1) * dy], &fri_map[0], dx*sizeof(NavColour)) != 0) {
            break;
        }
    }
    long i;
    for (i = 0; i < dx; i++) {
        fringe_y[fri_x+i] = fri_y+dy;
    }
    *oval = fri_map[0];
    *outfri_x1 = fri_x;
    *outfri_y1 = fri_y;
    *outfri_x2 = fri_x + dx;
    *outfri_y2 = fri_y + dy;
    return 1;
}

static TbBool point_redundant(long tri_idx, long cor_idx)
{
    long tri_first;
    long cor_first;
    long tri_secnd;
    long cor_secnd;
    tri_first = tri_idx;
    cor_first = cor_idx;
    unsigned long k;
    k = 0;
    while ( 1 )
    {
        tri_secnd = Triangles[tri_first].tags[cor_first];
        if ((tri_secnd < 0) || (tri_secnd >= TRIANLGLES_COUNT))
            break;
        if (get_triangle_tree_alt(tri_secnd) != get_triangle_tree_alt(tri_idx))
          break;
        cor_secnd = link_find(tri_secnd, tri_first);
        if (cor_secnd < 0)
            break;
        cor_first = MOD3[cor_secnd+1];
        tri_first = tri_secnd;
        if (tri_secnd == tri_idx)
        {
            return true;
        }
        k++;
        if (k >= TRIANLGLES_COUNT) {
            ERRORDBG(9,"Infinite loop detected");
            break;
        }
    }
    return false;
}

static long tri_split3(long btri_id, long pt_x, long pt_y)
{
    NAVIDBG(19,"Starting");
    struct Triangle *btri;
    struct Triangle *tri1;
    struct Triangle *tri2;
    long new_triangle1_id;
    long new_triangle2_id;
    new_triangle1_id = tri_new();
    if (new_triangle1_id < 0) {
        return -1;
    }
    new_triangle2_id = tri_new();
    if (new_triangle2_id < 0) {
        tri_dispose(new_triangle1_id);
        return -1;
    }
    btri = &Triangles[btri_id];
    tri1 = &Triangles[new_triangle1_id];
    tri2 = &Triangles[new_triangle2_id];
    memcpy(tri1,btri,sizeof(struct Triangle));
    memcpy(tri2,btri,sizeof(struct Triangle));
    long pt_id;
    pt_id = point_set_new_or_reuse(pt_x, pt_y);
    if (pt_id < 0) {
        tri_dispose(new_triangle1_id);
        tri_dispose(new_triangle2_id);
        return -1;
    }
    //point_set(pt_id, pt_x, pt_y); // from before point_set_new_or_reuse()
    btri->points[2] = pt_id;
    tri1->points[0] = pt_id;
    tri2->points[1] = pt_id;
    btri->tags[1] = new_triangle1_id;
    btri->tags[2] = new_triangle2_id;
    btri->navigation_flags |= 0x06;
    btri->navigation_flags &= 0x0F;
    tri1->tags[0] = btri_id;
    tri1->tags[2] = new_triangle2_id;
    tri1->navigation_flags |= 0x05;
    tri1->navigation_flags &= 0x17;
    tri2->tags[0] = btri_id;
    tri2->tags[1] = new_triangle1_id;
    tri2->navigation_flags |= 0x03;
    tri2->navigation_flags &= 0x27;

    long ttri_id;
    long ltri_id;
    ttri_id = tri1->tags[1];
    if (ttri_id != -1)
    {
        ltri_id = link_find(ttri_id, btri_id);
        if (ltri_id >= 0) {
            Triangles[ttri_id].tags[ltri_id] = new_triangle1_id;
        } else {
            ERRORLOG("A not found");
        }
    }
    ttri_id = tri2->tags[2];
    if (ttri_id != -1)
    {
        ltri_id = link_find(ttri_id, btri_id);
        if (ltri_id >= 0) {
            Triangles[ttri_id].tags[ltri_id] = new_triangle2_id;
        } else {
            ERRORLOG("B not found");
        }
    }
    long reg_id;
    reg_id = get_triangle_region_id(btri_id);
    if (reg_id > 0) {
        region_unset(btri_id, reg_id);
    }
    tri1->region_and_edgelen = 0;
    tri2->region_and_edgelen = 0;
    edgelen_set(btri_id);
    edgelen_set(new_triangle1_id);
    edgelen_set(new_triangle2_id);
    return pt_id;
}

static long tri_split2(long tri_id1, long cor_id1, long pt_x, long pt_y, long pt_id1)
{
    long tri_id2;
    tri_id2 = tri_new();
    if (tri_id2 < 0) {
        return -1;
    }
    struct Triangle *tri1;
    struct Triangle *tri2;
    tri1 = &Triangles[tri_id1];
    tri2 = &Triangles[tri_id2];
    memcpy(tri2, tri1, sizeof(struct Triangle));
    long cor_id2;
    long reg_id1;
    cor_id2 = MOD3[cor_id1 + 1];
    tri1->points[cor_id2] = pt_id1;
    tri2->points[cor_id1] = pt_id1;
    tri1->tags[cor_id2] = tri_id2;
    tri1->navigation_flags |= (1 << cor_id2);
    tri1->navigation_flags &= ~(1 << (cor_id2 + 3));
    long tri_id3;
    tri_id3 = MOD3[cor_id1 + 2];
    tri2->tags[tri_id3] = tri_id1;
    tri2->navigation_flags |= (1 << tri_id3);
    tri2->navigation_flags &= ~(1 << (tri_id3 + 3));
    long tri_id4;
    tri_id3 = tri2->tags[cor_id2];
    if (tri_id3 != -1)
    {
        tri_id4 = link_find(tri_id3, tri_id1);
        if (tri_id4 == -1) {
            ERRORLOG("A not found");
        }
        Triangles[tri_id3].tags[tri_id4] = tri_id2;
    }
    reg_id1 = get_triangle_region_id(tri_id1);
    if (reg_id1 > 0) {
        region_unset(tri_id1, reg_id1);
    }
    tri2->region_and_edgelen = 0;
    edgelen_set(tri_id1);
    edgelen_set(tri_id2);
    return tri_id2;
}

static long edge_split(long ntri, long ncor, long pt_x, long pt_y)
{
    long pt_idx;
    long ntr2;
    long ncr2;
    long tri_sp1;
    long tri_sp2;
    NAVIDBG(19,"Starting");
    // Create and fill new point
    pt_idx = point_set_new_or_reuse(pt_x, pt_y);
    if (pt_idx < 0) {
        return -1;
    }
    //point_set(pt_idx, pt_x, pt_y); // from before point_set_new_or_reuse()
    // Find second ntri and ncor
    ntr2 = Triangles[ntri].tags[ncor];
    ncr2 = link_find(ntr2, ntri);
    if (ncr2 < 0) {
        point_dispose(pt_idx);
        return -1;
    }
    // Do the splitting
    tri_sp1 = tri_split2(ntri, ncor, pt_x, pt_y, pt_idx);
    tri_sp2 = tri_split2(ntr2, ncr2, pt_x, pt_y, pt_idx);
    Triangles[ntr2].tags[ncr2] = tri_sp1;
    Triangles[ntri].tags[ncor] = tri_sp2;
    return pt_idx;
}

/** Returns if given coords can divide triangle into same areas.
 *
 * @param ntri Triangle index.
 * @param ncorA First tip of the edge to be divided.
 * @param ncorB Second tip of the edge to be divided.
 * @param pt_x Coord X of the dividing point.
 * @param pt_y Coord Y of the dividing point.
 * @return Zero if areas do not differ; -1 or 1 otherwise.
 */
static char triangle_divide_areas_differ(long ntri, long ncorA, long ncorB, long pt_x, long pt_y)
{
    long tipA_x;
    long tipA_y;
    long tipB_x;
    long tipB_y;
    struct Point *pt;

    pt = get_triangle_point(ntri,ncorA);
    tipA_x = pt->x;
    tipA_y = pt->y;
    pt = get_triangle_point(ntri,ncorB);
    tipB_x = pt->x;
    tipB_y = pt->y;
    return LbCompareMultiplications(pt_y-tipA_y, tipB_x-tipA_x, pt_x-tipA_x, tipB_y-tipA_y);
}

/*
 * There are mesh of all triangles on a map
 * This function inserts another point into mesh by splitting triangles into parts
 */
static TbBool insert_point(long pt_x, long pt_y)
{
    long ntri;
    NAVIDBG(19,"Starting for (%d,%d)", (int)pt_x, (int)pt_y);
    ntri = triangle_find8(pt_x << 8, pt_y << 8);
    if ((ntri < 0) || (ntri >= TRIANLGLES_COUNT))
    {
        ERRORLOG("triangle not found");
        return false;
    }
    if (triangle_tip_equals(ntri, 0, pt_x, pt_y))
        return true;
    if (triangle_tip_equals(ntri, 1, pt_x, pt_y))
        return true;
    if (triangle_tip_equals(ntri, 2, pt_x, pt_y))
        return true;

    if (triangle_divide_areas_differ(ntri, 0, 1, pt_x, pt_y) == 0)
    {
        return edge_split(ntri, 0, pt_x, pt_y) >= 0;
    }
    if (triangle_divide_areas_differ(ntri, 1, 2, pt_x, pt_y) == 0)
    {
        return edge_split(ntri, 1, pt_x, pt_y) >= 0;
    }
    if (triangle_divide_areas_differ(ntri, 2, 0, pt_x, pt_y) == 0)
    {
        return edge_split(ntri, 2, pt_x, pt_y) >= 0;
    }
    return tri_split3(ntri, pt_x, pt_y) >= 0;
}

static long fill_concave(long tri_beg_id, long tag_id, long tri_end_id)
{
    long tri_id;
    long cor_id;
    while ( 1 )
    {
      tri_id = Triangles[tri_beg_id].tags[tag_id];
      if (tri_id == -1) {
        return 0;
      }
      cor_id = link_find(tri_id, tri_beg_id);
      cor_id = MOD3[cor_id+1];
      int rotate_n;
      int rotate_y;
      rotate_y = 0;
      rotate_n = 0;
      while (Triangles[tri_id].tags[cor_id] != tri_end_id)
      {
          if (edge_rotateAC(tri_id, cor_id))
          {
              rotate_y++;
          } else
          {
              long n;
              n = Triangles[tri_id].tags[cor_id];
              if ((n == -1) || (n == tri_beg_id)) {
                  return 0;
              }
              cor_id = link_find(n, tri_id);
              cor_id = MOD3[cor_id+1];
              tri_id = n;
              rotate_n++;
          }
      }
      if ( !rotate_n || !rotate_y )
        break;
    }
    return 1;
}




static void make_edge_sub(long start_tri_id1, long start_cor_id1, long start_tri_id4, long start_cor_id4, long sx, long sy, long ex, long ey)
{
    struct Triangle *tri;
    struct Point *pt;
    struct Point *pt1;
    struct Point *pt2;
    struct Point *pt3;
    long tri_id1;
    long cor_id1;
    long tri_id2;
    long cor_id2;
    long tri_id3;
    long cor_id3;
    long tri_id4;
    long cor_id4;
    long i;
    long cx;
    long cy;
    unsigned long k;
    cor_id1 = start_cor_id1;
    tri_id1 = start_tri_id1;
    cor_id4 = start_cor_id4;
    tri_id4 = start_tri_id4;
    k = 0;
    do
    {
        tri = get_triangle(tri_id1);
        tri_id2 = tri->tags[cor_id1];
        i = link_find(tri_id2,tri_id1);
        cor_id2 = MOD3[i+2];
        tri_id1 = tri->tags[cor_id1];
        cor_id1 = MOD3[i+1];
        pt = get_triangle_point(tri_id2, cor_id2);
        cx = pt->x;
        cy = pt->y;
        if (LbCompareMultiplications(cy - ey, sx - ex, cx - ex, sy - ey) < 0) {
            continue;
        }
        tri = get_triangle(tri_id2);
        tri_id3 = tri->tags[cor_id1];
        cor_id3 = link_find(tri_id3,tri_id2);
        pt1 = get_triangle_point(tri_id3, cor_id3);
        pt2 = get_triangle_point(tri_id4, MOD3[cor_id4+1]);
        pt3 = get_triangle_point(tri_id4, cor_id4);
        //if (triangle_divide_areas_differ(tri_id4, MOD3[cor_id4+1], cor_id4, pt1->x, pt1->y) < 0)
        if (LbCompareMultiplications(pt1->y - pt2->y, pt3->x - pt2->x,
                                     pt1->x - pt2->x, pt3->y - pt2->y) < 0)
        {
            fill_concave(tri_id4, cor_id4, tri_id3);
            break;
        }
        tri_id4 = tri_id3;
        cor_id4 = cor_id3;
        cor_id1 = cor_id2;
        k++;
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected");
            break;
        }
    } while ((cx != sx) || (cy != sy));
}

static TbBool make_edge(long start_x, long start_y, long end_x, long end_y)
{
    struct Triangle *tri;
    struct Point *pt;
    long sx;
    long ex;
    long sy;
    long ey;
    int32_t tri_id1;
    int32_t cor_id1;
    int32_t tri_id2;
    int32_t cor_id2;
    int32_t tri_id3;
    int32_t cor_id3;
    long tmpX;
    long tmpY;
    long pt_cor;
    unsigned long k;
    NAVIDBG(19,"Starting");
    k = 0;
    sx = start_x;
    sy = start_y;
    ex = end_x;
    ey = end_y;
    while ((ex != sx) || (ey != sy))
    {
        // Find triangles to which start point and end point belongs
        if (!point_find(ex, ey, &tri_id1, &cor_id1))
            break;
        if (!point_find(sx, sy, &tri_id2, &cor_id2))
            break;
        pt_cor = pointed_at8(sx << 8, sy << 8, &tri_id1, &cor_id1);
        if (pt_cor == -1)
        {
            ERRORLOG("border point not found, pointed at %ld,%ld",sx,sy);
            return false;
        }
        pt = get_triangle_point(tri_id1, pt_cor);
        if ((pt->x == sx) && (pt->y == sy))
            break;
        tri = get_triangle(tri_id1);
        tri_id3 = tri->tags[cor_id1];
        cor_id3 = link_find(tri_id3,tri_id1);
        pt = get_triangle_point(tri_id3, cor_id3);
        tmpX = pt->x;
        tmpY = pt->y;
        SYNCDBG(18,"Triangle %d point %d is (%d,%d)",(int)tri_id3,(int)tri_id1,(int)tmpX,(int)tmpY);
        if (LbCompareMultiplications(tmpY-ey, sx-ex, tmpX-ex, sy-ey) == 0)
        {
            if (!make_edge(ex, ey, tmpX, tmpY)) {
                return false;
            }
            ex = tmpX;
            ey = tmpY;
            continue;
        }
        make_edge_sub(tri_id1, pt_cor, tri_id3, cor_id3, sx, sy, ex, ey);
        tmpX = ex;
        tmpY = ey;
        ex = sx;
        ey = sy;
        sx = tmpX;
        sy = tmpY;
        k++;
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected at area (%d,%d) to (%d,%d)",(int)sx,(int)sy,(int)ex,(int)ey);
            return false;
        }
    }
    return true;
}

static TbBool border_clip_horizontal(const NavColour *imap, long start_x, long end_x, long start_y, long end_y)
{
    NavColour map_center;
    NavColour map_up;
    const NavColour* mapp_center;
    const NavColour* mapp_up;
    TbBool clipping_successful;
    long i;
    clipping_successful = true;
    NAVIDBG(19,"Starting from (%ld,%ld) to (%ld,%ld)",start_x, start_y, end_x, end_y);
    i = start_x;
    {
        mapp_center = &imap[navmap_tile_number(i,start_y)];
        mapp_up = &imap[navmap_tile_number(i,start_y-1)];
        {
            clipping_successful &= insert_point(i, start_y);
            map_up = *mapp_up;
            map_center = *mapp_center;
        }
    }
    for (i++; i < end_x; i++)
    {
        mapp_center = &imap[navmap_tile_number(i,start_y)];
        mapp_up = &imap[navmap_tile_number(i,start_y-1)];
        if ((*mapp_center != map_center) || (*mapp_up != map_up))
        {
            clipping_successful &= insert_point(i, start_y);
            map_up = *mapp_up;
            map_center = *mapp_center;
        }
    }
    clipping_successful &= insert_point(end_x, start_y);
    if (!clipping_successful) {
        ERRORLOG("Couldn't insert points to make border");
        //TODO PATHFINDING on a failure, we could release all allocated points...
        return clipping_successful;
    }
    clipping_successful &= make_edge(start_x, start_y, end_x, start_y);
    if (!clipping_successful) {
        ERRORLOG("Couldn't make edge for border");
    }
    return clipping_successful;
}

static TbBool border_clip_vertical(const NavColour *imap, long start_x, long end_x, long start_y, long end_y)
{
    NavColour map_center;
    NavColour map_left;
    const NavColour* mapp_center;
    const NavColour* mapp_left;
    TbBool clipping_successful;
    long i;
    clipping_successful = true;
    NAVIDBG(19,"Starting from (%ld,%ld) to (%ld,%ld)",start_x, start_y, end_x, end_y);
    i = start_y;
    {
        mapp_center = &imap[navmap_tile_number(start_x,i)];
        mapp_left = &imap[navmap_tile_number(start_x-1,i)];
        {
            clipping_successful &= insert_point(start_x, i);
            map_left = *mapp_left;
            map_center = *mapp_center;
        }
    }
    for (i++; i < end_y; i++)
    {
        mapp_center = &imap[navmap_tile_number(start_x,i)];
        mapp_left = &imap[navmap_tile_number(start_x-1,i)];
        if ((*mapp_center != map_center) || (*mapp_left != map_left))
        {
            clipping_successful &= insert_point(start_x, i);
            map_left = *mapp_left;
            map_center = *mapp_center;
        }
    }
    clipping_successful &= insert_point(start_x, end_y);
    if (!clipping_successful) {
        ERRORLOG("Couldn't insert points to make border");
        //TODO PATHFINDING on a failure, we could release all allocated points...
        return clipping_successful;
    }
    clipping_successful &= make_edge(start_x, start_y, start_x, end_y);
    if (!clipping_successful) {
        ERRORLOG("Couldn't make edge for border");
    }
    return clipping_successful;
}

static long triangle_area1(long tri_idx)
{
    int ptidx0;
    int ptidx1;
    int ptidx2;
    ptidx0 = Triangles[tri_idx].points[0];
    ptidx1 = Triangles[tri_idx].points[1];
    ptidx2 = Triangles[tri_idx].points[2];
    long long area1;
    long long area2;
    area1 = (ari_Points[ptidx2].x - (int)ari_Points[ptidx0].x) * (ari_Points[ptidx0].y - (int)ari_Points[ptidx1].y);
    area2 = (ari_Points[ptidx1].x - (int)ari_Points[ptidx0].x) * (ari_Points[ptidx2].y - (int)ari_Points[ptidx0].y);
    return llabs(area1+area2);
}

static void brute_fill_rectangle(long start_x, long start_y, long end_x, long end_y, NavColour ntree_alt)
{
    // Replace start and end if they are switched
    if (end_x < start_x)
    {
        long i;
        i = end_x;
        end_x = start_x;
        start_x = i;
    }
    if (end_y < start_y)
    {
        long i;
        i = end_y;
        end_y = start_y;
        start_y = i;
    }
    long long area;
    area = 0;
    long tri_idx;
    for (tri_idx = ix_Triangles - 1; tri_idx >= 0; tri_idx--)
    {
        struct Triangle *tri;
        tri = &Triangles[tri_idx];
        int ptidx;
        long x;
        long y;
        ptidx = tri->points[0];
        x = ari_Points[ptidx].x;
        y = ari_Points[ptidx].y;
        if ((x >= start_x) && (x <= end_x) && (y >= start_y) && (y <= end_y))
        {
            ptidx = tri->points[1];
            x = ari_Points[ptidx].x;
            y = ari_Points[ptidx].y;
            if ((x >= start_x) && (x <= end_x) && (y >= start_y) && (y <= end_y))
            {
                ptidx = tri->points[2];
                x = ari_Points[ptidx].x;
                y = ari_Points[ptidx].y;
                if ((x >= start_x) && (x <= end_x) && (y >= start_y) && (y <= end_y))
                {
                    tri->tree_alt = ntree_alt;
                    area += triangle_area1(tri_idx);
                    if (2 * (end_x - start_x) * (end_y - start_y) == area) {
                        break;
                    }
                }
            }
        }
    }
}

#define fill_rectangle(start_x, start_y, end_x, end_y, nav_colour) fill_rectangle_f(start_x, start_y, end_x, end_y, nav_colour, __func__)
static void fill_rectangle_f(long start_x, long start_y, long end_x, long end_y, NavColour nav_colour, const char *func_name)
{
    int32_t tri_n0;
    int32_t tri_k0;
    int32_t tri_n1;
    int32_t tri_k1;
    int32_t tri_n2;
    int32_t tri_k2;
    int32_t tri_n3;
    int32_t tri_k3;
    long tri_area;
    long req_area;
    req_area = 2 * (end_x - start_x) * (end_y - start_y);
    if (!edge_find(start_x, start_y, start_x, end_y, &tri_n0, &tri_k0))
    {
        ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)start_x, (int)start_y, (int)start_x, (int)end_y);
        return;
    }
    Triangles[tri_n0].tree_alt = nav_colour;
    tri_area = triangle_area1(tri_n0);
    if (tri_area == req_area) {
        return;
    }
    if (!edge_find(end_x, end_y, end_x, start_y, &tri_n1, &tri_k1))
    {
        ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)end_x, (int)end_y, (int)end_x, (int)start_y);
        return;
    }
    if (tri_n1 != tri_n0)
    {
        Triangles[tri_n1].tree_alt = nav_colour;
        tri_area += triangle_area1(tri_n1);
    }
    if (tri_area == req_area) {
        return;
    }
    if (!edge_find(end_x, start_y, start_x, start_y, &tri_n2, &tri_k2))
    {
        ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)end_x, (int)start_y, (int)start_x, (int)start_y);
        return;
    }
    if ((tri_n2 != tri_n0) && (tri_n2 != tri_n1))
    {
        Triangles[tri_n2].tree_alt = nav_colour;
        tri_area += triangle_area1(tri_n2);
    }
    if (tri_area == req_area) {
        return;
    }
    if (!edge_find(start_x, end_y, end_x, end_y, &tri_n3, &tri_k3))
    {
        ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)start_x, (int)end_y, (int)end_x, (int)end_y);
        return;
    }
    if ((tri_n3 != tri_n0) && (tri_n1 != tri_n3) && (tri_n2 != tri_n3))
    {
        Triangles[tri_n3].tree_alt = nav_colour;
        tri_area += triangle_area1(tri_n3);
    }
    if (tri_area == req_area) {
        return;
    }
    brute_fill_rectangle(start_x, start_y, end_x, end_y, nav_colour);
}

static TbBool tri_set_rectangle(long start_x, long start_y, long end_x, long end_y, NavColour nav_colour)
{
    long sx;
    long sy;
    long ex;
    long ey;
    NAVIDBG(19,"Starting");
    sx = start_x;
    ex = end_x;
    if (sx > ex) {
      sx = end_x;
      ex = start_x;
    }
    sy = start_y;
    ey = end_y;
    if (sy > ey) {
      sy = end_y;
      ey = start_y;
    }
    TbBool rectangle_creation_successful;
    rectangle_creation_successful = true;
    rectangle_creation_successful &= insert_point(sx, sy);
    rectangle_creation_successful &= insert_point(sx, ey);
    rectangle_creation_successful &= insert_point(ex, ey);
    rectangle_creation_successful &= insert_point(ex, sy);
    if (!rectangle_creation_successful) {
        ERRORLOG("Couldn't insert points to make rectangle; start (%d,%d), end (%d,%d)",(int)start_x,(int)start_y,(int)end_x,(int)end_y);
        return rectangle_creation_successful;
    }
    rectangle_creation_successful &= make_edge(sx, sy, sx, ey);
    rectangle_creation_successful &= make_edge(sx, ey, ex, ey);
    rectangle_creation_successful &= make_edge(ex, ey, ex, sy);
    rectangle_creation_successful &= make_edge(ex, sy, sx, sy);
    if (!rectangle_creation_successful) {
        ERRORLOG("Couldn't make edge for rectangle; start (%d,%d), end (%d,%d)",(int)start_x,(int)start_y,(int)end_x,(int)end_y);
        return rectangle_creation_successful;
    }
    fill_rectangle(sx, sy, ex, ey, nav_colour);
    return rectangle_creation_successful;
}

static void triangulation_initxy(long startx, long starty, long endx, long endy)
{
    long i;
    for (i=0; i < TRIANLGLES_COUNT; i++)
    {
        struct Triangle *tri;
        tri = &Triangles[i];
        tri->tree_alt = NAV_COL_UNSET;
    }
    tri_initialised = 1;
    triangulation_initxy_points(startx, starty, endx, endy);
    triangulation_init_triangles(0, 1, 2, 3);
    edgelen_set(0);
    edgelen_set(1);
    triangulation_init_cache(triangle_find_first_used());
    triangulation_init_regions();
}

static void triangulation_init(void)
{
    if (!tri_initialised)
    {
        tri_initialised = 1;
        triangulation_initxy(-256, -256, 512, 512);
    }
}

static TbBool triangulation_border_start(int32_t *border_a, int32_t *border_b)
{
    struct Triangle *tri;
    long tri_idx;
    long brd_idx;
    long i;
    long k;
    // First try - border
    for (brd_idx=0; brd_idx < ix_Border; brd_idx++)
    {
        tri_idx = Border[brd_idx];
        if (get_triangle_tree_alt(tri_idx) != NAV_COL_UNSET)
        {
            tri = get_triangle(tri_idx);
            for (i=0; i < 3; i++)
            {
                k = tri->tags[i];
                if (k == -1)
                {
                    *border_a = tri_idx;
                    *border_b = i;
                    return true;
                }
            }
        }
    }
    // Second try - triangles
    for (tri_idx=0; tri_idx < ix_Triangles; tri_idx++)
    {
        if (get_triangle_tree_alt(tri_idx) != NAV_COL_UNSET)
        {
            tri = get_triangle(tri_idx);
            for (i=0; i < 3; i++)
            {
                k = tri->tags[i];
                if (k == -1)
                {
                    *border_a = tri_idx;
                    *border_b = i;
                    return true;
                }
            }
        }
    }
    // Failure
    ERRORLOG("not found");
    *border_a = 0;
    *border_b = 0;
    return false;
}

static void triangulation_border_init(void)
{
    int32_t border_a;
    int32_t border_b;
    long tri_a;
    long tri_b;
    long i;
    long n;
    NAVIDBG(9,"Starting");
    triangulation_border_start(&border_a, &border_b);
    tri_a = border_a;
    tri_b = border_b;
    ix_Border = 0;
    do
    {
      Border[ix_Border] = tri_a;
      ix_Border++;
      if (ix_Border >= BORDER_LENGTH)
      {
        ERRORLOG("Border overflow");
        break;
      }
      tri_b = MOD3[tri_b+1];
      while ( 1 )
      {
          i = Triangles[tri_a].tags[tri_b];
          n = link_find(i, tri_a);
          if (n < 0) {
              break;
          }
          tri_b = MOD3[n+1];
          tri_a = i;
      }
    }
    while ((tri_a != border_a) || (tri_b != border_b));
    NAVIDBG(19,"Finished");
}

static NavColour uniform_area_colour(const NavColour *imap, long start_x, long start_y, long end_x, long end_y)
{
    NavColour uniform;
    long x;
    long y;
    uniform = imap[navmap_tile_number(start_x,start_y)];
    for (y = start_y; y < end_y; y++)
    {
        for (x = start_x; x < end_x; x++)
        {
            if (imap[navmap_tile_number(x,y)] != uniform)
            {
                return NAV_COL_UNSET;
            }
        }
    }
    return uniform;
}


static void border_unlock(long start_x, long start_y, long end_x, long end_y)
{
    struct EdgePoint *ept;
    long ept_id;
    int32_t tri_idx;
    int32_t cor_idx;
    long nerr;
    edge_points_clean();
    edge_unlock_record_and_regions(start_x, start_y, start_x, end_y);
    edge_unlock_record_and_regions(start_x, end_y, end_x, end_y);
    edge_unlock_record_and_regions(end_x, end_y, end_x, start_y);
    edge_unlock_record_and_regions(end_x, start_y, start_x, start_y);
    if (ix_EdgePoints < 1)
        return;
    nerr = 0;
    for (ept_id = ix_EdgePoints; ept_id > 0; ept_id--)
    {
        ept = edge_point_get(ept_id-1);
        if (!point_find(ept->pt_x, ept->pt_y, &tri_idx, &cor_idx))
        {
            nerr++;
            continue;
        }
        if (point_redundant(tri_idx, cor_idx))
        {
            delete_point(tri_idx, cor_idx);
        }
    }
    if (nerr != 0)
    {
        ERRORLOG("Out of %ld edge points, %ld were not found",(long)ix_EdgePoints,nerr);
    }
}

static TbBool triangulate_area(NavColour *imap, long start_x, long start_y, long end_x, long end_y)
{
    TbBool one_tile;
    TbBool not_whole_map;
    NavColour colour;
    NavColour ccolour;
    int32_t rect_sx;
    int32_t rect_sy;
    int32_t rect_ex;
    int32_t rect_ey;
    TbBool triangulation_successful;
    long i;
    triangulation_successful = true;
    LastTriangulatedMap = imap;
    NAVIDBG(9,"Area from (%03ld,%03ld) to (%03ld,%03ld) with %04ld triangles",start_x,start_y,end_x,end_y,count_Triangles);
    // Switch coords to make end_x larger than start_x
    if (end_x < start_x)
    {
        i = start_x;
        start_x = end_x;
        end_x = i;
    }
    if (end_y < start_y)
    {
        i = start_y;
        start_y = end_y;
        end_y = i;
    }
    if ((start_x == -1) || (start_y == -1) || (end_x == start_x) || (end_y == start_y))
    {
        NAVIDBG(9,"Invalid area bounds");
        return false;
    }
    // Prepare some basic logic information
    one_tile = (((end_x - start_x) == 1) && ((end_y - start_y) == 1));
    not_whole_map = (start_x != 0) || (start_y != 0) || (end_x != game.map_subtiles_x + 1) || (end_y != game.map_subtiles_y + 1);
    // If coordinates are out of range, update the whole map area
    if ((start_x < 1) || (start_y < 1) || (end_x >= game.map_subtiles_x) || (end_y >= game.map_subtiles_y))
    {
        one_tile = 0;
        not_whole_map = 0;
        start_x = 0;
        end_x = game.map_subtiles_x + 1;
        start_y = 0;
        end_y = game.map_subtiles_y + 1;
    }
    triangulation_init();
    if ( not_whole_map )
    {
        triangulation_successful &= border_clip_horizontal(imap, start_x, end_x, start_y, 0);
        triangulation_successful &= border_clip_horizontal(imap, start_x, end_x, end_y, -1);
        triangulation_successful &= border_clip_vertical(imap, start_x, -1, start_y, end_y);
        triangulation_successful &= border_clip_vertical(imap, end_x, 0, start_y, end_y);
        triangulation_successful &= border_lock(start_x, start_y, end_x, end_y);
        if ( !one_tile ) {
            border_internal_points_delete(start_x, start_y, end_x, end_y);
        }
    } else
    {
        triangulation_initxy(-(game.map_subtiles_x + 1), -(game.map_subtiles_y + 1), (game.map_subtiles_x + 1) * 2, (game.map_subtiles_y + 1) * 2);
        tri_set_rectangle(start_x, start_y, end_x, end_y, 0);
    }
    colour = -1;
    if ( one_tile )
    {
        colour = imap[navmap_tile_number(start_x,start_y)];
    } else
    if ((not_whole_map) && (end_x - start_x <= 3) && (end_y - start_y <= 3))
    {
        colour = uniform_area_colour(imap, start_x, start_y, end_x, end_y);
    }

    if (colour == NAV_COL_UNSET)
    {
        fringe_map = imap;
        fringe_x1 = start_x;
        fringe_y1 = start_y;
        fringe_x2 = end_x;
        fringe_y2 = end_y;
        for (i = start_x; i < end_x; i++)
        {
            fringe_y[i] = start_y;
        }
        while ( fringe_get_rectangle(&rect_sx, &rect_sy, &rect_ex, &rect_ey, &ccolour) )
        {
            if ((ccolour) || (not_whole_map))
            {
                if (!tri_set_rectangle(rect_sx, rect_sy, rect_ex, rect_ey, ccolour))
                    break; // Run out of triangle space
                delaunay_seeded(rect_sx, rect_sy, rect_ex, rect_ey, true);
            }
        }
    } else
    {
        tri_set_rectangle(start_x, start_y, end_x, end_y, colour);
    }
    delaunay_seeded(start_x, start_y, end_x, end_y, false);
    if ( not_whole_map )
        border_unlock(start_x, start_y, end_x, end_y);
    triangulation_border_init();
    NAVIDBG(9,"Done");
    return triangulation_successful;
}

static void set_navigation_map(MapSubtlCoord stl_x, MapSubtlCoord stl_y, NavColour navcolour)
{
  if ((stl_x < 0) || (stl_x > game.map_subtiles_x))
      return;
  if ((stl_y < 0) || (stl_y > game.map_subtiles_y))
      return;
  game.navigation_map[navmap_tile_number(stl_x,stl_y)] = navcolour;
}

static NavColour get_navigation_colour_for_door(long stl_x, long stl_y)
{
    struct Thing *doortng;
    NavColour colour = (1 << NAVMAP_FLOORHEIGHT_BIT);

    doortng = get_door_for_position(stl_x, stl_y);
    if (thing_is_invalid(doortng))
    {
        ERRORLOG("Cannot find door for flagged position (%d,%d)",(int)stl_x,(int)stl_y);
        return colour;
    }

    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if ((players_are_mutual_allies(plyr_idx,doortng->owner) && doortng->door.is_locked) ||
            door_is_hidden_to_player(doortng,plyr_idx))
        {
            colour |= 1 << (NAVMAP_OWNERSELECT_BIT + plyr_idx);
        }
    }
    return colour;

}

static NavColour get_navigation_colour_for_cube(long stl_x, long stl_y)
{
    NavColour i;
    i = get_floor_filled_subtiles_at(stl_x, stl_y);
    if (i > NAVMAP_FLOORHEIGHT_MAX)
      i = NAVMAP_FLOORHEIGHT_MAX;
    if (subtile_is_unsafe(stl_x, stl_y))
      i |= NAVMAP_UNSAFE_SURFACE;
    return i;
}

static NavColour get_navigation_colour(long stl_x, long stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    if ((mapblk->flags & SlbAtFlg_IsDoor) != 0)
    {
        return get_navigation_colour_for_door(stl_x, stl_y);
    }
    if ((mapblk->flags & SlbAtFlg_Blocking) != 0)
    {
        return (NAVMAP_FLOORHEIGHT_MAX << NAVMAP_FLOORHEIGHT_BIT);
    }
    return get_navigation_colour_for_cube(stl_x, stl_y);
}

void init_navigation_map(void)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    memset(game.navigation_map, 0, sizeof(NavColour)*game.navigation_map_size_x*game.navigation_map_size_y);
    for (stl_y=0; stl_y < game.navigation_map_size_y; stl_y++)
    {
        for (stl_x=0; stl_x < game.navigation_map_size_x; stl_x++)
        {
            set_navigation_map(stl_x, stl_y, get_navigation_colour(stl_x, stl_y));
        }
    }
    nav_map_initialised = 1;
}

static void triangulate_map(NavColour *imap)
{
    triangulate_area(imap, 0, 0, game.navigation_map_size_x, game.navigation_map_size_y);
}

long init_navigation(void)
{
    
    NavColour *IanMap = (NavColour *)&game.navigation_map;
    init_navigation_map();
    triangulate_map(IanMap);
    set_nav_rule_default();
    
    game.map_changed_for_navigation = 1;
    return 1;
}

long update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y)
{
    long sx;
    long sy;
    long ex;
    long ey;
    long x;
    long y;
    if (!nav_map_initialised)
        init_navigation_map();
    // Prepare parameter bounds
    if (end_x < start_x)
      start_x = end_x;
    if (end_y < start_y)
      start_y = end_y;
    sx = start_x - 1;
    if (sx <= 2)
      sx = 2;
    sy = start_y - 1;
    if (sy <= 2)
      sy = 2;
    ex = end_x + 1;
    if (ex >= game.map_subtiles_x-2)
      ex = game.map_subtiles_x-2;
    ey = end_y + 1;
    if (ey >= game.map_subtiles_y-2)
      ey = game.map_subtiles_y-2;
    // Fill a rectangle with nav colors (based on columns and blocks)
    for (y = sy; y <= ey; y++)
    {
        for (x = sx; x <= ex; x++)
        {
            set_navigation_map(x, y, get_navigation_colour(x, y));
        }
    }
    triangulate_area(game.navigation_map, sx, sy, ex, ey);
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
