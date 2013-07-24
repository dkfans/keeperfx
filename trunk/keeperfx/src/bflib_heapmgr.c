/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_heapmgr.c
 *     Allocating and maintaining heap memory.
 * @par Purpose:
 *     Provides interface for heap memory allocation routines.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Aug 2009 - 02 Sep 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_heapmgr.h"

#include "bflib_basics.h"
#include "bflib_memory.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_heapmgr_free_oldest(struct HeapMgrHeader *hmhead);
DLLIMPORT struct HeapMgrHandle *_DK_heapmgr_add_item(struct HeapMgrHeader *hmhead, long idx);
DLLIMPORT void _DK_heapmgr_make_newest(struct HeapMgrHeader *hmhead, struct HeapMgrHandle *hmhandle);
DLLIMPORT struct HeapMgrHeader *_DK_heapmgr_init(unsigned char *a1, long a2, long a3);
DLLIMPORT void _DK_heapmgr_complete_defrag(struct HeapMgrHeader *hmhead);
/******************************************************************************/
struct HeapMgrHandle *find_free_handle(struct HeapMgrHeader *hmhead)
{
    struct HeapMgrHandle *hmh;
    if (hmhead->field_10 >= hmhead->field_C)
        return NULL;
    hmh = (struct HeapMgrHandle *)&hmhead[1];
    while (hmh->field_8)
        hmh++;
    return hmh;
}

long heapmgr_free_oldest(struct HeapMgrHeader *hmhead)
{
    return _DK_heapmgr_free_oldest(hmhead);
}

/**
 * Changes the heap manager linked list so that given element appears as the newest one.
 * @param hmhead
 * @param hmhandle
 */
void heapmgr_make_newest(struct HeapMgrHeader *hmhead, struct HeapMgrHandle *hmhandle)
{
    struct HeapMgrHandle *hnext;
    struct HeapMgrHandle *hprev;
    struct HeapMgrHandle *hsecond;
    //_DK_heapmgr_make_newest(hmhead, hmhandle);
    hnext = hmhandle->next_hndl;
    if (hnext != NULL)
    {
        hprev = hmhandle->prev_hndl;
        hnext->prev_hndl = hprev;
        if (hprev != NULL)
        {
            hprev->next_hndl = hmhandle->next_hndl;
        } else
        {
            hmhead->last_hndl = hmhandle->next_hndl;
        }
        hsecond = hmhead->first_hndl;
        hsecond->next_hndl = hmhandle;
        hmhandle->next_hndl = NULL;
        hmhandle->prev_hndl = hsecond;
        hmhead->first_hndl = hmhandle;
    }
}

struct HeapMgrHandle *heapmgr_add_item(struct HeapMgrHeader *hmhead, long idx)
{
    return _DK_heapmgr_add_item(hmhead, idx);
}

struct HeapMgrHeader *heapmgr_init(unsigned char *buf_end, long buf_remain, long nsamples)
{
    //return _DK_heapmgr_init(buf_end, buf_remain, nsamples);
    struct HeapMgrHeader *hmgr;
    long samples_len;
    samples_len = 28 * nsamples + 36;
    if (buf_remain <= samples_len)
        return NULL;
    hmgr = (struct HeapMgrHeader *)buf_end;
    hmgr->field_0 = (buf_end + samples_len);
    hmgr->field_4 = (buf_end + buf_remain);
    hmgr->field_8 = buf_remain - samples_len;
    hmgr->field_C = nsamples;
    hmgr->field_10 = 0;
    hmgr->field_14 = 0;
    hmgr->field_18 = 0;
    hmgr->last_hndl = NULL;
    hmgr->first_hndl = NULL;
    return hmgr;
}

void heapmgr_complete_defrag(struct HeapMgrHeader *hmhead)
{
    _DK_heapmgr_complete_defrag(hmhead); return;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
