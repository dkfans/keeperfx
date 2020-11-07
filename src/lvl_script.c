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
 * @date     12 Feb 2009 - 11 Apr 2014
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
#include "bflib_sound.h"
#include "bflib_math.h"
#include "bflib_guibtns.h"

#include "front_simple.h"
#include "config.h"
#include "config_crtrmodel.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "config_rules.h"
#include "config_lenses.h"
#include "config_magic.h"
#include "config_creature.h"
#include "config_effects.h"
#include "gui_soundmsgs.h"
#include "frontmenu_ingame_tabs.h"
#include "player_instances.h"
#include "player_data.h"
#include "player_utils.h"
#include "thing_factory.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "creature_states.h"
#include "creature_states_hero.h"
#include "creature_groups.h"
#include "power_hand.h"
#include "room_library.h"
#include "room_entrance.h"
#include "room_util.h"
#include "magic.h"
#include "map_blocks.h"
#include "lvl_filesdk1.h"
#include "frontend.h"
#include "game_merge.h"
#include "dungeon_data.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "music_player.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/**
 * Descriptions of script commands for parser.
 * Arguments are: A-string, N-integer, C-creature model, P- player, R- room kind, L- location, O- operator, S- slab kind, X- creature property
 * Lower case letters are optional arguments.
 */
const struct CommandDesc command_desc[] = {
  {"CREATE_PARTY",                      "A       ", Cmd_CREATE_PARTY},
  {"ADD_TO_PARTY",                      "ACNNAN  ", Cmd_ADD_TO_PARTY},
  {"ADD_PARTY_TO_LEVEL",                "PAAN    ", Cmd_ADD_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_LEVEL",             "PCANNN  ", Cmd_ADD_CREATURE_TO_LEVEL},
  {"IF",                                "PAON    ", Cmd_IF},
  {"IF_ACTION_POINT",                   "NP      ", Cmd_IF_ACTION_POINT},
  {"ENDIF",                             "        ", Cmd_ENDIF},
  {"SET_HATE",                          "PPN     ", Cmd_SET_HATE},
  {"SET_GENERATE_SPEED",                "N       ", Cmd_SET_GENERATE_SPEED},
  {"REM",                               "        ", Cmd_REM},
  {"START_MONEY",                       "PN      ", Cmd_START_MONEY},
  {"ROOM_AVAILABLE",                    "PRNN    ", Cmd_ROOM_AVAILABLE},
  {"CREATURE_AVAILABLE",                "PCNN    ", Cmd_CREATURE_AVAILABLE},
  {"MAGIC_AVAILABLE",                   "PANN    ", Cmd_MAGIC_AVAILABLE},
  {"TRAP_AVAILABLE",                    "PANN    ", Cmd_TRAP_AVAILABLE},
  {"RESEARCH",                          "PAAN    ", Cmd_RESEARCH},
  {"RESEARCH_ORDER",                    "PAAN    ", Cmd_RESEARCH_ORDER},
  {"COMPUTER_PLAYER",                   "PN      ", Cmd_COMPUTER_PLAYER},
  {"SET_TIMER",                         "PA      ", Cmd_SET_TIMER},
  {"ADD_TUNNELLER_TO_LEVEL",            "PAANNN  ", Cmd_ADD_TUNNELLER_TO_LEVEL},
  {"WIN_GAME",                          "        ", Cmd_WIN_GAME},
  {"LOSE_GAME",                         "        ", Cmd_LOSE_GAME},
  {"SET_FLAG",                          "PAN     ", Cmd_SET_FLAG},
  {"MAX_CREATURES",                     "PN      ", Cmd_MAX_CREATURES},
  {"NEXT_COMMAND_REUSABLE",             "        ", Cmd_NEXT_COMMAND_REUSABLE},
  {"DOOR_AVAILABLE",                    "PANN    ", Cmd_DOOR_AVAILABLE},
  {"DISPLAY_OBJECTIVE",                 "NL      ", Cmd_DISPLAY_OBJECTIVE},
  {"DISPLAY_OBJECTIVE_WITH_POS",        "NNN     ", Cmd_DISPLAY_OBJECTIVE_WITH_POS},
  {"DISPLAY_INFORMATION",               "NL      ", Cmd_DISPLAY_INFORMATION},
  {"DISPLAY_INFORMATION_WITH_POS",      "NNN     ", Cmd_DISPLAY_INFORMATION_WITH_POS},
  {"ADD_TUNNELLER_PARTY_TO_LEVEL",      "PAAANNN ", Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_POOL",              "CN      ", Cmd_ADD_CREATURE_TO_POOL},
  {"RESET_ACTION_POINT",                "N       ", Cmd_RESET_ACTION_POINT},
  {"SET_CREATURE_MAX_LEVEL",            "PCN     ", Cmd_SET_CREATURE_MAX_LEVEL},
  {"SET_MUSIC",                         "N       ", Cmd_SET_MUSIC},
  {"TUTORIAL_FLASH_BUTTON",             "NN      ", Cmd_TUTORIAL_FLASH_BUTTON},
  {"SET_CREATURE_STRENGTH",             "CN      ", Cmd_SET_CREATURE_STRENGTH},
  {"SET_CREATURE_HEALTH",               "CN      ", Cmd_SET_CREATURE_HEALTH},
  {"SET_CREATURE_ARMOUR",               "CN      ", Cmd_SET_CREATURE_ARMOUR},
  {"SET_CREATURE_FEAR_WOUNDED",         "CN      ", Cmd_SET_CREATURE_FEAR_WOUNDED},
  {"SET_CREATURE_FEAR_STRONGER",        "CN      ", Cmd_SET_CREATURE_FEAR_STRONGER},
  {"SET_CREATURE_FEARSOME_FACTOR",      "CN      ", Cmd_SET_CREATURE_FEARSOME_FACTOR},
  {"SET_CREATURE_PROPERTY",             "CXN     ", Cmd_SET_CREATURE_PROPERTY},
  {"IF_AVAILABLE",                      "PAON    ", Cmd_IF_AVAILABLE},
  {"IF_CONTROLS",                       "PAON    ", Cmd_IF_CONTROLS},
  {"SET_COMPUTER_GLOBALS",              "PNNNNNN ", Cmd_SET_COMPUTER_GLOBALS},
  {"SET_COMPUTER_CHECKS",               "PANNNNN ", Cmd_SET_COMPUTER_CHECKS},
  {"SET_COMPUTER_EVENT",                "PANNNNN ", Cmd_SET_COMPUTER_EVENT},
  {"SET_COMPUTER_PROCESS",              "PANNNNN ", Cmd_SET_COMPUTER_PROCESS},
  {"ALLY_PLAYERS",                      "PPN     ", Cmd_ALLY_PLAYERS},
  {"DEAD_CREATURES_RETURN_TO_POOL",     "N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL},
  {"BONUS_LEVEL_TIME",                  "N       ", Cmd_BONUS_LEVEL_TIME},
  {"QUICK_OBJECTIVE",                   "NAL     ", Cmd_QUICK_OBJECTIVE},
  {"QUICK_INFORMATION",                 "NAL     ", Cmd_QUICK_INFORMATION},
  {"QUICK_OBJECTIVE_WITH_POS",          "NANN    ", Cmd_QUICK_OBJECTIVE_WITH_POS},
  {"QUICK_INFORMATION_WITH_POS",        "NANN    ", Cmd_QUICK_INFORMATION_WITH_POS},
  {"SWAP_CREATURE",                     "AC      ", Cmd_SWAP_CREATURE},
  {"PRINT",                             "A       ", Cmd_PRINT},
  {"MESSAGE",                           "A       ", Cmd_MESSAGE},
  {"PLAY_MESSAGE",                      "PAN     ", Cmd_PLAY_MESSAGE},
  {"ADD_GOLD_TO_PLAYER",                "PN      ", Cmd_ADD_GOLD_TO_PLAYER},
  {"SET_CREATURE_TENDENCIES",           "PAN     ", Cmd_SET_CREATURE_TENDENCIES},
  {"REVEAL_MAP_RECT",                   "PNNNN   ", Cmd_REVEAL_MAP_RECT},
  {"REVEAL_MAP_LOCATION",               "PNN     ", Cmd_REVEAL_MAP_LOCATION},
  {"LEVEL_VERSION",                     "N       ", Cmd_LEVEL_VERSION},
  {"KILL_CREATURE",                     "PCAN    ", Cmd_KILL_CREATURE},
  {"USE_POWER_ON_CREATURE",             "PCAPANN ", Cmd_USE_POWER_ON_CREATURE},
  {"USE_POWER_AT_LOCATION",             "PNNANN  ", Cmd_USE_POWER_AT_LOCATION},
  {"USE_POWER",                         "PAN     ", Cmd_USE_POWER},
  {"ADD_TO_FLAG",                       "PAN     ", Cmd_ADD_TO_FLAG},
  {"SET_CAMPAIGN_FLAG",                 "PAN     ", Cmd_SET_CAMPAIGN_FLAG},
  {"ADD_TO_CAMPAIGN_FLAG",              "PAN     ", Cmd_ADD_TO_CAMPAIGN_FLAG},
  {"EXPORT_VARIABLE",                   "PAA     ", Cmd_EXPORT_VARIABLE},
  {"RUN_AFTER_VICTORY",                 "N       ", Cmd_RUN_AFTER_VICTORY},
  {"LEVEL_UP_CREATURE",                 "PCAN    ", Cmd_LEVEL_UP_CREATURE},
  {"CHANGE_CREATURE_OWNER",             "PCAP    ", Cmd_CHANGE_CREATURE_OWNER},
  {"SET_GAME_RULE",                     "AN      ", Cmd_SET_GAME_RULE},
  {"SET_TRAP_CONFIGURATION",            "ANNNNNNN", Cmd_SET_TRAP_CONFIGURATION},
  {"SET_DOOR_CONFIGURATION",            "ANNNN   ", Cmd_SET_DOOR_CONFIGURATION},
  {"CHANGE_SLAB_OWNER",                 "NNP     ", Cmd_CHANGE_SLAB_OWNER},
  {"CHANGE_SLAB_TYPE",                  "NNS     ", Cmd_CHANGE_SLAB_TYPE},
  {"IF_SLAB_OWNER",                     "NNP     ", Cmd_IF_SLAB_OWNER},
  {"IF_SLAB_TYPE",                      "NNS     ", Cmd_IF_SLAB_TYPE},
  {NULL,                                "        ", Cmd_NONE},
};

const struct CommandDesc dk1_command_desc[] = {
  {"CREATE_PARTY",                 "A       ", Cmd_CREATE_PARTY},
  {"ADD_TO_PARTY",                 "ACNNAN  ", Cmd_ADD_TO_PARTY},
  {"ADD_PARTY_TO_LEVEL",           "PAAN    ", Cmd_ADD_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_LEVEL",        "PCANNN  ", Cmd_ADD_CREATURE_TO_LEVEL},
  {"IF",                           "PAON    ", Cmd_IF},
  {"IF_ACTION_POINT",              "NP      ", Cmd_IF_ACTION_POINT},
  {"ENDIF",                        "        ", Cmd_ENDIF},
  {"SET_HATE",                     "PPN     ", Cmd_SET_HATE},
  {"SET_GENERATE_SPEED",           "N       ", Cmd_SET_GENERATE_SPEED},
  {"REM",                          "        ", Cmd_REM},
  {"START_MONEY",                  "PN      ", Cmd_START_MONEY},
  {"ROOM_AVAILABLE",               "PRNN    ", Cmd_ROOM_AVAILABLE},
  {"CREATURE_AVAILABLE",           "PCNN    ", Cmd_CREATURE_AVAILABLE},
  {"MAGIC_AVAILABLE",              "PANN    ", Cmd_MAGIC_AVAILABLE},
  {"TRAP_AVAILABLE",               "PANN    ", Cmd_TRAP_AVAILABLE},
  {"RESEARCH",                     "PAAN    ", Cmd_RESEARCH_ORDER},
  {"COMPUTER_PLAYER",              "PN      ", Cmd_COMPUTER_PLAYER},
  {"SET_TIMER",                    "PA      ", Cmd_SET_TIMER},
  {"ADD_TUNNELLER_TO_LEVEL",       "PAANNN  ", Cmd_ADD_TUNNELLER_TO_LEVEL},
  {"WIN_GAME",                     "        ", Cmd_WIN_GAME},
  {"LOSE_GAME",                    "        ", Cmd_LOSE_GAME},
  {"SET_FLAG",                     "PAN     ", Cmd_SET_FLAG},
  {"MAX_CREATURES",                "PN      ", Cmd_MAX_CREATURES},
  {"NEXT_COMMAND_REUSABLE",        "        ", Cmd_NEXT_COMMAND_REUSABLE},
  {"DOOR_AVAILABLE",               "PANN    ", Cmd_DOOR_AVAILABLE},
  {"DISPLAY_OBJECTIVE",            "NA      ", Cmd_DISPLAY_OBJECTIVE},
  {"DISPLAY_OBJECTIVE_WITH_POS",   "NNN     ", Cmd_DISPLAY_OBJECTIVE_WITH_POS},
  {"DISPLAY_INFORMATION",          "N       ", Cmd_DISPLAY_INFORMATION},
  {"DISPLAY_INFORMATION_WITH_POS", "NNN     ", Cmd_DISPLAY_INFORMATION_WITH_POS},
  {"ADD_TUNNELLER_PARTY_TO_LEVEL", "PAAANNN ", Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL},
  {"ADD_CREATURE_TO_POOL",         "CN      ", Cmd_ADD_CREATURE_TO_POOL},
  {"RESET_ACTION_POINT",           "N       ", Cmd_RESET_ACTION_POINT},
  {"SET_CREATURE_MAX_LEVEL",       "PCN     ", Cmd_SET_CREATURE_MAX_LEVEL},
  {"SET_MUSIC",                    "N       ", Cmd_SET_MUSIC},
  {"TUTORIAL_FLASH_BUTTON",        "NN      ", Cmd_TUTORIAL_FLASH_BUTTON},
  {"SET_CREATURE_STRENGTH",        "CN      ", Cmd_SET_CREATURE_STRENGTH},
  {"SET_CREATURE_HEALTH",          "CN      ", Cmd_SET_CREATURE_HEALTH},
  {"SET_CREATURE_ARMOUR",          "CN      ", Cmd_SET_CREATURE_ARMOUR},
  {"SET_CREATURE_FEAR",            "CN      ", Cmd_SET_CREATURE_FEAR_WOUNDED},
  {"IF_AVAILABLE",                 "PAON    ", Cmd_IF_AVAILABLE},
  {"SET_COMPUTER_GLOBALS",         "PNNNNNN ", Cmd_SET_COMPUTER_GLOBALS},
  {"SET_COMPUTER_CHECKS",          "PANNNNN ", Cmd_SET_COMPUTER_CHECKS},
  {"SET_COMPUTER_EVENT",           "PANN    ", Cmd_SET_COMPUTER_EVENT},
  {"SET_COMPUTER_PROCESS",         "PANNNNN ", Cmd_SET_COMPUTER_PROCESS},
  {"ALLY_PLAYERS",                 "PP      ", Cmd_ALLY_PLAYERS},
  {"DEAD_CREATURES_RETURN_TO_POOL","N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL},
  {"BONUS_LEVEL_TIME",             "N       ", Cmd_BONUS_LEVEL_TIME},
  {"QUICK_OBJECTIVE",              "NAA     ", Cmd_QUICK_OBJECTIVE},
  {"QUICK_INFORMATION",            "NA      ", Cmd_QUICK_INFORMATION},
  {"SWAP_CREATURE",                "AC      ", Cmd_SWAP_CREATURE},
  {"PRINT",                        "A       ", Cmd_PRINT},
  {"MESSAGE",                      "A       ", Cmd_MESSAGE},
  {"LEVEL_VERSION",                "N       ", Cmd_LEVEL_VERSION},
  {NULL,                           "        ", Cmd_NONE},
};

const struct CommandDesc subfunction_desc[] = {
    {"RANDOM",                     "Aaaaaaaa", Cmd_RANDOM},
    {"DRAWFROM",                   "Aaaaaaaa", Cmd_DRAWFROM},
    {"IMPORT",                     "PA      ", Cmd_IMPORT},
    {NULL,                         "        ", Cmd_NONE},
  };

const struct NamedCommand newcrtr_desc[] = {
  {"NEW_CREATURE_A",   1},
  {"NEW_CREATURE_B",   2},
  {NULL,               0},
};

const struct NamedCommand player_desc[] = {
  {"PLAYER0",          PLAYER0},
  {"PLAYER1",          PLAYER1},
  {"PLAYER2",          PLAYER2},
  {"PLAYER3",          PLAYER3},
  {"PLAYER_GOOD",      PLAYER_GOOD},
  {"ALL_PLAYERS",      ALL_PLAYERS},
  {"PLAYER_NEUTRAL",   PLAYER_NEUTRAL},
  {NULL,               0},
};

const struct NamedCommand variable_desc[] = {
    {"MONEY",                       SVar_MONEY},
    {"GAME_TURN",                   SVar_GAME_TURN},
    {"BREAK_IN",                    SVar_BREAK_IN},
    //{"CREATURE_NUM",              SVar_CREATURE_NUM},
    {"TOTAL_DIGGERS",               SVar_TOTAL_DIGGERS},
    {"TOTAL_CREATURES",             SVar_TOTAL_CREATURES},
    {"TOTAL_RESEARCH",              SVar_TOTAL_RESEARCH},
    {"TOTAL_DOORS",                 SVar_TOTAL_DOORS},
    {"TOTAL_AREA",                  SVar_TOTAL_AREA},
    {"TOTAL_CREATURES_LEFT",        SVar_TOTAL_CREATURES_LEFT},
    {"CREATURES_ANNOYED",           SVar_CREATURES_ANNOYED},
    {"BATTLES_LOST",                SVar_BATTLES_LOST},
    {"BATTLES_WON",                 SVar_BATTLES_WON},
    {"ROOMS_DESTROYED",             SVar_ROOMS_DESTROYED},
    {"SPELLS_STOLEN",               SVar_SPELLS_STOLEN},
    {"TIMES_BROKEN_INTO",           SVar_TIMES_BROKEN_INTO},
    {"GOLD_POTS_STOLEN",            SVar_GOLD_POTS_STOLEN},
    {"HEART_HEALTH",                SVar_HEART_HEALTH},
    {"GHOSTS_RAISED",               SVar_GHOSTS_RAISED},
    {"SKELETONS_RAISED",            SVar_SKELETONS_RAISED},
    {"VAMPIRES_RAISED",             SVar_VAMPIRES_RAISED},
    {"CREATURES_CONVERTED",         SVar_CREATURES_CONVERTED},
    {"TIMES_ANNOYED_CREATURE",      SVar_TIMES_ANNOYED_CREATURE},
    {"TIMES_TORTURED_CREATURE",     SVar_TIMES_TORTURED_CREATURE},
    {"TOTAL_DOORS_MANUFACTURED",    SVar_TOTAL_DOORS_MANUFACTURED},
    {"TOTAL_TRAPS_MANUFACTURED",    SVar_TOTAL_TRAPS_MANUFACTURED},
    {"TOTAL_MANUFACTURED",          SVar_TOTAL_MANUFACTURED},
    {"TOTAL_TRAPS_USED",            SVar_TOTAL_TRAPS_USED},
    {"TOTAL_DOORS_USED",            SVar_TOTAL_DOORS_USED},
    {"KEEPERS_DESTROYED",           SVar_KEEPERS_DESTROYED},
    {"CREATURES_SACRIFICED",        SVar_CREATURES_SACRIFICED},
    {"CREATURES_FROM_SACRIFICE",    SVar_CREATURES_FROM_SACRIFICE},
    {"TIMES_LEVELUP_CREATURE",      SVar_TIMES_LEVELUP_CREATURE},
    {"TOTAL_SALARY",                SVar_TOTAL_SALARY},
    {"CURRENT_SALARY",              SVar_CURRENT_SALARY},
    //{"TIMER",                     SVar_TIMER},
    {"DUNGEON_DESTROYED",           SVar_DUNGEON_DESTROYED},
    {"TOTAL_GOLD_MINED",            SVar_TOTAL_GOLD_MINED},
    //{"FLAG",                      SVar_FLAG},
    //{"ROOM",                      SVar_ROOM_SLABS},
    {"DOORS_DESTROYED",             SVar_DOORS_DESTROYED},
    {"CREATURES_SCAVENGED_LOST",    SVar_CREATURES_SCAVENGED_LOST},
    {"CREATURES_SCAVENGED_GAINED",  SVar_CREATURES_SCAVENGED_GAINED},
    {"ALL_DUNGEONS_DESTROYED",      SVar_ALL_DUNGEONS_DESTROYED},
    //{"DOOR",                      SVar_DOOR_NUM},
    {"GOOD_CREATURES",              SVar_GOOD_CREATURES},
    {"EVIL_CREATURES",              SVar_EVIL_CREATURES},
    {NULL,                           0},
};

const struct NamedCommand dk1_variable_desc[] = {
    {"MONEY",                       SVar_MONEY},
    {"GAME_TURN",                   SVar_GAME_TURN},
    {"BREAK_IN",                    SVar_BREAK_IN},
    //{"CREATURE_NUM",                SVar_CREATURE_NUM},
    {"TOTAL_IMPS",                  SVar_TOTAL_DIGGERS},
    {"TOTAL_CREATURES",             SVar_CONTROLS_TOTAL_CREATURES},
    {"TOTAL_RESEARCH",              SVar_TOTAL_RESEARCH},
    {"TOTAL_DOORS",                 SVar_TOTAL_DOORS},
    {"TOTAL_AREA",                  SVar_TOTAL_AREA},
    {"TOTAL_CREATURES_LEFT",        SVar_TOTAL_CREATURES_LEFT},
    {"CREATURES_ANNOYED",           SVar_CREATURES_ANNOYED},
    {"BATTLES_LOST",                SVar_BATTLES_LOST},
    {"BATTLES_WON",                 SVar_BATTLES_WON},
    {"ROOMS_DESTROYED",             SVar_ROOMS_DESTROYED},
    {"SPELLS_STOLEN",               SVar_SPELLS_STOLEN},
    {"TIMES_BROKEN_INTO",           SVar_TIMES_BROKEN_INTO},
    {"GOLD_POTS_STOLEN",            SVar_GOLD_POTS_STOLEN},
    //{"TIMER",                     SVar_TIMER},
    {"DUNGEON_DESTROYED",           SVar_DUNGEON_DESTROYED},
    {"TOTAL_GOLD_MINED",            SVar_TOTAL_GOLD_MINED},
    //{"FLAG",                      SVar_FLAG},
    //{"ROOM",                      SVar_ROOM_SLABS},
    {"DOORS_DESTROYED",             SVar_DOORS_DESTROYED},
    {"CREATURES_SCAVENGED_LOST",    SVar_CREATURES_SCAVENGED_LOST},
    {"CREATURES_SCAVENGED_GAINED",  SVar_CREATURES_SCAVENGED_GAINED},
    {"ALL_DUNGEONS_DESTROYED",      SVar_ALL_DUNGEONS_DESTROYED},
    //{"DOOR",                      SVar_DOOR_NUM},
    {NULL,                           0},
};

const struct NamedCommand controls_variable_desc[] = {
    {"TOTAL_DIGGERS",               SVar_CONTROLS_TOTAL_DIGGERS},
    {"TOTAL_CREATURES",             SVar_CONTROLS_TOTAL_CREATURES},
    {"TOTAL_DOORS",                 SVar_TOTAL_DOORS},
    {"TOTAL_AREA",                  SVar_TOTAL_AREA},
    {"GOOD_CREATURES",              SVar_CONTROLS_GOOD_CREATURES},
    {"EVIL_CREATURES",              SVar_CONTROLS_EVIL_CREATURES},
    {NULL,                           0},
};

const struct NamedCommand comparison_desc[] = {
  {"==",     MOp_EQUAL},
  {"!=",     MOp_NOT_EQUAL},
  {"<",      MOp_SMALLER},
  {">",      MOp_GREATER},
  {"<=",     MOp_SMALLER_EQ},
  {">=",     MOp_GREATER_EQ},
  {NULL,     0},
};

const struct NamedCommand head_for_desc[] = {
  {"ACTION_POINT",         MLoc_ACTIONPOINT},
  {"DUNGEON",              MLoc_PLAYERSDUNGEON},
  {"DUNGEON_HEART",        MLoc_PLAYERSHEART},
  {"APPROPIATE_DUNGEON",   MLoc_APPROPRTDUNGEON},
  {NULL,                   0},
};

const struct NamedCommand timer_desc[] = {
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

const struct NamedCommand flag_desc[] = {
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

const struct NamedCommand hero_objective_desc[] = {
  {"STEAL_GOLD",           CHeroTsk_StealGold},
  {"STEAL_SPELLS",         CHeroTsk_StealSpells},
  {"ATTACK_ENEMIES",       CHeroTsk_AttackEnemies},
  {"ATTACK_DUNGEON_HEART", CHeroTsk_AttackDnHeart},
  {"ATTACK_ROOMS",         CHeroTsk_AttackRooms},
  {"DEFEND_PARTY",         CHeroTsk_DefendParty},
  {"DEFEND_LOCATION",      CHeroTsk_DefendSpawn},
  {"DEFEND_HEART",         CHeroTsk_DefendHeart},
  {"DEFEND_ROOMS",         CHeroTsk_DefendRooms},
  {NULL,                   0},
};

const struct NamedCommand msgtype_desc[] = {
  {"SPEECH",           1},
  {"SOUND",            2},
  {NULL,               0},
};

const struct NamedCommand tendency_desc[] = {
  {"IMPRISON",         1},
  {"FLEE",             2},
  {NULL,               0},
};

const struct NamedCommand creature_select_criteria_desc[] = {
  {"MOST_EXPERIENCED",     CSelCrit_MostExperienced},
  {"MOST_EXP_WANDERING",   CSelCrit_MostExpWandering},
  {"MOST_EXP_WORKING",     CSelCrit_MostExpWorking},
  {"MOST_EXP_FIGHTING",    CSelCrit_MostExpFighting},
  {"LEAST_EXPERIENCED",    CSelCrit_LeastExperienced},
  {"LEAST_EXP_WANDERING",  CSelCrit_LeastExpWandering},
  {"LEAST_EXP_WORKING",    CSelCrit_LeastExpWorking},
  {"LEAST_EXP_FIGHTING",   CSelCrit_LeastExpFighting},
  {"NEAR_OWN_HEART",       CSelCrit_NearOwnHeart},
  {"NEAR_ENEMY_HEART",     CSelCrit_NearEnemyHeart},
  {"ON_ENEMY_GROUND",      CSelCrit_OnEnemyGround},
  {"ON_FRIENDLY_GROUND",   CSelCrit_OnFriendlyGround},
  {"ANYWHERE",             CSelCrit_Any},
  {NULL,                   0},
};

const struct NamedCommand game_rule_desc[] = {
  {"BodiesForVampire",         1},
  {"PrisonSkeletonChance",     2},
  {"GhostConvertChance",       3},
  {"TortureConvertChance",     4},
  {"TortureDeathChance",       5},
  {"FoodGenerationSpeed",      6},
  {"StunEvilEnemyChance",      7},
  {"StunGoodEnemyChance",      8},
  {"BodyRemainsFor",           9},
  {"FightHateKillValue",      10},
  {"PreserveClassicBugs",     11},
  {"DungeonHeartHealHealth",  12},
  {"ImpWorkExperience",       13},
  {"GemEffectiveness",        14},
  {"RoomSellGoldBackPercent", 15},
  {"PayDayGap",               16},
  {"PayDaySpeed",             17},
  {"PayDayProgress",          18},
  {"PlaceTrapsOnSubtiles",    19},
  {NULL,                      0},
};

/**
 * Text names of groups of GUI Buttons.
 */
const struct NamedCommand gui_button_group_desc[] = {
  {"MINIMAP",         GID_MINIMAP_AREA},
  {"TABS",            GID_TABS_AREA},
  {"INFO",            GID_INFO_PANE},
  {"ROOM",            GID_ROOM_PANE},
  {"POWER",           GID_POWER_PANE},
  {"TRAP",            GID_TRAP_PANE},
  {"DOOR",            GID_DOOR_PANE},
  {"CREATURE",        GID_CREATR_PANE},
  {"MESSAGE",         GID_MESSAGE_AREA},
  {NULL,               0},
};

/**
 * Text names of campaign flags.
 */
const struct NamedCommand campaign_flag_desc[] = {
  {"CAMPAIGN_FLAG0",  0},
  {"CAMPAIGN_FLAG1",  1},
  {"CAMPAIGN_FLAG2",  2},
  {"CAMPAIGN_FLAG3",  3},
  {"CAMPAIGN_FLAG4",  4},
  {"CAMPAIGN_FLAG5",  5},
  {"CAMPAIGN_FLAG6",  6},
  {"CAMPAIGN_FLAG7",  7},
  {NULL,     0},
};

/******************************************************************************/
DLLIMPORT long _DK_script_support_send_tunneller_to_appropriate_dungeon(struct Thing *creatng);
/******************************************************************************/
/**
 * Reads word from 'line' into 'param'. Sets if 'line_end' was reached.
 * @param line The input line position pointer.
 * @param param Output parameter acquired from the line.
 * @param parth_level Paraenesis level within the line, set to -1 on EOLN.
 */
const struct CommandDesc *get_next_word(char **line, char *param, int *para_level, const struct CommandDesc *cmdlist_desc)
{
    char chr;
    SCRIPTDBG(12,"Starting");
    const struct CommandDesc* cmnd_desc = NULL;
    // Find start of an item to read
    unsigned int pos = 0;
    param[pos] = '\0';
    while (1)
    {
        chr = **line;
        // letter or number
        if ((isalnum(chr)) || (chr == '-'))
            break;
        // operator
        if ((chr == '\"') || (chr == '=') || (chr == '!') || (chr == '<') || (chr == '>') || (chr == '~'))
            break;
        // end of line
        if ((chr == '\r') || (chr == '\n') || (chr == '\0'))
        {
            (*para_level) = -1;
            return NULL;
        }
        // paraenesis open
        if (chr == '(') {
            (*para_level)++;
        } else
        // paraenesis close
        if (chr == ')') {
            (*para_level)--;
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
        int i = 0;
        cmnd_desc = NULL;
        while (cmdlist_desc[i].textptr != NULL)
        {
            if (strcmp(param, cmdlist_desc[i].textptr) == 0)
            {
                cmnd_desc = &cmdlist_desc[i];
                break;
            }
            i++;
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
          SCRPTERRLOG("Unexpected '-' not followed by a number");
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
                SCRPTERRLOG("Expected '=' after '!'");
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
              SCRPTERRLOG("Expected '=' after '='");
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
        *para_level = -1;
    param[pos] = '\0';
    return cmnd_desc;
}

const char *script_get_command_name(long cmnd_index)
{
    long i = 0;
    while (command_desc[i].textptr != NULL)
    {
        if (command_desc[i].index == cmnd_index)
            return command_desc[i].textptr;
        i++;
  }
  return NULL;
}

/**
 * Returns if the command is 'preloaded'. Preloaded commands are initialized
 * before the whole level data is loaded.
 */
TbBool script_is_preloaded_command(long cmnd_index)
{
  switch (cmnd_index)
  {
  case Cmd_SWAP_CREATURE:
  case Cmd_LEVEL_VERSION:
      return true;
  default:
      return false;
  }
}

#define get_players_range_single(plr_range_id) get_players_range_single_f(plr_range_id, __func__, text_line_number)
long get_players_range_single_f(long plr_range_id, const char *func_name, long ln_num)
{
    if (plr_range_id < 0) {
        return -1;
    }
    if (plr_range_id == ALL_PLAYERS) {
        return -3;
    }
    if (plr_range_id == PLAYER_GOOD) {
        return game.hero_player_num;
    }
    if (plr_range_id == PLAYER_NEUTRAL) {
        return game.neutral_player_num;
    }
    if (plr_range_id < PLAYERS_COUNT)
    {
        return plr_range_id;
    }
    return -2;
}

#define get_players_range(plr_range_id, plr_start, plr_end) get_players_range_f(plr_range_id, plr_start, plr_end, __func__, text_line_number)
long get_players_range_f(long plr_range_id, int *plr_start, int *plr_end, const char *func_name, long ln_num)
{
    if (plr_range_id < 0)
    {
        return -1;
    }
    if (plr_range_id == ALL_PLAYERS)
    {
        *plr_start = 0;
        *plr_end = PLAYERS_COUNT;
        return plr_range_id;
    } else
    if (plr_range_id == PLAYER_GOOD)
    {
        *plr_start = game.hero_player_num;
        *plr_end = game.hero_player_num+1;
        return plr_range_id;
    } else
    if (plr_range_id == PLAYER_NEUTRAL)
    {
        *plr_start = game.neutral_player_num;
        *plr_end = game.neutral_player_num+1;
        return plr_range_id;
    } else
    if (plr_range_id < PLAYERS_COUNT)
    {
        *plr_start = plr_range_id;
        *plr_end = (*plr_start) + 1;
        return plr_range_id;
    }
    return -2;
}

#define get_players_range_from_str(plrname, plr_start, plr_end) get_players_range_from_str_f(plrname, plr_start, plr_end, __func__, text_line_number)
long get_players_range_from_str_f(const char *plrname, int *plr_start, int *plr_end, const char *func_name, long ln_num)
{
    long plr_range_id = get_rid(player_desc, plrname);
    switch (get_players_range_f(plr_range_id, plr_start, plr_end, func_name, ln_num))
    {
    case -1:
        ERRORMSG("%s(line %lu): Invalid player name, '%s'",func_name,ln_num, plrname);
        *plr_start = 0;
        *plr_end = 0;
        return -1;
    case -2:
        ERRORMSG("%s(line %lu): Player '%s' out of range",func_name,ln_num, plrname);
        *plr_start = 0;
        *plr_end = 0;
        return -2;
    default:
        break;
    }
    return plr_range_id;
}

#define get_player_id(plrname, plr_range_id) get_player_id_f(plrname, plr_range_id, __func__, text_line_number)
TbBool get_player_id_f(const char *plrname, long *plr_range_id, const char *func_name, long ln_num)
{
    *plr_range_id = get_rid(player_desc, plrname);
    if (*plr_range_id == -1)
    {
      ERRORMSG("%s(line %lu): Invalid player name, '%s'",func_name,ln_num, plrname);
      return false;
    }
    return true;
}

unsigned short get_map_location_type(TbMapLocation location)
{
  return location & 0x0F;
}

unsigned long get_map_location_longval(TbMapLocation location)
{
  return (location >> 4);
}

unsigned long get_map_location_plyrval(TbMapLocation location)
{
  return (location >> 12);
}

unsigned short get_map_location_plyridx(TbMapLocation location)
{
  return (location >> 4) & 0xFF;
}

/**
 * Returns location id for 1-param location from script.
 * @param locname
 * @param location
 * @return
 * @see get_map_heading_id()
 */
#define get_map_location_id(locname, location) get_map_location_id_f(locname, location, __func__, text_line_number)
TbBool get_map_location_id_f(const char *locname, TbMapLocation *location, const char *func_name, long ln_num)
{
    // If there's no locname, then coordinates are set directly as (x,y)
    if (locname == NULL)
    {
      *location = MLoc_NONE;
      return true;
    }
    // Player name means the location of player's Dungeon Heart
    long i = get_rid(player_desc, locname);
    if (i != -1)
    {
      if ((i != ALL_PLAYERS) && (i != PLAYER_NEUTRAL)) {
          if (!player_has_heart(i)) {
              WARNMSG("%s(line %lu): Target player %d has no heart",func_name,ln_num, (int)i);
          }
          *location = ((unsigned long)i << 4) | MLoc_PLAYERSHEART;
      } else {
          *location = MLoc_NONE;
      }
      return true;
    }
    // Creature name means location of such creature belonging to player0
    i = get_rid(creature_desc, locname);
    if (i != -1)
    {
        *location = ((unsigned long)i << 12) | ((unsigned long)my_player_number << 4) | MLoc_CREATUREKIND;
        return true;
    }
    // Room name means location of such room belonging to player0
    i = get_rid(room_desc, locname);
    if (i != -1)
    {
        *location = ((unsigned long)i << 12) | ((unsigned long)my_player_number << 4) | MLoc_ROOMKIND;
        return true;
    }
    // Todo list of functions
    if (strcmp(locname, "TRIGGERED_OBJECT") == 0)
    {
        *location = (((unsigned long)MML_TRIGGERED_OBJECT) << 12)
            | (((unsigned long)CurrentPlayer) << 4) //TODO: other players
            | MLoc_METALOCATION;
        return true;
    }
    else if (strcmp(locname, "COMBAT") == 0)
    {
        *location = (((unsigned long)MML_RECENT_COMBAT) << 12)
            | ((unsigned long)my_player_number << 4)
            | MLoc_METALOCATION;
        return true;
    }
    i = atol(locname);
    // Negative number means Hero Gate
    if (i < 0)
    {
        long n = -i;
        struct Thing* thing = find_hero_gate_of_number(n);
        if (thing_is_invalid(thing))
        {
            ERRORMSG("%s(line %lu): Non-existing Hero Door, no %d",func_name,ln_num,(int)-i);
            *location = MLoc_NONE;
            return false;
        }
        *location = (((unsigned long)n) << 4) | MLoc_HEROGATE;
    } else
    // Positive number means Action Point
    if (i > 0)
    {
        long n = action_point_number_to_index(i);
        if (!action_point_exists_idx(n))
        {
            ERRORMSG("%s(line %lu): Non-existing Action Point, no %d",func_name,ln_num,(int)i);
            *location = MLoc_NONE;
            return false;
        }
        // Set to action point number
        *location = (((unsigned long)n) << 4) | MLoc_ACTIONPOINT;
    } else
    // Zero is an error; reset to no location
    {
      ERRORMSG("%s(line %lu): Invalid LOCATION = '%s'",func_name,ln_num, locname);
      *location = MLoc_NONE;
    }
    return true;
}

/**
 * Writes Code Name (name to use in script file) of given map location to buffer.
 * @ name Output buffer. It should be COMMAND_WORD_LEN long.
 */
TbBool get_map_location_code_name(TbMapLocation location, char *name)
{
    long i;
    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:{
        i = get_map_location_longval(location);
        struct ActionPoint* apt = action_point_get(i);
        if (apt->num <= 0) {
            break;
        }
        itoa(apt->num, name, 10);
        };return true;
    case MLoc_HEROGATE:{
        i = get_map_location_longval(location);
        if (i <= 0) {
            break;
        }
        itoa(-i, name, 10);
        };return true;
    case MLoc_PLAYERSHEART:{
        i = get_map_location_longval(location);
        const char* cnstname = get_conf_parameter_text(player_desc, i);
        if (cnstname[0] == '\0') {
            break;
        }
        strcpy(name, cnstname);
        };return true;
    case MLoc_CREATUREKIND:{
        i = get_map_location_plyrval(location);
        const char* cnstname = get_conf_parameter_text(creature_desc, i);
        if (cnstname[0] == '\0') {
            break;
        }
        strcpy(name, cnstname);
        };return true;
    case MLoc_ROOMKIND:{
        i = get_map_location_plyrval(location);
        const char* cnstname = get_conf_parameter_text(room_desc, i);
        if (cnstname[0] == '\0') {
            break;
        }
        strcpy(name, cnstname);
        };return true;
    case MLoc_OBJECTKIND:
    case MLoc_THING:
    case MLoc_PLAYERSDUNGEON:
    case MLoc_APPROPRTDUNGEON:
    case MLoc_DOORKIND:
    case MLoc_TRAPKIND:
    case MLoc_NONE:
    default:
        break;
    }
    strcpy(name, "INVALID");
    return false;
}

/**
 * Returns location id for 2-param tunneler heading from script.
 * @param headname
 * @param target
 * @param location
 * @return
 * @see get_map_location_id()
 */
#define get_map_heading_id(headname, target, location) get_map_heading_id_f(headname, target, location, __func__, text_line_number)
TbBool get_map_heading_id_f(const char *headname, long target, TbMapLocation *location, const char *func_name, long ln_num)
{
    // If there's no headname, then there's an error
    if (headname == NULL)
    {
        SCRPTERRLOG("No heading objective");
        *location = MLoc_NONE;
        return false;
    }
    long head_id = get_rid(head_for_desc, headname);
    if (head_id == -1)
    {
        SCRPTERRLOG("Unhandled heading objective, '%s'", headname);
        *location = MLoc_NONE;
        return false;
    }
    // Check if the target place exists, and set 'location'
    // Note that we only need to support enum items which are in head_for_desc[].
    switch (head_id)
    {
    case MLoc_ACTIONPOINT:
    {
        long n = action_point_number_to_index(target);
        *location = ((unsigned long)n << 4) | head_id;
        if (!action_point_exists_idx(n)) {
            SCRPTWRNLOG("Target action point no %d doesn't exist", (int)target);
        }
        return true;
    }
    case MLoc_PLAYERSDUNGEON:
    case MLoc_PLAYERSHEART:
        *location = ((unsigned long)target << 4) | head_id;
        if (!player_has_heart(target)) {
            SCRPTWRNLOG("Target player %d has no heart", (int)target);
        }
        return true;
    case MLoc_APPROPRTDUNGEON:
        *location = (0) | head_id; // This option has no 'target' value
        return true;
    default:
        *location = MLoc_NONE;
        SCRPTWRNLOG("Unsupported Heading objective %d", (int)head_id);
        break;
    }
    return false;
}

TbBool script_support_setup_player_as_computer_keeper(PlayerNumber plyridx, long comp_model)
{
    struct PlayerInfo* player = get_player(plyridx);
    if (player_invalid(player)) {
        SCRPTWRNLOG("Tried to set up invalid player %d",(int)plyridx);
        return false;
    }
    // It uses >= because the count will be one higher than
    // the actual highest possible computer model number.
    if ((comp_model < 0) || (comp_model >= COMPUTER_MODELS_COUNT)) {
        SCRPTWRNLOG("Tried to set up player %d as outranged computer model %d",(int)plyridx,(int)comp_model);
        comp_model = 0;
    }
    player->allocflags |= PlaF_Allocated;
    player->id_number = plyridx;
    player->is_active = 1;
    player->allocflags |= PlaF_CompCtrl;
    init_player_start(player, false);
    if (!setup_a_computer_player(plyridx, comp_model)) {
        player->allocflags &= ~PlaF_CompCtrl;
        player->allocflags &= ~PlaF_Allocated;
        return false;
    }
    return true;
}

TbBool script_support_setup_player_as_zombie_keeper(unsigned short plyridx)
{
    SYNCDBG(8,"Starting for player %d",(int)plyridx);
    struct PlayerInfo* player = get_player(plyridx);
    if (player_invalid(player)) {
        SCRPTWRNLOG("Tried to set up invalid player %d",(int)plyridx);
        return false;
    }
    player->allocflags &= ~PlaF_Allocated; // mark as non-existing
    player->id_number = plyridx;
    player->is_active = 0;
    player->allocflags &= ~PlaF_CompCtrl;
    init_player_start(player, false);
    return true;
}

void command_create_party(const char *prtname)
{
    if (script_current_condition != -1)
    {
        SCRPTWRNLOG("Party '%s' defined inside conditional statement",prtname);
    }
    create_party(prtname);
}

long pop_condition(void)
{
  if (script_current_condition == -1)
  {
    SCRPTERRLOG("unexpected ENDIF");
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

void command_add_to_party(const char *prtname, const char *crtr_name, long crtr_level, long carried_gold, const char *objectv, long countdown)
{
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
      SCRPTERRLOG("Invalid Creature Level parameter; %ld not in range (%d,%d)",crtr_level,1,CREATURE_MAX_LEVEL);
      return;
    }
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
      return;
    }
    long objctv_id = get_rid(hero_objective_desc, objectv);
    if (objctv_id == -1)
    {
      SCRPTERRLOG("Unknown party member objective, '%s'", objectv);
      return;
    }
  //SCRPTLOG("Party '%s' member kind %d, level %d",prtname,crtr_id,crtr_level);
    if (script_current_condition != -1)
    {
      SCRPTWRNLOG("Party '%s' member added inside conditional statement",prtname);
    }
    add_member_to_party_name(prtname, crtr_id, crtr_level, carried_gold, objctv_id, countdown);
}

void command_tutorial_flash_button(long btn_id, long duration)
{
    command_add_value(Cmd_TUTORIAL_FLASH_BUTTON, ALL_PLAYERS, btn_id, duration, 0);
}

void command_add_party_to_level(long plr_range_id, const char *prtname, const char *locname, long ncopies)
{
    TbMapLocation location;
    if (ncopies < 1)
    {
        SCRPTERRLOG("Invalid NUMBER parameter");
        return;
    }
    if (game.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_CREATURE commands in script");
        return;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize party name
    long prty_id = get_party_index_of_name(prtname);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined",prtname);
        return;
    }
    if ((script_current_condition < 0) && (next_command_reusable == 0))
    {
        struct Party* party = &game.script.creature_partys[prty_id];
        script_process_new_party(party, plr_id, location, ncopies);
    } else
    {
        struct PartyTrigger* pr_trig = &game.script.party_triggers[game.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        set_flag_byte(&(pr_trig->flags), TrgF_REUSABLE, next_command_reusable);
        set_flag_byte(&(pr_trig->flags), TrgF_DISABLED, false);
        pr_trig->plyr_idx = plr_id;
        pr_trig->creatr_id = -prty_id;
        pr_trig->location = location;
        pr_trig->ncopies = ncopies;
        pr_trig->condit_idx = script_current_condition;
        game.script.party_triggers_num++;
    }
}

void command_add_creature_to_level(long plr_range_id, const char *crtr_name, const char *locname, long ncopies, long crtr_level, long carried_gold)
{
    TbMapLocation location;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if ((ncopies <= 0) || (ncopies >= CREATURES_COUNT))
    {
        SCRPTERRLOG("Invalid number of creatures to add");
        return;
    }
    if (game.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_CREATURE commands in script");
        return;
    }
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    if (script_current_condition < 0)
    {
        script_process_new_creatures(plr_id, crtr_id, location, ncopies, carried_gold, crtr_level-1);
    } else
    {
        struct PartyTrigger* pr_trig = &game.script.party_triggers[game.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        set_flag_byte(&(pr_trig->flags), TrgF_REUSABLE, next_command_reusable);
        set_flag_byte(&(pr_trig->flags), TrgF_DISABLED, false);
        pr_trig->plyr_idx = plr_id;
        pr_trig->creatr_id = crtr_id;
        pr_trig->crtr_level = crtr_level-1;
        pr_trig->carried_gold = carried_gold;
        pr_trig->location = location;
        pr_trig->ncopies = ncopies;
        pr_trig->condit_idx = script_current_condition;
        game.script.party_triggers_num++;
    }
}

void command_add_condition(long plr_range_id, long opertr_id, long varib_type, long varib_id, long value)
{
    // TODO: replace with pointer to functions
    struct Condition* condt = &game.script.conditions[game.script.conditions_num];
    condt->condit_idx = script_current_condition;
    condt->plyr_range = plr_range_id;
    condt->variabl_type = varib_type;
    condt->variabl_idx = varib_id;
    condt->operation = opertr_id;
    condt->rvalue = value;
    if (condition_stack_pos >= CONDITIONS_COUNT)
    {
        game.script.conditions_num++;
        SCRPTWRNLOG("Conditions too deep in script");
        return;
    }
    if (script_current_condition >= 0)
    {
        condition_stack[condition_stack_pos] = script_current_condition;
        condition_stack_pos++;
    }
    script_current_condition = game.script.conditions_num;
    game.script.conditions_num++;
}

static TbBool parse_varib(const char *varib_name, long *varib_id, long *varib_type)
{
    char c;

    if (level_file_version > 0)
    {
        *varib_type = get_id(variable_desc, varib_name);
    } else
    {
        *varib_type = get_id(dk1_variable_desc, varib_name);
    }
    if (*varib_type == -1)
      *varib_id = -1;
    else
      *varib_id = 0;
    if (*varib_id == -1)
    {
      *varib_id = get_id(creature_desc, varib_name);
      *varib_type = SVar_CREATURE_NUM;
    }
    //TODO: list of lambdas
    if (*varib_id == -1)
    {
      *varib_id = get_id(room_desc, varib_name);
      *varib_type = SVar_ROOM_SLABS;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(timer_desc, varib_name);
      *varib_type = SVar_TIMER;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(flag_desc, varib_name);
      *varib_type = SVar_FLAG;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(door_desc, varib_name);
      *varib_type = SVar_DOOR_NUM;
    }
    if (*varib_id == -1)
    {
        *varib_id = get_id(trap_desc, varib_name);
        *varib_type = SVar_TRAP_NUM;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(campaign_flag_desc, varib_name);
      *varib_type = SVar_CAMPAIGN_FLAG;
    }
    if (*varib_id == -1)
    {
        if (2 == sscanf(varib_name, "BOX%ld_ACTIVATE%c", varib_id, &c) && (c == 'D'))
        {
            // activateD
            *varib_type = SVar_BOX_ACTIVATED;
        }
        else
        {
          *varib_id = -1;
        }
    }
    if (*varib_id == -1)
    {
      SCRPTERRLOG("Unknown variable name, '%s'", varib_name);
      return false;
    }
    return true;
}

void command_if(long plr_range_id, const char *varib_name, const char *operatr, long value)
{
    long varib_type;
    long varib_id;
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
      SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
      return;
    }
    // Recognize variable
    if (!parse_varib(varib_name, &varib_id, &varib_type))
    {
        return;
    }
    { // Warn if using the command for a player without Dungeon struct
        int plr_start;
        int plr_end;
        if (get_players_range(plr_range_id, &plr_start, &plr_end) >= 0) {
            struct Dungeon* dungeon = get_dungeon(plr_start);
            if ((plr_start+1 == plr_end) && dungeon_invalid(dungeon)) {
                // Note that this list should be kept updated with the changes in get_condition_value()
                if ((varib_type != SVar_GAME_TURN) && (varib_type != SVar_ALL_DUNGEONS_DESTROYED)
                 && (varib_type != SVar_DOOR_NUM) && (varib_type != SVar_TRAP_NUM))
                    SCRPTWRNLOG("Found player without dungeon used in IF clause in script; this will not work correctly");
            }
        }
    }
    // Recognize comparison
    long opertr_id = get_id(comparison_desc, operatr);
    if (opertr_id == -1)
    {
      SCRPTERRLOG("Unknown comparison name, '%s'", operatr);
      return;
    }
    // Add the condition to script structure
    command_add_condition(plr_range_id, opertr_id, varib_type, varib_id, value);
}

struct ScriptValue *allocate_script_value(void)
{
  if (game.script.values_num >= SCRIPT_VALUES_COUNT)
    return NULL;
  struct ScriptValue* value = &game.script.values[game.script.values_num];
  game.script.values_num++;
  return value;
}

void command_add_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4)
{
    if ((script_current_condition < 0) && (next_command_reusable == 0))
    {
        script_process_value(var_index, plr_range_id, val2, val3, val4);
        return;
    }
    struct ScriptValue* value = allocate_script_value();
    if (value == NULL)
    {
        SCRPTERRLOG("Too many VALUEs in script (limit is %d)", SCRIPT_VALUES_COUNT);
        return;
    }
    set_flag_byte(&value->flags, TrgF_REUSABLE, next_command_reusable);
    set_flag_byte(&value->flags, TrgF_DISABLED, false);
    value->valtype = var_index;
    value->plyr_range = plr_range_id;
    value->field_4 = val2;
    value->field_8 = val3;
    value->field_C = val4;
    value->condit_idx = script_current_condition;
}

void command_display_information(long msg_num, const char *where, long x, long y)
{
    TbMapLocation location;
    if ((msg_num < 0) || (msg_num >= STRINGS_MAX))
    {
      SCRPTERRLOG("Invalid TEXT number");
      return;
    }
    if (!get_map_location_id(where, &location))
      return;
    command_add_value(Cmd_DISPLAY_INFORMATION, ALL_PLAYERS, msg_num, location, get_subtile_number(x,y));
}

void command_set_generate_speed(long game_turns)
{
    if (game_turns <= 0)
    {
      SCRPTERRLOG("Generation speed must be positive number");
      return;
    }
    command_add_value(Cmd_SET_GENERATE_SPEED, ALL_PLAYERS, game_turns, 0, 0);
}

void command_dead_creatures_return_to_pool(long val)
{
    command_add_value(Cmd_DEAD_CREATURES_RETURN_TO_POOL, ALL_PLAYERS, val, 0, 0);
}

void command_bonus_level_time(long game_turns)
{
    if (game_turns < 0)
    {
        SCRPTERRLOG("Bonus time must be nonnegative");
        return;
    }
    command_add_value(Cmd_BONUS_LEVEL_TIME, ALL_PLAYERS, game_turns, 0, 0);
}

void player_reveal_map_area(PlayerNumber plyr_idx, long x, long y, long w, long h)
{
  SYNCDBG(0,"Revealing around (%d,%d)",x,y);
  reveal_map_area(plyr_idx, x-(w>>1), x+(w>>1)+(w%1), y-(h>>1), y+(h>>1)+(h%1));
}

void player_reveal_map_location(int plyr_idx, TbMapLocation target, long r)
{
    SYNCDBG(0, "Revealing location type %d", target);
    long x = 0;
    long y = 0;
    find_map_location_coords(target, &x, &y, __func__);
    if ((x == 0) && (y == 0))
    {
        WARNLOG("Can't decode location %d", target);
        return;
  }
  reveal_map_area(plyr_idx, x-(r>>1), x+(r>>1)+(r%1), y-(r>>1), y+(r>>1)+(r%1));
}

void command_set_start_money(long plr_range_id, long gold_val)
{
    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %d is not supported in this command", (int)plr_range_id);
        return;
  }
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Start money set inside conditional block; condition ignored");
  }
  for (int i = plr_start; i < plr_end; i++)
  {
      player_add_offmap_gold(i, gold_val);
  }
}

void command_room_available(long plr_range_id, const char *roomname, unsigned long can_resrch, unsigned long can_build)
{
    long room_id = get_rid(room_desc, roomname);
    if (room_id == -1)
    {
      SCRPTERRLOG("Unknown room name, '%s'", roomname);
      return;
    }
    command_add_value(Cmd_ROOM_AVAILABLE, plr_range_id, room_id, can_resrch, can_build);
}

void command_creature_available(long plr_range_id, const char *crtr_name, unsigned long can_be_avail, unsigned long force_avail)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
      return;
    }
    command_add_value(Cmd_CREATURE_AVAILABLE, plr_range_id, crtr_id, can_be_avail, force_avail);
}

void command_magic_available(long plr_range_id, const char *magname, unsigned long can_resrch, unsigned long can_use)
{
    long mag_id = get_rid(power_desc, magname);
    if (mag_id == -1)
    {
      SCRPTERRLOG("Unknown magic, '%s'", magname);
      return;
    }
    command_add_value(Cmd_MAGIC_AVAILABLE, plr_range_id, mag_id, can_resrch, can_use);
}

void command_trap_available(long plr_range_id, const char *trapname, unsigned long can_build, unsigned long amount)
{
    long trap_id = get_rid(trap_desc, trapname);
    if (trap_id == -1)
    {
      SCRPTERRLOG("Unknown trap, '%s'", trapname);
      return;
    }
    command_add_value(Cmd_TRAP_AVAILABLE, plr_range_id, trap_id, can_build, amount);
}

/**
 * Updates amount of RESEARCH points needed for the item to be researched.
 * Will not reorder the RESEARCH items.
 */
void command_research(long plr_range_id, const char *trg_type, const char *trg_name, unsigned long val)
{
    long item_type = get_rid(research_desc, trg_type);
    long item_id = get_research_id(item_type, trg_name, __func__);
    if (item_id < 0)
      return;
    command_add_value(Cmd_RESEARCH, plr_range_id, item_type, item_id, val);
}

/**
 * Updates amount of RESEARCH points needed for the item to be researched.
 * Reorders the RESEARCH items - needs all items to be re-added.
 */
void command_research_order(long plr_range_id, const char *trg_type, const char *trg_name, unsigned long val)
{
    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
        SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
        return;
    }
    for (long i = plr_start; i < plr_end; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        if (dungeon->research_num >= DUNGEON_RESEARCH_COUNT)
        {
          SCRPTERRLOG("Too many RESEARCH ITEMS, for player %d", i);
          return;
        }
    }
    long item_type = get_rid(research_desc, trg_type);
    long item_id = get_research_id(item_type, trg_name, __func__);
    if (item_id < 0)
      return;
    command_add_value(Cmd_RESEARCH_ORDER, plr_range_id, item_type, item_id, val);
}

void command_if_action_point(long apt_num, long plr_range_id)
{
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    // Check the Action Point
    long apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        SCRPTERRLOG("Non-existing Action Point, no %d", apt_num);
        return;
    }
    command_add_condition(plr_range_id, 0, SVar_ACTION_POINT_TRIGGERED, apt_idx, 0);
}

void command_if_slab_owner(MapSlabCoord slb_x, MapSlabCoord slb_y, long plr_range_id)
{
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    command_add_condition(slb_x, 1, SVar_SLAB_OWNER, slb_y, plr_range_id);
}

void command_if_slab_type(MapSlabCoord slb_x, MapSlabCoord slb_y, long slab_type)
{
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    command_add_condition(slb_x, 1, SVar_SLAB_TYPE, slb_y, slab_type);
}

void command_computer_player(long plr_range_id, long comp_model)
{
    if (script_current_condition != -1)
    {
        SCRPTWRNLOG("Computer player setup inside conditional block; condition ignored");
    }
    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
        SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
        return;
    }
    for (long i = plr_start; i < plr_end; i++)
    {
        script_support_setup_player_as_computer_keeper(i, comp_model);
    }
}

void command_set_timer(long plr_range_id, const char *timrname)
{
    long timr_id = get_rid(timer_desc, timrname);
    if (timr_id == -1)
    {
        SCRPTERRLOG("Unknown timer, '%s'", timrname);
        return;
    }
    command_add_value(Cmd_SET_TIMER, plr_range_id, timr_id, 0, 0);
}

void command_win_game(void)
{
    if (script_current_condition == -1)
    {
        SCRPTERRLOG("Command WIN GAME found with no condition");
        return;
    }
    if (game.script.win_conditions_num >= WIN_CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many WIN GAME conditions in script");
        return;
    }
    game.script.win_conditions[game.script.win_conditions_num] = script_current_condition;
    game.script.win_conditions_num++;
}

void command_lose_game(void)
{
  if (script_current_condition == -1)
  {
    SCRPTERRLOG("Command LOSE GAME found with no condition");
    return;
  }
  if (game.script.lose_conditions_num >= WIN_CONDITIONS_COUNT)
  {
    SCRPTERRLOG("Too many LOSE GAME conditions in script");
    return;
  }
  game.script.lose_conditions[game.script.lose_conditions_num] = script_current_condition;
  game.script.lose_conditions_num++;
}

void command_set_flag(long plr_range_id, const char *flgname, long val)
{
    long flg_id = get_rid(flag_desc, flgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown flag, '%s'", flgname);
        return;
  }
  command_add_value(Cmd_SET_FLAG, plr_range_id, flg_id, val, 0);
}

void command_max_creatures(long plr_range_id, long val)
{
    command_add_value(Cmd_MAX_CREATURES, plr_range_id, val, 0, 0);
}

void command_door_available(long plr_range_id, const char *doorname, unsigned long a3, unsigned long a4)
{
    long door_id = get_rid(door_desc, doorname);
    if (door_id == -1)
    {
        SCRPTERRLOG("Unknown door, '%s'", doorname);
        return;
  }
  command_add_value(Cmd_DOOR_AVAILABLE, plr_range_id, door_id, a3, a4);
}

void command_display_objective(long msg_num, const char *where, long x, long y)
{
  TbMapLocation location;
  if ((msg_num < 0) || (msg_num >= STRINGS_MAX))
  {
    SCRPTERRLOG("Invalid TEXT number");
    return;
  }
  if (!get_map_location_id(where, &location))
    return;
  command_add_value(Cmd_DISPLAY_OBJECTIVE, ALL_PLAYERS, msg_num, location, get_subtile_number(x,y));
}

void command_add_tunneller_to_level(long plr_range_id, const char *locname, const char *objectv, long target, unsigned char crtr_level, unsigned long carried_gold)
{
    TbMapLocation location;
    TbMapLocation heading;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if (game.script.tunneller_triggers_num >= TUNNELLER_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_TUNNELLER commands in script");
        return;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize place where party is going
    if (!get_map_heading_id(objectv, target, &heading))
        return;
    if (script_current_condition < 0)
    {
        script_process_new_tunneler(plr_id, location, heading, crtr_level-1, carried_gold);
    } else
    {
        struct TunnellerTrigger* tn_trig = &game.script.tunneller_triggers[game.script.tunneller_triggers_num % TUNNELLER_TRIGGERS_COUNT];
        set_flag_byte(&(tn_trig->flags), TrgF_REUSABLE, next_command_reusable);
        set_flag_byte(&(tn_trig->flags), TrgF_DISABLED, false);
        tn_trig->plyr_idx = plr_id;
        tn_trig->location = location;
        tn_trig->heading = heading;
        tn_trig->heading_OLD = 0; //target is now contained in heading and this is unused
        tn_trig->carried_gold = carried_gold;
        tn_trig->crtr_level = crtr_level-1;
        tn_trig->carried_gold = carried_gold;
        tn_trig->party_id = 0;
        tn_trig->condit_idx = script_current_condition;
        game.script.tunneller_triggers_num++;
    }
}

void command_add_tunneller_party_to_level(long plr_range_id, const char *prtname, const char *locname, const char *objectv, long target, char crtr_level, unsigned long carried_gold)
{
    TbMapLocation location;
    TbMapLocation heading;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if (game.script.tunneller_triggers_num >= TUNNELLER_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_TUNNELLER commands in script");
        return;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize place where party is going
    if (!get_map_heading_id(objectv, target, &heading))
        return;
    // Recognize party name
    long prty_id = get_party_index_of_name(prtname);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined", prtname);
        return;
    }
    struct Party* party = &game.script.creature_partys[prty_id];
    if (party->members_num >= GROUP_MEMBERS_COUNT-1)
    {
        SCRPTERRLOG("Party too big for ADD_TUNNELLER (Max %d members)", GROUP_MEMBERS_COUNT-1);
        return;
    }
    // Either add the party or add item to conditional triggers list
    if (script_current_condition < 0)
    {
        script_process_new_tunneller_party(plr_id, prty_id, location, heading, crtr_level-1, carried_gold);
    } else
    {
        struct TunnellerTrigger* tn_trig = &game.script.tunneller_triggers[game.script.tunneller_triggers_num % TUNNELLER_TRIGGERS_COUNT];
        set_flag_byte(&(tn_trig->flags), TrgF_REUSABLE, next_command_reusable);
        set_flag_byte(&(tn_trig->flags), TrgF_DISABLED, false);
        tn_trig->plyr_idx = plr_id;
        tn_trig->location = location;
        tn_trig->heading = heading;
        tn_trig->heading_OLD = 0; //target is now contained in heading and this is unused
        tn_trig->carried_gold = carried_gold;
        tn_trig->crtr_level = crtr_level-1;
        tn_trig->carried_gold = carried_gold;
        tn_trig->party_id = prty_id+1;
        tn_trig->condit_idx = script_current_condition;
        game.script.tunneller_triggers_num++;
    }
}

void command_add_creature_to_pool(const char *crtr_name, long amount)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    if ((amount < 0) || (amount >= CREATURES_COUNT))
    {
        SCRPTERRLOG("Invalid number of '%s' creatures for pool, %d", crtr_name, amount);
        return;
    }
    command_add_value(Cmd_ADD_CREATURE_TO_POOL, ALL_PLAYERS, crtr_id, amount, 0);
}

void command_reset_action_point(long apt_num)
{
    long apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        SCRPTERRLOG("Non-existing Action Point, no %d", apt_num);
        return;
  }
  command_add_value(Cmd_RESET_ACTION_POINT, ALL_PLAYERS, apt_idx, 0, 0);
}

void command_set_creature_max_level(long plr_range_id, const char *crtr_name, long crtr_level)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
  {
    SCRPTERRLOG("Invalid '%s' experience level, %d", crtr_name, crtr_level);
  }
  command_add_value(Cmd_SET_CREATURE_MAX_LEVEL, plr_range_id, crtr_id, crtr_level-1, 0);
}

void command_set_music(long val)
{
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Music set inside conditional block; condition ignored");
  }
  if (val >= FIRST_TRACK && val <= max_track)
  {
    game.audiotrack = val;
  }
  else 
  {
    SCRPTERRLOG("Invalid music track %d, track must be between %d and %d", val,FIRST_TRACK,max_track);
    return;
  }
}

void command_set_hate(long trgt_plr_range_id, long enmy_plr_range_id, long hate_val)
{
    // Verify enemy player
    long enmy_plr_id = get_players_range_single(enmy_plr_range_id);
    if (enmy_plr_id < 0) {
        SCRPTERRLOG("Given enemy player is not supported in this command");
        return;
    }
    command_add_value(Cmd_SET_HATE, trgt_plr_range_id, enmy_plr_id, hate_val, 0);
}

void command_if_available(long plr_range_id, const char *varib_name, const char *operatr, long value)
{
    long varib_type;
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
      SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
      return;
    }
    // Recognize variable
    long varib_id = -1;
    if (varib_id == -1)
    {
      varib_id = get_id(door_desc, varib_name);
      varib_type = SVar_AVAILABLE_DOOR;
    }
    if (varib_id == -1)
    {
      varib_id = get_id(trap_desc, varib_name);
      varib_type = SVar_AVAILABLE_TRAP;
    }
    if (varib_id == -1)
    {
      varib_id = get_id(room_desc, varib_name);
      varib_type = SVar_AVAILABLE_ROOM;
    }
    if (varib_id == -1)
    {
      varib_id = get_id(power_desc, varib_name);
      varib_type = SVar_AVAILABLE_MAGIC;
    }
    if (varib_id == -1)
    {
      varib_id = get_id(creature_desc, varib_name);
      varib_type = SVar_AVAILABLE_CREATURE;
    }
    if (varib_id == -1)
    {
      SCRPTERRLOG("Unrecognized VARIABLE, '%s'", varib_name);
      return;
    }
    // Recognize comparison
    long opertr_id = get_id(comparison_desc, operatr);
    if (opertr_id == -1)
    {
      SCRPTERRLOG("Unknown comparison name, '%s'", operatr);
      return;
    }
    { // Warn if using the command for a player without Dungeon struct
        int plr_start;
        int plr_end;
        if (get_players_range(plr_range_id, &plr_start, &plr_end) >= 0) {
            struct Dungeon* dungeon = get_dungeon(plr_start);
            if ((plr_start+1 == plr_end) && dungeon_invalid(dungeon)) {
                SCRPTWRNLOG("Found player without dungeon used in IF_AVAILABLE clause in script; this will not work correctly");
            }
        }
    }
    // Add the condition to script structure
    command_add_condition(plr_range_id, opertr_id, varib_type, varib_id, value);
}

void command_if_controls(long plr_range_id, const char *varib_name, const char *operatr, long value)
{
    long varib_id;
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
      SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
      return;
    }
    // Recognize variable
    long varib_type = get_id(controls_variable_desc, varib_name);
    if (varib_type == -1)
      varib_id = -1;
    else
      varib_id = 0;
    if (varib_id == -1)
    {
      varib_id = get_id(creature_desc, varib_name);
      varib_type = SVar_CONTROLS_CREATURE;
    }
    if (varib_id == -1)
    {
      SCRPTERRLOG("Unrecognized VARIABLE, '%s'", varib_name);
      return;
    }
    // Recognize comparison
    long opertr_id = get_id(comparison_desc, operatr);
    if (opertr_id == -1)
    {
      SCRPTERRLOG("Unknown comparison name, '%s'", operatr);
      return;
    }
    { // Warn if using the command for a player without Dungeon struct
        int plr_start;
        int plr_end;
        if (get_players_range(plr_range_id, &plr_start, &plr_end) >= 0) {
            struct Dungeon* dungeon = get_dungeon(plr_start);
            if ((plr_start+1 == plr_end) && dungeon_invalid(dungeon)) {
                SCRPTWRNLOG("Found player without dungeon used in IF_CONTROLS clause in script; this will not work correctly");
            }
        }
    }
    // Add the condition to script structure
    command_add_condition(plr_range_id, opertr_id, varib_type, varib_id, value);
}

void command_set_computer_globals(long plr_range_id, long val1, long val2, long val3, long val4, long val5, long val6)
{
  int plr_start;
  int plr_end;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
      SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
      return;
  }
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Computer globals altered inside conditional block; condition ignored");
  }
  for (long i = plr_start; i < plr_end; i++)
  {
      struct Computer2* comp = get_computer_player(i);
      if (computer_player_invalid(comp))
      {
          continue;
    }
    comp->field_1C = val1;
    comp->field_14 = val2;
    comp->field_18 = val3;
    comp->max_room_build_tasks = val4;
    comp->field_2C = val5;
    comp->sim_before_dig = val6;
  }
}

void command_set_computer_checks(long plr_range_id, const char *chkname, long val1, long val2, long val3, long val4, long val5)
{
  int plr_start;
  int plr_end;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
      SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
      return;
  }
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Computer check altered inside conditional block; condition ignored");
  }
  long n = 0;
  for (long i = plr_start; i < plr_end; i++)
  {
      struct Computer2* comp = get_computer_player(i);
      if (computer_player_invalid(comp)) {
          continue;
      }
      for (long k = 0; k < COMPUTER_CHECKS_COUNT; k++)
      {
          struct ComputerCheck* ccheck = &comp->checks[k];
          if ((ccheck->flags & ComChk_Unkn0002) != 0)
            break;
          if (ccheck->name == NULL)
            break;
          if (strcasecmp(chkname, ccheck->name) == 0)
          {
            ccheck->turns_interval = val1;
            ccheck->param1 = val2;
            ccheck->param2 = val3;
            ccheck->param3 = val4;
            ccheck->last_run_turn = val5;
            n++;
          }
      }
  }
  if (n == 0)
  {
    SCRPTERRLOG("No computer check found named '%s' in players %d to %d",chkname,(int)plr_start,(int)plr_end-1);
    return;
  }
  SCRIPTDBG(6,"Altered %d checks named '%s'",n,chkname);
}


void refresh_trap_anim(long trap_id)
{
    int k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Trap);
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* traptng = thing_get(i);
        if (thing_is_invalid(traptng))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = traptng->next_of_class;
        // Per thing code
        if (traptng->model == trap_id)
        {
            traptng->anim_sprite = gameadd.trap_stats[trap_id].sprite_anim_idx;
            struct TrapStats* trapstat = &gameadd.trap_stats[traptng->model];
            char start_frame;
            if (trapstat->field_13) {
                start_frame = -1;
            }
            else {
                start_frame = 0;
            }
            set_thing_draw(traptng, trapstat->sprite_anim_idx, trapstat->anim_speed, trapstat->sprite_size_max, trapstat->unanimated, start_frame, 2);
        }
        // Per thing code ends
        k++;
        if (k > slist->index)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}


                                                 // Name, Shots, TimeBetweenShots, Model, TriggerType, ActivationType, EffectType, Hidden
void command_set_trap_configuration(const char* trapname, long val1, long val2, long val3, long val4, long val5, long val6, long val7)
{
    if (script_current_condition != -1)
    {
        SCRPTWRNLOG("Trap configured inside conditional block; condition ignored");
    }
    long trap_id = get_rid(trap_desc, trapname);
    if (trap_id == -1)
    {
        SCRPTERRLOG("Unknown trap, '%s'", trapname);
    }
    int validval1 = 1;
    if (val1 <= 0)
    {
        validval1 = 0;
        SCRPTERRLOG("Shots '%d' out of range", val1);
    }
    int validval2 = 1;
    if (val2 < 0)
    {
        validval2 = 0;
        SCRPTERRLOG("Model '%d' out of range", val2);
    }
    int validval3 = 1;
    if (val3 <= 0)
    {
        validval3 = 0;
        SCRPTERRLOG("Model '%d' out of range", val3);
    }
    int validval4 = 0;
    switch (val4) {
    case TrpTrg_LineOfSight90:
    case TrpTrg_Pressure:
    case TrpTrg_LineOfSight:
    case TrpTrg_None:
        validval4 = 1;
        break;
    default:
        SCRPTERRLOG("No TriggerType '%d' found", val4);
    }
    int validval5 = 0;
    switch (val5) {
    case TrpAcT_HeadforTarget90:
    case TrpAcT_EffectonTrap:
    case TrpAcT_ShotonTrap:
    case TrpAcT_SlabChange:
    case TrpAcT_CreatureShot:
    case TrpAcT_CreatureSpawn:
    case TrpAcT_Power:
        validval5 = 1;
        break;
    default: 
        SCRPTERRLOG("No ActivationType '%d' found", val5);
    }
    int validval6 = 1;
    if ((val6 <= 0) || 
        ((val6 > magic_conf.shot_types_count) && (val5 == (TrpAcT_HeadforTarget90 || TrpAcT_ShotonTrap || TrpAcT_CreatureShot))) ||
        ((val6 > slab_conf.slab_types_count ) && (val5 == TrpAcT_SlabChange)) ||
        ((val6 > effects_conf.effect_types_count) && (val5 == TrpAcT_EffectonTrap)) ||
        ((val6 >= CREATURE_TYPES_COUNT) && (val5 == TrpAcT_CreatureSpawn)) ||
        ((val6 >= magic_conf.power_types_count) && (val5 == TrpAcT_Power))
        )
    {
        validval6 = 0;
        SCRPTERRLOG("EffectType '%d' out of range", val6);
    }
    int validval7 = 1;
    if ((val7 < 0) || (val7 > 1))
    {
        validval7 = 0;
        SCRPTERRLOG("TriggerAlarm '%d' out of range", val7);
    }

    if (validval1 && validval2 && validval3 && validval4 && validval5 && validval6 && validval7)
    {
        struct TrapConfigStats* trapst;
        struct ManfctrConfig* mconf;
        trapst = &trapdoor_conf.trap_cfgstats[trap_id];   
        mconf = &gameadd.traps_config[trap_id];
        SCRIPTDBG(7, "Changing trap %d configuration from (%d,%d,%d,%d,%d,%d,%d)", trap_id, mconf->shots, mconf->shots_delay, gameadd.trap_stats[trap_id].sprite_anim_idx, gameadd.trap_stats[trap_id].trigger_type, gameadd.trap_stats[trap_id].activation_type, gameadd.trap_stats[trap_id].created_itm_model,trapst->hidden);
        SCRIPTDBG(7, "Changing trap %d configuration to (%d,%d,%d,%d,%d,%d,%d)", trap_id, val1, val2, val3, val4, val5, val6, val7);
        mconf->shots = val1;
        mconf->shots_delay = val2;
        gameadd.trap_stats[trap_id].sprite_anim_idx = val3;
        gameadd.trap_stats[trap_id].trigger_type = val4;
        gameadd.trap_stats[trap_id].activation_type = val5;
        gameadd.trap_stats[trap_id].created_itm_model = val6;
        trapst->hidden = val7;
        //trapst->notify = val8; cannot fit 9 variables
        refresh_trap_anim(trap_id);
    } else
    {
        return;
    }
}
                                              //Name,  ManufactureLevel, ManufactureRequired,SellingValue,Health
void command_set_door_configuration(const char* doorname, long val1, long val2, long val3, long val4)
{
    if (script_current_condition != -1)
    {
        SCRPTWRNLOG("Door configured inside conditional block; condition ignored");
    }
    long door_id = get_rid(door_desc, doorname);
    if (door_id == -1)
    {
        SCRPTERRLOG("Unknown door, '%s'", doorname);
    }
    if (!((val1 < 0) || (val2 < 0) || (val3 < 0) || (val4 < 0)))
    {
        struct ManfctrConfig* mconf;
        mconf = &gameadd.doors_config[door_id];
        SCRIPTDBG(7, "Changing door %d configuration from (%d,%d,%d,%d) to (%d,%d,%d,%d)", door_id, mconf->manufct_level, mconf->manufct_required, mconf->selling_value, door_stats[door_id][0].health, val1, val2, val3, val4);
        mconf->manufct_level = val1;
        mconf->manufct_required = val2;
        mconf->selling_value = val3;
        door_stats[door_id][0].health = val4;
        door_stats[door_id][1].health = val4;
    }
    else
    {
        SCRPTERRLOG("Negative values not allowed when setting door '%d' to (%d,%d,%d,%d)", door_id, val1, val2, val3, val4);
        return;
    }
}

void command_set_computer_events(long plr_range_id, const char *evntname, long val1, long val2, long val3, long val4, long val5)
{
  int plr_start;
  int plr_end;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
      SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
      return;
  }
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Computer event altered inside conditional block; condition ignored");
  }
  long n = 0;
  for (long i = plr_start; i < plr_end; i++)
  {
      struct Computer2* comp = get_computer_player(i);
      if (computer_player_invalid(comp)) {
          continue;
      }
      for (long k = 0; k < COMPUTER_EVENTS_COUNT; k++)
      {
          struct ComputerEvent* event = &comp->events[k];
          if (event->name == NULL)
              break;
          if (strcasecmp(evntname, event->name) == 0)
          {
              if (level_file_version > 0)
              {
                  SCRIPTDBG(7, "Changing computer %d event '%s' config from (%d,%d,%d,%d,%d) to (%d,%d,%d,%d,%d)", (int)i, event->name,
                      (int)event->test_interval, (int)event->param1, (int)event->param2, (int)event->param3, (int)event->last_test_gameturn, (int)val1, (int)val2, (int)val3, (int)val4);
                  event->test_interval = val1;
                  event->param1 = val2;
                  event->param2 = val3;
                  event->param3 = val4;
                  event->last_test_gameturn = val5;
                  n++;
              } else
              {
                SCRIPTDBG(7, "Changing computer %d event '%s' config from (%d,%d) to (%d,%d)", (int)i, event->name,
                  (int)event->param1, (int)event->param2, (int)val1, (int)val2);
                  event->param1 = val1;
                  event->param2 = val2;
                  n++;
              }
          }
      }
  }
  if (n == 0)
  {
    SCRPTERRLOG("No computer event found named '%s' in players %d to %d", evntname,(int)plr_start,(int)plr_end-1);
    return;
  }
  SCRIPTDBG(6,"Altered %d events named '%s'",n,evntname);
}

void command_set_computer_process(long plr_range_id, const char *procname, long val1, long val2, long val3, long val4, long val5)
{
  int plr_start;
  int plr_end;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
      SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
      return;
  }
  if (script_current_condition != -1)
  {
    SCRPTWRNLOG("Computer process altered inside conditional block; condition ignored");
  }
  long n = 0;
  for (long i = plr_start; i < plr_end; i++)
  {
      struct Computer2* comp = get_computer_player(i);
      if (computer_player_invalid(comp)) {
          continue;
      }
      for (long k = 0; k < COMPUTER_PROCESSES_COUNT; k++)
      {
          struct ComputerProcess* cproc = &comp->processes[k];
          if ((cproc->flags & ComProc_Unkn0002) != 0)
              break;
          if (cproc->name == NULL)
              break;
          if (strcasecmp(procname, cproc->name) == 0)
          {
              SCRIPTDBG(7,"Changing computer %d process '%s' config from (%d,%d,%d,%d,%d) to (%d,%d,%d,%d,%d)",(int)i,cproc->name,
                  (int)cproc->priority,(int)cproc->confval_2,(int)cproc->confval_3,(int)cproc->confval_4,(int)cproc->confval_5,
                  (int)val1,(int)val2,(int)val3,(int)val4,(int)val5);
              cproc->priority = val1;
              cproc->confval_2 = val2;
              cproc->confval_3 = val3;
              cproc->confval_4 = val4;
              cproc->confval_5 = val5;
              n++;
          }
      }
  }
  if (n == 0)
  {
    SCRPTERRLOG("No computer process found named '%s' in players %d to %d", procname,(int)plr_start,(int)plr_end-1);
    return;
  }
  SCRIPTDBG(6,"Altered %d processes named '%s'",n,procname);
}

void command_set_creature_health(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > 65535))
  {
    SCRPTERRLOG("Invalid '%s' health value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_HEALTH, ALL_PLAYERS, crtr_id, val, 0);
}

void command_set_creature_strength(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > 255))
  {
    SCRPTERRLOG("Invalid '%s' strength value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_STRENGTH, ALL_PLAYERS, crtr_id, val, 0);
}

void command_set_creature_armour(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > 255))
  {
    SCRPTERRLOG("Invalid '%s' armour value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_ARMOUR, ALL_PLAYERS, crtr_id, val, 0);
}

void command_set_creature_fear_wounded(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > 255))
  {
    SCRPTERRLOG("Invalid '%s' fear value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_FEAR_WOUNDED, ALL_PLAYERS, crtr_id, val, 0);
}

void command_set_creature_fear_stronger(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > 32767))
  {
    SCRPTERRLOG("Invalid '%s' fear value, %d", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_FEAR_STRONGER, ALL_PLAYERS, crtr_id, val, 0);
}

void command_set_creature_fearsome_factor(const char* crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    if ((val < 0) || (val > 32767))
    {
        SCRPTERRLOG("Invalid '%s' fearsome value, %d", crtr_name, val);
        return;
    }
    command_add_value(Cmd_SET_CREATURE_FEARSOME_FACTOR, ALL_PLAYERS, crtr_id, val, 0);
}

void command_set_creature_property(const char* crtr_name, long property, short val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    command_add_value(Cmd_SET_CREATURE_PROPERTY, ALL_PLAYERS, crtr_id, property, val);
}

/**
 * Enables or disables an alliance between two players.
 *
 * @param plr1_range_id First player range identifier.
 * @param plr2_range_id Second player range identifier.
 * @param ally Controls whether the alliance is being created or being broken.
 */
void command_ally_players(long plr1_range_id, long plr2_range_id, TbBool ally)
{
    // Verify enemy player
    long plr2_id = get_players_range_single(plr2_range_id);
    if (plr2_id < 0) {
        SCRPTERRLOG("Given second player is not supported in this command");
        return;
    }
    command_add_value(Cmd_ALLY_PLAYERS, plr1_range_id, plr2_id, ally, 0);
}

void command_quick_objective(int idx, const char *msgtext, const char *where, long x, long y)
{
  TbMapLocation location;
  if ((idx < 0) || (idx >= QUICK_MESSAGES_COUNT))
  {
    SCRPTERRLOG("Invalid QUICK OBJECTIVE number (%d)", idx);
    return;
  }
  if (strlen(msgtext) >= MESSAGE_TEXT_LEN)
  {
      SCRPTWRNLOG("Objective TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
  }
  if ((gameadd.quick_messages[idx][0] != '\0') && (strcmp(gameadd.quick_messages[idx],msgtext) != 0))
  {
      SCRPTWRNLOG("Quick Objective no %d overwritten by different text", idx);
  }
  strncpy(gameadd.quick_messages[idx], msgtext, MESSAGE_TEXT_LEN-1);
  gameadd.quick_messages[idx][MESSAGE_TEXT_LEN-1] = '\0';
  if (!get_map_location_id(where, &location))
    return;
  command_add_value(Cmd_QUICK_OBJECTIVE, ALL_PLAYERS, idx, location, get_subtile_number(x,y));
}

void command_quick_information(int idx, const char *msgtext, const char *where, long x, long y)
{
  TbMapLocation location;
  if ((idx < 0) || (idx >= QUICK_MESSAGES_COUNT))
  {
    SCRPTERRLOG("Invalid information ID number (%d)", idx);
    return;
  }
  if (strlen(msgtext) > MESSAGE_TEXT_LEN)
  {
      SCRPTWRNLOG("Information TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
  }
  if ((gameadd.quick_messages[idx][0] != '\0') && (strcmp(gameadd.quick_messages[idx],msgtext) != 0))
  {
      SCRPTWRNLOG("Quick Message no %d overwritten by different text", idx);
  }
  strncpy(gameadd.quick_messages[idx], msgtext, MESSAGE_TEXT_LEN-1);
  gameadd.quick_messages[idx][MESSAGE_TEXT_LEN-1] = '\0';
  if (!get_map_location_id(where, &location))
    return;
  command_add_value(Cmd_QUICK_INFORMATION, ALL_PLAYERS, idx, location, get_subtile_number(x,y));
}

void command_play_message(long plr_range_id, const char *msgtype, int msg_num)
{
    long msgtype_id = get_id(msgtype_desc, msgtype);
    if (msgtype_id == -1)
    {
        SCRPTERRLOG("Unrecognized message type, '%s'", msgtype);
        return;
  }
  command_add_value(Cmd_PLAY_MESSAGE, plr_range_id, msgtype_id, msg_num, 0);
}

void command_add_gold_to_player(long plr_range_id, long amount)
{
    command_add_value(Cmd_ADD_GOLD_TO_PLAYER, plr_range_id, amount, 0, 0);
}

void command_set_creature_tendencies(long plr_range_id, const char *tendency, long value)
{
    long tend_id = get_rid(tendency_desc, tendency);
    if (tend_id == -1)
    {
      SCRPTERRLOG("Unrecognized tendency type, '%s'", tendency);
      return;
    }
    command_add_value(Cmd_SET_CREATURE_TENDENCIES, plr_range_id, tend_id, value, 0);
}

void command_reveal_map_rect(long plr_range_id, long x, long y, long w, long h)
{
    command_add_value(Cmd_REVEAL_MAP_RECT, plr_range_id, x, y, (h<<16)+w);
}

void command_change_slab_owner(long x, long y, long plr_range_id)
{
    command_add_value(Cmd_CHANGE_SLAB_OWNER, plr_range_id, x, y, 0);
}

void command_change_slab_type(long x, long y, long slab_type)
{
    command_add_value(Cmd_CHANGE_SLAB_TYPE, 0, x, y, slab_type);
}

void command_reveal_map_location(long plr_range_id, const char *locname, long range)
{
    TbMapLocation location;
    if (!get_map_location_id(locname, &location)) {
        return;
    }
    command_add_value(Cmd_REVEAL_MAP_LOCATION, plr_range_id, location, range, 0);
}

void command_message(const char *msgtext, unsigned char kind)
{
  const char *cmd;
  if (kind == 80)
    cmd = script_get_command_name(Cmd_PRINT);
  else
    cmd = script_get_command_name(Cmd_MESSAGE);
  SCRPTWRNLOG("Command '%s' is only supported in Dungeon Keeper Beta", cmd);
}

void command_swap_creature(const char *ncrt_name, const char *crtr_name)
{
    long ncrt_id = get_rid(newcrtr_desc, ncrt_name);
    if (ncrt_id == -1)
    {
        SCRPTERRLOG("Unknown new creature, '%s'", ncrt_name);
        return;
  }
  long crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1)
  {
      SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
      return;
  }
  struct CreatureModelConfig* crconf = &crtr_conf.model[crtr_id];
  if ((crconf->model_flags & CMF_IsSpecDigger) != 0)
  {
      SCRPTERRLOG("Unable to swap special diggers");
  }
  if (script_current_condition != -1)
  {
      SCRPTWRNLOG("Creature swapping placed inside conditional statement");
  }
  if (!swap_creature(ncrt_id, crtr_id))
  {
      SCRPTERRLOG("Error swapping creatures '%s'<->'%s'", ncrt_name, crtr_name);
  }
}

void command_kill_creature(long plr_range_id, const char *crtr_name, const char *criteria, int count)
{
    SCRIPTDBG(11, "Starting");
    if (count <= 0)
    {
        SCRPTERRLOG("Bad creatures count, %d", count);
        return;
  }
  long crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = get_rid(creature_select_criteria_desc, criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  command_add_value(Cmd_KILL_CREATURE, plr_range_id, crtr_id, select_id, count);
}

void command_level_up_creature(long plr_range_id, const char *crtr_name, const char *criteria, int count)
{
    SCRIPTDBG(11, "Starting");
    if (count <= 0)
    {
        SCRPTERRLOG("Bad creatures count, %d", count);
        return;
  }
  long crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = get_rid(creature_select_criteria_desc, criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  if (count < 1)
  {
    SCRPTERRLOG("Parameter has no positive value; discarding command");
    return;
  }
  if (count > 9)
  {
      count = 9;
  }
  command_add_value(Cmd_LEVEL_UP_CREATURE, plr_range_id, crtr_id, select_id, count);
}

void command_use_power_on_creature(long plr_range_id, const char *crtr_name, const char *criteria, long caster_plyr_idx, const char *magname, int splevel, char free)
{
  SCRIPTDBG(11, "Starting");
  if (splevel < 1)
  {
    SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", magname, splevel);
    splevel = 1;
  }
  if (splevel > MAGIC_OVERCHARGE_LEVELS)
  {
    SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", magname, splevel, MAGIC_OVERCHARGE_LEVELS);
    splevel = MAGIC_OVERCHARGE_LEVELS;
  }
  splevel--;
  long mag_id = get_rid(power_desc, magname);
  if (mag_id == -1)
  {
    SCRPTERRLOG("Unknown magic, '%s'", magname);
    return;
  }
  long crtr_id = get_rid(creature_desc, crtr_name);
  if (crtr_id == -1) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = get_rid(creature_select_criteria_desc, criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  PowerKind pwr = mag_id;
  if((PlayerNumber) caster_plyr_idx > PLAYER3)
  {
    if(pwr == PwrK_CALL2ARMS || pwr == PwrK_LIGHTNING)
    {
        SCRPTERRLOG("Only players 0-3 can cast %s", magname);
        return;
    }
  }

  // encode params: free, magic, caster, level -> into 4xbyte: FMCL
  long fmcl_bytes;
  {
      signed char f = free, m = mag_id, c = caster_plyr_idx, lvl = splevel;
      fmcl_bytes = (f << 24) | (m << 16) | (c << 8) | lvl;
  }
  command_add_value(Cmd_USE_POWER_ON_CREATURE, plr_range_id, crtr_id, select_id, fmcl_bytes);
}

void command_use_power_at_location(long plr_range_id, int stl_x, int stl_y, const char *magname, int splevel, char free)
{
  SCRIPTDBG(11, "Starting");
  if (splevel < 1)
  {
    SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", magname, splevel);
    splevel = 1;
  }
  if (splevel > MAGIC_OVERCHARGE_LEVELS)
  {
    SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", magname, splevel, MAGIC_OVERCHARGE_LEVELS);
    splevel = MAGIC_OVERCHARGE_LEVELS;
  }
  splevel--;
  long mag_id = get_rid(power_desc, magname);
  if (mag_id == -1)
  {
    SCRPTERRLOG("Unknown magic, '%s'", magname);
    return;
  }
  PowerKind pwr = mag_id;
  if((PlayerNumber) plr_range_id > PLAYER3)
  {
    if(pwr == PwrK_CALL2ARMS || pwr == PwrK_LIGHTNING)
    {
        SCRPTERRLOG("Only players 0-3 can cast %s", magname);
        return;
    }
  }

  // encode params: free, magic, level -> into 3xbyte: FML
  long fml_bytes;
  {
      signed char f = free, m = mag_id, lvl = splevel;
      fml_bytes = (f << 16) | (m << 8) | lvl;
  }
  command_add_value(Cmd_USE_POWER_AT_LOCATION, plr_range_id, stl_x, stl_y, fml_bytes);
}

void command_use_power(long plr_range_id, const char *magname, char free)
{
    SCRIPTDBG(11, "Starting");
    long mag_id = get_rid(power_desc, magname);
    if (mag_id == -1)
    {
        SCRPTERRLOG("Unknown magic, '%s'", magname);
        return;
    }
    PowerKind pwr = mag_id;
    if (pwr == PwrK_ARMAGEDDON && (PlayerNumber) plr_range_id > PLAYER3)
    {
        SCRPTERRLOG("Only players 0-3 can cast %s", magname);
        return;
    }
    command_add_value(Cmd_USE_POWER, plr_range_id, mag_id, free, 0);
}

void command_change_creature_owner(long origin_plyr_idx, const char *crtr_name, const char *criteria, long dest_plyr_idx)
{
    SCRIPTDBG(11, "Starting");
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  long select_id = get_rid(creature_select_criteria_desc, criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  command_add_value(Cmd_CHANGE_CREATURE_OWNER, origin_plyr_idx, crtr_id, select_id, dest_plyr_idx);
}

void command_add_to_flag(long plr_range_id, const char *flgname, long val)
{
    long flg_id = get_rid(flag_desc, flgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown flag, '%s'", flgname);
        return;
  }
  command_add_value(Cmd_ADD_TO_FLAG, plr_range_id, flg_id, val, 0);
}

void command_set_campaign_flag(long plr_range_id, const char *cmpflgname, long val)
{
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown campaign flag, '%s'", cmpflgname);
        return;
  }
  command_add_value(Cmd_SET_CAMPAIGN_FLAG, plr_range_id, flg_id, val, 0);
}

void command_add_to_campaign_flag(long plr_range_id, const char *cmpflgname, long val)
{
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown campaign flag, '%s'", cmpflgname);
        return;
  }
  command_add_value(Cmd_ADD_TO_CAMPAIGN_FLAG, plr_range_id, flg_id, val, 0);
}

void command_export_variable(long plr_range_id, const char *varib_name, const char *cmpflgname)
{
    long varib_type;
    long varib_id;
    // Recognize flag
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown CAMPAIGN FLAG, '%s'", cmpflgname);
        return;
    }
    // Recognize variable
    if (level_file_version > 0)
    {
        varib_type = get_id(variable_desc, varib_name);
    } else
    {
        varib_type = get_id(dk1_variable_desc, varib_name);
    }
    if (varib_type == -1)
        varib_id = -1;
    else
        varib_id = 0;
    if (varib_id == -1)
    {
        varib_id = get_id(creature_desc, varib_name);
        varib_type = SVar_CREATURE_NUM;
    }
    if (varib_id == -1)
    {
        varib_id = get_id(room_desc, varib_name);
        varib_type = SVar_ROOM_SLABS;
    }
    if (varib_id == -1)
    {
        varib_id = get_id(timer_desc, varib_name);
        varib_type = SVar_TIMER;
    }
    if (varib_id == -1)
    {
        varib_id = get_id(flag_desc, varib_name);
        varib_type = SVar_FLAG;
    }
    if (varib_id == -1)
    {
        varib_id = get_id(door_desc, varib_name);
        varib_type = SVar_DOOR_NUM;
    }
    if (varib_id == -1)
    {
        varib_id = get_id(trap_desc, varib_name);
        varib_type = SVar_TRAP_NUM;
    }
    if (varib_id == -1)
    {
        SCRPTERRLOG("Unknown VARIABLE, '%s'", varib_name);
        return;
    }
    command_add_value(Cmd_EXPORT_VARIABLE, plr_range_id, varib_type, varib_id, flg_id);
}

void command_set_game_rule(const char* objectv, unsigned long roomvar)
{
    long ruledesc = get_id(game_rule_desc, objectv);
    if (ruledesc == -1)
    {
        SCRPTERRLOG("Unknown room variable");
        return;
    }
    command_add_value(Cmd_SET_GAME_RULE, 0, ruledesc, roomvar, 0);
}

/** Adds a script command to in-game structures.
 *
 * @param cmd_desc
 * @param scline
 */
void script_add_command(const struct CommandDesc *cmd_desc, const struct ScriptLine *scline)
{
    switch (cmd_desc->index)
    {
    case Cmd_CREATE_PARTY:
        command_create_party(scline->tp[0]);
        break;
    case Cmd_ADD_TO_PARTY:
        command_add_to_party(scline->tp[0], scline->tp[1], scline->np[2], scline->np[3], scline->tp[4], scline->np[5]);
        break;
    case Cmd_ADD_PARTY_TO_LEVEL:
        command_add_party_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_ADD_CREATURE_TO_LEVEL:
        command_add_creature_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5]);
        break;
    case Cmd_IF:
        command_if(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
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
        command_set_start_money(scline->np[0], scline->np[1]);
        break;
    case Cmd_ROOM_AVAILABLE:
        command_room_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        break;
    case Cmd_CREATURE_AVAILABLE:
        if (level_file_version > 0) {
            command_creature_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        } else {
            command_creature_available(scline->np[0], scline->tp[1], scline->np[3], 0);
        }
        break;
    case Cmd_MAGIC_AVAILABLE:
        command_magic_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        break;
    case Cmd_TRAP_AVAILABLE:
        command_trap_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        break;
    case Cmd_RESEARCH:
        command_research(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_RESEARCH_ORDER:
        command_research_order(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_COMPUTER_PLAYER:
        command_computer_player(scline->np[0], scline->np[1]);
        break;
    case Cmd_SET_TIMER:
        command_set_timer(scline->np[0], scline->tp[1]);
        break;
    case Cmd_IF_ACTION_POINT:
        command_if_action_point(scline->np[0], scline->np[1]);
        break;
    case Cmd_ADD_TUNNELLER_TO_LEVEL:
        command_add_tunneller_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5]);
        break;
    case Cmd_WIN_GAME:
        command_win_game();
        break;
    case Cmd_LOSE_GAME:
        command_lose_game();
        break;
    case Cmd_SET_FLAG:
        command_set_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_MAX_CREATURES:
        command_max_creatures(scline->np[0], scline->np[1]);
        break;
    case Cmd_NEXT_COMMAND_REUSABLE:
        next_command_reusable = 2;
        break;
    case Cmd_DOOR_AVAILABLE:
        command_door_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        break;
    case Cmd_DISPLAY_OBJECTIVE:
        command_display_objective(scline->np[0], scline->tp[1], 0, 0);
        break;
    case Cmd_DISPLAY_INFORMATION:
        if (level_file_version > 0)
          command_display_information(scline->np[0], scline->tp[1], 0, 0);
        else
          command_display_information(scline->np[0], "ALL_PLAYERS", 0, 0);
        break;
    case Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL:
        command_add_tunneller_party_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->tp[3], scline->np[4], scline->np[5], scline->np[6]);
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
        command_set_creature_max_level(scline->np[0], scline->tp[1], scline->np[2]);
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
    case Cmd_SET_CREATURE_FEAR_WOUNDED:
        if (level_file_version > 0)
            command_set_creature_fear_wounded(scline->tp[0], scline->np[1]);
        else
            command_set_creature_fear_wounded(scline->tp[0], 101*scline->np[1]/255); // old fear was scaled 0..255
        break;
    case Cmd_SET_CREATURE_FEAR_STRONGER:
        command_set_creature_fear_stronger(scline->tp[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_FEARSOME_FACTOR:
        command_set_creature_fearsome_factor(scline->tp[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_PROPERTY:
        command_set_creature_property(scline->tp[0], scline->np[1], scline->np[2]);
        break;
    case Cmd_DISPLAY_OBJECTIVE_WITH_POS:
        command_display_objective(scline->np[0], NULL, scline->np[1], scline->np[2]);
        break;
    case Cmd_IF_AVAILABLE:
        command_if_available(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_IF_CONTROLS:
        command_if_controls(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_IF_SLAB_OWNER:
        command_if_slab_owner(scline->np[0], scline->np[1], scline->np[2]);
        break;
    case Cmd_IF_SLAB_TYPE:
        command_if_slab_type(scline->np[0], scline->np[1], scline->np[2]);
        break;
    case Cmd_SET_COMPUTER_GLOBALS:
        command_set_computer_globals(scline->np[0], scline->np[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
        break;
    case Cmd_SET_COMPUTER_CHECKS:
        command_set_computer_checks(scline->np[0], scline->tp[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
        break;
    case Cmd_SET_COMPUTER_EVENT:
        command_set_computer_events(scline->np[0], scline->tp[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
        break;
    case Cmd_SET_COMPUTER_PROCESS:
        command_set_computer_process(scline->np[0], scline->tp[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6]);
        break;
    case Cmd_ALLY_PLAYERS:
        if (level_file_version > 0)
            command_ally_players(scline->np[0], scline->np[1], scline->np[2]);
        else
            command_ally_players(scline->np[0], scline->np[1], true);
        break;
    case Cmd_DEAD_CREATURES_RETURN_TO_POOL:
        command_dead_creatures_return_to_pool(scline->np[0]);
        break;
    case Cmd_DISPLAY_INFORMATION_WITH_POS:
        command_display_information(scline->np[0], NULL, scline->np[1], scline->np[2]);
        break;
    case Cmd_BONUS_LEVEL_TIME:
        command_bonus_level_time(scline->np[0]);
        break;
    case Cmd_QUICK_OBJECTIVE:
        command_quick_objective(scline->np[0], scline->tp[1], scline->tp[2], 0, 0);
        break;
    case Cmd_QUICK_INFORMATION:
        if (level_file_version > 0)
          command_quick_information(scline->np[0], scline->tp[1], scline->tp[2], 0, 0);
        else
          command_quick_information(scline->np[0], scline->tp[1], "ALL_PLAYERS", 0, 0);
        break;
    case Cmd_QUICK_OBJECTIVE_WITH_POS:
        command_quick_objective(scline->np[0], scline->tp[1], NULL, scline->np[2], scline->np[3]);
        break;
    case Cmd_QUICK_INFORMATION_WITH_POS:
        command_quick_information(scline->np[0], scline->tp[1], NULL, scline->np[2], scline->np[3]);
        break;
    case Cmd_SWAP_CREATURE:
        command_swap_creature(scline->tp[0], scline->tp[1]);
        break;
    case Cmd_PRINT:
        command_message(scline->tp[0],80);
        break;
    case Cmd_MESSAGE:
        command_message(scline->tp[0],68);
        break;
    case Cmd_PLAY_MESSAGE:
        command_play_message(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_ADD_GOLD_TO_PLAYER:
        command_add_gold_to_player(scline->np[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_TENDENCIES:
        command_set_creature_tendencies(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_REVEAL_MAP_RECT:
        command_reveal_map_rect(scline->np[0], scline->np[1], scline->np[2], scline->np[3], scline->np[4]);
        break;
    case Cmd_REVEAL_MAP_LOCATION:
        command_reveal_map_location(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_KILL_CREATURE:
        command_kill_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_LEVEL_UP_CREATURE:
        command_level_up_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_USE_POWER_ON_CREATURE:
        command_use_power_on_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->tp[4], scline->np[5], scline->np[6]);
        break;
    case Cmd_USE_POWER_AT_LOCATION:
        command_use_power_at_location(scline->np[0], scline->np[1], scline->np[2], scline->tp[3], scline->np[4], scline->np[5]);
        break;
    case Cmd_USE_POWER:
        command_use_power(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_CHANGE_CREATURE_OWNER:
        command_change_creature_owner(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_LEVEL_VERSION:
        level_file_version = scline->np[0];
        SCRPTLOG("Level files version %d.",level_file_version);
        break;
    case Cmd_ADD_TO_FLAG:
        command_add_to_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_SET_CAMPAIGN_FLAG:
        command_set_campaign_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_ADD_TO_CAMPAIGN_FLAG:
        command_add_to_campaign_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_EXPORT_VARIABLE:
        command_export_variable(scline->np[0], scline->tp[1], scline->tp[2]);
        break;
    case Cmd_RUN_AFTER_VICTORY:
        if (scline->np[0] == 1)
        {
            game.system_flags |= GSF_RunAfterVictory;
        }
        break;
    case Cmd_SET_GAME_RULE:
        command_set_game_rule(scline->tp[0], scline->np[1]);
        break;
    case Cmd_SET_TRAP_CONFIGURATION:
        command_set_trap_configuration(scline->tp[0], scline->np[1], scline->np[2], scline->np[3], scline->np[4], scline->np[5], scline->np[6], scline->np[7]);
        break;
    case Cmd_SET_DOOR_CONFIGURATION:
        command_set_door_configuration(scline->tp[0], scline->np[1], scline->np[2], scline->np[3], scline->np[4]);
        break;
    case Cmd_CHANGE_SLAB_OWNER:
        command_change_slab_owner(scline->np[0], scline->np[1], scline->np[2]);
        break;
    case Cmd_CHANGE_SLAB_TYPE:
        command_change_slab_type(scline->np[0], scline->np[1], scline->np[2]);
        break;
    default:
        SCRPTERRLOG("Unhandled SCRIPT command '%s'", scline->tcmnd);
        break;
    }
}

TbBool script_command_param_to_number(char type_chr, struct ScriptLine *scline, int idx)
{
    switch (toupper(type_chr))
    {
    case 'N':
    {
        char* text;
        scline->np[idx] = strtol(scline->tp[idx], &text, 0);
        if (text != &scline->tp[idx][strlen(scline->tp[idx])]) {
            SCRPTWRNLOG("Numerical value \"%s\" interpreted as %ld", scline->tp[idx], scline->np[idx]);
        }
        break;
    }
    case 'P':{
        long plr_range_id;
        if (!get_player_id(scline->tp[idx], &plr_range_id)) {
            return false;
        }
        scline->np[idx] = plr_range_id;
        };break;
    case 'C':{
        long crtr_id = get_rid(creature_desc, scline->tp[idx]);
        if (crtr_id == -1) {
            SCRPTERRLOG("Unknown creature, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = crtr_id;
        };break;
    case 'R':{
        long room_id = get_rid(room_desc, scline->tp[idx]);
        if (room_id == -1)
        {
            SCRPTERRLOG("Unknown room kind, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = room_id;
        };break;
    case 'S': {
        long slab_id = get_rid(slab_desc, scline->tp[idx]);
        if (slab_id == -1)
        {
            SCRPTERRLOG("Unknown slab kind, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = slab_id;
    }; break;
    case 'L':{
        TbMapLocation loc;
        if (!get_map_location_id(scline->tp[idx], &loc)) {
            return false;
        }
        scline->np[idx] = loc;
        };break;
    case 'O':{
        long opertr_id = get_rid(comparison_desc, scline->tp[idx]);
        if (opertr_id == -1) {
            SCRPTERRLOG("Unknown operator, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = opertr_id;
        };break;
    case 'X': {
        long prop_id = get_rid(creatmodel_properties_commands, scline->tp[idx]);
        if (prop_id == -1)
        {
            SCRPTERRLOG("Unknown creature property kind, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = prop_id;
    }; break;
    case 'A':
        break;
    default:
        return false;
    }
    return true;
}

TbBool script_command_param_to_text(char type_chr, struct ScriptLine *scline, int idx)
{
    switch (toupper(type_chr))
    {
    case 'N':
        itoa(scline->np[idx], scline->tp[idx], 10);
        break;
    case 'P':
        strcpy(scline->tp[idx], player_code_name(scline->np[idx]));
        break;
    case 'C':
        strcpy(scline->tp[idx], creature_code_name(scline->np[idx]));
        break;
    case 'R':
        strcpy(scline->tp[idx], room_code_name(scline->np[idx]));
        break;
    case 'L':
        get_map_location_code_name(scline->np[idx], scline->tp[idx]);
        break;
    case 'A':
        break;
    default:
        return false;
    }
    return true;
}

int script_recognize_params(char **line, const struct CommandDesc *cmd_desc, struct ScriptLine *scline, int *para_level, int expect_level)
{
    int i;
    for (i=0; i <= COMMANDDESC_ARGS_COUNT; i++)
    {
        char chr = cmd_desc->args[i];
        if (*para_level < expect_level)
            break;
        // Read the next parameter
        const struct CommandDesc *funcmd_desc;
        {
            char* funline = *line;
            int funpara_level = *para_level;
            char funcmd_buf[MAX_TEXT_LENGTH];
            LbMemorySet(funcmd_buf, 0, MAX_TEXT_LENGTH);
            funcmd_desc = get_next_word(&funline, funcmd_buf, &funpara_level, subfunction_desc);
            if (funpara_level < expect_level+1) {
                // Break the loop keeping variables as if the parameter wasn't read
                break;
            }
            if (funpara_level > (*para_level)+(i > 0 ? 0 : 1)) {
                SCRPTWRNLOG("Unexpected paraenesis in parameter %d of command \"%s\"", i+1, scline->tcmnd);
            }
            *line = funline;
            *para_level = funpara_level;
            if (!isalpha(chr)) {
                // Don't show parameter index - it may be bad, as we're decreasing i to not overflow cmd_desc->args
                SCRPTWRNLOG("Excessive parameter of command \"%s\", value \"%s\"; ignoring", scline->tcmnd, funcmd_buf);
                i--;
                continue;
            }
            // Access tp[i] only if we're sure i < COMMANDDESC_ARGS_COUNT
            LbMemoryCopy(scline->tp[i],  funcmd_buf, MAX_TEXT_LENGTH);
        }
        if (funcmd_desc != NULL)
        {
            struct ScriptLine* funscline = (struct ScriptLine*)LbMemoryAlloc(sizeof(struct ScriptLine));
            if (funscline == NULL) {
                SCRPTERRLOG("Can't allocate buffer to recognize line");
                return -1;
            }
            LbMemorySet(funscline, 0, sizeof(struct ScriptLine));
            LbMemoryCopy(funscline->tcmnd,  scline->tp[i], MAX_TEXT_LENGTH);
            int args_count = script_recognize_params(line, funcmd_desc, funscline, para_level, *para_level);
            if (args_count < 0)
            {
                LbMemoryFree(funscline);
                return -1;
            }
            if (args_count < COMMANDDESC_ARGS_COUNT)
            {
                chr = funcmd_desc->args[args_count];
                if (isupper(chr)) // Required arguments have upper-case type letters
                {
                  SCRPTERRLOG("Not enough parameters for \"%s\", got only %d", funcmd_desc->textptr,(int)args_count);
                  LbMemoryFree(funscline);
                  return -1;
                }
            }
            switch (funcmd_desc->index)
            {
            case Cmd_RANDOM:
            case Cmd_DRAWFROM:{
                // Create array of value ranges
                long range_total = 0;
                int fi;
                struct MinMax ranges[COMMANDDESC_ARGS_COUNT];
                if (level_file_version > 0)
                {
                    chr = cmd_desc->args[i];
                    int ri;
                    for (fi = 0, ri = 0; fi < COMMANDDESC_ARGS_COUNT; fi++, ri++)
                    {
                        if (funscline->tp[fi][0] == '\0') {
                            break;
                        }
                        if (toupper(chr) == 'A')
                        {
                            // Values which do not support range
                            if (strcmp(funscline->tp[fi],"~") == 0) {
                                SCRPTERRLOG("Parameter %d of function \"%s\" within command \"%s\" does not support range", fi+1, funcmd_desc->textptr, scline->tcmnd);
                                LbMemoryFree(funscline);
                                return -1;
                            }
                            // Values of that type cannot define ranges, as we cannot interpret them
                            ranges[ri].min = fi;
                            ranges[ri].max = fi;
                            range_total += 1;
                        } else
                        if ((ri > 0) && (strcmp(funscline->tp[fi],"~") == 0))
                        {
                            // Second step of defining range
                            ri--;
                            fi++;
                            if (!script_command_param_to_number(chr, funscline, fi)) {
                                SCRPTERRLOG("Parameter %d of function \"%s\" within command \"%s\" has unexpected range end value; discarding command", fi+1, funcmd_desc->textptr, scline->tcmnd);
                                LbMemoryFree(funscline);
                                return -1;
                            }
                            ranges[ri].max = funscline->np[fi];
                            if (ranges[ri].max < ranges[ri].min) {
                                SCRPTWRNLOG("Range definition in argument of function \"%s\" within command \"%s\" should have lower value first", funcmd_desc->textptr, scline->tcmnd);
                                ranges[ri].max = ranges[ri].min;
                            }
                            range_total += ranges[ri].max - ranges[ri].min; // +1 was already added
                        } else
                        {
                            // Single value or first step of defining range
                            if (!script_command_param_to_number(chr, funscline, fi)) {
                                SCRPTERRLOG("Parameter %d of function \"%s\" within command \"%s\" has unexpected value; discarding command", fi+1, funcmd_desc->textptr, scline->tcmnd);
                                LbMemoryFree(funscline);
                                return -1;
                            }
                            ranges[ri].min = funscline->np[fi];
                            ranges[ri].max = funscline->np[fi];
                            range_total += 1;
                        }
                    }
                } else
                {
                    // Old RANDOM command accepts only one range, and gives only numbers
                    fi = 0;
                    {
                        ranges[fi].min = atol(funscline->tp[0]);
                        ranges[fi].max = atol(funscline->tp[1]);
                    }
                    if (ranges[fi].max < ranges[fi].min) {
                        SCRPTWRNLOG("Range definition in argument of function \"%s\" within command \"%s\" should have lower value first", funcmd_desc->textptr, scline->tcmnd);
                        ranges[fi].max = ranges[fi].min;
                    }
                    range_total += ranges[fi].max - ranges[fi].min + 1;
                    fi++;
                }
                if (range_total <= 0) {
                    SCRPTERRLOG("Arguments of function \"%s\" within command \"%s\" define no values to select from", funcmd_desc->textptr, scline->tcmnd);
                    break;
                }
                if ((funcmd_desc->index != Cmd_RANDOM) && (level_file_version == 0)) {
                    SCRPTERRLOG("The function \"%s\" used within command \"%s\" is not supported in old level format", funcmd_desc->textptr, scline->tcmnd);
                    break;
                }
                // The new RANDOM command stores values to allow selecting different one every turn during gameplay
                if ((funcmd_desc->index == Cmd_RANDOM) && (level_file_version > 0))
                {
                    //TODO RANDOM make implementation - store ranges as variable to be used for selecting random value during gameplay
                    SCRPTERRLOG("The function \"%s\" used within command \"%s\" is not supported yet", funcmd_desc->textptr, scline->tcmnd);
                    break;
                }
                // DRAWFROM support - select random index now
                long range_index = rand() % range_total;
                // Get value from ranges array
                range_total = 0;
                for (fi=0; fi < COMMANDDESC_ARGS_COUNT; fi++)
                {
                    if ((range_index >= range_total) && (range_index <= range_total + ranges[fi].max - ranges[fi].min)) {
                        chr = cmd_desc->args[i];
                        if (toupper(chr) == 'A') {
                            strcpy(scline->tp[i], funscline->tp[ranges[fi].min]);
                        } else {
                            scline->np[i] = ranges[fi].min + range_index - range_total;
                            // Set text value for that number
                            script_command_param_to_text(chr, scline, i);
                        }
                        break;
                    }
                    range_total += ranges[fi].max - ranges[fi].min + 1;
                }
                SCRPTLOG("Function \"%s\" returned value \"%s\"", funcmd_desc->textptr, scline->tp[i]);
                };break;
            case Cmd_IMPORT:
            {
                long player_id = get_id(player_desc, funscline->tp[0]);
                if (player_id >= PLAYERS_FOR_CAMPAIGN_FLAGS)
                {
                    SCRPTERRLOG("Cannot fetch flag values for player, '%s'", funscline->tp[0]);
                    strcpy(scline->tp[i], "0");
                    break;
                }
                long flag_id = get_id(campaign_flag_desc, funscline->tp[1]);
                if (flag_id == -1)
                {
                    SCRPTERRLOG("Unknown campaign flag name, '%s'", funscline->tp[1]);
                    strcpy(scline->tp[i], "0");
                    break;
                }
                SCRPTLOG("Function \"%s\" returned value \"%ld\"", funcmd_desc->textptr,
                    intralvl.campaign_flags[player_id][flag_id]);
                ltoa(intralvl.campaign_flags[player_id][flag_id], scline->tp[i], 10);
                break;
            }
            default:
                SCRPTWRNLOG("Parameter value \"%s\" is a command which isn't supported as function", scline->tp[i]);
                break;
            }
            LbMemoryFree(funscline);
        }
        if (scline->tp[i][0] == '\0') {
          break;
        }
        if (*para_level > expect_level+2) {
            SCRPTWRNLOG("Parameter %d of command \"%s\", value \"%s\", is at too high paraenesis level %d", i+1, scline->tcmnd, scline->tp[i], (int)*para_level);
        }
        chr = cmd_desc->args[i];
        if (!script_command_param_to_number(chr, scline, i)) {
            SCRPTERRLOG("Parameter %d of command \"%s\", type %c, has unexpected value; discarding command", i+1, scline->tcmnd, chr);
            return -1;
        }
    }
    return i;
}

long script_scan_line(char *line,TbBool preloaded)
{
    const struct CommandDesc *cmd_desc;
    SCRIPTDBG(12,"Starting");
    struct ScriptLine* scline = (struct ScriptLine*)LbMemoryAlloc(sizeof(struct ScriptLine));
    if (scline == NULL)
    {
      SCRPTERRLOG("Can't allocate buffer to recognize line");
      return 0;
    }
    int para_level = 0;
    LbMemorySet(scline, 0, sizeof(struct ScriptLine));
    if (next_command_reusable > 0)
        next_command_reusable--;
    if (level_file_version > 0)
    {
        cmd_desc = get_next_word(&line, scline->tcmnd, &para_level, command_desc);
    } else
    {
        cmd_desc = get_next_word(&line, scline->tcmnd, &para_level, dk1_command_desc);
    }
    if (cmd_desc == NULL)
    {
        if (isalnum(scline->tcmnd[0])) {
          SCRPTERRLOG("Invalid command, '%s' (lev ver %d)", scline->tcmnd,level_file_version);
        }
        LbMemoryFree(scline);
        return 0;
    }
    SCRIPTDBG(12,"Executing command %lu",cmd_desc->index);
    // Handling comments
    if (cmd_desc->index == Cmd_REM)
    {
        LbMemoryFree(scline);
        return 0;
    }
    // selecting only preloaded/not preloaded commands
    if (script_is_preloaded_command(cmd_desc->index) != preloaded)
    {
        LbMemoryFree(scline);
        return 0;
    }
    // Recognizing parameters
    int args_count = script_recognize_params(&line, cmd_desc, scline, &para_level, 0);
    if (args_count < 0)
    {
        LbMemoryFree(scline);
        return -1;
    }
    if (args_count < COMMANDDESC_ARGS_COUNT)
    {
        char chr = cmd_desc->args[args_count];
        if (isupper(chr)) // Required arguments have upper-case type letters
        {
            SCRPTERRLOG("Not enough parameters for \"%s\", got only %d", cmd_desc->textptr,(int)args_count);
            LbMemoryFree(scline);
            return -1;
        }
    }
    script_add_command(cmd_desc, scline);
    LbMemoryFree(scline);
    SCRIPTDBG(13,"Finished");
    return 0;
}

short clear_script(void)
{
    LbMemorySet(&game.script, 0, sizeof(struct LevelScript));
    script_current_condition = -1;
    text_line_number = 1;
    return true;
}

short clear_quick_messages(void)
{
    for (long i = 0; i < QUICK_MESSAGES_COUNT; i++)
        LbMemorySet(gameadd.quick_messages[i], 0, MESSAGE_TEXT_LEN);
    return true;
}

short preload_script(long lvnum)
{
  SYNCDBG(7,"Starting");
  script_current_condition = -1;
  next_command_reusable = 0;
  text_line_number = 1;
  level_file_version = DEFAULT_LEVEL_VERSION;
  clear_quick_messages();
  // Load the file
  long script_len = 1;
  char* script_data = (char*)load_single_map_file_to_buffer(lvnum, "txt", &script_len, LMFF_None);
  if (script_data == NULL)
    return false;
  // Process the file lines
  char* buf = script_data;
  char* buf_end = script_data + script_len;
  while (buf < buf_end)
  {
    // Find end of the line
    int lnlen = 0;
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
    //SCRPTLOG("Analyse");
    // Analyze the line
    script_scan_line(buf, true);
    // Set new line start
    text_line_number++;
    buf += lnlen;
  }
  LbMemoryFree(script_data);
  SYNCDBG(8,"Finished");
  return true;
}

short load_script(long lvnum)
{
    SYNCDBG(7,"Starting");

    // Clear script data
    gui_set_button_flashing(0, 0);
    clear_script();
    script_current_condition = -1;
    next_command_reusable = 0;
    text_line_number = 1;
    game.bonus_time = 0;
    game.flags_gui &= ~GGUI_CountdownTimer;
    game.flags_cd |= MFlg_DeadBackToPool;
    reset_creature_max_levels();
    reset_script_timers_and_flags();
    if ((game.operation_flags & GOF_ColumnConvert) != 0)
    {
        convert_old_column_file(lvnum);
        game.operation_flags &= ~GOF_ColumnConvert;
    }
    // Load the file
    long script_len = 1;
    char* script_data = (char*)load_single_map_file_to_buffer(lvnum, "txt", &script_len, LMFF_None);
    if (script_data == NULL)
      return false;
    // Process the file lines
    char* buf = script_data;
    char* buf_end = script_data + script_len;
    while (buf < buf_end)
    {
      // Find end of the line
      int lnlen = 0;
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
      script_scan_line(buf, false);
      // Set new line start
      text_line_number++;
      buf += lnlen;
    }
    LbMemoryFree(script_data);
    if (game.script.win_conditions_num == 0)
      WARNMSG("No WIN GAME conditions in script file.");
    if (script_current_condition != -1)
      WARNMSG("Missing ENDIF's in script file.");
    JUSTLOG("Used script resources: %d/%d tunneller triggers, %d/%d party triggers, %d/%d script values, %d/%d IF conditions, %d/%d party definitions",
        (int)game.script.tunneller_triggers_num,TUNNELLER_TRIGGERS_COUNT,
        (int)game.script.party_triggers_num,PARTY_TRIGGERS_COUNT,
        (int)game.script.values_num,SCRIPT_VALUES_COUNT,
        (int)game.script.conditions_num,CONDITIONS_COUNT,
        (int)game.script.creature_partys_num,CREATURE_PARTYS_COUNT);
    return true;
}

void script_process_win_game(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    set_player_as_won_level(player);
}

void script_process_lose_game(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    set_player_as_lost_level(player);
}

struct Thing *create_thing_at_position_then_move_to_valid_and_add_light(struct Coord3d *pos, unsigned char tngclass, unsigned char tngmodel, unsigned char tngowner)
{
    struct Thing* thing = create_thing(pos, tngclass, tngmodel, tngowner, -1);
    if (thing_is_invalid(thing))
    {
        return INVALID_THING;
    }
    thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);
    // Try to move thing out of the solid wall if it's inside one
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        if (!move_creature_to_nearest_valid_position(thing)) {
            ERRORLOG("The %s was created in wall, removing",thing_model_name(thing));
            delete_thing_structure(thing, 0);
            return INVALID_THING;
        }
    }

    if (thing_is_creature(thing))
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        cctrl->flee_pos.x.val = thing->mappos.x.val;
        cctrl->flee_pos.y.val = thing->mappos.y.val;
        cctrl->flee_pos.z.val = thing->mappos.z.val;
        cctrl->flee_pos.z.val = get_thing_height_at(thing, &thing->mappos);
        cctrl->party.target_plyr_idx = -1;
    }

    long light_rand = ACTION_RANDOM(8);
    if (light_rand < 2)
    {
        struct InitLight ilght;
        LbMemorySet(&ilght, 0, sizeof(struct InitLight));
        ilght.mappos.x.val = thing->mappos.x.val;
        ilght.mappos.y.val = thing->mappos.y.val;
        ilght.mappos.z.val = thing->mappos.z.val;
        if (light_rand == 1)
        {
            ilght.field_2 = 48;
            ilght.field_3 = 5;
        } else
        {
            ilght.field_2 = 36;
            ilght.field_3 = 1;
        }
        ilght.is_dynamic = 1;
        ilght.field_0 = 2560;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id != 0) {
            light_set_light_never_cache(thing->light_id);
        } else {
            ERRORLOG("Cannot allocate light to new hero");
        }
    }
    return thing;
}

static TbBool get_coords_at_hero_door(struct Coord3d *pos, long gate_num, unsigned char random_factor)
{
    SYNCDBG(7,"Starting at HG%d", (int)gate_num);
    if (gate_num <= 0)
    {
        ERRORLOG("Script error - invalid hero gate index %d",(int)gate_num);
        return false;
    }
    struct Thing* gatetng = find_hero_gate_of_number(gate_num);
    if (thing_is_invalid(gatetng))
    {
        ERRORLOG("Script error - attempt to create thing at non-existing hero gate index %d",(int)gate_num);
        return false;
    }
    pos->x.val = gatetng->mappos.x.val;
    pos->y.val = gatetng->mappos.y.val;
    pos->z.val = gatetng->mappos.z.val + 384;
    return true;
}

static TbBool get_coords_at_action_point(struct Coord3d *pos, long apt_idx, unsigned char random_factor)
{
    SYNCDBG(7,"Starting at action point %d", (int)apt_idx);

    struct ActionPoint* apt = action_point_get(apt_idx);
    if (!action_point_exists(apt))
    {
        ERRORLOG("Script error - attempt to create thing at non-existing action point %d",(int)apt_idx);
        return false;
    }

    if ( (random_factor == 0) || (apt->range == 0) )
    {
        pos->x.val = apt->mappos.x.val;
        pos->y.val = apt->mappos.y.val;
    } else
    {
        long direction = ACTION_RANDOM(2 * LbFPMath_PI);
        long delta_x = (apt->range * LbSinL(direction) >> 8);
        long delta_y = (apt->range * LbCosL(direction) >> 8);
        pos->x.val = apt->mappos.x.val + (delta_x >> 8);
        pos->y.val = apt->mappos.y.val - (delta_y >> 8);
    }
    return true;
}

/**
 * Creates a thing on given players dungeon heart.
 * Originally was script_support_create_creature_at_dungeon_heart().
 * @param plyr_idx
 */
TbBool get_coords_at_dungeon_heart(struct Coord3d *pos, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting at player %d", (int)plyr_idx);
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng))
    {
        ERRORLOG("Script error - attempt to create thing in player %d dungeon with no heart",(int)plyr_idx);
        return false;
    }
    pos->x.val = heartng->mappos.x.val + ACTION_RANDOM(65) - 32;
    pos->y.val = heartng->mappos.y.val + ACTION_RANDOM(65) - 32;
    pos->z.val = heartng->mappos.z.val;
    return true;
}

TbBool get_coords_at_meta_action(struct Coord3d *pos, PlayerNumber plyr_idx, long i)
{
    SYNCDBG(7,"Starting at player %d", (int)plyr_idx);
    struct Coord3d *src;
    PlayerNumber loc_player = i & 0xF;
    if (loc_player == 15) // CURRENT_PLAYER
        loc_player = gameadd.script_current_player;

    struct DungeonAdd* dungeonadd = get_dungeonadd(loc_player);

    switch (i >> 8)
    {
    case MML_TRIGGERED_OBJECT:
        src = &gameadd.box_activation_location;
        break;
    case MML_RECENT_COMBAT:
        src = &dungeonadd->last_combat_location;
        break;
    default:
        return false;
    }

    pos->x.val = src->x.val + ACTION_RANDOM(33) - 16;
    pos->y.val = src->y.val + ACTION_RANDOM(33) - 16;
    pos->z.val = src->z.val;

    return true;
}

long send_tunneller_to_point(struct Thing *thing, struct Coord3d *pos)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->party.target_plyr_idx = -1;
    setup_person_tunnel_to_position(thing, pos->x.stl.num, pos->y.stl.num, 0);
    thing->continue_state = CrSt_TunnellerDoingNothing;
    return 1;
}

TbBool script_support_send_tunneller_to_action_point(struct Thing *thing, long apt_idx)
{
    SYNCDBG(7,"Starting");
    struct ActionPoint* apt = action_point_get(apt_idx);
    struct Coord3d pos;
    if (action_point_exists(apt)) {
        pos.x.val = apt->mappos.x.val;
        pos.y.val = apt->mappos.y.val;
    } else {
        ERRORLOG("Attempt to send to non-existing action point %d",(int)apt_idx);
        pos.x.val = subtile_coord_center(map_subtiles_x/2);
        pos.y.val = subtile_coord_center(map_subtiles_y/2);
    }
    pos.z.val = subtile_coord(1,0);
    send_tunneller_to_point(thing, &pos);
    return true;
}

TbBool script_support_send_tunneller_to_dungeon(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Send %s to player %d",thing_model_name(creatng),(int)plyr_idx);
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng))
    {
        WARNLOG("Tried to send %s to player %d which has no heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    struct Coord3d pos;
    if (!get_random_position_in_dungeon_for_creature(plyr_idx, CrWaS_WithinDungeon, creatng, &pos)) {
        WARNLOG("Tried to send %s to player %d but can't find position", thing_model_name(creatng), (int)plyr_idx);
        return send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &heartng->mappos);
    }
    if (!send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &pos)) {
        WARNLOG("Tried to send %s to player %d but can't start the task", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    SYNCDBG(17,"Moving %s to (%d,%d)",thing_model_name(creatng),(int)pos.x.stl.num,(int)pos.y.stl.num);
    return true;
}

TbBool script_support_send_tunneller_to_dungeon_heart(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Send %s to player %d",thing_model_name(creatng),(int)plyr_idx);
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng)) {
        WARNLOG("Tried to send %s to player %d which has no heart", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    if (!send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &heartng->mappos)) {
        WARNLOG("Tried to send %s to player %d but can't start the task", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    SYNCDBG(17,"Moving %s to (%d,%d)",thing_model_name(creatng),(int)heartng->mappos.x.stl.num,(int)heartng->mappos.y.stl.num);
    return true;
}

TbBool script_support_send_tunneller_to_appropriate_dungeon(struct Thing *creatng)
{
    SYNCDBG(7,"Starting");
    //return _DK_script_support_send_tunneller_to_appropriate_dungeon(thing);
    PlayerNumber plyr_idx;
    struct Coord3d pos;
    plyr_idx = get_best_dungeon_to_tunnel_to(creatng);
    if (plyr_idx == -1) {
        ERRORLOG("Could not find appropriate dungeon to send %s to",thing_model_name(creatng));
        return false;
    }
    if (!get_random_position_in_dungeon_for_creature(plyr_idx, CrWaS_WithinDungeon, creatng, &pos)) {
        WARNLOG("Tried to send %s to player %d but can't find position", thing_model_name(creatng), (int)plyr_idx);
        return false;
    }
    return send_tunneller_to_point_in_dungeon(creatng, plyr_idx, &pos);
}

static struct Thing *script_create_creature_at_location(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location)
{
    long effect;
    long i = get_map_location_longval(location);
    struct Coord3d pos;
    TbBool fall_from_gate = false;

    const unsigned char tngclass = TCls_Creature;

    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:
        if (!get_coords_at_action_point(&pos, i, 1))
        {
            return INVALID_THING;
        }
        effect = 1;
        break;
    case MLoc_HEROGATE:
        if (!get_coords_at_hero_door(&pos, i, 1))
        {
            return INVALID_THING;
        }
        effect = 0;
        fall_from_gate = true;
        break;
    case MLoc_PLAYERSHEART:
        if (!get_coords_at_dungeon_heart(&pos, i))
        {
            return INVALID_THING;
        }
        effect = 0;
        break;
    case MLoc_METALOCATION:
        if (!get_coords_at_meta_action(&pos, plyr_idx, i))
        {
            return INVALID_THING;
        }
        effect = 0;
        break;      
    case MLoc_CREATUREKIND:
    case MLoc_OBJECTKIND:
    case MLoc_ROOMKIND:
    case MLoc_THING:
    case MLoc_PLAYERSDUNGEON:
    case MLoc_APPROPRTDUNGEON:
    case MLoc_DOORKIND:
    case MLoc_TRAPKIND:
    case MLoc_NONE:
    default:
        effect = 0;
        return INVALID_THING;
    }
    struct Thing* thing = create_thing_at_position_then_move_to_valid_and_add_light(&pos, tngclass, crmodel, plyr_idx);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Couldn't create %s at location %d",thing_class_and_model_name(tngclass, crmodel),(int)location);
            // Error is already logged
        return INVALID_THING;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (fall_from_gate)
    {
        cctrl->field_AE |= 0x02;
        cctrl->spell_flags |= CSAfF_MagicFall;
        thing->veloc_push_add.x.val += ACTION_RANDOM(193) - 96;
        thing->veloc_push_add.y.val += ACTION_RANDOM(193) - 96;
        if ((thing->movement_flags & TMvF_Flying) != 0) {
            thing->veloc_push_add.z.val -= ACTION_RANDOM(32);
        } else {
            thing->veloc_push_add.z.val += ACTION_RANDOM(96) + 80;
        }
        thing->state_flags |= TF1_PushAdd;
    }

    if (thing->owner != PLAYER_NEUTRAL)
    {   // Was set only when spawned from action point

        struct Thing* heartng = get_player_soul_container(thing->owner);
        if (thing_exists(heartng) && creature_can_navigate_to(thing, &heartng->mappos, NavRtF_NoOwner))
        {
            cctrl->field_AE |= 0x01;
        }
    }
    
    if ((get_creature_model_flags(thing) & CMF_IsLordOTLand) != 0)
    {
        output_message(SMsg_LordOfLandComming, MESSAGE_DELAY_LORD, 1);
        output_message(SMsg_EnemyLordQuote + ACTION_RANDOM(8), MESSAGE_DELAY_LORD, 1);
    }
    switch (effect)
    {
    case 1:
        if (plyr_idx == game.hero_player_num)
        {
            thing->mappos.z.val = get_ceiling_height(&thing->mappos);
            create_effect(&thing->mappos, TngEff_Unknown36, thing->owner);
            initialise_thing_state(thing, CrSt_CreatureHeroEntering);
            thing->field_4F |= TF4F_Unknown01;
            cctrl->countdown_282 = 24;
        }
    default:
        break;
    }
    return thing;
}

struct Thing *script_process_new_tunneler(unsigned char plyr_idx, TbMapLocation location, TbMapLocation heading, unsigned char crtr_level, unsigned long carried_gold)
{
    ThingModel diggerkind = get_players_special_digger_model(game.hero_player_num);
    struct Thing* creatng = script_create_creature_at_location(plyr_idx, diggerkind, location);
    if (thing_is_invalid(creatng))
        return INVALID_THING;
    creatng->creature.gold_carried = carried_gold;
    init_creature_level(creatng, crtr_level);
    switch (get_map_location_type(heading))
    {
    case MLoc_ACTIONPOINT:
        script_support_send_tunneller_to_action_point(creatng, get_map_location_longval(heading));
        break;
    case MLoc_PLAYERSDUNGEON:
        script_support_send_tunneller_to_dungeon(creatng, get_map_location_longval(heading));
        break;
    case MLoc_PLAYERSHEART:
        script_support_send_tunneller_to_dungeon_heart(creatng, get_map_location_longval(heading));
        break;
    case MLoc_APPROPRTDUNGEON:
        script_support_send_tunneller_to_appropriate_dungeon(creatng);
        break;
    default:
        ERRORLOG("Invalid Heading objective %d",(int)get_map_location_type(heading));
        break;
    }
    return creatng;
}

/**
 * Spawns new creature parties. Makes given amount of the parties.
 * @param party The party to be spawned.
 * @param plyr_idx Player to own the creatures within group.
 * @param location Where the party will be spawned.
 * @param copies_num Amount of copies to be spawned.
 * @return Gives leader of last party spawned.
 */
struct Thing *script_process_new_party(struct Party *party, PlayerNumber plyr_idx, TbMapLocation location, long copies_num)
{
    struct Thing* leadtng = INVALID_THING;
    for (long i = 0; i < copies_num; i++)
    {
        struct Thing* grptng = INVALID_THING;
        for (long k = 0; k < party->members_num; k++)
        {
          if (k >= GROUP_MEMBERS_COUNT)
          {
              ERRORLOG("Party too big, %d is the limit",GROUP_MEMBERS_COUNT);
              break;
          }
          struct PartyMember* member = &(party->members[k]);
          struct Thing* thing = script_create_new_creature(plyr_idx, member->crtr_kind, location, member->carried_gold, member->crtr_level);
          if (!thing_is_invalid(thing))
          {
              struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
              cctrl->party_objective = member->objectv;
              cctrl->field_5 = game.play_gameturn + member->countdown;
              if (thing_is_invalid(grptng))
              {
                  // If it is the first creature - set it as only group member and leader
                  // Inside the thing, we don't need to mark it in any way (two creatures are needed to form a real group)
                  SYNCDBG(5,"First member %s index %d",thing_model_name(thing),(int)thing->index);
                  leadtng = thing;
                  grptng = thing;
              } else
              {
                  struct Thing* bestng = get_best_creature_to_lead_group(grptng);
                  struct CreatureControl* bestctrl = creature_control_get_from_thing(bestng);
                  // If current leader wants to defend, and current unit has an objective, new unit will be group leader.
                  if ((cctrl->party_objective != CHeroTsk_DefendParty) && (bestctrl->party_objective == CHeroTsk_DefendParty))
                  {
                      add_creature_to_group_as_leader(thing, grptng);
                      leadtng = thing;
                  } else
                  // if best and current unit want to defend party, or neither do, the strongest will be leader
                  if (((cctrl->party_objective == CHeroTsk_DefendParty) && (bestctrl->party_objective == CHeroTsk_DefendParty)) || ((cctrl->party_objective != CHeroTsk_DefendParty) && (bestctrl->party_objective != CHeroTsk_DefendParty)))
                  {
                      if ((cctrl->explevel > bestctrl->explevel) || ((cctrl->explevel == bestctrl->explevel) && (get_creature_thing_score(thing) > get_creature_thing_score(bestng))))
                      {
                          add_creature_to_group_as_leader(thing, grptng);
                          leadtng = thing;
                      }
                      else
                      // If it's weaker than the current leader, joind as a group
                      {
                          add_creature_to_group(thing, grptng);
                      }
                  }
                  else
                  // If it wants to defend, but the group leader has an objective, just add it to group
                  {
                      add_creature_to_group(thing, grptng);
                  }
              }
          }
        }
    }
    return leadtng;
}

struct Thing *script_create_new_creature(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, long carried_gold, long crtr_level)
{
    struct Thing* creatng = script_create_creature_at_location(plyr_idx, crmodel, location);
    if (thing_is_invalid(creatng))
        return INVALID_THING;
    creatng->creature.gold_carried = carried_gold;
    init_creature_level(creatng, crtr_level);
    return creatng;
}

void script_process_new_tunneller_party(PlayerNumber plyr_idx, long prty_id, TbMapLocation location, TbMapLocation heading, unsigned char crtr_level, unsigned long carried_gold)
{
    struct Thing* ldthing = script_process_new_tunneler(plyr_idx, location, heading, crtr_level, carried_gold);
    if (thing_is_invalid(ldthing))
    {
        ERRORLOG("Couldn't create tunneling group leader");
        return;
    }
    struct Thing* gpthing = script_process_new_party(&game.script.creature_partys[prty_id], plyr_idx, location, 1);
    if (thing_is_invalid(gpthing))
    {
        ERRORLOG("Couldn't create creature group");
        return;
    }
    add_creature_to_group_as_leader(ldthing, gpthing);
}

void script_process_new_creatures(PlayerNumber plyr_idx, long crmodel, long location, long copies_num, long carried_gold, long crtr_level)
{
    for (long i = 0; i < copies_num; i++)
    {
        script_create_new_creature(plyr_idx, crmodel, location, carried_gold, crtr_level);
    }
}

struct Thing *get_creature_in_range_around_any_of_enemy_heart(PlayerNumber plyr_idx, ThingModel crmodel, MapSubtlDelta range)
{
    int n = ACTION_RANDOM(PLAYERS_COUNT);
    for (int i = 0; i < PLAYERS_COUNT; i++, n = (n + 1) % PLAYERS_COUNT)
    {
        if (!players_are_enemies(plyr_idx, n))
            continue;
        struct Thing* heartng = get_player_soul_container(n);
        if (thing_exists(heartng))
        {
            struct Thing* creatng = get_creature_in_range_of_model_owned_and_controlled_by(heartng->mappos.x.val, heartng->mappos.y.val, range, crmodel, plyr_idx);
            if (!thing_is_invalid(creatng)) {
                return creatng;
            }
        }
    }
    return INVALID_THING;
}

struct Thing *script_get_creature_by_criteria(PlayerNumber plyr_idx, long crmodel, long criteria) {
    switch (criteria)
    {
    case CSelCrit_Any:
        return get_random_players_creature_of_model(plyr_idx, crmodel);
    case CSelCrit_MostExperienced:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Any, plyr_idx, 0);
    case CSelCrit_MostExpWandering:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Wandering, plyr_idx, 0);
    case CSelCrit_MostExpWorking:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Working, plyr_idx, 0);
    case CSelCrit_MostExpFighting:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Fighting, plyr_idx, 0);
    case CSelCrit_LeastExperienced:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Any, plyr_idx, 0);
    case CSelCrit_LeastExpWandering:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Wandering, plyr_idx, 0);
    case CSelCrit_LeastExpWorking:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Working, plyr_idx, 0);
    case CSelCrit_LeastExpFighting:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Fighting, plyr_idx, 0);
    case CSelCrit_NearOwnHeart:
    {
        const struct Coord3d* pos = dungeon_get_essential_pos(plyr_idx);
        return get_creature_near_and_owned_by(pos->x.val, pos->y.val, plyr_idx);
    }
    case CSelCrit_NearEnemyHeart:
        return get_creature_in_range_around_any_of_enemy_heart(plyr_idx, crmodel, 11);
    case CSelCrit_OnEnemyGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 0);
    case CSelCrit_OnFriendlyGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 1);
    default:
        ERRORLOG("Invalid level up criteria %d",(int)criteria);
        return INVALID_THING;
    }
}

/**
 * Kills a creature which meets given criteria.
 * @param plyr_idx The player whose creature will be affected.
 * @param crmodel Model of the creature to find.
 * @param criteria Criteria, from CreatureSelectCriteria enumeration.
 * @return True if a creature was found and killed.
 */
TbBool script_kill_creature_with_criteria(PlayerNumber plyr_idx, long crmodel, long criteria)
{
    struct Thing *thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing)) {
        SYNCDBG(5,"No matching player %d creature of model %d found to kill",(int)plyr_idx,(int)crmodel);
        return false;
    }
    kill_creature(thing, INVALID_THING, -1, CrDed_NoUnconscious);
    return true;
}
/**
 * Changes owner of a creature which meets given criteria.
 * @param origin_plyr_idx The player whose creature will be affected.
 * @param dest_plyr_idx The player who will receive the creature.
 * @param crmodel Model of the creature to find.
 * @param criteria Criteria, from CreatureSelectCriteria enumeration.
 * @return True if a creature was found and changed owner.
 */
TbBool script_change_creature_owner_with_criteria(PlayerNumber origin_plyr_idx, long crmodel, long criteria, PlayerNumber dest_plyr_idx)
{
    struct Thing *thing = script_get_creature_by_criteria(origin_plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing)) {
        SYNCDBG(5,"No matching player %d creature of model %d found to kill",(int)origin_plyr_idx,(int)crmodel);
        return false;
    }
    change_creature_owner(thing,dest_plyr_idx);
    return true;
}

void script_kill_creatures(PlayerNumber plyr_idx, long crmodel, long criteria, long copies_num)
{
    SYNCDBG(3,"Killing %d of %s owned by player %d.",(int)copies_num,creature_code_name(crmodel),(int)plyr_idx);
    for (long i = 0; i < copies_num; i++)
    {
        script_kill_creature_with_criteria(plyr_idx, crmodel, criteria);
    }
}

/**
 * Increase level of  a creature which meets given criteria.
 * @param plyr_idx The player whose creature will be affected.
 * @param crmodel Model of the creature to find.
 * @param criteria Criteria, from CreatureSelectCriteria enumeration.
 * @return True if a creature was found and leveled.
 */
TbBool script_level_up_creature(PlayerNumber plyr_idx, long crmodel, long criteria, int count)
{
    struct Thing *thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing)) {
        SYNCDBG(5,"No matching player %d creature of model %d found to level up",(int)plyr_idx,(int)crmodel);
        return false;
    }
    creature_increase_multiple_levels(thing,count);
    return true;
}

/**
 * Cast a spell on a creature which meets given criteria.
 * @param plyr_idx The player whose creature will be affected.
 * @param crmodel Model of the creature to find.
 * @param criteria Criteria, from CreatureSelectCriteria enumeration.
 * @param fmcl_bytes encoded bytes: f=cast for free flag,m=power kind,c=caster player index,l=spell level.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_power_on_creature(PlayerNumber plyr_idx, long crmodel, long criteria, long fmcl_bytes)
{
    struct Thing *thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing)) {
        SYNCDBG(5,"No matching player %d creature of model %d found to use power on.",(int)plyr_idx,(int)crmodel);
        return Lb_FAIL;
    }

    char is_free = (fmcl_bytes >> 24) != 0;
    PowerKind pwkind = (fmcl_bytes >> 16) & 255;
    PlayerNumber caster =  (fmcl_bytes >> 8) & 255;
    long splevel = fmcl_bytes & 255;

    if (thing_is_in_power_hand_list(thing, plyr_idx))
    {
        char block = pwkind == PwrK_SLAP;
        block |= pwkind == PwrK_CALL2ARMS;
        block |= pwkind == PwrK_CAVEIN;
        block |= pwkind == PwrK_LIGHTNING;
        block |= pwkind == PwrK_MKDIGGER;
        block |= pwkind == PwrK_SIGHT;
        if (block)
        {
          SYNCDBG(5,"Found creature to cast the spell on but it is being held.");
          return Lb_FAIL;
        }
        
    }

    MapSubtlCoord stl_x = thing->mappos.x.stl.num;
    MapSubtlCoord stl_y = thing->mappos.y.stl.num;
    unsigned long spell_flags = is_free ? PwMod_CastForFree : 0;

    switch (pwkind)
    {
      case PwrK_HEALCRTR:
        return magic_use_power_heal(caster, thing, 0, 0, splevel, spell_flags);
      case PwrK_SPEEDCRTR:
        return magic_use_power_speed(caster, thing, 0, 0, splevel, spell_flags);
      case PwrK_PROTECT:
        return magic_use_power_armour(caster, thing, 0, 0, splevel, spell_flags);
      case PwrK_CONCEAL:
        return magic_use_power_conceal(caster, thing, 0, 0, splevel, spell_flags);
      case PwrK_DISEASE:
        return magic_use_power_disease(caster, thing, 0, 0, splevel, spell_flags);
      case PwrK_CHICKEN:
        return magic_use_power_chicken(caster, thing, 0, 0, splevel, spell_flags);
      case PwrK_SLAP:
        return magic_use_power_slap_thing(caster, thing, spell_flags);
      case PwrK_CALL2ARMS:
        return magic_use_power_call_to_arms(caster, stl_x, stl_y, splevel, spell_flags);
      case PwrK_LIGHTNING:
        return magic_use_power_lightning(caster, stl_x, stl_y, splevel, spell_flags);
      case PwrK_CAVEIN:
        return magic_use_power_cave_in(caster, stl_x, stl_y, splevel, spell_flags);
      case PwrK_MKDIGGER:
        return magic_use_power_imp(caster, stl_x, stl_y, spell_flags);
      case PwrK_SIGHT:
        return magic_use_power_sight(caster, stl_x, stl_y, splevel, spell_flags);
      default:
        ERRORLOG("Power not supported at script use_power_on_creature: %d", (int) pwkind);
        return Lb_FAIL;
    }
}

/**
 * Casts spell at a location set by subtiles.
 * @param plyr_idx caster player.
 * @param stl_x subtile's x position.
 * @param stl_y subtile's y position
 * @param fml_bytes encoded bytes: f=cast for free flag,m=power kind,l=spell level.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_power_at_location(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long fml_bytes)
{
    char is_free = (fml_bytes >> 16) != 0;
    PowerKind powerKind = (fml_bytes >> 8) & 255;
    long splevel = fml_bytes & 255;
    
    unsigned long spell_flags = PwCast_AllGround | PwCast_Unrevealed;
    if (is_free)
        spell_flags |= PwMod_CastForFree;

    return magic_use_power_on_subtile(plyr_idx, powerKind, splevel, stl_x, stl_y, spell_flags);
}

/**
 * Casts a spell for player.
 * @param plyr_idx caster player.
 * @param power_kind the spell: magic id.
 * @param free cast for free flag.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_power(PlayerNumber plyr_idx, PowerKind power_kind, char free)
{
    return magic_use_power_on_level(plyr_idx, power_kind, 1, free != 0 ? PwMod_CastForFree : 0); // splevel gets ignored anyway -> pass 1
}

/**
 * Returns if the action point condition was activated.
 * Action point index and player to be activated should be stored inside condition.
 */
TbBool process_activation_status(struct Condition *condt)
{
    TbBool new_status;
    int plr_start;
    int plr_end;
    if (get_players_range(condt->plyr_range, &plr_start, &plr_end) < 0)
    {
        WARNLOG("Invalid player range %d in CONDITION command %d.",(int)condt->plyr_range,(int)condt->variabl_type);
        return false;
    }
    {
        new_status = false;
        for (long i = plr_start; i < plr_end; i++)
        {
            new_status = action_point_activated_by_player(condt->variabl_idx,i);
            if (new_status) break;
        }
    }
    return new_status;
}

/**
 * Returns if the action point of given index was triggered by given player.
 */
TbBool action_point_activated_by_player(ActionPointId apt_idx, PlayerNumber plyr_idx)
{
    unsigned long i = get_action_point_activated_by_players_mask(apt_idx);
    return ((i & (1 << plyr_idx)) != 0);
}

long get_condition_value(PlayerNumber plyr_idx, unsigned char valtype, unsigned char validx)
{
    SYNCDBG(10,"Checking condition %d for player %d",(int)valtype,(int)plyr_idx);
    struct Dungeon* dungeon;
    struct DungeonAdd* dungeonadd;
    struct Thing* thing;
    switch (valtype)
    {
    case SVar_MONEY:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->total_money_owned;
    case SVar_GAME_TURN:
        return game.play_gameturn;
    case SVar_BREAK_IN:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->times_breached_dungeon;
    case SVar_CREATURE_NUM:
        return count_player_creatures_of_model(plyr_idx, validx);
    case SVar_TOTAL_DIGGERS:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->num_active_diggers;
    case SVar_TOTAL_CREATURES:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->num_active_creatrs;
    case SVar_TOTAL_RESEARCH:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->total_research_points / 256;
    case SVar_TOTAL_DOORS:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->total_doors;
    case SVar_TOTAL_AREA:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->total_area;
    case SVar_TOTAL_CREATURES_LEFT:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->total_creatures_left;
    case SVar_CREATURES_ANNOYED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->creatures_annoyed;
    case SVar_BATTLES_LOST:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->battles_lost;
    case SVar_BATTLES_WON:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->battles_won;
    case SVar_ROOMS_DESTROYED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->rooms_destroyed;
    case SVar_SPELLS_STOLEN:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->spells_stolen;
    case SVar_TIMES_BROKEN_INTO:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->times_broken_into;
    case SVar_GHOSTS_RAISED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.ghosts_raised;
    case SVar_SKELETONS_RAISED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.skeletons_raised;
    case SVar_VAMPIRES_RAISED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.vamps_created;
    case SVar_CREATURES_CONVERTED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.creatures_converted;
    case SVar_TIMES_ANNOYED_CREATURE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.lies_told;
    case SVar_TOTAL_DOORS_MANUFACTURED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.manufactured_doors;
    case SVar_TOTAL_TRAPS_MANUFACTURED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.manufactured_traps;
    case SVar_TOTAL_MANUFACTURED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.manufactured_items;
    case SVar_TOTAL_TRAPS_USED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.traps_used;
    case SVar_TOTAL_DOORS_USED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.doors_used;
    case SVar_KEEPERS_DESTROYED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.keepers_destroyed;
    case SVar_TIMES_LEVELUP_CREATURE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.creatures_trained;
    case SVar_TIMES_TORTURED_CREATURE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.creatures_tortured;
    case SVar_CREATURES_SACRIFICED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.creatures_sacrificed;
    case SVar_CREATURES_FROM_SACRIFICE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.creatures_from_sacrifice;
    case SVar_TOTAL_SALARY:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.salary_cost;
    case SVar_CURRENT_SALARY:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->creatures_total_pay;
    case SVar_GOLD_POTS_STOLEN:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->gold_pots_stolen;
    case SVar_HEART_HEALTH:
        thing = get_player_soul_container(plyr_idx);
        if (thing_is_dungeon_heart(thing))
        {
            return thing->health;
        }
        return 0;
    case SVar_TIMER:
        dungeon = get_dungeon(plyr_idx);
        if (dungeon->turn_timers[validx].state)
          return game.play_gameturn - dungeon->turn_timers[validx].count;
        else
          return 0;
    case SVar_DUNGEON_DESTROYED:
        return !player_has_heart(plyr_idx);
    case SVar_TOTAL_GOLD_MINED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.gold_mined;
    case SVar_FLAG:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->script_flags[validx];
    case SVar_ROOM_SLABS:
        return get_room_slabs_count(plyr_idx, validx);
    case SVar_DOORS_DESTROYED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->doors_destroyed;
    case SVar_CREATURES_SCAVENGED_LOST:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->creatures_scavenge_lost;
    case SVar_CREATURES_SCAVENGED_GAINED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->creatures_scavenge_gain;
    case SVar_AVAILABLE_MAGIC: // IF_AVAILABLE(MAGIC)
        return is_power_available(plyr_idx, validx);
    case SVar_AVAILABLE_TRAP: // IF_AVAILABLE(TRAP)
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->mnfct_info.trap_amount_stored[validx%trapdoor_conf.trap_types_count]
              + dungeonadd->mnfct_info.trap_amount_offmap[validx%trapdoor_conf.trap_types_count];
    case SVar_AVAILABLE_DOOR: // IF_AVAILABLE(DOOR)
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->mnfct_info.door_amount_stored[validx%trapdoor_conf.door_types_count]
              + dungeonadd->mnfct_info.door_amount_offmap[validx%trapdoor_conf.door_types_count];
    case SVar_AVAILABLE_ROOM: // IF_AVAILABLE(ROOM)
        dungeon = get_dungeon(plyr_idx);
        return dungeon->room_buildable[validx%ROOM_TYPES_COUNT];
    case SVar_AVAILABLE_CREATURE: // IF_AVAILABLE(CREATURE)
        dungeon = get_dungeon(plyr_idx);
        if (creature_will_generate_for_dungeon(dungeon, validx)) {
            return min(game.pool.crtr_kind[validx%CREATURE_TYPES_COUNT],dungeon->max_creatures_attracted - (long)dungeon->num_active_creatrs);
        }
        return 0;
    case SVar_SLAB_OWNER: //IF_SLAB_OWNER
    {
        long varib_id = get_slab_number(plyr_idx, validx);
        struct SlabMap* slb = get_slabmap_direct(varib_id);
        return slabmap_owner(slb);
    }
    case SVar_SLAB_TYPE: //IF_SLAB_TYPE
    {
        long varib_id = get_slab_number(plyr_idx, validx);
        struct SlabMap* slb = get_slabmap_direct(varib_id);
        return slb->kind;
    }
    case SVar_CONTROLS_CREATURE: // IF_CONTROLS(CREATURE)
        dungeon = get_dungeon(plyr_idx);
        return dungeon->owned_creatures_of_model[validx%CREATURE_TYPES_COUNT]
          - count_player_list_creatures_of_model_matching_bool_filter(plyr_idx, validx, creature_is_kept_in_custody_by_enemy_or_dying);
    case SVar_CONTROLS_TOTAL_CREATURES:// IF_CONTROLS(TOTAL_CREATURES)
        dungeon = get_dungeon(plyr_idx);
        return dungeon->num_active_creatrs - count_player_creatures_not_counting_to_total(plyr_idx);
    case SVar_CONTROLS_TOTAL_DIGGERS:// IF_CONTROLS(TOTAL_DIGGERS)
        dungeon = get_dungeon(plyr_idx);
        return dungeon->num_active_diggers - count_player_diggers_not_counting_to_total(plyr_idx);
    case SVar_ALL_DUNGEONS_DESTROYED:
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        return all_dungeons_destroyed(player);
    }
    case SVar_DOOR_NUM:
        return count_player_deployed_doors_of_model(plyr_idx, validx);
    case SVar_TRAP_NUM:
        return count_player_deployed_traps_of_model(plyr_idx, validx);
    case SVar_GOOD_CREATURES:
        dungeon = get_dungeon(plyr_idx);
        return count_creatures_in_dungeon_of_model_flags(dungeon, 0, CMF_IsEvil|CMF_IsSpectator|CMF_IsSpecDigger);
    case SVar_EVIL_CREATURES:
        dungeon = get_dungeon(plyr_idx);
        return count_creatures_in_dungeon_of_model_flags(dungeon, CMF_IsEvil, CMF_IsSpectator|CMF_IsSpecDigger);
    case SVar_CONTROLS_GOOD_CREATURES:
        dungeon = get_dungeon(plyr_idx);
        return count_creatures_in_dungeon_controlled_and_of_model_flags(dungeon, 0, CMF_IsEvil|CMF_IsSpectator|CMF_IsSpecDigger);
    case SVar_CONTROLS_EVIL_CREATURES:
        dungeon = get_dungeon(plyr_idx);
        return count_creatures_in_dungeon_controlled_and_of_model_flags(dungeon, CMF_IsEvil, CMF_IsSpectator|CMF_IsSpecDigger);
    case SVar_CAMPAIGN_FLAG:
        return intralvl.campaign_flags[plyr_idx][validx];
    case SVar_BOX_ACTIVATED:
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->box_info.activated[validx];
    default:
        break;
    };
    return 0;
}

TbBool get_condition_status(unsigned char opkind, long val1, long val2)
{
  return LbMathOperation(opkind, val1, val2) != 0;
}

TbBool is_condition_met(long cond_idx)
{
    if ((cond_idx < 0) || (cond_idx >= CONDITIONS_COUNT))
    {
      if (cond_idx == -1)
          return true;
      else
          return false;
    }
    unsigned long i = game.script.conditions[cond_idx].status;
    return ((i & 0x01) != 0);
}

TbBool condition_inactive(long cond_idx)
{
  if ((cond_idx < 0) || (cond_idx >= CONDITIONS_COUNT))
  {
    if (cond_idx == -1)
      return false;
    else
      return false;
  }
  unsigned long i = game.script.conditions[cond_idx].status;
  if (((i & 0x01) == 0) || ((i & 0x04) != 0))
    return true;
  return false;
}

void process_condition(struct Condition *condt)
{
    TbBool new_status;
    int plr_start;
    int plr_end;
    long i;
    SYNCDBG(18,"Starting for type %d, player %d",(int)condt->variabl_type,(int)condt->plyr_range);
    if (condition_inactive(condt->condit_idx))
    {
        set_flag_byte(&condt->status, 0x01, false);
        return;
    }
    if ((condt->variabl_type == SVar_SLAB_OWNER) || (condt->variabl_type == SVar_SLAB_TYPE)) //These variable types abuse the plyr_range, since all slabs don't fit in an unsigned short
    {
        new_status = false;
        long k = get_condition_value(condt->plyr_range, condt->variabl_type, condt->variabl_idx);
        new_status = get_condition_status(condt->operation, k, condt->rvalue);
    }
    else
    {
        if (get_players_range(condt->plyr_range, &plr_start, &plr_end) < 0)
        {
            WARNLOG("Invalid player range %d in CONDITION command %d.", (int)condt->plyr_range, (int)condt->variabl_type);
            return;
        }
        if (condt->variabl_type == SVar_ACTION_POINT_TRIGGERED)
        {
            new_status = false;
            for (i = plr_start; i < plr_end; i++)
            {
                new_status = action_point_activated_by_player(condt->variabl_idx, i);
                if (new_status) break;
            }
        }
        else
        {
            new_status = false;
            for (i = plr_start; i < plr_end; i++)
            {
                long k = get_condition_value(i, condt->variabl_type, condt->variabl_idx);
                new_status = get_condition_status(condt->operation, k, condt->rvalue);
                if (new_status != false)
                {
                  break;
                }
            }
        }
    }
    SYNCDBG(19,"Condition type %d status %d",(int)condt->variabl_type,(int)new_status);
    set_flag_byte(&condt->status, 0x01,  new_status);
    if (((condt->status & 0x01) == 0) || ((condt->status & 0x02) != 0))
    {
        set_flag_byte(&condt->status, 0x04,  false);
    } else
    {
        set_flag_byte(&condt->status, 0x02,  true);
        set_flag_byte(&condt->status, 0x04,  true);
    }
    SCRIPTDBG(19,"Finished");
}

void process_conditions(void)
{
    if (game.script.conditions_num > CONDITIONS_COUNT)
      game.script.conditions_num = CONDITIONS_COUNT;
    for (long i = 0; i < game.script.conditions_num; i++)
    {
      process_condition(&game.script.conditions[i]);
    }
}

void process_check_new_creature_partys(void)
{
    for (long i = 0; i < game.script.party_triggers_num; i++)
    {
        struct PartyTrigger* pr_trig = &game.script.party_triggers[i];
        if ((pr_trig->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(pr_trig->condit_idx))
            {
                long n = pr_trig->creatr_id;
                if (n <= 0)
                {
                    SYNCDBG(6, "Adding player %d party %d in location %d", (int)pr_trig->plyr_idx, (int)-n, (int)pr_trig->location);
                    script_process_new_party(&game.script.creature_partys[-n],
                        pr_trig->plyr_idx, pr_trig->location, pr_trig->ncopies);
                }
                else
                {
                    SCRIPTDBG(6, "Adding creature %d", n);
                    script_process_new_creatures(pr_trig->plyr_idx, n, pr_trig->location,
                        pr_trig->ncopies, pr_trig->carried_gold, pr_trig->crtr_level);
                }
                if ((pr_trig->flags & TrgF_REUSABLE) == 0)
                    set_flag_byte(&pr_trig->flags, TrgF_DISABLED, true);
            }
      }
    }
}

void process_check_new_tunneller_partys(void)
{
    for (long i = 0; i < game.script.tunneller_triggers_num; i++)
    {
        struct TunnellerTrigger* tn_trig = &game.script.tunneller_triggers[i];
        if ((tn_trig->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(tn_trig->condit_idx))
            {
                long k = tn_trig->party_id;
                if (k > 0)
                {
                    long n = tn_trig->plyr_idx;
                    SCRIPTDBG(6, "Adding tunneler party %d", k);
                    struct Thing* thing = script_process_new_tunneler(n, tn_trig->location, tn_trig->heading,
                        tn_trig->crtr_level, tn_trig->carried_gold);
                    if (!thing_is_invalid(thing))
                    {
                        struct Thing* grptng = script_process_new_party(&game.script.creature_partys[k - 1], n, tn_trig->location, 1);
                        if (!thing_is_invalid(grptng))
                        {
                            add_creature_to_group_as_leader(thing, grptng);
                        }
                        else
                        {
                            WARNLOG("No party created, only lone %s", thing_model_name(thing));
                        }
                    }
                }
                else
                {
                    SCRIPTDBG(6, "Adding tunneler, heading %d", tn_trig->heading);
                    script_process_new_tunneler(tn_trig->plyr_idx, tn_trig->location, tn_trig->heading,
                        tn_trig->crtr_level, tn_trig->carried_gold);
                }
                if ((tn_trig->flags & TrgF_REUSABLE) == 0)
                    tn_trig->flags |= TrgF_DISABLED;
            }
      }
    }
}

void process_win_and_lose_conditions(PlayerNumber plyr_idx)
{
    long i;
    long k;
    struct PlayerInfo* player = get_player(plyr_idx);
    if ((game.system_flags & GSF_NetworkActive) != 0)
      return;
    for (i=0; i < game.script.win_conditions_num; i++)
    {
      k = game.script.win_conditions[i];
      if (is_condition_met(k)) {
          SYNCDBG(8,"Win condition %d (cond. %d) met for player %d.",(int)i,(int)k,(int)plyr_idx);
          set_player_as_won_level(player);
      }
    }
    for (i=0; i < game.script.lose_conditions_num; i++)
    {
      k = game.script.lose_conditions[i];
      if (is_condition_met(k)) {
          SYNCDBG(8,"Lose condition %d (cond. %d) met for player %d.",(int)i,(int)k,(int)plyr_idx);
          set_player_as_lost_level(player);
      }
    }
}

void process_values(void)
{
    for (long i = 0; i < game.script.values_num; i++)
    {
        struct ScriptValue* value = &game.script.values[i];
        if ((value->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(value->condit_idx))
            {
                script_process_value(value->valtype, value->plyr_range, value->field_4, value->field_8, value->field_C);
                if ((value->flags & TrgF_REUSABLE) == 0)
                  set_flag_byte(&value->flags, TrgF_DISABLED, true);
            }
        }
    }
}

/**
 * Processes given VALUE immediately.
 * This processes given script command. It is used to process VALUEs at start when they have
 * no conditions, or during the gameplay when conditions are met.
 */
void script_process_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4)
{
  struct CreatureStats *crstat;
  struct CreatureModelConfig *crconf;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct SlabMap *slb;
  int plr_start;
  int plr_end;
  long i;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0)
  {
      WARNLOG("Invalid player range %d in VALUE command %d.",(int)plr_range_id,(int)var_index);
      return;
  }
  switch (var_index)
  {
  case Cmd_SET_HATE:
      for (i=plr_start; i < plr_end; i++)
      {
        dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->hates_player[val2%DUNGEONS_COUNT] = val3;
      }
      break;
  case Cmd_SET_GENERATE_SPEED:
      game.generate_speed = saturate_set_unsigned(val2, 16);
      update_dungeon_generation_speeds();
      break;
  case Cmd_ROOM_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
        set_room_available(i, val2, val3, val4);
      }
      break;
  case Cmd_CREATURE_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
          if (!set_creature_available(i,val2,val3,val4)) {
              WARNLOG("Setting creature %s availability for player %d failed.",creature_code_name(val2),(int)i);
          }
      }
      break;
  case Cmd_MAGIC_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
          if (!set_power_available(i,val2,val3,val4)) {
              WARNLOG("Setting power %s availability for player %d failed.",power_code_name(val2),(int)i);
          }
      }
      break;
  case Cmd_TRAP_AVAILABLE:
      for (i=plr_start; i < plr_end; i++)
      {
          if (!set_trap_buildable_and_add_to_amount(i, val2, val3, val4)) {
              WARNLOG("Setting trap %s availability for player %d failed.",trap_code_name(val2),(int)i);
          }
      }
      break;
  case Cmd_RESEARCH:
      for (i=plr_start; i < plr_end; i++)
      {
          if (!update_or_add_players_research_amount(i, val2, val3, val4)) {
              WARNLOG("Updating research points for type %d kind %d of player %d failed.",(int)val2,(int)val3,(int)i);
          }
      }
      break;
  case Cmd_RESEARCH_ORDER:
      for (i=plr_start; i < plr_end; i++)
      {
        if (!research_overriden_for_player(i))
          remove_all_research_from_player(i);
        add_research_to_player(i, val2, val3, val4);
      }
      break;
  case Cmd_SET_TIMER:
      for (i=plr_start; i < plr_end; i++)
      {
          restart_script_timer(i,val2);
      }
      break;
  case Cmd_SET_FLAG:
      for (i=plr_start; i < plr_end; i++)
      {
          set_script_flag(i,val2,saturate_set_unsigned(val3, 8));
      }
      break;
  case Cmd_MAX_CREATURES:
      for (i=plr_start; i < plr_end; i++)
      {
          SYNCDBG(4,"Setting player %d max attracted creatures to %d.",(int)i,(int)val2);
          dungeon = get_dungeon(i);
          if (dungeon_invalid(dungeon))
              continue;
          dungeon->max_creatures_attracted = val2;
      }
      break;
  case Cmd_DOOR_AVAILABLE:
      for (i=plr_start; i < plr_end; i++) {
          set_door_buildable_and_add_to_amount(i, val2, val3, val4);
      }
      break;
  case Cmd_DISPLAY_OBJECTIVE:
      if ( (my_player_number >= plr_start) && (my_player_number < plr_end) ) {
          set_general_objective(val2, val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      }
      break;
  case Cmd_DISPLAY_INFORMATION:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end)) {
          set_general_information(val2, val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      }
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
          dungeon = get_dungeon(i);
          if (dungeon_invalid(dungeon))
              continue;
          dungeon->creature_max_level[val2%CREATURE_TYPES_COUNT] = val3;
      }
      break;
  case Cmd_SET_CREATURE_HEALTH:
      change_max_health_of_creature_kind(val2, val3);
      break;
  case Cmd_SET_CREATURE_STRENGTH:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->strength = saturate_set_unsigned(val3, 8);
      creature_stats_updated(val2);
      break;
  case Cmd_SET_CREATURE_ARMOUR:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->armour = saturate_set_unsigned(val3, 8);
      creature_stats_updated(val2);
      break;
  case Cmd_SET_CREATURE_FEAR_WOUNDED:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->fear_wounded = saturate_set_unsigned(val3, 8);
      creature_stats_updated(val2);
      break;
  case Cmd_SET_CREATURE_FEAR_STRONGER:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->fear_stronger = saturate_set_unsigned(val3, 16);
      creature_stats_updated(val2);
      break;
  case Cmd_SET_CREATURE_FEARSOME_FACTOR:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->fearsome_factor = saturate_set_unsigned(val3, 16);
      creature_stats_updated(val2);
      break;
  case Cmd_SET_CREATURE_PROPERTY:
      crconf = &crtr_conf.model[val2];
      crstat = creature_stats_get(val2);
      switch (val3)
      {
      case 1: // BLEEDS
          crstat->bleeds = val4;
          break;
      case 2: // UNAFFECTED_BY_WIND
          crstat->affected_by_wind = val4;
          break;
      case 3: // IMMUNE_TO_GAS
          crstat->immune_to_gas = val4;
          break;
      case 4: // HUMANOID_SKELETON
          crstat->humanoid_creature = val4;
          break;
      case 5: // PISS_ON_DEAD
          crstat->piss_on_dead = val4;
          break;
      case 7: // FLYING
          crstat->flying = val4;
          break;
      case 8: // SEE_INVISIBLE
          crstat->can_see_invisible = val4;
          break;
      case 9: // PASS_LOCKED_DOORS
          crstat->can_go_locked_doors = val4;
          break;
      case 10: // SPECIAL_DIGGER
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_IsSpecDigger;
          }
          else
          {
              crconf->model_flags ^= CMF_IsSpecDigger;
          }
          break;
      case 11: // ARACHNID
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_IsArachnid;
          }
          else
          {
              crconf->model_flags ^= CMF_IsArachnid;
          }
          break;
      case 12: // DIPTERA
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_IsDiptera;
          }
          else
          {
              crconf->model_flags ^= CMF_IsDiptera;
          }
          break;
      case 13: // LORD
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_IsLordOTLand;
          }
          else
          {
              crconf->model_flags ^= CMF_IsLordOTLand;
          }
          break;
      case 14: // SPECTATOR
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_IsSpectator;
          }
          else
          {
              crconf->model_flags ^= CMF_IsSpectator;
          }
          break;
      case 15: // EVIL
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_IsEvil;
          }
          else
          {
              crconf->model_flags ^= CMF_IsEvil;
          }
          break; 
      case 16: // NEVER_CHICKENS
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_NeverChickens;
          }
          else
          {
              crconf->model_flags ^= CMF_NeverChickens;
          }
          break; 
      case 17: // IMMUNE_TO_BOULDER
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_ImmuneToBoulder;
          }
          else
          {
              crconf->model_flags ^= CMF_ImmuneToBoulder;
          }
          break; 
      case 18: // NO_CORPSE_ROTTING
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_NoCorpseRotting;
          }
          else
          {
              crconf->model_flags ^= CMF_NoCorpseRotting;
          }
          break; 
      case 19: // NO_ENMHEART_ATTCK
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_NoEnmHeartAttack;
          }
          else
          {
              crconf->model_flags ^= CMF_NoEnmHeartAttack;
          }
          break; 
      case 20: // TREMBLING_FAT
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_TremblingFat;
          }
          else
          {
              crconf->model_flags ^= CMF_TremblingFat;
          }
          break; 
      case 21: // FEMALE
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_Female;
          }
          else
          {
              crconf->model_flags ^= CMF_Female;
          }
          break; 
      case 22: // INSECT
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_Insect;
          }
          else
          {
              crconf->model_flags ^= CMF_Insect;
          }
          break; 
      case 23: // ONE_OF_KIND
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_OneOfKind;
          }
          else
          {
              crconf->model_flags ^= CMF_OneOfKind;
          }
          break; 
      case 24: // NO_INPRISONMENT
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_NoImprisonment;
          }
          else
          {
              crconf->model_flags ^= CMF_NoImprisonment;
          }
          break; 
      default:
          SCRPTERRLOG("Unknown creature property '%d'", val3);
          break;
      }
      creature_stats_updated(val2);
      break;
  case Cmd_ALLY_PLAYERS:
      for (i=plr_start; i < plr_end; i++)
      {
          set_ally_with_player(i, val2, val3);
          set_ally_with_player(val2, i, val3);
      }
      break;
      break;
  case Cmd_DEAD_CREATURES_RETURN_TO_POOL:
      set_flag_byte(&game.flags_cd, MFlg_DeadBackToPool, val2);
      break;
  case Cmd_BONUS_LEVEL_TIME:
      if (val2 > 0) {
          game.bonus_time = game.play_gameturn + val2;
          game.flags_gui |= GGUI_CountdownTimer;
      } else {
          game.bonus_time = 0;
          game.flags_gui &= ~GGUI_CountdownTimer;
      }
      break;
  case Cmd_QUICK_OBJECTIVE:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end))
          process_objective(gameadd.quick_messages[val2%QUICK_MESSAGES_COUNT], val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      break;
  case Cmd_QUICK_INFORMATION:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end))
          set_quick_information(val2, val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      break;
  case Cmd_PLAY_MESSAGE:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end))
      {
          switch (val2)
          {
          case 1:
              output_message(val3, 0, true);
              break;
          case 2:
              play_non_3d_sample(val3);
              break;
          }
      }
      break;
  case Cmd_ADD_GOLD_TO_PLAYER:
      for (i=plr_start; i < plr_end; i++)
      {
          player_add_offmap_gold(i, val2);
      }
      break;
  case Cmd_SET_CREATURE_TENDENCIES:
      for (i=plr_start; i < plr_end; i++)
      {
          player = get_player(i);
          set_creature_tendencies(player, val2, val3);
          if (is_my_player(player)) {
              dungeon = get_players_dungeon(player);
              game.creatures_tend_imprison = ((dungeon->creature_tendencies & 0x01) != 0);
              game.creatures_tend_flee = ((dungeon->creature_tendencies & 0x02) != 0);
          }
      }
      break;
  case Cmd_REVEAL_MAP_RECT:
      for (i=plr_start; i < plr_end; i++)
      {
          player_reveal_map_area(i, val2, val3, (val4)&0xffff, (val4>>16)&0xffff);
      }
      break;
  case Cmd_REVEAL_MAP_LOCATION:
      for (i=plr_start; i < plr_end; i++)
      {
          player_reveal_map_location(i, val2, val3);
      }
      break;
  case Cmd_CHANGE_SLAB_OWNER:
      if (val2 < 0 || val2 > 85)
      {
          SCRPTERRLOG("Value '%d' out of range. Range 0-85 allowed.", val2);
      } else
      if (val3 < 0 || val3 > 85)
      {
          SCRPTERRLOG("Value '%d' out of range. Range 0-85 allowed.", val3);
      } else
      {
          slb = get_slabmap_block(val2, val3);
          if (slb->room_index)
          {
              struct Room* room = room_get(slb->room_index);
              take_over_room(room, plr_range_id);
          } else
          if (slb->kind >= SlbT_WALLDRAPE && slb->kind <= SlbT_CLAIMED) //All slabs that can be owned but aren't rooms
          {
              short slbkind;
              if (slb->kind == SlbT_PATH)
              {
                  slbkind = SlbT_CLAIMED;
              }
              else
              {
                  slbkind = slb->kind;
              }
              place_slab_type_on_map(slbkind, slab_subtile(val2, 0), slab_subtile(val3, 0), plr_range_id, 0);
          }
      }
      break;
  case Cmd_CHANGE_SLAB_TYPE:
      if (val2 < 0 || val2 > 85)
      {
          SCRPTERRLOG("Value '%d' out of range. Range 0-85 allowed.", val2); 
      } else
      if (val3 < 0 || val3 > 85)
      {
          SCRPTERRLOG("Value '%d' out of range. Range 0-85 allowed.", val3);
      } else
      if (val4 < 0 || val4 > 53)
      {
          SCRPTERRLOG("Unsupported slab '%d'. Slabs range 0-53 allowed.", val4);
      } else
      {
          replace_slab_from_script(val2, val3, val4);
      }
      break;
  case Cmd_KILL_CREATURE:
      for (i=plr_start; i < plr_end; i++)
      {
          script_kill_creatures(i, val2, val3, val4);
      }
      break;
    case Cmd_LEVEL_UP_CREATURE:
      for (i=plr_start; i < plr_end; i++)
      {
          script_level_up_creature(i, val2, val3, val4);
      }
      break;
    case Cmd_USE_POWER_ON_CREATURE:
      for (i=plr_start; i < plr_end; i++)
      {
          script_use_power_on_creature(i, val2, val3, val4);
      }
      break;
    case Cmd_USE_POWER_AT_LOCATION:
      for (i=plr_start; i < plr_end; i++)
      {
          script_use_power_at_location(i, val2, val3, val4);
      }
      break;
    case Cmd_USE_POWER:
      for (i=plr_start; i < plr_end; i++)
      {
          script_use_power(i, val2, val3);
      }
      break;
    case Cmd_CHANGE_CREATURE_OWNER:
      for (i=plr_start; i < plr_end; i++)
      {
          script_change_creature_owner_with_criteria(i, val2, val3, val4);
      }
      break;
  case Cmd_ADD_TO_FLAG:
      for (i=plr_start; i < plr_end; i++)
      {
          dungeon = get_dungeon(i);
          set_script_flag(i, val2, dungeon->script_flags[val2] + val3);
      }
      break;
  case Cmd_SET_CAMPAIGN_FLAG:
      for (i=plr_start; i < plr_end; i++)
      {
          intralvl.campaign_flags[i][val2] = saturate_set_signed(val3, 32);
      }
      break;
  case Cmd_ADD_TO_CAMPAIGN_FLAG:

      for (i=plr_start; i < plr_end; i++)
      {
          intralvl.campaign_flags[i][val2] = saturate_set_signed(intralvl.campaign_flags[i][val2] + val3, 32);
      }
      break;
  case Cmd_EXPORT_VARIABLE:
      for (i=plr_start; i < plr_end; i++)
      {
          SYNCDBG(8, "Setting campaign flag[%ld][%ld] to %ld.", i, val4, get_condition_value(i, val2, val3));
          intralvl.campaign_flags[i][val4] = get_condition_value(i, val2, val3);
      }
      break;
  case Cmd_SET_GAME_RULE:
      switch (val2)
      {
      case 1: //BodiesForVampire
          if (val3 >= 0)
          {
              SCRIPTDBG(7,"Changing rule %d from %d to %d", val2, game.bodies_for_vampire, val3);
              game.bodies_for_vampire = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 2: //PrisonSkeletonChance
          if (val3 >= 0 && val3 <= 100)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.prison_skeleton_chance, val3);
              game.prison_skeleton_chance = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 3: //GhostConvertChance
          if (val3 >= 0 && val3 <= 100)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.ghost_convert_chance, val3);
              game.ghost_convert_chance = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 4: //TortureConvertChance
          if (val3 >= 0 && val3 <= 100)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.torture_convert_chance, val3);
              gameadd.torture_convert_chance = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 5: //TortureDeathChance
          if (val3 >= 0 && val3 <= 100)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.torture_death_chance, val3);
              gameadd.torture_death_chance = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 6: //FoodGenerationSpeed
          if (val3 >= 0)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.food_generation_speed, val3);
              game.food_generation_speed = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 7: //StunEvilEnemyChance
          if (val3 >= 0 && val3 <= 100)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.stun_enemy_chance_evil, val3);
              gameadd.stun_enemy_chance_evil = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 8: //StunGoodEnemyChance
          if (val3 >= 0 && val3 <= 100)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.stun_enemy_chance_good, val3);
              gameadd.stun_enemy_chance_good = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 9: //BodyRemainsFor
          if (val3 >= 0)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.body_remains_for, val3);
              game.body_remains_for = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 10: //FightHateKillValue
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.fight_hate_kill_value, val3);
          game.fight_hate_kill_value = val3;
          break;
      case 11: //PreserveClassicBugs
          if (val3 >= 0 && val3 <= 100)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.classic_bugs_flags, val3);
              gameadd.classic_bugs_flags = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 12: //DungeonHeartHealHealth
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.dungeon_heart_heal_health, val3);
          game.dungeon_heart_heal_health = val3;
          break;
      case 13: //ImpWorkExperience
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.digger_work_experience, val3);
          gameadd.digger_work_experience = val3;
          break;
      case 14: //GemEffectiveness
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.gem_effectiveness, val3);
          gameadd.gem_effectiveness = val3;
          break;
      case 15: //RoomSellGoldBackPercent
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.room_sale_percent, val3);
          gameadd.room_sale_percent = val3;
          break;
      case 16: //PayDayGap
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.pay_day_gap, val3);
          game.pay_day_gap = val3;
          break;
      case 17: //PayDaySpeed
          if (val3 >= 0)
          {
              SCRIPTDBG(7, "Changing rule %s from %d to %d", val2, gameadd.pay_day_speed, val3);
              gameadd.pay_day_speed = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 18: //PayDayProgress
          if (val3 >= 0)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.pay_day_progress, val3);
              game.pay_day_progress = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
    case 19: //PlaceTrapsOnSubtiles
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.place_traps_on_subtiles, val3);
          gameadd.place_traps_on_subtiles = (TbBool)val3;
          break;
      default:
          WARNMSG("Unsupported Game RULE, command %d.", val2);
          break;
      }
      break;
  default:
      WARNMSG("Unsupported Game VALUE, command %d.",var_index);
      break;
  }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
