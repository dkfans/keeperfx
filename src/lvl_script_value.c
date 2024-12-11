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
#include "magic.h"
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
 * Reads word from 'line' into 'param'. Sets if 'line_end' was reached.
 * @param line The input line position pointer.
 * @param param Output parameter acquired from the line.
 * @param parth_level Paraenesis level within the line, set to -1 on EOLN.
 */


static void player_reveal_map_area(PlayerNumber plyr_idx, long x, long y, long w, long h)
{
  SYNCDBG(0,"Revealing around (%ld,%ld)",x,y);
  reveal_map_area(plyr_idx, x-(w>>1), x+(w>>1)+(w%1), y-(h>>1), y+(h>>1)+(h%1));
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
 * Cast a keeper power on a creature which meets given criteria.
 * @param plyr_idx The player whose creature will be affected.
 * @param crmodel Model of the creature to find.
 * @param criteria Criteria, from CreatureSelectCriteria enumeration.
 * @param fmcl_bytes encoded bytes: f=cast for free flag,m=power kind,c=caster player index,l=spell level.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_power_on_creature_matching_criterion(PlayerNumber plyr_idx, long crmodel, long criteria, long fmcl_bytes)
{
    struct Thing* thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing)) {
        SYNCDBG(5, "No matching player %d creature of model %d (%s) found to use power on.", (int)plyr_idx, (int)crmodel, creature_code_name(crmodel));
        return Lb_FAIL;
    }

    char is_free = (fmcl_bytes >> 24) != 0;
    PowerKind pwkind = (fmcl_bytes >> 16) & 255;
    PlayerNumber caster = (fmcl_bytes >> 8) & 255;
    long splevel = fmcl_bytes & 255;
    return script_use_power_on_creature(thing, pwkind, splevel, caster, is_free);
}

/**
 * Cast a spell on a creature which meets given criteria.
 * @param plyr_idx The player whose creature will be affected.
 * @param crmodel Model of the creature to find.
 * @param criteria Criteria, from CreatureSelectCriteria enumeration.
 * @param fmcl_bytes encoded bytes: f=cast for free flag,m=power kind,c=caster player index,l=spell level.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_spell_on_creature(PlayerNumber plyr_idx, ThingModel crmodel, long criteria, long fmcl_bytes)
{
    struct Thing *thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing))
    {
        SYNCDBG(5, "No matching player %d creature of model %d (%s) found to use spell on.", (int)plyr_idx, (int)crmodel, creature_code_name(crmodel));
        return Lb_FAIL;
    }
    SpellKind spkind = (fmcl_bytes >> 8) & 255; // What the hell is this?
    struct SpellConfig *spconf = get_spell_config(spkind);
    if ((spconf->caster_affected) && (!creature_is_immune_to_spell_flags(thing, spconf->spell_flags)))
    { // Immunity is handled in 'apply_spell_effect_to_thing', but this command plays sounds, so check for it.
        if (thing_is_picked_up(thing))
        {
            SYNCDBG(5, "Found creature to cast the spell on but it is being held.");
            return Lb_FAIL;
        }
        unsigned short sound;
        if (spconf->caster_affect_sound)
        {
            sound = spconf->caster_affect_sound;
        }
        else
        {
            sound = 0;
        }
        long splevel = fmcl_bytes & 255; // -.- Yes. Good luck to whoever wants to refactor this command to the new style.
        thing_play_sample(thing, sound, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
        apply_spell_effect_to_thing(thing, spkind, splevel);
        if (flag_is_set(spconf->spell_flags, CSAfF_Disease))
        {
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(thing);
            cctrl->disease_caster_plyridx = game.neutral_player_num; // Does not spread.
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
        WARNLOG("Can't decode origin location %ld", origin);
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
        WARNLOG("Can't decode destination location %ld", destination);
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

    unsigned long allow_flags = PwCast_AllGround | PwCast_Unrevealed;
    unsigned long mod_flags = 0;
    if (is_free)
        set_flag(mod_flags,PwMod_CastForFree);

    return magic_use_power_on_subtile(plyr_idx, powerKind, splevel, stl_x, stl_y, allow_flags, mod_flags);
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
    SYNCDBG(0, "Using power at location of type %lu", target);
    long x = 0;
    long y = 0;
    find_map_location_coords(target, &x, &y, plyr_idx, __func__);
    if ((x == 0) && (y == 0))
    {
        WARNLOG("Can't decode location %lu", target);
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
void script_make_safe(PlayerNumber plyr_idx)
{
    make_safe(get_player(plyr_idx));
}

/**
 * Defortifies player's dungeon.
 * @param plyr_idx target player
 */
void script_make_unsafe(PlayerNumber plyr_idx)
{
    make_unsafe(plyr_idx);
}

/**
 * Enables bonus level for current player.
 */
TbBool script_locate_hidden_world()
{
    return activate_bonus_level(get_player(my_player_number));
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
      context.plr_start = plr_start;
      context.plr_end = plr_end;
      // TODO: this should be checked for sanity
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
      add_creature_to_pool(val2, val3);
      break;
  case Cmd_TUTORIAL_FLASH_BUTTON:
      gui_set_button_flashing(val2, val3);
      break;
  case Cmd_SET_CREATURE_HEALTH:
      change_max_health_of_creature_kind(val2, val3);
      break;
  case Cmd_SET_CREATURE_STRENGTH:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->strength = saturate_set_unsigned(val3, 8);
      break;
  case Cmd_SET_CREATURE_ARMOUR:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->armour = saturate_set_unsigned(val3, 8);
      break;
  case Cmd_SET_CREATURE_FEAR_WOUNDED:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->fear_wounded = saturate_set_unsigned(val3, 8);
      break;
  case Cmd_SET_CREATURE_FEAR_STRONGER:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->fear_stronger = saturate_set_unsigned(val3, 16);
      break;
  case Cmd_SET_CREATURE_FEARSOME_FACTOR:
      crstat = creature_stats_get(val2);
      if (creature_stats_invalid(crstat))
          break;
      crstat->fearsome_factor = saturate_set_unsigned(val3, 16);
      break;
  case Cmd_SET_CREATURE_PROPERTY:
      crconf = &game.conf.crtr_conf.model[val2];
      crstat = creature_stats_get(val2);
      switch (val3)
      {
      case 1: // BLEEDS
          crstat->bleeds = val4;
          break;
      case 2: // UNAFFECTED_BY_WIND
          if (val4 >= 1)
          {
              set_flag(crstat->immunity_flags, CSAfF_Wind);
          }
          else
          {
              clear_flag(crstat->immunity_flags, CSAfF_Wind);
          }
          break;
      case 3: // IMMUNE_TO_GAS
          if (val4 >= 1)
          {
              set_flag(crstat->immunity_flags, CSAfF_PoisonCloud);
          }
          else
          {
              clear_flag(crstat->immunity_flags, CSAfF_PoisonCloud);
          }
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
              set_flag(crconf->model_flags,CMF_IsSpecDigger);
          }
          else
          {
              clear_flag(crconf->model_flags,CMF_IsSpecDigger);
          }
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
              set_flag(crstat->immunity_flags, CSAfF_Chicken);
          }
          else
          {
              clear_flag(crstat->immunity_flags, CSAfF_Chicken);
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
              set_flag(crstat->immunity_flags, CSAfF_Disease);
          }
          else
          {
              clear_flag(crstat->immunity_flags, CSAfF_Disease);
          }
          break;
      case 26: // ILLUMINATED
          crstat->illuminated = val4;
          break;
      case 27: // ALLURING_SCVNGR
          crstat->entrance_force = val4;
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
      set_flag_value(game.flags_cd, MFlg_DeadBackToPool, val2);
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
