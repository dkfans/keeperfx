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

#define EFFECTS_TYPES_MAX 128
#define EFFECTSGEN_TYPES_MAX 64

/******************************************************************************/


struct EffectConfigStats {
    char code_name[COMMAND_WORD_LEN];
    /** Health; decreases by 1 on every turn, so it works also as lifespan. */
    short start_health;
    unsigned char generation_type;
    short accel_xy_min;
    short accel_xy_max;
    short accel_z_min;
    short accel_z_max;
    unsigned char elements_count;
    short effect_sound;
    unsigned char kind_min;
    unsigned char kind_max;
    unsigned char area_affect_type;
    unsigned char always_generate;
    struct InitLight ilght;
    unsigned char affected_by_wind;
};

struct EffectGeneratorConfigStats {
    char code_name[COMMAND_WORD_LEN];
    long genation_delay_min;
    long genation_delay_max;
    long genation_amount;
    long effect_element_model;
    unsigned char ignore_terrain;
    long spawn_height;
    long acc_x_min;
    long acc_x_max;
    long acc_y_min;
    long acc_y_max;
    long acc_z_min;
    long acc_z_max;
    long sound_sample_idx;
    long sound_sample_rng;
};

struct EffectsConfig {
    long effect_types_count;
    struct EffectConfigStats effect_cfgstats[EFFECTS_TYPES_MAX];
    struct EffectGeneratorConfigStats effectgen_cfgstats[EFFECTSGEN_TYPES_MAX];
};
/******************************************************************************/
extern const char keeper_effects_file[];
extern struct NamedCommand effect_desc[EFFECTS_TYPES_MAX];
extern long const imp_spangle_effects[];
extern long const ball_puff_effects[];
/******************************************************************************/
TbBool load_effects_config(const char *conf_fname,unsigned short flags);
struct EffectConfigStats *get_effect_model_stats(ThingModel tngmodel);
struct EffectGeneratorConfigStats *get_effectgenerator_model_stats(ThingModel tngmodel);
const char *effect_code_name(ThingModel tngmodel);
ThingModel effect_model_id(const char * code_name);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
