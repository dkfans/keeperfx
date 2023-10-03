/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file music_player.c
 *     ogg music player functions.
 * @par Purpose:
 *     plays ogg tracks in folder music
 * @par Comment:
 *     None.
 * @author   Lukas Niemeier
 * @date     20 Feb 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "music_player.h"

#include "globals.h"
#include "bflib_sndlib.h"

#include "game_legacy.h"
#include "keeperfx.hpp"
#include "config.h"
#include "sounds.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// the 50 is a static value, idealy would be equal to max_track. May not be smaller.
Mix_Music* tracks[MUSIC_TRACKS_COUNT];
int current_track;
/******************************************************************************/

TbBool IsRedbookMusicActive(void)
{
    return (features_enabled & Ft_NoCdMusic) == 0;
}

int InitializeMusicPlayer(void)
{
    if (IsRedbookMusicActive())
    {
        return true;
    }

    current_track = -1;
    if ((sdl_flags & MIX_INIT_OGG) == MIX_INIT_OGG)
    {
        tracks[0] = NULL;
        tracks[1] = NULL;
        // There is no keeper01.ogg. FIRST_TRACK defined as 2.
        for (int i = FIRST_TRACK; i <= max_track; i++)
        {
            const char* fname = prepare_file_fmtpath(FGrp_Music, "keeper%02d.ogg", i);
            tracks[i] = Mix_LoadMUS(fname);
            if (tracks[i] == NULL)
            {
                WARNLOG("Can't load track %d: %s", i, Mix_GetError());
            }
        }
    }
    else
    {
        SYNCLOG("Missing music types. %s", Mix_GetError());
    }
    SYNCLOG("Music player using folder initialized");
    return true;
}

void ShutdownMusicPlayer(void)
{
    if (IsRedbookMusicActive())
    {
        return;
    }
    Mix_HaltMusic();
    for (int i = 0; i < max_track; i++)
    {
        if (tracks[i] != NULL)
        {
            Mix_FreeMusic(tracks[i]);
        }
    }
}

void PlayMusicPlayer(int track)
{
    if (IsRedbookMusicActive())
    {
        PlayRedbookTrack(track);
    } else
    {
        if (track != current_track)
        {
            current_track = track;
            if(tracks[track] != NULL)
            {
                if(Mix_PlayMusic(tracks[track], -1) == -1)
                {
                    SYNCLOG("Can't play track %d: %s", track, Mix_GetError());
                }
            }
            else
            {
                SYNCLOG("This track was not loaded: %d", track);
            }
        }
    }
}

void PauseMusicPlayer(void)
{
    if (IsRedbookMusicActive())
    {
        PauseRedbookTrack();
    } else
    {
        Mix_PauseMusic();
    }
}

void ResumeMusicPlayer(void)
{
    if (IsRedbookMusicActive())
    {
        ResumeRedbookTrack();
    } else
    {
        Mix_ResumeMusic();
    }
}

void StopMusicPlayer(void)
{
    if (IsRedbookMusicActive())
    {
        StopRedbookTrack();
    } else
    {
        if(Mix_PlayingMusic())
        {
            if(Mix_FadeOutMusic(1000) == 1)
            {
                current_track = -1;
            }
            else
            {
                SYNCLOG("Can't stop music: %s", Mix_GetError());
            }
        }
    }
}

void SetMusicPlayerVolume(int volume)
{
    if (IsRedbookMusicActive())
    {
        SetRedbookVolume(volume);
    } else
    {
        float volume_f = (float) volume;
        int normalized_volume = (int)((volume_f / MIX_MAX_VOLUME) * MIX_MAX_VOLUME);
        int old_volume = Mix_VolumeMusic(-1);
        Mix_VolumeMusic(normalized_volume);
        if (normalized_volume != old_volume)
        {
            SYNCLOG("Music volume set: %d", normalized_volume);
        }
    }
}

void music_reinit_after_load()
{
    for (int i = (max_track + 1); i <= game.last_audiotrack; i++)
    {
        tracks[i] = Mix_LoadMUS(game.loaded_track[i]);
        if (tracks[i] == NULL)
        {
            WARNLOG("Can't load track %d: %s", i, Mix_GetError());
        }
    }  
}

void free_custom_music()
{
    for (int i = (max_track + 1); i < MUSIC_TRACKS_COUNT; i++)
    {
        Mix_Music *music = tracks[i];
        if (music != NULL)
        {
            Mix_FreeMusic(music);
            tracks[i] = NULL;
        }
    }
    game.last_audiotrack = max_track;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
