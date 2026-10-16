/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file kernel_memtracker.h
 *     Header file for kernel_memtracker.c.
 * @par Purpose:
 *     Asks the OS which memory pages were written since the last clear.
 * @par Comment:
 *      Presently only supports one tracker at a time, and up to 32 distinct memory ranges.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_KERNEL_MEMTRACKER_H
#define DK_KERNEL_MEMTRACKER_H

#include "bflib_basics.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define KMT_MAX_RANGES 4 /* maximum distinct contiguous memory regions we can track (easily changed) */

TbBool kernel_memtracker_init(void);

// Constant, power of 2, >= 4096.
size_t kernel_memtracker_chunk_size(void);

// ptr and len must be aligned to chunk size.
// Should point to statically-allocated memory.
// returns false on failure to add.
TbBool kernel_memtracker_add(void *ptr, size_t len);
size_t kernel_memtracker_get_chunk_count(void);

// mark all chunks as non-dirty
void kernel_memtracker_clear(void);

// get dirty bits (one per unsigned char)
// dirty must point to uint8_t[kernel_memtracker_get_chunk_count()]
TbBool kernel_memtracker_collect(unsigned char *dirty);

// stop tracking chunk writes
void kernel_memtracker_disarm(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
