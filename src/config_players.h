/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_players.h
 *     Header file for config_players.c.
 * @par Purpose:
 *     Players configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     17 Sep 2012 - 06 Mar 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGPLAYERS_H
#define DK_CFGPLAYERS_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)


#pragma pack()
/******************************************************************************/
extern const struct NamedCommand player_state_commands[];
/******************************************************************************/
const char *player_state_code_name(int wrkstate);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
