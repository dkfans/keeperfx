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

#define CAMPAIGN_LEVELS_COUNT  50
#define HIGH_SCORES_COUNT      10

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

DLLIMPORT extern struct InstallInfo _DK_install_info;
#define install_info _DK_install_info
DLLIMPORT extern char _DK_keeper_runtime_directory[152];
#define keeper_runtime_directory _DK_keeper_runtime_directory
DLLIMPORT extern float _DK_phase_of_moon;
#define phase_of_moon _DK_phase_of_moon
DLLIMPORT extern struct HighScore _DK_high_score_table[HIGH_SCORES_COUNT];
#define high_score_table _DK_high_score_table

#pragma pack()
/******************************************************************************/

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
        FGrp_AtlSound,
        FGrp_Main,
        FGrp_Campgn,
};

enum TbFeature {
    Ft_EyeLens     =  0x0001,
    Ft_HiResVideo  =  0x0002,
    Ft_BigPointer  =  0x0004,
    Ft_HiResCreatr =  0x0008,
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
  char hiscore_fname[LINEMSG_SIZE];
};

struct HighScore {
        long score;
        char name[64];
        long level;
};

/******************************************************************************/
extern unsigned long features_enabled;
extern short is_full_moon;
extern short is_new_moon;
extern struct GameCampaign campaign;
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
/******************************************************************************/
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
