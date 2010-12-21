/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_crtrmodel.c
 *     Specific creature model configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for specific creatures.
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
#include "config_crtrmodel.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"
#include "config_campaigns.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_lenses.h"
#include "creature_control.h"
#include "creature_graphics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

const struct NamedCommand creatmodel_attributes_commands[] = {
  {"NAME",             1},
  {"HEALTH",           2},
  {"HEALREQUIRMENT",   3},
  {"HEALTHRESHOLD",    4},
  {"STRENGTH",         5},
  {"ARMOUR",           6},
  {"DEXTERITY",        7},
  {"FEAR",             8},
  {"DEFENCE",          9},
  {"LUCK",            10},
  {"RECOVERY",        11},
  {"HUNGERRATE",      12},
  {"HUNGERFILL",      13},
  {"LAIRSIZE",        14},
  {"HURTBYLAVA",      15},
  {"BASESPEED",       16},
  {"GOLDHOLD",        17},
  {"SIZE",            18},
  {"ATTACKPREFERENCE",19},
  {"PAY",             20},
  {"HEROVSKEEPERCOST",21},
  {"SLAPSTOKILL",     22},
  {"CREATURELOYALTY", 23},
  {"LOYALTYLEVEL",    24},
  {"DAMAGETOBOULDER", 25},
  {"THINGSIZE",       26},
  {"PROPERTIES",      27},
  {"NAMETEXTID",      28},
  {NULL,               0},
  };

const struct NamedCommand creatmodel_properties_commands[] = {
  {"BLEEDS",            1},
  {"UNAFFECTED_BY_WIND",2},
  {"IMMUNE_TO_GAS",     3},
  {"HUMANOID",          4},
  {"PISS_ON_DEAD",      5},
  {"FLYING",            7},
  {"SEE_INVISIBLE",     8},
  {"PASS_LOCKED_DOORS", 9},
  {"SPECIAL_DIGGER",   10},
  {"ARACHNID",         11},
  {"DIPTERA",          12},
  {"LORD",             13},
  {"SPECTATOR",        14},
  {"EVIL",             15},
  {"NEVER_CHICKENS",   16},
  {NULL,                0},
  };

const struct NamedCommand creatmodel_attraction_commands[] = {
  {"ENTRANCEROOM",       1},
  {"ROOMSLABSREQUIRED",  2},
  {"ENTRANCEFORCE",      3},
  {"SCAVENGEREQUIREMENT",4},
  {"TORTURETIME",        5},
  {NULL,                 0},
  };

const struct NamedCommand creatmodel_annoyance_commands[] = {
  {"EATFOOD",              1},
  {"WILLNOTDOJOB",         2},
  {"INHAND",               3},
  {"NOLAIR",               4},
  {"NOHATCHERY",           5},
  {"WOKENUP",              6},
  {"STANDINGONDEADENEMY",  7},
  {"SULKING",              8},
  {"NOSALARY",             9},
  {"SLAPPED",             10},
  {"STANDINGONDEADFRIEND",11},
  {"INTORTURE",           12},
  {"INTEMPLE",            13},
  {"SLEEPING",            14},
  {"GOTWAGE",             15},
  {"WINBATTLE",           16},
  {"UNTRAINED",           17},
  {"OTHERSLEAVING",       18},
  {"JOBSTRESS",           19},
  {"QUEUE",               20},
  {"LAIRENEMY",           21},
  {"ANNOYLEVEL",          22},
  {"ANGERJOBS",           23},
  {NULL,                   0},
  };

const struct NamedCommand creatmodel_senses_commands[] = {
  {"HEARING",              1},
  {"EYEHEIGHT",            2},
  {"FIELDOFVIEW",          3},
  {"EYEEFFECT",            4},
  {"MAXANGLECHANGE",       5},
  {NULL,                   0},
  };

const struct NamedCommand creatmodel_appearance_commands[] = {
  {"WALKINGANIMSPEED",     1},
  {"VISUALRANGE",          2},
  {NULL,                   0},
  };

const struct NamedCommand creatmodel_experience_commands[] = {
  {"POWERS",               1},
  {"POWERSLEVELREQUIRED",  2},
  {"LEVELSTRAINVALUES",    3},
  {"GROWUP",               4},
  {"SLEEPEXPERINCE",       5},
  {"EXPERIENCEFORHITTING", 6},
  {"REBIRTH",              7},
  {NULL,                   0},
  };

const struct NamedCommand creatmodel_jobs_commands[] = {
  {"PRIMARYJOBS",          1},
  {"SECONDARYJOBS",        2},
  {"NOTDOJOBS",            3},
  {"STRESSFULJOBS",        4},
  {"TRAININGVALUE",        5},
  {"TRAININGCOST",         6},
  {"SCAVENGEVALUE",        7},
  {"SCAVENGERCOST",        8},
  {"RESEARCHVALUE",        9},
  {"MANUFACTUREVALUE",    10},
  {"PARTNERTRAINING",     11},
  {NULL,                   0},
  };

/******************************************************************************/
TbBool parse_creaturemodel_attributes_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
  struct CreatureStats *crstat;
  struct CreatureModelConfig *crconf;
  long pos;
  int k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  crstat = creature_stats_get(crtr_model);
  crconf = &crtr_conf.model[crtr_model];
  if ((flags & CLd_AcceptPartial) == 0)
  {
      crstat->health = 1;
      crstat->heal_requirement = 1;
      crstat->heal_threshold = 1;
      crstat->strength = 0;
      crstat->armour = 0;
      crstat->dexterity = 0;
      crstat->fear = 32;
      crstat->defence = 0;
      crstat->luck = 0;
      crstat->recovery = 1;
      crstat->hunger_rate = 1;
      crstat->hunger_fill = 1;
      crstat->lair_size = 1;
      crstat->hurt_by_lava = 1;
      crstat->base_speed = 1;
      crstat->gold_hold = 100;
      crstat->size_xy = 1;
      crstat->size_yz = 1;
      crstat->attack_preference = 0;
      crstat->pay = 1;
      crstat->hero_vs_keeper_cost = 0;
      crstat->slaps_to_kill = 10;
      crstat->damage_to_boulder = 4;
      crstat->thing_size_xy = 128;
      crstat->thing_size_yz = 64;
      crstat->bleeds = false;
      crstat->affected_by_wind = true;
      crstat->immune_to_gas = false;
      crstat->humanoid_creature = false;
      crstat->piss_on_dead = false;
      crstat->flying = false;
      crstat->can_see_invisible = false;
      crstat->can_go_locked_doors = false;
      crconf->namestr_idx = 0;
      crconf->model_flags = 0;
  }
  // Find the block
  sprintf(block_buf,"attributes");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
      if ((flags & CLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_attributes_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,creatmodel_attributes_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          // Name is ignored - it was defined in creature.cfg
          break;
      case 2: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // HEALREQUIRMENT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->heal_requirement = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // HEALTHRESHOLD
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->heal_threshold = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // STRENGTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->strength = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 6: // ARMOUR
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->armour = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // DEXTERITY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->dexterity = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // FEAR
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->fear = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 9: // DEFENCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->defence = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 10: // LUCK
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->luck = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 11: // RECOVERY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->recovery = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 12: // HUNGERRATE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->hunger_rate = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 13: // HUNGERFILL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->hunger_fill = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 14: // LAIRSIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->lair_size = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 15: // HURTBYLAVA
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->hurt_by_lava = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 16: // BASESPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->base_speed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 17: // GOLDHOLD
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->gold_hold = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 18: // SIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->size_xy = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->size_yz = k;
            n++;
          }
          if (n < 2)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameters in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 19: // ATTACKPREFERENCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_id(attackpref_desc, word_buf);
            if (k > 0)
            {
              crstat->attack_preference = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 20: // PAY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->pay = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 21: // HEROVSKEEPERCOST
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->hero_vs_keeper_cost = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 22: // SLAPSTOKILL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->slaps_to_kill = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 23: // CREATURELOYALTY
          // Unused
          break;
      case 24: // LOYALTYLEVEL
          // Unused
          break;
      case 25: // DAMAGETOBOULDER
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->damage_to_boulder = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 26: // THINGSIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->thing_size_xy = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->thing_size_yz = k;
            n++;
          }
          if (n < 2)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameters in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 27: // PROPERTIES
          crstat->bleeds = false;
          crstat->affected_by_wind = true;
          crstat->immune_to_gas = false;
          crstat->humanoid_creature = false;
          crstat->piss_on_dead = false;
          crstat->flying = false;
          crstat->can_see_invisible = false;
          crstat->can_go_locked_doors = false;
          crconf->model_flags = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_id(creatmodel_properties_commands, word_buf);
            switch (k)
            {
            case 1: // BLEEDS
              crstat->bleeds = true;
              n++;
              break;
            case 2: // UNAFFECTED_BY_WIND
              crstat->affected_by_wind = false;
              n++;
              break;
            case 3: // IMMUNE_TO_GAS
              crstat->immune_to_gas = true;
              n++;
              break;
            case 4: // HUMANOID
              crstat->humanoid_creature = true;
              n++;
              break;
            case 5: // PISS_ON_DEAD
              crstat->piss_on_dead = true;
              n++;
              break;
            case 7: // FLYING
              crstat->flying = true;
              n++;
              break;
            case 8: // SEE_INVISIBLE
              crstat->can_see_invisible = true;
              n++;
              break;
            case 9: // PASS_LOCKED_DOORS
              crstat->can_go_locked_doors = true;
              n++;
              break;
            case 10: // SPECIAL_DIGGER
              crconf->model_flags |= MF_IsSpecDigger;
              n++;
              break;
            case 11: // ARACHNID
              crconf->model_flags |= MF_IsArachnid;
              n++;
              break;
            case 12: // DIPTERA
              crconf->model_flags |= MF_IsDiptera;
              n++;
              break;
            case 13: // LORD
              crconf->model_flags |= MF_IsLordOTLand;
              n++;
              break;
            case 14: // SPECTATOR
              crconf->model_flags |= MF_IsSpectator;
              n++;
              break;
            case 15: // EVIL
              crconf->model_flags |= MF_IsEvil;
              n++;
              break;
            case 16: // NEVER_CHICKENS
              crconf->model_flags |= MF_NeverChickens;
              n++;
              break;
            default:
              CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              break;
            }
          }
          break;
      case 28: // NAMETEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
              crconf->namestr_idx = k;
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
  // If the creature is a special breed, then update an attribute in CreatureConfig struct
  if ((crconf->model_flags & MF_IsSpecDigger) != 0)
  {
      if ((crconf->model_flags & MF_IsEvil) != 0)
          crtr_conf.special_digger_evil = crtr_model;
      else
          crtr_conf.special_digger_good = crtr_model;
  }
  if ((crconf->model_flags & MF_IsSpectator) != 0)
  {
      crtr_conf.spectator_breed = crtr_model;
  }
//  if (crstat->humanoid_creature) SYNCMSG("Creature is humanoid.");
  return true;
}

TbBool parse_creaturemodel_attraction_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
  struct CreatureStats *crstat;
  long pos;
  int k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  crstat = creature_stats_get(crtr_model);
  if ((flags & CLd_AcceptPartial) == 0)
  {
      for (n=0; n < 3; n++)
      {
        crstat->entrance_rooms[n] = 0;
        crstat->entrance_slabs_req[n] = 0;
      }
      crstat->entrance_force = 0;
      crstat->scavenge_require = 1;
      crstat->torture_time = 1;
  }
  // Find the block
  sprintf(block_buf,"attraction");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
      if ((flags & CLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_attraction_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,creatmodel_attraction_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // ENTRANCEROOM
          for (k=0; k < 3; k++)
            crstat->entrance_rooms[k] = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_id(room_desc, word_buf);
            if ((k >= 0) && (n < 3))
            {
              crstat->entrance_rooms[n] = k;
              n++;
            } else
            {
              CONFWRNLOG("Too many params, or incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
            }
          }
          break;
      case 2: // ROOMSLABSREQUIRED
          for (k=0; k < 3; k++)
            crstat->entrance_slabs_req[k] = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (n < 3)
            {
              crstat->entrance_slabs_req[n] = k;
              n++;
            } else
            {
              CONFWRNLOG("Too many parameters of \"%s\" in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
          }
          break;
      case 3: // ENTRANCEFORCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->entrance_force = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // SCAVENGEREQUIREMENT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->scavenge_require = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // TORTURETIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->torture_time = k;
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
  return true;
}

TbBool parse_creaturemodel_annoyance_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
    struct CreatureStats *crstat;
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    crstat = creature_stats_get(crtr_model);
    if ((flags & CLd_AcceptPartial) == 0)
    {
        crstat->annoy_eat_food = 0;
        crstat->annoy_will_not_do_job = 0;
        crstat->annoy_in_hand = 0;
        crstat->annoy_no_lair = 0;
        crstat->annoy_no_hatchery = 0;
        crstat->annoy_woken_up = 0;
        crstat->annoy_on_dead_enemy = 0;
        crstat->annoy_sulking = 0;
        crstat->annoy_no_salary = 0;
        crstat->annoy_slapped = 0;
        crstat->annoy_on_dead_friend = 0;
        crstat->annoy_in_torture = 0;
        crstat->annoy_in_temple = 0;
        crstat->annoy_sleeping = 0;
        crstat->annoy_got_wage = 0;
        crstat->annoy_win_battle = 0;
        crstat->annoy_untrained_time = 0;
        crstat->annoy_untrained = 0;
        crstat->annoy_others_leaving = 0;
        crstat->annoy_job_stress = 0;
        crstat->annoy_queue = 0;
        crstat->lair_enemy = 0;
        crstat->annoy_level = 0;
        crstat->jobs_anger = 0;
    }
    // Find the block
    sprintf(block_buf,"annoyance");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_annoyance_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,creatmodel_annoyance_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // EATFOOD
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_eat_food = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 2: // WILLNOTDOJOB
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_will_not_do_job = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // INHAND
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_in_hand = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // NOLAIR
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_no_lair = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // NOHATCHERY
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_no_hatchery = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // WOKENUP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_woken_up = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 7: // STANDINGONDEADENEMY
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_on_dead_enemy = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 8: // SULKING
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_sulking = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 9: // NOSALARY
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_no_salary = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 10: // SLAPPED
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_slapped = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 11: // STANDINGONDEADFRIEND
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_on_dead_friend = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 12: // INTORTURE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_in_torture = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 13: // INTEMPLE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_in_temple = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 14: // SLEEPING
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_sleeping = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 15: // GOTWAGE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_got_wage = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 16: // WINBATTLE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_win_battle = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 17: // UNTRAINED
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_untrained_time = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_untrained = k;
              n++;
            }
            if (n < 2)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 18: // OTHERSLEAVING
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_others_leaving = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 19: // JOBSTRESS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_job_stress = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 20: // QUEUE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_queue = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 21: // LAIRENEMY
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(creature_desc, word_buf);
              if (k >= 0)
              {
                crstat->lair_enemy = k;
                n++;
              } else
              {
                crstat->lair_enemy = 0;
                if (stricmp(word_buf,"NULL") == 0)
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 22: // ANNOYLEVEL
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_level = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 23: // ANGERJOBS
            crstat->jobs_anger = 0;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(angerjob_desc, word_buf);
              if (k > 0)
              {
                crstat->jobs_anger |= k;
                n++;
              } else
              {
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
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

TbBool parse_creaturemodel_senses_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
    struct CreatureStats *crstat;
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    crstat = creature_stats_get(crtr_model);
    if ((flags & CLd_AcceptPartial) == 0)
    {
        crstat->hearing = 0;
        crstat->eye_height = 0;
        crstat->field_of_view = 0;
        crstat->eye_effect = 0;
        crstat->max_angle_change = 1;
    }
    // Find the block
    sprintf(block_buf,"senses");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_senses_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,creatmodel_senses_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // HEARING
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->hearing = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 2: // EYEHEIGHT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->eye_height = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // FIELDOFVIEW
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->field_of_view = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // EYEEFFECT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(lenses_desc, word_buf);
              if (k >= 0)
              {
                crstat->eye_effect = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // MAXANGLECHANGE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->max_angle_change = (k << 11) / 360;
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
    return true;
}

TbBool parse_creaturemodel_appearance_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
    struct CreatureStats *crstat;
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    crstat = creature_stats_get(crtr_model);
    if ((flags & CLd_AcceptPartial) == 0)
    {
        crstat->walking_anim_speed = 1;
        crstat->visual_range = 1;
    }
    // Find the block
    sprintf(block_buf,"appearance");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_appearance_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,creatmodel_appearance_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // WALKINGANIMSPEED
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->walking_anim_speed = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 2: // VISUALRANGE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->visual_range = k;
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
    return true;
}

TbBool parse_creaturemodel_experience_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
    struct CreatureStats *crstat;
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    crstat = creature_stats_get(crtr_model);
    if ((flags & CLd_AcceptPartial) == 0)
    {
        for (n=0; n < 10; n++)
        {
          crstat->instance_spell[n] = 0;
          crstat->instance_level[n] = 0;
          crstat->to_level[n] = 0;
        }
        crstat->grow_up = 0;
        crstat->grow_up_level = 0;
        crstat->sleep_exp_slab = 0;
        crstat->sleep_experience = 0;
        crstat->exp_for_hitting = 0;
        crstat->rebirth = 0;
    }
    // Find the block
    sprintf(block_buf,"experience");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_experience_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,creatmodel_experience_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // POWERS
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(instance_desc, word_buf);
              if ((k >= 0) && (n < 10))
              {
                crstat->instance_spell[n] = k;
                n++;
              } else
              {
                CONFWRNLOG("Too many params, or incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 2: // POWERSLEVELREQUIRED
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k >= 0) && (n < 10))
              {
                crstat->instance_level[n] = k;
                n++;
              } else
              {
                CONFWRNLOG("Too many params, or incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 3: // LEVELSTRAINVALUES
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k >= 0) && (n < 10))
              {
                crstat->to_level[n] = k;
                n++;
              } else
              {
                CONFWRNLOG("Too many params, or incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 4: // GROWUP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->to_level[9] = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(creature_desc, word_buf);
              if (k >= 0)
              {
                crstat->grow_up = k;
                n++;
              } else
              {
                crstat->grow_up = 0;
                if (stricmp(word_buf,"NULL") == 0)
                  n++;
              }
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->grow_up_level = k;
              n++;
            }
            if (n < 3)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameters in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // SLEEPEXPERINCE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(slab_desc, word_buf);
              if (k >= 0)
              {
                crstat->sleep_exp_slab = k;
                n++;
              } else
              {
                crstat->sleep_exp_slab = 0;
              }
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->sleep_experience = k;
              n++;
            }
            if (n < 2)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameters in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // EXPERIENCEFORHITTING
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->exp_for_hitting = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 7: // REBIRTH
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->rebirth = k;
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
    return true;
}

TbBool parse_creaturemodel_jobs_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
    struct CreatureStats *crstat;
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    crstat = creature_stats_get(crtr_model);
    if ((flags & CLd_AcceptPartial) == 0)
    {
        crstat->job_primary = 0;
        crstat->job_secondary = 0;
        crstat->jobs_not_do = 0;
        crstat->job_stress = 0;
        crstat->training_value = 0;
        crstat->training_cost = 0;
        crstat->scavenge_value = 0;
        crstat->scavenger_cost = 0;
        crstat->research_value = 0;
        crstat->manufacture_value = 0;
        crstat->partner_training = 0;
    }
    // Find the block
    sprintf(block_buf,"jobs");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_jobs_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,creatmodel_jobs_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // PRIMARYJOBS
            crstat->job_primary = 0;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(creaturejob_desc, word_buf);
              if (k > 0)
              {
                crstat->job_primary |= k;
                n++;
              } else
              {
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 2: // SECONDARYJOBS
            crstat->job_secondary = 0;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(creaturejob_desc, word_buf);
              if (k > 0)
              {
                crstat->job_secondary |= k;
                n++;
              } else
              {
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 3: // NOTDOJOBS
            crstat->jobs_not_do = 0;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(creaturejob_desc, word_buf);
              if (k > 0)
              {
                crstat->jobs_not_do |= k;
                n++;
              } else
              {
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 4: // STRESSFULJOBS
            crstat->job_stress = 0;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(creaturejob_desc, word_buf);
              if (k > 0)
              {
                crstat->job_stress |= k;
                n++;
              } else
              {
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\", in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 5: // TRAININGVALUE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->training_value = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // TRAININGCOST
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->training_cost = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 7: // SCAVENGEVALUE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->scavenge_value = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 8: // SCAVENGERCOST
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->scavenger_cost = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 9: // RESEARCHVALUE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->research_value = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 10: // MANUFACTUREVALUE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->manufacture_value = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 11: // PARTNERTRAINING
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->partner_training = k;
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
    return true;
}

TbBool parse_creaturemodel_sprites_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
  long pos;
  int k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // If the file can't be partial, then initialize block data
  if ((flags & CLd_AcceptPartial) == 0)
  {
      for (n = 0; n < CREATURE_GRAPHICS_INSTANCES; n++)
      {
          set_creature_breed_graphics(crtr_model, n, 0);
      }
  }
  // Find the block
  sprintf(block_buf,"sprites");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
      if ((flags & CLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creature_graphics_desc,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,creature_graphics_desc);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      if ((cmd_num > 0) && (cmd_num <= CREATURE_GRAPHICS_INSTANCES))
      {
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            set_creature_breed_graphics(crtr_model, cmd_num-1, k);
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
      } else
      switch (cmd_num)
      {
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

TbBool load_creaturemodel_config_file(long crtr_model,const char *fname,unsigned short flags)
{
    static const char config_textname[] = "Creature Model config";
    char *buf;
    long len;
    TbBool result;
    len = LbFileLengthRnc(fname);
    if (len < 2)
    {
        if ((flags & CLd_IgnoreErrors) == 0)
            WARNMSG("%s file \"%s.cfg\" doesn't exist or is too small.",config_textname,fname);
        return false;
    }
    if (len > 65536)
    {
        if ((flags & CLd_IgnoreErrors) == 0)
            WARNMSG("%s file \"%s.cfg\" is too large.",config_textname,fname);
        return false;
    }
    buf = (char *)LbMemoryAlloc(len+256);
    if (buf == NULL)
      return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    result = (len > 0);
    if ((flags & CLd_AcceptPartial) == 0)
    {
      struct CreatureStats *crstat;
      crstat = creature_stats_get(crtr_model);
      LbMemorySet(crstat, '\0', sizeof(struct CreatureStats));
    }
    if (result)
    {
      result = parse_creaturemodel_attributes_blocks(crtr_model, buf, len, config_textname, flags);
      if ((flags & CLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s.cfg\" attributes blocks failed.",config_textname,fname);
    }
    if (result)
    {
      result = parse_creaturemodel_attraction_blocks(crtr_model, buf, len, config_textname, flags);
      if ((flags & CLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s.cfg\" attraction blocks failed.",config_textname,fname);
    }
    if (result)
    {
      result = parse_creaturemodel_annoyance_blocks(crtr_model, buf, len, config_textname, flags);
      if ((flags & CLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s.cfg\" annoyance blocks failed.",config_textname,fname);
    }
    if (result)
    {
      result = parse_creaturemodel_senses_blocks(crtr_model, buf, len, config_textname, flags);
      if ((flags & CLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s.cfg\" senses blocks failed.",config_textname,fname);
    }
    if (result)
    {
      result = parse_creaturemodel_appearance_blocks(crtr_model, buf, len, config_textname, flags);
      if ((flags & CLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s.cfg\" appearance blocks failed.",config_textname,fname);
    }
    if (result)
    {
      result = parse_creaturemodel_experience_blocks(crtr_model, buf, len, config_textname, flags);
      if ((flags & CLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s.cfg\" experience blocks failed.",config_textname,fname);
    }
    if (result)
    {
      result = parse_creaturemodel_jobs_blocks(crtr_model, buf, len, config_textname, flags);
      if ((flags & CLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s.cfg\" jobs blocks failed.",config_textname,fname);
    }
    if (result)
    {
      result = parse_creaturemodel_sprites_blocks(crtr_model, buf, len, config_textname, flags);
      if ((flags & CLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s.cfg\" sprites blocks failed.",config_textname,fname);
    }

    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_creaturemodel_config(long crtr_model,unsigned short flags)
{
    char conf_fnstr[COMMAND_WORD_LEN];
    char *fname;
    TbBool ret;
    LbStringToLowerCopy(conf_fnstr,get_conf_parameter_text(creature_desc,crtr_model),COMMAND_WORD_LEN);
    if (conf_fnstr[0] == '\0')
    {
      WARNMSG("Can't get config file name for creature %d.",crtr_model);
      return false;
    }
    SYNCDBG(0,"Reading \"%s.cfg\" as model %ld.",conf_fnstr,crtr_model);
    fname = prepare_file_fmtpath(FGrp_CrtrData,"%s.cfg",conf_fnstr);
    ret = load_creaturemodel_config_file(crtr_model,fname,flags);
    fname = prepare_file_fmtpath(FGrp_CmpgCrtrs,"%s.cfg",conf_fnstr);
    if (fname[0] != '\0')
    {
        SYNCDBG(0,"Reading campaign \"%s.cfg\".",conf_fnstr);
        load_creaturemodel_config_file(crtr_model,fname,flags|CLd_AcceptPartial|CLd_IgnoreErrors);
    }
    return ret;
}

/**
 * Zeroes all the maintenance costs for all creatures.
 */
TbBool make_all_creatures_free(void)
{
    struct CreatureStats *crstat;
    long i;
    for (i=0; i < crtr_conf.model_count; i++)
    {
      crstat = creature_stats_get(i);
      crstat->training_cost = 0;
      crstat->scavenger_cost = 0;
      crstat->pay = 0;
    }
    return true;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
