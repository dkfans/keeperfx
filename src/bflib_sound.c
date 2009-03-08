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
  return _DK_get_best_sound_heap_size(mem_size);
}

long S3DInit(void)
{
  return _DK_S3DInit();
}

long S3DSetNumberOfSounds(long nMaxSounds)
{
  return _DK_S3DSetNumberOfSounds(nMaxSounds);
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
  return _DK_S3DEmitterHasFinishedPlaying(eidx);
}

long S3DMoveSoundEmitterTo(long eidx, long x, long y, long z)
{
  return _DK_S3DMoveSoundEmitterTo(eidx, x, y, z);
}

short sound_emitter_in_use(long emidx)
{
  return (emidx!=0) && (_DK_emitter[emidx].flags & 1);
}


void play_non_3d_sample(long sample_idx)
{
  static const char *func_name="play_non_3d_sample";
  if ( SoundDisabled )
    return;
  if ( GetCurrentSoundMasterVolume() <= 0 )
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


/******************************************************************************/
#ifdef __cplusplus
}
#endif
