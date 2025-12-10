/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config.h
 *     Header file for config.c.
 * @par Purpose:
 *     Configuration and campaign files support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     30 Jan 2009 - 11 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_CONFIG_KEEPERFX_H
#define DK_CONFIG_KEEPERFX_H

#include "bflib_basics.h"
#include "globals.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct CreditsItem;
struct GameCampaign;
/******************************************************************************/


enum TbFeature {
    Ft_EyeLens      =  0x0001,
    Ft_HiResVideo   =  0x0002,
    Ft_BigPointer   =  0x0004,
    Ft_HiResCreatr  =  0x0008,
    Ft_AdvAmbSound  =  0x0010,
    Ft_Censorship   =  0x0020,
    Ft_Atmossounds  =  0x0040,
    Ft_Resizemovies =  0x0080,
    Ft_FreezeOnLoseFocus            = 0x0400,
    Ft_UnlockCursorOnPause          = 0x0800,
    Ft_LockCursorInPossession       = 0x1000,
    Ft_PauseMusicOnGamePause        = 0x2000,
    Ft_MuteAudioOnLoseFocus         = 0x4000,
    Ft_SkipHeartZoom                = 0x8000,
    Ft_SkipSplashScreens            = 0x10000, // no longer used
    Ft_DisableCursorCameraPanning   = 0x20000,
    Ft_DeltaTime                    = 0x40000,
    Ft_NoCdMusic                    = 0x80000,
};

enum TbLanguage {
    Lang_Unset    =  0,
    Lang_English,
    Lang_French,
    Lang_German,
    Lang_Italian,
    Lang_Spanish,
    Lang_Swedish,
    Lang_Polish,
    Lang_Dutch,
    Lang_Hungarian,
    Lang_Korean,
    Lang_Danish,
    Lang_Norwegian,
    Lang_Czech,
    Lang_Arabic,
    Lang_Russian,
    Lang_Japanese,
    Lang_ChineseInt,
    Lang_ChineseTra,
    Lang_Portuguese,
    Lang_Hindi,
    Lang_Bengali,
    Lang_Javanese,
    Lang_Latin,
    Lang_Ukrainian,
};

enum StartupFlags {
    SFlg_Legal        =  0x01,
    SFlg_FX           =  0x02,
    SFlg_Bullfrog     =  0x04,
    SFlg_EA           =  0x08,
    SFlg_Intro        =  0x10,
};


#pragma pack(1)


/******************************************************************************/

struct InstallInfo {
  char inst_path[150];
  int lang_id;
};

extern unsigned short AtmosRepeat;
extern unsigned short AtmosStart;
extern unsigned short AtmosEnd;
extern TbBool AssignCpuKeepers;

extern unsigned int vid_scale_flags;
/******************************************************************************/
extern struct InstallInfo install_info;
extern char keeper_runtime_directory[152];

#pragma pack()
/******************************************************************************/
extern unsigned long features_enabled;
extern const struct NamedCommand lang_type[];
extern const struct NamedCommand scrshot_type[];
extern char cmd_char;
extern short api_enabled;
extern uint16_t api_port;
extern TbBool exit_on_lua_error;
extern TbBool FLEE_BUTTON_DEFAULT;
extern TbBool IMPRISON_BUTTON_DEFAULT;
/******************************************************************************/
void load_configuration_for_mod_all(void);
short load_configuration(void);
void process_cmdline_overrides(void);
int parse_draw_fps_config_val(const char *arg, long *fps_draw_main, long *fps_draw_secondary);
/******************************************************************************/
TbBool is_feature_on(unsigned long feature);
void set_skip_heart_zoom_feature(TbBool enable);
TbBool get_skip_heart_zoom_feature(void);
TbBool censorship_enabled(void);
TbBool atmos_sounds_enabled(void);
TbBool resize_movies_enabled(void);
TbBool freeze_game_on_focus_lost(void);
TbBool unlock_cursor_when_game_paused(void);
TbBool lock_cursor_in_possession(void);
TbBool pause_music_when_game_paused(void);
TbBool mute_audio_on_focus_lost(void);
/******************************************************************************/
const char *get_language_lwrstr(int lang_id);
/******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif
