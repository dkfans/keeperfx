/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 16 Nov 2008

// Purpose:
//    Header file for bflib_sndlib.c.

// Comment:
//   Just a header file - #defines, typedefs, function prototypes etc.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#ifndef BFLIB_SNDLIB_H
#define BFLIB_SNDLIB_H

#include "bflib_basics.h"

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
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
int __stdcall MonitorStreamedSoundTrack(void);
int __stdcall StopRedbookTrack(void);
int __stdcall GetSoundDriver(void);
int __stdcall StopAllSamples(void);
int __stdcall GetFirstSampleInfoStructure(void);
int __stdcall LoadMusic(int);
int __stdcall InitAudio(int);
int __stdcall SetupAudioOptionDefaults(int);
int __stdcall StopStreamedSample(void);
int __stdcall StreamedSampleFinished(void);
int __stdcall SetStreamedSampleVolume(int);
int __stdcall GetLastSampleInfoStructure(void);
int __stdcall GetCurrentSoundMasterVolume(void);
int __stdcall StopMusic(void);
int __stdcall LoadAwe32Soundfont(int);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
