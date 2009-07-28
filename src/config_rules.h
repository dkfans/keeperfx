/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_rules.h
 *     Header file for config_rules.c.
 * @par Purpose:
 *     Various game configuration options support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 31 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGRULES_H
#define DK_CFGRULES_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
extern const char keeper_rules_file[];
extern const struct NamedCommand research_desc[];
/******************************************************************************/
long get_research_id(long item_type, char *trg_name, const char *func_name);
TbBool load_rules_config(const char *conf_fname,unsigned short flags);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
