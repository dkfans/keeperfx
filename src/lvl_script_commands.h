/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_commands.h
 *     Header file for lvl_script_commands.c.
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
#ifndef DK_LVLSCRIPTPARSER_H
#define DK_LVLSCRIPTPARSER_H

#ifdef __cplusplus
extern "C" {
#endif


#include "lvl_script_lib.h"

extern const struct CommandDesc command_desc[];
extern const struct CommandDesc dk1_command_desc[];
extern const struct CommandDesc subfunction_desc[];
extern const struct NamedCommand newcrtr_desc[];
extern const struct NamedCommand player_desc[];
//extern const struct NamedCommand variable_desc[];
extern const struct NamedCommand controls_variable_desc[];
extern const struct NamedCommand comparison_desc[];
extern const struct NamedCommand timer_desc[];
extern const struct NamedCommand flag_desc[];
extern const struct NamedCommand hero_objective_desc[];
extern const struct NamedCommand msgtype_desc[];
extern const struct NamedCommand tendency_desc[];
extern const struct NamedCommand creature_select_criteria_desc[];
extern const struct NamedCommand door_config_desc[];
extern const struct NamedCommand trap_config_desc[];
extern const struct NamedCommand gui_button_group_desc[];
extern const struct NamedCommand campaign_flag_desc[];
extern const struct NamedCommand script_operator_desc[];

#ifdef __cplusplus
}
#endif
#endif