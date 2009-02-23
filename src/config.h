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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT int __stdcall _DK_load_configuration(void);
/******************************************************************************/
#pragma pack(1)

DLLIMPORT extern struct InstallInfo _DK_install_info;
#define install_info _DK_install_info
DLLIMPORT extern char _DK_keeper_runtime_directory[152];
#define keeper_runtime_directory _DK_keeper_runtime_directory

#pragma pack()
/******************************************************************************/
#define CAMPAIGN_LEVELS_COUNT 50

enum TbFileGroups {
        FGrp_None,
        FGrp_StdData,
        FGrp_LrgData,
        FGrp_FxData,
        FGrp_LoData,
        FGrp_HiData,
        FGrp_Levels,
        FGrp_Save,
        FGrp_SShots,
        FGrp_StdSound,
        FGrp_LrgSound,
        FGrp_Main,
        FGrp_Campgn,
};

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

struct GameCampaign {
  char name[LINEMSG_SIZE];
  unsigned long single_levels[CAMPAIGN_LEVELS_COUNT];
  unsigned long multi_levels[CAMPAIGN_LEVELS_COUNT];
  unsigned long bonus_levels[CAMPAIGN_LEVELS_COUNT];
  unsigned long extra_levels[CAMPAIGN_LEVELS_COUNT];
};

/******************************************************************************/

/******************************************************************************/
char *prepare_file_path(short fgroup,const char *fname);
char *prepare_file_fmtpath(short fgroup, const char *fmt_str, ...);
short load_configuration(void);
short load_default_campaign(void);
short load_campaign(const char *cmpgn_fname,struct GameCampaign *campgn);
short is_bonus_level(long levidx);
short is_singleplayer_level(long levidx);
short is_original_singleplayer_level(long levidx);
short is_multiplayer_level(long levidx);
int array_index_for_levels_bonus(long levidx);
long first_singleplayer_level(void);
long next_singleplayer_level(long levidx);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
