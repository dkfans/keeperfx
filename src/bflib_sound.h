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
 * @author   KeeperFX Team
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
#define SOUNDS_MAX_COUNT  16
#define SOUND_EMITTERS_MAX 128
/******************************************************************************/
#pragma pack(1)

struct HeapMgrHeader;
struct HeapMgrHandle;

// Type definitions

enum SoundEmitterFlags {
    Emi_IsAllocated  = 0x01,
    Emi_UnknownPlay  = 0x02,
    Emi_IsMoving     = 0x04,
};

enum SoundSampleFlags {
    Smp_Unknown01  = 0x01,
    Smp_Unknown02  = 0x02,
};

typedef long (*S3D_LineOfSight_Func)(long, long, long, long, long, long);

struct SoundCoord3d {
    unsigned short val_x;
    unsigned short val_y;
    unsigned short val_z;
};

struct SoundEmitter {
    unsigned char flags;
    unsigned char field_1;
    short index;
    struct SoundCoord3d pos;
    unsigned char field_A[6];
    long pitch_doppler;
    unsigned char curr_pitch;
    unsigned char target_pitch;
};

struct SoundReceiver { // sizeof = 17
    struct SoundCoord3d pos;
    unsigned short orient_a;
    unsigned short orient_b;
    unsigned short orient_c;
    unsigned long flags;
    unsigned char sensivity;
};

struct S3DSample { // sizeof = 37
  unsigned long field_0;
  unsigned long time_turn;
  unsigned short smptbl_id;
  unsigned char bank_id;
  unsigned short base_pitch;
  unsigned short pan;
  unsigned short volume;
  struct SampleInfo *smpinfo;
  struct SoundEmitter *emit_ptr;
  long emit_idx;
  char field_1D; // signed
  unsigned char flags;
  unsigned char is_playing;
  unsigned char sfxid;
  unsigned long base_volume;
};

struct SampleTable { // sizeof = 16
  unsigned long file_pos;
  unsigned long data_size;
  unsigned long sfxid;
  struct HeapMgrHandle *hmhandle;
};

/** Sound bank ID. */
typedef unsigned char SoundBankID;
/** Sound SFXID parameter from bank table. */
typedef unsigned char SoundSFXID;
/** Sound emitter ID. */
typedef long SoundEmitterID;
/** Sound sample ID in bank table. */
typedef short SoundSmplTblID;
/** Volume level indicator, normal is 256. */
typedef long SoundVolume;
/** Pitch level indicator, normal is 100. */
typedef long SoundPitch;

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
DLLIMPORT extern long _DK_MaxNoSounds;
#define MaxNoSounds _DK_MaxNoSounds
DLLIMPORT extern struct S3DSample _DK_SampleList[SOUNDS_MAX_COUNT];
#define SampleList _DK_SampleList
DLLIMPORT TbFileHandle _DK_sound_file;
#define sound_file _DK_sound_file
DLLIMPORT TbFileHandle _DK_sound_file2;
#define sound_file2 _DK_sound_file2
DLLIMPORT unsigned char _DK_using_two_banks;
#define using_two_banks _DK_using_two_banks
DLLIMPORT long _DK_samples_in_bank;
#define samples_in_bank _DK_samples_in_bank
DLLIMPORT struct SampleTable *_DK_sample_table;
#define sample_table _DK_sample_table
DLLIMPORT long _DK_samples_in_bank2;
#define samples_in_bank2 _DK_samples_in_bank2
DLLIMPORT struct SampleTable *_DK_sample_table2;
#define sample_table2 _DK_sample_table2
DLLIMPORT struct HeapMgrHeader *_DK_sndheap;
#define sndheap _DK_sndheap
DLLIMPORT S3D_LineOfSight_Func _DK_LineOfSightFunction;
#define LineOfSightFunction _DK_LineOfSightFunction
DLLIMPORT long _DK_deadzone_radius;
#define deadzone_radius _DK_deadzone_radius

#pragma pack()
/******************************************************************************/
// Exported functions
long S3DSetSoundReceiverPosition(int pos_x, int pos_y, int pos_z);
long S3DSetSoundReceiverOrientation(int ori_a, int ori_b, int ori_c);
void S3DSetSoundReceiverFlags(unsigned long nflags);
void S3DSetSoundReceiverSensitivity(unsigned short nsensivity);
long S3DDestroySoundEmitter(SoundEmitterID eidx);
TbBool S3DEmitterHasFinishedPlaying(SoundEmitterID eidx);
TbBool S3DMoveSoundEmitterTo(SoundEmitterID eidx, long x, long y, long z);
long S3DInit(void);
long S3DSetNumberOfSounds(long nMaxSounds);
long S3DSetMaximumSoundDistance(long nDistance);
TbBool S3DAddSampleToEmitterPri(SoundEmitterID eidx, SoundSmplTblID smptbl_id, SoundBankID bank_id, SoundPitch pitch, SoundVolume loudness, long a6, char a7, long a8, long a9);
long S3DCreateSoundEmitterPri(long x, long y, long z, SoundSmplTblID smptbl_id, SoundBankID bank_id, SoundPitch pitch, SoundVolume loudness, long a8, long a9, long a10);
TbBool S3DEmitterIsAllocated(SoundEmitterID eidx);
TbBool S3DEmitterIsPlayingAnySample(SoundEmitterID eidx);
TbBool S3DEmitterIsPlayingSample(SoundEmitterID eidx, long smpl_idx, long a2);
TbBool S3DDeleteSampleFromEmitter(SoundEmitterID eidx, long smpl_idx, long bank_id);
TbBool S3DDeleteAllSamplesFromEmitter(SoundEmitterID eidx);
TbBool S3DDestroySoundEmitterAndSamples(SoundEmitterID eidx);
void S3DSetLineOfSightFunction(S3D_LineOfSight_Func);
void S3DSetDeadzoneRadius(long dzradius);
long S3DGetDeadzoneRadius(void);

void play_non_3d_sample(long sample_idx);
void play_non_3d_sample_no_overlap(long smpl_idx);
void play_atmos_sound(long smpl_idx);
short sound_emitter_in_use(SoundEmitterID eidx);
long get_best_sound_heap_size(long sh_mem_size);
struct SampleInfo *play_sample_using_heap(unsigned long a1, short a2, unsigned long a3, unsigned long a4, unsigned long a5, char a6, unsigned char a7, SoundBankID bank_id);
void stop_sample_using_heap(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundBankID bank_id);
long speech_sample_playing(void);
long play_speech_sample(SoundSmplTblID smptbl_id);
void close_sound_heap(void);
void close_sound_bank(SoundBankID bank_id);
long stop_emitter_samples(struct SoundEmitter *emit);
TbBool process_sound_emitters(void);
void increment_sample_times(void);
TbBool process_sound_samples(void);

extern int atmos_sound_volume;

struct SoundEmitter* S3DGetSoundEmitter(SoundEmitterID eidx);
SoundEmitterID get_emitter_id(struct SoundEmitter *emit);
void kick_out_sample(short smpl_id);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
