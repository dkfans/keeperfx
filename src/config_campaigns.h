/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_campaigns.h
 *     Header file for config_campaigns.c.
 * @par Purpose:
 *     Campaigns handling functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGCMPGNS_H
#define DK_CFGCMPGNS_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"
#include "config_strings.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CAMPAIGN_LEVELS_COUNT        50
#define EXTRA_LEVELS_COUNT           10
#define MULTI_LEVELS_COUNT         1000
#define FREE_LEVELS_COUNT          5000
#define VISIBLE_HIGH_SCORES_COUNT    10
#define LEVEL_INFO_GROW_DELTA        32
#define HISCORE_NAME_LENGTH          64
#define CAMPAIGNS_LIST_GROW_DELTA     8
#define CAMPAIGN_CREDITS_COUNT      360
/** Strings length */
#define CAMPAIGN_FNAME_LEN           64

enum CreditsItemKind {
    CIK_None,
    CIK_EmptyLine,
    CIK_DirectText,
    CIK_StringId,
};

enum LandMarkings {
    LndMk_ENSIGNS,
    LndMk_PINPOINTS,
};


/******************************************************************************/
struct CreditsItem {
  unsigned short kind;
  unsigned short font;
  union {
    long num;
    char *str;
  };
};

/*
 * Structure for storing campaign configuration.
 */
struct GameCampaign {
  char name[LINEMSG_SIZE];
  char fname[DISKPATH_SIZE];
  char levels_location[DISKPATH_SIZE];
  char speech_location[DISKPATH_SIZE];
  char land_location[DISKPATH_SIZE];
  char creatures_location[DISKPATH_SIZE];
  char configs_location[DISKPATH_SIZE];
  char media_location[DISKPATH_SIZE];
  LevelNumber single_levels[CAMPAIGN_LEVELS_COUNT];
  LevelNumber multi_levels[MULTI_LEVELS_COUNT];
  LevelNumber bonus_levels[CAMPAIGN_LEVELS_COUNT];
  LevelNumber extra_levels[EXTRA_LEVELS_COUNT];
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
  // Land view
  unsigned long ambient_good;
  unsigned long ambient_bad;
  char land_view_start[DISKPATH_SIZE];
  char land_window_start[DISKPATH_SIZE];
  char land_view_end[DISKPATH_SIZE];
  char land_window_end[DISKPATH_SIZE];
  unsigned char land_markers;
  char movie_intro_fname[DISKPATH_SIZE];
  char movie_outro_fname[DISKPATH_SIZE];
  // Credits
  char credits_fname[DISKPATH_SIZE];
  char *credits_data;
  struct CreditsItem credits[CAMPAIGN_CREDITS_COUNT];
  // Campaign strings
  char strings_fname[DISKPATH_SIZE];
  char *strings_data;
  char *strings[STRINGS_MAX+1];
  // High scores
  char hiscore_fname[DISKPATH_SIZE];
  struct HighScore *hiscore_table;
  unsigned long hiscore_count;
  // Human player color
  short human_player;
  TbBool assignCpuKeepers;
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
  TextStringId name_stridx;
  long players;
  long ensign_x;
  long ensign_y;
  long ensign_zoom_x;
  long ensign_zoom_y;
  unsigned long options;
  unsigned short state;
  unsigned short location;
};

struct CampaignsList {
  struct GameCampaign *items;
  unsigned long items_num;
  unsigned long items_count;
};

/******************************************************************************/
extern struct GameCampaign campaign;
extern struct CampaignsList campaigns_list;
extern struct CampaignsList mappacks_list;
extern const struct NamedCommand cmpgn_map_commands[];
extern const struct NamedCommand cmpgn_map_cmnds_options[];
extern const struct NamedCommand cmpgn_map_cmnds_kind[];
extern const struct NamedCommand cmpgn_human_player_options[];
/******************************************************************************/
TbBool load_campaign(const char *cmpgn_fname,struct GameCampaign *campgn,unsigned short flags, short fgroup);
TbBool free_campaign(struct GameCampaign *campgn);
long add_single_level_to_campaign(struct GameCampaign *campgn, LevelNumber lvnum);
long add_multi_level_to_campaign(struct GameCampaign *campgn, LevelNumber lvnum);
long add_bonus_level_to_campaign(struct GameCampaign *campgn, LevelNumber lvnum);
long add_extra_level_to_campaign(struct GameCampaign *campgn, LevelNumber lvnum);
long add_freeplay_level_to_campaign(struct GameCampaign *campgn,LevelNumber lvnum);
// Level info support for given campaign
struct LevelInformation *get_campaign_level_info(struct GameCampaign *campgn, LevelNumber lvnum);
TbBool init_level_info_entries(struct GameCampaign *campgn, long num_entries);
TbBool grow_level_info_entries(struct GameCampaign *campgn, long add_entries);
TbBool free_level_info_entries(struct GameCampaign *campgn);
struct LevelInformation *new_level_info_entry(struct GameCampaign *campgn, LevelNumber lvnum);
// Support for lists of campaigns
TbBool init_campaigns_list_entries(struct CampaignsList *clist, long num_entries);
TbBool grow_campaigns_list_entries(struct CampaignsList *clist, long add_entries);
TbBool free_campaigns_list_entries(struct CampaignsList *clist);
TbBool load_campaigns_list(void);
TbBool load_mappacks_list(void);
TbBool change_campaign(const char *cmpgn_fname);
TbBool is_campaign_loaded(void);
TbBool is_campaign_in_list(const char *cmpgn_fname, struct CampaignsList *clist);
TbBool check_lif_files_in_mappack(struct GameCampaign *campgn);
TbBool is_map_pack(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
