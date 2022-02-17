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
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/


#include "lvl_script_conditions.h"

#include "globals.h"
#include "dungeon_data.h"
#include "config_magic.h"
#include "game_legacy.h"
#include "room_entrance.h"
#include "creature_states.h"
#include "keeperfx.hpp"
#include "bflib_math.h"
#include "lvl_script_lib.h"

#ifdef __cplusplus
extern "C" {
#endif


static int script_current_condition = 0;


DLLIMPORT unsigned short _DK_condition_stack_pos;
#define condition_stack_pos _DK_condition_stack_pos
DLLIMPORT unsigned short _DK_condition_stack[48];
#define condition_stack _DK_condition_stack


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
        return dungeonadd->mnfct_info.trap_amount_stored[validx%gameadd.trapdoor_conf.trap_types_count]
              + dungeonadd->mnfct_info.trap_amount_offmap[validx%gameadd.trapdoor_conf.trap_types_count];
    case SVar_AVAILABLE_DOOR: // IF_AVAILABLE(DOOR)
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->mnfct_info.door_amount_stored[validx%gameadd.trapdoor_conf.door_types_count]
              + dungeonadd->mnfct_info.door_amount_offmap[validx%gameadd.trapdoor_conf.door_types_count];
    case SVar_AVAILABLE_ROOM: // IF_AVAILABLE(ROOM)
        dungeon = get_dungeon(plyr_idx);
        return (dungeon->room_buildable[validx%ROOM_TYPES_COUNT] & 1);
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
    case SVar_SACRIFICED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->creature_sacrifice[validx];
    case SVar_REWARDED:
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->creature_awarded[validx];
    case SVar_EVIL_CREATURES_CONVERTED:
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->evil_creatures_converted;
    case SVar_GOOD_CREATURES_CONVERTED:
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->good_creatures_converted;
    case SVar_TRAPS_SOLD:
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->traps_sold;
    case SVar_DOORS_SOLD:
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->doors_sold;
    case SVar_MANUFACTURED_SOLD:
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->traps_sold + dungeonadd->doors_sold;
    case SVar_MANUFACTURE_GOLD:
        dungeonadd = get_dungeonadd(plyr_idx);
        return dungeonadd->manufacture_gold;
    case SVar_TOTAL_SCORE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->total_score;
    case SVar_BONUS_TIME:
        return (game.bonus_time - game.play_gameturn);
    default:
        break;
    };
    return 0;
}

TbBool condition_inactive(long cond_idx)
{
  if ((cond_idx < 0) || (cond_idx >= CONDITIONS_COUNT))
  {
      return false;
  }
  unsigned long i = gameadd.script.conditions[cond_idx].status;
  if (((i & 0x01) == 0) || ((i & 0x04) != 0))
    return true;
  return false;
}

TbBool get_condition_status(unsigned char opkind, long val1, long val2)
{
  return LbMathOperation(opkind, val1, val2) != 0;
}

static void process_condition(struct Condition *condt, int idx)
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
    if (gameadd.script.conditions_num > CONDITIONS_COUNT)
      gameadd.script.conditions_num = CONDITIONS_COUNT;
    for (long i = 0; i < gameadd.script.conditions_num; i++)
    {
      process_condition(&gameadd.script.conditions[i], i);
    }
}

long pop_condition(void)
{
  if (script_current_condition == CONDITION_ALWAYS)
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
    script_current_condition = CONDITION_ALWAYS;
  }
  return script_current_condition;
}

int get_script_current_condition()
{
    return script_current_condition;
}

void set_script_current_condition(int current_condition)
{
    script_current_condition = current_condition;
}

void command_add_condition(long plr_range_id, long opertr_id, long varib_type, long varib_id, long value)
{
    // TODO: replace with pointer to functions
    struct Condition* condt = &gameadd.script.conditions[gameadd.script.conditions_num];
    condt->condit_idx = script_current_condition;
    condt->plyr_range = plr_range_id;
    condt->variabl_type = varib_type;
    condt->variabl_idx = varib_id;
    condt->operation = opertr_id;
    condt->rvalue = value;
    if (condition_stack_pos >= CONDITIONS_COUNT)
    {
        gameadd.script.conditions_num++;
        SCRPTWRNLOG("Conditions too deep in script");
        return;
    }
    if (script_current_condition != CONDITION_ALWAYS)
    {
        condition_stack[condition_stack_pos] = script_current_condition;
        condition_stack_pos++;
    }
    script_current_condition = gameadd.script.conditions_num;
    gameadd.script.conditions_num++;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
