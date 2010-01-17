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

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define LENSE_ITEMS_MAX 32

struct LensesConfig {
    long lense_types_count;
    struct CommandWord lense_names[LENSE_ITEMS_MAX];
};
/******************************************************************************/
extern const char keeper_lenses_file[];
extern struct NamedCommand lenses_desc[LENSE_ITEMS_MAX];
/******************************************************************************/
TbBool load_lenses_config(const char *conf_fname,unsigned short flags);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
