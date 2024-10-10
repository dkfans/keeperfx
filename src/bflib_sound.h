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

// Type definitions

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
/** Pan level indicator. */
typedef long SoundPan;
/** Miles Sound ID. */
typedef long SoundMilesID;

enum SoundEmitterFlags {
    Emi_IsAllocated  = 0x01,
    Emi_UnknownPlay  = 0x02,
    Emi_IsMoving     = 0x04,
};

enum SoundSampleFlags {
    Smp_Unknown01  = 0x01,
    Smp_Unknown02  = 0x02,
};

typedef void *SndData;
typedef long (*S3D_LineOfSight_Func)(long, long, long, long, long, long);

struct SoundCoord3d {
    unsigned long val_x;
    unsigned long val_y;
    unsigned long val_z;
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
  unsigned long priority;
  unsigned long time_turn;
  unsigned short smptbl_id;
  unsigned char bank_id;
  unsigned short base_pitch;
  unsigned short pan;
  unsigned short volume;
  SoundMilesID mss_id;
  struct SoundEmitter *emit_ptr;
  long emit_idx;
  char field_1D; // signed
  unsigned char flags;
  unsigned char is_playing;
  unsigned char sfxid;
  unsigned long base_volume;
};

/******************************************************************************/
// Exported variables
extern int atmos_sound_volume;
extern TbBool SoundDisabled;
extern long MaxSoundDistance;
extern struct SoundReceiver Receiver;
extern long Non3DEmitter;
extern long SpeechEmitter;
#pragma pack()
/******************************************************************************/
// Exported functions
long S3DSetSoundReceiverPosition(int pos_x, int pos_y, int pos_z);
long S3DSetSoundReceiverOrientation(int ori_a, int ori_b, int ori_c);
void S3DSetSoundReceiverFlags(unsigned long nflags);
void S3DSetSoundReceiverSensitivity(unsigned short nsensivity);
long S3DDestroySoundEmitter(SoundEmitterID);
TbBool S3DEmitterHasFinishedPlaying(SoundEmitterID);
TbBool S3DMoveSoundEmitterTo(SoundEmitterID, long x, long y, long z);
long S3DInit(void);
long S3DSetNumberOfSounds(long nMaxSounds);
long S3DSetMaximumSoundDistance(long nDistance);
TbBool S3DAddSampleToEmitterPri(SoundEmitterID, SoundSmplTblID, SoundBankID, SoundPitch, SoundVolume, long fil1D, char ctype, long flags, long priority);
long S3DCreateSoundEmitterPri(long x, long y, long z, SoundSmplTblID, SoundBankID, SoundPitch, SoundVolume, long fil1D, long flags, long priority);
TbBool S3DEmitterIsAllocated(SoundEmitterID);
TbBool S3DEmitterIsPlayingAnySample(SoundEmitterID);
TbBool S3DEmitterIsPlayingSample(SoundEmitterID, SoundSmplTblID, SoundBankID);
TbBool S3DDeleteSampleFromEmitter(SoundEmitterID, SoundSmplTblID, SoundBankID);
TbBool S3DDeleteAllSamplesFromEmitter(SoundEmitterID);
TbBool S3DDestroySoundEmitterAndSamples(SoundEmitterID);
void S3DSetLineOfSightFunction(S3D_LineOfSight_Func);
void S3DSetDeadzoneRadius(long dzradius);
long S3DGetDeadzoneRadius(void);

void play_non_3d_sample(SoundSmplTblID);
void play_non_3d_sample_no_overlap(SoundSmplTblID);
void play_atmos_sound(SoundSmplTblID);
short sound_emitter_in_use(SoundEmitterID);
SoundMilesID play_sample(SoundEmitterID, SoundSmplTblID, SoundVolume, SoundPan, SoundPitch, char fil1D, unsigned char ctype, SoundBankID);
void stop_sample(SoundEmitterID, SoundSmplTblID, SoundBankID);
long speech_sample_playing(void);
long play_speech_sample(SoundSmplTblID);
long stop_emitter_samples(struct SoundEmitter *emit);
TbBool process_sound_emitters(void);
void increment_sample_times(void);
TbBool process_sound_samples(void);

struct SoundEmitter* S3DGetSoundEmitter(SoundEmitterID);
SoundEmitterID get_emitter_id(struct SoundEmitter *);
void kick_out_sample(SoundSmplTblID);
SoundSFXID get_sample_sfxid(SoundSmplTblID smptbl_id, SoundBankID bank_id);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
