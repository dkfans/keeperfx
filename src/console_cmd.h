/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file console_cmd.h
 *     Header file for console_cmd.c.
 * @par Purpose:
 *     Console commands
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Sim
 * @date     07 Jul 2020 - 07 Jul 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CONSOLECMD_H
#define DK_CONSOLECMD_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

TbBool cmd_exec(PlayerNumber plyr_idx, char *msg);
long get_creature_model_for_command(char *msg);
PlayerNumber get_player_number_for_command(char *msg);
TbBool parameter_is_number(const char* parstr);
char get_trap_number_for_command(char* msg);
char get_door_number_for_command(char* msg);

#ifdef __cplusplus
}
#endif

#endif
