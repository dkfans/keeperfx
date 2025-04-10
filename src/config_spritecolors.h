/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_spritecolors.h
 *     Header file for config_spritecolors.c.
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
#ifndef DK_CFGSPRITECOLORS_H
#define DK_CFGSPRITECOLORS_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern const struct ConfigFileData keeper_spritecolors_file_data;

short get_player_colored_icon_idx(short base_icon_idx,PlayerNumber plyr_idx);
short get_player_colored_pointer_icon_idx(short base_icon_idx,PlayerNumber plyr_idx);
short get_player_colored_button_sprite_idx(short base_icon_idx,PlayerNumber plyr_idx);
ThingModel get_player_colored_object_model(ThingModel base_model_idx,PlayerNumber plyr_idx);
ThingModel get_coloured_object_base_model(ThingModel model_idx);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
