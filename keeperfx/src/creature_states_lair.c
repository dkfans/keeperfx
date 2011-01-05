/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_lair.c
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
#include "creature_states_lair.h"
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

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_lair_to_sleep(struct Thing *thing);
DLLIMPORT short _DK_cleanup_sleep(struct Thing *thing);
DLLIMPORT short _DK_creature_going_home_to_sleep(struct Thing *thing);
DLLIMPORT short _DK_creature_sleep(struct Thing *thing);
DLLIMPORT short _DK_creature_at_changed_lair(struct Thing *thing);
DLLIMPORT short _DK_creature_at_new_lair(struct Thing *thing);
DLLIMPORT short _DK_creature_change_lair(struct Thing *thing);
DLLIMPORT short _DK_creature_choose_room_for_lair_site(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_is_sleeping(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if ((i == CrSt_CreatureSleep))
        return true;
    return false;
}

long creature_will_sleep(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *lairtng;
    long dist_x,dist_y;
    cctrl = creature_control_get_from_thing(thing);
    lairtng = thing_get(cctrl->lairtng_idx);
    if (thing_is_invalid(lairtng))
        return false;
    dist_x = (long)thing->mappos.x.stl.num - (long)lairtng->mappos.x.stl.num;
    dist_y = (long)thing->mappos.y.stl.num - (long)lairtng->mappos.y.stl.num;
    return (abs(dist_x) < 1) && (abs(dist_y) < 1);
}

long process_lair_enemy(struct Thing *thing, struct Room *room)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Thing *enemytng;
    long combat_factor;
    cctrl = creature_control_get_from_thing(thing);
    // Shouldn't be possible. But just for sure.
    if (room_is_invalid(room))
    {
        return 0;
    }
    // If the room changed during creature's journey, end
    if ((room->kind != RoK_LAIR) || (room->owner != thing->owner) || (room->index != cctrl->field_68))
    {
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    // End if the creature has no lair enemy
    if (crstat->lair_enemy == 0)
    {
        return 0;
    }
    // Search for enemies no often than every 64 turns
    if (((game.play_gameturn + thing->index) & 0x3F) != 0)
    {
        return 0;
    }
    combat_factor = find_fellow_creature_to_fight_in_room(thing,room,crstat->lair_enemy,&enemytng);
    if (combat_factor < 1)
        return 0;
    set_creature_in_combat_to_the_death(thing, enemytng, combat_factor);
    return 1;
}

short creature_at_changed_lair(struct Thing *thing)
{
  return _DK_creature_at_changed_lair(thing);
}

short creature_at_new_lair(struct Thing *thing)
{
  return _DK_creature_at_new_lair(thing);
}

short creature_change_lair(struct Thing *thing)
{
  return _DK_creature_change_lair(thing);
}

short creature_choose_room_for_lair_site(struct Thing *thing)
{
  return _DK_creature_choose_room_for_lair_site(thing);
}

short at_lair_to_sleep(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *lairtng;
    struct Room *room;
    //return _DK_at_lair_to_sleep(thing);
    cctrl = creature_control_get_from_thing(thing);
    lairtng = thing_get(cctrl->lairtng_idx);
    cctrl->field_80 = 0;
    if (thing_is_invalid(lairtng) || (cctrl->field_21 == 0))
    {
        set_start_state(thing);
        return 0;
    }
    if (!creature_will_sleep(thing))
    {
        set_start_state(thing);
        return 0;
    }
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_LAIR) || (room->owner != thing->owner))
    {
        WARNLOG("Room of kind %d and owner %d is invalid for %s",(int)room->kind,(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if ((cctrl->field_68 == room->index))
    {
        set_start_state(thing);
        return 0;
    }
    if ( !creature_turn_to_face_angle(thing, lairtng->field_52) )
    {
      internal_set_thing_state(thing, CrSt_CreatureSleep);
      cctrl->field_82 = 200;
      thing->field_25 &= ~0x20;
    }
    process_lair_enemy(thing, room);
    return 1;
}

short cleanup_sleep(struct Thing *thing)
{
  return _DK_cleanup_sleep(thing);
}

short creature_going_home_to_sleep(struct Thing *thing)
{
  return _DK_creature_going_home_to_sleep(thing);
}

short creature_sleep(struct Thing *thing)
{
  return _DK_creature_sleep(thing);
}

/******************************************************************************/
