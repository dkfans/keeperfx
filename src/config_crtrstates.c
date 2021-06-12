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
#include "game_merge.h"

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
TbBool parse_creaturestates_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        //TODO CONFIG We will need to make some cleaning here
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
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatstate_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creatstate_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        switch (cmd_num)
        {
        case 1: // STATESCOUNT
        {
            char word_buf[COMMAND_WORD_LEN];
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= CREATURE_STATES_MAX))
              {
                gameadd.crtr_conf.states_count = k;
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
    if (gameadd.crtr_conf.states_count < 1)
    {
        WARNLOG("No creature states defined in [%s] block of %s file.",
            block_buf,config_textname);
    }
    return true;
}

TbBool parse_creaturestates_state_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    int i;
    // Block name and parameter word store variables
    // Initialize the array
    int arr_size;
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(gameadd.crtr_conf.states)/sizeof(gameadd.crtr_conf.states[0]);
        for (i=0; i < arr_size; i++)
        {
            LbMemorySet(gameadd.crtr_conf.states[i].name, 0, COMMAND_WORD_LEN);
            if (i < gameadd.crtr_conf.states_count)
            {
                creatrstate_desc[i].name = gameadd.crtr_conf.states[i].name;
                creatrstate_desc[i].num = i;
            } else
            {
                creatrstate_desc[i].name = NULL;
                creatrstate_desc[i].num = 0;
            }
        }
    }
    // Load the file blocks
    arr_size = gameadd.crtr_conf.states_count;
    for (i=0; i < arr_size; i++)
    {
        char block_buf[COMMAND_WORD_LEN];
        sprintf(block_buf, "state%d", i);
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
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creatstate_state_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creatstate_state_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        if ((flags & CnfLd_ListOnly) != 0) {
            // In "List only" mode, accept only name command
            if (cmd_num > 1) {
                cmd_num = 0;
            }
        }
        int n = 0;
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_single(buf,&pos,len,gameadd.crtr_conf.states[i].name,COMMAND_WORD_LEN) <= 0)
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

TbBool load_creaturestates_config_file(const char *textname, const char *fname, unsigned short flags)
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
        result = parse_creaturestates_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturestates_state_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
          WARNMSG("Parsing %s file \"%s\" state blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_creaturestates_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global creature states config";
    static const char config_campgn_textname[] = "campaign creature states config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_creaturestates_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_creaturestates_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

const char *creature_state_code_name(long crstate)
{
    const char* name = get_conf_parameter_text(creatrstate_desc, crstate);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
