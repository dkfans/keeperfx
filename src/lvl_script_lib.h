/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_lib.h
 *     Header file for lvl_script_lib.c.
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
#ifndef DK_LVLSCRIPTLIB_H
#define DK_LVLSCRIPTLIB_H

#include "creature_states_hero.h"
#include "frontmenu_ingame_tabs.h"

#include "globals.h"
#include "bflib_math.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "lvl_script_statehandler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMMANDDESC_ARGS_COUNT    8

enum TbScriptCommands {
    Cmd_NONE                              =  0,
    Cmd_CREATE_PARTY                      =  1,
    Cmd_ADD_TO_PARTY                      =  2,
    Cmd_ADD_PARTY_TO_LEVEL                =  3,
    Cmd_ADD_CREATURE_TO_LEVEL             =  4,
    Cmd_MESSAGE                           =  5, // from beta
    Cmd_IF                                =  6,
    Cmd_ENDIF                             =  7,
    Cmd_SET_HATE                          =  8,
    Cmd_SET_GENERATE_SPEED                =  9,
    Cmd_REM                               = 10,
    Cmd_START_MONEY                       = 11,
    Cmd_ROOM_AVAILABLE                    = 12,
    Cmd_CREATURE_AVAILABLE                = 13,
    Cmd_MAGIC_AVAILABLE                   = 14,
    Cmd_TRAP_AVAILABLE                    = 15,
    Cmd_RESEARCH                          = 16,
    Cmd_COMPUTER_PLAYER                   = 17,
    Cmd_SET_TIMER                         = 18,
    Cmd_IF_ACTION_POINT                   = 19,
    Cmd_ADD_TUNNELLER_TO_LEVEL            = 20,
    Cmd_WIN_GAME                          = 21,
    Cmd_LOSE_GAME                         = 22,
    Cmd_SET_FLAG                          = 25,
    Cmd_MAX_CREATURES                     = 26,
    Cmd_NEXT_COMMAND_REUSABLE             = 27,
    Cmd_RANDOM                            = 28,
    Cmd_DRAWFROM                          = 29,
    Cmd_DOOR_AVAILABLE                    = 30,
    Cmd_DISPLAY_OBJECTIVE                 = 37,
    Cmd_DISPLAY_INFORMATION               = 38,
    Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL      = 40,
    Cmd_ADD_CREATURE_TO_POOL              = 41,
    Cmd_RESET_ACTION_POINT                = 42,
    Cmd_SET_CREATURE_MAX_LEVEL            = 59,
    Cmd_SET_MUSIC                         = 60,
    Cmd_TUTORIAL_FLASH_BUTTON             = 54,
    Cmd_SET_CREATURE_STRENGTH             = 62,
    Cmd_SET_CREATURE_HEALTH               = 61,
    Cmd_SET_CREATURE_ARMOUR               = 63,
    Cmd_SET_CREATURE_FEAR_WOUNDED         = 64,
    Cmd_IF_AVAILABLE                      = 66,
    Cmd_SET_COMPUTER_GLOBALS              = 68,
    Cmd_SET_COMPUTER_CHECKS               = 69,
    Cmd_SET_COMPUTER_EVENT                = 70,
    Cmd_SET_COMPUTER_PROCESS              = 71,
    Cmd_ALLY_PLAYERS                      = 72,
    Cmd_DEAD_CREATURES_RETURN_TO_POOL     = 73,
    Cmd_BONUS_LEVEL_TIME                  = 75,
    Cmd_QUICK_OBJECTIVE                   = 44,
    Cmd_QUICK_INFORMATION                 = 45,
    Cmd_QUICK_OBJECTIVE_WITH_POS          = 46,
    Cmd_QUICK_INFORMATION_WITH_POS        = 47,
    Cmd_DISPLAY_OBJECTIVE_WITH_POS        = 65,
    Cmd_DISPLAY_INFORMATION_WITH_POS      = 74,
    Cmd_PRINT                             = 76, // from beta
    Cmd_SWAP_CREATURE                     = 77,
  // New commands propositions - KeeperFX only
    Cmd_CHANGE_SLAB_TYPE                  = 78,
    Cmd_CHANGE_SLAB_OWNER                 = 79,
    Cmd_IF_SLAB_TYPE                      = 80,
    Cmd_IF_SLAB_OWNER                     = 81,
    Cmd_SET_CREATURE_TENDENCIES           = 84,
    Cmd_PLAY_MESSAGE                      = 85,
    Cmd_ADD_GOLD_TO_PLAYER                = 86,
    Cmd_REVEAL_MAP_RECT                   = 87,
    Cmd_REVEAL_MAP_LOCATION               = 88,
    Cmd_LEVEL_VERSION                     = 90,
    Cmd_RESEARCH_ORDER                    = 91,
    Cmd_KILL_CREATURE                     = 92,
    Cmd_SET_CREATURE_FEAR_STRONGER        = 93,
    Cmd_IF_CONTROLS                       = 94,
    Cmd_ADD_TO_FLAG                       = 95,
    Cmd_SET_CAMPAIGN_FLAG                 = 96,
    Cmd_ADD_TO_CAMPAIGN_FLAG              = 97,
    Cmd_EXPORT_VARIABLE                   = 98,
    Cmd_IMPORT                            = 99,
    Cmd_RUN_AFTER_VICTORY                 = 100,
    Cmd_LEVEL_UP_CREATURE                 = 101,
    Cmd_CHANGE_CREATURE_OWNER             = 102,
    Cmd_SET_GAME_RULE                     = 103,
    Cmd_SET_TRAP_CONFIGURATION            = 104,
    Cmd_SET_DOOR_CONFIGURATION            = 105,
    Cmd_SET_OBJECT_CONFIGURATION          = 106,
    Cmd_SET_CREATURE_CONFIGURATION        = 107,
    Cmd_SET_CREATURE_PROPERTY             = 108,
    Cmd_SET_CREATURE_FEARSOME_FACTOR      = 109,
    Cmd_USE_POWER_ON_CREATURE             = 110,
    Cmd_USE_POWER_AT_POS                  = 111,
    Cmd_USE_POWER                         = 112,
    Cmd_USE_POWER_AT_LOCATION             = 113,
    Cmd_ADD_OBJECT_TO_LEVEL               = 114,
    Cmd_USE_SPECIAL_INCREASE_LEVEL        = 115,
    Cmd_USE_SPECIAL_MULTIPLY_CREATURES    = 116,
    Cmd_USE_SPECIAL_MAKE_SAFE             = 117,
    Cmd_USE_SPECIAL_LOCATE_HIDDEN_WORLD   = 118,
    Cmd_CHANGE_CREATURES_ANNOYANCE        = 119,
    Cmd_COMPUTER_DIG_TO_LOCATION          = 120,
    Cmd_DELETE_FROM_PARTY                 = 121,
    Cmd_SET_SACRIFICE_RECIPE              = 122,
    Cmd_REMOVE_SACRIFICE_RECIPE           = 123,
    Cmd_SET_BOX_TOOLTIP                   = 124,
    Cmd_SET_BOX_TOOLTIP_ID                = 125,
    Cmd_CREATE_EFFECTS_LINE               = 126,
    Cmd_DISPLAY_MESSAGE                   = 127,
    Cmd_QUICK_MESSAGE                     = 128,
    Cmd_USE_SPELL_ON_CREATURE             = 129,
    Cmd_SET_HEART_HEALTH                  = 130,
    Cmd_ADD_HEART_HEALTH                  = 131,
    Cmd_CREATURE_ENTRANCE_LEVEL           = 132,
    Cmd_RANDOMISE_FLAG                    = 133,
    Cmd_COMPUTE_FLAG                      = 134,
    Cmd_CONCEAL_MAP_RECT                  = 135,
    Cmd_DISPLAY_TIMER                     = 136,
    Cmd_ADD_TO_TIMER                      = 137,
    Cmd_ADD_BONUS_TIME                    = 138,
    Cmd_DISPLAY_VARIABLE                  = 139,
    Cmd_DISPLAY_COUNTDOWN                 = 140,
    Cmd_HIDE_TIMER                        = 141,
    Cmd_HIDE_VARIABLE                     = 142,
    Cmd_CREATE_EFFECT                     = 143,
    Cmd_CREATE_EFFECT_AT_POS              = 144,
    Cmd_HEART_LOST_QUICK_OBJECTIVE        = 145,
    Cmd_HEART_LOST_OBJECTIVE              = 146,
};

struct ScriptLine {
  enum TbScriptCommands command;
  long np[COMMANDDESC_ARGS_COUNT]; /**< Numeric parameters (to be changed into interpreted parameters, containing ie. in-game random) */
  char tcmnd[MAX_TEXT_LENGTH]; /**< Command text */
  char tp[COMMANDDESC_ARGS_COUNT][MAX_TEXT_LENGTH]; /**< Text parameters */
};
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
//extern const struct NamedCommand variable_desc[];
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
TbBool parse_get_varib(const char *varib_name, long *varib_id, long *varib_type);
char get_player_number_from_value(const char* txt);

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