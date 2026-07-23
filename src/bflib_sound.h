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

/** Sound SFXID parameter from bank table. */
typedef unsigned char SoundSFXID;
/** Sound emitter ID. */
typedef int32_t SoundEmitterID;
/** Sound sample ID in bank table. */
typedef short SoundSmplTblID;
/** Volume level indicator, normal is 256. */
typedef int32_t SoundVolume;
/** Pitch level indicator, normal is 100. */
typedef int32_t SoundPitch;
/** Pan level indicator. */
typedef int32_t SoundPan;
/** Miles Sound ID. */
typedef int32_t SoundMilesID;

enum SoundEmitterFlags {
    Emi_IsAllocated  = 0x01,
    Emi_IsPlaying    = 0x02,
    Emi_IsMoving     = 0x04,
};

enum SoundSampleFlags {
    Smp_NoPitchUpdate  = 0x01,
    Smp_NoVolumeUpdate = 0x02,
};

typedef void *SndData;
typedef int32_t (*S3D_LineOfSight_Func)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);

struct SoundCoord3d {
    uint32_t val_x;
    uint32_t val_y;
    uint32_t val_z;
};

struct SoundEmitter {
    unsigned char flags;
    unsigned char emitter_flags;
    short index;
    struct SoundCoord3d pos;
    unsigned char reserved[6];
    int32_t pitch_doppler;
    unsigned char curr_pitch;
    unsigned char target_pitch;
};

struct SoundReceiver { // sizeof = 17
    struct SoundCoord3d pos;
    unsigned short rotation_angle_x;
    unsigned short rotation_angle_y;
    unsigned short rotation_angle_z;
    uint32_t flags;
    unsigned char sensivity;
};

struct S3DSample { // sizeof = 37
  uint32_t priority;
  uint32_t time_turn;
  unsigned short smptbl_id;
  unsigned short base_pitch;
  unsigned short pan;
  unsigned short volume;
  SoundMilesID mss_id;
  struct SoundEmitter *emit_ptr;
  int32_t emit_idx;
  char repeat_count; // signed
  unsigned char flags;
  unsigned char is_playing;
  unsigned char sfxid;
  uint32_t base_volume;
};

/******************************************************************************/
// Exported variables
extern int atmos_sound_volume;
extern TbBool SoundDisabled;
extern int32_t MaxSoundDistance;
extern struct SoundReceiver Receiver;
extern int32_t Non3DEmitter;
extern int32_t SpeechEmitter;
#pragma pack()
/******************************************************************************/
// Exported functions
int32_t S3DSetSoundReceiverPosition(int pos_x, int pos_y, int pos_z);
int32_t S3DSetSoundReceiverOrientation(int ori_a, int ori_b, int ori_c);
void S3DSetSoundReceiverSensitivity(unsigned short nsensivity);
int32_t S3DDestroySoundEmitter(SoundEmitterID);
TbBool S3DEmitterHasFinishedPlaying(SoundEmitterID);
TbBool S3DMoveSoundEmitterTo(SoundEmitterID, int32_t x, int32_t y, int32_t z);
int32_t S3DInit(void);
int32_t S3DSetNumberOfSounds(int32_t nMaxSounds);
int32_t S3DSetMaximumSoundDistance(int32_t nDistance);
TbBool S3DAddSampleToEmitterPri(SoundEmitterID, SoundSmplTblID, SoundPitch, SoundVolume, int32_t repeats, char ctype, int32_t flags, int32_t priority);
int32_t S3DCreateSoundEmitterPri(int32_t x, int32_t y, int32_t z, SoundSmplTblID, SoundPitch, SoundVolume, int32_t repeats, int32_t flags, int32_t priority);
TbBool S3DEmitterIsAllocated(SoundEmitterID);
TbBool S3DEmitterIsPlayingAnySample(SoundEmitterID);
TbBool S3DEmitterIsPlayingSample(SoundEmitterID, SoundSmplTblID);
TbBool S3DDeleteSampleFromEmitter(SoundEmitterID, SoundSmplTblID);
TbBool S3DDeleteAllSamplesFromEmitter(SoundEmitterID);
TbBool S3DDestroySoundEmitterAndSamples(SoundEmitterID);
void S3DSetLineOfSightFunction(S3D_LineOfSight_Func);
void S3DSetDeadzoneRadius(int32_t dzradius);

void play_non_3d_sample(SoundSmplTblID);
void play_non_3d_sample_no_overlap(SoundSmplTblID);
void play_atmos_sound(SoundSmplTblID);
short sound_emitter_in_use(SoundEmitterID);
SoundMilesID play_sample(SoundEmitterID, SoundSmplTblID, SoundVolume, SoundPan, SoundPitch, char repeats, unsigned char ctype);
void stop_sample(SoundEmitterID, SoundSmplTblID);
int32_t speech_sample_playing(void);
int32_t play_speech_sample(SoundSmplTblID);
int32_t stop_emitter_samples(struct SoundEmitter *emit);
TbBool process_sound_emitters(void);
void increment_sample_times(void);
TbBool process_sound_samples(void);
void stop_atmos_sounds(void);

struct SoundEmitter* S3DGetSoundEmitter(SoundEmitterID);
SoundEmitterID get_emitter_id(struct SoundEmitter *);
void kick_out_sample(SoundSmplTblID);
SoundSFXID get_sample_sfxid(SoundSmplTblID smptbl_id);
SoundSmplTblID get_speech_offset(void);
SoundSmplTblID get_custom_offset(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
