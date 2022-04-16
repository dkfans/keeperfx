/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_campaigns.c
 *     Campaigns handling functions.
 * @par Purpose:
 *     Functions to support campaign file and lists of campaigns.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     15 Mar 2009 - 16 Apr 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_campaigns.h"

#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_strings.h"
#include "lvl_filesdk1.h"
#include "frontmenu_ingame_tabs.h"

#include "game_merge.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_campaign_file[]="keeporig.cfg";
const char deeper_mappack_file[]="deepdngn.cfg";

const struct NamedCommand cmpgn_common_commands[] = {
  {"NAME",                1},
  {"SINGLE_LEVELS",       2},
  {"MULTI_LEVELS",        3},
  {"BONUS_LEVELS",        4},
  {"EXTRA_LEVELS",        5},
  {"HIGH_SCORES",         6},
  {"LAND_VIEW_START",     7},
  {"LAND_VIEW_END",       8},
  {"LAND_AMBIENT",        9},
  {"LEVELS_LOCATION",    10},
  {"LAND_LOCATION",      11},
  {"CREATURES_LOCATION", 12},
  {"CONFIGS_LOCATION",   13},
  {"CREDITS",            14},
  {"MEDIA_LOCATION",     15},
  {"INTRO_MOVIE",        16},
  {"OUTRO_MOVIE",        17},
  {"LAND_MARKERS",       18},
  {"HUMAN_PLAYER",       19},
  {"NAME_TEXT_ID",       20},
  {"ASSIGN_CPU_KEEPERS", 21},
  {NULL,                  0},
  };

const struct NamedCommand cmpgn_map_commands[] = {
  {"NAME_TEXT",       1},
  {"NAME_ID",         2},
  {"ENSIGN_POS",      3},
  {"ENSIGN_ZOOM",     4},
  {"PLAYERS",         5},
  {"OPTIONS",         6},
  {"SPEECH",          7},
  {"LAND_VIEW",       8},
  {"KIND",            9}, // for LOF files only
  {"AUTHOR",         10},
  {"DESCRIPTION",    11},
  {"DATE",           12},
  {NULL,              0},
  };

const struct NamedCommand cmpgn_map_cmnds_options[] = {
  {"TUTORIAL",        LvOp_Tutorial},
  {NULL,              0},
  };

const struct NamedCommand cmpgn_level_markers_options[] = {
  {"ENSIGNS",        LndMk_ENSIGNS},
  {"PINPOINTS",      LndMk_PINPOINTS},
  {NULL,              0},
  };

const struct NamedCommand cmpgn_map_cmnds_kind[] = {
  {"SINGLE",          LvOp_IsSingle},
  {"MULTI",           LvOp_IsMulti},
  {"BONUS",           LvOp_IsBonus},
  {"EXTRA",           LvOp_IsExtra},
  {"FREE",            LvOp_IsFree},
  {NULL,              0},
  };

const struct NamedCommand cmpgn_human_player_options[] = {
  {"RED",        0},
  {"BLUE",       1},
  {"GREEN",      2},
  {"YELLOW",     3},
  {"WHITE",      4},
  {NULL,         0},
  };

/******************************************************************************/
struct GameCampaign campaign;
struct CampaignsList campaigns_list;
struct CampaignsList mappacks_list;

/******************************************************************************/
/*
 * Frees campaign sub-entries memory without NULLing invalid pointers.
 * Call clear_campaign() to reset values after they're freed.
 */
TbBool free_campaign(struct GameCampaign *campgn)
{
  LbMemoryFree(campgn->lvinfos);
  LbMemoryFree(campgn->hiscore_table);
  LbMemoryFree(campgn->strings_data);
  LbMemoryFree(campgn->credits_data);
  return true;
}

void clear_level_info(struct LevelInformation *lvinfo)
{
  LbMemorySet(lvinfo,0,sizeof(struct LevelInformation));
  lvinfo->lvnum = 0;
  LbMemorySet(lvinfo->name, 0, LINEMSG_SIZE);
  LbMemorySet(lvinfo->speech_before, 0, DISKPATH_SIZE);
  LbMemorySet(lvinfo->speech_after, 0, DISKPATH_SIZE);
  LbMemorySet(lvinfo->land_view, 0, DISKPATH_SIZE);
  LbMemorySet(lvinfo->land_window, 0, DISKPATH_SIZE);
  lvinfo->name_stridx = 0;
  lvinfo->players = 1;
  lvinfo->ensign_x = (LANDVIEW_MAP_WIDTH>>1);
  lvinfo->ensign_y = (LANDVIEW_MAP_HEIGHT>>1);
  lvinfo->ensign_zoom_x = (LANDVIEW_MAP_WIDTH>>1);
  lvinfo->ensign_zoom_y = (LANDVIEW_MAP_HEIGHT>>1);
  lvinfo->options = LvOp_None;
  lvinfo->state = LvSt_Hidden;
  lvinfo->location = LvLc_VarLevels;
}

/**
 * Clears campaign entries without freeing memory.
 */
TbBool clear_campaign(struct GameCampaign *campgn)
{
  int i;
  SYNCDBG(10,"Starting");
  LbMemorySet(campgn->name,0,LINEMSG_SIZE);
  LbMemorySet(campgn->fname,0,DISKPATH_SIZE);
  LbMemorySet(campgn->levels_location,0,DISKPATH_SIZE);
  LbMemorySet(campgn->speech_location,0,DISKPATH_SIZE);
  LbMemorySet(campgn->land_location,0,DISKPATH_SIZE);
  LbMemorySet(campgn->creatures_location,0,DISKPATH_SIZE);
  LbMemorySet(campgn->configs_location,0,DISKPATH_SIZE);
  LbMemorySet(campgn->media_location,0,DISKPATH_SIZE);
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    campgn->single_levels[i] = 0;
    campgn->bonus_levels[i] = 0;
  }
  for (i=0; i<EXTRA_LEVELS_COUNT; i++)
  {
    campgn->extra_levels[i] = 0;
  }
  for (i=0; i<MULTI_LEVELS_COUNT; i++)
  {
    campgn->multi_levels[i] = 0;
  }
  for (i=0; i<FREE_LEVELS_COUNT; i++)
  {
    campgn->freeplay_levels[i] = 0;
  }
  campgn->single_levels_count = 0;
  campgn->multi_levels_count = 0;
  campgn->bonus_levels_count = 0;
  campgn->extra_levels_count = 0;
  campgn->freeplay_levels_count = 0;
  campgn->bonus_levels_index = 0;
  campgn->extra_levels_index = 0;
  campgn->lvinfos_count = 0;
  campgn->lvinfos = NULL;
  campgn->ambient_good = 0;
  campgn->ambient_bad = 0;
  LbMemorySet(campgn->land_view_start,0,DISKPATH_SIZE);
  LbMemorySet(campgn->land_window_start,0,DISKPATH_SIZE);
  LbMemorySet(campgn->land_view_end,0,DISKPATH_SIZE);
  LbMemorySet(campgn->land_window_end,0,DISKPATH_SIZE);
  campgn->land_markers = LndMk_ENSIGNS;
  LbMemorySet(campgn->movie_intro_fname,0,DISKPATH_SIZE);
  LbMemorySet(campgn->movie_outro_fname,0,DISKPATH_SIZE);
  LbMemorySet(campgn->strings_fname,0,DISKPATH_SIZE);
  campgn->strings_data = NULL;
  reset_strings(campgn->strings, STRINGS_MAX);
  LbMemorySet(campgn->hiscore_fname,0,DISKPATH_SIZE);
  campgn->hiscore_table = NULL;
  campgn->hiscore_count = 0;
  LbMemorySet(campgn->credits_fname,0,DISKPATH_SIZE);
  campgn->credits_data = NULL;
  reset_credits(campgn->credits);
  campgn->human_player = -1;
  campgn->assignCpuKeepers = 0;
  return true;
}

long add_single_level_to_campaign(struct GameCampaign *campgn,LevelNumber lvnum)
{
  if (lvnum <= 0) return LEVELNUMBER_ERROR;
  long i = campgn->single_levels_count;
  if (i < CAMPAIGN_LEVELS_COUNT)
  {
    campgn->single_levels[i] = lvnum;
    campgn->single_levels_count++;
    return i;
  }
  return LEVELNUMBER_ERROR;
}

long add_multi_level_to_campaign(struct GameCampaign *campgn,LevelNumber lvnum)
{
  if (lvnum <= 0) return LEVELNUMBER_ERROR;
  long i = campgn->multi_levels_count;
  if (i < MULTI_LEVELS_COUNT)
  {
    campgn->multi_levels[i] = lvnum;
    campgn->multi_levels_count++;
    return i;
  }
  return LEVELNUMBER_ERROR;
}

long add_bonus_level_to_campaign(struct GameCampaign *campgn,LevelNumber lvnum)
{
  if (lvnum < 0) lvnum = 0;
  // adding bonus level
  long i = campgn->bonus_levels_index;
  if (i < CAMPAIGN_LEVELS_COUNT)
  {
    campgn->bonus_levels[i] = lvnum;
    campgn->bonus_levels_index++;
    if (lvnum > 0)
      campgn->bonus_levels_count++;
    return i;
  }
  return LEVELNUMBER_ERROR;
}

long add_extra_level_to_campaign(struct GameCampaign *campgn,LevelNumber lvnum)
{
  if (lvnum < 0) lvnum = 0;
  // adding extra level
  long i = campgn->extra_levels_index;
  if (i < EXTRA_LEVELS_COUNT)
  {
    campgn->extra_levels[i] = lvnum;
    campgn->extra_levels_index++;
    if (lvnum > 0)
      campgn->extra_levels_count++;
    return i;
  }
  return LEVELNUMBER_ERROR;
}

long add_freeplay_level_to_campaign(struct GameCampaign *campgn,LevelNumber lvnum)
{
  if (lvnum <= 0) return LEVELNUMBER_ERROR;
  // check if already in list
  long i = 0;
  while (i < campgn->freeplay_levels_count)
  {
    if (campgn->freeplay_levels[i] == lvnum)
      return i;
    i++;
  }
  // add the free level to list
  if (i < FREE_LEVELS_COUNT)
  {
    campgn->freeplay_levels[i] = lvnum;
    campgn->freeplay_levels_count++;
    return i;
  }
  return LEVELNUMBER_ERROR;
}

struct LevelInformation *get_campaign_level_info(struct GameCampaign *campgn, LevelNumber lvnum)
{
  if (lvnum <= 0)
      return NULL;
  if (campgn->lvinfos == NULL)
      return NULL;
  for (long i = 0; i < campgn->lvinfos_count; i++)
  {
      if (campgn->lvinfos[i].lvnum == lvnum)
      {
          return &campgn->lvinfos[i];
      }
  }
  return NULL;
}

struct LevelInformation *new_level_info_entry(struct GameCampaign *campgn, LevelNumber lvnum)
{
  long i;
  if (lvnum <= 0)
    return NULL;
  if (campgn->lvinfos == NULL)
    return NULL;
  // Find empty allocated slot
  for (i=0; i < campgn->lvinfos_count; i++)
  {
      if (campgn->lvinfos[i].lvnum <= 0)
      {
          clear_level_info(&campgn->lvinfos[i]);
          campgn->lvinfos[i].lvnum = lvnum;
          return &campgn->lvinfos[i];
      }
  }
  // No empty slot - reallocate memory to get more slots
  if (!grow_level_info_entries(campgn,LEVEL_INFO_GROW_DELTA))
    return NULL;
  campgn->lvinfos[i].lvnum = lvnum;
  return &campgn->lvinfos[i];
}

TbBool init_level_info_entries(struct GameCampaign *campgn, long num_entries)
{
    if (campgn->lvinfos != NULL)
      LbMemoryFree(campgn->lvinfos);
    campgn->lvinfos = (struct LevelInformation *)LbMemoryAlloc(num_entries*sizeof(struct LevelInformation));
    if (campgn->lvinfos == NULL)
    {
      WARNMSG("Can't allocate memory for LevelInformation list.");
      campgn->lvinfos_count = 0;
      return false;
    }
    campgn->lvinfos_count = num_entries;
    for (long i = 0; i < num_entries; i++)
    {
        clear_level_info(&campgn->lvinfos[i]);
    }
    return true;
}

TbBool grow_level_info_entries(struct GameCampaign *campgn, long add_entries)
{
    long i = campgn->lvinfos_count;
    long num_entries = campgn->lvinfos_count + add_entries;
    campgn->lvinfos = (struct LevelInformation*)LbMemoryGrow(campgn->lvinfos, num_entries * sizeof(struct LevelInformation));
    if (campgn->lvinfos == NULL)
    {
        WARNMSG("Can't enlarge memory for LevelInformation list.");
        campgn->lvinfos_count = 0;
        return false;
  }
  campgn->lvinfos_count = num_entries;
  while (i < num_entries)
  {
    clear_level_info(&campgn->lvinfos[i]);
    i++;
  }
  return true;
}

TbBool free_level_info_entries(struct GameCampaign *campgn)
{
  if (campgn->lvinfos != NULL)
    LbMemoryFree(campgn->lvinfos);
  campgn->lvinfos = NULL;
  campgn->lvinfos_count = 0;
  return true;
}

short parse_campaign_common_blocks(struct GameCampaign *campgn,char *buf,long len)
{
  static const char config_textname[] = "Campaign config";
  // Initialize block data in campaign
  LbMemoryFree(campgn->hiscore_table);
  campgn->hiscore_table = NULL;
  campgn->hiscore_count = VISIBLE_HIGH_SCORES_COUNT;
  // Find the block
  char block_buf[32];
  sprintf(block_buf, "common");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return 0;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(cmpgn_common_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, cmpgn_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      int i = 0, n = 0;
      char word_buf[32];
      switch (cmd_num)
      {
      case 1: // NAME
          i = get_conf_parameter_whole(buf,&pos,len,campgn->name,LINEMSG_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 2: // SINGLE_LEVELS
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
              if (add_single_level_to_campaign(campgn,k) < 0)
                  CONFWRNLOG("No free slot to add level %d from \"%s\" command of %s file.",
                      k,COMMAND_TEXT(cmd_num),config_textname);
            } else
            {
                CONFWRNLOG("Couldn't recognize level in \"%s\" command of %s file.",
                  COMMAND_TEXT(cmd_num),config_textname);
            }
          }
          if (campgn->single_levels_count <= 0)
              CONFWRNLOG("Levels list empty in \"%s\" command of %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 3: // MULTI_LEVELS
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
              if (add_multi_level_to_campaign(campgn,k) < 0)
                  CONFWRNLOG("No free slot to add level %d from \"%s\" command of %s file.",
                      k,COMMAND_TEXT(cmd_num),config_textname);
            } else
            {
                CONFWRNLOG("Couldn't recognize level in \"%s\" command of %s file.",
                  COMMAND_TEXT(cmd_num),config_textname);
            }
          }
          if (campgn->multi_levels_count <= 0)
          {
              CONFWRNLOG("Levels list empty in \"%s\" command of %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case 4: // BONUS_LEVELS
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0) // Some bonus levels may not exist
            {
              if (add_bonus_level_to_campaign(campgn,k) < 0)
                  CONFWRNLOG("No free slot to add level %d from \"%s\" command of %s file.",
                      k,COMMAND_TEXT(cmd_num),config_textname);
            } else
            {
                CONFWRNLOG("Couldn't recognize level in \"%s\" command of %s file.",
                  COMMAND_TEXT(cmd_num),config_textname);
            }
          }
          //if (campgn->bonus_levels_count <= 0)
                //CONFLOG("Levels list empty in \"%s\" command of campaign file.","BONUS_LEVELS");
          break;
      case 5: // EXTRA_LEVELS
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
              if (add_extra_level_to_campaign(campgn,k) < 0)
                  CONFWRNLOG("No free slot to add level %d from \"%s\" command of %s file.",
                      k,COMMAND_TEXT(cmd_num),config_textname);
            } else
            {
                CONFWRNLOG("Couldn't recognize level in \"%s\" command of %s file.",
                  COMMAND_TEXT(cmd_num),config_textname);
            }
          }
          if (campgn->extra_levels_count <= 0)
              CONFLOG("Levels list empty in \"%s\" command of %s file.",
                    COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 6: // HIGH_SCORES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                campgn->hiscore_count = k;
                n++;
            }
          }
          if (get_conf_parameter_whole(buf,&pos,len,campgn->hiscore_fname,DISKPATH_SIZE) > 0)
          {
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" parameters in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // LAND_VIEW_START
          if (get_conf_parameter_single(buf,&pos,len,campgn->land_view_start,DISKPATH_SIZE) > 0)
          {
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,campgn->land_window_start,DISKPATH_SIZE) > 0)
          {
            n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" file names in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // LAND_VIEW_END
          if (get_conf_parameter_single(buf,&pos,len,campgn->land_view_end,DISKPATH_SIZE) > 0)
          {
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,campgn->land_window_end,DISKPATH_SIZE) > 0)
          {
            n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" file names in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 9: // LAND_AMBIENT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
              campgn->ambient_good = k;
              n++;
            }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
              campgn->ambient_bad = k;
              n++;
            }
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" coordinates in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 10: // LEVELS_LOCATION
          i = get_conf_parameter_whole(buf,&pos,len,campgn->levels_location,DISKPATH_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 11: // LAND_LOCATION
          i = get_conf_parameter_whole(buf,&pos,len,campgn->land_location,DISKPATH_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 12: // CREATURES_LOCATION
          i = get_conf_parameter_whole(buf,&pos,len,campgn->creatures_location,DISKPATH_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 13: // CONFIGS_LOCATION
          i = get_conf_parameter_whole(buf,&pos,len,campgn->configs_location,DISKPATH_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 14: // CREDITS
          i = get_conf_parameter_whole(buf,&pos,len,campgn->credits_fname,DISKPATH_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 15: // MEDIA_LOCATION
          i = get_conf_parameter_whole(buf,&pos,len,campgn->media_location,DISKPATH_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 16: // INTRO_MOVIE
          i = get_conf_parameter_whole(buf,&pos,len,campgn->movie_intro_fname,DISKPATH_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 17: // OUTRO_MOVIE
          i = get_conf_parameter_whole(buf,&pos,len,campgn->movie_outro_fname,DISKPATH_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 18: // LAND_MARKERS
          i = recognize_conf_parameter(buf,&pos,len,cmpgn_level_markers_options);
          if (i >= 0) {
              campgn->land_markers = i;
              n++;
          }
          if (n < 1)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 19: // HUMAN_PLAYER
          i = recognize_conf_parameter(buf,&pos,len,cmpgn_human_player_options);
          if (i >= 0) {
              campgn->human_player = i;
              n++;
          }
          if (n < 1)
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          break;
      case 20: // NAME_TEXT_ID
          i = get_conf_parameter_whole(buf,&pos,len,word_buf,sizeof(word_buf));
          if (i <= 0) {
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          }
          else {
              k = atoi(word_buf);
                if (k > 0) {
                    const char* newname = get_string(STRINGS_MAX+k);
                    if (strcasecmp(newname,"") != 0) {
                        LbStringCopy(campgn->name,newname,LINEMSG_SIZE); // use the index provided in the config file to get a specific UI string
                    }
                    else {
                    CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file. NAME_TEXT_ID is too high, NAME used instead.",
                      COMMAND_TEXT(cmd_num),config_textname);
                    }
                }
                else {
                    CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file. Not a valid number.",
                      COMMAND_TEXT(cmd_num),config_textname);
                }
          }
          break;
      case 21: // ASSIGN_CPU_KEEPERS
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0) {
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          }
          else if (i < 2) { // ASSIGN_CPU_KEEPERS = ON
              campgn->assignCpuKeepers = 1;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
#undef COMMAND_TEXT
  if (campgn->single_levels_count != campgn->bonus_levels_index)
  {
    WARNMSG("Amount of SP levels (%d) and bonuses (%d) do not match in [%s] block of %s file.",
      campgn->single_levels_count, campgn->bonus_levels_count, block_buf, config_textname);
  }
  return 1;
}

short parse_campaign_strings_blocks(struct GameCampaign *campgn,char *buf,long len)
{
  static const char config_textname[] = "Campaign config";
  // Block name and parameter word store variables
  char block_buf[32];
  // Find the block
  sprintf(block_buf,"strings");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return 0;
  }
  int n = 0;
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, lang_type);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      if (cmd_num <= 0)
      {
        if ((cmd_num != 0) && (cmd_num != -1))
            CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",cmd_num,block_buf,config_textname);
      } else
      if ((cmd_num == install_info.lang_id) || (n == 0))
      {
          int i = get_conf_parameter_whole(buf, &pos, len, campgn->strings_fname, LINEMSG_SIZE);
          if (i <= 0)
              CONFWRNLOG("Couldn't read file name in [%s] block parameter of %s file.",block_buf,config_textname);
          else
            n++;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  if (campgn->strings_fname[0] == '\0')
    WARNMSG("Strings file name not set after parsing [%s] block of %s file.", block_buf,config_textname);
  return 1;
}

short parse_campaign_speech_blocks(struct GameCampaign *campgn,char *buf,long len)
{
  static const char config_textname[] = "Campaign config";
  // Block name and parameter word store variables
  char block_buf[32];
  // Find the block
  sprintf(block_buf,"speech");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return 0;
  }
  int n = 0;
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, lang_type);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      if (cmd_num <= 0)
      {
        if ((cmd_num != 0) && (cmd_num != -1))
        {
            CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file, starting on byte %d.",
              cmd_num,block_buf,config_textname,pos);
        }
      } else
      if ((cmd_num == install_info.lang_id) || (n == 0))
      {
          int i = get_conf_parameter_whole(buf, &pos, len, campgn->speech_location, LINEMSG_SIZE);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't read folder name in [%s] block parameter of %s file.",
                block_buf,config_textname);
          } else
            n++;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  if (campgn->speech_location[0] == '\0')
  {
      WARNMSG("Speech folder name not set after parsing [%s] block of %s file.",
          block_buf,config_textname);
  }
  return 1;
}

/*
 * Parses campaign block for specific level number.
 * Stores data in lvinfo structure.
 */
short parse_campaign_map_block(long lvnum, unsigned long lvoptions, char *buf, long len)
{
    static const char config_textname[] = "Campaign config";
    // Block name and parameter word store variables
    SYNCDBG(18,"Starting for level %ld",lvnum);
    struct LevelInformation* lvinfo = get_or_create_level_info(lvnum, lvoptions);
    if (lvinfo == NULL)
    {
      WARNMSG("Can't get LevelInformation item to store level %ld data from %s file.",
          lvnum,config_textname);
      return 0;
    }
    lvinfo->location = LvLc_Campaign;
    char block_buf[32];
    sprintf(block_buf, "map%05lu", lvnum);
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return 0;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(cmpgn_map_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, cmpgn_map_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[32];
        switch (cmd_num)
        {
        case 1: // NAME_TEXT
            if (get_conf_parameter_whole(buf,&pos,len,lvinfo->name,LINEMSG_SIZE) <= 0)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 2: // NAME_ID
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                lvinfo->name_stridx = k;
                n++;
              }
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't recognize \"%s\" number in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // ENSIGN_POS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->ensign_x = k;
                  n++;
                }
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->ensign_y = k;
                  n++;
                }
            }
            if (n < 2)
            {
                CONFWRNLOG("Couldn't recognize \"%s\" coordinates in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // ENSIGN_ZOOM
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->ensign_zoom_x = k;
                  n++;
                }
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->ensign_zoom_y = k;
                  n++;
                }
            }
            if (n < 2)
            {
                CONFWRNLOG("Couldn't recognize \"%s\" coordinates in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // PLAYERS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                lvinfo->players = k;
                n++;
              }
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't recognize \"%s\" number in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // OPTIONS
            while ((k = recognize_conf_parameter(buf,&pos,len,cmpgn_map_cmnds_options)) > 0)
            {
              switch (k)
              {
              case LvOp_Tutorial:
                lvinfo->options |= k;
                break;
              }
              n++;
            }
            break;
        case 7: // SPEECH
            if (get_conf_parameter_single(buf,&pos,len,lvinfo->speech_before,DISKPATH_SIZE) > 0)
            {
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,lvinfo->speech_after,DISKPATH_SIZE) > 0)
            {
              n++;
            }
            if (n < 2)
            {
                CONFWRNLOG("Couldn't recognize \"%s\" file names in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 8: // LAND_VIEW
            if (get_conf_parameter_single(buf,&pos,len,lvinfo->land_view,DISKPATH_SIZE) > 0)
            {
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,lvinfo->land_window,DISKPATH_SIZE) > 0)
            {
              n++;
            }
            if (n < 2)
            {
                CONFWRNLOG("Couldn't recognize \"%s\" file names in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 10: // AUTHOR
        case 11: // DESCRIPTION
        case 12: // DATE
            // As for now, ignore these
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
            break;
        default:
            CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
                cmd_num,block_buf,config_textname);
            break;
        }
        skip_conf_to_next_line(buf,&pos,len);
    }
    SYNCDBG(18,"Level %ld ensign (%d,%d) zoom (%d,%d)",lvnum,(int)lvinfo->ensign_x,(int)lvinfo->ensign_y,(int)lvinfo->ensign_zoom_x,(int)lvinfo->ensign_zoom_y);
#undef COMMAND_TEXT
    return 1;
}

short parse_campaign_map_blocks(struct GameCampaign *campgn, char *buf, long len)
{
    static const char config_textname[] = "Campaign config";
    SYNCDBG(8,"Starting");
    long i = campgn->single_levels_count + campgn->multi_levels_count + campgn->bonus_levels_count + campgn->extra_levels_count + campgn->freeplay_levels_count;
    if (i <= 0)
    {
        WARNMSG("There's zero used levels - no [mapX] blocks to parse in %s file.",config_textname);
        return 0;
    }
    // Initialize the lvinfos array
    if (!init_level_info_entries(campgn,i))
    if (campgn->lvinfos == NULL)
    {
        WARNMSG("Can't allocate memory for LevelInformation list in %s file.",config_textname);
        return 0;
    }
    long lvnum = first_singleplayer_level();
    while (lvnum > 0)
    {
        parse_campaign_map_block(lvnum, LvOp_IsSingle, buf, len);
        long bn_lvnum = bonus_level_for_singleplayer_level(lvnum);
        if (bn_lvnum > 0)
        {
          parse_campaign_map_block(bn_lvnum, LvOp_IsBonus, buf, len);
        }
        lvnum = next_singleplayer_level(lvnum);
    }
    lvnum = first_multiplayer_level();
    while (lvnum > 0)
    {
        parse_campaign_map_block(lvnum, LvOp_IsMulti, buf, len);
        lvnum = next_multiplayer_level(lvnum);
    }
    lvnum = first_extra_level();
    while (lvnum > 0)
    {
        parse_campaign_map_block(lvnum, LvOp_IsExtra, buf, len);
        lvnum = next_extra_level(lvnum);
    }
    return 1;
}

TbBool load_campaign(const char *cmpgn_fname,struct GameCampaign *campgn,unsigned short flags, short fgroup)
{
    // Preparing campaign file name and checking the file
    clear_campaign(campgn);
    LbStringCopy(campgn->fname,cmpgn_fname,DISKPATH_SIZE);
    LbStringCopy(campgn->name,cmpgn_fname,DISKPATH_SIZE);
    SYNCDBG(0,"%s campaign file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",cmpgn_fname);
    char* fname = prepare_file_path(fgroup, cmpgn_fname);
    long len = LbFileLengthRnc(fname);
    if (len < 2)
    {
        WARNMSG("Campaign file \"%s\" doesn't exist or is too small.",cmpgn_fname);
        return false;
    }
    if (len > 65536)
    {
        WARNMSG("Campaign file \"%s\" is too large.",cmpgn_fname);
        return false;
    }
    char* buf = (char*)LbMemoryAlloc(len + 256);
    if (buf == NULL)
      return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    if (result)
    {
        result = parse_campaign_common_blocks(campgn, buf, len);
        if (!result)
          WARNMSG("Parsing campaign file \"%s\" common blocks failed.",cmpgn_fname);
    }
    if ((result) && ((flags & CnfLd_ListOnly) == 0)) // This block doesn't have anything we'd like to parse in list mode
    {
        result = parse_campaign_strings_blocks(campgn, buf, len);
        if (!result)
          WARNMSG("Parsing campaign file \"%s\" strings block failed.",cmpgn_fname);
    }
    if ((result) && ((flags & CnfLd_ListOnly) == 0) && (fgroup == FGrp_Campgn))
    {
        result = parse_campaign_speech_blocks(campgn, buf, len);
        if (!result)
          WARNMSG("Parsing campaign file \"%s\" speech block failed.",cmpgn_fname);
    }
    if ((result) && ((flags & CnfLd_ListOnly) == 0) && (fgroup == FGrp_Campgn))
    {
        result = parse_campaign_map_blocks(campgn, buf, len);
        if (!result)
          WARNMSG("Parsing campaign file \"%s\" map blocks failed.",cmpgn_fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    if ((flags & CnfLd_ListOnly) == 0)
    {
        setup_campaign_strings_data(campgn);
        if (fgroup == FGrp_Campgn) {
            setup_campaign_credits_data(campgn);
        }
    }
    if (result && fgroup == FGrp_Campgn)
        return (campgn->single_levels_count > 0) || (campgn->multi_levels_count > 0);
    if (result && fgroup == FGrp_VarLevels){
        return (true);
    }
    return false;
}

TbBool change_campaign(const char *cmpgn_fname)
{
    TbBool result;
    SYNCDBG(8,"Starting");
    if ((campaign.fname[0] != '\0') && (strcasecmp(campaign.fname,cmpgn_fname) == 0))
        return true;
    free_campaign(&campaign);
    // Determine type of campaign (currently campaign and mappack)
    short fgroup = FGrp_Campgn; //use this as a default
    if (is_campaign_in_list(cmpgn_fname, &mappacks_list)) // check if this is a map pack CFG file
        fgroup = FGrp_VarLevels;
    if ((cmpgn_fname != NULL) && (cmpgn_fname[0] != '\0'))
        result = load_campaign(cmpgn_fname,&campaign,CnfLd_Standard, fgroup);
    else
        result = load_campaign(keeper_campaign_file,&campaign,CnfLd_Standard, FGrp_Campgn);
    // Configs which may change within a level should be initialized outside
    //load_computer_player_config(CnfLd_Standard);
    //load_stats_files();
    //check_and_auto_fix_stats();
    // Make sure all additional levels are loaded
    //   Only the original campaign need to list the multiplayer levels 
    //   (until there are multiplayer mappacks) as all multi maps go on 
    //   the same "campaign" screen (and need to be in the same list to do so)
    if (strcasecmp(campaign.fname,keeper_campaign_file) == 0)
    {
        find_and_load_lof_files();
    }
    if (fgroup == FGrp_VarLevels)
    {
        find_and_load_lif_files();
    }
    load_or_create_high_score_table();
    // Update GUI arrays to new config
    update_room_tab_to_config();
    update_trap_tab_to_config();
    update_powers_tab_to_config();
    return result;
}

TbBool is_campaign_loaded(void)
{
    if (campaign.fname[0]=='\0')
        return false;
    return (campaign.single_levels_count > 0) || (campaign.multi_levels_count > 0) || (campaign.freeplay_levels_count > 0);
}

/**
 * Initializes list of campaigns, creating given number of empty list entries.
 */
TbBool init_campaigns_list_entries(struct CampaignsList *clist, long num_entries)
{
    if (clist->items != NULL)
        LbMemoryFree(clist->items);
    clist->items = (struct GameCampaign *)LbMemoryAlloc(num_entries*sizeof(struct GameCampaign));
    if (clist->items == NULL)
    {
        WARNMSG("Can't allocate memory for GameCampaigns list.");
        clist->items_count = 0;
        clist->items_num = 0;
        return false;
    }
    clist->items_count = num_entries;
    clist->items_num = 0;
    for (long i = 0; i < num_entries; i++)
        clear_campaign(&clist->items[i]);
    return true;
}

/**
 * Allocates more items in list of campaigns, adding given number of empty list entries.
 */
TbBool grow_campaigns_list_entries(struct CampaignsList *clist, long add_entries)
{
    long i = clist->items_count;
    long num_entries = clist->items_count + add_entries;
    clist->items = (struct GameCampaign *)LbMemoryGrow(clist->items, num_entries*sizeof(struct GameCampaign));
    if (clist->items == NULL)
    {
        WARNMSG("Can't enlarge memory for GameCampaigns list.");
        clist->items_count = 0;
        return false;
    }
    clist->items_count = num_entries;
    while (i < num_entries)
    {
        clear_campaign(&clist->items[i]);
        i++;
    }
    return true;
}

/**
 * Frees memory allocated for list of campaigns.
 */
TbBool free_campaigns_list_entries(struct CampaignsList *clist)
{
  if (clist->items != NULL)
    LbMemoryFree(clist->items);
  clist->items = NULL;
  clist->items_count = 0;
  return true;
}

TbBool load_campaign_to_list(const char *cmpgn_fname,struct CampaignsList *clist, short fgroup)
{
    if (clist->items_num >= clist->items_count)
      grow_campaigns_list_entries(clist, CAMPAIGNS_LIST_GROW_DELTA);
    if (clist->items_num >= clist->items_count)
      return false;
    struct GameCampaign* campgn = &clist->items[clist->items_num];
    if (load_campaign(cmpgn_fname,campgn,CnfLd_ListOnly, fgroup))
    {
        switch(fgroup)
        {
         case FGrp_VarLevels:
                if (check_lif_files_in_mappack(campgn)) { // if this returns false, then the map pack is "empty"
                    clist->items_num++;
                    return true;
                }
            break;
         case FGrp_Campgn:
         default:
             if (campgn->single_levels_count > 0)
            {
                clist->items_num++;
                return true;
            }
            break;
        }
    }
    return false;
}

TbBool swap_campaigns_in_list(struct CampaignsList *clist, int idx1, int idx2)
{
    if ((idx1 < 0) || (idx1 >= clist->items_num) || (idx2 < 0) || (idx2 >= clist->items_num))
      return false;
    struct GameCampaign campbuf;
    LbMemoryCopy(&campbuf, &clist->items[idx1], sizeof(struct GameCampaign));
    LbMemoryCopy(&clist->items[idx1],&clist->items[idx2],sizeof(struct GameCampaign));
    LbMemoryCopy(&clist->items[idx2],&campbuf,sizeof(struct GameCampaign));
    return true;
}

/** Simple quick sort algorithm implementation.
 *  Not very optimized, but we do not expect thousands of campaigns.
 *
 * @param clist
 * @param beg
 * @param end
 */
void sort_campaigns_quicksort(struct CampaignsList *clist, int beg, int end)
{
  if (end > beg + 1)
  {
      int l = beg + 1;
      int r = end;
      struct GameCampaign* campiv = &clist->items[beg];
      while (l < r)
      {
          if (strcasecmp(clist->items[l].name, campiv->name) <= 0)
          {
              l++;
          } else
        {
            swap_campaigns_in_list(clist, l, --r);
        }
    }
    swap_campaigns_in_list(clist, --l, beg);
    sort_campaigns_quicksort(clist, beg, l);
    sort_campaigns_quicksort(clist, r, end);
  }
}

void sort_campaigns(struct CampaignsList *clist,const char *fname_first)
{
    int beg = 0;
    for (int i = 0; i < clist->items_num; i++)
    {
        if (strcasecmp(clist->items[i].fname,fname_first) == 0)
        {
            if (i > 0)
            {
                swap_campaigns_in_list(clist, 0, i);
            }
            beg++;
            break;
        }
    }
    sort_campaigns_quicksort(clist, beg, clist->items_num);
}

/**
 * Searches for campaign files and creates a list of campaigns.
 */
TbBool load_campaigns_list(void)
{
    init_campaigns_list_entries(&campaigns_list, CAMPAIGNS_LIST_GROW_DELTA);
    char* fname = prepare_file_path(FGrp_Campgn, "*.cfg"); // add campaigns
    struct TbFileFind fileinfo;
    int rc = LbFileFindFirst(fname, &fileinfo, 0x21u);
    long cnum_all = 0;
    long cnum_ok = 0;
    while (rc != -1)
    {
        if (load_campaign_to_list(fileinfo.Filename, &campaigns_list, FGrp_Campgn))
        {
            cnum_ok++;
        }
      rc = LbFileFindNext(&fileinfo);
      cnum_all++;
    }
    LbFileFindEnd(&fileinfo);
    SYNCDBG(0,"Found %d campaign files, properly loaded %d.",cnum_all,cnum_ok);
    sort_campaigns(&campaigns_list,keeper_campaign_file);
    return (campaigns_list.items_num > 0);
}

/**
 * Searches for map pack files and creates a list of map packs.
 */
TbBool load_mappacks_list(void)
{
    init_campaigns_list_entries(&mappacks_list, CAMPAIGNS_LIST_GROW_DELTA);
    char* fname = prepare_file_path(FGrp_VarLevels, "*.cfg"); // add map packs
    struct TbFileFind fileinfo;
    int rc = LbFileFindFirst(fname, &fileinfo, 0x21u);
    long cnum_all = 0;
    long cnum_ok = 0;
    while (rc != -1)
    {
        if (is_campaign_in_list(fileinfo.Filename, &campaigns_list))
        {
                WARNMSG("Couldn't load Map Pack \"%s\", as it is a duplicate of an existing Campaign.", fileinfo.Filename);
        }
        else if (load_campaign_to_list(fileinfo.Filename, &mappacks_list, FGrp_VarLevels))
        {
            cnum_ok++;
        }
      rc = LbFileFindNext(&fileinfo);
      cnum_all++;
    }
    LbFileFindEnd(&fileinfo);
    SYNCDBG(0,"Found %d map pack files, properly loaded %d.",cnum_all,cnum_ok);
    sort_campaigns(&mappacks_list,deeper_mappack_file);
    return (mappacks_list.items_num > 0);
}

TbBool is_campaign_in_list(const char *cmpgn_fname, struct CampaignsList *clist)
{
    if (clist->items == NULL || clist->items_num < 1)
    {
        return false;
    }
    for (int i = 0; i < clist->items_num; i++)
    {
        if (strcasecmp(clist->items[i].fname,cmpgn_fname) == 0)
        {
            return true;
        }
    }
    return false;
}

TbBool check_lif_files_in_mappack(struct GameCampaign *campgn)
{
    struct GameCampaign campbuf;
    LbMemoryCopy(&campbuf, &campaign, sizeof(struct GameCampaign));
    LbMemoryCopy(&campaign, campgn, sizeof(struct GameCampaign));
    find_and_load_lif_files();
    TbBool result  = (campaign.freeplay_levels_count != 0);
    if (!result) {
        // Could be either: no valid levels in LEVELS_LOCATION, no LEVELS_LOCATION specified, or LEVELS_LOCATION does not exist
        WARNMSG("Couldn't load Map Pack \"%s\", no .LIF files could be found.", campgn->fname); 
    }
    LbMemoryCopy(campgn, &campaign, sizeof(struct GameCampaign));
    LbMemoryCopy(&campaign, &campbuf, sizeof(struct GameCampaign));
    return result;
}

TbBool is_map_pack(void)
{
    if (campaign.fname[0]=='\0')
        return false;
    return (campaign.single_levels_count == 0) && (campaign.multi_levels_count == 0) && (campaign.freeplay_levels_count > 0);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
