/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_navigate.c
 *     Things movement navigation functions.
 * @par Purpose:
 *     Functions to support move and following paths by things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Jan 2010 - 12 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_navigate.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "bflib_memory.h"
#include "creature_control.h"
#include "creature_instances.h"
#include "creature_states.h"
#include "creature_states_mood.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "dungeon_data.h"
#include "ariadne.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_creature_turn_to_face_backwards(struct Thing *creatng, struct Coord3d *pos);
DLLIMPORT long _DK_creature_turn_to_face_angle(struct Thing *creatng, long a2);
DLLIMPORT unsigned char _DK_get_nearest_valid_position_for_creature_at(struct Thing *creatng, struct Coord3d *pos);
DLLIMPORT long _DK_get_next_gap_creature_can_fit_in_below_point(struct Thing *creatng, struct Coord3d *pos);
DLLIMPORT long _DK_thing_covers_same_blocks_in_two_positions(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_can_navigate_to_with_storage_f(const struct Thing *creatng, const struct Coord3d *pos, NaviRouteFlags flags, const char *func_name)
{
    NAVIDBG(8,"%s: Route for %s index %d from %3d,%3d to %3d,%3d", func_name, thing_model_name(creatng),(int)creatng->index,
        (int)creatng->mappos.x.stl.num, (int)creatng->mappos.y.stl.num, (int)pos->x.stl.num, (int)pos->y.stl.num);
    AriadneReturn aret = ariadne_initialise_creature_route_f((struct Thing*)creatng, pos, get_creature_speed(creatng), flags, func_name);
    NAVIDBG(18,"Ariadne returned %d",(int)aret);
    return (aret == AridRet_OK);
}

unsigned char get_nearest_valid_position_for_creature_at(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_get_nearest_valid_position_for_creature_at(thing, pos);
}

static void get_nearest_navigable_point_for_thing(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, NaviRouteFlags flags)
{
    long nav_sizexy;
    long px;
    long py;
    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(thing);
    if ((flags & AridRtF_NoOwner) != 0)
        owner_player_navigating = -1;
    else
        owner_player_navigating = thing->owner;
    nav_sizexy = thing_nav_block_sizexy(thing);
    if (nav_sizexy > 0) nav_sizexy--;
    nearest_search(nav_sizexy, thing->mappos.x.val, thing->mappos.y.val,
      pos1->x.val, pos1->y.val, &px, &py);
    pos2->x.val = px;
    pos2->y.val = py;
    pos2->z.val = get_thing_height_at(thing, pos2);
    if (thing_in_wall_at(thing, pos2))
        get_nearest_valid_position_for_creature_at(thing, pos2);
    nav_thing_can_travel_over_lava = 0;
}
HOOK_DK_FUNC(get_nearest_navigable_point_for_thing)

TbBool setup_person_move_to_position_f(struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, NaviRouteFlags flags, const char *func_name)
{
    SYNCDBG(18,"%s: Moving %s index %d to (%d,%d)",func_name,thing_model_name(thing),(int)thing->index,(int)stl_x,(int)stl_y);
    TRACE_THING(thing);
    struct Coord3d locpos;
    locpos.x.val = subtile_coord_center(stl_x);
    locpos.y.val = subtile_coord_center(stl_y);
    locpos.z.val = thing->mappos.z.val;
    locpos.z.val = get_thing_height_at(thing, &locpos);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        WARNLOG("%s: Tried to move invalid creature to (%d,%d)",func_name,(int)stl_x,(int)stl_y);
        return false;
    }
    if (thing_in_wall_at(thing, &locpos))
    {
        SYNCDBG(16,"%s: The %s would be trapped in wall at (%d,%d)",func_name,thing_model_name(thing),(int)stl_x,(int)stl_y);
        return false;
    }
    if (!creature_can_navigate_to_with_storage_f(thing, &locpos, flags, func_name))
    {
        SYNCDBG(19,"%s: The %s cannot reach subtile (%d,%d)",func_name,thing_model_name(thing),(int)stl_x,(int)stl_y);
        return false;
    }
    cctrl->move_flags = flags;
    internal_set_thing_state(thing, CrSt_MoveToPosition);
    cctrl->moveto_pos.x.val = locpos.x.val;
    cctrl->moveto_pos.y.val = locpos.y.val;
    cctrl->moveto_pos.z.val = locpos.z.val;
    SYNCDBG(19,"%s: Done",func_name);
    return true;
}

TbBool setup_person_move_close_to_position(struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, NaviRouteFlags flags)
{
    SYNCDBG(18,"Moving %s index %d to (%d,%d)",thing_model_name(thing),(int)thing->index,(int)stl_x,(int)stl_y);
    struct Coord3d trgpos;
    trgpos.x.val = subtile_coord_center(stl_x);
    trgpos.y.val = subtile_coord_center(stl_y);
    trgpos.z.val = thing->mappos.z.val;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        WARNLOG("Tried to move invalid creature to (%d,%d)",(int)stl_x,(int)stl_y);
        return false;
    }
    struct Coord3d navpos;
    get_nearest_navigable_point_for_thing(thing, &trgpos, &navpos, flags);
    if (!creature_can_navigate_to_with_storage(thing, &navpos, flags))
    {
        SYNCDBG(19,"The %s cannot reach subtile (%d,%d)",thing_model_name(thing),(int)stl_x,(int)stl_y);
        return false;
    }
    cctrl->move_flags = flags;
    internal_set_thing_state(thing, CrSt_MoveToPosition);
    cctrl->moveto_pos.x.val = navpos.x.val;
    cctrl->moveto_pos.y.val = navpos.y.val;
    cctrl->moveto_pos.z.val = navpos.z.val;
    return true;
}

TbBool setup_person_move_backwards_to_position_f(struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, NaviRouteFlags flags, const char *func_name)
{
    SYNCDBG(18,"%s: Moving %s index %d to (%d,%d)",func_name,thing_model_name(thing),(int)thing->index,(int)stl_x,(int)stl_y);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Coord3d locpos;
    locpos.x.val = subtile_coord_center(stl_x);
    locpos.y.val = subtile_coord_center(stl_y);
    locpos.z.val = 0;
    locpos.z.val = get_thing_height_at(thing, &locpos);
    if (thing_in_wall_at(thing, &locpos))
    {
        SYNCDBG(16,"%s: The %s would be trapped in wall at (%d,%d)",func_name,thing_model_name(thing),(int)stl_x,(int)stl_y);
        return false;
    }
    if (!creature_can_navigate_to_with_storage(thing, &locpos, flags))
    {
        SYNCDBG(19,"%s: The %s cannot reach subtile (%d,%d)",func_name,thing_model_name(thing),(int)stl_x,(int)stl_y);
        return false;
    }
    cctrl->move_flags = flags;
    internal_set_thing_state(thing, CrSt_MoveBackwardsToPosition);
    cctrl->moveto_pos.x.val = locpos.x.val;
    cctrl->moveto_pos.y.val = locpos.y.val;
    cctrl->moveto_pos.z.val = locpos.z.val;
    return true;
}

TbBool setup_person_move_to_coord_f(struct Thing *thing, const struct Coord3d *pos, NaviRouteFlags flags, const char *func_name)
{
    return setup_person_move_to_position_f(thing, pos->x.stl.num, pos->y.stl.num, flags, func_name);
}

TbBool setup_person_move_backwards_to_coord(struct Thing *thing, const struct Coord3d *pos, NaviRouteFlags flags)
{
    return setup_person_move_backwards_to_position(thing, pos->x.stl.num, pos->y.stl.num, flags);
}

/**
 * Returns a hero gate object to which given hero can navigate.
 * @todo CREATURE_AI It returns first hero door found, not the best one.
 *     Maybe it should find the one he will reach faster, or at least a random one?
 * @param herotng The hero to be able to make it to gate.
 * @return The gate thing, or invalid thing.
 */
struct Thing *find_hero_door_hero_can_navigate_to(struct Thing *herotng)
{
    unsigned long k = 0;
    int i = game.thing_lists[TngList_Objects].index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per thing code
        if (object_is_hero_gate(thing))
        {
            if (creature_can_navigate_to_with_storage(herotng, &thing->mappos, NavRtF_Default)) {
                return thing;
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return NULL;
}

void move_thing_in_map_f(struct Thing *thing, const struct Coord3d *pos, const char *func_name)
{
    SYNCDBG(18,"%s: Starting for %s index %d",func_name,thing_model_name(thing),(int)thing->index);
    TRACE_THING(thing);
    if ((thing->mappos.x.stl.num == pos->x.stl.num) && (thing->mappos.y.stl.num == pos->y.stl.num))
    {
        SYNCDBG(19,"Moving %s index %d from (%d,%d) to (%d,%d)",thing_model_name(thing),
            (int)thing->index,(int)thing->mappos.x.val,(int)thing->mappos.y.val,(int)pos->x.val,(int)pos->y.val);
        thing->mappos.x.val = pos->x.val;
        thing->mappos.y.val = pos->y.val;
        thing->mappos.z.val = pos->z.val;
    } else
    {
        SYNCDBG(19,"Moving %s index %d from (%d,%d) to (%d,%d), subtile changed",thing_model_name(thing),
            (int)thing->index,(int)thing->mappos.x.val,(int)thing->mappos.y.val,(int)pos->x.val,(int)pos->y.val);
        remove_thing_from_mapwho(thing);
        thing->mappos.x.val = pos->x.val;
        thing->mappos.y.val = pos->y.val;
        thing->mappos.z.val = pos->z.val;
        place_thing_in_mapwho(thing);
    }
    thing->field_60 = get_thing_height_at(thing, &thing->mappos);
}

TbBool move_creature_to_nearest_valid_position(struct Thing *thing)
{
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    if (!get_nearest_valid_position_for_creature_at(thing, &pos))
    {
        return false;
    }
    move_thing_in_map(thing, &pos);
    return true;
}

/**
 * Returns if a creature can currently travel over lava.
 * @param thing
 * @return
 */
TbBool creature_can_travel_over_lava(const struct Thing *creatng)
{
    const struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    // Check if a creature can fly in this moment - we don't care if it's natural ability
    // or temporary spell effect
    return (crstat->hurt_by_lava <= 0) || ((creatng->movement_flags & TMvF_Flying) != 0);
}

/**
 * Checks if a creature can navigate to target by tracing a complete route.
 * @param thing
 * @param dstpos
 * @param flags
 * @param func_name
 * @return
 * @see ariadne_prepare_creature_route_to_target() does the same thing but writes resulting route into Ariadne struct.
 */
TbBool creature_can_navigate_to_f(const struct Thing *thing, struct Coord3d *dstpos, NaviRouteFlags flags, const char *func_name)
{
    long waypoints_num = ariadne_count_waypoints_on_creature_route_to_target_f(thing, &thing->mappos, dstpos, flags, func_name);
    return (waypoints_num > 0);
}

/**
 * Returns if a creature can get to given players dungeon.
 * @param thing
 * @param plyr_idx
 * @return
 * @see creature_can_get_to_any_of_players_rooms() is a function used in similar manner.
 */
TbBool creature_can_get_to_dungeon(struct Thing *creatng, PlayerNumber plyr_idx)
{
    SYNCDBG(18,"Starting");
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player) || (player->is_active != 1))
    {
        SYNCDBG(18,"The %s index %d cannot get to inactive player %d",thing_model_name(creatng),(int)creatng->index,(int)plyr_idx);
        return false;
    }
    struct Thing* heartng = get_player_soul_container(player->id_number);
    if (thing_is_invalid(heartng))
    {
        SYNCDBG(18,"The %s index %d cannot get to player %d without heart",thing_model_name(creatng),(int)creatng->index,(int)plyr_idx);
        return false;
    }
    if (heartng->active_state == ObSt_BeingDestroyed)
    {
        SYNCDBG(18,"The %s index %d cannot get to player %d due to heart state",thing_model_name(creatng),(int)creatng->index,(int)plyr_idx);
        return false;
    }
    return creature_can_navigate_to(creatng, &heartng->mappos, NavRtF_Default);
}

long creature_turn_to_face(struct Thing *thing, const struct Coord3d *pos)
{
    //TODO enable when issue in pathfinding is solved
    /*if (get_2d_box_distance(&thing->mappos, pos) <= 0)
        return -1;*/
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    long angle = get_angle_xy_to(&thing->mappos, pos);
    long angle_diff = get_angle_difference(thing->move_angle_xy, angle);
    long angle_sign = get_angle_sign(thing->move_angle_xy, angle);
    long angle_delta = crstat->max_angle_change;
    if (angle_delta < 1) {
        angle_delta = 1;
    }
    if (angle_delta > angle_diff) {
        angle_delta = angle_diff;
    }
    if (angle_sign < 0) {
        angle_delta = -angle_delta;
    }
    long i = (thing->move_angle_xy + angle_delta);
    thing->move_angle_xy = i & LbFPMath_AngleMask;
    return get_angle_difference(thing->move_angle_xy, angle);
}

long creature_turn_to_face_backwards(struct Thing *thing, struct Coord3d *pos)
{
    //TODO enable when issue in pathfinding is solved
    /*if (get_2d_box_distance(&thing->mappos, pos) <= 0)
        return -1;*/
    return _DK_creature_turn_to_face_backwards(thing, pos);
}

long creature_turn_to_face_angle(struct Thing *thing, long a2)
{
    return _DK_creature_turn_to_face_angle(thing, a2);
}

long creature_move_to_using_gates(struct Thing *thing, struct Coord3d *pos, MoveSpeed speed, long a4, NaviRouteFlags flags, TbBool backward)
{
    long i;
    SYNCDBG(18,"Starting to move %s index %d from (%d,%d) to (%d,%d) with speed %d",thing_model_name(thing),
        (int)thing->index,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num,(int)pos->x.stl.num,(int)pos->y.stl.num,(int)speed);
    TRACE_THING(thing);
    if ( backward )
    {
        // Rotate the creature 180 degrees to trace route with forward move
        i = (thing->move_angle_xy + LbFPMath_PI);
        thing->move_angle_xy = i & LbFPMath_AngleMask;
    }
    struct Coord3d nextpos;
    AriadneReturn follow_result = creature_follow_route_to_using_gates(thing, pos, &nextpos, speed, flags);
    SYNCDBG(18,"The %s index %d route result: %d, next pos (%d,%d)",thing_model_name(thing),(int)thing->index,(int)follow_result,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num);
    if ( backward )
    {
        // Rotate the creature back
        i = (thing->move_angle_xy + LbFPMath_PI);
        thing->move_angle_xy = i & LbFPMath_AngleMask;
    }
    if ((follow_result == AridRet_PartOK) || (follow_result == AridRet_Val2))
    {
        creature_set_speed(thing, 0);
        return -1;
    }
    if (follow_result == AridRet_FinalOK)
    {
        return  1;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ( backward )
    {
        if (creature_turn_to_face_backwards(thing, &nextpos) > 0)
        {
            // Creature is turning - don't let it move
            creature_set_speed(thing, 0);
        } else
        {
            creature_set_speed(thing, -speed);
            cctrl->flgfield_2 |= TF2_Unkn01;
            if (get_2d_box_distance(&thing->mappos, &nextpos) > -2*cctrl->move_speed)
            {
                ERRORDBG(3,"The %s index %d tried to reach (%d,%d) from (%d,%d) with excessive backward speed",
                    thing_model_name(thing),(int)thing->index,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num,
                    (int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
                cctrl->moveaccel.x.val = distance_with_angle_to_coord_x(cctrl->move_speed, thing->move_angle_xy);
                cctrl->moveaccel.y.val = distance_with_angle_to_coord_y(cctrl->move_speed, thing->move_angle_xy);
                cctrl->moveaccel.z.val = 0;
            } else
            {
                cctrl->moveaccel.x.val = nextpos.x.val - (MapCoordDelta)thing->mappos.x.val;
                cctrl->moveaccel.y.val = nextpos.y.val - (MapCoordDelta)thing->mappos.y.val;
                cctrl->moveaccel.z.val = 0;
            }
        }
        SYNCDBG(18,"Backward target set, speed %d, accel (%d,%d)",(int)cctrl->move_speed,(int)cctrl->moveaccel.x.val,(int)cctrl->moveaccel.y.val);
    } else
    {
        if (creature_turn_to_face(thing, &nextpos) > 0)
        {
            // Creature is turning - don't let it move
            creature_set_speed(thing, 0);
        } else
        {
            creature_set_speed(thing, speed);
            cctrl->flgfield_2 |= TF2_Unkn01;
            if (get_2d_box_distance(&thing->mappos, &nextpos) > 2*cctrl->move_speed)
            {
                ERRORDBG(3,"The %s index %d tried to reach (%d,%d) from (%d,%d) with excessive forward speed",
                    thing_model_name(thing),(int)thing->index,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num,
                    (int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
                cctrl->moveaccel.x.val = distance_with_angle_to_coord_x(cctrl->move_speed, thing->move_angle_xy);
                cctrl->moveaccel.y.val = distance_with_angle_to_coord_y(cctrl->move_speed, thing->move_angle_xy);
                cctrl->moveaccel.z.val = 0;
            } else
            {
                cctrl->moveaccel.x.val = nextpos.x.val - (MapCoordDelta)thing->mappos.x.val;
                cctrl->moveaccel.y.val = nextpos.y.val - (MapCoordDelta)thing->mappos.y.val;
                cctrl->moveaccel.z.val = 0;
            }
        }
        SYNCDBG(18,"Forward target set, speed %d, accel (%d,%d)",(int)cctrl->move_speed,(int)cctrl->moveaccel.x.val,(int)cctrl->moveaccel.y.val);
    }
    return 0;
}

long creature_move_to(struct Thing *creatng, struct Coord3d *pos, MoveSpeed speed, NaviRouteFlags flags, TbBool backward)
{
    SYNCDBG(18,"Starting to move %s index %d into (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)pos->x.stl.num,(int)pos->y.stl.num);
    return creature_move_to_using_gates(creatng, pos, speed, -2, flags, backward);
}

TbBool creature_move_to_using_teleport(struct Thing *thing, struct Coord3d *pos, long walk_speed)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_instance_is_available(thing, CrInst_TELEPORT)
     && creature_instance_has_reset(thing, CrInst_TELEPORT)
     && (cctrl->instance_id == CrInst_NULL))
    {
        // Creature can only be teleported to a revealed location
        short destination_valid = true;
        if (!is_hero_thing(thing) && !is_neutral_thing(thing)) {
            destination_valid = subtile_revealed(pos->x.stl.num, pos->y.stl.num, thing->owner);
        }
        if (destination_valid)
         {
             // Use teleport only over large enough distances
             if (get_2d_box_distance(&thing->mappos, pos) > COORD_PER_STL*game.min_distance_for_teleport)
             {
                 set_creature_instance(thing, CrInst_TELEPORT, 1, 0, pos);
                 return true;
             }
         }
    }
    return false;
}

short move_to_position(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    long speed = get_creature_speed(creatng);
    SYNCDBG(18,"Starting to move %s index %d into (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)cctrl->moveto_pos.x.stl.num,(int)cctrl->moveto_pos.y.stl.num);
    // Try teleporting the creature
    if (creature_move_to_using_teleport(creatng, &cctrl->moveto_pos, speed)) {
        SYNCDBG(8,"Teleporting %s index %d owner %d into (%d,%d) for %s",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,
            (int)cctrl->moveto_pos.x.stl.num,(int)cctrl->moveto_pos.y.stl.num,creature_state_code_name(creatng->continue_state));
        return 1;
    }
    long move_result = creature_move_to(creatng, &cctrl->moveto_pos, speed, cctrl->move_flags, 0);
    CrCheckRet state_check = CrCkRet_Available;
    struct StateInfo* stati = get_thing_continue_state_info(creatng);
    if (!state_info_invalid(stati))
    {
        CreatureStateCheck callback = stati->move_check;
        if (callback != NULL)
        {
            SYNCDBG(18,"Doing move check callback for continue state %s",creature_state_code_name(creatng->continue_state));
            state_check = callback(creatng);
        }
    }
    if (state_check == CrCkRet_Available)
    {
        // If moving was successful
        if (move_result == 1) {
            // Back to "main state"
            internal_set_thing_state(creatng, creatng->continue_state);
            return CrStRet_Modified;
        }
        // If moving failed, do a reset
        if (move_result == -1) {
            CrtrStateId cntstat = creatng->continue_state;
            internal_set_thing_state(creatng, cntstat);
            set_start_state(creatng);
            SYNCDBG(8,"Couldn't move %s to place required for state %s; reset to state %s",thing_model_name(creatng),creature_state_code_name(cntstat),creatrtng_actstate_name(creatng));
            return CrStRet_ResetOk;
        }
        // If continuing the job, check for job stress
        process_job_stress_and_going_postal(creatng);
    }
    switch (state_check)
    {
    case CrCkRet_Deleted:
        return CrStRet_Deleted;
    case CrCkRet_Available:
        return CrStRet_Modified;
    default:
        return CrStRet_ResetOk;
    }
}

long get_next_gap_creature_can_fit_in_below_point(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_get_next_gap_creature_can_fit_in_below_point(thing, pos);
}

long thing_covers_same_blocks_in_two_positions(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2)
{
    return _DK_thing_covers_same_blocks_in_two_positions(thing, pos1, pos2);
}

long get_thing_blocked_flags_at(struct Thing *thing, struct Coord3d *pos)
{
    unsigned short flags = SlbBloF_None;
    struct Coord3d locpos;
    locpos.x.val = pos->x.val;
    locpos.y.val = thing->mappos.y.val;
    locpos.z.val = thing->mappos.z.val;
    if ( thing_in_wall_at(thing, &locpos) )
        flags |= SlbBloF_WalledX;
    locpos.x.val = thing->mappos.x.val;
    locpos.y.val = pos->y.val;
    locpos.z.val = thing->mappos.z.val;
    if ( thing_in_wall_at(thing, &locpos) )
        flags |= SlbBloF_WalledY;
    locpos.x.val = thing->mappos.x.val;
    locpos.y.val = thing->mappos.y.val;
    locpos.z.val = pos->z.val;
    if ( thing_in_wall_at(thing, &locpos) )
        flags |= SlbBloF_WalledZ;
    switch ( flags )
    {
    case SlbBloF_None:
        locpos.x.val = pos->x.val;
        locpos.y.val = pos->y.val;
        locpos.z.val = pos->z.val;
        if ( thing_in_wall_at(thing, &locpos) )
            flags = SlbBloF_WalledX | SlbBloF_WalledY | SlbBloF_WalledZ;
        break;
    case SlbBloF_WalledX:
        locpos.x.val = thing->mappos.x.val;
        locpos.y.val = pos->y.val;
        locpos.z.val = pos->z.val;
        if ( thing_in_wall_at(thing, &locpos) )
            flags = SlbBloF_WalledX | SlbBloF_WalledY | SlbBloF_WalledZ;
        break;
    case SlbBloF_WalledY:
        locpos.x.val = pos->x.val;
        locpos.y.val = thing->mappos.y.val;
        locpos.z.val = pos->z.val;
        if ( thing_in_wall_at(thing, &locpos) )
            flags = SlbBloF_WalledX | SlbBloF_WalledY | SlbBloF_WalledZ;
        break;
    case SlbBloF_WalledZ:
        locpos.x.val = pos->x.val;
        locpos.y.val = pos->y.val;
        locpos.z.val = thing->mappos.z.val;
        if ( thing_in_wall_at(thing, &locpos) )
            flags = SlbBloF_WalledX | SlbBloF_WalledY | SlbBloF_WalledZ;
        break;
    }
    return flags;
}

/******************************************************************************/
