/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_effects.h
 *     Header file for config_effects.c.
 * @par Purpose:
 *     Effects configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 11 Mar 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGEFFECTS_H
#define DK_CFGEFFECTS_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"
#include "light_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define EFFECTS_TYPES_MAX 2048
#define EFFECTSGEN_TYPES_MAX 2048
#define EFFECTSELLEMENTS_TYPES_MAX 2048

/******************************************************************************/

extern const struct NamedFieldSet effects_effectgenerator_named_fields_set;

struct EffectConfigStats {
    char code_name[COMMAND_WORD_LEN];
    /** Health; decreases by 1 on every turn, so it works also as lifespan. */
    HitPoints start_health;
    unsigned char generation_type;
    short accel_xy_min;
    short accel_xy_max;
    short accel_z_min;
    short accel_z_max;
    unsigned char elements_count;
    short effect_sound;
    ThingModel kind_min;
    ThingModel kind_max;
    unsigned char area_affect_type;
    unsigned char always_generate;
    struct InitLight ilght;
    unsigned char affected_by_wind;
    ThingHitType effect_hit_type;
    SpellKind spell_effect;
};

struct EffectGeneratorConfigStats {
    char code_name[COMMAND_WORD_LEN];
    int32_t generation_delay_min;
    int32_t generation_delay_max;
    int32_t generation_amount;
    ThingModel effect_model;
    unsigned char ignore_terrain;
    int32_t spawn_height;
    int32_t acc_x_min;
    int32_t acc_x_max;
    int32_t acc_y_min;
    int32_t acc_y_max;
    int32_t acc_z_min;
    int32_t acc_z_max;
    int32_t sound_sample_idx;
    int32_t sound_sample_rng;
};

struct EffectElementConfigStats {
    char code_name[COMMAND_WORD_LEN];
    unsigned char draw_class; /**< See enum ObjectsDrawClasses. */
    unsigned char move_type;
    unsigned char unanimated;
    short lifespan;
    short lifespan_random;
    short sprite_idx;
    short sprite_size_min;
    short sprite_size_max;
    unsigned char animate_once;
    unsigned short sprite_speed_min;
    unsigned short sprite_speed_max;
    TbBool animate_on_floor;
    TbBool unshaded;
    unsigned char transparent;  // transparency flags in bits 4-5
    TbBool movable;
    unsigned char through_walls;
    unsigned char size_change; /**< See enum ThingSizeChange. */
    char fall_acceleration;
    short inertia_floor;
    short inertia_air;
    unsigned short subeffect_model;
    unsigned short subeffect_delay;
    TbBool impacts;
    ThingModel solidgnd_effmodel;
    unsigned short solidgnd_snd_smpid;
    unsigned short solidgnd_loudness;
    TbBool solidgnd_destroy_on_impact;
    ThingModel water_effmodel;
    unsigned short water_snd_smpid;
    unsigned short water_loudness;
    TbBool water_destroy_on_impact;
    ThingModel lava_effmodel;
    unsigned short lava_snd_smpid;
    unsigned short lava_loudness;
    TbBool lava_destroy_on_impact;
    unsigned short transform_model;
    unsigned short light_radius;
    unsigned char light_intensity;
    int32_t light_flags;
    unsigned char affected_by_wind;
};

struct EffectsConfig {
    struct EffectConfigStats effect_cfgstats[EFFECTS_TYPES_MAX];
    int32_t effectgen_cfgstats_count;
    struct EffectGeneratorConfigStats effectgen_cfgstats[EFFECTSGEN_TYPES_MAX];
    struct EffectElementConfigStats effectelement_cfgstats[EFFECTSELLEMENTS_TYPES_MAX];
};
/******************************************************************************/
extern const struct ConfigFileData keeper_effects_file_data;
extern struct NamedCommand effect_desc[EFFECTS_TYPES_MAX];
extern int32_t const imp_spangle_effects[];
extern int32_t const ball_puff_effects[];

extern struct NamedCommand effectgen_desc[EFFECTSGEN_TYPES_MAX];
extern struct NamedCommand effectelem_desc[EFFECTSELLEMENTS_TYPES_MAX];
/******************************************************************************/
struct EffectConfigStats *get_effect_model_stats(ThingModel tngmodel);
struct EffectGeneratorConfigStats *get_effectgenerator_model_stats(ThingModel tngmodel);
const char *effect_code_name(ThingModel tngmodel);
const char* effect_element_code_name(ThingModel tngmodel);
const char *effectgenerator_code_name(ThingModel tngmodel);
short effect_or_effect_element_id(const char * code_name);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
