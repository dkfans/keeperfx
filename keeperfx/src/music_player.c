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

    SYNCLOG("Music player using folder not supported.");
    return true;
}

void ShutdownMusicPlayer(void)
{
    if (IsRedbookMusicActive())
    {
        return;
    }
}

void PlayMusicPlayer(int track)
{
    if (IsRedbookMusicActive())
    {
        PlayRedbookTrack(track);
    } else
    {
        SYNCLOG("Music player using folder not supported.");
    }
}

void StopMusicPlayer(void)
{
    if (IsRedbookMusicActive())
    {
        StopRedbookTrack();
    } else
    {
        SYNCLOG("Music player using folder not supported.");
    }
}

void SetMusicPlayerVolume(int volume)
{
    if (IsRedbookMusicActive())
    {
        SetRedbookVolume(volume);
    } else
    {
        SYNCLOG("Music player using folder not supported.");
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
