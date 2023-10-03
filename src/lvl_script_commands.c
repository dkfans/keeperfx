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
#include "lvl_script_commands.h"

#include "lvl_script_conditions.h"
#include "lvl_script_lib.h"

#include <math.h>

#include "dungeon_data.h"
#include "thing_data.h"
#include "player_instances.h"
#include "keeperfx.hpp"
#include "custom_sprites.h"
#include "gui_soundmsgs.h"
#include "config_settings.h"
#include "config_effects.h"
#include "config_trapdoor.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "console_cmd.h"
#include "creature_states_pray.h"
#include "creature_states_mood.h"
#include "room_util.h"
#include "creature_instances.h"
#include "power_hand.h"
#include "power_specials.h"
#include "creature_states.h"
#include "map_blocks.h"
#include "bflib_memory.h"
#include "post_inc.h"
#include "music_player.h"
#include "bflib_sound.h"
#include "sounds.h"

#ifdef __cplusplus
extern "C" {
#endif

extern long level_file_version;



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
  {NULL,               0},
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

const struct NamedCommand trap_config_desc[] = {
  {"NameTextID",           1},
  {"TooltipTextID",        2},
  {"SymbolSprites",        3},
  {"PointerSprites",       4},
  {"PanelTabIndex",        5},
  {"Crate",                6},
  {"ManufactureLevel",     7},
  {"ManufactureRequired",  8},
  {"Shots",                9},
  {"TimeBetweenShots",    10},
  {"SellingValue",        11},
  {"Model",               12},
  {"ModelSize",           13},
  {"AnimationSpeed",      14},
  {"TriggerType",         15},
  {"ActivationType",      16},
  {"EffectType",          17},
  {"Hidden",              18},
  {"TriggerAlarm",        19},
  {"Slappable",           20},
  {"Unanimated",          21},
  {"Health",              22},
  {"Unshaded",            23},
  {"RandomStartFrame",    24},
  {"ThingSize",           25},
  {"HitType",             26},
  {"LightRadius",         27},
  {"LightIntensity",      28},
  {"LightFlags",          29},
  {"TransparencyFlags",   30},
  {"ShotVector",          31},
  {"Destructible",        32},
  {"Unstable",            33},
  {"Unsellable",          34},
  {"PlaceOnBridge",       35},
  {NULL,                   0},
};

const struct NamedCommand room_config_desc[] = {
  {"NameTextID",           1},
  {"TooltipTextID",        2},
  {"SymbolSprites",        3},
  {"PointerSprites",       4},
  {"PanelTabIndex",        5},
  {"Cost",                 6},
  {"Health",               7},
  {"CreatureCreation",     8},
  {"AmbientSndSample",     9},
  {"SlabAssign",          10},
  {"Messages",            11},
  {"Properties",          12},
  {"Roles",               13},
  {"TotalCapacity",       14},
  {"UsedCapacity",        15},
  {"StorageHeight",       16},
  {NULL,                   0},
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

const struct NamedCommand fill_desc[] = {
  {"NONE",          FillIterType_NoFill},
  {"MATCH",         FillIterType_Match},
  {"FLOOR",         FillIterType_Floor},
  {"BRIDGE",        FillIterType_FloorBridge},
  {NULL,            0},
};

const struct NamedCommand set_door_desc[] = {
  {"LOCKED", 1},
  {"UNLOCKED", 2},
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

Mix_Chunk* Ext_Sounds[];

static int sac_compare_fn(const void *ptr_a, const void *ptr_b)
{
    const char *a = (const char*)ptr_a;
    const char *b = (const char*)ptr_b;
    return *a < *b;
}



// For dynamic strings
static char* script_strdup(const char *src)
{
    char *ret = gameadd.script.next_string;
    int remain_len = sizeof(gameadd.script.strings) - (gameadd.script.next_string - gameadd.script.strings);
    if (strlen(src) >= remain_len)
    {
        return NULL;
    }
    strcpy(ret, src);
    gameadd.script.next_string += strlen(src) + 1;
    return ret;
}


/**
 * Modifies player's creatures' anger.
 * @param plyr_idx target player
 * @param anger anger value. Use double AnnoyLevel (from creature's config file) to fully piss creature. More for longer calm time
 */
TbBool script_change_creatures_annoyance(PlayerNumber plyr_idx, ThingModel crmodel, long operation, long anger)
{
    SYNCDBG(8, "Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    unsigned long k = 0;
    TbBool is_spec_digger;
    int i = dungeon->creatr_list_start;
    if ((crmodel == get_players_special_digger_model(plyr_idx)) || (crmodel == CREATURE_DIGGER))
    {
        i = dungeon->digger_list_start;
    }
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        is_spec_digger = (thing->model > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, thing->model);
        i = cctrl->players_next_creature_idx;
        // Per creature code
       
        if (thing->model == crmodel || crmodel == 0 || (!is_spec_digger && (crmodel == CREATURE_ANY)) || (is_spec_digger && (crmodel == CREATURE_DIGGER)))
        {
            i = cctrl->players_next_creature_idx;
            if (operation == SOpr_SET)
            {
                anger_set_creature_anger(thing, anger, AngR_Other);
            }
            else if (operation == SOpr_INCREASE)
            {
                anger_increase_creature_anger(thing, anger, AngR_Other);
            }
            else if (operation == SOpr_DECREASE)
            {
                anger_reduce_creature_anger(thing, -anger, AngR_Other);
            }
            else if (operation == SOpr_MULTIPLY)
            {
                anger_set_creature_anger(thing, cctrl->annoyance_level[AngR_Other] * anger, AngR_Other);
            }

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
    return true;
}

long parse_creature_name(const char *creature_name)
{
    long ret = get_rid(creature_desc, creature_name);
    if (ret == -1)
    {
        if (0 == strcasecmp(creature_name, "ANY_CREATURE"))
        {
            return CREATURE_ANY;
        }
    }
    return ret;
}

// Variables that could be set
TbBool parse_set_varib(const char *varib_name, long *varib_id, long *varib_type)
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
        if (2 == sscanf(varib_name, "BOX%ld_ACTIVATE%c", varib_id, &c) && (c == 'D'))
        {
            // activateD
            *varib_type = SVar_BOX_ACTIVATED;
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

TbBool parse_get_varib(const char *varib_name, long *varib_id, long *varib_type)
{
    char c;
    int len = 0;
    char arg[MAX_TEXT_LENGTH];

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

static void add_to_party_check(const struct ScriptLine *scline)
{
    int party_id = get_party_index_of_name(scline->tp[0]);
    if (party_id < 0)
    {
        SCRPTERRLOG("Invalid Party:%s",scline->tp[1]);
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
    long objective_id = get_rid(hero_objective_desc, scline->tp[4]);
    if (objective_id == -1)
    {
      SCRPTERRLOG("Unknown party member objective, '%s'", scline->tp[4]);
      return;
    }
  //SCRPTLOG("Party '%s' member kind %d, level %d",prtname,crtr_id,crtr_level);

    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0))
    {
        add_member_to_party(party_id, crtr_id, scline->np[2], scline->np[3], objective_id, scline->np[5]);
    } else
    {
        struct PartyTrigger* pr_trig = &gameadd.script.party_triggers[gameadd.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        pr_trig->flags = TrgF_ADD_TO_PARTY;
        pr_trig->flags |= next_command_reusable?TrgF_REUSABLE:0;
        pr_trig->party_id = party_id;
        pr_trig->creatr_id = crtr_id;
        pr_trig->crtr_level = scline->np[2];
        pr_trig->carried_gold = scline->np[3];
        pr_trig->objectv = objective_id;
        pr_trig->countdown = scline->np[5];
        pr_trig->condit_idx = get_script_current_condition();

        gameadd.script.party_triggers_num++;
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
        struct PartyTrigger* pr_trig = &gameadd.script.party_triggers[gameadd.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        pr_trig->flags = TrgF_DELETE_FROM_PARTY;
        pr_trig->flags |= next_command_reusable?TrgF_REUSABLE:0;
        pr_trig->party_id = party_id;
        pr_trig->creatr_id = creature_id;
        pr_trig->crtr_level = scline->np[2];
        pr_trig->condit_idx = get_script_current_condition();

        gameadd.script.party_triggers_num++;
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
    if ( (my_player_number >= context->plr_start) && (my_player_number < context->plr_end) )
    {
        set_general_objective(context->value->arg0,
        context->value->arg1,
        stl_num_decode_x(context->value->arg2),
        stl_num_decode_y(context->value->arg2));
    }
}

static void conceal_map_rect_check(const struct ScriptLine *scline)
{
    TbBool all = strcmp(scline->tp[5], "ALL") == 0;
    if (!all)
    {
        all = strcmp(scline->tp[5], "1") == 0;
    }
    if (!all && strcmp(scline->tp[5], "") != 0)
    {
        SCRPTWRNLOG("Hide value \"%s\" not recognized", scline->tp[5]);
    }

    command_add_value(Cmd_CONCEAL_MAP_RECT, scline->np[0], scline->np[1], scline->np[2],
                      (scline->np[4]<<16) | scline->np[3] | (all?1<<24:0));
}

static void conceal_map_rect_process(struct ScriptContext *context)
{
    long w = context->value->bytes[8];
    long h = context->value->bytes[10];

    conceal_map_area(context->value->plyr_range, context->value->arg0 - (w>>1), context->value->arg0 + (w>>1) + (w&1),
                     context->value->arg1 - (h>>1), context->value->arg1 + (h>>1) + (h&1), context->value->bytes[11]);
}

/**
 * Transfers creatures for a player
 * @param plyr_idx target player
 * @param crmodel the creature model to transfer
 * @param criteria the creature selection criterion
 * @param count the amount of units to transfer
 */
short script_transfer_creature(long plyr_idx, long crmodel, long criteria, int count)
{
    short transferred = 0;
    struct Thing* thing;
    struct DungeonAdd* dungeonadd;
    struct CreatureControl* cctrl;
    for (int i = 0; i < count; i++)
    {
        thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
        cctrl = creature_control_get_from_thing(thing);
        if ((thing_is_invalid(thing)) && (i == 0))
        {
            SYNCDBG(5, "No matching player %d creature of model %d found to transfer.", (int)plyr_idx, (int)crmodel);
            break;
        }
        
        if (add_transfered_creature(plyr_idx, thing->model, cctrl->explevel))
        {
            transferred++;
            dungeonadd = get_dungeonadd(plyr_idx);
            dungeonadd->creatures_transferred++;
            remove_thing_from_power_hand_list(thing, plyr_idx);
            struct SpecialConfigStats* specst = get_special_model_stats(SpcKind_Resurrect);
            create_used_effect_or_element(&thing->mappos, specst->effect_id, plyr_idx);
            kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects | CrDed_NotReallyDying);
        }
    }
    return transferred;
}

static void special_transfer_creature_process(struct ScriptContext* context)
{
    if ((my_player_number >= context->plr_start) && (my_player_number < context->plr_end))
    {
        struct Thing *heartng = get_player_soul_container(context->plr_start);
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
        SCRPTWRNLOG("Trying to transfer %d creatures out of a possible 255",count);
        count = 255;
    }
    command_add_value(Cmd_TRANSFER_CREATURE, scline->np[0], crtr_id, select_id, count);
}

static void script_transfer_creature_process(struct ScriptContext* context)
{
    for (int i = context->plr_start; i < context->plr_end; i++)
    {
        script_transfer_creature(i, context->value->arg0, context->value->arg1, context->value->arg2);
    }
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
    for (int i = context->plr_start; i < context->plr_end; i++)
    {
        script_change_creatures_annoyance(i, context->value->arg0, context->value->arg1, context->value->arg2);
    }
}

static void set_trap_configuration_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    const char *trapname = scline->tp[0];
    const char *valuestring = scline->tp[2];
    long newvalue;
    short trap_id = get_id(trap_desc, trapname);
    if (trap_id == -1)
    {
        SCRPTERRLOG("Unknown trap, '%s'", trapname);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    short trapvar = get_id(trap_config_desc, scline->tp[1]);
    if (trapvar == -1)
    {
        SCRPTERRLOG("Unknown trap variable");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[0] = trap_id;
    value->shorts[1] = trapvar;
    value->uarg1 = scline->np[2];
    value->shorts[4] = scline->np[3];
    value->shorts[5] = scline->np[4];
    if (trapvar == 3) // SymbolSprites
    {
        char *tmp = malloc(strlen(scline->tp[2]) + strlen(scline->tp[3]) + 3);
        // Pass two vars along as one merged val like: first\nsecond\m
        strcpy(tmp, scline->tp[2]);
        strcat(tmp, "|");
        strcat(tmp,scline->tp[3]);
        value->str2 = script_strdup(tmp); // first\0second
        value->str2[strlen(scline->tp[2])] = 0;
        free(tmp);
        if (value->str2 == NULL)
        {
            SCRPTERRLOG("Run out script strings space");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    else if ((trapvar != 4) && (trapvar != 12))  // PointerSprites && Model
    {
        if (parameter_is_number(valuestring))
        {
            newvalue = atoi(valuestring);
            if ((newvalue > LONG_MAX) || (newvalue < 0))
            {
                SCRPTERRLOG("Value out of range: %d", newvalue);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->shorts[2] = newvalue;
        }
        else if (trapvar == 6)
        {
            newvalue = get_id(object_desc, valuestring);
            if ((newvalue > SHRT_MAX) || (newvalue < 0))
            {
                SCRPTERRLOG("Unknown crate object: %s", valuestring);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->uarg1 = newvalue;
        }
        else
        {

            SCRPTERRLOG("Trap property %s needs a number value, '%s' is invalid.", scline->tp[1], scline->tp[2]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    else
    {
        value->str2 = script_strdup(scline->tp[2]);
        if (value->str2 == NULL)
        {
            SCRPTERRLOG("Run out script strings space");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    SCRIPTDBG(7, "Setting trap %s property %s to %d", trapname, scline->tp[1], value->shorts[2]);
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_room_configuration_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    const char *roomname = scline->tp[0];
    const char *valuestring = scline->tp[2];
    const char* valuestring2 = scline->tp[3];
    long newvalue;
    long newvalue2;
    short room_id = get_id(room_desc, roomname);
    if (room_id == -1)
    {
        SCRPTERRLOG("Unknown room, '%s'", roomname);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    short roomvar = get_id(room_config_desc, scline->tp[1]);
    if (roomvar == -1)
    {
        SCRPTERRLOG("Unknown room variable");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[0] = room_id;
    value->shorts[1] = roomvar;
    value->shorts[2] = scline->np[2];
    value->shorts[3] = scline->np[3];
    value->shorts[4] = scline->np[4];
    if (roomvar == 3) // SymbolSprites
    {
        char *tmp = malloc(strlen(scline->tp[2]) + strlen(scline->tp[3]) + 3);
        // Pass two vars along as one merged val like: first\nsecond\m
        strcpy(tmp, scline->tp[2]);
        strcat(tmp, "|");
        strcat(tmp,scline->tp[3]);
        value->str2 = script_strdup(tmp); // first\0second
        value->str2[strlen(scline->tp[2])] = 0;
        free(tmp);
        if (value->str2 == NULL)
        {
            SCRPTERRLOG("Run out script strings space");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    else if (roomvar == 5) // PanelTabIndex
    {
        if (parameter_is_number(valuestring))
        {
            newvalue = atoi(valuestring);
            if ((newvalue > 16) || (newvalue < 0))
            {
                SCRPTERRLOG("Value out of range: %d", newvalue);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->shorts[2] = newvalue;
        }
        else 
        {
            SCRPTERRLOG("Room property %s needs a number value, '%s' is invalid.", scline->tp[1], scline->tp[2]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    else if (roomvar == 8) // CreatureCreation
    {
        newvalue = get_id(creature_desc, valuestring);
        if (newvalue == -1)
            {
                SCRPTERRLOG("Unknown CreatureCreation variable");
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
        value->shorts[2] = newvalue;
    }
    else if (roomvar == 10) // SlabAssign
    {
        newvalue = get_id(slab_desc, valuestring);
        if (newvalue == -1)
            {
                SCRPTERRLOG("Unknown slab variable");
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
        value->shorts[2] = newvalue;
    }
    else if (roomvar == 12) // Properties
    {
        if (parameter_is_number(valuestring))
        {
            newvalue = atoi(valuestring);
            if ((newvalue >= RoCFlg_ListEnd) || (newvalue < 0))
            {
                SCRPTERRLOG("Value out of range: %d", newvalue);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->shorts[2] = newvalue;
        }
        else 
        {
            newvalue = get_id(terrain_room_properties_commands, valuestring);
            if (newvalue == -1)
                {
                    SCRPTERRLOG("Unknown Properties variable");
                    DEALLOCATE_SCRIPT_VALUE
                        return;
                }
            value->shorts[2] = newvalue;
        }
    }
    else if (roomvar == 13) // Roles
    {
        if (parameter_is_number(valuestring))
        {
            newvalue = atoi(valuestring);
            if ((newvalue > 33554431) || (newvalue < 0))
            {
                SCRPTERRLOG("Value out of range: %d", newvalue);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->uarg1 = newvalue;
        }
        else 
        {
            newvalue = get_id(room_roles_desc, valuestring);
            if (newvalue == -1)
                {
                    SCRPTERRLOG("Unknown Roles variable");
                    DEALLOCATE_SCRIPT_VALUE
                        return;
                }
            value->uarg1 = newvalue;
        }
        if (parameter_is_number(valuestring2))
        {
            newvalue2 = atoi(valuestring2);
            if ((newvalue2 > 33554431) || (newvalue2 < 0))
            {
                SCRPTERRLOG("Value out of range: %d", newvalue2);
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
            value->uarg2 = newvalue2;
        }
        else
        {
            newvalue2 = get_id(room_roles_desc, valuestring2);
            if (newvalue2 == -1)
            {
                SCRPTERRLOG("Unknown Roles variable");
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
            value->uarg2 = newvalue2;
        }
    }
    else if (roomvar == 14) // TotalCapacity
    {
        newvalue = get_id(terrain_room_total_capacity_func_type, valuestring);
        if (newvalue == -1)
            {
                SCRPTERRLOG("Unknown TotalCapacity variable '%s'", valuestring);
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
        value->shorts[2] = newvalue;
    }
    else if (roomvar == 15) // UsedCapacity
    {
        newvalue = get_id(terrain_room_used_capacity_func_type, valuestring);
        if (newvalue == -1)
            {
                SCRPTERRLOG("Unknown UsedCapacity variable '%s'", valuestring);
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
        value->shorts[2] = newvalue;

        newvalue2 = get_id(terrain_room_used_capacity_func_type, valuestring2);
        if (newvalue2 == -1)
        {
            SCRPTERRLOG("Unknown UsedCapacity variable '%s'", valuestring2);
            DEALLOCATE_SCRIPT_VALUE
                return;
        }
        value->shorts[3] = newvalue2;
    }
    else if (roomvar != 4) // NameTextID, TooltipTextID, Cost, Health, AmbientSndSample, Messages, StorageHeight
    {
        if (parameter_is_number(valuestring))
        {
            newvalue = atoi(valuestring);
            if ((newvalue > SHRT_MAX) || (newvalue < 0))
            {
                SCRPTERRLOG("Value out of range: %d", newvalue);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->shorts[2] = newvalue;
        }
        else 
        {
            SCRPTERRLOG("Room property %s needs a number value, '%s' is invalid.", scline->tp[1], scline->tp[2]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    else // PointerSprites
    {
        value->str2 = script_strdup(scline->tp[2]);
        if (value->str2 == NULL)
        {
            SCRPTERRLOG("Run out script strings space");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    SCRIPTDBG(7, "Setting room %s property %s to %d", roomname, scline->tp[1], value->shorts[2]);
    PROCESS_SCRIPT_VALUE(scline->command);
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
        SCRPTERRLOG("Bad creatures count, %d", count);
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
    value->uarg0 = location;
    value->arg1 = select_id;
    value->shorts[4] = effct_id;
    value->bytes[10] = count;
    value->bytes[11] = crmodel;

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void count_creatures_at_action_point_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, scline->np[1]);

    long crmodel = parse_creature_name(scline->tp[2]);
    if (crmodel == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", scline->tp[2]);
        return;
    }
    short ap_num = scline->np[0];
    char flag_player_id = scline->np[3];
    const char *flag_name = scline->tp[4];

    long flag_id, flag_type;
    if (!parse_get_varib(flag_name, &flag_id, &flag_type))
    {
        SCRPTERRLOG("Unknown flag, '%s'", flag_name);
        return;
    }

    value->shorts[0] = ap_num;
    value->bytes[2] = crmodel;
    value->chars[3] = flag_player_id;
    value->shorts[2] = flag_id;
    value->chars[6] = flag_type;

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void new_creature_type_check(const struct ScriptLine* scline)
{
    if (gameadd.crtr_conf.model_count >= CREATURE_TYPES_MAX)
    {
        SCRPTERRLOG("Cannot increase creature type count for creature type '%s', already at maximum %d types.", scline->tp[0], CREATURE_TYPES_MAX);
        return;
    }

    int i = gameadd.crtr_conf.model_count;
    gameadd.crtr_conf.model_count++;
    LbStringCopy(gameadd.crtr_conf.model[i].name, scline->tp[0], COMMAND_WORD_LEN);
    creature_desc[i-1].name = gameadd.crtr_conf.model[i].name;
    creature_desc[i-1].num = i;

    if (load_creaturemodel_config(i, 0))
    {
        SCRPTLOG("Adding creature type %s and increasing creature types to %d", creature_code_name(i), gameadd.crtr_conf.model_count - 1);
    }
    else
    {
        SCRPTERRLOG("Failed to load config for creature '%s'(%d).", gameadd.crtr_conf.model[i].name,i);
    }
}

static void new_room_type_check(const struct ScriptLine* scline)
{
    if (game.slab_conf.room_types_count >= TERRAIN_ITEMS_MAX - 1)
    {
        SCRPTERRLOG("Cannot increase room count for room type '%s', already at maximum %d rooms.", scline->tp[0], TERRAIN_ITEMS_MAX - 1);
        return;
    }

    SCRPTLOG("Adding room type %s and increasing 'RoomsCount to %d", scline->tp[0], game.slab_conf.room_types_count + 1);
    game.slab_conf.room_types_count++;

    struct RoomConfigStats* roomst;
    int i = game.slab_conf.room_types_count - 1;

    roomst = &game.slab_conf.room_cfgstats[i];
    LbMemorySet(roomst->code_name, 0, COMMAND_WORD_LEN);
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
    if (gameadd.object_conf.object_types_count >= OBJECT_TYPES_MAX-1)
    {
        SCRPTERRLOG("Cannot increase object count for object type '%s', already at maximum %d objects.", scline->tp[0], OBJECT_TYPES_MAX-1);
        return;
    }

    SCRPTLOG("Adding object type %s and increasing 'ObjectsCount to %d", scline->tp[0], gameadd.object_conf.object_types_count + 1);
    gameadd.object_conf.object_types_count++;

    struct ObjectConfigStats* objst;
    int tmodel = gameadd.object_conf.object_types_count -1;

    objst = &gameadd.object_conf.object_cfgstats[tmodel];
    LbMemorySet(objst->code_name, 0, COMMAND_WORD_LEN);
    snprintf(objst->code_name, COMMAND_WORD_LEN, "%s", scline->tp[0]);
    objst->name_stridx = 201;
    objst->map_icon = 0;
    objst->genre = 0;
    object_desc[tmodel].name = objst->code_name;
    object_desc[tmodel].num = tmodel;
    if (tmodel > OBJECT_TYPES_COUNT_ORIGINAL)
    {
        define_custom_object(tmodel, 0);
    }
}

static void new_trap_type_check(const struct ScriptLine* scline)
{
    if (gameadd.trapdoor_conf.trap_types_count >= TRAPDOOR_TYPES_MAX)
    {
        SCRPTERRLOG("Cannot increase trap count for trap type '%s', already at maximum %d traps.", scline->tp[0], TRAPDOOR_TYPES_MAX);
        return;
    }

    SCRPTLOG("Adding trap type %s and increasing 'TrapsCount to %d", scline->tp[0], gameadd.trapdoor_conf.trap_types_count + 1);
    gameadd.trapdoor_conf.trap_types_count++;

    short i = gameadd.trapdoor_conf.trap_types_count-1;

    struct TrapConfigStats* trapst = &gameadd.trapdoor_conf.trap_cfgstats[i];
    LbMemorySet(trapst->code_name, 0, COMMAND_WORD_LEN);
    snprintf(trapst->code_name, COMMAND_WORD_LEN, "%s", scline->tp[0]);
    trapst->name_stridx = GUIStr_Empty;
    trapst->tooltip_stridx = GUIStr_Empty;
    trapst->bigsym_sprite_idx = 0;
    trapst->medsym_sprite_idx = 0;
    trapst->pointer_sprite_idx = 0;
    trapst->panel_tab_idx = 0;
    trapst->hidden = 0;
    trapst->slappable = 0;
    trapst->destructible = 0;
    trapst->unstable = 0;
    trapst->unsellable = 0;
    trapst->notify = 0;
    trapst->placeonbridge = 0;

    gameadd.trap_stats[i].health = 0;
    gameadd.trap_stats[i].sprite_anim_idx = 0;
    gameadd.trap_stats[i].sprite_size_max = 0;
    gameadd.trap_stats[i].unanimated = 0;
    gameadd.trap_stats[i].anim_speed = 0;
    gameadd.trap_stats[i].unshaded = 0;
    gameadd.trap_stats[i].transparency_flag = 0;
    gameadd.trap_stats[i].random_start_frame = 0;
    gameadd.trap_stats[i].size_xy = 0;
    gameadd.trap_stats[i].size_yz = 0;
    gameadd.trap_stats[i].trigger_type = 0;
    gameadd.trap_stats[i].activation_type = 0;
    gameadd.trap_stats[i].created_itm_model = 0;
    gameadd.trap_stats[i].hit_type = 0;
    gameadd.trap_stats[i].light_radius = 0;
    gameadd.trap_stats[i].light_intensity = 0;
    gameadd.trap_stats[i].light_flag = 0;
    gameadd.trap_stats[i].shotvector.x = 0;
    gameadd.trap_stats[i].shotvector.y = 0;
    gameadd.trap_stats[i].shotvector.z = 0;
    trap_desc[i].name = trapst->code_name;
    trap_desc[i].num = i;
    struct ManfctrConfig* mconf = &gameadd.traps_config[i];
    mconf->manufct_level = 0;
    mconf->manufct_required = 0;
    mconf->shots = 0;
    mconf->shots_delay = 0;
    mconf->selling_value = 0;

    create_manufacture_array_from_trapdoor_data();
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
            if (trapstat->random_start_frame) {
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

static void set_trap_configuration_process(struct ScriptContext *context)
{
    long trap_type = context->value->shorts[0];
    struct TrapConfigStats *trapst = &gameadd.trapdoor_conf.trap_cfgstats[trap_type];
    struct ManfctrConfig *mconf = &gameadd.traps_config[trap_type];
    struct ManufactureData *manufctr = get_manufacture_data(trap_type);
    long value = context->value->uarg1;
    short value2 = context->value->shorts[4];
    short value3 = context->value->shorts[5];
    switch (context->value->shorts[1])
    {
        case 1: // NameTextID
            trapst->name_stridx = value;
            break;
        case 2: // TooltipTextID
            trapst->tooltip_stridx = value;
            manufctr->tooltip_stridx = trapst->tooltip_stridx;
            update_trap_tab_to_config();
            break;
        case 3: // SymbolSprites
        {
            trapst->bigsym_sprite_idx = get_icon_id(context->value->str2); // First
            trapst->medsym_sprite_idx = get_icon_id(context->value->str2 + strlen(context->value->str2) + 1); // Second
            if (trapst->bigsym_sprite_idx < 0)
                trapst->bigsym_sprite_idx = bad_icon_id;
            if (trapst->medsym_sprite_idx < 0)
                trapst->medsym_sprite_idx = bad_icon_id;
            manufctr->bigsym_sprite_idx = trapst->bigsym_sprite_idx;
            manufctr->medsym_sprite_idx = trapst->medsym_sprite_idx;
            update_trap_tab_to_config();
        }
            break;
        case 4: // PointerSprites
            trapst->pointer_sprite_idx = get_icon_id(context->value->str2);
            if (trapst->pointer_sprite_idx < 0)
                trapst->pointer_sprite_idx = bad_icon_id;
            update_trap_tab_to_config();
            break;
        case 5: // PanelTabIndex
            trapst->panel_tab_idx = value;
            manufctr->panel_tab_idx = value;
            update_trap_tab_to_config();
            break;
        case 6: // Crate
            gameadd.object_conf.object_to_door_or_trap[value] = trap_type;
            gameadd.object_conf.workshop_object_class[value] = TCls_Trap;
            gameadd.trapdoor_conf.trap_to_object[trap_type] = value;
            break;
        case 7: // ManufactureLevel
            mconf->manufct_level = value;
            break;
        case 8: // ManufactureRequired
            mconf->manufct_required = value;
            break;
        case 9: // Shots
            mconf->shots = value;
            break;
        case 10: // TimeBetweenShots
            mconf->shots_delay = value;
            break;
        case 11: // SellingValue
            mconf->selling_value = value;
            break;
        case 12: // Model
        {
            struct Objects obj_tmp;
            gameadd.trap_stats[trap_type].sprite_anim_idx = get_anim_id(context->value->str2, &obj_tmp);
            refresh_trap_anim(trap_type);
        }
            break;
        case 13: // ModelSize
            gameadd.trap_stats[trap_type].sprite_size_max = value;
            refresh_trap_anim(trap_type);
            break;
        case 14: // AnimationSpeed
            gameadd.trap_stats[trap_type].anim_speed = value;
            refresh_trap_anim(trap_type);
            break;
        case 15: // TriggerType
            gameadd.trap_stats[trap_type].trigger_type = value;
            break;
        case 16: // ActivationType
            gameadd.trap_stats[trap_type].activation_type = value;
            break;
        case 17: // EffectType
            gameadd.trap_stats[trap_type].created_itm_model = value;
            break;
        case 18: // Hidden
            trapst->hidden = value;
            break;
        case 19: // TriggerAlarm
            trapst->notify = value;
            break;
        case 20: // Slappable
            trapst->slappable = value;
            break;
        case 21: // Unanimated
            gameadd.trap_stats[trap_type].unanimated = value;
            refresh_trap_anim(trap_type);
            break;
        case 22: // Health
            gameadd.trap_stats[trap_type].health = value;
            break;
        case 23: // Unshaded
            gameadd.trap_stats[trap_type].unshaded = value;
            break;
        case 24: // RandomStartFrame
            gameadd.trap_stats[trap_type].random_start_frame = value;
            break;
        case 25: // ThingSize
            gameadd.trap_stats[trap_type].size_xy = value; // First
            gameadd.trap_stats[trap_type].size_yz = value2; // Second
            break;
        case 26: // HitType
            gameadd.trap_stats[trap_type].hit_type = value;
            break;
        case 27: // LightRadius
            gameadd.trap_stats[trap_type].light_radius = value;
            break;
        case 28: // LightIntensity
            gameadd.trap_stats[trap_type].light_intensity = value;
            break;
        case 29: // LightFlags
            gameadd.trap_stats[trap_type].light_flag = value;
            break;
        case 30: // TransparencyFlags
            gameadd.trap_stats[trap_type].transparency_flag = value;
            break;
        case 31: // ShotVector
            gameadd.trap_stats[trap_type].shotvector.x = value;
            gameadd.trap_stats[trap_type].shotvector.y = value2;
            gameadd.trap_stats[trap_type].shotvector.z = value3;
            break;
        case 32: // Destructible
            trapst->destructible = value;
            break;
        case 33: // Unstable
            trapst->unstable = value;
            break;
        case 34: // Unsellable
            trapst->unsellable = value;
            break;
        case 35: // PlaceOnBridge
            trapst->placeonbridge = value;
            break;
        default:
            WARNMSG("Unsupported Trap configuration, variable %d.", context->value->shorts[1]);
            break;
    }
}

static void set_room_configuration_process(struct ScriptContext *context)
{
    long room_type = context->value->shorts[0];
    struct RoomConfigStats *roomst = &game.slab_conf.room_cfgstats[room_type];
    unsigned short value;
    short value2;
    short value3;
    if (context->value->shorts[1] != 13) // Roles need larger values, so can fit fewer
    {
        value = context->value->shorts[2];
        value2 = context->value->shorts[3];
        value3 = context->value->shorts[4];
    }
    switch (context->value->shorts[1])
    {
        case 1: // NameTextID
            roomst->name_stridx = value;
            break;
        case 2: // TooltipTextID
            roomst->tooltip_stridx = value;
            update_room_tab_to_config();
            break;
        case 3: // SymbolSprites
        {
            roomst->bigsym_sprite_idx = get_icon_id(context->value->str2); // First
            roomst->medsym_sprite_idx = get_icon_id(context->value->str2 + strlen(context->value->str2) + 1); // Second
            if (roomst->bigsym_sprite_idx < 0)
                roomst->bigsym_sprite_idx = bad_icon_id;
            if (roomst->medsym_sprite_idx < 0)
                roomst->medsym_sprite_idx = bad_icon_id;
            update_room_tab_to_config();
        }
            break;
        case 4: // PointerSprites
            roomst->pointer_sprite_idx = get_icon_id(context->value->str2);
            if (roomst->pointer_sprite_idx < 0)
                roomst->pointer_sprite_idx = bad_icon_id;
            update_room_tab_to_config();
            break;
        case 5: // PanelTabIndex
            roomst->panel_tab_idx = value;
            update_room_tab_to_config();
            break;
        case 6: // Cost
            roomst->cost = value;
            break;
        case 7: // Health
            roomst->health = value;
            break;
        case 8: // CreatureCreation
            roomst->creature_creation_model = value;
            break;
        case 9: // AmbientSndSample
            roomst->ambient_snd_smp_id = value;
            break;
        case 10: // SlabAssign
            roomst->assigned_slab = value;
            break;
        case 11: // Messages
            roomst->msg_needed = value;
            roomst->msg_too_small = value2;
            roomst->msg_no_route = value3;
            break;
        case 12: // Properties
            roomst->flags = value;
            roomst->flags |= value2;
            roomst->flags |= value3;
            break;
        case 13: // Roles
            roomst->roles = context->value->uarg1;
            if (context->value->uarg2 > 0)
                roomst->roles |= context->value->uarg2;
            break;
        case 14: // TotalCapacity
            roomst->update_total_capacity_idx = value;
            roomst->update_total_capacity = terrain_room_total_capacity_func_list[value];
            reinitialise_rooms_of_kind(room_type);
            break;
        case 15: // UsedCapacity
            roomst->update_storage_in_room_idx = value;
            roomst->update_storage_in_room = terrain_room_used_capacity_func_list[value];
            roomst->update_workers_in_room_idx = value2;
            roomst->update_workers_in_room = terrain_room_used_capacity_func_list[value2];
            reinitialise_rooms_of_kind(room_type);
            break;
        case 16: // StorageHeight
            roomst->storage_height = value;
            break;
        default:
            WARNMSG("Unsupported Room configuration, variable %d.", context->value->shorts[1]);
            break;
    }
}

static void set_hand_rule_process(struct ScriptContext* context)
{
    long crtr_id = context->value->shorts[0];
    long hand_rule_action = context->value->shorts[1];
    long hand_rule_slot = context->value->shorts[2];
    long hand_rule_type = context->value->shorts[3];
    long param = context->value->shorts[4];
    long crtr_id_start = crtr_id == CREATURE_ANY ? 0 : crtr_id;
    long crtr_id_end = crtr_id == CREATURE_ANY ? CREATURE_TYPES_MAX : crtr_id + 1;

    struct DungeonAdd* dungeonadd;
    for (int i = context->plr_start; i < context->plr_end; i++)
    {
        for (int ci = crtr_id_start; ci < crtr_id_end; ci++)
        {
            dungeonadd = get_dungeonadd(i);
            if (hand_rule_action == HandRuleAction_Allow || hand_rule_action == HandRuleAction_Deny)
            {
                dungeonadd->hand_rules[ci][hand_rule_slot].enabled = 1;
                dungeonadd->hand_rules[ci][hand_rule_slot].type = hand_rule_type;
                dungeonadd->hand_rules[ci][hand_rule_slot].allow = hand_rule_action;
                dungeonadd->hand_rules[ci][hand_rule_slot].param = param;
            } else
            {
                dungeonadd->hand_rules[ci][hand_rule_slot].enabled = hand_rule_action == HandRuleAction_Enable;
            }
        }
    }
}

static void move_creature_process(struct ScriptContext* context)
{
    TbMapLocation location = context->value->uarg0;
    long select_id = context->value->arg1;
    long effect_id = context->value->shorts[4];
    long count = context->value->bytes[10];
    long crmodel = context->value->bytes[11];

    for (int i = context->plr_start; i < context->plr_end; i++)
    {
        for (int count_i = 0; count_i < count; count_i++)
        {
            struct Thing *thing = script_get_creature_by_criteria(i, crmodel, select_id);
            if (thing_is_invalid(thing) || thing_is_picked_up(thing)) {
                continue;
            }

            if (effect_id < 0)
            {
                effect_id = ball_puff_effects[thing->owner];
            }

            struct Coord3d pos;
            if(!get_coords_at_location(&pos,location)) {
                SYNCDBG(5,"No valid coords for location",(int)location);
                return;
            }
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(thing);

            if (effect_id > 0)
            {
                create_effect(&thing->mappos, effect_id, game.neutral_player_num);
                create_effect(&pos, effect_id, game.neutral_player_num);
            }
            move_thing_in_map(thing, &pos);
            reset_interpolation_of_thing(thing);
            initialise_thing_state(thing, CrSt_CreatureDoingNothing);
            cctrl->turns_at_job = -1;
            check_map_explored(thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        }
    }
}

static void count_creatures_at_action_point_process(struct ScriptContext* context)
{
    long ap_num = context->value->shorts[0];
    long crmodel = context->value->bytes[2];
    long flag_player_id = context->value->chars[3];
    long flag_id = context->value->shorts[2];
    long flag_type = context->value->chars[6];

    long sum = 0;
    for (int i = context->plr_start; i < context->plr_end; i++) {
        sum += count_player_creatures_of_model_in_action_point(i, crmodel, action_point_number_to_index(ap_num));
    }
    set_variable(flag_player_id, flag_type, flag_id, sum);
}

static void set_door_configuration_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    const char *doorname = scline->tp[0];
    short door_id = get_id(door_desc, doorname);
    const char* valuestring = scline->tp[2];
    long newvalue;

    if (door_id == -1)
    {
        SCRPTERRLOG("Unknown door, '%s'", doorname);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    short doorvar = get_id(trapdoor_door_commands, scline->tp[1]);
    if (doorvar == -1)
    {
        SCRPTERRLOG("Unknown door variable");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[0] = door_id;
    value->shorts[1] = doorvar;
    if (doorvar == 4) // SlabKind
    {
        const char* slab_name = scline->tp[2];
        const char* slab2_name = scline->tp[3];
        long slab_id = get_rid(slab_desc, slab_name);
        long slab2_id = get_rid(slab_desc, slab2_name);
        if (slab_id == -1)
        {
            if (parameter_is_number(slab_name))
            {
                slab_id = atoi(slab_name);
            }
            else
            {
                SCRPTERRLOG("Error slab %s not recognized", scline->tp[2]);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
        }
        if (slab2_id == -1)
        {
            if (parameter_is_number(slab2_name))
            {
                slab_id = atoi(slab2_name);
            }
            else
            {
                SCRPTERRLOG("Error slab %s not recognized", scline->tp[2]);
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
        }
        value->uarg1 = slab_id;
        value->shorts[4] = slab2_id;
    }

    else if (doorvar == 10) // SymbolSprites
    {
        char *tmp = malloc(strlen(scline->tp[2]) + strlen(scline->tp[3]) + 3);
        // Pass two vars along as one merged val like: first\nsecond\m
        strcpy(tmp, scline->tp[2]);
        strcat(tmp, "|");
        strcat(tmp,scline->tp[3]);
        value->str2 = script_strdup(tmp); // first\0second
        value->str2[strlen(scline->tp[2])] = 0;
        free(tmp);
        if (value->str2 == NULL)
        {
            SCRPTERRLOG("Run out script strings space");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    else if (doorvar != 11) // Not PointerSprites
    {
        if (parameter_is_number(valuestring))
        {
            newvalue = atoi(valuestring);
            if ((newvalue > LONG_MAX) || (newvalue < 0))
            {
                SCRPTERRLOG("Value out of range: %d", newvalue);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->uarg1 = newvalue;
        }
        else if (doorvar == 9) // Crate
        {
            newvalue = get_id(object_desc, valuestring);
            if ((newvalue > SHRT_MAX) || (newvalue < 0))
            {
                SCRPTERRLOG("Unknown crate object: %s", valuestring);
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->uarg1 = newvalue;
        }
        else
        {
            SCRPTERRLOG("Door property %s needs a number value, '%s' is invalid.", scline->tp[1], scline->tp[2]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    else
    {
        value->str2 = script_strdup(scline->tp[2]);
        if (value->str2 == NULL)
        {
            SCRPTERRLOG("Run out script strings space");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
    }
    SCRIPTDBG(7, "Setting door %s property %s to %lu", doorname, scline->tp[1], value->uarg1);
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_door_configuration_process(struct ScriptContext *context)
{
    long door_type = context->value->shorts[0];
    struct DoorConfigStats *doorst = get_door_model_stats(door_type);
    struct ManfctrConfig *mconf = &gameadd.doors_config[door_type];
    struct ManufactureData *manufctr = get_manufacture_data(gameadd.trapdoor_conf.trap_types_count - 1 + door_type);
    short value = context->value->arg1;
    short value2 = context->value->shorts[4];
    switch (context->value->shorts[1])
    {
        case 2: // ManufactureLevel
            mconf->manufct_level = value;
            break;
        case 3: // ManufactureRequired
            mconf->manufct_required = value;
            break;
        case 4: // SlabKind
            if (door_type < gameadd.trapdoor_conf.door_types_count)
            {
                doorst->slbkind[0] = value2;
                doorst->slbkind[1] = value;
            }
            update_all_door_stats();
            break;
        case 5: // Health
            if (door_type < gameadd.trapdoor_conf.door_types_count)
            {
                doorst->health = value;
            }
            update_all_door_stats();
            break;
        case 6: //SellingValue
            mconf->selling_value = value;
            break;
        case 7: // NametextId
            doorst->name_stridx = value;
            break;
        case 8: // TooltipTextId
            doorst->tooltip_stridx = value;
            manufctr->tooltip_stridx = doorst->tooltip_stridx;
            update_trap_tab_to_config();
            break;
        case 9: // Crate
            gameadd.object_conf.object_to_door_or_trap[value] = door_type;
            gameadd.object_conf.workshop_object_class[value] = TCls_Door;
            gameadd.trapdoor_conf.door_to_object[door_type] = value;
            break;
        case 10: //SymbolSprites
            {
                doorst->bigsym_sprite_idx = get_icon_id(context->value->str2); // First
                doorst->medsym_sprite_idx = get_icon_id(context->value->str2 + strlen(context->value->str2) + 1); // Second
                if (doorst->bigsym_sprite_idx < 0)
                    doorst->bigsym_sprite_idx = bad_icon_id;
                if (doorst->medsym_sprite_idx < 0)
                    doorst->medsym_sprite_idx = bad_icon_id;
                manufctr->bigsym_sprite_idx = doorst->bigsym_sprite_idx;
                manufctr->medsym_sprite_idx = doorst->medsym_sprite_idx;
                update_trap_tab_to_config();
            }
            break;
        case 11: // PointerSprites
            doorst->pointer_sprite_idx = get_icon_id(context->value->str2);
            if (doorst->pointer_sprite_idx < 0)
                doorst->pointer_sprite_idx = bad_icon_id;
            update_trap_tab_to_config();
            break;
        case 12: // PanelTabIndex
            doorst->panel_tab_idx = value;
            manufctr->panel_tab_idx = value;
            update_trap_tab_to_config();
            break;
        case 13: // OpenSpeed
            if (door_type < gameadd.trapdoor_conf.door_types_count)
            {
                doorst->open_speed = value;
            }
            break;
        default:
            WARNMSG("Unsupported Door configuration, variable %d.", context->value->shorts[1]);
            break;
    }
}

static void create_effect_process(struct ScriptContext *context)
{
    struct Coord3d pos;
    pos.x.stl.num = (MapSubtlCoord)context->value->bytes[1];
    pos.y.stl.num = (MapSubtlCoord)context->value->bytes[2];
    pos.z.val = get_floor_height(pos.x.stl.num, pos.y.stl.num);
    TbBool Price = (context->value->chars[0] == -(TngEffElm_Price));
    if (Price)
    {
        pos.z.val += 128;
    }
    else
    {
        pos.z.val += context->value->arg1;
    }
    struct Thing* efftng = create_used_effect_or_element(&pos, context->value->chars[0], game.neutral_player_num);
    if (!thing_is_invalid(efftng))
    {
        if (thing_in_wall_at(efftng, &efftng->mappos))
        {
            move_creature_to_nearest_valid_position(efftng);
        }
        if (Price)
        {
            efftng->price_effect.number = context->value->arg1;
        }
    }
}

static void set_heart_health_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->arg0 = scline->np[0];
    if (scline->np[1] > (signed long)game.dungeon_heart_health)
    {
        SCRPTWRNLOG("Value %ld is greater than maximum: %ld", scline->np[1], game.dungeon_heart_health);
        value->arg1 = game.dungeon_heart_health;
    }
    else
    {
        value->arg1 = scline->np[1];
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_heart_health_process(struct ScriptContext *context)
{
    struct Thing* heartng = get_player_soul_container(context->value->arg0);
    if (!thing_is_invalid(heartng))
    {
        heartng->health = (short)context->value->arg1;
    }
}

static void add_heart_health_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->arg0 = scline->np[0];
    value->arg1 = scline->np[1];
    value->arg2 = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_heart_health_process(struct ScriptContext *context)
{
    struct Thing* heartng = get_player_soul_container(context->value->arg0);
    if (!thing_is_invalid(heartng))
    {
        long old_health = heartng->health;
        long long new_health = heartng->health + context->value->arg1;
        if (new_health > (signed long)game.dungeon_heart_health)
        {
            SCRIPTDBG(7,"Player %d's calculated heart health (%ld) is greater than maximum: %ld", heartng->owner, new_health, game.dungeon_heart_health);
            new_health = game.dungeon_heart_health;
        }
        heartng->health = new_health;
        TbBool warn_on_damage = (context->value->arg2);
        if (warn_on_damage)
        {
            if (heartng->health < old_health)
            {
                event_create_event_or_update_nearby_existing_event(heartng->mappos.x.val, heartng->mappos.y.val, EvKind_HeartAttacked, heartng->owner, heartng->index);
                if (is_my_player_number(heartng->owner))
                {
                    output_message(SMsg_HeartUnderAttack, 400, true);
                }
            }
        }
    }
}

static void heart_lost_quick_objective_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if ((scline->np[0] < 0) || (scline->np[0] >= QUICK_MESSAGES_COUNT))
    {
        SCRPTERRLOG("Invalid QUICK OBJECTIVE number (%d)", scline->np[0]);
        return;
    }
    if (strlen(scline->tp[1]) >= MESSAGE_TEXT_LEN)
    {
        SCRPTWRNLOG("Objective TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
    }
    if ((gameadd.quick_messages[scline->np[0]][0] != '\0') && (strcmp(gameadd.quick_messages[scline->np[0]],scline->tp[1]) != 0))
    {
        SCRPTWRNLOG("Quick Objective no %d overwritten by different text", scline->np[0]);
    }
    snprintf(gameadd.quick_messages[scline->np[0]], MESSAGE_TEXT_LEN, "%s", scline->tp[1]);
    
    TbMapLocation location;
    if (scline->tp[2][0] != '\0')
    {
        get_map_location_id(scline->tp[2], &location);
    }

    value->arg0 = scline->np[0];
    value->uarg2 = location;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void heart_lost_quick_objective_process(struct ScriptContext *context)
{
    gameadd.heart_lost_display_message = true;
    gameadd.heart_lost_quick_message = true;
    gameadd.heart_lost_message_id = context->value->arg0;
    gameadd.heart_lost_message_target = context->value->arg2;
}

static void heart_lost_objective_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->arg0 = scline->np[0];
    TbMapLocation location;
    if (scline->tp[1][0] != '\0')
    {
        get_map_location_id(scline->tp[1], &location);
    }
    value->uarg1 = location;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void heart_lost_objective_process(struct ScriptContext *context)
{
    gameadd.heart_lost_display_message = true;
    gameadd.heart_lost_quick_message = false;
    gameadd.heart_lost_message_id = context->value->arg0;
    gameadd.heart_lost_message_target = context->value->arg1;
}

static void set_door_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long doorAction = get_id(set_door_desc, scline->tp[0]);
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
        case 1:
            lock_door(doortng);
            break;
        case 2:
            unlock_door(doortng);
            break;
        }
    }
}

static void create_effects_line_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    value->arg0 = scline->np[0]; // AP `from`
    value->arg1 = scline->np[1]; // AP `to`
    value->chars[8] = scline->np[2]; // curvature
    value->bytes[9] = scline->np[3]; // spatial stepping
    value->bytes[10] = scline->np[4]; // temporal stepping

    const char* effect_name = scline->tp[5];
    long effct_id = get_rid(effect_desc, effect_name);
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

    value->chars[11] = effct_id; // effect

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void create_effects_line_process(struct ScriptContext *context)
{
    struct ScriptFxLine *fx_line = NULL;
    for (int i = 0; i < (sizeof(gameadd.fx_lines) / sizeof(gameadd.fx_lines[0])); i++)
    {
        if (!gameadd.fx_lines[i].used)
        {
            fx_line = &gameadd.fx_lines[i];
            fx_line->used = true;
            gameadd.active_fx_lines++;
            break;
        }
    }
    if (fx_line == NULL)
    {
        ERRORLOG("Too many fx_lines");
        return;
    }
    find_location_pos(context->value->arg0, context->player_idx, &fx_line->from, __func__);
    find_location_pos(context->value->arg1, context->player_idx, &fx_line->to, __func__);
    fx_line->curvature = context->value->chars[8];
    fx_line->spatial_step = context->value->bytes[9] * 32;
    fx_line->steps_per_turn = context->value->bytes[10];
    fx_line->effect = context->value->chars[11];
    fx_line->here = fx_line->from;
    fx_line->step = 0;

    if (fx_line->steps_per_turn <= 0)
    {
        fx_line->steps_per_turn = 32 * 255; // whole map
    }

    int dx = fx_line->to.x.val - fx_line->from.x.val;
    int dy = fx_line->to.y.val - fx_line->from.y.val;
    if ((dx * dx + dy * dy) != 0)
    {
        double len = sqrt((double)dx * dx + (double)dy * dy);
        fx_line->total_steps = (int)(len / fx_line->spatial_step) + 1;

        int d_cx = -dy * fx_line->curvature / 32;
        int d_cy = +dx * fx_line->curvature / 32;
        fx_line->cx = (fx_line->to.x.val + fx_line->from.x.val - d_cx)/2;
        fx_line->cy = (fx_line->to.y.val + fx_line->from.y.val - d_cy)/2;
    }
    else
    {
      fx_line->total_steps = 1;
    }
    fx_line->partial_steps = FX_LINE_TIME_PARTS;
}

static void set_object_configuration_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    const char *objectname = scline->tp[0];
    const char *property = scline->tp[1];
    const char *new_value = scline->tp[2];

    long objct_id = get_id(object_desc, objectname);
    if (objct_id == -1)
    {
        SCRPTERRLOG("Unknown object, '%s'", objectname);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    long number_value = 0;
    long objectvar = get_id(objects_object_commands, property);
    if (objectvar == -1)
    {
        SCRPTERRLOG("Unknown object variable");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    switch (objectvar)
    {
        case 2: // Genre
            number_value = get_id(objects_genres_desc, new_value);
            if (number_value == -1)
            {
                SCRPTERRLOG("Unknown object variable");
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->arg2 = number_value;
            break;
        case 3: // RELATEDCREATURE
            number_value = get_id(creature_desc, new_value);
            if (number_value == -1)
            {
                SCRPTERRLOG("Unknown object variable");
                DEALLOCATE_SCRIPT_VALUE
                    return;
            }
            value->arg2 = number_value;
            break;
        case  5: // AnimId
        {
            struct Objects obj_tmp;
            number_value = get_anim_id(new_value, &obj_tmp);
            if (number_value == 0)
            {
                SCRPTERRLOG("Invalid animation id");
                DEALLOCATE_SCRIPT_VALUE
                return;
            }

            value->str2 = script_strdup(new_value);
            if (value->str2 == NULL)
            {
                SCRPTERRLOG("Run out script strings space");
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            break;
        }
        case 18: // MapIcon
        {
            number_value = get_icon_id(new_value);
            if (number_value < 0)
            {
                SCRPTERRLOG("Invalid icon id");
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->arg2 = number_value;
            break;
        }
        case 20: // UPDATEFUNCTION
        {
            number_value = get_id(object_update_functions_desc,new_value);
            if (number_value < 0)
            {
                SCRPTERRLOG("Invalid object update function id");
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            value->arg2 = number_value;
            break;
        }
        default:
            value->arg2 = atoi(new_value);
    }
    
    SCRIPTDBG(7, "Setting object %s property %s to %d", objectname, property, number_value);
    value->arg0 = objct_id;
    value->arg1 = objectvar;

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_creature_configuration_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    short creatvar = get_id(creatmodel_attributes_commands, scline->tp[1]);
    if (creatvar == -1)
    {
        creatvar = get_id(creatmodel_jobs_commands, scline->tp[1]);
        if (creatvar == -1)
        {
            SCRPTERRLOG("Unknown creature attribute");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        creatvar = (creatvar << 8);
    }

    short attribute_value;
    short attribute2_value = 0;
    if (creatvar == 20) // ATTACKPREFERENCE
    {
        attribute_value = get_id(attackpref_desc, scline->tp[2]);
    }
    else if (creatvar == 34) // LAIROBJECT
    {
        attribute_value = get_id(object_desc, scline->tp[2]);
    }
    else if (((creatvar>>8) > 0 ) && ((creatvar >> 8) <= 4)) // Jobs
    {
        long job_value = get_id(creaturejob_desc, scline->tp[2]);
        if (job_value > SHRT_MAX)
        {
            SCRPTERRLOG("JOB %s not supported",creature_job_code_name(job_value));
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        attribute_value = job_value;

        if (scline->tp[3][0] != '\0')
        {
            long job2_value = get_id(creaturejob_desc, scline->tp[3]);
            if (job2_value > SHRT_MAX)
            {
                SCRPTERRLOG("JOB %s not supported", creature_job_code_name(job_value));
                DEALLOCATE_SCRIPT_VALUE
                return;
            }
            attribute2_value = job2_value;
        }
    }
    else
    {
            attribute_value = atoi(scline->tp[2]);
            if (scline->tp[3][0] != '\0')
            {
                attribute2_value = atoi(scline->tp[3]);
            }
    }
    if (attribute_value == -1)
    {
        SCRPTERRLOG("Unknown creature attribute value %s", scline->tp[2]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if (attribute2_value == -1)
    {
        SCRPTERRLOG("Unknown second creature attribute value %s", scline->tp[3]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[0] = scline->np[0];
    value->shorts[1] = creatvar;
    value->shorts[2] = attribute_value;
    value->shorts[3] = attribute2_value;
    SCRIPTDBG(7,"Setting creature %s attribute %d to %d (%d)", creature_code_name(value->shorts[0]), value->shorts[1], value->shorts[2], value->shorts[3]);

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_creature_configuration_process(struct ScriptContext* context)
{
    short creatid = context->value->shorts[0];
    struct CreatureStats* crstat = creature_stats_get(creatid);
    struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[creatid];

    short attribute = context->value->shorts[1];
    short value = context->value->shorts[2];
    short value2 = context->value->shorts[3];

    if (attribute <= CHAR_MAX)
    {
        switch (attribute)
        {
        case 1: // NAME
            CONFWRNLOG("Attribute (%d) not supported", attribute);
            break;
        case 2: // HEALTH
            crstat->health = value;
            break;
        case 3: // HEALREQUIREMENT
            crstat->heal_requirement = value;
            break;
        case 4: // HEALTHRESHOLD
            crstat->heal_threshold = value;
            break;
        case 5: // STRENGTH
            crstat->strength = value;
            break;
        case 6: // ARMOUR
            crstat->armour = value;
            break;
        case 7: // DEXTERITY
            crstat->dexterity = value;
            break;
        case 8: // FEARWOUNDED
            crstat->fear_wounded = value;
            break;
        case 9: // FEARSTRONGER
            crstat->fear_stronger = value;
            break;
        case 10: // DEFENCE
            crstat->defense = value;
            break;
        case 11: // LUCK
            crstat->luck = value;
            break;
        case 12: // RECOVERY
            crstat->sleep_recovery = value;
            break;
        case 13: // HUNGERRATE
            crstat->hunger_rate = value;
            break;
        case 14: // HUNGERFILL
            crstat->hunger_fill = value;
            break;
        case 15: // LAIRSIZE
            crstat->lair_size = value;
            break;
        case 16: // HURTBYLAVA
            crstat->hurt_by_lava = value;
            break;
        case 17: // BASESPEED
            crstat->base_speed = value;
            break;
        case 18: // GOLDHOLD
            crstat->gold_hold = value;
            break;
        case 19: // SIZE
            crstat->size_xy = value;
            crstat->size_yz = value2;
            break;
        case 20: // ATTACKPREFERENCE
            crstat->attack_preference = value;
            break;
        case 21: // PAY
            crstat->pay = value;
            break;
        case 22: // HEROVSKEEPERCOST
            crstat->hero_vs_keeper_cost = value;
            break;
        case 23: // SLAPSTOKILL
            crstat->slaps_to_kill = value;
            break;
        case 24: // CREATURELOYALTY
        case 25: // LOYALTYLEVEL
        case 28: // PROPERTIES
            CONFWRNLOG("Attribute (%d) not supported", attribute);
            break;
        case 26: // DAMAGETOBOULDER
            crstat->damage_to_boulder = value;
            break;
        case 27: // THINGSIZE
            crstat->thing_size_xy = value;
            crstat->thing_size_yz = value2;
            break;
        case 29: // NAMETEXTID
            crconf->namestr_idx = value;
            break;
        case 30: // FEARSOMEFACTOR
            crstat->fearsome_factor = value;
            break;
        case 31: // TOKINGRECOVERY
            crstat->toking_recovery = value;
            break;
        case 32: // CORPSEVANISHEFFECT
            crstat->corpse_vanish_effect = value;
            break;
        case 33: // FOOTSTEPPITCH
            crstat->footstep_pitch = value;
            break;
        case 34: // LAIROBJECT
            crstat->lair_object = value;
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
            break;
        default:
            CONFWRNLOG("Unrecognized command (%d)", attribute);
            break;
        }
    }
    else
    {
        attribute = (attribute >> 8); // creatmodel_jobs_commands
        switch (attribute)
        {
        case 1: // PRIMARYJOBS
            crstat->job_primary = value;
            crstat->job_primary |= value2;
            break;
        case 2: // SECONDARYJOBS
            crstat->job_secondary = value;
            crstat->job_secondary |= value2;
            break;
        case 3: // NOTDOJOBS
            crstat->jobs_not_do = value;
            crstat->jobs_not_do |= value2;
            break;
        case 4: // STRESSFULJOBS
            crstat->job_stress = value;
            crstat->job_stress |= value2;
            break;
        case 5: // TRAININGVALUE
            crstat->training_value = value;
            break;
        case 6: // TRAININGCOST
            crstat->training_cost = value;
            break;
        case 7: // SCAVENGEVALUE
            crstat->scavenge_value = value;
            break;
        case 8: // SCAVENGERCOST
            crstat->scavenger_cost = value;
            break;
        case 9: // RESEARCHVALUE
            crstat->research_value = value;
            break;
        case 10: // MANUFACTUREVALUE
            crstat->manufacture_value = value;
            break;
        case 11: // PARTNERTRAINING
            crstat->partner_training = value;
            break;
        default:
            CONFWRNLOG("Unrecognized Job command (%d)", attribute);
            break;
        }
    }
    check_and_auto_fix_stats();
}

static void set_object_configuration_process(struct ScriptContext *context)
{
    struct Objects* objdat = get_objects_data(context->value->arg0);
    struct ObjectConfigStats* objst = &gameadd.object_conf.object_cfgstats[context->value->arg0];
    struct ObjectConfig* objbc = &gameadd.object_conf.base_config[context->value->arg0];
    switch (context->value->arg1)
    {
        case 2: // GENRE
            objst->genre = context->value->arg2;
            break;
        case 3: // RELATEDCREATURE
            objdat->related_creatr_model = context->value->arg2;
            break;
        case 4: // PROPERTIES
            objst->model_flags = context->value->arg2;
            break;
        case 5: // ANIMATIONID
            objdat->sprite_anim_idx = get_anim_id(context->value->str2, objdat);
            break;
        case 6: // ANIMATIONSPEED
            objdat->anim_speed = context->value->arg2;
            break;
        case 7: //SIZE_XY
            objdat->size_xy = context->value->arg2;
            break;
        case 8: // SIZE_YZ
            objdat->size_yz = context->value->arg2;
            break;
        case 9: // MAXIMUMSIZE
            objdat->sprite_size_max = context->value->arg2;
            break;
        case 10: // DESTROYONLIQUID
            objdat->destroy_on_liquid = context->value->arg2;
            break;
        case 11: // DESTROYONLAVA
            objdat->destroy_on_lava = context->value->arg2;
            break;
        case 12: // HEALTH
            objbc->health = context->value->arg2;
            break;
        case 13: // FALLACCELERATION
            objbc->fall_acceleration = context->value->arg2;
            break;
        case 14: // LIGHTUNAFFECTED
            objbc->light_unaffected = context->value->arg2;
            break;
        case 15: // LIGHTINTENSITY
            objbc->ilght.intensity = context->value->arg2;
            break;
        case 16: // LIGHTRADIUS
            objbc->ilght.radius = context->value->arg2 << 8; //Mystery bit shift. Remove it to get divide by 0 errors.
            break;
        case 17: // LIGHTISDYNAMIC
            objbc->ilght.is_dynamic = context->value->arg2;
            break;
        case 18: // MAPICON
            objst->map_icon = context->value->arg2;
            break;
        case 19: // AMBIENCESOUND
            objdat->fp_smpl_idx = context->value->arg2;
            break;
        case 20: // UPDATEFUNCTION
            objdat->updatefn_idx = context->value->arg2;
            break;
        case 21: // DRAWCLASS
            objdat->draw_class = context->value->arg2;
            break;
        case 22: // PERSISTENCE
            objdat->persistence = context->value->arg2;
            break;
        default:
            WARNMSG("Unsupported Object configuration, variable %d.", context->value->arg1);
            break;
    }
    update_all_object_stats();
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
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->bytes[0] = (unsigned char)scline->np[0];
    value->bytes[1] = timr_id;
    value->arg1 = 0;
    value->bytes[2] = (TbBool)scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void display_timer_process(struct ScriptContext *context)
{
    gameadd.script_player = context->value->bytes[0];
    gameadd.script_timer_id = context->value->bytes[1];
    gameadd.script_timer_limit = context->value->arg1;
    gameadd.timer_real = context->value->bytes[2];
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
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->arg0 = scline->np[0];
    value->arg1 = timr_id;
    value->arg2 = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_to_timer_process(struct ScriptContext *context)
{
   add_to_script_timer(context->value->arg0, context->value->arg1, context->value->arg2);
}

static void add_bonus_time_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->arg0 = scline->np[0];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void add_bonus_time_process(struct ScriptContext *context)
{
   game.bonus_time += context->value->arg0;
}

static void display_variable_check(const struct ScriptLine *scline)
{
    long varib_id, varib_type;
    if (!parse_get_varib(scline->tp[1], &varib_id, &varib_type))
    {
        SCRPTERRLOG("Unknown variable, '%s'", scline->tp[1]);
        return;
    }
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->bytes[0] = scline->np[0];
    value->bytes[1] = scline->np[3];
    value->bytes[2] = varib_type;
    value->arg1 = varib_id;
    value->arg2 = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void display_variable_process(struct ScriptContext *context)
{
   gameadd.script_player = context->value->bytes[0];
   gameadd.script_value_type = context->value->bytes[2];
   gameadd.script_value_id = context->value->arg1;
   gameadd.script_variable_target = context->value->arg2;
   gameadd.script_variable_target_type = context->value->bytes[1];
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
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    value->bytes[0] = (unsigned char)scline->np[0];
    value->bytes[1] = timr_id;
    value->arg1 = scline->np[2];
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
    long effct_id = get_rid(effect_desc, effect_name);
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
    value->chars[0] = effct_id;
    const char *locname = scline->tp[1];
    if (!get_map_location_id(locname, &location))
    {
        return;
    }
    long stl_x;
    long stl_y;
    find_map_location_coords(location, &stl_x, &stl_y, 0, __func__);
    value->bytes[1] = stl_x;
    value->bytes[2] = stl_y;
    value->arg1 = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void create_effect_at_pos_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    const char *effect_name = scline->tp[0];
    long effct_id = get_rid(effect_desc, effect_name);
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
    value->chars[0] = effct_id;
    if (subtile_coords_invalid(scline->np[1], scline->np[2]))
    {
        SCRPTERRLOG("Invalid coordinates: %ld, %ld", scline->np[1], scline->np[2]);
        return;
    }
    value->bytes[1] = scline->np[1];
    value->bytes[2] = scline->np[2];
    value->arg1 = scline->np[3];
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
    qsort(value->sac.victims, MAX_SACRIFICE_VICTIMS, sizeof(value->sac.victims[0]), &sac_compare_fn);

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
    qsort(value->sac.victims, MAX_SACRIFICE_VICTIMS, sizeof(value->sac.victims[0]), &sac_compare_fn);

    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_sacrifice_recipe_process(struct ScriptContext *context)
{
    long victims[MAX_SACRIFICE_VICTIMS];
    struct Coord3d pos;
    int action = context->value->sac.action;
    int param = context->value->sac.param;
    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
        victims[i] = context->value->sac.victims[i];
    }
    for (int i = 1; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &gameadd.sacrifice_recipes[i];
        if (sac->action == (long)SacA_None)
        {
            break;
        }
        if (memcmp(victims, sac->victims, sizeof(victims)) == 0)
        {
            sac->action = action;
            sac->param = param;
            if (action == (long)SacA_None)
            {
                // remove empty space
                memmove(sac, sac + 1, (MAX_SACRIFICE_RECIPES - 1 - (sac - &gameadd.sacrifice_recipes[0])) * sizeof(*sac));
            }
            return;
        }
    }
    if (action == (long)SacA_None) // No rule found
    {
        WARNLOG("Unable to find sacrifice rule to remove");
        return;
    }
    struct SacrificeRecipe* sac = get_unused_sacrifice_recipe_slot();
    if (sac == &gameadd.sacrifice_recipes[0])
    {
        ERRORLOG("No free sacrifice rules");
        return;
    }
    memcpy(sac->victims, victims, sizeof(victims));
    sac->action = action;
    sac->param = param;

    if (find_temple_pool(context->player_idx, &pos))
    {
        // Check if sacrifice pool already matches
        for (int i = 0; i < sizeof(victims); i++)
        {
            if (victims[i] == 0)
                break;
            process_sacrifice_creature(&pos, victims[i], context->player_idx, false);
        }
    }
}

static void set_box_tooltip(const struct ScriptLine *scline)
{
  if ((scline->np[0] < 0) || (scline->np[0] >= CUSTOM_BOX_COUNT))
  {
    SCRPTERRLOG("Invalid CUSTOM_BOX number (%ld)", scline->np[0]);
    return;
  }
  int idx = scline->np[0];
  if (strlen(scline->tp[1]) >= MESSAGE_TEXT_LEN)
  {
      SCRPTWRNLOG("Tooltip TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
  }
  if ((gameadd.box_tooltip[idx][0] != '\0') && (strcmp(gameadd.box_tooltip[idx], scline->tp[1]) != 0))
  {
      SCRPTWRNLOG("Box tooltip #%d overwritten by different text", idx);
  }
  snprintf(gameadd.box_tooltip[idx], MESSAGE_TEXT_LEN, "%s", scline->tp[1]);
}

static void set_box_tooltip_id(const struct ScriptLine *scline)
{
  if ((scline->np[0] < 0) || (scline->np[0] >= CUSTOM_BOX_COUNT))
  {
    SCRPTERRLOG("Invalid CUSTOM_BOX number (%ld)", scline->np[0]);
    return;
  }
  int idx = scline->np[0];
  snprintf(gameadd.box_tooltip[idx], MESSAGE_TEXT_LEN, "%s", get_string(scline->np[1]));
}

static void change_slab_owner_check(const struct ScriptLine *scline)
{

    if (scline->np[0] < 0 || scline->np[0] > gameadd.map_tiles_x) //x coord
    {
        SCRPTERRLOG("Value '%d' out of range. Range 0-%d allowed.", scline->np[0],gameadd.map_tiles_x);
        return;
    }
    if (scline->np[1] < 0 || scline->np[1] > gameadd.map_tiles_y) //y coord
    {
        SCRPTERRLOG("Value '%d' out of range. Range 0-%d allowed.", scline->np[1],gameadd.map_tiles_y);
        return;
    }
    long filltype = get_id(fill_desc, scline->tp[3]);
    if ((scline->tp[3] != NULL) && (strcmp(scline->tp[3], "") != 0) && (filltype == -1))
    {
        SCRPTWRNLOG("Fill type %s not recognized", scline->tp[3]);
    }

    command_add_value(Cmd_CHANGE_SLAB_OWNER, scline->np[2], scline->np[0], scline->np[1], get_id(fill_desc, scline->tp[3]));
}

static void change_slab_owner_process(struct ScriptContext *context)
{
    MapSlabCoord x = context->value->arg0;
    MapSlabCoord y = context->value->arg1;
    long fill_type = context->value->arg2;
    if (fill_type > 0)
    {
        struct CompoundCoordFilterParam iter_param;
        iter_param.plyr_idx = context->player_idx;
        iter_param.num1 = fill_type;
        iter_param.num2 = get_slabmap_block(x, y)->kind;
        slabs_fill_iterate_from_slab(x, y, slabs_change_owner, &iter_param);
    } else {
        change_slab_owner_from_script(x, y, context->player_idx);
    }
}

static void change_slab_type_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    if (scline->np[0] < 0 || scline->np[0] > gameadd.map_tiles_x) //x coord
    {
        SCRPTERRLOG("Value '%d' out of range. Range 0-%d allowed.", scline->np[0],gameadd.map_tiles_x);
        return;
    }
    else
    {
        value->shorts[0] = scline->np[0];
    }

    if (scline->np[1] < 0 || scline->np[1] > gameadd.map_tiles_y) //y coord
    {
        SCRPTERRLOG("Value '%d' out of range. Range 0-%d allowed.", scline->np[0],gameadd.map_tiles_y);
        return;
    }
    else
    {
        value->shorts[1] = scline->np[1];
    }

    if (scline->np[2] < 0 || scline->np[2] > 53) //slab kind
    {
        SCRPTERRLOG("Unsupported slab '%d'. Slabs range 0-53 allowed.", scline->np[2]);
        return;
    }
    else
    {
        value->shorts[2] = scline->np[2];
    }

    value->shorts[3] = get_id(fill_desc, scline->tp[3]);
    if ((scline->tp[3] != NULL) && (strcmp(scline->tp[3],"") != 0) && (value->shorts[3] == -1))
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
        iter_param.num1 = slab_kind;
        iter_param.num2 = fill_type;
        iter_param.num3 = get_slabmap_block(x, y)->kind;
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
    TbMapLocation target = context->value->arg0;
    SYNCDBG(0, "Revealing location type %d", target);
    long x = 0;
    long y = 0;
    long r = context->value->arg1;
    find_map_location_coords(target, &x, &y, context->player_idx, __func__);
    if ((x == 0) && (y == 0))
    {
        WARNLOG("Can't decode location %d", target);
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

static void use_spell_on_creature_check(const struct ScriptLine* scline)
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
    short splevel = scline->np[3];

    if (mag_id == -1)
    {
        SCRPTERRLOG("Invalid spell: %s", mag_name);
        return;
    }

    if (splevel < 1)
    {
        if ((mag_id == SplK_Heal) || (mag_id == SplK_Armour) || (mag_id == SplK_Speed) || (mag_id == SplK_Disease) || (mag_id == SplK_Invisibility) || (mag_id == SplK_Chicken))
        {
            SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", mag_name, splevel);
        }
        splevel = 1;
    }
    if (splevel > (MAGIC_OVERCHARGE_LEVELS + 1)) //Creatures cast spells from level 1 to 10, but 10=9.
    {
        SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", mag_name, splevel, (MAGIC_OVERCHARGE_LEVELS + 1));
        splevel = MAGIC_OVERCHARGE_LEVELS;
    }
    splevel--;
    if (mag_id == -1)
    {
        SCRPTERRLOG("Unknown magic, '%s'", mag_name);
        return;
    }
    value->shorts[1] = crtr_id;
    value->shorts[2] = mag_id;
    value->shorts[3] = splevel;
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void use_spell_on_creature_process(struct ScriptContext* context)
{
    long crmodel = context->value->shorts[1];
    long spl_idx = context->value->shorts[2];
    long overchrg = context->value->shorts[3];

    for (int i = context->plr_start; i < context->plr_end; i++)
    {
        apply_spell_effect_to_players_creatures(i, crmodel, spl_idx, overchrg);
    }
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
    struct CreatureStats *crstat = creature_stats_get(context->value->bytes[0]);
    if (!creature_stats_invalid(crstat))
    {
        CrInstance old_instance = crstat->learned_instance_id[context->value->bytes[1] - 1];
        crstat->learned_instance_id[context->value->bytes[1] - 1] = context->value->bytes[2];
        crstat->learned_instance_level[context->value->bytes[1] - 1] = context->value->bytes[3];
        for (short i = 0; i < THINGS_COUNT; i++)
        {
            struct Thing* thing = thing_get(i);
            if (thing_is_creature(thing))
            {
                if (thing->model == context->value->bytes[0])
                {
                    if (old_instance != CrInst_NULL)
                    {
                        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
                        cctrl->instance_available[old_instance] = false;
                    }
                    creature_increase_available_instances(thing);
                }
            }
        }
    }
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
        SCRPTERRLOG("Invalid hero gate: %d", scline->np[0]);
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
        create_effect(&thing->mappos, TngEff_BallPuffWhite, thing->owner);
        place_thing_in_creature_controlled_limbo(thing);
    }
    else
    {
        create_effect(&thing->mappos, TngEff_BallPuffWhite, thing->owner);
        remove_thing_from_creature_controlled_limbo(thing);
    }
}

static void if_check(const struct ScriptLine *scline)
{

    long plr_range_id = scline->np[0];
    const char *varib_name = scline->tp[1];
    const char *operatr = scline->tp[2];

    long plr_range_id_right;
    const char *varib_name_right = scline->tp[4];

    long value;

    TbBool double_var_mode = false;
    long varib_type;
    long varib_id;
    long varib_type_right;
    long varib_id_right;


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


    if (gameadd.script.conditions_num >= CONDITIONS_COUNT)
    {
      SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
      return;
    }
    // Recognize variable
    if (!parse_get_varib(varib_name, &varib_id, &varib_type))
    {
        return;
    }
    if (double_var_mode && !parse_get_varib(varib_name_right, &varib_id_right, &varib_type_right))
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

    long plr_range_id_right;
    const char *varib_name_right = scline->tp[4];

    long value;

    TbBool double_var_mode = false;
    long varib_type_right;
    long varib_id_right;


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

    long varib_type;
    if (gameadd.script.conditions_num >= CONDITIONS_COUNT)
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
    if (double_var_mode && !parse_get_varib(varib_name_right, &varib_id_right, &varib_type_right))
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

    long plr_range_id_right;
    const char *varib_name_right = scline->tp[4];

    long value;

    TbBool double_var_mode = false;
    long varib_type_right;
    long varib_id_right;


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
    if (gameadd.script.conditions_num >= CONDITIONS_COUNT)
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

    if (double_var_mode && !parse_get_varib(varib_name_right, &varib_id_right, &varib_type_right))
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

    if (gameadd.script.conditions_num >= CONDITIONS_COUNT)
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
    long texture_id = context->value->shorts[0];
    struct Dungeon* dungeon;
    for (int i = context->plr_start; i < context->plr_end; i++)
    {
        dungeon = get_dungeon(i);
        dungeon->texture_pack = texture_id;



        for (MapSlabCoord slb_y=0; slb_y < gameadd.map_tiles_y; slb_y++)
        {
            for (MapSlabCoord slb_x=0; slb_x < gameadd.map_tiles_x; slb_x++)
            {
                struct SlabMap* slb = get_slabmap_block(slb_x,slb_y);
                if (slabmap_owner(slb) == i)
                {
                    if (texture_id == 0)
                    {
                        gameadd.slab_ext_data[get_slab_number(slb_x,slb_y)] = gameadd.slab_ext_data_initial[get_slab_number(slb_x,slb_y)];
                    }
                    else
                    {
                        gameadd.slab_ext_data[get_slab_number(slb_x,slb_y)] = texture_id;
                    }
                }
            }
        }
    }
}

static void set_music_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    if (parameter_is_number(scline->tp[0]))
    {
        value->chars[0] = atoi(scline->tp[0]);
    }
    else
    {
        if (IsRedbookMusicActive())
        {
            SCRPTWRNLOG("Level script wants to play custom track from disk, but game is playing music from CD.");
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        // See if a file with this name is already loaded, if so, reuse the same track
        char* compare_fname = prepare_file_fmtpath(FGrp_CmpgMedia, "%s", scline->tp[0]);
        for (int i = max_track + 1; i <= game.last_audiotrack; i++)
        {
            if (strcmp(compare_fname, game.loaded_track[i]) == 0)
            {
                value->chars[0] = i;
                PROCESS_SCRIPT_VALUE(scline->command);
                return;
            }
        }
        if ( (game.last_audiotrack < max_track) || (game.last_audiotrack >= MUSIC_TRACKS_COUNT) )
        {
            WARNLOG("Music track %d is out of range - resetting.", game.last_audiotrack);
            game.last_audiotrack = max_track;
        }
        if (game.last_audiotrack < MUSIC_TRACKS_COUNT-1)
        {
            game.last_audiotrack++;
        }
        short tracknumber = game.last_audiotrack;
            
        if (tracks[tracknumber] != NULL)
        {
            WARNLOG("Overwriting music track %d.", tracknumber);
            Mix_FreeMusic(tracks[tracknumber]);
        }
        const char* fname = prepare_file_fmtpath(FGrp_CmpgMedia, "%s", scline->tp[0]);
        LbStringCopy(game.loaded_track[tracknumber], fname, DISKPATH_SIZE);
        tracks[tracknumber] = Mix_LoadMUS(game.loaded_track[tracknumber]);
        if (tracks[tracknumber] == NULL)
        {
            SCRPTERRLOG("Can't load track %ld (%s): %s", tracknumber, game.loaded_track[tracknumber], Mix_GetError());
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        else
        {
            SCRPTLOG("Loaded file %s into music track %ld.", game.loaded_track[tracknumber], tracknumber);
        }
        value->chars[0] = tracknumber;
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_music_process(struct ScriptContext *context)
{
    
    short track_number = context->value->chars[0];
    if (track_number >= FIRST_TRACK && track_number <= MUSIC_TRACKS_COUNT)
    {
        if (track_number != game.audiotrack)
        {
            if (IsRedbookMusicActive())
            {
                SCRPTLOG("Setting music track to %d.", track_number);
            }
            else
            {
                char info[255];
                sprintf(info, "%s", Mix_GetMusicTitle(tracks[track_number]));
                const char* artist = Mix_GetMusicArtistTag(tracks[track_number]);
                if (artist[0] != '\0')
                {
                    sprintf(info, "%s by %s", info, artist);
                }
                const char* copyright = Mix_GetMusicCopyrightTag(tracks[track_number]);
                if (copyright[0] != '\0')
                {
                    sprintf(info, "%s (%s)", info, copyright);
                }
                SCRPTLOG("Setting music track to %d: %s", track_number, info);
            }
            game.audiotrack = track_number;
        }
    }
    else
    {
        SCRPTERRLOG("Invalid music track: %d. Track must be between %d and %d.", track_number,FIRST_TRACK,MUSIC_TRACKS_COUNT);
    }
}

static void play_message_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    long msgtype_id = get_id(msgtype_desc, scline->tp[1]);
    if (msgtype_id == -1)
    {
        SCRPTERRLOG("Unrecognized message type: '%s'", scline->tp[1]);
        return;
    }
    value->chars[0] = scline->np[0];
    value->chars[1] = msgtype_id;
    if (parameter_is_number(scline->tp[2]))
    {
        value->shorts[1] = atoi(scline->tp[2]);
        value->bytes[4] = 0;
    }
    else
    {
        value->bytes[4] = 1;
        for (unsigned char i = 0; i <= EXTERNAL_SOUNDS_COUNT; i++)
        {
            if (strcmp(scline->tp[2], game.loaded_sound[i]) == 0)
            {
                value->bytes[2] = i;
                PROCESS_SCRIPT_VALUE(scline->command);
                return;
            }
        }
        if (game.sounds_count >= EXTERNAL_SOUNDS_COUNT)
        {
            ERRORLOG("All external sounds slots are used.");
            return;
        }
        unsigned char slot = game.sounds_count+1;
        if (sprintf(&game.loaded_sound[slot][0], "%s", script_strdup(scline->tp[2])) < 0)
        {
            SCRPTERRLOG("Unable to store filename for external sound %s", scline->tp[1]);
            return;
        }
        char *fname = prepare_file_fmtpath(FGrp_CmpgMedia,"%s", &game.loaded_sound[slot][0]);
        Ext_Sounds[slot] = Mix_LoadWAV(fname);
        if (Ext_Sounds[slot] != NULL)
        {
            Mix_VolumeChunk(Ext_Sounds[slot], settings.sound_volume);
            game.sounds_count++;
        }
        else
        {
            SCRPTERRLOG("Could not load sound %s: %s", fname, Mix_GetError());
            return;
        }
        SCRPTLOG("Loaded sound file %s into slot %u.", fname, slot);
        value->bytes[2] = slot;
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void play_message_process(struct ScriptContext *context)
{
    if ((context->value->chars[0] == my_player_number) || (context->value->chars[0] == ALL_PLAYERS))
    {
        if (!context->value->bytes[4])
        {
            switch (context->value->chars[1])
            {
                case 1:
                {
                    output_message(context->value->shorts[1], 0, true);
                    break;
                }
                case 2:
                {
                    play_non_3d_sample(context->value->shorts[1]);
                    break;
                }
            }
        }
        else
        {
            if (!SoundDisabled)
            {
                switch (context->value->chars[1])
                {
                    case 1:
                    {
                        output_message(-context->value->bytes[2], 0, true);
                        break;
                    }
                    case 2:
                    {
                        play_external_sound_sample(context->value->bytes[2]);
                        break;
                    }
                }
            }
        }
    }
}

/**
 * Descriptions of script commands for parser.
 * Arguments are: A-string, N-integer, C-creature model, P- player, R- room kind, L- location, O- operator, S- slab kind
 * Lower case letters are optional arguments, Exclamation points sets 'extended' option, for example 'ANY_CREATURE' for creatures.
 */
const struct CommandDesc command_desc[] = {
  {"CREATE_PARTY",                      "A       ", Cmd_CREATE_PARTY, NULL, NULL},
  {"ADD_TO_PARTY",                      "ACNNAN  ", Cmd_ADD_TO_PARTY, &add_to_party_check, NULL},
  {"DELETE_FROM_PARTY",                 "ACN     ", Cmd_DELETE_FROM_PARTY, &delete_from_party_check, NULL},
  {"ADD_PARTY_TO_LEVEL",                "PAAN    ", Cmd_ADD_PARTY_TO_LEVEL, NULL, NULL},
  {"ADD_CREATURE_TO_LEVEL",             "PCANNN  ", Cmd_ADD_CREATURE_TO_LEVEL, NULL, NULL},
  {"ADD_OBJECT_TO_LEVEL",               "AANp    ", Cmd_ADD_OBJECT_TO_LEVEL, NULL, NULL},
  {"IF",                                "PAOAa   ", Cmd_IF, &if_check, NULL},
  {"IF_ACTION_POINT",                   "NP      ", Cmd_IF_ACTION_POINT, NULL, NULL},
  {"ENDIF",                             "        ", Cmd_ENDIF, NULL, NULL},
  {"SET_HATE",                          "PPN     ", Cmd_SET_HATE, NULL, NULL},
  {"SET_GENERATE_SPEED",                "N       ", Cmd_SET_GENERATE_SPEED, NULL, NULL},
  {"REM",                               "        ", Cmd_REM, NULL, NULL},
  {"START_MONEY",                       "PN      ", Cmd_START_MONEY, NULL, NULL},
  {"ROOM_AVAILABLE",                    "PRNN    ", Cmd_ROOM_AVAILABLE, NULL, NULL},
  {"CREATURE_AVAILABLE",                "PCNN    ", Cmd_CREATURE_AVAILABLE, NULL, NULL},
  {"MAGIC_AVAILABLE",                   "PANN    ", Cmd_MAGIC_AVAILABLE, NULL, NULL},
  {"TRAP_AVAILABLE",                    "PANN    ", Cmd_TRAP_AVAILABLE, NULL, NULL},
  {"RESEARCH",                          "PAAN    ", Cmd_RESEARCH, NULL, NULL},
  {"RESEARCH_ORDER",                    "PAAN    ", Cmd_RESEARCH_ORDER, NULL, NULL},
  {"COMPUTER_PLAYER",                   "PN      ", Cmd_COMPUTER_PLAYER, NULL, NULL},
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
  {"RESET_ACTION_POINT",                "N       ", Cmd_RESET_ACTION_POINT, NULL, NULL},
  {"SET_CREATURE_MAX_LEVEL",            "PCN     ", Cmd_SET_CREATURE_MAX_LEVEL, NULL, NULL},
  {"SET_MUSIC",                         "A       ", Cmd_SET_MUSIC, &set_music_check, &set_music_process},
  {"TUTORIAL_FLASH_BUTTON",             "NN      ", Cmd_TUTORIAL_FLASH_BUTTON, NULL, NULL},
  {"SET_CREATURE_STRENGTH",             "CN      ", Cmd_SET_CREATURE_STRENGTH, NULL, NULL},
  {"SET_CREATURE_HEALTH",               "CN      ", Cmd_SET_CREATURE_HEALTH, NULL, NULL},
  {"SET_CREATURE_ARMOUR",               "CN      ", Cmd_SET_CREATURE_ARMOUR, NULL, NULL},
  {"SET_CREATURE_FEAR_WOUNDED",         "CN      ", Cmd_SET_CREATURE_FEAR_WOUNDED, NULL, NULL},
  {"SET_CREATURE_FEAR_STRONGER",        "CN      ", Cmd_SET_CREATURE_FEAR_STRONGER, NULL, NULL},
  {"SET_CREATURE_FEARSOME_FACTOR",      "CN      ", Cmd_SET_CREATURE_FEARSOME_FACTOR, NULL, NULL},
  {"SET_CREATURE_PROPERTY",             "CAN     ", Cmd_SET_CREATURE_PROPERTY, NULL, NULL},
  {"IF_AVAILABLE",                      "PAOAa   ", Cmd_IF_AVAILABLE, &if_available_check, NULL},
  {"IF_CONTROLS",                       "PAOAa   ", Cmd_IF_CONTROLS,  &if_controls_check, NULL},
  {"SET_COMPUTER_GLOBALS",              "PNNNNNNn", Cmd_SET_COMPUTER_GLOBALS, NULL, NULL},
  {"SET_COMPUTER_CHECKS",               "PANNNNN ", Cmd_SET_COMPUTER_CHECKS, NULL, NULL},
  {"SET_COMPUTER_EVENT",                "PANNNNN ", Cmd_SET_COMPUTER_EVENT, NULL, NULL},
  {"SET_COMPUTER_PROCESS",              "PANNNNN ", Cmd_SET_COMPUTER_PROCESS, NULL, NULL},
  {"ALLY_PLAYERS",                      "PPN     ", Cmd_ALLY_PLAYERS, NULL, NULL},
  {"DEAD_CREATURES_RETURN_TO_POOL",     "N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL, NULL, NULL},
  {"BONUS_LEVEL_TIME",                  "Nn      ", Cmd_BONUS_LEVEL_TIME, NULL, NULL},
  {"QUICK_OBJECTIVE",                   "NAl     ", Cmd_QUICK_OBJECTIVE, NULL, NULL},
  {"QUICK_INFORMATION",                 "NAl     ", Cmd_QUICK_INFORMATION, NULL, NULL},
  {"QUICK_OBJECTIVE_WITH_POS",          "NANN    ", Cmd_QUICK_OBJECTIVE_WITH_POS, NULL, NULL},
  {"QUICK_INFORMATION_WITH_POS",        "NANN    ", Cmd_QUICK_INFORMATION_WITH_POS, NULL, NULL},
  {"SWAP_CREATURE",                     "AC      ", Cmd_SWAP_CREATURE, NULL, NULL},
  {"PRINT",                             "A       ", Cmd_PRINT, NULL, NULL},
  {"MESSAGE",                           "A       ", Cmd_MESSAGE, NULL, NULL},
  {"PLAY_MESSAGE",                      "PAA     ", Cmd_PLAY_MESSAGE, &play_message_check, &play_message_process},
  {"ADD_GOLD_TO_PLAYER",                "PN      ", Cmd_ADD_GOLD_TO_PLAYER, NULL, NULL},
  {"SET_CREATURE_TENDENCIES",           "PAN     ", Cmd_SET_CREATURE_TENDENCIES, NULL, NULL},
  {"REVEAL_MAP_RECT",                   "PNNNN   ", Cmd_REVEAL_MAP_RECT, NULL, NULL},
  {"CONCEAL_MAP_RECT",                  "PNNNNa  ", Cmd_CONCEAL_MAP_RECT, &conceal_map_rect_check, &conceal_map_rect_process},
  {"REVEAL_MAP_LOCATION",               "PNN     ", Cmd_REVEAL_MAP_LOCATION, &reveal_map_location_check, &reveal_map_location_process},
  {"LEVEL_VERSION",                     "N       ", Cmd_LEVEL_VERSION, NULL, NULL},
  {"KILL_CREATURE",                     "PC!AN   ", Cmd_KILL_CREATURE, NULL, NULL},
  {"COMPUTER_DIG_TO_LOCATION",          "PLL     ", Cmd_COMPUTER_DIG_TO_LOCATION, NULL, NULL},
  {"USE_POWER_ON_CREATURE",             "PC!APANN", Cmd_USE_POWER_ON_CREATURE, NULL, NULL},
  {"USE_POWER_AT_POS",                  "PNNANN  ", Cmd_USE_POWER_AT_POS, NULL, NULL},
  {"USE_POWER_AT_SUBTILE",              "PNNANN  ", Cmd_USE_POWER_AT_POS, NULL, NULL},  //todo: Remove after mapmakers have received time to use USE_POWER_AT_POS
  {"USE_POWER_AT_LOCATION",             "PLANN   ", Cmd_USE_POWER_AT_LOCATION, NULL, NULL},
  {"USE_POWER",                         "PAN     ", Cmd_USE_POWER, NULL, NULL},
  {"USE_SPECIAL_INCREASE_LEVEL",        "PN      ", Cmd_USE_SPECIAL_INCREASE_LEVEL, NULL, NULL},
  {"USE_SPECIAL_MULTIPLY_CREATURES",    "PN      ", Cmd_USE_SPECIAL_MULTIPLY_CREATURES, NULL, NULL},
  {"USE_SPECIAL_MAKE_SAFE",             "P       ", Cmd_USE_SPECIAL_MAKE_SAFE, NULL, NULL},
  {"USE_SPECIAL_LOCATE_HIDDEN_WORLD",   "        ", Cmd_USE_SPECIAL_LOCATE_HIDDEN_WORLD, NULL, NULL},
  {"USE_SPECIAL_TRANSFER_CREATURE",     "P       ", Cmd_USE_SPECIAL_TRANSFER_CREATURE, &special_transfer_creature_check, &special_transfer_creature_process},
  {"TRANSFER_CREATURE",                 "PC!An   ", Cmd_TRANSFER_CREATURE, &script_transfer_creature_check, &script_transfer_creature_process},
  {"CHANGE_CREATURES_ANNOYANCE",        "PC!AN   ", Cmd_CHANGE_CREATURES_ANNOYANCE, &change_creatures_annoyance_check, &change_creatures_annoyance_process},
  {"ADD_TO_FLAG",                       "PAN     ", Cmd_ADD_TO_FLAG, NULL, NULL},
  {"SET_CAMPAIGN_FLAG",                 "PAN     ", Cmd_SET_CAMPAIGN_FLAG, NULL, NULL},
  {"ADD_TO_CAMPAIGN_FLAG",              "PAN     ", Cmd_ADD_TO_CAMPAIGN_FLAG, NULL, NULL},
  {"EXPORT_VARIABLE",                   "PAA     ", Cmd_EXPORT_VARIABLE, NULL, NULL},
  {"RUN_AFTER_VICTORY",                 "N       ", Cmd_RUN_AFTER_VICTORY, NULL, NULL},
  {"LEVEL_UP_CREATURE",                 "PC!AN   ", Cmd_LEVEL_UP_CREATURE, NULL, NULL},
  {"CHANGE_CREATURE_OWNER",             "PC!AP   ", Cmd_CHANGE_CREATURE_OWNER, NULL, NULL},
  {"SET_GAME_RULE",                     "AN      ", Cmd_SET_GAME_RULE, NULL, NULL},
  {"SET_ROOM_CONFIGURATION",            "AAAa!n! ", Cmd_SET_ROOM_CONFIGURATION, &set_room_configuration_check, &set_room_configuration_process},
  {"SET_TRAP_CONFIGURATION",            "AAAn!n! ", Cmd_SET_TRAP_CONFIGURATION, &set_trap_configuration_check, &set_trap_configuration_process},
  {"SET_DOOR_CONFIGURATION",            "AAAn!   ", Cmd_SET_DOOR_CONFIGURATION, &set_door_configuration_check, &set_door_configuration_process},
  {"SET_OBJECT_CONFIGURATION",          "AAA     ", Cmd_SET_OBJECT_CONFIGURATION, &set_object_configuration_check, &set_object_configuration_process},
  {"SET_CREATURE_CONFIGURATION",        "CAAa    ", Cmd_SET_CREATURE_CONFIGURATION, &set_creature_configuration_check, &set_creature_configuration_process},
  {"SET_SACRIFICE_RECIPE",              "AAA+    ", Cmd_SET_SACRIFICE_RECIPE, &set_sacrifice_recipe_check, &set_sacrifice_recipe_process},
  {"REMOVE_SACRIFICE_RECIPE",           "A+      ", Cmd_REMOVE_SACRIFICE_RECIPE, &remove_sacrifice_recipe_check, &set_sacrifice_recipe_process},
  {"SET_BOX_TOOLTIP",                   "NA      ", Cmd_SET_BOX_TOOLTIP, &set_box_tooltip, &null_process},
  {"SET_BOX_TOOLTIP_ID",                "NN      ", Cmd_SET_BOX_TOOLTIP_ID, &set_box_tooltip_id, &null_process},
  {"CHANGE_SLAB_OWNER",                 "NNPa    ", Cmd_CHANGE_SLAB_OWNER, &change_slab_owner_check, &change_slab_owner_process},
  {"CHANGE_SLAB_TYPE",                  "NNSa    ", Cmd_CHANGE_SLAB_TYPE, &change_slab_type_check, &change_slab_type_process},
  {"CREATE_EFFECTS_LINE",               "LLNNNA  ", Cmd_CREATE_EFFECTS_LINE, &create_effects_line_check, &create_effects_line_process},
  {"IF_SLAB_OWNER",                     "NNP     ", Cmd_IF_SLAB_OWNER, NULL, NULL},
  {"IF_SLAB_TYPE",                      "NNS     ", Cmd_IF_SLAB_TYPE, NULL, NULL},
  {"QUICK_MESSAGE",                     "NAA     ", Cmd_QUICK_MESSAGE, NULL, NULL},
  {"DISPLAY_MESSAGE",                   "NA      ", Cmd_DISPLAY_MESSAGE, NULL, NULL},
  {"USE_SPELL_ON_CREATURE",             "PC!AAn  ", Cmd_USE_SPELL_ON_CREATURE, NULL, NULL},
  {"USE_SPELL_ON_PLAYERS_CREATURES",    "PC!An   ", Cmd_USE_SPELL_ON_PLAYERS_CREATURES, &use_spell_on_creature_check, &use_spell_on_creature_process },
  {"SET_HEART_HEALTH",                  "PN      ", Cmd_SET_HEART_HEALTH, &set_heart_health_check, &set_heart_health_process},
  {"ADD_HEART_HEALTH",                  "PNn     ", Cmd_ADD_HEART_HEALTH, &add_heart_health_check, &add_heart_health_process},
  {"CREATURE_ENTRANCE_LEVEL",           "PN      ", Cmd_CREATURE_ENTRANCE_LEVEL, NULL, NULL},
  {"RANDOMISE_FLAG",                    "PAN     ", Cmd_RANDOMISE_FLAG, NULL, NULL},
  {"COMPUTE_FLAG",                      "PAAPAN  ", Cmd_COMPUTE_FLAG, NULL, NULL},
  {"DISPLAY_TIMER",                     "PAn     ", Cmd_DISPLAY_TIMER, &display_timer_check, &display_timer_process},
  {"ADD_TO_TIMER",                      "PAN     ", Cmd_ADD_TO_TIMER, &add_to_timer_check, &add_to_timer_process},
  {"ADD_BONUS_TIME",                    "N       ", Cmd_ADD_BONUS_TIME, &add_bonus_time_check, &add_bonus_time_process},
  {"DISPLAY_VARIABLE",                  "PAnn    ", Cmd_DISPLAY_VARIABLE, &display_variable_check, &display_variable_process},
  {"DISPLAY_COUNTDOWN",                 "PANn    ", Cmd_DISPLAY_COUNTDOWN, &display_countdown_check, &display_timer_process},
  {"HIDE_TIMER",                        "        ", Cmd_HIDE_TIMER, &cmd_no_param_check, &hide_timer_process},
  {"HIDE_VARIABLE",                     "        ", Cmd_HIDE_VARIABLE, &cmd_no_param_check, &hide_variable_process},
  {"CREATE_EFFECT",                     "AAn     ", Cmd_CREATE_EFFECT, &create_effect_check, &create_effect_process},
  {"CREATE_EFFECT_AT_POS",              "ANNn    ", Cmd_CREATE_EFFECT_AT_POS, &create_effect_at_pos_check, &create_effect_process},
  {"HEART_LOST_QUICK_OBJECTIVE",        "NAl     ", Cmd_HEART_LOST_QUICK_OBJECTIVE, &heart_lost_quick_objective_check, &heart_lost_quick_objective_process},
  {"HEART_LOST_OBJECTIVE",              "Nl      ", Cmd_HEART_LOST_OBJECTIVE, &heart_lost_objective_check, &heart_lost_objective_process},
  {"SET_DOOR",                          "ANN     ", Cmd_SET_DOOR, &set_door_check, &set_door_process},
  {"SET_CREATURE_INSTANCE",             "CNAN    ", Cmd_SET_CREATURE_INSTANCE, &set_creature_instance_check, &set_creature_instance_process},
  {"SET_HAND_RULE",                     "PC!Aaaa ", Cmd_SET_HAND_RULE, &set_hand_rule_check, &set_hand_rule_process},
  {"MOVE_CREATURE",                     "PC!ANLa ", Cmd_MOVE_CREATURE, &move_creature_check, &move_creature_process},
  {"COUNT_CREATURES_AT_ACTION_POINT",   "NPC!PA  ", Cmd_COUNT_CREATURES_AT_ACTION_POINT, &count_creatures_at_action_point_check, &count_creatures_at_action_point_process},
  {"IF_ALLIED",                         "PPON    ", Cmd_IF_ALLIED, &if_allied_check, NULL},
  {"SET_TEXTURE",                       "PA      ", Cmd_SET_TEXTURE, &set_texture_check, &set_texture_process},
  {"HIDE_HERO_GATE",                    "Nn      ", Cmd_HIDE_HERO_GATE, &hide_hero_gate_check, &hide_hero_gate_process},
  {"NEW_TRAP_TYPE",                     "A       ", Cmd_NEW_TRAP_TYPE, &new_trap_type_check, &null_process},
  {"NEW_OBJECT_TYPE",                   "A       ", Cmd_NEW_OBJECT_TYPE, &new_object_type_check, &null_process},
  {"NEW_ROOM_TYPE",                     "A       ", Cmd_NEW_ROOM_TYPE, &new_room_type_check, &null_process},
  {"NEW_CREATURE_TYPE",                 "A       ", Cmd_NEW_CREATURE_TYPE, &new_creature_type_check, &null_process },
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
  {"SET_HATE",                     "PPN     ", Cmd_SET_HATE, NULL, NULL},
  {"SET_GENERATE_SPEED",           "N       ", Cmd_SET_GENERATE_SPEED, NULL, NULL},
  {"REM",                          "        ", Cmd_REM, NULL, NULL},
  {"START_MONEY",                  "PN      ", Cmd_START_MONEY, NULL, NULL},
  {"ROOM_AVAILABLE",               "PRNN    ", Cmd_ROOM_AVAILABLE, NULL, NULL},
  {"CREATURE_AVAILABLE",           "PCNN    ", Cmd_CREATURE_AVAILABLE, NULL, NULL},
  {"MAGIC_AVAILABLE",              "PANN    ", Cmd_MAGIC_AVAILABLE, NULL, NULL},
  {"TRAP_AVAILABLE",               "PANN    ", Cmd_TRAP_AVAILABLE, NULL, NULL},
  {"RESEARCH",                     "PAAN    ", Cmd_RESEARCH_ORDER, NULL, NULL},
  {"COMPUTER_PLAYER",              "PN      ", Cmd_COMPUTER_PLAYER, NULL, NULL},
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
  {"RESET_ACTION_POINT",           "N       ", Cmd_RESET_ACTION_POINT, NULL, NULL},
  {"SET_CREATURE_MAX_LEVEL",       "PCN     ", Cmd_SET_CREATURE_MAX_LEVEL, NULL, NULL},
  {"SET_MUSIC",                    "N       ", Cmd_SET_MUSIC, NULL, NULL},
  {"TUTORIAL_FLASH_BUTTON",        "NN      ", Cmd_TUTORIAL_FLASH_BUTTON, NULL, NULL},
  {"SET_CREATURE_STRENGTH",        "CN      ", Cmd_SET_CREATURE_STRENGTH, NULL, NULL},
  {"SET_CREATURE_HEALTH",          "CN      ", Cmd_SET_CREATURE_HEALTH, NULL, NULL},
  {"SET_CREATURE_ARMOUR",          "CN      ", Cmd_SET_CREATURE_ARMOUR, NULL, NULL},
  {"SET_CREATURE_FEAR",            "CN      ", Cmd_SET_CREATURE_FEAR_WOUNDED, NULL, NULL},
  {"IF_AVAILABLE",                 "PAOAa   ", Cmd_IF_AVAILABLE, &if_available_check, NULL},
  {"SET_COMPUTER_GLOBALS",         "PNNNNNN ", Cmd_SET_COMPUTER_GLOBALS, NULL, NULL},
  {"SET_COMPUTER_CHECKS",          "PANNNNN ", Cmd_SET_COMPUTER_CHECKS, NULL, NULL},
  {"SET_COMPUTER_EVENT",           "PANN    ", Cmd_SET_COMPUTER_EVENT, NULL, NULL},
  {"SET_COMPUTER_PROCESS",         "PANNNNN ", Cmd_SET_COMPUTER_PROCESS, NULL, NULL},
  {"ALLY_PLAYERS",                 "PP      ", Cmd_ALLY_PLAYERS, NULL, NULL},
  {"DEAD_CREATURES_RETURN_TO_POOL","N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL, NULL, NULL},
  {"BONUS_LEVEL_TIME",             "N       ", Cmd_BONUS_LEVEL_TIME, NULL, NULL},
  {"QUICK_OBJECTIVE",              "NAA     ", Cmd_QUICK_OBJECTIVE, NULL, NULL},
  {"QUICK_INFORMATION",            "NA      ", Cmd_QUICK_INFORMATION, NULL, NULL},
  {"SWAP_CREATURE",                "AC      ", Cmd_SWAP_CREATURE, NULL, NULL},
  {"PRINT",                        "A       ", Cmd_PRINT, NULL, NULL},
  {"MESSAGE",                      "A       ", Cmd_MESSAGE, NULL, NULL},
  {"LEVEL_VERSION",                "N       ", Cmd_LEVEL_VERSION, NULL, NULL},
  {NULL,                           "        ", Cmd_NONE, NULL, NULL},
};


#ifdef __cplusplus
}
#endif
