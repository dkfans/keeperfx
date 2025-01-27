/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file music_player.h
 *     Header file for music_player.c.
 * @par Purpose:
 *     ogg music player.
 * @par Comment:
 *     Uses SDL_mixer
 * @author   Lukas Niemeier
 * @date     20 Feb 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_MUSICPLAYER_H
#define DK_MUSICPLAYER_H

#include "globals.h"
#include "bflib_basics.h"
#include "config_campaigns.h"
#include <SDL2/SDL_mixer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAMPAIGN_LANDMAP_TRACK 1
#define FIRST_TRACK 2
#define MUSIC_TRACKS_COUNT 126
extern Mix_Music* tracks[];
extern int max_track;

TbBool IsRedbookMusicActive(void);
int InitializeMusicPlayer(void);
void ShutdownMusicPlayer(void);
void PlayMusicPlayer(int track);
void PauseMusicPlayer(void);
void ResumeMusicPlayer(void);
void StopMusicPlayer(void);
void SetMusicPlayerVolume(int volume);
void free_custom_music();
void music_reinit_after_load();
TbBool load_campaign_soundtrack(struct GameCampaign *campgn);

#ifdef __cplusplus
}
#endif

#endif
