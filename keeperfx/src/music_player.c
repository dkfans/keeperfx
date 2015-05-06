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
#include "music_player.h"

#include "globals.h"
#include "bflib_sndlib.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MAX_TRACK 8

Mix_Music* tracks[MAX_TRACK];
int current_track;
/******************************************************************************/

int IsRedbookMusicActive(void)
{
    return (game.flags_cd & MFlg_NoCdMusic) == 0;
}

int InitializeMusicPlayer(void)
{
    if (IsRedbookMusicActive())
    {
        return true;
    }

    current_track = -1;
    int initted = Mix_Init(MIX_INIT_OGG);
    if((initted & MIX_INIT_OGG) == MIX_INIT_OGG)
    {
        if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) >= 0)
        {
            tracks[0] = NULL;
            int i;
            // There is no keeper01.ogg.
            for (i = 2; i < MAX_TRACK; i++)
            {
                const char *fname;
                fname = prepare_file_fmtpath(FGrp_Music, "keeper%02d.ogg", i);
                tracks[i] = Mix_LoadMUS(fname);
                if (tracks[i] == NULL)
                {
                    WARNLOG("Can't load track %d: %s", i, Mix_GetError());
                }
            }
        }
        else
        {
            SYNCLOG("Can't open music device: %s", Mix_GetError());
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

    while(Mix_Init(0))
    {
        Mix_Quit();
    }
    Mix_HaltMusic();
    int i;
    for (i = 0; i < MAX_TRACK; i++)
    {
        if (tracks[i] != NULL)
        {
            Mix_FreeMusic(tracks[i]);
        }
    }
    Mix_CloseAudio();
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
        SetRedbookVolume(lbAppActive ? volume : 0);
    } else
    {
        float volume_f = (float) volume;
        int normalized_volume = (int)((volume_f / MIX_MAX_VOLUME) * MIX_MAX_VOLUME);
        Mix_VolumeMusic(lbAppActive ? normalized_volume : 0);
        // SYNCLOG("Music volume set: %d", normalized_volume);
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
