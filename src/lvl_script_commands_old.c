/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_commands_old.c
 * @par  This file is the old way of working 
 * DON'T ADD NEW LOGIC HERE
 * see lvl_script_commands.c on how new commands should be added
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * @author   KeeperFX Team
 */
/******************************************************************************/
#include "lvl_script_commands_old.h"

#include <strings.h>

#include "bflib_math.h"
#include "config_strings.h"
#include "config_magic.h"
#include "config_terrain.h"
#include "player_instances.h"
#include "player_data.h"
#include "lvl_filesdk1.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "music_player.h"
#include "keeperfx.hpp"

#include "lvl_script_value.h"
#include "lvl_script_lib.h"
#include "lvl_script_conditions.h"
#include "lvl_script_commands.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/


const struct NamedCommand newcrtr_desc[] = {
  {"NEW_CREATURE_A",   1},
  {"NEW_CREATURE_B",   2},
  {NULL,               0},
};
const struct NamedCommand game_rule_desc[] = {
  {"BodiesForVampire",           1},
  {"PrisonSkeletonChance",       2},
  {"GhostConvertChance",         3},
  {"TortureConvertChance",       4},
  {"TortureDeathChance",         5},
  {"FoodGenerationSpeed",        6},
  {"StunEvilEnemyChance",        7},
  {"StunGoodEnemyChance",        8},
  {"BodyRemainsFor",             9},
  {"FightHateKillValue",        10},
  {"PreserveClassicBugs",       11},
  {"DungeonHeartHealHealth",    12},
  {"ImpWorkExperience",         13},
  {"GemEffectiveness",          14},
  {"RoomSellGoldBackPercent",   15},
  {"DoorSellValuePercent",      16},
  {"TrapSellValuePercent",      17},
  {"PayDayGap",                 18},
  {"PayDaySpeed",               19},
  {"PayDayProgress",            20},
  {"PlaceTrapsOnSubtiles",      21},
  {"DiseaseHPTemplePercentage", 22},
  {"DungeonHeartHealth",        23},
  {NULL,                         0},
};


#define CONDITION_ALWAYS (CONDITIONS_COUNT)


void command_add_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4)
{
    ALLOCATE_SCRIPT_VALUE(var_index, plr_range_id);

    value->arg0 = val2;
    value->arg1 = val3;
    value->arg2 = val4;

    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0))
    {
        script_process_value(var_index, plr_range_id, val2, val3, val4, value);
        return;
    }
}

static void command_create_party(const char *prtname)
{
    if (get_script_current_condition() != CONDITION_ALWAYS)
    {
        SCRPTWRNLOG("Party '%s' defined inside conditional statement",prtname);
    }
    create_party(prtname);
}

static void command_tutorial_flash_button(long btn_id, long duration)
{
    command_add_value(Cmd_TUTORIAL_FLASH_BUTTON, ALL_PLAYERS, btn_id, duration, 0);
}

static void command_add_party_to_level(long plr_range_id, const char *prtname, const char *locname, long ncopies)
{
    TbMapLocation location;
    if (ncopies < 1)
    {
        SCRPTERRLOG("Invalid NUMBER parameter");
        return;
    }
    if (gameadd.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
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
    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0))
    {
        struct Party* party = &gameadd.script.creature_partys[prty_id];
        script_process_new_party(party, plr_id, location, ncopies);
    } else
    {
        struct PartyTrigger* pr_trig = &gameadd.script.party_triggers[gameadd.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        pr_trig->flags = TrgF_CREATE_PARTY;
        pr_trig->flags |= next_command_reusable?TrgF_REUSABLE:0;
        pr_trig->plyr_idx = plr_id;
        pr_trig->creatr_id = prty_id;
        pr_trig->location = location;
        pr_trig->ncopies = ncopies;
        pr_trig->condit_idx = get_script_current_condition();
        gameadd.script.party_triggers_num++;
    }
}

static void command_add_object_to_level(const char *obj_name, const char *locname, long arg)
{
    TbMapLocation location;
    long obj_id = get_rid(object_desc, obj_name);
    if (obj_id == -1)
    {
        SCRPTERRLOG("Unknown object, '%s'", obj_name);
        return;
    }
    if (gameadd.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_CREATURE commands in script");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        script_process_new_object(obj_id, location, arg);
    } else
    {
        struct PartyTrigger* pr_trig = &gameadd.script.party_triggers[gameadd.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        pr_trig->flags = TrgF_CREATE_OBJECT;
        pr_trig->flags |= next_command_reusable?TrgF_REUSABLE:0;
        pr_trig->plyr_idx = 0; //That is because script is inside `struct Game` and it is not possible to enlarge it
        pr_trig->creatr_id = obj_id & 0x7F;
        pr_trig->crtr_level = ((obj_id >> 7) & 7); // No more than 1023 different classes of objects :)
        pr_trig->carried_gold = arg;
        pr_trig->location = location;
        pr_trig->ncopies = 1;
        pr_trig->condit_idx = get_script_current_condition();
        gameadd.script.party_triggers_num++;
    }
}

static void command_add_creature_to_level(long plr_range_id, const char *crtr_name, const char *locname, long ncopies, long crtr_level, long carried_gold)
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
    if (gameadd.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
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
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        script_process_new_creatures(plr_id, crtr_id, location, ncopies, carried_gold, crtr_level-1);
    } else
    {
        struct PartyTrigger* pr_trig = &gameadd.script.party_triggers[gameadd.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        pr_trig->flags = TrgF_CREATE_CREATURE;
        pr_trig->flags |= next_command_reusable?TrgF_REUSABLE:0;

        pr_trig->plyr_idx = plr_id;
        pr_trig->creatr_id = crtr_id;
        pr_trig->crtr_level = crtr_level-1;
        pr_trig->carried_gold = carried_gold;
        pr_trig->location = location;
        pr_trig->ncopies = ncopies;
        pr_trig->condit_idx = get_script_current_condition();
        gameadd.script.party_triggers_num++;
    }
}

static void command_if(long plr_range_id, const char *varib_name, const char *operatr, long value)
{
    long varib_type;
    long varib_id;
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

static void command_display_information(long msg_num, const char *where, long x, long y)
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

static void command_set_generate_speed(long game_turns)
{
    if (game_turns <= 0)
    {
      SCRPTERRLOG("Generation speed must be positive number");
      return;
    }
    command_add_value(Cmd_SET_GENERATE_SPEED, ALL_PLAYERS, game_turns, 0, 0);
}

static void command_dead_creatures_return_to_pool(long val)
{
    command_add_value(Cmd_DEAD_CREATURES_RETURN_TO_POOL, ALL_PLAYERS, val, 0, 0);
}

static void command_bonus_level_time(long game_turns, long real)
{
    if (game_turns < 0)
    {
        SCRPTERRLOG("Bonus time must be nonnegative");
        return;
    }
    command_add_value(Cmd_BONUS_LEVEL_TIME, ALL_PLAYERS, game_turns, real, 0);
}

static void command_set_start_money(long plr_range_id, long gold_val)
{
    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %d is not supported in this command", (int)plr_range_id);
        return;
  }
  if (get_script_current_condition() != CONDITION_ALWAYS)
  {
    SCRPTWRNLOG("Start money set inside conditional block; condition ignored");
  }
  for (int i = plr_start; i < plr_end; i++)
  {
      if (gold_val > SENSIBLE_GOLD)
      {
          gold_val = SENSIBLE_GOLD;
          SCRPTWRNLOG("Gold added to player %d reduced to %d", (int)plr_range_id, SENSIBLE_GOLD);
      }
      player_add_offmap_gold(i, gold_val);
  }
}

static void command_room_available(long plr_range_id, const char *roomname, unsigned long can_resrch, unsigned long can_build)
{
    long room_id = get_rid(room_desc, roomname);
    if (room_id == -1)
    {
      SCRPTERRLOG("Unknown room name, '%s'", roomname);
      return;
    }
    command_add_value(Cmd_ROOM_AVAILABLE, plr_range_id, room_id, can_resrch, can_build);
}

static void command_creature_available(long plr_range_id, const char *crtr_name, unsigned long can_be_avail, unsigned long force_avail)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
      return;
    }
    command_add_value(Cmd_CREATURE_AVAILABLE, plr_range_id, crtr_id, can_be_avail, force_avail);
}

static void command_magic_available(long plr_range_id, const char *magname, unsigned long can_resrch, unsigned long can_use)
{
    long mag_id = get_rid(power_desc, magname);
    if (mag_id == -1)
    {
      SCRPTERRLOG("Unknown magic, '%s'", magname);
      return;
    }
    command_add_value(Cmd_MAGIC_AVAILABLE, plr_range_id, mag_id, can_resrch, can_use);
}

static void command_trap_available(long plr_range_id, const char *trapname, unsigned long can_build, unsigned long amount)
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
static void command_research(long plr_range_id, const char *trg_type, const char *trg_name, unsigned long val)
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
static void command_research_order(long plr_range_id, const char *trg_type, const char *trg_name, unsigned long val)
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

static void command_if_action_point(long apt_num, long plr_range_id)
{
    if (gameadd.script.conditions_num >= CONDITIONS_COUNT)
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

static void command_if_slab_owner(MapSlabCoord slb_x, MapSlabCoord slb_y, long plr_range_id)
{
    if (gameadd.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    command_add_condition(slb_x, 1, SVar_SLAB_OWNER, slb_y, plr_range_id);
}

static void command_if_slab_type(MapSlabCoord slb_x, MapSlabCoord slb_y, long slab_type)
{
    if (gameadd.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    command_add_condition(slb_x, 1, SVar_SLAB_TYPE, slb_y, slab_type);
}

static void command_computer_player(long plr_range_id, long comp_model)
{
    if (get_script_current_condition() != CONDITION_ALWAYS)
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

static void command_set_timer(long plr_range_id, const char *timrname)
{
    long timr_id = get_rid(timer_desc, timrname);
    if (timr_id == -1)
    {
        SCRPTERRLOG("Unknown timer, '%s'", timrname);
        return;
    }
    command_add_value(Cmd_SET_TIMER, plr_range_id, timr_id, 0, 0);
}

static void command_win_game(void)
{
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        SCRPTERRLOG("Command WIN GAME found with no condition");
        return;
    }
    if (gameadd.script.win_conditions_num >= WIN_CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many WIN GAME conditions in script");
        return;
    }
    gameadd.script.win_conditions[gameadd.script.win_conditions_num] = get_script_current_condition();
    gameadd.script.win_conditions_num++;
}

static void command_lose_game(void)
{
  if (get_script_current_condition() == CONDITION_ALWAYS)
  {
    SCRPTERRLOG("Command LOSE GAME found with no condition");
    return;
  }
  if (gameadd.script.lose_conditions_num >= WIN_CONDITIONS_COUNT)
  {
    SCRPTERRLOG("Too many LOSE GAME conditions in script");
    return;
  }
  gameadd.script.lose_conditions[gameadd.script.lose_conditions_num] = get_script_current_condition();
  gameadd.script.lose_conditions_num++;
}

static void command_set_flag(long plr_range_id, const char *flgname, long val)
{
    long flg_id;
    long flag_type;
    if (!parse_set_varib(flgname, &flg_id, &flag_type))
    {
        SCRPTERRLOG("Unknown flag, '%s'", flgname);
        return;
    }
    command_add_value(Cmd_SET_FLAG, plr_range_id, flg_id, val, flag_type);
}

static void command_add_to_flag(long plr_range_id, const char *flgname, long val)
{
    long flg_id;
    long flag_type;

    if (!parse_set_varib(flgname, &flg_id, &flag_type))
    {
        SCRPTERRLOG("Unknown flag, '%s'", flgname);
        return;
    }
    command_add_value(Cmd_ADD_TO_FLAG, plr_range_id, flg_id, val, flag_type);
}

static void command_max_creatures(long plr_range_id, long val)
{
    command_add_value(Cmd_MAX_CREATURES, plr_range_id, val, 0, 0);
}

static void command_door_available(long plr_range_id, const char *doorname, unsigned long a3, unsigned long a4)
{
    long door_id = get_rid(door_desc, doorname);
    if (door_id == -1)
    {
        SCRPTERRLOG("Unknown door, '%s'", doorname);
        return;
  }
  command_add_value(Cmd_DOOR_AVAILABLE, plr_range_id, door_id, a3, a4);
}

static void command_add_tunneller_to_level(long plr_range_id, const char *locname, const char *objectv, long target, unsigned char crtr_level, unsigned long carried_gold)
{
    TbMapLocation location;
    TbMapLocation heading;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if (gameadd.script.tunneller_triggers_num >= TUNNELLER_TRIGGERS_COUNT)
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
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        script_process_new_tunneler(plr_id, location, heading, crtr_level-1, carried_gold);
    } else
    {
        struct TunnellerTrigger* tn_trig = &gameadd.script.tunneller_triggers[gameadd.script.tunneller_triggers_num % TUNNELLER_TRIGGERS_COUNT];
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
        tn_trig->condit_idx = get_script_current_condition();
        gameadd.script.tunneller_triggers_num++;
    }
}

static void command_add_tunneller_party_to_level(long plr_range_id, const char *prtname, const char *locname, const char *objectv, long target, char crtr_level, unsigned long carried_gold)
{
    TbMapLocation location;
    TbMapLocation heading;
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if (gameadd.script.tunneller_triggers_num >= TUNNELLER_TRIGGERS_COUNT)
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
    struct Party* party = &gameadd.script.creature_partys[prty_id];
    if (party->members_num >= GROUP_MEMBERS_COUNT-1)
    {
        SCRPTERRLOG("Party too big for ADD_TUNNELLER (Max %d members)", GROUP_MEMBERS_COUNT-1);
        return;
    }
    // Either add the party or add item to conditional triggers list
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        script_process_new_tunneller_party(plr_id, prty_id, location, heading, crtr_level-1, carried_gold);
    } else
    {
        struct TunnellerTrigger* tn_trig = &gameadd.script.tunneller_triggers[gameadd.script.tunneller_triggers_num % TUNNELLER_TRIGGERS_COUNT];
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
        tn_trig->condit_idx = get_script_current_condition();
        gameadd.script.tunneller_triggers_num++;
    }
}

static void command_add_creature_to_pool(const char *crtr_name, long amount)
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

static void command_reset_action_point(long apt_num)
{
    long apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        SCRPTERRLOG("Non-existing Action Point, no %d", apt_num);
        return;
  }
  command_add_value(Cmd_RESET_ACTION_POINT, ALL_PLAYERS, apt_idx, 0, 0);
}

static void command_set_creature_max_level(long plr_range_id, const char *crtr_name, long crtr_level)
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

static void command_set_music(long val)
{
  if (get_script_current_condition() != CONDITION_ALWAYS)
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

static void command_set_hate(long trgt_plr_range_id, long enmy_plr_range_id, long hate_val)
{
    // Verify enemy player
    long enmy_plr_id = get_players_range_single(enmy_plr_range_id);
    if (enmy_plr_id < 0) {
        SCRPTERRLOG("Given enemy player is not supported in this command");
        return;
    }
    command_add_value(Cmd_SET_HATE, trgt_plr_range_id, enmy_plr_id, hate_val, 0);
}

static void command_if_available(long plr_range_id, const char *varib_name, const char *operatr, long value)
{
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
    // Add the condition to script structure
    command_add_condition(plr_range_id, opertr_id, varib_type, varib_id, value);
}

static void command_if_controls(long plr_range_id, const char *varib_name, const char *operatr, long value)
{
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
    }
    // Add the condition to script structure
    command_add_condition(plr_range_id, opertr_id, varib_type, varib_id, value);
}

static void command_set_computer_globals(long plr_range_id, long val1, long val2, long val3, long val4, long val5, long val6)
{
  int plr_start;
  int plr_end;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
      SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
      return;
  }
  if (get_script_current_condition() != CONDITION_ALWAYS)
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

static void command_set_computer_checks(long plr_range_id, const char *chkname, long val1, long val2, long val3, long val4, long val5)
{
  int plr_start;
  int plr_end;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
      SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
      return;
  }
  if (!player_exists(get_player(plr_range_id)))
  {
      SCRPTERRLOG("Player %d does not exist; cannot modify check", (int)plr_range_id);
      return;
  }
  if (get_script_current_condition() != CONDITION_ALWAYS)
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

static void command_set_computer_events(long plr_range_id, const char *evntname, long val1, long val2, long val3, long val4, long val5)
{
  int plr_start;
  int plr_end;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
      SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
      return;
  }
  if (!player_exists(get_player(plr_range_id)))
  {
      SCRPTERRLOG("Player %d does not exist; cannot modify events", (int)plr_range_id);
      return;
  }
  if (get_script_current_condition() != CONDITION_ALWAYS)
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

static void command_set_computer_process(long plr_range_id, const char *procname, long val1, long val2, long val3, long val4, long val5)
{
  int plr_start;
  int plr_end;
  if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
      SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
      return;
  }
  if (get_script_current_condition() != CONDITION_ALWAYS)
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

static void command_set_creature_health(const char *crtr_name, long val)
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

static void command_set_creature_strength(const char *crtr_name, long val)
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

static void command_set_creature_armour(const char *crtr_name, long val)
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

static void command_set_creature_fear_wounded(const char *crtr_name, long val)
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

static void command_set_creature_fear_stronger(const char *crtr_name, long val)
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

static void command_set_creature_fearsome_factor(const char* crtr_name, long val)
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

static void command_set_creature_property(const char* crtr_name, long property, short val)
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
static void command_ally_players(long plr1_range_id, long plr2_range_id, TbBool ally)
{
    // Verify enemy player
    long plr2_id = get_players_range_single(plr2_range_id);
    if (plr2_id < 0) {
        SCRPTERRLOG("Given second player is not supported in this command");
        return;
    }
    command_add_value(Cmd_ALLY_PLAYERS, plr1_range_id, plr2_id, ally, 0);
}

static void command_quick_objective(int idx, const char *msgtext, const char *where, long x, long y)
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

static void command_quick_information(int idx, const char *msgtext, const char *where, long x, long y)
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

static void command_play_message(long plr_range_id, const char *msgtype, int msg_num)
{
    long msgtype_id = get_id(msgtype_desc, msgtype);
    if (msgtype_id == -1)
    {
        SCRPTERRLOG("Unrecognized message type, '%s'", msgtype);
        return;
  }
  command_add_value(Cmd_PLAY_MESSAGE, plr_range_id, msgtype_id, msg_num, 0);
}

static void command_add_gold_to_player(long plr_range_id, long amount)
{
    command_add_value(Cmd_ADD_GOLD_TO_PLAYER, plr_range_id, amount, 0, 0);
}

static void command_set_creature_tendencies(long plr_range_id, const char *tendency, long value)
{
    long tend_id = get_rid(tendency_desc, tendency);
    if (tend_id == -1)
    {
      SCRPTERRLOG("Unrecognized tendency type, '%s'", tendency);
      return;
    }
    command_add_value(Cmd_SET_CREATURE_TENDENCIES, plr_range_id, tend_id, value, 0);
}

static void command_reveal_map_rect(long plr_range_id, long x, long y, long w, long h)
{
    command_add_value(Cmd_REVEAL_MAP_RECT, plr_range_id, x, y, (h<<16)+w);
}

static const char *script_get_command_name(long cmnd_index)
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

static void command_message(const char *msgtext, unsigned char kind)
{
  const char *cmd;
  if (kind == 80)
    cmd = script_get_command_name(Cmd_PRINT);
  else
    cmd = script_get_command_name(Cmd_MESSAGE);
  SCRPTWRNLOG("Command '%s' is only supported in Dungeon Keeper Beta", cmd);
}

static void command_quick_message(int idx, const char *msgtext, const char *range_id)
{
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
  char id = get_player_number_from_value(range_id);
  command_add_value(Cmd_QUICK_MESSAGE, 0, id, idx, 0);
}

static void command_display_message(int msg_num, const char *range_id)
{
    char id = get_player_number_from_value(range_id);
    command_add_value(Cmd_DISPLAY_MESSAGE, 0, id, msg_num, 0);
}

static void command_swap_creature(const char *ncrt_name, const char *crtr_name)
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
  struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[crtr_id];
  if ((crconf->model_flags & CMF_IsSpecDigger) != 0)
  {
      SCRPTERRLOG("Unable to swap special diggers");
  }
  if (get_script_current_condition() != CONDITION_ALWAYS)
  {
      SCRPTWRNLOG("Creature swapping placed inside conditional statement");
  }
  if (!swap_creature(ncrt_id, crtr_id))
  {
      SCRPTERRLOG("Error swapping creatures '%s'<->'%s'", ncrt_name, crtr_name);
  }
}

static void command_kill_creature(long plr_range_id, const char *crtr_name, const char *criteria, int count)
{
    SCRIPTDBG(11, "Starting");
    if (count <= 0)
    {
        SCRPTERRLOG("Bad creatures count, %d", count);
        return;
  }
  long crtr_id = parse_creature_name(crtr_name);
  if (crtr_id == CREATURE_NONE) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = parse_criteria(criteria);
  if (select_id == -1)
  {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  command_add_value(Cmd_KILL_CREATURE, plr_range_id, crtr_id, select_id, count);
}

static void command_level_up_creature(long plr_range_id, const char *crtr_name, const char *criteria, int count)
{
    SCRIPTDBG(11, "Starting");
    if (count <= 0)
    {
        SCRPTERRLOG("Bad count, %d", count);
        return;
  }
  long crtr_id = parse_creature_name(crtr_name);
  if (crtr_id == CREATURE_NONE)
  {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = parse_criteria(criteria);
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

static void command_use_power_on_creature(long plr_range_id, const char *crtr_name, const char *criteria, long caster_plyr_idx, const char *magname, int splevel, char free)
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
  long crtr_id = parse_creature_name(crtr_name);
  if (crtr_id == CREATURE_NONE) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = parse_criteria(criteria);
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

static void command_use_power_at_pos(long plr_range_id, int stl_x, int stl_y, const char *magname, int splevel, char free)
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
  command_add_value(Cmd_USE_POWER_AT_POS, plr_range_id, stl_x, stl_y, fml_bytes);
}

static void command_use_power_at_location(long plr_range_id, const char *locname, const char *magname, int splevel, char free)
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

  TbMapLocation location;
  if (!get_map_location_id(locname, &location))
  {
    SCRPTWRNLOG("Use power script command at invalid location: %s", locname);
    return;
  }

  // encode params: free, magic, level -> into 3xbyte: FML
  long fml_bytes;
  {
      signed char f = free, m = mag_id, lvl = splevel;
      fml_bytes = (f << 16) | (m << 8) | lvl;
  }
  command_add_value(Cmd_USE_POWER_AT_LOCATION, plr_range_id, location, fml_bytes, 0);
}

static void command_use_power(long plr_range_id, const char *magname, char free)
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

static void command_use_special_increase_level(long plr_range_id, long count)
{
    if (count < 1)
    {
        SCRPTWRNLOG("Invalid count: %d, setting to 1.", count);
        count = 1;
    }

    if (count > 9)
    {
        SCRPTWRNLOG("Count too high: %d, setting to 9.", count);
        count = 9;
    }
    command_add_value(Cmd_USE_SPECIAL_INCREASE_LEVEL, plr_range_id, count, 0, 0);
}

static void command_use_special_multiply_creatures(long plr_range_id, long count)
{
    if (count < 1)
    {
        SCRPTWRNLOG("Invalid count: %d, setting to 1.", count);
        count = 1;
    }

    if (count > 9)
    {
        SCRPTWRNLOG("Count too high: %d, setting to 9.", count);
        count = 9;
    }
    command_add_value(Cmd_USE_SPECIAL_MULTIPLY_CREATURES, plr_range_id, count, 0, 0);
}

static void command_use_special_make_safe(long plr_range_id)
{
    command_add_value(Cmd_USE_SPECIAL_MAKE_SAFE, plr_range_id, 0, 0, 0);
}

static void command_use_special_locate_hidden_world()
{
    command_add_value(Cmd_USE_SPECIAL_LOCATE_HIDDEN_WORLD, 0, 0, 0, 0);
}

static void command_change_creature_owner(long origin_plyr_idx, const char *crtr_name, const char *criteria, long dest_plyr_idx)
{
    SCRIPTDBG(11, "Starting");
    long crtr_id = parse_creature_name(crtr_name);
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  long select_id = parse_criteria(criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  command_add_value(Cmd_CHANGE_CREATURE_OWNER, origin_plyr_idx, crtr_id, select_id, dest_plyr_idx);
}


static void command_computer_dig_to_location(long plr_range_id, const char* origin, const char* destination)
{
    TbMapLocation orig_loc;
    if (!get_map_location_id(origin, &orig_loc))
    {
        SCRPTWRNLOG("Dig to location script command has invalid source location: %s", origin);
        return;
    }
    TbMapLocation dest_loc;
    if (!get_map_location_id(destination, &dest_loc))
    {
        SCRPTWRNLOG("Dig to location script command has invalid destination location: %s", destination);
        return;
    }

    command_add_value(Cmd_COMPUTER_DIG_TO_LOCATION, plr_range_id, orig_loc, dest_loc, 0);
}

static void command_set_campaign_flag(long plr_range_id, const char *cmpflgname, long val)
{
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown campaign flag, '%s'", cmpflgname);
        return;
  }
  command_add_value(Cmd_SET_CAMPAIGN_FLAG, plr_range_id, flg_id, val, 0);
}

static void command_add_to_campaign_flag(long plr_range_id, const char *cmpflgname, long val)
{
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown campaign flag, '%s'", cmpflgname);
        return;
  }
  command_add_value(Cmd_ADD_TO_CAMPAIGN_FLAG, plr_range_id, flg_id, val, 0);
}

static void command_export_variable(long plr_range_id, const char *varib_name, const char *cmpflgname)
{
    long src_type;
    long src_id;
    // Recognize flag
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown CAMPAIGN FLAG, '%s'", cmpflgname);
        return;
    }
    if (!parse_get_varib(varib_name, &src_id, &src_type))
    {
        SCRPTERRLOG("Unknown VARIABLE, '%s'", varib_name);
        return;
    }
    command_add_value(Cmd_EXPORT_VARIABLE, plr_range_id, src_type, src_id, flg_id);
}

static void command_set_game_rule(const char* objectv, unsigned long roomvar)
{
    long ruledesc = get_id(game_rule_desc, objectv);
    if (ruledesc == -1)
    {
        SCRPTERRLOG("Unknown game rule variable");
        return;
    }
    command_add_value(Cmd_SET_GAME_RULE, 0, ruledesc, roomvar, 0);
}

static void command_use_spell_on_creature(long plr_range_id, const char *crtr_name, const char *criteria, const char *magname, int splevel)
{
  SCRIPTDBG(11, "Starting");
  long mag_id = get_rid(spell_desc, magname);
  if (splevel < 1)
  {
    if ( (mag_id == SplK_Heal) || (mag_id == SplK_Armour) || (mag_id == SplK_Speed) || (mag_id == SplK_Disease) || (mag_id == SplK_Invisibility) || (mag_id == SplK_Chicken) )
    {
        SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", magname, splevel);
    }
    splevel = 1;
  }
  if (splevel > (MAGIC_OVERCHARGE_LEVELS+1)) //Creatures cast spells from level 1 to 10, but 10=9.
  {
    SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", magname, splevel, (MAGIC_OVERCHARGE_LEVELS+1));
    splevel = MAGIC_OVERCHARGE_LEVELS;
  }
  splevel--;
  if (mag_id == -1)
  {
    SCRPTERRLOG("Unknown magic, '%s'", magname);
    return;
  }
  long crtr_id = parse_creature_name(crtr_name);
  if (crtr_id == CREATURE_NONE) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = parse_criteria(criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  // SpellKind sp = mag_id;
  // encode params: free, magic, caster, level -> into 4xbyte: FMCL
  long fmcl_bytes;
  {
      signed char m = mag_id, lvl = splevel;
      fmcl_bytes = (m << 8) | lvl;
  }
  command_add_value(Cmd_USE_SPELL_ON_CREATURE, plr_range_id, crtr_id, select_id, fmcl_bytes);
}

static void command_creature_entrance_level(long plr_range_id, unsigned char val)
{
  command_add_value(Cmd_CREATURE_ENTRANCE_LEVEL, plr_range_id, val, 0, 0);
}

static void command_randomise_flag(long plr_range_id, const char *flgname, long val)
{
    long flg_id;
    long flag_type;
    if (!parse_set_varib(flgname, &flg_id, &flag_type))
    {
        SCRPTERRLOG("Unknown flag, '%s'", flgname);
        return;
    }
  command_add_value(Cmd_RANDOMISE_FLAG, plr_range_id, flg_id, val, flag_type);
}

static void command_compute_flag(long plr_range_id, const char *flgname, const char *operator_name, long src_plr_range_id, const char *src_flgname, long alt)
{
    long flg_id;
    long flag_type;
    if (!parse_set_varib(flgname, &flg_id, &flag_type))
    {
        SCRPTERRLOG("Unknown target flag, '%s'", flgname);
        return;
    }

    long src_flg_id;
    long src_flag_type;
    // try to identify source flag as a power, if it agrees, change flag type to SVar_AVAILABLE_MAGIC, keep power id
    // with rooms, traps, doors, etc. parse_get_varib assumes we want the count flag of them. Change it later in 'alt' switch if 'available' flag is needed
    src_flg_id = get_id(power_desc, src_flgname);
    if (src_flg_id == -1)
    {
        if (!parse_get_varib(src_flgname, &src_flg_id, &src_flag_type))
        {
            SCRPTERRLOG("Unknown source flag, '%s'", src_flgname);
            return;
        }
    } else
    {
        src_flag_type = SVar_AVAILABLE_MAGIC;
    }

    long op_id = get_rid(script_operator_desc, operator_name);
    if (op_id == -1)
    {
        SCRPTERRLOG("Invalid operation for modifying flag's value: '%s'", operator_name);
        return;
    }

    if (alt != 0)
    {
        switch (src_flag_type)
        {
            case SVar_CREATURE_NUM:
                src_flag_type = SVar_CONTROLS_CREATURE;
                break;
            case SVar_TOTAL_CREATURES:
                src_flag_type = SVar_CONTROLS_TOTAL_CREATURES;
                break;
            case SVar_TOTAL_DIGGERS:
                src_flag_type = SVar_CONTROLS_TOTAL_DIGGERS;
                break;
            case SVar_GOOD_CREATURES:
                src_flag_type = SVar_CONTROLS_GOOD_CREATURES;
                break;
            case SVar_EVIL_CREATURES:
                src_flag_type = SVar_CONTROLS_EVIL_CREATURES;
                break;
            case SVar_DOOR_NUM:
                src_flag_type = SVar_AVAILABLE_DOOR;
                break;
            case SVar_TRAP_NUM:
                src_flag_type = SVar_AVAILABLE_TRAP;
                break;
            case SVar_ROOM_SLABS:
                src_flag_type = SVar_AVAILABLE_ROOM;
                break;
        }
    }
    // encode 4 byte params into 4xbyte integer (from high-order bit to low-order):
    // 1st byte: src player range idx
    // 2nd byte: operation id
    // 3rd byte: flag type
    // 4th byte: src flag type
    long srcplr_op_flagtype_srcflagtype = (src_plr_range_id << 24) | (op_id << 16) | (flag_type << 8) | src_flag_type;
    command_add_value(Cmd_COMPUTE_FLAG, plr_range_id, srcplr_op_flagtype_srcflagtype, flg_id, src_flg_id);
}

/** Adds a script command to in-game structures.
 *
 * @param cmd_desc
 * @param scline
 */
void script_add_command(const struct CommandDesc *cmd_desc, const struct ScriptLine *scline)
{
    if (cmd_desc->check_fn != NULL)
    {
        cmd_desc->check_fn(scline);
        return;
    }
    switch (cmd_desc->index)
    {
    case Cmd_CREATE_PARTY:
        command_create_party(scline->tp[0]);
        break;
    case Cmd_ADD_PARTY_TO_LEVEL:
        command_add_party_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_ADD_CREATURE_TO_LEVEL:
        command_add_creature_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5]);
        break;
    case Cmd_ADD_OBJECT_TO_LEVEL:
        command_add_object_to_level(scline->tp[0], scline->tp[1], scline->np[2]);
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
        command_bonus_level_time(scline->np[0], scline->np[1]);
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
    case Cmd_QUICK_MESSAGE:
        command_quick_message(scline->np[0], scline->tp[1], scline->tp[2]);
        break;
    case Cmd_DISPLAY_MESSAGE:
        command_display_message(scline->np[0], scline->tp[1]);
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
    case Cmd_KILL_CREATURE:
        command_kill_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_LEVEL_UP_CREATURE:
        command_level_up_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_USE_POWER_ON_CREATURE:
        command_use_power_on_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->tp[4], scline->np[5], scline->np[6]);
        break;
    case Cmd_USE_SPELL_ON_CREATURE:
        command_use_spell_on_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->tp[3], scline->np[4]);
        break;
    case Cmd_USE_POWER_AT_POS:
        command_use_power_at_pos(scline->np[0], scline->np[1], scline->np[2], scline->tp[3], scline->np[4], scline->np[5]);
        break;
    case Cmd_USE_POWER_AT_LOCATION:
        command_use_power_at_location(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4]);
        break;
    case Cmd_USE_POWER:
        command_use_power(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_USE_SPECIAL_INCREASE_LEVEL:
        command_use_special_increase_level(scline->np[0], scline->np[1]);
        break;
    case Cmd_USE_SPECIAL_MULTIPLY_CREATURES:
        command_use_special_multiply_creatures(scline->np[0], scline->np[1]);
        break;
    case Cmd_USE_SPECIAL_MAKE_SAFE:
        command_use_special_make_safe(scline->np[0]);
        break;
    case Cmd_USE_SPECIAL_LOCATE_HIDDEN_WORLD:
        command_use_special_locate_hidden_world();
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
    case Cmd_COMPUTER_DIG_TO_LOCATION:
        command_computer_dig_to_location(scline->np[0], scline->tp[1], scline->tp[2]);
        break;
    case Cmd_CREATURE_ENTRANCE_LEVEL:
        command_creature_entrance_level(scline->np[0], scline->np[1]);
        break;
    case Cmd_RANDOMISE_FLAG:
        command_randomise_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_COMPUTE_FLAG:
        command_compute_flag(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->tp[4], scline->np[5]);
        break;
    default:
        SCRPTERRLOG("Unhandled SCRIPT command '%s'", scline->tcmnd);
        break;
    }
}




