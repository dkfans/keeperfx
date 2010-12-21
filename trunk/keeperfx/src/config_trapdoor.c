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
#include "thing_doors.h"

#include "keeperfx.hpp"

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
  {NULL,              0},
  };

const struct NamedCommand trapdoor_trap_commands[] = {
  {"NAME",            1},
  {"MANUFACTURELEVEL",2},
  {"MANUFACTUREREQUIRED",3},
  {"SHOTS",           4},
  {"TIMEBETWEENSHOTS",5},
  {"SELLINGVALUE",    6},
  {NULL,              0},
  };
/******************************************************************************/
struct TrapDoorConfig trapdoor_conf;
struct NamedCommand trap_desc[TRAPDOOR_ITEMS_MAX];
struct NamedCommand door_desc[TRAPDOOR_ITEMS_MAX];
/******************************************************************************/
TbBool parse_trapdoor_common_blocks(char *buf,long len,const char *config_textname)
{
  long pos;
  int k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  trapdoor_conf.trap_types_count = 1;
  trapdoor_conf.door_types_count = 1;
  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
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

TbBool parse_trapdoor_trap_blocks(char *buf,long len,const char *config_textname)
{
  struct ManfctrConfig *mconf;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the traps array
  int arr_size = sizeof(trapdoor_conf.trap_names)/sizeof(trapdoor_conf.trap_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(trapdoor_conf.trap_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < trapdoor_conf.trap_types_count)
    {
      trap_desc[i].name = trapdoor_conf.trap_names[i].text;
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
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"trap%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      continue;
    }
    mconf = &game.traps_config[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_trap_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,trapdoor_trap_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,trapdoor_conf.trap_names[i].text,COMMAND_WORD_LEN) <= 0)
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

TbBool parse_trapdoor_door_blocks(char *buf,long len,const char *config_textname)
{
  struct ManfctrConfig *mconf;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the doors array
  int arr_size = sizeof(trapdoor_conf.door_names)/sizeof(trapdoor_conf.door_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(trapdoor_conf.door_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < trapdoor_conf.door_types_count)
    {
      door_desc[i].name = trapdoor_conf.door_names[i].text;
      door_desc[i].num = i;
    } else
    {
      door_desc[i].name = NULL;
      door_desc[i].num = 0;
    }
  }
  arr_size = trapdoor_conf.door_types_count;
  // Load the file
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"door%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      continue;
    }
    mconf = &game.doors_config[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_door_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,trapdoor_door_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,trapdoor_conf.door_names[i].text,COMMAND_WORD_LEN) <= 0)
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

TbBool load_trapdoor_config(const char *conf_fname,unsigned short flags)
{
  static const char config_textname[] = "Traps and doors config";
  char *fname;
  char *buf;
  long len;
  TbBool result;
  SYNCDBG(0,"Reading %s file \"%s\".",config_textname,conf_fname);
  fname = prepare_file_path(FGrp_FxData,conf_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("%s file \"%s\" doesn't exist or is too small.",config_textname,conf_fname);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("%s file \"%s\" is too large.",config_textname,conf_fname);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  result = (len > 0);
  if (result)
  {
    result = parse_trapdoor_common_blocks(buf, len, config_textname);
    if (!result)
      WARNMSG("Parsing %s file \"%s\" common blocks failed.",config_textname,conf_fname);
  }
  if (result)
  {
    result = parse_trapdoor_trap_blocks(buf, len, config_textname);
    if (!result)
      WARNMSG("Parsing %s file \"%s\" trap blocks failed.",config_textname,conf_fname);
  }
  if (result)
  {
    result = parse_trapdoor_door_blocks(buf, len, config_textname);
    if (!result)
      WARNMSG("Parsing %s file \"%s\" door blocks failed.",config_textname,conf_fname);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return result;
}

/**
 * Returns Code Name (name to use in script file) of given door model.
 */
const char *door_code_name(long tngmodel)
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
const char *trap_code_name(long tngmodel)
{
    const char *name;
    name = get_conf_parameter_text(trap_desc,tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
