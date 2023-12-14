/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_trapdoor.h
 *     Header file for config_trapdoor.c.
 * @par Purpose:
 *     Traps and doors configuration loading functions.
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
#ifndef DK_CFGSLABS_H
#define DK_CFGSLABS_H

#include "globals.h"
#include "bflib_basics.h"
#include "map_columns.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern const char keeper_slabset_file[];
extern const char keeper_columns_file[];


TbBool load_slabset_config(const char *conf_fname,unsigned short flags);
TbBool load_columns_config(const char *conf_fname,unsigned short flags,struct Column *cols,long *ccount);

void clear_slabsets(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
