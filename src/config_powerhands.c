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
    
    if (!load_toml_file(textname, fname,&file_root))
        return false;

    VALUE *common_section = value_dict_get(&file_root, "common");
    if (!common_section)
    {
        WARNMSG("No [common] in %s for file %d", textname, fname);
        value_fini(&file_root);
        return false;
    }

    long count = value_int32(value_dict_get(common_section, "HandsCount"));
    if (count > game.power_hand_conf.variants_count)
    {
        game.power_hand_conf.variants_count = count;
    }
    if (game.power_hand_conf.variants_count > NUM_VARIANTS)
    {
        ERRORLOG("more hands then allowed in %s %d/%d",textname,game.power_hand_conf.variants_count,NUM_VARIANTS);
        game.power_hand_conf.variants_count = NUM_VARIANTS;
    }

    char key[64];
    VALUE *section;
    // Create sections
    for (int variant_no = 0; variant_no < game.power_hand_conf.variants_count; variant_no++)
    {
        sprintf(key, "hand%d", variant_no);
        section = value_dict_get(&file_root, key);

        if (value_type(section) != VALUE_DICT && (flags & CnfLd_IgnoreErrors))
        {
            WARNMSG("Invalid powerhand section %d", variant_no);
        }
        else
        {
            const char* name = value_string(value_dict_get(section, "Name"));
            if(name != NULL)
            {
                if(strlen(name) > COMMAND_WORD_LEN - 1 )
                {
                    ERRORLOG("PowerHand name (%s) to long max %d chars", name,COMMAND_WORD_LEN - 1);
                    break;
                }
                strcpy(game.power_hand_conf.pwrhnd_cfg_stats[variant_no].code_name,name);
            }
            
            game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_speed     = value_int32(value_dict_get(section, "AnimationSpeed"));

            game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_Hold]      = value_parse_anim(value_dict_get(section, "AnimHold"));
            game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_HoldGold]  = value_parse_anim(value_dict_get(section, "AnimHoldGold"));
            game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_Hover]     = value_parse_anim(value_dict_get(section, "AnimHover"));
            game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_Pickup]    = value_parse_anim(value_dict_get(section, "AnimPickup"));
            game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_SideHover] = value_parse_anim(value_dict_get(section, "AnimSideHover"));
            game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_SideSlap]  = value_parse_anim(value_dict_get(section, "AnimSideSlap"));
            game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_Slap]      = value_parse_anim(value_dict_get(section, "AnimSlap"));
            JUSTLOG("game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_Hold] %d",game.power_hand_conf.pwrhnd_cfg_stats[variant_no].anim_idx[HndA_Hold]);
        }
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
