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
#include "config_effects.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "console_cmd.h"
#include "creature_states_pray.h"
#include "creature_states_mood.h"
#include "room_util.h"

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
  {"ON_NEUTRAL_GROUND",    CSelCrit_OnNeutralGround},
  {"ANYWHERE",             CSelCrit_Any},
  {NULL,                   0},
};

const struct NamedCommand door_config_desc[] = {
  {"ManufactureLevel",      1},
  {"ManufactureRequired",   2},
  {"Health",                3},
  {"SellingValue",          4},
  {"NametextId",            5},
  {"TooltipTextId",         6},
  {"Crate",                 7},
  {"SymbolSprites",         8},
  {"PointerSprites",        9},
  {"PanelTabIndex",        10},
  {NULL,                    0},
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
    int i = dungeon->creatr_list_start;
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
        i = cctrl->players_next_creature_idx;
        // Per creature code
        if (thing->model == crmodel || crmodel == 0)
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
        all = strcmp(scline->tp[5], "1") == 0;

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
    else if (
        (trapvar != 4) && // PointerSprites
        (trapvar != 12) // Model
        )
    {
        if ((scline->np[2] > 0xFFFF) || (scline->np[2] < 0))
        {
            SCRPTERRLOG("Value out of range: %d", scline->np[2]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        value->shorts[2] = (short)scline->np[2];
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
    SCRIPTDBG(7, "Setting trap %s property %d to %d", trapname, trapvar, value->shorts[2]);
    PROCESS_SCRIPT_VALUE(scline->command);
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

static void set_trap_configuration_process(struct ScriptContext *context)
{
    long trap_type = context->value->shorts[0];
    struct TrapConfigStats *trapst = &gameadd.trapdoor_conf.trap_cfgstats[trap_type];
    struct ManfctrConfig *mconf = &gameadd.traps_config[trap_type];
    struct ManufactureData *manufctr = get_manufacture_data(trap_type);
    short value = context->value->shorts[2];
    switch (context->value->shorts[1])
    {
        case 1: // NameTextID
            trapst->name_stridx = value;
            break;
        case 2: // TooltipTextID
            trapst->tooltip_stridx = value;
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
        default:
            WARNMSG("Unsupported Trap configuration, variable %d.", context->value->shorts[1]);
            break;
    }
}

static void set_door_configuration_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    const char *doorname = scline->tp[0];
    short door_id = get_id(door_desc, doorname);
    if (door_id == -1)
    {
        SCRPTERRLOG("Unknown door, '%s'", doorname);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    short doorvar = get_id(door_config_desc, scline->tp[1]);
    if (doorvar == -1)
    {
        SCRPTERRLOG("Unknown door variable");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }

    value->shorts[0] = door_id;
    value->shorts[1] = doorvar;
    if (doorvar == 8) // SymbolSprites
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
    else if (doorvar != 9) // PointerSprites
    {
        if ((scline->np[2] > 0xFFFF) || (scline->np[2] < 0))
        {
            SCRPTERRLOG("Value out of range: %d", scline->np[2]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        value->shorts[2] = (short)scline->np[2];
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
    SCRIPTDBG(7, "Setting door %s property %s to %d", doorname, doorvar, value->shorts[2]);
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void set_door_configuration_process(struct ScriptContext *context)
{
    long door_type = context->value->shorts[0];
    struct DoorConfigStats *doorst = get_door_model_stats(door_type);
    struct ManfctrConfig *mconf = &gameadd.doors_config[door_type];
    struct ManufactureData *manufctr = get_manufacture_data(gameadd.trapdoor_conf.trap_types_count - 1 + door_type);
    short value = context->value->shorts[2];
    switch (context->value->shorts[1])
    {
        case 1: // ManufactureLevel
            mconf->manufct_level = value;
            break;
        case 2: // ManufactureRequired
            mconf->manufct_required = value;
            break;
        case 3: // Health
            if (door_type < DOOR_TYPES_COUNT)
            {
                door_stats[door_type][0].health = value;
                door_stats[door_type][1].health = value;
            }
            update_all_door_stats();
            break;
        case 4: //SellingValue
            mconf->selling_value = value;
            break;
        case 5: // NametextId
            doorst->name_stridx = value;
            break;
        case 6: // TooltipTextId
            doorst->tooltip_stridx = value;
            break;
        case 7: // Crate
            gameadd.object_conf.object_to_door_or_trap[value] = door_type;
            gameadd.object_conf.workshop_object_class[value] = TCls_Door;
            gameadd.trapdoor_conf.door_to_object[door_type] = value;
            break;
        case 8: //SymbolSprites
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
        case 9: // PointerSprites
            doorst->pointer_sprite_idx = get_icon_id(context->value->str2);
            if (doorst->pointer_sprite_idx < 0)
                doorst->pointer_sprite_idx = bad_icon_id;
            update_trap_tab_to_config();
            break;
        case 10: // PanelTabIndex
            doorst->panel_tab_idx = value;
            manufctr->panel_tab_idx = value;
            update_trap_tab_to_config();
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
    struct Thing* efftng;
    if (context->value->chars[0] >= 0)
    {
        efftng = create_effect(&pos, context->value->bytes[0], game.neutral_player_num);
    }
    else
    {
        efftng = create_effect_element(&pos, ~(context->value->bytes[0]) + 1, game.neutral_player_num);
    }
    if (!thing_is_invalid(efftng))
    {
        if (thing_in_wall_at(efftng, &efftng->mappos))
        {
            move_creature_to_nearest_valid_position(efftng);
        }
        if (Price)
        {
            efftng->long_13 = context->value->arg1;
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
        short old_health = heartng->health;
        long new_health = heartng->health + context->value->arg1;
        if (new_health > (signed long)game.dungeon_heart_health)
        {
            SCRIPTDBG(7,"Player %d's calculated heart health (%ld) is greater than maximum: %ld", heartng->owner, new_health, game.dungeon_heart_health);
            new_health = game.dungeon_heart_health;
        }
        heartng->health = (short)new_health;
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
    strncpy(gameadd.quick_messages[scline->np[0]], scline->tp[1], MESSAGE_TEXT_LEN-1);
    gameadd.quick_messages[scline->np[0]][MESSAGE_TEXT_LEN-1] = '\0';
    
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

static void create_effects_line_check(const struct ScriptLine *scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);

    ((long*)(&value->bytes[0]))[0] = scline->np[0]; // AP `from`
    ((long*)(&value->bytes[4]))[0] = scline->np[1]; // AP `to`
    value->chars[8] = scline->np[2]; // curvature
    value->bytes[9] = scline->np[3]; // spatial stepping
    value->bytes[10] = scline->np[4]; // temporal stepping
    // TODO: use effect elements when below zero?
    value->chars[11] = scline->np[5]; // effect

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
    find_location_pos(((long *)(&context->value->bytes[0]))[0], context->player_idx, &fx_line->from, __func__);
    find_location_pos(((long *)(&context->value->bytes[4]))[0], context->player_idx, &fx_line->to, __func__);
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
        float len = sqrt((float)dx * dx + dy * dy);
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

    long number_value;
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
        SCRPTERRLOG("Unknown creature attribute");
        DEALLOCATE_SCRIPT_VALUE
            return;
    }

    if (creatvar == 20) // ATTACKPREFERENCE
    {
        value->shorts[2] = get_id(attackpref_desc, scline->tp[2]);
    }
    else
    {
        value->shorts[2] = atoi(scline->tp[2]);
    }

    SCRIPTDBG(7, "Setting creature %s attribute %d to %d (%d)", creature_code_name(scline->np[0]), scline->np[1], scline->np[2], scline->np[3]);
    value->shorts[0] = scline->np[0];
    value->shorts[1] = creatvar;
    value->shorts[3] = scline->np[3];

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
        //todo
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
    case 0: // comment
        break;
    case -1: // end of buffer
        break;
    default:
        CONFWRNLOG("Unrecognized command (%d)",attribute);
        break;
    }
    check_and_auto_fix_stats();
    creature_stats_updated(creatid);
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
    gameadd.script_value_type = varib_type;
    value->arg1 = varib_id;
    value->arg2 = scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

static void display_variable_process(struct ScriptContext *context)
{
   gameadd.script_player = context->value->bytes[0];
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
        SCRPTERRLOG("Invalid co-ordinates: %ld, %ld", scline->np[1], scline->np[2]);
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
  strncpy(gameadd.box_tooltip[idx], scline->tp[1], MESSAGE_TEXT_LEN-1);
  gameadd.box_tooltip[idx][MESSAGE_TEXT_LEN-1] = '\0';
}

static void set_box_tooltip_id(const struct ScriptLine *scline)
{
  if ((scline->np[0] < 0) || (scline->np[0] >= CUSTOM_BOX_COUNT))
  {
    SCRPTERRLOG("Invalid CUSTOM_BOX number (%ld)", scline->np[0]);
    return;
  }
  int idx = scline->np[0];
  strncpy(gameadd.box_tooltip[idx], get_string(scline->np[1]), MESSAGE_TEXT_LEN-1);
  gameadd.box_tooltip[idx][MESSAGE_TEXT_LEN-1] = '\0';
}

static void change_slab_owner_check(const struct ScriptLine *scline)
{

    if (scline->np[0] < 0 || scline->np[0] > 85) //x coord
    {
        SCRPTERRLOG("Value '%d' out of range. Range 0-85 allowed.", scline->np[0]);
        return;
    }
    if (scline->np[1] < 0 || scline->np[1] > 85) //y coord
    {
        SCRPTERRLOG("Value '%d' out of range. Range 0-85 allowed.", scline->np[1]);
        return;
    }

    long filltype = get_id(fill_desc, scline->tp[3]);
    if ((scline->tp[3] != NULL) && (filltype == -1))
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

    if (scline->np[0] < 0 || scline->np[0] > 85) //x coord
    {
        SCRPTERRLOG("Value '%d' out of range. Range 0-85 allowed.", scline->np[0]);
        return;
    }
    else
    {
        value->shorts[0] = scline->np[0];
    }

    if (scline->np[1] < 0 || scline->np[1] > 85) //y coord
    {
        SCRPTERRLOG("Value '%d' out of range. Range 0-85 allowed.", scline->np[0]);
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
    if ((scline->tp[3] != NULL) && (value->shorts[3] == -1))
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

/**
 * Descriptions of script commands for parser.
 * Arguments are: A-string, N-integer, C-creature model, P- player, R- room kind, L- location, O- operator, S- slab kind, X- creature property
 * Lower case letters are optional arguments.
 */
const struct CommandDesc command_desc[] = {
  {"CREATE_PARTY",                      "A       ", Cmd_CREATE_PARTY, NULL, NULL},
  {"ADD_TO_PARTY",                      "ACNNAN  ", Cmd_ADD_TO_PARTY, &add_to_party_check, NULL},
  {"DELETE_FROM_PARTY",                 "ACN     ", Cmd_DELETE_FROM_PARTY, &delete_from_party_check, NULL},
  {"ADD_PARTY_TO_LEVEL",                "PAAN    ", Cmd_ADD_PARTY_TO_LEVEL, NULL, NULL},
  {"ADD_CREATURE_TO_LEVEL",             "PCANNN  ", Cmd_ADD_CREATURE_TO_LEVEL, NULL, NULL},
  {"ADD_OBJECT_TO_LEVEL",               "AAN     ", Cmd_ADD_OBJECT_TO_LEVEL, NULL, NULL},
  {"IF",                                "PAON    ", Cmd_IF, NULL, NULL},
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
  {"DISPLAY_OBJECTIVE",                 "NL      ", Cmd_DISPLAY_OBJECTIVE, &display_objective_check, &display_objective_process},
  {"DISPLAY_OBJECTIVE_WITH_POS",        "NNN     ", Cmd_DISPLAY_OBJECTIVE_WITH_POS, &display_objective_check, &display_objective_process},
  {"DISPLAY_INFORMATION",               "NL      ", Cmd_DISPLAY_INFORMATION, NULL, NULL},
  {"DISPLAY_INFORMATION_WITH_POS",      "NNN     ", Cmd_DISPLAY_INFORMATION_WITH_POS, NULL, NULL},
  {"ADD_TUNNELLER_PARTY_TO_LEVEL",      "PAAANNN ", Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL, NULL, NULL},
  {"ADD_CREATURE_TO_POOL",              "CN      ", Cmd_ADD_CREATURE_TO_POOL, NULL, NULL},
  {"RESET_ACTION_POINT",                "N       ", Cmd_RESET_ACTION_POINT, NULL, NULL},
  {"SET_CREATURE_MAX_LEVEL",            "PCN     ", Cmd_SET_CREATURE_MAX_LEVEL, NULL, NULL},
  {"SET_MUSIC",                         "N       ", Cmd_SET_MUSIC, NULL, NULL},
  {"TUTORIAL_FLASH_BUTTON",             "NN      ", Cmd_TUTORIAL_FLASH_BUTTON, NULL, NULL},
  {"SET_CREATURE_STRENGTH",             "CN      ", Cmd_SET_CREATURE_STRENGTH, NULL, NULL},
  {"SET_CREATURE_HEALTH",               "CN      ", Cmd_SET_CREATURE_HEALTH, NULL, NULL},
  {"SET_CREATURE_ARMOUR",               "CN      ", Cmd_SET_CREATURE_ARMOUR, NULL, NULL},
  {"SET_CREATURE_FEAR_WOUNDED",         "CN      ", Cmd_SET_CREATURE_FEAR_WOUNDED, NULL, NULL},
  {"SET_CREATURE_FEAR_STRONGER",        "CN      ", Cmd_SET_CREATURE_FEAR_STRONGER, NULL, NULL},
  {"SET_CREATURE_FEARSOME_FACTOR",      "CN      ", Cmd_SET_CREATURE_FEARSOME_FACTOR, NULL, NULL},
  {"SET_CREATURE_PROPERTY",             "CXN     ", Cmd_SET_CREATURE_PROPERTY, NULL, NULL},
  {"IF_AVAILABLE",                      "PAON    ", Cmd_IF_AVAILABLE, NULL, NULL},
  {"IF_CONTROLS",                       "PAON    ", Cmd_IF_CONTROLS, NULL, NULL},
  {"SET_COMPUTER_GLOBALS",              "PNNNNNN ", Cmd_SET_COMPUTER_GLOBALS, NULL, NULL},
  {"SET_COMPUTER_CHECKS",               "PANNNNN ", Cmd_SET_COMPUTER_CHECKS, NULL, NULL},
  {"SET_COMPUTER_EVENT",                "PANNNNN ", Cmd_SET_COMPUTER_EVENT, NULL, NULL},
  {"SET_COMPUTER_PROCESS",              "PANNNNN ", Cmd_SET_COMPUTER_PROCESS, NULL, NULL},
  {"ALLY_PLAYERS",                      "PPN     ", Cmd_ALLY_PLAYERS, NULL, NULL},
  {"DEAD_CREATURES_RETURN_TO_POOL",     "N       ", Cmd_DEAD_CREATURES_RETURN_TO_POOL, NULL, NULL},
  {"BONUS_LEVEL_TIME",                  "Nn      ", Cmd_BONUS_LEVEL_TIME, NULL, NULL},
  {"QUICK_OBJECTIVE",                   "NAL     ", Cmd_QUICK_OBJECTIVE, NULL, NULL},
  {"QUICK_INFORMATION",                 "NAL     ", Cmd_QUICK_INFORMATION, NULL, NULL},
  {"QUICK_OBJECTIVE_WITH_POS",          "NANN    ", Cmd_QUICK_OBJECTIVE_WITH_POS, NULL, NULL},
  {"QUICK_INFORMATION_WITH_POS",        "NANN    ", Cmd_QUICK_INFORMATION_WITH_POS, NULL, NULL},
  {"SWAP_CREATURE",                     "AC      ", Cmd_SWAP_CREATURE, NULL, NULL},
  {"PRINT",                             "A       ", Cmd_PRINT, NULL, NULL},
  {"MESSAGE",                           "A       ", Cmd_MESSAGE, NULL, NULL},
  {"PLAY_MESSAGE",                      "PAN     ", Cmd_PLAY_MESSAGE, NULL, NULL},
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
  {"USE_POWER_AT_LOCATION",             "PNANN   ", Cmd_USE_POWER_AT_LOCATION, NULL, NULL},
  {"USE_POWER",                         "PAN     ", Cmd_USE_POWER, NULL, NULL},
  {"USE_SPECIAL_INCREASE_LEVEL",        "PN      ", Cmd_USE_SPECIAL_INCREASE_LEVEL, NULL, NULL},
  {"USE_SPECIAL_MULTIPLY_CREATURES",    "PN      ", Cmd_USE_SPECIAL_MULTIPLY_CREATURES, NULL, NULL},
  {"USE_SPECIAL_MAKE_SAFE",             "P       ", Cmd_USE_SPECIAL_MAKE_SAFE, NULL, NULL},
  {"USE_SPECIAL_LOCATE_HIDDEN_WORLD",   "        ", Cmd_USE_SPECIAL_LOCATE_HIDDEN_WORLD, NULL, NULL},
  {"CHANGE_CREATURES_ANNOYANCE",        "PC!AN   ", Cmd_CHANGE_CREATURES_ANNOYANCE, &change_creatures_annoyance_check, &change_creatures_annoyance_process},
  {"ADD_TO_FLAG",                       "PAN     ", Cmd_ADD_TO_FLAG, NULL, NULL},
  {"SET_CAMPAIGN_FLAG",                 "PAN     ", Cmd_SET_CAMPAIGN_FLAG, NULL, NULL},
  {"ADD_TO_CAMPAIGN_FLAG",              "PAN     ", Cmd_ADD_TO_CAMPAIGN_FLAG, NULL, NULL},
  {"EXPORT_VARIABLE",                   "PAA     ", Cmd_EXPORT_VARIABLE, NULL, NULL},
  {"RUN_AFTER_VICTORY",                 "N       ", Cmd_RUN_AFTER_VICTORY, NULL, NULL},
  {"LEVEL_UP_CREATURE",                 "PC!AN   ", Cmd_LEVEL_UP_CREATURE, NULL, NULL},
  {"CHANGE_CREATURE_OWNER",             "PC!AP   ", Cmd_CHANGE_CREATURE_OWNER, NULL, NULL},
  {"SET_GAME_RULE",                     "AN      ", Cmd_SET_GAME_RULE, NULL, NULL},
  {"SET_TRAP_CONFIGURATION",            "AANn    ", Cmd_SET_TRAP_CONFIGURATION, &set_trap_configuration_check, &set_trap_configuration_process},
  {"SET_DOOR_CONFIGURATION",            "AANn    ", Cmd_SET_DOOR_CONFIGURATION, &set_door_configuration_check, &set_door_configuration_process},
  {"SET_OBJECT_CONFIGURATION",          "AAA     ", Cmd_SET_OBJECT_CONFIGURATION, &set_object_configuration_check, &set_object_configuration_process},
  {"SET_CREATURE_CONFIGURATION",        "CAAn    ", Cmd_SET_CREATURE_CONFIGURATION, &set_creature_configuration_check, &set_creature_configuration_process},
  {"SET_SACRIFICE_RECIPE",              "AAA+    ", Cmd_SET_SACRIFICE_RECIPE, &set_sacrifice_recipe_check, &set_sacrifice_recipe_process},
  {"REMOVE_SACRIFICE_RECIPE",           "A+      ", Cmd_REMOVE_SACRIFICE_RECIPE, &remove_sacrifice_recipe_check, &set_sacrifice_recipe_process},
  {"SET_BOX_TOOLTIP",                   "NA      ", Cmd_SET_BOX_TOOLTIP, &set_box_tooltip, &null_process},
  {"SET_BOX_TOOLTIP_ID",                "NN      ", Cmd_SET_BOX_TOOLTIP_ID, &set_box_tooltip_id, &null_process},
  {"CHANGE_SLAB_OWNER",                 "NNPa    ", Cmd_CHANGE_SLAB_OWNER, &change_slab_owner_check, &change_slab_owner_process},
  {"CHANGE_SLAB_TYPE",                  "NNSa    ", Cmd_CHANGE_SLAB_TYPE, &change_slab_type_check, &change_slab_type_process},
  {"CREATE_EFFECTS_LINE",               "LLNNNN  ", Cmd_CREATE_EFFECTS_LINE, &create_effects_line_check, &create_effects_line_process},
  {"IF_SLAB_OWNER",                     "NNP     ", Cmd_IF_SLAB_OWNER, NULL, NULL},
  {"IF_SLAB_TYPE",                      "NNS     ", Cmd_IF_SLAB_TYPE, NULL, NULL},
  {"QUICK_MESSAGE",                     "NAA     ", Cmd_QUICK_MESSAGE, NULL, NULL},
  {"DISPLAY_MESSAGE",                   "NA      ", Cmd_DISPLAY_MESSAGE, NULL, NULL},
  {"USE_SPELL_ON_CREATURE",             "PC!AAN  ", Cmd_USE_SPELL_ON_CREATURE, NULL, NULL},
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
  {NULL,                                "        ", Cmd_NONE, NULL, NULL},
};

const struct CommandDesc dk1_command_desc[] = {
  {"CREATE_PARTY",                 "A       ", Cmd_CREATE_PARTY, NULL, NULL},
  {"ADD_TO_PARTY",                 "ACNNAN  ", Cmd_ADD_TO_PARTY, &add_to_party_check, NULL},
  {"ADD_PARTY_TO_LEVEL",           "PAAN    ", Cmd_ADD_PARTY_TO_LEVEL, NULL, NULL},
  {"ADD_CREATURE_TO_LEVEL",        "PCANNN  ", Cmd_ADD_CREATURE_TO_LEVEL, NULL, NULL},
  {"IF",                           "PAON    ", Cmd_IF, NULL, NULL},
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
  {"IF_AVAILABLE",                 "PAON    ", Cmd_IF_AVAILABLE, NULL, NULL},
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