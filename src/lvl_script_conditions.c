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
#include "pre_inc.h"
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
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif


static int script_current_condition = 0;
static unsigned short condition_stack_pos;
static unsigned short condition_stack[CONDITIONS_COUNT];


long get_condition_value(PlayerNumber plyr_idx, unsigned char valtype, short validx)
{
    SYNCDBG(10,"Checking condition %d for player %d",(int)valtype,(int)plyr_idx);
    struct Dungeon* dungeon;
    struct Thing* thing;
    struct PlayerInfo* player;
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
    case SVar_TOTAL_TRAPS:
        return count_player_deployed_traps_of_model(plyr_idx, -1);
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
    case SVar_DESTROYED_KEEPER:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.keeper_destroyed[validx];
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
        dungeon = get_dungeon(plyr_idx);
        if (is_power_available(plyr_idx, validx)) {
            return dungeon->magic_level[validx];
        }
        return 0;
    case SVar_AVAILABLE_TRAP: // IF_AVAILABLE(TRAP)
        return count_player_available_traps_of_model(plyr_idx, (validx % game.conf.trapdoor_conf.trap_types_count));
    case SVar_AVAILABLE_DOOR: // IF_AVAILABLE(DOOR)
        return count_player_available_doors_of_model(plyr_idx, (validx % game.conf.trapdoor_conf.door_types_count));
    case SVar_AVAILABLE_ROOM: // IF_AVAILABLE(ROOM)
        dungeon = get_dungeon(plyr_idx);
        return (dungeon->room_buildable[validx%game.conf.slab_conf.room_types_count] & 1);
    case SVar_AVAILABLE_CREATURE: // IF_AVAILABLE(CREATURE)
        return count_player_available_creatures_of_model(plyr_idx, (validx % game.conf.crtr_conf.model_count));
    case SVar_AVAILABLE_TOTAL_TRAPS:
        return count_player_available_traps_of_model(plyr_idx, -1);
    case SVar_AVAILABLE_TOTAL_DOORS:
        return count_player_available_doors_of_model(plyr_idx, -1);
    case SVar_AVAILABLE_TOTAL_CREATURES:
        return count_player_available_creatures_of_model(plyr_idx, CREATURE_ANY);
    case SVar_SLAB_OWNER: //IF_SLAB_OWNER
    {
        long varib_id = get_slab_number((unsigned char)plyr_idx, validx);
        struct SlabMap* slb = get_slabmap_direct(varib_id);
        return slabmap_owner(slb);
    }
    case SVar_SLAB_TYPE: //IF_SLAB_TYPE
    {
        long varib_id = get_slab_number((unsigned char)plyr_idx, validx);
        struct SlabMap* slb = get_slabmap_direct(varib_id);
        return slb->kind;
    }
    case SVar_CONTROLS_CREATURE: // IF_CONTROLS(CREATURE)
        dungeon = get_dungeon(plyr_idx);
        return dungeon->owned_creatures_of_model[validx%game.conf.crtr_conf.model_count]
          - count_player_list_creatures_of_model_matching_bool_filter(plyr_idx, validx, creature_is_kept_in_custody_by_enemy_or_dying);
    case SVar_CONTROLS_TOTAL_CREATURES:// IF_CONTROLS(TOTAL_CREATURES)
        dungeon = get_dungeon(plyr_idx);
        return dungeon->num_active_creatrs - count_player_creatures_not_counting_to_total(plyr_idx);
    case SVar_CONTROLS_TOTAL_DIGGERS:// IF_CONTROLS(TOTAL_DIGGERS)
        dungeon = get_dungeon(plyr_idx);
        return dungeon->num_active_diggers - count_player_diggers_not_counting_to_total(plyr_idx);
    case SVar_ALL_DUNGEONS_DESTROYED:
    {
        player = get_player(plyr_idx);
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
        dungeon = get_dungeon(plyr_idx);
        return dungeon->box_info.activated[validx];
    case SVar_TRAP_ACTIVATED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->trap_info.activated[validx];
    case SVar_SACRIFICED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->creature_sacrifice[validx];
    case SVar_REWARDED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->creature_awarded[validx];
    case SVar_EVIL_CREATURES_CONVERTED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->evil_creatures_converted;
    case SVar_GOOD_CREATURES_CONVERTED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->good_creatures_converted;
    case SVar_TRAPS_SOLD:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->traps_sold;
    case SVar_DOORS_SOLD:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->doors_sold;
    case SVar_MANUFACTURED_SOLD:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->traps_sold + dungeon->doors_sold;
    case SVar_MANUFACTURE_GOLD:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->manufacture_gold;
    case SVar_TOTAL_SCORE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->total_score;
    case SVar_BONUS_TIME:
        return (game.bonus_time - game.play_gameturn);
    case SVar_CREATURES_TRANSFERRED:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->creatures_transferred;
    case SVar_ALLIED_PLAYER:
    {
        player = get_player(plyr_idx);
        return player_allied_with(player, validx);
    }
    case SVar_ACTIVE_BATTLES:
        return count_active_battles(plyr_idx);
    case SVar_VIEW_TYPE:
        player = get_player(plyr_idx);
        return player->view_type;
    case SVar_CONTROLLED_THING:
        player = get_player(plyr_idx);
        return player->controlled_thing_idx;
    case SVar_TOTAL_SLAPS:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.num_slaps;
    case SVar_SCORE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->score;
    case SVar_PLAYER_SCORE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->lvstats.player_score;
    case SVar_MANAGE_SCORE:
        dungeon = get_dungeon(plyr_idx);
        return dungeon->manage_score;
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
  unsigned long i = game.script.conditions[cond_idx].status;
  if (((i & 0x01) == 0) || ((i & 0x04) != 0))
    return true;
  return false;
}

TbBool get_condition_status(unsigned char opkind, long left_value, long right_value)
{
  return LbMathOperation(opkind, left_value, right_value) != 0;
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
        clear_flag(condt->status, 0x01);
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
                long left_value = get_condition_value(i, condt->variabl_type, condt->variabl_idx);

                long right_value;
                if (condt->use_second_variable)
                {
                    int plr_start_right;
                    int plr_end_right;
                    if (get_players_range(condt->plyr_range_right, &plr_start_right, &plr_end_right) < 0)
                    {
                        WARNLOG("Invalid player range %d in CONDITION command %d.", (int)condt->plyr_range, (int)condt->variabl_type);
                        return;
                    }
                    for (long j = plr_start_right; j < plr_end_right; j++)
                    {
                        right_value = get_condition_value(j, condt->variabl_type_right, condt->variabl_idx_right);
                        new_status = get_condition_status(condt->operation, left_value, right_value);
                        if (new_status != false)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    right_value = condt->rvalue;
                    new_status = get_condition_status(condt->operation, left_value, right_value);
                }

                if (new_status != false)
                {
                  break;
                }
            }
        }
    }

    SYNCDBG(19,"Condition type %d status %d",(int)condt->variabl_type,(int)new_status);
    set_flag_value(condt->status, 0x01, new_status);
    if (((condt->status & 0x01) == 0) || ((condt->status & 0x02) != 0))
    {
        clear_flag(condt->status, 0x04);
    } else
    {
        set_flag(condt->status, 0x02);
        set_flag(condt->status, 0x04);
    }
    SCRIPTDBG(19,"Finished");
}

void process_conditions(void)
{
    if (game.script.conditions_num > CONDITIONS_COUNT)
      game.script.conditions_num = CONDITIONS_COUNT;
    for (long i = 0; i < game.script.conditions_num; i++)
    {
      process_condition(&game.script.conditions[i], i);
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
    struct Condition* condt = &game.script.conditions[game.script.conditions_num];
    condt->condit_idx = script_current_condition;
    condt->plyr_range = plr_range_id;
    condt->variabl_type = varib_type;
    condt->variabl_idx = varib_id;
    condt->operation = opertr_id;
    condt->rvalue = value;
    condt->use_second_variable = false;

    if (condition_stack_pos >= CONDITIONS_COUNT)
    {
        game.script.conditions_num++;
        SCRPTWRNLOG("Conditions too deep in script");
        return;
    }
    if (script_current_condition != CONDITION_ALWAYS)
    {
        condition_stack[condition_stack_pos] = script_current_condition;
        condition_stack_pos++;
    }
    script_current_condition = game.script.conditions_num;
    game.script.conditions_num++;
}

void command_add_condition_2variables(long plr_range_id, long opertr_id, long varib_type, long varib_id,long plr_range_id_right, long varib_type_right, long varib_id_right)
{
    // TODO: replace with pointer to functions
    struct Condition* condt = &game.script.conditions[game.script.conditions_num];
    condt->condit_idx = script_current_condition;
    condt->plyr_range = plr_range_id;
    condt->variabl_type = varib_type;
    condt->variabl_idx = varib_id;
    condt->operation = opertr_id;
    condt->plyr_range_right = plr_range_id_right;
    condt->variabl_type_right = varib_type_right;
    condt->variabl_idx_right = varib_id_right;
    condt->use_second_variable = true;

    if (condition_stack_pos >= CONDITIONS_COUNT)
    {
        game.script.conditions_num++;
        SCRPTWRNLOG("Conditions too deep in script");
        return;
    }
    if (script_current_condition != CONDITION_ALWAYS)
    {
        condition_stack[condition_stack_pos] = script_current_condition;
        condition_stack_pos++;
    }
    script_current_condition = game.script.conditions_num;
    game.script.conditions_num++;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
