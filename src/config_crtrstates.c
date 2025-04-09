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
#include "pre_inc.h"
#include "config_crtrstates.h"
#include "globals.h"
#include "game_legacy.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_data.h"
#include "config_creature.h"
#include "creature_states.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct NamedCommand creatrstate_desc[CREATURE_STATES_MAX];
/******************************************************************************/
const struct NamedField crstates_states_named_fields[] = {
    //name   //pos   //field                                //default //min   //max //NamedCommand   //parse function //script assign function
    {"NAME",   0, field(game.conf.crtr_conf.states[0].name), 0,        0,      0, creatrstate_desc,  value_name,      assign_null},
    {NULL},
};

const struct NamedFieldSet crstates_states_named_fields_set = {
    &game.conf.crtr_conf.states_count,
    "state",
    crstates_states_named_fields,
    creatrstate_desc,
    CREATURE_STATES_MAX,
    sizeof(game.conf.crtr_conf.states[0]),
    game.conf.crtr_conf.states,
    {"crstates.cfg","INVALID_SCRIPT"},
};

static TbBool load_creaturestates_config_file(const char *textname, const char *fname, unsigned short flags);

const struct ConfigFileData creature_states_file_data = {
    .filename = "crstates.cfg",
    .description = "creature states",
    .load_func = load_creaturestates_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

/******************************************************************************/

static TbBool load_creaturestates_config_file(const char *textname, const char *fname, unsigned short flags)
{
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file
    parse_named_field_blocks(buf, len, textname, flags, &crstates_states_named_fields_set);

    //Freeing and exiting
    free(buf);
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
