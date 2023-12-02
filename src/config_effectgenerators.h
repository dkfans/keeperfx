/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_effectgenerators.h
 *     Header file for config_effectgenerators.c.
 * @par Purpose:
 *     texture animation configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGEFFECTGEN_H
#define DK_CFGEFFECTGEN_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern const char keeper_effectgenerator_file[];

struct EffectGeneratorConfigStats {
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

TbBool load_effectgenerator_config(const char *conf_fname,unsigned short flags);

struct EffectGeneratorConfigStats *get_effectgenerator_model_stats(ThingModel tngmodel);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
