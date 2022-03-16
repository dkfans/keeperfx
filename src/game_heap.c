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
#include "game_heap.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "bflib_heapmgr.h"
#include "config.h"
#include "front_simple.h"
#include "engine_render.h"
#include "sounds.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
const char *sound_fname = "sound.dat";
const char *speech_fname = "speech.dat";
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

TbBool setup_heap_manager(void)
{
    SYNCDBG(8,"Starting");
    if (heap == NULL)
    {
        ERRORLOG("Graphics Heap not allocated");
        return false;
    }
    long i = heap_size / 512;
    if (i >= KEEPSPRITE_LENGTH)
      i = KEEPSPRITE_LENGTH-1;
    graphics_heap = heapmgr_init(heap, heap_size, i);
    if (graphics_heap == NULL)
    {
        ERRORLOG("Not enough memory to initialize heap.");
        return false;
    }
    wait_for_cd_to_be_available();
#ifdef SPRITE_FORMAT_V2
    fname = prepare_file_fmtpath(FGrp_StdData,"thingspr-%d.jty",32);
#else
    const char* fname = prepare_file_path(FGrp_StdData, "creature.jty");
#endif
    file_handle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (file_handle == -1) {
        ERRORLOG("Can not open JTY file, \"%s\"",fname);
        return false;
    }
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        keepsprite[i] = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        heap_handle[i] = NULL;
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
  heap_size = get_best_sound_heap_size(i);
  while ( 1 )
  {
    heap = LbMemoryAlloc(heap_size);
    if (heap != NULL)
      break;
    i = get_smaller_memory_amount(i);
    if (i > 8)
    {
      heap_size = get_best_sound_heap_size(i);
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
    if (file_handle != -1)
    {
        LbFileClose(file_handle);
        file_handle = -1;
    }
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        keepsprite[i] = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        heap_handle[i] = NULL;
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
      close_sound_heap();
      if (sound_heap_memory != NULL)
      {
        LbMemoryFree(sound_heap_memory);
        sound_heap_memory = NULL;
      }
    }
    if (heap != NULL)
    {
      ERRORLOG("Graphics heap already allocated");
      LbMemoryFree(heap);
      heap = NULL;
    }
    // Allocate sound heap
    if (!SoundDisabled)
    {
      i = mem_size;
      while (sound_heap_memory == NULL)
      {
        sound_heap_size = get_best_sound_heap_size(i);
        i = get_smaller_memory_amount(i);
        sound_heap_memory = LbMemoryAlloc(sound_heap_size);
        if ((i <= 8) && (sound_heap_memory == NULL))
        {
          low_memory = true;
          break;
        }
      }
    }
    // Allocate graphics heap
    i = mem_size;
    while (heap == NULL)
    {
      heap_size = get_best_sound_heap_size(i);
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
        if ((!SoundDisabled) && (sound_heap_memory == NULL))
        {
          break;
        }
        if (!SoundDisabled)
        {
          if (sound_heap_size < heap_size)
          {
            heap_size -= 16384;
          } else
          if (sound_heap_size == heap_size)
          {
            heap_size -= 16384;
            sound_heap_size -= 16384;
          } else
          {
            sound_heap_size -= 16384;
          }
          if (sound_heap_size < 524288)
          {
            ERRORLOG("Unable to allocate heaps (small_mem)");
            return false;
          }
        } else
        {
          heap_size -= 16384;
        }
        if (heap_size < 524288)
        {
          if (sound_heap_memory != NULL)
          {
            LbMemoryFree(sound_heap_memory);
            sound_heap_memory = NULL;
          }
          ERRORLOG("Unable to allocate heaps (small_mem)");
          return false;
          }
        }
        if (sound_heap_memory != NULL)
        {
          LbMemoryFree(sound_heap_memory);
          sound_heap_memory = NULL;
        }
        if (heap != NULL)
        {
          LbMemoryFree(heap);
          heap = NULL;
        }
        if (!SoundDisabled)
        {
          sound_heap_memory = LbMemoryAlloc(sound_heap_size);
        }
        heap = LbMemoryAlloc(heap_size);
    }
    if (!SoundDisabled)
    {
      SYNCMSG("SoundHeap Size %d", sound_heap_size);
      // Prepare sound sample bank file names
      char snd_fname[2048];
      prepare_file_path_buf(snd_fname, FGrp_LrgSound, sound_fname);
      // language-specific speech file
      char* spc_fname = prepare_file_fmtpath(FGrp_LrgSound, "speech_%s.dat", get_language_lwrstr(install_info.lang_id));
      // default speech file
      if (!LbFileExists(spc_fname))
        spc_fname = prepare_file_path(FGrp_LrgSound,speech_fname);
      // speech file for english
      if (!LbFileExists(spc_fname))
        spc_fname = prepare_file_fmtpath(FGrp_LrgSound,"speech_%s.dat",get_language_lwrstr(1));
      // Initialize sample banks
      if (!init_sound_heap_two_banks(sound_heap_memory, sound_heap_size, snd_fname, spc_fname, 1622))
      {
        LbMemoryFree(sound_heap_memory);
        sound_heap_memory = NULL;
        SoundDisabled = true;
        ERRORLOG("Unable to initialize sound heap. Sound disabled.");
      }
    }
    return true;
}

TbBool read_heap_item(struct HeapMgrHandle *hmhandle, long offs, long len)
{
    if (file_handle == -1) {
        return false;
    }
    // TODO make error handling
    LbFileSeek(file_handle, offs, 0);
    LbFileRead(file_handle, hmhandle->buf, len);
    return true;
}
/******************************************************************************/
