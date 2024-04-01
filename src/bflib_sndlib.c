/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sndlib.c
 *     Low-level sound and music related routines.
 * @par Purpose:
 *     Hardware wrapper to play music and sound in games.
 * @par Comment:
 *     Windows version os Bullfrog Engine uses Miles Sound System, wrapped
 *     with WSND7R.DLL. This library contains definitions of the exported functions.
 * @author   KeeperFX Team
 * @date     16 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_sndlib.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "bflib_basics.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C"
{
#endif
    /******************************************************************************/
    // Global variables

    typedef int(WINAPI *FARPROCI)(int);
    typedef int(WINAPI *FARPROCII)(int, int);
    typedef int(WINAPI *FARPROCIII)(int, int, int);
    typedef int(WINAPI *FARPROCS)(const char *);
    typedef int(WINAPI *FARPROCP)(const void *);
    typedef int(WINAPI *FARPROCSIII)(const char *, int, int, int);
    typedef int(WINAPI *FARPROCIIII)(int, int, int, int);
    typedef struct SampleInfo *(WINAPI *FARPROC_PLAY1)(int, int, int, int, int, unsigned char, unsigned char, void *, int);

    HMODULE WSND7R;

    FARPROC _FreeAudio;
    FARPROC _SetRedbookVolume;
    FARPROC _SetSoundMasterVolume;
    FARPROC _SetMusicMasterVolume;
    FARPROC _GetSoundInstalled;
    FARPROC _PlayRedbookTrack;
    FARPROC _PauseRedbookTrack;
    FARPROC _ResumeRedbookTrack;
    FARPROC _MonitorStreamedSoundTrack;
    FARPROC _StopRedbookTrack;
    FARPROC _GetSoundDriver;
    FARPROC _StopAllSamples;
    FARPROC _GetFirstSampleInfoStructure;
    FARPROC _InitAudio;
    FARPROC _SetupAudioOptionDefaults;
    FARPROC _IsSamplePlaying;
    FARPROC _GetLastSampleInfoStructure;
    FARPROC _GetCurrentSoundMasterVolume;
    FARPROC _StopSample;
    FARPROC _SetSampleVolume;
    FARPROC _SetSamplePan;
    FARPROC _SetSamplePitch;
    FARPROC _PlaySampleFromAddress;

    /******************************************************************************/
    // Functions

    // GetModuleHandleExA(0,"WSND7R",&hModule); seems not to work

    int init_miles_sound_system()
    {
        if (SoundDisabled == true)
        {
            return -1;
        }

        WSND7R = LoadLibrary("WSND7R");

        if (WSND7R == NULL)
        {
            ERRORLOG("Failed to load Miles Sound System");
            return 0;
        }

        _FreeAudio = GetProcAddress(WSND7R, "_FreeAudio@0");
        if (_FreeAudio == NULL)
        {
            ERRORLOG("Can't get address of FreeAudio function; skipped.");
        }

        _SetRedbookVolume = GetProcAddress(WSND7R, "_SetRedbookVolume@4");
        if (_SetRedbookVolume == NULL)
        {
            ERRORLOG("Can't get address of SetRedbookVolume function; skipped.");
        }

        _SetSoundMasterVolume = GetProcAddress(WSND7R, "_SetSoundMasterVolume@4");
        if (_SetSoundMasterVolume == NULL)
        {
            ERRORLOG("Can't get address of SetSoundMasterVolume function; skipped.");
        }

        _SetMusicMasterVolume = GetProcAddress(WSND7R, "_SetMusicMasterVolume@4");
        if (_SetMusicMasterVolume == NULL)
        {
            ERRORLOG("Can't get address of SetMusicMasterVolume function; skipped.");
        }

        _GetSoundInstalled = GetProcAddress(WSND7R, "_GetSoundInstalled@0");
        if (_GetSoundInstalled == NULL)
        {
            ERRORLOG("Can't get address of GetSoundInstalled function; skipped.");
        }

        _PlayRedbookTrack = GetProcAddress(WSND7R, "_PlayRedbookTrack@4");
        if (_PlayRedbookTrack == NULL)
        {
            ERRORLOG("Can't get address of PlayRedbookTrack function; skipped.");
        }

        _PauseRedbookTrack = GetProcAddress(WSND7R, "_PauseRedbookTrack@0");
        if (_PauseRedbookTrack == NULL)
        {
            ERRORLOG("Can't get address of PauseRedbookTrack function; skipped.");
        }

        _ResumeRedbookTrack = GetProcAddress(WSND7R, "_ResumeRedbookTrack@0");
        if (_ResumeRedbookTrack == NULL)
        {
            ERRORLOG("Can't get address of ResumeRedbookTrack function; skipped.");
        }

        _MonitorStreamedSoundTrack = GetProcAddress(WSND7R, "_MonitorStreamedSoundTrack@0");
        if (_MonitorStreamedSoundTrack == NULL)
        {
            ERRORLOG("Can't get address of MonitorStreamedSoundTrack function; skipped.");
        }

        _StopRedbookTrack = GetProcAddress(WSND7R, "_StopRedbookTrack@0");
        if (_StopRedbookTrack == NULL)
        {
            ERRORLOG("Can't get address of StopRedbookTrack function; skipped.");
        }

        _GetSoundDriver = GetProcAddress(WSND7R, "_GetSoundDriver@0");
        if (_GetSoundDriver == NULL)
        {
            ERRORLOG("Can't get address of GetSoundDriver function; skipped.");
        }

        _StopAllSamples = GetProcAddress(WSND7R, "_StopAllSamples@0");
        if (_StopAllSamples == NULL)
        {
            ERRORLOG("Can't get address of StopAllSamples function; skipped.");
        }

        _GetFirstSampleInfoStructure = GetProcAddress(WSND7R, "_GetFirstSampleInfoStructure@0");
        if (_GetFirstSampleInfoStructure == NULL)
        {
            ERRORLOG("Can't get address of GetFirstSampleInfoStructure function; skipped.");
        }

        _InitAudio = GetProcAddress(WSND7R, "_InitAudio@4");
        if (_InitAudio == NULL)
        {
            ERRORLOG("Can't get address of InitAudio function; skipped.");
        }

        _SetupAudioOptionDefaults = GetProcAddress(WSND7R, "_SetupAudioOptionDefaults@4");
        if (_SetupAudioOptionDefaults == NULL)
        {
            ERRORLOG("Can't get address of SetupAudioOptionDefaults function; skipped.");
        }

        _IsSamplePlaying = GetProcAddress(WSND7R, "_IsSamplePlaying@12");
        if (_IsSamplePlaying == NULL)
        {
            ERRORLOG("Can't get address of IsSamplePlaying function; skipped.");
        }

        _GetLastSampleInfoStructure = GetProcAddress(WSND7R, "_GetLastSampleInfoStructure@0");
        if (_GetLastSampleInfoStructure == NULL)
        {
            ERRORLOG("Can't get address of GetLastSampleInfoStructure function; skipped.");
        }

        _GetCurrentSoundMasterVolume = GetProcAddress(WSND7R, "_GetCurrentSoundMasterVolume@0");
        if (_GetCurrentSoundMasterVolume == NULL)
        {
            ERRORLOG("Can't get address of GetCurrentSoundMasterVolume function; skipped.");
        }

        _StopSample = GetProcAddress(WSND7R, "_StopSample@8");
        if (_StopSample == NULL)
        {
            ERRORLOG("Can't get address of StopSample function; skipped.");
        }

        _SetSampleVolume = GetProcAddress(WSND7R, "_SetSampleVolume@16");
        if (_SetSampleVolume == NULL)
        {
            ERRORLOG("Can't get address of SetSampleVolume function; skipped.");
        }

        _SetSamplePan = GetProcAddress(WSND7R, "_SetSamplePan@16");
        if (_SetSamplePan == NULL)
        {
            ERRORLOG("Can't get address of SetSamplePan function; skipped.");
        }

        _SetSamplePitch = GetProcAddress(WSND7R, "_SetSamplePitch@16");
        if (_SetSamplePitch == NULL)
        {
            ERRORLOG("Can't get address of SetSamplePitch function; skipped.");
        }

        _PlaySampleFromAddress = GetProcAddress(WSND7R, "_PlaySampleFromAddress@36");
        if (_PlaySampleFromAddress == NULL)
        {
            ERRORLOG("Can't get address of PlaySampleFromAddress function; skipped.");
        }

        return 1;
    }

    int FreeAudio(void)
    {
        if (_FreeAudio != NULL)
        {
            return _FreeAudio();
        }

        return 0;
    }

    int SetRedbookVolume(int volume)
    {
        if (_SetRedbookVolume != NULL)
        {
            return ((FARPROCI)_SetRedbookVolume)(volume);
        }

        return 0;
    }

    int SetSoundMasterVolume(int volume)
    {
        if (_SetSoundMasterVolume != NULL)
        {
            return ((FARPROCI)_SetSoundMasterVolume)(volume);
        }

        return 0;
    }

    int SetMusicMasterVolume(int volume)
    {
        if (_SetMusicMasterVolume != NULL)
        {
            return ((FARPROCI)_SetMusicMasterVolume)(volume);
        }

        return 0;
    }

    int GetSoundInstalled(void)
    {
        if (_GetSoundInstalled != NULL)
        {
            return _GetSoundInstalled();
        }

        return 0;
    }

    int PlayRedbookTrack(int track)
    {
        if (_PlayRedbookTrack != NULL)
        {
            return ((FARPROCI)_PlayRedbookTrack)(track);
        }

        return 0;
    }

    int PauseRedbookTrack(void)
    {
        if (_PauseRedbookTrack != NULL)
        {
            return _PauseRedbookTrack();
        }

        return 0;
    }

    int ResumeRedbookTrack(void)
    {
        if (_ResumeRedbookTrack != NULL)
        {
            return _ResumeRedbookTrack();
        }

        return 0;
    }

    int MonitorStreamedSoundTrack(void)
    {
        if (_MonitorStreamedSoundTrack != NULL)
        {
            return _MonitorStreamedSoundTrack();
        }

        return 0;
    }

    int StopRedbookTrack(void)
    {
        if (_StopRedbookTrack != NULL)
        {
            return _StopRedbookTrack();
        }

        return 0;
    }

    void *GetSoundDriver(void)
    {
        if (_GetSoundDriver != NULL)
        {
            return (void *)_GetSoundDriver();
        }

        return 0;
    }

    int StopAllSamples(void)
    {
        if (_StopAllSamples != NULL)
        {
            return _StopAllSamples();
        }

        return 0;
    }

    struct SampleInfo *GetFirstSampleInfoStructure(void)
    {
        if (_GetFirstSampleInfoStructure != NULL)
        {
            return (struct SampleInfo *)_GetFirstSampleInfoStructure();
        }

        return 0;
    }

    int InitAudio(void *i)
    {
        if (_InitAudio != NULL)
        {
            return ((FARPROCP)_InitAudio)(i);
        }

        return 0;
    }

    int SetupAudioOptionDefaults(void *i)
    {
        if (_SetupAudioOptionDefaults != NULL)
        {
            return ((FARPROCP)_SetupAudioOptionDefaults)(i);
        }

        return 0;
    }

    int IsSamplePlaying(int a1, int a2, int a3)
    {
        if (_IsSamplePlaying != NULL)
        {
            return (unsigned char)((FARPROCIII)_IsSamplePlaying)(a1, a2, a3);
        }

        return 0;
    }

    struct SampleInfo *GetLastSampleInfoStructure(void)
    {
        if (_GetLastSampleInfoStructure != NULL)
        {
            return (struct SampleInfo *)_GetLastSampleInfoStructure();
        }

        return 0;
    }

    int GetCurrentSoundMasterVolume(void)
    {
        if (_GetCurrentSoundMasterVolume != NULL)
        {
            return _GetCurrentSoundMasterVolume();
        }

        return 0;
    }

    int StopSample(SoundEmitterID emit_id, long smptbl_id)
    {
        if (_StopSample != NULL)
        {
            return ((FARPROCII)_StopSample)(emit_id, smptbl_id);
        }

        return 0;
    }

    int SetSampleVolume(SoundEmitterID emit_id, long smptbl_id, long volume, long d)
    {
        if (_SetSampleVolume != NULL)
        {
            return ((FARPROCIIII)_SetSampleVolume)(emit_id, smptbl_id, volume, d);
        }

        return 0;
    }

    int SetSamplePan(SoundEmitterID emit_id, long smptbl_id, long pan, int d)
    {
        if (_SetSamplePan != NULL)
        {
            return ((FARPROCIIII)_SetSamplePan)(emit_id, smptbl_id, pan, d);
        }

        return 0;
    }

    int SetSamplePitch(SoundEmitterID emit_id, long smptbl_id, long pitch, int d)
    {
        if (_SetSamplePitch != NULL)
        {
            return ((FARPROCIIII)_SetSamplePitch)(emit_id, smptbl_id, pitch, d);
        }

        return 0;
    }

    struct SampleInfo *PlaySampleFromAddress(SoundEmitterID emit_id, int smpl_idx, int a3, int a4, int a5, unsigned char a6, unsigned char a7, void *buf, int sfxid)
    {
        if (_PlaySampleFromAddress != NULL)
        {
            return ((FARPROC_PLAY1)(void *)_PlaySampleFromAddress)(emit_id, smpl_idx, a3, a4, a5, a6, a7, buf, sfxid);
        }

        return 0;
    }

    void unload_miles_sound_system()
    {
        if (WSND7R != NULL)
        {
            FreeLibrary(WSND7R);
            WSND7R = NULL;
        }

        _FreeAudio = NULL;
        _SetRedbookVolume = NULL;
        _SetSoundMasterVolume = NULL;
        _SetMusicMasterVolume = NULL;
        _GetSoundInstalled = NULL;
        _PlayRedbookTrack = NULL;
        _PauseRedbookTrack = NULL;
        _ResumeRedbookTrack = NULL;
        _MonitorStreamedSoundTrack = NULL;
        _StopRedbookTrack = NULL;
        _GetSoundDriver = NULL;
        _StopAllSamples = NULL;
        _GetFirstSampleInfoStructure = NULL;
        _InitAudio = NULL;
        _SetupAudioOptionDefaults = NULL;
        _IsSamplePlaying = NULL;
        _GetLastSampleInfoStructure = NULL;
        _GetCurrentSoundMasterVolume = NULL;
        _StopSample = NULL;
        _SetSampleVolume = NULL;
        _SetSamplePan = NULL;
        _SetSamplePitch = NULL;
        _PlaySampleFromAddress = NULL;
    }

/******************************************************************************/
#ifdef __cplusplus
}
#endif
