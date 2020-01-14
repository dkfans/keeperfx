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
DLLIMPORT long _DK_heapmgr_free_handle(struct HeapMgrHeader *hmhead, struct HeapMgrHandle *hmhandle);
/******************************************************************************/
struct HeapMgrHandle *find_free_handle(struct HeapMgrHeader *hmhead)
{
    if (hmhead->field_10 >= hmhead->handles_count)
        return NULL;
    struct HeapMgrHandle* hmh = (struct HeapMgrHandle*)&hmhead[1];
    while (hmh->flags)
        hmh++;
    return hmh;
}

long heapmgr_free_handle(struct HeapMgrHeader *hmhead, struct HeapMgrHandle *hmhandle)
{
    return _DK_heapmgr_free_handle(hmhead, hmhandle);
}

long heapmgr_free_oldest(struct HeapMgrHeader *hmhead)
{
    //return _DK_heapmgr_free_oldest(hmhead);
    struct HeapMgrHandle* hlast = hmhead->last_hndl;
    if (hlast == NULL) {
        return -1;
    }
    while ((hlast->flags & 0x02) != 0)
    {
        hlast = hlast->next_hndl;
        if (hlast == NULL) {
            return -1;
        }
    }
    return heapmgr_free_handle(hmhead, hlast);
}

/**
 * Changes the heap manager linked list so that given element appears as the newest one.
 * @param hmhead
 * @param hmhandle
 */
void heapmgr_make_newest(struct HeapMgrHeader *hmhead, struct HeapMgrHandle *hmhandle)
{
    //_DK_heapmgr_make_newest(hmhead, hmhandle);
    struct HeapMgrHandle* hnext = hmhandle->next_hndl;
    if (hnext != NULL)
    {
        struct HeapMgrHandle* hprev = hmhandle->prev_hndl;
        hnext->prev_hndl = hprev;
        if (hprev != NULL)
        {
            hprev->next_hndl = hmhandle->next_hndl;
        } else
        {
            hmhead->last_hndl = hmhandle->next_hndl;
        }
        struct HeapMgrHandle* hsecond = hmhead->first_hndl;
        hsecond->next_hndl = hmhandle;
        hmhandle->next_hndl = NULL;
        hmhandle->prev_hndl = hsecond;
        hmhead->first_hndl = hmhandle;
    }
}

struct HeapMgrHandle *heapmgr_add_item(struct HeapMgrHeader *hmhdr, long idx)
{
    if ((hmhdr->databuf_free - hmhdr->field_14) < idx)
        return NULL;

    struct HeapMgrHandle* res = NULL;
    if (hmhdr->handles_count > hmhdr->field_10)
    {
        res = (struct HeapMgrHandle*)&hmhdr[1];
        if ((uint16_t)hmhdr[1].databuf_free) // TODO: why were only 16bit checked, padding? hidden member?
            while ((++res)->flags) {}
    }
    if (!res)
        return NULL;
    struct HeapMgrHandle* alloc_iter = hmhdr->first_alloc;
    if (alloc_iter)
    {
        if ((uint8_t*)alloc_iter->buf - hmhdr->databuf_start < idx)
        {
            while (1)
            {
                uint8_t* buf;
                if (alloc_iter->next_alloc)
                    buf = (uint8_t*)alloc_iter->next_alloc->buf;
                else
                    buf = hmhdr->databuf_end;
                int size = &buf[-alloc_iter->len] - (uint8_t*)alloc_iter->buf;
                if (size < 0)
                {
                    ERRORLOG("Overlapping heap blocks");
                    return NULL;
                }
                if (size >= idx)
                    break;
                if (!alloc_iter->next_alloc)
                    return NULL;
                alloc_iter = alloc_iter->next_alloc;
            }
            res->buf = (char*)alloc_iter->buf + alloc_iter->len;
            res->field_C = alloc_iter;
            res->next_alloc = alloc_iter->next_alloc;
            if (alloc_iter->next_alloc)
                alloc_iter->next_alloc->field_C = res;
            alloc_iter->next_alloc = res;
        }
        else
        {
            res->buf = hmhdr->databuf_start;
            res->next_alloc = alloc_iter;
            hmhdr->first_alloc = res;
            alloc_iter->field_C = res;
        }
    }
    else
    {
        hmhdr->first_alloc = res;
        res->buf = hmhdr->databuf_start;
    }
    res->flags = 1;
    res->len = idx;
    hmhdr->field_14 += idx;
    ++hmhdr->field_10;
    if (hmhdr->first_hndl)
    {
        hmhdr->first_hndl->next_hndl = res;
        res->prev_hndl = hmhdr->first_hndl;
    }
    else
        hmhdr->last_hndl = res;

    hmhdr->first_hndl = res;
    if ((uint8_t*)res->buf + res->len >= hmhdr->databuf_end)
        ERRORLOG("Disaster. Handle allocated past end of heap area");
    return res;
}

struct HeapMgrHeader *heapmgr_init(unsigned char *buf_end, long buf_remain, long nheaders)
{
    //return _DK_heapmgr_init(buf_end, buf_remain, nsamples);
    long headers_len = sizeof(struct HeapMgrHeader) + sizeof(struct HeapMgrHandle) * nheaders;
    if (buf_remain <= headers_len)
        return NULL;
    struct HeapMgrHeader* hmgr = (struct HeapMgrHeader*)buf_end;
    hmgr->databuf_start = (buf_end + headers_len);
    hmgr->databuf_end = (buf_end + buf_remain);
    hmgr->databuf_free = buf_remain - headers_len;
    hmgr->handles_count = nheaders;
    hmgr->field_10 = 0;
    hmgr->field_14 = 0;
    hmgr->first_alloc = NULL;
    hmgr->last_hndl = NULL;
    hmgr->first_hndl = NULL;
    return hmgr;
}

void heapmgr_complete_defrag(struct HeapMgrHeader *hmhead)
{
    //_DK_heapmgr_complete_defrag(hmhead); return;
    for (struct HeapMgrHandle* hmhandle = hmhead->first_alloc; hmhandle->next_alloc != NULL; hmhandle = hmhandle->next_alloc)
    {
        void *bufend = (char*)hmhandle->buf + hmhandle->len;
        struct HeapMgrHandle* hnext = hmhandle->next_alloc;
        if (hnext->buf > bufend)
        {
            memmove(bufend, hnext->buf, hnext->len);
            hmhandle->next_alloc = bufend;
        }
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
