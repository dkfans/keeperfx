/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_spdig.c
 *     Creature state machine functions for special diggers (imps).
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
#include "pre_inc.h"
#include "creature_states_spdig.h"
#include "globals.h"

#include "bflib_sound.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "creature_states.h"
#include "thing_list.h"
#include "thing_physics.h"
#include "creature_control.h"
#include "creature_instances.h"
#include "creature_states_combt.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_corpses.h"
#include "thing_traps.h"
#include "room_data.h"
#include "room_lair.h"
#include "room_jobs.h"
#include "room_workshop.h"
#include "room_library.h"
#include "room_graveyard.h"
#include "room_util.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "spdigger_stack.h"
#include "power_hand.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "player_instances.h"
#include "post_inc.h"
#include "creature_states_lair.h"

const unsigned char reinforce_edges[] = { 3, 0, 0, 3, 0, 1, 2, 2, 1, };

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

struct Thing *check_for_empty_trap_for_imp(struct Thing *spdigtng, long tngmodel)
{
    TRACE_THING(spdigtng);
    unsigned long k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Trap);
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->model == tngmodel) && (thing->trap.num_shots == 0) && (thing->owner == spdigtng->owner))
        {
            if (!imp_will_soon_be_arming_trap(thing)) {
                return thing;
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

long check_out_unclaimed_unconscious_bodies(struct Thing *spdigtng, long range)
{
    if (!player_has_room_of_role(spdigtng->owner, RoRoF_Prison)) {
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    struct Room* room = find_nearest_room_of_role_for_thing_with_spare_capacity(spdigtng, spdigtng->owner, RoRoF_Prison, NavRtF_Default, 1);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    unsigned long k = 0;
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (!thing_is_dragged_or_pulled(thing) && (thing->owner != spdigtng->owner)
          && thing_revealed(thing, spdigtng->owner) && creature_is_being_unconscious(thing))
        {
            if ((range < 0) || get_chessboard_distance(&thing->mappos, &spdigtng->mappos) < range)
            {
                if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                {
                    // We have a thing which we should pick - now check if the room we found is correct
                    if (room_is_invalid(room)) {
                        update_cannot_find_room_of_role_wth_spare_capacity_event(spdigtng->owner, spdigtng, RoRoF_Prison);
                        return 0;
                    }
                    if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default)) {
                        spdigtng->continue_state = CrSt_CreaturePickUpUnconsciousBody;
                        cctrl->pickup_creature_id = thing->index;
                        return 1;
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return 0;
}

/**
 * @brief dropped special digger look for unconscious creature to save
 *
 * only if drag_to_lair rule in activated
 *
 * @param spdigtng
 * @param range maximum distance to look for unconscious creatures
 * @return return 1 if special digger is succesfully assigned
 */
long check_out_unsaved_unconscious_creature(struct Thing *spdigtng, long range)
{
    if (!player_has_room_of_role(spdigtng->owner, RoRoF_LairStorage) || !game.conf.rules[spdigtng->owner].workers.drag_to_lair)
    {
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    unsigned long k = 0;
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code

        if (!thing_is_dragged_or_pulled(thing) && (thing->owner == spdigtng->owner)
          && thing_revealed(thing, spdigtng->owner) && creature_is_being_unconscious(thing))
        {
            if ((range < 0) || get_chessboard_distance(&thing->mappos, &spdigtng->mappos) < range)
            {
                if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                {
                    // only save creatures with lair
                    if (game.conf.rules[spdigtng->owner].workers.drag_to_lair == 1) {
                        struct Room * room = get_creature_lair_room(thing);
                        if (room_is_invalid(room))
                        {
                            return 0;
                        }
                    }
                    //or creature who can have have and use a lair
                    else if (game.conf.rules[spdigtng->owner].workers.drag_to_lair == 2 && !creature_can_do_healing_sleep(thing))
                    {
                        return 0;
                    }

                    if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                    {
                        spdigtng->continue_state = CrSt_CreatureSaveUnconsciousCreature;
                        cctrl->pickup_creature_id = thing->index;
                        return 1;
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return 0;
}

long check_out_unclaimed_dead_bodies(struct Thing *spdigtng, long range)
{
    if (!player_has_room_of_role(spdigtng->owner, RoRoF_DeadStorage)) {
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    struct Room* room = find_nearest_room_of_role_for_thing_with_spare_capacity(spdigtng, spdigtng->owner, RoRoF_DeadStorage, NavRtF_Default, 1);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList* slist = get_list_for_thing_class(TCls_DeadCreature);
    unsigned long k = 0;
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (corpse_ready_for_collection(thing) && thing_revealed(thing, spdigtng->owner)
         && players_creatures_tolerate_each_other(spdigtng->owner,get_slab_owner_thing_is_on(thing)))
        {
            if ((range < 0) || get_chessboard_distance(&thing->mappos, &spdigtng->mappos) < range)
            {
                if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                {
                    // We have a thing which we should pick - now check if the room we found is correct
                    if (room_is_invalid(room)) {
                        update_cannot_find_room_of_role_wth_spare_capacity_event(spdigtng->owner, spdigtng, RoRoF_DeadStorage);
                        return 0;
                    }
                    if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default)) {
                        SYNCDBG(8,"Assigned %s with %s pickup at subtile (%d,%d)",thing_model_name(spdigtng),
                            thing_model_name(thing),(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
                        spdigtng->continue_state = CrSt_CreaturePicksUpCorpse;
                        cctrl->pickup_object_id = thing->index;
                        return 1;
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return 0;
}

long check_out_unclaimed_spells(struct Thing *spdigtng, long range)
{
    if (!player_has_room_of_role(spdigtng->owner, RoRoF_PowersStorage)) {
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    struct Room* room = find_nearest_room_of_role_for_thing_with_spare_item_capacity(spdigtng, spdigtng->owner, RoRoF_PowersStorage, NavRtF_Default);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    unsigned long k = 0;
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_spellbook(thing) || thing_is_special_box(thing))
        {
            if ((thing->owner != spdigtng->owner) && !thing_is_dragged_or_pulled(thing)
              && (get_slab_owner_thing_is_on(thing) == spdigtng->owner) && thing_revealed(thing, spdigtng->owner)
                &! object_is_ignored_by_imps(thing))
            {
                if ((range < 0) || get_chessboard_distance(&thing->mappos, &spdigtng->mappos) < range)
                {
                    if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                    {
                        // We have a thing which we should pick - now check if the room we found is correct
                        if (room_is_invalid(room)) {
                            update_cannot_find_room_of_role_wth_spare_capacity_event(spdigtng->owner, spdigtng, RoRoF_PowersStorage);
                            return 0;
                        }
                        if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default)) {
                            SYNCDBG(8,"Assigned %s with %s pickup at subtile (%d,%d)",thing_model_name(spdigtng),
                                thing_model_name(thing),(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
                            if (thing_is_spellbook(thing))
                            {
                                event_create_event_or_update_nearby_existing_event(thing->mappos.x.val, thing->mappos.y.val,
                                    EvKind_SpellPickedUp, spdigtng->owner, thing->index);
                            } else
                            if (thing_is_special_box(thing))
                            {
                                event_create_event_or_update_nearby_existing_event(thing->mappos.x.val, thing->mappos.y.val,
                                    EvKind_DnSpecialFound, spdigtng->owner, thing->index);
                            }
                            spdigtng->continue_state = CrSt_CreaturePicksUpSpellObject;
                            cctrl->pickup_object_id = thing->index;
                            return 1;
                        }
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return 0;
}

long check_out_unclaimed_traps(struct Thing *spdigtng, long range)
{
    if (!player_has_room_of_role(spdigtng->owner, RoRoF_CratesStorage)) {
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    struct Room* room = find_nearest_room_of_role_for_thing_with_spare_item_capacity(spdigtng, spdigtng->owner, RoRoF_CratesStorage, NavRtF_Default);
    // We either found a room or not - but we can't generate event based on it yet, because we don't even know if there's any thing to pick
    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    unsigned long k = 0;
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing_can_be_picked_to_place_in_player_room_of_role(thing, spdigtng->owner, RoRoF_CratesStorage, TngFRPickF_AllowStoredInOwnedRoom))
        {
            if ((range < 0) || get_chessboard_distance(&thing->mappos, &spdigtng->mappos) < range)
            {
                if (!imp_will_soon_be_getting_object(spdigtng->owner, thing))
                {
                    // If there is a trap to arm, go arming
                    struct Thing *traptng;
                    if (thing_is_trap_crate(thing)) {
                        traptng = check_for_empty_trap_for_imp(spdigtng, crate_to_workshop_item_model(thing->model));
                    } else {
                        traptng = INVALID_THING;
                    }
                    if (!thing_is_invalid(traptng))
                    {
                        if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                        {
                            spdigtng->continue_state = CrSt_CreaturePicksUpTrapObject;
                            cctrl->pickup_object_id = thing->index;
                            cctrl->arming_thing_id = traptng->index;
                            return 1;
                        }
                    }
                    // No trap to arm - get the crate into workshop, if it's not already on it
                    if (thing_can_be_picked_to_place_in_player_room_of_role(thing, spdigtng->owner, RoRoF_CratesStorage, TngFRPickF_Default))
                    {
                        // We have a thing which we should pick - now check if the room we found is correct
                        if (room_is_invalid(room)) {
                            update_cannot_find_room_of_role_wth_spare_capacity_event(spdigtng->owner, spdigtng, RoRoF_CratesStorage);
                            return 0;
                        }
                        if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                        {
                            SYNCDBG(8,"Assigned %s with %s pickup at subtile (%d,%d)",thing_model_name(spdigtng),
                                thing_model_name(thing),(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
                            if (thing_is_trap_crate(thing))
                            {
                                event_create_event_or_update_nearby_existing_event(thing->mappos.x.val, thing->mappos.y.val,
                                    EvKind_TrapCrateFound, spdigtng->owner, thing->index);
                            } else
                            if (thing_is_door_crate(thing))
                            {
                                event_create_event_or_update_nearby_existing_event(thing->mappos.x.val, thing->mappos.y.val,
                                    EvKind_DoorCrateFound, spdigtng->owner, thing->index);
                            }
                            spdigtng->continue_state = CrSt_CreaturePicksUpCrateForWorkshop;
                            cctrl->pickup_object_id = thing->index;
                            return 1;
                        }
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return 0;
}

long slab_is_my_door(long plyr_idx, long slb_x, long slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    struct SlabConfigStats* slabst = get_slab_stats(slb);
    return (slabmap_owner(slb) == plyr_idx) && ((slabst->block_flags & SlbAtFlg_IsDoor) != 0);
}

long check_out_place_for_convert_behind_door(struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    for (int n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        MapSlabCoord sslb_x = slb_x + small_around[n].delta_x;
        MapSlabCoord sslb_y = slb_y + small_around[n].delta_y;
        if (slab_is_my_door(thing->owner, sslb_x, sslb_y))
        {
            // Go one slab more in that direction
            sslb_x = slb_x + 2 * small_around[n].delta_x;
            sslb_y = slb_y + 2 * small_around[n].delta_y;
            if (check_place_to_convert_excluding(thing, sslb_x, sslb_y) &&
               !imp_will_soon_be_working_at_excluding(thing, sslb_x, sslb_y))
            {
                if (setup_person_move_to_position(thing, slab_subtile_center(sslb_x), slab_subtile_center(sslb_y), 0))
                {
                    thing->continue_state = CrSt_ImpArrivesAtConvertDungeon;
                    return 1;
                }
            }
        }
    }
    return 0;
}

long check_out_unconverted_drop_place(struct Thing *thing)
{
    MapSlabCoord slb_x = subtile_slab(thing->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab(thing->mappos.y.stl.num);
    if (check_place_to_convert_excluding(thing, slb_x, slb_y))
    {
        if (imp_will_soon_be_working_at_excluding(thing, slab_subtile_center(slb_x), slab_subtile_center(slb_y)))
        {
            if (setup_person_move_to_position(thing, slab_subtile_center(slb_x), slab_subtile_center(slb_y), 0))
            {
                thing->continue_state = CrSt_ImpArrivesAtConvertDungeon;
                return 1;
            }
        }
    }
    if (check_out_unconverted_spiral(thing, 1)) {
        return 1;
    }
    if (check_out_place_for_convert_behind_door(thing, slb_x, slb_y) >= 1) {
        return 1;
    }
    return 0;
}

static TbBool check_out_undug_drop_place(struct Thing *spdigtng)
{

    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);

    MapCoord slb_x = subtile_slab(spdigtng->mappos.x.stl.num);
    MapCoord slb_y = subtile_slab(spdigtng->mappos.y.stl.num);
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    MapSubtlCoord dig_place_stl_x = 0;
    MapSubtlCoord dig_place_stl_y = 0;
    SubtlCodedCoords stl_num;
    int task_idx = 0;
    int rand = THING_RANDOM(spdigtng,3);

    for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        MapSubtlCoord check_stl_x = stl_x + STL_PER_SLB * (int)small_around[rand].delta_x;
        MapSubtlCoord check_stl_y = stl_y + STL_PER_SLB * (int)small_around[rand].delta_y;

        stl_num =  get_subtile_number(check_stl_x,check_stl_y);

        task_idx = find_dig_from_task_list(spdigtng->owner, stl_num);
        if ( task_idx != -1
            && check_place_to_dig_and_get_position(spdigtng, stl_num, &dig_place_stl_x, &dig_place_stl_y)
            && setup_person_move_to_position(spdigtng, dig_place_stl_x, dig_place_stl_y, 0) )
        {
            break;
        }
        rand = (rand + 1) % 4;
        if ( n >= SMALL_AROUND_LENGTH - 1 )
            return false;
    }

    cctrl->digger.task_idx = task_idx;
    cctrl->digger.task_stl = stl_num;
    struct Dungeon* dungeon = get_dungeon(spdigtng->owner);
    struct MapTask* mtask = get_dungeon_task_list_entry(dungeon, task_idx);
    if ( mtask->kind == SDDigTask_MineGold )
        spdigtng->continue_state = CrSt_ImpArrivesAtMineGold;
    else
        spdigtng->continue_state = CrSt_ImpArrivesAtDigDirt;
    return true;
}

long check_out_unclaimed_gold(struct Thing *spdigtng, long range)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(spdigtng);
    // If the creature holds more gold than its able
    if (spdigtng->creature.gold_carried >= crconf->gold_hold) {
        return 0;
    }
    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    unsigned long k = 0;
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing_is_object(thing) && !thing_is_picked_up(thing) && thing_revealed(thing, spdigtng->owner))
        {
            if (object_is_gold_pile(thing))
            {
                struct SlabMap* slb = get_slabmap_thing_is_on(thing);
                if ((slabmap_owner(slb) == spdigtng->owner) || (slabmap_owner(slb) == game.neutral_player_num))
                {
                    if ((range < 0) || get_chessboard_distance(&thing->mappos, &spdigtng->mappos) < range)
                    {
                        if (!imp_will_soon_be_working_at_excluding(spdigtng, thing->mappos.x.stl.num, thing->mappos.y.stl.num))
                        {
                            if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default)) {
                                spdigtng->continue_state = CrSt_ImpPicksUpGoldPile;
                                //cctrl->pickup_object_id = thing->index; -- don't do that; picking up gold destroys the object
                                return 1;
                            }
                        }
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return 0;
}

static TbBool check_out_place_for_pretty_behind_door(struct Thing *spdigting, MapCoord slb_x, MapCoord slb_y)
{
    for ( int i = 0; i < SMALL_AROUND_LENGTH; ++i )
    {
        if ( slab_is_my_door(spdigting->owner, slb_x + small_around[i].delta_x, slb_y + small_around[i].delta_y) )
        {
            MapCoord around_slb_x = slb_x + 2 * small_around[i].delta_x;
            MapCoord around_slb_y = slb_y + 2 * small_around[i].delta_y;
            MapSubtlCoord around_stl_x = slab_subtile_center(around_slb_x);
            MapSubtlCoord around_stl_y = slab_subtile_center(around_slb_y);
            if ( check_place_to_pretty_excluding(spdigting, around_slb_x, around_slb_y)
                && !imp_will_soon_be_working_at_excluding(spdigting, around_slb_x, around_slb_y)
                && setup_person_move_to_position(spdigting, around_stl_x, around_stl_y, 0) )
            {
                spdigting->continue_state = CrSt_ImpArrivesAtImproveDungeon;
                return true;
            }
        }
    }
    return false;
}

TbBool check_out_unprettied_drop_place(struct Thing *thing)
{
    MapCoord slb_x = subtile_slab(thing->mappos.x.stl.num);
    MapCoord slb_y = subtile_slab(thing->mappos.y.stl.num);
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);

    if (check_place_to_pretty_excluding(thing, slb_x, slb_y)
        && !imp_will_soon_be_working_at_excluding(thing, stl_x, stl_y)
        && setup_person_move_to_position(thing, stl_x, stl_y, 0))
    {
        thing->continue_state = CrSt_ImpArrivesAtImproveDungeon;
        return true;
    }

    if (check_out_unprettied_spiral(thing, 1))
        return true;

    return check_out_place_for_pretty_behind_door(thing, slb_x, slb_y);
}

long check_out_object_for_trap(struct Thing *spdigtng, struct Thing *traptng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    // We're supposed to be in our own workshop; fail if we're not
    struct Room* room = get_room_thing_is_on(spdigtng);
    if (room_is_invalid(room)) {
        return 0;
    }
    if (!room_role_matches(room->kind, RoRoF_CratesStorage) || (room->owner != spdigtng->owner)) {
        return 0;
    }
    long find_model = trap_crate_object_model(traptng->model);
    long find_owner = spdigtng->owner;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    unsigned long k = 0;
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if (thing->model == find_model)
        {
            struct SlabMap* slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            if ( (slabmap_owner(slb) == find_owner) && ((thing->state_flags & TF1_IsDragged1) == 0) )
            {
                if (!imp_will_soon_be_getting_object(find_owner, thing))
                {
                    if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                    {
                        spdigtng->continue_state = CrSt_CreaturePicksUpTrapObject;
                        cctrl->pickup_object_id = thing->index;
                        cctrl->arming_thing_id = traptng->index;
                        return 1;
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return 0;
}

long check_out_empty_traps(struct Thing *spdigtng, long range)
{
    unsigned long k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Trap);
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->trap.num_shots == 0) && (thing->owner == spdigtng->owner))
        {
            if ( (range < 0) || (get_chessboard_distance(&thing->mappos, &spdigtng->mappos) < range) )
            {

                if ( !imp_will_soon_be_arming_trap(thing) && check_out_object_for_trap(spdigtng, thing) ) {
                    return 1;
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return 0;
}

long check_out_unreinforced_drop_place(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    MapSubtlCoord digger_stl_x;
    MapSubtlCoord digger_stl_y;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    long stl_num;
    MapSubtlCoord dest_stl_x;
    MapSubtlCoord dest_stl_y;
    long i;
    long n;
    digger_stl_x = thing->mappos.x.stl.num;
    digger_stl_y = thing->mappos.y.stl.num;
    cctrl = creature_control_get_from_thing(thing);
    n = reinforce_edges[STL_PER_SLB * (digger_stl_y % STL_PER_SLB) + (digger_stl_x % STL_PER_SLB)];
    for (i=0; i < SMALL_AROUND_LENGTH; i++)
    {
        slb_x = subtile_slab(digger_stl_x) + (long)small_around[n].delta_x;
        slb_y = subtile_slab(digger_stl_y) + (long)small_around[n].delta_y;
        if ( check_place_to_reinforce(thing, slb_x, slb_y) > 0 )
        {
            stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
            if ( check_out_uncrowded_reinforce_position(thing, stl_num, &dest_stl_x, &dest_stl_y) )
            {
                if ( setup_person_move_to_position(thing, dest_stl_x, dest_stl_y, NavRtF_Default) )
                {

                    thing->continue_state = CrSt_ImpArrivesAtReinforce;
                    cctrl->digger.working_stl = stl_num;
                    cctrl->digger.consecutive_reinforcements = 0;
                    SYNCDBG(8,"Assigned reinforce at (%d,%d) to %s index %d",(int)dest_stl_x,(int)dest_stl_y,thing_model_name(thing),(int)thing->index);
                    return 1;
                }
            }
        }
        n = (n + 1) % SMALL_AROUND_LENGTH;
    }
    SYNCDBG(18,"No job for %s index %d",thing_model_name(thing),(int)thing->index);
    return 0;
}

/**
 * Checks if there are crates in room the creature is on, which could be used to re-arm one of players traps.
 * If there are, setups given digger to do the task of re-arming trap.
 * @param spdigtng
 */
TbBool check_out_crates_to_arm_trap_in_room(struct Thing *spdigtng)
{
    struct Room* room = get_room_thing_is_on(spdigtng);
    if (room_is_invalid(room)) {
        return false;
    }
    if (!room_role_matches(room->kind, RoRoF_CratesStorage) || (room->owner != spdigtng->owner)) {
        return false;
    }

    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    unsigned long k = 0;
    long i = slist->index;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        // Per-thing code
        if ( thing_is_trap_crate(thing) )
        {
          if ( ((thing->state_flags & TF1_IsDragged1) == 0) && (get_room_thing_is_on(thing) == room) )
          {
              struct Thing* traptng = check_for_empty_trap_for_imp(spdigtng, crate_thing_to_workshop_item_model(thing));
              if (!thing_is_invalid(traptng) && !imp_will_soon_be_getting_object(spdigtng->owner, thing))
              {
                  if (setup_person_move_to_coord(spdigtng, &thing->mappos, NavRtF_Default))
                  {
                      struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
                      spdigtng->continue_state = CrSt_CreaturePicksUpTrapObject;
                      cctrl->pickup_object_id = thing->index;
                      cctrl->arming_thing_id = traptng->index;
                      return true;
                  }
              }
          }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return false;
}

/**
 * Checks for a special digger task for a creature in its vicinity.
 * @param spdigtng
 * @note originally was check_out_available_imp_drop_tasks()
 */
long check_out_available_spdigger_drop_tasks(struct Thing *spdigtng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    SYNCDBG(9,"Starting for %s index %d",thing_model_name(spdigtng),(int)spdigtng->index);
    TRACE_THING(spdigtng);

    if ( check_out_unclaimed_unconscious_bodies(spdigtng, 3 * COORD_PER_STL) )
    {
        cctrl->digger.task_repeats = 0;
        return 1;
    }
    if ( check_out_unsaved_unconscious_creature(spdigtng, 3 * COORD_PER_STL) )
    {
        cctrl->digger.task_repeats = 0;
        return 1;
    }
    if ( check_out_unclaimed_dead_bodies(spdigtng, 3 * COORD_PER_STL) )
    {
        cctrl->digger.task_repeats = 0;
        return 1;
    }
    if ( check_out_unclaimed_spells(spdigtng, 3 * COORD_PER_STL) )
    {
        cctrl->digger.task_repeats = 0;
        return 1;
    }
    if ( check_out_unclaimed_traps(spdigtng, 3 * COORD_PER_STL) )
    {
        cctrl->digger.task_repeats = 0;
        return 1;
    }
    if ( check_out_empty_traps(spdigtng, 3 * COORD_PER_STL) )
    {
        cctrl->digger.task_repeats = 0;
        return 1;
    }
    if ( check_out_undug_drop_place(spdigtng) )
    {
        cctrl->digger.task_repeats = 0;
        cctrl->digger.last_did_job = SDLstJob_DigOrMine;
        return 1;
    }
    if ( check_out_unconverted_drop_place(spdigtng) )
    {
        cctrl->digger.task_repeats = 0;
        cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
        return 1;
    }
    if ( check_out_unprettied_drop_place(spdigtng) )
    {
        cctrl->digger.task_repeats = 0;
        cctrl->digger.last_did_job = SDLstJob_ConvImprDungeon;
        return 1;
    }
    if ( check_out_unclaimed_gold(spdigtng, 3 * COORD_PER_STL) )
    {
        cctrl->digger.task_repeats = 0;
        return 1;
    }
    if ( check_out_unreinforced_drop_place(spdigtng) )
    {
        cctrl->digger.task_repeats = 0;
        cctrl->digger.last_did_job = SDLstJob_ReinforceWallAssigned;
        return 1;
    }
    if ( check_out_crates_to_arm_trap_in_room(spdigtng) )
    {
        cctrl->digger.task_repeats = 0;
        return 1;
    }
    struct Room* room = get_room_thing_is_on(spdigtng);
    if (!room_is_invalid(room))
    {
        if (room->owner == spdigtng->owner)
        {
            if (room_role_matches(room->kind,RoRoF_GoldStorage))
            {
                if (check_out_imp_has_money_for_treasure_room(spdigtng))
                {
                    return 1;
                }
            }
        }
    }
    cctrl->digger.task_repeats = 0;
    cctrl->digger.last_did_job = SDLstJob_None;
    return 0;
}

short imp_arrives_at_convert_dungeon(struct Thing *thing)
{
    TRACE_THING(thing);
    if (check_place_to_convert_excluding(thing,
        subtile_slab(thing->mappos.x.stl.num),
        subtile_slab(thing->mappos.y.stl.num)) )
    {
      internal_set_thing_state(thing, CrSt_ImpConvertsDungeon);
    } else
    {
      internal_set_thing_state(thing, CrSt_ImpLastDidJob);
    }
    return 1;
}

TbBool move_imp_to_uncrowded_dig_mine_access_point(struct Thing *spdigtng, SubtlCodedCoords stl_num)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    TRACE_THING(spdigtng);
    if (!check_place_to_dig_and_get_position(spdigtng, stl_num, &stl_x, &stl_y))
        return false;
    if (!setup_person_move_to_position(spdigtng, stl_x, stl_y, NavRtF_Default))
        return false;
    spdigtng->continue_state = CrSt_ImpArrivesAtDigDirt;
    return true;
}

short imp_arrives_at_dig_or_mine(struct Thing *spdigtng)
{
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    if (imp_already_digging_at_excluding(spdigtng, spdigtng->mappos.x.stl.num, spdigtng->mappos.y.stl.num))
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);

        if (!move_imp_to_uncrowded_dig_mine_access_point(spdigtng, cctrl->digger.task_stl))
        {
            internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
            return 1;
        }
    } else
    {
        if (spdigtng->active_state == CrSt_ImpArrivesAtDigDirt)
            internal_set_thing_state(spdigtng, CrSt_ImpDigsDirt);
        else
            internal_set_thing_state(spdigtng, CrSt_ImpMinesGold);
    }
    return 1;
}

short imp_arrives_at_improve_dungeon(struct Thing *spdigtng)
{
    TRACE_THING(spdigtng);
    if ( check_place_to_pretty_excluding(spdigtng,
        subtile_slab(spdigtng->mappos.x.stl.num),
        subtile_slab(spdigtng->mappos.y.stl.num)) )
    {
        internal_set_thing_state(spdigtng, CrSt_ImpImprovesDungeon);
    } else
    {
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
    }
    return 1;
}



short imp_arrives_at_reinforce(struct Thing *spdigtng)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);

    if ( imp_already_reinforcing_at_excluding(spdigtng,spdigtng->mappos.x.stl.num,spdigtng->mappos.y.stl.num))
    {
        if ( !check_out_uncrowded_reinforce_position(spdigtng, cctrl->digger.working_stl, &stl_x, &stl_y)
            || !setup_person_move_to_position(spdigtng, stl_x, stl_y, 0) )
        {
            internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
            return 1;
        }
        else
        {
            spdigtng->continue_state = CrSt_ImpArrivesAtReinforce;
        }
    }
    else
    {
        internal_set_thing_state(spdigtng, CrSt_ImpReinforces);
    }
    return 1;
}


short imp_birth(struct Thing *thing)
{
    TRACE_THING(thing);
    if ( thing_touching_floor(thing) || (((thing->movement_flags & TMvF_Flying) != 0) && thing_touching_flight_altitude(thing)) )
    {
        // If the creature has flight ability, make sure it returns to flying state
        restore_creature_flight_flag(thing);
        if (thing_is_creature_digger(thing))
        {
            if (!check_out_available_spdigger_drop_tasks(thing)) {
                set_start_state(thing);
            }
        }
        else
        {
            set_start_state(thing);
        }
        return 1;
    }
    long i = game.play_gameturn - thing->creation_turn;
    if ((i % 2) == 0) {
      create_effect_element(&thing->mappos, birth_effect_element[get_player_color_idx(thing->owner)], thing->owner);
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    thing->movement_flags &= ~TMvF_Flying;
    creature_turn_to_face_angle(thing, i * (long)crconf->max_turning_speed);
    return 0;
}

long digger_work_experience(struct Thing* spdigtng)
{
    if (creature_can_gain_experience(spdigtng))
    {
        return game.conf.rules[spdigtng->owner].workers.digger_work_experience;
    }
    return 0;
}

short imp_converts_dungeon(struct Thing *spdigtng)
{
    TRACE_THING(spdigtng);
    MapSubtlCoord stl_x = spdigtng->mappos.x.stl.num;
    MapSubtlCoord stl_y = spdigtng->mappos.y.stl.num;
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    if ( (stl_x - (MapSubtlDelta)cctrl->moveto_pos.x.stl.num >= 1) || (stl_y - (MapSubtlDelta)cctrl->moveto_pos.y.stl.num >= 1) )
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 0;
    }
    if ( check_place_to_convert_excluding(spdigtng, slb_x, slb_y) )
    {
      if (cctrl->instance_id == CrInst_NULL)
      {
          CrInstance inst_idx = get_self_spell_casting(spdigtng);
          if (inst_idx > CrInst_NULL)
          {
              set_creature_instance(spdigtng, inst_idx, 0, 0);
              return 1;
          }
          else
          {
              set_creature_instance(spdigtng, CrInst_DESTROY_AREA, 0, 0);
              struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
              struct SlabConfigStats* slabst = get_slab_stats(slb);
              // If the area we're converting is an enemy room, issue event to that player
              if (slabst->category == SlbAtCtg_RoomInterior)
              {
                  struct Room* room = room_get(slb->room_index);
                  if (!room_is_invalid(room))
                  {
                      MapCoord coord_x = subtile_coord_center(room->central_stl_x);
                      MapCoord coord_y = subtile_coord_center(room->central_stl_y);
                      event_create_event_or_update_nearby_existing_event(coord_x, coord_y,
                          EvKind_RoomUnderAttack, room->owner, 0);
                      if (is_my_player_number(room->owner))
                      {
                          output_message(SMsg_EnemyDestroyRooms, MESSAGE_DURATION_FIGHT);
                      }
                }
              }
          }
      }
      if (game.conf.rules[spdigtng->owner].workers.digger_work_experience != 0)
      {
          cctrl->exp_points += (digger_work_experience(spdigtng) / 6);
          check_experience_upgrade(spdigtng);
      }
      return 1;
    }
    if ( !check_place_to_pretty_excluding(spdigtng, slb_x, slb_y) )
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 0;
    }
    if (cctrl->instance_id != CrInst_PRETTY_PATH) {
        set_creature_instance(spdigtng, CrInst_PRETTY_PATH, 0, 0);
    }
    return 1;
}

TbBool too_much_gold_lies_around_thing(const struct Thing *thing)
{
    TRACE_THING(thing);
    return gold_pile_with_maximum_at_xy(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

short imp_digs_mines(struct Thing *spdigtng)
{
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    if (game.conf.rules[spdigtng->owner].workers.digger_work_experience != 0)
    {
        cctrl->exp_points += digger_work_experience(spdigtng);
        check_experience_upgrade(spdigtng);
    }
    struct MapTask* mtask = get_task_list_entry(spdigtng->owner, cctrl->digger.task_idx);
    MapSubtlCoord stl_x = stl_num_decode_x(cctrl->digger.task_stl);
    MapSubtlCoord stl_y = stl_num_decode_y(cctrl->digger.task_stl);
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);

    // Check if we've arrived at the destination
    MapSubtlDelta delta_x = abs(spdigtng->mappos.x.stl.num - (MapSubtlDelta)cctrl->moveto_pos.x.stl.num);
    MapSubtlDelta delta_y = abs(spdigtng->mappos.y.stl.num - (MapSubtlDelta)cctrl->moveto_pos.y.stl.num);
    if ((mtask->coords != cctrl->digger.task_stl) || (delta_x > 0) || (delta_y > 0))
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }
    // If gems are marked for digging, but there is too much gold laying around, then don't dig
    if (slab_kind_is_indestructible(slb->kind) && too_much_gold_lies_around_thing(spdigtng))
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }
    // Turn to the correct direction to do the task
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    if (creature_turn_to_face(spdigtng, &pos) > 0)
    {
      return 1;
    }

    if (mtask->kind == SDDigTask_None)
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }

    if (cctrl->instance_id == CrInst_NULL)
    {
        set_creature_instance(spdigtng, CrInst_DIG, 0, 0);
    }

    if (mtask->kind == SDDigTask_MineGold)
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(spdigtng);
        // If the creature holds more gold than its able
        if (spdigtng->creature.gold_carried > crconf->gold_hold)
        {
            if (game.play_gameturn - cctrl->tasks_check_turn > 128)
            {
                if (check_out_imp_has_money_for_treasure_room(spdigtng)) {
                    // Note - do not increase cctrl->digger.task_repeats here; the task is to mine, not to return gold.
                    return 1;
                }
                cctrl->tasks_check_turn = game.play_gameturn;
            }
            drop_gold_pile(spdigtng->creature.gold_carried - crconf->gold_hold, &spdigtng->mappos);
            spdigtng->creature.gold_carried = crconf->gold_hold;
        }
    }
    return 1;
}

short imp_doing_nothing(struct Thing *spdigtng)
{
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(spdigtng),(int)spdigtng->index);
    TRACE_THING(spdigtng);
    if (!thing_is_creature_digger(spdigtng))
    {
        ERRORLOG("Non digger thing %ld, %s, owner %ld - reset",(long)spdigtng->index,thing_model_name(spdigtng),(long)spdigtng->owner);
        set_start_state(spdigtng);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }

    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    if (game.play_gameturn-cctrl->idle.start_gameturn <= 1) {
        return 1;
    }
    if (check_out_imp_last_did(spdigtng)) {
        return 1;
    }
    cctrl->healing_sleep_check_turn = game.play_gameturn; //imp is now free to check if he needs healing, since there is no assigned job to do.
    if (check_out_available_imp_tasks(spdigtng)) {
        return 1;
    }
    if (check_out_imp_tokes(spdigtng)) {
        return 1;
    }
    if (creature_choose_random_destination_on_valid_adjacent_slab(spdigtng))
    {
        spdigtng->continue_state = CrSt_ImpDoingNothing;
        return 1;
    }
    struct Dungeon* dungeon = get_dungeon(spdigtng->owner);
    if (!dungeon_invalid(dungeon))
        dungeon->lvstats.promises_broken++;
    return 1;
}

short imp_drops_gold(struct Thing *spdigtng)
{
    if (spdigtng->creature.gold_carried == 0)
    {
        set_start_state(spdigtng);
        return 1;
    }
    struct Room* room = get_room_thing_is_on(spdigtng);
    if (room_is_invalid(room) || (room->owner != spdigtng->owner) || (!room_role_matches(room->kind,RoRoF_GoldStorage)))
    {
        WARNLOG("Tried to drop gold in %s of player %d, but room %s owned by played %d is no longer valid to do that",
            room_role_code_name(RoRoF_GoldStorage),(int)spdigtng->owner,room_code_name(room->kind),(int)room->owner);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }
    MapSubtlCoord center_stl_x = slab_subtile_center(subtile_slab(spdigtng->mappos.x.stl.num));
    MapSubtlCoord center_stl_y = slab_subtile_center(subtile_slab(spdigtng->mappos.y.stl.num));
    struct Room* curoom = subtile_room_get(spdigtng->mappos.x.stl.num, spdigtng->mappos.y.stl.num);
    if (!room_exists(curoom) || (curoom->index != room->index))
    {
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 1;
    }
    unsigned char state = ((spdigtng->alloc_flags & TAlF_IsControlled) == 0) ? CrSt_ImpLastDidJob : CrSt_Unused;
    long gold_added = 0;
    TbBool gold_created = false;
    struct Thing* gldtng = find_gold_hoard_at(center_stl_x, center_stl_y);
    if (!thing_is_invalid(gldtng))
    {
        gold_added = add_gold_to_hoarde(gldtng, room, spdigtng->creature.gold_carried);
        spdigtng->creature.gold_carried -= gold_added;
    } else
    {
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(center_stl_x);
        pos.y.val = subtile_coord_center(center_stl_y);
        pos.z.val = get_floor_height_at(&pos);
        gldtng = create_gold_hoarde(room, &pos, spdigtng->creature.gold_carried);
        if (!thing_is_invalid(gldtng))
        {
            spdigtng->creature.gold_carried -= gldtng->valuable.gold_stored;
            gold_created = true;
        }
    }
    if ( (gold_added > 0) || (gold_created) )
    {
        thing_play_sample(spdigtng, SOUND_RANDOM(3) + 32, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        if (game.conf.rules[spdigtng->owner].workers.digger_work_experience != 0)
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
            cctrl->exp_points += digger_work_experience(spdigtng);
            check_experience_upgrade(spdigtng);
        }
    }
    else
    {
        if (is_thing_directly_controlled_by_player(spdigtng, my_player_number))
        {
            play_non_3d_sample(119);
            internal_set_thing_state(spdigtng, state);
            return 1;
        }
    }
    if ((spdigtng->creature.gold_carried > 0) && (room->used_capacity < room->total_capacity))
    {
        if ((spdigtng->alloc_flags & TAlF_IsControlled) == 0)
        {
            if (setup_head_for_empty_treasure_space(spdigtng, room)) {
                spdigtng->continue_state = CrSt_ImpDropsGold;
                return 1;
            }
        }
    }
    internal_set_thing_state(spdigtng, state);
    return 1;
}

short imp_improves_dungeon(struct Thing *spdigtng)
{
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(spdigtng);
    if (game.conf.rules[spdigtng->owner].workers.digger_work_experience != 0)
    {
        cctrl->exp_points += (digger_work_experience(spdigtng) / 6);
        check_experience_upgrade(spdigtng);
    }
    // Check if we've arrived at the destination
    MapSubtlDelta delta_x = abs(spdigtng->mappos.x.stl.num - (MapSubtlDelta)cctrl->moveto_pos.x.stl.num);
    MapSubtlDelta delta_y = abs(spdigtng->mappos.y.stl.num - (MapSubtlDelta)cctrl->moveto_pos.y.stl.num);
    if ( (delta_x > 0) || (delta_y > 0) )
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 0;
    }
    long slb_x = subtile_slab(spdigtng->mappos.x.stl.num);
    long slb_y = subtile_slab(spdigtng->mappos.y.stl.num);
    if (!check_place_to_pretty_excluding(spdigtng, slb_x, slb_y))
    {
        clear_creature_instance(spdigtng);
        internal_set_thing_state(spdigtng, CrSt_ImpLastDidJob);
        return 0;
    }
    if (cctrl->instance_id == CrInst_NULL) {
        set_creature_instance(spdigtng, CrInst_PRETTY_PATH, 0, 0);
    }
    return 1;
}

short imp_last_did_job(struct Thing *creatng)
{
    if (!check_out_imp_last_did(creatng))
    {
        set_start_state(creatng);
        return 0;
    }
    return 1;
}

GoldAmount take_from_gold_pile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long limit)
{
    GoldAmount total_taken = 0;
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if ((thing->class_id == TCls_Object) && object_is_gold_pile(thing))
        {
            GoldAmount pot_stored = thing->valuable.gold_stored;
            if ((limit - total_taken >= pot_stored) || (limit == -1))
            {
                total_taken += pot_stored;
                delete_thing_structure(thing, 0);
            } else
            {
                thing->valuable.gold_stored += total_taken - limit;
                add_gold_to_pile(thing, 0);
                return limit;
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return total_taken;
}

short imp_picks_up_gold_pile(struct Thing *spdigtng)
{
    SYNCDBG(19,"Starting");
    TRACE_THING(spdigtng);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(spdigtng);
    unsigned char state = CrSt_ImpLastDidJob;
    if (crconf->gold_hold > spdigtng->creature.gold_carried)
    {
        MapSubtlCoord stl_x, stl_y;
        if (is_thing_directly_controlled(spdigtng))
        {
            struct CreatureControl *cctrl = creature_control_get_from_thing(spdigtng);
            struct Thing *goldtng = thing_get(cctrl->pickup_object_id);
            stl_x = goldtng->mappos.x.stl.num;
            stl_y = goldtng->mappos.y.stl.num;
            state = CrSt_Unused;
        }
        else
        {
            stl_x = spdigtng->mappos.x.stl.num;
            stl_y = spdigtng->mappos.y.stl.num;
        }
        long gold_taken = take_from_gold_pile(stl_x, stl_y, crconf->gold_hold - spdigtng->creature.gold_carried);
        spdigtng->creature.gold_carried += gold_taken;
        if (gold_taken > 0)
        {
            thing_play_sample(spdigtng, 32, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }
    internal_set_thing_state(spdigtng, state);
    return 0;
}

short imp_reinforces(struct Thing *thing)
{
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    MapSubtlCoord stl_x = stl_num_decode_x(cctrl->digger.working_stl);
    MapSubtlCoord stl_y = stl_num_decode_y(cctrl->digger.working_stl);
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = subtile_coord(1,0);
    MapSubtlDelta dist_x = abs(thing->mappos.x.stl.num - (MapSubtlDelta)cctrl->moveto_pos.x.stl.num);
    MapSubtlDelta dist_y = abs(thing->mappos.y.stl.num - (MapSubtlDelta)cctrl->moveto_pos.y.stl.num);
    if (dist_x + dist_y >= 1)
    {
        clear_creature_instance(thing);
        internal_set_thing_state(thing, CrSt_ImpLastDidJob);
        return 0;
    }
    long check_ret = check_place_to_reinforce(thing, subtile_slab(stl_x), subtile_slab(stl_y));
    if (check_ret <= 0)
    {
        if (check_ret < 0)
        {
            cctrl->digger.task_repeats = 0;
            cctrl->digger.last_did_job = SDLstJob_DigOrMine;
            cctrl->digger.task_stl = get_subtile_number_at_slab_center(subtile_slab(stl_x), subtile_slab(stl_y));
        }
        clear_creature_instance(thing);
        internal_set_thing_state(thing, CrSt_ImpLastDidJob);
        return 0;
    }
    if (game.conf.rules[thing->owner].workers.digger_work_experience != 0)
    {
        cctrl->exp_points += digger_work_experience(thing);
        check_experience_upgrade(thing);
    }
    if (creature_turn_to_face(thing, &pos) > 0) {
        return 1;
    }
    if (cctrl->instance_id == CrInst_NULL) {
        set_creature_instance(thing, CrInst_REINFORCE, 0, 0);
    }
    return 1;
}

short creature_going_to_safety_for_toking(struct Thing *thing)
{
    struct Coord3d locpos = {};
    if (!get_flee_position(thing, &locpos))
    {
        ERRORLOG("Couldn't get a flee position for %s index %d",thing_model_name(thing),(int)thing->index);
        internal_set_thing_state(thing, CrSt_ImpToking);
        return 1;
    }
    if (setup_person_move_close_to_position(thing, locpos.x.stl.num, locpos.y.stl.num, NavRtF_Default))
    {
        thing->continue_state = CrSt_ImpToking;
        return 1;
    }
    internal_set_thing_state(thing, CrSt_ImpToking);
    return 1;
}

short imp_toking(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (!creature_would_benefit_from_healing(creatng))
    {
        internal_set_thing_state(creatng, creatng->continue_state);
        return 0;
    }
    if (cctrl->instance_id == CrInst_NULL)
    {
        if (THING_RANDOM(creatng, 8))
        {
            set_creature_instance(creatng, CrInst_RELAXING, 0, 0);
        }
        else
        {
            set_creature_instance(creatng, CrInst_TOKING, 0, 0);
        }
    }
    if ((cctrl->instance_id == CrInst_TOKING) && (cctrl->inst_turn == cctrl->inst_action_turns))
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
        if (crconf->toking_recovery != 0)
        {
            HitPoints recover = compute_creature_max_health(crconf->toking_recovery, cctrl->exp_level);
            apply_health_to_thing_and_display_health(creatng, recover);
        }
    }
    return 1;
}

/**
 * For a creature dragging a thing, this function searches for another room
 * where the thing could be placed.
 * @param thing The creature dragging another thing.
 * @return Gives true if the setup to the new room has succeeded.
 */
TbBool creature_drop_thing_to_another_room(struct Thing *thing, struct Room *skiproom, RoomRole rrole)
{
    struct Coord3d pos;
    TRACE_THING(thing);
    struct Room* ownroom = find_nearest_room_of_role_for_thing_with_spare_capacity(thing, thing->owner, rrole, NavRtF_Default, 1);
    if ( room_is_invalid(ownroom) || (ownroom->index == skiproom->index) )
    {
        WARNLOG("Couldn't find a new %s for object dragged by %s owned by %d",room_role_code_name(rrole),thing_model_name(thing),(int)thing->owner);
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room_avoiding_object(thing, ownroom, &pos) )
    {
        WARNLOG("Couldn't find a new destination in %s for object dragged by %s owned by %d",room_role_code_name(rrole),thing_model_name(thing),(int)thing->owner);
        return false;
    }
    if (!setup_person_move_to_coord(thing, &pos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot move %s to %s at subtile (%d,%d)",thing_model_name(thing),room_role_code_name(rrole),(int)pos.x.stl.num,(int)pos.y.stl.num);
        return false;
    }
    return true;
}

TbBool set_creature_being_dragged_by(struct Thing *dragtng, struct Thing *thing)
{
    TRACE_THING(dragtng);
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureControl* dragctrl = creature_control_get_from_thing(dragtng);
    // Check if we're already dragging
    struct Thing* picktng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(picktng);
    if (thing_exists(picktng)) {
        ERRORLOG("Thing is already dragging something");
        return false;
    }
    picktng = thing_get(dragctrl->dragtng_idx);
    TRACE_THING(picktng);
    if (thing_exists(picktng)) {
        if (picktng->index != thing->index)
        {
            ERRORLOG("Thing is already dragged by something");
            return false;
        }
    }
    // Set the new dragging
    cctrl->dragtng_idx = dragtng->index;
    dragtng->state_flags |= TF1_IsDragged1;
    dragctrl->dragtng_idx = thing->index;
    return false;
}

/**
 * Returns if a creature is either being dragged or is dragging something.
 * @param thing The creature to be checked.
 * @return True if the creature has something to do with dragging, false otherwise.
 * @see thing_is_dragged_or_pulled() Checks any thing if it's being dragged.
 */
TbBool creature_is_dragging_or_being_dragged(const struct Thing *thing)
{
    const struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    return (cctrl->dragtng_idx != 0);
}

short creature_pick_up_unconscious_body(struct Thing *thing)
{
    struct Coord3d pos;
    SYNCDBG(9,"Starting");
    TRACE_THING(thing);
    // Check if the player has means to do such kind of action
     if (!player_has_room_of_role(thing->owner, RoRoF_Prison) || !player_creature_tends_to(thing->owner, CrTend_Imprison))
     {
         SYNCDBG(19,"Player %d has no %s or has imprison tendency off",(int)thing->owner,room_role_code_name(RoRoF_Prison));
         set_start_state(thing);
         return 0;
     }
     struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
     struct Thing* picktng = thing_get(cctrl->pickup_creature_id);
     TRACE_THING(picktng);
     if (!thing_exists(picktng) || (picktng->active_state != CrSt_CreatureUnconscious) || thing_is_dragged_or_pulled(picktng) || (get_chessboard_distance(&thing->mappos, &picktng->mappos) >= 512))
     {
         SYNCDBG(8, "The %s index %d to be picked up isn't in correct place or state", thing_model_name(picktng), (int)picktng->index);
         set_start_state(thing);
         return 0;
    }
    struct Room* dstroom = find_nearest_room_of_role_for_thing_with_spare_capacity(thing, thing->owner, RoRoF_Prison, NavRtF_Default, 1);
    if (room_is_invalid(dstroom))
    {
        // Check why the treasure room search failed and inform the player
        update_cannot_find_room_of_role_wth_spare_capacity_event(thing->owner, thing, RoRoF_Prison);
        set_start_state(thing);
        return 0;
    }
    if (!find_random_valid_position_for_thing_in_room(thing, dstroom, &pos))
    {
        WARNLOG("Player %d can't pick %s - no position within %s to store it",(int)thing->owner,thing_model_name(picktng),room_role_code_name(RoRoF_Prison));
        set_start_state(thing);
        return 0;
    }
    if (!setup_person_move_backwards_to_coord(thing, &pos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot drag %s to (%d,%d)",thing_model_name(picktng),(int)pos.x.stl.num,(int)pos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    set_creature_being_dragged_by(picktng, thing);
    thing->continue_state = CrSt_CreatureDropBodyInPrison;
    return 1;
}

/**
 * @brief special digger will drag unconscious creature to their lair
 *
 * only if drag_to_lair rule in activated
 *
 * @param thing creature that is being dragged
 * @return returns 1 if creature successfully arrived at its lair
 */
short creature_save_unconscious_creature(struct Thing *thing)
{
    SYNCDBG(9,"Starting");
    TRACE_THING(thing);
    // Check if the player has means to do such kind of action
     if (!player_has_room_of_role(thing->owner, RoRoF_LairStorage))
     {
         SYNCDBG(19,"Player %d has no %s or has flee tendency off",(int)thing->owner,room_role_code_name(RoRoF_LairStorage));
         set_start_state(thing);
         return 0;
     }
     struct Coord3d pos;
     struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
     struct Thing* picktng = thing_get(cctrl->pickup_creature_id);
     struct CreatureControl* cctrlpicktng = creature_control_get_from_thing(picktng);
     struct Thing* lairtng = thing_get(cctrlpicktng->lairtng_idx);
     TRACE_THING(picktng);

     if (!thing_exists(picktng) || (picktng->active_state != CrSt_CreatureUnconscious) || thing_is_dragged_or_pulled(picktng) || (get_chessboard_distance(&thing->mappos, &picktng->mappos) >= 512))
     {
         SYNCDBG(8, "The %s index %d to be picked up isn't in correct place or state", thing_model_name(picktng), (int)picktng->index);
         set_start_state(thing);
         return 0;
    }

    struct Room* dstroom = get_creature_lair_room(picktng);
    //if creature doesn't have a lair but need one
    if (room_is_invalid(dstroom) && creature_can_do_healing_sleep(picktng) && game.conf.rules[thing->owner].workers.drag_to_lair == 2)
    {
        dstroom = get_best_new_lair_for_creature(picktng);
        //send special digger directly at the right place for the lair-totem of the creature
        if (!setup_head_for_random_unused_lair_subtile(thing, dstroom))
        {
            set_start_state(thing);
            return 0;
        }
        // we still need the position to let him walk backwards
        pos.x.val = cctrl->moveto_pos.x.val;
        pos.y.val = cctrl->moveto_pos.y.val;
        pos.z.val = cctrl->moveto_pos.z.val;
    }
    //creature have already a lair-room
    else{
        if (room_is_invalid(dstroom))
        {
            set_start_state(thing);
            return 0;
        }
        if (!find_lair_totem_at(lairtng->mappos.x.stl.num,lairtng->mappos.y.stl.num))
        {
            WARNLOG("Player %d can't pick %s - no position within %s to store it",(int)thing->owner,thing_model_name(picktng),room_role_code_name(RoRoF_Prison));
            set_start_state(thing);
            return 0;
        }
        if (!creature_can_do_healing_sleep(picktng))
        {
            set_start_state(thing);
            return 0;
        }
        pos = lairtng->mappos;
    }

    if (!setup_person_move_backwards_to_coord(thing, &pos, NavRtF_Default))
        {
            SYNCDBG(8,"Cannot drag %s to (%u,%u)",thing_model_name(picktng), pos.x.stl.num, pos.y.stl.num);
            set_start_state(thing);
            return 0;
        }
    set_creature_being_dragged_by(picktng, thing);
    thing->continue_state = CrSt_CreatureDropBodyInLair;
    return 1;
}

short creature_picks_up_corpse(struct Thing *creatng)
{
    struct Coord3d pos;
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* picktng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(picktng);
    if (!thing_exists(picktng) || (flag_is_set(picktng->alloc_flags,TAlF_IsDragged))
        || (get_chessboard_distance(&creatng->mappos, &picktng->mappos) >= subtile_coord(2, 0)))
    {
        set_start_state(creatng);
        return 0;
    }
    struct Room* dstroom = find_nearest_room_of_role_for_thing_with_spare_capacity(creatng, creatng->owner, RoRoF_DeadStorage, NavRtF_Default, 1);
    if (room_is_invalid(dstroom))
    {
        // Check why the treasure room search failed and inform the player
        update_cannot_find_room_of_role_wth_spare_capacity_event(creatng->owner, creatng, RoRoF_Prison);
        set_start_state(creatng);
        return 0;
    }
    if (!find_random_valid_position_for_thing_in_room_avoiding_object(creatng, dstroom, &pos) )
    {
        WARNLOG("Player %d can't pick %s - no position within %s to store it",(int)creatng->owner,thing_model_name(picktng),room_role_code_name(RoRoF_DeadStorage));
        set_start_state(creatng);
        return 0;
    }
    creature_drag_object(creatng, picktng);
    if (!setup_person_move_backwards_to_coord(creatng, &pos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot move to (%d,%d)",(int)pos.x.stl.num, (int)pos.y.stl.num);
        set_start_state(creatng);
        return 0;
    }
    creatng->continue_state = CrSt_CreatureDropsCorpseInGraveyard;
    return 1;
}

/**
 * Picks up spell or special.
 * @param creatng
 */
short creature_picks_up_spell_object(struct Thing *creatng)
{
    struct Coord3d pos;
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* picktng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(picktng);
    if (!thing_can_be_picked_to_place_in_player_room_of_role(picktng, creatng->owner, RoRoF_PowersStorage, TngFRPickF_Default)
     || (get_chessboard_distance(&creatng->mappos, &picktng->mappos) >= subtile_coord(2,0)))
    {
        set_start_state(creatng);
        return 0;
    }
    struct Room* enmroom = get_room_thing_is_on(picktng);
    struct Room* dstroom = find_nearest_room_of_role_for_thing_with_spare_capacity(creatng, creatng->owner, RoRoF_PowersStorage, NavRtF_Default, 1);
    if ( room_is_invalid(dstroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(creatng, dstroom, &pos) )
    {
        WARNLOG("Player %d can't pick %s - doesn't have proper %s to store it",(int)creatng->owner,thing_model_name(picktng),room_role_code_name(RoRoF_PowersStorage));
        set_start_state(creatng);
        return 0;
    }
    // Check if we're stealing the spell from a library
    if (!room_is_invalid(enmroom))
    {
        remove_spell_from_library(enmroom, picktng, creatng->owner);
    }
    // Create event to inform player about the spell or special (need to be done before pickup due to ownership changes)
    update_library_object_pickup_event(creatng, picktng);
    creature_drag_object(creatng, picktng);
    if (!setup_person_move_backwards_to_coord(creatng, &pos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot move to (%d,%d)",(int)pos.x.stl.num, (int)pos.y.stl.num);
        set_start_state(creatng);
        return 0;
    }
    creatng->continue_state = CrSt_CreatureDropsSpellObjectInLibrary;
    return 1;
}

short creature_picks_up_crate_for_workshop(struct Thing *creatng)
{
    struct Coord3d pos;
    TRACE_THING(creatng);
    // Get the crate thing
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* cratetng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(cratetng);
    // Check if everything is right
    if (!thing_can_be_picked_to_place_in_player_room_of_role(cratetng, creatng->owner, RoRoF_CratesStorage, TngFRPickF_Default)
     || (get_chessboard_distance(&creatng->mappos, &cratetng->mappos) >= subtile_coord(2,0)))
    {
        set_start_state(creatng);
        return 0;
    }
    // Find room to drag the crate to
    struct Room* dstroom = find_nearest_room_of_role_for_thing_with_spare_item_capacity(creatng, creatng->owner, RoRoF_CratesStorage, NavRtF_Default);
    if ( room_is_invalid(dstroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(creatng, dstroom, &pos) )
    {
        WARNLOG("Player %d can't pick %s - doesn't have proper %s to store it",(int)creatng->owner,thing_model_name(cratetng),room_role_code_name(RoRoF_CratesStorage));
        set_start_state(creatng);
        return 0;
    }
    // Initialize dragging
    if (!setup_person_move_backwards_to_coord(creatng, &pos, NavRtF_Default))
    {
        set_start_state(creatng);
        return 0;
    }
    update_workshop_object_pickup_event(creatng, cratetng);
    creature_drag_object(creatng, cratetng);
    creatng->continue_state = CrSt_CreatureDropsCrateInWorkshop;
    return 1;
}

/** Being in workshop, pick up a trap crate to be dragged to a trap that needs re-arming.
 *
 * @param thing Special worker creature.
 * @return
 */
short creature_picks_up_trap_object(struct Thing *thing)
{
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Thing* cratetng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(cratetng);
    struct Room* room = get_room_thing_is_on(cratetng);
    struct Thing* traptng = thing_get(cctrl->arming_thing_id);
    TRACE_THING(traptng);
    if (!thing_exists(cratetng) || !thing_exists(traptng))
    {
        WARNLOG("The %s index %d or %s index %d no longer exists",thing_model_name(cratetng),(int)cratetng->index,thing_model_name(traptng),(int)traptng->index);
        cctrl->arming_thing_id = 0;
        set_start_state(thing);
        return 0;
    }
    if (thing_is_dragged_or_pulled(cratetng)
      || (traptng->class_id != TCls_Trap) || (crate_thing_to_workshop_item_model(cratetng) != traptng->model))
    {
        WARNLOG("Cannot use %s index %d to refill %s index %d",thing_model_name(cratetng),(int)cratetng->index,thing_model_name(traptng),(int)traptng->index);
        cctrl->arming_thing_id = 0;
        set_start_state(thing);
        return 0;
    }
    if (get_chessboard_distance(&thing->mappos, &cratetng->mappos) >= 512)
    {
        WARNLOG("The %s index %d was supposed to be near %s index %d for pickup, but it's too far",thing_model_name(cratetng),(int)cratetng->index,thing_model_name(thing),(int)thing->index);
        cctrl->arming_thing_id = 0;
        set_start_state(thing);
        return 0;
    }
    if ( !setup_person_move_backwards_to_coord(thing, &traptng->mappos, NavRtF_Default) )
    {
        WARNLOG("Cannot deliver crate to position of %s index %d",thing_model_name(traptng),(int)traptng->index);
        cctrl->arming_thing_id = 0;
        set_start_state(thing);
        return 0;
    }
    SYNCDBG(18,"Moving %s index %d",thing_model_name(thing),(int)thing->index);
    if (room_exists(room))
    {
        if (remove_workshop_object_from_workshop(room, cratetng))
        {
            if (!is_hero_thing(cratetng) && !is_neutral_thing(cratetng))
            {
                remove_workshop_item_from_amount_stored(cratetng->owner,
                    crate_thing_to_workshop_item_class(cratetng),
                    crate_thing_to_workshop_item_model(cratetng), WrkCrtF_NoOffmap);
            }
        }
    }
    creature_drag_object(thing, cratetng);
    thing->continue_state = CrSt_CreatureArmsTrap;
    return 1;
}

short creature_drops_corpse_in_graveyard(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* deadtng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(deadtng);
    // Check if corpse is ok
    if (!thing_exists(deadtng) || !thing_is_dead_creature(deadtng))
    {
        ERRORLOG("The %s index %d tried to drop a corpse, but it's gone",thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return 0;
    }
    // Check if we're on correct room
    struct Room* room = get_room_thing_is_on(creatng);
    if ( room_is_invalid(room) )
    {
        WARNLOG("Tried to drop %s index %d in %s, but room no longer exists",thing_model_name(deadtng),(int)deadtng->index,room_role_code_name(RoRoF_DeadStorage));
        if (creature_drop_thing_to_another_room(creatng, room, RoRoF_DeadStorage)) {
            creatng->continue_state = CrSt_CreatureDropsCorpseInGraveyard;
            return 1;
        }
        set_start_state(creatng);
        return 0;
    }

    if (!room_role_matches(room->kind, RoRoF_DeadStorage) || (room->owner != creatng->owner)
        || (room->used_capacity >= room->total_capacity) )
    {
        WARNLOG("Tried to drop %s index %d in %s, but room won't accept it",thing_model_name(deadtng),(int)deadtng->index,room_role_code_name(RoRoF_DeadStorage));
        if (creature_drop_thing_to_another_room(creatng, room, RoRoF_DeadStorage)) {
            creatng->continue_state = CrSt_CreatureDropsCorpseInGraveyard;
            return 1;
        }
        set_start_state(creatng);
        return 0;
    }
    // Do the dropping
    creature_drop_dragged_object(creatng, deadtng);
    deadtng->owner = creatng->owner;
    add_body_to_graveyard(deadtng, room);
    // The action of moving object is now finished
    set_start_state(creatng);
    if (game.conf.rules[creatng->owner].workers.digger_work_experience != 0)
    {
        cctrl->exp_points += digger_work_experience(creatng);
        check_experience_upgrade(creatng);
    }
    return 1;
}

short creature_drops_crate_in_workshop(struct Thing *thing)
{
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Thing* cratetng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(cratetng);
    // Check if crate is ok
    if ( !thing_exists(cratetng) )
    {
        ERRORLOG("The %s index %d tried to drop crate, but it's gone",thing_model_name(thing),(int)thing->index);
        set_start_state(thing);
        return 0;
    }
    // Check if we're on correct room
    struct Room* room = get_room_thing_is_on(thing);
    if ( room_is_invalid(room) )
    {
        SYNCDBG(7,"Tried to drop %s index %d in %s, but room no longer exists",thing_model_name(cratetng),(int)cratetng->index,room_role_code_name(RoRoF_CratesStorage));
        if (creature_drop_thing_to_another_room(thing, room, RoRoF_CratesStorage)) {
            thing->continue_state = CrSt_CreatureDropsCrateInWorkshop;
            return 1;
        }
        set_start_state(thing);
        return 0;
    }
    if (!room_role_matches(room->kind, RoRoF_CratesStorage) || (room->owner != thing->owner)
        || (room->used_capacity >= room->total_capacity))
    {
        SYNCDBG(7,"Tried to drop %s index %d in %s, but room won't accept it",thing_model_name(cratetng),(int)cratetng->index,room_role_code_name(RoRoF_CratesStorage));
        if (creature_drop_thing_to_another_room(thing, room, RoRoF_CratesStorage)) {
            thing->continue_state = CrSt_CreatureDropsCrateInWorkshop;
            return 1;
        }
        set_start_state(thing);
        return 0;
    }
    creature_drop_dragged_object(thing, cratetng);
    cratetng->owner = game.neutral_player_num;
    if (thing_is_workshop_crate(cratetng))
    {
        if (!add_workshop_object_to_workshop(room, cratetng))
        {
            WARNLOG("Adding %s index %d to %s room capacity failed",thing_model_name(cratetng),(int)cratetng->index,room_role_code_name(RoRoF_CratesStorage));
            set_start_state(thing);
            return 1;
        }
        cratetng->owner = thing->owner;
        add_workshop_item_to_amounts(room->owner, crate_thing_to_workshop_item_class(cratetng),
            crate_thing_to_workshop_item_model(cratetng));
    }
    // The action of moving object is now finished
    set_start_state(thing);
    if (game.conf.rules[thing->owner].workers.digger_work_experience != 0)
    {
        cctrl->exp_points += game.conf.rules[thing->owner].workers.digger_work_experience;
        check_experience_upgrade(thing);
    }
    return 1;
}

/**
 * Drops a previously picked up spell into a library.
 * @param thing The creature dragging a spell.
 * @return Gives true if the action shall continue, false if it's finished.
 */
short creature_drops_spell_object_in_library(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* spelltng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(spelltng);
    // Check if spell is ok
    if ( !thing_exists(spelltng) )
    {
        ERRORLOG("The %s index %d tried to drop a spell, but it's gone",thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return 0;
    }
    // Check if we're on correct room
    struct Room* room = get_room_thing_is_on(creatng);
    if ( room_is_invalid(room) )
    {
        WARNLOG("Tried to drop %s index %d in %s, but room no longer exists",thing_model_name(spelltng),(int)spelltng->index,room_role_code_name(RoRoF_PowersStorage));
        if (creature_drop_thing_to_another_room(creatng, room, RoRoF_PowersStorage)) {
            creatng->continue_state = CrSt_CreatureDropsSpellObjectInLibrary;
            return 1;
        }
        set_start_state(creatng);
        return 0;
    }
    if (!room_role_matches(room->kind, RoRoF_PowersStorage) || (room->owner != creatng->owner)
        || (room->used_capacity >= room->total_capacity))
    {
        WARNLOG("Tried to drop %s index %d in %s room, but room won't accept it",thing_model_name(spelltng),(int)spelltng->index,room_role_code_name(RoRoF_PowersStorage));
        if (creature_drop_thing_to_another_room(creatng, room, RoRoF_PowersStorage)) {
            creatng->continue_state = CrSt_CreatureDropsSpellObjectInLibrary;
            return 1;
        }
        set_start_state(creatng);
        return 0;
    }
    // Do the dropping
    creature_drop_dragged_object(creatng, spelltng);
    spelltng->owner = game.neutral_player_num;
    if (thing_is_spellbook(spelltng))
    {
        if (!add_item_to_room_capacity(room, true)) {
            WARNLOG("Adding %s index %d to %s room capacity failed",thing_model_name(spelltng),(int)spelltng->index,room_role_code_name(RoRoF_PowersStorage));
            set_start_state(creatng);
            return 1;
        }
        spelltng->owner = creatng->owner;
        add_power_to_player(book_thing_to_power_kind(spelltng), creatng->owner);
    } else
    if (thing_is_special_box(spelltng))
    {
        spelltng->owner = creatng->owner;
    }
    // The action of moving object is now finished
    set_start_state(creatng);
    if (game.conf.rules[creatng->owner].workers.digger_work_experience != 0)
    {
        cctrl->exp_points += digger_work_experience(creatng);
        check_experience_upgrade(creatng);
    }
    return 1;
}

short creature_arms_trap(struct Thing *thing)
{
    TRACE_THING(thing);
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Creature has invalid control structure!");
        return 0;
    }
    struct Thing* cratetng = thing_get(cctrl->dragtng_idx);
    TRACE_THING(cratetng);
    struct Thing* traptng = thing_get(cctrl->arming_thing_id);
    TRACE_THING(traptng);
    if ( !thing_exists(cratetng) || !thing_exists(traptng) )
    {
        set_start_state(thing);
        return 0;
    }
    if (is_thing_directly_controlled(thing))
    {
        return creature_arms_trap_first_person(thing);
    }
    struct Thing* postng = get_trap_at_subtile_of_model_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, traptng->model, thing->owner);
    // Note that this means there can be only one trap of given kind at a subtile.
    // Otherwise it won't be possible to re-arm it, as the condition below will fail.
    if ( (postng->index != traptng->index) || (traptng->trap.num_shots > 0) )
    {
        ERRORLOG("The %s has moved or been already rearmed",thing_model_name(traptng));
        set_start_state(thing);
        return 0;
    }
    rearm_trap(traptng);
    dungeon = get_dungeon(thing->owner);
    dungeon->lvstats.traps_armed++;
    creature_drop_dragged_object(thing, cratetng);
    delete_thing_structure(cratetng, 0);
    thing_play_sample(traptng, 1000, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    // The action of moving object is now finished
    set_start_state(thing);
    if (imp_will_soon_be_arming_trap(traptng)) //Another crate is still earmarked for this trap, refund it.
    {
        readd_workshop_item_to_amount_placeable(traptng->owner, traptng->class_id, traptng->model);
    }
    if (game.conf.rules[thing->owner].workers.digger_work_experience != 0)
    {
        cctrl->exp_points += digger_work_experience(thing);
        check_experience_upgrade(thing);
    }
    return 1;
}

short creature_arms_trap_first_person(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* cratetng = thing_get(cctrl->dragtng_idx);
    struct Thing* traptng = thing_get(cctrl->arming_thing_id);
    controlled_creature_drop_thing(creatng, cratetng, get_appropriate_player_for_creature(creatng));
    move_thing_in_map(cratetng, &traptng->mappos);
    rearm_trap(traptng);
    thing_play_sample(traptng, 1000, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    dungeon->lvstats.traps_armed++;
    if (game.conf.rules[creatng->owner].workers.digger_work_experience != 0)
    {
        cctrl->exp_points += digger_work_experience(creatng);
        check_experience_upgrade(creatng);
    }
    delete_thing_structure(cratetng, 0);
    set_start_state(creatng);
    return 1;
}

/******************************************************************************/
