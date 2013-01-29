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
struct HeapMgrHeader { // sizeof = 36
  unsigned char *field_0;
  unsigned char *field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned long field_14;
  unsigned long field_18;
  unsigned long field_1C;
  struct HeapMgrHandle *field_20;
};

struct HeapMgrHandle { // sizeof = 28
  void *buf;
  unsigned long field_4;
  unsigned short field_8;
  unsigned short field_A;
  unsigned long field_C;
  unsigned long field_10;
  struct HeapMgrHandle *field_14;
  struct HeapMgrHandle *field_18;
};



/******************************************************************************/
struct HeapMgrHandle *find_free_handle(struct HeapMgrHeader *hmhead);
long heapmgr_free_oldest(struct HeapMgrHeader *hmhead);
void heapmgr_make_newest(struct HeapMgrHeader *hmhead, struct HeapMgrHandle *hmhandle);
struct HeapMgrHandle *heapmgr_add_item(struct HeapMgrHeader *hmhead, long idx);
struct HeapMgrHeader *heapmgr_init(unsigned char *buf_end, long buf_remain, long nsamples);


/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
