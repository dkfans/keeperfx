/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sound.h
 *     Header file for bflib_sound.c.
 * @par Purpose:
 *     Sound and music related routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     16 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_SOUND_H
#define BFLIB_SOUND_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

// Type definitions
struct SoundEmitter {
    unsigned char flags;
    unsigned char field_1;
    unsigned char field_2[2];
    short pos_x;
    short pos_y;
    short pos_z;
    unsigned char field_A[10];
    unsigned char field_14;
    unsigned char field_15;
};

struct SoundReceiver { // sizeof = 17
  short pos_x;
  short pos_y;
  short pos_z;
  unsigned short field_6;
  unsigned short field_8;
  unsigned short field_A;
  long field_C;
  char field_10;
};

/******************************************************************************/
// Exported variables
DLLIMPORT extern int _DK_SoundDisabled;
#define SoundDisabled _DK_SoundDisabled
DLLIMPORT extern struct SoundEmitter _DK_emitter[128];
#define emitter _DK_emitter
DLLIMPORT extern long _DK_Non3DEmitter;
#define Non3DEmitter _DK_Non3DEmitter
DLLIMPORT extern long _DK_SpeechEmitter;
#define SpeechEmitter _DK_SpeechEmitter

DLLIMPORT extern struct SoundReceiver _DK_Receiver;
#define Receiver _DK_Receiver
DLLIMPORT extern long _DK_MaxSoundDistance;
#define MaxSoundDistance _DK_MaxSoundDistance

#pragma pack()
/******************************************************************************/
// Exported variables

/******************************************************************************/
// Exported functions
long S3DSetSoundReceiverPosition(int pos_x, int pos_y, int pos_z);
long S3DSetSoundReceiverOrientation(int ori_a, int ori_b, int ori_c);
long S3DDestroySoundEmitter(long eidx);
long S3DEmitterHasFinishedPlaying(long eidx);
long S3DMoveSoundEmitterTo(long eidx, long x, long y, long z);
long S3DInit(void);
long S3DSetNumberOfSounds(long nMaxSounds);
long S3DSetMaximumSoundDistance(long nDistance);
long S3DAddSampleToEmitterPri(long emidx, long a2, long a3, long a4, long a5, long a6, char a7, long a8, long a9);
long S3DCreateSoundEmitterPri(long x, long y, long z, long a4, long a5, long a6, long a7, long a8, long a9, long a10);

void play_non_3d_sample(long sample_idx);
short sound_emitter_in_use(long emidx);
long get_best_sound_heap_size(long mem_size);
struct SampleInfo *play_sample_using_heap(unsigned long a1, short a2, unsigned long a3, unsigned long a4, unsigned long a5, char a6, unsigned char a7, unsigned char a8);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
