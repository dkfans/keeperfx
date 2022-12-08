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
#include "pre_inc.h"
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
#include "map_blocks.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "dungeon_data.h"
#include "ariadne.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

// Call this function if you don't want the creature/thing to (visually) fly across the map whenever suddenly moving a very far distance. (teleporting for example)
void reset_interpolation_of_thing(struct Thing *thing)
{
    struct ThingAdd* thingadd = get_thingadd(thing->index);
    thingadd->previous_mappos = thing->mappos;
    thingadd->previous_floor_height = thing->floor_height;
    thingadd->interp_mappos = thing->mappos;
    thingadd->interp_floor_height = thing->floor_height;
    thingadd->previous_minimap_pos_x = 0;
    thingadd->previous_minimap_pos_y = 0;
}

TbBool creature_can_navigate_to_with_storage_f(const struct Thing *creatng, const struct Coord3d *pos, NaviRouteFlags flags, const char *func_name)
{
    NAVIDBG(8,"%s: Route for %s index %d from %3d,%3d to %3d,%3d", func_name, thing_model_name(creatng),(int)creatng->index,
        (int)creatng->mappos.x.stl.num, (int)creatng->mappos.y.stl.num, (int)pos->x.stl.num, (int)pos->y.stl.num);
    AriadneReturn aret = ariadne_initialise_creature_route_f((struct Thing*)creatng, pos, get_creature_speed(creatng), flags, func_name);
    NAVIDBG(18,"Ariadne returned %d",(int)aret);
    return (aret == AridRet_OK);
}

TbBool get_nearest_valid_position_for_creature_at(struct Thing *thing, struct Coord3d *pos)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    struct Coord3d spiral_pos;
    struct Map* mapblk;
    struct MapOffset* sstep;

    for (int i = 0; i < SPIRAL_STEPS_COUNT; i++)
    {
        sstep = &spiral_step[i];
        stl_x = sstep->h + pos->x.stl.num;
        stl_y = sstep->v + pos->y.stl.num;
        if ( stl_x < 0 )
        {
            stl_x = 0; 
        }
        else if ( stl_x > map_subtiles_x )
        {
            stl_x = map_subtiles_x;
        }

        if ( stl_y < 0 )
        {
            stl_y = 0; 
        }
        else if ( stl_y > map_subtiles_y )
        {
            stl_y = map_subtiles_y;
        }

        mapblk = get_map_block_at(stl_x, stl_y);
        
        if ( (mapblk->flags & SlbAtFlg_Blocking) == 0 )
        {
            spiral_pos.x.val = (stl_x << 8) + 128;
            spiral_pos.y.val = (stl_y << 8) + 128;
            spiral_pos.z.val = get_thing_height_at(thing, &spiral_pos);
            if ( !thing_in_wall_at(thing, &spiral_pos) )
            {
                pos->x.val = spiral_pos.x.val;
                pos->y.val = spiral_pos.y.val;
                pos->z.val = spiral_pos.z.val;
                return true;
            }
        }
    }

    ERRORLOG("Cannot find valid position near %d, %d to place thing", pos->x.stl.num, pos->y.stl.num);
    return false;

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
    if (thing->index == 0)
    {
        ERRORLOG("Moving deleted object (from %s)", func_name);
    }
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
    thing->floor_height = get_thing_height_at(thing, &thing->mappos);
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

TbBool creature_can_head_for_room(struct Thing *thing, struct Room *room, int flags)
{
    struct Coord3d pos;
    return find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos)
        && creature_can_navigate_to_with_storage(thing, &pos, flags);
}

long creature_turn_to_face(struct Thing *thing, const struct Coord3d *pos)
{
    //TODO enable when issue in pathfinding is solved
    /*if (get_2d_box_distance(&thing->mappos, pos) <= 0)
        return -1;*/
    long angle = get_angle_xy_to(&thing->mappos, pos);

    return creature_turn_to_face_angle(thing,angle);
}

long creature_turn_to_face_backwards(struct Thing *thing, struct Coord3d *pos)
{
    //TODO enable when issue in pathfinding is solved
    /*if (get_2d_box_distance(&thing->mappos, pos) <= 0)
        return -1;*/

    long angle = (get_angle_xy_to(&thing->mappos, pos)
        + LbFPMath_PI) & LbFPMath_AngleMask;

    return creature_turn_to_face_angle(thing,angle);
}

long creature_turn_to_face_angle(struct Thing *thing, long angle)
{

    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    long angle_diff = get_angle_difference(thing->move_angle_xy, angle);
    long angle_sign = get_angle_sign(thing->move_angle_xy, angle);
    int angle_delta = crstat->max_angle_change;

    if (angle_delta > angle_diff) {
        angle_delta = angle_diff;
    }
    if (angle_sign < 0) {
        angle_delta = -angle_delta;
    }

    thing->move_angle_xy = (thing->move_angle_xy + angle_delta) & LbFPMath_AngleMask;

    return get_angle_difference(thing->move_angle_xy, angle);
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
        // If continuing the job, check for job stress. Several - but not all - jobs use the move_to_position function.
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

long get_next_gap_creature_can_fit_in_below_point_new(struct Thing *thing, struct Coord3d *pos)
{

    unsigned int x;
    unsigned int v11;
    char bitfields;
    unsigned int v13;
    unsigned int v14;
    unsigned int v18;
    char v19;
    unsigned int v20;
    unsigned int v21;
    unsigned int k;
    unsigned int v23;
    unsigned int v26;
    char v27;
    unsigned int v28;
    unsigned int v31;
    char v32;
    unsigned int v33;
    int v34;
    unsigned int floor_height;
    unsigned int v38;
    unsigned int v39;
    unsigned int v40;
    unsigned int v41;
    unsigned int y;
    int v43;

    MapCoordDelta clipbox_size_xy;
    if (thing_is_creature(thing))
        clipbox_size_xy = thing_nav_sizexy(thing);
    else
        clipbox_size_xy = thing->clipbox_size_xy;

    MapCoordDelta nav_radius = clipbox_size_xy / 2;

    MapCoord start_x = pos->x.val - nav_radius;
    if (start_x < 0)
        start_x = 0;
    MapCoord start_y = pos->y.val - nav_radius;
    if (start_y < 0)
        start_y = 0;
    MapCoord end_x = nav_radius + pos->x.val;
    v43 = start_y;
    if (end_x > map_subtiles_x * COORD_PER_STL - 1)
        end_x = map_subtiles_x * COORD_PER_STL - 1;
    MapCoord end_y = pos->y.val + nav_radius;
    if (end_y > map_subtiles_y * COORD_PER_STL - 1)
        end_y = map_subtiles_y * COORD_PER_STL - 1;
    floor_height = 0;
    v40 = 15;
    y = start_y;
    if (end_y > start_y)
    {
        x = start_x;
        if (end_x > start_x)
            goto LABEL_18;
        while (1)
        {
            y += 256;
            if (y >= end_y)
                 break;
            for (x = start_x; x < end_x; x += 256)
            {
            LABEL_18:
                 1 + 1;
                 struct Map *mapblk = get_map_block_at(x / COORD_PER_STL, y / COORD_PER_STL);
                 struct Column *col = get_map_column(mapblk);
                 v11 = (unsigned __int8)col->bitfields >> 4;
                 if (v11 <= floor_height)
                     v11 = floor_height;
                 floor_height = v11;
                 bitfields = col->bitfields;
                 if ((bitfields & 0xE) != 0)
                 {
                     v13 = 8 - ((unsigned __int8)(16 * bitfields) >> 5);
                     if (v13 > v40)
                         v13 = v40;
                     v40 = v13;
                 }
                 else
                 {
                     v14 = (unsigned int)(16 * mapblk->data) >> 28;
                     if (v14 >= v40)
                         v14 = v40;
                     v40 = v14;
                 }
            }
        }
    }
    for (MapCoord j = v43; j < end_y; j += COORD_PER_STL)
    {
        struct Map *mapblk2 = get_map_block_at(end_x / COORD_PER_STL, j / COORD_PER_STL);
        struct Column *col2 = get_map_column(mapblk2);
        v18 = (unsigned __int8)col2->bitfields >> 4;
        if (v18 <= floor_height)
            v18 = floor_height;
        floor_height = v18;
        v19 = col2->bitfields;
        if ((v19 & 0xE) != 0)
        {
            v20 = 8 - ((unsigned __int8)(16 * v19) >> 5);
            if (v20 > v40)
                 v20 = v40;
            v40 = v20;
        }
        else
        {
            v21 = (unsigned int)(16 * mapblk2->data) >> 28;
            if (v21 >= v40)
                 v21 = v40;
            v40 = v21;
        }
    }
    for (k = start_x; k < end_x; k += 256)
    {
        struct Map *mapblk3 = get_map_block_at(k / COORD_PER_STL, end_y / COORD_PER_STL);
        struct Column *col3 = get_map_column(mapblk3);
        v26 = (unsigned __int8)col3->bitfields >> 4;
        if (v26 <= floor_height)
            v26 = floor_height;
        floor_height = v26;
        v27 = col3->bitfields;
        if ((v27 & 0xE) != 0)
        {
            v28 = 8 - ((unsigned __int8)(16 * v27) >> 5);
            if (v28 > v40)
                 v28 = v40;
            v40 = v28;
        }
        else
        {
            v23 = (unsigned int)(16 * mapblk3->data) >> 28;
            if (v23 >= v40)
                 v23 = v40;
            v40 = v23;
        }
    }
    struct Map *mapblk4 = get_map_block_at(end_x / COORD_PER_STL, end_y / COORD_PER_STL);
    struct Column *col4 = get_map_column(mapblk4);
    v31 = (unsigned __int8)col4->bitfields >> 4;
    if (v31 <= floor_height)
        v31 = floor_height;
    v38 = v31;
    v32 = col4->bitfields;
    if ((v32 & 0xE) != 0)
    {
        v33 = 8 - ((unsigned __int8)(16 * v32) >> 5);
        if (v33 > v40)
            v33 = v40;
    }
    else
    {
        v33 = (unsigned int)(16 * mapblk4->data) >> 28;
        if (v33 >= v40)
            v33 = v40;
    }
    v39 = v38 << 8;
    v41 = v33 << 8;
    if (*(int *)&pos->y.val >> 16 < v39)
        return *(int *)&pos->y.val >> 16;
    v34 = *(unsigned __int16 *)&thing->clipbox_size_yz;
    if (v41 - (unsigned __int16)v34 <= v39)
        return *(int *)&pos->y.val >> 16;
    else
        return v41 - 1 - v34;





    /*
    MapCoordDelta clipbox_size_xy;
    MapSubtlCoord floor_height;
    unsigned int v30;

    if (thing_is_creature(thing))
        clipbox_size_xy = thing_nav_sizexy(thing);
    else
        clipbox_size_xy = thing->clipbox_size_xy;

    MapCoordDelta nav_radius = clipbox_size_xy / 2;

    MapCoord start_x = (pos->x.val - nav_radius) & ((pos->x.val - nav_radius < 0) - 1);
    MapCoord start_y = (pos->y.val - nav_radius) & ((pos->y.val - nav_radius < 0) - 1);

    MapCoord end_x = nav_radius + pos->x.val;
    if (end_x >= map_subtiles_x * COORD_PER_STL - 1)
        end_x = map_subtiles_x * COORD_PER_STL - 1;
    MapCoord end_y = pos->y.val + nav_radius;
    if (pos->y.val + nav_radius >= map_subtiles_y * COORD_PER_STL - 1)
        end_y = map_subtiles_y * COORD_PER_STL - 1;

    floor_height = 0;
    col4 = 15;


    for (MapCoord y = start_y; y < end_y; y += COORD_PER_STL)
    {
        MapCoord x = start_x;
        while (x < end_x)
        {
            struct Map *mapblk = get_map_block_at(x / COORD_PER_STL + 1, y / COORD_PER_STL + 1);
            struct Column *col = get_map_column(mapblk);
            unsigned int v12 = col->bitfields >> 4;
            if (floor_height < v12)
                 floor_height = v12;
            if ((col->bitfields & CLF_CEILING_MASK) != 0)
            {
                 unsigned int v13 = COLUMN_STACK_HEIGHT - ((col->bitfields & CLF_CEILING_MASK) >> 1);
                 if (v13 >= col4)
                     v13 = col4;
                 col4 = v13;
            }
            else
            {
                 unsigned int filled_subtiles = get_mapblk_filled_subtiles(mapblk);
                 if (filled_subtiles >= col4)
                     filled_subtiles = col4;
                 col4 = filled_subtiles;
            }
            x += COORD_PER_STL;
        }
    }

    MapCoord y = start_y;
    while (y < end_y)
    {
        struct Map *mapblk = get_map_block_at(end_x / COORD_PER_STL + 1, y / COORD_PER_STL + 1);
        struct Column *col = get_map_column(mapblk);

        unsigned int v19 = col->bitfields >> 4;
        if (floor_height < v19)
            floor_height = v19;
        if ((col->bitfields & CLF_CEILING_MASK) != 0)
        {
            unsigned int v20 = COLUMN_STACK_HEIGHT - ((col->bitfields & CLF_CEILING_MASK) >> 1);
            if (v20 >= col4)
                 v20 = col4;
            col4 = v20;
        }
        else
        {
            unsigned int filled_subtiles = get_mapblk_filled_subtiles(mapblk);
            if (filled_subtiles >= col4)
                 filled_subtiles = col4;
            col4 = filled_subtiles;
        }
        y += COORD_PER_STL;
    }

    MapCoord x = start_x;
    while (x < end_x)
    {
        struct Map *mapblk = get_map_block_at(x / COORD_PER_STL + 1, end_y / COORD_PER_STL + 1);
        struct Column *col = get_map_column(mapblk);

        unsigned __int8 v24 = col->bitfields >> 4;
        if (floor_height < v24)
            floor_height = v24;
        if ((col->bitfields & CLF_CEILING_MASK) != 0)
        {
            unsigned int v25 = COLUMN_STACK_HEIGHT - ((col->bitfields & CLF_CEILING_MASK) >> 1);
            if (v25 >= col4)
                 v25 = col4;
            col4 = v25;
        }
        else
        {
            unsigned int filled_subtiles = get_mapblk_filled_subtiles(mapblk);
            if (filled_subtiles >= col4)
                 filled_subtiles = col4;
            col4 = filled_subtiles;
        }
        x += COORD_PER_STL;
    }

    
    MapSubtlCoord ceiling_height;
    update_floor_and_ceiling_heights_at(end_x / COORD_PER_STL, end_y / COORD_PER_STL, &floor_height, &ceiling_height);
    floor_height <<= 8;
    col4 <<= 8;
    long result = pos->z.val;
    if (floor_height <= result)
    {
        unsigned int v28 = col4 - thing->clipbox_size_yz;
        if (floor_height < v28)
            return v28 - 1;
    }
    return result;
    */
}

//TODO remove testcode
DLLIMPORT long _DK_get_next_gap_creature_can_fit_in_below_point(struct Thing *creatng, struct Coord3d *pos);
long get_next_gap_creature_can_fit_in_below_point(struct Thing *thing, struct Coord3d *pos)
{
    long old = _DK_get_next_gap_creature_can_fit_in_below_point(thing, pos);
    long new = get_next_gap_creature_can_fit_in_below_point_new(thing, pos);

    if (old == new)
        JUSTLOG("OK %d", old);
    else
        JUSTLOG("NOT_OK %d,%d", old,new);

    return new;
}

TbBool thing_covers_same_blocks_in_two_positions(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2)
{
    long nav_radius = thing_nav_sizexy(thing) /2;

    if ((abs((pos2->x.val - nav_radius) - (pos1->x.val - nav_radius)) < COORD_PER_STL)
     && (abs((pos2->x.val + nav_radius) - (pos1->x.val + nav_radius)) < COORD_PER_STL)
     && (abs((pos2->y.val - nav_radius) - (pos1->y.val - nav_radius)) < COORD_PER_STL)
     && (abs((pos2->y.val + nav_radius) - (pos1->y.val + nav_radius)) < COORD_PER_STL)
     && (abs(pos2->z.val - pos1->z.val) < COORD_PER_STL)
     && (abs((thing->clipbox_size_yz + pos2->z.val) - (thing->clipbox_size_yz + pos1->z.val)) < COORD_PER_STL) )
    {
        return true;
    }
    return false;
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
