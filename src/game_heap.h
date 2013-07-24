/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_heap.h
 *     Header file for game_heap.c.
 * @par Purpose:
 *     Definition of heap, used for storing memory-expensive sounds and graphics.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GAME_HEAP_H
#define DK_GAME_HEAP_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_heapmgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

DLLIMPORT long _DK_sound_heap_size;
#define sound_heap_size _DK_sound_heap_size
DLLIMPORT unsigned char *_DK_sound_heap_memory;
#define sound_heap_memory _DK_sound_heap_memory
DLLIMPORT long _DK_heap_size;
#define heap_size _DK_heap_size
DLLIMPORT unsigned char *_DK_heap;
#define heap _DK_heap

#pragma pack()
/******************************************************************************/
TbBool setup_heap_manager(void);
TbBool setup_heap_memory(void);
void reset_heap_manager(void);
void reset_heap_memory(void);
TbBool setup_heaps(void);

TbBool read_heap_item(struct HeapMgrHandle *hmhandle, long offs, long len);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
