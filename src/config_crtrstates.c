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
const struct NamedCommand creature_state_types_commands[] = {
    {"Idle",        CrStTyp_Idle},
    {"Work",        CrStTyp_Work},
    {"OwnNeeds",    CrStTyp_OwnNeeds},
    {"Sleep",       CrStTyp_Sleep},
    {"Feed",        CrStTyp_Feed},
    {"FightCrtr",   CrStTyp_FightCrtr},
    {"Move",        CrStTyp_Move},
    {"GetsSalary",  CrStTyp_GetsSalary},
    {"Escape",      CrStTyp_Escape},
    {"Unconscious", CrStTyp_Unconscious},
    {"AngerJob",    CrStTyp_AngerJob},
    {"FightDoor",   CrStTyp_FightDoor},
    {"FightObj",    CrStTyp_FightObj},
    {"Called2Arms", CrStTyp_Called2Arms},
    {"Follow",      CrStTyp_Follow},
    {NULL,0}
};

const struct NamedCommand follow_behavior_commands[] = {
    {"none",                FlwB_None},
    {"FollowLeader",        FlwB_FollowLeader},
    {"MatchWorkRoom",       FlwB_MatchWorkRoom},
    {"JoinCombatOrFollow",  FlwB_JoinCombatOrFollow},
    {NULL,0}
};

int64_t value_overrides(const struct NamedField* named_field, const char* value_text,
    const struct NamedFieldSet* named_fields_set, int idx,
    const char* src_str, unsigned char flags)
{
    struct CreatureStateConfig* state = &game.conf.crtr_conf.states[idx];

    state->override_feed          = 0;
    state->override_own_needs     = 0;
    state->override_sleep         = 0;
    state->override_fight_crtr    = 0;
    state->override_gets_salary   = 0;
    state->override_captive       = 0;
    state->override_transition    = 0;
    state->override_escape        = 0;
    state->override_unconscious   = 0;
    state->override_anger_job     = 0;
    state->override_fight_object  = 0;
    state->override_fight_door    = 0;
    state->override_call2arms     = 0;
    state->override_follow        = 0;

    char buffer[256];
    strncpy(buffer, value_text, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* token = strtok(buffer, " ");
    while (token) {
        if (strcasecmp(token, "FEED") == 0) state->override_feed = 1;
        else if (strcasecmp(token, "OWN_NEEDS") == 0) state->override_own_needs = 1;
        else if (strcasecmp(token, "SLEEP") == 0) state->override_sleep = 1;
        else if (strcasecmp(token, "FIGHT_CRTR") == 0) state->override_fight_crtr = 1;
        else if (strcasecmp(token, "GETS_SALARY") == 0) state->override_gets_salary = 1;
        else if (strcasecmp(token, "CAPTIVE") == 0) state->override_captive = 1;
        else if (strcasecmp(token, "TRANSITION") == 0) state->override_transition = 1;
        else if (strcasecmp(token, "ESCAPE") == 0) state->override_escape = 1;
        else if (strcasecmp(token, "UNCONSCIOUS") == 0) state->override_unconscious = 1;
        else if (strcasecmp(token, "ANGER_JOB") == 0) state->override_anger_job = 1;
        else if (strcasecmp(token, "FIGHT_OBJECT") == 0) state->override_fight_object = 1;
        else if (strcasecmp(token, "FIGHT_DOOR") == 0) state->override_fight_door = 1;
        else if (strcasecmp(token, "CALL2ARMS") == 0) state->override_call2arms = 1;
        else if (strcasecmp(token, "FOLLOW") == 0) state->override_follow = 1;
        else {
            WARNLOG("Unknown override name: '%s' in '%s'", token, src_str);
        }

        token = strtok(NULL, " ");
    }

    return 0;
}

const struct NamedField crstates_states_named_fields[] = {
    {"NAME",                   0, field(game.conf.crtr_conf.states[0].name),                        0,        0,      0,              creatrstate_desc,  value_name,      assign_null},
    {"PROCESSFUNCTION",        0, field(game.conf.crtr_conf.states[0].process_state),               0,        0,      0,         process_func_commands,  value_function,  assign_default},
    {"CLEANUPFUNCTION",        0, field(game.conf.crtr_conf.states[0].cleanup_state),               0,        0,      0,         cleanup_func_commands,  value_function,  assign_default},
    {"MOVEFROMSLABFUNCTION",   0, field(game.conf.crtr_conf.states[0].move_from_slab),              0,        0,      0,  move_from_slab_func_commands,  value_function,  assign_default},
    {"MOVECHECKFUNCTION",      0, field(game.conf.crtr_conf.states[0].move_check),                  0,        0,      0,      move_check_func_commands,  value_function,  assign_default},
    {"OVERRIDES",             -1, NULL,0,                                                           0,        0,      0,                          NULL,  value_overrides, assign_null},
    {"STATETYPE",              0, field(game.conf.crtr_conf.states[0].state_type),                  0,        0,      0, creature_state_types_commands,  value_default,   assign_default},
    {"CAPTIVE",                0, field(game.conf.crtr_conf.states[0].captive),                     0,        0,      1,                          NULL,  value_default,   assign_default},
    {"TRANSITION",             0, field(game.conf.crtr_conf.states[0].transition),                  0,        0,      1,                          NULL,  value_default,   assign_default},
    {"FOLLOWBEHAVIOR",         0, field(game.conf.crtr_conf.states[0].follow_behavior),             0,        0,      0,      follow_behavior_commands,  value_default,   assign_default},
    {"BLOCKSALLSTATECHANGES",  0, field(game.conf.crtr_conf.states[0].blocks_all_state_changes),    0,        0,      1,                          NULL,  value_default,   assign_default},
    {"SPRITEIDX",              0, field(game.conf.crtr_conf.states[0].sprite_idx),                  0,        0,      0,                          NULL,  value_icon,      assign_icon},
    {"DISPLAYTHOUGHTBUBBLE",   0, field(game.conf.crtr_conf.states[0].display_thought_bubble),      0,        0,      1,                          NULL,  value_default,   assign_default},
    {"SNEAKY",                 0, field(game.conf.crtr_conf.states[0].sneaky),                      0,        0,      1,                          NULL,  value_default,   assign_default},
    {"REACTTOCTA",             0, field(game.conf.crtr_conf.states[0].react_to_cta),                0,        0,      1,                          NULL,  value_default,   assign_default},
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
};

static TbBool load_creaturestates_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData creature_states_file_data = {
    .filename = "crstates.cfg",
    .load_func = load_creaturestates_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

/******************************************************************************/

static TbBool load_creaturestates_config_file(const char *fname, unsigned short flags)
{
    SYNCDBG(0,"%s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("file \"%s\" doesn't exist or is too small.",fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file
    parse_named_field_blocks(buf, len, fname, flags, &crstates_states_named_fields_set);

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
