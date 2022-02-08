/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script.c
 *     Level script commands support.
 * @par Purpose:
 *     Load, recognize and maintain the level script.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     12 Feb 2009 - 11 Apr 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <math.h>

#include "lvl_script.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_sound.h"
#include "bflib_guibtns.h"
#include "engine_redraw.h"
#include "front_simple.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_effects.h"
#include "config_lenses.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "creature_states_mood.h"
#include "creature_control.h"
#include "gui_msgs.h"
#include "gui_soundmsgs.h"
#include "gui_topmsg.h"
#include "frontmenu_ingame_evnt.h"
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
#include "power_specials.h"
#include "map_blocks.h"
#include "lvl_filesdk1.h"
#include "frontend.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "music_player.h"
#include "custom_sprites.h"
#include "console_cmd.h"
#include "creature_states_hero.h"
#include "map_locations.h"
#include "creature_groups.h"
#include "actionpt.h"


#include "lvl_script_commands.h"
#include "lvl_script_lib.h"
#include "lvl_script_statehandler.h"
#include "lvl_script_conditions.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

extern void process_sacrifice_creature(struct Coord3d *pos, int model, int owner, TbBool partial);
extern TbBool find_temple_pool(int player_idx, struct Coord3d *pos);
extern void find_location_pos(long location, PlayerNumber plyr_idx, struct Coord3d *pos, const char *func_name);

extern long near_map_block_creature_filter_diagonal_random(const struct Thing *thing, MaxTngFilterParam param, long maximizer);

void script_process_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4, struct ScriptValue *value);



const struct CommandDesc dk1_command_desc[];
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

#define DEALLOCATE_SCRIPT_VALUE \
    if (value != &tmp_value) \
    {                           \
        value->flags = TrgF_DISABLED; \
        gameadd.script.values_num--; \
    }

#define PROCESS_SCRIPT_VALUE(cmd) \
    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0)) \
    { \
        script_process_value(cmd, 0, 0, 0, 0, value); \
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
/******************************************************************************/
DLLIMPORT long _DK_script_support_send_tunneller_to_appropriate_dungeon(struct Thing *creatng);
/******************************************************************************/
static int filter_criteria_type(long desc_type)
{
    return desc_type & 0x0F;
}

static long filter_criteria_loc(long desc_type)
{
    return desc_type >> 4;
}
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
        while (isalnum(chr) || (chr == '_') || (chr == '[') || (chr == ']') || (chr == ':'))
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
    if (plr_range_id == -1)
    {
        plr_range_id = get_rid(cmpgn_human_player_options, plrname);
    }
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
      *plr_range_id = get_rid(cmpgn_human_player_options, plrname);
      if (*plr_range_id == -1)
      {
        ERRORMSG("%s(line %lu): Invalid player name, '%s'",func_name,ln_num, plrname);
        return false;
      }
    }
    return true;
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

static void null_process(struct ScriptContext *context)
{
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

static void add_to_party_process(struct ScriptContext *context)
{
    struct PartyTrigger* pr_trig = context->pr_trig;
    add_member_to_party(pr_trig->party_id, pr_trig->creatr_id, pr_trig->crtr_level, pr_trig->carried_gold, pr_trig->objectv, pr_trig->countdown);
}

static int sac_compare_fn(const void *ptr_a, const void *ptr_b)
{
    const char *a = (const char*)ptr_a;
    const char *b = (const char*)ptr_b;
    return *a < *b;
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

// 1/4 turn minimal
#define FX_LINE_TIME_PARTS 4

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

static void process_fx_line(struct ScriptFxLine *fx_line)
{
    fx_line->partial_steps += fx_line->steps_per_turn;
    for (;fx_line->partial_steps >= FX_LINE_TIME_PARTS; fx_line->partial_steps -= FX_LINE_TIME_PARTS)
    {
        fx_line->here.z.val = get_floor_height_at(&fx_line->here);
        if (fx_line->here.z.val < FILLED_COLUMN_HEIGHT)
        {
          if (fx_line->effect > 0)
          {
            create_effect(&fx_line->here, fx_line->effect, 5); // Owner - neutral
          } else if (fx_line->effect < 0)
          {
            create_effect_element(&fx_line->here, -fx_line->effect, 5); // Owner - neutral
          }
        }

        fx_line->step++;
        if (fx_line->step >= fx_line->total_steps)
        {
          fx_line->used = false;
          break;
        }

        int remain_t = fx_line->total_steps - fx_line->step;

        int bx = fx_line->from.x.val * remain_t + fx_line->cx * fx_line->step;
        int by = fx_line->from.y.val * remain_t + fx_line->cy * fx_line->step;
        int dx = fx_line->cx * remain_t + fx_line->to.x.val * fx_line->step;
        int dy = fx_line->cy * remain_t + fx_line->to.y.val * fx_line->step;

        fx_line->here.x.val = (bx * remain_t + dx * fx_line->step) / fx_line->total_steps / fx_line->total_steps;
        fx_line->here.y.val = (by * remain_t + dy * fx_line->step) / fx_line->total_steps / fx_line->total_steps;
    }
}



static void player_reveal_map_area(PlayerNumber plyr_idx, long x, long y, long w, long h)
{
  SYNCDBG(0,"Revealing around (%d,%d)",x,y);
  reveal_map_area(plyr_idx, x-(w>>1), x+(w>>1)+(w%1), y-(h>>1), y+(h>>1)+(h%1));
}

void player_reveal_map_location(int plyr_idx, TbMapLocation target, long r)
{
    SYNCDBG(0, "Revealing location type %d", target);
    long x = 0;
    long y = 0;
    find_map_location_coords(target, &x, &y, plyr_idx, __func__);
    if ((x == 0) && (y == 0))
    {
        WARNLOG("Can't decode location %d", target);
        return;
  }
  reveal_map_area(plyr_idx, x-(r>>1), x+(r>>1)+(r&1), y-(r>>1), y+(r>>1)+(r&1));
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

void set_trap_configuration_process(struct ScriptContext *context)
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

void set_door_configuration_process(struct ScriptContext *context)
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

static TbBool script_command_param_to_number(char type_chr, struct ScriptLine *scline, int idx, TbBool extended)
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
        if (extended)
        {
            if (crtr_id == -1)
            {
                if (0 == strcmp(scline->tp[idx], "ANY_CREATURE"))
                {
                    crtr_id = 0;
                }
            }
        }
        if (crtr_id == -1)
        {
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
    case '!': // extended sign
        return true;
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
    case '!': // extended sign
        return true;
    default:
        return false;
    }
    return true;
}

static int count_required_parameters(const char *args)
{
    int required = 0;
    for (int i = 0; i < COMMANDDESC_ARGS_COUNT; i++)
    {
        char chr = args[i];
        if (isupper(chr)) // Required arguments have upper-case type letters
        {
            required++;
        }
        else if (chr == '!')
        {
            continue;
        }
        else
            break;
    }
    return required;
}

int script_recognize_params(char **line, const struct CommandDesc *cmd_desc, struct ScriptLine *scline, int *para_level, int expect_level)
{
    int dst, src;
    for (dst = 0, src = 0; dst <= COMMANDDESC_ARGS_COUNT; dst++, src++)
    {
        TbBool extended = false;
        char chr = cmd_desc->args[src];
        if (*para_level < expect_level)
            break;
        if (chr == '!')
        {
            dst--;
            continue;
        }
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
            if (funpara_level > (*para_level)+(dst > 0 ? 0 : 1)) {
                SCRPTWRNLOG("Unexpected paraenesis in parameter %d of command \"%s\"", dst + 1, scline->tcmnd);
            }
            *line = funline;
            *para_level = funpara_level;
            if (!isalpha(chr))
            {
                // Don't show parameter index - it may be bad, as we're decreasing dst to not overflow cmd_desc->args
                SCRPTWRNLOG("Excessive parameter of command \"%s\", value \"%s\"; ignoring", scline->tcmnd, funcmd_buf);
                dst--;
                continue;
            }
            // Access tp[dst] only if we're sure dst < COMMANDDESC_ARGS_COUNT
            LbMemoryCopy(scline->tp[dst], funcmd_buf, MAX_TEXT_LENGTH);
        }
        if (funcmd_desc != NULL)
        {
            struct ScriptLine* funscline = (struct ScriptLine*)LbMemoryAlloc(sizeof(struct ScriptLine));
            if (funscline == NULL) {
                SCRPTERRLOG("Can't allocate buffer to recognize line");
                return -1;
            }
            LbMemorySet(funscline, 0, sizeof(struct ScriptLine));
            LbMemoryCopy(funscline->tcmnd, scline->tp[dst], MAX_TEXT_LENGTH);
            int args_count = script_recognize_params(line, funcmd_desc, funscline, para_level, *para_level);
            if (args_count < 0)
            {
                LbMemoryFree(funscline);
                return -1;
            }
            // Count valid args
            if (args_count < COMMANDDESC_ARGS_COUNT)
            {
                int required = count_required_parameters(funcmd_desc->args);
                if (args_count < required)
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
                    chr = cmd_desc->args[src];
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
                            if (!script_command_param_to_number(chr, funscline, fi, false)) {
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
                            if (!script_command_param_to_number(chr, funscline, fi, false)) {
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
                        chr = cmd_desc->args[src];
                        if (toupper(chr) == 'A') {
                            strcpy(scline->tp[dst], funscline->tp[ranges[fi].min]);
                        } else {
                            scline->np[dst] = ranges[fi].min + range_index - range_total;
                            // Set text value for that number
                            script_command_param_to_text(chr, scline, dst);
                        }
                        break;
                    }
                    range_total += ranges[fi].max - ranges[fi].min + 1;
                }
                SCRPTLOG("Function \"%s\" returned value \"%s\"", funcmd_desc->textptr, scline->tp[dst]);
                };break;
            case Cmd_IMPORT:
            {
                long player_id = get_id(player_desc, funscline->tp[0]);
                if (player_id >= PLAYERS_FOR_CAMPAIGN_FLAGS)
                {
                    SCRPTERRLOG("Cannot fetch flag values for player, '%s'", funscline->tp[0]);
                    strcpy(scline->tp[dst], "0");
                    break;
                }
                long flag_id = get_id(campaign_flag_desc, funscline->tp[1]);
                if (flag_id == -1)
                {
                    SCRPTERRLOG("Unknown campaign flag name, '%s'", funscline->tp[1]);
                    strcpy(scline->tp[dst], "0");
                    break;
                }
                SCRPTLOG("Function \"%s\" returned value \"%ld\"", funcmd_desc->textptr,
                    intralvl.campaign_flags[player_id][flag_id]);
                ltoa(intralvl.campaign_flags[player_id][flag_id], scline->tp[dst], 10);
                break;
            }
            default:
                SCRPTWRNLOG("Parameter value \"%s\" is a command which isn't supported as function", scline->tp[dst]);
                break;
            }
            LbMemoryFree(funscline);
        }
        if (scline->tp[dst][0] == '\0') {
          break;
        }
        if (*para_level > expect_level+2) {
            SCRPTWRNLOG("Parameter %d of command \"%s\", value \"%s\", is at too high paraenesis level %d", dst + 1, scline->tcmnd, scline->tp[dst], (int)*para_level);
        }
        chr = cmd_desc->args[src];
        if (cmd_desc->args[src + 1] == '+')
        {
            // All other parameters will be same
            src -= 1;
        }
        if (cmd_desc->args[src + 1] == '!') //extended set (dst.e. ANY_CREATURE)
        {
            extended = true;
        }
        if (!script_command_param_to_number(chr, scline, dst, extended)) {
            SCRPTERRLOG("Parameter %d of command \"%s\", type %c, has unexpected value; discarding command", dst + 1, scline->tcmnd, chr);
            return -1;
        }
    }
    return dst;
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
    scline->command = cmd_desc->index;
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
        int required = count_required_parameters(cmd_desc->args);
        if (args_count < required) // Required arguments have upper-case type letters
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
    LbMemorySet(&game.script, 0, sizeof(struct LevelScriptOld));
    LbMemorySet(&gameadd.script, 0, sizeof(struct LevelScript));
    gameadd.script.next_string = gameadd.script.strings;
    set_script_current_condition(CONDITION_ALWAYS);
    text_line_number = 1;
    return true;
}

short clear_quick_messages(void)
{
    for (long i = 0; i < QUICK_MESSAGES_COUNT; i++)
        LbMemorySet(gameadd.quick_messages[i], 0, MESSAGE_TEXT_LEN);
    return true;
}

static char* process_multiline_comment(char *buf, char *buf_end)
{
    for (char *p = buf; p < buf_end - 1; p++)
    {
        if ((*p == ' ') || (*p == 9)) // Tabs or spaces
            continue;
        if (p[0] == '/') // /
        {
            if (p[1] != '*') // /*
                break;
            p += 2;
            for (; p < buf_end - 1; p++)
            {
                if ((p[0] == '*') && (p[1] == '/'))
                {
                    buf = p + 2;
                    break;
                }
            }
            break;
        }
        else
            break;
    }
    return buf;
}

short preload_script(long lvnum)
{
  SYNCDBG(7,"Starting");
  set_script_current_condition(CONDITION_ALWAYS);
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
      // Check for long comment
      buf = process_multiline_comment(buf, buf_end);
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
    set_script_current_condition(CONDITION_ALWAYS);
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
        buf = process_multiline_comment(buf, buf_end);
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
    if (gameadd.script.win_conditions_num == 0)
      WARNMSG("No WIN GAME conditions in script file.");
    if (get_script_current_condition() != CONDITION_ALWAYS)
      WARNMSG("Missing ENDIF's in script file.");
    JUSTLOG("Used script resources: %d/%d tunneller triggers, %d/%d party triggers, %d/%d script values, %d/%d IF conditions, %d/%d party definitions",
        (int)gameadd.script.tunneller_triggers_num,TUNNELLER_TRIGGERS_COUNT,
        (int)gameadd.script.party_triggers_num,PARTY_TRIGGERS_COUNT,
        (int)gameadd.script.values_num,SCRIPT_VALUES_COUNT,
        (int)gameadd.script.conditions_num,CONDITIONS_COUNT,
        (int)gameadd.script.creature_partys_num,CREATURE_PARTYS_COUNT);
    return true;
}



struct Thing *get_creature_in_range_around_any_of_enemy_heart(PlayerNumber plyr_idx, ThingModel crmodel, MapSubtlDelta range)
{
    int n = GAME_RANDOM(PLAYERS_COUNT);
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

static struct Thing *script_get_creature_by_criteria(PlayerNumber plyr_idx, long crmodel, long criteria) {
    switch (filter_criteria_type(criteria))
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
        return get_creature_near_and_owned_by(pos->x.val, pos->y.val, plyr_idx, crmodel);
    }
    case CSelCrit_NearEnemyHeart:
        return get_creature_in_range_around_any_of_enemy_heart(plyr_idx, crmodel, 11);
    case CSelCrit_OnEnemyGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 0);
    case CSelCrit_OnFriendlyGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 1);
    case CSelCrit_NearAP:
    {
        int loc = filter_criteria_loc(criteria);
        struct ActionPoint *apt = action_point_get(loc);
        if (!action_point_exists(apt))
        {
            WARNLOG("Action point is invalid:%d", apt->num);
            return INVALID_THING;
        }
        if (apt->range == 0)
        {
            WARNLOG("Action point with zero range:%d", apt->num);
            return INVALID_THING;
        }
        // Action point range should be inside spiral in subtiles
        int dist = 2 * coord_subtile(apt->range + COORD_PER_STL - 1 ) + 1;
        dist = dist * dist;

        Thing_Maximizer_Filter filter = near_map_block_creature_filter_diagonal_random;
        struct CompoundTngFilterParam param;
        param.model_id = crmodel;
        param.plyr_idx = (unsigned char)plyr_idx;
        param.num1 = apt->mappos.x.val;
        param.num2 = apt->mappos.y.val;
        param.num3 = apt->range;
        return get_thing_spiral_near_map_block_with_filter(apt->mappos.x.val, apt->mappos.y.val,
                                                           dist,
                                                           filter, &param);
    }
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
    change_creature_owner(thing, dest_plyr_idx);
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
          SYNCDBG(5,"Found creature to use power on but it is being held.");
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
        SCRPTERRLOG("Power not supported for this command: %d", (int) pwkind);
        return Lb_FAIL;
    }
}

TbResult script_use_spell_on_creature(PlayerNumber plyr_idx, long crmodel, long criteria, long fmcl_bytes)
{
    struct Thing *thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing)) {
        SYNCDBG(5,"No matching player %d creature of model %d found to use spell on.",(int)plyr_idx,(int)crmodel);
        return Lb_FAIL;
    }
    SpellKind spkind = (fmcl_bytes >> 8) & 255;
    const struct SpellInfo* spinfo = get_magic_info(spkind);

    if (spinfo->caster_affected ||
            (spkind == SplK_Freeze) || (spkind == SplK_Slow) || // These four should be also marked at configs somehow
            ( (spkind == SplK_Disease) && ((get_creature_model_flags(thing) & CMF_NeverSick) == 0) ) ||
            ( (spkind == SplK_Chicken) && ((get_creature_model_flags(thing) & CMF_NeverChickens) == 0) ) )
    {
        if (thing_is_picked_up(thing))
        {
            SYNCDBG(5,"Found creature to cast the spell on but it is being held.");
            return Lb_FAIL;
        }
        unsigned short sound;
        if (spinfo->caster_affected)
        {
            sound = spinfo->caster_affect_sound;
        }
        else if ( (spkind == SplK_Freeze) || (spkind == SplK_Slow) )
        {
            sound = 50;
        }
        else if (spkind == SplK_Disease)
        {
            sound = 59;
        }
        else if (spkind == SplK_Chicken)
        {
            sound = 109;
        }
        else
        {
            sound = 0;
        }
        long splevel = fmcl_bytes & 255;
        thing_play_sample(thing, sound, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
        apply_spell_effect_to_thing(thing, spkind, splevel);
        if (spkind == SplK_Disease)
        {
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(thing);
            cctrl->disease_caster_plyridx = game.neutral_player_num;
        }
        return Lb_SUCCESS;
    }
    else
    {
        SCRPTERRLOG("Spell not supported for this command: %d", (int)spkind);
        return Lb_FAIL;
    }
}

/**
 * Adds a dig task for the player between 2 map locations.
 * @param plyr_idx: The player who does the task.
 * @param origin: The start location of the disk task.
 * @param destination: The desitination of the disk task.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_computer_dig_to_location(long plyr_idx, long origin, long destination)
{
    struct Computer2* comp = get_computer_player(plyr_idx);
    long orig_x, orig_y = 0;
    long dest_x, dest_y = 0;

    //dig origin
    find_map_location_coords(origin, &orig_x, &orig_y, plyr_idx, __func__);
    if ((orig_x == 0) && (orig_y == 0))
    {
        WARNLOG("Can't decode origin location %d", origin);
        return Lb_FAIL;
    }
    struct Coord3d startpos;
    startpos.x.val = subtile_coord_center(stl_slab_center_subtile(orig_x));
    startpos.y.val = subtile_coord_center(stl_slab_center_subtile(orig_y));
    startpos.z.val = subtile_coord(1, 0);

    //dig destination
    find_map_location_coords(destination, &dest_x, &dest_y, plyr_idx, __func__);
    if ((dest_x == 0) && (dest_y == 0))
    {
        WARNLOG("Can't decode destination location %d", destination);
        return Lb_FAIL;
    }
    struct Coord3d endpos;
    endpos.x.val = subtile_coord_center(stl_slab_center_subtile(dest_x));
    endpos.y.val = subtile_coord_center(stl_slab_center_subtile(dest_y));
    endpos.z.val = subtile_coord(1, 0);

    if (create_task_dig_to_neutral(comp, startpos, endpos))
    {
        return Lb_SUCCESS;
    }
    return Lb_FAIL;
}

/**
 * Casts spell at a location set by subtiles.
 * @param plyr_idx caster player.
 * @param stl_x subtile's x position.
 * @param stl_y subtile's y position
 * @param fml_bytes encoded bytes: f=cast for free flag,m=power kind,l=spell level.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_power_at_pos(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long fml_bytes)
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
 * Casts spell at a location set by action point/hero gate.
 * @param plyr_idx caster player.
 * @param target action point/hero gate.
 * @param fml_bytes encoded bytes: f=cast for free flag,m=power kind,l=spell level.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_power_at_location(PlayerNumber plyr_idx, TbMapLocation target, long fml_bytes)
{
    SYNCDBG(0, "Using power at location of type %d", target);
    long x = 0;
    long y = 0;
    find_map_location_coords(target, &x, &y, plyr_idx, __func__);
    if ((x == 0) && (y == 0))
    {
        WARNLOG("Can't decode location %d", target);
        return Lb_FAIL;
    }
    return script_use_power_at_pos(plyr_idx, x, y, fml_bytes);
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
 * Increases creatures' levels for player.
 * @param plyr_idx target player
 * @param count how many times should the level be increased
 */
void script_use_special_increase_level(PlayerNumber plyr_idx, int count)
{
    increase_level(get_player(plyr_idx), count);
}

/**
 * Multiplies every creature for player.
 * @param plyr_idx target player
 */
void script_use_special_multiply_creatures(PlayerNumber plyr_idx)
{
    multiply_creatures(get_player(plyr_idx));
}

/**
 * Fortifies player's dungeon.
 * @param plyr_idx target player
 */
void script_use_special_make_safe(PlayerNumber plyr_idx)
{
    make_safe(get_player(plyr_idx));
}

/**
 * Enables bonus level for current player.
 */
TbBool script_use_special_locate_hidden_world()
{
    return activate_bonus_level(get_player(my_player_number));
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



static TbBool is_condition_met(unsigned char cond_idx)
{
    if (cond_idx >= CONDITIONS_COUNT)
    {
      if (cond_idx == CONDITION_ALWAYS)
          return true;
      else
          return false;
    }
    unsigned long i = gameadd.script.conditions[cond_idx].status;
    return ((i & 0x01) != 0);
}

static void process_party(struct PartyTrigger* pr_trig)
{
    struct ScriptContext context = {0};
    long n = pr_trig->creatr_id;

    context.pr_trig = pr_trig;

    switch (pr_trig->flags & TrgF_COMMAND_MASK)
    {
    case TrgF_ADD_TO_PARTY:
        add_to_party_process(&context);
        break;
    case TrgF_DELETE_FROM_PARTY:
        delete_member_from_party(pr_trig->party_id, pr_trig->creatr_id, pr_trig->crtr_level);
        break;
    case TrgF_CREATE_OBJECT:
        n |= ((pr_trig->crtr_level & 7) << 7);
        SYNCDBG(6, "Adding object %d at location %d", (int)n, (int)pr_trig->location);
        script_process_new_object(n, pr_trig->location, pr_trig->carried_gold);
        break;
    case TrgF_CREATE_PARTY:
        SYNCDBG(6, "Adding player %d party %d at location %d", (int)pr_trig->plyr_idx, (int)n, (int)pr_trig->location);
        script_process_new_party(&gameadd.script.creature_partys[n],
            pr_trig->plyr_idx, pr_trig->location, pr_trig->ncopies);
        break;
    case TrgF_CREATE_CREATURE:
        SCRIPTDBG(6, "Adding creature %d", n);
        script_process_new_creatures(pr_trig->plyr_idx, n, pr_trig->location,
            pr_trig->ncopies, pr_trig->carried_gold, pr_trig->crtr_level);
        break;
    }
}

void process_check_new_creature_partys(void)
{
    for (long i = 0; i < gameadd.script.party_triggers_num; i++)
    {
        struct PartyTrigger* pr_trig = &gameadd.script.party_triggers[i];
        if ((pr_trig->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(pr_trig->condit_idx))
            {
                process_party(pr_trig);
                if ((pr_trig->flags & TrgF_REUSABLE) == 0)
                    set_flag_byte(&pr_trig->flags, TrgF_DISABLED, true);
            }
      }
    }
}

void process_check_new_tunneller_partys(void)
{
    for (long i = 0; i < gameadd.script.tunneller_triggers_num; i++)
    {
        struct TunnellerTrigger* tn_trig = &gameadd.script.tunneller_triggers[i];
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
                        struct Thing* grptng = script_process_new_party(&gameadd.script.creature_partys[k - 1], n, tn_trig->location, 1);
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
    for (i=0; i < gameadd.script.win_conditions_num; i++)
    {
      k = gameadd.script.win_conditions[i];
      if (is_condition_met(k)) {
          SYNCDBG(8,"Win condition %d (cond. %d) met for player %d.",(int)i,(int)k,(int)plyr_idx);
          set_player_as_won_level(player);
      }
    }
    for (i=0; i < gameadd.script.lose_conditions_num; i++)
    {
      k = gameadd.script.lose_conditions[i];
      if (is_condition_met(k)) {
          SYNCDBG(8,"Lose condition %d (cond. %d) met for player %d.",(int)i,(int)k,(int)plyr_idx);
          set_player_as_lost_level(player);
      }
    }
}

void process_values(void)
{
    for (long i = 0; i < gameadd.script.values_num; i++)
    {
        struct ScriptValue* value = &gameadd.script.values[i];
        if ((value->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(value->condit_idx))
            {
                script_process_value(value->valtype, value->plyr_range, value->arg0, value->arg1, value->arg2, value);
                if ((value->flags & TrgF_REUSABLE) == 0)
                  set_flag_byte(&value->flags, TrgF_DISABLED, true);
            }
        }
    }

    for (int i = 0; i < gameadd.active_fx_lines; i++)
    {
        if (gameadd.fx_lines[i].used)
        {
            process_fx_line(&gameadd.fx_lines[i]);
        }
    }
    for (int i = gameadd.active_fx_lines; i > 0; i--)
    {
        if (gameadd.fx_lines[i-1].used)
        {
            break;
        }
        gameadd.active_fx_lines--;
    }
}

static void set_variable(int player_idx, long var_type, long var_idx, long new_val)
{
    struct Dungeon *dungeon = get_dungeon(player_idx);
    struct DungeonAdd *dungeonadd = get_dungeonadd(player_idx);
    struct Coord3d pos = {0};

    switch (var_type)
    {
    case SVar_FLAG:
        set_script_flag(player_idx, var_idx, saturate_set_unsigned(new_val, 8));
        break;
    case SVar_CAMPAIGN_FLAG:
        intralvl.campaign_flags[player_idx][var_idx] = new_val;
        break;
    case SVar_BOX_ACTIVATED:
        dungeonadd->box_info.activated[var_idx] = new_val;
        break;
    case SVar_SACRIFICED:
        dungeon->creature_sacrifice[var_idx] = new_val;
        if (find_temple_pool(player_idx, &pos))
        {
            process_sacrifice_creature(&pos, var_idx, player_idx, false);
        }
        break;
    case SVar_REWARDED:
        dungeonadd->creature_awarded[var_idx] = new_val;
        break;
    default:
        WARNLOG("Unexpected type:%d",(int)var_type);
    }
}
/**
 * Processes given VALUE immediately.
 * This processes given script command. It is used to process VALUEs at start when they have
 * no conditions, or during the gameplay when conditions are met.
 */
void script_process_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4, struct ScriptValue *value)
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
  //TODO: split and make indexed by var_index
  const struct CommandDesc *desc;
  for (desc = command_desc; desc->textptr != NULL; desc++)
      if (desc-> index == var_index)
          break;
  if (desc == NULL)
  {
      WARNLOG("Unexpected index:%d", var_index);
      return;
  }
  if (desc->process_fn)
  {
      // TODO: move two functions up
      struct ScriptContext context;
      context.plr_start = plr_start;
      context.plr_end = plr_end;
      // TODO: this should be checked for sanity
      //for (i=plr_start; i < plr_end; i++)
      {
          context.player_idx = plr_start;
          context.value = value;
          desc->process_fn(&context);
      }
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
          set_variable(i, val4, val2, val3);
      }
      break;
  case Cmd_ADD_TO_FLAG:
      for (i=plr_start; i < plr_end; i++)
      {
          set_variable(i, val4, val2, get_condition_value(i, val4, val2) + val3);
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
      crconf = &gameadd.crtr_conf.model[val2];
      crstat = creature_stats_get(val2);
      switch (val3)
      {
      case 1: // BLEEDS
          crstat->bleeds = val4;
          break;
      case 2: // UNAFFECTED_BY_WIND
          if (val4)
          {
              crstat->affected_by_wind = 0;
          }
          else
          {
              crstat->affected_by_wind = 1;
          }
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
      case 24: // NO_IMPRISONMENT
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_NoImprisonment;
          }
          else
          {
              crconf->model_flags ^= CMF_NoImprisonment;
          }
          break;
      case 25: // NEVER_SICK
          if (val4 >= 1)
          {
              crconf->model_flags |= CMF_NeverSick;
          }
          else
          {
              crconf->model_flags ^= CMF_NeverSick;
          }
          break;
      case 26: // ILLUMINATED
          crstat->illuminated = val4;
          break;
      case 27: // ALLURING_SCVNGR
          crstat->entrance_force = val4;
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
      if (level_file_version > 0)
      {
          gameadd.timer_real = (TbBool)val3;
      }
      else
      {
          gameadd.timer_real = false;
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
          if (val2 > SENSIBLE_GOLD)
          {
              val2 = SENSIBLE_GOLD;
              SCRPTWRNLOG("Gold added to player %d reduced to %d", (int)plr_range_id, SENSIBLE_GOLD);
          }
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
      if (val4 < 0 || val4 > 54)
      {
          SCRPTERRLOG("Unsupported slab '%d'. Slabs range 0-54 allowed.", val4);
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
    case Cmd_USE_SPELL_ON_CREATURE:
      script_use_spell_on_creature(plr_range_id, val2, val3, val4);
      break;
    case Cmd_COMPUTER_DIG_TO_LOCATION:
        for (i = plr_start; i < plr_end; i++)
        {
            script_computer_dig_to_location(i, val2, val3);
        }
        break;
    case Cmd_USE_POWER_AT_POS:
      for (i=plr_start; i < plr_end; i++)
      {
          script_use_power_at_pos(i, val2, val3, val4);
      }
      break;
    case Cmd_USE_POWER_AT_LOCATION:
      for (i=plr_start; i < plr_end; i++)
      {
          script_use_power_at_location(i, val2, val3);
      }
      break;
    case Cmd_USE_POWER:
      for (i=plr_start; i < plr_end; i++)
      {
          script_use_power(i, val2, val3);
      }
      break;
    case Cmd_USE_SPECIAL_INCREASE_LEVEL:
      for (i=plr_start; i < plr_end; i++)
      {
          script_use_special_increase_level(i, val2);
      }
      break;
    case Cmd_USE_SPECIAL_MULTIPLY_CREATURES:
      for (i=plr_start; i < plr_end; i++)
      {
          for (int count = 0; count < val2; count++)
          {
            script_use_special_multiply_creatures(i);
          }
      }
      break;
    case Cmd_USE_SPECIAL_MAKE_SAFE:
      for (i=plr_start; i < plr_end; i++)
      {
          script_use_special_make_safe(i);
      }
      break;
    case Cmd_USE_SPECIAL_LOCATE_HIDDEN_WORLD:
      script_use_special_locate_hidden_world();
      break;
    case Cmd_CHANGE_CREATURE_OWNER:
      for (i=plr_start; i < plr_end; i++)
      {
          script_change_creature_owner_with_criteria(i, val2, val3, val4);
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
  case Cmd_QUICK_MESSAGE:
  {
      message_add_fmt(val2, "%s", gameadd.quick_messages[val3]);
      break;
  }
  case Cmd_DISPLAY_MESSAGE:
  {
        message_add_fmt(val2, "%s", get_string(val3));
        break;
  }
  case Cmd_CREATURE_ENTRANCE_LEVEL:
  {
    if (val2 > 0)
    {
        struct DungeonAdd* dungeonadd;
        if (plr_range_id == ALL_PLAYERS)
        {
            for (i = PLAYER3; i >= PLAYER0; i--)
            {
                dungeonadd = get_dungeonadd(i);
                if (!dungeonadd_invalid(dungeonadd))
                {
                    dungeonadd->creature_entrance_level = (val2 - 1);
                }
            }
        }
        else
        {
            dungeonadd = get_dungeonadd(plr_range_id);
            if (!dungeonadd_invalid(dungeonadd))
            {
                dungeonadd->creature_entrance_level = (val2 - 1);
            }
        }
    }
    break;
  }
  case Cmd_RANDOMISE_FLAG:
      for (i=plr_start; i < plr_end; i++)
      {
          set_variable(i, val4, val2, (rand() % val3) + 1);
      }
      break;
  case Cmd_COMPUTE_FLAG:
      {
        long src_plr_range = (val2 >> 24) & 255;
        long operation = (val2 >> 16) & 255;
        unsigned char flag_type = (val2 >> 8) & 255;
        unsigned char src_flag_type = val2 & 255;
        int src_plr_start, src_plr_end;
        if (get_players_range(src_plr_range, &src_plr_start, &src_plr_end) < 0)
        {
            WARNLOG("Invalid player range %d in VALUE command %d.",(int)src_plr_range,(int)var_index);
            return;
        }
        long sum = 0;
        for (i=src_plr_start; i < src_plr_end; i++)
        {
            sum += get_condition_value(i, src_flag_type, val4);
        }
        for (i=plr_start; i < plr_end; i++)
        {
            long current_flag_val = get_condition_value(i, flag_type, val3);
            long computed = sum;
            if (operation == SOpr_INCREASE) computed = current_flag_val + sum;
            if (operation == SOpr_DECREASE) computed = current_flag_val - sum;
            if (operation == SOpr_MULTIPLY) computed = current_flag_val * sum;
            computed = min(255, max(0, computed));
            SCRIPTDBG(7,"Changing player%d's %d flag from %d to %d based on flag of type %d.", i, val3, current_flag_val, computed, src_flag_type);
            set_variable(i, flag_type, val3, computed);
        }
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
          if (val3 >= 0 && val3 <= 4096)
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
      case 16: //DoorSellGoldBackPercent
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.door_sale_percent, val3);
          gameadd.door_sale_percent = val3;
          break;
      case 17: //TrapSellGoldBackPercent
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.trap_sale_percent, val3);
          gameadd.trap_sale_percent = val3;
          break;
      case 18: //PayDayGap
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.pay_day_gap, val3);
          game.pay_day_gap = val3;
          break;
      case 19: //PayDaySpeed
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
      case 20: //PayDayProgress
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
      case 21: //PlaceTrapsOnSubtiles
          SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.place_traps_on_subtiles, val3);
          gameadd.place_traps_on_subtiles = (TbBool)val3;
          break;
      case 22: //DiseaseHPTemplePercentage
          if (val3 >= 0 && val3 <= 100)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, gameadd.disease_to_temple_pct, val3);
              gameadd.disease_to_temple_pct = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range", val2, val3);
          }
          break;
      case 23:  //DungeonHeartHealth
          if (val3 <= SHRT_MAX)
          {
              SCRIPTDBG(7, "Changing rule %d from %d to %d", val2, game.dungeon_heart_health, val3);
              game.dungeon_heart_health = val3;
              game.objects_config[5].health = val3;
              gameadd.object_conf.base_config[5].health = val3;
          }
          else
          {
              SCRPTERRLOG("Rule '%d' value %d out of range. Max %d.", val2, val3, SHRT_MAX);
          }
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

// TODO: z location
void find_location_pos(long location, PlayerNumber plyr_idx, struct Coord3d *pos, const char *func_name)
{
  struct ActionPoint *apt;
  struct Thing *thing;
  unsigned long i = get_map_location_longval(location);
  memset(pos, 0, sizeof(*pos));

  switch (get_map_location_type(location))
  {
    case MLoc_ACTIONPOINT:
      // Location stores action point index
      apt = action_point_get(i);
      if (!action_point_is_invalid(apt))
      {
        pos->x.val = apt->mappos.x.val;
        pos->y.val = apt->mappos.y.val;
      } else
        WARNMSG("%s: Action Point %d location not found",func_name,i);
      break;
    case MLoc_HEROGATE:
      thing = find_hero_gate_of_number(i);
      if (!thing_is_invalid(thing))
      {
        *pos = thing->mappos;
      } else
        WARNMSG("%s: Hero Gate %d location not found",func_name,i);
      break;
    case MLoc_PLAYERSHEART:
      if (i < PLAYERS_COUNT)
      {
        thing = get_player_soul_container(i);
      } else
        thing = INVALID_THING;
      if (!thing_is_invalid(thing))
      {
        *pos = thing->mappos;
      } else
        WARNMSG("%s: Dungeon Heart location for player %d not found",func_name,i);
      break;
    case MLoc_NONE:
      pos->x.val = 0;
      pos->y.val = 0;
      pos->z.val = 0;
      break;
    case MLoc_THING:
      thing = thing_get(i);
      if (!thing_is_invalid(thing))
      {
        *pos = thing->mappos;
      } else
        WARNMSG("%s: Thing %d location not found",func_name,i);
      break;
    case MLoc_METALOCATION:
      if (!get_coords_at_meta_action(pos, plyr_idx, i))
        WARNMSG("%s: Metalocation not found %d",func_name,i);
      break;
    case MLoc_CREATUREKIND:
    case MLoc_OBJECTKIND:
    case MLoc_ROOMKIND:
    case MLoc_PLAYERSDUNGEON:
    case MLoc_APPROPRTDUNGEON:
    case MLoc_DOORKIND:
    case MLoc_TRAPKIND:
    default:
      WARNMSG("%s: Unsupported location, %lu.",func_name,location);
      break;
  }
  SYNCDBG(15,"From %s; Location %ld, pos(%ld,%ld)",func_name, location, pos->x.stl.num, pos->y.stl.num);
}

void process_level_script(void)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo *player;
  player = get_my_player();
  // Do NOT stop executing scripts after winning if the RUN_AFTER_VICTORY(1) script command has been issued
  if ((player->victory_state == VicS_Undecided) || (game.system_flags & GSF_RunAfterVictory))
  {
      process_conditions();
      process_check_new_creature_partys();
    //script_process_messages(); is not here, but it is in beta - check why
      process_check_new_tunneller_partys();
      process_values();
      process_win_and_lose_conditions(my_player_number); //player->id_number may be uninitialized yet
    //  show_onscreen_msg(8, "Flags %d %d %d %d %d %d", game.dungeon[0].script_flags[0],game.dungeon[0].script_flags[1],
    //    game.dungeon[0].script_flags[2],game.dungeon[0].script_flags[3],game.dungeon[0].script_flags[4],game.dungeon[0].script_flags[5]);
  }
  SYNCDBG(19,"Finished");
}


/******************************************************************************/
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
  {"REVEAL_MAP_LOCATION",               "PNN     ", Cmd_REVEAL_MAP_LOCATION, NULL, NULL},
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
  {"CHANGE_SLAB_OWNER",                 "NNP     ", Cmd_CHANGE_SLAB_OWNER, NULL, NULL},
  {"CHANGE_SLAB_TYPE",                  "NNS     ", Cmd_CHANGE_SLAB_TYPE, NULL, NULL},
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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
