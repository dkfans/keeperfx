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
#include "creature_states_rsrch.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "power_hand.h"
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_temple(struct Thing *thing);
DLLIMPORT short _DK_praying_in_temple(struct Thing *thing);
DLLIMPORT long _DK_process_temple_function(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_in_temple(struct Thing *thing);
DLLIMPORT short _DK_cleanup_sacrifice(struct Thing *thing);
DLLIMPORT short _DK_creature_being_sacrificed(struct Thing *thing);
DLLIMPORT short _DK_creature_sacrifice(struct Thing *thing);
DLLIMPORT long _DK_process_sacrifice_award(struct Coord3d *pos, long model, long plyr_idx);
DLLIMPORT short _DK_creature_being_summoned(struct Thing *thing);
DLLIMPORT long _DK_make_all_players_creatures_angry(long plyr_idx);
DLLIMPORT long _DK_force_complete_current_manufacturing(long plyr_idx);
DLLIMPORT void _DK_apply_spell_effect_to_players_creatures(long a1, long a2, long a3);
DLLIMPORT void _DK_kill_all_players_chickens(long plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_is_doing_temple_activity(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_AtTemple) || (i == CrSt_PrayingInTemple))
        return true;
    return false;
}

short at_temple(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_temple(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_TEMPLE) || (room->owner != thing->owner))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    dungeon = get_dungeon(thing->owner);
    if ( !add_creature_to_work_room(thing, room) )
    {
        if (is_my_player_number(thing->owner))
            output_message(SMsg_TempleTooSmall, 0, true);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_PrayingInTemple);
    dungeon->creatures_praying[thing->model]++;
    cctrl->field_82 = 0;
    return 1;
}

short praying_in_temple(struct Thing *thing)
{
  return _DK_praying_in_temple(thing);
}

long process_temple_function(struct Thing *thing)
{
  return _DK_process_temple_function(thing);
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
  internal_set_thing_state(thing, 95);
  thing->field_25 |= 0x04;
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
  //return _DK_make_all_players_creatures_angry(plyr_idx);
  dungeon = get_players_num_dungeon(plyr_idx);
  k = 0;
  i = dungeon->creatr_list_start;
  while (i != 0)
  {
    thing = thing_get(i);
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
  return _DK_force_complete_current_manufacturing(plyr_idx);
}

void apply_spell_effect_to_players_creatures(long plyr_idx, long spl_idx, long overchrg)
{
  _DK_apply_spell_effect_to_players_creatures(plyr_idx, spl_idx, overchrg);
}

void kill_all_players_chickens(long plyr_idx)
{
  _DK_kill_all_players_chickens(plyr_idx);
}

short creature_being_summoned(struct Thing *thing)
{
  return _DK_creature_being_summoned(thing);
}

short cleanup_sacrifice(struct Thing *thing)
{
  return _DK_cleanup_sacrifice(thing);
}

long create_sacrifice_unique_award(struct Coord3d *pos, long plyr_idx, long sacfunc, long explevel)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(plyr_idx);
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
      ERRORLOG("Unsupported unique secrifice award!");
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

long process_sacrifice_award(struct Coord3d *pos, long model, long plyr_idx)
{
  struct SacrificeRecipe *sac;
  struct Dungeon *dungeon;
  long explevel;
  long ret;
  //return _DK_process_sacrifice_award(pos, model, plyr_idx);
  dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
  {
    ERRORLOG("Player %d cannot sacrifice creatures.",plyr_idx);
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
  //return _DK_creature_being_sacrificed(thing);

  cctrl = creature_control_get_from_thing(thing);
  cctrl->word_9A--;
  if (cctrl->word_9A > 0)
  {
    award = creature_turn_to_face_angle(thing, thing->field_52 + 256);
    thing->field_25 &= 0xDFu;
    return 0;
  }
  slb = get_slabmap_for_subtile(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
  owner = slabmap_owner(slb);
  add_creature_to_sacrifice_list(owner, thing->model, cctrl->explevel);
  pos.x.val = thing->mappos.x.val;
  pos.y.val = thing->mappos.y.val;
  pos.z.val = thing->mappos.z.val;
  model = thing->model;
  kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
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
        ERRORLOG("Invalid sacrifice return");
        break;
    }
  }
  return -1;
}

short creature_sacrifice(struct Thing *thing)
{
  return _DK_creature_sacrifice(thing);
}

/******************************************************************************/
