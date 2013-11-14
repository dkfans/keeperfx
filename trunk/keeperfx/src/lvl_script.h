/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script.h
 *     Header file for lvl_script.c.
 * @par Purpose:
 *     Level script commands support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LVLSCRIPT_H
#define DK_LVLSCRIPT_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define COMMANDDESC_ARGS_COUNT    8

#define PARTY_TRIGGERS_COUNT     48
#define CREATURE_PARTYS_COUNT    16
#define PARTY_MEMBERS_COUNT       8
#define CONDITIONS_COUNT         48
#define TUNNELLER_TRIGGERS_COUNT 16
#define SCRIPT_VALUES_COUNT      64
#define WIN_CONDITIONS_COUNT      4

enum TbScriptCommands {
    Cmd_NONE                           =  0,
    Cmd_CREATE_PARTY                   =  1,
    Cmd_ADD_TO_PARTY                   =  2,
    Cmd_ADD_PARTY_TO_LEVEL             =  3,
    Cmd_ADD_CREATURE_TO_LEVEL          =  4,
    Cmd_MESSAGE                        =  5, // from beta
    Cmd_IF                             =  6,
    Cmd_ENDIF                          =  7,
    Cmd_SET_HATE                       =  8,
    Cmd_SET_GENERATE_SPEED             =  9,
    Cmd_REM                            = 10,
    Cmd_START_MONEY                    = 11,
    Cmd_ROOM_AVAILABLE                 = 12,
    Cmd_CREATURE_AVAILABLE             = 13,
    Cmd_MAGIC_AVAILABLE                = 14,
    Cmd_TRAP_AVAILABLE                 = 15,
    Cmd_RESEARCH                       = 16,
    Cmd_COMPUTER_PLAYER                = 17,
    Cmd_SET_TIMER                      = 18,
    Cmd_IF_ACTION_POINT                = 19,
    Cmd_ADD_TUNNELLER_TO_LEVEL         = 20,
    Cmd_WIN_GAME                       = 21,
    Cmd_LOSE_GAME                      = 22,
    Cmd_SET_FLAG                       = 25,
    Cmd_MAX_CREATURES                  = 26,
    Cmd_NEXT_COMMAND_REUSABLE          = 27,
    Cmd_DOOR_AVAILABLE                 = 30,
    Cmd_DISPLAY_OBJECTIVE              = 37,
    Cmd_DISPLAY_INFORMATION            = 38,
    Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL   = 40,
    Cmd_ADD_CREATURE_TO_POOL           = 41,
    Cmd_RESET_ACTION_POINT             = 42,
    Cmd_SET_CREATURE_MAX_LEVEL         = 59,
    Cmd_SET_MUSIC                      = 60,
    Cmd_TUTORIAL_FLASH_BUTTON          = 54,
    Cmd_SET_CREATURE_STRENGTH          = 62,
    Cmd_SET_CREATURE_HEALTH            = 61,
    Cmd_SET_CREATURE_ARMOUR            = 63,
    Cmd_SET_CREATURE_FEAR_WOUNDED      = 64,
    Cmd_IF_AVAILABLE                   = 66,
    Cmd_SET_COMPUTER_GLOBALS           = 68,
    Cmd_SET_COMPUTER_CHECKS            = 69,
    Cmd_SET_COMPUTER_EVENT             = 70,
    Cmd_SET_COMPUTER_PROCESS           = 71,
    Cmd_ALLY_PLAYERS                   = 72,
    Cmd_DEAD_CREATURES_RETURN_TO_POOL  = 73,
    Cmd_BONUS_LEVEL_TIME               = 75,
    Cmd_QUICK_OBJECTIVE                = 44,
    Cmd_QUICK_INFORMATION              = 45,
    Cmd_QUICK_OBJECTIVE_WITH_POS       = 46,
    Cmd_QUICK_INFORMATION_WITH_POS     = 47,
    Cmd_DISPLAY_OBJECTIVE_WITH_POS     = 65,
    Cmd_DISPLAY_INFORMATION_WITH_POS   = 74,
    Cmd_PRINT                          = 76, // from beta
    Cmd_SWAP_CREATURE                  = 77,
  // New commands propositions - KeeperFX only
    Cmd_CHANGE_SLAB_TYPE               = 78,
    Cmd_CHANGE_SLAB_OWNER              = 79,
    Cmd_IF_SLAB_TYPE                   = 80,
    Cmd_IF_SLAB_OWNER                  = 81,
    Cmd_SET_CREATURE_TENDENCIES        = 84,
    Cmd_PLAY_MESSAGE                   = 85,
    Cmd_ADD_GOLD_TO_PLAYER             = 86,
    Cmd_REVEAL_MAP_RECT                = 87,
    Cmd_REVEAL_MAP_LOCATION            = 88,
    Cmd_LEVEL_VERSION                  = 90,
    Cmd_RESEARCH_ORDER                 = 91,
    Cmd_KILL_CREATURE                  = 92,
    Cmd_SET_CREATURE_FEAR_STRONGER     = 93,
};

enum ScriptVatiables {
  SVar_MONEY                           =  1,
  SVar_GAME_TURN                       =  5,
  SVar_BREAK_IN                        =  6,
  SVar_CREATURE_NUM                    =  7,
  SVar_TOTAL_IMPS                      =  8,
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
  SVar_TIMES_BROKEN_INTO               = 19,
  SVar_GOLD_POTS_STOLEN                = 20,
  SVar_TIMER                           = 21,
  SVar_DUNGEON_DESTROYED               = 22,
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
  SVar_ALL_DUNGEONS_DESTROYED          = 34,
  SVar_DOOR_NUM                        = 35,
};

enum MapLocationTypes {
    MLoc_NONE                          =  0,
    MLoc_ACTIONPOINT                   =  1,
    MLoc_HEROGATE                      =  2,
    MLoc_PLAYERSHEART                  =  3,
    MLoc_CREATUREKIND                  =  4,
    MLoc_OBJECTKIND                    =  5,
    MLoc_ROOMKIND                      =  6,
    MLoc_THING                         =  7,
    MLoc_PLAYERSDUNGEON                =  8,
    MLoc_APPROPRTDUNGEON               =  9,
    MLoc_DOORKIND                      = 10,
    MLoc_TRAPKIND                      = 11,
};

/******************************************************************************/
#pragma pack(1)

struct Condition;
struct Party;
typedef unsigned long TbMapLocation;

struct CommandDesc { // sizeof = 14 // originally was 13
  const char *textptr;
  char args[COMMANDDESC_ARGS_COUNT+1]; // originally was [8]
  unsigned char index;
};

struct ScriptLine {
  long np[COMMANDDESC_ARGS_COUNT];
  char tcmnd[MAX_TEXT_LENGTH];
  char tp[COMMANDDESC_ARGS_COUNT][MAX_TEXT_LENGTH];
};

struct TunnellerTrigger { // sizeof = 18
  unsigned char flags;
  char condit_idx;
  unsigned char plyr_idx;
  unsigned long location;
  unsigned char heading_OLD;//no longer used
  unsigned long heading; // originally was 'target'
  long carried_gold;
  unsigned char crtr_level;
  char party_id;
};

struct PartyTrigger { // sizeof = 13
  unsigned char flags;
  char condit_idx;
  char creatr_id;
  unsigned char plyr_idx;
  unsigned long location;
  unsigned char crtr_level;
  unsigned short carried_gold;
  unsigned short ncopies;
};

struct ScriptValue { // sizeof = 16
  unsigned char flags;
  char condit_idx;
  unsigned char valtype;
  unsigned char field_3;
  long field_4;
  long field_8;
  long field_C;
};

struct Condition { // sizeof = 12
  short condit_idx;
  unsigned char status;
  unsigned char plyr_idx;
  unsigned char variabl_type;
  unsigned short variabl_idx;
  unsigned char operation;
  unsigned long rvalue;
};

struct PartyMember { // sizeof = 13
  unsigned char flags;
  unsigned char field_65;
  unsigned char crtr_kind;
  unsigned char objectv;
  long countdown;
  unsigned char crtr_level;
  unsigned short carried_gold;
  unsigned short field_6F;
};


struct Party { // sizeof = 208
  char prtname[100];
  struct PartyMember members[PARTY_MEMBERS_COUNT];
  unsigned long members_num;
};

struct LevelScript { // sizeof = 5884
    struct TunnellerTrigger tunneller_triggers[TUNNELLER_TRIGGERS_COUNT];
    unsigned long tunneller_triggers_num;
    struct PartyTrigger party_triggers[PARTY_TRIGGERS_COUNT];
    unsigned long party_triggers_num;
    struct ScriptValue values[SCRIPT_VALUES_COUNT];
    unsigned long values_num;
    struct Condition conditions[CONDITIONS_COUNT];
    unsigned long conditions_num;
    struct Party creature_partys[CREATURE_PARTYS_COUNT];
    unsigned long creature_partys_num;
    unsigned short win_conditions[WIN_CONDITIONS_COUNT];
    unsigned long win_conditions_num;
    unsigned short lose_conditions[WIN_CONDITIONS_COUNT];
    unsigned long lose_conditions_num;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT short _DK_script_current_condition;
#define script_current_condition _DK_script_current_condition
DLLIMPORT unsigned long _DK_script_line_number;
DLLIMPORT unsigned char _DK_next_command_reusable;
#define next_command_reusable _DK_next_command_reusable
DLLIMPORT unsigned short _DK_condition_stack_pos;
#define condition_stack_pos _DK_condition_stack_pos
DLLIMPORT unsigned short _DK_condition_stack[48];
#define condition_stack _DK_condition_stack
//DLLIMPORT struct NamedCommand _DK_player_desc[8];
//DLLIMPORT struct NamedCommand _DK_magic_desc[20];
//DLLIMPORT struct NamedCommand _DK_creature_desc[32];
//DLLIMPORT struct NamedCommand _DK_room_desc[16];
//DLLIMPORT struct NamedCommand _DK_timer_desc[9];
//DLLIMPORT struct NamedCommand _DK_flag_desc[9];
//DLLIMPORT struct NamedCommand _DK_door_desc[5];
//DLLIMPORT struct NamedCommand _DK_trap_desc[8];
//DLLIMPORT struct NamedCommand _DK_hero_objective_desc[];
/******************************************************************************/
extern const struct CommandDesc command_desc[];
/******************************************************************************/
TbBool script_support_setup_player_as_computer_keeper(unsigned short plyridx, long comp_model);
TbBool script_support_setup_player_as_zombie_keeper(unsigned short plyridx);
long script_scan_line(char *line,TbBool preloaded);
const struct CommandDesc *get_next_word(char **line, char *params, unsigned char *nparam);
const char *script_get_command_name(long cmnd_index);

void command_add_to_party(char *prtname, char *crtr_name, long crtr_level, long carried_gold, char *objectv, long countdown);
void command_add_party_to_level(char *plrname, char *prtname, char *dst_place, long ncopies);
void command_add_creature_to_level(char *plrname, char *crtr_name, char *dst_place, long ncopies, long crtr_level, long carried_gold);
void command_if(char *plrname, char *varib_name, char *operatr, long value);
void command_add_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4);
void command_display_information(long msg_num, const char *where, long x, long y);
void command_research(char *plrname, char *trg_type, char *trg_name, unsigned long val);
void command_if_action_point(long apt_idx, char *plrname);
void command_add_tunneller_to_level(char *plrname, char *dst_place, char *objectv, long target, unsigned char crtr_level, unsigned long carried_gold);
void command_display_objective(long msg_num, char *plrname, long a3, long a4);
void command_add_tunneller_party_to_level(char *plrname, char *prtname, char *apt_num, char *objectv, long target, char crtr_level, unsigned long carried_gold);
void command_if_available(char *plrname, char *varib_name, char *operatr, long value);
void command_set_computer_globals(char *plrname, long a1, long a2, long a3, long a4, long a5, long a6);
void command_set_computer_checks(char *plrname, char *chkname, long a1, long a2, long a3, long a4, long a5);
void command_set_computer_events(char *plrname, char *evntname, long a1, long a2);
void command_set_computer_process(char *plrname, char *procname, long a1, long a2, long a3, long a4, long a5);
void command_message(char *msgtext, unsigned char kind);
unsigned short get_map_location_type(TbMapLocation location);
unsigned long get_map_location_longval(TbMapLocation location);
unsigned long get_map_location_plyrval(TbMapLocation location);
unsigned short get_map_location_plyridx(TbMapLocation location);

short clear_script(void);
short load_script(long lvl_num);
short preload_script(long lvnum);
/******************************************************************************/
void script_process_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4);
void script_process_win_game(unsigned short plyr_idx);
void script_process_lose_game(unsigned short plyr_idx);
struct Thing *script_process_new_tunneler(unsigned char plyr_idx, TbMapLocation location, TbMapLocation heading, unsigned char crtr_level, unsigned long carried_gold);
struct Thing *script_process_new_party(struct Party *party, unsigned char plyr_idx, TbMapLocation location, long copies_num);
void script_process_new_tunneller_party(unsigned char plyr_idx, long prty_id, TbMapLocation location, TbMapLocation heading, unsigned char crtr_level, unsigned long carried_gold);
long script_support_create_thing_at_hero_door(long a1, unsigned char a2, unsigned char a3, unsigned char a4, unsigned char a5);
long script_support_create_thing_at_action_point(long a1, unsigned char a2, unsigned char a3, unsigned char a4, unsigned char a5);
long script_support_create_creature_at_dungeon_heart(unsigned char a1, unsigned char a2, unsigned char a3);
long script_support_send_tunneller_to_action_point(struct Thing *thing, long a2);
TbBool script_support_send_tunneller_to_dungeon(struct Thing *creatng, PlayerNumber plyr_idx);
TbBool script_support_send_tunneller_to_dungeon_heart(struct Thing *creatng, PlayerNumber plyr_idx);
long script_support_send_tunneller_to_appropriate_dungeon(struct Thing *thing);
struct Thing *script_create_new_creature(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, long carried_gold, long crtr_level);
TbBool process_activation_status(struct Condition *condt);
long get_condition_value(char plyr_idx, unsigned char valtype, unsigned char a3);
TbBool get_condition_status(unsigned char opkind, long val1, long val2);
TbBool condition_inactive(long cond_idx);
TbBool action_point_activated_by_player(long apt_idx,long plyr_idx);
TbBool process_avialable_status(long plyr_idx, long itype, long ikind, long val);
TbBool is_condition_met(long condit_idx);
void process_conditions(void);
void process_values(void);
void process_win_and_lose_conditions(long plyr_idx);
void script_process_new_creatures(unsigned char a1, long a2, long a3, long a4, long a5, long a6);
void process_check_new_creature_partys(void);
void process_check_new_tunneller_partys(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
