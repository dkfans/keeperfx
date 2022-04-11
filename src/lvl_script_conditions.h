/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_conditions.h
 *     Header file for lvl_script_conditions.c.
 * @par Purpose:
 *     should only be used by files under lvl_script_*
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_LVLSCRIPTCOND_H
#define DK_LVLSCRIPTCOND_H


#include "globals.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif



extern const struct NamedCommand variable_desc[];
extern const struct NamedCommand dk1_variable_desc[];


long get_condition_value(PlayerNumber plyr_idx, unsigned char valtype, unsigned char a3);
void process_conditions(void);
long pop_condition(void);

int get_script_current_condition();
void set_script_current_condition(int current_condition);

void command_add_condition(long plr_range_id, long opertr_id, long varib_type, long varib_id, long value);

#ifdef __cplusplus
}
#endif
#endif