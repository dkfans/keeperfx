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
#include "pre_inc.h"
#include "ariadne_navitree.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"

#include "ariadne.h"
#include "ariadne_tringls.h"
#include "ariadne_points.h"
#include "ariadne_findcache.h"
#include "ariadne_naviheap.h"
#include "gui_topmsg.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned char Tags[TREEITEMS_COUNT];
int32_t tree_dad[TREEITEMS_COUNT];
unsigned char tag_current = 0;
int32_t ix_delaunay = 0;
int32_t delaunay_stack[DELAUNAY_COUNT];
int32_t tree_val[TREEVALS_COUNT];

/******************************************************************************/

/** Copies the current tree into given route.
 *
 * @param tag_start_id Starting tag ID to place in the route.
 * @param tag_end_id Ending tag ID to place in the route.
 * @param route_pts Route array.
 * @param route_len Length of the given route array.
 * @return Returns index of the last point filled.
 *     If route_len is too small, points up to route_len are filled and -1 is returned.
 */
int32_t copy_tree_to_route(int32_t tag_start_id, int32_t tag_end_id, int32_t *route_pts, int32_t route_len)
{
    int32_t itag = tag_start_id;
    int32_t ipt = 0;
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

void tags_init(void)
{
    //Note that tag_current is a tag value, not tag index
    if (tag_current >= 255)
    {
        memset(Tags, 0, sizeof(Tags));
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
int32_t update_border_tags(int32_t tag_id, int32_t *border_pt, int32_t border_len)
{
    int32_t iset = 0;
    for (int32_t ipt = 0; ipt < border_len; ipt++)
    {
        int32_t n = border_pt[ipt];
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

int32_t border_tags_to_current(int32_t *border_pt, int32_t border_len)
{
    return update_border_tags(tag_current, border_pt, border_len);
}

TbBool is_current_tag(int32_t tag_id)
{
    return (tag_current == Tags[tag_id]);
}

void store_current_tag(int32_t tag_id)
{
    Tags[tag_id] = tag_current;
}

TbBool navitree_add(int32_t itm_pos, int32_t itm_dat, int32_t mvcost)
{
    int32_t tag_pos = tag_current;
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

TbBool delaunay_add(int32_t itm_pos)
{
    if (ix_delaunay >= DELAUNAY_COUNT) {
        return false;
    }
    delaunay_stack[ix_delaunay] = itm_pos;
    ix_delaunay++;
    Tags[itm_pos] = tag_current;
    return true;
}

TbBool delaunay_add_triangle(int32_t tri_idx)
{
    NavColour i = get_triangle_tree_alt(tri_idx);
    if (i != NAV_COL_UNSET)
    {
        if (!is_current_tag(tri_idx))
        {
            if ((i & NAVMAP_FLOORHEIGHT_MASK) != NAVMAP_FLOORHEIGHT_MAX)
            {
                return delaunay_add(tri_idx);
            }
        }
    }
    return false;
}

static void delaunay_stack_point(int32_t pt_x, int32_t pt_y)
{
    int32_t cor_idx;
    int32_t dst_tri_idx;
    int32_t dst_cor_idx;
    int32_t adjacent_triangle_index;
    NAVIDBG(19,"Starting");

    int32_t found_triangle_index = triangle_find8(pt_x << 8, pt_y << 8);
    if (found_triangle_index == -1) {
        NAVIDBG(19,"Tri not found");
        return;
    }
    delaunay_add_triangle(found_triangle_index);
    for (cor_idx=0; cor_idx < 3; cor_idx++)
    {
        adjacent_triangle_index = Triangles[found_triangle_index].tags[cor_idx];
        if (adjacent_triangle_index != -1) {
            delaunay_add_triangle(adjacent_triangle_index);
        }
    }
    if (point_find(pt_x, pt_y, &dst_tri_idx, &dst_cor_idx))
    {
      found_triangle_index = dst_tri_idx;
      cor_idx = dst_cor_idx;
      uint32_t k = 0;
      do
      {
          adjacent_triangle_index = Triangles[found_triangle_index].tags[cor_idx];
          if (adjacent_triangle_index == -1) {
              NAVIDBG(19,"Tag not found");
              break;
          }
          int32_t i = link_find(adjacent_triangle_index, found_triangle_index);
          if (i == -1) {
              NAVIDBG(19,"Link not found");
              break;
          }
          cor_idx = MOD3[i+1];
          found_triangle_index = adjacent_triangle_index;
          delaunay_add_triangle(found_triangle_index);
          k++;
          if (k >= TRIANLGLES_COUNT) {
              ERRORDBG(9,"Infinite loop detected");
              break;
          }
      }
      while (found_triangle_index != dst_tri_idx);
    }
    NAVIDBG(19,"Done");
}

int32_t optimise_heuristic(int32_t tri_id1, int32_t cor_id1)
{
    struct Triangle* tri1 = get_triangle(tri_id1);
    int32_t tri_id3 = tri1->tags[cor_id1];
    if (tri_id3 == -1)
        return 0;
    struct Triangle* tri3 = get_triangle(tri_id3);
    if (get_triangle_tree_alt(tri_id3) != get_triangle_tree_alt(tri_id1))
    {
        return 0;
    }
    int32_t tri_lnk = link_find(tri_id3, tri_id1);
    if (( (tri1->navigation_flags & (1 << cor_id1)) == 0)
     || ( (tri3->navigation_flags & (1 << tri_lnk)) == 0))
    {
        return 0;
    }
    struct Point* pt = get_triangle_point(tri_id3, MOD3[tri_lnk + 2]);
    int32_t Ax = pt->x;
    int32_t Ay = pt->y;
    pt = get_triangle_point(tri_id1, MOD3[cor_id1+2]);
    int32_t Bx = pt->x;
    int32_t By = pt->y;
    pt = get_triangle_point(tri_id1, MOD3[cor_id1+1]);
    int32_t Cx = pt->x;
    int32_t Cy = pt->y;
    pt = get_triangle_point(tri_id1, MOD3[cor_id1]);
    int32_t Dx = pt->x;
    int32_t Dy = pt->y;
    if (LbCompareMultiplications(Ay-By, Dx-Bx, Ax-Bx, Dy-By) >= 0)
        return 0;
    if (LbCompareMultiplications(Ay-By, Cx-Bx, Ax-Bx, Cy-By) <= 0)
        return 0;

    return ((Bx-Ax) * (Bx-Ax)) + ((By-Ay) * (By-Ay)) <
           ((Dy-Cy) * (Dy-Cy)) + ((Dx-Cx) * (Dx-Cx));
}

int32_t delaunay_seeded(int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y, TbBool keep_edge)
{
    NAVIDBG(19,"Starting");
    tags_init();
    delaunay_init();
    delaunay_stack_point(start_x, start_y);
    delaunay_stack_point(start_x, end_y);
    delaunay_stack_point(end_x, start_y);
    delaunay_stack_point(end_x, end_y);
    int32_t count = 0;
    while (ix_delaunay > 0)
    {
        ix_delaunay--;
        int32_t tri_idx = delaunay_stack[ix_delaunay];
        for (int32_t cor_idx = 0; cor_idx < 3; cor_idx++)
        {
            if (!optimise_heuristic(tri_idx, cor_idx))
                continue;
            count++;

            TbBool need_rotate = true;
            if (keep_edge) {
                struct Point* pt1 = get_triangle_point(tri_idx, cor_idx);
                struct Point* pt2 = get_triangle_point(tri_idx, MOD3[cor_idx+1]);
                if (pt1->y == pt2->y && (pt1->y == start_y || pt1->y == end_y)) {
                    if ((pt1->x >= start_x && pt1->x <= end_x) && (pt2->x >= start_x && pt2->x <= end_x)) {
                        need_rotate = false; // On the horizontal edge
                    }
                }
                else if (pt1->x == pt2->x && (pt1->x == start_x || pt1->x == end_x)) {
                    if ((pt1->y >= start_y && pt1->y <= end_y) && (pt2->y >= start_y && pt2->y <= end_y)) {
                        need_rotate = false; // On the vertical edge
                    }
                }
            }
            if (need_rotate) {
                edge_rotateAC(tri_idx, cor_idx);
            }

            if (ix_delaunay+4 >= DELAUNAY_COUNT)
            {
              ERRORLOG("stack full");
              return count;
            }
            for (int32_t cor_id2 = 0; cor_id2 < 3; cor_id2++)
            {
                int32_t local_adjacent_triangle = Triangles[tri_idx].tags[cor_id2];
                if (local_adjacent_triangle == -1)
                    continue;
                delaunay_add_triangle(local_adjacent_triangle);
            }
        }
    }
    return count;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
