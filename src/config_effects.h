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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define EFFECTS_TYPES_MAX 128

/******************************************************************************/
struct EffectConfigStats {
    char code_name[COMMAND_WORD_LEN];
    struct InitEffect *old;
};

struct EffectsConfig {
    long effect_types_count;
    struct EffectConfigStats effect_cfgstats[EFFECTS_TYPES_MAX];
};
/******************************************************************************/
DLLIMPORT long _DK_imp_spangle_effects[];
/******************************************************************************/
extern const char keeper_effects_file[];
extern struct NamedCommand effect_desc[EFFECTS_TYPES_MAX];
extern long const imp_spangle_effects[];
extern struct EffectsConfig effects_conf;
/******************************************************************************/
TbBool load_effects_config(const char *conf_fname,unsigned short flags);
struct EffectConfigStats *get_effect_model_stats(int tngmodel);
const char *effect_code_name(int tngmodel);
int effect_model_id(const char * code_name);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
