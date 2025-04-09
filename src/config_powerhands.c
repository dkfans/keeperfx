/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_powerhands.c
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
#include "pre_inc.h"
#include "config_powerhands.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "value_util.h"

#include <toml.h>
#include "config_strings.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const struct ConfigFileData keeper_powerhands_file_data = {
    filename = "powerhands.toml",
    description = "powerhands",
    load_func = load_powerhands_config_file,
    post_load_func = NULL,
};
/******************************************************************************/
typedef struct VALUE VALUE;

struct NamedCommand powerhand_desc[NUM_VARIANTS + 1];

TbBool load_powerhands_config_file(const char *textname, const char *fname, unsigned short flags)
{
    VALUE file_root;
    
    if (!load_toml_file(textname, fname,&file_root,flags))
        return false;

    char key[64];
    VALUE *section;
    // Create sections
    for (int id = 0; id < NUM_VARIANTS; id++)
    {
        sprintf(key, "hand%d", id);
        section = value_dict_get(&file_root, key);
        if (value_type(section) == VALUE_DICT)
        {
            struct PowerHandConfigStats *pwrhnd_cfg_stat = &game.conf.power_hand_conf.pwrhnd_cfg_stats[id];

            SET_NAME(section,powerhand_desc,pwrhnd_cfg_stat->code_name);
            
            CONDITIONAL_ASSIGN_ANIMID(section, "HoldAnim",      pwrhnd_cfg_stat->anim_idx[HndA_Hold]);
            CONDITIONAL_ASSIGN_ANIMID(section, "HoldGoldAnim",  pwrhnd_cfg_stat->anim_idx[HndA_HoldGold]);
            CONDITIONAL_ASSIGN_ANIMID(section, "HoverAnim",     pwrhnd_cfg_stat->anim_idx[HndA_Hover]);
            CONDITIONAL_ASSIGN_ANIMID(section, "PickupAnim",    pwrhnd_cfg_stat->anim_idx[HndA_Pickup]);
            CONDITIONAL_ASSIGN_ANIMID(section, "SideHoverAnim", pwrhnd_cfg_stat->anim_idx[HndA_SideHover]);
            CONDITIONAL_ASSIGN_ANIMID(section, "SideSlapAnim",  pwrhnd_cfg_stat->anim_idx[HndA_SideSlap]);
            CONDITIONAL_ASSIGN_ANIMID(section, "SlapAnim",      pwrhnd_cfg_stat->anim_idx[HndA_Slap]);

            CONDITIONAL_ASSIGN_INT(section, "HoldSpeed",      pwrhnd_cfg_stat->anim_speed[HndA_Hold]);
            CONDITIONAL_ASSIGN_INT(section, "HoldGoldSpeed",  pwrhnd_cfg_stat->anim_speed[HndA_HoldGold]);
            CONDITIONAL_ASSIGN_INT(section, "HoverSpeed",     pwrhnd_cfg_stat->anim_speed[HndA_Hover]);
            CONDITIONAL_ASSIGN_INT(section, "PickupSpeed",    pwrhnd_cfg_stat->anim_speed[HndA_Pickup]);
            CONDITIONAL_ASSIGN_INT(section, "SideHoverSpeed", pwrhnd_cfg_stat->anim_speed[HndA_SideHover]);
            CONDITIONAL_ASSIGN_INT(section, "SideSlapSpeed",  pwrhnd_cfg_stat->anim_speed[HndA_SideSlap]);
            CONDITIONAL_ASSIGN_INT(section, "SlapSpeed",      pwrhnd_cfg_stat->anim_speed[HndA_Slap]);
        }
    }

    sprintf(key, "hand%d", NUM_VARIANTS);
    section = value_dict_get(&file_root, key);
    if (value_type(section) == VALUE_DICT)
    {
        WARNMSG("more powerhands defined then max of %d", NUM_VARIANTS);
    }

    value_fini(&file_root);
    
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
