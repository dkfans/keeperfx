/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sound.c
 *     Sound and music related routines.
 * @par Purpose:
 *     Sound and music routines to use in games.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     16 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_sound.h"

#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"
#include "bflib_sndlib.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_play_non_3d_sample(long sidx);
DLLIMPORT struct SampleInfo *_DK_play_sample_using_heap(unsigned long a1, short a2, unsigned long a3, unsigned long a4, unsigned long a5, char a6, unsigned char a7, unsigned char a8);
DLLIMPORT long _DK_S3DAddSampleToEmitterPri(long, long, long, long, long, long, char, long, long);
DLLIMPORT long _DK_S3DCreateSoundEmitterPri(long, long, long, long, long, long, long, long, long, long);
DLLIMPORT long _DK_S3DSetSoundReceiverPosition(int pos_x, int pos_y, int pos_z);
DLLIMPORT long _DK_S3DSetSoundReceiverOrientation(int ori_a, int ori_b, int ori_c);
DLLIMPORT long _DK_S3DDestroySoundEmitter(long eidx);
DLLIMPORT long _DK_S3DEmitterHasFinishedPlaying(long eidx);
DLLIMPORT long _DK_S3DMoveSoundEmitterTo(long eidx, long x, long y, long z);
DLLIMPORT long _DK_get_best_sound_heap_size(long mem_size);
DLLIMPORT long _DK_S3DInit(void);
DLLIMPORT long _DK_S3DSetNumberOfSounds(long nMaxSounds);
DLLIMPORT long _DK_S3DSetMaximumSoundDistance(long nDistance);

// Global variables
/******************************************************************************/
// Functions

long get_best_sound_heap_size(long mem_size)
{
  static const char *func_name="get_best_sound_heap_size";
  //return _DK_get_best_sound_heap_size(mem_size);
  if (mem_size < 8)
  {
    error(func_name, 59, "Unhandled PhysicalMemory");
    return 0;
  }
  if (mem_size <= 8)
    return 0x100000; // 1MB
  if (mem_size <= 16)
    return 0x200000; // 2MB
  if (mem_size <= 24)
    return 0x500000; // 5MB
  if (mem_size <= 32)
    return 0x800000; // 8MB
  return 0xC00000; // 12MB
}

long S3DInit(void)
{
  return _DK_S3DInit();
}

long S3DSetNumberOfSounds(long nMaxSounds)
{
  if (nMaxSounds > SOUNDS_MAX_COUNT)
    nMaxSounds = SOUNDS_MAX_COUNT;
  if (nMaxSounds < 1)
    nMaxSounds = 1;
  MaxNoSounds = nMaxSounds;
  return true;
}

long S3DSetMaximumSoundDistance(long nDistance)
{
  return _DK_S3DSetMaximumSoundDistance(nDistance);
}

long S3DSetSoundReceiverPosition(int pos_x, int pos_y, int pos_z)
{
  return _DK_S3DSetSoundReceiverPosition(pos_x, pos_y, pos_z);
}

long S3DSetSoundReceiverOrientation(int ori_a, int ori_b, int ori_c)
{
  return _DK_S3DSetSoundReceiverOrientation(ori_a, ori_b, ori_c);
}

long S3DDestroySoundEmitter(long eidx)
{
  return _DK_S3DDestroySoundEmitter(eidx);
}

long S3DEmitterHasFinishedPlaying(long eidx)
{
  return ((emitter[eidx].flags & 0x02) == 0);
}

long S3DMoveSoundEmitterTo(long eidx, long x, long y, long z)
{
  return _DK_S3DMoveSoundEmitterTo(eidx, x, y, z);
}

long S3DAddSampleToEmitterPri(long emidx, long a2, long a3, long a4, long a5, long a6, char a7, long a8, long a9)
{
  return _DK_S3DAddSampleToEmitterPri(emidx, a2, a3, a4, a5, a6, a7, a8, a9);
}

long S3DCreateSoundEmitterPri(long x, long y, long z, long a4, long a5, long a6, long a7, long a8, long a9, long a10)
{
  return _DK_S3DCreateSoundEmitterPri(x, y, z, a4, a5, a6, a7, a8, a9, a10);
}

long S3DEmitterIsAllocated(long eidx)
{
  return (eidx > 0) && (emitter[eidx].flags & 0x01);
}

long S3DEmitterIsPlayingAnySample(long eidx)
{
  struct S3DSample *sample;
  long i;
  if (MaxNoSounds <= 0)
    return false;
  for (i=0; i < MaxNoSounds; i++)
  {
    sample = &SampleList[i];
    if ((sample->field_1F) && (sample->emit_ptr == &emitter[eidx]))
      return true;
  }
  return false;
}

short sound_emitter_in_use(long eidx)
{
  return S3DEmitterIsAllocated(eidx);
}

void play_non_3d_sample(long sample_idx)
{
  static const char *func_name="play_non_3d_sample";
  if (SoundDisabled)
    return;
  if (GetCurrentSoundMasterVolume() <= 0)
    return;
  if (Non3DEmitter!=0)
    if ( sound_emitter_in_use(Non3DEmitter) == 0 )
    {
      error(func_name, 263, "Non 3d Emitter has been deleted!");
      Non3DEmitter = 0;
    }
  if (Non3DEmitter!=0)
  {
    _DK_S3DAddSampleToEmitterPri(Non3DEmitter, sample_idx, 0, 100, 256, 0, 3, 8, 2147483646);
  } else
  {
    Non3DEmitter = _DK_S3DCreateSoundEmitterPri(0, 0, 0, sample_idx, 0, 100, 256, 0, 8, 2147483646);
  }
}

struct SampleInfo *play_sample_using_heap(unsigned long a1, short a2, unsigned long a3, unsigned long a4, unsigned long a5, char a6, unsigned char a7, unsigned char a8)
{
  return _DK_play_sample_using_heap(a1, a2, a3, a4, a5, a6, a7, a8);
}

long speech_sample_playing(void)
{
  static const char *func_name="speech_sample_playing";
  long sp_emiter;
  if (SoundDisabled)
    return false;
  if (GetCurrentSoundMasterVolume() <= 0)
    return false;
  sp_emiter = SpeechEmitter;
  if (sp_emiter != 0)
  {
    if (S3DEmitterIsAllocated(SpeechEmitter))
    {
      sp_emiter = SpeechEmitter;
    } else
    {
      error(func_name, 339, "Speech Emitter has been deleted");
      sp_emiter = 0;
    }
  }
  SpeechEmitter = sp_emiter;
  if (sp_emiter == 0)
    return false;
  return S3DEmitterIsPlayingAnySample(sp_emiter);
}

long play_speech_sample(long smpl_idx)
{
  static const char *func_name="play_speech_sample";
  long sp_emiter;
  if (SoundDisabled)
    return false;
  if (GetCurrentSoundMasterVolume() <= 0)
    return false;
  sp_emiter = SpeechEmitter;
  if (sp_emiter != 0)
  {
    if (S3DEmitterIsAllocated(SpeechEmitter))
    {
      sp_emiter = SpeechEmitter;
    } else
    {
      error(func_name, 295, "Speech Emitter has been deleted");
      sp_emiter = 0;
    }
  }
  SpeechEmitter = sp_emiter;
  if (sp_emiter != 0)
  {
    if (S3DEmitterHasFinishedPlaying(sp_emiter))
      if (S3DAddSampleToEmitterPri(SpeechEmitter, smpl_idx, 1, 100, 256, 0, 3, 8, 2147483647))
        return true;
    return false;
  }
  sp_emiter = S3DCreateSoundEmitterPri(0, 0, 0, smpl_idx, 1, 100, 256, 0, 8, 2147483647);
  SpeechEmitter = sp_emiter;
  if (sp_emiter == 0)
  {
    error(func_name, 308, "Cannot create speech emitter.");
    return false;
  }
  return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
