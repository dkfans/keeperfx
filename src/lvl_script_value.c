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
#include "pre_inc.h"
#include "thing_data.h"
#include "thing_list.h"
#include "map_data.h"
#include "map_locations.h"
#include "player_data.h"
#include "magic_powers.h"
#include "keeperfx.hpp"
#include "lvl_filesdk1.h"
#include "power_hand.h"
#include "power_specials.h"
#include "creature_states_pray.h"
#include "player_utils.h"
#include "room_library.h"
#include "gui_soundmsgs.h"
#include "bflib_sound.h"
#include "map_blocks.h"
#include "room_util.h"

#include "lvl_script_lib.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

extern const struct CommandDesc command_desc[];
extern const struct CommandDesc dk1_command_desc[];


/******************************************************************************/

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
        SYNCDBG(5,"No matching player %d creature of model %d (%s) found to kill",(int)plyr_idx,(int)crmodel, creature_code_name(crmodel));
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
        SYNCDBG(5,"No matching player %d creature of model %d (%s) found to kill",(int)origin_plyr_idx,(int)crmodel, creature_code_name(crmodel));
        return false;
    }
    if (is_thing_some_way_controlled(thing))
    {
        //does not kill the creature, but does the preparations needed for when it is possessed
        prepare_to_controlled_creature_death(thing);
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
        SYNCDBG(5,"No matching player %d creature of model %d (%s) found to level up",(int)plyr_idx,(int)crmodel, creature_code_name(crmodel));
        return false;
    }
    creature_change_multiple_levels(thing,count);
    return true;
}

/**
 * Processes given VALUE immediately.
 * This processes given script command. It is used to process VALUEs at start when they have
 * no conditions, or during the gameplay when conditions are met.
 */
void script_process_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4, struct ScriptValue *value)
{
  struct CreatureModelConfig *crconf;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
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
      WARNLOG("Unexpected index:%lu", var_index);
      return;
  }
  if (desc->process_fn)
  {
      // TODO: move two functions up
      struct ScriptContext context;
      
      for (i=plr_start; i < plr_end; i++)
      {
          context.player_idx = i;
          context.value = value;
          desc->process_fn(&context);
      }
      return;
  }

  switch (var_index)
  {
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
      add_creature_to_pool(val2, val3);
      break;
  case Cmd_SET_CREATURE_HEALTH:
      change_max_health_of_creature_kind(val2, val3);
      break;
  case Cmd_SET_CREATURE_STRENGTH:
      crconf = creature_stats_get(val2);
      if (creature_stats_invalid(crconf))
          break;
      crconf->strength = saturate_set_unsigned(val3, 16);
      break;
  case Cmd_SET_CREATURE_ARMOUR:
      crconf = creature_stats_get(val2);
      if (creature_stats_invalid(crconf))
          break;
      crconf->armour = saturate_set_unsigned(val3, 8);
      break;
  case Cmd_SET_CREATURE_FEAR_WOUNDED:
      crconf = creature_stats_get(val2);
      if (creature_stats_invalid(crconf))
          break;
      crconf->fear_wounded = saturate_set_unsigned(val3, 8);
      break;
  case Cmd_SET_CREATURE_FEAR_STRONGER:
      crconf = creature_stats_get(val2);
      if (creature_stats_invalid(crconf))
          break;
      crconf->fear_stronger = saturate_set_unsigned(val3, 16);
      break;
  case Cmd_SET_CREATURE_FEARSOME_FACTOR:
      crconf = creature_stats_get(val2);
      if (creature_stats_invalid(crconf))
          break;
      crconf->fearsome_factor = saturate_set_unsigned(val3, 16);
      break;
  case Cmd_SET_CREATURE_PROPERTY:
      crconf = &game.conf.crtr_conf.model[val2];
      crconf = creature_stats_get(val2);
      switch (val3)
      {
      case 1: // BLEEDS
          crconf->bleeds = val4;
          break;
      case 2: // UNAFFECTED_BY_WIND
          if (val4 >= 1)
          {
              set_flag(crconf->immunity_flags, CSAfF_Wind);
          }
          else
          {
              clear_flag(crconf->immunity_flags, CSAfF_Wind);
          }
          break;
      case 3: // IMMUNE_TO_GAS
          if (val4 >= 1)
          {
              set_flag(crconf->immunity_flags, CSAfF_PoisonCloud);
          }
          else
          {
              clear_flag(crconf->immunity_flags, CSAfF_PoisonCloud);
          }
          break;
      case 4: // HUMANOID_SKELETON
          crconf->humanoid_creature = val4;
          break;
      case 5: // PISS_ON_DEAD
          crconf->piss_on_dead = val4;
          break;
      case 7: // FLYING
          crconf->flying = val4;
          break;
      case 8: // SEE_INVISIBLE
          crconf->can_see_invisible = val4;
          break;
      case 9: // PASS_LOCKED_DOORS
          crconf->can_go_locked_doors = val4;
          break;
      case 10: // SPECIAL_DIGGER
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_IsSpecDigger);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_IsSpecDigger);
          }
          recalculate_all_creature_digger_lists();
          update_creatr_model_activities_list(1);
          break;
      case 11: // ARACHNID
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_IsArachnid);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_IsArachnid);
          }
          break;
      case 12: // DIPTERA
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_IsDiptera);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_IsDiptera);
          }
          break;
      case 13: // LORD
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_IsLordOfLand);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_IsLordOfLand);
          }
          break;
      case 14: // SPECTATOR
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_IsSpectator);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_IsSpectator);
          }
          break;
      case 15: // EVIL
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_IsEvil);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_IsEvil);
          }
          break;
      case 16: // NEVER_CHICKENS
          if (val4 >= 1)
          {
              set_flag(crconf->immunity_flags, CSAfF_Chicken);
          }
          else
          {
              clear_flag(crconf->immunity_flags, CSAfF_Chicken);
          }
          break;
      case 17: // IMMUNE_TO_BOULDER
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_ImmuneToBoulder);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_ImmuneToBoulder);
          }
          break;
      case 18: // NO_CORPSE_ROTTING
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_NoCorpseRotting);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_NoCorpseRotting);
          }
          break;
      case 19: // NO_ENMHEART_ATTCK
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_NoEnmHeartAttack);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_NoEnmHeartAttack);
          }
          break;
      case 20: // TREMBLING_FAT
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_Trembling);
              set_flag(crconf->model_flags,CMF_Fat);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_Trembling);
              clear_flag(crconf->model_flags,CMF_Fat);
          }
          break;
      case 21: // FEMALE
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_Female);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_Female);
          }
          break;
      case 22: // INSECT
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_Insect);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_Insect);
          }
          break;
      case 23: // ONE_OF_KIND
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_OneOfKind);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_OneOfKind);
          }
          break;
      case 24: // NO_IMPRISONMENT
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_NoImprisonment);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_NoImprisonment);
          }
          break;
      case 25: // NEVER_SICK
          if (val4 >= 1)
          {
              set_flag(crconf->immunity_flags, CSAfF_Disease);
          }
          else
          {
              clear_flag(crconf->immunity_flags, CSAfF_Disease);
          }
          break;
      case 26: // ILLUMINATED
          crconf->illuminated = val4;
          break;
      case 27: // ALLURING_SCVNGR
          crconf->entrance_force = val4;
          break;
      case 28: // NO_RESURRECT
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags, CMF_NoResurrect);
          }
          else
          {
              clear_flag(crconf->model_flags, CMF_NoResurrect);
          }
          break;
      case 29: // NO_TRANSFER
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags, CMF_NoTransfer);
          }
          else
          {
              clear_flag(crconf->model_flags, CMF_NoTransfer);
          }
          break;
      case 30: // TREMBLING
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_Trembling);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_Trembling);
          }
          break;
      case 31: // FAT
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_Fat);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_Fat);
          }
          break;
      case 32: // NO_STEAL_HERO
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_NoStealHero);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_NoStealHero);
          }
          break;
      case 33: // PREFER_STEAL
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags,CMF_PreferSteal);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_PreferSteal);
          }
          break;
      case 34: // EVENTFUL_DEATH
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags, CMF_EventfulDeath);
          }
          else
          {
              clear_flag(crconf->model_flags, CMF_EventfulDeath);
          }
          break;
      case 35: // DIGGING_CREATURE
          if (val4 >= 1)
          {
              set_flag(crconf->model_flags, CMF_IsDiggingCreature);
          }
          else
          {
              clear_flag(crconf->model_flags, CMF_IsDiggingCreature);
          }
          recalculate_all_creature_digger_lists();
          update_creatr_model_activities_list(1);
          break;
      default:
          SCRPTERRLOG("Unknown creature property '%ld'", val3);
          break;
      }
      break;
  case Cmd_ALLY_PLAYERS:
      for (i=plr_start; i < plr_end; i++)
      {
          set_ally_with_player(i, val2, (val3 & 1) ? true : false);
          set_ally_with_player(val2, i, (val3 & 1) ? true : false);
          set_player_ally_locked(i, val2, (val3 & 2) ? true : false);
          set_player_ally_locked(val2, i, (val3 & 2) ? true : false);
      }
      break;
  case Cmd_DEAD_CREATURES_RETURN_TO_POOL:
      set_flag_value(game.mode_flags, MFlg_DeadBackToPool, val2);
      break;
  case Cmd_BONUS_LEVEL_TIME:
      if (val2 > 0) {
          game.bonus_time = game.play_gameturn + val2;
          set_flag(game.flags_gui,GGUI_CountdownTimer);
      } else {
          game.bonus_time = 0;
          clear_flag(game.flags_gui,GGUI_CountdownTimer);
      }
      if (level_file_version > 0)
      {
          game.timer_real = (TbBool)val3;
      }
      else
      {
          game.timer_real = false;
      }
      break;
  case Cmd_QUICK_OBJECTIVE:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end))
          process_objective(game.quick_messages[val2%QUICK_MESSAGES_COUNT], val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      break;
  case Cmd_QUICK_INFORMATION:
      if ((my_player_number >= plr_start) && (my_player_number < plr_end))
          set_quick_information(val2, val3, stl_num_decode_x(val4), stl_num_decode_y(val4));
      break;
  case Cmd_ADD_GOLD_TO_PLAYER:
      for (i=plr_start; i < plr_end; i++)
      {
          if (val2 > SENSIBLE_GOLD)
          {
              val2 = SENSIBLE_GOLD;
              SCRPTWRNLOG("Gold added to player %d reduced to %d", (int)plr_range_id, SENSIBLE_GOLD);
          }
          if (val2 >= 0)
          {
              player_add_offmap_gold(i, val2);
          }
          else
          {
              take_money_from_dungeon(i, -val2, 0);
          }
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
          script_use_power_on_creature_matching_criterion(i, val2, val3, val4);
      }
      break;
    case Cmd_USE_SPELL_ON_CREATURE:
      script_use_spell_on_creature_with_criteria(plr_range_id, val2, val3, val4);
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
    case Cmd_MAKE_SAFE:
      for (i=plr_start; i < plr_end; i++)
      {
          script_make_safe(i);
      }
      break;
    case Cmd_LOCATE_HIDDEN_WORLD:
      script_locate_hidden_world();
      break;
    case Cmd_CHANGE_CREATURE_OWNER:
      for (i=plr_start; i < plr_end; i++)
      {
          script_change_creature_owner_with_criteria(i, val2, val3, val4);
      }
      break;
    case Cmd_MAKE_UNSAFE:
      for (i=plr_start; i < plr_end; i++)
      {
          script_make_unsafe(i);
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
  case Cmd_CREATURE_ENTRANCE_LEVEL:
  {
    if (val2 > 0)
    {
        if (plr_range_id == ALL_PLAYERS)
        {
            for (i = 0; i < PLAYERS_COUNT; i++)
            {
                dungeon = get_dungeon(i);
                if (!dungeon_invalid(dungeon))
                {
                    dungeon->creature_entrance_level = (val2 - 1);
                }
            }
        }
        else
        {
            dungeon = get_dungeon(plr_range_id);
            if (!dungeon_invalid(dungeon))
            {
                dungeon->creature_entrance_level = (val2 - 1);
            }
        }
    }
    break;
  }
  case Cmd_RANDOMISE_FLAG:
      for (i=plr_start; i < plr_end; i++)
      {
          if (val3 == 0)
          {
              long current_flag_val = get_condition_value(i, val4, val2);
              set_variable(i, val4, val2, GAME_RANDOM(current_flag_val) + 1);
          }
          else
          {
              set_variable(i, val4, val2, GAME_RANDOM(val3) + 1);
          }
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
            SCRIPTDBG(7,"Changing player%ld's %ld flag from %ld to %ld based on flag of type %u.",
                i, val3, current_flag_val, computed, src_flag_type);
            set_variable(i, flag_type, val3, computed);
        }
      }
      break;
  default:
      WARNMSG("Unsupported Game VALUE, command %lu.",var_index);
      break;
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
