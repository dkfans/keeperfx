/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_prisn.c
 *     Creature state machine functions for their job in various rooms.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_prisn.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
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
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_cleanup_prison(struct Thing *thing);
DLLIMPORT short _DK_creature_arrived_at_prison(struct Thing *thing);
DLLIMPORT short _DK_creature_drop_body_in_prison(struct Thing *thing);
DLLIMPORT short _DK_creature_freeze_prisonors(struct Thing *thing);
DLLIMPORT short _DK_creature_in_prison(struct Thing *thing);
DLLIMPORT long _DK_process_prison_function(struct Thing *thing);
DLLIMPORT long _DK_process_prison_food(struct Thing *thing, struct Room *room);
DLLIMPORT long _DK_setup_prison_move(struct Thing *thing, struct Room *room);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool jailbreak_possible(struct Room *room, long plyr_idx)
{
  unsigned long i;
  unsigned long k;
  struct SlabMap *slb;
  if ( (room->owner == plyr_idx) || (!room->slabs_list) )
    return false;
  k = 0;
  i = room->slabs_list;
  while (i > 0)
  {
    slb = get_slabmap_direct(i);
    if (slabmap_block_invalid(slb))
    {
      ERRORLOG("Jump to invalid room slab detected");
      break;
    }
    if (slab_by_players_land(plyr_idx, slb_num_decode_x(i), slb_num_decode_y(i)))
      return true;
    i = get_next_slab_number_in_room(i);
    k++;
    if (k > map_tiles_x*map_tiles_y)
    {
      ERRORLOG("Infinite loop detected when sweeping room slabs");
      break;
    }
  }
  return false;
}

short cleanup_prison(struct Thing *thing)
{
  return _DK_cleanup_prison(thing);
}

short creature_arrived_at_prison(struct Thing *thing)
{
  return _DK_creature_arrived_at_prison(thing);
}

short creature_drop_body_in_prison(struct Thing *thing)
{
  return _DK_creature_drop_body_in_prison(thing);
}

short creature_freeze_prisonors(struct Thing *thing)
{
  return _DK_creature_freeze_prisonors(thing);
}

long setup_prison_move(struct Thing *thing, struct Room *room)
{
  return _DK_setup_prison_move(thing, room);
}

long process_prison_visuals(struct Thing *thing, struct Room *room)
{
  struct CreatureControl *cctrl;
  cctrl = creature_control_get_from_thing(thing);
  if (cctrl->field_D2 != 0)
    return Lb_OK;
  if (game.play_gameturn-cctrl->field_82 > 200)
  {
    if (game.play_gameturn-cctrl->field_82 < 250)
    {
      set_creature_instance(thing, 44, 1, 0, 0);
      if (game.play_gameturn-cctrl->long_9A > 32)
      {
        play_creature_sound(thing, CrSnd_PrisonMoan, 2, 0);
        cctrl->long_9A = game.play_gameturn;
      }
      return Lb_SUCCESS;
    }
    cctrl->field_82 = game.play_gameturn;
  }
  if ( setup_prison_move(thing, room) )
  {
    thing->continue_state = CrSt_CreatureInPrison;
    return Lb_SUCCESS;
  }
  return Lb_OK;
}

short creature_in_prison(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct Room *room;
  TbResult ret;
  //return _DK_creature_in_prison(thing);
  cctrl = creature_control_get_from_thing(thing);
  room = get_room_thing_is_on(thing);
  if (room_is_invalid(room))
  {
      set_start_state(thing);
      return Lb_OK;
  }
  if ( (room->kind != RoK_PRISON) || (cctrl->work_room_id != room->index) )
  {
      set_start_state(thing);
      return Lb_OK;
  }
  if (room->total_capacity < room->used_capacity)
  {
    if (is_my_player_number(room->owner))
      output_message(26, 0, 1);
    set_start_state(thing);
    return Lb_OK;
  }
  ret = process_prison_function(thing);
  if (ret == Lb_OK)
    process_prison_visuals(thing,room);
  return ret;
}

void prison_convert_creature_to_skeleton(struct Room *room, struct Thing *thing)
{
  struct Dungeon *dungeon;
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  struct Thing *crthing;
  long crmodel;
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  crmodel = get_room_create_creature_model(room->kind); // That normally returns skeleton breed
  crthing = create_creature(&thing->mappos, crmodel, room->owner);
  if (thing_is_invalid(crthing))
  {
      ERRORLOG("Couldn't create creature model %ld in prison", crmodel);
      return;
  }
  init_creature_level(crthing, cctrl->explevel);
  set_start_state(crthing);
  if (creature_model_bleeds(thing->model))
    create_effect_around_thing(thing, 10);
  kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
  dungeon = get_dungeon(room->owner);
  if (!dungeon_invalid(dungeon))
      dungeon->lvstats.skeletons_raised++;
}

TbBool process_prisoner_skelification(struct Thing *thing, struct Room *room)
{
  struct CreatureStats *crstat;
  crstat = creature_stats_get_from_thing(thing);
  if ( (thing->health >= 0) || (!crstat->humanoid_creature) )
    return false;
  if (ACTION_RANDOM(101) > game.prison_skeleton_chance)
    return false;
  if (is_my_player_number(thing->owner))
    output_message(55, 0, 1);
  prison_convert_creature_to_skeleton(room,thing);
  return true;
}

long process_prison_food(struct Thing *thing, struct Room *room)
{
  return _DK_process_prison_food(thing, room);
}

long process_prison_function(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct Room *room;
  //return _DK_process_prison_function(thing);
  cctrl = creature_control_get_from_thing(thing);
  room = room_get(cctrl->work_room_id);
  if ( !room_still_valid_as_type_for_thing(room, 4, thing) )
  {
    set_start_state(thing);
    return 1;
  }
  process_creature_hunger(thing);
  if ( process_prisoner_skelification(thing,room) )
    return -1;
  if ((cctrl->field_D2 == 0) && process_prison_food(thing, room) )
    return 1;
  if ((game.play_gameturn & 0x3F) != 0)
    return 0;
  if (!jailbreak_possible(room, thing->owner))
    return 0;
  if ( is_my_player_number(room->owner) )
    output_message(57, 0, 1);
  set_start_state(thing);
  return 1;
}

/******************************************************************************/
