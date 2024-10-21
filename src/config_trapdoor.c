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
#include "pre_inc.h"
#include "config_trapdoor.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_sound.h"

#include "config.h"
#include "config_strings.h"
#include "console_cmd.h"
#include "thing_doors.h"
#include "player_instances.h"
#include "config_players.h"
#include "game_legacy.h"
#include "custom_sprites.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_trapdoor_file[]="trapdoor.cfg";

const struct NamedCommand trapdoor_door_commands[] = {
  {"NAME",                  1},
  {"MANUFACTURELEVEL",      2},
  {"MANUFACTUREREQUIRED",   3},
  {"SLABKIND",              4},
  {"HEALTH",                5},
  {"SELLINGVALUE",          6},
  {"NAMETEXTID",            7},
  {"TOOLTIPTEXTID",         8},
  {"CRATE",                 9},
  {"SYMBOLSPRITES",        10},
  {"POINTERSPRITES",       11},
  {"PANELTABINDEX",        12},
  {"OPENSPEED",            13},
  {"PROPERTIES",           14},
  {"PLACESOUND",           15},
  {"UNSELLABLE",           16},
  {NULL,                    0},
};

const struct NamedCommand trapdoor_trap_commands[] = {
  {"NAME",                  1},
  {"MANUFACTURELEVEL",      2},
  {"MANUFACTUREREQUIRED",   3},
  {"SHOTS",                 4},
  {"TIMEBETWEENSHOTS",      5},
  {"SELLINGVALUE",          6},
  {"NAMETEXTID",            7},
  {"TOOLTIPTEXTID",         8},
  {"CRATE",                 9},
  {"SYMBOLSPRITES",        10},
  {"POINTERSPRITES",       11},
  {"PANELTABINDEX",        12},
  {"TRIGGERTYPE",          13},
  {"ACTIVATIONTYPE",       14},
  {"EFFECTTYPE",           15},
  {"ANIMATIONID",          16},
  {"MODEL",                16},//backward compatibility
  {"MODELSIZE",            17},
  {"ANIMATIONSPEED",       18},
  {"UNANIMATED",           19},
  {"HIDDEN",               20},
  {"SLAPPABLE",            21},
  {"TRIGGERALARM",         22},
  {"HEALTH",               23},
  {"UNSHADED",             24},
  {"RANDOMSTARTFRAME",     25},
  {"THINGSIZE",            26},
  {"HITTYPE",              27},
  {"LIGHTRADIUS",          28},
  {"LIGHTINTENSITY",       29},
  {"LIGHTFLAGS",           30},
  {"TRANSPARENCYFLAGS",    31},
  {"SHOTVECTOR",           32},
  {"DESTRUCTIBLE",         33},
  {"UNSTABLE",             34},
  {"UNSELLABLE",           35},
  {"PLACEONBRIDGE",        36},
  {"SHOTORIGIN",           37},
  {"PLACESOUND",           38},
  {"TRIGGERSOUND",         39},
  {"RECHARGEANIMATIONID",  40},
  {"ATTACKANIMATIONID",    41},
  {"DESTROYEDEFFECT",      42},
  {"INITIALDELAY",         43},
  {"PLACEONSUBTILE",       44},
  {"FLAMEANIMATIONID",     45},
  {"FLAMEANIMATIONSPEED",  46},
  {"FLAMEANIMATIONSIZE",   47},
  {"FLAMEANIMATIONOFFSET",    48},
  {"FLAMETRANSPARENCYFLAGS",  49},
  {"DETECTINVISIBLE",         50},
  {NULL,                       0},
};

const struct NamedCommand door_properties_commands[] = {
  {"RESIST_NON_MAGIC",     1},
  {"SECRET",               2},
  {"THICK",                3},  
  {"MIDAS",                4},
  {NULL,                   0},
  };


/******************************************************************************/
struct NamedCommand trap_desc[TRAPDOOR_TYPES_MAX];
struct NamedCommand door_desc[TRAPDOOR_TYPES_MAX];
/******************************************************************************/
struct TrapConfigStats *get_trap_model_stats(int tngmodel)
{
    if (tngmodel >= game.conf.trapdoor_conf.trap_types_count)
        return &game.conf.trapdoor_conf.trap_cfgstats[0];
    return &game.conf.trapdoor_conf.trap_cfgstats[tngmodel];
}

struct DoorConfigStats *get_door_model_stats(int tngmodel)
{
    if (tngmodel >= game.conf.trapdoor_conf.door_types_count)
        return &game.conf.trapdoor_conf.door_cfgstats[0];
    return &game.conf.trapdoor_conf.door_cfgstats[tngmodel];
}

/**
 * Returns manufacture data for a given manufacture index.
 * @param manufctr_idx Manufacture array index.
 * @return Dummy entry pinter if not found, manufacture data pointer otherwise.
 */
struct ManufactureData *get_manufacture_data(int manufctr_idx)
{
    if ((manufctr_idx < 0) || (manufctr_idx >= game.conf.trapdoor_conf.manufacture_types_count)) {
        return &game.conf.trapdoor_conf.manufacture_data[0];
    }
    return &game.conf.trapdoor_conf.manufacture_data[manufctr_idx];
}

/**
 * Finds index into manufactures data array for a given trap/door class and model.
 * @param tngclass Manufacturable thing class.
 * @param tngmodel Manufacturable thing model.
 * @return 0 if not found, otherwise index where 1 <= index < manufacture_types_count
 */
int get_manufacture_data_index_for_thing(ThingClass tngclass, ThingModel tngmodel)
{
    for (int i = 1; i < game.conf.trapdoor_conf.manufacture_types_count; i++)
    {
        struct ManufactureData* manufctr = &game.conf.trapdoor_conf.manufacture_data[i];
        if ((manufctr->tngclass == tngclass) && (manufctr->tngmodel == tngmodel)) {
            return i;
        }
    }
    return 0;
}

TbBool parse_trapdoor_trap_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct ManfctrConfig *mconf;
  struct TrapConfigStats *trapst;
  // Block name and parameter word store variables
  SYNCDBG(19,"Starting");
  // Initialize the traps array
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      for (int i=0; i < TRAPDOOR_TYPES_MAX; i++)
      {
          trapst = &game.conf.trapdoor_conf.trap_cfgstats[i];
          LbMemorySet(trapst->code_name, 0, COMMAND_WORD_LEN);
          trapst->name_stridx = GUIStr_Empty;
          trapst->tooltip_stridx = GUIStr_Empty;
          trapst->bigsym_sprite_idx = 0;
          trapst->medsym_sprite_idx = 0;
          trapst->pointer_sprite_idx = 0;
          // Default trap sounds, so that they aren't broken if custom trap is bundled into map.
          trapst->place_sound_idx = 117; 
          trapst->trigger_sound_idx = 176;
          trapst->panel_tab_idx = 0;
          trapst->hidden = false;
          trapst->slappable = 0;
          trapst->destructible = 0;
          trapst->unstable = 0;
          trapst->unsellable = false;
          trapst->notify = false;
          trapst->place_on_bridge = false;
          trapst->place_on_subtile = false;
          // Default destroyed_effect is TngEffElm_Blast2.
          trapst->destroyed_effect = -39;

          game.conf.trap_stats[i].health = 0;
          game.conf.trap_stats[i].sprite_anim_idx = 0;
          game.conf.trap_stats[i].recharge_sprite_anim_idx = 0;
          game.conf.trap_stats[i].attack_sprite_anim_idx = 0;
          game.conf.trap_stats[i].sprite_size_max = 0;
          game.conf.trap_stats[i].unanimated = 0;
          game.conf.trap_stats[i].anim_speed = 0;
          game.conf.trap_stats[i].unshaded = 0;
          game.conf.trap_stats[i].transparency_flag = 0;
          game.conf.trap_stats[i].random_start_frame = 0;
          game.conf.trap_stats[i].size_xy = 0;
          game.conf.trap_stats[i].size_z = 0;
          game.conf.trap_stats[i].trigger_type = 0;
          game.conf.trap_stats[i].activation_type = 0;
          game.conf.trap_stats[i].created_itm_model = 0;
          game.conf.trap_stats[i].hit_type = 0;
          game.conf.trap_stats[i].light_radius = 0;
          game.conf.trap_stats[i].light_intensity = 0;
          game.conf.trap_stats[i].light_flag = 0;
          game.conf.trap_stats[i].shotvector.x = 0;
          game.conf.trap_stats[i].shotvector.y = 0;
          game.conf.trap_stats[i].shotvector.z = 0;
          game.conf.trap_stats[i].shot_shift_x = 0;
          game.conf.trap_stats[i].shot_shift_y = 0;
          game.conf.trap_stats[i].shot_shift_z = 0;
          game.conf.trap_stats[i].initial_delay = 0;
          game.conf.trap_stats[i].detect_invisible = 1; // Set to 1 by default: backward compatibility for custom traps made before this implementation.
          mconf = &game.conf.traps_config[i];
          mconf->manufct_level = 0;
          mconf->manufct_required = 0;
          mconf->shots = 0;
          mconf->shots_delay = 0;
          mconf->selling_value = 0;
          trap_desc[i].name = trapst->code_name;
          trap_desc[i].num = i;
      }
  }
  trap_desc[TRAPDOOR_TYPES_MAX - 1].name = NULL; // must be null for get_id
  // Parse every numbered block within range
  const char * blockname = NULL;
  int blocknamelen = 0;
  long pos = 0;
  int k = 0;
  while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
  {
    // look for blocks starting with "trap", followed by one or more digits
    if (blocknamelen < 5) {
        continue;
    } else if (memcmp(blockname, "trap", 4) != 0) {
        continue;
    }
    const int i = natoi(&blockname[4], blocknamelen - 4);
    if (i < 0 || i >= TRAPDOOR_TYPES_MAX) {
        continue;
    } else if (i >= game.conf.trapdoor_conf.trap_types_count) {
        game.conf.trapdoor_conf.trap_types_count = i + 1;
    }
    mconf = &game.conf.traps_config[i];
    trapst = &game.conf.trapdoor_conf.trap_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_trap_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, trapdoor_trap_commands);
      SYNCDBG(19,"Command %s",COMMAND_TEXT(cmd_num));
      // Now store the config item in correct place
      if (cmd_num == ccr_endOfBlock) break; // if next block starts
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
          if (get_conf_parameter_single(buf,&pos,len,trapst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 9: // CRATE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              n = get_id(object_desc, word_buf);
          }
          if (n < 0)
          {
              CONFWRNLOG("Incorrect crate object \"%s\" in [%.*s] block of %s file.",
                  word_buf, blocknamelen, blockname, config_textname);
              break;
          }
          game.conf.object_conf.object_to_door_or_trap[n] = i;
          game.conf.object_conf.workshop_object_class[n] = TCls_Trap;
          game.conf.trapdoor_conf.trap_to_object[i] = n;
          break;
      case 10: // SYMBOLSPRITES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              trapst->bigsym_sprite_idx = bad_icon_id;
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  trapst->bigsym_sprite_idx = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              trapst->medsym_sprite_idx = bad_icon_id;
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  trapst->medsym_sprite_idx = k;
                  n++;
              }
          }
          if (n < 2)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 11: // POINTERSPRITES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  trapst->pointer_sprite_idx = k;
                  n++;
              }
          }
          if (n < 1)
          {
            trapst->pointer_sprite_idx = bad_icon_id;
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 12: // PANELTABINDEX
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                trapst->panel_tab_idx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 13: // TRIGGERTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                game.conf.trap_stats[i].trigger_type = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 14: // ACTIVATIONTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                game.conf.trap_stats[i].activation_type = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 15: // EFFECTTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                game.conf.trap_stats[i].created_itm_model = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 16: // ANIMATIONID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_anim_id_(word_buf);
            if (k >= 0)
            {
                game.conf.trap_stats[i].sprite_anim_idx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 17: // MODELSIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                game.conf.trap_stats[i].sprite_size_max = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 18: // ANIMATIONSPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                game.conf.trap_stats[i].anim_speed = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 19: // UNANIMATED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                game.conf.trap_stats[i].unanimated = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 20: // HIDDEN
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                trapst->hidden = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 21: // SLAPPABLE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                trapst->slappable = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 22: // TRIGGERALARM
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                trapst->notify = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 23: // HEALTH
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].health = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 24: // UNSHADED
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].unshaded = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 25: // RANDOMSTARTFRAME
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].random_start_frame = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 26: // THINGSIZE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].size_xy = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].size_z = k;
                  n++;
              }
          }
          if (n < 2)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 27: // HITTYPE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].hit_type = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 28: // LIGHTRADIUS
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].light_radius = k * COORD_PER_STL;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
        case 29: // LIGHTINTENSITY
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k >= 0)
                {
                    game.conf.trap_stats[i].light_intensity = k;
                    n++;
                }
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 30: // LIGHTFLAGS
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k >= 0)
                {
                    game.conf.trap_stats[i].light_flag = k;
                    n++;
                }
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
      case 31: // TRANSPARENCY_FLAGS
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].transparency_flag = k<<4;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 32: // SHOTVECTOR
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].shotvector.x = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].shotvector.y = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].shotvector.z = k;
                  n++;
              }
          }
          if (n < 3)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 33: // DESTRUCTIBLE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              trapst->destructible = k;
          }
          if (!parameter_is_number(word_buf))
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 34: // UNSTABLE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  trapst->unstable = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 35: // UNSELLABLE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  trapst->unsellable = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 36: // PLACEONBRIDGE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  trapst->place_on_bridge = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 37: // SHOTORIGIN
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].shot_shift_x = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].shot_shift_y = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].shot_shift_z = k;
                  n++;
              }
          }
          if (n < 3)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 38: // PLACESOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              n = atoi(word_buf);
              if (n < 0)
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
              }
              else
              {
                  trapst->place_sound_idx = n;
              }
          }
          break;
      case 39: // TRIGGERSOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              n = atoi(word_buf);
              if (n < 0)
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                      COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
              }
              else
              {
                  trapst->trigger_sound_idx = n;
              }
          }
          break;
      case 40: // RECHARGEANIMATIONID
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              struct ObjectConfigStats obj_tmp;
              k = get_anim_id(word_buf, &obj_tmp);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].recharge_sprite_anim_idx = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 41: // ATTACKANIMATIONID
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              struct ObjectConfigStats obj_tmp;
              k = get_anim_id(word_buf, &obj_tmp);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].attack_sprite_anim_idx = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 42: // DESTROYEDEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = effect_or_effect_element_id(word_buf);
              if (k != 0)
              {
                  trapst->destroyed_effect = k;
                  n++;
              }
              else if (parameter_is_number(word_buf))
              {
                  //No error when it is set to 0
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 43: // INITIALDELAY
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].initial_delay = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 44: // PLACEONSUBTILE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  trapst->place_on_subtile = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 45: // FLAMEANIMATIONID
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = get_anim_id_(word_buf);
              if (k >= 0)
              {
                  trapst->flame.animation_id = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 46: // FLAMEANIMATIONSPEED
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  trapst->flame.anim_speed = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 47: // FLAMEANIMATIONSIZE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  trapst->flame.sprite_size = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 48: // FLAMEANIMATIONOFFSET
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              trapst->flame.fp_add_x = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              trapst->flame.fp_add_y = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              trapst->flame.td_add_x = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              trapst->flame.td_add_y = k;
              n++;
          }
          if (n < 4)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 49: // FLAMETRANSPARENCYFLAGS
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  trapst->flame.transparency_flags = k << 4;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 50: // DETECTINVISIBLE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  game.conf.trap_stats[i].detect_invisible = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case ccr_comment:
          break;
      case ccr_endOfFile:
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%.*s] block of %s file.",
              cmd_num, blocknamelen, blockname, config_textname);
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
  struct DoorConfigStats *doorst;
  // Block name and parameter word store variables
  SYNCDBG(19,"Starting");
  // Initialize the doors array
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      for (int i=0; i < TRAPDOOR_TYPES_MAX; i++)
      {
          doorst = &game.conf.trapdoor_conf.door_cfgstats[i];
          LbMemorySet(doorst->code_name, 0, COMMAND_WORD_LEN);
          doorst->name_stridx = GUIStr_Empty;
          doorst->tooltip_stridx = GUIStr_Empty;
          doorst->bigsym_sprite_idx = 0;
          doorst->medsym_sprite_idx = 0;
          doorst->pointer_sprite_idx = 0;
          doorst->unsellable = 0;
          // Default door placement sound, so that placement sound isn't broken if custom doors is bundled into maps
          doorst->place_sound_idx = 117;
          doorst->panel_tab_idx = 0;
          door_desc[i].name = doorst->code_name;
          door_desc[i].num = i;
      }
  }
  door_desc[TRAPDOOR_TYPES_MAX - 1].name = NULL; // must be null for get_id
  // Parse every numbered block within range
  const char * blockname = NULL;
  int blocknamelen = 0, k = 0;
  long pos = 0;
  while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
  {
    // look for blocks starting with "door", followed by one or more digits
    if (blocknamelen < 5) {
        continue;
    } else if (memcmp(blockname, "door", 4) != 0) {
        continue;
    }
    const int i = natoi(&blockname[4], blocknamelen - 4);
    if (i < 0 || i >= TRAPDOOR_TYPES_MAX) {
        continue;
    } else if (i >= game.conf.trapdoor_conf.door_types_count) {
        game.conf.trapdoor_conf.door_types_count = i + 1;
    }
    struct ManfctrConfig* mconf = &game.conf.doors_config[i];
    doorst = &game.conf.trapdoor_conf.door_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_door_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, trapdoor_door_commands);
      // Now store the config item in correct place
      if (cmd_num == ccr_endOfBlock) break; // if next block starts
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
          if (get_conf_parameter_single(buf,&pos,len,doorst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 4: // SLABKIND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = get_id(slab_desc, word_buf);
              doorst->slbkind[1] = k;
              n++;
          }
          else
          {
              CONFWRNLOG("Incorrect slab name \"%s\" in [%.*s] block of %s file.",
                  word_buf, blocknamelen, blockname, config_textname);
              break;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = get_id(slab_desc, word_buf);
              doorst->slbkind[0] = k;
              n++;
          }
          else
          {
              CONFWRNLOG("Incorrect slab name \"%s\" in [%.*s] block of %s file.",
                  word_buf, blocknamelen, blockname, config_textname);
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
              break;
          }
          break;
      case 5: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (i < game.conf.trapdoor_conf.door_types_count)
            {
              doorst->health = k;
            }
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 7: // NAMETEXTID
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 8: // TOOLTIPTEXTID
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 9: // CRATE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              n = get_id(object_desc, word_buf);
          }
          if (n < 0)
          {
              CONFWRNLOG("Incorrect crate object \"%s\" in [%.*s] block of %s file.",
                  word_buf, blocknamelen, blockname, config_textname);
              break;
          }
          game.conf.object_conf.object_to_door_or_trap[n] = i;
          game.conf.object_conf.workshop_object_class[n] = TCls_Door;
          game.conf.trapdoor_conf.door_to_object[i] = n;
          break;
      case 10: // SYMBOLSPRITES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              doorst->bigsym_sprite_idx = bad_icon_id;
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  doorst->bigsym_sprite_idx = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              doorst->medsym_sprite_idx = bad_icon_id;
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  doorst->medsym_sprite_idx = k;
                  n++;
              }
          }
          if (n < 2)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 11: // POINTERSPRITES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  doorst->pointer_sprite_idx = k;
                  n++;
              }
          }
          if (n < 1)
          {
            doorst->pointer_sprite_idx = bad_icon_id;
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 12: // PANELTABINDEX
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                doorst->panel_tab_idx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 13: // OPENSPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                doorst->open_speed = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 14: // PROPERTIES
          doorst->model_flags = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_id(door_properties_commands, word_buf);
            switch (k)
            {
            case 1: // RESIST_NON_MAGIC
                doorst->model_flags |= DoMF_ResistNonMagic;
                n++;
                break;
            case 2: // SECRET
                doorst->model_flags |= DoMF_Secret;
                n++;
                break;
            case 3: // THICK
                doorst->model_flags |= DoMF_Thick;
                n++;
                break;
            case 4: // MIDAS
                doorst->model_flags |= DoMF_Midas;
                n++;
                break;
            default:
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), word_buf, blocknamelen, blockname, config_textname);
            }
          }
          break;
      case 15: // PLACESOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              n = atoi(word_buf);
              if (n < 0)
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                      COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
              }
              else
              {
                  doorst->place_sound_idx = n;
              }
          }
          break;
      case 16: // UNSELLABLE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                doorst->unsellable = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case ccr_comment:
          break;
      case ccr_endOfFile:
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%.*s] block of %s file.",
              cmd_num, blocknamelen, blockname, config_textname);
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
    
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        for (int i = 0; i < TRAPDOOR_TYPES_MAX; i++)
        {
            game.conf.object_conf.object_to_door_or_trap[i] = 0;
        }
    }

    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file
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

TbBool create_manufacture_array_from_trapdoor_data(void)
{
    int i;
    struct ManufactureData *manufctr;
    // Initialize the manufacture array
    game.conf.trapdoor_conf.manufacture_types_count = 0;
    int arr_size = sizeof(game.conf.trapdoor_conf.manufacture_data) / sizeof(game.conf.trapdoor_conf.manufacture_data[0]);
    for (i=0; i < arr_size; i++)
    {
        manufctr = &game.conf.trapdoor_conf.manufacture_data[i];
        manufctr->tngclass = TCls_Empty;
        manufctr->tngmodel = 0;
        manufctr->work_state = PSt_None;
        manufctr->tooltip_stridx = GUIStr_Empty;
        manufctr->bigsym_sprite_idx = 0;
        manufctr->medsym_sprite_idx = 0;
        manufctr->panel_tab_idx = 0;
    }
    // Let manufacture 0 be empty
    game.conf.trapdoor_conf.manufacture_types_count++;
    // Fill manufacture entries
    for (i=1; i < game.conf.trapdoor_conf.trap_types_count; i++)
    {
        struct TrapConfigStats* trapst = get_trap_model_stats(i);
        manufctr = &game.conf.trapdoor_conf.manufacture_data[game.conf.trapdoor_conf.manufacture_types_count];
        manufctr->tngclass = TCls_Trap;
        manufctr->tngmodel = i;
        manufctr->work_state = PSt_PlaceTrap;
        manufctr->tooltip_stridx = trapst->tooltip_stridx;
        manufctr->bigsym_sprite_idx = trapst->bigsym_sprite_idx;
        manufctr->medsym_sprite_idx = trapst->medsym_sprite_idx;
        manufctr->panel_tab_idx = trapst->panel_tab_idx;
        game.conf.trapdoor_conf.manufacture_types_count++;
    }
    for (i=1; i < game.conf.trapdoor_conf.door_types_count; i++)
    {
        struct DoorConfigStats* doorst = get_door_model_stats(i);
        manufctr = &game.conf.trapdoor_conf.manufacture_data[game.conf.trapdoor_conf.manufacture_types_count];
        manufctr->tngclass = TCls_Door;
        manufctr->tngmodel = i;
        manufctr->work_state = PSt_PlaceDoor;
        manufctr->tooltip_stridx = doorst->tooltip_stridx;
        manufctr->bigsym_sprite_idx = doorst->bigsym_sprite_idx;
        manufctr->medsym_sprite_idx = doorst->medsym_sprite_idx;
        manufctr->panel_tab_idx = doorst->panel_tab_idx;
        game.conf.trapdoor_conf.manufacture_types_count++;
    }
    return true;
}

TbBool load_trapdoor_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global traps and doors config";
    static const char config_campgn_textname[] = "campaign traps and doors config";
    static const char config_level_textname[] = "level traps and doors config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_trapdoor_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_trapdoor_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), conf_fname);
    if (strlen(fname) > 0)
    {
        load_trapdoor_config_file(config_level_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    // Creating arrays derived from the original config
    create_manufacture_array_from_trapdoor_data();
    //Freeing and exiting
    return result;
}

ThingModel door_crate_object_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= TRAPDOOR_TYPES_MAX))
        return game.conf.trapdoor_conf.door_to_object[0];
    return game.conf.trapdoor_conf.door_to_object[tngmodel];

}

ThingModel trap_crate_object_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= TRAPDOOR_TYPES_MAX))
        return game.conf.trapdoor_conf.trap_to_object[0];
    return game.conf.trapdoor_conf.trap_to_object[tngmodel];
}

/**
 * Returns Code Name (name to use in script file) of given door model.
 */
const char *door_code_name(int tngmodel)
{
    const char* name = get_conf_parameter_text(door_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given trap model.
 */
const char *trap_code_name(int tngmodel)
{
    const char* name = get_conf_parameter_text(trap_desc, tngmodel);
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
    for (int i = 0; i < game.conf.trapdoor_conf.door_types_count; ++i)
    {
        if (strncasecmp(game.conf.trapdoor_conf.door_cfgstats[i].code_name, code_name,
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
    for (int i = 0; i < game.conf.trapdoor_conf.trap_types_count; ++i)
    {
        if (strncasecmp(game.conf.trapdoor_conf.trap_cfgstats[i].code_name, code_name,
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
TbBool is_trap_placeable(PlayerNumber plyr_idx, long tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to place traps
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= game.conf.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if (dungeon->mnfct_info.trap_amount_placeable[tngmodel] > 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the trap can be manufactured by a player.
 * Checks only if it's set as buildable in level script.
 * Doesn't check if player has workshop or workforce for the task.
 */
TbBool is_trap_buildable(PlayerNumber plyr_idx, long tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= game.conf.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if ((dungeon->mnfct_info.trap_build_flags[tngmodel] & MnfBldF_Manufacturable) != 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the trap was at least once built by a player.
 */
TbBool is_trap_built(PlayerNumber plyr_idx, long tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= game.conf.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if ((dungeon->mnfct_info.trap_build_flags[tngmodel] & MnfBldF_Built) != 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the door can be placed by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if map position is on correct spot.
 */
TbBool is_door_placeable(PlayerNumber plyr_idx, long tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to place doors
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= game.conf.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if (dungeon->mnfct_info.door_amount_placeable[tngmodel] > 0) {
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
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((door_idx <= 0) || (door_idx >= game.conf.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)door_idx, (int)plyr_idx);
        return false;
    }
    if ((dungeon->mnfct_info.door_build_flags[door_idx] & MnfBldF_Manufacturable) != 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the door was at least one built by a player.
 */
TbBool is_door_built(PlayerNumber plyr_idx, long door_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((door_idx <= 0) || (door_idx >= game.conf.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)door_idx, (int)plyr_idx);
        return false;
    }
    if ((dungeon->mnfct_info.door_build_flags[door_idx] & MnfBldF_Built) != 0) {
        return true;
    }
    return false;
}

/**
 * Makes all door types manufacturable.
 */
TbBool make_available_all_doors(PlayerNumber plyr_idx)
{
  SYNCDBG(0,"Starting");
  struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon)) {
      ERRORDBG(11,"Cannot make doors available; player %d has no dungeon",(int)plyr_idx);
      return false;
  }
  for (long i = 1; i < game.conf.trapdoor_conf.door_types_count; i++)
  {
    if (!set_door_buildable_and_add_to_amount(plyr_idx, i, 1, 0))
    {
        ERRORLOG("Could not make door %s available for player %d", door_code_name(i), plyr_idx);
        return false;
    }
  }
  return true;
}

/**
 * Makes all trap types manufacturable.
 */
TbBool make_available_all_traps(PlayerNumber plyr_idx)
{
  SYNCDBG(0,"Starting");
  struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon)) {
      ERRORDBG(11,"Cannot make traps available; player %d has no dungeon",(int)plyr_idx);
      return false;
  }
  for (long i = 1; i < game.conf.trapdoor_conf.trap_types_count; i++)
  {
    if (!set_trap_buildable_and_add_to_amount(plyr_idx, i, 1, 0))
    {
        ERRORLOG("Could not make trap %s available for player %d", trap_code_name(i), plyr_idx);
        return false;
    }
  }
  return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
