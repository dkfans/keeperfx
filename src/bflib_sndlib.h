/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sndlib.h
 *     Header file for bflib_sndlib.c.
 * @par Purpose:
 *     Low-level sound and music related routines.
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
#ifndef BFLIB_SNDLIB_H
#define BFLIB_SNDLIB_H

#include "bflib_basics.h"
#include "bflib_sound.h"

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

// Data structures

struct HeapMgrHeader;
struct HeapMgrHandle;


struct SampleInfo { // sizeof = 29
    long field_0;
  unsigned char field_4[4];
  unsigned char field_8;
  unsigned char field_9[9];
  SoundSmplTblID smptbl_id;
  unsigned char field_14[3];
  unsigned char flags_17;
  unsigned long field_18;
  unsigned char field_1C;
};

#pragma pack()
/******************************************************************************/
// Exported variables

/******************************************************************************/
// Exported functions

int FreeAudio(void);
int SetRedbookVolume(int volume);
int SetSoundMasterVolume(int volume);
int SetMusicMasterVolume(int volume);
int GetSoundInstalled(void);
int PlayRedbookTrack(int);
int PauseRedbookTrack(void);
int ResumeRedbookTrack(void);
int MonitorStreamedSoundTrack(void);
int StopRedbookTrack(void);
void * GetSoundDriver(void);
int StopAllSamples(void);
struct SampleInfo * GetFirstSampleInfoStructure(void);
int InitAudio(void *);
int SetupAudioOptionDefaults(void *);
int PlayStreamedSample(char *fname, int sample_type, int a3, int a4);
int IsSamplePlaying(int a1, int a2, int a3);
int StopStreamedSample(void);
int StreamedSampleFinished(void);
int SetStreamedSampleVolume(int);
struct SampleInfo * GetLastSampleInfoStructure(void);
int GetCurrentSoundMasterVolume(void);
int StopSample(SoundEmitterID emit_id, long smptbl_id);
int SetSampleVolume(SoundEmitterID emit_id, long smptbl_id,long volume,long d);
int SetSamplePan(SoundEmitterID emit_id, long smptbl_id,long pan,int d);
int SetSamplePitch(SoundEmitterID emit_id, long smptbl_id,long pitch,int d);
struct SampleInfo * PlaySampleFromAddress(SoundEmitterID emit_id, int smpl_idx, int a3, int a4, int a5, unsigned char a6, unsigned char a7, void * buf, int sfxid);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
