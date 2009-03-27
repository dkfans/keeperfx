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
#define COMMANDDESC_ARGS_COUNT 8

enum TbScriptCommands {
    Cmd_NONE                           =  0,
    Cmd_CREATE_PARTY                   =  1,
    Cmd_ADD_TO_PARTY                   =  2,
    Cmd_ADD_PARTY_TO_LEVEL             =  3,
    Cmd_ADD_CREATURE_TO_LEVEL          =  4,
    Cmd_MESSAGE                        =  5, // from beta
    Cmd_IF                             =  6,
    Cmd_IF_ACTION_POINT                = 19,
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
    Cmd_ADD_TUNNELLER_TO_LEVEL         = 20,
    Cmd_WIN_GAME                       = 21,
    Cmd_LOSE_GAME                      = 22,
    Cmd_SET_FLAG                       = 25,
    Cmd_MAX_CREATURES                  = 26,
    Cmd_NEXT_COMMAND_REUSABLE          = 27,
    Cmd_DOOR_AVAILABLE                 = 30,
    Cmd_DISPLAY_OBJECTIVE              = 37,
    Cmd_DISPLAY_OBJECTIVE_WITH_POS     = 65,
    Cmd_DISPLAY_INFORMATION            = 38,
    Cmd_DISPLAY_INFORMATION_WITH_POS   = 74,
    Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL   = 40,
    Cmd_ADD_CREATURE_TO_POOL           = 41,
    Cmd_RESET_ACTION_POINT             = 42,
    Cmd_SET_CREATURE_MAX_LEVEL         = 59,
    Cmd_SET_MUSIC                      = 60,
    Cmd_TUTORIAL_FLASH_BUTTON          = 54,
    Cmd_SET_CREATURE_STRENGTH          = 62,
    Cmd_SET_CREATURE_HEALTH            = 61,
    Cmd_SET_CREATURE_ARMOUR            = 63,
    Cmd_SET_CREATURE_FEAR              = 64,
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
    Cmd_SWAP_CREATURE                  = 77,
    Cmd_PRINT                          = 76, // from beta
};

#pragma pack(1)

struct CommandDesc { // sizeof = 14 // originally was 13
  const char *textptr;
  char args[COMMANDDESC_ARGS_COUNT+1]; // originally was [8]
  unsigned char index;
};

struct Description { // sizeof = 5
  const char *textptr;
  unsigned char index;
};

struct ScriptLine {
  long np[COMMANDDESC_ARGS_COUNT];
  char tcmnd[MAX_TEXT_LENGTH];
  char tp[COMMANDDESC_ARGS_COUNT][MAX_TEXT_LENGTH];
};

#pragma pack()

/******************************************************************************/
extern const struct CommandDesc command_desc[];
/******************************************************************************/
DLLIMPORT short _DK_script_current_condition;
#define script_current_condition _DK_script_current_condition
DLLIMPORT unsigned long _DK_script_line_number;
#define script_line_number _DK_script_line_number
DLLIMPORT unsigned char _DK_next_command_reusable;
#define next_command_reusable _DK_next_command_reusable
DLLIMPORT unsigned short _DK_condition_stack_pos;
#define condition_stack_pos _DK_condition_stack_pos
DLLIMPORT unsigned short _DK_condition_stack[48];
#define condition_stack _DK_condition_stack
//DLLIMPORT struct Description _DK_player_desc[8];
//DLLIMPORT struct Description _DK_magic_desc[20];
//DLLIMPORT struct Description _DK_creature_desc[32];
//DLLIMPORT struct Description _DK_room_desc[16];
//DLLIMPORT struct Description _DK_timer_desc[9];
//DLLIMPORT struct Description _DK_flag_desc[9];
//DLLIMPORT struct Description _DK_door_desc[5];
//DLLIMPORT struct Description _DK_trap_desc[8];
//DLLIMPORT struct Description _DK_hero_objective_desc[];
/******************************************************************************/
short script_support_setup_player_as_computer_keeper(unsigned short plyridx, long comp_model);
long scan_line(char *line);
const struct CommandDesc *get_next_word(char **line, char *params, unsigned char *nparam);
long get_id(const struct Description *desc, char *itmname);
const char *script_get_command_name(long cmnd_index);

void command_add_to_party(char *prtname, char *crtr_name, long crtr_level, long carried_gold, char *objectv, long countdown);
void command_add_party_to_level(char *plrname, char *prtname, char *dst_place, long ncopies);
void command_add_creature_to_level(char *plrname, char *crtr_name, char *dst_place, long ncopies, long crtr_level, long carried_gold);
void command_if(char *plrname, char *varib_name, char *operatr, long value);
void command_add_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4);
void command_display_information(long info_idx, long pos_x, long pos_y);
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

short clear_script(void);
short load_script(long lvl_num);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
