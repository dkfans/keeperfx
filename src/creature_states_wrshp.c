/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_wrshp.c
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
#include "creature_states_wrshp.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "gui_soundmsgs.h"

#include "game_legacy.h"
#include "keeperfx.hpp"

/******************************************************************************/
TbBool creature_can_do_manufacturing(const struct Thing *creatng)
{
    if (is_neutral_thing(creatng)) {
        return false;
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    return (crstat->manufacture_value > 0);
}

TbBool setup_workshop_move(struct Thing *thing, SubtlCodedCoords stl_num)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->moveto_pos.x.stl.num = stl_num_decode_x(stl_num);
    cctrl->moveto_pos.x.stl.pos = 128;
    cctrl->moveto_pos.y.stl.num = stl_num_decode_y(stl_num);
    cctrl->moveto_pos.y.stl.pos = 128;
    cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
    if (thing_in_wall_at(thing, &cctrl->moveto_pos))
    {
        ERRORLOG("Illegal setup to subtile (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
        set_start_state(thing);
        return false;
    }
    return true;
}

struct Thing *get_workshop_equipment_to_work_with_on_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
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
        if (thing_is_object(thing) && object_is_room_equipment(thing, RoK_WORKSHOP))
        {
            if (thing->owner == plyr_idx) {
              return thing;
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
    return INVALID_THING;
}

/**
 * Returns a creature manufacturing on a subtile other than given creature.
 * @param plyr_idx
 * @param stl_x
 * @param stl_y
 * @param othertng
 * @return
 */
struct Thing *get_other_creature_manufacturing_on_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *othertng)
{
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
        if (thing_is_creature(thing) && (thing->active_state == CrSt_Manufacturing) && (thing->index != othertng->index))
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            if ((cctrl->byte_9A > 1) && (thing->owner == plyr_idx)) {
                return thing;
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
    return INVALID_THING;
}

/**
 * Finds a safe and unused, adjacent position in room for a creature.
 *
 * @param pos Position of the creature to be moved.
 * @param owner Room owner to keep.
 * @return Coded subtiles of the new position, or 0 on failure.
 * @see person_get_somewhere_adjacent_in_room()
 */
SubtlCodedCoords find_unused_adjacent_position_in_workshop(const struct Coord3d *pos, long owner)
{
    static const struct Around corners[] = { {1,2}, {0,1}, {1,0}, {2,1} };
    for (long i = 0; i < SMALL_AROUND_LENGTH; i++)
    {
        MapSlabCoord slb_x = subtile_slab_fast(pos->x.stl.num) + (long)small_around[i].delta_x;
        MapSlabCoord slb_y = subtile_slab_fast(pos->y.stl.num) + (long)small_around[i].delta_y;
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        if ((slb->kind == SlbT_WORKSHOP) && (slabmap_owner(slb) == owner))
        {
            MapSubtlCoord stl_x = slab_subtile(slb_x, corners[i].delta_x);
            MapSubtlCoord stl_y = slab_subtile(slb_y, corners[i].delta_y);
            struct Thing* mnfc_creatng = get_other_creature_manufacturing_on_subtile(owner, stl_x, stl_y, INVALID_THING);
            if (!thing_is_invalid(mnfc_creatng)) {
                // Position used by another manufacturer
                continue;
            }
            struct Thing* objtng = get_workshop_equipment_to_work_with_on_subtile(owner, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
            if (thing_is_invalid(objtng)) {
                // Position has no work equipment nearby
                continue;
            }
            // Found an acceptable position
            return get_subtile_number(stl_x, stl_y);
        }
    }
    return 0;
}

TbBool setup_move_to_new_workshop_position(struct Thing *thing, struct Room *room, unsigned long a3)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ( a3 )
        cctrl->byte_9E = 50;
    cctrl->byte_9A = 1;
    SubtlCodedCoords stl_num = find_position_around_in_room(&thing->mappos, thing->owner, room->kind, thing);
    if (stl_num <= 0)
    {
        WARNLOG("Could not find position around in %s of %d slabs",room_code_name(room->kind),(int)room->slabs_count);
        return false;
    }
    return setup_workshop_move(thing,stl_num);
}

short at_workshop_room(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->target_room_id = 0;
    struct Room* room = get_room_thing_is_on(creatng);
    if (!room_initially_valid_as_type_for_thing(room, get_room_for_job(Job_MANUFACTURE), creatng))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(creatng));
        set_start_state(creatng);
        return 0;
    }
    if (!add_creature_to_work_room(creatng, room, Job_MANUFACTURE))
    {
        set_start_state(creatng);
        return 0;
    }
    internal_set_thing_state(creatng, get_continue_state_for_job(Job_MANUFACTURE));
    setup_move_to_new_workshop_position(creatng, room, 1);
    return 1;
}

void setup_workshop_search_for_post(struct Thing *creatng)
{
    struct Thing* postng = INVALID_THING;
    struct Room* room = get_room_thing_is_on(creatng);
    // Find a random slab in the room to be used as our starting point
    long i = CREATURE_RANDOM(creatng, room->slabs_count);
    unsigned long n = room->slabs_list;
    while (i > 0)
    {
        n = get_next_slab_number_in_room(n);
        i--;
    }
    i = room->slabs_count;
    while (i > 0)
    {
        // Loop the slabs list
        if (n <= 0) {
            n = room->slabs_list;
        }
        MapSlabCoord slb_x = slb_num_decode_x(n);
        MapSlabCoord slb_y = slb_num_decode_y(n);
        struct Thing* objtng = get_workshop_equipment_to_work_with_on_subtile(creatng->owner, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (!thing_is_invalid(objtng)) {
            postng = objtng;
        }
        n = get_next_slab_number_in_room(n);
        i--;
    }
    if (thing_is_invalid(postng))
    {
        SYNCDBG(9,"Work in %s, the %s moves to new pos",room_code_name(room->kind),thing_model_name(creatng));
        setup_move_to_new_workshop_position(creatng, room, 1);
    } else
    {
        SYNCDBG(9,"Work in %s, the %s found a post",room_code_name(room->kind),thing_model_name(creatng));
        setup_workshop_move(creatng, get_subtile_number(postng->mappos.x.stl.num, postng->mappos.y.stl.num));
    }
}

long process_creature_in_workshop(struct Thing *creatng, struct Room *room)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    struct Dungeon *dungeon;
    dungeon = get_dungeon(creatng->owner);
    if ((game.play_gameturn - dungeon->field_118B < 50) && ((game.play_gameturn + creatng->index) & 3) == 0)
    {
        if (cctrl->instance_id == CrInst_NULL) {
            set_creature_instance(creatng, CrInst_CELEBRATE_SHORT, 1, 0, 0);
        }
        return 1;
    }
    if (cctrl->instance_id != CrInst_NULL) {
        return 1;
    }
    long mvret;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    SYNCDBG(19,"Work in %s, the %s in state %d",room_code_name(room->kind),thing_model_name(creatng),(int)cctrl->byte_9A);
    switch (cctrl->byte_9A)
    {
    case 1:
        cctrl->byte_9E--;
        if (cctrl->byte_9E <= 0)
        {
            setup_workshop_search_for_post(creatng);
            cctrl->byte_9E = 100;
            break;
        }
        mvret = creature_move_to(creatng, &cctrl->moveto_pos, get_creature_speed(creatng), 0, 0);
        if (mvret != 1)
        {
            if (mvret == -1) {
                SYNCDBG(9,"Room %s move problem, the %s goes from %d to start state",room_code_name(room->kind),thing_model_name(creatng),(int)cctrl->byte_9A);
                set_start_state(creatng);
            }
            break;
        }
        slb_x = subtile_slab_fast(creatng->mappos.x.stl.num);
        slb_y = subtile_slab_fast(creatng->mappos.y.stl.num);
        struct Thing *objtng;
        objtng = get_workshop_equipment_to_work_with_on_subtile(creatng->owner, slab_subtile_center(slb_x),slab_subtile_center(slb_y));
        if (!thing_is_invalid(objtng))
        {
            SYNCDBG(19,"Got %s post, the %s goes from %d to 2",room_code_name(room->kind),thing_model_name(creatng),(int)cctrl->byte_9A);
            cctrl->byte_9A = 2;
            cctrl->byte_9E = 100;
            break;
        }
        SYNCDBG(19,"No %s post at current pos, the %s goes from %d to search position",room_code_name(room->kind),thing_model_name(creatng),(int)cctrl->byte_9A);
        setup_move_to_new_workshop_position(creatng, room, 0);
        break;
    case 2:
    {
        SubtlCodedCoords stl_num;
        stl_num = find_unused_adjacent_position_in_workshop(&creatng->mappos, creatng->owner);
        if (stl_num != 0) {
            slb_x = subtile_slab_fast(stl_num_decode_x(stl_num));
            slb_y = subtile_slab_fast(stl_num_decode_y(stl_num));
            cctrl->byte_9C = slab_subtile_center(slb_x);
            cctrl->byte_9D = slab_subtile_center(slb_y);
            setup_workshop_move(creatng, stl_num);
            cctrl->byte_9A = 3;
            break;
        }
        SYNCDBG(9,"No free adjacent %s post, the %s goes from %d to search position",room_code_name(room->kind),thing_model_name(creatng),(int)cctrl->byte_9A);
        setup_move_to_new_workshop_position(creatng, room, 1);
        break;
    }
    case 3:
    {
        mvret = creature_move_to(creatng, &cctrl->moveto_pos, get_creature_speed(creatng), 0, 0);
        if (mvret != 1)
        {
            if (mvret == -1) {
                SYNCDBG(9,"Room %s move problem, the %s goes from %d to start state",room_code_name(room->kind),thing_model_name(creatng),(int)cctrl->byte_9A);
                set_start_state(creatng);
            }
            break;
        }
        struct Thing *mnfc_creatng;
        mnfc_creatng = get_other_creature_manufacturing_on_subtile(creatng->owner, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, creatng);
        if (thing_is_invalid(mnfc_creatng)) {
            cctrl->byte_9A = 4;
            break;
        }
        // Position used by another manufacturer
        SYNCDBG(9,"The %s post already in use, the %s goes from %d to search position",room_code_name(room->kind),thing_model_name(creatng),(int)cctrl->byte_9A);
        setup_move_to_new_workshop_position(creatng, room, 1);
        break;
    }
    case 4:
    {
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(cctrl->byte_9C);
        pos.y.val = subtile_coord_center(cctrl->byte_9D);
        if (creature_turn_to_face(creatng, &pos) < LbFPMath_PI/18)
        {
            cctrl->byte_9A = 5;
            cctrl->byte_9B = 75;
        }
        break;
    }
    case 5:
    default:
        cctrl->byte_9B--;
        if (cctrl->byte_9B <= 0)
        {
            SYNCDBG(9,"Room %s move counter %d, the %s keeps moving in state %d",room_code_name(room->kind),(int)cctrl->byte_9B,thing_model_name(creatng),(int)cctrl->byte_9A);
            setup_move_to_new_workshop_position(creatng, room, 1);
        } else
        if ((cctrl->byte_9B % 8) == 0) {
            set_creature_instance(creatng, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
        }
        break;
    }
    return 1;
}

short manufacturing(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Room* room = get_room_thing_is_on(creatng);
    if (creature_job_in_room_no_longer_possible(room, Job_MANUFACTURE, creatng))
    {
        remove_creature_from_work_room(creatng);
        set_start_state(creatng);
        return CrStRet_ResetFail;
    }
    if (room->used_capacity > room->total_capacity)
    {
        output_message_room_related_from_computer_or_player_action(room->owner, room->kind, OMsg_RoomFull);
        remove_creature_from_work_room(creatng);
        set_start_state(creatng);
        return CrStRet_ResetOk;
    }
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    if (dungeon->manufacture_class != TCls_Empty)
    {
        long work_value = compute_creature_work_value_for_room_role(creatng, RoRoF_CratesManufctr, room->efficiency);
        SYNCDBG(9,"The %s index %d owner %d produced %d manufacture points",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,(int)work_value);
        dungeon->manufacture_progress += work_value;
        dungeon->total_manufacture_points += work_value;
    } else
    {
        WARNDBG(9,"The %s index %d owner %d is manufacturing nothing",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        // This may be cause by a creature taking up place in workshop where crate should be created; the creature should take a break
        if (room->used_capacity >= room->total_capacity) {
            external_set_thing_state(creatng, CrSt_CreatureGoingHomeToSleep);
            return CrStRet_Modified;
        }
    }
    process_creature_in_workshop(creatng, room);
    return CrStRet_Modified;
}

/******************************************************************************/
