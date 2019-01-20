/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_heapmgr.h
 *     Header file for bflib_heapmgr.c.
 * @par Purpose:
 *     Allocating and maintaining heap memory.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     04 Jan 2009 - 10 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_HEAPMGR_H
#define BFLIB_HEAPMGR_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct HeapMgrHeader { // sizeof = 36
  unsigned char *databuf_start;
  unsigned char *databuf_end;
  unsigned long databuf_free;
  unsigned long handles_count;
  unsigned long field_10;
  unsigned long field_14;
  struct HeapMgrHandle *first_alloc;
  struct HeapMgrHandle *last_hndl;
  struct HeapMgrHandle *first_hndl;
};

struct HeapMgrHandle { // sizeof = 28
  void *buf;
  unsigned long len;
  unsigned short flags;
  unsigned short idx;
  struct HeapMgrHandle* field_C;
  struct HeapMgrHandle *next_alloc;
  struct HeapMgrHandle *prev_hndl;
  struct HeapMgrHandle *next_hndl;
};

#pragma pack()
/******************************************************************************/
struct HeapMgrHandle *find_free_handle(struct HeapMgrHeader *hmhead);
long heapmgr_free_oldest(struct HeapMgrHeader *hmhead);
void heapmgr_make_newest(struct HeapMgrHeader *hmhead, struct HeapMgrHandle *hmhandle);
struct HeapMgrHandle *heapmgr_add_item(struct HeapMgrHeader *hmhead, long idx);
struct HeapMgrHeader *heapmgr_init(unsigned char *buf_end, long buf_remain, long nsamples);
void heapmgr_complete_defrag(struct HeapMgrHeader *hmhead);


/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
