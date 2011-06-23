/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_crtrstates.c
 *     Specific creature model configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for specific creatures.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     25 May 2009 - 23 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_crtrstates.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_data.h"
#include "config_creature.h"
#include "creature_states.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const struct NamedCommand creatstate_common_commands[] = {
  {"STATESCOUNT",      1},
  {NULL,               0},
  };

const struct NamedCommand creatstate_state_commands[] = {
  {"NAME",             1},
  {NULL,               0},
  };

const char creature_states_file[]="crstates.cfg";
/******************************************************************************/
struct NamedCommand creatrstate_desc[CREATURE_STATES_MAX];
/******************************************************************************/
TbBool parse_creaturestates_common_blocks(char *buf,long len,const char *config_textname)
{
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data

    // Find the block
    sprintf(block_buf,"common");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatstate_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,creatstate_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // STATESCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= CREATURE_STATES_MAX))
              {
                crtr_conf.states_count = k;
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
    if (crtr_conf.states_count < 1)
    {
        WARNLOG("No creature states defined in [%s] block of %s file.",
            block_buf,config_textname);
    }
    return true;
}

TbBool parse_creaturestates_state_blocks(char *buf,long len,const char *config_textname)
{
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    int arr_size;
    // Initialize the array
    arr_size = sizeof(crtr_conf.state_names)/sizeof(crtr_conf.state_names[0]);
    for (i=0; i < arr_size; i++)
    {
        LbMemorySet(crtr_conf.state_names[i].text, 0, COMMAND_WORD_LEN);
        if (i < crtr_conf.states_count)
        {
            creatrstate_desc[i].name = crtr_conf.state_names[i].text;
            creatrstate_desc[i].num = i;
        } else
        {
            creatrstate_desc[i].name = NULL;
            creatrstate_desc[i].num = 0;
        }
    }
    arr_size = crtr_conf.states_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
      sprintf(block_buf,"state%d",i);
      pos = 0;
      k = find_conf_block(buf,&pos,len,block_buf);
      if (k < 0)
      {
        WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        continue;
      }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatstate_state_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,creatstate_state_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_single(buf,&pos,len,crtr_conf.state_names[i].text,COMMAND_WORD_LEN) <= 0)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
                break;
            }
            n++;
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

TbBool load_creaturestates_config(const char *conf_fname,unsigned short flags)
{
    static const char config_textname[] = "Creature States config";
    char *fname;
    char *buf;
    long len;
    TbBool result;
    SYNCDBG(0,"Reading %s file \"%s\".",config_textname,conf_fname);
    fname = prepare_file_path(FGrp_FxData,conf_fname);
    len = LbFileLengthRnc(fname);
    if (len < 2)
    {
      WARNMSG("Game %s file \"%s\" doesn't exist or is too small.",config_textname,conf_fname);
      return false;
    }
    if (len > 65536)
    {
      WARNMSG("Game %s file \"%s\" is too large.",config_textname,conf_fname);
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
      result = parse_creaturestates_common_blocks(buf, len, config_textname);
      if (!result)
        WARNMSG("Parsing %s file \"%s\" common blocks failed.",config_textname,conf_fname);
    }
    if (result)
    {
        result = parse_creaturestates_state_blocks(buf, len, config_textname);
        if (!result)
          WARNMSG("Parsing %s file \"%s\" state blocks failed.",config_textname,conf_fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

const char *creature_state_code_name(long crstate)
{
    const char *name;
    name = get_conf_parameter_text(creatrstate_desc,crstate);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
