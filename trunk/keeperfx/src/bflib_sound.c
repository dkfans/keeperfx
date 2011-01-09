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
 * @author   KeeperFX Team
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
#include "bflib_memory.h"
#include "bflib_heapmgr.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_stop_sample_using_heap(unsigned long a1, short a2, unsigned char a3);
DLLIMPORT long _DK_start_emitter_playing(struct SoundEmitter *emit, long a2, long a3, long a4, long a5, long a6, long a7, long a8, long a9);
DLLIMPORT void _DK_close_sound_heap(void);
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
long NoSoundEmitters = SOUND_EMITTERS_MAX;
/******************************************************************************/
// Internal routines
long allocate_free_sound_emitter(void);
void delete_sound_emitter(long idx);
long start_emitter_playing(struct SoundEmitter *emit, long a2, long a3, long a4, long a5, long a6, long a7, long a8, long a9);
/******************************************************************************/
// Functions

long get_best_sound_heap_size(long mem_size)
{
  //return _DK_get_best_sound_heap_size(mem_size);
  if (mem_size < 8)
  {
    ERRORLOG("Unhandled PhysicalMemory");
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

TbBool S3DEmitterIsPlayingSample(long emitr, long smpl_idx, long a2)
{
  struct S3DSample *smpl3d;
  long i;
  for (i=0; i < MaxNoSounds; i++)
  {
    smpl3d = &SampleList[i];
    if ((smpl3d->field_1F != 0) && (smpl3d->emit_ptr == &emitter[emitr]) && (smpl3d->field_8 == smpl_idx) && (smpl3d->field_A == a2))
      return true;
  }
  return false;
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

TbBool S3DAddSampleToEmitterPri(long eidx, long a2, long a3, long a4, long a5, long a6, char a7, long a8, long a9)
{
  struct SoundEmitter *emit;
  //return _DK_S3DAddSampleToEmitterPri(emidx, a2, a3, a4, a5, a6, a7, a8, a9);
  emit = &emitter[eidx];
  return start_emitter_playing(emit, a2, a3, a4, a5, a6, a7, a8, a9) != 0;
}

long S3DCreateSoundEmitterPri(long x, long y, long z, long a4, long a5, long a6, long a7, long a8, long a9, long a10)
{
  struct SoundEmitter *emit;
  long eidx;
  //return _DK_S3DCreateSoundEmitterPri(x, y, z, a4, a5, a6, a7, a8, a9, a10);
  eidx = allocate_free_sound_emitter();
  if (eidx <= 0)
    return 0;
  emit = &emitter[eidx];
  emit->field_14 = 100;
  emit->field_15 = 100;
  emit->pos_x = x;
  emit->pos_z = z;
  emit->field_1 = a9;
  emit->pos_y = y;
  if (start_emitter_playing(emit, a4, a5, a6, a7, a8, 3, a9, a10))
    return eidx;
  delete_sound_emitter(eidx);
  return 0;
}

TbBool S3DDestroySoundEmitterAndSamples(long eidx)
{
  stop_emitter_samples(&emitter[eidx]);
  delete_sound_emitter(eidx);
  return true;
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

void S3DSetLineOfSightFunction(S3D_LineOfSight_Func callback)
{
  LineOfSightFunction = callback;
}

void S3DSetDeadzoneRadius(long radius)
{
  deadzone_radius = radius;
}

long S3DGetDeadzoneRadius(void)
{
  return deadzone_radius;
}

short sound_emitter_in_use(long eidx)
{
  return S3DEmitterIsAllocated(eidx);
}

void close_sound_heap(void)
{
  // TODO: use rewritten version when sound routines are rewritten
  _DK_close_sound_heap(); return;

  if (sound_file != -1)
  {
    LbFileClose(sound_file);
    sound_file = -1;
  }
  if (sound_file2 != -1)
  {
    LbFileClose(sound_file2);
    sound_file2 = -1;
  }
  using_two_banks = 0;
}

void play_non_3d_sample(long sample_idx)
{
  if (SoundDisabled)
    return;
  if (GetCurrentSoundMasterVolume() <= 0)
    return;
  if (Non3DEmitter != 0)
    if (!sound_emitter_in_use(Non3DEmitter))
    {
      ERRORLOG("Non 3d Emitter has been deleted!");
      Non3DEmitter = 0;
    }
  if (Non3DEmitter == 0)
  {
    Non3DEmitter = S3DCreateSoundEmitterPri(0, 0, 0, sample_idx, 0, 100, 256, 0, 8, 2147483646);
  } else
  {
    S3DAddSampleToEmitterPri(Non3DEmitter, sample_idx, 0, 100, 256, 0, 3, 8, 2147483646);
  }
}

void play_non_3d_sample_no_overlap(long smpl_idx)
{
  if (SoundDisabled)
    return;
  if (GetCurrentSoundMasterVolume() <= 0)
    return;
  if (Non3DEmitter != 0)
  {
    if (!sound_emitter_in_use(Non3DEmitter))
    {
      ERRORLOG("Non 3d Emitter has been deleted!");
      Non3DEmitter = 0;
    }
  }
  if (Non3DEmitter == 0)
  {
    Non3DEmitter = S3DCreateSoundEmitterPri(0, 0, 0, smpl_idx, 0, 100, 256, 0, 8, 0x7FFFFFFE);
  } else
  if (!S3DEmitterIsPlayingSample(Non3DEmitter, smpl_idx, 0))
  {
    S3DAddSampleToEmitterPri(Non3DEmitter, smpl_idx, 0, 100, 256, 0, 3, 8, 0x7FFFFFFE);
  }
}

/*
 * Initializes and returns sound emitter structure.
 * Returns its index; if no free emitter is found, returns 0.
 */
long allocate_free_sound_emitter(void)
{
  struct SoundEmitter *emit;
  long i;
  for (i=1; i < NoSoundEmitters; i++)
  {
    emit = &emitter[i];
    if ((emit->flags & 0x01) == 0)
    {
      emit->flags = 0x01;
      emit->field_2 = i;
      return i;
    }
  }
  return 0;
}

/*
 * Clears sound emitter structure and marks it as unused.
 */
void delete_sound_emitter(long idx)
{
  struct SoundEmitter *emit;
  emit = &emitter[idx];
  if ((emit <= &emitter[0]) || (emit > &emitter[SOUND_EMITTERS_MAX-1]))
  {
    WARNLOG("Tried to delete outranged emitter");
  }
  if ((emit->flags & 0x01) != 0)
  {
    LbMemorySet(emit, 0, sizeof(struct SoundEmitter));
  }
}

long stop_emitter_samples(struct SoundEmitter *emit)
{
  struct S3DSample *smpl3d;
  long num_stopped;
  long i;
  num_stopped = 0;
  for (i=0; i < MaxNoSounds; i++)
  {
    smpl3d = &SampleList[i];
    if ((smpl3d->field_1F != 0) && (smpl3d->emit_ptr == emit))
    {
      stop_sample_using_heap(emit->field_2 + 4000, smpl3d->field_8, smpl3d->field_A);
      smpl3d->field_1F = 0;
      num_stopped++;
    }
  }
  return num_stopped;
}

struct HeapMgrHandle *find_handle_for_new_sample(long smpl_len, long smpl_idx, long file_pos, unsigned char bank_id)
{
  struct SampleTable *smp_table;
  struct HeapMgrHandle *hmhandle;
  long i;
  if ((!using_two_banks) && (bank_id > 0))
  {
    ERRORLOG("Trying to use two sound banks when only one has been set up");
    return NULL;
  }
  hmhandle = heapmgr_add_item(sndheap, smpl_len);
  if (hmhandle == NULL)
  {
    while (sndheap->field_10)
    {
      hmhandle = heapmgr_add_item(sndheap, smpl_len);
      if (hmhandle != NULL)
        break;
      i = heapmgr_free_oldest(sndheap);
      if (i < 0)
        break;
      if (i < samples_in_bank)
      {
        smp_table = &sample_table[i];
      } else
      {
        smp_table = &sample_table2[i-samples_in_bank];
      }
      smp_table->hmhandle = hmhandle;
    }
  }
  if (hmhandle == NULL)
    return NULL;
  if (bank_id > 0)
  {
    hmhandle->field_A = samples_in_bank + smpl_idx;
    LbFileSeek(sound_file2, file_pos, Lb_FILE_SEEK_BEGINNING);
    LbFileRead(sound_file2, hmhandle->field_0, smpl_len);
  } else
  {
    hmhandle->field_A = smpl_idx;
    LbFileSeek(sound_file, file_pos, Lb_FILE_SEEK_BEGINNING);
    LbFileRead(sound_file, hmhandle->field_0, smpl_len);
  }
  return hmhandle;
}

struct SampleInfo *play_sample_using_heap(unsigned long a1, short smpl_idx, unsigned long a3, unsigned long a4, unsigned long a5, char a6, unsigned char a7, unsigned char bank_id)
{
  struct SampleInfo *sample;
  struct SampleTable *smp_table;
  if ((!using_two_banks) && (bank_id > 0))
  {
    ERRORLOG("Trying to use two sound banks when only one has been set up");
    return NULL;
  }
  // TODO: use rewritten version when sound routines are rewritten
  return _DK_play_sample_using_heap(a1, smpl_idx, a3, a4, a5, a6, a7, bank_id);

  if (bank_id > 0)
  {
    if (sound_file2 == -1)
      return 0;
    if ((smpl_idx <= 0) || (smpl_idx >= samples_in_bank2))
    {
      ERRORLOG("Sample %d exceeds bank %d bounds",smpl_idx,2);
      return NULL;
    }
    smp_table = &sample_table2[smpl_idx];
  } else
  {
    if (sound_file == -1)
      return 0;
    if ((smpl_idx <= 0) || (smpl_idx >= samples_in_bank))
    {
      ERRORLOG("Sample %d exceeds bank %d bounds",smpl_idx,1);
      return NULL;
    }
    smp_table = &sample_table[smpl_idx];
  }
  if (smp_table->hmhandle == NULL)
    smp_table->hmhandle = find_handle_for_new_sample(smp_table->field_4, smpl_idx, smp_table->field_0, bank_id);
  if (smp_table->hmhandle == NULL)
  {
    ERRORLOG("Can't find handle to play sample %d",smpl_idx);
    return NULL;
  }
  heapmgr_make_newest(sndheap, smp_table->hmhandle);
  sample = PlaySampleFromAddress(a1, smpl_idx, a3, a4, a5, a6, a7, smp_table->hmhandle, smp_table->field_8);
  if (sample == NULL)
  {
    ERRORLOG("Can't start playing sample %d",smpl_idx);
    return NULL;
  }
  sample->field_17 |= 0x01;
  if (bank_id != 0)
    sample->field_17 |= 0x04;
  smp_table->hmhandle->field_8 |= 0x06;
  return sample;
}

void stop_sample_using_heap(unsigned long a1, short a2, unsigned char a3)
{
  _DK_stop_sample_using_heap(a1, a2, a3);
}

long speech_sample_playing(void)
{
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
      ERRORLOG("Speech Emitter has been deleted");
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
      ERRORLOG("Speech Emitter has been deleted");
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
    ERRORLOG("Cannot create speech emitter.");
    return false;
  }
  return true;
}

long start_emitter_playing(struct SoundEmitter *emit, long a2, long a3, long a4, long a5, long a6, long a7, long a8, long a9)
{
  return _DK_start_emitter_playing(emit, a2, a3, a4, a5, a6, a7, a8, a9);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
