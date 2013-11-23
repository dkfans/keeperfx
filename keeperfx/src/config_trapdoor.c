/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_trapdoor.c
 *     Slabs, rooms, traps and doors configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for trap and door elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_trapdoor.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_strings.h"
#include "thing_doors.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_trapdoor_file[]="trapdoor.cfg";

const struct NamedCommand trapdoor_common_commands[] = {
  {"TRAPSCOUNT",      1},
  {"DOORSCOUNT",      2},
  {NULL,              0},
  };

const struct NamedCommand trapdoor_door_commands[] = {
  {"NAME",            1},
  {"MANUFACTURELEVEL",2},
  {"MANUFACTUREREQUIRED",3},
  {"SELLINGVALUE",    4},
  {"HEALTH",          5},
  {"NAMETEXTID",      6},
  {"TOOLTIPTEXTID",   7},
  {NULL,              0},
  };

const struct NamedCommand trapdoor_trap_commands[] = {
  {"NAME",            1},
  {"MANUFACTURELEVEL",2},
  {"MANUFACTUREREQUIRED",3},
  {"SHOTS",           4},
  {"TIMEBETWEENSHOTS",5},
  {"SELLINGVALUE",    6},
  {"NAMETEXTID",      7},
  {"TOOLTIPTEXTID",   8},
  {NULL,              0},
  };
/******************************************************************************/
struct TrapDoorConfig trapdoor_conf;
struct NamedCommand trap_desc[TRAPDOOR_ITEMS_MAX];
struct NamedCommand door_desc[TRAPDOOR_ITEMS_MAX];
/******************************************************************************/
struct TrapConfigStats *get_trap_model_stats(int tngmodel)
{
    if ((tngmodel < 0) || (tngmodel >= trapdoor_conf.trap_types_count))
        return &trapdoor_conf.trap_cfgstats[0];
    return &trapdoor_conf.trap_cfgstats[tngmodel];
}

struct DoorConfigStats *get_door_model_stats(int tngmodel)
{
    if ((tngmodel < 0) || (tngmodel >= trapdoor_conf.door_types_count))
        return &trapdoor_conf.door_cfgstats[0];
    return &trapdoor_conf.door_cfgstats[tngmodel];
}

TbBool parse_trapdoor_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    SYNCDBG(19,"Starting");
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        trapdoor_conf.trap_types_count = 1;
        trapdoor_conf.door_types_count = 1;
    }
    // Find the block
    sprintf(block_buf,"common");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,trapdoor_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // TRAPSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= TRAPDOOR_ITEMS_MAX))
              {
                trapdoor_conf.trap_types_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 2: // DOORSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= TRAPDOOR_ITEMS_MAX))
              {
                trapdoor_conf.door_types_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
    return true;
}

TbBool parse_trapdoor_trap_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct ManfctrConfig *mconf;
  struct TrapConfigStats *trapst;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  SYNCDBG(19,"Starting");
  // Initialize the traps array
  int arr_size;
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      arr_size = sizeof(trapdoor_conf.trap_cfgstats)/sizeof(trapdoor_conf.trap_cfgstats[0]);
      for (i=0; i < arr_size; i++)
      {
          trapst = &trapdoor_conf.trap_cfgstats[i];
          LbMemorySet(trapst->code_name, 0, COMMAND_WORD_LEN);
          trapst->name_stridx = GUIStr_Empty;
          trapst->tooltip_stridx = GUIStr_Empty;
          if (i < trapdoor_conf.trap_types_count)
          {
              trap_desc[i].name = trapst->code_name;
              trap_desc[i].num = i;
          } else
          {
              trap_desc[i].name = NULL;
              trap_desc[i].num = 0;
          }
      }
      arr_size = trapdoor_conf.trap_types_count;
      for (i=0; i < arr_size; i++)
      {
          mconf = &game.traps_config[i];
          mconf->manufct_level = 0;
          mconf->manufct_required = 0;
          mconf->shots = 0;
          mconf->shots_delay = 0;
          mconf->selling_value = 0;
      }
  }
  // Parse every numbered block within range
  arr_size = trapdoor_conf.trap_types_count;
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"trap%d",i);
    SYNCDBG(19,"Block [%s]",block_buf);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0) {
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
            return false;
        }
        continue;
    }
    mconf = &game.traps_config[i];
    trapst = &trapdoor_conf.trap_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_trap_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,trapdoor_trap_commands);
      SYNCDBG(19,"Command %s",COMMAND_TEXT(cmd_num));
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      if ((flags & CnfLd_ListOnly) != 0) {
          // In "List only" mode, accept only name command
          if (cmd_num > 1) {
              cmd_num = 0;
          }
      }
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,trapst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
            break;
          }
          break;
      case 2: // MANUFACTURELEVEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->manufct_level = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // MANUFACTUREREQUIRED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->manufct_required = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // SHOTS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->shots = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // TIMEBETWEENSHOTS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->shots_delay = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 6: // SELLINGVALUE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->selling_value = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // NAMETEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
                trapst->name_stridx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // TOOLTIPTEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
                trapst->tooltip_stridx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
  }
  return true;
}

TbBool parse_trapdoor_door_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct ManfctrConfig *mconf;
  struct DoorConfigStats *doorst;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  SYNCDBG(19,"Starting");
  // Initialize the doors array
  int arr_size;
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      arr_size = sizeof(trapdoor_conf.door_cfgstats)/sizeof(trapdoor_conf.door_cfgstats[0]);
      for (i=0; i < arr_size; i++)
      {
          doorst = &trapdoor_conf.door_cfgstats[i];
          LbMemorySet(doorst->code_name, 0, COMMAND_WORD_LEN);
          doorst->name_stridx = GUIStr_Empty;
          doorst->tooltip_stridx = GUIStr_Empty;
          if (i < trapdoor_conf.door_types_count)
          {
              door_desc[i].name = doorst->code_name;
              door_desc[i].num = i;
          } else
          {
              door_desc[i].name = NULL;
              door_desc[i].num = 0;
          }
      }
  }
  // Parse every numbered block within range
  arr_size = trapdoor_conf.door_types_count;
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"door%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0) {
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
            return false;
        }
        continue;
    }
    mconf = &game.doors_config[i];
    doorst = &trapdoor_conf.door_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_door_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,trapdoor_door_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      if ((flags & CnfLd_ListOnly) != 0) {
          // In "List only" mode, accept only name command
          if (cmd_num > 1) {
              cmd_num = 0;
          }
      }
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,doorst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
            break;
          }
          break;
      case 2: // MANUFACTURELEVEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->manufct_level = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;

      case 3: // MANUFACTUREREQUIRED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->manufct_required = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // SELLINGVALUE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            mconf->selling_value = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            door_stats[i][0].health = k;
            door_stats[i][1].health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 6: // NAMETEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
                doorst->name_stridx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // TOOLTIPTEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
                doorst->tooltip_stridx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
  }
  return true;
}

TbBool load_trapdoor_config_file(const char *textname, const char *fname, unsigned short flags)
{
    char *buf;
    long len;
    TbBool result;
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    if (len > MAX_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" is too large.",textname,fname);
        return false;
    }
    buf = (char *)LbMemoryAlloc(len+256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    result = (len > 0);
    // Parse blocks of the config file
    if (result)
    {
        result = parse_trapdoor_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_trapdoor_trap_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" trap blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_trapdoor_door_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" door blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    SYNCDBG(19,"Done");
    return result;
}

TbBool load_trapdoor_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global traps and doors config";
    static const char config_campgn_textname[] = "campaign traps and doors config";
    char *fname;
    TbBool result;
    fname = prepare_file_path(FGrp_FxData,conf_fname);
    result = load_trapdoor_config_file(config_global_textname,fname,flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_trapdoor_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given door model.
 */
const char *door_code_name(int tngmodel)
{
    const char *name;
    name = get_conf_parameter_text(door_desc,tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given trap model.
 */
const char *trap_code_name(int tngmodel)
{
    const char *name;
    name = get_conf_parameter_text(trap_desc,tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the door model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the door model if found, otherwise -1
 */
int door_model_id(const char * code_name)
{
    int i;

    for (i = 0; i < trapdoor_conf.door_types_count; ++i) {
        if (strncasecmp(trapdoor_conf.door_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Returns the trap model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the trap model if found, otherwise -1
 */
int trap_model_id(const char * code_name)
{
    int i;

    for (i = 0; i < trapdoor_conf.trap_types_count; ++i) {
        if (strncasecmp(trapdoor_conf.trap_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Returns if the trap can be placed by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if map position is on correct spot.
 */
TbBool is_trap_placeable(PlayerNumber plyr_idx, long trap_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to place traps
    if (dungeon->dnheart_idx <= 0) {
        return false;
    }
    if ((trap_idx <= 0) || (trap_idx >= TRAP_TYPES_COUNT)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)trap_idx, (int)plyr_idx);
        return false;
    }
    if (dungeon->trap_placeable[trap_idx]) {
        return true;
    }
    return false;
}

/**
 * Returns if the trap can be manufactured by a player.
 * Checks only if it's set as buildable in level script.
 * Doesn't check if player has workshop or workforce for the task.
 */
TbBool is_trap_buildable(PlayerNumber plyr_idx, long trap_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (dungeon->dnheart_idx <= 0) {
        return false;
    }
    if ((trap_idx <= 0) || (trap_idx >= TRAP_TYPES_COUNT)) {
        ERRORLOG("Incorrect trap %ld (player %ld)",trap_idx, plyr_idx);
        return false;
    }
    if (dungeon->trap_buildable[trap_idx] > 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the door can be placed by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if map position is on correct spot.
 */
TbBool is_door_placeable(PlayerNumber plyr_idx, long door_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to place doors
    if (dungeon->dnheart_idx <= 0) {
        return false;
    }
    if ((door_idx <= 0) || (door_idx >= DOOR_TYPES_COUNT)) {
        ERRORLOG("Incorrect door %ld (player %ld)",door_idx, plyr_idx);
        return false;
    }
    if (dungeon->door_placeable[door_idx]) {
        return true;
    }
    return false;
}

/**
 * Returns if the door can be manufactured by a player.
 * Checks only if it's set as buildable in level script.
 * Doesn't check if player has workshop or workforce for the task.
 */
TbBool is_door_buildable(PlayerNumber plyr_idx, long door_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (dungeon->dnheart_idx <= 0) {
        return false;
    }
    if ((door_idx <= 0) || (door_idx >= DOOR_TYPES_COUNT)) {
        ERRORLOG("Incorrect door %ld (player %ld)",door_idx, plyr_idx);
        return false;
    }
    if (dungeon->door_buildable[door_idx] > 0) {
        return true;
    }
    return false;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
