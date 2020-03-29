/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_effects.c
 *     Effects configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for effects and effect elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 11 Mar 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_effects.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_strings.h"
#include "thing_effects.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_effects_file[]="effects.cfg";

const struct NamedCommand effects_common_commands[] = {
  {"EFFECTSCOUNT",    1},
  {NULL,              0},
  };

const struct NamedCommand effects_effect_commands[] = {
  {"NAME",                    1},
  {"HEALTH",                  2},
  {"GENERATIONTYPE",          3},
  {"GENERATIONACCELXYRANGE",  4},
  {"GENERATIONACCELZRANGE",   5},
  {"GENERATIONKINDRANGE",     6},
  {"AREAAFFECTTYPE",          7},
  {"SOUND",                   8},
  {"AFFECTEDBYWIND",          9},
  {NULL,                      0},
  };

long const imp_spangle_effects[] = {
    TngEff_ImpSpangleRed, TngEff_ImpSpangleBlue, TngEff_ImpSpangleGreen, TngEff_ImpSpangleYellow, TngEff_None, TngEff_None,
};

/******************************************************************************/
struct EffectsConfig effects_conf;
struct NamedCommand effect_desc[EFFECTS_TYPES_MAX];
extern struct InitEffect effect_info[];
/******************************************************************************/
struct EffectConfigStats *get_effect_model_stats(int tngmodel)
{
    if (tngmodel >= effects_conf.effect_types_count)
        return &effects_conf.effect_cfgstats[0];
    return &effects_conf.effect_cfgstats[tngmodel];
}

short write_effects_effect_to_log(const struct EffectConfigStats *effcst, int num)
{
  JUSTMSG("[effect%d]",(int)num);
  JUSTMSG("Name = %s",effcst->code_name);
  JUSTMSG("Health = %d",(int)effcst->old->start_health);
  JUSTMSG("GenerationType = %d",(int)effcst->old->generation_type);
  JUSTMSG("GenerationAccelXYRange = %d %d",(int)effcst->old->accel_xy_min,(int)effcst->old->accel_xy_max);
  JUSTMSG("GenerationAccelZRange = %d %d",(int)effcst->old->accel_z_min,(int)effcst->old->accel_z_max);
  JUSTMSG("GenerationKindRange = %d %d",(int)effcst->old->kind_min,(int)effcst->old->kind_max);
  JUSTMSG("AreaAffectType = %d",(int)effcst->old->area_affect_type);
  JUSTMSG("");
  return true;
}

TbBool parse_effects_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    SYNCDBG(19,"Starting");
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        effects_conf.effect_types_count = 1;
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
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(effects_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, effects_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        switch (cmd_num)
        {
        case 1: // EFFECTSCOUNT
        {
            char word_buf[COMMAND_WORD_LEN];
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= EFFECTS_TYPES_MAX))
              {
                effects_conf.effect_types_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        }
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

TbBool parse_effects_effect_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct EffectConfigStats *effcst;
  int i;
  // Block name and parameter word store variables
  SYNCDBG(19,"Starting");
  // Initialize the effects array
  int arr_size;
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      arr_size = sizeof(effects_conf.effect_cfgstats)/sizeof(effects_conf.effect_cfgstats[0]);
      for (i=0; i < arr_size; i++)
      {
          effcst = &effects_conf.effect_cfgstats[i];
          LbMemorySet(effcst->code_name, 0, COMMAND_WORD_LEN);
          if (i < 69)
              effcst->old = &effect_info[i];
          else
              effcst->old = &effect_info[0];
          if (i < effects_conf.effect_types_count)
          {
              effect_desc[i].name = effcst->code_name;
              effect_desc[i].num = i;
          } else
          {
              effect_desc[i].name = NULL;
              effect_desc[i].num = 0;
          }
      }
  }
  // Parse every numbered block within range
  arr_size = effects_conf.effect_types_count;
  for (i=0; i < arr_size; i++)
  {
      char block_buf[COMMAND_WORD_LEN];
      sprintf(block_buf, "effect%d", i);
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
    effcst = &effects_conf.effect_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(effects_effect_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, effects_effect_commands);
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
          if (get_conf_parameter_single(buf,&pos,len,effcst->code_name,COMMAND_WORD_LEN) <= 0)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
              break;
          }
          break;
      case 2: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->start_health = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // GENERATIONTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->generation_type = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // GENERATIONACCELXYRANGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->accel_xy_min = k;
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->accel_xy_max = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read all values of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // GENERATIONACCELZRANGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->accel_z_min = k;
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->accel_z_max = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read all values of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 6: // GENERATIONKINDRANGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->kind_min = k;
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->kind_max = k;
              n++;
          }
          if (n < 2)
          {
              CONFWRNLOG("Couldn't read all values of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
          break;
      case 7: // AREAAFFECTTYPE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->area_affect_type = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // SOUND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->effect_sound = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 9: // AFFECTEDBYWIND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              effcst->old->affected_by_wind = k;
              n++;
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
      //write_effects_effect_to_log(effcst, i);
    }
#undef COMMAND_TEXT
  }
  return true;
}


TbBool load_effects_config_file(const char *textname, const char *fname, unsigned short flags)
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
        result = parse_effects_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_effects_effect_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" effect blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    SYNCDBG(19,"Done");
    return result;
}

TbBool load_effects_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global effects config";
    static const char config_campgn_textname[] = "campaign effects config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_effects_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_effects_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given effect model.
 */
const char *effect_code_name(int tngmodel)
{
    const char* name = get_conf_parameter_text(effect_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the effect model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the effect model if found, otherwise -1
 */
int effect_model_id(const char * code_name)
{
    for (int i = 0; i < effects_conf.effect_types_count; ++i)
    {
        if (strncasecmp(effects_conf.effect_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
