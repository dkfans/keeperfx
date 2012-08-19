/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_objects.c
 *     Object things configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for object things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Jun 2012 - 16 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_objects.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_objects.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_objects_file[]="objects.cfg";

const struct NamedCommand objects_common_commands[] = {
  {"OBJECTSCOUNT",    1},
  {NULL,              0},
  };

const struct NamedCommand objects_object_commands[] = {
  {"NAME",            1},
  {"GERNE",           2},
  {NULL,              0},
  };

const struct NamedCommand objects_gernes_desc[] = {
  {"NONE",            0},
  {"DECORATION",      1},
  {"FURNITURE",       2},
  {"VALUABLE",        3},
  {"SPELLBOOK",       4},
  {"WORKSHOPBOX",     5},
  {"FOOD",            6},
  {"POWER",           7},
  {"LAIR",            8},
  {"EFFECT",          9},
  {NULL,              0},
  };

/******************************************************************************/
struct ObjectsConfig object_conf;
struct NamedCommand object_desc[OBJECT_ITEMS_MAX];
/******************************************************************************/
struct ObjectConfigStats *get_object_model_stats(ThingModel tngmodel)
{
    if ((tngmodel < 0) || (tngmodel >= object_conf.object_types_count))
        return &object_conf.object_cfgstats[0];
    return &object_conf.object_cfgstats[tngmodel];
}

TbBool parse_objects_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        object_conf.object_types_count = 1;
    }
    // Find the block
    sprintf(block_buf,"common");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(objects_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,objects_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // OBJECTSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= OBJECT_ITEMS_MAX))
              {
                  object_conf.object_types_count = k;
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

TbBool parse_objects_object_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct ObjectConfigStats *objst;
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize the objects array
    int arr_size;
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(object_conf.object_cfgstats)/sizeof(object_conf.object_cfgstats[0]);
        for (i=0; i < arr_size; i++)
        {
            objst = &object_conf.object_cfgstats[i];
            LbMemorySet(objst->code_name, 0, COMMAND_WORD_LEN);
            objst->name_stridx = 201;
            objst->gerne = 0;
            if (i < object_conf.object_types_count)
            {
                object_desc[i].name = objst->code_name;
                object_desc[i].num = i;
            } else
            {
                object_desc[i].name = NULL;
                object_desc[i].num = 0;
            }
        }
    }
    // Load the file
    arr_size = object_conf.object_types_count;
    for (i=0; i < arr_size; i++)
    {
        sprintf(block_buf,"object%d",i);
        pos = 0;
        k = find_conf_block(buf,&pos,len,block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0) {
                WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
                return false;
            }
            continue;
        }
        objst = &object_conf.object_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(objects_object_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            cmd_num = recognize_conf_command(buf,&pos,len,objects_object_commands);
            // Now store the config item in correct place
            if (cmd_num == -3) break; // if next block starts
            if ((flags & CnfLd_ListOnly) != 0) {
                // In "List only" mode, accept only name command
                if (cmd_num > 1) {
                    cmd_num = 0;
                }
            }
            n = 0;
            switch (cmd_num)
            {
            case 1: // NAME
                if (get_conf_parameter_single(buf,&pos,len,objst->code_name,COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                break;
            case 2: // GERNE
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    n = get_id(objects_gernes_desc, word_buf);
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect object gerne \"%s\" in [%s] block of %s file.",
                        word_buf,block_buf,config_textname);
                    break;
                }
                objst->gerne = n;
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

TbBool load_objects_config_file(const char *textname, const char *fname, unsigned short flags)
{
    char *buf;
    long len;
    TbBool result;
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    len = LbFileLengthRnc(fname);
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
    buf = (char *)LbMemoryAlloc(len+256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    result = (len > 0);
    // Parse blocks of the config file
    if (result)
    {
        result = parse_objects_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_objects_object_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" object blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_objects_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global objects config";
    static const char config_campgn_textname[] = "campaign objects config";
    char *fname;
    TbBool result;
    fname = prepare_file_path(FGrp_FxData,conf_fname);
    result = load_objects_config_file(config_global_textname,fname,flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_objects_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given object model.
 */
const char *object_code_name(ThingModel tngmodel)
{
    const char *name;
    name = get_conf_parameter_text(object_desc,tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the object model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the object model if found, otherwise -1
 */
ThingModel object_model_id(const char * code_name)
{
    int i;

    for (i = 0; i < object_conf.object_types_count; ++i) {
        if (strncasecmp(object_conf.object_cfgstats[i].code_name, code_name,
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
