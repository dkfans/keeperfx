/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_lenses.h
 *     Header file for config_lenses.c.
 * @par Purpose:
 *     Support of configuration files for eye lenses.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGLENS_H
#define DK_CFGLENS_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_video.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define LENS_ITEMS_MAX 256

enum LensConfigFlags {
    LCF_HasMist     = 0x01,
    LCF_HasDisplace = 0x02,
    LCF_HasPalette  = 0x04,
    LCF_HasOverlay  = 0x08,
};

struct LensConfig {
    char code_name[COMMAND_WORD_LEN];
    unsigned char flags;
    TbPixel palette[PALETTE_SIZE];
    short mist_lightness;
    short mist_ghost;
    char mist_file[DISKPATH_SIZE];
    short displace_kind;
    short displace_magnitude;
    short displace_period;
    char overlay_file[DISKPATH_SIZE];
    short overlay_alpha;
};

struct LensesConfig {
    int32_t lenses_count;
    struct LensConfig lenses[LENS_ITEMS_MAX];
};
/******************************************************************************/
extern const struct ConfigFileData keeper_lenses_file_data;
extern struct LensesConfig lenses_conf;
extern struct NamedCommand lenses_desc[LENS_ITEMS_MAX];
/******************************************************************************/
struct LensConfig *get_lens_config(long lens_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
