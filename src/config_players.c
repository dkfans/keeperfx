/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_players.c
 *     Players configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for players.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     17 Sep 2012 - 06 Mar 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_players.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"

#include <toml.h>
#include "value_util.h"
#include "config.h"
#include "config_players.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static TbBool load_playerstate_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_playerstates_file_data = {
    .filename = "playerstates.toml",
    .load_func = load_playerstate_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

struct NamedCommand player_state_commands[PLAYER_STATES_COUNT_MAX];

static const struct NamedCommand pointer_group_commands[] = {
    {"NONE"          ,PsPg_None        },
    {"CTRLDUNGEON"   ,PsPg_CtrlDungeon },
    {"BUILDROOM"     ,PsPg_BuildRoom   },
    {"INVISIBLE"     ,PsPg_Invisible   },
    {"SPELL"         ,PsPg_Spell       },
    {"QUERY"         ,PsPg_Query       },
    {"PLACETRAP"     ,PsPg_PlaceTrap   },
    {"PLACEDOOR"     ,PsPg_PlaceDoor   },
    {"SELL"          ,PsPg_Sell        },
    {"PLACETERRAIN"  ,PsPg_PlaceTerrain},
    {"MAKEDIGGER"    ,PsPg_MkDigger    },
    {"MAKECREATURE"  ,PsPg_MkCreatr    },
    {"ORDERCREATURE" ,PsPg_OrderCreatr }
};

/******************************************************************************/
/******************************************************************************/

static TbBool load_playerstate_config_file(const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(fname,&file_root,flags))
        return false;
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        memset(player_state_commands,0,sizeof(player_state_commands));
    }

    char key[64] = "";
    VALUE *section;
    for (int id = 0; id < PLAYER_STATES_COUNT_MAX; id++)
    {
        {
            snprintf(key, sizeof(key), "playerstate%d", id);
            section = value_dict_get(&file_root, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct PlayerStateConfigStats *plrst_cfg_stat = &game.conf.plyr_conf.plrst_cfg_stats[id];
            SET_NAME(section, player_state_commands, plrst_cfg_stat->code_name);
            VALUE *pointer_group_val = value_dict_get(section, "PointerGroup");
            plrst_cfg_stat->pointer_group = get_id(pointer_group_commands,value_string(pointer_group_val));

            VALUE *stop_own_units_val = value_dict_get(section, "StopOwnUnits");
            plrst_cfg_stat->stop_own_units = value_bool(stop_own_units_val);
        }
    }
    value_fini(&file_root);
    return true;
}

/**
 * Returns Code Name (name to use in script file) of given player state.
 */
const char *player_state_code_name(int wrkstate)
{
    const char* name = get_conf_parameter_text(player_state_commands, wrkstate);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

struct PlayerStateConfigStats *get_player_state_stats(PlayerState plyr_state)
{
    return &game.conf.plyr_conf.plrst_cfg_stats[plyr_state];
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
