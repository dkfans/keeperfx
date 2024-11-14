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

    /**
     * Initialize the Miles Sound System library.
     *
     * When sounds have been disabled by the user, the library will not be loaded.
     * The seperate functions can still be called and will silently return 0 in that case.
     *
     * If the library is successfuly loaded it's possible that the functions have not.
     * In that case it will simply log an error and continue execution.
     *
     * @returns -1 when sound is disabled, 0 if the library failed to load, 1 if the library is loaded
     */
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

    void FreeAudio(void)
    {
        if (_FreeAudio != NULL)
        {
            _FreeAudio();
        }
    }

    void SetRedbookVolume(SoundVolume volume)
    {
        if (_SetRedbookVolume != NULL)
        {
            ((FARPROCI)_SetRedbookVolume)(volume);
        }
    }

    void SetSoundMasterVolume(SoundVolume volume)
    {
        if (_SetSoundMasterVolume != NULL)
        {
            ((FARPROCI)_SetSoundMasterVolume)(volume);
        }
    }

    void SetMusicMasterVolume(SoundVolume volume)
    {
        if (_SetMusicMasterVolume != NULL)
        {
            ((FARPROCI)_SetMusicMasterVolume)(volume);
        }
    }

    TbBool GetSoundInstalled(void)
    {
        if (_GetSoundInstalled != NULL)
        {
            return _GetSoundInstalled();
        }

        return 0;
    }

    void PlayRedbookTrack(int track)
    {
        if (_PlayRedbookTrack != NULL)
        {
            ((FARPROCI)_PlayRedbookTrack)(track);
        }
    }

    void PauseRedbookTrack(void)
    {
        if (_PauseRedbookTrack != NULL)
        {
            _PauseRedbookTrack();
        }
    }

    void ResumeRedbookTrack(void)
    {
        if (_ResumeRedbookTrack != NULL)
        {
            _ResumeRedbookTrack();
        }
    }

    void MonitorStreamedSoundTrack(void)
    {
        if (_MonitorStreamedSoundTrack != NULL)
        {
            _MonitorStreamedSoundTrack();
        }
    }

    void StopRedbookTrack(void)
    {
        if (_StopRedbookTrack != NULL)
        {
            _StopRedbookTrack();
        }
    }

    void *GetSoundDriver(void)
    {
        if (_GetSoundDriver != NULL)
        {
            return (void *)_GetSoundDriver();
        }

        return 0;
    }

    void StopAllSamples(void)
    {
        if (_StopAllSamples != NULL)
        {
            _StopAllSamples();
        }
    }

    struct SampleInfo *GetFirstSampleInfoStructure(void)
    {
        if (_GetFirstSampleInfoStructure != NULL)
        {
            return (struct SampleInfo *)_GetFirstSampleInfoStructure();
        }

        return 0;
    }

    TbBool InitAudio(struct SoundSettings * settings)
    {
        if (_InitAudio != NULL)
        {
            return ((FARPROCP)_InitAudio)(settings);
        }

        return false;
    }

    void SetupAudioOptionDefaults(struct SoundSettings * settings)
    {
        if (_SetupAudioOptionDefaults != NULL)
        {
            ((FARPROCP)_SetupAudioOptionDefaults)(settings);
        }
    }

    TbBool IsSamplePlaying(SoundMilesID a3)
    {
        if (_IsSamplePlaying != NULL)
        {
            return (unsigned char)((FARPROCIII)_IsSamplePlaying)(0, 0, a3);
        }

        return false;
    }

    struct SampleInfo *GetLastSampleInfoStructure(void)
    {
        if (_GetLastSampleInfoStructure != NULL)
        {
            return (struct SampleInfo *)_GetLastSampleInfoStructure();
        }

        return NULL;
    }

    SoundVolume GetCurrentSoundMasterVolume(void)
    {
        if (_GetCurrentSoundMasterVolume != NULL)
        {
            return _GetCurrentSoundMasterVolume();
        }

        return 0;
    }

    void StopSample(SoundEmitterID emit_id, SoundSmplTblID smptbl_id)
    {
        if (_StopSample != NULL)
        {
            ((FARPROCII)_StopSample)(emit_id, smptbl_id);
        }
    }

    void SetSampleVolume(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundVolume volume)
    {
        if (_SetSampleVolume != NULL)
        {
            ((FARPROCIIII)_SetSampleVolume)(emit_id, smptbl_id, volume, 0);
        }
    }

    void SetSamplePan(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPan pan)
    {
        if (_SetSamplePan != NULL)
        {
            ((FARPROCIIII)_SetSamplePan)(emit_id, smptbl_id, pan, 0);
        }
    }

    void SetSamplePitch(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPitch pitch)
    {
        if (_SetSamplePitch != NULL)
        {
            ((FARPROCIIII)_SetSamplePitch)(emit_id, smptbl_id, pitch, 0);
        }
    }

    struct SampleInfo *PlaySampleFromAddress(SoundEmitterID emit_id, SoundSmplTblID smpl_idx, SoundVolume volume, SoundPan pan, SoundPitch pitch, unsigned char a6, unsigned char a7, void *buf, SoundSFXID sfxid)
    {
        if (_PlaySampleFromAddress != NULL)
        {
            return ((FARPROC_PLAY1)(void *)_PlaySampleFromAddress)(emit_id, smpl_idx, volume, pan, pitch, a6, a7, buf, sfxid);
        }

        return NULL;
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
