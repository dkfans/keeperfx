/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_crtrmodel.h
 *     Header file for config_crtrmodel.c.
 * @par Purpose:
 *     Support of configuration files for specific creatures.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 04 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGCRMODL_H
#define DK_CFGCRMODL_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
TbBool load_creaturemodel_config(long crmodel,unsigned short flags);

TbBool make_all_creatures_free(void);
TbBool change_max_health_of_creature_kind(ThingModel crmodel, long new_max);
extern const struct NamedCommand creatmodel_properties_commands[];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
