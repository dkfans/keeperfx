/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_cubes.c
 *     Terrain cubes configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for terrain cubes, which make up columns.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Jun 2012 - 24 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_cubes.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_cubes_file[]="cubes.cfg";

const struct NamedCommand cubes_common_commands[] = {
  {"CUBESCOUNT",      1},
  {NULL,              0},
  };

const struct NamedCommand cubes_cube_commands[] = {
  {"NAME",            1},
  {"TEXTURES",        2},
  {"FLAGS",           3},
  {NULL,              0},
  };
/******************************************************************************/
struct CubesConfig cube_conf;
struct NamedCommand cube_desc[CUBE_ITEMS_MAX];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct CubeConfigStats *get_cube_model_stats(long cumodel)
{
    if ((cumodel < 0) || (cumodel >= cube_conf.cube_types_count))
        return &cube_conf.cube_cfgstats[0];
    return &cube_conf.cube_cfgstats[cumodel];
}

TbBool parse_cubes_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        cube_conf.cube_types_count = 1;
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
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(cubes_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, cubes_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        switch (cmd_num)
        {
        case 1: // CUBESSCOUNT
        {
            char word_buf[COMMAND_WORD_LEN];
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= CUBE_ITEMS_MAX))
              {
                  cube_conf.cube_types_count = k;
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

TbBool parse_cubes_cube_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct CubeConfigStats *objst;
    int i;
    // Block name and parameter word store variables
    // Initialize the cubes array
    int arr_size;
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(cube_conf.cube_cfgstats)/sizeof(cube_conf.cube_cfgstats[0]);
        for (i=0; i < arr_size; i++)
        {
            objst = &cube_conf.cube_cfgstats[i];
            LbMemorySet(objst->code_name, 0, COMMAND_WORD_LEN);
            if (i < cube_conf.cube_types_count)
            {
                cube_desc[i].name = objst->code_name;
                cube_desc[i].num = i;
            } else
            {
                cube_desc[i].name = NULL;
                cube_desc[i].num = 0;
            }
        }
    }
    // Load the file
    arr_size = cube_conf.cube_types_count;
    for (i=0; i < arr_size; i++)
    {
        char block_buf[COMMAND_WORD_LEN];
        sprintf(block_buf, "cube%d", i);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0) {
                WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
                return false;
            }
            continue;
        }
        objst = &cube_conf.cube_cfgstats[i];
        struct CubeAttribs* cubed = &game.cubes_data[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(cubes_cube_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            int cmd_num = recognize_conf_command(buf, &pos, len, cubes_cube_commands);
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
                if (get_conf_parameter_single(buf,&pos,len,objst->code_name,COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                break;
            case 2: // TEXTURES
                while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = atoi(word_buf);
                    if (n >= CUBE_TEXTURES)
                    {
                      CONFWRNLOG("Too many \"%s\" parameters in [%s] block of %s file.",
                          COMMAND_TEXT(cmd_num),block_buf,config_textname);
                      break;
                    }
                    cubed->texture_id[n] = k;
                    n++;
                }
                if (n < CUBE_TEXTURES)
                {
                    CONFWRNLOG("Couldn't read all \"%s\" parameters in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                }
                break;
            case 3: // FLAGS
                while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = atoi(word_buf);
                    if (n >= CUBE_TEXTURES)
                    {
                      CONFWRNLOG("Too many \"%s\" parameters in [%s] block of %s file.",
                          COMMAND_TEXT(cmd_num),block_buf,config_textname);
                      break;
                    }
                    cubed->field_C[n] = k;
                    n++;
                }
                if (n < CUBE_TEXTURES)
                {
                    CONFWRNLOG("Couldn't read all \"%s\" parameters in [%s] block of %s file.",
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

TbBool load_cubes_config_file(const char *textname, const char *fname, unsigned short flags)
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
        result = parse_cubes_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_cubes_cube_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" cube blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_cubes_config(unsigned short flags)
{
    static const char config_global_textname[] = "global cubes config";
    static const char config_campgn_textname[] = "campaign cubes config";
    char* fname = prepare_file_path(FGrp_FxData, keeper_cubes_file);
    TbBool result = load_cubes_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,keeper_cubes_file);
    if (strlen(fname) > 0)
    {
        load_cubes_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given cube model.
 */
const char *cube_code_name(long model)
{
    const char* name = get_conf_parameter_text(cube_desc, model);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the cube model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the cube model if found, otherwise -1
 */
ThingModel cube_model_id(const char * code_name)
{
    for (int i = 0; i < cube_conf.cube_types_count; ++i)
    {
        if (strncasecmp(cube_conf.cube_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

void clear_cubes(void)
{
    for (int i = 0; i < CUBE_ITEMS_MAX; i++)
    {
        struct CubeAttribs* cubed = &game.cubes_data[i];
        int n;
        for (n = 0; n < CUBE_TEXTURES; n++)
        {
            cubed->texture_id[n] = 0;
        }
        for (n = 0; n < CUBE_TEXTURES; n++)
        {
            cubed->field_C[n] = 0;
        }
  }
}

/**
 * Loads binary config of cubes.
 * @deprecated Replaced by text config - remove pending.
 */
long load_cube_file(void)
{
    static const char textname[] = "binary cubes config";
    char* fname = prepare_file_path(FGrp_StdData, "cube.dat");
    SYNCDBG(0,"%s %s file \"%s\".","Reading",textname,fname);
    clear_cubes();
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    if (len > MAX_CONFIG_FILE_SIZE)
    {
        WARNMSG("The %s file \"%s\" is too large.",textname,fname);
        return false;
    }
    char* buf = (char*)LbMemoryAlloc(len + 256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse the config file
    if (result)
    {
        long count = *(long*)&buf[0];
        if (count > len/sizeof(struct CubeAttribs)) {
            count = len/sizeof(struct CubeAttribs);
            WARNMSG("The %s file \"%s\" seem truncated.",textname,fname);
        }
        if (count > CUBE_ITEMS_MAX-1)
            count = CUBE_ITEMS_MAX-1;
        if (count < 0)
            count = 0;
        struct CubeAttribs* cubuf = (struct CubeAttribs*)&buf[4];
        for (long i = 0; i < count; i++)
        {
            struct CubeAttribs* cubed = &game.cubes_data[i];
            int n;
            for (n=0; n < CUBE_TEXTURES; n++) {
                cubed->texture_id[n] = cubuf->texture_id[n];
            }
            for (n=0; n < CUBE_TEXTURES; n++) {
                cubed->field_C[n] = cubuf->field_C[n];
            }
            cubuf++;
        }
        result = true;
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}
/******************************************************************************/
