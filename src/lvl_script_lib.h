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

#ifdef __cplusplus
extern "C" {
#endif

#define COMMANDDESC_ARGS_COUNT    8

enum TbScriptCommands {
    Cmd_NONE                               =  0,
    Cmd_CREATE_PARTY                       =  1,
    Cmd_ADD_TO_PARTY                       =  2,
    Cmd_ADD_PARTY_TO_LEVEL                 =  3,
    Cmd_ADD_CREATURE_TO_LEVEL              =  4,
    Cmd_MESSAGE                            =  5, // from beta
    Cmd_IF                                 =  6,
    Cmd_ENDIF                              =  7,
    Cmd_SET_GENERATE_SPEED                 =  9,
    Cmd_REM                                = 10,
    Cmd_START_MONEY                        = 11,
    Cmd_ROOM_AVAILABLE                     = 12,
    Cmd_CREATURE_AVAILABLE                 = 13,
    Cmd_MAGIC_AVAILABLE                    = 14,
    Cmd_TRAP_AVAILABLE                     = 15,
    Cmd_RESEARCH                           = 16,
    Cmd_COMPUTER_PLAYER                    = 17,
    Cmd_SET_TIMER                          = 18,
    Cmd_IF_ACTION_POINT                    = 19,
    Cmd_ADD_TUNNELLER_TO_LEVEL             = 20,
    Cmd_WIN_GAME                           = 21,
    Cmd_LOSE_GAME                          = 22,
    Cmd_SET_FLAG                           = 25,
    Cmd_MAX_CREATURES                      = 26,
    Cmd_NEXT_COMMAND_REUSABLE              = 27,
    Cmd_RANDOM                             = 28,
    Cmd_DRAWFROM                           = 29,
    Cmd_DOOR_AVAILABLE                     = 30,
    Cmd_DISPLAY_OBJECTIVE                  = 37,
    Cmd_DISPLAY_INFORMATION                = 38,
    Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL       = 40,
    Cmd_ADD_CREATURE_TO_POOL               = 41,
    Cmd_RESET_ACTION_POINT                 = 42,
    Cmd_SET_CREATURE_MAX_LEVEL             = 59,
    Cmd_SET_MUSIC                          = 60,
    Cmd_TUTORIAL_FLASH_BUTTON              = 54,
    Cmd_SET_CREATURE_STRENGTH              = 62,
    Cmd_SET_CREATURE_HEALTH                = 61,
    Cmd_SET_CREATURE_ARMOUR                = 63,
    Cmd_SET_CREATURE_FEAR_WOUNDED          = 64,
    Cmd_IF_AVAILABLE                       = 66,
    Cmd_SET_COMPUTER_GLOBALS               = 68,
    Cmd_SET_COMPUTER_CHECKS                = 69,
    Cmd_SET_COMPUTER_EVENT                 = 70,
    Cmd_SET_COMPUTER_PROCESS               = 71,
    Cmd_ALLY_PLAYERS                       = 72,
    Cmd_DEAD_CREATURES_RETURN_TO_POOL      = 73,
    Cmd_BONUS_LEVEL_TIME                   = 75,
    Cmd_QUICK_OBJECTIVE                    = 44,
    Cmd_QUICK_INFORMATION                  = 45,
    Cmd_QUICK_OBJECTIVE_WITH_POS           = 46,
    Cmd_QUICK_INFORMATION_WITH_POS         = 47,
    Cmd_DISPLAY_OBJECTIVE_WITH_POS         = 65,
    Cmd_DISPLAY_INFORMATION_WITH_POS       = 74,
    Cmd_PRINT                              = 76, // from beta
    Cmd_SWAP_CREATURE                      = 77,
  // New commands propositions - KeeperFX only
    Cmd_CHANGE_SLAB_TYPE                   = 78,
    Cmd_CHANGE_SLAB_OWNER                  = 79,
    Cmd_IF_SLAB_TYPE                       = 80,
    Cmd_IF_SLAB_OWNER                      = 81,
    Cmd_SET_CREATURE_TENDENCIES            = 84,
    Cmd_PLAY_MESSAGE                       = 85,
    Cmd_ADD_GOLD_TO_PLAYER                 = 86,
    Cmd_REVEAL_MAP_RECT                    = 87,
    Cmd_REVEAL_MAP_LOCATION                = 88,
    Cmd_LEVEL_VERSION                      = 90,
    Cmd_RESEARCH_ORDER                     = 91,
    Cmd_KILL_CREATURE                      = 92,
    Cmd_SET_CREATURE_FEAR_STRONGER         = 93,
    Cmd_IF_CONTROLS                        = 94,
    Cmd_ADD_TO_FLAG                        = 95,
    Cmd_SET_CAMPAIGN_FLAG                  = 96,
    Cmd_ADD_TO_CAMPAIGN_FLAG               = 97,
    Cmd_EXPORT_VARIABLE                    = 98,
    Cmd_IMPORT                             = 99,
    Cmd_RUN_AFTER_VICTORY                  = 100,
    Cmd_LEVEL_UP_CREATURE                  = 101,
    Cmd_CHANGE_CREATURE_OWNER              = 102,
    Cmd_SET_GAME_RULE                      = 103,
    Cmd_SET_TRAP_CONFIGURATION             = 104,
    Cmd_SET_DOOR_CONFIGURATION             = 105,
    Cmd_SET_OBJECT_CONFIGURATION           = 106,
    Cmd_SET_CREATURE_CONFIGURATION         = 107,
    Cmd_SET_CREATURE_PROPERTY              = 108,
    Cmd_SET_CREATURE_FEARSOME_FACTOR       = 109,
    Cmd_USE_POWER_ON_CREATURE              = 110,
    Cmd_USE_POWER_AT_POS                   = 111,
    Cmd_USE_POWER                          = 112,
    Cmd_USE_POWER_AT_LOCATION              = 113,
    Cmd_ADD_OBJECT_TO_LEVEL                = 114,
    Cmd_USE_SPECIAL_INCREASE_LEVEL         = 115,
    Cmd_USE_SPECIAL_MULTIPLY_CREATURES     = 116,
    Cmd_MAKE_SAFE                          = 117,
    Cmd_LOCATE_HIDDEN_WORLD                = 118,
    Cmd_USE_SPECIAL_TRANSFER_CREATURE      = 119,
    Cmd_CHANGE_CREATURES_ANNOYANCE         = 120,
    Cmd_COMPUTER_DIG_TO_LOCATION           = 121,
    Cmd_DELETE_FROM_PARTY                  = 122,
    Cmd_SET_SACRIFICE_RECIPE               = 123,
    Cmd_REMOVE_SACRIFICE_RECIPE            = 124,
    Cmd_SET_BOX_TOOLTIP                    = 125,
    Cmd_SET_BOX_TOOLTIP_ID                 = 126,
    Cmd_CREATE_EFFECTS_LINE                = 127,
    Cmd_DISPLAY_MESSAGE                    = 128,
    Cmd_QUICK_MESSAGE                      = 129,
    Cmd_CLEAR_MESSAGE                      = 130,
    Cmd_SET_HEART_HEALTH                   = 131,
    Cmd_ADD_HEART_HEALTH                   = 132,
    Cmd_CREATURE_ENTRANCE_LEVEL            = 133,
    Cmd_RANDOMISE_FLAG                     = 134,
    Cmd_COMPUTE_FLAG                       = 135,
    Cmd_CONCEAL_MAP_RECT                   = 136,
    Cmd_DISPLAY_TIMER                      = 137,
    Cmd_ADD_TO_TIMER                       = 138,
    Cmd_ADD_BONUS_TIME                     = 139,
    Cmd_DISPLAY_VARIABLE                   = 140,
    Cmd_DISPLAY_COUNTDOWN                  = 141,
    Cmd_HIDE_TIMER                         = 142,
    Cmd_HIDE_VARIABLE                      = 143,
    Cmd_CREATE_EFFECT                      = 144,
    Cmd_CREATE_EFFECT_AT_POS               = 145,
    Cmd_HEART_LOST_QUICK_OBJECTIVE         = 146,
    Cmd_HEART_LOST_OBJECTIVE               = 147,
    Cmd_SET_DOOR                           = 148,
    Cmd_SET_CREATURE_INSTANCE              = 149,
    Cmd_TRANSFER_CREATURE                  = 150,
    Cmd_SET_HAND_RULE                      = 151,
    Cmd_MOVE_CREATURE                      = 152,
    Cmd_COUNT_CREATURES_AT_ACTION_POINT    = 153,
    Cmd_IF_ALLIED                          = 154,
    Cmd_SET_TEXTURE                        = 155,
    Cmd_USE_SPELL_ON_CREATURE              = 156,
    Cmd_USE_SPELL_ON_PLAYERS_CREATURES     = 157,
    Cmd_SET_ROOM_CONFIGURATION             = 158,
    Cmd_NEW_TRAP_TYPE                      = 159,
    Cmd_NEW_OBJECT_TYPE                    = 160,
    Cmd_NEW_ROOM_TYPE                      = 161,
    Cmd_NEW_CREATURE_TYPE                  = 162,
    Cmd_SET_HAND_GRAPHIC                   = 163,
    Cmd_ADD_EFFECT_GENERATOR_TO_LEVEL      = 164,
    Cmd_SET_EFFECT_GENERATOR_CONFIGURATION = 165,
    Cmd_SET_POWER_CONFIGURATION            = 166,
    Cmd_SET_PLAYER_COLOUR                  = 167,
    Cmd_MAKE_UNSAFE                        = 168,
    Cmd_LEVEL_UP_PLAYERS_CREATURES         = 169,
    Cmd_SET_INCREASE_ON_EXPERIENCE         = 170,
    Cmd_SET_PLAYER_MODIFIER                = 171,
    Cmd_ADD_TO_PLAYER_MODIFIER             = 172,
    Cmd_USE_POWER_ON_PLAYERS_CREATURES     = 173,
    Cmd_CHANGE_SLAB_TEXTURE                = 174,
    Cmd_MOVE_PLAYER_CAMERA_TO              = 175,
    Cmd_ADD_OBJECT_TO_LEVEL_AT_POS         = 176,
    Cmd_PLACE_DOOR                         = 177,
    Cmd_PLACE_TRAP                         = 178,
    Cmd_LOCK_POSSESSION                    = 179,
    Cmd_SET_DIGGER                         = 180,
    Cmd_RUN_LUA_CODE                       = 181,
    Cmd_TAG_MAP_RECT                       = 182,
    Cmd_UNTAG_MAP_RECT                     = 183,
    Cmd_SET_NEXT_LEVEL                     = 184,
    Cmd_SHOW_BONUS_LEVEL                   = 185,
    Cmd_HIDE_BONUS_LEVEL                   = 186,
    Cmd_HIDE_HERO_GATE                     = 187,
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

enum ScriptVariables {
  SVar_MONEY                           =  1,
  SVar_GAME_TURN                       =  5,
  SVar_BREAK_IN                        =  6,
  SVar_CREATURE_NUM                    =  7,
  SVar_TOTAL_DIGGERS                   =  8,
  SVar_TOTAL_CREATURES                 =  9,
  SVar_TOTAL_RESEARCH                  = 10,
  SVar_TOTAL_DOORS                     = 11,
  SVar_TOTAL_AREA                      = 12,
  SVar_TOTAL_CREATURES_LEFT            = 13,
  SVar_CREATURES_ANNOYED               = 14,
  SVar_BATTLES_LOST                    = 15,
  SVar_BATTLES_WON                     = 16,
  SVar_ROOMS_DESTROYED                 = 17,
  SVar_SPELLS_STOLEN                   = 18,
  SVar_ACTION_POINT_TRIGGERED          = 19,
  SVar_GOLD_POTS_STOLEN                = 20,
  SVar_TIMER                           = 21,
  SVar_DUNGEON_DESTROYED               = 22,
  SVar_TIMES_BROKEN_INTO               = 23,
  SVar_TOTAL_GOLD_MINED                = 24,
  SVar_FLAG                            = 25,
  SVar_ROOM_SLABS                      = 26,
  SVar_DOORS_DESTROYED                 = 27,
  SVar_CREATURES_SCAVENGED_LOST        = 28,
  SVar_CREATURES_SCAVENGED_GAINED      = 29,
  SVar_AVAILABLE_MAGIC                 = 30,
  SVar_AVAILABLE_TRAP                  = 31,
  SVar_AVAILABLE_DOOR                  = 32,
  SVar_AVAILABLE_ROOM                  = 33,
  SVar_AVAILABLE_CREATURE              = 34,
  SVar_CONTROLS_CREATURE               = 35,
  SVar_CONTROLS_TOTAL_CREATURES        = 36,
  SVar_CONTROLS_TOTAL_DIGGERS          = 37,
  SVar_ALL_DUNGEONS_DESTROYED          = 38,
  SVar_DOOR_NUM                        = 39,
  SVar_TRAP_NUM                        = 40,
  SVar_GOOD_CREATURES                  = 41,
  SVar_EVIL_CREATURES                  = 42,
  SVar_CONTROLS_GOOD_CREATURES         = 43,
  SVar_CONTROLS_EVIL_CREATURES         = 44,
  SVar_CAMPAIGN_FLAG                   = 45,
  SVar_SLAB_OWNER                      = 46,
  SVar_SLAB_TYPE                       = 47,
  SVar_HEART_HEALTH                    = 48,
  SVar_GHOSTS_RAISED                   = 49,
  SVar_SKELETONS_RAISED                = 50,
  SVar_VAMPIRES_RAISED                 = 51,
  SVar_CREATURES_CONVERTED             = 52,
  SVar_TIMES_ANNOYED_CREATURE          = 53,
  SVar_TIMES_TORTURED_CREATURE         = 54,
  SVar_TOTAL_DOORS_MANUFACTURED        = 55,
  SVar_TOTAL_TRAPS_MANUFACTURED        = 56,
  SVar_TOTAL_MANUFACTURED              = 57,
  SVar_TOTAL_TRAPS_USED                = 58,
  SVar_TOTAL_DOORS_USED                = 59,
  SVar_KEEPERS_DESTROYED               = 60,
  SVar_CREATURES_SACRIFICED            = 61, // Total
  SVar_CREATURES_FROM_SACRIFICE        = 62, // Total
  SVar_TIMES_LEVELUP_CREATURE          = 63,
  SVar_TOTAL_SALARY                    = 64,
  SVar_CURRENT_SALARY                  = 65,
  SVar_BOX_ACTIVATED                   = 66,
  SVar_TRAP_ACTIVATED                  = 86,
  SVar_SACRIFICED                      = 67,  // Per model
  SVar_REWARDED                        = 68,  // Per model
  SVar_EVIL_CREATURES_CONVERTED        = 69,
  SVar_GOOD_CREATURES_CONVERTED        = 70,
  SVar_TRAPS_SOLD                      = 71,
  SVar_DOORS_SOLD                      = 72,
  SVar_MANUFACTURED_SOLD               = 73,
  SVar_MANUFACTURE_GOLD                = 74,
  SVar_TOTAL_SCORE                     = 75,
  SVar_BONUS_TIME                      = 76,
  SVar_CREATURES_TRANSFERRED           = 77,
  SVar_ALLIED_PLAYER                   = 78,
  SVar_ACTIVE_BATTLES                  = 79,
  SVar_VIEW_TYPE                       = 80,
  SVar_TOTAL_TRAPS                     = 81,
  SVar_AVAILABLE_TOTAL_TRAPS           = 82,
  SVar_AVAILABLE_TOTAL_DOORS           = 83,
  SVar_AVAILABLE_TOTAL_CREATURES       = 84,
  SVar_DESTROYED_KEEPER                = 85,
  SVar_TOTAL_SLAPS                     = 87,
  SVar_SCORE                           = 88,
  SVar_PLAYER_SCORE                    = 89,
  SVar_MANAGE_SCORE                    = 90,
 };



extern const struct NamedCommand player_desc[];
extern const struct NamedCommand controls_variable_desc[];
extern const struct NamedCommand timer_desc[];
extern const struct NamedCommand flag_desc[];
extern const struct NamedCommand hand_rule_desc[];
extern const struct NamedCommand rule_slot_desc[];
extern const struct NamedCommand rule_action_desc[];
extern const struct NamedCommand hero_objective_desc[];
extern const struct NamedCommand msgtype_desc[];
extern const struct NamedCommand tendency_desc[];
extern const struct NamedCommand creature_select_criteria_desc[];
extern const struct NamedCommand gui_button_group_desc[];
extern const struct NamedCommand campaign_flag_desc[];
extern const struct NamedCommand script_operator_desc[];
extern const struct NamedCommand variable_desc[];
extern const struct NamedCommand dk1_variable_desc[];
extern const struct NamedCommand fill_desc[];
extern const struct NamedCommand set_door_desc[];
extern const struct NamedCommand texture_pack_desc[];
extern const struct NamedCommand locked_desc[];

ThingModel parse_creature_name(const char *creature_name);
struct ScriptValue *allocate_script_value(void);
struct Thing *script_process_new_object(ThingModel tngmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long arg, PlayerNumber plyr_idx, short move_angle);
struct Thing* script_process_new_effectgen(ThingModel crmodel, TbMapLocation location, long range);
void command_init_value(struct ScriptValue* value, unsigned long var_index, unsigned long plr_range_id);
void command_add_value(unsigned long var_index, unsigned long plr_range_id, long param1, long param2, long param3);
void set_variable(int player_idx, long var_type, long var_idx, long new_val);
#define get_players_range(plr_range_id, plr_start, plr_end) get_players_range_f(plr_range_id, plr_start, plr_end, __func__, text_line_number)
long get_players_range_f(long plr_range_id, int *plr_start, int *plr_end, const char *func_name, long ln_num);
TbBool parse_set_varib(const char *varib_name, int32_t *varib_id, int32_t *varib_type);
long parse_criteria(const char *criteria);
#define get_players_range_single(plr_range_id) get_players_range_single_f(plr_range_id, __func__, text_line_number)
long get_players_range_single_f(long plr_range_id, const char *func_name, long ln_num);
TbBool parse_get_varib(const char *varib_name, int32_t *varib_id, int32_t *varib_type, long level_file_version);
void get_chat_icon_from_value(const char* txt, char* id, char* type);
#define get_player_id(plrname, plr_range_id) get_player_id_f(plrname, plr_range_id, __func__, text_line_number)
TbBool get_player_id_f(const char *plrname, int32_t *plr_range_id, const char *func_name, long ln_num);
PlayerNumber get_objective_id_with_potential_target(const char* locname, PlayerNumber* target);
TbResult script_use_power_on_creature(struct Thing* thing, short pwkind, KeepPwrLevel power_level, PlayerNumber caster, TbBool is_free);
const char * script_strval(long offset);
long script_strdup(const char *src);

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

#define DEALLOCATE_SCRIPT_VALUE \
    if (value != &tmp_value) \
    {                           \
        value->flags = TrgF_DISABLED; \
        game.script.values_num--; \
    }

    void script_process_value(unsigned long var_index, unsigned long plr_range_id, long param1, long param2, long param3, struct ScriptValue *value);

#define PROCESS_SCRIPT_VALUE(cmd) \
    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0)) \
    { \
        script_process_value(cmd, value->plyr_range, 0, 0, 0, value); \
    }


#ifdef __cplusplus
}
#endif
#endif
