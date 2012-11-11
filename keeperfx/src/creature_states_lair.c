/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_lair.c
 *     Creature state machine functions for their job in various rooms.
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
#include "creature_states_lair.h"
#include "globals.h"

#include "bflib_math.h"
#include "thing_physics.h"
#include "creature_states.h"
#include "creature_states_combt.h"
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
#include "engine_arrays.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_lair_to_sleep(struct Thing *creatng);
DLLIMPORT short _DK_cleanup_sleep(struct Thing *creatng);
DLLIMPORT short _DK_creature_going_home_to_sleep(struct Thing *creatng);
DLLIMPORT short _DK_creature_sleep(struct Thing *creatng);
DLLIMPORT short _DK_creature_at_changed_lair(struct Thing *creatng);
DLLIMPORT short _DK_creature_at_new_lair(struct Thing *creatng);
DLLIMPORT short _DK_creature_change_lair(struct Thing *creatng);
DLLIMPORT short _DK_creature_choose_room_for_lair_site(struct Thing *creatng);
DLLIMPORT long _DK_creature_add_lair_to_room(struct Thing *creatng, struct Room *room);
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
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    lairtng = thing_get(cctrl->lairtng_idx);
    TRACE_THING(lairtng);
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
    if ((room->kind != RoK_LAIR) || (room->owner != thing->owner) || (room->index != cctrl->lair_room_id))
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
    if (!set_creature_in_combat_to_the_death(thing, enemytng, combat_factor))
        return 0;
    return 1;
}

long creature_add_lair_to_room(struct Thing *creatng, struct Room *room)
{
    struct Thing *lairtng;
    if (!room_has_enough_free_capacity_for_creature(room, creatng))
        return 0;
    //return _DK_creature_add_lair_to_room(thing, room);
    // Make sure we don't already have a lair on that position
    lairtng = find_creature_lair_at_subtile(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, 0);
    if (!thing_is_invalid(lairtng))
        return 0;
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    crstat = creature_stats_get_from_thing(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    room->field_17[creatng->model]++;
    room->used_capacity += crstat->lair_size;
    if ((cctrl->lair_room_id > 0) && (cctrl->lairtng_idx > 0))
    {
        struct Room *room;
        room = room_get(cctrl->lair_room_id);
        creature_remove_lair_from_room(creatng, room);
    }
    cctrl->lair_room_id = room->index;
    // Create the lair thing
    struct CreatureData *crdata;
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    crdata = creature_data_get_from_thing(creatng);
    lairtng = create_object(&pos, crdata->field_1, creatng->owner, -1);
    if (thing_is_invalid(lairtng))
    {
        ERRORLOG("Could not create lair totem");
        remove_thing_from_mapwho(creatng);
        place_thing_in_mapwho(creatng);
        return 1; // Return that so we won't try to redo the action over and over
    }
    lairtng->mappos.z.val = get_thing_height_at(lairtng, &lairtng->mappos);
    // Associate creature with the lair
    cctrl->lairtng_idx = lairtng->index;
    lairtng->word_13 = creatng->index;
    lairtng->word_15 = 1;
    // Lair size depends on creature level
    lairtng->word_17 = 300 * cctrl->explevel / 20 + 300;
    lairtng->field_52 = ACTION_RANDOM(0x800);
    struct Objects *objdat;
    unsigned long i;
    objdat = get_objects_data_for_thing(lairtng);
    i = convert_td_iso(objdat->field_5);
    set_thing_draw(lairtng, i, objdat->field_7, lairtng->word_15, 0, -1, objdat->field_11);
    thing_play_sample(creatng, 158, 100, 0, 3u, 1u, 2, 256);
    create_effect(&pos, imp_spangle_effects[creatng->owner], creatng->owner);
    anger_set_creature_anger(creatng, 0, 3);
    remove_thing_from_mapwho(creatng);
    place_thing_in_mapwho(creatng);
    return 1;
}

CrStateRet creature_at_changed_lair(struct Thing *creatng)
{
    struct Room *room;
    TRACE_THING(creatng);
    //return _DK_creature_at_changed_lair(thing);
    if (!thing_is_on_own_room_tile(creatng))
    {
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }
    room = get_room_thing_is_on(creatng);
    if (!room_initially_valid_as_type_for_thing(room, RoK_LAIR, creatng))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng));
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }
    if (!creature_add_lair_to_room(creatng, room)) {
        internal_set_thing_state(creatng, CrSt_CreatureChooseRoomForLairSite);
        return CrStRet_Modified;
    }
    // All done - finish the state
    set_start_state(creatng);
    return CrStRet_ResetOk;
}

CrStateRet creature_at_new_lair(struct Thing *thing)
{
    struct Room *room;
    TRACE_THING(thing);
    //return _DK_creature_at_new_lair(thing);
    room = get_room_thing_is_on(thing);
    if ( !room_still_valid_as_type_for_thing(room, RoK_LAIR, thing) )
    {
        WARNLOG("Room %s owned by player %d is bad work place for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->owner);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    if (!creature_add_lair_to_room(thing, room))
    {
        internal_set_thing_state(thing, CrSt_CreatureChooseRoomForLairSite);
        return CrStRet_Modified;
    }
    set_start_state(thing);
    return CrStRet_ResetOk;
}

short creature_change_lair(struct Thing *thing)
{
    TRACE_THING(thing);
    return _DK_creature_change_lair(thing);
}

short creature_choose_room_for_lair_site(struct Thing *thing)
{
    TRACE_THING(thing);
    return _DK_creature_choose_room_for_lair_site(thing);
}

short at_lair_to_sleep(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *lairtng;
    struct Room *room;
    TRACE_THING(thing);
    //return _DK_at_lair_to_sleep(thing);
    cctrl = creature_control_get_from_thing(thing);
    lairtng = thing_get(cctrl->lairtng_idx);
    TRACE_THING(lairtng);
    cctrl->target_room_id = 0;
    if (thing_is_invalid(lairtng) || (cctrl->field_21 != 0))
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
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if ((cctrl->lair_room_id != room->index))
    {
        set_start_state(thing);
        return 0;
    }
    if ( !creature_turn_to_face_angle(thing, lairtng->field_52) )
    {
        internal_set_thing_state(thing, CrSt_CreatureSleep);
        cctrl->field_82 = 200;
        thing->movement_flags &= ~TMvF_Flying;
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
