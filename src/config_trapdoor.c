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
#include "player_instances.h"
#include "player_states.h"
#include "game_legacy.h"
#include "custom_sprites.h"

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
  {"NAME",                  1},
  {"MANUFACTURELEVEL",      2},
  {"MANUFACTUREREQUIRED",   3},
  {"UNUSEDUNUSED",          4},// replace by any command later; this is just to keep same indexing below
  {"HEALTH",                5},
  {"SELLINGVALUE",          6},
  {"NAMETEXTID",            7},
  {"TOOLTIPTEXTID",         8},
  {"CRATE",                 9},
  {"SYMBOLSPRITES",        10},
  {"POINTERSPRITES",       11},
  {"PANELTABINDEX",        12},
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
  {"MODEL",                16},
  {"MODELSIZE",            17},
  {"ANIMATIONSPEED",       18},
  {"UNANIMATED",           19},
  {"HIDDEN",               20},
  {"SLAPPABLE",            21},
  {"TRIGGERALARM",         22},
  {NULL,                    0},
};
/******************************************************************************/
struct NamedCommand trap_desc[TRAPDOOR_TYPES_MAX];
struct NamedCommand door_desc[TRAPDOOR_TYPES_MAX];
/******************************************************************************/
struct TrapConfigStats *get_trap_model_stats(int tngmodel)
{
    if (tngmodel >= gameadd.trapdoor_conf.trap_types_count)
        return &gameadd.trapdoor_conf.trap_cfgstats[0];
    return &gameadd.trapdoor_conf.trap_cfgstats[tngmodel];
}

struct DoorConfigStats *get_door_model_stats(int tngmodel)
{
    if (tngmodel >= gameadd.trapdoor_conf.door_types_count)
        return &gameadd.trapdoor_conf.door_cfgstats[0];
    return &gameadd.trapdoor_conf.door_cfgstats[tngmodel];
}

/**
 * Returns manufacture data for a given manufacture index.
 * @param manufctr_idx Manufacture array index.
 * @return Dummy entry pinter if not found, manufacture data pointer otherwise.
 */
struct ManufactureData *get_manufacture_data(int manufctr_idx)
{
    if ((manufctr_idx < 0) || (manufctr_idx >= gameadd.trapdoor_conf.manufacture_types_count)) {
        return &gameadd.trapdoor_conf.manufacture_data[0];
    }
    return &gameadd.trapdoor_conf.manufacture_data[manufctr_idx];
}

/**
 * Finds index into manufactures data array for a given trap/door class and model.
 * @param tngclass Manufacturable thing class.
 * @param tngmodel Manufacturable thing model.
 * @return 0 if not found, otherwise index where 1 <= index < manufacture_types_count
 */
int get_manufacture_data_index_for_thing(ThingClass tngclass, ThingModel tngmodel)
{
    for (int i = 1; i < gameadd.trapdoor_conf.manufacture_types_count; i++)
    {
        struct ManufactureData* manufctr = &gameadd.trapdoor_conf.manufacture_data[i];
        if ((manufctr->tngclass == tngclass) && (manufctr->tngmodel == tngmodel)) {
            return i;
        }
    }
    return 0;
}

TbBool parse_trapdoor_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    SYNCDBG(19,"Starting");
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        gameadd.trapdoor_conf.trap_types_count = 1;
        gameadd.trapdoor_conf.door_types_count = 1;
        int arr_size = sizeof(gameadd.object_conf.object_to_door_or_trap) / sizeof(gameadd.object_conf.object_to_door_or_trap[0]);
        for (int i = 0; i < arr_size; i++)
        {
            gameadd.object_conf.object_to_door_or_trap[i] = 0;
        }
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
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, trapdoor_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // TRAPSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= TRAPDOOR_TYPES_MAX))
              {
                gameadd.trapdoor_conf.trap_types_count = k;
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
              if ((k > 0) && (k <= TRAPDOOR_TYPES_MAX))
              {
                gameadd.trapdoor_conf.door_types_count = k;
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
  int i;
  // Block name and parameter word store variables
  SYNCDBG(19,"Starting");
  // Initialize the traps array
  int arr_size;
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      arr_size = sizeof(gameadd.trapdoor_conf.trap_cfgstats)/sizeof(gameadd.trapdoor_conf.trap_cfgstats[0]);
      for (i=0; i < arr_size; i++)
      {
          trapst = &gameadd.trapdoor_conf.trap_cfgstats[i];
          LbMemorySet(trapst->code_name, 0, COMMAND_WORD_LEN);
          trapst->name_stridx = GUIStr_Empty;
          trapst->tooltip_stridx = GUIStr_Empty;
          trapst->bigsym_sprite_idx = 0;
          trapst->medsym_sprite_idx = 0;
          trapst->pointer_sprite_idx = 0;
          trapst->panel_tab_idx = 0;
          trapst->hidden = 0;
          trapst->slappable = 0;
          trapst->notify = 0;
          if (i < gameadd.trapdoor_conf.trap_types_count)
          {
              trap_desc[i].name = trapst->code_name;
              trap_desc[i].num = i;
          } else
          {
              trap_desc[i].name = NULL;
              trap_desc[i].num = 0;
          }
      }
      arr_size = gameadd.trapdoor_conf.trap_types_count;
      for (i=0; i < arr_size; i++)
      {
          mconf = &gameadd.traps_config[i];
          mconf->manufct_level = 0;
          mconf->manufct_required = 0;
          mconf->shots = 0;
          mconf->shots_delay = 0;
          mconf->selling_value = 0;
      }
  }
  // Parse every numbered block within range
  arr_size = gameadd.trapdoor_conf.trap_types_count;
  for (i=0; i < arr_size; i++)
  {
      char block_buf[COMMAND_WORD_LEN];
      sprintf(block_buf, "trap%d", i);
      SYNCDBG(19, "Block [%s]", block_buf);
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
    mconf = &gameadd.traps_config[i];
    trapst = &gameadd.trapdoor_conf.trap_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_trap_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, trapdoor_trap_commands);
      SYNCDBG(19,"Command %s",COMMAND_TEXT(cmd_num));
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
          if (get_conf_parameter_single(buf,&pos,len,trapst->code_name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
            break;
          }
          trap_desc[i].name = trapst->code_name;
          trap_desc[i].num = i;
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
      case 9: // CRATE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              n = get_id(object_desc, word_buf);
          }
          if (n < 0)
          {
              CONFWRNLOG("Incorrect crate object \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
          }
          gameadd.object_conf.object_to_door_or_trap[n] = i;
          gameadd.object_conf.workshop_object_class[n] = TCls_Trap;
          gameadd.trapdoor_conf.trap_to_object[i] = n;
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
                  trapst->pointer_sprite_idx = k;
                  n++;
              }
          }
          if (n < 1)
          {
            trapst->pointer_sprite_idx = bad_icon_id;
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
                trapst->panel_tab_idx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 13: // TRIGGERTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                gameadd.trap_stats[i].trigger_type = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 14: // ACTIVATIONTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                gameadd.trap_stats[i].activation_type = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 15: // EFFECTTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                gameadd.trap_stats[i].created_itm_model = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 16: // MODEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            struct Objects obj_tmp;
            k = get_anim_id(word_buf, &obj_tmp);
            if (k >= 0)
            {
                gameadd.trap_stats[i].sprite_anim_idx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 17: // MODELSIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                gameadd.trap_stats[i].sprite_size_max = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 18: // ANIMATIONSPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                gameadd.trap_stats[i].anim_speed = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 19: // UNANIMATED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k >= 0)
            {
                gameadd.trap_stats[i].unanimated = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
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
  struct DoorConfigStats *doorst;
  int i;
  // Block name and parameter word store variables
  SYNCDBG(19,"Starting");
  // Initialize the doors array
  int arr_size;
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      arr_size = sizeof(gameadd.trapdoor_conf.door_cfgstats)/sizeof(gameadd.trapdoor_conf.door_cfgstats[0]);
      for (i=0; i < arr_size; i++)
      {
          doorst = &gameadd.trapdoor_conf.door_cfgstats[i];
          LbMemorySet(doorst->code_name, 0, COMMAND_WORD_LEN);
          doorst->name_stridx = GUIStr_Empty;
          doorst->tooltip_stridx = GUIStr_Empty;
          doorst->bigsym_sprite_idx = 0;
          doorst->medsym_sprite_idx = 0;
          doorst->pointer_sprite_idx = 0;
          doorst->panel_tab_idx = 0;
          if (i < gameadd.trapdoor_conf.door_types_count)
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
  arr_size = gameadd.trapdoor_conf.door_types_count;
  for (i=0; i < arr_size; i++)
  {
      char block_buf[COMMAND_WORD_LEN];
      sprintf(block_buf, "door%d", i);
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
    struct ManfctrConfig* mconf = &gameadd.doors_config[i];
    doorst = &gameadd.trapdoor_conf.door_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(trapdoor_door_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, trapdoor_door_commands);
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
      case 5: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (i < DOOR_TYPES_COUNT)
            {
              door_stats[i][0].health = k;
              door_stats[i][1].health = k;
            }
            //TODO: set stats
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
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 9: // CRATE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              n = get_id(object_desc, word_buf);
          }
          if (n < 0)
          {
              CONFWRNLOG("Incorrect crate object \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
          }
          gameadd.object_conf.object_to_door_or_trap[n] = i;
          gameadd.object_conf.workshop_object_class[n] = TCls_Door;
          gameadd.trapdoor_conf.door_to_object[i] = n;
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
                  doorst->pointer_sprite_idx = k;
                  n++;
              }
          }
          if (n < 1)
          {
            doorst->pointer_sprite_idx = bad_icon_id;
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
                doorst->panel_tab_idx = k;
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
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
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

TbBool create_manufacture_array_from_trapdoor_data(void)
{
    int i;
    struct ManufactureData *manufctr;
    // Initialize the manufacture array
    gameadd.trapdoor_conf.manufacture_types_count = 0;
    int arr_size = sizeof(gameadd.trapdoor_conf.manufacture_data) / sizeof(gameadd.trapdoor_conf.manufacture_data[0]);
    for (i=0; i < arr_size; i++)
    {
        manufctr = &gameadd.trapdoor_conf.manufacture_data[i];
        manufctr->tngclass = TCls_Empty;
        manufctr->tngmodel = 0;
        manufctr->work_state = PSt_None;
        manufctr->tooltip_stridx = GUIStr_Empty;
        manufctr->bigsym_sprite_idx = 0;
        manufctr->medsym_sprite_idx = 0;
        manufctr->panel_tab_idx = 0;
    }
    // Let manufacture 0 be empty
    gameadd.trapdoor_conf.manufacture_types_count++;
    // Fill manufacture entries
    for (i=1; i < gameadd.trapdoor_conf.trap_types_count; i++)
    {
        struct TrapConfigStats* trapst = get_trap_model_stats(i);
        manufctr = &gameadd.trapdoor_conf.manufacture_data[gameadd.trapdoor_conf.manufacture_types_count];
        manufctr->tngclass = TCls_Trap;
        manufctr->tngmodel = i;
        manufctr->work_state = PSt_PlaceTrap;
        manufctr->tooltip_stridx = trapst->tooltip_stridx;
        manufctr->bigsym_sprite_idx = trapst->bigsym_sprite_idx;
        manufctr->medsym_sprite_idx = trapst->medsym_sprite_idx;
        manufctr->panel_tab_idx = trapst->panel_tab_idx;
        gameadd.trapdoor_conf.manufacture_types_count++;
    }
    for (i=1; i < gameadd.trapdoor_conf.door_types_count; i++)
    {
        struct DoorConfigStats* doorst = get_door_model_stats(i);
        manufctr = &gameadd.trapdoor_conf.manufacture_data[gameadd.trapdoor_conf.manufacture_types_count];
        manufctr->tngclass = TCls_Door;
        manufctr->tngmodel = i;
        manufctr->work_state = PSt_PlaceDoor;
        manufctr->tooltip_stridx = doorst->tooltip_stridx;
        manufctr->bigsym_sprite_idx = doorst->bigsym_sprite_idx;
        manufctr->medsym_sprite_idx = doorst->medsym_sprite_idx;
        manufctr->panel_tab_idx = doorst->panel_tab_idx;
        gameadd.trapdoor_conf.manufacture_types_count++;
    }
    return true;
}

TbBool load_trapdoor_config(const char *conf_fname, unsigned short flags)
{
    memcpy(gameadd.trap_stats, old_trap_stats, sizeof(old_trap_stats));
    static const char config_global_textname[] = "global traps and doors config";
    static const char config_campgn_textname[] = "campaign traps and doors config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_trapdoor_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_trapdoor_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    // Creating arrays derived from the original config
    create_manufacture_array_from_trapdoor_data();
    //Freeing and exiting
    return result;
}

ThingModel door_crate_object_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= TRAPDOOR_TYPES_MAX))
        return gameadd.trapdoor_conf.door_to_object[0];
    return gameadd.trapdoor_conf.door_to_object[tngmodel];

}

ThingModel trap_crate_object_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= TRAPDOOR_TYPES_MAX))
        return gameadd.trapdoor_conf.trap_to_object[0];
    return gameadd.trapdoor_conf.trap_to_object[tngmodel];
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
    for (int i = 0; i < gameadd.trapdoor_conf.door_types_count; ++i)
    {
        if (strncasecmp(gameadd.trapdoor_conf.door_cfgstats[i].code_name, code_name,
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
    for (int i = 0; i < gameadd.trapdoor_conf.trap_types_count; ++i)
    {
        if (strncasecmp(gameadd.trapdoor_conf.trap_cfgstats[i].code_name, code_name,
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(dungeon->owner);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to place traps
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= gameadd.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if (dungeonadd->mnfct_info.trap_amount_placeable[tngmodel] > 0) {
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(dungeon->owner);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= gameadd.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if ((dungeonadd->mnfct_info.trap_build_flags[tngmodel] & MnfBldF_Manufacturable) != 0) {
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(dungeon->owner);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= gameadd.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if ((dungeonadd->mnfct_info.trap_build_flags[tngmodel] & MnfBldF_Built) != 0) {
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(dungeon->owner);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to place doors
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= gameadd.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if (dungeonadd->mnfct_info.door_amount_placeable[tngmodel] > 0) {
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(dungeon->owner);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((door_idx <= 0) || (door_idx >= gameadd.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)door_idx, (int)plyr_idx);
        return false;
    }
    if ((dungeonadd->mnfct_info.door_build_flags[door_idx] & MnfBldF_Manufacturable) != 0) {
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(dungeon->owner);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((door_idx <= 0) || (door_idx >= gameadd.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)door_idx, (int)plyr_idx);
        return false;
    }
    if ((dungeonadd->mnfct_info.door_build_flags[door_idx] & MnfBldF_Built) != 0) {
        return true;
    }
    return false;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
