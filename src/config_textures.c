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
#include "kfx_memory.h"
#include "pre_inc.h"
#include "config_textures.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "value_util.h"
#include "config_sounds.h"
#include "thing_effects.h"
#include "thing_list.h"

#include <string.h>
#include <toml.h>
#include "config_strings.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static TbBool load_textureanim_config_file(const char *fname, unsigned short flags);
static TbBool load_tileset_config_file(const char *fname, unsigned short flags);
static TbBool tileset_names_post_load(void);

const struct ConfigFileData keeper_textureanim_file_data = {
    .filename = "textureanim.toml",
    .load_func = load_textureanim_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

const struct ConfigFileData keeper_tileset_file_data = {
    .filename = "tileset.toml",
    .load_func = load_tileset_config_file,
    .pre_load_func = NULL,
    .post_load_func = tileset_names_post_load,
};

static char tileset_name[TEXTURE_VARIATIONS_COUNT][COMMAND_WORD_LEN];
struct NamedCommand texture_pack_desc[TEXTURE_VARIATIONS_COUNT + 2];
ThingModel tileset_footprint[TEXTURE_VARIATIONS_COUNT];
unsigned short tileset_footstep_snd[TEXTURE_VARIATIONS_COUNT];
int tileset_footstep_cnt[TEXTURE_VARIATIONS_COUNT];
/******************************************************************************/

static TbBool load_textureanim_config_file(const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(fname,&file_root,flags))
        return false;

    char key[64];
    VALUE *section;
    for (int tex_no = 0; tex_no < TEXTURE_BLOCKS_ANIM_COUNT; tex_no++)
    {

        {
            snprintf(key, sizeof(key), "texture%d", tex_no + TEXTURE_BLOCKS_STAT_COUNT_A);
            section = value_dict_get(&file_root, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            VALUE *frames_arr = value_dict_get(section, "frames");
            if(value_array_size(frames_arr) != TEXTURE_BLOCKS_ANIM_FRAMES)
            {
                WARNLOG("invalid frame no, expected %d got %d",TEXTURE_BLOCKS_ANIM_FRAMES, (int) value_array_size(frames_arr));
            }
            for (size_t frame_no = 0; frame_no < TEXTURE_BLOCKS_ANIM_FRAMES; frame_no++)
            {
                game.texture_animation[ tex_no * TEXTURE_BLOCKS_ANIM_FRAMES + frame_no] = value_int32(value_array_get(frames_arr, frame_no));
            }
        }
    }
    value_fini(&file_root);

    return true;
}

static TbBool tileset_names_post_load(void)
{
    int n = 0;
    for (int i = 0; i < TEXTURE_VARIATIONS_COUNT; i++)
    {
        if (tileset_name[i][0] == '\0')
            continue;
        texture_pack_desc[n].name = tileset_name[i];
        texture_pack_desc[n].num = i;
        n++;
    }
    texture_pack_desc[n].name = "DEFAULT";
    texture_pack_desc[n].num = 0;
    n++;
    texture_pack_desc[n].name = NULL;
    return true;
}

static TbBool load_tileset_config_file(const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(fname, &file_root, flags))
        return false;

    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        memset(tileset_name, 0, sizeof(tileset_name));
        memset(tileset_footprint, 0, sizeof(tileset_footprint));
        memset(tileset_footstep_snd, 0, sizeof(tileset_footstep_snd));
        memset(tileset_footstep_cnt, 0, sizeof(tileset_footstep_cnt));
        snprintf(tileset_name[0], COMMAND_WORD_LEN, "NONE");
    }

    char key[64];
    for (int i = 0; i < TEXTURE_VARIATIONS_COUNT - 1; i++)
    {
        int slot = i + 1;
        snprintf(key, sizeof(key), "texture%d", i);
        VALUE *section = value_dict_get(&file_root, key);
        if (value_type(section) != VALUE_DICT)
            continue;
        VALUE *name = value_dict_get(section, "Name");
        if (value_type(name) == VALUE_STRING)
            snprintf(tileset_name[slot], COMMAND_WORD_LEN, "%s", value_string(name));
        int model = value_parse_model(TCls_EffectElem, value_dict_get(section, "Footprint"));
        if (model > 0)
            tileset_footprint[slot] = model;
        CONDITIONAL_ASSIGN_ARR2_SOUND(section, "FootstepSound", tileset_footstep_snd[slot], tileset_footstep_cnt[slot]);
    }
    value_fini(&file_root);
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
