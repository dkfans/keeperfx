/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_lib.h
 *     Header file for lvl_script_lib.c.
 * @par Purpose:
 *     Lcollection of functions used by multiple files under lvl_script_*
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.

 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LVLSCRIPTLIB_H
#define DK_LVLSCRIPTLIB_H

#include "creature_states_hero.h"
#include "frontmenu_ingame_tabs.h"

#include "globals.h"
#include "bflib_math.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "lvl_script.h"
#include "lvl_script_statehandler.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CommandDesc { // sizeof = 14 // originally was 13
  const char *textptr;
  char args[COMMANDDESC_ARGS_COUNT+1]; // originally was [8]
  unsigned char index;
  void (*check_fn)(const struct ScriptLine *scline); // should check
  void (*process_fn)(struct ScriptContext *context); // called from value or from
};

extern const struct CommandDesc command_desc[];

extern const struct CommandDesc subfunction_desc[];
extern const struct NamedCommand newcrtr_desc[];
extern const struct NamedCommand player_desc[];
extern const struct NamedCommand variable_desc[];
extern const struct NamedCommand dk1_variable_desc[];
extern const struct NamedCommand controls_variable_desc[];
extern const struct NamedCommand comparison_desc[];
extern const struct NamedCommand head_for_desc[];
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



long parse_creature_name(const char *creature_name);
#define get_map_location_id(locname, location) get_map_location_id_f(locname, location, __func__, text_line_number)
TbBool get_map_location_id_f(const char *locname, TbMapLocation *location, const char *func_name, long ln_num);
struct ScriptValue *allocate_script_value(void);
struct Thing *script_process_new_object(long crmodel, TbMapLocation location, long arg);
TbBool get_coords_at_action_point(struct Coord3d *pos, long apt_idx, unsigned char random_factor);
TbBool get_coords_at_hero_door(struct Coord3d *pos, long gate_num, unsigned char random_factor);
TbBool get_coords_at_dungeon_heart(struct Coord3d *pos, PlayerNumber plyr_idx);
void command_init_value(struct ScriptValue* value, unsigned long var_index, unsigned long plr_range_id);
void command_add_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4);
#define get_map_heading_id(headname, target, location) get_map_heading_id_f(headname, target, location, __func__, text_line_number)
TbBool get_map_heading_id_f(const char *headname, long target, TbMapLocation *location, const char *func_name, long ln_num);
#define get_players_range(plr_range_id, plr_start, plr_end) get_players_range_f(plr_range_id, plr_start, plr_end, __func__, text_line_number)
long get_players_range_f(long plr_range_id, int *plr_start, int *plr_end, const char *func_name, long ln_num);
TbBool parse_set_varib(const char *varib_name, long *varib_id, long *varib_type);
long parse_criteria(const char *criteria);
#define get_players_range_single(plr_range_id) get_players_range_single_f(plr_range_id, __func__, text_line_number)
long get_players_range_single_f(long plr_range_id, const char *func_name, long ln_num);

#define ALLOCATE_SCRIPT_VALUE(var_index, plr_range_id) \
    struct ScriptValue tmp_value = {0}; \
    struct ScriptValue* value; \
    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0)) \
    { \
    /* Fill local structure */ \
        value = &tmp_value; \
    } \
    else \
    { \
        value = allocate_script_value(); \
        if (value == NULL) \
        { \
            SCRPTERRLOG("Too many VALUEs in script (limit is %d)", SCRIPT_VALUES_COUNT); \
            return; \
        } \
    } \
    command_init_value(value, var_index, plr_range_id);


#ifdef __cplusplus
}
#endif
#endif