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
#include "game_merge.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "config_campaigns.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_lenses.h"
#include "creature_control.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "player_data.h"
#include "custom_sprites.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

const struct NamedCommand creatmodel_attributes_commands[] = {
  {"NAME",             1},
  {"HEALTH",           2},
  {"HEALREQUIREMENT",  3},
  {"HEALTHRESHOLD",    4},
  {"STRENGTH",         5},
  {"ARMOUR",           6},
  {"DEXTERITY",        7},
  {"FEARWOUNDED",      8},
  {"FEARSTRONGER",     9},
  {"DEFENCE",         10},
  {"LUCK",            11},
  {"RECOVERY",        12},
  {"HUNGERRATE",      13},
  {"HUNGERFILL",      14},
  {"LAIRSIZE",        15},
  {"HURTBYLAVA",      16},
  {"BASESPEED",       17},
  {"GOLDHOLD",        18},
  {"SIZE",            19},
  {"ATTACKPREFERENCE",20},
  {"PAY",             21},
  {"HEROVSKEEPERCOST",22},
  {"SLAPSTOKILL",     23},
  {"CREATURELOYALTY", 24},
  {"LOYALTYLEVEL",    25},
  {"DAMAGETOBOULDER", 26},
  {"THINGSIZE",       27},
  {"PROPERTIES",      28},
  {"NAMETEXTID",      29},
  {"FEARSOMEFACTOR",  30},
  {"TOKINGRECOVERY",  31},
  {NULL,               0},
  };

const struct NamedCommand creatmodel_properties_commands[] = {
  {"BLEEDS",             1},
  {"UNAFFECTED_BY_WIND", 2},
  {"IMMUNE_TO_GAS",      3},
  {"HUMANOID_SKELETON",  4},
  {"PISS_ON_DEAD",       5},
  {"FLYING",             7},
  {"SEE_INVISIBLE",      8},
  {"PASS_LOCKED_DOORS",  9},
  {"SPECIAL_DIGGER",    10},
  {"ARACHNID",          11},
  {"DIPTERA",           12},
  {"LORD",              13},
  {"SPECTATOR",         14},
  {"EVIL",              15},
  {"NEVER_CHICKENS",    16},
  {"IMMUNE_TO_BOULDER", 17},
  {"NO_CORPSE_ROTTING", 18},
  {"NO_ENMHEART_ATTCK", 19},
  {"TREMBLING_FAT",     20},
  {"FEMALE",            21},
  {"INSECT",            22},
  {"ONE_OF_KIND",       23},
  {"NO_IMPRISONMENT",   24},
  {"IMMUNE_TO_DISEASE", 25},
  {"ILLUMINATED",       26},
  {"ALLURING_SCVNGR",   27},
  {NULL,                 0},
  };

const struct NamedCommand creatmodel_attraction_commands[] = {
  {"ENTRANCEROOM",       1},
  {"ROOMSLABSREQUIRED",  2},
  {"BASEENTRANCESCORE",  3},
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
  {"GOINGPOSTAL",         24},
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
  {"POSSESSSWIPEINDEX",    3},
  {"NATURALDEATHKIND",     4},
  {"SHOTORIGIN",           5},
  {NULL,                   0},
  };

const struct NamedCommand creature_deathkind_desc[] = {
    {"NORMAL",          Death_Normal},
    {"FLESHEXPLODE",    Death_FleshExplode},
    {"GASFLESHEXPLODE", Death_GasFleshExplode},
    {"SMOKEEXPLODE",    Death_SmokeExplode},
    {"ICEEXPLODE",      Death_IceExplode},
    {NULL,              0},
    };


const struct NamedCommand creatmodel_experience_commands[] = {
  {"POWERS",               1},
  {"POWERSLEVELREQUIRED",  2},
  {"LEVELSTRAINVALUES",    3},
  {"GROWUP",               4},
  {"SLEEPEXPERIENCE",      5},
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
  
const struct NamedCommand creatmodel_sounds_commands[] = {
  {"HURT",                 CrSnd_Hurt},
  {"HIT",                  CrSnd_Hit},
  {"HAPPY",                CrSnd_Happy},
  {"SAD",                  CrSnd_Sad},
  {"HANG",                 CrSnd_Hang},
  {"DROP",                 CrSnd_Drop},
  {"TORTURE",              CrSnd_Torture},
  {"SLAP",                 CrSnd_Slap},
  {"DIE",                  CrSnd_Die},
  {"FOOT",                 CrSnd_Foot},
  {"FIGHT",                CrSnd_Fight},
  {NULL,                   0},
  };

/******************************************************************************/
TbBool parse_creaturemodel_attributes_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
  // Block name and parameter word store variables
  // Initialize block data
  struct CreatureStats* crstat = creature_stats_get(crtr_model);
  struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[crtr_model];
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      crstat->health = 1;
      crstat->heal_requirement = 1;
      crstat->heal_threshold = 1;
      crstat->strength = 0;
      crstat->armour = 0;
      crstat->dexterity = 0;
      crstat->fear_wounded = 12;
      crstat->fear_stronger = 65000;
      crstat->fearsome_factor = 100;
      crstat->defense = 0;
      crstat->luck = 0;
      crstat->sleep_recovery = 1;
      crstat->toking_recovery = 10;
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
  char block_buf[COMMAND_WORD_LEN];
  sprintf(block_buf, "attributes");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_attributes_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, creatmodel_attributes_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
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
      case 3: // HEALREQUIREMENT
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
      case 8: // FEARWOUNDED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->fear_wounded = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 9: // FEARSTRONGER
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->fear_stronger = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 10: // DEFENCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->defense = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 11: // LUCK
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
      case 12: // RECOVERY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->sleep_recovery = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 13: // HUNGERRATE
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
      case 14: // HUNGERFILL
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
      case 15: // LAIRSIZE
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
      case 16: // HURTBYLAVA
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
      case 17: // BASESPEED
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
      case 18: // GOLDHOLD
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
      case 19: // SIZE
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
      case 20: // ATTACKPREFERENCE
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
      case 21: // PAY
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
      case 22: // HEROVSKEEPERCOST
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
      case 23: // SLAPSTOKILL
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
      case 24: // CREATURELOYALTY
          // Unused
          break;
      case 25: // LOYALTYLEVEL
          // Unused
          break;
      case 26: // DAMAGETOBOULDER
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
      case 27: // THINGSIZE
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
      case 28: // PROPERTIES
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
            case 4: // HUMANOID_SKELETON
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
              crconf->model_flags |= CMF_IsSpecDigger;
              n++;
              break;
            case 11: // ARACHNID
              crconf->model_flags |= CMF_IsArachnid;
              n++;
              break;
            case 12: // DIPTERA
              crconf->model_flags |= CMF_IsDiptera;
              n++;
              break;
            case 13: // LORD
              crconf->model_flags |= CMF_IsLordOTLand;
              n++;
              break;
            case 14: // SPECTATOR
              crconf->model_flags |= CMF_IsSpectator;
              n++;
              break;
            case 15: // EVIL
              crconf->model_flags |= CMF_IsEvil;
              n++;
              break;
            case 16: // NEVER_CHICKENS
              crconf->model_flags |= CMF_NeverChickens;
              n++;
              break;
            case 17: // IMMUNE_TO_BOULDER
                crconf->model_flags |= CMF_ImmuneToBoulder;
                n++;
                break;
            case 18: // NO_CORPSE_ROTTING
                crconf->model_flags |= CMF_NoCorpseRotting;
                n++;
                break;
            case 19: // NO_ENMHEART_ATTCK
                crconf->model_flags |= CMF_NoEnmHeartAttack;
                n++;
                break;
            case 20: // TREMBLING_FAT
                crconf->model_flags |= CMF_TremblingFat;
                n++;
                break;
            case 21: // FEMALE
                crconf->model_flags |= CMF_Female;
                n++;
                break;
            case 22: // INSECT
                crconf->model_flags |= CMF_Insect;
                n++;
                break;
            case 23: // ONE_OF_KIND
                crconf->model_flags |= CMF_OneOfKind;
                n++;
                break;
            case 24: // NO_IMPRISONMENT
                crconf->model_flags |= CMF_NoImprisonment;
                n++;
                break;
            case 25: // IMMUNE_TO_DISEASE
                crconf->model_flags |= CMF_NeverSick;
                n++;
                break;
            case 26: // ILLUMINATED
                crstat->illuminated = true;
                n++;
                break;
            case 27: // ALLURING_SCVNGR
                crstat->entrance_force = true;
                n++;
                break;
            default:
              CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              break;
            }
          }
          break;
      case 29: // NAMETEXTID
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
      case 30: // FEARSOMEFACTOR
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->fearsome_factor = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 31: // TOKINGRECOVERY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->toking_recovery = k;
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
  // If the creature is a special breed, then update an attribute in CreatureConfig struct
  if ((crconf->model_flags & CMF_IsSpecDigger) != 0)
  {
      if ((crconf->model_flags & CMF_IsEvil) != 0) {
          gameadd.crtr_conf.special_digger_evil = crtr_model;
      } else {
          gameadd.crtr_conf.special_digger_good = crtr_model;
      }
  }
  if ((crconf->model_flags & CMF_IsSpectator) != 0)
  {
      gameadd.crtr_conf.spectator_breed = crtr_model;
  }
  // Set creature start states based on the flags
  if ((crconf->model_flags & CMF_IsSpecDigger) != 0)
  {
      creatures[crtr_model].evil_start_state = CrSt_ImpDoingNothing;
      creatures[crtr_model].good_start_state = CrSt_TunnellerDoingNothing;
  } else
  {
      creatures[crtr_model].evil_start_state = CrSt_CreatureDoingNothing;
      creatures[crtr_model].good_start_state = CrSt_GoodDoingNothing;
  }
  return true;
}

TbBool parse_creaturemodel_attraction_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
  int n;
  // Block name and parameter word store variables
  // Initialize block data
  struct CreatureStats* crstat = creature_stats_get(crtr_model);
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      for (n=0; n < ENTRANCE_ROOMS_COUNT; n++)
      {
        crstat->entrance_rooms[n] = 0;
        crstat->entrance_slabs_req[n] = 0;
      }
      crstat->entrance_score = 10;
      crstat->scavenge_require = 1;
      crstat->torture_break_time = 1;
  }
  // Find the block
  char block_buf[COMMAND_WORD_LEN];
  sprintf(block_buf, "attraction");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_attraction_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, creatmodel_attraction_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // ENTRANCEROOM
          for (k=0; k < ENTRANCE_ROOMS_COUNT; k++)
            crstat->entrance_rooms[k] = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_id(room_desc, word_buf);
            if ((k >= 0) && (n < ENTRANCE_ROOMS_COUNT))
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
          for (k=0; k < ENTRANCE_ROOMS_COUNT; k++)
            crstat->entrance_slabs_req[k] = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (n < ENTRANCE_ROOMS_COUNT)
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
      case 3: // BASEENTRANCESCORE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            crstat->entrance_score = k;
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
            crstat->torture_break_time = k;
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
    // Block name and parameter word store variables
    // Initialize block data
    struct CreatureStats* crstat = creature_stats_get(crtr_model);
    if ((flags & CnfLd_AcceptPartial) == 0)
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
        crstat->annoy_going_postal = 0;
        crstat->annoy_queue = 0;
        crstat->lair_enemy = 0;
        crstat->annoy_level = 0;
        crstat->jobs_anger = 0;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "annoyance");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_annoyance_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creatmodel_annoyance_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
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
                  if (strcasecmp(word_buf,"NULL") == 0)
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
        case 24: // GOINGPOSTAL
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              crstat->annoy_going_postal = k;
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

TbBool parse_creaturemodel_senses_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    struct CreatureStats* crstat = creature_stats_get(crtr_model);
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        crstat->hearing = 0;
        crstat->eye_height = 0;
        crstat->field_of_view = 0;
        crstat->eye_effect = 0;
        crstat->max_angle_change = 1;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "senses");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_senses_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creatmodel_senses_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
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
              if (k > 0)
              {
                  crstat->max_angle_change = (k * LbFPMath_PI) / 180;
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

TbBool parse_creaturemodel_appearance_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    struct CreatureStats* crstat = creature_stats_get(crtr_model);
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        crstat->walking_anim_speed = 1;
        crstat->visual_range = 1;
        creatures[crtr_model].swipe_idx = 0;
        creatures[crtr_model].natural_death_kind = Death_Normal;
        creatures[crtr_model].shot_shift_x = 0;
        creatures[crtr_model].shot_shift_y = 0;
        creatures[crtr_model].shot_shift_z = 0;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "appearance");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_appearance_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creatmodel_appearance_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
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
        case 3: // SWIPEINDEX
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k >= 0)
                {
                    creatures[crtr_model].swipe_idx = k;
                    n++;
                }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // NATURALDEATHKIND
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = get_id(creature_deathkind_desc, word_buf);
                if (k > 0)
                {
                    creatures[crtr_model].natural_death_kind = k;
                    n++;
                }
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // SHOTORIGIN
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creatures[crtr_model].shot_shift_x = k;
                n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creatures[crtr_model].shot_shift_y = k;
                n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creatures[crtr_model].shot_shift_z = k;
                n++;
            }
            if (n < 3)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameters in [%s] block of %s file.",
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
    int n;
    // Block name and parameter word store variables
    // Initialize block data
    struct CreatureStats* crstat = creature_stats_get(crtr_model);
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        for (n=0; n < LEARNED_INSTANCES_COUNT; n++)
        {
            crstat->learned_instance_id[n] = 0;
            crstat->learned_instance_level[n] = 0;
        }
        for (n=0; n < CREATURE_MAX_LEVEL; n++)
        {
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
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "experience");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_experience_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creatmodel_experience_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // POWERS
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(instance_desc, word_buf);
              if ((k >= 0) && (n < LEARNED_INSTANCES_COUNT))
              {
                crstat->learned_instance_id[n] = k;
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
              if ((k >= 0) && (n < LEARNED_INSTANCES_COUNT))
              {
                crstat->learned_instance_level[n] = k;
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
              if ((k >= 0) && (n < CREATURE_MAX_LEVEL-1))
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
              crstat->to_level[CREATURE_MAX_LEVEL-1] = k;
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
                if (strcasecmp(word_buf,"NULL") == 0)
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
        case 5: // SLEEPEXPERIENCE
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
    // Block name and parameter word store variables
    // Initialize block data
    struct CreatureStats* crstat = creature_stats_get(crtr_model);
    if ((flags & CnfLd_AcceptPartial) == 0)
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
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "jobs");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_jobs_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creatmodel_jobs_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
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
  int n;
  // Block name and parameter word store variables
  // If the file can't be partial, then initialize block data
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      for (n = 0; n < CREATURE_GRAPHICS_INSTANCES; n++)
      {
          set_creature_model_graphics(crtr_model, n, 0);
      }
  }
  // Find the block
  char block_buf[COMMAND_WORD_LEN];
  sprintf(block_buf, "sprites");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creature_graphics_desc,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, creature_graphics_desc);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      if ((cmd_num == (CGI_HandSymbol + 1)) || (cmd_num == (CGI_QuerySymbol + 1)))
      {
          char word_buf[COMMAND_WORD_LEN];
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              n = get_icon_id(word_buf);
              if (n >= 0)
              {
                  set_creature_model_graphics(crtr_model, cmd_num-1, n);
              }
              else
              {
                  set_creature_model_graphics(crtr_model, cmd_num-1, bad_icon_id);
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                             COMMAND_TEXT(cmd_num),block_buf,config_textname);
              }
          }
      }
      else if ((cmd_num > 0) && (cmd_num <= CREATURE_GRAPHICS_INSTANCES))
      {
          char word_buf[COMMAND_WORD_LEN];
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k == 0) && (strcmp(word_buf, "0") != 0))
            {
                CONFWRNLOG("Custom animations are not supported yet");
            }
            set_creature_model_graphics(crtr_model, cmd_num-1, k);
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

TbBool parse_creaturemodel_sounds_blocks(long crtr_model,char *buf,long len,const char *config_textname,unsigned short flags)
{
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "sounds");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatmodel_sounds_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creatmodel_sounds_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case CrSnd_Hurt:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creature_sounds[crtr_model].hurt.index = k;
                n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creature_sounds[crtr_model].hurt.count = k;
                n++;
            }            
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Hit:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creature_sounds[crtr_model].hit.index = k;
                n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creature_sounds[crtr_model].hit.count = k;
                n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Happy:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creature_sounds[crtr_model].happy.index = k;
                n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creature_sounds[crtr_model].happy.count = k;
                n++;
            }            
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Sad:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creature_sounds[crtr_model].sad.index = k;
                n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                creature_sounds[crtr_model].sad.count = k;
                n++;
            }            
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Hang:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].hang.index = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].hang.count = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Drop:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].drop.index = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].drop.count = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Torture:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].torture.index = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].torture.count = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Slap:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].slap.index = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].slap.count = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Die:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].die.index = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].die.count = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Foot:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].foot.index = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].foot.count = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case CrSnd_Fight:
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].fight.index = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              creature_sounds[crtr_model].fight.count = k;
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

TbBool load_creaturemodel_config_file(long crtr_model,const char *textname,const char *fname,unsigned short flags)
{
    SYNCDBG(0,"%s model %ld from %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",crtr_model,textname,fname);
    long len = LbFileLengthRnc(fname);
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
    char* buf = (char*)LbMemoryAlloc(len + 256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        struct CreatureStats* crstat = creature_stats_get(crtr_model);
        LbMemorySet(crstat, '\0', sizeof(struct CreatureStats));
    }
    // Parse blocks of the config file
    if (result)
    {
        result = parse_creaturemodel_attributes_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" attributes blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturemodel_attraction_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" attraction blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturemodel_annoyance_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" annoyance blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturemodel_senses_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" senses blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturemodel_appearance_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" appearance blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturemodel_experience_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" experience blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturemodel_jobs_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" jobs blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturemodel_sprites_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" sprites blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturemodel_sounds_blocks(crtr_model, buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" sounds blocks failed.",textname,fname);
    }
    // Mark the fact that stats were updated
    creature_stats_updated(crtr_model);
    // Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_creaturemodel_config(long crmodel, unsigned short flags)
{
    static const char config_global_textname[] = "global creature model config";
    static const char config_campgn_textname[] = "campaing creature model config";
    char conf_fnstr[COMMAND_WORD_LEN];
    LbStringToLowerCopy(conf_fnstr,get_conf_parameter_text(creature_desc,crmodel),COMMAND_WORD_LEN);
    if (strlen(conf_fnstr) == 0)
    {
        WARNMSG("Cannot get config file name for creature %d.",crmodel);
        return false;
    }
    char* fname = prepare_file_fmtpath(FGrp_CrtrData, "%s.cfg", conf_fnstr);
    TbBool result = load_creaturemodel_config_file(crmodel, config_global_textname, fname, flags);
    fname = prepare_file_fmtpath(FGrp_CmpgCrtrs,"%s.cfg",conf_fnstr);
    if (strlen(fname) > 0)
    {
        load_creaturemodel_config_file(crmodel,config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Zeroes all the maintenance costs for all creatures.
 */
TbBool make_all_creatures_free(void)
{
    for (long i = 0; i < gameadd.crtr_conf.model_count; i++)
    {
        struct CreatureStats* crstat = creature_stats_get(i);
        crstat->training_cost = 0;
        crstat->scavenger_cost = 0;
        crstat->pay = 0;
    }
    return true;
}

/**
 * Changes max health of creatures, and updates all creatures to max.
 */
TbBool change_max_health_of_creature_kind(ThingModel crmodel, long new_max)
{
    struct CreatureStats* crstat = creature_stats_get(crmodel);
    if (creature_stats_invalid(crstat)) {
        ERRORLOG("Invalid creature model %d",(int)crmodel);
        return false;
    }
    SYNCDBG(3,"Changing all %s health from %d to %d.",creature_code_name(crmodel),(int)crstat->health,(int)new_max);
    crstat->health = saturate_set_signed(new_max, 16);
    creature_stats_updated(crmodel);
    int n = do_to_all_things_of_class_and_model(TCls_Creature, crmodel, update_creature_health_to_max);
    return (n > 0);
}

TbBool heal_completely_all_players_creatures(PlayerNumber plyr_idx, ThingModel crmodel)
{
    SYNCDBG(3,"Healing all player %d creatures of model %s",(int)plyr_idx,creature_code_name(crmodel));
    int n = do_to_players_all_creatures_of_model(plyr_idx, crmodel, update_creature_health_to_max);
    return (n > 0);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
