/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_trapdoor.h
 *     Header file for config_trapdoor.c.
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
#ifndef DK_CFGTEXTURES_H
#define DK_CFGTEXTURES_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"
#include "engine_textures.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern const struct ConfigFileData keeper_textureanim_file_data;
extern const struct ConfigFileData keeper_tileset_file_data;
extern struct NamedCommand texture_pack_desc[];
extern ThingModel tileset_footprint[TEXTURE_VARIATIONS_COUNT];
extern unsigned short tileset_footstep_snd[TEXTURE_VARIATIONS_COUNT];
extern int tileset_footstep_cnt[TEXTURE_VARIATIONS_COUNT];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
