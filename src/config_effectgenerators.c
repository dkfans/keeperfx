/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_effectgenerators.c
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
#include "config_effectgenerators.h"
#include "config_effects.h"
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
const char keeper_effectgenerator_file[]="effectgenerator.cfg";
/******************************************************************************/

TbBool load_effectgenerator_config_file(const char *textname, const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(textname, fname,&file_root))
        return false;

    char key[64];
    VALUE *section;
    for (int id = 0; id < EFFECTSGEN_TYPES_MAX; id++)
    {
        {
            sprintf(key, "effectGenerator%d", id);
            section = value_dict_get(&file_root, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
/*
Name = "Lava"
GenationDelayMin = 10
GenationDelayMax = 20
GenationAmount = 1
EffectElementModel = 30
IgnoreTerrain = 1
SpawnHeight = 0
AccelerationMin = [-40,-40,80]
AccelerationMax = [40,40,150]
Sound = [147,3]

                    EffectGeneratorConfigStats effgenstat = ;

                    game.slabobjs[game.slabobjs_num].range    = value_int32(value_dict_get(object, "EffectRange"));
                    game.slabobjs[game.slabobjs_num].range    = value_int32(value_dict_get(object, "EffectRange"));
                    game.slabobjs[game.slabobjs_num].range    = value_int32(value_dict_get(object, "EffectRange"));
                    game.slabobjs[game.slabobjs_num].range    = value_int32(value_dict_get(object, "EffectRange"));
                    VALUE *accelerationMin_arr = value_dict_get(object, "AccelerationMin");
                    game.slabobjs[game.slabobjs_num].offset_x = value_int32(value_array_get(accelerationMin_arr, 0));
                    game.slabobjs[game.slabobjs_num].offset_y = value_int32(value_array_get(accelerationMin_arr, 1));

                    game.slabobjs[game.slabobjs_num].range    = value_int32(value_dict_get(object, "EffectRange"));

*/

        }
    }
    value_fini(&file_root);
    
    return true;
}


TbBool load_effectgenerator_config(const char *conf_fname,unsigned short flags)
{
    static const char config_global_textname[] = "global effectgenerator config";
    static const char config_campgn_textname[] = "campaign effectgenerator config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_effectgenerator_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_effectgenerator_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting

    return result;
}

struct EffectGeneratorConfigStats *get_effectgenerator_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= EFFECTSGEN_TYPES_MAX)
        return &effects_conf.effectgen_cfgstats[0];
    return &effects_conf.effectgen_cfgstats[tngmodel];
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
