/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_filesdk1.h
 *     Header file for lvl_filesdk1.c.
 * @par Purpose:
 *     Level files reading routines fore standard DK1 levels.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_LVL_FILESDK1_H
#define DK_LVL_FILESDK1_H

#include "bflib_basics.h"
#include "globals.h"
#include "config_campaigns.h"
#include "config_strings.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MAX_LIF_SIZE 65535
#define DEFAULT_LEVEL_VERSION 0

enum LoadMapFileFlags {
    LMFF_None     = 0x00,
    LMFF_Optional = 0x01,
};
/******************************************************************************/
extern long level_file_version;
extern char *level_strings[];
/******************************************************************************/
unsigned char *load_single_map_file_to_buffer(LevelNumber lvnum,const char *fext,long *ldsize,unsigned short flags);
TbBool find_and_load_lif_files(void);
TbBool find_and_load_lof_files(void);
long convert_old_column_file(LevelNumber lv_num);

TbBool load_map_file(LevelNumber lvnum);

void load_map_string_data(struct GameCampaign *campgn, LevelNumber lvnum, short fgroup);
void free_level_strings_data();
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
