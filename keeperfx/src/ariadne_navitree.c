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
#include "bflib_math.h"

#include "ariadne_tringls.h"
#include "ariadne_points.h"
#include "ariadne_findcache.h"
#include "gui_topmsg.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_heap_end;
#define heap_end _DK_heap_end
DLLIMPORT long _DK_Heap[PATH_HEAP_LEN];
#define Heap _DK_Heap
DLLIMPORT long _DK_tree_val[TREEVALS_COUNT];
#define tree_val _DK_tree_val
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
/******************************************************************************/
DLLIMPORT void _DK_heap_down(long heapid);
/******************************************************************************/
void nodes_classify(void)
{
}

/** Initializes navigation heap for new use.
 */
void naviheap_init(void)
{
    heap_end = 0;
}

/** Checks if the navigation heap is empty.
 *
 * @return
 */
TbBool naviheap_empty(void)
{
    return (heap_end == 0);
}

/** Retrieves top element of the navigation heap.
 *
 * @return
 */
long naviheap_top(void)
{
    if (heap_end < 1)
        return -1;
    return Heap[1];
}

/** Retrieves given element of the navigation heap.
 *
 * @param heapid
 * @return
 */
long naviheap_get(long heapid)
{
    if ((heapid < 0) || (heapid > heap_end+1))
        return -1;
    return Heap[heapid];
}

long naviheap_item_tree_val(long heapid)
{
    long tree_id;
    if ((heapid < 0) || (heapid >= PATH_HEAP_LEN))
    {
        erstat_inc(ESE_BadPathHeap);
        return -1;
    }
    tree_id = Heap[heapid];
    if ((tree_id < 0) || (tree_id >= TREEVALS_COUNT))
    {
        erstat_inc(ESE_BadPathHeap);
        return -1;
    }
    return tree_val[tree_id];
}

/** Moves heap elements down, removing element of given index.
 *
 * @param heapid
 */
void heap_down(long heapid)
{
    unsigned long hpos,hnew,hend;
    long tree_idb,tree_ids;
    long tval_idb;
    //_DK_heap_down(heapid); return;
    Heap[heap_end+1] = TREEVALS_COUNT-1;
    tree_val[TREEVALS_COUNT-1] = LONG_MAX;
    hend = (heap_end >> 1);
    tree_idb = Heap[heapid];
    tval_idb = tree_val[tree_idb];
    hpos = heapid;
    while (hpos <= hend)
    {
        hnew = (hpos << 1);
        if (naviheap_item_tree_val(hnew) < naviheap_item_tree_val(hnew+1))
            hnew++;
        tree_ids = Heap[hnew];
        if (tree_val[tree_ids] > tval_idb)
          break;
        Heap[hpos] = tree_ids;
        hpos = hnew;
    }
    Heap[hpos] = tree_idb;
}

/** Removes one element from the heap and returns it.
 *
 * @return The removed element value.
 */
long naviheap_remove(void)
{
  long popval;
  if (heap_end < 1)
  {
      erstat_inc(ESE_BadPathHeap);
      return -1;
  }
  popval = Heap[1];
  Heap[1] = Heap[heap_end];
  heap_end--;
  heap_down(1);
  return popval;
}

void heap_up(long heapid)
{
    unsigned long nmask,pmask;
    long i,k;
    pmask = heapid;
    Heap[0] = TREEVALS_COUNT-1;
    tree_val[TREEVALS_COUNT-1] = -1;
    nmask = pmask;
    k = Heap[pmask];
    while ( 1 )
    {
        nmask >>= 1;
        i = Heap[nmask];
        if (tree_val[k] > tree_val[i])
          break;
        if (pmask == 0)
        {
            erstat_inc(ESE_BadPathHeap);
            ERRORDBG(8,"sabotaged navigate heap");
            break;
        }
        Heap[pmask] = i;
        pmask = nmask;
    }
    Heap[pmask] = k;
}

TbBool naviheap_add(long heapid)
{
    // Always leave one unused element (not sure why, but originally 2 were left)
    if (heap_end >= PATH_HEAP_LEN-1)
    {
        return false;
    }
    heap_end++;
    Heap[heap_end] = heapid;
    heap_up(heap_end);
    return true;
}

void tree_init(void)
{
  long i;
  for (i=0; i < TREEVALS_COUNT; i++)
  {
      tree_val[i] = -2147483647;
  }
}

/** Copies the current tree into given route.
 *
 * @param tag_start_id Starting tag ID to place in the route.
 * @param tag_end_id Ending tag ID to place in the route.
 * @param route_pts Route array.
 * @param route_len Length of the given route array.
 * @return Returns amount of points filled.
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

TbBool navitree_add(long itm_pos, long itm_dat, long mvcost)
{
    long tag_pos;
    tag_pos = tag_current;
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
    delaunay_stack[ix_delaunay] = itm_pos;
    ix_delaunay++;
    Tags[itm_pos] = tag_current;
    return true;
}

TbBool delaunay_add_triangle(long tri_idx)
{
    long i;
    i = get_triangle_field_C(tri_idx);
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

void delaunay_stack_point(long a1, long a2)
{
    long tri_idx,cor_idx;
    long dst_tri_idx,dst_cor_idx;
    long tri_id2, i;

    tri_idx = triangle_find8(a1 << 8, a2 << 8);
    if (tri_idx == -1)
        return;
    delaunay_add_triangle(tri_idx);
    for (cor_idx=0; cor_idx < 3; cor_idx++)
    {
        tri_id2 = Triangles[tri_idx].field_6[cor_idx];
        if (tri_id2 != -1)
        {
            delaunay_add_triangle(tri_id2);
        }
    }
    if (point_find(a1, a2, &dst_tri_idx, &dst_cor_idx))
    {
      tri_idx = dst_tri_idx;
      cor_idx = dst_cor_idx;
      do
      {
        tri_id2 = Triangles[tri_idx].field_6[cor_idx];
        if (tri_id2 == -1)
          break;
        i = link_find(tri_id2, tri_idx);
        cor_idx = MOD3[i+1];
        tri_idx = tri_id2;
        if (tri_idx != -1)
        {
            delaunay_add_triangle(tri_idx);
        }
      }
      while (tri_idx != dst_tri_idx);
    }
}

long optimise_heuristic(long tri_id1, long tri_id2)
{
    struct Triangle *tri1;
    struct Triangle *tri3;
    struct Point *pt;
    long tri_id3,tri_lnk;
    long Ax,Ay,Bx,By,Cx,Cy,Dx,Dy;

    tri1 = get_triangle(tri_id1);
    tri_id3 = tri1->field_6[tri_id2];
    if (tri_id3 == -1)
        return 0;
    tri3 = get_triangle(tri_id3);
    if (tri3->field_C != tri1->field_C)
    {
        return 0;
    }
    tri_lnk = link_find(tri_id3, tri_id1);
    if ((tri1->field_D & (1 << tri_id2) == 0)
     || (tri3->field_D & (1 << tri_lnk) == 0))
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

    if (LbCompareMultiplications(Ay-By, Dx-Bx, Ay-By, Cx-Bx) >= 0)
        return 0;

    return ((Bx-Ax) * (Bx-Ax)) + ((By-Ay) * (By-Ay)) <
           ((Dy-Ay) - (Cy-Ay)) * ((Dy-Ay) - (Cy-Ay)) +
           ((Dx-Ax) - (Cx-Ax)) * ((Dx-Ax) - (Cx-Ax));
}

long delaunay_seeded(long a1, long a2, long a3, long a4)
{
    long tri_idx,cor_idx;
    long tri_id2,cor_id2;
    long count;
    //return _DK_delaunay_seeded(a1, a2, a3, a4);
    tags_init();
    delaunay_init();
    delaunay_stack_point(a1, a2);
    delaunay_stack_point(a1, a4);
    delaunay_stack_point(a3, a2);
    delaunay_stack_point(a3, a4);
    count = 0;
    while (ix_delaunay > 0)
    {
        tri_idx = delaunay_stack[ix_delaunay-1];
        ix_delaunay--;
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
                tri_id2 = Triangles[tri_idx].field_6[cor_id2];
                if (tri_id2 == -1)
                    continue;
                delaunay_add_triangle(tri_id2);
            }
        }
    }
    return count;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
