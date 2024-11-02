/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_powerhands.h
 *     Header file for config_powerhands.c.
 * @par Purpose:
 *     powerhand visuals configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 21 Dec 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGPOWERHANDS_H
#define DK_CFGPOWERHANDS_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern const char keeper_powerhands_file[];

TbBool load_powerhands_config(const char *conf_fname,unsigned short flags);

#define NUM_VARIANTS 16
#define NUM_ANIMS_PER_VARIANT 7

enum HandAnims {
    HndA_Hold = 0,
    HndA_HoldGold,
    HndA_Hover,
    HndA_Pickup,
    HndA_SideHover,
    HndA_SideSlap,
    HndA_Slap,
};

struct PowerHandConfigStats {
    char  code_name[COMMAND_WORD_LEN];
    short anim_idx[NUM_ANIMS_PER_VARIANT];
    short anim_speed[NUM_ANIMS_PER_VARIANT];
};

struct PowerHandConfig {
    struct PowerHandConfigStats pwrhnd_cfg_stats[NUM_VARIANTS];
};

extern struct NamedCommand powerhand_desc[NUM_VARIANTS + 1];

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
