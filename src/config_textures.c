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
#include "config_textures.h"
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
const char keeper_textureanim_file[]="textureanim.cfg";
/******************************************************************************/

TbBool load_textureanim_config_file(const char *textname, const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(textname, fname,&file_root))
        return false;

    char key[64];
    VALUE *section;
    for (int tex_no = 0; tex_no < TEXTURE_BLOCKS_ANIM_COUNT; tex_no++)
    {
       
        {
            sprintf(key, "texture%d", tex_no + TEXTURE_BLOCKS_STAT_COUNT_A);
            section = value_dict_get(&file_root, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            VALUE *frames_arr = value_dict_get(section, "frames");
            if(value_array_size(frames_arr) != TEXTURE_BLOCKS_ANIM_FRAMES)
            {
                WARNLOG("invalid frame no, expected %d got %d",TEXTURE_BLOCKS_ANIM_FRAMES,value_array_size(frames_arr));
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


TbBool load_textureanim_config(const char *conf_fname,unsigned short flags)
{
    static const char config_global_textname[] = "global texture config";
    static const char config_campgn_textname[] = "campaign texture config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_textureanim_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_textureanim_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting

    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
