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
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "config_strings.h"
#include "custom_sprites.h"
#include "gui_draw.h"
#include "game_legacy.h"
#include "player_instances.h"
#include "value_util.h"

#include <toml.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static TbBool load_spritecolors_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_spritecolors_file_data = {
    .filename = "spritecolors.toml",
    .load_func = load_spritecolors_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};
/******************************************************************************/
#define MAX_COLORED_SPRITES 255
#define PLAYER_COLORS_COUNT (COLOURS_COUNT + 2)
static short gui_panel_sprites_eq[MAX_COLORED_SPRITES * PLAYER_COLORS_COUNT];
static short pointer_sprites_eq[MAX_COLORED_SPRITES * PLAYER_COLORS_COUNT];
static short button_sprite_eq[MAX_COLORED_SPRITES * PLAYER_COLORS_COUNT];
static short animationIds_eq[MAX_COLORED_SPRITES * PLAYER_COLORS_COUNT];
static short objects_eq[MAX_COLORED_SPRITES * PLAYER_COLORS_COUNT];
/******************************************************************************/
static short get_player_colored_idx(short base_icon_idx,unsigned char color_idx,short *arr);
/******************************************************************************/

static void load_array(VALUE* file_root, const char *arr_name,short *arr, unsigned short flags,short (*string_to_id_f)(const char *))
{
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        memset(arr,0,sizeof(short) * MAX_COLORED_SPRITES * PLAYER_COLORS_COUNT );
    }
    VALUE *toml_arr = value_dict_get(file_root, arr_name);
    if (value_array_size(toml_arr) > MAX_COLORED_SPRITES)
    {
        WARNLOG("too many colored frames, max %d got %d",MAX_COLORED_SPRITES,value_array_size(toml_arr));
    }
    for (size_t sprite_no = 0; sprite_no < value_array_size(toml_arr); sprite_no++)
    {
        VALUE *col_arr = value_array_get(toml_arr, sprite_no);
        if (value_array_size(col_arr) > PLAYER_COLORS_COUNT)
        {
            WARNLOG("too many colors for %s, max %d got %d",arr_name,PLAYER_COLORS_COUNT,value_array_size(col_arr));
            continue;
        }
        for (size_t plr_idx = 0; plr_idx < value_array_size(col_arr); plr_idx++)
        {
            VALUE * entry = value_array_get(col_arr, plr_idx);
            if (value_type(entry) == VALUE_INT32)
            {
                arr[sprite_no * PLAYER_COLORS_COUNT + plr_idx] = value_int32(entry);
            }
            else
            {
                short icon_id = string_to_id_f(value_string(entry));
                if (icon_id == -2)
                {
                    WARNLOG("unknown sprite %s",value_string(entry));
                }
                arr[sprite_no * PLAYER_COLORS_COUNT + plr_idx] = icon_id;
            }
        }
    }
}

static TbBool load_spritecolors_config_file(const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(fname,&file_root,flags))
        return false;

    load_array(&file_root,"gui_panel_sprites",gui_panel_sprites_eq,flags,get_icon_id);
    load_array(&file_root,"pointer_sprites",pointer_sprites_eq,flags,get_icon_id);
    load_array(&file_root,"button_sprite",button_sprite_eq,flags,get_icon_id);
    load_array(&file_root,"button_sprite",button_sprite_eq,flags,get_icon_id);
    load_array(&file_root,"animationIds",animationIds_eq,flags,get_anim_id_);
    load_array(&file_root,"objects",objects_eq,flags,get_anim_id_);

    extern struct CallToArmsGraphics call_to_arms_graphics[];
    for (size_t plr_idx = 0; plr_idx < PLAYER_COLORS_COUNT; plr_idx++)
    {
        call_to_arms_graphics[plr_idx].birth_anim_idx = get_player_colored_idx(867,plr_idx + 1,animationIds_eq);
        call_to_arms_graphics[plr_idx].alive_anim_idx = get_player_colored_idx(868,plr_idx + 1,animationIds_eq);
        call_to_arms_graphics[plr_idx].leave_anim_idx = get_player_colored_idx(869,plr_idx + 1,animationIds_eq);
    }


    value_fini(&file_root);

    return true;
}

static short get_player_colored_idx(short base_icon_idx,unsigned char color_idx,short *arr)
{
    if (color_idx >= PLAYER_COLORS_COUNT)
    {
        return base_icon_idx;
    }
    for (size_t i = 0; i < MAX_COLORED_SPRITES; i++)
    {
        if (arr[i * PLAYER_COLORS_COUNT] == base_icon_idx)
        {
            return arr[i * PLAYER_COLORS_COUNT + color_idx];
        }
        else if (arr[i * PLAYER_COLORS_COUNT] == 0)
        {
            return base_icon_idx;
        }
    }
    return base_icon_idx;
}

short get_player_colored_icon_idx(short base_icon_idx,PlayerNumber plyr_idx)
{
    return get_player_colored_idx(base_icon_idx,get_player_color_idx(plyr_idx) + 1,gui_panel_sprites_eq);
}
short get_player_colored_pointer_icon_idx(short base_icon_idx,PlayerNumber plyr_idx)
{
    return get_player_colored_idx(base_icon_idx,get_player_color_idx(plyr_idx) + 1,pointer_sprites_eq);
}

short get_player_colored_button_sprite_idx(const short base_icon_idx,const PlayerNumber plyr_idx)
{
    unsigned char color_idx;
    if (plyr_idx == PLAYER_NEUTRAL)
    {
        color_idx = (game.play_gameturn % (4 * neutral_flash_rate)) / neutral_flash_rate;
    }
    else
    {
        color_idx = get_player_color_idx(plyr_idx);
    }

    return get_player_colored_idx(base_icon_idx,color_idx + 1,button_sprite_eq);
}

ThingModel get_player_colored_object_model(ThingModel base_model_idx,PlayerNumber plyr_idx)
{
    return get_player_colored_idx(base_model_idx,get_player_color_idx(plyr_idx) + 1,objects_eq);
}

ThingModel get_coloured_object_base_model(ThingModel model_idx)
{
    for (size_t i = 0; i < MAX_COLORED_SPRITES; i++)
    {
        ThingModel base = objects_eq[i*PLAYER_COLORS_COUNT];
        if (base == 0)
            return 0;

        for (size_t j = 0; j < PLAYER_COLORS_COUNT; j++)
        {
            if (model_idx == objects_eq[i*PLAYER_COLORS_COUNT+j])
                return base;
        }
    }
    return 0;

}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
