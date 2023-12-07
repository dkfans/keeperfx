/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_effects.c
 *     Effects configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for effects and effect elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 11 Mar 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_effects.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "value_util.h"
#include <toml.h>
#include "config.h"
#include "config_strings.h"
#include "thing_effects.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_effects_file[]="effects.toml";

long const imp_spangle_effects[] = {
    TngEff_ImpSpangleRed, TngEff_ImpSpangleBlue, TngEff_ImpSpangleGreen, TngEff_ImpSpangleYellow, TngEff_None, TngEff_None,
};

long const ball_puff_effects[] = {
    TngEff_BallPuffRed, TngEff_BallPuffBlue, TngEff_BallPuffGreen, TngEff_BallPuffYellow, TngEff_BallPuffWhite, TngEff_BallPuffWhite,
};

/******************************************************************************/
struct NamedCommand effect_desc[EFFECTS_TYPES_MAX];
struct NamedCommand effectgen_desc[EFFECTSGEN_TYPES_MAX];
/******************************************************************************/

static void load_effects(VALUE *value, unsigned short flags)
{
    char key[64];
    VALUE *section;
    for (int id = 0; id < EFFECTS_TYPES_MAX; id++)
    {
        {
            sprintf(key, "effect%d", id);
            section = value_dict_get(value, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct EffectConfigStats *effcst = &gameadd.effects_conf.effect_cfgstats[id];

            const char* name = value_string(value_dict_get(section, "Name"));
            if(name != NULL)
            {
                if(strlen(name) > COMMAND_WORD_LEN - 1 )
                {
                    ERRORLOG("effect name (%s) to long max %d chars", name,COMMAND_WORD_LEN - 1);
                    break;
                }

                strcpy(effcst->code_name,name);
                effect_desc[id].name = effcst->code_name;
                effect_desc[id].num = id;
            }
            if ((flags & CnfLd_ListOnly) != 0)
            {
                continue;
            }

            effcst->start_health   = value_int32(value_dict_get(section,"Health"));
            effcst->generation_type   = value_int32(value_dict_get(section,"GenerationType"));

            VALUE *generationAccelXYRange_arr = value_dict_get(section, "GenerationAccelXYRange");
            effcst->accel_xy_min = value_int32(value_array_get(generationAccelXYRange_arr, 0));
            effcst->accel_xy_max = value_int32(value_array_get(generationAccelXYRange_arr, 1));

            VALUE *generationAccelZRange_arr = value_dict_get(section, "GenerationAccelZRange");
            effcst->accel_z_min = value_int32(value_array_get(generationAccelZRange_arr, 0));
            effcst->accel_z_max = value_int32(value_array_get(generationAccelZRange_arr, 1));

            VALUE *generationKindRange_arr = value_dict_get(section, "GenerationKindRange");
            effcst->kind_min = value_int32(value_array_get(generationKindRange_arr, 0));
            effcst->kind_max = value_int32(value_array_get(generationKindRange_arr, 1));

            effcst->area_affect_type  = value_int32(value_dict_get(section,"AreaAffectType"));
            effcst->effect_sound      = value_int32(value_dict_get(section,"Sound"));
            effcst->affected_by_wind  = value_int32(value_dict_get(section,"AffectedByWind"));
            effcst->ilght.radius      = value_int32(value_dict_get(section,"LightRadius"));
            effcst->ilght.intensity   = value_int32(value_dict_get(section,"LightIntensity"));
            effcst->ilght.field_3     = value_int32(value_dict_get(section,"LightFlags"));
            effcst->elements_count    = value_int32(value_dict_get(section,"ElementsCount"));
            effcst->always_generate   = value_int32(value_dict_get(section,"AlwaysGenerate"));

        }
    }
}

static void load_effectsgenerators(VALUE *value, unsigned short flags)
{
    char key[64];
    VALUE *section;
    for (int id = 0; id < EFFECTSGEN_TYPES_MAX; id++)
    {
        {
            sprintf(key, "effectGenerator%d", id);
            section = value_dict_get(value, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct EffectGeneratorConfigStats *effgencst = &gameadd.effects_conf.effectgen_cfgstats[id];

            const char* name = value_string(value_dict_get(section, "Name"));
            if(name != NULL)
            {
                if(strlen(name) > COMMAND_WORD_LEN - 1 )
                {
                    ERRORLOG("effectgenerator name (%s) to long max %d chars", name,COMMAND_WORD_LEN - 1);
                    break;
                }

                strcpy(effgencst->code_name,name);
                effectgen_desc[id].name = effgencst->code_name;
                effectgen_desc[id].num = id;
            }
            if ((flags & CnfLd_ListOnly) != 0)
            {
                continue;
            }

            effgencst->genation_delay_min   = value_int32(value_dict_get(section,"GenationDelayMin"));
            effgencst->genation_delay_max   = value_int32(value_dict_get(section,"GenationDelayMax"));
            effgencst->genation_amount      = value_int32(value_dict_get(section,"GenationAmount"));
            effgencst->effect_element_model = value_int32(value_dict_get(section,"EffectElementModel"));
            effgencst->ignore_terrain       = value_int32(value_dict_get(section,"IgnoreTerrain"));
            effgencst->spawn_height         = value_int32(value_dict_get(section,"SpawnHeight"));

            VALUE *accelerationMin_arr = value_dict_get(section, "AccelerationMin");
            effgencst->acc_x_min = value_int32(value_array_get(accelerationMin_arr, 0));
            effgencst->acc_y_min = value_int32(value_array_get(accelerationMin_arr, 1));
            effgencst->acc_z_min = value_int32(value_array_get(accelerationMin_arr, 2));

            VALUE *accelerationMax_arr = value_dict_get(section, "AccelerationMax");
            effgencst->acc_x_max = value_int32(value_array_get(accelerationMax_arr, 0));
            effgencst->acc_y_max = value_int32(value_array_get(accelerationMax_arr, 1));
            effgencst->acc_z_max = value_int32(value_array_get(accelerationMax_arr, 2));

            VALUE *sound_arr = value_dict_get(section, "Sound");
            effgencst->sound_sample_idx = value_int32(value_array_get(sound_arr, 0));
            effgencst->sound_sample_rng = value_int32(value_array_get(sound_arr, 1));
        }
    }
}

static TbBool load_effects_config_file(const char *textname, const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(textname, fname,&file_root,flags))
        return false;
    load_effects(&file_root,flags);
    load_effectsgenerators(&file_root,flags);

    value_fini(&file_root);
    
    return true;
}

TbBool load_effects_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global effects config";
    static const char config_campgn_textname[] = "campaign effects config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_effects_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_effects_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given effect model.
 */
const char *effect_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(effect_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

const char *effectgenerator_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(effectgen_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

struct EffectGeneratorConfigStats *get_effectgenerator_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= EFFECTSGEN_TYPES_MAX)
        return &gameadd.effects_conf.effectgen_cfgstats[0];
    return &gameadd.effects_conf.effectgen_cfgstats[tngmodel];
}

struct EffectConfigStats *get_effect_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= EFFECTS_TYPES_MAX)
        return &gameadd.effects_conf.effect_cfgstats[0];
    return &gameadd.effects_conf.effect_cfgstats[tngmodel];
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
