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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

extern long near_map_block_creature_filter_diagonal_random(const struct Thing *thing, MaxTngFilterParam param, long maximizer);

extern const struct CommandDesc command_desc[];
extern const struct CommandDesc dk1_command_desc[];


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


static void player_reveal_map_area(PlayerNumber plyr_idx, long x, long y, long w, long h)
{
  SYNCDBG(0,"Revealing around (%d,%d)",x,y);
  reveal_map_area(plyr_idx, x-(w>>1), x+(w>>1)+(w%1), y-(h>>1), y+(h>>1)+(h%1));
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
    case CSelCrit_OnNeutralGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 2);
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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
