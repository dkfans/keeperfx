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
#include "ariadne_naviheap.h"

#include "globals.h"
#include "bflib_basics.h"
#include "ariadne_tringls.h"
#include "ariadne_navitree.h"
#include "gui_topmsg.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_heap_end;
#define heap_end _DK_heap_end
DLLIMPORT long _DK_Heap[PATH_HEAP_LEN];
#define Heap _DK_Heap
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

/** Moves heap elements down, removing element of given index.
 *
 * @param heapid
 */
void heap_down(long heapid)
{
    // Insert dummy value (there is no associated triangle for it)
    Heap[heap_end+1] = TREEVALS_COUNT-1;
    tree_val[TREEVALS_COUNT-1] = LONG_MAX;
    unsigned long hend = (heap_end >> 1);
    long tree_idb = Heap[heapid];
    long tval_idb = tree_val[tree_idb];
    unsigned long hpos = heapid;
    while (hpos <= hend)
    {
        unsigned long hnew = (hpos << 1);
        /* Select the cone with smaller tree value */
        if (naviheap_item_tree_val(hnew+1) < naviheap_item_tree_val(hnew))
            hnew++;
        long tree_ids = Heap[hnew];
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
  if (heap_end < 1)
  {
      erstat_inc(ESE_BadPathHeap);
      return -1;
  }
  long popval = Heap[1];
  Heap[1] = Heap[heap_end];
  heap_end--;
  heap_down(1);
  return popval;
}

#define heap_up(heapid) heap_up_f(heapid, __func__)
void heap_up_f(long heapid, const char *func_name)
{
    unsigned long pmask = heapid;
    Heap[0] = TREEVALS_COUNT-1;
    tree_val[TREEVALS_COUNT-1] = -1;
    unsigned long nmask = pmask;
    long k = Heap[pmask];
    while ( 1 )
    {
        nmask >>= 1;
        long i = Heap[nmask];
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

TbBool naviheap_add(long heapid)
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
long naviheap_item_tree_val(long heapid)
{
    long tree_id = naviheap_get(heapid);
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
