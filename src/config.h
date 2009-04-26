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

#ifndef DK_CONFIG_H
#define DK_CONFIG_H

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CAMPAIGN_LEVELS_COUNT        50
#define FREE_LEVELS_COUNT         10000
#define VISIBLE_HIGH_SCORES_COUNT    10
#define LEVEL_INFO_GROW_DELTA        32
#define HISCORE_NAME_LENGTH          64

#define SINGLEPLAYER_FINISHED    -1
#define SINGLEPLAYER_NOTSTARTED   0
#define LEVELNUMBER_ERROR        -2

#define MAP_SCREEN_WIDTH    1280
#define MAP_SCREEN_HEIGHT    960

#define MESSAGE_TEXT_LEN           1024
#define QUICK_MESSAGES_COUNT         50

enum TbFileGroups {
        FGrp_None,
        FGrp_StdData,
        FGrp_LrgData,
        FGrp_FxData,
        FGrp_LoData,
        FGrp_HiData,
        FGrp_VarLevels,
        FGrp_Save,
        FGrp_SShots,
        FGrp_StdSound,
        FGrp_LrgSound,
        FGrp_AtlSound,
        FGrp_Main,
        FGrp_Campgn,
        FGrp_CmpgLvls,
};

enum TbFeature {
    Ft_EyeLens     =  0x0001,
    Ft_HiResVideo  =  0x0002,
    Ft_BigPointer  =  0x0004,
    Ft_HiResCreatr =  0x0008,
    Ft_AdvAmbSonud =  0x0010,
};

enum TbExtraLevels {
    ExLv_None      =  0,
    ExLv_FullMoon  =  1,
    ExLv_NewMoon   =  2,
};

enum TbLevelOptions {
    LvOp_None      =  0x00,
    LvOp_IsSingle  =  0x01,
    LvOp_IsMulti   =  0x02,
    LvOp_IsBonus   =  0x04,
    LvOp_IsExtra   =  0x08,
    LvOp_IsFree    =  0x10,
    LvOp_AlwsVisbl =  0x20,
};

enum TbLevelState {
    LvSt_Hidden    =  0,
    LvSt_HalfShow  =  1,
    LvSt_Visible   =  2,
};

enum TbLevelLocation {
    LvLc_VarLevels =  0,
    LvLc_Campaign  =  1,
    LvLc_Custom    =  2,
};

/******************************************************************************/
#pragma pack(1)

struct LanguageType {
  const char *name;
  int num;
  };

struct ConfigCommand {
  const char *name;
  int num;
  };

struct InstallInfo {
  char inst_path[150];
int field_96;
int field_9A;
  };

/*
 * Structure for storing campaign configuration.
 */
struct GameCampaign {
  char name[LINEMSG_SIZE];
  char location[DISKPATH_SIZE];
  LevelNumber single_levels[CAMPAIGN_LEVELS_COUNT];
  LevelNumber multi_levels[CAMPAIGN_LEVELS_COUNT];
  LevelNumber bonus_levels[CAMPAIGN_LEVELS_COUNT];
  LevelNumber extra_levels[CAMPAIGN_LEVELS_COUNT];
  LevelNumber freeplay_levels[FREE_LEVELS_COUNT];
  unsigned long single_levels_count;
  unsigned long multi_levels_count;
  unsigned long bonus_levels_count;
  unsigned long extra_levels_count;
  unsigned long freeplay_levels_count;
  unsigned long bonus_levels_index;
  unsigned long extra_levels_index;
  struct LevelInformation *lvinfos;
  unsigned long lvinfos_count;
  unsigned long ambient_good;
  unsigned long ambient_bad;
  char land_view_start[DISKPATH_SIZE];
  char land_window_start[DISKPATH_SIZE];
  char land_view_end[DISKPATH_SIZE];
  char land_window_end[DISKPATH_SIZE];
  char strings_fname[DISKPATH_SIZE];
  char *strings_data;
  char *strings[STRINGS_MAX+1];
  char hiscore_fname[DISKPATH_SIZE];
  struct HighScore *hiscore_table;
  unsigned long hiscore_count;
};

struct HighScore {
  long score;
  char name[HISCORE_NAME_LENGTH];
  LevelNumber lvnum;
};

struct LevelInformation {
  LevelNumber lvnum;
  char speech_before[DISKPATH_SIZE];
  char speech_after[DISKPATH_SIZE];
  char land_view[DISKPATH_SIZE];
  char land_window[DISKPATH_SIZE];
  char name[LINEMSG_SIZE];
  long name_id;
  long players;
  long ensign_x;
  long ensign_y;
  long ensign_zoom_x;
  long ensign_zoom_y;
  unsigned long options;
  unsigned short state;
  unsigned short location;
};

struct NetLevelDesc { // sizeof = 14
  unsigned char field_0;
  unsigned char field_1;
  unsigned long field_2;
  unsigned long field_6;
  unsigned long field_A;
};

/******************************************************************************/
DLLIMPORT extern struct InstallInfo _DK_install_info;
#define install_info _DK_install_info
DLLIMPORT extern char _DK_keeper_runtime_directory[152];
#define keeper_runtime_directory _DK_keeper_runtime_directory
DLLIMPORT extern float _DK_phase_of_moon;
#define phase_of_moon _DK_phase_of_moon
//DLLIMPORT extern struct HighScore _DK_high_score_table[VISIBLE_HIGH_SCORES_COUNT];
//#define high_score_table _DK_high_score_table
DLLIMPORT extern long _DK_net_number_of_levels;
#define net_number_of_levels _DK_net_number_of_levels
DLLIMPORT extern struct NetLevelDesc _DK_net_level_desc[100];
#define net_level_desc _DK_net_level_desc

#pragma pack()
/******************************************************************************/
extern unsigned long features_enabled;
extern short is_full_moon;
extern short is_near_full_moon;
extern short is_new_moon;
extern short is_near_new_moon;
extern struct GameCampaign campaign;
extern char quick_messages[MESSAGE_TEXT_LEN][QUICK_MESSAGES_COUNT];
/******************************************************************************/
DLLIMPORT int __stdcall _DK_load_configuration(void);
/******************************************************************************/
char *prepare_file_path_buf(char *ffullpath,short fgroup,const char *fname);
char *prepare_file_path(short fgroup,const char *fname);
char *prepare_file_fmtpath(short fgroup, const char *fmt_str, ...);
unsigned char *load_data_file_to_buffer(long *ldsize, short fgroup, const char *fmt_str, ...);
/******************************************************************************/
short update_features(unsigned long mem_size);
short load_configuration(void);
short calculate_moon_phase(short do_calculate,short add_to_log);
short load_high_score_table(void);
short save_high_score_table(void);
short create_empty_high_score_table(void);
int add_high_score_entry(unsigned long score, LevelNumber lvnum, char *name);
unsigned long get_level_highest_score(LevelNumber lvnum);
/******************************************************************************/
short load_default_campaign(void);
short load_campaign(const char *cmpgn_fname,struct GameCampaign *campgn);
short free_campaign(struct GameCampaign *campgn);
long add_single_level_to_campaign(struct GameCampaign *campgn, LevelNumber lvnum);
long add_multi_level_to_campaign(struct GameCampaign *campgn, LevelNumber lvnum);
long add_bonus_level_to_campaign(struct GameCampaign *campgn, LevelNumber lvnum);
long add_extra_level_to_campaign(struct GameCampaign *campgn, LevelNumber lvnum);
long add_freeplay_level_to_campaign(struct GameCampaign *campgn,LevelNumber lvnum);
short is_bonus_level(LevelNumber lvnum);
short is_extra_level(LevelNumber lvnum);
short is_singleplayer_level(LevelNumber lvnum);
short is_original_singleplayer_level(LevelNumber lvnum);
short is_singleplayer_like_level(LevelNumber lvnum);
short is_multiplayer_level(LevelNumber lvnum);
short is_campaign_level(LevelNumber lvnum);
short is_freeplay_level(LevelNumber lvnum);
int array_index_for_bonus_level(LevelNumber bn_lvnum);
int array_index_for_extra_level(LevelNumber ex_lvnum);
int array_index_for_singleplayer_level(LevelNumber sp_lvnum);
int array_index_for_multiplayer_level(LevelNumber mp_lvnum);
int array_index_for_freeplay_level(LevelNumber fp_lvnum);
int storage_index_for_bonus_level(LevelNumber bn_lvnum);
LevelNumber first_singleplayer_level(void);
LevelNumber last_singleplayer_level(void);
LevelNumber next_singleplayer_level(LevelNumber sp_lvnum);
LevelNumber prev_singleplayer_level(LevelNumber sp_lvnum);
LevelNumber bonus_level_for_singleplayer_level(LevelNumber sp_lvnum);
LevelNumber first_multiplayer_level(void);
LevelNumber last_multiplayer_level(void);
LevelNumber next_multiplayer_level(LevelNumber mp_lvnum);
LevelNumber prev_multiplayer_level(LevelNumber mp_lvnum);
LevelNumber first_freeplay_level(void);
LevelNumber last_freeplay_level(void);
LevelNumber next_freeplay_level(LevelNumber fp_lvnum);
LevelNumber prev_freeplay_level(LevelNumber fp_lvnum);
LevelNumber first_extra_level(void);
LevelNumber next_extra_level(LevelNumber ex_lvnum);
LevelNumber get_extra_level(unsigned short elv_kind);
// Level info support for given campaign
struct LevelInformation *get_campaign_level_info(struct GameCampaign *campgn, LevelNumber lvnum);
short init_level_info_entries(struct GameCampaign *campgn, long num_entries);
short grow_level_info_entries(struct GameCampaign *campgn, long add_entries);
short free_level_info_entries(struct GameCampaign *campgn);
struct LevelInformation *new_level_info_entry(struct GameCampaign *campgn, LevelNumber lvnum);
// Level info support for active campaign
struct LevelInformation *get_level_info(LevelNumber lvnum);
struct LevelInformation *get_or_create_level_info(LevelNumber lvnum, unsigned long lvoptions);
struct LevelInformation *get_first_level_info(void);
struct LevelInformation *get_last_level_info(void);
struct LevelInformation *get_next_level_info(struct LevelInformation *previnfo);
struct LevelInformation *get_prev_level_info(struct LevelInformation *nextinfo);
short set_level_info_text_name(LevelNumber lvnum, char *name, unsigned long lvoptions);
short get_level_fgroup(LevelNumber lvnum);
/******************************************************************************/
short setup_gui_strings_data(void);
short free_gui_strings_data(void);
short reset_strings(char **strings);
short setup_campaign_strings_data(struct GameCampaign *campgn);
/******************************************************************************/
short find_conf_block(const char *buf,long *pos,long buflen,const char *blockname);
int recognize_conf_command(const char *buf,long *pos,long buflen,const struct ConfigCommand *commands);
short skip_conf_to_next_line(const char *buf,long *pos,long buflen);
int get_conf_parameter_single(const char *buf,long *pos,long buflen,char *dst,long dstlen);
int get_conf_parameter_whole(const char *buf,long *pos,long buflen,char *dst,long dstlen);
int recognize_conf_parameter(const char *buf,long *pos,long buflen,const struct ConfigCommand *commands);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
