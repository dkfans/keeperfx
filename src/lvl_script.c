/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script.c
 *     Level script commands support.
 * @par Purpose:
 *     Load, recognize and maintain the level script.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "lvl_script.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "front_simple.h"
#include "config.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
const struct CommandDesc command_desc[] = {
  {"CREATE_PARTY",                 "A       ", Cmd_CREATE_PARTY},
  {"ADD_TO_PARTY",                 "AANNAN  ", Cmd_ADD_TO_PARTY},
  {"ADD_PARTY_TO_LEVEL",           "AAAN    ", Cmd_ADD_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_LEVEL",        "AAANNN  ", Cmd_ADD_CREATURE_TO_LEVEL},
  {"IF",                           "AAAN    ", Cmd_IF},
  {"IF_ACTION_POINT",              "NA      ", Cmd_IF_ACTION_POINT},
  {"ENDIF",                        "        ", Cmd_ENDIF},
  {"SET_HATE",                     "NNN     ", Cmd_SET_HATE},
  {"SET_GENERATE_SPEED",           "N       ", Cmd_SET_GENERATE_SPEED},
  {"REM",                          "        ", Cmd_REM},
  {"START_MONEY",                  "AN      ", Cmd_START_MONEY},
  {"ROOM_AVAILABLE",               "AANN    ", Cmd_ROOM_AVAILABLE},
  {"CREATURE_AVAILABLE",           "AANN    ", Cmd_CREATURE_AVAILABLE},
  {"MAGIC_AVAILABLE",              "AANN    ", Cmd_MAGIC_AVAILABLE},
  {"TRAP_AVAILABLE",               "AANN    ", Cmd_TRAP_AVAILABLE},
  {"RESEARCH",                     "AAAN    ", Cmd_RESEARCH},
  {"COMPUTER_PLAYER",              "AN      ", Cmd_COMPUTER_PLAYER},
  {"SET_TIMER",                    "AA      ", Cmd_SET_TIMER},
  {"ADD_TUNNELLER_TO_LEVEL",       "AAANNN  ", Cmd_ADD_TUNNELLER_TO_LEVEL},
  {"WIN_GAME",                     "        ", Cmd_WIN_GAME},
  {"LOSE_GAME",                    "        ", Cmd_LOSE_GAME},
  {"SET_FLAG",                     "AAN     ", Cmd_SET_FLAG},
  {"MAX_CREATURES",                "AN      ", Cmd_MAX_CREATURES},
  {"NEXT_COMMAND_REUSABLE",        "        ", Cmd_NEXT_COMMAND_REUSABLE},
  {"DOOR_AVAILABLE",               "AANN    ", Cmd_DOOR_AVAILABLE},
  {"DISPLAY_OBJECTIVE",            "NA      ", Cmd_DISPLAY_OBJECTIVE},
  {"DISPLAY_OBJECTIVE_WITH_POS",   "NNN     ", Cmd_DISPLAY_OBJECTIVE_WITH_POS},
  {"DISPLAY_INFORMATION",          "N       ", Cmd_DISPLAY_INFORMATION},
  {"DISPLAY_INFORMATION_WITH_POS", "NNN     ", Cmd_DISPLAY_INFORMATION_WITH_POS},
  {"ADD_TUNNELLER_PARTY_TO_LEVEL", "AAAANNN ", Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_POOL",         "AN      ", Cmd_ADD_CREATURE_TO_POOL},
  {"RESET_ACTION_POINT",           "N       ", Cmd_RESET_ACTION_POINT},
  {"SET_CREATURE_MAX_LEVEL",       "AAN     ", Cmd_SET_CREATURE_MAX_LEVEL},
  {"SET_MUSIC",                    "N       ", Cmd_SET_MUSIC},
  {"TUTORIAL_FLASH_BUTTON",        "NN      ", Cmd_TUTORIAL_FLASH_BUTTON},
  {"SET_CREATURE_STRENGTH",        "AN      ", Cmd_SET_CREATURE_STRENGTH},
  {"SET_CREATURE_HEALTH",          "AN      ", Cmd_SET_CREATURE_HEALTH},
  {"SET_CREATURE_ARMOUR",          "AN      ", Cmd_SET_CREATURE_ARMOUR},
  {"SET_CREATURE_FEAR",            "AN      ", Cmd_SET_CREATURE_FEAR},
  {"IF_AVAILABLE",                 "AAAN    ", Cmd_IF_AVAILABLE},
  {"SET_COMPUTER_GLOBALS",         "ANNNNNN ", Cmd_SET_COMPUTER_GLOBALS},
  {"SET_COMPUTER_CHECKS",          "AANNNNN ", Cmd_SET_COMPUTER_CHECKS},
  {"SET_COMPUTER_EVENT",           "AANN    ", Cmd_SET_COMPUTER_EVENT},
  {"SET_COMPUTER_PROCESS",         "AANNNNN ", Cmd_SET_COMPUTER_PROCESS},
  {"ALLY_PLAYERS",                 "AA      ", Cmd_ALLY_PLAYERS},
  {"DEAD_CREATURES_RETURN_TO_POOL","N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL},
  {"BONUS_LEVEL_TIME",             "N       ", Cmd_BONUS_LEVEL_TIME},
  {"QUICK_OBJECTIVE",              "NAA     ", Cmd_QUICK_OBJECTIVE},
  {"QUICK_INFORMATION",            "NA      ", Cmd_QUICK_INFORMATION},
  {"SWAP_CREATURE",                "AA      ", Cmd_SWAP_CREATURE},
  {"PRINT",                        "A       ", Cmd_PRINT},
  {"MESSAGE",                      "A       ", Cmd_MESSAGE},
  {NULL,                           "        ", Cmd_NONE},
};

const struct Description room_desc[] = {
  {"ENTRANCE",         1},
  {"TREASURE",         2},
  {"RESEARCH",         3},
  {"PRISON",           4},
  {"TORTURE",          5},
  {"TRAINING",         6},
  {"WORKSHOP",         8},
  {"SCAVENGER",        9},
  {"TEMPLE",          10},
  {"GRAVEYARD",       11},
  {"BARRACKS",        12},
  {"GARDEN",          13},
  {"LAIR",            14},
  {"BRIDGE",          15},
  {"GUARD_POST",      16},
  {NULL,               0},
};

const struct Description creature_desc[] = {
  {"WIZARD",           1},
  {"BARBARIAN",        2},
  {"ARCHER",           3},
  {"MONK",             4},
  {"DWARFA",           5},
  {"KNIGHT",           6},
  {"AVATAR",           7},
  {"TUNNELLER",        8},
  {"WITCH",            9},
  {"GIANT",           10},
  {"FAIRY",           11},
  {"THIEF",           12},
  {"SAMURAI",         13},
  {"HORNY",           14},
  {"SKELETON",        15},
  {"TROLL",           16},
  {"DRAGON",          17},
  {"DEMONSPAWN",      18},
  {"FLY",             19},
  {"DARK_MISTRESS",   20},
  {"SORCEROR",        21},
  {"BILE_DEMON",      22},
  {"IMP",             23},
  {"BUG",             24},
  {"VAMPIRE",         25},
  {"SPIDER",          26},
  {"HELL_HOUND",      27},
  {"GHOST",           28},
  {"TENTACLE",        29},
  {"ORC",             30},
  {"FLOATING_SPIRIT", 31},
  {NULL,               0},
};

const struct Description newcrtr_desc[] = {
  {"NEW_CREATURE_A",   1},
  {"NEW_CREATURE_B",   2},
  {NULL,               0},
};

const struct Description player_desc[] = {
  {"PLAYER0",          0},
  {"PLAYER1",          1},
  {"PLAYER2",          2},
  {"PLAYER3",          3},
  {"PLAYER_GOOD",      4},
  {"ALL_PLAYERS",      8},
  {NULL,               0},
};

const struct Description variable_desc[] = {
  {"MONEY",                        1},
  {"GAME_TURN",                    5},
  {"BREAK_IN",                     6},
  {"TOTAL_IMPS",                   8},
  {"TOTAL_CREATURES",              9},
  {"TOTAL_RESEARCH",              10},
  {"TOTAL_DOORS",                 11},
  {"TOTAL_AREA",                  12},
  {"TOTAL_CREATURES_LEFT",        13},
  {"CREATURES_ANNOYED",           14},
  {"BATTLES_LOST",                15},
  {"BATTLES_WON",                 16},
  {"ROOMS_DESTROYED",             17},
  {"SPELLS_STOLEN",               18},
  {"TIMES_BROKEN_INTO",           19},
  {"GOLD_POTS_STOLEN",            20},
  {"DUNGEON_DESTROYED",           22},
  {"TOTAL_GOLD_MINED",            24},
  {"DOORS_DESTROYED",             27},
  {"CREATURES_SCAVENGED_LOST",    28},
  {"CREATURES_SCAVENGED_GAINED",  29},
  {"ALL_DUNGEONS_DESTROYED",      34},
  {NULL,                           0},
};

const struct Description comparison_desc[] = {
  {"==",     1},
  {"!=",     2},
  {"<",      3},
  {">",      4},
  {"<=",     5},
  {">=",     6},
  {NULL,     0},
};

const struct Description magic_desc[] = {
  {"POWER_HAND",           1},
  {"POWER_IMP",            2},
  {"POWER_OBEY",           3},
  {"POWER_SLAP",           4},
  {"POWER_SIGHT",          5},
  {"POWER_CALL_TO_ARMS",   6},
  {"POWER_CAVE_IN",        7},
  {"POWER_HEAL_CREATURE",  8},
  {"POWER_HOLD_AUDIENCE",  9},
  {"POWER_LIGHTNING",     10},
  {"POWER_SPEED",         11},
  {"POWER_PROTECT",       12},
  {"POWER_CONCEAL",       13},
  {"POWER_DISEASE",       14},
  {"POWER_CHICKEN",       15},
  {"POWER_DESTROY_WALLS", 16},
  {"POWER_POSSESS",       18},
  {"POWER_ARMAGEDDON",    19},
  {NULL,                   0},
};

const struct Description trap_desc[] = {
  {"BOULDER",              1},
  {"ALARM",                2},
  {"POISON_GAS",           3},
  {"LIGHTNING",            4},
  {"WORD_OF_POWER",        5},
  {"LAVA",                 6},
  {NULL,                   0},
};

const struct Description research_desc[] = {
  {"MAGIC",                1},
  {"ROOM",                 2},
  {"CREATURE",             3},
  {NULL,                   0},
};

const struct Description head_for_desc[] = {
  {"ACTION_POINT",         1},
  {"DUNGEON",              2},
  {"DUNGEON_HEART",        3},
  {"APPROPIATE_DUNGEON",   4},
  {NULL,                   0},
};

const struct Description timer_desc[] = {
  {"TIMER0", 0},
  {"TIMER1", 1},
  {"TIMER2", 2},
  {"TIMER3", 3},
  {"TIMER4", 4},
  {"TIMER5", 5},
  {"TIMER6", 6},
  {"TIMER7", 7},
  {NULL,     0},
};

const struct Description flag_desc[] = {
  {"FLAG0",  0},
  {"FLAG1",  1},
  {"FLAG2",  2},
  {"FLAG3",  3},
  {"FLAG4",  4},
  {"FLAG5",  5},
  {"FLAG6",  6},
  {"FLAG7",  7},
  {NULL,     0},
};

const struct Description door_desc[] = {
  {"WOOD",                 1},
  {"BRACED",               2},
  {"STEEL",                3},
  {"MAGIC",                4},
  {NULL,                   0},
};

const struct Description hero_objective_desc[] = {
  {"STEAL_GOLD",           4},
  {"STEAL_SPELLS",         5},
  {"ATTACK_ENEMIES",       2},
  {"ATTACK_DUNGEON",       3},
  {"ATTACK_ROOMS",         1},
  {"DEFEND_PARTY",         6},
  {NULL,                   0},
};

/******************************************************************************/
DLLIMPORT void _DK_command_if_available(char *plrname, char *varib_name, char *operatr, long value);
DLLIMPORT void _DK_command_set_computer_globals(char *plrname, long a1, long a2, long a3, long a4, long a5, long a6);
DLLIMPORT void _DK_command_set_computer_checks(char *plrname, char *chkname, long a1, long a2, long a3, long a4, long a5);
DLLIMPORT void _DK_command_set_computer_events(char *plrname, char *evntname, long a1, long a2);
DLLIMPORT void _DK_command_set_computer_process(char *plrname, char *procname, long a1, long a2, long a3, long a4, long a5);
DLLIMPORT void _DK_command_display_objective(long msg_num, char *plrname, long a2, long a3);
DLLIMPORT void _DK_command_add_tunneller_party_to_level(char *plrname, char *prtname, char *apt_num, char *objectv, long target, char crtr_level, unsigned long carried_gold);
DLLIMPORT long _DK_script_support_setup_player_as_computer_keeper(unsigned char plyridx, long a2);
DLLIMPORT void _DK_command_research(char *plrname, char *trg_type, char *trg_name, unsigned long val);
DLLIMPORT void _DK_command_if_action_point(long apt_idx, char *plrname);
DLLIMPORT void _DK_command_add_tunneller_to_level(char *plrname, char *dst_place, char *objectv, long target, unsigned char crtr_level, unsigned long carried_gold);
DLLIMPORT long _DK_get_id(const struct Description *desc, char *itmname);
DLLIMPORT void _DK_command_add_to_party(char *prtname, char *crtr_name, long crtr_level, long carried_gold, char *objectv, long countdown);
DLLIMPORT void _DK_command_add_party_to_level(char *plrname, char *prtname, char *dst_place, long ncopies);
DLLIMPORT void _DK_command_add_creature_to_level(char *plrname, char *crtr_name, char *dst_place, long ncopies, long crtr_level, long carried_gold);
DLLIMPORT void _DK_command_if(char *plrname, char *varib_name, char *operatr, long value);
DLLIMPORT void _DK_command_add_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4);
DLLIMPORT struct CommandDesc *_DK_get_next_word(char **line, char *params, unsigned char *line_end);
DLLIMPORT long _DK_scan_line(char *line);
DLLIMPORT short _DK_load_script(long lvl_num);
DLLIMPORT void _DK_script_process_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4);
DLLIMPORT struct Thing *_DK_script_process_new_tunneller(unsigned char a1, long a2, unsigned char a3, long a4, unsigned char a5, unsigned long a6);
DLLIMPORT struct Thing *_DK_script_process_new_party(struct Party *party, unsigned char a2, long a3, long a4);
DLLIMPORT void _DK_script_process_new_creatures(unsigned char a1, long a2, long a3, long a4, long a5, long a6);
/******************************************************************************/
/*
 * Reads word from 'line' into 'param'. Sets if 'line_end' was reached.
 */
const struct CommandDesc *get_next_word(char **line, char *param, unsigned char *line_end)
{
  static const char *func_name="get_next_word";
  const struct CommandDesc *cmnd_desc;
  long rnd_min,rnd_max;
  unsigned int pos;
  char *text;
  char chr;
  int i;
#if (BFDEBUG_LEVEL > 12)
  LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_get_next_word(line, param, line_end);
  cmnd_desc = NULL;
  // Find start of an item to read
  pos = 0;
  param[pos] = '\0';
  while (1)
  {
    chr = **line;
    // number
    if ((isalnum(chr)) || (chr == '-'))
        break;
    // operator
    if ((chr == '\"') || (chr == '=') || (chr == '!') || (chr == '<') || (chr == '>'))
        break;
    // end of line
    if ((chr == '\r') || (chr == '\n') || (chr == '\0'))
    {
        (*line_end) = true;
        return NULL;
    }
    (*line)++;
  }

  chr = **line;
  // Text string
  if (isalpha(chr))
  {
    // Read the parameter
    while (isalnum(chr) || (chr == '_'))
    {
      param[pos] = chr;
      pos++;
      (*line)++;
      chr = **line;
      if (pos+1 >= MAX_TEXT_LENGTH) break;
    }
    param[pos] = '\0';
    strupr(param);
    // Check if it's a command
    i = 0;
    cmnd_desc = NULL;
    while (command_desc[i].textptr != NULL)
    {
      if (strcmp(param, command_desc[i].textptr) == 0)
      {
        cmnd_desc = &command_desc[i];
        break;
      }
      i++;
    }
    // Support of the RANDOM function
    if (strcmp(param, "RANDOM") == 0)
    {
      // Get the minimum random number
      // skip some chars at start
      pos = 0;
      chr = **line;
      while ((!isdigit(chr)) && (chr != '-'))
      {
        if ((chr == '\r') || (chr == '\n') || (chr == '\0'))
        {
          text = buf_sprintf("(script:%lu) Invalid first argument for RANDOM command", script_line_number);
          error(func_name, 900, text);
          (*line_end) = true;
          return NULL;
        }
        (*line)++;
        chr = **line;
      }
      // copy the number as string
      if (chr == '-')
      {
        param[pos] = chr;
        pos++;
        (*line)++;
      }
      chr = **line;
      while ( isdigit(chr) )
      {
        param[pos] = chr;
        pos++;
        (*line)++;
        chr = **line;
      }
      param[pos] = '\0';
      rnd_min = atoi(param);
      // Get the maximum random number
      // skip some chars at start
      pos = 0;
      chr = **line;
      while ((!isdigit(chr)) && (chr != '-'))
      {
        if ((chr == '\r') || (chr == '\n') || (chr == '\0'))
        {
          text = buf_sprintf("(script:%lu) Invalid second argument for RANDOM command", script_line_number);
          error(func_name, 901, text);
          (*line_end) = true;
          return NULL;
        }
        (*line)++;
        chr = **line;
      }
      // copy the number as string
      if (chr == '-')
      {
        param[pos] = chr;
        pos++;
        (*line)++;
      }
      chr = **line;
      while ( isdigit(chr) )
      {
        param[pos] = chr;
        pos++;
        (*line)++;
        chr = **line;
      }
      param[pos] = '\0';
      rnd_max = atoi(param);
      // Prepare the value
      itoa((rand() % (rnd_max-rnd_min+1)) + rnd_min, param, 10);
      pos = 16; // we won't have numbers greater than 16 chars
    }
  } else
  // Number string
  if (isdigit(chr) || (chr == '-'))
  {
      if (chr == '-')
      {
        param[pos] = chr;
        pos++;
        (*line)++;
      }
      chr = **line;
      if (!isdigit(chr))
      {
        text = buf_sprintf("(script:%lu) Unexpected '-' not followed by a number", script_line_number);
        error(func_name, 904, text);
        return NULL;
      }
      while ( isdigit(chr) )
      {
        param[pos] = chr;
        pos++;
        (*line)++;
        chr = **line;
        if (pos+1 >= MAX_TEXT_LENGTH) break;
      }
  } else
  // Multiword string taken into quotes
  if (chr == '\"')
  {
      (*line)++;
      chr = **line;
      while ((chr != '\0') && (chr != '\n') && (chr != '\r'))
      {
        if (chr == '\"')
        {
          (*line)++;
          break;
        }
        param[pos] = chr;
        pos++;
        (*line)++;
        chr = **line;
        if (pos+1 >= MAX_TEXT_LENGTH) break;
    }
  } else
  // Other cases - only operators are left
  {
      param[pos] = chr;
      pos++;
      (*line)++;
      switch (chr)
      {
      case '!':
          chr = **line;
          if (chr != '=')
          {
            text = buf_sprintf("(script:%lu) Expected '=' after '!'", script_line_number);
            error(func_name, 947, text);
            return NULL;
          }
          param[pos] = chr;
          pos++;
          (*line)++;
          break;
      case '>':
      case '<':
          chr = **line;
          if (chr == '=')
          {
            param[pos] = chr;
            pos++;
            (*line)++;
          }
          break;
      case '=':
          chr = **line;
          if (chr != '=')
          {
            text = buf_sprintf("(script:%lu) Expected '=' after '='", script_line_number);
            error(func_name, 936, text);
            return 0;
          }
          param[pos] = chr;
          pos++;
          (*line)++;
          break;
      default:
          break;
      }
  }
  chr = **line;
  if ((chr == '\0') || (chr == '\r')  || (chr == '\n'))
    *line_end = true;
  param[pos] = '\0';
  return cmnd_desc;
}

const char *script_get_command_name(long cmnd_index)
{
  long i;
  static const char *func_name="script_get_command_name";
  i = 0;
  while (command_desc[i].textptr != NULL)
  {
    if (command_desc[i].index == cmnd_index)
      return command_desc[i].textptr;
    i++;
  }
  return NULL;
}

short script_support_setup_player_as_computer_keeper(unsigned short plyridx, long comp_model)
{
  struct PlayerInfo *player;
  player = &(game.players[plyridx%PLAYERS_COUNT]);
  //return _DK_script_support_setup_player_as_computer_keeper(plyridx, comp_model);
  player->field_0 |= 0x01u;
  player->field_2B = plyridx;
  player->field_2C = 1;
  player->field_0 |= 0x40u;
  init_player_start(player);
  setup_a_computer_player(plyridx, comp_model);
  return true;
}

void command_create_party(char *prtname)
{
  static const char *func_name="command_create_party";
  char *text;
  if (game.script.creature_partys_num >= CREATURE_PARTYS_COUNT)
  {
    text = buf_sprintf("(script:%lu) Too many partys in script", script_line_number);
    error(func_name, 1086, text);
    return;
  }
  if (script_current_condition != -1)
  {
    LbWarnLog("(script:%lu) Party '%s' defined inside conditional statement\n",script_line_number,prtname);
  }
  strcpy(game.script.creature_partys[game.script.creature_partys_num].prtname, prtname);
  game.script.creature_partys_num++;
}

long pop_condition(void)
{
  static const char *func_name="pop_condition";
  char *text;
  if (script_current_condition == -1)
  {
    text = buf_sprintf("(script:%lu) unexpected ENDIF", script_line_number);
    error(func_name, 1070, text);
    return -1;
  }
  if ( condition_stack_pos )
  {
    condition_stack_pos--;
    script_current_condition = condition_stack[condition_stack_pos];
  } else
  {
    script_current_condition = -1;
  }
  return script_current_condition;
}

void command_add_to_party(char *prtname, char *crtr_name, long crtr_level, long carried_gold, char *objectv, long countdown)
{
  _DK_command_add_to_party(prtname, crtr_name, crtr_level, carried_gold, objectv, countdown);
  if (script_current_condition != -1)
  {
    LbWarnLog("(script:%lu) Party '%s' member added inside conditional statement\n",script_line_number,prtname);
  }
}

void command_tutorial_flash_button(long btn_id, long duration)
{
  command_add_value(54, 0, btn_id, duration, 0);
}

void command_add_party_to_level(char *plrname, char *prtname, char *dst_place, long ncopies)
{
  _DK_command_add_party_to_level(plrname, prtname, dst_place, ncopies);
}

void command_add_creature_to_level(char *plrname, char *crtr_name, char *dst_place, long ncopies, long crtr_level, long carried_gold)
{
  _DK_command_add_creature_to_level(plrname, crtr_name, dst_place, ncopies, crtr_level, carried_gold);
}

void command_if(char *plrname, char *varib_name, char *operatr, long value)
{
  _DK_command_if(plrname, varib_name, operatr, value);
}

void command_add_value(unsigned long var_index, unsigned long val1, long val2, long val3, long val4)
{
  static const char *func_name="command_add_value";
  struct ScriptValue *value;
  char *text;
  //_DK_command_add_value(var_index, val1, val2, val3, val4);
  if (game.script.values_num >= SCRIPT_VALUES_COUNT)
  {
    text = buf_sprintf("(script:%lu) Too many VALUEs in script (limit is %d)", script_line_number,SCRIPT_VALUES_COUNT);
    error(func_name, 1396, text);
    return;
  }
  if (script_current_condition == -1)
  {
    script_process_value(var_index, val1, val2, val3, val4);
  } else
  {
    value = &game.script.values[game.script.values_num];
    set_flag_byte(&value->field_0, 0x01, next_command_reusable);
    set_flag_byte(&value->field_0, 0x02, false);
    value->field_2 = var_index;
    value->field_3 = val1;
    value->field_4 = val2;
    value->field_8 = val3;
    value->field_C = val4;
    value->field_1 = script_current_condition;
    game.script.values_num++;
  }
}

void command_display_information(long info_idx, long pos_x, long pos_y)
{
  static const char *func_name="command_display_information";
  char *text;
  if (info_idx < 0)
  {
    text = buf_sprintf("(script:%lu) Invalid INFORMATION number", script_line_number);
    error(func_name, 2198, text);
    return;
  }
  command_add_value(38, 0, info_idx, pos_x, pos_y);
}

void command_set_generate_speed(long game_turns)
{
  static const char *func_name="command_set_generate_speed";
  char *text;
  if (game_turns <= 0)
  {
    text = buf_sprintf("(script:%lu) Genaration speed must be positive number", script_line_number);
    error(func_name, 2228, text);
    return;
  }
  command_add_value(9, 0, game_turns, 0, 0);
}

void command_dead_creatures_return_to_pool(long val)
{
  static const char *func_name="command_dead_creatures_return_to_pool";
  command_add_value(73, 0, val, 0, 0);
}

void command_bonus_level_time(long game_turns)
{
  static const char *func_name="command_bonus_level_time";
  char *text;
  if (game_turns < 0)
  {
    text = buf_sprintf("(script:%lu) Bonus time must be nonnegative", script_line_number);
    error(func_name, 2228, text);
    return;
  }
  command_add_value(75, 0, game_turns, 0, 0);
}

long get_id(const struct Description *desc, char *itmname)
{
  return _DK_get_id(desc, itmname);
}

void player_command_add_start_money(int plridx, long gold_val)
{
  struct Dungeon *dungeon;
  dungeon = &(game.dungeon[plridx%DUNGEONS_COUNT]);
  dungeon->field_AFD += gold_val;
  dungeon->field_AF9 += gold_val;
}

void command_set_start_money(char *plrname, long gold_val)
{
  static const char *func_name="command_set_start_money";
  long plr_id;
  int plr_start, plr_end;
  char *text;
  int i;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) invalid player name, '%s'", script_line_number, plrname);
    error(func_name, 1373, text);
    return;
  }
  if (plr_id == 8)
  {
    plr_start = 0;
    plr_end = PLAYERS_COUNT;
  } else
  {
    plr_start = plr_id;
    plr_end = plr_id+1;
  }
  for (i=plr_start; i < plr_end; i++)
    player_command_add_start_money(i, gold_val);
  if (script_current_condition != -1)
  {
    LbWarnLog("(script:%lu) Start money set inside conditional loop\n",script_line_number);
  }
}

void command_room_available(char *plrname, char *roomname, unsigned long a3, unsigned long a4)
{
  static const char *func_name="command_room_available";
  long plr_id,room_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1434, text);
    return;
  }
  room_id = get_id(room_desc, roomname);
  if (room_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown room name, '%s'", script_line_number, roomname);
    error(func_name, 1434, text);
    return;
  }
  command_add_value(12, plr_id, room_id, a3, a4);
}

void command_creature_available(char *plrname, char *crtr_name, unsigned long a3, unsigned long a4)
{
  static const char *func_name="command_creature_available";
  long plr_id,crtr_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1456, text);
    return;
  }
  crtr_id = get_id(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown creature, '%s'", script_line_number, crtr_name);
    error(func_name, 1457, text);
    return;
  }
  command_add_value(13, plr_id, crtr_id, a3, a4);
}

void command_magic_available(char *plrname, char *magname, unsigned long a3, unsigned long a4)
{
  static const char *func_name="command_magic_available";
  long plr_id,mag_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1479, text);
    return;
  }
  mag_id = get_id(magic_desc, magname);
  if (mag_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown magic, '%s'", script_line_number, magname);
    error(func_name, 1480, text);
    return;
  }
  command_add_value(14, plr_id, mag_id, a3, a4);
}

void command_trap_available(char *plrname, char *trapname, unsigned long a3, unsigned long a4)
{
  static const char *func_name="command_trap_available";
  long plr_id,trap_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1502, text);
    return;
  }
  trap_id = get_id(trap_desc, trapname);
  if (trap_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown trap, '%s'", script_line_number, trapname);
    error(func_name, 1503, text);
    return;
  }
  command_add_value(15, plr_id, trap_id, a3, a4);
}

void command_research(char *plrname, char *trg_type, char *trg_name, unsigned long val)
{
  _DK_command_research(plrname, trg_type, trg_name, val);
}

void command_if_action_point(long apt_idx, char *plrname)
{
  _DK_command_if_action_point(apt_idx, plrname);
}

void command_add_tunneller_to_level(char *plrname, char *dst_place, char *objectv, long target, unsigned char crtr_level, unsigned long carried_gold)
{
  _DK_command_add_tunneller_to_level(plrname, dst_place, objectv, target, crtr_level, carried_gold);
}

void command_computer_player(char *plrname, long comp_model)
{
  static const char *func_name="command_computer_player";
  long plr_id,trap_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1636, text);
    return;
  }
  script_support_setup_player_as_computer_keeper(plr_id, comp_model);
  if (script_current_condition != -1)
  {
    LbWarnLog("(script:%lu) Computer player setup inside conditional loop\n",script_line_number);
  }
}

void command_set_timer(char *plrname, char *timrname)
{
  static const char *func_name="command_set_timer";
  long plr_id,timr_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1709, text);
    return;
  }
  timr_id = get_id(timer_desc, timrname);
  if (timr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown timer, '%s'", script_line_number, timrname);
    error(func_name, 1715, text);
    return;
  }
  command_add_value(18, plr_id, timr_id, 0, 0);
}

void command_win_game(void)
{
  static const char *func_name="command_win_game";
  char *text;
  if (script_current_condition == -1)
  {
    text = buf_sprintf("(script:%lu) Command WIN GAME found with no condition", script_line_number);
    error(func_name, 1822, text);
    return;
  }
  if (game.script.win_conditions_num >= WIN_CONDITIONS_COUNT)
  {
    error(func_name, 1827, "Too many WIN GAME conditions in script");
    return;
  }
  game.script.win_conditions[game.script.win_conditions_num] = script_current_condition;
  game.script.win_conditions_num++;
}

void command_lose_game(void)
{
  static const char *func_name="command_lose_game";
  char *text;
  if (script_current_condition == -1)
  {
    text = buf_sprintf("(script:%lu) Command LOSE GAME found with no condition", script_line_number);
    error(func_name, 1839, text);
    return;
  }
  if (game.script.lose_conditions_num >= WIN_CONDITIONS_COUNT)
  {
    error(func_name, 1844, "Too many LOSE GAME conditions in script");
    return;
  }
  game.script.lose_conditions[game.script.lose_conditions_num] = script_current_condition;
  game.script.lose_conditions_num++;
}

void command_set_flag(char *plrname, char *flgname, long val)
{
  static const char *func_name="command_set_flag";
  long plr_id,flg_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1940, text);
    return;
  }
  flg_id = get_id(flag_desc, flgname);
  if (flg_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown flag, '%s'", script_line_number, flgname);
    error(func_name, 1946, text);
    return;
  }
  command_add_value(25, plr_id, flg_id, val, 0);
}

void command_max_creatures(char *plrname, long val)
{
  static const char *func_name="command_max_creatures";
  long plr_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1959, text);
    return;
  }
  command_add_value(26, plr_id, val, 0, 0);
}

void command_door_available(char *plrname, char *doorname, unsigned long a3, unsigned long a4)
{
  static const char *func_name="command_door_available";
  long plr_id,door_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1940, text);
    return;
  }
  door_id = get_id(door_desc, doorname);
  if (door_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown door, '%s'", script_line_number, doorname);
    error(func_name, 1526, text);
    return;
  }
  command_add_value(30, plr_id, door_id, a3, a4);
}

void command_display_objective(long msg_num, char *plrname, long a3, long a4)
{
  _DK_command_display_objective(msg_num, plrname, a3, a4);
}

void command_add_tunneller_party_to_level(char *plrname, char *prtname, char *apt_num, char *objectv, long target, char crtr_level, unsigned long carried_gold)
{
  _DK_command_add_tunneller_party_to_level(plrname, prtname, apt_num, objectv, target, crtr_level, carried_gold);
}

void command_add_creature_to_pool(char *crtr_name, long amount)
{
  static const char *func_name="command_add_creature_to_pool";
  long crtr_id;
  char *text;
  crtr_id = get_id(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown creature, '%s'", script_line_number, crtr_name);
    error(func_name, 2324, text);
    return;
  }
  if ((amount < 0) || (amount >= CREATURES_COUNT))
  {
    text = buf_sprintf("(script:%lu) Invalid number of '%s' creatures for pool, %d", script_line_number, crtr_name, amount);
    error(func_name, 2330, text);
    return;
  }
  command_add_value(41, 0, crtr_id, amount, 0);
}

void command_reset_action_point(long apt_num)
{
  static const char *func_name="command_reset_action_point";
  long apt_idx;
  char *text;
  apt_idx = action_point_number_to_index(apt_num);
  if (!action_point_exists_idx(apt_idx))
  {
    text = buf_sprintf("(script:%lu) Nonexisting Action Point, no %d", script_line_number, apt_num);
    error(func_name, 2347, text);
    return;
  }
  command_add_value(Cmd_RESET_ACTION_POINT, 0, apt_idx, 0, 0);
}

void command_set_creature_max_level(char *plrname, char *crtr_name, long crtr_level)
{
  static const char *func_name="command_set_creature_max_level";
  long plr_id,crtr_id;
  char *text;
  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown player name, '%s'", script_line_number, plrname);
    error(func_name, 1456, text);
    return;
  }
  crtr_id = get_id(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown creature, '%s'", script_line_number, crtr_name);
    error(func_name, 1457, text);
    return;
  }
  if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
  {
    text = buf_sprintf("(script:%lu) Invalid '%s' experience level, %d", script_line_number, crtr_name, crtr_level);
    error(func_name, 2379, text);
  }
  command_add_value(59, plr_id, crtr_id, crtr_level-1, 0);
}

void command_set_music(long val)
{
  game.field_1506D5 = val;
  if (script_current_condition != -1)
  {
    LbWarnLog("(script:%lu) Music set inside conditional loop\n",script_line_number);
  }
}

void command_set_hate(long a1, long a2, long a3)
{
  command_add_value(Cmd_SET_HATE, a1, a2, a3, 0);
}

void command_if_available(char *plrname, char *varib_name, char *operatr, long value)
{
  _DK_command_if_available(plrname, varib_name, operatr, value);
}

void command_set_computer_globals(char *plrname, long a1, long a2, long a3, long a4, long a5, long a6)
{
  _DK_command_set_computer_globals(plrname, a1, a2, a3, a4, a5, a6);
}

void command_set_computer_checks(char *plrname, char *chkname, long a1, long a2, long a3, long a4, long a5)
{
  static const char *func_name="command_set_computer_checks";
  struct ComputerCheck *check;
  int plr_start, plr_end;
  long plr_id;
  char *text;
  long i,k,n;
  //_DK_command_set_computer_checks(plrname, chkname, a1, a2, a3, a4, a5);

  plr_id = get_id(player_desc, plrname);
  if (plr_id == -1)
  {
    text = buf_sprintf("(script:%lu) invalid player name, '%s'", script_line_number, plrname);
    error(func_name, 1373, text);
    return;
  }
  if (plr_id == 8)
  {
    plr_start = 0;
    plr_end = PLAYERS_COUNT;
  } else
  {
    plr_start = plr_id&PLAYERS_COUNT;
    plr_end = plr_start + 1;
  }
  n = 0;
  for (i=plr_start; i < plr_end; i++)
  {
    for (k=0; k < COMPUTER_CHECKS_COUNT; k++)
    {
      check = &game.computer[i].checks[k];
      if ((check->field_4 & 0x02) != 0)
        break;
      if (check->name == NULL)
        break;
      if (stricmp(chkname, check->name) == 0)
      {
        check->field_8 = a1;
        check->param1 = a2;
        check->param2 = a3;
        check->param3 = a4;
        check->param4 = a5;
        n++;
      }
    }
  }
  if (n == 0)
  {
    text = buf_sprintf("(script:%lu) no computer check found called '%s'", script_line_number, chkname);
    error(func_name, 2732, text);
    return;
  }
  if (script_current_condition != -1)
  {
    LbWarnLog("(script:%lu) Computer check altered inside conditional loop\n",script_line_number);
  }
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void command_set_computer_events(char *plrname, char *evntname, long a1, long a2)
{
  _DK_command_set_computer_events(plrname, evntname, a1, a2);
}

void command_set_computer_process(char *plrname, char *procname, long a1, long a2, long a3, long a4, long a5)
{
  _DK_command_set_computer_process(plrname, procname, a1, a2, a3, a4, a5);
}

void command_set_creature_health(char *crtr_name, long val)
{
  static const char *func_name="command_set_creature_health";
  long crtr_id;
  char *text;
  crtr_id = get_id(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown creature, '%s'", script_line_number, crtr_name);
    error(func_name, 2324, text);
    return;
  }
  if ((val < 0) || (val > 65535))
  {
    text = buf_sprintf("(script:%lu) Invalid '%s' health value, %d", script_line_number, crtr_name, val);
    error(func_name, 2330, text);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_HEALTH, 0, crtr_id, val, 0);
}

void command_set_creature_strength(char *crtr_name, long val)
{
  static const char *func_name="command_set_creature_strength";
  long crtr_id;
  char *text;
  crtr_id = get_id(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown creature, '%s'", script_line_number, crtr_name);
    error(func_name, 2324, text);
    return;
  }
  if ((val < 0) || (val > 255))
  {
    text = buf_sprintf("(script:%lu) Invalid '%s' strength value, %d", script_line_number, crtr_name, val);
    error(func_name, 2330, text);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_STRENGTH, 0, crtr_id, val, 0);
}

void command_set_creature_armour(char *crtr_name, long val)
{
  static const char *func_name="command_set_creature_armour";
  long crtr_id;
  char *text;
  crtr_id = get_id(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown creature, '%s'", script_line_number, crtr_name);
    error(func_name, 2324, text);
    return;
  }
  if ((val < 0) || (val > 255))
  {
    text = buf_sprintf("(script:%lu) Invalid '%s' armour value, %d", script_line_number, crtr_name, val);
    error(func_name, 2330, text);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_ARMOUR, 0, crtr_id, val, 0);
}

void command_set_creature_fear(char *crtr_name, long val)
{
  static const char *func_name="command_set_creature_fear";
  long crtr_id;
  char *text;
  crtr_id = get_id(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown creature, '%s'", script_line_number, crtr_name);
    error(func_name, 2324, text);
    return;
  }
  if ((val < 0) || (val > 255))
  {
    text = buf_sprintf("(script:%lu) Invalid '%s' fear value, %d", script_line_number, crtr_name, val);
    error(func_name, 2330, text);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_FEAR, 0, crtr_id, val, 0);
}

void command_ally_players(char *plr1name, char *plr2name)
{
  static const char *func_name="command_set_creature_max_level";
  long plr1_id,plr2_id;
  char *text;
  plr1_id = get_id(player_desc, plr1name);
  if (plr1_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown first player name, '%s'", script_line_number, plr1name);
    error(func_name, 1456, text);
    return;
  }
  plr2_id = get_id(player_desc, plr2name);
  if (plr2_id == -1)
  {
    text = buf_sprintf("(script:%lu) Unknown second player name, '%s'", script_line_number, plr2name);
    error(func_name, 1456, text);
    return;
  }
  command_add_value(Cmd_ALLY_PLAYERS, 0, plr1_id, plr2_id, 0);
}

void command_quick_objective(int idx, char *msgtext, char *where)
{
  static const char *func_name="command_quick_objective";
  struct Thing *thing;
  long where_id;
  char *text;
  if ((idx < 0) || (idx >= QUICK_MESSAGES_COUNT))
  {
    text = buf_sprintf("(script:%lu) Invalid QUICK OBJECTIVE number (%d)", script_line_number, idx);
    error(func_name, 2105, text);
    return;
  }
  if (strlen(msgtext) > MESSAGE_TEXT_LEN)
  {
    text = buf_sprintf("(script:%lu) QUICK OBJECTIVE too long; truncating", script_line_number);
    error(func_name, 2111, text);
    msgtext[MESSAGE_TEXT_LEN-1] = '\0';
  }
  strcpy(quick_messages[idx], msgtext);
  where_id = get_id(player_desc, where);
  if (where_id == -1)
  {
    where_id = atol(where);
    if (where_id < 0)
    {
      thing = find_hero_gate_of_number(-where_id);
      if (thing_is_invalid(thing))
      {
        text = buf_sprintf("(script:%lu) Nonexisting Hero Door, no %d", script_line_number, -where_id);
        error(func_name, 2152, text);
        return;
      }
      where_id = -1000-thing_get_index(thing);
    } else
    if (where_id > 0)
    {
      if (!action_point_exists_number(where_id))
      {
        text = buf_sprintf("(script:%lu) Nonexisting Action Point, no %d", script_line_number, where_id);
        error(func_name, 2347, text);
        return;
      }
      where_id = -where_id;
    } else
    {
      LbWarnLog("(script:%lu) Invalid QUICK OBJECTIVE where_id = '%s'; reset to my_player.\n",where);
      where_id = my_player_number;
    }
  }
  command_add_value(Cmd_QUICK_OBJECTIVE, 0, idx, where_id, 0);
}

void command_quick_information(int idx, char *msgtext)
{
  static const char *func_name="command_quick_information";
  char *text;
  if ((idx < 0) || (idx >= QUICK_MESSAGES_COUNT))
  {
    text = buf_sprintf("(script:%lu) Invalid QUICK INFORMATION number (%d)", script_line_number, idx);
    error(func_name, 2105, text);
    return;
  }
  if (strlen(msgtext) > MESSAGE_TEXT_LEN)
  {
    text = buf_sprintf("(script:%lu) QUICK INFORMATION text too long; truncating", script_line_number);
    error(func_name, 2111, text);
    msgtext[MESSAGE_TEXT_LEN-1] = '\0';
  }
  strcpy(quick_messages[idx], msgtext);
  command_add_value(Cmd_QUICK_INFORMATION, 0, idx, 0, 0);
}

void command_message(char *msgtext, unsigned char kind)
{
  static const char *func_name="command_message";
  const char *cmd;
  if (kind == 80)
    cmd=script_get_command_name(Cmd_PRINT);
  else
    cmd=script_get_command_name(Cmd_MESSAGE);
  LbWarnLog("(script:%lu) Command '%s' is only supported in Dungeon Keeper Beta\n", script_line_number,cmd);
}

long scan_line(char *line)
{
  static const char *func_name="scan_line";
  const struct CommandDesc *cmd_desc;
  struct ScriptLine *scline;
  unsigned char line_end;
  char *text;
  char chr;
  int i;
#if (BFDEBUG_LEVEL > 12)
  LbSyncLog("%s: Starting for line %d\n",func_name,script_line_number);
#endif
//  return _DK_scan_line(line);
  scline = (struct ScriptLine *)LbMemoryAlloc(sizeof(struct ScriptLine));
  if (scline == NULL)
  {
    text = buf_sprintf("(script:%lu) Can't allocate buffer to recognize line", script_line_number);
    error(func_name, 811, text);
    return 0;
  }
  line_end = false;
  LbMemorySet(scline, 0, sizeof(struct ScriptLine));
  if (next_command_reusable)
    next_command_reusable--;
  cmd_desc = get_next_word(&line, scline->tcmnd, &line_end);
  if (cmd_desc == NULL)
  {
    if ( isalnum(scline->tcmnd[0]) )
    {
      text = buf_sprintf("(script:%lu) Invalid command, '%s'", script_line_number, scline->tcmnd);
      error(func_name, 817, text);
    }
    LbMemoryFree(scline);
    return 0;
  }
#if (BFDEBUG_LEVEL > 12)
  LbSyncLog("%s: Executing command %lu\n",func_name,cmd_desc->index);
#endif
  // Handling comments
  if (cmd_desc->index == Cmd_REM)
  {
    LbMemoryFree(scline);
    return 0;
  }
  // Recognising parameters
  for (i=0; i < COMMANDDESC_ARGS_COUNT; i++)
  {
    chr = cmd_desc->args[i];
    if ((chr == ' ') || (chr == '\0'))
      break;
    if (line_end)
      break;
    get_next_word(&line, scline->tp[i], &line_end);
    if (scline->tp[i][0] == '\0')
      break;
    chr = cmd_desc->args[i];
    if ((chr == 'N') || (chr == 'n'))
    {
      scline->np[i] = strtol(scline->tp[i],&text,0);
      if (text != &scline->tp[i][strlen(scline->tp[i])])
        LbWarnLog("(script:%lu) numerical value '%s' interpreted as %ld\n", script_line_number, scline->tp[i], scline->np[i]);
    }
  }
  if (i < COMMANDDESC_ARGS_COUNT)
  {
    chr = cmd_desc->args[i];
    if ((chr == 'A') || (chr == 'N'))
    {
      text = buf_sprintf("(script:%lu) Invalid number of parameters, %s", script_line_number, cmd_desc->textptr);
      error(func_name, 536, text);
      LbMemoryFree(scline);
      return -1;
    }
  }
  switch (cmd_desc->index)
  {
  case Cmd_CREATE_PARTY:
      command_create_party(scline->tp[0]);
      break;
  case Cmd_ADD_TO_PARTY:
      command_add_to_party(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3], scline->tp[4], scline->np[5]);
      break;
  case Cmd_ADD_PARTY_TO_LEVEL:
      command_add_party_to_level(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_ADD_CREATURE_TO_LEVEL:
      command_add_creature_to_level(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5]);
      break;
  case Cmd_IF:
      command_if(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_ENDIF:
      pop_condition();
      break;
  case Cmd_SET_HATE:
      command_set_hate(scline->np[0], scline->np[1], scline->np[2]);
      break;
  case Cmd_SET_GENERATE_SPEED:
      command_set_generate_speed(scline->np[0]);
      break;
  case Cmd_START_MONEY:
      command_set_start_money(scline->tp[0], scline->np[1]);
      break;
  case Cmd_ROOM_AVAILABLE:
      command_room_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_CREATURE_AVAILABLE:
      command_creature_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_MAGIC_AVAILABLE:
      command_magic_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_TRAP_AVAILABLE:
      command_trap_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_RESEARCH:
      command_research(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_COMPUTER_PLAYER:
      command_computer_player(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_TIMER:
      command_set_timer(scline->tp[0], scline->tp[1]);
      break;
  case Cmd_IF_ACTION_POINT:
      command_if_action_point(scline->np[0], scline->tp[1]);
      break;
  case Cmd_ADD_TUNNELLER_TO_LEVEL:
      command_add_tunneller_to_level(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5]);
      break;
  case Cmd_WIN_GAME:
      command_win_game();
      break;
  case Cmd_LOSE_GAME:
      command_lose_game();
      break;
  case Cmd_SET_FLAG:
      command_set_flag(scline->tp[0], scline->tp[1], scline->np[2]);
      break;
  case Cmd_MAX_CREATURES:
      command_max_creatures(scline->tp[0], scline->np[1]);
      break;
  case Cmd_NEXT_COMMAND_REUSABLE:
      next_command_reusable = 2;
      break;
  case Cmd_DOOR_AVAILABLE:
      command_door_available(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_DISPLAY_OBJECTIVE:
      command_display_objective(scline->np[0], scline->tp[1], -1, -1);
      break;
  case Cmd_DISPLAY_INFORMATION:
      command_display_information(scline->np[0], 0, 0);
      break;
  case Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL:
      command_add_tunneller_party_to_level(scline->tp[0], scline->tp[1], scline->tp[2], scline->tp[3], scline->np[4], scline->np[5], scline->np[6]);
      break;
  case Cmd_ADD_CREATURE_TO_POOL:
      command_add_creature_to_pool(scline->tp[0], scline->np[1]);
      break;
  case Cmd_RESET_ACTION_POINT:
      command_reset_action_point(scline->np[0]);
      break;
  case Cmd_TUTORIAL_FLASH_BUTTON:
      command_tutorial_flash_button(scline->np[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_MAX_LEVEL:
      command_set_creature_max_level(scline->tp[0], scline->tp[1], scline->np[2]);
      break;
  case Cmd_SET_MUSIC:
      command_set_music(scline->np[0]);
      break;
  case Cmd_SET_CREATURE_HEALTH:
      command_set_creature_health(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_STRENGTH:
      command_set_creature_strength(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_ARMOUR:
      command_set_creature_armour(scline->tp[0], scline->np[1]);
      break;
  case Cmd_SET_CREATURE_FEAR:
      command_set_creature_fear(scline->tp[0], scline->np[1]);
      break;
  case Cmd_DISPLAY_OBJECTIVE_WITH_POS:
      command_display_objective(scline->np[0], NULL, scline->np[1], scline->np[2]);
      break;
  case Cmd_IF_AVAILABLE:
      command_if_available(scline->tp[0], scline->tp[1], scline->tp[2], scline->np[3]);
      break;
  case Cmd_SET_COMPUTER_GLOBALS:
      command_set_computer_globals(scline->tp[0], scline->np[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
      break;
  case Cmd_SET_COMPUTER_CHECKS:
      command_set_computer_checks(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
      break;
  case Cmd_SET_COMPUTER_EVENT:
      command_set_computer_events(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3]);
      break;
  case Cmd_SET_COMPUTER_PROCESS:
      command_set_computer_process(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
      break;
  case Cmd_ALLY_PLAYERS:
      command_ally_players(scline->tp[0], scline->tp[1]);
      break;
  case Cmd_DEAD_CREATURES_RETURN_TO_POOL:
      command_dead_creatures_return_to_pool(scline->np[0]);
      break;
  case Cmd_DISPLAY_INFORMATION_WITH_POS:
      command_display_information(scline->np[0], scline->np[1], scline->np[2]);
      break;
  case Cmd_BONUS_LEVEL_TIME:
      command_bonus_level_time(scline->np[0]);
      break;
  case Cmd_QUICK_OBJECTIVE:
      command_quick_objective(scline->np[0], scline->tp[1], scline->tp[2]);
      break;
  case Cmd_QUICK_INFORMATION:
      command_quick_information(scline->np[0], scline->tp[1]);
      break;
  case Cmd_SWAP_CREATURE:
      LbWarnLog("(script:%lu) Command '%s' is only supported in Deeper Dungeons\n", script_line_number,scline->tcmnd);
      break;
  case Cmd_PRINT:
      command_message(scline->tp[0],80);
      break;
  case Cmd_MESSAGE:
      command_message(scline->tp[0],68);
      break;
  default:
      text = buf_sprintf("(script:%lu) Unhandled SCRIPT command '%s'", script_line_number,scline->tcmnd);
      error(func_name, 809, text);
      break;
  }
  LbMemoryFree(scline);
#if (BFDEBUG_LEVEL > 13)
  LbSyncLog("%s: Finished\n",func_name);
#endif
  return 0;
}

short clear_script(void)
{
  memset(&game.script, 0, sizeof(struct LevelScript));
  script_current_condition = -1;
  script_line_number = 1;
  return true;
}

short load_script(long lvnum)
{
  static const char *func_name="load_script";
  char *fname;
  char *buf;
  short fgroup;
  char *buf_end;
  char *script_data;
  int lnlen;
  long script_len;
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_load_script(lvnum);

  gui_set_button_flashing(0, 0);
  clear_script();
  script_current_condition = -1;
  script_line_number = 1;
  game.bonus_time = 0;
  game.flags_cd |= 0x08;

  struct Dungeon *dungeon;
  int i,k;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    for (k=1; k < CREATURE_TYPES_COUNT; k++)
    {
      game.dungeon[i].creature_max_level[k] = 11;
    }
  }

  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
    dungeon->field_1006 = 1;
    dungeon->field_FF2 = 1;
    for (k=0; k<TURN_TIMERS_COUNT; k++)
    {
      memset(&dungeon->turn_timers[k], 0, sizeof(struct TurnTimer));
      dungeon->turn_timers[k].state = 0;
    }
    for (k=0; k<SCRIPT_FLAGS_COUNT; k++)
    {
      dungeon->script_flags[k] = 0;
    }
    
  }

  wait_for_cd_to_be_available();
  fgroup = get_level_fgroup(lvnum);
  fname = prepare_file_fmtpath(fgroup,"map%05d.txt",lvnum);
  script_len = LbFileLengthRnc(fname);
  if (script_len <= 0)
  {
    buf = buf_sprintf("Cannot find SCRIPT file \"%s\"",fname);
    error(func_name, 2765, buf);
    return false;
  }
  if (script_len > 1048576)
  {
    error(func_name, 2766, "Script file is too large to be proper");
    return false;
  }
  script_data = (char *)LbMemoryAlloc(script_len+16);
  if (script_data == NULL)
  {
    error(func_name, 2767, "Cannot allocate memory to load script file");
    return false;
  }
  wait_for_cd_to_be_available();
  if (LbFileLoadAt(fname, script_data) != script_len)
  {
    error(func_name, 2772, "Cannot load SCRIPT file");
    return false;
  }
  buf_end = &script_data[script_len];
  buf = script_data;
  while (buf < buf_end)
  {
    // Find end of the line
    lnlen = 0;
    while (&buf[lnlen] < buf_end)
    {
      if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
        break;
      lnlen++;
    }
    // Get rid of the next line characters
    buf[lnlen] = 0;
    lnlen++;
    if (&buf[lnlen] < buf_end)
    {
      if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
        lnlen++;
    }
    // Analyze the line
    scan_line(buf);
    // Set new line start
    script_line_number++;
    buf += lnlen;
  }
  LbMemoryFree(script_data);
  if (game.script.win_conditions_num == 0)
    LbWarnLog("No WIN GAME conditions in script file.\n");
  if (script_current_condition != -1)
    LbWarnLog("Missing ENDIF's in script file.\n");
  return true;
}

void script_process_win_game(unsigned short plyr_idx)
{
  struct PlayerInfo *player;
  player = &(game.players[plyr_idx%PLAYERS_COUNT]);
  set_player_as_won_level(player);
}

void script_process_lose_game(unsigned short plyr_idx)
{
  struct PlayerInfo *player;
  player = &(game.players[plyr_idx%PLAYERS_COUNT]);
  set_player_as_lost_level(player);
}

struct Thing *script_process_new_tunneller(unsigned char a1, long a2, unsigned char a3, long a4, unsigned char a5, unsigned long a6)
{
  return _DK_script_process_new_tunneller(a1, a2, a3, a4, a5, a6);
}

struct Thing *script_process_new_party(struct Party *party, unsigned char a2, long a3, long a4)
{
  return _DK_script_process_new_party(party, a2, a3, a4);
}

void script_process_new_creatures(unsigned char a1, long a2, long a3, long a4, long a5, long a6)
{
  _DK_script_process_new_creatures(a1, a2, a3, a4, a5, a6);
}

unsigned char script_support_action_point_activated_by_players(long apt_idx)
{
  return game.action_points[apt_idx].field_9;
}

long process_activation_status(struct Condition *condt)
{
  if (!script_support_action_point_activated_by_players(condt->field_5))
    return false;
  if (condt->field_3 == 8)
    return 1;
  else
    return (1 << condt->field_3);
}

/*
 * Function name spelling after the one in original game.
 */
TbBool process_avialable_status(long plyr_idx, long itype, long ikind, long val)
{
  static const char *func_name="process_avialable_status";
  struct Dungeon *dungeon;
  long i;
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  switch (itype)
  {
  case 30:
      i = dungeon->field_FE0[ikind];
      break;
  case 31:
      i = dungeon->trap_amount[ikind%TRAP_TYPES_COUNT];
      break;
  case 32:
      i = dungeon->door_amount[ikind%DOOR_TYPES_COUNT];
      break;
  case 33:
      i = dungeon->room_buildable[ikind%ROOM_TYPES_COUNT];
      break;
  default:
      LbWarnLog("%s: Unrecognized item type, no %d\n",func_name,itype);
      return false;
  }
  return (i == val);
}

void script_process_value(unsigned long var_index, unsigned long plr_id, long val2, long val3, long val4)
{
  struct CreatureStats *crstat;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  int plr_start, plr_end;
  long i;
//  _DK_script_process_value(var_index, plr_id, val2, val3, val4);
  if (plr_id == 8)
  {
    plr_start = 0;
    plr_end = PLAYERS_COUNT;
  } else
  {
    plr_start = plr_id;
    plr_end = plr_id+1;
  }
  switch (var_index)
  {
  case Cmd_SET_HATE:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->hates_player[val2%DUNGEONS_COUNT] = val3;
      }
      break;
  case Cmd_SET_GENERATE_SPEED:
      game.generate_speed = saturate_set_unsigned(val2, 16);
      break;
  case Cmd_ROOM_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->room_resrchable[val2%ROOM_TYPES_COUNT] = val3;
        dungeon->room_buildable[val2%ROOM_TYPES_COUNT] = val4;
      }
      break;
  case Cmd_CREATURE_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->creature_allowed[val2] = val3;
        dungeon->creature_enabled[val2] = val4;
      }
      break;
  case Cmd_MAGIC_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->field_FF4[val2] = val3;
        if (val4 != 0)
          add_spell_to_player(val2, i);
        else
          dungeon->field_FE0[val2] = val4;
      }
      break;
  case Cmd_TRAP_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->trap_buildable[val2%TRAP_TYPES_COUNT] = val3;
        dungeon->trap_amount[val2%TRAP_TYPES_COUNT] = val4;
        if (val4 != 0)
          dungeon->trap_placeable[val2%TRAP_TYPES_COUNT] = val4;
      }
      break;
  case Cmd_RESEARCH:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        if (!dungeon->field_14AD)
          remove_all_research_from_player(i);
        add_research_to_player(i, val2, val3, val4);
      }
      break;
  case Cmd_SET_TIMER:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->turn_timers[val2%TURN_TIMERS_COUNT].state = 1;
        dungeon->turn_timers[val2%TURN_TIMERS_COUNT].count = game.seedchk_random_used;
      }
      break;
  case Cmd_SET_FLAG:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->script_flags[val2%SCRIPT_FLAGS_COUNT] = val3;
      }
      break;
  case Cmd_MAX_CREATURES:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->max_creatures = val2;
      }
      break;
  case Cmd_DOOR_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->door_buildable[val2%DOOR_TYPES_COUNT] = val3;
        dungeon->door_amount[val2%DOOR_TYPES_COUNT] = val4;
        if (val4 != 0)
          dungeon->door_placeable[val2%DOOR_TYPES_COUNT] = val4;
      }
      break;
  case Cmd_DISPLAY_OBJECTIVE:
      process_objective(campaign.strings[val2%STRINGS_MAX], val3, val4/(map_subtiles_x+1), val4%(map_subtiles_x+1));
      break;
  case Cmd_DISPLAY_INFORMATION:
      set_general_information(val2, val3, val4);
      break;
  case Cmd_ADD_CREATURE_TO_POOL:
      add_creature_to_pool(val2, val3, 0);
      break;
  case Cmd_RESET_ACTION_POINT:
      action_point_reset_idx(val2);
      break;
  case Cmd_TUTORIAL_FLASH_BUTTON:
      gui_set_button_flashing(val2, val3);
      break;
  case Cmd_SET_CREATURE_MAX_LEVEL:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
        dungeon->creature_max_level[val2%CREATURE_TYPES_COUNT] = val3;
      }
      break;
  case Cmd_SET_CREATURE_HEALTH:
      crstat = &game.creature_stats[val2%CREATURE_TYPES_COUNT];
      crstat->health = saturate_set_signed(val3, 16);
      break;
  case Cmd_SET_CREATURE_STRENGTH:
      crstat = &game.creature_stats[val2%CREATURE_TYPES_COUNT];
      crstat->strength = saturate_set_unsigned(val3, 8);
      break;
  case Cmd_SET_CREATURE_ARMOUR:
      crstat = &game.creature_stats[val2%CREATURE_TYPES_COUNT];
      crstat->armour = saturate_set_unsigned(val3, 8);
      break;
  case Cmd_SET_CREATURE_FEAR:
      crstat = &game.creature_stats[val2%CREATURE_TYPES_COUNT];
      crstat->fear = saturate_set_unsigned(val3, 8);
      break;
  case Cmd_ALLY_PLAYERS:
      toggle_ally_with_player(val2, val3);
      toggle_ally_with_player(val3, val2);
      break;
  case Cmd_DEAD_CREATURES_RETURN_TO_POOL:
      set_flag_byte(&game.flags_cd, 0x08, val2);
      break;
  case Cmd_BONUS_LEVEL_TIME:
      game.bonus_time = val2;
      break;
  case Cmd_QUICK_OBJECTIVE:
      process_objective(quick_messages[val2%QUICK_MESSAGES_COUNT], val3, 0, 0);
      break;
  case Cmd_QUICK_INFORMATION:
      set_quick_information(val2, 0, 0);
      break;
  default:
      LbWarnLog("Unsupported Game VALUE, type %d.\n",var_index);
      break;
  }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif

