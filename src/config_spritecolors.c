/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_textures.c
 *     texture animation configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for trap and door elements.
 * @par Comment:
 *     None.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_spritecolors.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "value_util.h"

#include <toml.h>
#include "config_strings.h"
#include "custom_sprites.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_spritecolors_file[]="spritecolors.toml";
/******************************************************************************/
#define MAX_COLORED_SPRITES 255
#define PLAYER_COLORS_COUNT 5
short sprite_color_equivalent[MAX_COLORED_SPRITES][PLAYER_COLORS_COUNT];
/******************************************************************************/

TbBool load_spritecolors_config_file(const char *textname, const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(textname, fname,&file_root,flags))
        return false;

    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        memset(sprite_color_equivalent,0,sizeof(sprite_color_equivalent));
    }

    VALUE *gui_panel_sprites_arr = value_dict_get(&file_root, "gui_panel_sprites");
    if(value_array_size(gui_panel_sprites_arr) > MAX_COLORED_SPRITES)
    {
        WARNLOG("to many colored frames, max %d got %d",MAX_COLORED_SPRITES,value_array_size(gui_panel_sprites_arr));
    }
    for (size_t sprite_no = 0; sprite_no < value_array_size(gui_panel_sprites_arr); sprite_no++)
    {
        VALUE *col_arr = value_array_get(gui_panel_sprites_arr, sprite_no);
        if (value_array_size(col_arr) > PLAYER_COLORS_COUNT)
        {
            WARNLOG("to many colors for sprite, max %d got %d",PLAYER_COLORS_COUNT,value_array_size(col_arr));
        }

        for (size_t i = 0; i < value_array_size(col_arr); i++)
        {
            VALUE * entry = value_array_get(col_arr, i);
            JUSTLOG("value_type(entry) %d",value_type(entry));
            if(value_type(entry) == VALUE_INT32)
                sprite_color_equivalent[sprite_no][i] = value_int32(entry);
            else
            {
                short icon_id = get_icon_id(value_string(entry));
                if(icon_id == -2)
                {
                    WARNLOG("unknown sprite %s",value_string(entry));
                }
                sprite_color_equivalent[sprite_no][i] = icon_id;
            }
                
        }
    
    }

    value_fini(&file_root);
    
    return true;
}


TbBool load_spritecolors_config(const char *conf_fname,unsigned short flags)
{
    static const char config_global_textname[] = "global spritecolors config";
    static const char config_campgn_textname[] = "campaign spritecolors config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_spritecolors_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_spritecolors_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting

    return result;
}

short get_player_colored_icon_idx(short base_icon_idx,PlayerNumber plyr_idx)
{
    if(plyr_idx == PLAYER0)
    {
        plyr_idx = PLAYER1;
        //return base_icon_idx;
    }
    for (size_t i = 0; i < MAX_COLORED_SPRITES; i++)
    {
        if(sprite_color_equivalent[i][PLAYER0] == base_icon_idx)
        {
            JUSTLOG("sprite_color_equivalent[i][plyr_idx]; %d",sprite_color_equivalent[i][plyr_idx]);
            return sprite_color_equivalent[i][plyr_idx];
        }
        else if (sprite_color_equivalent[i][PLAYER0] == 0)
        {
            return base_icon_idx;
        }
    }
    return base_icon_idx;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
