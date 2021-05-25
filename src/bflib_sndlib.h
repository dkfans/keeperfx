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
    unsigned short field_12;
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

int __stdcall FreeAudio(void);
int __stdcall SetRedbookVolume(int volume);
int __stdcall SetSoundMasterVolume(int volume);
int __stdcall SetMusicMasterVolume(int volume);
int __stdcall GetSoundInstalled(void);
int __stdcall PlayRedbookTrack(int);
int __stdcall PauseRedbookTrack(void);
int __stdcall ResumeRedbookTrack(void);
int __stdcall MonitorStreamedSoundTrack(void);
int __stdcall StopRedbookTrack(void);
void * __stdcall GetSoundDriver(void);
int __stdcall StopAllSamples(void);
struct SampleInfo * __stdcall GetFirstSampleInfoStructure(void);
int __stdcall InitAudio(void *);
int __stdcall SetupAudioOptionDefaults(void *);
int __stdcall PlayStreamedSample(char *fname, int a2, int a3, int a4);
int __stdcall IsSamplePlaying(int a1, int a2, int a3);
int __stdcall StopStreamedSample(void);
int __stdcall StreamedSampleFinished(void);
int __stdcall SetStreamedSampleVolume(int);
struct SampleInfo * __stdcall GetLastSampleInfoStructure(void);
int __stdcall GetCurrentSoundMasterVolume(void);
int __stdcall StopSample(int a,int b);
int __stdcall SetSampleVolume(int a,int b,int c,int d);
int __stdcall SetSamplePan(int a,int b,int c,int d);
int __stdcall SetSamplePitch(int a,int b,int c,int d);
struct SampleInfo * __stdcall PlaySampleFromAddress(int a1, int smpl_idx, int a3, int a4, int a5, unsigned char a6, unsigned char a7, void * buf, int a9);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
