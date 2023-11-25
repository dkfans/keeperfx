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
#include "bflib_memory.h"
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
const char keeper_powerhands_file[]="powerhands.cfg";
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
    for (int variant_no = 0; variant_no < NUM_VARIANTS; variant_no++)
    {
        sprintf(key, "hand%d", variant_no);
        section = value_dict_get(&file_root, key);
        if (value_type(section) == VALUE_DICT)
        {
            struct PowerHandConfigStats *pwrhnd_cfg_stat = &game.power_hand_conf.pwrhnd_cfg_stats[variant_no];

            const char* name = value_string(value_dict_get(section, "Name"));
            if(name != NULL)
            {
                if(strlen(name) > COMMAND_WORD_LEN - 1 )
                {
                    ERRORLOG("PowerHand name (%s) to long max %d chars", name,COMMAND_WORD_LEN - 1);
                    break;
                }

                strcpy(pwrhnd_cfg_stat->code_name,name);
                powerhand_desc[variant_no].name = pwrhnd_cfg_stat->code_name;
                powerhand_desc[variant_no].num = variant_no;
            }
            
            pwrhnd_cfg_stat->anim_speed = value_int32(value_dict_get(section, "AnimationSpeed"));

            pwrhnd_cfg_stat->anim_idx[HndA_Hold]      = value_parse_anim(value_dict_get(section, "AnimHold"));
            pwrhnd_cfg_stat->anim_idx[HndA_HoldGold]  = value_parse_anim(value_dict_get(section, "AnimHoldGold"));
            pwrhnd_cfg_stat->anim_idx[HndA_Hover]     = value_parse_anim(value_dict_get(section, "AnimHover"));
            pwrhnd_cfg_stat->anim_idx[HndA_Pickup]    = value_parse_anim(value_dict_get(section, "AnimPickup"));
            pwrhnd_cfg_stat->anim_idx[HndA_SideHover] = value_parse_anim(value_dict_get(section, "AnimSideHover"));
            pwrhnd_cfg_stat->anim_idx[HndA_SideSlap]  = value_parse_anim(value_dict_get(section, "AnimSideSlap"));
            pwrhnd_cfg_stat->anim_idx[HndA_Slap]      = value_parse_anim(value_dict_get(section, "AnimSlap"));
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

TbBool load_powerhands_config(const char *conf_fname,unsigned short flags)
{
    static const char config_global_textname[] = "global powerhand config";
    static const char config_campgn_textname[] = "campaign powerhand config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_powerhands_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_powerhands_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
