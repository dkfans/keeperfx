/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_pray.c
 *     Creature state machine functions related to temple.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_pray.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_states_rsrch.h"
#include "creature_states_mood.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "creature_graphics.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_workshop.h"
#include "power_hand.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_state_cleanup_in_temple(struct Thing *creatng);
DLLIMPORT short _DK_creature_sacrifice(struct Thing *creatng);
DLLIMPORT void _DK_apply_spell_effect_to_players_creatures(long a1, long a2, long a3);
DLLIMPORT void _DK_kill_all_players_chickens(long plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_is_doing_temple_pray_activity(const struct Thing *thing)
{
    long i;
    i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_AtTemple) || (i == CrSt_PrayingInTemple))
        return true;
    return false;
}

CrStateRet process_temple_visuals(struct Thing *creatng, struct Room *room)
{
    struct CreatureControl *cctrl;
    long turns_in_temple;
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->instance_id != CrInst_NULL)
        return CrStRet_Unchanged;
    turns_in_temple = cctrl->field_82;
    if (turns_in_temple <= 120)
    {
        // Walk for 120 turns
        if (creature_setup_adjacent_move_for_job_within_room(creatng, room, Job_TEMPLE_PRAY)) {
            creatng->continue_state = get_continue_state_for_job(Job_TEMPLE_PRAY);
        }
    } else
    if (turns_in_temple < 120 + 50)
    {
        // Then celebrate for 50 turns
        set_creature_instance(creatng, CrInst_CELEBRATE_SHORT, 1, 0, 0);
    } else
    {
        // Then start from the beginning
        cctrl->field_82 = 0;
    }
    return CrStRet_Modified;
}

short at_temple(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Room *room;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, RoK_TEMPLE, thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s index %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    dungeon = get_dungeon(thing->owner);
    if (!add_creature_to_work_room(thing, room))
    {
        output_message_room_related_from_computer_or_player_action(room->owner, room->kind, OMsg_RoomTooSmall);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_PrayingInTemple);
    dungeon->creatures_praying[thing->model]++;
    cctrl->field_82 = 0;
    return 1;
}

CrStateRet praying_in_temple(struct Thing *thing)
{
    struct Room *room;
    TRACE_THING(thing);
    room = get_room_thing_is_on(thing);
    if (creature_job_in_room_no_longer_possible(room, Job_TEMPLE_PRAY, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    switch (process_temple_function(thing))
    {
    case CrCkRet_Deleted:
        return CrStRet_Deleted;
    case CrCkRet_Available:
        process_temple_visuals(thing, room);
        return CrStRet_Modified;
    default:
        return CrStRet_ResetOk;
    }
}

long process_temple_cure(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (creature_affected_by_spell(creatng, SplK_Disease))
        terminate_thing_spell_effect(creatng, SplK_Disease);
    if (creature_affected_by_spell(creatng, SplK_Chicken))
        terminate_thing_spell_effect(creatng, SplK_Chicken);
    cctrl->temple_cure_gameturn = game.play_gameturn;
    return 1;
}

CrCheckRet process_temple_function(struct Thing *thing)
{
    struct Room *room;
    room = get_room_thing_is_on(thing);
    if (!room_still_valid_as_type_for_thing(room, RoK_TEMPLE, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrCkRet_Continue;
    }
    { // Modify anger
        long anger_change;
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(thing);
        anger_change = process_work_speed_on_work_value(thing, crstat->annoy_in_temple);
        anger_apply_anger_to_creature(thing, anger_change, AngR_Other, 1);
    }
    // Terminate spells
    process_temple_cure(thing);
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        cctrl->field_82++;
    }
    return CrCkRet_Available;
}

short state_cleanup_in_temple(struct Thing *thing)
{
  return _DK_state_cleanup_in_temple(thing);
}

TbBool summon_creature(long model, struct Coord3d *pos, long owner, long explevel)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    SYNCDBG(4,"Creating model %ld for player %ld",model,owner);
    thing = create_creature(pos, model, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Could not create creature");
        return false;
    }
    init_creature_level(thing, explevel);
    internal_set_thing_state(thing, CrSt_CreatureBeingSummoned);
    thing->movement_flags |= TMvF_Unknown04;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->word_9C = 48;
    return true;
}

TbBool make_all_players_creatures_angry(long plyr_idx)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    int i;
    SYNCDBG(8,"Starting");
    dungeon = get_players_num_dungeon(plyr_idx);
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        anger_make_creature_angry(thing, 4);
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19,"Finished");
    return true;
}

long force_complete_current_manufacturing(long plyr_idx)
{
    struct Dungeon *dungeon;
    int manufct_required;
    long i;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Player %d cannot manufacture.",(int)plyr_idx);
        return 0;
    }
    if (dungeon->manufacture_class == TCls_Empty)
    {
        WARNLOG("No manufacture in progress for player %d",(int)plyr_idx);
        return 0;
    }
    manufct_required = manufacture_points_required(dungeon->manufacture_class, dungeon->manufacture_kind);
    if (manufct_required <= 0)
    {
        WARNLOG("No points required to finish manufacture of class %d",(int)dungeon->manufacture_class);
        return 0;
    }
    i = manufct_required << 8;
    if (i <= dungeon->manufacture_progress)
        i = dungeon->manufacture_progress;
    dungeon->manufacture_progress = i;
    return 0;
}

void apply_spell_effect_to_players_creatures(PlayerNumber plyr_idx, long spl_idx, long overchrg)
{
    //_DK_apply_spell_effect_to_players_creatures(plyr_idx, spl_idx, overchrg);
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    int i;
    SYNCDBG(8,"Starting");
    dungeon = get_players_num_dungeon(plyr_idx);
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        apply_spell_effect_to_thing(thing, spl_idx, overchrg);
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19,"Finished");
}

TbBool kill_creature_if_under_chicken_spell(struct Thing *thing)
{
    if (creature_affected_by_spell(thing, SplK_Chicken) && !thing_is_picked_up(thing))
    {
        thing->health = -1;
        return true;
    }
    SYNCDBG(19,"Skipped %s index %d",thing_model_name(thing),(int)thing->index);
    return false;
}

void kill_all_players_chickens(PlayerNumber plyr_idx)
{
    //_DK_kill_all_players_chickens(plyr_idx);
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    k = 0;
    i = slist->index;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing_exists(thing) && thing_is_mature_food(thing) && (thing->owner == plyr_idx)
          && !thing_is_picked_up(thing)) {
            thing->byte_17 = 1;
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    // Force leave or kill normal creatures and special diggers
    do_to_players_all_creatures_of_model(plyr_idx, -1, kill_creature_if_under_chicken_spell);
}

short creature_being_summoned(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    short orig_w, orig_h;
    short unsc_w, unsc_h;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        return 0;
    }
    if (cctrl->word_9A <= 0)
    {
        get_keepsprite_unscaled_dimensions(thing->anim_sprite, thing->move_angle_xy, thing->field_48, &orig_w, &orig_h, &unsc_w, &unsc_h);
        create_effect(&thing->mappos, TngEff_Unknown04, thing->owner);
        thing->movement_flags |= TMvF_Unknown04;
        cctrl->word_9A = 1;
        cctrl->word_9C = 48;//orig_h;
        return 0;
    }
    cctrl->word_9A++;
    if (cctrl->word_9A > cctrl->word_9C)
    {
        thing->movement_flags &= ~TMvF_Unknown04;
        set_start_state(thing);
        return 0;
    }
    // Rotate the creature as it appears from temple
    creature_turn_to_face_angle(thing, thing->move_angle_xy + LbFPMath_PI/4);
    return 0;
}

short cleanup_sacrifice(struct Thing *creatng)
{
    // If the creature has flight ability, return it to flying state
    restore_creature_flight_flag(creatng);
    creatng->movement_flags &= ~TMvF_Unknown04;
    return 1;
}

long create_sacrifice_unique_award(struct Coord3d *pos, PlayerNumber plyr_idx, long sacfunc, long explevel)
{
  switch (sacfunc)
  {
  case UnqF_MkAllAngry:
      make_all_players_creatures_angry(plyr_idx);
      return SacR_Punished;
  case UnqF_ComplResrch:
      force_complete_current_research(plyr_idx);
      return SacR_Awarded;
  case UnqF_ComplManufc:
      force_complete_current_manufacturing(plyr_idx);
      return SacR_Awarded;
  case UnqF_KillChickns:
      kill_all_players_chickens(plyr_idx);
      return SacR_Punished;
  case UnqF_CheaperImp:
      // No processing needed - just don't clear the amount of sacrificed imps.
      return SacR_Pleased;
  default:
      ERRORLOG("Unsupported unique sacrifice award!");
      return SacR_AngryWarn;
  }
}

long creature_sacrifice_average_explevel(struct Dungeon *dungeon, struct SacrificeRecipe *sac)
{
  long num;
  long exp;
  long i;
  long model;
  num = 0;
  exp = 0;
  for (i=0; i < MAX_SACRIFICE_VICTIMS; i++)
  {
    model = sac->victims[i];
    // Do not count the same model twice
    if (i > 0)
    {
      if (model == sac->victims[i-1])
        break;
    }
    num += dungeon->creature_sacrifice[model];
    exp += dungeon->creature_sacrifice_exp[model];
  }
  if (num < 1) num = 1;
  exp = (exp/num);
  if (exp < 0) return 0;
  return exp;
}

void creature_sacrifice_reset(struct Dungeon *dungeon, struct SacrificeRecipe *sac)
{
  long i;
  long model;
  // Some models may be set more than once; dut we don't really care...
  for (i=0; i < MAX_SACRIFICE_VICTIMS; i++)
  {
    model = sac->victims[i];
    dungeon->creature_sacrifice[model] = 0;
    dungeon->creature_sacrifice_exp[model] = 0;
  }
}

long sacrifice_victim_model_count(struct SacrificeRecipe *sac, long model)
{
  long i;
  long k;
  k = 0;
  for (i=0; i < MAX_SACRIFICE_VICTIMS; i++)
  {
    if (sac->victims[i] == model) k++;
  }
  return k;
}

TbBool sacrifice_victim_conditions_met(struct Dungeon *dungeon, struct SacrificeRecipe *sac)
{
  long i,required;
  long model;
  // Some models may be checked more than once; dut we don't really care...
  for (i=0; i < MAX_SACRIFICE_VICTIMS; i++)
  {
    model = sac->victims[i];
    if (model < 1) continue;
    required = sacrifice_victim_model_count(sac, model);
    SYNCDBG(6,"Model %d exists %d times",(int)model,(int)required);
    if (dungeon->creature_sacrifice[model] < required)
      return false;
  }
  return true;
}

long process_sacrifice_award(struct Coord3d *pos, long model, PlayerNumber plyr_idx)
{
  struct SacrificeRecipe *sac;
  struct Dungeon *dungeon;
  long explevel;
  long ret;
  dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
  {
    ERRORLOG("Player %d cannot sacrifice creatures.",(int)plyr_idx);
    return 0;
  }
  ret = SacR_DontCare;
  sac = &gameadd.sacrifice_recipes[0];
  do {
    // Check if the just sacrificed creature is in the sacrifice
    if (sacrifice_victim_model_count(sac,model) > 0)
    {
      // Set the return value in case of partial sacrifice recipe
      if (ret != SacR_Pleased)
      {
        switch (sac->action)
        {
        case SacA_MkGoodHero:
        case SacA_NegSpellAll:
        case SacA_NegUniqFunc:
          ret = SacR_AngryWarn;
          break;
        default:
          ret = SacR_Pleased;
          break;
        }
      }
      SYNCDBG(8,"Creature %d used in sacrifice %d",(int)model,(int)(sac-&gameadd.sacrifice_recipes[0]));
      // Check if the complete sacrifice condition is met
      if (sacrifice_victim_conditions_met(dungeon, sac))
      {
        SYNCDBG(6,"Sacrifice recipe %d condition met, action %d for player %d",(int)(sac-&gameadd.sacrifice_recipes[0]),(int)sac->action,(int)plyr_idx);
        explevel = creature_sacrifice_average_explevel(dungeon, sac);
        switch (sac->action)
        {
        case SacA_MkCreature:
            if (explevel >= CREATURE_MAX_LEVEL) explevel = CREATURE_MAX_LEVEL-1;
            if ( summon_creature(sac->param, pos, plyr_idx, explevel) )
              dungeon->lvstats.creatures_from_sacrifice++;
            ret = SacR_Awarded;
            break;
        case SacA_MkGoodHero:
            if (explevel >= CREATURE_MAX_LEVEL) explevel = CREATURE_MAX_LEVEL-1;
            if ( summon_creature(sac->param, pos, 4, explevel) )
              dungeon->lvstats.creatures_from_sacrifice++;
            ret = SacR_Punished;
            break;
        case SacA_NegSpellAll:
            if (explevel > SPELL_MAX_LEVEL) explevel = SPELL_MAX_LEVEL;
            apply_spell_effect_to_players_creatures(plyr_idx, sac->param, explevel);
            ret = SacR_Punished;
            break;
        case SacA_PosSpellAll:
            if (explevel > SPELL_MAX_LEVEL) explevel = SPELL_MAX_LEVEL;
            apply_spell_effect_to_players_creatures(plyr_idx, sac->param, explevel);
            ret = SacR_Awarded;
            break;
        case SacA_NegUniqFunc:
        case SacA_PosUniqFunc:
            ret = create_sacrifice_unique_award(pos, plyr_idx, sac->param, explevel);
            break;
        default:
            ERRORLOG("Unsupported sacrifice action %d!",(int)sac->action);
            ret = SacR_Pleased;
            break;
        }
        if ((ret != SacR_Pleased) && (ret != SacR_AngryWarn))
          creature_sacrifice_reset(dungeon, sac);
        return ret;
      }
    }
    sac++;
  } while (sac->action != SacA_None);
  return ret;
}

short creature_being_sacrificed(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct SlabMap *slb;
    struct Coord3d pos;
    long owner,model,award;
    SYNCDBG(6,"Starting");

    cctrl = creature_control_get_from_thing(thing);
    cctrl->word_9A--;
    if (cctrl->word_9A > 0)
    {
        // No flying while being sacrificed
        creature_turn_to_face_angle(thing, thing->move_angle_xy + LbFPMath_PI/4);
        thing->movement_flags &= ~TMvF_Flying;
        return 0;
    }
    slb = get_slabmap_for_subtile(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
    owner = slabmap_owner(slb);
    add_creature_to_sacrifice_list(owner, thing->model, cctrl->explevel);
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    model = thing->model;
    kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects|CrDed_NotReallyDying);
    award = process_sacrifice_award(&pos, model, owner);
    if (is_my_player_number(owner))
    {
      switch (award)
      {
      case SacR_AngryWarn:
          output_message(SMsg_SacrificeBad, 0, true);
          break;
      case SacR_DontCare:
          output_message(SMsg_SacrificeNeutral, 0, true);
          break;
      case SacR_Pleased:
          output_message(SMsg_SacrificeGood, 0, true);
          break;
      case SacR_Awarded:
          output_message(SMsg_SacrificeReward, 0, true);
          break;
      case SacR_Punished:
          output_message(SMsg_SacrificePunish, 0, true);
          break;
      default:
          ERRORLOG("Invalid sacrifice return, %d",(int)award);
          break;
      }
    }
    return -1;
}

short creature_sacrifice(struct Thing *thing)
{
    //return _DK_creature_sacrifice(thing);
    if ((thing->movement_flags & TMvF_Flying) != 0) {
        thing->movement_flags &= ~TMvF_Flying;
    }
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_slab_center_subtile(thing->mappos.x.stl.num));
    pos.y.val = subtile_coord_center(stl_slab_center_subtile(thing->mappos.y.stl.num));
    pos.z.val = thing->mappos.z.val;
    if (!subtile_has_sacrificial_on_top(pos.x.stl.num, pos.y.stl.num)) {
        set_start_state(thing);
        return 1;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_move_to(thing, &pos, cctrl->max_speed, 0, 0) == 0) {
        return 0;
    }
    if (get_thing_height_at(thing, &pos) != pos.z.val) {
        return 0;
    }
    if (thing_touching_floor(thing))
    {
        cctrl->word_9A = 48;
        cctrl->word_9C = 48;
        thing->movement_flags |= TMvF_Unknown04;
        internal_set_thing_state(thing, CrSt_CreatureBeingSacrificed);
        thing->creature.gold_carried = 0;
        struct SlabMap *slb;
        slb = get_slabmap_thing_is_on(thing);
        create_effect(&thing->mappos, TngEff_Unknown35, slabmap_owner(slb));
    }
    return 1;
}

/******************************************************************************/
