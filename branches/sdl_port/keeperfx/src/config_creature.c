/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_creature.c
 *     Creature names, appearance and parameters configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for creatures list.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 03 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_creature.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "globals.h"
#include "config.h"
#include "thing_doors.h"
#include "thing_creature.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_creaturetp_file[]="creature.cfg";

const struct NamedCommand creaturetype_common_commands[] = {
  {"CREATURES",              1},
  {"INSTANCESCOUNT",         2},
  {"JOBSCOUNT",              3},
  {"ANGERJOBSCOUNT",         4},
  {"ATTACKPREFERENCESCOUNT", 5},
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_instance_commands[] = {
  {"NAME",            1},
  {"TIME",            2},
  {"ACTIONTIME",      3},
  {"RESETTIME",       4},
  {"FPTIME",          5},
  {"FPACTIONTIME",    6},
  {"FPRESETTIME",     7},
  {"FORCEVISIBILITY", 8},
  {NULL,              0},
  };

const struct NamedCommand angerjob_desc[] = {
  {"KILL_CREATURES",  1},
  {"DESTROY_ROOMS",   2},
  {"LEAVE_DUNGEON",   4},
  {"STEAL_GOLD",      8},
  {"DAMAGE_WALLS",   16},
  {"MAD_PSYCHO",     32},
  {"PERSUADE",       64},
  {"JOIN_ENEMY",    128},
  {"UNKNOWN1",      256},
  {NULL,              0},
  };

const struct NamedCommand creaturejob_desc[] = {
  {"NULL",               0},
  {"TUNNEL",             1},
  {"DIG",                2},
  {"RESEARCH",           4},
  {"TRAIN",              8},
  {"MANUFACTURE",       16},
  {"SCAVENGE",          32},
  {"KINKY_TORTURE",     64},
  {"FIGHT",            128},
  {"SEEK_THE_ENEMY",   256},
  {"GUARD",            512},
  {"GROUP",           1024},
  {"BARRACK",         2048},
  {"TEMPLE",          4096},
  {"FREEZE_PRISONERS",8192},
  {"EXPLORE",        16384},
  {NULL,                 0},
  };

const struct NamedCommand attackpref_desc[] = {
  {"MELEE",              1},
  {"RANGED",             2},
  {NULL,                 0},
  };

struct CreatureData creature_data[] = {
  {0x00,  0, 201},
  {0x05, 57, 277},
  {0x01, 58, 275},
  {0x01, 59, 285},
  {0x01, 60, 286},
  {0x01, 61, 279},
  {0x01, 62, 276},
  {0x01, 63, 547},
  {0x01, 64, 546},
  {0x05, 65, 283},
  {0x01, 66, 284},
  {0x01, 67, 258},
  {0x01, 68, 281},
  {0x01, 69, 282},
  {0x01, 70, 267},
  {0x01, 71, 266},
  {0x01, 72, 261},
  {0x15, 73, 268},
  {0x02, 74, 262},
  {0x02, 75, 264},
  {0x02, 76, 272},
  {0x02, 77, 263},
  {0x02, 78, 273},
  {0x02, 79, 259},
  {0x02, 80, 260},
  {0x02, 81, 274},
  {0x02, 82, 265},
  {0x02, 83, 270},
  {0x02, 84, 271},
  {0x02, 85, 269},
  {0x01,126, 278},
  {0x00,  0, 201},
  };

/******************************************************************************/
struct CreatureConfig crtr_conf;
struct NamedCommand creature_desc[CREATURE_TYPES_MAX];
struct NamedCommand instance_desc[INSTANCE_TYPES_MAX];
/******************************************************************************/
/*
 * Returns CreatureStats of given creature model.
 */
struct CreatureStats *creature_stats_get(long crstat_idx)
{
  if ((crstat_idx < 1) || (crstat_idx >= CREATURE_TYPES_COUNT))
    return &game.creature_stats[0];
  return &game.creature_stats[crstat_idx];
}

/*
 * Returns CreatureStats assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureStats *creature_stats_get_from_thing(const struct Thing *thing)
{
  if ((thing->model < 1) || (thing->model >= CREATURE_TYPES_COUNT))
    return &game.creature_stats[0];
  return &game.creature_stats[thing->model];
}

/*
 * Returns if given CreatureStats pointer is incorrect.
 */
TbBool creature_stats_invalid(const struct CreatureStats *crstat)
{
  return (crstat <= &game.creature_stats[0]) || (crstat == NULL);
}

/*
 * Returns CreatureData of given creature model.
 */
struct CreatureData *creature_data_get(long crstat_idx)
{
  if ((crstat_idx < 1) || (crstat_idx >= CREATURE_TYPES_COUNT))
    return &creature_data[0];
  return &creature_data[crstat_idx];
}

/*
 * Returns CreatureData assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureData *creature_data_get_from_thing(const struct Thing *thing)
{
  if ((thing->model < 1) || (thing->model >= CREATURE_TYPES_COUNT))
    return &creature_data[0];
  return &creature_data[thing->model];
}

/*
 * Returns Code Name (name to use in script file) of given creature model.
 */
const char *creature_code_name(long crmodel)
{
    const char *name;
    return get_conf_parameter_text(creature_desc,crmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

TbBool parse_creaturetypes_common_blocks(char *buf,long len)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  crtr_conf.model_count = 1;
  crtr_conf.instance_count = 1;
  crtr_conf.job_count = 1;
  crtr_conf.angerjob_count = 1;
  crtr_conf.attackpref_count = 1;
  crtr_conf.special_digger_good = 0;
  crtr_conf.special_digger_evil = 0;
  k = sizeof(crtr_conf.model)/sizeof(crtr_conf.model[0]);
  for (i=0; i < k; i++)
  {
    LbMemorySet(crtr_conf.model[i].name, 0, COMMAND_WORD_LEN);
  }
  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in Creature config file.",block_buf);
    return false;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // CREATURES
          while (get_conf_parameter_single(buf,&pos,len,crtr_conf.model[n].name,COMMAND_WORD_LEN) > 0)
          {
            creature_desc[n].name = crtr_conf.model[n].name;
            creature_desc[n].num = n+1;
            n++;
            if (n >= CREATURE_TYPES_MAX)
            {
              CONFWRNLOG("Too many spiecies defined with \"%s\" in [%s] block of Creature file.",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
              break;
            }
          }
          crtr_conf.model_count = n;
          while (n < CREATURE_TYPES_MAX)
          {
            creature_desc[n].name = NULL;
            creature_desc[n].num = 0;
            n++;
          }
          break;
      case 2: // INSTANCESCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
            {
              crtr_conf.instance_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Creature file.",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          break;
      case 3: // JOBSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
            {
              crtr_conf.job_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Creature file.",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          break;
      case 4: // ANGERJOBSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
            {
              crtr_conf.angerjob_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Creature file.",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          break;
      case 5: // ATTACKPREFERENCESCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
            {
              crtr_conf.attackpref_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of Creature file.",
                get_conf_parameter_text(creaturetype_common_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Creature file.",cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  if (crtr_conf.model_count < 1)
  {
      WARNLOG("No creature spiecies defined in [%s] block of Creature file.",block_buf);
  }
  return true;
}

TbBool parse_creaturetype_instance_blocks(char *buf,long len)
{
  struct InstanceInfo *inst_inf;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  int arr_size;
  // Initialize the array
  arr_size = sizeof(crtr_conf.instance_names)/sizeof(crtr_conf.instance_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(crtr_conf.instance_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < crtr_conf.instance_count)
    {
      instance_desc[i].name = crtr_conf.instance_names[i].text;
      instance_desc[i].num = i;
    } else
    {
      instance_desc[i].name = NULL;
      instance_desc[i].num = 0;
    }
  }
  arr_size = crtr_conf.instance_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"instance%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Creature Types config file.",block_buf);
      continue;
    }
    inst_inf = &instance_info[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_instance_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,crtr_conf.instance_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
            break;
          }
          n++;
          break;
      case 2: // TIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 3: // ACTIONTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->action_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 4: // RESETTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->reset_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 5: // FPTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->fp_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 6: // FPACTIONTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->fp_action_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 7: // FPRESETTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->fp_reset_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 8: // FORCEVISIBILITY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            inst_inf->force_visibility = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of Creature Types config file.",
            get_conf_parameter_text(creaturetype_instance_commands,cmd_num),block_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of Creature Types file.",
              cmd_num,block_buf);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return true;
}

TbBool load_creaturetypes_config(const char *conf_fname,unsigned short flags)
{
  char *fname;
  char *buf;
  long len;
  TbBool result;
  SYNCDBG(0,"Reading Creature config file \"%s\".",conf_fname);
  fname = prepare_file_path(FGrp_FxData,conf_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("Creature config file \"%s\" doesn't exist or is too small.",conf_fname);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("Creature config file \"%s\" is too large.",conf_fname);
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
    result = parse_creaturetypes_common_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing Creature file \"%s\" common blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_creaturetype_instance_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing Creature file \"%s\" instance blocks failed.",conf_fname);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return result;
}

long calculate_correct_creature_maxspeed(const struct Thing *thing)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  struct Dungeon *dungeon;
  long speed;
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  speed = crstat->base_speed;
  if (cctrl->field_21)
    speed *= 2;
  if ((cctrl->field_AC & 0x02) != 0)
    speed *= 2;
  if ((cctrl->field_AC & 0x01) != 0)
    speed /= 2;
  if (game.neutral_player_num != thing->owner)
  {
    dungeon = get_dungeon(thing->owner);
    if (dungeon->field_1420[thing->model])
      speed = 5 * speed / 4;
    if (dungeon->field_888)
      speed = 5 * speed / 4;
  }
  return speed;
}

unsigned short get_creature_model_flags(const struct Thing *thing)
{
  return crtr_conf.model[thing->model%CREATURE_TYPES_MAX].model_flags;
}

/**
 * Sets creature availability state.
 */
TbBool set_creature_available(long plyr_idx, long crtr_model, long can_be_avail, long force_avail)
{
    struct Dungeon *dungeon;
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return false;
    dungeon->creature_allowed[crtr_model] = can_be_avail;
    dungeon->creature_enabled[crtr_model] = force_avail;
    return true;
}

long get_players_special_digger_breed(long plyr_idx)
{
    long breed;
    if (plyr_idx == hero_player_number)
    {
        breed = crtr_conf.special_digger_good;
        if (breed == 0)
        {
            WARNLOG("Heroes have no digger breed!");
            breed = crtr_conf.special_digger_evil;
        }
    } else
    {
        breed = crtr_conf.special_digger_evil;
        if (breed == 0)
        {
            WARNLOG("Keepers have no digger breed!");
            breed = crtr_conf.special_digger_good;
        }
    }
    return breed;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
