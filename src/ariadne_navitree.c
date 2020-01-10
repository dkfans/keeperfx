/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_navitree.c
 *     Navigation Tree support functions.
 * @par Purpose:
 *     Functions to maintain Navigation Tree for Ariadne Pathfinding.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Jun 2010 - 16 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "ariadne_navitree.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"

#include "ariadne_tringls.h"
#include "ariadne_points.h"
#include "ariadne_findcache.h"
#include "ariadne_naviheap.h"
#include "gui_topmsg.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#if USE_ORIGINAL_TRIANGLES_DATA
DLLIMPORT unsigned char _DK_Tags[TREEITEMS_COUNT];
#define Tags _DK_Tags
DLLIMPORT long _DK_tree_dad[TREEITEMS_COUNT];
#define tree_dad _DK_tree_dad
DLLIMPORT unsigned char _DK_tag_current;
#define tag_current _DK_tag_current
DLLIMPORT long _DK_ix_delaunay;
#define ix_delaunay _DK_ix_delaunay
DLLIMPORT long _DK_delaunay_stack[DELAUNAY_COUNT];
#define delaunay_stack _DK_delaunay_stack
#else
unsigned char Tags[TREEITEMS_COUNT];
long tree_dad[TREEITEMS_COUNT];
unsigned char tag_current = 0;
long ix_delaunay = 0;
long delaunay_stack[DELAUNAY_COUNT];
long tree_val[TREEVALS_COUNT];
#endif

/******************************************************************************/
/******************************************************************************/
void nodes_classify(void)
{
}

void tree_init(void)
{
    long i;
    for (i=0; i < TREEVALS_COUNT; i++)
    {
        tree_val[i] = -LONG_MAX;
    }
}

/** Computes the cost for route through the current tree.
 *
 * @param tag_start_id Starting tag ID to place in the route.
 * @param tag_end_id Ending tag ID to place in the route.
 * @return Returns cost of the route.
 */
long compute_tree_move_cost(long tag_start_id, long tag_end_id)
{
    long ipt,itag;
    long long rcost;
    rcost = 0;
    itag = tag_start_id;
    ipt = 0;
    while (itag != tag_end_id)
    {
        rcost += tree_val[itag];
        ipt++;
        if (ipt >= TREEITEMS_COUNT)
            return LONG_MAX;
        itag = tree_dad[itag];
    }
    if (rcost >= LONG_MAX)
        return LONG_MAX;
    if (rcost <= LONG_MIN)
        return LONG_MIN;
    return rcost;
}

/** Copies the current tree into given route.
 *
 * @param tag_start_id Starting tag ID to place in the route.
 * @param tag_end_id Ending tag ID to place in the route.
 * @param route_pts Route array.
 * @param route_len Length of the given route array.
 * @return Returns index of the last point filled.
 *     If route_len is too small, points up to route_len are filled and -1 is returned.
 */
long copy_tree_to_route(long tag_start_id, long tag_end_id, long *route_pts, long route_len)
{
    long ipt,itag;
    itag = tag_start_id;
    ipt = 0;
    while (itag != tag_end_id)
    {
        route_pts[ipt] = itag;
        ipt++;
        if (ipt >= route_len)
        {
            return -1;
        }
        itag = tree_dad[itag];
    }
    route_pts[ipt] = tag_end_id;
    return ipt;
}

long tree_to_route(long tag_start_id, long tag_end_id, long *route_pts)
{
    long ipt;
    if (tag_current != Tags[tag_start_id])
        return -1;
    ipt = copy_tree_to_route(tag_start_id, tag_end_id, route_pts, 3000+1);
    if (ipt < 0)
    {
        erstat_inc(ESE_BadRouteTree);
        ERRORDBG(6,"route length overflow");
    }
    return ipt;

}

void tags_init(void)
{
    //Note that tag_current is a tag value, not tag index
    if (tag_current >= 255)
    {
        LbMemorySet(Tags, 0, sizeof(Tags));
        tag_current = 0;
    }
    tag_current++;
}

/** Sets tags if indices from given border to given tag_id.
 *
 * @param tag_id
 * @param border_pt
 * @param border_len
 * @return
 */
long update_border_tags(long tag_id, long *border_pt, long border_len)
{
    long ipt,n;
    long iset;
    iset = 0;
    for (ipt=0; ipt < border_len; ipt++)
    {
        n = border_pt[ipt];
        if ((n < 0) || (n >= TREEITEMS_COUNT))
        {
            erstat_inc(ESE_BadRouteTree);
            continue;
        }
        Tags[n] = tag_id;
        iset++;
    }
    tag_current = tag_id;
    return iset;
}

long border_tags_to_current(long *border_pt, long border_len)
{
    return update_border_tags(tag_current, border_pt, border_len);
}

TbBool is_current_tag(long tag_id)
{
    return (tag_current == Tags[tag_id]);
}

void store_current_tag(long tag_id)
{
    Tags[tag_id] = tag_current;
}

TbBool navitree_add(long itm_pos, long itm_dat, long mvcost)
{
    long tag_pos;
    tag_pos = tag_current;
    if (itm_pos >= TRIANLGLES_COUNT) {
        WARNLOG("Inserting outranged pos %d",(int)itm_pos);
    }
    if (itm_dat >= TREEITEMS_COUNT) {
        WARNLOG("Inserting outranged dat %d",(int)itm_dat);
    }
    tree_val[itm_pos] = mvcost;
    Tags[itm_pos] = tag_pos;
    tree_dad[itm_pos] = itm_dat;
    return naviheap_add(itm_pos);
}

void delaunay_init(void)
{
    ix_delaunay = 0;
}

TbBool delaunay_add(long itm_pos)
{
    if (ix_delaunay >= DELAUNAY_COUNT) {
        return false;
    }
    delaunay_stack[ix_delaunay] = itm_pos;
    ix_delaunay++;
    Tags[itm_pos] = tag_current;
    return true;
}

TbBool delaunay_add_triangle(long tri_idx)
{
    long i;
    i = get_triangle_tree_alt(tri_idx);
    if (i != -1)
    {
        if (!is_current_tag(tri_idx))
        {
            if ((i & 0x0F) != 15)
            {
                return delaunay_add(tri_idx);
            }
        }
    }
    return false;
}

static void delaunay_stack_point(long pt_x, long pt_y)
{
    long tri_idx,cor_idx;
    long dst_tri_idx,dst_cor_idx;
    long tri_id2, i;
    NAVIDBG(19,"Starting");
    //_DK_delaunay_stack_point(pt_x, pt_y); return;

    tri_idx = triangle_find8(pt_x << 8, pt_y << 8);
    if (tri_idx == -1) {
        NAVIDBG(19,"Tri not found");
        return;
    }
    delaunay_add_triangle(tri_idx);
    for (cor_idx=0; cor_idx < 3; cor_idx++)
    {
        tri_id2 = Triangles[tri_idx].tags[cor_idx];
        if (tri_id2 != -1) {
            delaunay_add_triangle(tri_id2);
        }
    }
    if (point_find(pt_x, pt_y, &dst_tri_idx, &dst_cor_idx))
    {
      tri_idx = dst_tri_idx;
      cor_idx = dst_cor_idx;
      unsigned long k;
      k = 0;
      do
      {
          tri_id2 = Triangles[tri_idx].tags[cor_idx];
          if (tri_id2 == -1) {
              NAVIDBG(19,"Tag not found");
              break;
          }
          i = link_find(tri_id2, tri_idx);
          if (i == -1) {
              NAVIDBG(19,"Link not found");
              break;
          }
          cor_idx = MOD3[i+1];
          tri_idx = tri_id2;
          delaunay_add_triangle(tri_idx);
          k++;
          if (k >= TRIANLGLES_COUNT) {
              ERRORDBG(9,"Infinite loop detected");
              break;
          }
      }
      while (tri_idx != dst_tri_idx);
    }
    NAVIDBG(19,"Done");
}
HOOK_DK_FUNC(delaunay_stack_point)

long optimise_heuristic(long tri_id1, long tri_id2)
{
    //return _DK_optimise_heuristic(tri_id1, tri_id2);
    struct Triangle *tri1;
    struct Triangle *tri3;
    struct Point *pt;
    long tri_id3,tri_lnk;
    long Ax,Ay,Bx,By,Cx,Cy,Dx,Dy;

    tri1 = get_triangle(tri_id1);
    tri_id3 = tri1->tags[tri_id2];
    if (tri_id3 == -1)
        return 0;
    tri3 = get_triangle(tri_id3);
    if (get_triangle_tree_alt(tri_id3) != get_triangle_tree_alt(tri_id1))
    {
        return 0;
    }
    tri_lnk = link_find(tri_id3, tri_id1);
    if (( (tri1->field_D & (1 << tri_id2)) == 0)
     || ( (tri3->field_D & (1 << tri_lnk)) == 0))
    {
        return 0;
    }
    pt = get_triangle_point(tri_id3, MOD3[tri_lnk+2]);
    Ax = pt->x;
    Ay = pt->y;
    pt = get_triangle_point(tri_id1, MOD3[tri_id2+2]);
    Bx = pt->x;
    By = pt->y;
    pt = get_triangle_point(tri_id1, MOD3[tri_id2+1]);
    Cx = pt->x;
    Cy = pt->y;
    pt = get_triangle_point(tri_id1, MOD3[tri_id2]);
    Dx = pt->x;
    Dy = pt->y;
    if (LbCompareMultiplications(Ay-By, Dx-Bx, Ax-Bx, Dy-By) >= 0)
        return 0;
    if (LbCompareMultiplications(Ay-By, Cx-Bx, Ax-Bx, Cy-By) <= 0)
        return 0;

    return ((Bx-Ax) * (Bx-Ax)) + ((By-Ay) * (By-Ay)) <
           ((Dy-Ay) - (Cy-Ay)) * ((Dy-Ay) - (Cy-Ay)) +
           ((Dx-Ax) - (Cx-Ax)) * ((Dx-Ax) - (Cx-Ax));
}

long delaunay_seeded(long start_x, long start_y, long end_x, long end_y)
{
    long tri_idx,cor_idx;
    long tri_id2,cor_id2;
    long count;
    NAVIDBG(19,"Starting");
    //return _DK_delaunay_seeded(start_x, start_y, end_x, end_y);
    tags_init();
    delaunay_init();
    delaunay_stack_point(start_x, start_y);
    delaunay_stack_point(start_x, end_y);
    delaunay_stack_point(end_x, start_y);
    delaunay_stack_point(end_x, end_y);
    count = 0;
    while (ix_delaunay > 0)
    {
        ix_delaunay--;
        tri_idx = delaunay_stack[ix_delaunay];
        for (cor_idx=0; cor_idx < 3; cor_idx++)
        {
            if (!optimise_heuristic(tri_idx, cor_idx))
                continue;
            count++;
            edge_rotateAC(tri_idx, cor_idx);
            if (ix_delaunay+4 >= DELAUNAY_COUNT)
            {
              ERRORLOG("stack full");
              return count;
            }
            for (cor_id2=0; cor_id2 < 3; cor_id2++)
            {
                tri_id2 = Triangles[tri_idx].tags[cor_id2];
                if (tri_id2 == -1)
                    continue;
                delaunay_add_triangle(tri_id2);
            }
        }
    }
    return count;
}
HOOK_DK_FUNC(delaunay_seeded)

/******************************************************************************/
#ifdef __cplusplus
}
#endif
