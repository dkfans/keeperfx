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
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "console_cmd.h"

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
static TbBool load_effects_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_effects_file_data = {
    .filename = "effects.toml",
    .load_func = load_effects_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};


const struct NamedField effects_effectgenerator_named_fields[] = {
    {"NAME",                   0, field(game.conf.effects_conf.effectgen_cfgstats[0].code_name),            0,    INT32_MIN, UINT32_MAX, effectgen_desc,  value_name,      assign_null},
    {"GENERATIONDELAYMIN",     0, field(game.conf.effects_conf.effectgen_cfgstats[0].generation_delay_min), 0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GENERATIONDELAYMAX",     0, field(game.conf.effects_conf.effectgen_cfgstats[0].generation_delay_max), 0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GENERATIONAMOUNT",       0, field(game.conf.effects_conf.effectgen_cfgstats[0].generation_amount),    0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"EFFECTMODEL",            0, field(game.conf.effects_conf.effectgen_cfgstats[0].effect_model),         0,    INT32_MIN, UINT32_MAX, NULL,            value_effOrEffEl,assign_default},
    {"IGNORETERRAIN",          0, field(game.conf.effects_conf.effectgen_cfgstats[0].ignore_terrain),       0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SPAWNHEIGHT",            0, field(game.conf.effects_conf.effectgen_cfgstats[0].spawn_height),         1,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMIN",        0, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_x_min),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMIN",        1, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_y_min),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMIN",        2, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_z_min),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMAX",        0, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_x_max),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMAX",        1, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_y_max),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMAX",        2, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_z_max),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SOUND",                  0, field(game.conf.effects_conf.effectgen_cfgstats[0].sound_sample_idx),     0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SOUND",                  1, field(game.conf.effects_conf.effectgen_cfgstats[0].sound_sample_rng),     0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {NULL},
};


const struct NamedFieldSet effects_effectgenerator_named_fields_set = {
    &game.conf.effects_conf.effectgen_cfgstats_count,
    "effectGenerator",
    effects_effectgenerator_named_fields,
    effectgen_desc,
    EFFECTSGEN_TYPES_MAX,
    sizeof(game.conf.effects_conf.effectgen_cfgstats[0]),
    game.conf.effects_conf.effectgen_cfgstats,
};

int32_t const imp_spangle_effects[] = {
    TngEff_ImpSpangleRed, TngEff_ImpSpangleBlue, TngEff_ImpSpangleGreen, TngEff_ImpSpangleYellow, TngEff_ImpSpangleWhite,
    TngEff_None, TngEff_ImpSpanglePurple, TngEff_ImpSpangleBlack, TngEff_ImpSpangleOrange
};

int32_t const ball_puff_effects[] = {
    TngEff_BallPuffRed, TngEff_BallPuffBlue, TngEff_BallPuffGreen, TngEff_BallPuffYellow, TngEff_BallPuffWhite,
    TngEff_BallPuffWhite, TngEff_BallPuffPurple, TngEff_BallPuffBlack, TngEff_BallPuffOrange
};

/******************************************************************************/
struct NamedCommand effect_desc[EFFECTS_TYPES_MAX];
struct NamedCommand effectgen_desc[EFFECTSGEN_TYPES_MAX];
struct NamedCommand effectelem_desc[EFFECTSELLEMENTS_TYPES_MAX];
/******************************************************************************/

static void load_effects(VALUE *value, unsigned short flags)
{
    char key[64] = "";
    VALUE *section;
    for (int id = 0; id < EFFECTS_TYPES_MAX; id++)
    {
        {
            snprintf(key, sizeof(key), "effect%d", id);
            section = value_dict_get(value, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct EffectConfigStats *effcst = &game.conf.effects_conf.effect_cfgstats[id];

            SET_NAME(section,effect_desc,effcst->code_name);

            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"GenerationAccelXYRange",effcst->accel_xy_min,effcst->accel_xy_max);
            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"GenerationAccelZRange", effcst->accel_z_min, effcst->accel_z_max);
            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"GenerationKindRange",   effcst->kind_min,    effcst->kind_max);
            CONDITIONAL_ASSIGN_INT(section,"Health",        effcst->start_health);
            CONDITIONAL_ASSIGN_INT(section,"GenerationType",effcst->generation_type);
            CONDITIONAL_ASSIGN_INT(section,"AreaAffectType",effcst->area_affect_type);
            CONDITIONAL_ASSIGN_INT(section,"Sound"         ,effcst->effect_sound    );
            CONDITIONAL_ASSIGN_INT(section,"AffectedByWind",effcst->affected_by_wind);
            CONDITIONAL_ASSIGN_INT_SCALED(section,"LightRadius"   ,effcst->ilght.radius, COORD_PER_STL);
            CONDITIONAL_ASSIGN_INT(section,"LightIntensity",effcst->ilght.intensity );
            CONDITIONAL_ASSIGN_INT(section,"LightFlags"    ,effcst->ilght.flags   );
            CONDITIONAL_ASSIGN_INT(section,"ElementsCount" ,effcst->elements_count  );
            CONDITIONAL_ASSIGN_INT(section,"AlwaysGenerate",effcst->always_generate );
            CONDITIONAL_ASSIGN_INT(section,"HitType",effcst->effect_hit_type);
            CONDITIONAL_ASSIGN_SPELL(section,"SpellEffect",effcst->spell_effect);
        }
    }
}

static void load_effectsgenerators(VALUE *value, unsigned short flags)
{
    char key[KEY_SIZE];
    VALUE *section;
    for (int id = 0; id < EFFECTSGEN_TYPES_MAX; id++)
    {
        {
            snprintf(key, sizeof(key), "effectGenerator%d", id);
            section = value_dict_get(value, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct EffectGeneratorConfigStats *effgencst = &game.conf.effects_conf.effectgen_cfgstats[id];

            SET_NAME(section,effectgen_desc,effgencst->code_name);

            CONDITIONAL_ASSIGN_INT(section,"GenerationDelayMin",effgencst->generation_delay_min);
            CONDITIONAL_ASSIGN_INT(section,"GenerationDelayMax",effgencst->generation_delay_max);
            CONDITIONAL_ASSIGN_INT(section,"GenerationAmount"  ,effgencst->generation_amount);

            CONDITIONAL_ASSIGN_EFFECT_OR_EL_MODEL(section,"EffectModel",effgencst->effect_model);
            CONDITIONAL_ASSIGN_INT(section,"IgnoreTerrain",effgencst->ignore_terrain);
            CONDITIONAL_ASSIGN_INT(section,"SpawnHeight",effgencst->spawn_height);

            CONDITIONAL_ASSIGN_ARR3_INT(section,"AccelerationMin",effgencst->acc_x_min,effgencst->acc_y_min,effgencst->acc_z_min);
            CONDITIONAL_ASSIGN_ARR3_INT(section,"AccelerationMax",effgencst->acc_x_max,effgencst->acc_y_max,effgencst->acc_z_max);
            CONDITIONAL_ASSIGN_ARR2_INT(section,"Sound",effgencst->sound_sample_idx,effgencst->sound_sample_rng);
        }
    }
}

static void load_effectelements(VALUE *value, unsigned short flags)
{
    char key[KEY_SIZE];
    VALUE *section;
    for (int id = 0; id < EFFECTSELLEMENTS_TYPES_MAX; id++)
    {
        {
            snprintf(key, sizeof(key), "effectElement%d", id);
            section = value_dict_get(value, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct EffectElementConfigStats *effelcst = &game.conf.effects_conf.effectelement_cfgstats[id];

            SET_NAME(section,effectelem_desc,effelcst->code_name);

            CONDITIONAL_ASSIGN_INT(section,"DrawClass", effelcst->draw_class);
            CONDITIONAL_ASSIGN_INT(section,"MoveType",  effelcst->move_type);
            CONDITIONAL_ASSIGN_INT(section,"Unanimated",effelcst->unanimated);
            CONDITIONAL_ASSIGN_ARR2_INT(section,"Lifespan",effelcst->lifespan,effelcst->lifespan_random);
            CONDITIONAL_ASSIGN_ANIMID(section,"AnimationId",effelcst->sprite_idx);
            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"SpriteSize",effelcst->sprite_size_min,effelcst->sprite_size_max);
            CONDITIONAL_ASSIGN_INT(section,"RenderFlags",effelcst->animate_once); //todo Remove after people have had time to handle the rename
            CONDITIONAL_ASSIGN_INT(section, "AnimateOnce", effelcst->animate_once);
            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"SpriteSpeed",effelcst->sprite_speed_min,effelcst->sprite_speed_max);

            CONDITIONAL_ASSIGN_BOOL(section,"AnimateOnFloor",  effelcst->animate_on_floor);
            CONDITIONAL_ASSIGN_BOOL(section,"Unshaded",        effelcst->unshaded);
            CONDITIONAL_ASSIGN_INT(section,"Transparent",      effelcst->transparent);
            CONDITIONAL_ASSIGN_INT(section,"MovementFlags",    effelcst->through_walls); //todo Remove after people have had time to handle the rename
            CONDITIONAL_ASSIGN_INT(section,"ThroughWalls",     effelcst->through_walls);
            CONDITIONAL_ASSIGN_INT(section,"SizeChange",       effelcst->size_change);
            CONDITIONAL_ASSIGN_INT(section,"FallAcceleration", effelcst->fall_acceleration);
            CONDITIONAL_ASSIGN_INT(section,"InertiaFloor",     effelcst->inertia_floor);
            CONDITIONAL_ASSIGN_INT(section,"InertiaAir",       effelcst->inertia_air);
            CONDITIONAL_ASSIGN_INT(section,"SubeffectModel",   effelcst->subeffect_model);
            CONDITIONAL_ASSIGN_INT(section,"SubeffectDelay",   effelcst->subeffect_delay);
            CONDITIONAL_ASSIGN_BOOL(section,"Movable",         effelcst->movable);
            CONDITIONAL_ASSIGN_BOOL(section,"Impacts",         effelcst->impacts);
            if(effelcst->impacts)
            {
                CONDITIONAL_ASSIGN_INT(section,"SolidGroundEffmodel", effelcst->solidgnd_effmodel);
                CONDITIONAL_ASSIGN_INT(section,"SolidGroundSoundId", effelcst->solidgnd_snd_smpid);
                CONDITIONAL_ASSIGN_INT(section,"SolidGroundLoudness", effelcst->solidgnd_loudness);
                CONDITIONAL_ASSIGN_BOOL(section,"SolidGroundDestroyOnImpact", effelcst->solidgnd_destroy_on_impact);
                CONDITIONAL_ASSIGN_INT(section,"WaterEffmodel", effelcst->water_effmodel);
                CONDITIONAL_ASSIGN_INT(section,"WaterSoundId", effelcst->water_snd_smpid);
                CONDITIONAL_ASSIGN_INT(section,"WaterLoudness", effelcst->water_loudness);
                CONDITIONAL_ASSIGN_BOOL(section,"WaterDestroyOnImpact", effelcst->water_destroy_on_impact);
                CONDITIONAL_ASSIGN_INT(section,"LavaEffmodel", effelcst->lava_effmodel);
                CONDITIONAL_ASSIGN_INT(section,"LavaSoundId", effelcst->lava_snd_smpid);
                CONDITIONAL_ASSIGN_INT(section,"LavaLoudness", effelcst->lava_loudness);
                CONDITIONAL_ASSIGN_BOOL(section,"LavaDestroyOnImpact", effelcst->lava_destroy_on_impact);
            }
            CONDITIONAL_ASSIGN_INT(section,"TransformModel", effelcst->transform_model  );
            CONDITIONAL_ASSIGN_INT_SCALED(section,"LightRadius",    effelcst->light_radius, COORD_PER_STL);
            CONDITIONAL_ASSIGN_INT(section,"LightIntensity", effelcst->light_intensity  );
            CONDITIONAL_ASSIGN_INT(section,"LightFlags",     effelcst->light_flags   );
            CONDITIONAL_ASSIGN_INT(section,"AffectedByWind", effelcst->affected_by_wind );
        }
    }
}

static TbBool load_effects_config_file(const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(fname,&file_root,flags))
        return false;
    load_effects(&file_root,flags);
    load_effectsgenerators(&file_root,flags);
    load_effectelements(&file_root,flags);

    value_fini(&file_root);

    return true;
}

/**
 * Returns Code Name (name to use in script file) of given effect element model.
 */
const char* effect_element_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(effectelem_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
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
        return &game.conf.effects_conf.effectgen_cfgstats[0];
    return &game.conf.effects_conf.effectgen_cfgstats[tngmodel];
}

struct EffectConfigStats *get_effect_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= EFFECTS_TYPES_MAX)
        return &game.conf.effects_conf.effect_cfgstats[0];
    return &game.conf.effects_conf.effect_cfgstats[tngmodel];
}

short effect_or_effect_element_id(const char *code_name)
{
    if (code_name == NULL)
    {
        return 0;
    }
    if (parameter_is_number(code_name))
    {
        return atoi(code_name);
    }
    short id = get_id(effect_desc, code_name);
    if (id > 0)
    {
        return id;
    }
    id = get_id(effectelem_desc, code_name);
    if (id > 0)
    {
        return -id;
    }
    return 0;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
