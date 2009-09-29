/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_magic.c
 *     Keeper and creature spells configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for magic spells.
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
#include "config_magic.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"

#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_magic_file[]="magic.cfg";

const struct NamedCommand magic_common_commands[] = {
  {"SPELLSCOUNT",     1},
  {"SHOTSCOUNT",      2},
  {"POWERCOUNT",      3},
  {NULL,              0},
  };

const struct NamedCommand magic_spell_commands[] = {
  {"NAME",            1},
  {"DURATION",        2},
  {NULL,              0},
  };

const struct NamedCommand magic_shot_commands[] = {
  {"NAME",            1},
  {"HEALTH",          2},
  {"DAMAGE",          3},
  {"SPEED",           4},
  {NULL,              0},
  };

const struct NamedCommand magic_power_commands[] = {
  {"NAME",            1},
  {"POWER",           2},
  {"COST",            3},
  {"TIME",            4},
  {NULL,              0},
  };

/******************************************************************************/
struct MagicConfig magic_conf;
struct NamedCommand spell_desc[MAGIC_ITEMS_MAX];
struct NamedCommand shot_desc[MAGIC_ITEMS_MAX];
struct NamedCommand power_desc[MAGIC_ITEMS_MAX];
/******************************************************************************/

TbBool parse_magic_common_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  magic_conf.spell_types_count = 1;
  magic_conf.shot_types_count = 1;
  magic_conf.power_types_count = 1;
  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in Magic config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,magic_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // SPELLSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= MAGIC_ITEMS_MAX))
            {
              magic_conf.spell_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Magic file.",
                get_conf_parameter_text(magic_common_commands,cmd_num),block_buf);
          }
          break;
      case 2: // SHOTSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= MAGIC_ITEMS_MAX))
            {
              magic_conf.shot_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Magic file.",
                get_conf_parameter_text(magic_common_commands,cmd_num),block_buf);
          }
          break;
      case 3: // POWERCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= MAGIC_ITEMS_MAX))
            {
              magic_conf.power_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Magic file.",
                get_conf_parameter_text(magic_common_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Magic file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return true;
}

TbBool parse_magic_spell_blocks(char *buf,long len)
{
  struct SpellConfig *splconf;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(magic_conf.spell_names)/sizeof(magic_conf.spell_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(magic_conf.spell_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < magic_conf.spell_types_count)
    {
      spell_desc[i].name = magic_conf.spell_names[i].text;
      spell_desc[i].num = i;
    } else
    {
      spell_desc[i].name = NULL;
      spell_desc[i].num = 0;
    }
  }
  arr_size = magic_conf.spell_types_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"spell%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Magic config file.",block_buf);
      continue;
    }
    splconf = &game.spells_config[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,magic_spell_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,magic_conf.spell_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_spell_commands,cmd_num),block_buf);
            break;
          }
          n++;
          break;
      case 2: // DURATION
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            splconf->duration = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_spell_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Magic file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return true;
}

TbBool parse_magic_shot_blocks(char *buf,long len)
{
  struct ShotStats *shotstat;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(magic_conf.shot_names)/sizeof(magic_conf.shot_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(magic_conf.shot_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < magic_conf.shot_types_count)
    {
      shot_desc[i].name = magic_conf.shot_names[i].text;
      shot_desc[i].num = i;
    } else
    {
      shot_desc[i].name = NULL;
      shot_desc[i].num = 0;
    }
  }
  arr_size = magic_conf.shot_types_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"shot%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Magic config file.",block_buf);
      continue;
    }
    shotstat = &shot_stats[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,magic_shot_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,magic_conf.shot_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_shot_commands,cmd_num),block_buf);
            break;
          }
          n++;
          break;
      case 2: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            shotstat->health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_spell_commands,cmd_num),block_buf);
          }
          break;
      case 3: // DAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            shotstat->damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_spell_commands,cmd_num),block_buf);
          }
          break;
      case 4: // SPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            shotstat->speed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_spell_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Magic file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return true;
}

TbBool parse_magic_power_blocks(char *buf,long len)
{
  struct MagicStats *magstat;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(magic_conf.power_names)/sizeof(magic_conf.power_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(magic_conf.power_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < magic_conf.power_types_count)
    {
      power_desc[i].name = magic_conf.power_names[i].text;
      power_desc[i].num = i;
    } else
    {
      power_desc[i].name = NULL;
      power_desc[i].num = 0;
    }
  }
  arr_size = magic_conf.power_types_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"power%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Magic config file.",block_buf);
      continue;
    }
    magstat = &game.magic_stats[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,magic_power_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,magic_conf.power_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_power_commands,cmd_num),block_buf);
            break;
          }
          break;
      case 2: // POWER
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (n >= MAGIC_OVERCHARGE_LEVELS)
            {
              CONFWRNLOG("Too many \"%s\" parameters in [%s] block of Magic config file.",
              get_conf_parameter_text(magic_power_commands,cmd_num),block_buf);
              break;
            }
            magstat->power[n] = k;
            n++;
          }
          if (n < MAGIC_OVERCHARGE_LEVELS)
          {
            CONFWRNLOG("Couldn't read all \"%s\" parameters in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_power_commands,cmd_num),block_buf);
          }
          break;
      case 3: // COST
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (n >= MAGIC_OVERCHARGE_LEVELS)
            {
              CONFWRNLOG("Too many \"%s\" parameters in [%s] block of Magic config file.",
              get_conf_parameter_text(magic_power_commands,cmd_num),block_buf);
              break;
            }
            magstat->cost[n] = k;
            n++;
          }
          if (n < MAGIC_OVERCHARGE_LEVELS)
          {
            CONFWRNLOG("Couldn't read all \"%s\" parameters in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_power_commands,cmd_num),block_buf);
          }
          break;
      case 4: // TIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            magstat->time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Magic config file.",
            get_conf_parameter_text(magic_power_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Magic file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return true;
}

TbBool load_magic_config(const char *conf_fname,unsigned short flags)
{
  char *fname;
  char *buf;
  long len,pos;
  int cmd_num;
  TbBool result;
  SYNCDBG(0,"Reading Magic config file \"%s\".",conf_fname);
  fname = prepare_file_path(FGrp_FxData,conf_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("Magic config file \"%s\" doesn't exist or is too small.",conf_fname);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("Magic config file \"%s\" is too large.",conf_fname);
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
    result = parse_magic_common_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing Magic file \"%s\" common blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_magic_spell_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing Magic file \"%s\" spell blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_magic_shot_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing Magic file \"%s\" shot blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_magic_power_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing Magic file \"%s\" power blocks failed.",conf_fname);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return result;
}

/*
 * Zeroes all the costs for all spells.
 */
TbBool make_all_powers_free(void)
{
  struct MagicStats *magstat;
  long i,n;
  for (i=0; i < magic_conf.power_types_count; i++)
  {
    magstat = &game.magic_stats[i];
    for (n=0; n < MAGIC_OVERCHARGE_LEVELS; n++)
      magstat->cost[n] = 0;
  }
  return true;
}

/*
 * Makes all keeper spells to be available to research.
 */
TbBool make_all_powers_researchable(long plyr_idx)
{
  struct Dungeon *dungeon;
  long i;
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  for (i=0; i < KEEPER_SPELLS_COUNT; i++)
  {
    dungeon->magic_resrchable[i] = 1;
  }
  return true;
}

/**
 * Sets power availability state.
 */
TbBool set_power_available(long plyr_idx, long spl_idx, long resrch, long avail)
{
  struct Dungeon *dungeon;
  SYNCDBG(8,"Starting for spell %ld, player %ld, state %ld,%ld",spl_idx,plyr_idx,resrch,avail);
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  dungeon->magic_resrchable[spl_idx] = resrch;
  if (avail != 0)
    add_spell_to_player(spl_idx, plyr_idx);
  else
    dungeon->magic_level[spl_idx] = avail;
  return true;
}

/*
 * Makes all the powers, which are researchable, to be instantly available.
 */
TbBool make_available_all_researchable_powers(long plyr_idx)
{
  struct Dungeon *dungeon;
  TbBool ret;
  long i;
  SYNCDBG(0,"Starting");
  ret = true;
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  for (i=0; i < KEEPER_SPELLS_COUNT; i++)
  {
    if (dungeon->magic_resrchable[i])
    {
      add_spell_to_player(i, plyr_idx);
    }
  }
  return ret;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
