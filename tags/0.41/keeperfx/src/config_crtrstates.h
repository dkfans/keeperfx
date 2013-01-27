/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_crtrstates.h
 *     Header file for config_crtrstates.c.
 * @par Purpose:
 *     Support of configuration files for specific creatures.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     25 May 2009 - 23 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGCRSTAT_H
#define DK_CFGCRSTAT_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern const char creature_states_file[];
/******************************************************************************/
TbBool load_creaturestates_config(const char *conf_fname,unsigned short flags);
const char *creature_state_code_name(long crstate);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
