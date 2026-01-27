/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_commands.c
 *     Commands that can be used by level script
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include <math.h>
#include <string.h>
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "config_effects.h"
#include "config_lenses.h"
#include "config_magic.h"
#include "config_players.h"
#include "config_powerhands.h"
#include "config_settings.h"
#include "config_spritecolors.h"
#include "config_trapdoor.h"
#include "console_cmd.h"
#include "config_rules.h"
#include "creature_instances.h"
#include "creature_states.h"
#include "creature_states_mood.h"
#include "creature_states_pray.h"
#include "custom_sprites.h"
#include "dungeon_data.h"
#include "frontmenu_ingame_map.h"
#include "frontend.h"
#include "gui_soundmsgs.h"
#include "gui_frontmenu.h"
#include "keeperfx.hpp"
#include "lens_api.h"
#include "lvl_script_commands.h"
#include "lvl_script_conditions.h"
#include "lvl_script_lib.h"
#include "lua_base.h"
#include "map_blocks.h"
#include "player_instances.h"
#include "player_utils.h"
#include "power_hand.h"
#include "power_specials.h"
#include "room_util.h"
#include "sounds.h"
#include "spdigger_stack.h"
#include "thing_data.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_physics.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern long level_file_version;

#define MAX_CONFIG_VALUES 4

const struct CommandDesc subfunction_desc[] = {
    {"RANDOM",                     "Aaaaaaaa", Cmd_RANDOM, NULL, NULL},
    {"DRAWFROM",                   "Aaaaaaaa", Cmd_DRAWFROM, NULL, NULL},
    {"IMPORT",                     "PA      ", Cmd_IMPORT, NULL, NULL},
    {NULL,                         "        ", Cmd_NONE, NULL, NULL},
  };

const struct NamedCommand player_desc[] = {
  {"PLAYER0",          PLAYER0},
  {"PLAYER1",          PLAYER1},
  {"PLAYER2",          PLAYER2},
  {"PLAYER3",          PLAYER3},
  {"PLAYER_GOOD",      PLAYER_GOOD},
  {"ALL_PLAYERS",      ALL_PLAYERS},
  {"PLAYER_NEUTRAL",   PLAYER_NEUTRAL},
  {"PLAYER4",          PLAYER4},
  {"PLAYER5",          PLAYER5},
  {"PLAYER6",          PLAYER6},
  {NULL,               0},
};

const struct NamedCommand controls_variable_desc[] = {
    {"TOTAL_DIGGERS",               SVar_CONTROLS_TOTAL_DIGGERS},
    {"TOTAL_CREATURES",             SVar_CONTROLS_TOTAL_CREATURES},
    {"TOTAL_DOORS",                 SVar_TOTAL_DOORS},
    {"TOTAL_TRAPS",                 SVar_TOTAL_TRAPS},
    {"TOTAL_AREA",                  SVar_TOTAL_AREA},
    {"GOOD_CREATURES",              SVar_CONTROLS_GOOD_CREATURES},
    {"EVIL_CREATURES",              SVar_CONTROLS_EVIL_CREATURES},
    {NULL,                           0},
};

const struct NamedCommand available_variable_desc[] = {
    {"TOTAL_CREATURES",             SVar_AVAILABLE_TOTAL_CREATURES},
    {"TOTAL_DOORS",                 SVar_AVAILABLE_TOTAL_DOORS},
    {"TOTAL_TRAPS",                 SVar_AVAILABLE_TOTAL_TRAPS},
    {"TOTAL_AREA",                  SVar_TOTAL_AREA},
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

const struct NamedCommand hand_rule_desc[] = {
  {"ALWAYS",                HandRule_Always},
  {"AGE_LOWER",             HandRule_AgeLower},
  {"AGE_HIGHER",            HandRule_AgeHigher},
  {"LEVEL_LOWER",           HandRule_LvlLower},
  {"LEVEL_HIGHER",          HandRule_LvlHigher},
  {"AT_ACTION_POINT",       HandRule_AtActionPoint},
  {"AFFECTED_BY",           HandRule_AffectedBy},
  {"WANDERING",             HandRule_Wandering},
  {"WORKING",               HandRule_Working},
  {"FIGHTING",              HandRule_Fighting},
  {"DROPPED_TIME_HIGHER",   HandRule_DroppedTimeHigher},
  {"DROPPED_TIME_LOWER",    HandRule_DroppedTimeLower},
  {"BLOCKED_FOR_PICKUP",    HandRule_BlockedPickup},
  {NULL,                    0},
};

const struct NamedCommand rule_slot_desc[] = {
  {"RULE0",  0},
  {"RULE1",  1},
  {"RULE2",  2},
  {"RULE3",  3},
  {"RULE4",  4},
  {"RULE5",  5},
  {"RULE6",  6},
  {"RULE7",  7},
  {NULL,     0},
};

const struct NamedCommand rule_action_desc[] = {
  {"DENY",      HandRuleAction_Deny},
  {"ALLOW",     HandRuleAction_Allow},
  {"ENABLE",    HandRuleAction_Enable},
  {"DISABLE",   HandRuleAction_Disable},
  {NULL,     0},
};

const struct NamedCommand hero_objective_desc[] = {
  {"STEAL_GOLD",           CHeroTsk_StealGold},
  {"STEAL_SPELLS",         CHeroTsk_StealSpells},
  {"ATTACK_ENEMIES",       CHeroTsk_AttackEnemies},
  {"ATTACK_DUNGEON_HEART", CHeroTsk_AttackDnHeart},
  {"SNIPE_DUNGEON_HEART",  CHeroTsk_SnipeDnHeart},
  {"ATTACK_ROOMS",         CHeroTsk_AttackRooms},
  {"SABOTAGE_ROOMS",       CHeroTsk_SabotageRooms},
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
  {"ON_NEUTRAL_GROUND",    CSelCrit_OnNeutralGround},
  {"ANYWHERE",             CSelCrit_Any},
  {NULL,                   0},
};

const struct NamedCommand on_experience_desc[] = {
  {"SizeIncreaseOnExp",            1},
  {"PayIncreaseOnExp",             2},
  {"SpellDamageIncreaseOnExp",     3},
  {"RangeIncreaseOnExp",           4},
  {"JobValueIncreaseOnExp",        5},
  {"HealthIncreaseOnExp",          6},
  {"StrengthIncreaseOnExp",        7},
  {"DexterityIncreaseOnExp",       8},
  {"DefenseIncreaseOnExp",         9},
  {"LoyaltyIncreaseOnExp",        10},
  {"ExpForHittingIncreaseOnExp",  11},
  {"TrainingCostIncreaseOnExp",   12},
  {"ScavengingCostIncreaseOnExp", 13},
  {NULL,                           0},
};

const struct NamedCommand modifier_desc[] = {
  {"Health",          1},
  {"Strength",        2},
  {"Armour",          3},
  {"SpellDamage",     4},
  {"Speed",           5},
  {"Salary",          6},
  {"TrainingCost",    7},
  {"ScavengingCost",  8},
  {"Loyalty",         9},
  {NULL,              0},
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

const struct NamedCommand script_operator_desc[] = {
  {"SET",         1},
  {"INCREASE",    2},
  {"DECREASE",    3},
  {"MULTIPLY",    4},
  {NULL,          0},
};

const struct NamedCommand script_boolean_desc[] = {
    {"0",        0},
    {"OFF",      0},
    {"NO",       0},
    {"FALSE",    0},
    {"DISABLE",  0},
    {"DISABLED", 0},
    {"1",        1},
    {"ON",       1},
    {"YES",      1},
    {"TRUE",     1},
    {"ENABLE",   1},
    {"ENABLED",  1},
    {NULL,       0},
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
    {"TOTAL_TRAPS",                 SVar_TOTAL_TRAPS},
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
    {"EVIL_CREATURES_CONVERTED",    SVar_EVIL_CREATURES_CONVERTED},
    {"GOOD_CREATURES_CONVERTED",    SVar_GOOD_CREATURES_CONVERTED},
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
    {"TRAPS_SOLD",                  SVar_TRAPS_SOLD},
    {"DOORS_SOLD",                  SVar_DOORS_SOLD},
    {"MANUFACTURED_SOLD",           SVar_MANUFACTURED_SOLD},
    {"MANUFACTURE_GOLD",            SVar_MANUFACTURE_GOLD},
    {"TOTAL_SCORE",                 SVar_TOTAL_SCORE},
    {"BONUS_TIME",                  SVar_BONUS_TIME},
    {"CREATURES_TRANSFERRED",       SVar_CREATURES_TRANSFERRED},
    {"ACTIVE_BATTLES",              SVar_ACTIVE_BATTLES},
    {"VIEW_TYPE",                   SVar_VIEW_TYPE},
    {"TOTAL_SLAPS",                 SVar_TOTAL_SLAPS},
    {"SCORE",                       SVar_SCORE},
    {"PLAYER_SCORE",                SVar_PLAYER_SCORE},
    {"MANAGE_SCORE",                SVar_MANAGE_SCORE},
    {NULL,                          0},
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

const struct NamedCommand fill_desc[] = {
  {"NONE",          FillIterType_NoFill},
  {"MATCH",         FillIterType_Match},
  {"FLOOR",         FillIterType_Floor},
  {"BRIDGE",        FillIterType_FloorBridge},
  {NULL,            0},
};

const struct NamedCommand locked_desc[] = {
  {"LOCKED", 1},
  {"UNLOCKED", 0},
  {NULL, 0}
};

const struct NamedCommand is_free_desc[] = {
  {"PAID", 0},
  {"FREE", 1},
  {NULL, 0}
};

const struct NamedCommand orientation_desc[] = {
  {"North",     ANGLE_NORTH},
  {"NorthEast", ANGLE_NORTHEAST},
  {"East",      ANGLE_EAST},
  {"SouthEast", ANGLE_SOUTHEAST},
  {"South",     ANGLE_SOUTH},
  {"SouthWest", ANGLE_SOUTHWEST},
  {"West",      ANGLE_WEST},
  {"NorthWest", ANGLE_NORTHWEST},
  {NULL, 0}
};

const struct NamedCommand texture_pack_desc[] = {
  {"NONE",         0},
  {"STANDARD",     1},
  {"ANCIENT",      2},
  {"WINTER",       3},
  {"SNAKE_KEY",    4},
  {"STONE_FACE",   5},
  {"VOLUPTUOUS",   6},
  {"BIG_BREASTS",  6},
  {"ROUGH_ANCIENT",7},
  {"SKULL_RELIEF", 8},
  {"DESERT_TOMB",  9},
  {"GYPSUM",       10},
  {"LILAC_STONE",  11},
  {"SWAMP_SERPENT",12},
  {"LAVA_CAVERN",  13},
  {"LATERITE_CAVERN",14},
  {NULL,           0},
};

ThingModel parse_creature_name(const char *creature_name)
{
    ThingModel ret = get_rid(creature_desc, creature_name);
    if (ret == -1)
    {
        if (0 == strcasecmp(creature_name, "ANY_CREATURE"))
        {
            return CREATURE_NOT_A_DIGGER; //For scripts, when we say 'ANY_CREATURE' we exclude diggers.
        }
    }
    return ret;
}

// Variables that could be set
TbBool parse_set_varib(const char *varib_name, int32_t *varib_id, int32_t *varib_type)
{
    char c;
    int len = 0;
    char arg[MAX_TEXT_LENGTH];

    *varib_id = -1;
    if (*varib_id == -1)
    {
      *varib_id = get_id(flag_desc, varib_name);
      *varib_type = SVar_FLAG;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(campaign_flag_desc, varib_name);
      *varib_type = SVar_CAMPAIGN_FLAG;
    }
    if (*varib_id == -1)
    {
        if (2 == sscanf(varib_name, "BOX%d_ACTIVATE%c", varib_id, &c) && (c == 'D'))
        {
            // activateD
            *varib_type = SVar_BOX_ACTIVATED;
        }
        else
        if (2 == sscanf(varib_name, "TRAP%d_ACTIVATE%c", varib_id, &c) && (c == 'D'))
        {
            // activateD
            *varib_type = SVar_TRAP_ACTIVATED;
        }
        else
        {
            *varib_id = -1;
        }
        if (2 == sscanf(varib_name, "SACRIFICED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(creature_desc, arg);
            *varib_type = SVar_SACRIFICED;
        }
        if (2 == sscanf(varib_name, "REWARDED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(creature_desc, arg);
            *varib_type = SVar_REWARDED;
        }
    }
    if (*varib_id == -1)
    {
      SCRPTERRLOG("Unknown variable name, '%s'", varib_name);
      return false;
    }
    return true;
}

TbBool parse_get_varib(const char *varib_name, int32_t *varib_id, int32_t *varib_type, long lvl_file_version)
{
    char c;
    int len = 0;
    char arg[MAX_TEXT_LENGTH];

    if (lvl_file_version > 0)
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
        if (2 == sscanf(varib_name, "BOX%d_ACTIVATE%c", varib_id, &c) && (c == 'D'))
        {
            // activateD
            *varib_type = SVar_BOX_ACTIVATED;
        }
        else if (2 == sscanf(varib_name, "TRAP%d_ACTIVATE%c", varib_id, &c) && (c == 'D'))
        {
            // activateD
            *varib_type = SVar_TRAP_ACTIVATED;
        }
        else if (2 == sscanf(varib_name, "KEEPERS_DESTROYED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(player_desc, arg);
            if (*varib_id == -1)
            {
                *varib_id = get_id(cmpgn_human_player_options, arg);
            }
            *varib_type = SVar_DESTROYED_KEEPER;
        }
        else if (2 == sscanf(varib_name, "SACRIFICED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(creature_desc, arg);
            *varib_type = SVar_SACRIFICED;
        }
        else if (2 == sscanf(varib_name, "REWARDED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(creature_desc, arg);
            *varib_type = SVar_REWARDED;
        }
    }
    if (*varib_id == -1)
    {
      SCRPTERRLOG("Unknown variable name, '%s'", varib_name);
      return false;
    }
    return true;
}

static void set_config_check(const struct NamedFieldSet* named_fields_set, const struct ScriptLine* scline, const char* src_str)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    const char* id_str = scline->tp[0];
    const char* property = scline->tp[1];
    const char* valuestrings[MAX_CONFIG_VALUES] = {scline->tp[2],scline->tp[3],scline->tp[4],scline->tp[5]};

    short id = get_id(named_fields_set->names, id_str);
    if (id == -1)
    {
        SCRPTERRLOG("Unknown %s, '%s'",named_fields_set->block_basename, id_str);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    long property_id = get_named_field_id(named_fields_set->named_fields, property);
    if (property_id == -1)
    {
        SCRPTERRLOG("Unknown property, '%s'", property);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    if (id > named_fields_set->max_count)
    {
        SCRPTERRLOG("'%s%d' is out of range",named_fields_set->block_basename, id);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    const struct NamedField* field = &named_fields_set->named_fields[property_id];

    char concatenated_values[MAX_TEXT_LENGTH];
    if (field->argnum == -1)
    {
        snprintf(concatenated_values, sizeof(concatenated_values), "%s %s %s %s", scline->tp[2],scline->tp[3],scline->tp[4],scline->tp[5]);
        value->longs[1] = parse_named_field_value(field, concatenated_values,named_fields_set,id,src_str,ccf_SplitExecution|ccf_DuringLevel);
    }
    else
    {
        for (size_t i = 0; i < MAX_CONFIG_VALUES; i++)
        {
            if(valuestrings[i][0] == '\0')
            {
                break;
            }

            if( named_fields_set->named_fields[property_id + i].name == NULL || (strcmp(named_fields_set->named_fields[property_id + i].name, named_fields_set->named_fields[property_id].name) != 0))
            {
                SCRPTERRLOG("more values then expected for property: '%s' '%s'", property, valuestrings[i]);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            else if (valuestrings[i][0] == '\0')
            {
                break;
            }
            value->longs[1 + i] = parse_named_field_value(&named_fields_set->named_fields[property_id + i], valuestrings[i],named_fields_set,id,src_str,ccf_SplitExecution|ccf_DuringLevel);
        }
    }

    value->shorts[0] = id;
    value->shorts[1] = property_id;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_config_process(const struct NamedFieldSet* named_fields_set, struct ScriptContext* context, const char* src_str)
{
    short id          = context->value->shorts[0];
    short property_id = context->value->shorts[1];

    for (size_t i = 0; i < MAX_CONFIG_VALUES; i++)
    {
        if( named_fields_set->named_fields[property_id + i].name == NULL ||
            (strcmp(named_fields_set->named_fields[property_id + i].name, named_fields_set->named_fields[property_id].name) != 0))
        {
            return;
        }
        else
        {
            assign_named_field_value(&named_fields_set->named_fields[property_id + i],context->value->longs[i+1],named_fields_set,id,src_str,ccf_SplitExecution|ccf_DuringLevel);
        }
    }
}

static void add_to_party_check(const struct ScriptLine *scline)
{
    int party_id = get_party_index_of_name(scline->tp[0]);
    if (party_id < 0)
    {
        SCRPTERRLOG("Invalid Party:%s",scline->tp[0]);
        return;
    }
    if ((scline->np[2] < 1) || (scline->np[2] > CREATURE_MAX_LEVEL))
    {
      SCRPTERRLOG("Invalid Creature Level parameter; %ld not in range (%d,%d)",scline->np[2],1,CREATURE_MAX_LEVEL);
      return;
    }
    long crtr_id = get_rid(creature_desc, scline->tp[1]);
    if (crtr_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
      return;
    }
    PlayerNumber target = -1;
    long objective_id = get_objective_id_with_potential_target(scline->tp[4], &target);
    if (objective_id == -1)
    {
      SCRPTERRLOG("Unknown party member objective, '%s'", scline->tp[4]);
      return;
    }
  //SCRPTLOG("Party '%s' member kind %d, level %d",prtname,crtr_id,exp_level);

    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0))
    {
        add_member_to_party(party_id, crtr_id, scline->np[2], scline->np[3], objective_id, scline->np[5], target);
    } else
    {
        if (game.script.party_triggers_num < PARTY_TRIGGERS_COUNT)
        {
            struct PartyTrigger* pr_trig = &game.script.party_triggers[game.script.party_triggers_num];
            pr_trig->flags = TrgF_ADD_TO_PARTY;
            pr_trig->flags |= next_command_reusable ? TrgF_REUSABLE : 0;
            pr_trig->party_id = party_id;
            pr_trig->creatr_id = crtr_id;
            pr_trig->exp_level = scline->np[2];
            pr_trig->carried_gold = scline->np[3];
            pr_trig->objectv = objective_id;
            pr_trig->countdown = scline->np[5];
            pr_trig->condit_idx = get_script_current_condition();
            pr_trig->target = target;
        }
        else
        {
            SCRPTERRLOG("Max party triggers reached, failed to update %s with %s", scline->tp[0], scline->tp[1]);
        }
        game.script.party_triggers_num++;
    }
}

static void delete_from_party_check(const struct ScriptLine *scline)
{
    int party_id = get_party_index_of_name(scline->tp[0]);
    if (party_id < 0)
    {
        SCRPTERRLOG("Invalid Party:%s",scline->tp[0]);
        return;
    }
    long creature_id = get_rid(creature_desc, scline->tp[1]);
    if (creature_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
      return;
    }
    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0))
    {
        delete_member_from_party(party_id, creature_id, scline->np[2]);
    } else
    {
        if (game.script.party_triggers_num < PARTY_TRIGGERS_COUNT)
        {
            struct PartyTrigger* pr_trig = &game.script.party_triggers[game.script.party_triggers_num];
            pr_trig->flags = TrgF_DELETE_FROM_PARTY;
            pr_trig->flags |= next_command_reusable ? TrgF_REUSABLE : 0;
            pr_trig->party_id = party_id;
            pr_trig->creatr_id = creature_id;
            pr_trig->exp_level = scline->np[2];
            pr_trig->condit_idx = get_script_current_condition();
        }
        else
        {
            SCRPTERRLOG("Max party triggers reached, failed to update %s with %s", scline->tp[0], scline->tp[1]);
        }
        game.script.party_triggers_num++;
    }
}

static void display_objective_check(const struct ScriptLine *scline)
{
  long msg_num = scline->np[0];
  long x, y;
  TbMapLocation location = 0;
  if ((msg_num < 0) || (msg_num >= STRINGS_MAX))
  {
    SCRPTERRLOG("Invalid TEXT number");
    return;
  }
  if (scline->command == Cmd_DISPLAY_OBJECTIVE)
  {
    const char *where = scline->tp[1];
    if (!get_map_location_id(where, &location))
    {
      return;
    }
    command_add_value(Cmd_DISPLAY_OBJECTIVE, ALL_PLAYERS, msg_num, location, 0);
  }
  else
  {
    x = scline->np[1];
    y = scline->np[2];
    command_add_value(Cmd_DISPLAY_OBJECTIVE, ALL_PLAYERS, msg_num, location, get_subtile_number(x,y));
  }
}

static void display_objective_process(struct ScriptContext *context)
{
    if (my_player_number == context->player_idx)
    {
        set_general_objective(context->value->longs[0],
        context->value->longs[1],
        stl_num_decode_x(context->value->longs[2]),
        stl_num_decode_y(context->value->longs[2]));
    }
}

static void tag_map_rect_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);

    MapSlabCoord x = scline->np[1];
    MapSlabCoord y = scline->np[2];
    MapSlabDelta width;
    MapSlabDelta height;

    if (scline->np[3] != '\0')
        width = scline->np[3];
    else
        width = 1;
    if (scline->np[4] != '\0')
        height = scline->np[4];
    else
        height = 1;

    MapSlabCoord start_x = x - (width / 2);
    MapSlabCoord end_x = x + (width / 2) + (width & 1);
    MapSlabCoord start_y = y - (height / 2);
    MapSlabCoord end_y = y + (height / 2) + (height & 1);

    if (start_x < 0)
    {
        SCRPTWRNLOG("Starting X slab '%d' (from %d-%d/2) is out of range, fixing it to '0'.", start_x, x, width);
        start_x = 0;
    }
    else if (start_x > game.map_tiles_x)
    {
        SCRPTWRNLOG("Starting X slab '%d' (from %d-%d/2) is out of range, fixing it to '%d'.", start_x, x, width, game.map_tiles_x);
        start_x = game.map_tiles_x;
    }
    if (end_x < 0)
    {
        SCRPTWRNLOG("Ending X slab '%d' (from %d+%d/2) is out of range, fixing it to '0'.", end_x, x, width);
        end_x = 0;
    }
    else if (end_x > game.map_tiles_x)
    {
        SCRPTWRNLOG("Ending X slab '%d' (from %d+%d/2) is out of range, fixing it to '%d'.", end_x, x, width, game.map_tiles_x);
        end_x = game.map_tiles_x;
    }
    if (start_y < 0)
    {
        SCRPTWRNLOG("Starting Y slab '%d' (from %d-%d/2) is out of range, fixing it to '0'.", start_y, y, height);
        start_y = 0;
    }
    else if (start_y > game.map_tiles_y)
    {
        SCRPTWRNLOG("Starting Y slab '%d' (from %d-%d/2) is out of range, fixing it to '%d'.", start_y, y, height, game.map_tiles_y);
        start_y = game.map_tiles_y;
    }
    if (end_y < 0)
    {
        SCRPTWRNLOG("Ending Y slab '%d' (from %d+%d/2) is out of range, fixing it to '0'.", end_y, y, height);
        end_y = 0;
    }
    else if (end_y > game.map_tiles_y)
    {
        SCRPTWRNLOG("Ending Y slab '%d' (from %d+%d/2) is out of range, fixing it to '%d'.", end_y, y, height, game.map_tiles_y);
        end_y = game.map_tiles_y;
    }
    if ((x < 0) || (x > game.map_tiles_y) || (y < 0) || (y > game.map_tiles_y))
    {
        SCRPTERRLOG("Conceal slabs out of range, trying to set conceal center point to (%d,%d) on map that's %dx%d slabs", x, y, game.map_tiles_x, game.map_tiles_y);
        DEALLOCATE_SCRIPT_VALUE
            return;
    }
    value->shorts[1] = start_x;
    value->shorts[2] = end_x;
    value->shorts[3] = start_y;
    value->shorts[4] = end_y;

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void tag_map_rect_process(struct ScriptContext* context)
{
    MapSlabCoord start_x = context->value->shorts[1];
    MapSlabCoord end_x = context->value->shorts[2];
    MapSlabCoord start_y = context->value->shorts[3];
    MapSlabCoord end_y = context->value->shorts[4];

    for (short x = start_x; x < end_x; x++)
    {
        for (short y = start_y; y < end_y; y++)
        {
            MapSubtlCoord stl_x = slab_subtile_center(x);
            MapSubtlCoord stl_y = slab_subtile_center(y);

            if (subtile_is_diggable_for_player(context->player_idx, stl_x, stl_y, false))
            {
                tag_blocks_for_digging_in_area(stl_x, stl_y, context->player_idx);
            }
        }
    }
}

static void untag_map_rect_process(struct ScriptContext* context)
{
    MapSlabCoord start_x = context->value->shorts[1];
    MapSlabCoord end_x = context->value->shorts[2];
    MapSlabCoord start_y = context->value->shorts[3];
    MapSlabCoord end_y = context->value->shorts[4];

    for (short x = start_x; x < end_x; x++)
    {
        for (short y = start_y; y < end_y; y++)
        {
            MapSubtlCoord stl_x = slab_subtile_center(x);
            MapSubtlCoord stl_y = slab_subtile_center(y);

            if (subtile_is_diggable_for_player(context->player_idx, stl_x, stl_y, false))
            {
                untag_blocks_for_digging_in_area(stl_x, stl_y, context->player_idx);
            }
        }
    }
}


static void conceal_map_rect_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    TbBool conceal_all = false;

    if (scline->np[5] == -1)
    {
        if ((strcmp(scline->tp[5], "") == 0))
        {
            conceal_all = false;
        }
        else if ((strcmp(scline->tp[5], "ALL") == 0))
        {
            conceal_all = true;
        }
        else
        {
            SCRPTWRNLOG("Hide value \"%s\" not recognized", scline->tp[5]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    else
    {
        conceal_all = scline->np[5];
    }

    MapSubtlCoord x = scline->np[1];
    MapSubtlCoord y = scline->np[2];
    MapSubtlDelta width = scline->np[3];
    MapSubtlDelta height = scline->np[4];

    MapSubtlCoord start_x = x - (width / 2);
    MapSubtlCoord end_x = x + (width / 2) + (width & 1);
    MapSubtlCoord start_y = y - (height / 2);
    MapSubtlCoord end_y = y + (height / 2) + (height & 1);

    if (start_x < 0)
    {
        SCRPTWRNLOG("Starting X coordinate '%d' (from %d-%d/2) is out of range, fixing it to '0'.", start_x,x,width);
        start_x = 0;
    }
    else if (start_x > game.map_subtiles_x)
    {
        SCRPTWRNLOG("Starting X coordinate '%d' (from %d-%d/2) is out of range, fixing it to '%d'.", start_x, x, width, game.map_subtiles_x);
        start_x = game.map_subtiles_x;
    }
    if (end_x < 0)
    {
        SCRPTWRNLOG("Ending X coordinate '%d' (from %d+%d/2) is out of range, fixing it to '0'.", end_x, x, width);
        end_x = 0;
    }
    else if (end_x > game.map_subtiles_x)
    {
        SCRPTWRNLOG("Ending X coordinate '%d' (from %d+%d/2) is out of range, fixing it to '%d'.", end_x, x, width, game.map_subtiles_x);
        end_x = game.map_subtiles_x;
    }
    if (start_y < 0)
    {
        SCRPTWRNLOG("Starting Y coordinate '%d' (from %d-%d/2) is out of range, fixing it to '0'.", start_y, y, height);
        start_y = 0;
    }
    else if (start_y > game.map_subtiles_y)
    {
        SCRPTWRNLOG("Starting Y coordinate '%d' (from %d-%d/2) is out of range, fixing it to '%d'.", start_y, y, height, game.map_subtiles_y);
        start_y = game.map_subtiles_y;
    }
    if (end_y < 0)
    {
        SCRPTWRNLOG("Ending Y coordinate '%d' (from %d+%d/2) is out of range, fixing it to '0'.", end_y, y, height);
        end_y = 0;
    }
    else if (end_y > game.map_subtiles_y)
    {
        SCRPTWRNLOG("Ending Y coordinate '%d' (from %d+%d/2) is out of range, fixing it to '%d'.", end_y, y, height, game.map_subtiles_y);
        end_y = game.map_subtiles_y;
    }
    if ((x < 0) || (x > game.map_subtiles_x) || (y < 0) || (y > game.map_subtiles_y))
    {
        SCRPTERRLOG("Conceal coordinates out of range, trying to set conceal center point to (%d,%d) on map that's %dx%d subtiles", x, y, game.map_subtiles_x, game.map_subtiles_y);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[1] = start_x;
    value->shorts[2] = end_x;
    value->shorts[3] = start_y;
    value->shorts[4] = end_y;
    value->shorts[5] = conceal_all;

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void conceal_map_rect_process(struct ScriptContext *context)
{
    MapSubtlCoord start_x = context->value->shorts[1];
    MapSubtlCoord end_x = context->value->shorts[2];
    MapSubtlCoord start_y = context->value->shorts[3];
    MapSubtlCoord end_y = context->value->shorts[4];
    TbBool conceal_all = context->value->shorts[5];
    conceal_map_area(context->player_idx, start_x, end_x, start_y, end_y, conceal_all);
}

/**
 * Transfers creatures for a player
 * @param plyr_idx target player
 * @param crmodel the creature model to transfer
 * @param criteria the creature selection criterion
 * @param count the amount of units to transfer
 */
static int script_transfer_creature(PlayerNumber plyr_idx, ThingModel crmodel, long criteria, int count)
{
    short transferred = 0;
    struct Thing* thing;
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl;
    for (int i = 0; i < count; i++)
    {
        thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
        cctrl = creature_control_get_from_thing(thing);
        if ((!thing_exists(thing)) && (i == 0))
        {
            SYNCDBG(5, "No matching player %d creature of model %d found to transfer.", (int)plyr_idx, (int)crmodel);
            break;
        }

        if (add_transfered_creature(plyr_idx, thing->model, cctrl->exp_level, cctrl->creature_name))
        {
            transferred++;
            dungeon = get_dungeon(plyr_idx);
            dungeon->creatures_transferred++;
            remove_thing_from_power_hand_list(thing, plyr_idx);
            struct SpecialConfigStats* specst = get_special_model_stats(SpcKind_TrnsfrCrtr);
            create_used_effect_or_element(&thing->mappos, specst->effect_id, plyr_idx, thing->index);
            kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects | CrDed_NotReallyDying);
        }
    }
    return transferred;
}

static void special_transfer_creature_process(struct ScriptContext* context)
{
    if (my_player_number == context->player_idx)
    {
        struct Thing *heartng = get_player_soul_container(context->player_idx);
        struct PlayerInfo* player = get_my_player();
        start_transfer_creature(player, heartng);
    }
}

static void special_transfer_creature_check(const struct ScriptLine* scline)
{
    command_add_value(Cmd_USE_SPECIAL_TRANSFER_CREATURE, scline->np[0],0,0,0);
}

static void script_transfer_creature_check(const struct ScriptLine* scline)
{
    long crtr_id = parse_creature_name(scline->tp[1]);
    long count = scline->np[3];
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        return;
    }
    long select_id = parse_criteria(scline->tp[2]);
    if (select_id == -1) {
        SCRPTERRLOG("Unknown select criteria, '%s'", scline->tp[2]);
        return;
    }
    if (scline->np[3] == '\0')
    {
        count = 1;
    }
    if (count == 0)
    {
        SCRPTERRLOG("Transferring 0 creatures of type '%s'", scline->tp[1]);
    }
    if (count > 255)
    {
        SCRPTWRNLOG("Trying to transfer %ld creatures out of a possible 255",count);
        count = 255;
    }
    command_add_value(Cmd_TRANSFER_CREATURE, scline->np[0], crtr_id, select_id, count);
}

static void script_transfer_creature_process(struct ScriptContext* context)
{
    script_transfer_creature(context->player_idx, context->value->longs[0], context->value->longs[1], context->value->longs[2]);
}

static void change_creatures_annoyance_check(const struct ScriptLine* scline)
{
    long crtr_id = parse_creature_name(scline->tp[1]);
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        return;
    }
    long op_id = get_rid(script_operator_desc, scline->tp[2]);
    if (op_id == -1)
    {
        SCRPTERRLOG("Invalid operation for changing creatures' annoyance: '%s'", scline->tp[2]);
        return;
    }
    command_add_value(Cmd_CHANGE_CREATURES_ANNOYANCE, scline->np[0], crtr_id, op_id, scline->np[3]);
}

static void change_creatures_annoyance_process(struct ScriptContext* context)
{
    script_change_creatures_annoyance(context->player_idx, context->value->longs[0], context->value->longs[1], context->value->longs[2]);
}

static void set_trap_configuration_check(const struct ScriptLine* scline)
{
    set_config_check(&trapdoor_trap_named_fields_set, scline, "SET_TRAP_CONFIGURATION");
}

static void set_room_configuration_check(const struct ScriptLine* scline)
{
    set_config_check(&terrain_room_named_fields_set, scline, "SET_ROOM_CONFIGURATION");
}

static void set_hand_rule_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);

    const char *param_name = scline->tp[5];
    long crtr_id = parse_creature_name(scline->tp[1]);
    short hr_action, hr_slot, hr_type, param;

    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        return;
    }
    hr_slot = get_id(rule_slot_desc, scline->tp[2]);
    if (hr_slot == -1) {
        SCRPTERRLOG("Invalid hand rule slot: '%s'", scline->tp[2]);
        return;
    }
    hr_action = get_id(rule_action_desc, scline->tp[3]);
    if (hr_action == -1) {
        SCRPTERRLOG("Invalid hand rule action: '%s'", scline->tp[3]);
        return;
    }
    if (hr_action == HandRuleAction_Allow || hr_action == HandRuleAction_Deny)
    {
        hr_type = get_id(hand_rule_desc, scline->tp[4]);
        if (hr_type == -1) {
            SCRPTERRLOG("Invalid hand rule: '%s'", scline->tp[4]);
            return;
        }
        param = hr_type == HandRule_AffectedBy ? 0 : atol(param_name);
        if (hr_type == HandRule_AtActionPoint && action_point_number_to_index(param) == -1)
        {
            SCRPTERRLOG("Unknown action point param for hand rule: '%d'", param);
            return;
        }
        if (hr_type == HandRule_AffectedBy)
        {
            long mag_id = get_id(spell_desc, param_name);
            if (mag_id == -1)
            {
                SCRPTERRLOG("Unknown magic, '%s'", param_name);
                return;
            }
            param = mag_id;
        }
    } else
    {
        hr_type = 0;
        param = 0;
    }

    value->shorts[0] = crtr_id;
    value->shorts[1] = hr_action;
    value->shorts[2] = hr_slot;
    value->shorts[3] = hr_type;
    value->shorts[4] = param;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void move_creature_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);

    long crmodel = parse_creature_name(scline->tp[1]);
    if (crmodel == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        return;
    }
    long select_id = parse_criteria(scline->tp[2]);
    if (select_id == -1) {
        SCRPTERRLOG("Unknown select criteria, '%s'", scline->tp[2]);
        return;
    }

    long count = scline->np[3];
    if (count <= 0)
    {
        SCRPTERRLOG("Bad creatures count, %ld", count);
        return;
    }

    TbMapLocation location;
    if (!get_map_location_id(scline->tp[4], &location))
    {
        SCRPTWRNLOG("Invalid location: %s", scline->tp[4]);
        return;
    }

    const char *effect_name = scline->tp[5];
    long effct_id = 0;
    if (scline->tp[5][0] != '\0')
    {
        effct_id = get_rid(effect_desc, effect_name);
        if (effct_id == -1)
        {
            if (parameter_is_number(effect_name))
            {
                effct_id = atoi(effect_name);
            }
            else
            {
                SCRPTERRLOG("Unrecognised effect: %s", effect_name);
                return;
            }
        }
    }
    else
    {
        effct_id = -1;
    }
    value->ulongs[0] = location;
    value->longs[1] = select_id;
    value->shorts[4] = effct_id;
    value->bytes[10] = count;
    value->bytes[11] = crmodel;

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void count_creatures_at_action_point_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    PlayerNumber player_id = scline->np[1];
    long crmodel = parse_creature_name(scline->tp[2]);
    if (crmodel == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[2]);
        return;
    }
    short ap_num = scline->np[0];
    char flag_player_id = scline->np[3];
    const char *flag_name = scline->tp[4];

    int32_t flag_id, flag_type;
    if (!parse_get_varib(flag_name, &flag_id, &flag_type, level_file_version))
    {
        SCRPTERRLOG("Unknown flag, '%s'", flag_name);
        return;
    }

    value->shorts[0] = ap_num;
    value->bytes[2] = crmodel;
    value->chars[3] = flag_player_id;
    value->shorts[2] = flag_id;
    value->chars[6] = flag_type;
    value->longs[3] = player_id;

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void new_creature_type_check(const struct ScriptLine* scline)
{
    if (game.conf.crtr_conf.model_count >= CREATURE_TYPES_MAX)
    {
        SCRPTERRLOG("Cannot increase creature type count for creature type '%s', already at maximum %d types.", scline->tp[0], CREATURE_TYPES_MAX);
        return;
    }

    int i = game.conf.crtr_conf.model_count;
    game.conf.crtr_conf.model_count++;
    snprintf(game.conf.crtr_conf.model[i].name, COMMAND_WORD_LEN, "%s", scline->tp[0]);
    creature_desc[i-1].name = game.conf.crtr_conf.model[i].name;
    creature_desc[i-1].num = i;

    if (load_creaturemodel_config(i, 0))
    {
        SCRPTLOG("Adding creature type %s and increasing creature types to %d", creature_code_name(i), game.conf.crtr_conf.model_count - 1);
    }
    else
    {
        SCRPTERRLOG("Failed to load config for creature '%s'(%d).", game.conf.crtr_conf.model[i].name,i);
    }
}

static void new_room_type_check(const struct ScriptLine* scline)
{
    if (game.conf.slab_conf.room_types_count >= TERRAIN_ITEMS_MAX - 1)
    {
        SCRPTERRLOG("Cannot increase room count for room type '%s', already at maximum %d rooms.", scline->tp[0], TERRAIN_ITEMS_MAX - 1);
        return;
    }

    SCRPTLOG("Adding room type %s and increasing 'RoomsCount to %d", scline->tp[0], game.conf.slab_conf.room_types_count + 1);
    game.conf.slab_conf.room_types_count++;

    struct RoomConfigStats* roomst;
    int i = game.conf.slab_conf.room_types_count - 1;

    roomst = &game.conf.slab_conf.room_cfgstats[i];
    memset(roomst->code_name, 0, COMMAND_WORD_LEN);
    snprintf(roomst->code_name, COMMAND_WORD_LEN, "%s", scline->tp[0]);
    roomst->name_stridx = GUIStr_Empty;
    roomst->tooltip_stridx = GUIStr_Empty;
    roomst->creature_creation_model = 0;
    roomst->bigsym_sprite_idx = 0;
    roomst->medsym_sprite_idx = 0;
    roomst->pointer_sprite_idx = 0;
    roomst->panel_tab_idx = 0;
    roomst->ambient_snd_smp_id = 0;
    roomst->msg_needed = 0;
    roomst->msg_too_small = 0;
    roomst->msg_no_route = 0;
    roomst->roles = RoRoF_None;
    roomst->cost = 0;
    roomst->health = 0;
    room_desc[i].name = roomst->code_name;
    room_desc[i].num = i;
}

static void new_object_type_check(const struct ScriptLine* scline)
{
    if (game.conf.object_conf.object_types_count >= OBJECT_TYPES_MAX-1)
    {
        SCRPTERRLOG("Cannot increase object count for object type '%s', already at maximum %d objects.", scline->tp[0], OBJECT_TYPES_MAX-1);
        return;
    }

    SCRPTLOG("Adding object type %s and increasing 'ObjectsCount to %d", scline->tp[0], game.conf.object_conf.object_types_count + 1);
    game.conf.object_conf.object_types_count++;

    int tmodel = game.conf.object_conf.object_types_count -1;
    struct ObjectConfigStats* objst = get_object_model_stats(tmodel);
    memset(objst->code_name, 0, COMMAND_WORD_LEN);
    snprintf(objst->code_name, COMMAND_WORD_LEN, "%s", scline->tp[0]);
    objst->map_icon = 0;
    objst->hand_icon = 0;
    objst->genre = 0;
    objst->draw_class = ODC_Default;
    object_desc[tmodel].name = objst->code_name;
    object_desc[tmodel].num = tmodel;
}

static void new_trap_type_check(const struct ScriptLine* scline)
{
    if (game.conf.trapdoor_conf.trap_types_count >= TRAPDOOR_TYPES_MAX)
    {
        SCRPTERRLOG("Cannot increase trap count for trap type '%s', already at maximum %d traps.", scline->tp[0], TRAPDOOR_TYPES_MAX);
        return;
    }
    SCRPTLOG("Adding trap type %s and increasing 'TrapsCount to %d", scline->tp[0], game.conf.trapdoor_conf.trap_types_count + 1);
    game.conf.trapdoor_conf.trap_types_count++;
    short i = game.conf.trapdoor_conf.trap_types_count-1;
    struct TrapConfigStats *trapst = get_trap_model_stats(i);
    memset(trapst->code_name, 0, COMMAND_WORD_LEN);
    snprintf(trapst->code_name, COMMAND_WORD_LEN, "%s", scline->tp[0]);
    trapst->name_stridx = GUIStr_Empty;
    trapst->tooltip_stridx = GUIStr_Empty;
    trapst->bigsym_sprite_idx = 0;
    trapst->medsym_sprite_idx = 0;
    trapst->pointer_sprite_idx = 0;
    trapst->panel_tab_idx = 0;
    trapst->manufct_level = 0;
    trapst->manufct_required = 0;
    trapst->shots = 0;
    trapst->shots_delay = 0;
    trapst->initial_delay = 0;
    trapst->trigger_type = 0;
    trapst->activation_type = 0;
    trapst->created_itm_model = 0;
    trapst->hit_type = 0;
    trapst->hidden = true;
    trapst->slappable = 0;
    trapst->detect_invisible = true;
    trapst->notify = false;
    trapst->place_on_bridge = false;
    trapst->place_on_subtile = false;
    trapst->instant_placement = false;
    trapst->remove_once_depleted = false;
    trapst->health = 1;
    trapst->destructible = 0;
    trapst->unstable = 0;
    trapst->destroyed_effect = -39;
    trapst->size_xy = 0;
    trapst->size_z = 0;
    trapst->sprite_anim_idx = 0;
    trapst->attack_sprite_anim_idx = 0;
    trapst->recharge_sprite_anim_idx = 0;
    trapst->sprite_size_max = 0;
    trapst->anim_speed = 0;
    trapst->unanimated = 0;
    trapst->unshaded = 0;
    trapst->random_start_frame = 0;
    trapst->light_radius = 0;
    trapst->light_intensity = 0;
    trapst->light_flag = 0;
    trapst->transparency_flag = 0;
    trapst->shot_shift_x = 0;
    trapst->shot_shift_y = 0;
    trapst->shot_shift_z = 0;
    trapst->shotvector.x = 0;
    trapst->shotvector.y = 0;
    trapst->shotvector.z = 0;
    trapst->selling_value = 0;
    trapst->unsellable = false;
    trapst->place_sound_idx = 117;
    trapst->trigger_sound_idx = 176;
    trap_desc[i].name = trapst->code_name;
    trap_desc[i].num = i;
    create_manufacture_array_from_trapdoor_data();
}

static void set_trap_configuration_process(struct ScriptContext *context)
{
    set_config_process(&trapdoor_trap_named_fields_set, context, "SET_TRAP_CONFIGURATION");
}

static void set_room_configuration_process(struct ScriptContext *context)
{
    set_config_process(&terrain_room_named_fields_set, context, "SET_ROOM_CONFIGURATION");
}

static void set_hand_rule_process(struct ScriptContext* context)
{
    PlayerNumber plyr_idx = context->player_idx;
    long crtr_id = context->value->shorts[0];
    long hand_rule_action = context->value->shorts[1];
    long hand_rule_slot = context->value->shorts[2];
    long hand_rule_type = context->value->shorts[3];
    long param = context->value->shorts[4];

    script_set_hand_rule(plyr_idx, crtr_id, hand_rule_action, hand_rule_slot, hand_rule_type, param);
}

static void move_creature_process(struct ScriptContext* context)
{
    TbMapLocation location = context->value->ulongs[0];
    long select_id = context->value->longs[1];
    long effect_id = context->value->shorts[4];
    long count = context->value->bytes[10];
    long crmodel = context->value->bytes[11];
    PlayerNumber plyr_idx = context->player_idx;

    script_move_creature_with_criteria(plyr_idx, crmodel, select_id, location, effect_id, count);
}

static void count_creatures_at_action_point_process(struct ScriptContext* context)
{
    long ap_num = context->value->shorts[0];
    long crmodel = context->value->bytes[2];
    long flag_player_id = context->value->chars[3];
    long flag_id = context->value->shorts[2];
    long flag_type = context->value->chars[6];
    PlayerNumber player_id = context->value->longs[3];

    long sum = 0;

    if (player_id == ALL_PLAYERS)
    {
        for (int i = 0; i < PLAYERS_COUNT; i++)
        {
            sum += count_player_creatures_of_model_in_action_point(i, crmodel, action_point_number_to_index(ap_num));
        }
    }
    else
    {
        sum = count_player_creatures_of_model_in_action_point(player_id, crmodel, action_point_number_to_index(ap_num));
    }

    set_variable(flag_player_id, flag_type, flag_id, sum);
}

static void set_door_configuration_check(const struct ScriptLine* scline)
{
    set_config_check(&trapdoor_door_named_fields_set, scline, "SET_DOOR_CONFIGURATION");
}

static void set_door_configuration_process(struct ScriptContext *context)
{
    set_config_process(&trapdoor_door_named_fields_set, context, "SET_DOOR_CONFIGURATION");
}

static void create_effect_at_pos_process(struct ScriptContext* context)
{
    struct Coord3d pos;
    set_coords_to_subtile_center(&pos, context->value->shorts[1], context->value->shorts[2], 0);
    pos.z.val += get_floor_height(pos.x.stl.num, pos.y.stl.num);
    script_create_effect(&pos,context->value->shorts[0],context->value->longs[2]);

}

static void create_effect_process(struct ScriptContext *context)
{
    struct Coord3d pos;
    if (!get_coords_at_location(&pos, context->value->ulongs[1],true))
    {
        SCRPTWRNLOG("Could not find location %lu to create effect", context->value->ulongs[1]);
        return;
    }
    script_create_effect(&pos,context->value->shorts[0],context->value->longs[2]);

}

static void set_heart_health_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    struct Thing* heartng = get_player_soul_container(value->longs[0]);
    struct ObjectConfigStats* objst = get_object_model_stats(heartng->model);
    if (scline->np[1] > objst->health)
    {
        SCRPTWRNLOG("Value %ld is greater than maximum: %d", scline->np[1], objst->health);
        value->longs[1] = objst->health;
    }
    else
    {
        value->longs[1] = scline->np[1];
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_heart_health_process(struct ScriptContext *context)
{
    struct Thing* heartng = get_player_soul_container(context->player_idx);
    if (thing_exists(heartng))
    {
        heartng->health = (short)context->value->longs[1];
    }
}

static void add_heart_health_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    value->longs[1] = scline->np[1];
    value->longs[2] = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_heart_health_process(struct ScriptContext *context)
{
    PlayerNumber plyr_idx = context->player_idx;
    HitPoints healthdelta = context->value->longs[1];
    TbBool warn_on_damage = context->value->longs[2];

    add_heart_health(plyr_idx,healthdelta,warn_on_damage);
}

static void lock_possession_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    short locked = scline->np[1];
    if (locked == -1)
    {
        locked = get_id(locked_desc, scline->tp[1]);
        if (locked == -1)
        {
            SCRPTERRLOG("Invalid Possession lock value (%s) not recognized.", scline->tp[1]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }

    value->chars[1] = locked;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void lock_possession_process(struct ScriptContext* context)
{
    struct PlayerInfo *player = get_player(context->player_idx);
    if (player_exists(player))
    {
        player->possession_lock = context->value->chars[1];
    }
}

static void heart_lost_quick_objective_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if ((scline->np[0] < 0) || (scline->np[0] >= QUICK_MESSAGES_COUNT))
    {
        SCRPTERRLOG("Invalid QUICK OBJECTIVE number (%ld)", scline->np[0]);
        return;
    }
    if (strlen(scline->tp[1]) >= MESSAGE_TEXT_LEN)
    {
        SCRPTWRNLOG("Objective TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
    }
    if ((game.quick_messages[scline->np[0]][0] != '\0') && (strcmp(game.quick_messages[scline->np[0]],scline->tp[1]) != 0))
    {
        SCRPTWRNLOG("Quick Objective no %ld overwritten by different text", scline->np[0]);
    }
    snprintf(game.quick_messages[scline->np[0]], MESSAGE_TEXT_LEN, "%s", scline->tp[1]);

    TbMapLocation location = 0;
    if (scline->tp[2][0] != '\0')
    {
        get_map_location_id(scline->tp[2], &location);
    }

    value->longs[0] = scline->np[0];
    value->ulongs[2] = location;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void heart_lost_quick_objective_process(struct ScriptContext *context)
{
    game.heart_lost_display_message = true;
    game.heart_lost_quick_message = true;
    game.heart_lost_message_id = context->value->longs[0];
    game.heart_lost_message_target = context->value->longs[2];
}

static void heart_lost_objective_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->longs[0] = scline->np[0];
    TbMapLocation location = 0;
    if (scline->tp[1][0] != '\0')
    {
        get_map_location_id(scline->tp[1], &location);
    }
    value->ulongs[1] = location;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void heart_lost_objective_process(struct ScriptContext *context)
{
    game.heart_lost_display_message = true;
    game.heart_lost_quick_message = false;
    game.heart_lost_message_id = context->value->longs[0];
    game.heart_lost_message_target = context->value->longs[1];
}

static void set_door_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long doorAction = get_id(locked_desc, scline->tp[0]);
    if (doorAction == -1)
    {
        SCRPTERRLOG("Set Door state %s not recognized", scline->tp[0]);
        return;
    }

    if (slab_coords_invalid(scline->np[1], scline->np[2]))
    {
        SCRPTERRLOG("Invalid slab coordinates: %ld, %ld", scline->np[1], scline->np[2]);
        return;
    }

    value->shorts[0] = doorAction;
    value->shorts[1] = scline->np[1];
    value->shorts[2] = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_door_process(struct ScriptContext* context)
{
    struct Thing* doortng = get_door_for_position(slab_subtile_center(context->value->shorts[1]), slab_subtile_center(context->value->shorts[2]));
    if (!thing_is_invalid(doortng))
    {
        switch (context->value->shorts[0])
        {
        case 0:
            unlock_door(doortng);
            break;
        case 1:
            lock_door(doortng);
            break;
        }
    }
}

static void place_door_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    const char* doorname = scline->tp[1];
    short door_id = get_id(door_desc, doorname);

    if (door_id == -1)
    {
        SCRPTERRLOG("Unknown door, '%s'", doorname);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    if (slab_coords_invalid(scline->np[2], scline->np[3]))
    {
        SCRPTERRLOG("Invalid slab coordinates: %ld, %ld", scline->np[2], scline->np[3]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    short locked = scline->np[4];
    if (locked == -1)
    {
        locked = get_id(locked_desc, scline->tp[4]);
        if (locked == -1)
        {
            SCRPTERRLOG("Door locked state %s not recognized", scline->tp[4]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }

    short free = scline->np[5];
    if (free == -1)
    {
        free = get_id(is_free_desc, scline->tp[5]);
        if (free == -1)
        {
            SCRPTERRLOG("Place Door free state '%s' not recognized", scline->tp[5]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }

    value->shorts[1] = door_id;
    value->shorts[2] = scline->np[2];
    value->shorts[3] = scline->np[3];
    value->shorts[4] = locked;
    value->shorts[5] = free;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void place_door_process(struct ScriptContext* context)
{
    ThingModel doorkind = context->value->shorts[1];
    MapSlabCoord slb_x = context->value->shorts[2];
    MapSlabCoord slb_y = context->value->shorts[3];
    TbBool locked = context->value->shorts[4];
    TbBool free = context->value->shorts[5];
    PlayerNumber plyridx = context->player_idx;

    script_place_door(plyridx, doorkind, slb_x, slb_y, locked, free);
}

static void place_trap_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    const char* trapname = scline->tp[1];
    short trap_id = get_id(trap_desc, trapname);

    if (trap_id == -1)
    {
        SCRPTERRLOG("Unknown trap, '%s'", trapname);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    if (subtile_coords_invalid(scline->np[2], scline->np[3]))
    {
        SCRPTERRLOG("Invalid subtile coordinates: %ld, %ld", scline->np[2], scline->np[3]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    short free = scline->np[4];
    if (free == -1)
    {
        free = get_id(is_free_desc, scline->tp[4]);
        if (free == -1)
        {
            SCRPTERRLOG("Place Trap free state '%s' not recognized", scline->tp[4]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }

    value->shorts[1] = trap_id;
    value->shorts[2] = scline->np[2];
    value->shorts[3] = scline->np[3];
    value->shorts[4] = free;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void place_trap_process(struct ScriptContext* context)
{
    ThingModel trapkind = context->value->shorts[1];
    MapSubtlCoord stl_x = context->value->shorts[2];
    MapSubtlCoord stl_y = context->value->shorts[3];
    TbBool free = context->value->shorts[4];
    PlayerNumber plyridx = context->player_idx;
    script_place_trap(plyridx, trapkind, stl_x, stl_y, free);

}

static void create_effects_line_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    value->longs[0] = scline->np[0]; // AP `from`
    value->longs[1] = scline->np[1]; // AP `to`
    value->chars[8] = scline->np[2]; // curvature
    value->bytes[9] = scline->np[3]; // spatial stepping
    value->bytes[10] = scline->np[4]; // temporal stepping
    const char* effect_name = scline->tp[5];

    EffectOrEffElModel effct_id = effect_or_effect_element_id(effect_name);
    if (effct_id == 0)
    {
        SCRPTERRLOG("Unrecognised effect: %s", effect_name);
        return;
    }

    value->shorts[6] = effct_id; // effect

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void create_effects_line_process(struct ScriptContext *context)
{
    TbMapLocation from = context->value->longs[0];
    TbMapLocation to   = context->value->longs[1];
    char curvature = context->value->chars[8];
    unsigned char spatial_stepping = context->value->bytes[9];
    unsigned char temporal_stepping = context->value->bytes[10];
    EffectOrEffElModel effct_id = context->value->shorts[6];

    create_effects_line(from, to, curvature, spatial_stepping, temporal_stepping, effct_id);
}

static void set_object_configuration_check(const struct ScriptLine *scline)
{
    set_config_check(&objects_named_fields_set, scline, "SET_OBJECT_CONFIGURATION");
}

enum CreatureConfiguration
{
    CrtConf_NONE,
    CrtConf_ATTRIBUTES,
    CrtConf_ATTRACTION,
    CrtConf_ANNOYANCE,
    CrtConf_SENSES,
    CrtConf_APPEARANCE,
    CrtConf_EXPERIENCE,
    CrtConf_JOBS,
    CrtConf_SPRITES,
    CrtConf_SOUNDS,
    CrtConf_LISTEND
};

static void set_creature_configuration_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    short creatvar = get_id(creatmodel_attributes_commands, scline->tp[1]);
    short block = CrtConf_ATTRIBUTES;
    if (creatvar == -1)
    {
        creatvar = get_id(creatmodel_jobs_commands, scline->tp[1]);
        block = CrtConf_JOBS;
        if (creatvar == -1)
        {
            block = CrtConf_ATTRACTION;
            creatvar = get_id(creatmodel_attraction_commands, scline->tp[1]);
            if (creatvar == -1)
            {
                block = CrtConf_SOUNDS;
                creatvar = get_id(creatmodel_sounds_commands, scline->tp[1]);
                if (creatvar == -1)
                {
                    block = CrtConf_SPRITES;
                    creatvar = get_id(creature_graphics_desc, scline->tp[1]);
                    if (creatvar == -1)
                    {
                        block = CrtConf_ANNOYANCE;
                        creatvar = get_id(creatmodel_annoyance_commands, scline->tp[1]);
                        if (creatvar == -1)
                        {
                            block = CrtConf_EXPERIENCE;
                            creatvar = get_id(creatmodel_experience_commands, scline->tp[1]);
                            if (creatvar == -1)
                            {
                                block = CrtConf_APPEARANCE;
                                creatvar = get_id(creatmodel_appearance_commands, scline->tp[1]);
                                if (creatvar == -1)
                                {
                                    block = CrtConf_SENSES;
                                    creatvar = get_id(creatmodel_senses_commands, scline->tp[1]);
                                    if (creatvar == -1)
                                    {
                                        SCRPTERRLOG("Unknown creature configuration variable");
                                        DEALLOCATE_SCRIPT_VALUE
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    long config_value_primary = 0, config_value_secondary = 0, config_value_tertiary = 0, config_value_quaternary = 0, config_value_quinary = 0, config_value_senary = 0;
    if (block == CrtConf_ATTRIBUTES)
    {
        if (creatvar == 20) // ATTACKPREFERENCE
        {
            config_value_primary = get_id(attackpref_desc, scline->tp[2]);
        }
        else if (creatvar == 34) // LAIROBJECT
        {
            if (parameter_is_number(scline->tp[2])) // Support name or number for lair object.
            {
                config_value_primary = atoi(scline->tp[2]);
            }
            else
            {
                config_value_primary = get_id(object_desc, scline->tp[2]);
            }
        }
        else if ((creatvar == 35) || (creatvar == 36)) // PRISONKIND or TORTUREKIND
        {
            if (parameter_is_number(scline->tp[2])) // Support name or number for prison kind or torture kind.
            {
                config_value_primary = atoi(scline->tp[2]);
            }
            else
            {
                config_value_primary = get_id(creature_desc, scline->tp[2]);
            }
        }
        else if (creatvar == 37) // SPELLIMMUNITY
        {
            if (parameter_is_number(scline->tp[2]))
            {
                config_value_primary = atoi(scline->tp[2]);
            }
            else
            {
                config_value_primary = get_id(spell_effect_flags, scline->tp[2]);
            }
            if (config_value_primary < 0)
            {
                SCRPTERRLOG("SpellImmunity flag %s is out of range or doesn't exist.", scline->tp[2]);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            // value 2: 'empty' is 'set', '1' is 'add', '0' is 'clear'.
            if (scline->tp[3][0] != '\0')
            {
                config_value_secondary = atoi(scline->tp[3]);
            }
            else
            {
                // tp[3] is empty, set it to UCHAR_MAX to process.
                config_value_secondary = UCHAR_MAX;
            }
        }
        else if (creatvar == 38) // HOSTILETOWARDS
        {
            if (parameter_is_number(scline->tp[2])) // Support name or number for hostile towards.
            {
                config_value_primary = atoi(scline->tp[2]);
            }
            else if (0 == strcmp(scline->tp[2], "ANY_CREATURE")) // Support ANY_CREATURE for hostile towards.
            {
                config_value_primary = CREATURE_ANY;
            }
            else if (strcasecmp(scline->tp[2], "NULL") == 0)  // Support NULL for hostile towards.
            {
                config_value_primary = 0;
            }
            else
            {
                config_value_primary = get_id(creature_desc, scline->tp[2]);
            }
        }
        else
        {
            config_value_primary = atoi(scline->tp[2]);
            if (scline->tp[3][0] != '\0')
            {
                config_value_secondary = atoi(scline->tp[3]);
            }
            // nothing there that would need the third value.
        }
    }
    else if (block == CrtConf_JOBS)
    {
        if ((creatvar > 0) && (creatvar <= 4)) // Jobs
        {
            if (parameter_is_number(scline->tp[2]))
            {
                config_value_primary = atoi(scline->tp[2]);
                if ((config_value_primary < 0) || (config_value_primary > SHRT_MAX))
                {
                    SCRPTERRLOG("Job value %ld out of range `0~%d`.", config_value_primary, SHRT_MAX);
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
            }
            else
            {
                config_value_primary = get_id(creaturejob_desc, scline->tp[2]);
                if (config_value_primary > SHRT_MAX)
                {
                    SCRPTERRLOG("Job %s not supported", creature_job_code_name(config_value_primary));
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
                else if (config_value_primary < 0)
                {
                    SCRPTERRLOG("Job %s is out of range or doesn't exist.", scline->tp[2]);
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
            }
            // value 2: 'empty' is 'set', '1' is 'add', '0' is 'clear'.
            if (scline->tp[3][0] != '\0')
            {
                config_value_secondary = atoi(scline->tp[3]);
            }
            else
            {
                // tp[3] is empty, set it to UCHAR_MAX to process.
                config_value_secondary = UCHAR_MAX;
            }
        }
        else
        {
            config_value_primary = atoi(scline->tp[2]);
            // Nothing there that would need the second or third value.
        }
    }
    else if (block == CrtConf_SOUNDS)
    {
        config_value_primary = atoi(scline->tp[2]);
        if (scline->tp[3][0] != '\0')
        {
            config_value_secondary = atoi(scline->tp[3]);
        }
        if (scline->tp[3][0] != '\0')
        {
            config_value_tertiary = atoi(scline->tp[4]);
        }
    }
    else if (block == CrtConf_SPRITES)
    {
        if ((creatvar == (CGI_HandSymbol + 1)) || (creatvar == (CGI_QuerySymbol + 1)))
        {
            config_value_primary = get_icon_id(scline->tp[2]);
        }
        else
        {
            config_value_primary = get_anim_id_(scline->tp[2]);
        }
    }
    else if (block == CrtConf_ATTRACTION)
    {
        if (creatvar == 1) //ENTRANCEROOM
        {
            config_value_primary = get_id(room_desc, scline->tp[2]);
            if (scline->tp[3][0] != '\0')
            {
                config_value_secondary = get_id(room_desc, scline->tp[3]);
            }
            if (scline->tp[4][0] != '\0')
            {
                config_value_tertiary = get_id(room_desc, scline->tp[4]);
            }
        }
        else
        {
            config_value_primary = atoi(scline->tp[2]);
            if (scline->tp[3][0] != '\0')
            {
                config_value_secondary = atoi(scline->tp[3]);
            }
            if (scline->tp[4][0] != '\0')
            {
                config_value_tertiary = atoi(scline->tp[4]);
            }
        }
    }
    else if (block == CrtConf_ANNOYANCE)
    {
        if (creatvar == 21) //LairEnemy
        {
            ThingModel creature_model[3];
            for (int j = 0; j < 2; j++)
            {
                //Only needs one enemy, but can do up to 3
                if ((j > 0) && (scline->tp[j + 2][0] == '\0'))
                    break;

                if (parameter_is_number(scline->tp[j + 2]))
                {
                    creature_model[j] = atoi(scline->tp[j + 2]);
                    if (creature_model[j] > CREATURE_TYPES_MAX)
                    {
                        SCRPTERRLOG("Value %d out of range.", atoi(scline->tp[j + 2]));
                        DEALLOCATE_SCRIPT_VALUE
                        return;
                    }
                }
                else
                {
                    creature_model[j] = parse_creature_name(scline->tp[j + 2]);
                    if (creature_model[j] < 0)
                    {
                        if (0 == strcmp(scline->tp[j + 2], "ANY_CREATURE"))
                        {
                            creature_model[j] = CREATURE_ANY;
                        }
                        else if (strcasecmp(scline->tp[j + 2], "NULL") == 0)
                        {
                            creature_model[j] = 0;
                        }
                        if (creature_model[j] < 0)
                        {
                            SCRPTERRLOG("Invalid creature model %s", scline->tp[j + 2]);
                            DEALLOCATE_SCRIPT_VALUE
                            return;
                        }
                    }
                }
            }
            config_value_primary = creature_model[0];
            config_value_secondary = creature_model[1];
            config_value_tertiary = creature_model[2];
        } else
        if (creatvar == 23) // AngerJobs
        {
            if (parameter_is_number(scline->tp[2]))
            {
                config_value_primary = atoi(scline->tp[2]);
                if ((config_value_primary < 0) || (config_value_primary > SHRT_MAX))
                {
                    SCRPTERRLOG("Job value %ld out of range `0~%d`.", config_value_primary, SHRT_MAX);
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
            }
            else
            {
                config_value_primary = get_id(angerjob_desc, scline->tp[2]);
                if (config_value_primary > SHRT_MAX)
                {
                    SCRPTERRLOG("Job %s not supported", creature_job_code_name(config_value_primary));
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
                else if (config_value_primary < 0)
                {
                    SCRPTERRLOG("Job %s is out of range or doesn't exist.", scline->tp[2]);
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
            }
            // value 2: 'empty' is 'set', '1' is 'add', '0' is 'clear'.
            if (scline->tp[3][0] != '\0')
            {
                config_value_secondary = atoi(scline->tp[3]);
            }
            else
            {
                // tp[3] is empty, set it to UCHAR_MAX to process.
                config_value_secondary = UCHAR_MAX;
            }
        }
        else
        {
            config_value_primary = atoi(scline->tp[2]);
            config_value_secondary = atoi(scline->tp[3]);
        }
    } else
    if (block == CrtConf_EXPERIENCE)
    {
        if (creatvar == 1) // POWERS
        {
            long instance = 0;
            if (!parameter_is_number(scline->tp[2]))
            {
                instance = get_id(instance_desc, scline->tp[2]);

            }
            else
            {
                instance = atoi(scline->tp[2]);
            }
            if (instance >= 0)
            {
                config_value_primary = instance;
            }
            else
            {
                SCRPTERRLOG("Unknown instance %s ", scline->tp[2]);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            if ((atoi(scline->tp[3]) >= CREATURE_MAX_LEVEL) || (atoi(scline->tp[3]) <= 0)) //Powers
            {
                SCRPTERRLOG("Value %d out of range, only %d slots for Powers.", atoi(scline->tp[3]), CREATURE_MAX_LEVEL - 1);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            config_value_secondary = atoi(scline->tp[3]);
        } else
        if (creatvar == 2) // POWERSLEVELREQUIRED
        {
            if ((atoi(scline->tp[2]) <= 0) || (atoi(scline->tp[2]) > CREATURE_MAX_LEVEL)) //value
            {
                SCRPTERRLOG("Value %d out of range, only %d levels for PowersLevelRequired supported", atoi(scline->tp[2]), CREATURE_MAX_LEVEL);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            if ((atoi(scline->tp[3]) > CREATURE_MAX_LEVEL) || (atoi(scline->tp[3]) <= 0)) //slot
            {
                SCRPTERRLOG("Value %d out of range, only %d levels for PowersLevelRequired supported", atoi(scline->tp[3]), CREATURE_MAX_LEVEL);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            config_value_primary = atoi(scline->tp[2]);
            config_value_secondary = atoi(scline->tp[3]);
        } else
        if (creatvar == 3) // LEVELSTRAINVALUES
        {
            if (atoi(scline->tp[2]) < 0) //value
            {
                SCRPTERRLOG("Value %d out of range.", atoi(scline->tp[2]));
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
            if ((atoi(scline->tp[3]) <= 0) || (atoi(scline->tp[3]) > CREATURE_MAX_LEVEL)) //slot
            {
                SCRPTERRLOG("Value %d out of range, only %d levels for LevelsTrainValues supported", atoi(scline->tp[3]), CREATURE_MAX_LEVEL - 1);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            config_value_primary = atoi(scline->tp[2]);
            config_value_secondary = atoi(scline->tp[3]);
        } else
        if (creatvar == 4) // GROWUP
        {
            config_value_primary = atoi(scline->tp[2]);
            ThingModel creature_model = 0;
            if (parameter_is_number(scline->tp[3]))
            {
                creature_model = atoi(scline->tp[3]);
                if (creature_model > CREATURE_TYPES_MAX)
                {
                    SCRPTERRLOG("Value %d out of range.", atoi(scline->tp[3]));
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
            }
            else
            {
                creature_model = parse_creature_name(scline->tp[3]);
                if (creature_model <  0)
                {
                    if (strcasecmp(scline->tp[3], "NULL") == 0)
                    {
                        creature_model = 0;
                    }
                    if (creature_model < 0)
                    {
                        SCRPTERRLOG("Invalid creature model %s", scline->tp[3]);
                        DEALLOCATE_SCRIPT_VALUE
                        return;
                    }
                }
            }
            config_value_secondary = creature_model;
            short level = 0;
            if (config_value_secondary > 0)
            {
                level = atoi(scline->tp[4]);
                if ((level < 1) || (level > CREATURE_MAX_LEVEL))
                {
                    SCRPTERRLOG("Value %d out of range.", atoi(scline->tp[4]));
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
            }
            config_value_tertiary = level;
        } else
        if (creatvar == 5) // SLEEPEXPERIENCE
        {
            long slabtype = get_id(slab_desc, scline->tp[2]);
            if (slabtype < 0)
            {
                SCRPTERRLOG("Unknown slab type %s.", scline->tp[2]);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            else
            {
                config_value_primary = slabtype;
            }
            config_value_secondary = atoi(scline->tp[3]);
            if (config_value_secondary < 0)
            {
                SCRPTERRLOG("Slab sleep experience value (%s %ld) must be 0 or greater.", scline->tp[2], config_value_secondary);
                config_value_secondary = 0;
            }
            slabtype = (scline->tp[4][0] != '\0') ? get_id(slab_desc, scline->tp[4]) : 0;
            if (slabtype < 0)
            {
                if (scline->tp[4][0] != '\0')
                {
                    SCRPTERRLOG("Unknown slab type %s.", scline->tp[4]);
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
            }
            else
            {
                config_value_tertiary = slabtype;
            }
            config_value_quaternary = atoi(scline->tp[5]);
            if (config_value_quaternary < 0)
            {
                SCRPTERRLOG("Slab sleep experience value (%s %ld) must be 0 or greater.", scline->tp[5], config_value_quaternary);
                config_value_quaternary = 0;
            }
            slabtype = (scline->tp[6][0] != '\0') ? get_id(slab_desc, scline->tp[6]) : 0;
            if (slabtype < 0)
            {
                if (scline->tp[6][0] != '\0')
                {
                    SCRPTERRLOG("Unknown slab type %s.", scline->tp[6]);
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
            }
            else
            {
                config_value_quinary = slabtype;
            }
            config_value_senary = atoi(scline->tp[7]);
            if (config_value_senary < 0)
            {
                SCRPTERRLOG("Slab sleep experience value (%s %ld) must be 0 or greater.", scline->tp[7], config_value_senary);
                config_value_senary = 0;
            }
        }
        else
        {
            config_value_primary = atoi(scline->tp[2]);
        }
    } else
    if (block == CrtConf_APPEARANCE)
    {
        if (creatvar == 4) // NATURALDEATHKIND
        {
            config_value_primary = get_id(creature_deathkind_desc, scline->tp[2]);
        }
        else
        {
            config_value_primary = atoi(scline->tp[2]);
            config_value_secondary = atoi(scline->tp[3]);
            config_value_tertiary = atoi(scline->tp[4]);
        }
    } else
    if (block == CrtConf_SENSES)
    {
        if (creatvar == 4) // EYEEFFECT
        {
            config_value_primary = get_id(lenses_desc, scline->tp[2]);
        }
        else
        {
            config_value_primary = atoi(scline->tp[2]);
            // nothing to fill for config_value_secondary or config_value_tertiary
        }
    }

    if (config_value_primary == -1)
    {
        SCRPTERRLOG("Unknown creature configuration value %s", scline->tp[2]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (config_value_secondary == -1)
    {
        SCRPTERRLOG("Unknown second creature configuration value %s", scline->tp[3]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (config_value_tertiary == -1)
    {
        SCRPTERRLOG("Unknown third creature configuration value %s", scline->tp[3]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[0] = scline->np[0];
    value->shorts[1] = creatvar;
    value->shorts[2] = block;
    value->longs[2] = config_value_primary;
    value->longs[3] = config_value_secondary;
    value->longs[4] = config_value_tertiary;
    value->longs[5] = config_value_quaternary;
    value->longs[6] = config_value_quinary;
    value->longs[7] = config_value_senary;

    SCRIPTDBG(7,"Setting creature %s configuration value %d:%d to %d (%d)", creature_code_name(value->shorts[0]), value->shorts[4], value->shorts[1], value->shorts[2], value->shorts[3]);

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_creature_configuration_process(struct ScriptContext* context)
{
    short creatid = context->value->shorts[0];
    struct CreatureModelConfig* crconf = creature_stats_get(creatid);

    short creature_variable = context->value->shorts[1];
    short block  = context->value->shorts[2];
    long value  = context->value->longs[2];
    long config_value_secondary = context->value->longs[3];
    long config_value_tertiary = context->value->longs[4];
    long config_value_quaternary = context->value->longs[5];
    long config_value_quinary = context->value->longs[6];
    long config_value_senary = context->value->longs[7];

    if (block == CrtConf_ATTRIBUTES)
    {
        switch (creature_variable)
        {
        case 1: // NAME
            CONFWRNLOG("Attribute (%d) not supported", creature_variable);
            break;
        case 2: // HEALTH
            if (crconf->health != value)
            {
                crconf->health = value;
                for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
                {
                    do_to_players_all_creatures_of_model(plyr_idx, creatid, update_relative_creature_health);
                }
            }
            break;
        case 3: // HEALREQUIREMENT
            crconf->heal_requirement = value;
            break;
        case 4: // HEALTHRESHOLD
            crconf->heal_threshold = value;
            break;
        case 5: // STRENGTH
            crconf->strength = value;
            break;
        case 6: // ARMOUR
            crconf->armour = value;
            break;
        case 7: // DEXTERITY
            crconf->dexterity = value;
            break;
        case 8: // FEARWOUNDED
            crconf->fear_wounded = value;
            break;
        case 9: // FEARSTRONGER
            crconf->fear_stronger = value;
            break;
        case 10: // DEFENCE
            crconf->defense = value;
            break;
        case 11: // LUCK
            crconf->luck = value;
            break;
        case 12: // RECOVERY
            crconf->sleep_recovery = value;
            break;
        case 13: // HUNGERRATE
            crconf->hunger_rate = value;
            break;
        case 14: // HUNGERFILL
            crconf->hunger_fill = value;
            break;
        case 15: // LAIRSIZE
            crconf->lair_size = value;
            break;
        case 16: // HURTBYLAVA
            crconf->hurt_by_lava = value;
            break;
        case 17: // BASESPEED
            if (crconf->base_speed != value)
            {
                crconf->base_speed = value;
                for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
                {
                    update_speed_of_player_creatures_of_model(plyr_idx, creatid);
                }
            }
            break;
        case 18: // GOLDHOLD
            crconf->gold_hold = value;
            break;
        case 19: // SIZE
            crconf->size_xy = value;
            crconf->size_z = config_value_secondary;
            break;
        case 20: // ATTACKPREFERENCE
            crconf->attack_preference = value;
            break;
        case 21: // PAY
            crconf->pay = value;
            break;
        case 22: // HEROVSKEEPERCOST
            break;
        case 23: // SLAPSTOKILL
            crconf->slaps_to_kill = value;
            break;
        case 24: // CREATURELOYALTY
        case 25: // LOYALTYLEVEL
        case 28: // PROPERTIES
            CONFWRNLOG("Attribute (%d) not supported", creature_variable);
            break;
        case 26: // DAMAGETOBOULDER
            crconf->damage_to_boulder = value;
            break;
        case 27: // THINGSIZE
            crconf->thing_size_xy = value;
            crconf->thing_size_z = config_value_secondary;
            break;
        case 29: // NAMETEXTID
            crconf->namestr_idx = value;
            break;
        case 30: // FEARSOMEFACTOR
            crconf->fearsome_factor = value;
            break;
        case 31: // TOKINGRECOVERY
            crconf->toking_recovery = value;
            break;
        case 32: // CORPSEVANISHEFFECT
            crconf->corpse_vanish_effect = value;
            break;
        case 33: // FOOTSTEPPITCH
            crconf->footstep_pitch = value;
            break;
        case 34: // LAIROBJECT
            if (crconf->lair_object != value)
            {
                for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
                {
                    do_to_players_all_creatures_of_model(plyr_idx, creatid, remove_creature_lair);
                }
                crconf->lair_object = value;
            }
            break;
        case 35: // PRISONKIND
            crconf->prison_kind = value;
            break;
        case 36: // TORTUREKIND
            crconf->torture_kind = value;
            break;
        case 37: // SPELLIMMUNITY
            if (config_value_secondary == 0)
            {
                clear_flag(crconf->immunity_flags, value);
            }
            else if (config_value_secondary == 1)
            {
                set_flag(crconf->immunity_flags, value);
            }
            else
            {
                crconf->immunity_flags = value;
            }
            break;
        case 38: // HOSTILETOWARDS
            // Assume the mapmaker wants to reset it.
            for (int i = 0; i < CREATURE_TYPES_MAX; i++)
            {
                crconf->hostile_towards[i] = 0;
            }
            if (value != 0)
            {
                crconf->hostile_towards[0] = value; // Then apply the change on the first only.
            }
            break;
        case ccr_comment:
            break;
        case ccr_endOfFile:
            break;
        default:
            CONFWRNLOG("Unrecognized attribute (%d)", creature_variable);
            break;
        }
    }
    else if (block == CrtConf_JOBS)
    {
        switch (creature_variable)
        {
        case 1: // PRIMARYJOBS
            if (config_value_secondary == 0)
            {
                clear_flag(crconf->job_primary, value);
            }
            else if (config_value_secondary == 1)
            {
                set_flag(crconf->job_primary, value);
            }
            else
            {
                crconf->job_primary = value;
            }
            break;
        case 2: // SECONDARYJOBS
            if (config_value_secondary == 0)
            {
                clear_flag(crconf->job_secondary, value);
            }
            else if (config_value_secondary == 1)
            {
                set_flag(crconf->job_secondary, value);
            }
            else
            {
                crconf->job_secondary = value;
            }
            break;
        case 3: // NOTDOJOBS
            if (config_value_secondary == 0)
            {
                clear_flag(crconf->jobs_not_do, value);
            }
            else if (config_value_secondary == 1)
            {
                set_flag(crconf->jobs_not_do, value);
            }
            else
            {
                crconf->jobs_not_do = value;
            }
            break;
        case 4: // STRESSFULJOBS
            if (config_value_secondary == 0)
            {
                clear_flag(crconf->job_stress, value);
            }
            else if (config_value_secondary == 1)
            {
                set_flag(crconf->job_stress, value);
            }
            else
            {
                crconf->job_stress = value;
            }
            break;
        case 5: // TRAININGVALUE
            crconf->training_value = value;
            break;
        case 6: // TRAININGCOST
            crconf->training_cost = value;
            break;
        case 7: // SCAVENGEVALUE
            crconf->scavenge_value = value;
            break;
        case 8: // SCAVENGERCOST
            crconf->scavenger_cost = value;
            break;
        case 9: // RESEARCHVALUE
            crconf->research_value = value;
            break;
        case 10: // MANUFACTUREVALUE
            crconf->manufacture_value = value;
            break;
        case 11: // PARTNERTRAINING
            crconf->partner_training = value;
            break;
        default:
            CONFWRNLOG("Unrecognized Job command (%d)", creature_variable);
            break;
        }
    }
    else if (block == CrtConf_ATTRACTION)
    {

        switch (creature_variable)
        {
        case 1: // ENTRANCEROOM
            crconf->entrance_rooms[0] = value;
            crconf->entrance_rooms[1] = config_value_secondary;
            crconf->entrance_rooms[2] = config_value_tertiary;
            break;
        case 2: // ROOMSLABSREQUIRED
            crconf->entrance_slabs_req[0] = value;
            crconf->entrance_slabs_req[1] = config_value_secondary;
            crconf->entrance_slabs_req[2] = config_value_tertiary;
            break;
        case 3: // BASEENTRANCESCORE
            crconf->entrance_score = value;
            break;
        case 4: // SCAVENGEREQUIREMENT
            crconf->scavenge_require = value;
            break;
        case 5: // TORTURETIME
            crconf->torture_break_time = value;
            break;
        default:
            CONFWRNLOG("Unrecognized Attraction command (%d)", creature_variable);
            break;
        }
    }
    else if (block == CrtConf_SOUNDS)
    {
        switch (creature_variable)
        {
        case 1: // HURT
            game.conf.crtr_conf.creature_sounds[creatid].hurt.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].hurt.count = config_value_secondary;
            break;
        case 2: // HIT
            game.conf.crtr_conf.creature_sounds[creatid].hit.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].hit.count = config_value_secondary;
            break;
        case 3: // HAPPY
            game.conf.crtr_conf.creature_sounds[creatid].happy.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].happy.count = config_value_secondary;
            break;
        case 4: // SAD
            game.conf.crtr_conf.creature_sounds[creatid].sad.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].sad.count = config_value_secondary;
            break;
        case 5: // HANG
            game.conf.crtr_conf.creature_sounds[creatid].hang.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].hang.count = config_value_secondary;
            break;
        case 6: // DROP
            game.conf.crtr_conf.creature_sounds[creatid].drop.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].drop.count = config_value_secondary;
            break;
        case 7: // TORTURE
            game.conf.crtr_conf.creature_sounds[creatid].torture.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].torture.count = config_value_secondary;
            break;
        case 8: // SLAP
            game.conf.crtr_conf.creature_sounds[creatid].slap.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].slap.count = config_value_secondary;
            break;
        case 9: // DIE
            game.conf.crtr_conf.creature_sounds[creatid].die.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].die.count = config_value_secondary;
            break;
        case 10: // FOOT
            game.conf.crtr_conf.creature_sounds[creatid].foot.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].foot.count = config_value_secondary;
            break;
        case 11: // FIGHT
            game.conf.crtr_conf.creature_sounds[creatid].fight.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].fight.count = config_value_secondary;
            break;
        case 12: // PISS
            game.conf.crtr_conf.creature_sounds[creatid].piss.index = value;
            game.conf.crtr_conf.creature_sounds[creatid].piss.count = config_value_secondary;
            break;
        default:
            CONFWRNLOG("Unrecognized Spound command (%d)", creature_variable);
            break;
        }
    }
    else if (block == CrtConf_SPRITES)
    {
        set_creature_model_graphics(creatid, creature_variable-1, value);
    }
    else if (block == CrtConf_ANNOYANCE)
    {
        switch (creature_variable)
        {
        case 1: // EATFOOD
        {
            crconf->annoy_eat_food = value;
            break;
        }
        case 2: // WILLNOTDOJOB
        {
            crconf->annoy_will_not_do_job = value;
            break;
        }
        case 3: // INHAND
        {
            crconf->annoy_in_hand = value;
            break;
        }
        case 4: // NOLAIR
        {
            crconf->annoy_no_lair = value;
            break;
        }
        case 5: // NOHATCHERY
        {
            crconf->annoy_no_hatchery = value;
            break;
        }
        case 6: // WOKENUP
        {
            crconf->annoy_woken_up = value;
            break;
        }
        case 7: // STANDINGONDEADENEMY
        {
            crconf->annoy_on_dead_enemy = value;
            break;
        }
        case 8: // SULKING
        {
            crconf->annoy_sulking = value;
            break;
        }
        case 9: // NOSALARY
        {
            crconf->annoy_no_salary = value;
            break;
        }
        case 10: // SLAPPED
        {
            crconf->annoy_slapped = value;
            break;
        }
        case 11: // STANDINGONDEADFRIEND
        {
            crconf->annoy_on_dead_friend = value;
            break;
        }
        case 12: // INTORTURE
        {
            crconf->annoy_in_torture = value;
            break;
        }
        case 13: // INTEMPLE
        {
            crconf->annoy_in_temple = value;
            break;
        }
        case 14: // SLEEPING
        {
            crconf->annoy_sleeping = value;
            break;
        }
        case 15: // GOTWAGE
        {
            crconf->annoy_got_wage = value;
            break;
        }
        case 16: // WINBATTLE
        {
            crconf->annoy_win_battle = value;
            break;
        }
        case 17: // UNTRAINED
        {
            crconf->annoy_untrained_time = value;
            crconf->annoy_untrained = config_value_secondary;
            break;
        }
        case 18: // OTHERSLEAVING
        {
            crconf->annoy_others_leaving = value;
            break;
        }
        case 19: // JOBSTRESS
        {
            crconf->annoy_job_stress = value;
            break;
        }
        case 20: // QUEUE
        {
            crconf->annoy_queue = value;
            break;
        }
        case 21: // LAIRENEMY
        {
            crconf->lair_enemy[0] = value;
            crconf->lair_enemy[1] = config_value_secondary;
            crconf->lair_enemy[2] = config_value_tertiary;
            //clear out the other ones.
            crconf->lair_enemy[3] = 0;
            crconf->lair_enemy[4] = 0;
            break;
        }
        case 22: // ANNOYLEVEL
        {
            crconf->annoy_level = value;
            break;
        }
        case 23: // ANGERJOBS
        {
            if (config_value_secondary == 0)
            {
                clear_flag(crconf->jobs_anger, value);
            }
            else if (config_value_secondary == 1)
            {
                set_flag(crconf->jobs_anger, value);
            }
            else
            {
                crconf->jobs_anger = value;
            }
            break;
        }
        case 24: // GOINGPOSTAL
        {
            crconf->annoy_going_postal = value;
            break;
        }
        default:
            CONFWRNLOG("Unrecognized Annoyance command (%d)", creature_variable);
            break;
        }
    }
    else if (block == CrtConf_EXPERIENCE)
    {
        switch (creature_variable)
        {
        case 1: // POWERS
        {
            crconf->learned_instance_id[config_value_secondary-1] = value;
            break;
        }
        case 2: // POWERSLEVELREQUIRED
        {
            crconf->learned_instance_level[config_value_secondary-1] = value;
            break;
        }
        case 3: // LEVELSTRAINVALUES
        {
            crconf->to_level[config_value_secondary-1] = value;
            break;
        }
        case 4: // GROWUP
        {
            crconf->to_level[CREATURE_MAX_LEVEL - 1] = value;
            crconf->grow_up = config_value_secondary;
            crconf->grow_up_level = config_value_tertiary;
            break;
        }
        case 5: // SLEEPEXPERIENCE
        {
            crconf->sleep_exp_slab[0] = value;
            crconf->sleep_experience[0] = config_value_secondary;
            crconf->sleep_exp_slab[1] = config_value_tertiary;
            crconf->sleep_experience[1] = config_value_quaternary;
            crconf->sleep_exp_slab[2] = config_value_quinary;
            crconf->sleep_experience[2] = config_value_senary;
            break;
        }
        case 6: // EXPERIENCEFORHITTING
        {
            crconf->exp_for_hitting = value;
            break;
        }
        case 7: // REBIRTH
        {
            crconf->rebirth = value;
            break;
        }
        default:
            CONFWRNLOG("Unrecognized Experience command (%d)", creature_variable);
            break;
        }
    }
    else if (block == CrtConf_APPEARANCE)
    {
        switch (creature_variable)
        {
        case 1: // WALKINGANIMSPEED
        {
            crconf->walking_anim_speed = value;
            break;
        }
        case 2: // VISUALRANGE
        {
            crconf->visual_range = value;
            break;
        }
        case 3: // SWIPEINDEX
        {
            crconf->swipe_idx = value;
            break;
        }
        case 4: // NATURALDEATHKIND
        {
            crconf->natural_death_kind = value;
            break;
        }
        case 5: // SHOTORIGIN
        {
            crconf->shot_shift_x = value;
            crconf->shot_shift_y = config_value_secondary;
            crconf->shot_shift_z = config_value_tertiary;
            break;
        }
        case 6: // CORPSEVANISHEFFECT
        {
            crconf->corpse_vanish_effect = value;
            break;
        }
        case 7: // FOOTSTEPPITCH
        {
            crconf->footstep_pitch = value;
            break;
        }
        case 8: // PICKUPOFFSET
        {
            crconf->creature_picked_up_offset.delta_x = value;
            crconf->creature_picked_up_offset.delta_y = config_value_secondary;
            break;
        }
        case 9: // STATUSOFFSET
        {
            crconf->status_offset = value;
            break;
        }
        case 10: // TRANSPARENCYFLAGS
        {
            crconf->transparency_flags = value<<4;
            break;
        }
        case 11: // FIXEDANIMSPEED
        {
            crconf->fixed_anim_speed = value;
            break;
        }
        default:
            CONFWRNLOG("Unrecognized Appearance command (%d)", creature_variable);
            break;
        }
    }
    else if (block == CrtConf_SENSES)
    {
        switch (creature_variable)
        {
        case 1: // HEARING
        {
            crconf->hearing = value;
            break;
        }
        case 2: // EYEHEIGHT
        {
            crconf->base_eye_height = value;
            break;
        }
        case 3: // FIELDOFVIEW
        {
            crconf->field_of_view = value;
            break;
        }
        case 4: // EYEEFFECT
        {
            crconf->eye_effect = value;
            struct Thing* thing = thing_get(get_my_player()->influenced_thing_idx);
            if(thing_exists(thing))
            {
                if (thing->model == creatid)
                {
                    struct LensConfig* lenscfg = get_lens_config(value);
                    initialise_eye_lenses();
                    if (flag_is_set(lenscfg->flags, LCF_HasPalette))
                    {
                        PaletteSetPlayerPalette(get_my_player(), lenscfg->palette);
                    }
                    else
                    {
                        PaletteSetPlayerPalette(get_my_player(), engine_palette);
                    }
                    setup_eye_lens(value);
                }
            }
            break;
        }
        case 5: // MAXANGLECHANGE
        {
            crconf->max_turning_speed = (value * DEGREES_180) / 180;
            break;
        }
        default:
            CONFWRNLOG("Unrecognized Senses command (%d)", creature_variable);
            break;
        }
    }
    else
    {
        ERRORLOG("Trying to configure unsupported creature block (%d)",block);
    }
    check_and_auto_fix_stats();
}

static void set_object_configuration_process(struct ScriptContext *context)
{
    set_config_process(&objects_named_fields_set, context,"SET_OBJECT_CONFIGURATION");

    ThingModel model = context->value->shorts[0];
    update_all_objects_of_model(model);
}

static void display_timer_check(const struct ScriptLine *scline)
{
    const char *timrname = scline->tp[1];
    char timr_id = get_rid(timer_desc, timrname);
    if (timr_id == -1)
    {
        SCRPTERRLOG("Unknown timer, '%s'", timrname);
        return;
    }
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    value->bytes[1] = timr_id;
    value->longs[1] = 0;
    value->bytes[2] = (TbBool)scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void display_timer_process(struct ScriptContext *context)
{
    game.script_timer_player = context->player_idx;
    game.script_timer_id = context->value->bytes[1];
    game.script_timer_limit = context->value->longs[1];
    game.timer_real = context->value->bytes[2];
    game.flags_gui |= GGUI_ScriptTimer;
}

static void add_to_timer_check(const struct ScriptLine *scline)
{
    const char *timrname = scline->tp[1];
    long timr_id = get_rid(timer_desc, timrname);
    if (timr_id == -1)
    {
        SCRPTERRLOG("Unknown timer, '%s'", timrname);
        return;
    }
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    value->longs[1] = timr_id;
    value->longs[2] = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_to_timer_process(struct ScriptContext *context)
{
   add_to_script_timer(context->player_idx, context->value->longs[1], context->value->longs[2]);
}

static void add_bonus_time_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->longs[0] = scline->np[0];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_bonus_time_process(struct ScriptContext *context)
{
   game.bonus_time += context->value->longs[0];
}

static void display_variable_check(const struct ScriptLine *scline)
{
    int32_t varib_id, varib_type;
    if (!parse_get_varib(scline->tp[1], &varib_id, &varib_type, level_file_version))
    {
        SCRPTERRLOG("Unknown variable, '%s'", scline->tp[1]);
        return;
    }
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    value->bytes[1] = scline->np[3];
    value->bytes[2] = varib_type;
    value->longs[1] = varib_id;
    value->longs[2] = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void display_variable_process(struct ScriptContext *context)
{
   game.script_variable_player = context->player_idx;
   game.script_value_type = context->value->bytes[2];
   game.script_value_id = context->value->longs[1];
   game.script_variable_target = context->value->longs[2];
   game.script_variable_target_type = context->value->bytes[1];
   game.flags_gui |= GGUI_Variable;
}

static void display_countdown_check(const struct ScriptLine *scline)
{
    if (scline->np[2] <= 0)
    {
        SCRPTERRLOG("Can't have a countdown to %ld turns.", scline->np[2]);
        return;
    }
    const char *timrname = scline->tp[1];
    char timr_id = get_rid(timer_desc, timrname);
    if (timr_id == -1)
    {
        SCRPTERRLOG("Unknown timer, '%s'", timrname);
        return;
    }
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    value->bytes[1] = timr_id;
    value->longs[1] = scline->np[2];
    value->bytes[2] = (TbBool)scline->np[3];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void cmd_no_param_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void hide_timer_process(struct ScriptContext *context)
{
   game.flags_gui &= ~GGUI_ScriptTimer;
}

static void hide_variable_process(struct ScriptContext *context)
{
   game.flags_gui &= ~GGUI_Variable;
}

static void create_effect_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    TbMapLocation location;
    const char *effect_name = scline->tp[0];
    long effct_id = effect_or_effect_element_id(effect_name);
    if (effct_id == 0)
    {
        SCRPTERRLOG("Unrecognised effect: %s", effect_name);
        return;
    }
    value->shorts[0] = effct_id;
    const char *locname = scline->tp[1];
    if (!get_map_location_id(locname, &location))
    {
        return;
    }
    value->ulongs[1] = location;
    value->longs[2] = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void create_effect_at_pos_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    const char *effect_name = scline->tp[0];
    long effct_id = effect_or_effect_element_id(effect_name);
    if (effct_id == 0)
    {
        SCRPTERRLOG("Unrecognised effect: %s", effect_name);
        return;
    }
    value->shorts[0] = effct_id;
    if (subtile_coords_invalid(scline->np[1], scline->np[2]))
    {
        SCRPTERRLOG("Invalid coordinates: %ld, %ld", scline->np[1], scline->np[2]);
        return;
    }
    value->shorts[1] = scline->np[1];
    value->shorts[2] = scline->np[2];
    value->longs[2] = scline->np[3];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void null_process(struct ScriptContext *context)
{
}



static void set_sacrifice_recipe_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    value->sac.action = get_rid(rules_sacrifices_commands, scline->tp[0]);
    if (value->sac.action == -1)
    {
        SCRPTERRLOG("Unexpected action:%s", scline->tp[0]);
        return;
    }
    long param;
    if ((value->sac.action == SacA_CustomPunish) || (value->sac.action == SacA_CustomReward))
    {
        param = get_id(flag_desc, scline->tp[1]) + 1;
    }
    else
    {
        param = get_id(creature_desc, scline->tp[1]);
        if (param == -1)
        {
            param = get_id(sacrifice_unique_desc, scline->tp[1]);
        }
        if (param == -1)
        {
            param = get_id(spell_desc, scline->tp[1]);
        }
    }
    if (param == -1 && (strcmp(scline->tp[1], "NONE") == 0))
    {
        param = 0;
    }

    if (param < 0)
    {
        param = 0;
        value->sac.action = SacA_None;
        SCRPTERRLOG("Unexpected parameter:%s", scline->tp[1]);
    }
    value->sac.param = param;

    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
       long vi = get_rid(creature_desc, scline->tp[i + 2]);
       if (vi < 0)
         vi = 0;
       value->sac.victims[i] = vi;
    }

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void remove_sacrifice_recipe_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    value->sac.action = SacA_None;
    value->sac.param = 0;

    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
       long vi = get_rid(creature_desc, scline->tp[i]);
       if (vi < 0)
         vi = 0;
       value->sac.victims[i] = vi;
    }

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_sacrifice_recipe_process(struct ScriptContext *context)
{
    ThingModel victims[MAX_SACRIFICE_VICTIMS];
    int action = context->value->sac.action;
    int param = context->value->sac.param;

    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
        victims[i] = context->value->sac.victims[i];
    }

    script_set_sacrifice_recipe(action, param, victims);
}

static void set_box_tooltip_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if ((scline->np[0] < 0) || (scline->np[0] >= CUSTOM_BOX_COUNT))
    {
        SCRPTERRLOG("Invalid CUSTOM_BOX number (%ld)", scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE;
    }
    value->shorts[0] = scline->np[0];

    if (strlen(scline->tp[1]) >= MESSAGE_TEXT_LEN)
    {
        SCRPTWRNLOG("Tooltip TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN - 1);
    }
    value->longs[2] = script_strdup(scline->tp[1]);
    if (value->longs[2] < 0)
    {
        SCRPTERRLOG("Run out script strings space");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    PROCESS_SCRIPT_VALUE(scline->command);
}


static void set_box_tooltip_process(struct ScriptContext* context)
{
    int idx = context->value->shorts[0];
    snprintf(game.box_tooltip[idx], MESSAGE_TEXT_LEN, "%s", script_strval(context->value->longs[2]));
}

static void set_box_tooltip_id_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if ((scline->np[0] < 0) || (scline->np[0] >= CUSTOM_BOX_COUNT))
    {
        SCRPTERRLOG("Invalid CUSTOM_BOX number (%ld)", scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE;
        return;
    }
    value->shorts[0] = scline->np[0];
    value->shorts[1] = scline->np[1];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_box_tooltip_id_process(struct ScriptContext* context)
{
    int idx = context->value->shorts[0];
    int string = context->value->shorts[1];
    snprintf(game.box_tooltip[idx], MESSAGE_TEXT_LEN, "%s", get_string(string));
}

static void change_slab_owner_check(const struct ScriptLine *scline)
{

    if (scline->np[0] < 0 || scline->np[0] > game.map_tiles_x) //x coord
    {
        SCRPTERRLOG("Value '%ld' out of range. Range 0-%d allowed.", scline->np[0],game.map_tiles_x);
        return;
    }
    if (scline->np[1] < 0 || scline->np[1] > game.map_tiles_y) //y coord
    {
        SCRPTERRLOG("Value '%ld' out of range. Range 0-%d allowed.", scline->np[1],game.map_tiles_y);
        return;
    }
    long filltype = get_id(fill_desc, scline->tp[3]);
    if ((scline->tp[3][0] != '\0') && (filltype == -1))
    {
        SCRPTWRNLOG("Fill type %s not recognized", scline->tp[3]);
    }

    command_add_value(Cmd_CHANGE_SLAB_OWNER, scline->np[2], scline->np[0], scline->np[1], filltype);
}

static void change_slab_owner_process(struct ScriptContext *context)
{
    MapSlabCoord x = context->value->longs[0];
    MapSlabCoord y = context->value->longs[1];
    long fill_type = context->value->longs[2];
    if (fill_type > 0)
    {
        struct CompoundCoordFilterParam iter_param;
        iter_param.plyr_idx = context->player_idx;
        iter_param.primary_number = fill_type;
        iter_param.secondary_number = get_slabmap_block(x, y)->kind;
        slabs_fill_iterate_from_slab(x, y, slabs_change_owner, &iter_param);
    } else {
        change_slab_owner_from_script(x, y, context->player_idx);
    }
}

static void change_slab_type_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    if (scline->np[0] < 0 || scline->np[0] > game.map_tiles_x) //x coord
    {
        SCRPTERRLOG("Value '%ld' out of range. Range 0-%d allowed.", scline->np[0],game.map_tiles_x);
        return;
    }
    else
    {
        value->shorts[0] = scline->np[0];
    }

    if (scline->np[1] < 0 || scline->np[1] > game.map_tiles_y) //y coord
    {
        SCRPTERRLOG("Value '%ld' out of range. Range 0-%d allowed.", scline->np[0],game.map_tiles_y);
        return;
    }
    else
    {
        value->shorts[1] = scline->np[1];
    }

    if (scline->np[2] < 0 || scline->np[2] >= game.conf.slab_conf.slab_types_count) //slab kind
    {
        SCRPTERRLOG("Unsupported slab '%ld'. Slabs range 0-%d allowed.", scline->np[2],game.conf.slab_conf.slab_types_count-1);
        return;
    }
    else
    {
        value->shorts[2] = scline->np[2];
    }

    value->shorts[3] = get_id(fill_desc, scline->tp[3]);
    if ((scline->tp[3][0] != '\0') && (value->shorts[3] == -1))
    {
        SCRPTWRNLOG("Fill type %s not recognized", scline->tp[3]);
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void change_slab_type_process(struct ScriptContext *context)
{
    long x = context->value->shorts[0];
    long y = context->value->shorts[1];
    long slab_kind = context->value->shorts[2];
    long fill_type = context->value->shorts[3];

    if (fill_type > 0)
    {
        struct CompoundCoordFilterParam iter_param;
        iter_param.primary_number = slab_kind;
        iter_param.secondary_number = fill_type;
        iter_param.tertiary_number = get_slabmap_block(x, y)->kind;
        slabs_fill_iterate_from_slab(x, y, slabs_change_type, &iter_param);
    }
    else
    {
        replace_slab_from_script(x, y, slab_kind);
    }
}

static void reveal_map_location_check(const struct ScriptLine *scline)
{
    TbMapLocation location;
    if (!get_map_location_id(scline->tp[1], &location)) {
        return;
    }
    command_add_value(Cmd_REVEAL_MAP_LOCATION, scline->np[0], location, scline->np[2], 0);
}

static void reveal_map_location_process(struct ScriptContext *context)
{
    TbMapLocation target = context->value->longs[0];
    SYNCDBG(0, "Revealing location type %u", target);
    MapSubtlCoord x = 0;
    MapSubtlCoord y = 0;
    long r = context->value->longs[1];
    find_map_location_coords(target, &x, &y, context->player_idx, __func__);
    if ((x == 0) && (y == 0))
    {
        WARNLOG("Can't decode location %u", target);
        return;
    }
    if (r == -1)
    {
        struct CompoundCoordFilterParam iter_param;
        iter_param.plyr_idx = context->player_idx;
        slabs_fill_iterate_from_slab(subtile_slab(x), subtile_slab(y), slabs_reveal_slab_and_corners, &iter_param);
    } else
        reveal_map_area(context->player_idx, x-(r>>1), x+(r>>1)+(r&1), y-(r>>1), y+(r>>1)+(r&1));
}

static void player_zoom_to_check(const struct ScriptLine *scline)
{
    TbMapLocation location;
    const char *where = scline->tp[1];
    if (!get_map_location_id(where, &location) || location == MLoc_NONE) {
        SCRPTERRLOG("invalid zoom location \"%s\"",where);
        return;
    }

    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    value->longs[0] = location;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void player_zoom_to_process(struct ScriptContext *context)
{
    TbMapLocation target = context->value->longs[0];
    struct Coord3d pos;

    find_location_pos(target, context->player_idx, &pos, __func__);
    set_player_zoom_to_position(get_player(context->player_idx),&pos);
}

static void level_up_players_creatures_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    long crmodel = parse_creature_name(scline->tp[1]);
    char count = scline->np[2];

    if (crmodel == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (scline->np[2] == '\0')
    {
        count = 1;
    }
    if (count == 0)
    {
        SCRPTERRLOG("Trying to level up %ld times", scline->np[2]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[1] = crmodel;
    value->shorts[2] = count;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void level_up_players_creatures_process(struct ScriptContext* context)
{
    long crmodel = context->value->shorts[1];
    long count = context->value->shorts[2];
    PlayerNumber plyridx = context->player_idx;
    struct Dungeon* dungeon = get_players_num_dungeon(plyridx);
    unsigned long k = 0;

    TbBool need_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(dungeon->owner, crmodel);
    struct Thing* thing = INVALID_THING;
    int i;
    if ((!need_spec_digger) || (crmodel == CREATURE_ANY) || (crmodel == CREATURE_NOT_A_DIGGER))
    {
        i = dungeon->creatr_list_start;
    }
    else
    {
        i = dungeon->digger_list_start;
    }

    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (creature_matches_model(thing, crmodel))
        {
            creature_change_multiple_levels(thing, count);
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19, "Finished");
}

static void use_spell_on_creature_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    long crtr_id = parse_creature_name(scline->tp[1]);
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        return;
    }
    long select_id = parse_criteria(scline->tp[2]);
    if (select_id == -1) {
        SCRPTERRLOG("Unknown select criteria, '%s'", scline->tp[2]);
        return;
    }
    const char* mag_name = scline->tp[3];
    short mag_id = get_rid(spell_desc, mag_name);
    CrtrExpLevel spell_level = scline->np[4];
    if (mag_id == -1)
    {
        SCRPTERRLOG("Invalid spell: %s", mag_name);
        return;
    }
    struct SpellConfig* spconf = get_spell_config(mag_id);
    if (spconf->linked_power) // Only check for spells linked to a keeper power.
    {
        if (spell_level < 1)
        {
            SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", mag_name, spell_level);
            spell_level = 1;
        }
        if (spell_level > (MAGIC_OVERCHARGE_LEVELS + 1)) // Creatures cast spells from level 1 to 10.
        {
            SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", mag_name, spell_level, (MAGIC_OVERCHARGE_LEVELS + 1));
            spell_level = MAGIC_OVERCHARGE_LEVELS;
        }
    }
    spell_level--;
    value->shorts[1] = crtr_id;
    value->shorts[2] = select_id;
    value->shorts[3] = mag_id;
    value->shorts[4] = spell_level;
    PROCESS_SCRIPT_VALUE(scline->command);
}
static void use_spell_on_creature_process(struct ScriptContext* context)
{
    ThingModel crmodel = context->value->shorts[1];
    short select_id = context->value->shorts[2];
    SpellKind spell_idx = context->value->shorts[3];
    CrtrExpLevel overchrg = context->value->shorts[4];
    script_use_spell_on_creature_with_criteria(context->player_idx, crmodel, select_id, spell_idx, overchrg);
}

static void use_spell_on_players_creatures_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    long crtr_id = parse_creature_name(scline->tp[1]);
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        return;
    }
    const char *mag_name = scline->tp[2];
    short mag_id = get_rid(spell_desc, mag_name);
    CrtrExpLevel spell_level = scline->np[3];
    if (mag_id == -1)
    {
        SCRPTERRLOG("Invalid spell: %s", mag_name);
        return;
    }
    struct SpellConfig *spconf = get_spell_config(mag_id);
    if (spconf->linked_power) // Only check for spells linked to a keeper power.
    {
        if (spell_level < 1)
        {
            SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", mag_name, spell_level);
            spell_level = 1;
        }
        if (spell_level > (MAGIC_OVERCHARGE_LEVELS + 1)) // Creatures cast spells from level 1 to 10.
        {
            SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", mag_name, spell_level, (MAGIC_OVERCHARGE_LEVELS + 1));
            spell_level = MAGIC_OVERCHARGE_LEVELS;
        }
    }
    spell_level--;
    value->shorts[1] = crtr_id;
    value->shorts[2] = mag_id;
    value->shorts[3] = spell_level;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void use_spell_on_players_creatures_process(struct ScriptContext *context)
{
    long crmodel = context->value->shorts[1];
    long spell_idx = context->value->shorts[2];
    CrtrExpLevel overchrg = context->value->shorts[3];
    apply_spell_effect_to_players_creatures(context->player_idx, crmodel, spell_idx, overchrg);
}

static void use_power_on_players_creatures_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    long crtr_id = parse_creature_name(scline->tp[1]);
    PlayerNumber caster_player = scline->np[2];
    const char* pwr_name = scline->tp[3];
    short pwr_id = get_rid(power_desc, pwr_name);
    KeepPwrLevel power_level = scline->np[4];
    short free = scline->np[5];
    if (free == -1)
    {
        free = get_id(is_free_desc, scline->tp[5]);
        if (free == -1)
        {
            SCRPTERRLOG("Unknown free value '%s' not recognized", scline->tp[5]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }

    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        DEALLOCATE_SCRIPT_VALUE
    }
    if (pwr_id == -1)
    {
        SCRPTERRLOG("Invalid power: %s", pwr_name);
        DEALLOCATE_SCRIPT_VALUE
    }
    switch (pwr_id)
    {
    case PwrK_HEALCRTR:
    case PwrK_SPEEDCRTR:
    case PwrK_PROTECT:
    case PwrK_REBOUND:
    case PwrK_CONCEAL:
    case PwrK_DISEASE:
    case PwrK_CHICKEN:
    case PwrK_FREEZE:
    case PwrK_SLOW:
    case PwrK_FLIGHT:
    case PwrK_VISION:
    case PwrK_CALL2ARMS:
    case PwrK_LIGHTNING:
    case PwrK_CAVEIN:
    case PwrK_SIGHT:
    case PwrK_TIMEBOMB:
        if ((power_level < 1) || (power_level > MAGIC_OVERCHARGE_LEVELS))
        {
            SCRPTERRLOG("Power %s level %d out of range. Acceptible values are %d~%d", pwr_name, power_level, 1, MAGIC_OVERCHARGE_LEVELS);
            DEALLOCATE_SCRIPT_VALUE
        }
        power_level--; // transform human 1~9 range into computer 0~8 range
        break;
    case PwrK_SLAP:
    case PwrK_MKDIGGER:
        break;
    default:
        SCRPTERRLOG("Power not supported for this command: %s", power_code_name(pwr_id));
        DEALLOCATE_SCRIPT_VALUE
    }
    value->shorts[1] = crtr_id;
    value->shorts[2] = pwr_id;
    value->shorts[3] = power_level;
    value->shorts[4] = caster_player;
    value->shorts[5] = free;
    PROCESS_SCRIPT_VALUE(scline->command);
}

/**
 * Casts a keeper power on all creatures of a specific model, or positions of all creatures depending on the power.
 * @param crmodel The creature model to target, accepts wildcards.
 * @param pwr_idx The ID of the Keeper Power.
 * @param overchrg The overcharge level of the keeperpower. Is ignored when not applicable.
 * @param caster The player number of the player who is made to cast the spell.
 * @param free If gold is used when casting the spell. It will fail to cast if it is not free and money is not available.
 */
void cast_power_on_players_creatures(PlayerNumber plyr_idx, ThingModel crmodel, short pwr_idx, KeepPwrLevel overchrg, PlayerNumber caster, TbBool free)
{
    SYNCDBG(8, "Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    unsigned long k = 0;

    TbBool need_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel);
    struct Thing* thing = INVALID_THING;
    int i;
    if ((!need_spec_digger) || (crmodel == CREATURE_ANY) || (crmodel == CREATURE_NOT_A_DIGGER))
    {
        i = dungeon->creatr_list_start;
    }
    else
    {
        i = dungeon->digger_list_start;
    }

    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (creature_matches_model(thing, crmodel))
        {
            script_use_power_on_creature(thing, pwr_idx, overchrg, caster, free);
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19, "Finished");
}

static void use_power_on_players_creatures_process(struct ScriptContext* context)
{
    short crmodel = context->value->shorts[1];
    short pwr_idx = context->value->shorts[2];
    KeepPwrLevel overchrg = context->value->shorts[3];
    PlayerNumber caster = context->value->shorts[4];
    TbBool free = context->value->shorts[5];
    cast_power_on_players_creatures(context->player_idx, crmodel, pwr_idx, overchrg, caster, free);
}

static void set_creature_instance_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->bytes[0] = scline->np[0];
    value->bytes[1] = scline->np[1];
    if (scline->tp[2][0] != '\0')
    {
        int instance = get_rid(instance_desc, scline->tp[2]);
        if (instance != -1)
        {
            value->bytes[2] = instance;
        }
        else
        {
            SCRPTERRLOG("Invalid instance: %s", scline->tp[2]);
            return;
        }
    }
    value->bytes[3] = scline->np[3];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_creature_instance_process(struct ScriptContext *context)
{
    ThingModel crmodel = context->value->bytes[0];
    int slot = context->value->bytes[1];
    int instance = context->value->bytes[2];
    unsigned char level = context->value->bytes[3];

    script_set_creature_instance(crmodel, slot, instance, level);

}


static void hide_hero_gate_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    short n = scline->np[0];
    if (scline->np[0] < 0)
    {
        n = -scline->np[0];
    }
    struct Thing* thing = find_hero_gate_of_number(n);
    if (thing_is_invalid(thing))
    {
        SCRPTERRLOG("Invalid hero gate: %ld", scline->np[0]);
        return;
    }
    value->bytes[0] = n;
    value->bytes[1] = scline->np[1];

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void hide_hero_gate_process(struct ScriptContext* context)
{
    struct Thing* thing = find_hero_gate_of_number(context->value->bytes[0]);
    if (context->value->bytes[1])
    {
        light_turn_light_off(thing->light_id);
        create_effect(&thing->mappos, TngEff_BallPuffWhite, thing->owner);
        place_thing_in_creature_controlled_limbo(thing);
    }
    else
    {
        create_effect(&thing->mappos, TngEff_BallPuffWhite, thing->owner);
        remove_thing_from_creature_controlled_limbo(thing);
        light_turn_light_on(thing->light_id);
    }
}

static void if_check(const struct ScriptLine *scline)
{

    long plr_range_id = scline->np[0];
    const char *varib_name = scline->tp[1];
    const char *operatr = scline->tp[2];

    int32_t plr_range_id_right = -1;
    const char *varib_name_right = scline->tp[4];

    long value = 0;

    TbBool double_var_mode = false;
    int32_t varib_type;
    int32_t varib_id;
    int32_t varib_type_right;
    int32_t varib_id_right;


    if (*varib_name_right != '\0')
    {
        double_var_mode = true;

        if (!get_player_id(scline->tp[3], &plr_range_id_right)) {

            SCRPTWRNLOG("failed to parse \"%s\" as a player", scline->tp[3]);
        }
    }
    else
    {
        double_var_mode = false;

        char* text;
        value = strtol(scline->tp[3], &text, 0);
        if (text != &scline->tp[3][strlen(scline->tp[3])]) {
            SCRPTWRNLOG("Numerical value \"%s\" interpreted as %ld", scline->tp[3], value);
        }
    }


    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
      SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
      return;
    }
    // Recognize variable
    if (!parse_get_varib(varib_name, &varib_id, &varib_type, level_file_version))
    {
        return;
    }
    if (double_var_mode && !parse_get_varib(varib_name_right, &varib_id_right, &varib_type_right, level_file_version))
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
                if (((varib_type != SVar_GAME_TURN) && (varib_type != SVar_ALL_DUNGEONS_DESTROYED)
                 && (varib_type != SVar_DOOR_NUM) && (varib_type != SVar_TRAP_NUM)))
                    SCRPTWRNLOG("Found player without dungeon used in IF clause in script; this will not work correctly");

            }
        }
        if (double_var_mode && get_players_range(plr_range_id_right, &plr_start, &plr_end) >= 0) {
            struct Dungeon* dungeon = get_dungeon(plr_start);
            if ((plr_start+1 == plr_end) && dungeon_invalid(dungeon)) {
                // Note that this list should be kept updated with the changes in get_condition_value()
                if (((varib_type_right != SVar_GAME_TURN) && (varib_type_right != SVar_ALL_DUNGEONS_DESTROYED)
                 && (varib_type_right != SVar_DOOR_NUM) && (varib_type_right != SVar_TRAP_NUM)))
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
    if (double_var_mode)
    {
        command_add_condition_2variables(plr_range_id, opertr_id, varib_type, varib_id,plr_range_id_right, varib_type_right, varib_id_right);
    }
    else{
        command_add_condition(plr_range_id, opertr_id, varib_type, varib_id, value);
    }
}

static void if_available_check(const struct ScriptLine *scline)
{

    long plr_range_id = scline->np[0];
    const char *varib_name = scline->tp[1];
    const char *operatr = scline->tp[2];

    int32_t plr_range_id_right;
    const char *varib_name_right = scline->tp[4];

    long value;

    TbBool double_var_mode = false;
    int32_t varib_type_right;
    int32_t varib_id_right;


    if (*varib_name_right != '\0')
    {
        double_var_mode = true;

        if (!get_player_id(scline->tp[3], &plr_range_id_right)) {

            SCRPTWRNLOG("failed to parse \"%s\" as a player", scline->tp[3]);
        }
    }
    else
    {
        double_var_mode = false;

        char* text;
        value = strtol(scline->tp[3], &text, 0);
        if (text != &scline->tp[3][strlen(scline->tp[3])]) {
            SCRPTWRNLOG("Numerical value \"%s\" interpreted as %ld", scline->tp[3], value);
        }
    }

    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
      SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
      return;
    }
    // Recognize variable
    long varib_id;
    long varib_type = get_id(available_variable_desc, varib_name);
    if (varib_type == -1)
        varib_id = -1;
    else
        varib_id = 0;
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
    if (double_var_mode && !parse_get_varib(varib_name_right, &varib_id_right, &varib_type_right, level_file_version))
    {
        return;
    }
    // Add the condition to script structure
    if (double_var_mode)
    {
        command_add_condition_2variables(plr_range_id, opertr_id, varib_type, varib_id,plr_range_id_right, varib_type_right, varib_id_right);
    }
    else{
        command_add_condition(plr_range_id, opertr_id, varib_type, varib_id, value);
    }
}

static void if_controls_check(const struct ScriptLine *scline)
{

    long plr_range_id = scline->np[0];
    const char *varib_name = scline->tp[1];
    const char *operatr = scline->tp[2];

    int32_t plr_range_id_right;
    const char *varib_name_right = scline->tp[4];

    long value;

    TbBool double_var_mode = false;
    int32_t varib_type_right = 0;
    int32_t varib_id_right = 0;


    if (*varib_name_right != '\0')
    {
        double_var_mode = true;

        if (!get_player_id(scline->tp[3], &plr_range_id_right)) {

            SCRPTWRNLOG("failed to parse \"%s\" as a player", scline->tp[3]);
        }
    }
    else
    {
        double_var_mode = false;

        char* text;
        value = strtol(scline->tp[3], &text, 0);
        if (text != &scline->tp[3][strlen(scline->tp[3])]) {
            SCRPTWRNLOG("Numerical value \"%s\" interpreted as %ld", scline->tp[3], value);
        }
    }

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
        if (double_var_mode && get_players_range(plr_range_id_right, &plr_start, &plr_end) >= 0) {
            struct Dungeon* dungeon = get_dungeon(plr_start);
            if ((plr_start+1 == plr_end) && dungeon_invalid(dungeon)) {
                // Note that this list should be kept updated with the changes in get_condition_value()
                if (((varib_type_right != SVar_GAME_TURN) && (varib_type_right != SVar_ALL_DUNGEONS_DESTROYED)
                 && (varib_type_right != SVar_DOOR_NUM) && (varib_type_right != SVar_TRAP_NUM)))
                    SCRPTWRNLOG("Found player without dungeon used in IF clause in script; this will not work correctly");

            }
        }
    }

    if (double_var_mode && !parse_get_varib(varib_name_right, &varib_id_right, &varib_type_right, level_file_version))
    {
        return;
    }
    // Add the condition to script structure
    if (double_var_mode)
    {
        command_add_condition_2variables(plr_range_id, opertr_id, varib_type, varib_id,plr_range_id_right, varib_type_right, varib_id_right);
    }
    else
    {
        command_add_condition(plr_range_id, opertr_id, varib_type, varib_id, value);
    }
}

static void if_allied_check(const struct ScriptLine *scline)
{
    long pA = scline->np[0];
    long pB = scline->np[1];
    long op = scline->np[2];
    long val = scline->np[3];

    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }

    command_add_condition(pA, op, SVar_ALLIED_PLAYER, pB, val);
}

static void set_texture_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);

    long texture_id = get_rid(texture_pack_desc, scline->tp[1]);
    if (texture_id == -1)
    {
        if (parameter_is_number(scline->tp[1]))
        {
            texture_id = atoi(scline->tp[1]) + 1;
        }
        else
        {
            SCRPTERRLOG("Invalid texture pack: '%s'", scline->tp[1]);
            return;
        }
    }
    value->shorts[0] = texture_id;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_texture_process(struct ScriptContext *context)
{
    PlayerNumber plyr_idx = context->player_idx;
    long texture_id = context->value->shorts[0];

    set_player_texture(plyr_idx, texture_id);
}

static void set_music_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if (parameter_is_number(scline->tp[0])) {
        value->chars[0] = atoi(scline->tp[0]);
    } else {
        value->chars[0] = -1;
        value->longs[1] = script_strdup(scline->tp[0]);
        if (value->longs[1] < 0) {
            SCRPTERRLOG("Run out script strings space");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_music_process(struct ScriptContext *context)
{
    short track = context->value->chars[0];
    if (track == 0) {
        SCRPTLOG("Stopping music");
        stop_music();
    } else if (track < 0) {
        const char * fname = script_strval(context->value->longs[1]);
        SCRPTLOG("Playing music from %s", fname);
        play_music(prepare_file_fmtpath(FGrp_CmpgMedia, "%s", fname));
    } else {
        SCRPTLOG("Playing music track %d", track);
        play_music_track(track);
    }
}

static void play_message_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    long msgtype_id = get_id(msgtype_desc, scline->tp[1]);
    if (msgtype_id == -1)
    {
        SCRPTERRLOG("Unrecognized message type: '%s'", scline->tp[1]);
        return;
    }
    value->chars[1] = msgtype_id;
    if (parameter_is_number(scline->tp[2]))
    {
        value->shorts[1] = atoi(scline->tp[2]);
        value->bytes[4] = 0;
    }
    else
    {
        value->bytes[4] = 1;
        value->longs[2] = script_strdup(scline->tp[2]);
        if (value->longs[2] < 0) {
            SCRPTERRLOG("Run out script strings space");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void play_message_process(struct ScriptContext *context)
{
    const TbBool param_is_string = context->value->bytes[4];
    const char msgtype_id = context->value->chars[1];
    const short msg_id = context->value->shorts[1];
    const char * filename = script_strval(context->value->longs[2]);


    if (context->player_idx == my_player_number)
    {
        script_play_message(param_is_string,msgtype_id,msg_id,filename);
    }
}

static void set_power_hand_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);

    long hand_idx = get_rid(powerhand_desc, scline->tp[1]);
    if (hand_idx == -1)
    {
        if (parameter_is_number(scline->tp[1]))
        {
            hand_idx = atoi(scline->tp[1]);
        }
        else
        {
            SCRPTERRLOG("Invalid hand_idx: '%s'", scline->tp[1]);
            return;
        }
    }
    value->shorts[0] = hand_idx;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_power_hand_process(struct ScriptContext *context)
{
    long hand_idx = context->value->shorts[0];
    struct PlayerInfo * player;
    player = get_player(context->player_idx);
    player->hand_idx = hand_idx;
}

static void add_effectgen_to_level_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    const char* generator_name = scline->tp[0];
    const char* locname = scline->tp[1];
    long range = scline->np[2];

    TbMapLocation location;
    ThingModel gen_id;
    if (parameter_is_number(generator_name))
    {
        gen_id = atoi(generator_name);
    }
    else
    {
        gen_id = get_id(effectgen_desc, generator_name);
    }
    if (gen_id <= 0)
    {
        SCRPTERRLOG("Unknown effect generator, '%s'", generator_name);
        DEALLOCATE_SCRIPT_VALUE;
        return;
    }
    if (game.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_CREATURE commands to spawn effect generator in script");
        DEALLOCATE_SCRIPT_VALUE;
        return;
    }

    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
    {
        DEALLOCATE_SCRIPT_VALUE;
        return;
    }
    value->shorts[0] = (short)gen_id;
    value->shorts[1] = location;
    value->shorts[2] = range * COORD_PER_STL;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_effectgen_to_level_process(struct ScriptContext* context)
{
    ThingModel gen_id = context->value->shorts[0];
    short location = context->value->shorts[1];
    short range = context->value->shorts[2];
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        script_process_new_effectgen(gen_id, location, range);
    }
    else
    {
        if (game.script.party_triggers_num < PARTY_TRIGGERS_COUNT)
        {
            struct PartyTrigger* pr_trig = &game.script.party_triggers[game.script.party_triggers_num];
            pr_trig->flags = TrgF_CREATE_EFFECT_GENERATOR;
            pr_trig->flags |= next_command_reusable ? TrgF_REUSABLE : 0;
            pr_trig->plyr_idx = 0; //not needed
            pr_trig->creatr_id = 0; //not needed
            pr_trig->exp_level = gen_id;
            pr_trig->carried_gold = range;
            pr_trig->location = location;
            pr_trig->ncopies = 1;
            pr_trig->condit_idx = get_script_current_condition();
        }
        else
        {
            SCRPTERRLOG("Max party triggers reached, failed to spawn effect generator");
        }
        game.script.party_triggers_num++;
    }
}

static void set_effectgen_configuration_check(const struct ScriptLine* scline)
{
    set_config_check(&effects_effectgenerator_named_fields_set, scline,"SET_EFFECTGEN_CONFIG");
}

static void set_effectgen_configuration_process(struct ScriptContext* context)
{
    set_config_process(&effects_effectgenerator_named_fields_set, context,"SET_EFFECTGEN_CONFIG");
}

static void set_power_configuration_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    const char *powername = scline->tp[0];
    const char *property = scline->tp[1];
    char *new_value = (char*)scline->tp[2];

    long power_id = get_id(power_desc, powername);
    if (power_id == -1)
    {
        SCRPTERRLOG("Unknown power, '%s'", powername);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    long powervar = get_id(magic_power_commands, property);
    if (powervar == -1)
    {
        SCRPTERRLOG("Unknown power variable: %s", new_value);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    long long number_value = 0;
    long k;
    switch (powervar)
    {
        case 2: // Power
        case 3: // Cost
        {
            value->bytes[3] = atoi(scline->tp[3]) - 1; //-1 because we want slot 1 to 9, not 0 to 8
            value->longs[2] = atoi(new_value);
            break;
        }
        case 5: // Castability
        {
            long long j;
            if (scline->tp[3][0] != '\0')
            {
                j = get_long_id(powermodel_castability_commands, new_value);
                if (j <= 0)
                {
                    SCRPTERRLOG("Incorrect castability value: %s", new_value);
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
                else
                {
                    number_value = j;
                }
                value->chars[3] = atoi(scline->tp[3]);
            }
            else
            {
                if (parameter_is_number(new_value))
                {
                    number_value = atoll(new_value);
                }
                else
                {
                    char *flag = strtok(new_value," ");
                    while ( flag != NULL )
                    {
                        j = get_long_id(powermodel_castability_commands, flag);
                        if (j > 0)
                        {
                            number_value |= j;
                        } else
                        {
                            SCRPTERRLOG("Incorrect castability value: %s", new_value);
                            DEALLOCATE_SCRIPT_VALUE
                            return;
                        }
                        flag = strtok(NULL, " " );
                    }
                }
                value->chars[3] = -1;
            }
            value->ulonglongs[1] = number_value;
            break;
        }
        case 6: // Artifact
        {
            k = get_id(object_desc, new_value);
            if (k >= 0)
            {
                  number_value = k;
            }
            value->longs[2] = number_value;
            break;
        }
        case 10: // SymbolSprites
        {
            value->longs[1] = atoi(new_value);
            value->longs[2] = atoi(scline->tp[3]);
            break;
        }
        case 14: // Properties
        {
            if (scline->tp[3][0] != '\0')
            {
                k = get_id(powermodel_properties_commands, new_value);
                if (k <= 0)
                {
                    SCRPTERRLOG("Incorrect property value: %s", new_value);
                    DEALLOCATE_SCRIPT_VALUE
                    return;
                }
                else
                {
                    number_value = k;
                }
                value->chars[3] = atoi(scline->tp[3]);
            }
            else
            {
                if (parameter_is_number(new_value))
                {
                    number_value = atoi(new_value);
                }
                else
                {
                    char *flag = strtok(new_value," ");
                    while ( flag != NULL )
                    {
                        k = get_id(powermodel_properties_commands, flag);
                        if (k > 0)
                        {
                            number_value |= k;
                        } else
                        {
                            SCRPTERRLOG("Incorrect property value: %s", new_value);
                            DEALLOCATE_SCRIPT_VALUE
                            return;
                        }
                        flag = strtok(NULL, " " );
                    }
                }
                value->chars[3] = -1;
            }
            value->longs[2] = number_value;
            break;
        }
        case 15: // OverchargeCheck
        {
            number_value = get_id(powermodel_expand_check_func_type,new_value);
            if (number_value < 0)
            {
                SCRPTERRLOG("Invalid OverchargeCheckt: %s", new_value);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->longs[2] = number_value;
            break;
        }
        case 16: // PlayerState
        {
            k = get_id(player_state_commands, new_value);
            if (k >= 0)
            {
                number_value = k;
            }
            value->longs[2] = number_value;
            break;
        }
        case 17: // ParentPower
        {
            k = get_id(power_desc, new_value);
            if (k >= 0)
            {
                number_value = k;
            }
            value->longs[2] = number_value;
            break;
        }
        case 20: // Spell
        {
            k = get_id(spell_desc, new_value);
            if (k >= 0)
            {
                number_value = k;
            }
            else
            {
                SCRPTERRLOG("Incorrect Spell valuet: %s", new_value);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            break;
        }
        case 21: // Effect
        {
            k = effect_or_effect_element_id(new_value);
            if (k == 0)
            {
                SCRPTERRLOG("Unrecognised effect: %s", new_value);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            else
            {
                number_value = k;
            }
            break;
        }
        case 22: // UseFunction
        {
            k = get_id(magic_use_func_commands, new_value);
            if (k >= 0)
            {
                number_value = k;
            }
            else
            {
                SCRPTERRLOG("Incorrect UseFunction: %s", new_value);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            break;
        }
        case 23: // CreatureType
        {
            k = get_id(creature_desc, new_value);
            if (k >= 0)
            {
                number_value = k;
            }
            else
            {
                SCRPTERRLOG("Incorrect Creature type: %s", new_value);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            break;
        }
        case 24: // CostFormula
        {
            k = get_id(magic_cost_formula_commands, new_value);
            if (k >= 0)
            {
                number_value = k;
            }
            else
            {
                SCRPTERRLOG("Incorrect Cost formula: %s", new_value);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            break;
        }
        default:
            value->longs[2] = atoi(new_value);
    }
    #if (BFDEBUG_LEVEL >= 7)
    {
        if ( (powervar == 5) && (value->chars[3] != -1) )
        {
            SCRIPTDBG(7, "Toggling %s castability flag: %I64d", powername, number_value);
        }
        else if ( (powervar == 14) && (value->chars[3] != -1) )
        {
            SCRIPTDBG(7, "Toggling %s property flag: %I64d", powername, number_value);
        }
        else
        {
            SCRIPTDBG(7, "Setting power %s property %s to %I64d", powername, property, number_value);
        }
    }
    #endif
    value->shorts[0] = power_id;
    value->bytes[2] = powervar;

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_power_configuration_process(struct ScriptContext *context)
{
    struct PowerConfigStats *powerst = get_power_model_stats(context->value->shorts[0]);
    switch (context->value->bytes[2])
    {
        case 2: // Power
            powerst->strength[context->value->bytes[3]] = context->value->longs[2];
            break;
        case 3: // Cost
            powerst->cost[context->value->bytes[3]] = context->value->longs[2];
            break;
        case 4: // Duration
            powerst->duration = context->value->longs[2];
            break;
        case 5: // Castability
        {
            unsigned long long flag = context->value->ulonglongs[1];
            if (context->value->chars[3] == 1)
            {
                set_flag(powerst->can_cast_flags, flag);
            }
            else if (context->value->chars[3] == 0)
            {
                clear_flag(powerst->can_cast_flags, flag);
            }
            else
            {
                powerst->can_cast_flags = flag;
            }
            break;
        }
        case 6: // Artifact
            powerst->artifact_model = context->value->longs[2];
            game.conf.object_conf.object_to_power_artifact[powerst->artifact_model] = context->value->shorts[0];
            break;
        case 7: // NameTextID
            powerst->name_stridx = context->value->longs[2];
            break;
        case 8: // TooltipTextID
            powerst->tooltip_stridx = context->value->longs[2];
            break;
        case 10: // SymbolSprites
            powerst->bigsym_sprite_idx = context->value->longs[1];
            powerst->medsym_sprite_idx = context->value->longs[2];
            break;
        case 11: // PointerSprites
            powerst->pointer_sprite_idx = context->value->longs[2];
            break;
        case 12: // PanelTabIndex
            powerst->panel_tab_idx = context->value->longs[2];
            break;
        case 13: // SoundSamples
            powerst->select_sample_idx = context->value->longs[2];
            break;
        case 14: // Properties
            if (context->value->chars[3] == 1)
            {
                set_flag(powerst->config_flags, context->value->longs[2]);
            }
            else if (context->value->chars[3] == 0)
            {
                clear_flag(powerst->config_flags, context->value->longs[2]);
            }
            else
            {
                powerst->config_flags = context->value->longs[2];
            }
            break;
        case 15: // OverchargeCheck
            powerst->overcharge_check_idx = context->value->longs[2];
            break;
        case 16: // PlayerState
            powerst->work_state = context->value->longs[2];
            break;
        case 17: // ParentPower
            powerst->parent_power = context->value->longs[2];
            break;
        case 18: // SoundPlayed
            powerst->select_sound_idx = context->value->longs[2];
            break;
        case 19: // Cooldown
            powerst->cast_cooldown = context->value->longs[2];
            break;
        case 20: // Spell
            powerst->cast_cooldown = context->value->longs[2];
            break;
        case 21: // Effect
            powerst->effect_id = context->value->longs[2];
            break;
        case 22: // UseFunction
            powerst->magic_use_func_idx = context->value->longs[2];
            break;
        case 23: // CreatureType
            powerst->creature_model = context->value->longs[2];
            break;
        case 24: // CostFormula
            powerst->cost_formula = context->value->longs[2];
            break;
        default:
            WARNMSG("Unsupported power configuration, variable %d.", context->value->bytes[2]);
            break;
    }
    update_powers_tab_to_config();
    struct PlayerInfo *player = get_my_player();
    if (player->view_type == PVT_DungeonTop)
    {
        if (menu_is_active(GMnu_SPELL))
        {
            turn_off_menu(GMnu_SPELL);
            turn_on_menu(GMnu_SPELL);
        }
        else if (menu_is_active(GMnu_SPELL2))
        {
            turn_off_menu(GMnu_SPELL2);
            turn_on_menu(GMnu_SPELL2);
        }
    }
}

static void set_player_colour_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    long color_idx = get_rid(cmpgn_human_player_options, scline->tp[1]);
    if (scline->np[0] == game.neutral_player_num)
    {
        SCRPTERRLOG("Can't change color of Neutral player.");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (color_idx == -1)
    {
        if (parameter_is_number(scline->tp[1]))
        {
            color_idx = atoi(scline->tp[1]);
        }
        else
        {
            SCRPTERRLOG("Invalid color: '%s'", scline->tp[1]);
            return;
        }
    }
    value->bytes[0] = (unsigned char)color_idx;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_player_colour_process(struct ScriptContext *context)
{
    if (context->player_idx == PLAYER_NEUTRAL)
    {
        return;
    }
    set_player_colour(context->player_idx, context->value->bytes[0]);
}

static void set_game_rule_check(const struct ScriptLine* scline)
{
    char* rulevalue_str = strdup(scline->tp[1]);
    PlayerNumber plyr_idx;
    if (scline->tp[2][0] == '\0')
    {
        plyr_idx = ALL_PLAYERS;
    }
    else
    {
        plyr_idx = get_id(player_desc, scline->tp[2]);
        if (plyr_idx == -1)
        {
            if (!parameter_is_number(scline->tp[2]))
            {
                SCRPTERRLOG("Invalid player: %s", scline->tp[1]);
                free(rulevalue_str);
                return;
            }
            plyr_idx = ALL_PLAYERS;
            snprintf(rulevalue_str, MAX_TEXT_LENGTH, "%s %s", scline->tp[1], scline->tp[2]);
        }
    }
    ALLOCATE_SCRIPT_VALUE(scline->command, plyr_idx);

    const char* rulename = scline->tp[0];


    long rulegroup = 0;
    long ruleval = 0;
    long ruledesc = 0;

    for (size_t i = 0; i < sizeof(ruleblocks)/sizeof(ruleblocks[0]); i++)
    {
        ruledesc = get_named_field_id(ruleblocks[i], rulename);
        if (ruledesc != -1)
        {
            rulegroup = i;
            ruleval = parse_named_field_value(ruleblocks[i]+ruledesc, rulevalue_str,&rules_named_fields_set, 0,"SET_GAME_RULE",ccf_SplitExecution|ccf_DuringLevel);
            break;
        }
    }
    free(rulevalue_str);
    if (ruledesc == -1)
    {
        SCRPTERRLOG("Unknown Game Rule '%s'.", rulename);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[0] = rulegroup;
    value->shorts[1] = ruledesc;
    value->longs[1] = ruleval;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_game_rule_process(struct ScriptContext* context)
{
    short rulegroup = context->value->shorts[0];
    short ruledesc  = context->value->shorts[1];
    long rulevalue  = context->value->longs[1];


    SCRIPTDBG(7,"Changing Game Rule '%s' to %ld", (ruleblocks[rulegroup]+ruledesc)->name, rulevalue);

    assign_named_field_value((ruleblocks[rulegroup]+ruledesc),rulevalue,&rules_named_fields_set,context->player_idx,"SET_GAME_RULE",ccf_SplitExecution|ccf_DuringLevel);
}

static void set_increase_on_experience_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long onexpdesc = get_id(on_experience_desc, scline->tp[0]);
    if (onexpdesc == -1)
    {
        SCRPTERRLOG("Unknown variable '%s'.", scline->tp[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (scline->np[1] < 0)
    {
        SCRPTERRLOG("Value %ld out of range for variable '%s'.", scline->np[1], scline->tp[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = onexpdesc;
    value->shorts[1] = scline->np[1];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_increase_on_experience_process(struct ScriptContext* context)
{
    short variable = context->value->shorts[0];
  #if (BFDEBUG_LEVEL > 0)
    const char *varname = on_experience_desc[variable - 1].name;
  #endif
    switch (variable)
    {
    case 1: //SizeIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.size_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.size_increase_on_exp = context->value->shorts[1];
        break;
    case 2: //PayIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.pay_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.pay_increase_on_exp = context->value->shorts[1];
        break;
    case 3: //SpellDamageIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.spell_damage_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.spell_damage_increase_on_exp = context->value->shorts[1];
        break;
    case 4: //RangeIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.range_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.range_increase_on_exp = context->value->shorts[1];
        break;
    case 5: //JobValueIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.job_value_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.job_value_increase_on_exp = context->value->shorts[1];
        break;
    case 6: //HealthIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.health_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.health_increase_on_exp = context->value->shorts[1];
        break;
    case 7: //StrengthIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.strength_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.strength_increase_on_exp = context->value->shorts[1];
        break;
    case 8: //DexterityIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.dexterity_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.dexterity_increase_on_exp = context->value->shorts[1];
        break;
    case 9: //DefenseIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.defense_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.defense_increase_on_exp = context->value->shorts[1];
        break;
    case 10: //LoyaltyIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.loyalty_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.loyalty_increase_on_exp = context->value->shorts[1];
        break;
    case 11: //ExpForHittingIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.exp_on_hitting_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.exp_on_hitting_increase_on_exp = context->value->shorts[1];
        break;
    case 12: //TrainingCostIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.training_cost_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.training_cost_increase_on_exp = context->value->shorts[1];
        break;
    case 13: //ScavengingCostIncreaseOnExp
        SCRIPTDBG(7,"Changing variable %s from %d to %d.", varname, game.conf.crtr_conf.exp.scavenging_cost_increase_on_exp, context->value->shorts[1]);
        game.conf.crtr_conf.exp.scavenging_cost_increase_on_exp = context->value->shorts[1];
        break;
    default:
        WARNMSG("Unsupported variable, command %d.", context->value->shorts[0]);
        break;
    }
}

static void set_player_modifier_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    short mdfrdesc = get_id(modifier_desc, scline->tp[1]);
    short mdfrval = scline->np[2];
    const char *mdfrname = get_conf_parameter_text(modifier_desc,mdfrdesc);
    if (mdfrdesc == -1)
    {
        SCRPTERRLOG("Unknown Player Modifier '%s'.", scline->tp[1]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (mdfrval < 0)
    {
        SCRPTERRLOG("Value %d out of range for Player Modifier '%s'.", mdfrval, mdfrname);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (scline->np[0] == game.neutral_player_num)
    {
        SCRPTERRLOG("Can't manipulate Player Modifier '%s', player %ld has no dungeon.", mdfrname, scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = mdfrdesc;
    value->shorts[1] = mdfrval;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_player_modifier_process(struct ScriptContext* context)
{
    struct Dungeon* dungeon;
    short mdfrdesc = context->value->shorts[0];
    short mdfrval = context->value->shorts[1];
    #if (BFDEBUG_LEVEL > 0)
        const char *mdfrname = get_conf_parameter_text(modifier_desc,mdfrdesc);
    #endif
    PlayerNumber plyr_idx = context->player_idx;
    dungeon = get_dungeon(plyr_idx);
    switch (mdfrdesc)
    {
        case 1: // Health
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.health, mdfrval);
            dungeon->modifier.health = mdfrval;
            do_to_players_all_creatures_of_model(plyr_idx, CREATURE_ANY, update_relative_creature_health);
            break;
        case 2: // Strength
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.strength, mdfrval);
            dungeon->modifier.strength = mdfrval;
            break;
        case 3: // Armour
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.armour, mdfrval);
            dungeon->modifier.armour = mdfrval;
            break;
        case 4: // SpellDamage
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.spell_damage, mdfrval);
            dungeon->modifier.spell_damage = mdfrval;
            break;
        case 5: // Speed
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.speed, mdfrval);
            dungeon->modifier.speed = mdfrval;
            do_to_players_all_creatures_of_model(plyr_idx, CREATURE_ANY, update_creature_speed);
            break;
        case 6: // Salary
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.pay, mdfrval);
            dungeon->modifier.pay = mdfrval;
            break;
        case 7: // TrainingCost
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.training_cost, mdfrval);
            dungeon->modifier.training_cost = mdfrval;
            break;
        case 8: // ScavengingCost
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.scavenging_cost, mdfrval);
            dungeon->modifier.scavenging_cost = mdfrval;
            break;
        case 9: // Loyalty
            SCRIPTDBG(7,"Changing Player Modifier '%s' of player %d from %d to %d.", mdfrname, (int)plyr_idx, dungeon->modifier.loyalty, mdfrval);
            dungeon->modifier.loyalty = mdfrval;
            break;
        default:
            WARNMSG("Unsupported Player Modifier, command %d.", mdfrdesc);
            break;
    }
}

static void add_to_player_modifier_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    short mdfrdesc = get_id(modifier_desc, scline->tp[1]);
    short mdfrval = scline->np[2];
    const char *mdfrname = get_conf_parameter_text(modifier_desc,mdfrdesc);
    if (mdfrdesc == -1)
    {
        SCRPTERRLOG("Unknown Player Modifier '%s'.", scline->tp[1]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (scline->np[0] == game.neutral_player_num)
    {
        SCRPTERRLOG("Can't manipulate Player Modifier '%s', player %ld has no dungeon.", mdfrname, scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = mdfrdesc;
    value->shorts[1] = mdfrval;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_to_player_modifier_process(struct ScriptContext* context)
{
    struct Dungeon* dungeon;
    short mdfrdesc = context->value->shorts[0];
    short mdfrval = context->value->shorts[1];
    short mdfradd;
    const char *mdfrname = get_conf_parameter_text(modifier_desc,mdfrdesc);
    PlayerNumber plyr_idx = context->player_idx;
    dungeon = get_dungeon(plyr_idx);
    switch (mdfrdesc)
    {
        case 1: // Health
            mdfradd = dungeon->modifier.health + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.health = mdfradd;
                do_to_players_all_creatures_of_model(plyr_idx, CREATURE_ANY, update_relative_creature_health);
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.health);
            }
            break;
        case 2: // Strength
            mdfradd = dungeon->modifier.strength + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.strength = mdfradd;
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.strength);
            }
            break;
        case 3: // Armour
            mdfradd = dungeon->modifier.armour + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.armour = mdfradd;
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.armour);
            }
            break;
        case 4: // SpellDamage
            mdfradd = dungeon->modifier.spell_damage + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.spell_damage = mdfradd;
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.spell_damage);
            }
            break;
        case 5: // Speed
            mdfradd = dungeon->modifier.speed + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.speed = mdfradd;
                do_to_players_all_creatures_of_model(plyr_idx, CREATURE_ANY, update_creature_speed);
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.speed);
            }
            break;
        case 6: // Salary
            mdfradd = dungeon->modifier.pay + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.pay = mdfradd;
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.pay);
            }
            break;
        case 7: // TrainingCost
            mdfradd = dungeon->modifier.training_cost + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.training_cost = mdfradd;
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.training_cost);
            }
            break;
        case 8: // ScavengingCost
            mdfradd = dungeon->modifier.scavenging_cost + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.scavenging_cost = mdfradd;
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.scavenging_cost);
            }
            break;
        case 9: // Loyalty
            mdfradd = dungeon->modifier.loyalty + mdfrval;
            if (mdfradd >= 0) {
                SCRIPTDBG(7,"Adding %d to Player %d Modifier '%s'.", mdfrval, (int)plyr_idx, mdfrname);
                dungeon->modifier.loyalty = mdfradd;
            } else {
                SCRPTERRLOG("Player %d Modifier '%s' may not be negative. Tried to add %d to value %d", (int)plyr_idx, mdfrname, mdfrval, dungeon->modifier.loyalty);
            }
            break;
        default:
            WARNMSG("Unsupported Player Modifier, command %d.", mdfrdesc);
            break;
    }
}

static void set_creature_max_level_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    short crtr_id = parse_creature_name(scline->tp[1]);
    short crtr_lvl = scline->np[2];
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unable to manipulate max level of creature '%s', creature doesn't exist.", scline->tp[1]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if ((crtr_lvl < -1) || (crtr_lvl > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Unable to set max level of creature '%s' to %d, value is out of range.", creature_code_name(crtr_id), crtr_lvl);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = crtr_id;
    value->shorts[1] = crtr_lvl;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_creature_max_level_process(struct ScriptContext* context)
{
    struct Dungeon* dungeon;
    short crtr_id = context->value->shorts[0];
    short crtr_lvl = context->value->shorts[1];
    PlayerNumber plyr_idx = context->player_idx;
    dungeon = get_dungeon(plyr_idx);
    if (!dungeon_invalid(dungeon))
    {
        if (!is_creature_model_wildcard(crtr_id))
        {
            if (crtr_id < game.conf.crtr_conf.model_count) {
                if (crtr_lvl < 0)
                {
                    crtr_lvl = CREATURE_MAX_LEVEL + 1;
                    dungeon->creature_max_level[crtr_id] = crtr_lvl;
                    SCRIPTDBG(7,"Max level of creature '%s' set to default for player %d.", creature_code_name(crtr_id), (int)plyr_idx);
                } else {
                    dungeon->creature_max_level[crtr_id] = crtr_lvl-1;
                    SCRIPTDBG(7,"Max level of creature '%s' set to %d for player %d.", creature_code_name(crtr_id), crtr_lvl, (int)plyr_idx);
                }
            }
        } else
        {
            for (int i = 1; i < game.conf.crtr_conf.model_count; i++)
            {
                if (creature_model_matches_model(i, plyr_idx , crtr_id))
                {
                    if (crtr_lvl < 0)
                    {
                        crtr_lvl = CREATURE_MAX_LEVEL + 1;
                        dungeon->creature_max_level[i] = crtr_lvl;
                        SCRIPTDBG(7,"Max level of creature '%s' set to default for player %d.", creature_code_name(i), (int)plyr_idx);
                    } else {
                        dungeon->creature_max_level[i] = crtr_lvl-1;
                        SCRIPTDBG(7,"Max level of creature '%s' set to %d for player %d.", creature_code_name(i), crtr_lvl, (int)plyr_idx);
                    }
                }
            }
        }
    } else
    {
        SCRPTERRLOG("Unable to manipulate max level of creature '%s', player %d has no dungeon.", creature_code_name(crtr_id), (int)plyr_idx);
    }
}

static void reset_action_point_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long apt_idx = action_point_number_to_index(scline->np[0]);
    if (!action_point_exists_idx(apt_idx))
    {
        SCRPTERRLOG("Non-existing Action Point, no %ld", scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->longs[0] = apt_idx;
    PlayerNumber plyr_idx = (scline->tp[1][0] == '\0') ? ALL_PLAYERS : get_id(player_desc, scline->tp[1]);
    if (plyr_idx == -1)
    {
        SCRPTERRLOG("Invalid player: %s", scline->tp[1]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->chars[4] = plyr_idx;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void reset_action_point_process(struct ScriptContext* context)
{
    action_point_reset_idx(context->value->longs[0], context->value->chars[4]);
}

static void quick_message_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if ((scline->np[0] < 0) || (scline->np[0] >= QUICK_MESSAGES_COUNT))
    {
        SCRPTERRLOG("Invalid information ID number (%ld)", scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (strlen(scline->tp[1]) > MESSAGE_TEXT_LEN)
    {
        SCRPTWRNLOG("Information TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
    }
    if ((game.quick_messages[scline->np[0]][0] != '\0') && (strcmp(game.quick_messages[scline->np[0]],scline->tp[1]) != 0))
    {
        SCRPTWRNLOG("Quick Message no %ld overwritten by different text", scline->np[0]);
    }
    snprintf(game.quick_messages[scline->np[0]], MESSAGE_TEXT_LEN, "%s", scline->tp[1]);
    value->longs[0]= scline->np[0];
    get_chat_icon_from_value(scline->tp[2], &value->chars[4], &value->chars[5]);
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void quick_message_process(struct ScriptContext* context)
{
    message_add_fmt(context->value->chars[5], context->value->chars[4], "%s", game.quick_messages[context->value->ulongs[0]]);
}

static void display_message_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->ulongs[0] = scline->np[0];
    get_chat_icon_from_value(scline->tp[1], &value->chars[4], &value->chars[5]);
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void display_message_process(struct ScriptContext* context)
{
    message_add_fmt(context->value->chars[5], context->value->chars[4], "%s", get_string(context->value->ulongs[0]));
}

static void clear_message_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if ((scline->np[0] > GUI_MESSAGES_COUNT) || (scline->np[0] <= 0))
    {
        value->chars[1] = GUI_MESSAGES_COUNT;
    }
    else
    {
        value->chars[1] = scline->np[0];
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void clear_message_process(struct ScriptContext* context)
{
    unsigned char count = min(context->value->chars[1], game.active_messages_count);
    for (int k = game.active_messages_count-1; k >= (game.active_messages_count-count); k--)
    {
        game.messages[k].expiration_turn = game.play_gameturn;
    }
}

static void change_slab_texture_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if ( (scline->np[0] < 0) || (scline->np[0] >= game.map_tiles_x) || (scline->np[1] < 0) || (scline->np[1] >= game.map_tiles_y) )
    {
        SCRPTERRLOG("Invalid co-ordinates: %ld, %ld", scline->np[0], scline->np[1]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    long texture_id = get_id(texture_pack_desc, scline->tp[2]);
    if (texture_id == -1)
    {
        if (parameter_is_number(scline->tp[2]))
        {
            texture_id = atol(scline->tp[2]) + 1;
        }
        else
        {
            SCRPTERRLOG("Invalid texture pack: '%s'", scline->tp[2]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    if ( (scline->np[2] < 0) || (scline->np[2] >= TEXTURE_VARIATIONS_COUNT) )
    {
        SCRPTERRLOG("Invalid texture ID: %ld", scline->np[2]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = scline->np[0];
    value->shorts[1] = scline->np[1];
    value->bytes[4] = (unsigned char)texture_id;
    value->chars[5] = get_id(fill_desc, scline->tp[3]);
    if ((scline->tp[3][0] != '\0') && (value->chars[5] == -1))
    {
        SCRPTWRNLOG("Fill type %s not recognized", scline->tp[3]);
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void change_slab_texture_process(struct ScriptContext* context)
{
    if (context->value->chars[5] > 0)
    {
        MapSlabCoord slb_x = context->value->shorts[0];
        MapSlabCoord slb_y = context->value->shorts[1];
        struct CompoundCoordFilterParam iter_param;
        iter_param.primary_number = context->value->bytes[4]; // new texture
        iter_param.secondary_number = context->value->chars[5]; // fill type
        iter_param.tertiary_number = get_slabmap_block(slb_x, slb_y)->kind;
        slabs_fill_iterate_from_slab(slb_x, slb_y, slabs_change_texture, &iter_param);
    }
    else
    {
        SlabCodedCoords slb_num = get_slab_number(context->value->shorts[0], context->value->shorts[1]);
        game.slab_ext_data[slb_num] = context->value->bytes[4];
        game.slab_ext_data_initial[slb_num] = context->value->bytes[4];
    }
}

static void computer_player_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long plr_range_id = scline->np[0];
    const char* comp_model = scline->tp[1];
    int plr_start;
    int plr_end;
    char model = 0;
    char type = PT_Keeper;
    TbBool toggle = true;

    if (level_file_version == 0 && plr_range_id == PLAYER_GOOD)
    {
        SCRPTERRLOG("PLAYER_GOOD COMPUTER_PLAYER cannot be set in level version 0.");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %d is not supported in this command", (int)plr_range_id);
        DEALLOCATE_SCRIPT_VALUE
    }
    for (long i = plr_start; i < plr_end; i++)
    {
        set_flag(value->shorts[2], to_flag(i));
    }
    if (parameter_is_number(comp_model))
    {
        model = atoi(comp_model);
    }
    else if (strcasecmp(comp_model, "ROAMING") == 0)
    {
        type = PT_Roaming;
    }
    else if (strcasecmp(comp_model, "OFF") == 0)
    {
        toggle = false;
    }
    else
    {
        SCRPTERRLOG("invalid COMPUTER_PLAYER param '%s'", comp_model);
        DEALLOCATE_SCRIPT_VALUE
    }

    value->bytes[0] = plr_start;
    value->bytes[1] = plr_end;
    value->bytes[2] = type;
    value->bytes[3] = model;
    value->bytes[6] = toggle;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void computer_player_process(struct ScriptContext* context)
{
    char plr_start = context->value->bytes[0];
    char plr_end = context->value->bytes[1];
    char playertype = context->value->bytes[2];
    char model = context->value->bytes[3];
    short owner_flags = context->value->shorts[2];
    TbBool toggle = context->value->bytes[6];
    struct PlayerInfo* player = INVALID_PLAYER;
    for (int i = plr_start; i < plr_end; i++)
    {
        if (i == PLAYER_NEUTRAL)
        {
            continue;
        }
        if (playertype == PT_Roaming)
        {
            player = get_player(i);
            player->player_type = PT_Roaming;
            player->allocflags |= PlaF_Allocated;
            player->allocflags |= PlaF_CompCtrl;
            player->id_number = i;
        }
        else
        {
            if (flag_is_set(owner_flags, to_flag(i)))
            {
                if (toggle == true)
                {
                    script_support_setup_player_as_computer_keeper(i, model);
                    player = get_player(i);
                    struct Dungeon* dungeon = get_dungeon(i);
                    dungeon->turns_between_entrance_generation = player->generate_speed;
                    init_creature_states_for_player(i);
                    post_init_player(player);
                }
                else
                {
                    script_support_setup_player_as_zombie_keeper(i);
                }
            }
        }
        recalculate_player_creature_digger_lists(i);
    }
}

static void add_object_to_level_at_pos_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    short tngmodel = get_rid(object_desc, scline->tp[0]);
    if (tngmodel == -1)
    {
        SCRPTERRLOG("Unknown object: %s", scline->tp[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = tngmodel;
    if (!subtile_coords_invalid(scline->np[1], scline->np[2]))
    {
        value->shorts[2] = scline->np[1];
        value->shorts[3] = scline->np[2];
    }
    else
    {
        SCRPTERRLOG("Invalid subtile co-ordinates: %ld, %ld", scline->np[1], scline->np[2]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->longs[2] = scline->np[3];
    PlayerNumber plyr_idx = get_rid(player_desc, scline->tp[4]); // Optional variable
    if ((plyr_idx == -1) || (plyr_idx == ALL_PLAYERS))
    {
        plyr_idx = PLAYER_NEUTRAL;
    }
    short angle = 0;
    if (strcmp(scline->tp[5], "") != 0) // Optional variable
    {
        if (parameter_is_number(scline->tp[5]))
        {
            angle = atoi(scline->tp[5]) % DEGREES_360;
        }
        else
        {
            angle = get_rid(orientation_desc, scline->tp[5]);
            if (angle < 0)
            {
                SCRPTERRLOG("Unknown orientation: %s", scline->tp[5]);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
        }
    }

    value->chars[2] = plyr_idx;
    value->shorts[6] = angle;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_object_to_level_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    short obj_id = get_rid(object_desc, scline->tp[0]);
    if (obj_id == -1)
    {
        SCRPTERRLOG("Unknown object, '%s'", scline->tp[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = obj_id;
    TbMapLocation location;
    if (!get_map_location_id(scline->tp[1], &location))
    {
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->ulongs[1] = location;
    value->longs[2] = scline->np[2];
    PlayerNumber plyr_idx = get_rid(player_desc, scline->tp[3]);
    if ((plyr_idx == -1) || (plyr_idx == ALL_PLAYERS)) //Optional variable
    {
        plyr_idx = PLAYER_NEUTRAL;
    }

    short angle = 0;
    if (strcmp(scline->tp[4], "") != 0) //Optional variable
    {
        if (parameter_is_number(scline->tp[4]))
        {
            angle = atoi(scline->tp[4]) % DEGREES_360;
        }
        else
        {
            angle = get_rid(orientation_desc, scline->tp[4]);
            if (angle < 0)
            {
                SCRPTERRLOG("Unknown orientation: %s", scline->tp[4]);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
        }
    }

    value->chars[2] = plyr_idx;
    value->shorts[8] = angle;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_object_to_level_process(struct ScriptContext* context)
{
    struct Coord3d pos;
    if (get_coords_at_location(&pos,context->value->ulongs[1],true))
    {
        script_process_new_object(context->value->shorts[0], pos.x.stl.num, pos.y.stl.num, context->value->longs[2], context->value->chars[2], context->value->shorts[8]);
    }
}

static void add_object_to_level_at_pos_process(struct ScriptContext* context)
{
    script_process_new_object(context->value->shorts[0], context->value->shorts[2], context->value->shorts[3], context->value->longs[2], context->value->chars[2],context->value->shorts[6]);
}

static void set_computer_globals_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long plr_range_id = scline->np[0];

    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
        SCRPTERRLOG("Given owning player range %d is not supported in this command", (int)plr_range_id);
        return;
    }

    value->shorts[0] = plr_start;
    value->shorts[1] = plr_end;
    value->longs[1] = scline->np[1];
    value->longs[2] = scline->np[2];
    value->longs[3] = scline->np[3];
    value->longs[4] = scline->np[4];
    value->longs[5] = scline->np[5];
    value->longs[6] = scline->np[6];
    value->longs[7] = -1;
    if (scline->np[7] != '\0')
    {
        value->longs[7] = scline->np[7];
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_computer_globals_process(struct ScriptContext* context)
{
    int plr_start = context->value->shorts[0];
    int plr_end = context->value->shorts[1];
    long dig_stack_size = context->value->longs[1];
    long processes_time = context->value->longs[2];
    long click_rate = context->value->longs[3];
    long max_room_build_tasks = context->value->longs[4];
    long turn_begin = context->value->longs[5];
    long sim_before_dig = context->value->longs[6];
    long task_delay = context->value->longs[7];

    for (long i = plr_start; i < plr_end; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp))
        {
            continue;
        }
        comp->dig_stack_size = dig_stack_size;
        comp->processes_time = processes_time;
        comp->click_rate = click_rate;
        comp->max_room_build_tasks = max_room_build_tasks;
        comp->turn_begin = turn_begin;
        comp->sim_before_dig = sim_before_dig;
        if (task_delay != -1)
        {
            comp->task_delay = task_delay;
        }
    }
}

static void set_computer_process_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long plr_range_id = scline->np[0];

    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
        SCRPTERRLOG("Given owning player range %d is not supported in this command", (int)plr_range_id);
        return;
    }

    value->shorts[0] = plr_start;
    value->shorts[1] = plr_end;
    value->longs[1] = scline->np[2];
    value->longs[2] = scline->np[3];
    value->longs[3] = scline->np[4];
    value->longs[4] = scline->np[5];
    value->longs[5] = scline->np[6];
    value->longs[6] = script_strdup(scline->tp[1]);
    if (value->longs[6] < 0) {
        SCRPTERRLOG("Run out script strings space");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_computer_process_process(struct ScriptContext* context)
{
    int plr_start = context->value->shorts[0];
    int plr_end = context->value->shorts[1];
    const char* procname = script_strval(context->value->longs[6]);
    long priority = context->value->longs[1];
    long config_value_2 = context->value->longs[2];
    long config_value_3 = context->value->longs[3];
    long config_value_4 = context->value->longs[4];
    long config_value_5 = context->value->longs[5];
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
            if (flag_is_set(cproc->flags, ComProc_ListEnd))
                break;
            if (strcasecmp(procname, cproc->name) == 0)
            {
                SCRPTLOG("Changing computer %d process '%s' config from (%d,%d,%d,%d,%d) to (%d,%d,%d,%d,%d)", (int)i, cproc->name,
                    (int)cproc->priority, (int)cproc->process_configuration_value_2, (int)cproc->process_configuration_value_3, (int)cproc->process_configuration_value_4, (int)cproc->process_configuration_value_5,
                    (int)priority, (int)config_value_2, (int)config_value_3, (int)config_value_4, (int)config_value_5);
                cproc->priority = priority;
                cproc->process_configuration_value_2 = config_value_2;
                cproc->process_configuration_value_3 = config_value_3;
                cproc->process_configuration_value_4 = config_value_4;
                cproc->process_configuration_value_5 = config_value_5;
                n++;
            }
        }
    }
    if (n == 0)
    {
        SCRIPTDBG(6, "No computer process found named '%s' in players %d to %d", procname, (int)plr_start, (int)plr_end - 1);
        return;
    }
    SCRIPTDBG(6, "Altered %ld processes named '%s'", n, procname);
}

static void set_computer_checks_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    long plr_range_id = scline->np[0];
    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
        SCRPTERRLOG("Given owning player range %d is not supported in this command", (int)plr_range_id);
        return;
    }

    value->shorts[0] = plr_start;
    value->shorts[1] = plr_end;
    value->longs[1] = scline->np[2];
    value->longs[2] = scline->np[3];
    value->longs[3] = scline->np[4];
    value->longs[4] = scline->np[5];
    value->longs[5] = scline->np[6];
    value->longs[6] = script_strdup(scline->tp[1]);
    if (value->longs[6] < 0) {
        SCRPTERRLOG("Run out script strings space");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_computer_checks_process(struct ScriptContext* context)
{
    int plr_start = context->value->shorts[0];
    int plr_end = context->value->shorts[1];
    const char* chkname = script_strval(context->value->longs[6]);
    long turns_interval = context->value->longs[1];
    long primary_parameter = context->value->longs[2];
    long secondary_parameter = context->value->longs[3];
    long tertiary_parameter = context->value->longs[4];
    long last_run_turn = context->value->longs[5];

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
            if (ccheck->name[0] == '\0')
                break;
            if (strcasecmp(chkname, ccheck->name) == 0)
            {
                SCRPTLOG("Changing computer %d check '%s' config from (%d,%d,%d,%d,%d) to (%d,%d,%d,%d,%d)", (int)i, ccheck->name,
                    (int)ccheck->turns_interval, (int)ccheck->primary_parameter, (int)ccheck->secondary_parameter, (int)ccheck->tertiary_parameter, (int)ccheck->last_run_turn,
                    (int)turns_interval, (int)primary_parameter, (int)secondary_parameter, (int)tertiary_parameter, (int)last_run_turn);
                ccheck->turns_interval = turns_interval;
                ccheck->primary_parameter = primary_parameter;
                ccheck->secondary_parameter = secondary_parameter;
                ccheck->tertiary_parameter = tertiary_parameter;
                ccheck->last_run_turn = last_run_turn;
                n++;
            }
        }
    }
    if (n == 0)
    {
        SCRPTERRLOG("No computer check found named '%s' in players %d to %d", chkname, (int)plr_start, (int)plr_end - 1);
        return;
    }
    SCRIPTDBG(6, "Altered %ld checks named '%s'", n, chkname);
}

static void set_computer_event_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    long plr_range_id = scline->np[0];
    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
        SCRPTERRLOG("Given owning player range %d is not supported in this command", (int)plr_range_id);
        return;
    }
    if (!player_exists(get_player(plr_range_id)))
    {
        SCRPTERRLOG("Player %d does not exist; cannot modify events", (int)plr_range_id);
        return;
    }
    value->shorts[0] = plr_start;
    value->shorts[1] = plr_end;
    value->longs[1] = scline->np[2];
    value->longs[2] = scline->np[3];
    value->longs[3] = scline->np[4];
    value->longs[4] = scline->np[5];
    value->longs[5] = scline->np[6];
    value->longs[6] = script_strdup(scline->tp[1]);
    if (value->longs[6] < 0) {
        SCRPTERRLOG("Run out script strings space");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_computer_event_process(struct ScriptContext* context)
{
    int plr_start = context->value->shorts[0];
    int plr_end = context->value->shorts[1];
    const char* evntname = script_strval(context->value->longs[6]);
    long test_interval = context->value->longs[1];
    long primary_parameter = context->value->longs[2];
    long secondary_parameter = context->value->longs[3];
    long tertiary_parameter = context->value->longs[4];
    long last_test_gameturn = context->value->longs[5];

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
            if (event->name[0] == '\0')
                break;
            if (strcasecmp(evntname, event->name) == 0)
            {
                if (level_file_version > 0)
                {
                    SCRPTLOG("Changing computer %d event '%s' config from (%d,%d,%d,%d,%d) to (%d,%d,%d,%d,%d)",
                        (int)i, event->name,
                        (int)event->test_interval, (int)event->primary_parameter, (int)event->secondary_parameter, (int)event->tertiary_parameter, (int)event->last_test_gameturn,
                        (int)test_interval, (int)primary_parameter, (int)secondary_parameter, (int)tertiary_parameter, (int)last_test_gameturn);
                    event->test_interval = test_interval;
                    event->primary_parameter = primary_parameter;
                    event->secondary_parameter = secondary_parameter;
                    event->tertiary_parameter = tertiary_parameter;
                    event->last_test_gameturn = last_test_gameturn;
                    n++;
                }
                else
                {
                    SCRPTLOG("Changing computer %d event '%s' config from (%d,%d) to (%d,%d)", (int)i, event->name,
                        (int)event->primary_parameter, (int)event->secondary_parameter, (int)test_interval, (int)primary_parameter);
                    event->primary_parameter = test_interval;
                    event->secondary_parameter = primary_parameter;
                    n++;
                }
            }
        }
    }
    if (n == 0)
    {
        SCRPTERRLOG("No computer event found named '%s' in players %d to %d", evntname, (int)plr_start, (int)plr_end - 1);
        return;
    }
    SCRIPTDBG(6, "Altered %ld events named '%s'", n, evntname);
}

static void swap_creature_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    ThingModel ncrt_id = scline->np[0];
    ThingModel crtr_id = scline->np[1];

    value->shorts[0] = ncrt_id;
    value->shorts[1] = crtr_id;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void swap_creature_process(struct ScriptContext* context)
{
    ThingModel ncrt_id = context->value->shorts[0];
    ThingModel crtr_id = context->value->shorts[1];

    if (!swap_creature(ncrt_id, crtr_id))
    {
        SCRPTERRLOG("Error swapping creatures '%s'<->'%s'", creature_code_name(ncrt_id), creature_code_name(crtr_id));
    }
}

static void set_digger_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[0]);
    ThingModel crtr_id = get_rid(creature_desc, scline->tp[1]);

    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[1]);
        return;
    }

    value->shorts[0] = crtr_id;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_digger_process(struct ScriptContext* context)
{
    ThingModel new_dig_model = context->value->shorts[0];
    PlayerNumber plyr_idx = context->player_idx;

    update_players_special_digger_model(plyr_idx, new_dig_model);
}

static void set_next_level_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    short next_level = scline->np[0];
    TbBool correct = false;

    if (!is_campaign_level(game.loaded_level_number))
    {
        SCRPTERRLOG("Script command %s only functions in campaigns.", scline->tcmnd);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        if (campaign.single_levels[i] == next_level)
        {
            correct = true;
            break;
        }
    }
    if (correct == false)
    {
        SCRPTERRLOG("Cannot find level number '%d' in single levels of campaign.",next_level);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[1] = next_level;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_next_level_process(struct ScriptContext* context)
{
    intralvl.next_level = context->value->shorts[1];
}

static void show_bonus_level_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    short bonus_level = scline->np[0];

    if (!is_campaign_level(game.loaded_level_number))
    {
        SCRPTERRLOG("Script command %s only functions in campaigns.", scline->tcmnd);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    if (!is_bonus_level(bonus_level))
    {
        SCRPTERRLOG("Level %d not found as bonus level in campaign.", bonus_level);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[1] = bonus_level;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void show_bonus_level_process(struct ScriptContext* context)
{
    set_bonus_level_visibility(context->value->shorts[1], 1);
}
static void hide_bonus_level_process(struct ScriptContext* context)
{
    set_bonus_level_visibility(context->value->shorts[1], 0);
}

static void run_lua_code_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    const char* code = scline->tp[0];

    value->longs[0] = script_strdup(code);
    if (value->longs[0] < 0) {
        SCRPTERRLOG("Run out script strings space");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void run_lua_code_process(struct ScriptContext* context)
{
    const char* code = script_strval(context->value->longs[0]);
    execute_lua_code_from_script(code);
}

static void set_generate_speed_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if (scline->tp[1][0] == '\0')
    {
        if (scline->np[0] <= 0)
        {
            SCRPTERRLOG("Generation speed must be positive number");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        value->chars[2] = ALL_PLAYERS;
    }
    else
    {
        if (scline->np[0] < 0)
        {
            SCRPTERRLOG("Generation speed must be positive number");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        value->chars[2] = get_id(player_desc, scline->tp[1]);
        if (value->chars[2] == -1)
        {
            SCRPTERRLOG("Invalid player: %d", value->chars[2]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    value->ushorts[0] = saturate_set_unsigned(scline->np[0], 16);
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_generate_speed_process(struct ScriptContext* context)
{
    struct PlayerInfo* player;
    switch (context->value->chars[2])
    {
        case ALL_PLAYERS:
        {
            for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
            {
                player = get_player(plyr_idx);
                if (!player_invalid(player))
                {
                    player->generate_speed = context->value->ushorts[0];
                }
            }
            break;
        }
        default:
        {
            player = get_player(context->value->chars[2]);
            if (!player_invalid(player))
            {
                player->generate_speed = context->value->ushorts[0];
            }
            break;
        }
    }
    update_dungeon_generation_speeds();
}

static void tutorial_flash_button_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long id;
    if (level_file_version > 0)
    {
        if (parameter_is_number(scline->tp[0]))
        {
            id = atoi(scline->tp[0]);
            value->shorts[0] = GID_NONE;
        }
        else
        {
            static const struct NamedCommand *desc[4] = {room_desc, power_desc, trap_desc, door_desc};
            static const short btn_group[4] = {GID_ROOM_PANE, GID_POWER_PANE, GID_TRAP_PANE, GID_DOOR_PANE};
            for (int i = 0; i < 4; i++)
            {
                id = get_rid(desc[i], scline->tp[0]);
                if (id >= 0)
                {
                    value->shorts[0] = btn_group[i];
                    break;
                }
            }
            if (id < 0)
            {
                SCRPTERRLOG("Unrecognised parameter: %s", scline->tp[0]);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
        }
    }
    else
    {
        id = scline->np[0];
    }
    if (id < 0)
    {
        SCRPTERRLOG("Button ID must be positive number");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[1] = saturate_set_signed(id, 16);
    value->longs[1] = scline->np[1];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void tutorial_flash_button_process(struct ScriptContext* context)
{
    if (level_file_version > 0)
    {
        if (context->value->shorts[0] > GID_NONE)
        {
            short button_id = get_button_designation(context->value->shorts[0], context->value->shorts[1]);
            if (button_id >= 0)
            {
                gui_set_button_flashing(button_id, context->value->longs[1]);
            }
        }
        else
        {
            gui_set_button_flashing(context->value->shorts[1], context->value->longs[1]);
        }
    }
    else
    {
        gui_set_button_flashing(context->value->shorts[1], context->value->longs[1]);
    }
}

/**
 * Descriptions of script commands for parser.
 * Arguments are: A-string, N-integer, C-creature model, P-player, R-room kind, L-location, O-operator, S-slab kind, B-boolean
 * Lower case letters are optional arguments, Exclamation points sets 'extended' option, for example 'ANY_CREATURE' for creatures.
 */
const struct CommandDesc command_desc[] = {
  {"CREATE_PARTY",                      "A       ", Cmd_CREATE_PARTY, NULL, NULL},
  {"ADD_TO_PARTY",                      "ACNNAN  ", Cmd_ADD_TO_PARTY, &add_to_party_check, NULL},
  {"DELETE_FROM_PARTY",                 "ACN     ", Cmd_DELETE_FROM_PARTY, &delete_from_party_check, NULL},
  {"ADD_PARTY_TO_LEVEL",                "PAAN    ", Cmd_ADD_PARTY_TO_LEVEL, NULL, NULL},
  {"ADD_CREATURE_TO_LEVEL",             "PCANNNa ", Cmd_ADD_CREATURE_TO_LEVEL, NULL, NULL},
  {"ADD_OBJECT_TO_LEVEL",               "AANpa   ", Cmd_ADD_OBJECT_TO_LEVEL, &add_object_to_level_check, &add_object_to_level_process},
  {"IF",                                "PAOAa   ", Cmd_IF, &if_check, NULL},
  {"IF_ACTION_POINT",                   "NP      ", Cmd_IF_ACTION_POINT, NULL, NULL},
  {"ENDIF",                             "        ", Cmd_ENDIF, NULL, NULL},
  {"SET_GENERATE_SPEED",                "Np      ", Cmd_SET_GENERATE_SPEED, &set_generate_speed_check, &set_generate_speed_process},
  {"REM",                               "        ", Cmd_REM, NULL, NULL},
  {"START_MONEY",                       "PN      ", Cmd_START_MONEY, NULL, NULL},
  {"ROOM_AVAILABLE",                    "PRNN    ", Cmd_ROOM_AVAILABLE, NULL, NULL},
  {"CREATURE_AVAILABLE",                "PCNN    ", Cmd_CREATURE_AVAILABLE, NULL, NULL},
  {"MAGIC_AVAILABLE",                   "PANN    ", Cmd_MAGIC_AVAILABLE, NULL, NULL},
  {"TRAP_AVAILABLE",                    "PANN    ", Cmd_TRAP_AVAILABLE, NULL, NULL},
  {"RESEARCH",                          "PAAN    ", Cmd_RESEARCH, NULL, NULL},
  {"RESEARCH_ORDER",                    "PAAN    ", Cmd_RESEARCH_ORDER, NULL, NULL},
  {"COMPUTER_PLAYER",                   "PA      ", Cmd_COMPUTER_PLAYER, &computer_player_check, &computer_player_process},
  {"SET_TIMER",                         "PA      ", Cmd_SET_TIMER, NULL, NULL},
  {"ADD_TUNNELLER_TO_LEVEL",            "PAANNN  ", Cmd_ADD_TUNNELLER_TO_LEVEL, NULL, NULL},
  {"WIN_GAME",                          "        ", Cmd_WIN_GAME, NULL, NULL},
  {"LOSE_GAME",                         "        ", Cmd_LOSE_GAME, NULL, NULL},
  {"SET_FLAG",                          "PAN     ", Cmd_SET_FLAG, NULL, NULL},
  {"MAX_CREATURES",                     "PN      ", Cmd_MAX_CREATURES, NULL, NULL},
  {"NEXT_COMMAND_REUSABLE",             "        ", Cmd_NEXT_COMMAND_REUSABLE, NULL, NULL},
  {"DOOR_AVAILABLE",                    "PANN    ", Cmd_DOOR_AVAILABLE, NULL, NULL},
  {"DISPLAY_OBJECTIVE",                 "Nl      ", Cmd_DISPLAY_OBJECTIVE, &display_objective_check, &display_objective_process},
  {"DISPLAY_OBJECTIVE_WITH_POS",        "NNN     ", Cmd_DISPLAY_OBJECTIVE_WITH_POS, &display_objective_check, &display_objective_process},
  {"DISPLAY_INFORMATION",               "Nl      ", Cmd_DISPLAY_INFORMATION, NULL, NULL},
  {"DISPLAY_INFORMATION_WITH_POS",      "NNN     ", Cmd_DISPLAY_INFORMATION_WITH_POS, NULL, NULL},
  {"ADD_TUNNELLER_PARTY_TO_LEVEL",      "PAAANNN ", Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL, NULL, NULL},
  {"ADD_CREATURE_TO_POOL",              "CN      ", Cmd_ADD_CREATURE_TO_POOL, NULL, NULL},
  {"RESET_ACTION_POINT",                "Na      ", Cmd_RESET_ACTION_POINT, &reset_action_point_check, &reset_action_point_process},
  {"SET_CREATURE_MAX_LEVEL",            "PC!N    ", Cmd_SET_CREATURE_MAX_LEVEL, &set_creature_max_level_check, &set_creature_max_level_process},
  {"SET_MUSIC",                         "A       ", Cmd_SET_MUSIC, &set_music_check, &set_music_process},
  {"TUTORIAL_FLASH_BUTTON",             "AN      ", Cmd_TUTORIAL_FLASH_BUTTON, &tutorial_flash_button_check, &tutorial_flash_button_process},
  {"SET_CREATURE_STRENGTH",             "CN      ", Cmd_SET_CREATURE_STRENGTH, NULL, NULL},
  {"SET_CREATURE_HEALTH",               "CN      ", Cmd_SET_CREATURE_HEALTH, NULL, NULL},
  {"SET_CREATURE_ARMOUR",               "CN      ", Cmd_SET_CREATURE_ARMOUR, NULL, NULL},
  {"SET_CREATURE_FEAR_WOUNDED",         "CN      ", Cmd_SET_CREATURE_FEAR_WOUNDED, NULL, NULL},
  {"SET_CREATURE_FEAR_STRONGER",        "CN      ", Cmd_SET_CREATURE_FEAR_STRONGER, NULL, NULL},
  {"SET_CREATURE_FEARSOME_FACTOR",      "CN      ", Cmd_SET_CREATURE_FEARSOME_FACTOR, NULL, NULL},
  {"SET_CREATURE_PROPERTY",             "CAB     ", Cmd_SET_CREATURE_PROPERTY, NULL, NULL},
  {"IF_AVAILABLE",                      "PAOAa   ", Cmd_IF_AVAILABLE, &if_available_check, NULL},
  {"IF_CONTROLS",                       "PAOAa   ", Cmd_IF_CONTROLS,  &if_controls_check, NULL},
  {"SET_COMPUTER_GLOBALS",              "PNNNNNNn", Cmd_SET_COMPUTER_GLOBALS, &set_computer_globals_check, &set_computer_globals_process},
  {"SET_COMPUTER_CHECKS",               "PANNNNN ", Cmd_SET_COMPUTER_CHECKS, &set_computer_checks_check, &set_computer_checks_process},
  {"SET_COMPUTER_EVENT",                "PANNNNN ", Cmd_SET_COMPUTER_EVENT, &set_computer_event_check, &set_computer_event_process},
  {"SET_COMPUTER_PROCESS",              "PANNNNN ", Cmd_SET_COMPUTER_PROCESS, &set_computer_process_check, &set_computer_process_process},
  {"ALLY_PLAYERS",                      "PPN     ", Cmd_ALLY_PLAYERS, NULL, NULL},
  {"DEAD_CREATURES_RETURN_TO_POOL",     "B       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL, NULL, NULL},
  {"BONUS_LEVEL_TIME",                  "Nb      ", Cmd_BONUS_LEVEL_TIME, NULL, NULL},
  {"QUICK_OBJECTIVE",                   "NAl     ", Cmd_QUICK_OBJECTIVE, NULL, NULL},
  {"QUICK_INFORMATION",                 "NAl     ", Cmd_QUICK_INFORMATION, NULL, NULL},
  {"QUICK_OBJECTIVE_WITH_POS",          "NANN    ", Cmd_QUICK_OBJECTIVE_WITH_POS, NULL, NULL},
  {"QUICK_INFORMATION_WITH_POS",        "NANN    ", Cmd_QUICK_INFORMATION_WITH_POS, NULL, NULL},
  {"SWAP_CREATURE",                     "CC      ", Cmd_SWAP_CREATURE, &swap_creature_check, &swap_creature_process},
  {"PRINT",                             "A       ", Cmd_PRINT, NULL, NULL},
  {"MESSAGE",                           "A       ", Cmd_MESSAGE, NULL, NULL},
  {"PLAY_MESSAGE",                      "PAA     ", Cmd_PLAY_MESSAGE, &play_message_check, &play_message_process},
  {"ADD_GOLD_TO_PLAYER",                "PN      ", Cmd_ADD_GOLD_TO_PLAYER, NULL, NULL},
  {"SET_CREATURE_TENDENCIES",           "PAB     ", Cmd_SET_CREATURE_TENDENCIES, NULL, NULL},
  {"REVEAL_MAP_RECT",                   "PNNNN   ", Cmd_REVEAL_MAP_RECT, NULL, NULL},
  {"CONCEAL_MAP_RECT",                  "PNNNNb! ", Cmd_CONCEAL_MAP_RECT, &conceal_map_rect_check, &conceal_map_rect_process},
  {"REVEAL_MAP_LOCATION",               "PLN     ", Cmd_REVEAL_MAP_LOCATION, &reveal_map_location_check, &reveal_map_location_process},
  {"TAG_MAP_RECT",                      "PNNnn   ", Cmd_TAG_MAP_RECT, &tag_map_rect_check, &tag_map_rect_process},
  {"UNTAG_MAP_RECT",                    "PNNnn   ", Cmd_UNTAG_MAP_RECT, &tag_map_rect_check, &untag_map_rect_process},
  {"LEVEL_VERSION",                     "N       ", Cmd_LEVEL_VERSION, NULL, NULL},
  {"KILL_CREATURE",                     "PC!AN   ", Cmd_KILL_CREATURE, NULL, NULL},
  {"COMPUTER_DIG_TO_LOCATION",          "PLL     ", Cmd_COMPUTER_DIG_TO_LOCATION, NULL, NULL},
  {"USE_POWER_ON_CREATURE",             "PC!APANA", Cmd_USE_POWER_ON_CREATURE, NULL, NULL},
  {"USE_POWER_ON_PLAYERS_CREATURES",    "PC!PANB!", Cmd_USE_POWER_ON_PLAYERS_CREATURES, &use_power_on_players_creatures_check, &use_power_on_players_creatures_process},
  {"USE_POWER_AT_POS",                  "PNNANA  ", Cmd_USE_POWER_AT_POS, NULL, NULL},
  {"USE_POWER_AT_LOCATION",             "PLANA   ", Cmd_USE_POWER_AT_LOCATION, NULL, NULL},
  {"USE_POWER",                         "PAA     ", Cmd_USE_POWER, NULL, NULL},
  {"USE_SPECIAL_INCREASE_LEVEL",        "PN      ", Cmd_USE_SPECIAL_INCREASE_LEVEL, NULL, NULL},
  {"USE_SPECIAL_MULTIPLY_CREATURES",    "PN      ", Cmd_USE_SPECIAL_MULTIPLY_CREATURES, NULL, NULL},
  {"MAKE_SAFE",                         "P       ", Cmd_MAKE_SAFE, NULL, NULL},
  {"USE_SPECIAL_MAKE_SAFE",             "P       ", Cmd_MAKE_SAFE, NULL, NULL}, // Legacy command
  {"LOCATE_HIDDEN_WORLD",               "        ", Cmd_LOCATE_HIDDEN_WORLD, NULL, NULL},
  {"USE_SPECIAL_LOCATE_HIDDEN_WORLD",   "        ", Cmd_LOCATE_HIDDEN_WORLD, NULL, NULL}, // Legacy command
  {"USE_SPECIAL_TRANSFER_CREATURE",     "P       ", Cmd_USE_SPECIAL_TRANSFER_CREATURE, &special_transfer_creature_check, &special_transfer_creature_process},
  {"TRANSFER_CREATURE",                 "PC!An   ", Cmd_TRANSFER_CREATURE, &script_transfer_creature_check, &script_transfer_creature_process},
  {"CHANGE_CREATURES_ANNOYANCE",        "PC!AN   ", Cmd_CHANGE_CREATURES_ANNOYANCE, &change_creatures_annoyance_check, &change_creatures_annoyance_process},
  {"ADD_TO_FLAG",                       "PAN     ", Cmd_ADD_TO_FLAG, NULL, NULL},
  {"SET_CAMPAIGN_FLAG",                 "PAN     ", Cmd_SET_CAMPAIGN_FLAG, NULL, NULL},
  {"ADD_TO_CAMPAIGN_FLAG",              "PAN     ", Cmd_ADD_TO_CAMPAIGN_FLAG, NULL, NULL},
  {"EXPORT_VARIABLE",                   "PAA     ", Cmd_EXPORT_VARIABLE, NULL, NULL},
  {"RUN_AFTER_VICTORY",                 "B       ", Cmd_RUN_AFTER_VICTORY, NULL, NULL},
  {"SET_NEXT_LEVEL",                    "N       ", Cmd_SET_NEXT_LEVEL, &set_next_level_check, &set_next_level_process},
  {"SHOW_BONUS_LEVEL",                  "N       ", Cmd_SHOW_BONUS_LEVEL, &show_bonus_level_check, &show_bonus_level_process},
  {"HIDE_BONUS_LEVEL",                  "N       ", Cmd_HIDE_BONUS_LEVEL, &show_bonus_level_check, &hide_bonus_level_process},
  {"LEVEL_UP_CREATURE",                 "PC!AN   ", Cmd_LEVEL_UP_CREATURE, NULL, NULL},
  {"LEVEL_UP_PLAYERS_CREATURES",        "PC!n    ", Cmd_LEVEL_UP_PLAYERS_CREATURES, &level_up_players_creatures_check, level_up_players_creatures_process},
  {"CHANGE_CREATURE_OWNER",             "PC!AP   ", Cmd_CHANGE_CREATURE_OWNER, NULL, NULL},
  {"SET_GAME_RULE",                     "AAa     ", Cmd_SET_GAME_RULE, &set_game_rule_check, &set_game_rule_process},
  {"SET_ROOM_CONFIGURATION",            "AAAan   ", Cmd_SET_ROOM_CONFIGURATION, &set_room_configuration_check, &set_room_configuration_process},
  {"SET_TRAP_CONFIGURATION",            "AAAnnn  ", Cmd_SET_TRAP_CONFIGURATION, &set_trap_configuration_check, &set_trap_configuration_process},
  {"SET_DOOR_CONFIGURATION",            "AAAn    ", Cmd_SET_DOOR_CONFIGURATION, &set_door_configuration_check, &set_door_configuration_process},
  {"SET_OBJECT_CONFIGURATION",          "AAAnnn  ", Cmd_SET_OBJECT_CONFIGURATION, &set_object_configuration_check, &set_object_configuration_process},
  {"SET_CREATURE_CONFIGURATION",        "CAAaaaaa", Cmd_SET_CREATURE_CONFIGURATION, &set_creature_configuration_check, &set_creature_configuration_process},
  {"SET_SACRIFICE_RECIPE",              "AAA+    ", Cmd_SET_SACRIFICE_RECIPE, &set_sacrifice_recipe_check, &set_sacrifice_recipe_process},
  {"REMOVE_SACRIFICE_RECIPE",           "A+      ", Cmd_REMOVE_SACRIFICE_RECIPE, &remove_sacrifice_recipe_check, &set_sacrifice_recipe_process},
  {"SET_BOX_TOOLTIP",                   "NA      ", Cmd_SET_BOX_TOOLTIP, &set_box_tooltip_check, &set_box_tooltip_process},
  {"SET_BOX_TOOLTIP_ID",                "NN      ", Cmd_SET_BOX_TOOLTIP_ID, &set_box_tooltip_id_check, &set_box_tooltip_id_process},
  {"CHANGE_SLAB_OWNER",                 "NNPa    ", Cmd_CHANGE_SLAB_OWNER, &change_slab_owner_check, &change_slab_owner_process},
  {"CHANGE_SLAB_TYPE",                  "NNSa    ", Cmd_CHANGE_SLAB_TYPE, &change_slab_type_check, &change_slab_type_process},
  {"CREATE_EFFECTS_LINE",               "LLNNNA  ", Cmd_CREATE_EFFECTS_LINE, &create_effects_line_check, &create_effects_line_process},
  {"IF_SLAB_OWNER",                     "NNP     ", Cmd_IF_SLAB_OWNER, NULL, NULL},
  {"IF_SLAB_TYPE",                      "NNS     ", Cmd_IF_SLAB_TYPE, NULL, NULL},
  {"QUICK_MESSAGE",                     "NAA     ", Cmd_QUICK_MESSAGE, &quick_message_check, &quick_message_process},
  {"DISPLAY_MESSAGE",                   "NA      ", Cmd_DISPLAY_MESSAGE, &display_message_check, &display_message_process},
  {"CLEAR_MESSAGE",                     "n       ", Cmd_CLEAR_MESSAGE, &clear_message_check, &clear_message_process},
  {"USE_SPELL_ON_CREATURE",             "PC!AAn  ", Cmd_USE_SPELL_ON_CREATURE, &use_spell_on_creature_check, &use_spell_on_creature_process},
  {"USE_SPELL_ON_PLAYERS_CREATURES",    "PC!An   ", Cmd_USE_SPELL_ON_PLAYERS_CREATURES, &use_spell_on_players_creatures_check, &use_spell_on_players_creatures_process},
  {"SET_HEART_HEALTH",                  "PN      ", Cmd_SET_HEART_HEALTH, &set_heart_health_check, &set_heart_health_process},
  {"ADD_HEART_HEALTH",                  "PNb     ", Cmd_ADD_HEART_HEALTH, &add_heart_health_check, &add_heart_health_process},
  {"CREATURE_ENTRANCE_LEVEL",           "PN      ", Cmd_CREATURE_ENTRANCE_LEVEL, NULL, NULL},
  {"RANDOMISE_FLAG",                    "PAn     ", Cmd_RANDOMISE_FLAG, NULL, NULL},
  {"RANDOMIZE_FLAG",                    "PAn     ", Cmd_RANDOMISE_FLAG, NULL, NULL},
  {"COMPUTE_FLAG",                      "PAAPAb  ", Cmd_COMPUTE_FLAG, NULL, NULL},
  {"DISPLAY_TIMER",                     "PAb     ", Cmd_DISPLAY_TIMER, &display_timer_check, &display_timer_process},
  {"ADD_TO_TIMER",                      "PAN     ", Cmd_ADD_TO_TIMER, &add_to_timer_check, &add_to_timer_process},
  {"ADD_BONUS_TIME",                    "N       ", Cmd_ADD_BONUS_TIME, &add_bonus_time_check, &add_bonus_time_process},
  {"DISPLAY_VARIABLE",                  "PAnn    ", Cmd_DISPLAY_VARIABLE, &display_variable_check, &display_variable_process},
  {"DISPLAY_COUNTDOWN",                 "PANb    ", Cmd_DISPLAY_COUNTDOWN, &display_countdown_check, &display_timer_process},
  {"HIDE_TIMER",                        "        ", Cmd_HIDE_TIMER, &cmd_no_param_check, &hide_timer_process},
  {"HIDE_VARIABLE",                     "        ", Cmd_HIDE_VARIABLE, &cmd_no_param_check, &hide_variable_process},
  {"CREATE_EFFECT",                     "AAn     ", Cmd_CREATE_EFFECT, &create_effect_check, &create_effect_process},
  {"CREATE_EFFECT_AT_POS",              "ANNn    ", Cmd_CREATE_EFFECT_AT_POS, &create_effect_at_pos_check, &create_effect_at_pos_process},
  {"HEART_LOST_QUICK_OBJECTIVE",        "NAl     ", Cmd_HEART_LOST_QUICK_OBJECTIVE, &heart_lost_quick_objective_check, &heart_lost_quick_objective_process},
  {"HEART_LOST_OBJECTIVE",              "Nl      ", Cmd_HEART_LOST_OBJECTIVE, &heart_lost_objective_check, &heart_lost_objective_process},
  {"SET_DOOR",                          "ANN     ", Cmd_SET_DOOR, &set_door_check, &set_door_process},
  {"PLACE_DOOR",                        "PANNb!b!", Cmd_PLACE_DOOR, &place_door_check, &place_door_process},
  {"PLACE_TRAP",                        "PANNb!  ", Cmd_PLACE_TRAP, &place_trap_check, &place_trap_process },
  {"ZOOM_TO_LOCATION",                  "PL      ", Cmd_MOVE_PLAYER_CAMERA_TO, &player_zoom_to_check, &player_zoom_to_process},
  {"SET_CREATURE_INSTANCE",             "CNAN    ", Cmd_SET_CREATURE_INSTANCE, &set_creature_instance_check, &set_creature_instance_process},
  {"SET_HAND_RULE",                     "PC!Aaaa ", Cmd_SET_HAND_RULE, &set_hand_rule_check, &set_hand_rule_process},
  {"MOVE_CREATURE",                     "PC!ANLa ", Cmd_MOVE_CREATURE, &move_creature_check, &move_creature_process},
  {"COUNT_CREATURES_AT_ACTION_POINT",   "NPC!PA  ", Cmd_COUNT_CREATURES_AT_ACTION_POINT, &count_creatures_at_action_point_check, &count_creatures_at_action_point_process},
  {"IF_ALLIED",                         "PPON    ", Cmd_IF_ALLIED, &if_allied_check, NULL},
  {"SET_TEXTURE",                       "PA      ", Cmd_SET_TEXTURE, &set_texture_check, &set_texture_process},
  {"HIDE_HERO_GATE",                    "NB      ", Cmd_HIDE_HERO_GATE, &hide_hero_gate_check, &hide_hero_gate_process},
  {"NEW_TRAP_TYPE",                     "A       ", Cmd_NEW_TRAP_TYPE, &new_trap_type_check, &null_process},
  {"NEW_OBJECT_TYPE",                   "A       ", Cmd_NEW_OBJECT_TYPE, &new_object_type_check, &null_process},
  {"NEW_ROOM_TYPE",                     "A       ", Cmd_NEW_ROOM_TYPE, &new_room_type_check, &null_process},
  {"NEW_CREATURE_TYPE",                 "A       ", Cmd_NEW_CREATURE_TYPE, &new_creature_type_check, &null_process},
  {"SET_HAND_GRAPHIC",                  "PA      ", Cmd_SET_HAND_GRAPHIC, &set_power_hand_check, &set_power_hand_process},
  {"ADD_EFFECT_GENERATOR_TO_LEVEL",     "AAN     ", Cmd_ADD_EFFECT_GENERATOR_TO_LEVEL, &add_effectgen_to_level_check, &add_effectgen_to_level_process},
  {"SET_EFFECT_GENERATOR_CONFIGURATION","AAAnn   ", Cmd_SET_EFFECT_GENERATOR_CONFIGURATION, &set_effectgen_configuration_check, &set_effectgen_configuration_process},
  {"SET_POWER_CONFIGURATION",           "AAAa    ", Cmd_SET_POWER_CONFIGURATION, &set_power_configuration_check, &set_power_configuration_process},
  {"SET_PLAYER_COLOR",                  "PA      ", Cmd_SET_PLAYER_COLOUR, &set_player_colour_check, &set_player_colour_process},
  {"SET_PLAYER_COLOUR",                 "PA      ", Cmd_SET_PLAYER_COLOUR, &set_player_colour_check, &set_player_colour_process},
  {"MAKE_UNSAFE",                       "P       ", Cmd_MAKE_UNSAFE, NULL, NULL},
  {"SET_INCREASE_ON_EXPERIENCE",        "AN      ", Cmd_SET_INCREASE_ON_EXPERIENCE, &set_increase_on_experience_check, &set_increase_on_experience_process},
  {"SET_PLAYER_MODIFIER",               "PAN     ", Cmd_SET_PLAYER_MODIFIER, &set_player_modifier_check, &set_player_modifier_process},
  {"ADD_TO_PLAYER_MODIFIER",            "PAN     ", Cmd_ADD_TO_PLAYER_MODIFIER, &add_to_player_modifier_check, &add_to_player_modifier_process},
  {"CHANGE_SLAB_TEXTURE",               "NNAa    ", Cmd_CHANGE_SLAB_TEXTURE , &change_slab_texture_check, &change_slab_texture_process},
  {"ADD_OBJECT_TO_LEVEL_AT_POS",        "ANNNpa  ", Cmd_ADD_OBJECT_TO_LEVEL_AT_POS, &add_object_to_level_at_pos_check, &add_object_to_level_at_pos_process},
  {"LOCK_POSSESSION",                   "PB!     ", Cmd_LOCK_POSSESSION, &lock_possession_check, &lock_possession_process},
  {"SET_DIGGER",                        "PC      ", Cmd_SET_DIGGER , &set_digger_check, &set_digger_process},
  {"RUN_LUA_CODE",                      "A       ", Cmd_RUN_LUA_CODE , &run_lua_code_check, &run_lua_code_process},
  {NULL,                                "        ", Cmd_NONE, NULL, NULL},
};

const struct CommandDesc dk1_command_desc[] = {
  {"CREATE_PARTY",                 "A       ", Cmd_CREATE_PARTY, NULL, NULL},
  {"ADD_TO_PARTY",                 "ACNNAN  ", Cmd_ADD_TO_PARTY, &add_to_party_check, NULL},
  {"ADD_PARTY_TO_LEVEL",           "PAAN    ", Cmd_ADD_PARTY_TO_LEVEL, NULL, NULL},
  {"ADD_CREATURE_TO_LEVEL",        "PCANNN  ", Cmd_ADD_CREATURE_TO_LEVEL, NULL, NULL},
  {"IF",                           "PAOAa   ", Cmd_IF, &if_check, NULL},
  {"IF_ACTION_POINT",              "NP      ", Cmd_IF_ACTION_POINT, NULL, NULL},
  {"ENDIF",                        "        ", Cmd_ENDIF, NULL, NULL},
  {"SET_GENERATE_SPEED",           "N       ", Cmd_SET_GENERATE_SPEED, &set_generate_speed_check, &set_generate_speed_process},
  {"REM",                          "        ", Cmd_REM, NULL, NULL},
  {"START_MONEY",                  "PN      ", Cmd_START_MONEY, NULL, NULL},
  {"ROOM_AVAILABLE",               "PRNN    ", Cmd_ROOM_AVAILABLE, NULL, NULL},
  {"CREATURE_AVAILABLE",           "PCNN    ", Cmd_CREATURE_AVAILABLE, NULL, NULL},
  {"MAGIC_AVAILABLE",              "PANN    ", Cmd_MAGIC_AVAILABLE, NULL, NULL},
  {"TRAP_AVAILABLE",               "PANN    ", Cmd_TRAP_AVAILABLE, NULL, NULL},
  {"RESEARCH",                     "PAAN    ", Cmd_RESEARCH_ORDER, NULL, NULL},
  {"COMPUTER_PLAYER",              "PN      ", Cmd_COMPUTER_PLAYER, computer_player_check, computer_player_process},
  {"SET_TIMER",                    "PA      ", Cmd_SET_TIMER, NULL, NULL},
  {"ADD_TUNNELLER_TO_LEVEL",       "PAANNN  ", Cmd_ADD_TUNNELLER_TO_LEVEL, NULL, NULL},
  {"WIN_GAME",                     "        ", Cmd_WIN_GAME, NULL, NULL},
  {"LOSE_GAME",                    "        ", Cmd_LOSE_GAME, NULL, NULL},
  {"SET_FLAG",                     "PAN     ", Cmd_SET_FLAG, NULL, NULL},
  {"MAX_CREATURES",                "PN      ", Cmd_MAX_CREATURES, NULL, NULL},
  {"NEXT_COMMAND_REUSABLE",        "        ", Cmd_NEXT_COMMAND_REUSABLE, NULL, NULL},
  {"DOOR_AVAILABLE",               "PANN    ", Cmd_DOOR_AVAILABLE, NULL, NULL},
  {"DISPLAY_OBJECTIVE",            "NA      ", Cmd_DISPLAY_OBJECTIVE, &display_objective_check, &display_objective_process},
  {"DISPLAY_OBJECTIVE_WITH_POS",   "NNN     ", Cmd_DISPLAY_OBJECTIVE_WITH_POS, &display_objective_check, &display_objective_process},
  {"DISPLAY_INFORMATION",          "N       ", Cmd_DISPLAY_INFORMATION, NULL, NULL},
  {"DISPLAY_INFORMATION_WITH_POS", "NNN     ", Cmd_DISPLAY_INFORMATION_WITH_POS, NULL, NULL},
  {"ADD_TUNNELLER_PARTY_TO_LEVEL", "PAAANNN ", Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL, NULL, NULL},
  {"ADD_CREATURE_TO_POOL",         "CN      ", Cmd_ADD_CREATURE_TO_POOL, NULL, NULL},
  {"RESET_ACTION_POINT",           "N       ", Cmd_RESET_ACTION_POINT, &reset_action_point_check, &reset_action_point_process},
  {"SET_CREATURE_MAX_LEVEL",       "PC!N    ", Cmd_SET_CREATURE_MAX_LEVEL, &set_creature_max_level_check, &set_creature_max_level_process},
  {"SET_MUSIC",                    "N       ", Cmd_SET_MUSIC, NULL, NULL},
  {"TUTORIAL_FLASH_BUTTON",        "NN      ", Cmd_TUTORIAL_FLASH_BUTTON, &tutorial_flash_button_check, &tutorial_flash_button_process},
  {"SET_CREATURE_STRENGTH",        "CN      ", Cmd_SET_CREATURE_STRENGTH, NULL, NULL},
  {"SET_CREATURE_HEALTH",          "CN      ", Cmd_SET_CREATURE_HEALTH, NULL, NULL},
  {"SET_CREATURE_ARMOUR",          "CN      ", Cmd_SET_CREATURE_ARMOUR, NULL, NULL},
  {"SET_CREATURE_FEAR",            "CN      ", Cmd_SET_CREATURE_FEAR_WOUNDED, NULL, NULL},
  {"IF_AVAILABLE",                 "PAOAa   ", Cmd_IF_AVAILABLE, &if_available_check, NULL},
  {"SET_COMPUTER_GLOBALS",         "PNNNNNN ", Cmd_SET_COMPUTER_GLOBALS, &set_computer_globals_check, &set_computer_globals_process},
  {"SET_COMPUTER_CHECKS",          "PANNNNN ", Cmd_SET_COMPUTER_CHECKS, &set_computer_checks_check, &set_computer_checks_process},
  {"SET_COMPUTER_EVENT",           "PANN    ", Cmd_SET_COMPUTER_EVENT, &set_computer_event_check, &set_computer_event_process},
  {"SET_COMPUTER_PROCESS",         "PANNNNN ", Cmd_SET_COMPUTER_PROCESS, &set_computer_process_check, &set_computer_process_process},
  {"ALLY_PLAYERS",                 "PP      ", Cmd_ALLY_PLAYERS, NULL, NULL},
  {"DEAD_CREATURES_RETURN_TO_POOL","N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL, NULL, NULL},
  {"BONUS_LEVEL_TIME",             "N       ", Cmd_BONUS_LEVEL_TIME, NULL, NULL},
  {"QUICK_OBJECTIVE",              "NAA     ", Cmd_QUICK_OBJECTIVE, NULL, NULL},
  {"QUICK_INFORMATION",            "NA      ", Cmd_QUICK_INFORMATION, NULL, NULL},
  {"SWAP_CREATURE",                "CC      ", Cmd_SWAP_CREATURE, &swap_creature_check, &swap_creature_process},
  {"PRINT",                        "A       ", Cmd_PRINT, NULL, NULL},
  {"MESSAGE",                      "A       ", Cmd_MESSAGE, NULL, NULL},
  {"LEVEL_VERSION",                "N       ", Cmd_LEVEL_VERSION, NULL, NULL},
  {NULL,                           "        ", Cmd_NONE, NULL, NULL},
};

#ifdef __cplusplus
}
#endif
