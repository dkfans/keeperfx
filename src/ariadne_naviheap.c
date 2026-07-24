/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_naviheap.c
 *     Navigation heap for Ariadne system support functions.
 * @par Purpose:
 *     Functions to manage navigation heap.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2011 - 14 Jun 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "ariadne_naviheap.h"

#include "globals.h"
#include "bflib_basics.h"
#include "ariadne_tringls.h"
#include "ariadne_navitree.h"
#include "gui_topmsg.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static int32_t heap_end;
static int32_t Heap[PATH_HEAP_LEN];

static int32_t naviheap_item_tree_val(int32_t heapid);
/******************************************************************************/
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
int32_t naviheap_top(void)
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
static int32_t naviheap_get(int32_t heapid)
{
    if ((heapid < 0) || (heapid > heap_end+1))
        return -1;
    return Heap[heapid];
}

/** Moves heap elements down, removing element of given index.
 *
 * @param heapid
 */
void heap_down(int32_t heapid)
{
    // Insert dummy value (there is no associated triangle for it)
    Heap[heap_end+1] = TREEVALS_COUNT-1;
    tree_val[TREEVALS_COUNT-1] = INT32_MAX;
    uint32_t hend = (heap_end >> 1);
    int32_t tree_idb = Heap[heapid];
    int32_t tval_idb = tree_val[tree_idb];
    uint32_t hpos = heapid;
    while (hpos <= hend)
    {
        uint32_t hnew = (hpos << 1);
        /* Select the cone with smaller tree value */
        if (naviheap_item_tree_val(hnew+1) < naviheap_item_tree_val(hnew))
            hnew++;
        int32_t tree_ids = Heap[hnew];
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
int32_t naviheap_remove(void)
{
  if (heap_end < 1)
  {
      erstat_inc(ESE_BadPathHeap);
      return -1;
  }
  int32_t popval = Heap[1];
  Heap[1] = Heap[heap_end];
  heap_end--;
  heap_down(1);
  return popval;
}

#define heap_up(heapid) heap_up_f(heapid, __func__)
void heap_up_f(int32_t heapid, const char *func_name)
{
    uint32_t pmask = heapid;
    Heap[0] = TREEVALS_COUNT-1;
    tree_val[TREEVALS_COUNT-1] = -1;
    uint32_t nmask = pmask;
    int32_t k = Heap[pmask];
    while ( 1 )
    {
        nmask >>= 1;
        int32_t i = Heap[nmask];
        if (tree_val[k] > tree_val[i])
          break;
        if (pmask == 0)
        {
            erstat_inc(ESE_BadPathHeap);
            ERRORDBG(8,"%s: sabotaged navigate heap, heapid=%d",func_name,(int)heapid);
            break;
        }
        Heap[pmask] = i;
        pmask = nmask;
    }
    Heap[pmask] = k;
}

TbBool naviheap_add(int32_t heapid)
{
    // Always leave one unused element (not sure why, but originally 2 were left)
    // The element is needed because we sometimes fill Heap[heap_end+1] and this must work
    if (heap_end >= PATH_HEAP_LEN-1)
    {
        return false;
    }
    heap_end++;
    Heap[heap_end] = heapid;
    heap_up(heap_end);
    return true;
}

/**
 * Returns tree item value for given heap position.
 * @param heapid
 * @return
 */
static int32_t naviheap_item_tree_val(int32_t heapid)
{
    int32_t tree_id = naviheap_get(heapid);
    if ((tree_id < 0) || (tree_id >= TREEVALS_COUNT))
    {
        erstat_inc(ESE_BadPathHeap);
        return -1;
    }
    return tree_val[tree_id];
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
