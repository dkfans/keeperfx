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
#include "config_magic.hpp"
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
  {"AREADAMAGE",      3},
  {NULL,              0},
  };

const struct NamedCommand magic_shot_commands[] = {
  {"NAME",            1},
  {"HEALTH",          2},
  {"DAMAGE",          3},
  {"SPEED",           4},
  {"PROPERTIES",      5},
  {NULL,              0},
  };

const struct NamedCommand magic_power_commands[] = {
  {"NAME",            1},
  {"POWER",           2},
  {"COST",            3},
  {"TIME",            4},
  {NULL,              0},
  };

const struct NamedCommand shotmodel_properties_commands[] = {
  {"SLAPPABLE",         1},
  {"NAVIGABLE",         2},
  {NULL,                0},
  };

struct SpellInfo spell_info[] = {
  {0,  0, 0,  0,  0,  0, 0,    0, 0}, // [0] NULL
  {1,  1, 0,  0,  5,  0, 0,    0, 0}, // [1] FIREBALL
  {1,  2, 0,  0,  6,  0, 0,    0, 0},
  {1,  3, 0,  0,  7,  0, 0,    0, 0},
  {0,  0, 1,  0,  8, 37, 0,    0, 0},
  {1,  4, 0,  0,  9,  0, 0,    0, 0}, // [5] LIGHTNING
  {0,  0, 1,  0, 10, 37, 0,    0, 0},
  {0,  0, 1,  0, 11, 37, 0,    0, 0},
  {1,  5, 0,  0, 12,  0, 0,    0, 0},
  {0,  0, 1,  0, 13, 37, 0,    0, 0},
  {0,  0, 1,  0, 14,  0, 0,    0, 0}, // [10] TELEPORT
  {0,  0, 1,  0, 15, 38, 0,    0, 0},
  {1, 10, 0,  0, 16,  0, 0,    0, 0},
  {1, 12, 0,  0, 17,  0, 0,    0, 0},
  {0,  0, 1,  0, 18,  0, 0,    0, 0},
  {1,  9, 0,  0, 19,  0, 0,    0, 0}, // [15] MISSILE
  {1,  6, 0,  0, 20,  0, 0,    0, 0},
  {1,  7, 0,  0, 21,  0, 0,    0, 0},
  {0,  8, 0,  0, 22,  0, 0,    0, 0},
  {0,  0, 1,  0, 23,  0, 0,    0, 0},
  {0,  0, 1,  0, 24, 37, 0,    0, 0}, // [20] FLY
  {0,  0, 1,  0, 25, 37, 0,    0, 0},
  {0, 11, 0,  0, 26,  0, 0,    0, 0},
  {1, 13, 0,  0, 27,  0, 0,    0, 0}, // [23] HAILSTORM
  {0,  0, 0, 14, 28,  0, 8, 4000, 4}, // [24] WORD_OF_POWER
  {0,  0, 0,  0,  0,  0, 0,    0, 0},
  {1, 26, 0,  0, 41,  0, 0,    0, 0},
  {1, 27, 0,  0, 42,  0, 0,    0, 0},
  {1, 28, 0,  0, 43,  0, 0,    0, 0},
  {1, 25, 0,  0, 40,  0, 0,    0, 0},
};

struct SpellData spell_data[] = {
  {36, 11, 0,   0,   0,   0,   0,   0,  0, NULL,                 0, 0},      //[0]
  { 0,  0, 0,   0,   0,   0,   0,   0,  0, NULL,                 0, 0},      //[1]
  {36, 24, 0,  95, 118, 631, 648, 831,  5, NULL,                 0, 0},      //[2]
  {97,  0, 0, 394, 452, 636, 653, 834,  0, NULL,                 0, 0},      //[3]
  { 0,  0, 0,   0,   0,   0,   0,   0,  0, NULL,                 0, 0},      //[4]
  {36,  8, 1,  85, 108, 632, 649, 828, 12, sight_of_evil_expand_check,0, 0}, //[5] Sight of Evil
  {36,  6, 1,  93, 116, 633, 650, 826,  0, call_to_arms_expand_check, 1, 1}, //[6] Call To Arms
  {36,  7, 1,  97, 120, 635, 652, 837, 10, general_expand_check, 0, 0},      //[7]
  {36, 22, 0,  87, 110, 644, 661, 829,  8, general_expand_check, 1, 0},      //[8]
  {41,  0, 0,  89, 112, 634, 651, 830,  0, general_expand_check, 0, 0},      //[9] Hold Audience
  {36, 17, 0, 101, 124, 640, 657, 833,  6, general_expand_check, 1, 1},      //[10]
  {36, 19, 0,  99, 122, 637, 654, 838, 11, general_expand_check, 1, 0},      //[11]
  {36, 20, 0, 103, 126, 638, 655, 825,  9, general_expand_check, 1, 0},      //[12]
  {36, 21, 0, 105, 128, 639, 656, 832,  1, general_expand_check, 1, 0},      //[13]
  {36, 26, 0, 310, 319, 642, 659, 835,  3, general_expand_check, 1, 1},      //[14]
  {36, 27, 0, 306, 314, 641, 658, 827,  2, general_expand_check, 1, 1},      //[15]
  {36, 25, 0, 308, 317, 643, 660, 839,  4, general_expand_check, 0, 0},      //[16]
  {36, 28, 0, 105, 128, 645, 662,   0,  0, NULL,                 0, 0},      //[17]
  {36, 11, 0,  91, 114, 630, 647, 836,  7, NULL,                 1, 0},      //[18] Possession
  {98,  0, 0, 312, 321, 646, 663, 824,  0, NULL,                 0, 0},      //[19] Armageddon
  { 0,  0, 0,   0,   0,   0,   0,   0,  0, NULL,                 0, 0},      //[20]
};

/******************************************************************************/
struct MagicConfig magic_conf;
struct NamedCommand spell_desc[MAGIC_ITEMS_MAX];
struct NamedCommand shot_desc[MAGIC_ITEMS_MAX];
struct NamedCommand power_desc[MAGIC_ITEMS_MAX];
/******************************************************************************/
struct SpellInfo *get_magic_info(int mgc_idx)
{
  if ((mgc_idx < 0) || (mgc_idx >= MAGIC_TYPES_COUNT))
    return &spell_info[0];
  return &spell_info[mgc_idx];
}

TbBool magic_info_is_invalid(const struct SpellInfo *mgcinfo)
{
  if (mgcinfo <= &spell_info[0])
    return true;
  return false;
}

struct SpellData *get_power_data(int pwr_idx)
{
  if ((pwr_idx > 0) && (pwr_idx < POWER_TYPES_COUNT))
    return &spell_data[pwr_idx];
  if ((pwr_idx < -1) || (pwr_idx >= POWER_TYPES_COUNT))
    ERRORLOG("Request of invalid power (no %d) intercepted",pwr_idx);
  return &spell_data[0];
}

long get_power_description_strindex(int pwr_idx)
{
  if ((pwr_idx < 0) || (pwr_idx >= POWER_TYPES_COUNT))
    return 0;
  return spell_data[pwr_idx].field_D;
}

TbBool power_data_is_invalid(const struct SpellData *pwrdata)
{
  if (pwrdata <= &spell_data[0])
    return true;
  return false;
}

long get_power_index_for_work_state(long work_state)
{
  long i;
  for (i=0; i<POWER_TYPES_COUNT; i++)
  {
    if (spell_data[i].field_4 == work_state)
    {
      return i;
    }
  }
  return 0;
}

TbBool spell_is_stupid(int sptype)
{
  struct SpellData *pwrdata;
  pwrdata = get_power_data(sptype);
  // now a test similar to the one in power_data_is_invalid()
  // but we accept the NULL power (power 0)
  if (pwrdata < &spell_data[0])
    return true;
  return (pwrdata->field_0 <= 0);
}

struct SpellConfigStats *get_spell_model_stats(SpellKind spmodel)
{
    if ((spmodel < 0) || (spmodel >= magic_conf.spell_types_count))
        return &magic_conf.spell_cfgstats[0];
    return &magic_conf.spell_cfgstats[spmodel];
}

struct ShotConfigStats *get_shot_model_stats(ThingModel tngmodel)
{
    if ((tngmodel < 0) || (tngmodel >= magic_conf.shot_types_count))
        return &magic_conf.shot_cfgstats[0];
    return &magic_conf.shot_cfgstats[tngmodel];
}

struct PowerConfigStats *get_power_model_stats(PowerKind pwmodel)
{
    if ((pwmodel < 0) || (pwmodel >= magic_conf.power_types_count))
        return &magic_conf.power_cfgstats[0];
    return &magic_conf.power_cfgstats[pwmodel];
}

TbBool parse_magic_common_blocks(char *buf,long len)
{
  static const char config_textname[] = "Magic config";
  long pos;
  int k,n;
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
    WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
    return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_common_commands,cmd_num)
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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

TbBool parse_magic_spell_blocks(char *buf,long len)
{
  static const char config_textname[] = "Magic config";
  struct SpellConfigStats *spellst;
  struct SpellConfig *splconf;
  struct SpellInfo *magicinf;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(magic_conf.spell_cfgstats)/sizeof(magic_conf.spell_cfgstats[0]);
  for (i=0; i < arr_size; i++)
  {
      spellst = get_spell_model_stats(i);
      LbMemorySet(spellst->code_name, 0, COMMAND_WORD_LEN);
      if (i < magic_conf.spell_types_count)
      {
        spell_desc[i].name = spellst->code_name;
        spell_desc[i].num = i;
      } else
      {
        spell_desc[i].name = NULL;
        spell_desc[i].num = 0;
      }
  }
  arr_size = magic_conf.spell_types_count;
  for (i=0; i < arr_size; i++)
  {
      splconf = &game.spells_config[i];
      splconf->duration = 0;
      magicinf = get_magic_info(i);
      magicinf->area_hit_type = 0;
      magicinf->area_range = 0;
      magicinf->area_damage = 0;
  }
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"spell%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      continue;
    }
    splconf = &game.spells_config[i];
    magicinf = get_magic_info(i);
    spellst = get_spell_model_stats(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_spell_commands,cmd_num)
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
          if (get_conf_parameter_single(buf,&pos,len,spellst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // AREADAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            magicinf->area_hit_type = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            magicinf->area_range = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            magicinf->area_damage = k;
            n++;
          }
          if (n < 3)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
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

TbBool parse_magic_shot_blocks(char *buf,long len)
{
  static const char config_textname[] = "Magic config";
  struct ShotConfigStats *shotst;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(magic_conf.shot_cfgstats)/sizeof(magic_conf.shot_cfgstats[0]);
  for (i=0; i < arr_size; i++)
  {
      shotst = get_shot_model_stats(i);
      LbMemorySet(shotst->code_name, 0, COMMAND_WORD_LEN);
      shotst->model_flags = 0;
      shotst->old = &shot_stats[i];
      if (i < magic_conf.shot_types_count)
      {
        shot_desc[i].name = shotst->code_name;
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
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      continue;
    }
    shotst = get_shot_model_stats(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_shot_commands,cmd_num)
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
          if (get_conf_parameter_single(buf,&pos,len,shotst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
            break;
          }
          n++;
          break;
      case 2: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            shotst->old->health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // DAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            shotst->old->damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // SPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            shotst->old->speed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // PROPERTIES
          shotst->model_flags = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_id(shotmodel_properties_commands, word_buf);
            switch (k)
            {
            case 1: // SLAPPABLE
                shotst->model_flags |= ShMF_Slappable;
                n++;
                break;
            case 2: // NAVIGABLE
                shotst->model_flags |= ShMF_Navigable;
                n++;
                break;
            default:
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                break;
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
  }
  return true;
}

TbBool parse_magic_power_blocks(char *buf,long len)
{
  static const char config_textname[] = "Magic config";
  struct PowerConfigStats *powerst;
  struct MagicStats *magstat;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(magic_conf.power_cfgstats)/sizeof(magic_conf.power_cfgstats[0]);
  for (i=0; i < arr_size; i++)
  {
      powerst = get_power_model_stats(i);
      LbMemorySet(powerst->code_name, 0, COMMAND_WORD_LEN);
      if (i < magic_conf.power_types_count)
      {
        power_desc[i].name = powerst->code_name;
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
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      continue;
    }
    magstat = &game.magic_stats[i];
    powerst = get_power_model_stats(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_power_commands,cmd_num)
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
          if (get_conf_parameter_single(buf,&pos,len,powerst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
            break;
          }
          break;
      case 2: // POWER
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (n > SPELL_MAX_LEVEL)
            {
              CONFWRNLOG("Too many \"%s\" parameters in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
              break;
            }
            magstat->power[n] = k;
            n++;
          }
          if (n <= SPELL_MAX_LEVEL)
          {
            CONFWRNLOG("Couldn't read all \"%s\" parameters in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // COST
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (n > SPELL_MAX_LEVEL)
            {
              CONFWRNLOG("Too many \"%s\" parameters in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
              break;
            }
            magstat->cost[n] = k;
            n++;
          }
          if (n <= SPELL_MAX_LEVEL)
          {
            CONFWRNLOG("Couldn't read all \"%s\" parameters in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
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

TbBool load_magic_config(const char *conf_fname,unsigned short flags)
{
  char *fname;
  char *buf;
  long len;
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

/**
 * Returns Code Name (name to use in script file) of given spell model.
 */
const char *spell_code_name(SpellKind spmodel)
{
    const char *name;
    name = get_conf_parameter_text(spell_desc,spmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given shot model.
 */
const char *shot_code_name(ThingModel tngmodel)
{
    const char *name;
    name = get_conf_parameter_text(shot_desc,tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given keepers power model.
 */
const char *power_code_name(PowerKind pwmodel)
{
    const char *name;
    name = get_conf_parameter_text(power_desc,pwmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the power model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the power model if found, otherwise -1
 */
int power_model_id(const char * code_name)
{
    int i;

    for (i = 0; i < magic_conf.power_types_count; ++i) {
        if (strncmp(magic_conf.power_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Returns a flag which is stored in a creature affected by given spell.
 * @param spkind
 * @return
 */
unsigned long spell_to_creature_affected_flag(SpellKind spkind)
{
    switch (spkind)
    {
    case SplK_Armour:
        return CSAfF_Armour;
    case SplK_Rebound:
        return CSAfF_Rebound;
    case SplK_Invisibility:
        return CSAfF_Invisibility;
    case SplK_Speed:
        return CSAfF_Speed;
    case SplK_Slow:
        return CSAfF_Slow;
    case SplK_Fly:
        return CSAfF_Fly;
    case SplK_Sight:
        return CSAfF_Sight;
    default:
        break;
    }
    return 0;
}

/**
 * Zeroes all the costs for all spells.
 */
TbBool make_all_powers_cost_free(void)
{
  struct MagicStats *magstat;
  long i,n;
  for (i=0; i < magic_conf.power_types_count; i++)
  {
    magstat = &game.magic_stats[i];
    for (n=0; n <= SPELL_MAX_LEVEL; n++)
      magstat->cost[n] = 0;
  }
  return true;
}

/**
 * Makes all keeper spells to be available to research.
 */
TbBool make_all_powers_researchable(PlayerNumber plyr_idx)
{
  struct Dungeon *dungeon;
  long i;
  dungeon = get_players_num_dungeon(plyr_idx);
  for (i=0; i < KEEPER_SPELLS_COUNT; i++)
  {
    dungeon->magic_resrchable[i] = 1;
  }
  return true;
}

/**
 * Sets power availability state.
 */
TbBool set_power_available(PlayerNumber plyr_idx, SpellKind spl_idx, long resrch, long avail)
{
  struct Dungeon *dungeon;
  SYNCDBG(8,"Starting for spell %ld, player %ld, state %ld,%ld",spl_idx,plyr_idx,resrch,avail);
  // note that we can't get_players_num_dungeon() because players
  // may be uninitialized yet when this is called.
  dungeon = get_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
      return false;
  dungeon->magic_resrchable[spl_idx] = resrch;
  if (avail <= 0)
  {
    dungeon->magic_level[spl_idx] = 0;
    if (game.chosen_spell_type == spl_idx)
      set_chosen_spell_none();
    return true;
  }
  return add_spell_to_player(spl_idx, plyr_idx);
}

/**
 * Returns if the power can be used by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if the player has enough money or map position is on correct spot.
 */
TbBool is_power_available(PlayerNumber plyr_idx, SpellKind spl_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon))
        return false;
    // Player must have dungeon heart to cast spells
    if (dungeon->dnheart_idx <= 0) {
        return false;
    }
    if ((spl_idx < 0) || (spl_idx >= KEEPER_SPELLS_COUNT)) {
      ERRORLOG("Incorrect spell %ld (player %ld)",spl_idx, plyr_idx);
      return false;
    }
    if (dungeon->magic_level[spl_idx] > 0) {
      return true;
    }
    return false;
}

/**
 * Makes all the powers, which are researchable, to be instantly available.
 */
TbBool make_available_all_researchable_powers(PlayerNumber plyr_idx)
{
  struct Dungeon *dungeon;
  TbBool ret;
  long i;
  SYNCDBG(0,"Starting");
  ret = true;
  dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
      return false;
  for (i=0; i < KEEPER_SPELLS_COUNT; i++)
  {
    if (dungeon->magic_resrchable[i])
    {
      ret &= add_spell_to_player(i, plyr_idx);
    }
  }
  return ret;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
