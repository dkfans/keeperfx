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
#include "pre_inc.h"
#include "config_magic.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_effects.h"
#include "config_objects.h"
#include "config_players.h"
#include "config_cubes.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_effects.h"
#include "config_objects.h"
#include "config_rules.h"
#include "custom_sprites.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "power_process.h"
#include "game_legacy.h"
#include "console_cmd.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_magic_file[]="magic.cfg";

const struct NamedCommand magic_common_commands[] = {
  {"SPELLSCOUNT",     1},
  {"SHOTSCOUNT",      2},
  {"POWERCOUNT",      3},
  {"SPECIALSCOUNT",   4},
  {NULL,              0},
  };

const struct NamedCommand magic_spell_commands[] = {
  {"NAME",            1},
  {"DURATION",        2},
  {"SELFCASTED",      3},
  {"CASTATTHING",     4},
  {"SHOTMODEL",       5},
  {"EFFECTMODEL",     6},
  {"SYMBOLSPRITES",   7},
  {"SPELLPOWER",      8},
  {"AURAEFFECT",      9},
  {"SPELLFLAGS",     10},
  {"SUMMONCREATURE", 11},
  {NULL,              0},
  };

const struct NamedCommand magic_shot_commands[] = {
  {"NAME",                   1},
  {"HEALTH",                 2},
  {"DAMAGE",                 3},
  {"DAMAGETYPE",             4},
  {"HITTYPE",                5},
  {"AREADAMAGE",             6},
  {"SPEED",                  7},
  {"PROPERTIES",             8},
  {"PUSHONHIT",              9},
  {"FIRINGSOUND",           10},
  {"SHOTSOUND",             11},
  {"FIRINGSOUNDVARIANTS",   12},
  {"MAXRANGE",              13},
  {"ANIMATION",             14},
  {"ANIMATIONSIZE",         15},
  {"SPELLEFFECT",           16},
  {"BOUNCEANGLE",           17},
  {"SIZE_XY",               18},
  {"SIZE_YZ",               19},
  {"SIZE_Z",                19},
  {"FALLACCELERATION",      20},
  {"VISUALEFFECT",          21},
  {"VISUALEFFECTAMOUNT",    22},
  {"VISUALEFFECTSPREAD",    23},
  {"VISUALEFFECTHEALTH",    24},
  {"HITWALLEFFECT",         25},
  {"HITWALLSOUND",          26},
  {"HITCREATUREEFFECT",     27},
  {"HITCREATURESOUND",      28},
  {"HITDOOREFFECT",         29},
  {"HITDOORSOUND",          30},
  {"HITWATEREFFECT",        31},
  {"HITWATERSOUND",         32},
  {"HITLAVAEFFECT",         33},
  {"HITLAVASOUND",          34},
  {"DIGHITEFFECT",          35},
  {"DIGHITSOUND",           36},
  {"EXPLOSIONEFFECTS",      37},
  {"WITHSTANDHITAGAINST",   38},
  {"ANIMATIONTRANSPARENCY", 39},
  {"DESTROYONHIT",          40},
  {"BASEEXPERIENCEGAIN",    41},
  {"TARGETHITSTOPTURNS",    42},
  {"SHOTSOUNDPRIORITY",     43},
  {"LIGHTING",              44},
  {"INERTIA",               45},
  {"UNSHADED",              46},
  {"SOFTLANDING",           47},
  {"EFFECTMODEL",           48},
  {"FIRELOGIC",             49},
  {"UPDATELOGIC",           50},
  {"EFFECTSPACING",         51},
  {"EFFECTAMOUNT",          52},
  {NULL,                     0},
  };

const struct NamedCommand magic_power_commands[] = {
  {"NAME",            1},
  {"POWER",           2},
  {"COST",            3},
  {"DURATION",        4},
  {"CASTABILITY",     5},
  {"ARTIFACT",        6},
  {"NAMETEXTID",      7},
  {"TOOLTIPTEXTID",   8},
  {"SYMBOLSPRITES",  10},
  {"POINTERSPRITES", 11},
  {"PANELTABINDEX",  12},
  {"SOUNDSAMPLES",   13},
  {"PROPERTIES",     14},
  {"FUNCTIONS",      15},
  {"PLAYERSTATE",    16},
  {"PARENTPOWER",    17},
  {"SOUNDPLAYED",    18},
  {"COOLDOWN",       19},
  {NULL,              0},
  };

const struct NamedCommand magic_special_commands[] = {
  {"NAME",             1},
  {"ARTIFACT",         2},
  {"TOOLTIPTEXTID",    3},
  {"SPEECHPLAYED",     4},
  {"ACTIVATIONEFFECT", 5},
  {"VALUE",            6},
  {NULL,               0},
  };


const struct NamedCommand shotmodel_withstand_types[] = {
  {"CREATURE",      1},
  {"WALL",          2},
  {"DOOR",          3},
  {"WATER",         4},
  {"LAVA",          5},
  {"DIG",           6},
  {NULL,            0},
};

const struct NamedCommand shotmodel_properties_commands[] = {
  {"SLAPPABLE",            1},
  {"NAVIGABLE",            2},
  {"BOULDER",              3},
  {"REBOUND_IMMUNE",       4},
  {"DIGGING",              5},
  {"LIFE_DRAIN",           6},
  {"GROUP_UP",             7},
  {"NO_STUN",              8},
  {"NO_HIT",               9},
  {"STRENGTH_BASED",      10},
  {"ALARMS_UNITS",        11},
  {"CAN_COLLIDE",         12},
  {"EXPLODE_FLESH",       13},
  {"NO_AIR_DAMAGE",       14},
  {"WIND_IMMUNE",         15},
  {"FIXED_DAMAGE",        16},
  {"HIDDEN_PROJECTILE",   17},
  {"DISARMING",           18},
  {"BLOCKS_REBIRTH",      19},
  {"PENETRATING",         20},
  {NULL,                   0},
  };

const struct LongNamedCommand powermodel_castability_commands[] = {
  {"CUSTODY_CRTRS",    PwCast_CustodyCrtrs},
  {"OWNED_CRTRS",      PwCast_OwnedCrtrs},
  {"ALLIED_CRTRS",     PwCast_AlliedCrtrs},
  {"ENEMY_CRTRS",      PwCast_EnemyCrtrs},
  {"UNCONSC_CRTRS",    PwCast_NConscCrtrs},
  {"BOUND_CRTRS",      PwCast_BoundCrtrs},
  {"UNCLMD_GROUND",    PwCast_UnclmdGround},
  {"NEUTRL_GROUND",    PwCast_NeutrlGround},
  {"OWNED_GROUND",     PwCast_OwnedGround},
  {"ALLIED_GROUND",    PwCast_AlliedGround},
  {"ENEMY_GROUND",     PwCast_EnemyGround},
  {"NEUTRL_TALL",      PwCast_NeutrlTall},
  {"OWNED_TALL",       PwCast_OwnedTall},
  {"ALLIED_TALL",      PwCast_AlliedTall},
  {"ENEMY_TALL",       PwCast_EnemyTall},
  {"OWNED_FOOD",       PwCast_OwnedFood},
  {"NEUTRL_FOOD",      PwCast_NeutrlFood},
  {"ENEMY_FOOD",       PwCast_EnemyFood},
  {"OWNED_GOLD",       PwCast_OwnedGold},
  {"NEUTRL_GOLD",      PwCast_NeutrlGold},
  {"ENEMY_GOLD",       PwCast_EnemyGold},
  {"OWNED_SPELL",      PwCast_OwnedSpell},
  {"OWNED_BOULDERS",   PwCast_OwnedBoulders},
  {"NEEDS_DELAY",      PwCast_NeedsDelay},
  {"CLAIMABLE",        PwCast_Claimable},
  {"UNREVEALED",       PwCast_Unrevealed},
  {"REVEALED_TEMP",    PwCast_RevealedTemp},
  {"THING_OR_MAP",     PwCast_ThingOrMap},
  {"ONLY_DIGGERS",     PwCast_DiggersOnly},
  {"NO_DIGGERS",       PwCast_DiggersNot},
  {"ANYWHERE",         PwCast_Anywhere},
  {"ALL_CRTRS",        PwCast_AllCrtrs},
  {"ALL_FOOD",         PwCast_AllFood},
  {"ALL_GOLD",         PwCast_AllGold},
  {"ALL_THINGS",       PwCast_AllThings},
  {"ALL_GROUND",       PwCast_AllGround},
  {"NOT_ENEMY_GROUND", PwCast_NotEnemyGround},
  {"ALL_TALL",         PwCast_AllTall},
  {NULL,                0},
  };

const struct NamedCommand powermodel_properties_commands[] = {
    {"INSTINCTIVE",       PwCF_Instinctive},
    {"HAS_PROGRESS",      PwCF_HasProgress},
    {NULL,                0},
};

const struct NamedCommand shotmodel_damagetype_commands[] = {
  {"NONE",        DmgT_None},
  {"PHYSICAL",    DmgT_Physical},
  {"ELECTRIC",    DmgT_Electric},
  {"COMBUSTION",  DmgT_Combustion},
  {"FROSTBITE",   DmgT_Frostbite},
  {"HEATBURN",    DmgT_Heatburn},
  {"BIOLOGICAL",  DmgT_Biological},
  {"MAGICAL",     DmgT_Magical},
  {"RESPIRATORY", DmgT_Respiratory},
  {"RESTORATION", DmgT_Restoration},
  {NULL,          DmgT_None},
  };

const struct NamedCommand powermodel_expand_check_func_type[] = {
  {"general_expand",           1},
  {"sight_of_evil_expand",     2},
  {"call_to_arms_expand",      3},
  {"do_not_expand",            4},
  {NULL,                       0},
};

const Expand_Check_Func powermodel_expand_check_func_list[] = {
  NULL,
  general_expand_check,
  sight_of_evil_expand_check,
  call_to_arms_expand_check,
  NULL,
  NULL,
};

/******************************************************************************/
struct NamedCommand spell_desc[MAGIC_ITEMS_MAX];
struct NamedCommand shot_desc[MAGIC_ITEMS_MAX];
struct NamedCommand power_desc[MAGIC_ITEMS_MAX];
struct NamedCommand special_desc[MAGIC_ITEMS_MAX];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct SpellConfig *get_spell_config(int mgc_idx)
{
  if ((mgc_idx < 0) || (mgc_idx >= game.conf.magic_conf.spell_types_count))
    return &game.conf.magic_conf.spell_config[0];
  return &game.conf.magic_conf.spell_config[mgc_idx];
}

TbBool spell_config_is_invalid(const struct SpellConfig *mgcinfo)
{
  if (mgcinfo <= &game.conf.magic_conf.spell_config[0])
    return true;
  return false;
}

TextStringId get_power_name_strindex(PowerKind pwkind)
{
    if (pwkind >= game.conf.magic_conf.power_types_count)
        return game.conf.magic_conf.power_cfgstats[0].name_stridx;
    return game.conf.magic_conf.power_cfgstats[pwkind].name_stridx;
}

TextStringId get_power_description_strindex(PowerKind pwkind)
{
  if (pwkind >= game.conf.magic_conf.power_types_count)
    return game.conf.magic_conf.power_cfgstats[0].tooltip_stridx;
  return game.conf.magic_conf.power_cfgstats[pwkind].tooltip_stridx;
}

long get_special_description_strindex(int spckind)
{
  if ((spckind < 0) || (spckind >= game.conf.magic_conf.power_types_count))
    return game.conf.magic_conf.special_cfgstats[0].tooltip_stridx;
  return game.conf.magic_conf.special_cfgstats[spckind].tooltip_stridx;
}

long get_power_index_for_work_state(long work_state)
{
    for (long i = 0; i < game.conf.magic_conf.power_types_count; i++)
    {
        if (game.conf.magic_conf.power_cfgstats[i].work_state == work_state) {
            return i;
        }
    }
    return 0;
}

struct SpellConfigStats *get_spell_model_stats(SpellKind spmodel)
{
    if (spmodel >= game.conf.magic_conf.spell_types_count)
        return &game.conf.magic_conf.spell_cfgstats[0];
    return &game.conf.magic_conf.spell_cfgstats[spmodel];
}

struct ShotConfigStats *get_shot_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= game.conf.magic_conf.shot_types_count)
        return &game.conf.magic_conf.shot_cfgstats[0];
    return &game.conf.magic_conf.shot_cfgstats[tngmodel];
}

struct PowerConfigStats *get_power_model_stats(PowerKind pwkind)
{
    if (pwkind >= game.conf.magic_conf.power_types_count)
        return &game.conf.magic_conf.power_cfgstats[0];
    return &game.conf.magic_conf.power_cfgstats[pwkind];
}

TbBool power_model_stats_invalid(const struct PowerConfigStats *powerst)
{
  if (powerst <= &game.conf.magic_conf.power_cfgstats[0])
    return true;
  return false;
}

struct MagicStats *get_power_dynamic_stats(PowerKind pwkind)
{
    if (pwkind >= game.conf.magic_conf.power_types_count)
        return &game.conf.magic_conf.keeper_power_stats[0];
    return &game.conf.magic_conf.keeper_power_stats[pwkind];
}

TbBool power_is_instinctive(int pwkind)
{
    const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
    // Invalid powers are instinctive (as this usually means skipping an action)
    if (power_model_stats_invalid(powerst))
        return true;
    return ((powerst->config_flags & PwCF_Instinctive) != 0);
}

struct SpecialConfigStats *get_special_model_stats(SpecialKind spckind)
{
    if (spckind >= game.conf.magic_conf.special_types_count)
        return &game.conf.magic_conf.special_cfgstats[0];
    return &game.conf.magic_conf.special_cfgstats[spckind];
}

short write_magic_shot_to_log(const struct ShotConfigStats *shotst, int num)
{
  JUSTMSG("[shot%d]",(int)num);
  JUSTMSG("Name = %s",shotst->code_name);
  JUSTMSG("Values = %d %d",(int)shotst->damage_type,(int)shotst->experience_given_to_shooter);
  return true;
}

TbBool parse_magic_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        game.conf.magic_conf.spell_types_count = 1;
        game.conf.magic_conf.shot_types_count = 1;
        game.conf.magic_conf.power_types_count = 1;
        game.conf.magic_conf.special_types_count = 1;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "common");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, magic_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // SPELLSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= MAGIC_ITEMS_MAX))
              {
                game.conf.magic_conf.spell_types_count = k;
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
                game.conf.magic_conf.shot_types_count = k;
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
                game.conf.magic_conf.power_types_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // SPECIALSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= MAGIC_ITEMS_MAX))
              {
                game.conf.magic_conf.special_types_count = k;
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

TbBool parse_magic_spell_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct SpellConfigStats *spellst;
  struct SpellConfig *spconf;
  int i;
  // Block name and parameter word store variables
  // Initialize the array
  int arr_size = sizeof(game.conf.magic_conf.spell_cfgstats) / sizeof(game.conf.magic_conf.spell_cfgstats[0]);
    for (i=0; i < arr_size; i++)
    {
        if (((flags & CnfLd_AcceptPartial) == 0) || (strlen(game.conf.magic_conf.spell_cfgstats[i].code_name) <= 0))
        {
            spellst = get_spell_model_stats(i);
            LbMemorySet(&game.conf.magic_conf.spell_cfgstats[i].code_name, 0, COMMAND_WORD_LEN);
            if (i < game.conf.magic_conf.spell_types_count)
            {
                spell_desc[i].name = game.conf.magic_conf.spell_cfgstats[i].code_name;
                spell_desc[i].num = i;
            }
            else
            {
                spell_desc[i].name = NULL;
                spell_desc[i].num = 0;
            }

            spconf = get_spell_config(i);
            spconf->linked_power = 0;
            spconf->duration = 0;
            spconf->caster_affected = 0;
            spconf->caster_affect_sound = 0;
            spconf->cast_at_thing = 0;
            spconf->shot_model = 0;
            spconf->cast_effect_model = 0;
            spconf->bigsym_sprite_idx = 0;
            spconf->medsym_sprite_idx = 0;
            spconf->crtr_summon_model = 0;
            spconf->crtr_summon_level = 0;
            spconf->crtr_summon_amount = 0;
            spconf->aura_effect = 0;
            spconf->spell_flags = 0;
        }
    }
    
  // Load the file
  arr_size = game.conf.magic_conf.spell_types_count;
  for (i=0; i < arr_size; i++)
  {
      char block_buf[COMMAND_WORD_LEN];
      sprintf(block_buf, "spell%d", i);
      long pos = 0;
      int k = find_conf_block(buf, &pos, len, block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0)
          {
              WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
              return false;
          }
          continue;
    }
    spconf = get_spell_config(i);
    spellst = get_spell_model_stats(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_spell_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, magic_spell_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      if ((flags & CnfLd_ListOnly) != 0) {
          // In "List only" mode, accept only name command
          if (cmd_num > 1) {
              cmd_num = 0;
          }
      }
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,spellst->code_name,COMMAND_WORD_LEN) <= 0)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
              break;
          }
          if (spell_desc[i].name == NULL)
          {
              spell_desc[i].name = spellst->code_name;
              spell_desc[i].num = i;
          }
          n++;
          break;
      case 2: // DURATION
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->duration = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // SELFCASTED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->caster_affected = k;
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->caster_affect_sound = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // CASTATTHING
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->cast_at_thing = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // SHOTMODEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(shot_desc, word_buf);
              if (k >= 0) {
                  spconf->shot_model = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect shot model \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
          }
          break;
      case 6: // EFFECTMODEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(effect_desc, word_buf);
              if (k < 0)
              {
                  if (parameter_is_number(word_buf))
                  {
                      k = atoi(word_buf);
                      spconf->cast_effect_model = k;
                      n++;
                  }
              } else
              {
                  spconf->cast_effect_model = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect effect model \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
          }
          break;
      case 7: // SYMBOLSPRITES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              spconf->bigsym_sprite_idx = bad_icon_id;
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  spconf->bigsym_sprite_idx = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              spconf->medsym_sprite_idx = bad_icon_id;
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  spconf->medsym_sprite_idx = k;
                  n++;
              }
          }
          if (n < 2)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // SPELLPOWER
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              if (parameter_is_number(word_buf))
              {
                  k = atoi(word_buf);
              }
              else
              {
                  k = get_id(power_desc, word_buf);
              }
              if (k >= 0)
              {
                  spconf->linked_power = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 9: // AURAEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              if (parameter_is_number(word_buf))
              {
                  k = atoi(word_buf);
                  spconf->aura_effect = k;
                  n++;
              }
              else
              {
                  k = get_id(effect_desc, word_buf);
                  if (k >= 0)
                  {
                      spconf->aura_effect = k;
                      n++;
                  }
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 10: // SPELLFLAGS
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  spconf->spell_flags = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
        case 11: // SUMMONCREATURE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = get_id(creature_desc, word_buf);
              if (k < 0)
              {
                  if (parameter_is_number(word_buf))
                  {
                      k = atoi(word_buf);
                      spconf->crtr_summon_model = k;
                      n++;
                  }
              }
              else
              {
                  spconf->crtr_summon_model = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->crtr_summon_level = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->crtr_summon_amount = k;
              n++;
          }
          if (n < 3)
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

TbBool parse_magic_shot_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct ShotConfigStats *shotst;
  int i;
  // Block name and parameter word store variables
  // Initialize the array
  int arr_size = sizeof(game.conf.magic_conf.shot_cfgstats) / sizeof(game.conf.magic_conf.shot_cfgstats[0]);
    for (i = 0; i < arr_size; i++)
    {
        shotst = get_shot_model_stats(i);
        if (((flags & CnfLd_AcceptPartial) == 0) || (strlen(shotst->code_name) <= 0))
        {
            LbMemorySet(shotst->code_name, 0, COMMAND_WORD_LEN);
            shotst->model_flags = 0;
            if (i < game.conf.magic_conf.shot_types_count)
            {
                shot_desc[i].name = shotst->code_name;
                shot_desc[i].num = i;
            }
            else
            {
                shot_desc[i].name = NULL;
                shot_desc[i].num = 0;
            }
            shotst->area_hit_type = THit_CrtrsOnly;
            shotst->area_range = 0;
            shotst->area_damage = 0;
            shotst->area_blow = 0;
            shotst->bounce_angle = 0;
            shotst->damage = 0;
            shotst->fall_acceleration = 0;
            shotst->hidden_projectile = 0;
            shotst->hit_door.withstand = 0;
            shotst->hit_generic.withstand = 0;
            shotst->hit_lava.withstand = 0;
            shotst->hit_water.withstand = 0;
            shotst->no_air_damage = 0;
            shotst->push_on_hit = 0;
            shotst->max_range = 0;
            shotst->size_xy = 0;
            shotst->size_z = 0;
            shotst->speed = 0;
            shotst->destroy_on_first_hit = 0;
            shotst->experience_given_to_shooter = 0;
            shotst->wind_immune = 0;
            shotst->animation_transparency = 0;
            shotst->fixed_damage = 0;
            shotst->sound_priority = 0;
            shotst->light_radius = 0;
            shotst->light_intensity = 0;
            shotst->lightf_53 = 0;
            shotst->inertia_air = 0;
            shotst->inertia_floor = 0;
            shotst->target_hitstop_turns = 0;
            shotst->soft_landing = 0;
        }
    }
  // Load the file
  arr_size = game.conf.magic_conf.shot_types_count;
  for (i=0; i < arr_size; i++)
  {
      char block_buf[COMMAND_WORD_LEN];
      sprintf(block_buf, "shot%d", i);
      long pos = 0;
      int k = find_conf_block(buf, &pos, len, block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0)
          {
              WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
              return false;
          }
          continue;
      }
      shotst = get_shot_model_stats(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_shot_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, magic_shot_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      if ((flags & CnfLd_ListOnly) != 0) {
          // In "List only" mode, accept only name command
          if (cmd_num > 1) {
              cmd_num = 0;
          }
      }
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,shotst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
            break;
          }
          if (shot_desc[i].name == NULL) 
          {
              shot_desc[i].name = shotst->code_name;
              shot_desc[i].num = i;
          }
          n++;
          break;
      case 2: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            shotst->health = k;
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
            shotst->damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // DAMAGETYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(shotmodel_damagetype_commands, word_buf);
              if (k >= 0) {
                  shotst->damage_type = k;
                  n++;
              }
          }
          if (n < 1)
          {
              //CONFWRNLOG("Incorrect shot model \"%s\" in [%s] block of %s file.",word_buf,block_buf,config_textname);
              shotst->damage_type = 0; //Default damage type to "none", to allow empty values in config.
              break;
          }
          break;
      case 5: // HITTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->area_hit_type = k;
              n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 6: // AREADAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->area_range = k;
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->area_damage = k;
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->area_blow = k;
              n++;
          }
          if (n < 3)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // SPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            shotst->speed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // PROPERTIES
          shotst->model_flags = 0;
          shotst->no_air_damage = 0;
          shotst->wind_immune = 0;
          shotst->hidden_projectile = 0;
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
            case 3: // BOULDER
                shotst->model_flags |= ShMF_Boulder;
                n++;
                break;
            case 4: // REBOUND_IMMUNE
                shotst->model_flags |= ShMF_ReboundImmune;
                n++;
                break;
            case 5: // DIGGING
                shotst->model_flags |= ShMF_Digging;
                n++;
                break;
            case 6: // LIFE_DRAIN
                shotst->model_flags |= ShMF_LifeDrain;
                n++;
                break;
            case 7: // GROUP_UP
                shotst->model_flags |= ShMF_GroupUp;
                n++;
                break;
            case 8: // NO_STUN
                shotst->model_flags |= ShMF_NoStun;
                n++;
                break;
            case 9: // NO_HIT
                shotst->model_flags |= ShMF_NoHit;
                n++;
                break;
            case 10: // STRENGTH_BASED
                shotst->model_flags |= ShMF_StrengthBased;
                n++;
                break;
            case 11: // ALARMS_UNITS
                shotst->model_flags |= ShMF_AlarmsUnits;
                n++;
                break;
            case 12: // CAN_COLLIDE
                shotst->model_flags |= ShMF_CanCollide;
                n++;
                break;
            case 13: // EXPLODE_FLESH
                shotst->model_flags |= ShMF_Exploding;
                n++;
                break;
            case 14: // NO_AIR_DAMAGE
                shotst->no_air_damage = 1;
                n++;
                break;
            case 15: // WIND_IMMUNE
                shotst->wind_immune = 1;
                n++;
                break;
            case 16: // FIXED_DAMAGE
                shotst->fixed_damage = 1;
                n++;
                break;
            case 17: // HIDDEN_PROJECTILE
                shotst->hidden_projectile = 1;
                n++;
                break;
            case 18: // DISARMING
                shotst->model_flags |= ShMF_Disarming;
                n++;
                break;
            case 19: // BlocksRebirth
                shotst->model_flags |= ShMF_BlocksRebirth;
                n++;
                break;
            case 20: // Penetrating
                shotst->model_flags |= ShMF_Penetrating;
                n++;
                break;
            default:
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
            }
          }
          break;
      case 9: // PUSHONHIT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->push_on_hit = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
       case 10: //FIRINGSOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->firing_sound = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
                   break;
      case 11: //SHOTSOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->shot_sound = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
                   break;
      case 12: //FIRINGSOUNDVARIANTS
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->firing_sound_variants = k;
              n++;
          }
          if (n < 1)
          {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
                   break;
      case 13: //MAXRANGE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->max_range = k;
              n++;
          }
          if (n < 1)
          {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
                   break;
      case 14: //ANIMATION
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              struct ObjectConfigStats obj_tmp;
              k = get_anim_id(word_buf, &obj_tmp);
              shotst->sprite_anim_idx = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 15: //ANIMATIONSIZE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->sprite_size_max = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 16: //SPELLEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->cast_spell_kind = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 17: //BOUNCEANGLE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->bounce_angle = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 18: //SIZE_XY
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->size_xy = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 19: //SIZE_Z
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->size_z = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 20: //FALLACCELERATION
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->fall_acceleration = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break; 
      case 21: //VISUALEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->visual.effect_model = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 22: //VISUALEFFECTAMOUNT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->visual.amount = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 23: //VISUALEFFECTSPREAD
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->visual.random_range = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 24: //VISUALEFFECTHEALTH
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->visual.shot_health = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 25: //HITWALLEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_generic.effect_model = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 26: //HITWALLSOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_generic.sndsample_idx = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_generic.sndsample_range = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 27: //HITCREATUREEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_creature.effect_model = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 28: //HITCREATURESOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_creature.sndsample_idx = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_creature.sndsample_range = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 29: //HITDOOREFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_door.effect_model = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 30: //HITDOORSOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_door.sndsample_idx = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_door.sndsample_range = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 31: //HITWATEREFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_water.effect_model = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 32: //HITWATERSOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_water.sndsample_idx = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_water.sndsample_range = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 33: //HITLAVAEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_lava.effect_model = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 34: //HITLAVASOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_lava.sndsample_idx = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->hit_lava.sndsample_range = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 35: //DIGHITEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->dig.effect_model = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 36: //DIGHITSOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->dig.sndsample_idx = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->dig.sndsample_range = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 37: // EXPLOSIONEFFECTS
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->explode.effect1_model = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->explode.effect2_model = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->explode.around_effect1_model = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->explode.around_effect2_model = k;
              n++;
          }
          if (n < 4)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 38: //WITHSTANDHITAGAINST
          while (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = get_id(shotmodel_withstand_types, word_buf);
              switch (k)
              {
              case 1: // CREATURE
                  shotst->hit_creature.withstand = 1;
                  n++;
                  break;
              case 2: // WALL
                  shotst->hit_generic.withstand = 1;
                  n++;
                  break;
              case 3: // DOOR
                  shotst->hit_door.withstand = 1;
                  n++;
                  break;
              case 4: // WATER
                  shotst->hit_water.withstand = 1;
                  n++;
                  break;
              case 5: // LAVA
                  shotst->hit_lava.withstand = 1;
                  n++;
                  break;
              case 6: // DIG
                  shotst->dig.withstand = 1;
                  n++;
                  break;
              default:
                  CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num), word_buf, block_buf, config_textname);
              }
          }
          break;
      case 39: //ANIMATIONTRANSPARENCY
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->animation_transparency = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 40: //DESTROYONHIT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->destroy_on_first_hit = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 41: //BASEEXPERIENCEGAIN
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->experience_given_to_shooter = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 42: //TARGETHITSTOPTURNS
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->target_hitstop_turns = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 43: //SHOTSOUNDPRIORITY
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->sound_priority = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 44: // LIGHTING
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->light_radius = k * COORD_PER_STL;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->light_intensity = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->lightf_53 = k;
              n++;
          }
          if (n < 3)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 45: // INERTIA
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->inertia_floor = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->inertia_air = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 46: //UNSHADED
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->unshaded = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 47: //SOFTLANDING
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->soft_landing = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 48: //EFFECTMODEL
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = effect_or_effect_element_id(word_buf);
              shotst->effect_id = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 49: //FIRELOGIC
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->fire_logic = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 50: //UPDATELOGIC
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->update_logic = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 51: //EFFECTSPACING
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->effect_spacing = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 52: //EFFECTAMOUNT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              shotst->effect_amount = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
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
    //write_magic_shot_to_log(shotst, i);
#undef COMMAND_TEXT
  }
  return true;
}

TbBool parse_magic_power_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct PowerConfigStats *powerst;
  int i;
  // Block name and parameter word store variables
  // Initialize the array
  int arr_size;
  arr_size = sizeof(game.conf.magic_conf.power_cfgstats) / sizeof(game.conf.magic_conf.power_cfgstats[0]);
  for (i = 0; i < arr_size; i++)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
      {
          if ((i < game.conf.magic_conf.power_types_count) || (strlen(power_desc[i].name) <= 0))
          {
              powerst = get_power_model_stats(i);
              LbMemorySet(powerst->code_name, 0, COMMAND_WORD_LEN);
              powerst->artifact_model = 0;
              powerst->can_cast_flags = 0;
              powerst->config_flags = 0;
              powerst->overcharge_check_idx = 0;
              powerst->work_state = 0;
              powerst->bigsym_sprite_idx = 0;
              powerst->medsym_sprite_idx = 0;
              powerst->name_stridx = 0;
              powerst->tooltip_stridx = 0;
              powerst->select_sample_idx = 0;
              powerst->pointer_sprite_idx = 0;
              powerst->panel_tab_idx = 0;
              powerst->select_sound_idx = 0;
              powerst->cast_cooldown = 0;
              power_desc[i].name = powerst->code_name;
              power_desc[i].num = i;
          } else
          {
              power_desc[i].name = NULL;
              power_desc[i].num = 0;
          }
      }
      arr_size = sizeof(game.conf.object_conf.object_to_power_artifact)/sizeof(game.conf.object_conf.object_to_power_artifact[0]);
      for (i = 0; i < arr_size; i++)
      {
          if ((flags & CnfLd_AcceptPartial) == 0)
          {
              game.conf.object_conf.object_to_power_artifact[i] = 0;
          }
      }
  }
  arr_size = game.conf.magic_conf.power_types_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
      char block_buf[COMMAND_WORD_LEN];
      sprintf(block_buf, "power%d", i);
      long pos = 0;
      long long k = find_conf_block(buf, &pos, len, block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0)
          {
              WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
              return false;
          }
          continue;
    }
    struct MagicStats* pwrdynst = get_power_dynamic_stats(i);
    powerst = get_power_model_stats(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_power_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, magic_power_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      if ((flags & CnfLd_ListOnly) != 0) {
          // In "List only" mode, accept only name command
          if (cmd_num > 1) {
              cmd_num = 0;
          }
      }
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,powerst->code_name,COMMAND_WORD_LEN) <= 0)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
              break;
          }
          power_desc[i].name = powerst->code_name;
          power_desc[i].num = i;
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
              pwrdynst->strength[n] = k;
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
              pwrdynst->cost[n] = k;
              n++;
          }
          if (n <= SPELL_MAX_LEVEL)
          {
              CONFWRNLOG("Couldn't read all \"%s\" parameters in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // Duration
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              pwrdynst->duration = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // CASTABILITY
          powerst->can_cast_flags = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_long_id(powermodel_castability_commands, word_buf);
              if ((k != 0) && (k != -1))
              {
                  powerst->can_cast_flags |= k;
                  n++;
              } else
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
          }
          break;
      case 6: // ARTIFACT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(object_desc, word_buf);
              if (k >= 0) {
                  powerst->artifact_model = k;
                  game.conf.object_conf.object_to_power_artifact[k] = i;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect object model \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
          }
          break;
      case 7: // NAMETEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              powerst->name_stridx = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // TOOLTIPTEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              powerst->tooltip_stridx = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 10: // SYMBOLSPRITES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              powerst->bigsym_sprite_idx = bad_icon_id;
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  powerst->bigsym_sprite_idx = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              powerst->medsym_sprite_idx = bad_icon_id;
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  powerst->medsym_sprite_idx = k;
                  n++;
              }
          }
          if (n < 2)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 11: // POINTERSPRITES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  powerst->pointer_sprite_idx = k;
                  n++;
              }
          }
          if (n < 1)
          {
              powerst->pointer_sprite_idx = bad_icon_id;
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 12: // PANELTABINDEX
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                powerst->panel_tab_idx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 13: // SOUNDSAMPLES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                powerst->select_sample_idx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 14: // PROPERTIES
          powerst->config_flags = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(powermodel_properties_commands, word_buf);
              if (k > 0) {
                  powerst->config_flags |= k;
                  n++;
              } else {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
          }
          break;
      case 15: // FUNCTIONS
          powerst->overcharge_check_idx = 0;
          k = recognize_conf_parameter(buf,&pos,len,powermodel_expand_check_func_type);
          if (k > 0)
          {
              powerst->overcharge_check_idx = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 16: // PLAYERSTATE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(player_state_commands, word_buf);
              if (k >= 0) {
                  powerst->work_state = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect object model \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
          }
          break;
      case 17: // PARENTPOWER
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(power_desc, word_buf);
              if (k >= 0) {
                  powerst->parent_power = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect object model \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
          }
          break;
          case 18: //SOUNDPLAYED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                powerst->select_sound_idx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
          case 19: //COOLDOWN
              if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
              {
                  k = atoi(word_buf);
                  if (k >= 0)
                  {
                      powerst->cast_cooldown = k;
                      n++;
                  }
              }
              if (n < 1)
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num), block_buf, config_textname);
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
  if ((flags & CnfLd_ListOnly) == 0)
  {
    // Mark powers which have children
    for (i = 0; i < game.conf.magic_conf.power_types_count; i++)
    {
        powerst = get_power_model_stats(i);
        struct PowerConfigStats* parent_powerst = get_power_model_stats(powerst->parent_power);
        if (!power_model_stats_invalid(parent_powerst)) {
            parent_powerst->config_flags |= PwCF_IsParent;
        }
    }
  }
  return true;
}

TbBool parse_magic_special_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct SpecialConfigStats *specst;
  int i;
  // Block name and parameter word store variables
  // Initialize the array
  int arr_size;
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      arr_size = sizeof(game.conf.magic_conf.special_cfgstats)/sizeof(game.conf.magic_conf.special_cfgstats[0]);
      for (i=0; i < arr_size; i++)
      {
          specst = get_special_model_stats(i);
          LbMemorySet(specst->code_name, 0, COMMAND_WORD_LEN);
          specst->artifact_model = 0;
          specst->tooltip_stridx = 0;
          if (i < game.conf.magic_conf.special_types_count)
          {
              special_desc[i].name = specst->code_name;
              special_desc[i].num = i;
          } else
          {
              special_desc[i].name = NULL;
              special_desc[i].num = 0;
          }
      }
      arr_size = sizeof(game.conf.object_conf.object_to_special_artifact)/sizeof(game.conf.object_conf.object_to_special_artifact[0]);
      for (i=0; i < arr_size; i++) {
          game.conf.object_conf.object_to_special_artifact[i] = 0;
      }
  }
  arr_size = game.conf.magic_conf.special_types_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
      char block_buf[COMMAND_WORD_LEN];
      sprintf(block_buf, "special%d", i);
      long pos = 0;
      int k = find_conf_block(buf, &pos, len, block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0)
          {
              WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
              return false;
          }
          continue;
    }
    specst = get_special_model_stats(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_special_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, magic_special_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      if ((flags & CnfLd_ListOnly) != 0) {
          // In "List only" mode, accept only name command
          if (cmd_num > 1) {
              cmd_num = 0;
          }
      }
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,specst->code_name,COMMAND_WORD_LEN) <= 0)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
              break;
          }
          break;
      case 2: // ARTIFACT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(object_desc, word_buf);
              if (k >= 0) {
                  specst->artifact_model = k;
                  game.conf.object_conf.object_to_special_artifact[k] = i;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect object model \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
          }
          break;
      case 3: // TOOLTIPTEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
                specst->tooltip_stridx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // SPEECHPLAYED
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  specst->speech = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 5: // ACTIVATIONEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = effect_or_effect_element_id(word_buf);
              specst->effect_id = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 6: // VALUE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              specst->value = k;
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

TbBool load_magic_config_file(const char *textname, const char *fname, unsigned short flags)
{
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    char* buf = (char*)LbMemoryAlloc(len + 256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file
    if (result)
    {
        result = parse_magic_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_magic_spell_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" spell blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_magic_shot_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" shot blocks failed.",textname,fname);
    }
    if (result)
    {
      result = parse_magic_power_blocks(buf, len, textname, flags);
      if ((flags & CnfLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s\" power blocks failed.",textname,fname);
    }
    if (result)
    {
      result = parse_magic_special_blocks(buf, len, textname, flags);
      if ((flags & CnfLd_AcceptPartial) != 0)
          result = true;
      if (!result)
          WARNMSG("Parsing %s file \"%s\" special blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_magic_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global magic config";
    static const char config_campgn_textname[] = "campaign magic config";
    static const char config_level_textname[] = "level magic config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_magic_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_magic_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), conf_fname);
    if (strlen(fname) > 0)
    {
        load_magic_config_file(config_level_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given spell model.
 */
const char *spell_code_name(SpellKind spmodel)
{
    const char* name = get_conf_parameter_text(spell_desc, spmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given shot model.
 */
const char *shot_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(shot_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given keepers power kind.
 */
const char *power_code_name(PowerKind pwkind)
{
    const char* name = get_conf_parameter_text(power_desc, pwkind);
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
    for (int i = 0; i < game.conf.magic_conf.power_types_count; ++i)
    {
        if (strncmp(game.conf.magic_conf.power_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Adds given power to players available powers.
 *
 * @param pwkind
 * @param plyr_idx
 * @return
 * @note originally add_spell_to_player()
 */
TbBool add_power_to_player(PowerKind pwkind, PlayerNumber plyr_idx)
{
    if (pwkind >= game.conf.magic_conf.power_types_count)
    {
        ERRORLOG("Can't add incorrect power %d to player %d",(int)pwkind, (int)plyr_idx);
        return false;
    }
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Can't add %s to player %d which has no dungeon",power_code_name(pwkind), (int)plyr_idx);
        return false;
    }
    long i = dungeon->magic_level[pwkind];
    if (i >= 255)
    {
        ERRORLOG("Power %s has bad magic_level=%d for player %d, reset", power_code_name(pwkind), (int)i, (int)plyr_idx);
        i = 0;
    }
    dungeon->magic_level[pwkind] = i+1;
    return true;
}

void remove_power_from_player(PowerKind pwkind, PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Cannot remove spell %s from invalid dungeon %d!",power_code_name(pwkind),(int)plyr_idx);
        return;
    }
    long i = dungeon->magic_level[pwkind];
    if (i < 1)
    {
        ERRORLOG("Cannot remove spell %s (%d) from player %d as he doesn't have it!",power_code_name(pwkind),(int)pwkind,(int)plyr_idx);
        return;
    }
    SYNCDBG(4,"Decreasing spell %s of player %d to level %d",power_code_name(pwkind),(int)plyr_idx,(int)i-1);
    dungeon->magic_level[pwkind] = i-1;
    switch (pwkind)
    {
    case PwrK_OBEY:
        if (player_uses_power_obey(plyr_idx))
            turn_off_power_obey(plyr_idx);
        break;
    case PwrK_SIGHT:
        if (player_uses_power_sight(plyr_idx))
            turn_off_power_sight_of_evil(plyr_idx);
        break;
    case PwrK_CALL2ARMS:
        if (player_uses_power_call_to_arms(plyr_idx))
            turn_off_power_call_to_arms(plyr_idx);
        break;
    }
    if (game.chosen_spell_type == pwkind)
    {
        set_chosen_power_none();
    }
}

/**
 * Zeroes all the costs for all spells.
 */
TbBool make_all_powers_cost_free(void)
{
    for (long i = 0; i < game.conf.magic_conf.power_types_count; i++)
    {
        struct MagicStats* pwrdynst = get_power_dynamic_stats(i);
        for (long n = 0; n < MAGIC_OVERCHARGE_LEVELS; n++)
            pwrdynst->cost[n] = 0;
  }
  return true;
}

/**
 * Makes all keeper spells to be available to research.
 */
TbBool make_all_powers_researchable(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    for (long i = 0; i < game.conf.magic_conf.power_types_count; i++)
    {
        dungeon->magic_resrchable[i] = 1;
    }
    return true;
}

/**
 * Sets power availability state.
 */
TbBool set_power_available(PlayerNumber plyr_idx, PowerKind pwkind, long resrch, long avail)
{
    SYNCDBG(8,"Starting for power %d, player %d, state %ld,%ld",(int)pwkind,(int)plyr_idx,resrch,avail);
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot set power availability; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    dungeon->magic_resrchable[pwkind] = resrch;
    if (avail <= 0)
    {
        if (is_power_available(plyr_idx, pwkind))
        {
            remove_power_from_player(pwkind, plyr_idx);
        }
        return true;
    }
    if (is_power_available(plyr_idx, pwkind))
    {
        return true;
    }
    return add_power_to_player(pwkind, plyr_idx);
}

/**
 * Returns if the power can be used by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if the player has enough money or map position is on correct spot.
 */
TbBool is_power_available(PlayerNumber plyr_idx, PowerKind pwkind)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    //TODO POWERS Mapping child powers to their parent - remove that when magic_level array is enlarged
    {
        const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
        if (powerst->parent_power != 0)
            pwkind = powerst->parent_power;
    }
    // Player must have dungeon heart to cast spells, with no heart only floating spirit spell works
    if (!player_has_heart(plyr_idx) && (pwkind != PwrK_POSSESS)) {
        return false;
    }
    if (pwkind >= game.conf.magic_conf.power_types_count)
    {
        ERRORLOG("Incorrect power %ld (player %ld)", pwkind, plyr_idx);
        return false;
    }
    if (dungeon->magic_level[pwkind] > 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the power can be or already is obtained by a player.
 */
TbBool is_power_obtainable(PlayerNumber plyr_idx, PowerKind pwkind)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    //TODO POWERS Mapping child powers to their parent - remove that when magic_level array is enlarged
    {
        const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
        if (powerst->parent_power != 0)
            pwkind = powerst->parent_power;
    }
    // Player must have dungeon heart to cast spells, with no heart only floating spirit spell works
    if (!player_has_heart(plyr_idx) && (pwkind != PwrK_POSSESS)) {
        return false;
    }
    if (pwkind >= game.conf.magic_conf.power_types_count) {
        ERRORLOG("Incorrect power %ld (player %ld)",pwkind, plyr_idx);
        return false;
    }
    return (dungeon->magic_level[pwkind] > 0) || (dungeon->magic_resrchable[pwkind]);
}

/**
 * Makes all the powers, which are researchable, to be instantly available.
 */
TbBool make_available_all_researchable_powers(PlayerNumber plyr_idx)
{
  SYNCDBG(0,"Starting");
  TbBool ret = true;
  struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon)) {
      ERRORDBG(11,"Cannot make research available; player %d has no dungeon",(int)plyr_idx);
      return false;
  }
  for (long i = 0; i < game.conf.magic_conf.power_types_count; i++)
  {
    if (dungeon->magic_resrchable[i])
    {
      ret &= add_power_to_player(i, plyr_idx);
    }
  }
  return ret;
}

/******************************************************************************/
