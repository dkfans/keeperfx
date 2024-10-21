/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_heap.c
 *     Definition of heap, used for storing memory-expensive sounds and graphics.
 * @par Purpose:
 *     Functions to create and maintain memory heap.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_heap.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "config.h"
#include "front_simple.h"
#include "engine_render.h"
#include "sounds.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
static unsigned char *heap;
static long heap_size;
/******************************************************************************/
long get_smaller_memory_amount(long amount)
{
    if (amount > 64)
      return 64;
    if (amount > 48)
      return 48;
    if (amount > 32)
      return 32;
    if (amount > 24)
      return 24;
    if (amount > 16)
      return 16;
    if (amount >  8)
      return  8;
    return 6;
}

long get_best_heap_size(long sh_mem_size)
{
    if (sh_mem_size < 8)
    {
      ERRORLOG("Unhandled PhysicalMemory");
      return 0;
    }
    if (sh_mem_size <= 8)
      return 0x0100000; // 1MB
    if (sh_mem_size <= 16)
      return 0x0200000; // 2MB
    if (sh_mem_size <= 24)
      return 0x0500000; // 5MB
    if (sh_mem_size <= 32)
      return 0x0800000; // 8MB
    if (sh_mem_size <= 48)
        return 0x0c00000; // 12MB

    return 0x3000000; // 50MB
}

TbBool setup_heap_manager(void)
{
    SYNCDBG(8,"Starting");
    if (heap == NULL)
    {
        ERRORLOG("Graphics Heap not allocated");
        return false;
    }
    long i;
#ifdef SPRITE_FORMAT_V2
    fname = prepare_file_fmtpath(FGrp_StdData,"thingspr-%d.jty",32);
#else
    const char* fname = prepare_file_path(FGrp_StdData, "creature.jty");
#endif
    jty_file_handle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (!jty_file_handle) {
        ERRORLOG("Can not open JTY file, \"%s\"",fname);
        return false;
    }
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        keepsprite[i] = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        sprite_heap_handle[i] = NULL;
    return true;
}

/**
 * Allocates graphics heap.
 */
TbBool setup_heap_memory(void)
{
  SYNCDBG(8,"Starting");
  if (heap != NULL)
  {
    SYNCDBG(0,"Freeing old Graphics heap");
    LbMemoryFree(heap);
    heap = NULL;
  }
  long i = mem_size;
  heap_size = get_best_heap_size(i);
  while ( 1 )
  {
    heap = LbMemoryAlloc(heap_size);
    if (heap != NULL)
      break;
    i = get_smaller_memory_amount(i);
    if (i > 8)
    {
      heap_size = get_best_heap_size(i);
    } else
    {
      if (heap_size < 524288)
      {
        ERRORLOG("Unable to allocate Graphic Heap");
        heap_size = 0;
        return false;
      }
      heap_size -= 16384;
    }
  }
  SYNCMSG("GraphicsHeap Size %d", heap_size);
  return true;
}

void reset_heap_manager(void)
{
    long i;
    SYNCDBG(8,"Starting");
    LbFileClose(jty_file_handle);
    jty_file_handle = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        keepsprite[i] = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        sprite_heap_handle[i] = NULL;
}

void reset_heap_memory(void)
{
  SYNCDBG(8,"Starting");
  LbMemoryFree(heap);
  heap = NULL;
}

TbBool setup_heaps(void)
{
    long i;
    SYNCDBG(8,"Starting");
    TbBool low_memory = false;
    if (!SoundDisabled)
    {
      StopAllSamples();
    }
    if (heap != NULL)
    {
      ERRORLOG("Graphics heap already allocated");
      LbMemoryFree(heap);
      heap = NULL;
    }
    // Allocate graphics heap
    i = mem_size;
    while (heap == NULL)
    {
      heap_size = get_best_heap_size(i);
      i = get_smaller_memory_amount(i);
      heap = LbMemoryAlloc(heap_size);
      if ((i <= 8) && (heap == NULL))
      {
        low_memory = true;
        break;
      }
    }
    SYNCMSG("GraphicsHeap Size %d", heap_size);

    if (low_memory)
    {
      SYNCDBG(8,"Low memory mode entered on heap allocation.");
      while (heap != NULL)
      {
        if (SoundDisabled)
        {
          heap_size -= 16384;
        }
        if (heap_size < 524288)
        {
          ERRORLOG("Unable to allocate heaps (small_mem)");
          return false;
          }
        }
        if (heap != NULL)
        {
          LbMemoryFree(heap);
          heap = NULL;
        }
        heap = LbMemoryAlloc(heap_size);
    }
    return true;
}

void *he_alloc(size_t size)
{
    // We could need some wrapper
    return malloc(size);
}

void he_free(void *data)
{
    if (data)
        free(data);
}
/******************************************************************************/
