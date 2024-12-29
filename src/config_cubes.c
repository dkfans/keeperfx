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
#include "pre_inc.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "globals.h"
#include "game_legacy.h"
#include "config.h"
#include "config_cubes.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_cubes_file[] = "cubes.cfg";

const struct NamedCommand cubes_cube_commands[] = {
    {"Name",           1},
    {"Textures",       2},
    {"OwnershipGroup", 3},
    {"Owner",          4},
    {"Properties",     5},
    {NULL, 0},
};

const struct NamedCommand cubes_properties_flags[] = {
    {"LAVA",           CPF_IsLava},
    {"WATER",          CPF_IsWater},
    {"SACRIFICIAL",    CPF_IsSacrificial},
    {"UNCLAIMED_PATH", CPF_IsUnclaimedPath},
    {NULL,             0},
};

/******************************************************************************/
struct NamedCommand cube_desc[CUBE_ITEMS_MAX];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct CubeConfigStats *get_cube_model_stats(long cumodel)
{
    if ((cumodel < 0) || (cumodel >= CUBE_ITEMS_MAX))
    {
        return &game.conf.cube_conf.cube_cfgstats[0];
    }
    return &game.conf.cube_conf.cube_cfgstats[cumodel];
}

TbBool parse_cubes_cube_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct CubeConfigStats *cubest;
    int k = 0;
    // Initialize the cubes array.
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        for (int i = 0; i < CUBE_ITEMS_MAX; i++)
        {
            cubest = &game.conf.cube_conf.cube_cfgstats[i];
            memset(cubest->code_name, 0, COMMAND_WORD_LEN);
            cube_desc[i].name = cubest->code_name;
            cube_desc[i].num = i;
        }
    }
    cube_desc[CUBE_ITEMS_MAX - 1].name = NULL; // Must be null for get_id.
    // Load the file.
    const char *blockname = NULL;
    int blocknamelen = 0;
    long pos = 0;
    while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
    {
        // Look for blocks starting with "cube", followed by one or more digits.
        if (blocknamelen < 5)
        {
            continue;
        }
        else if (memcmp(blockname, "cube", 4) != 0)
        {
            continue;
        }
        const int i = natoi(&blockname[4], blocknamelen - 4);
        if (i < 0 || i >= CUBE_ITEMS_MAX)
        {
            continue;
        }
        else if (i >= game.conf.cube_conf.cube_types_count)
        {
            game.conf.cube_conf.cube_types_count = i + 1;
        }
        cubest = &game.conf.cube_conf.cube_cfgstats[i];
        struct CubeConfigStats *cubed = get_cube_model_stats(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(cubes_cube_commands, cmd_num)
        while (pos < len)
        {
            // Finding command number in this line.
            int cmd_num = recognize_conf_command(buf, &pos, len, cubes_cube_commands);
            // Now store the config item in correct place.
            if (cmd_num == ccr_endOfBlock)
            {
                break; // If next block starts.
            }
            if ((flags & CnfLd_ListOnly) != 0)
            {
                // In "List only" mode, accept only name command.
                if (cmd_num > 1)
                {
                    cmd_num = 0;
                }
            }
            int n = 0;
            char word_buf[COMMAND_WORD_LEN];
            switch (cmd_num)
            {
            case 1: // Name
                if (get_conf_parameter_single(buf, &pos, len, cubest->code_name, COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                        COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
                    break;
                }
                break;
            case 2: // Textures
                while (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    k = atoi(word_buf);
                    if (n >= CUBE_TEXTURES)
                    {
                        CONFWRNLOG("Too many \"%s\" parameters in [%.*s] block of %s file.",
                            COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
                        break;
                    }
                    cubed->texture_id[n] = k;
                    n++;
                }
                if (n < CUBE_TEXTURES)
                {
                    CONFWRNLOG("Couldn't read all \"%s\" parameters in [%.*s] block of %s file.",
                        COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
                }
                break;
            case 3: // OwnershipGroup
                while (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    k = atoi(word_buf);
                    if (k >= CUBE_OWNERSHIP_GROUPS)
                    {
                        CONFWRNLOG("exceeding max amount of ownership groups (%d >= %d)", k, CUBE_OWNERSHIP_GROUPS);
                    }
                    cubed->ownershipGroup = k;
                    n++;
                }
                break;
            case 4: // Owner
                while (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    if (cubed->ownershipGroup <= 0)
                    {
                        CONFWRNLOG("Player without PlayerOwnership in [%.*s] block of %s file.",
                            blocknamelen, blockname, config_textname);
                        break;
                    }
                    k = get_id(cmpgn_human_player_options, word_buf);
                    if (k < 0 || k >= COLOURS_COUNT)
                    {
                        CONFWRNLOG("invalid player in [%.*s] block of %s file.",
                            blocknamelen, blockname, config_textname);
                        cubed->ownershipGroup = 0;
                        break;
                    }
                    cubed->owner = k;
                    game.conf.cube_conf.cube_bits[cubed->ownershipGroup][k] = i;
                    n++;
                }
                break;
            case 5: // Properties
                cubed->properties_flags = 0;
                while (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    k = get_id(cubes_properties_flags, word_buf);
                    switch (k)
                    {
                        case CPF_IsLava: // LAVA
                        {
                            cubed->properties_flags |= CPF_IsLava;
                            n++;
                            break;
                        }
                        case CPF_IsWater: // WATER
                        {
                            cubed->properties_flags |= CPF_IsWater;
                            n++;
                            break;
                        }
                        case CPF_IsSacrificial: // SACRIFICIAL
                        {
                            cubed->properties_flags |= CPF_IsSacrificial;
                            n++;
                            break;
                        }
                        case CPF_IsUnclaimedPath: // UNCLAIMED_PATH
                        {
                            cubed->properties_flags |= CPF_IsUnclaimedPath;
                            n++;
                            break;
                        }
                        default:
                        {
                            CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%.*s] block of %s file.",
                                COMMAND_TEXT(cmd_num), word_buf, blocknamelen, blockname, config_textname);
                            n++;
                            break;
                        }
                    }
                }
                if (n < 0)
                {
                    CONFWRNLOG("Couldn't read all \"%s\" parameters in [%.*s] block of %s file.",
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
            skip_conf_to_next_line(buf, &pos, len);
        }
#undef COMMAND_TEXT
    }
    return true;
}

TbBool load_cubes_config_file(const char *textname, const char *fname, unsigned short flags)
{
    SYNCDBG(0, "%s %s file \"%s\".", ((flags & CnfLd_ListOnly) == 0) ? "Reading" : "Parsing", textname, fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
        {
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.", textname, fname);
        }
        return false;
    }
    char *buf = (char *)LbMemoryAlloc(len + 256);
    if (buf == NULL)
    {
        return false;
    }
    // Loading file data.
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file.
    if (result)
    {
        result = parse_cubes_cube_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
        {
            result = true;
        }
        if (!result)
        {
            WARNMSG("Parsing %s file \"%s\" cube blocks failed.", textname, fname);
        }
    }
    // Freeing and exiting.
    free(buf);
    return result;
}

TbBool load_cubes_config(unsigned short flags)
{
    static const char config_global_textname[] = "global cubes config";
    static const char config_campgn_textname[] = "campaign cubes config";
    static const char config_level_textname[] = "level cubes config";
    char *fname = prepare_file_path(FGrp_FxData, keeper_cubes_file);
    TbBool result = load_cubes_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig, keeper_cubes_file);
    if (strlen(fname) > 0)
    {
        load_cubes_config_file(config_campgn_textname, fname, flags | CnfLd_AcceptPartial | CnfLd_IgnoreErrors);
    }
    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), keeper_cubes_file);
    if (strlen(fname) > 0)
    {
        load_cubes_config_file(config_level_textname, fname, flags | CnfLd_AcceptPartial | CnfLd_IgnoreErrors);
    }
    // Freeing and exiting.
    return result;
}

/* Returns Code Name (name to use in script file) of given cube model. */
const char *cube_code_name(long model)
{
    const char *name = get_conf_parameter_text(cube_desc, model);
    if (name[0] != '\0')
    {
        return name;
    }
    return "INVALID";
}

/*
 * Returns the cube model identifier for a given code name (found in script file). Linear running time.
 * @param code_name
 * @return A positive integer for the cube model if found, otherwise -1.
 */
ThingModel cube_model_id(const char *code_name)
{
    for (int i = 0; i < game.conf.cube_conf.cube_types_count; ++i)
    {
        if (strncasecmp(game.conf.cube_conf.cube_cfgstats[i].code_name, code_name, COMMAND_WORD_LEN) == 0)
        {
            return i;
        }
    }
    return -1;
}

void clear_cubes(void)
{
    memset(&game.conf.cube_conf, 0, sizeof(game.conf.cube_conf));
}

/******************************************************************************/
