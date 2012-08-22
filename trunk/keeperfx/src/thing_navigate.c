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
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "dungeon_data.h"
#include "ariadne.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned char _DK_creature_can_navigate_to(struct Thing *thing, struct Coord3d *pos, unsigned char no_owner);
DLLIMPORT long _DK_creature_move_to_using_gates(struct Thing *thing, struct Coord3d *pos, short speed, long storage, long a5, unsigned char a6);
DLLIMPORT long _DK_creature_turn_to_face(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_creature_turn_to_face_backwards(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_creature_turn_to_face_angle(struct Thing *thing, long a2);
DLLIMPORT unsigned char _DK_get_nearest_valid_position_for_creature_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT void _DK_nearest_search(long size, long srcx, long srcy, long dstx, long dsty, long *px, long *py);
DLLIMPORT short _DK_setup_person_move_to_position(struct Thing *thing, long stl_x, long stl_y, unsigned char storage);
DLLIMPORT short _DK_move_to_position(struct Thing *thing);
DLLIMPORT long _DK_get_next_gap_creature_can_fit_in_below_point(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_thing_covers_same_blocks_in_two_positions(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2);
DLLIMPORT long _DK_get_thing_blocked_flags_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT void _DK_move_thing_in_map(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT short _DK_setup_person_move_to_coord(struct Thing *thing, struct Coord3d *pos, unsigned char a3);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_can_navigate_to_with_storage(struct Thing *crtng, struct Coord3d *pos, unsigned char storage)
{
    AriadneReturn ret;
    SYNCDBG(18,"Starting");
    ret = ariadne_initialise_creature_route(crtng, pos, get_creature_speed(crtng), storage);
    SYNCDBG(18,"Ariadne returned %d",(int)ret);
    return (ret == AridRet_OK);
}

short setup_person_move_to_coord(struct Thing *thing, struct Coord3d *pos, unsigned char a3)
{
    struct CreatureControl *cctrl;
    struct Coord3d locpos;
    //return _DK_setup_person_move_to_coord(thing, pos, a3);
    cctrl = creature_control_get_from_thing(thing);
    locpos.x.val = subtile_coord_center(pos->x.stl.num);
    locpos.y.val = subtile_coord_center(pos->y.stl.num);
    locpos.z.val = thing->mappos.z.val;
    locpos.z.val = get_thing_height_at(thing, &locpos);
    if ( thing_in_wall_at(thing, &locpos) || !creature_can_navigate_to_with_storage(thing, &locpos, a3) )
    {
        return 0;
    }
    cctrl->field_88 = a3;
    internal_set_thing_state(thing, CrSt_MoveToPosition);
    cctrl->moveto_pos.x.val = locpos.x.val;
    cctrl->moveto_pos.y.val = locpos.y.val;
    cctrl->moveto_pos.z.val = locpos.z.val;
    return 1;
}

/**
 * Returns a hero gate object to which given hero can navigate.
 * @todo HERO_AI It returns first hero door found, not the best one.
 *     Maybe it should find the one he will reach faster, or at least a random one?
 * @param herotng The hero to be able to make it to gate.
 * @return The gate thing, or invalid thing.
 */
struct Thing *find_hero_door_hero_can_navigate_to(struct Thing *herotng)
{
    struct Thing *thing;
    unsigned long k;
    int i;
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per thing code
        if (object_is_hero_gate(thing) && creature_can_navigate_to_with_storage(herotng, &thing->mappos, 0))
            return thing;
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

unsigned char get_nearest_valid_position_for_creature_at(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_get_nearest_valid_position_for_creature_at(thing, pos);
}

void move_thing_in_map(struct Thing *thing, const struct Coord3d *pos)
{
    SYNCDBG(18,"Starting");
    if ((thing->mappos.x.stl.num == pos->x.stl.num) && (thing->mappos.y.stl.num == pos->y.stl.num))
    {
        thing->mappos.x.val = pos->x.val;
        thing->mappos.y.val = pos->y.val;
        thing->mappos.z.val = pos->z.val;
    } else
    {
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

void nearest_search(long size, long srcx, long srcy, long dstx, long dsty, long *px, long *py)
{
    _DK_nearest_search(size, srcx, srcy, dstx, dsty, px, py);
}

void get_nearest_navigable_point_for_thing(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, unsigned char a4)
{
    long nav_sizexy;
    long px, py;
    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(thing);
    if ( a4 )
    {
        owner_player_navigating = -1;
    } else
    {
        owner_player_navigating = thing->owner;
    }
    nav_sizexy = thing_nav_block_sizexy(thing) - 1;
    nearest_search(nav_sizexy, thing->mappos.x.val, thing->mappos.y.val,
      pos1->x.val, pos1->y.val, &px, &py);
    pos2->x.val = px;
    pos2->y.val = py;
    pos2->z.val = get_thing_height_at(thing, pos2);
    if (thing_in_wall_at(thing, pos2))
        get_nearest_valid_position_for_creature_at(thing, pos2);
    nav_thing_can_travel_over_lava = 0;
}

TbBool setup_person_move_to_position(struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char a4)
{
    struct CreatureControl *cctrl;
    struct Coord3d pos;
    SYNCDBG(18,"Starting");
    //return _DK_setup_person_move_to_position(thing, stl_x, stl_y, a4);
    pos.x.stl.num = stl_x;
    pos.x.stl.pos = 128;
    pos.y.stl.num = stl_y;
    pos.y.stl.pos = 128;
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        SYNCDBG(16,"The %s would be trapped in wall at (%ld,%ld)",thing_model_name(thing),stl_x,stl_y);
        return false;
    }
    if (!creature_can_navigate_to_with_storage(thing, &pos, a4))
    {
        SYNCDBG(19,"The %s cannot reach subtile (%ld,%ld)",thing_model_name(thing),stl_x,stl_y);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        WARNLOG("Tried to move invalid creature to (%ld,%ld)",stl_x,stl_y);
        return false;
    }
    cctrl->field_88 = a4;
    internal_set_thing_state(thing, CrSt_MoveToPosition);
    memcpy(&cctrl->moveto_pos,&pos,sizeof(struct Coord3d));
    SYNCDBG(19,"Done");
    return true;
}

TbBool setup_person_move_close_to_position(struct Thing *thing, long x, long y, unsigned char a4)
{
    struct CreatureControl *cctrl;
    struct Coord3d trgpos;
    struct Coord3d navpos;

    cctrl = creature_control_get_from_thing(thing);
    trgpos.x.stl.num = x;
    trgpos.y.stl.num = y;
    trgpos.x.stl.pos = 128;
    trgpos.y.stl.pos = 128;
    get_nearest_navigable_point_for_thing(thing, &trgpos, &navpos, a4);
    if (!creature_can_navigate_to_with_storage(thing, &navpos, a4))
    {
        return false;
    }
    cctrl->field_88 = a4;
    internal_set_thing_state(thing, CrSt_MoveToPosition);
    cctrl->moveto_pos.x.val = navpos.x.val;
    cctrl->moveto_pos.y.val = navpos.y.val;
    cctrl->moveto_pos.z.val = navpos.z.val;
    return true;
}

TbBool setup_person_move_backwards_to_position(struct Thing *thing, long stl_x, long stl_y, unsigned char storage)
{
    struct CreatureControl *cctrl;
    struct Coord3d pos;
    SYNCDBG(18,"Moving %s index %d to (%d,%d)",thing_model_name(thing),(int)thing->index,(int)stl_x,(int)stl_y);
    cctrl = creature_control_get_from_thing(thing);
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos) || !creature_can_navigate_to_with_storage(thing, &pos, storage))
    {
      return false;
    }
    cctrl->field_88 = storage;
    internal_set_thing_state(thing, CrSt_MoveBackwardsToPosition);
    cctrl->moveto_pos.x.val = pos.x.val;
    cctrl->moveto_pos.y.val = pos.y.val;
    cctrl->moveto_pos.z.val = pos.z.val;
    return true;
}

TbBool setup_person_move_backwards_to_coord(struct Thing *thing, struct Coord3d *pos, unsigned char a4)
{
    return setup_person_move_backwards_to_position(thing, pos->x.stl.num, pos->y.stl.num, a4);
}

TbBool creature_can_travel_over_lava(const struct Thing *thing)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return (crstat->hurt_by_lava <= 0) || ((thing->movement_flags & TMvF_Flying) != 0);
}

TbBool creature_can_navigate_to(struct Thing *thing, struct Coord3d *pos, TbBool no_owner)
{
    SYNCDBG(17,"Starting");
    //result = _DK_creature_can_navigate_to(thing, pos, no_owner);
    struct Path path;
    long nav_sizexy;
    memset(&path, 0, sizeof(struct Path));
    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(thing);
    if (no_owner)
      owner_player_navigating = -1;
    else
      owner_player_navigating = thing->owner;
    nav_sizexy = thing_nav_block_sizexy(thing) - 1;
    path_init8_wide(&path, thing->mappos.x.val, thing->mappos.y.val,
        pos->x.val, pos->y.val, -2, nav_sizexy);
    nav_thing_can_travel_over_lava = 0;
    SYNCDBG(17,"Finished, %ld waypoints",(long)path.waypoints_num);
    return (path.waypoints_num > 0);
}

/**
 * Returns if a creature can get to given players dungeon.
 * @todo This function assumes that getting to players dungeon is equal
 *   to getting to his dungeon heart. It should be later modified, so that
 *   other rooms may also be treated as players dungeon.
 * @param thing
 * @param plyr_idx
 * @return
 */
TbBool creature_can_get_to_dungeon(struct Thing *thing, long plyr_idx)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *heartng;
    SYNCDBG(18,"Starting");
    player = get_player(plyr_idx);
    if (!player_exists(player) || (player->field_2C != 1))
    {
        //WARNLOG("Cannot navigate to inactive player");
        return false;
    }
    dungeon = get_dungeon(player->id_number);
    heartng = NULL;
    if (!dungeon_invalid(dungeon))
        heartng = thing_get(dungeon->dnheart_idx);
    if (thing_is_invalid(heartng))
    {
        //WARNLOG("Cannot navigate to player without heart");
        return false;
    }
    if (heartng->active_state == 3)
        return false;
    return  creature_can_navigate_to(thing, &heartng->mappos, 0);
}

long creature_turn_to_face(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_creature_turn_to_face(thing, pos);
}

long creature_turn_to_face_backwards(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_creature_turn_to_face_backwards(thing, pos);
}

long creature_turn_to_face_angle(struct Thing *thing, long a2)
{
    return _DK_creature_turn_to_face_angle(thing, a2);
}

long creature_move_to_using_gates(struct Thing *thing, struct Coord3d *pos, MoveSpeed speed, long a4, long a5, TbBool backward)
{
    struct CreatureControl *cctrl;
    struct Coord3d nextpos;
    AriadneReturn follow_result;
    long i;
    SYNCDBG(18,"Starting to move thing %d into (%d,%d)",(int)thing->index,(int)pos->x.stl.num,(int)pos->y.stl.num);
    //return _DK_creature_move_to_using_gates(thing, pos, speed, a4, a5, backward);
    cctrl = creature_control_get_from_thing(thing);
    if ( backward )
    {
        i = (thing->field_52 + LbFPMath_PI);
        thing->field_52 = i & LbFPMath_AngleMask;
    }
    follow_result = creature_follow_route_to_using_gates(thing, pos, &nextpos, speed, a5);
    SYNCDBG(18,"Route result: %d, next pos (%d,%d)",(int)follow_result,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num);
    if ( backward )
    {
        i = (thing->field_52 + LbFPMath_PI);
        thing->field_52 = i & LbFPMath_AngleMask;
    }
    if ((follow_result == AridRet_PartOK) || (follow_result == AridRet_Val2))
    {
        creature_set_speed(thing, 0);
        return -1;
    }
    if (follow_result == AridRet_Val1)
    {
        return  1;
    }
    if ( backward )
    {
        if ( creature_turn_to_face_backwards(thing, &nextpos) )
        {
            creature_set_speed(thing, 0);
        } else
        {
            creature_set_speed(thing, -speed);
            cctrl->field_2 |= 0x01;
            cctrl->pos_BB.x.val = (long)nextpos.x.val - (long)thing->mappos.x.val;
            cctrl->pos_BB.y.val = (long)nextpos.y.val - (long)thing->mappos.y.val;
            cctrl->pos_BB.z.val = 0;
        }
        SYNCDBG(18,"Backward target set");
    } else
    {
        if ( creature_turn_to_face(thing, &nextpos) )
        {
            creature_set_speed(thing, 0);
        } else
        {
            creature_set_speed(thing, speed);
            cctrl->field_2 |= 0x01;
            cctrl->pos_BB.x.val = (long)nextpos.x.val - (long)thing->mappos.x.val;
            cctrl->pos_BB.y.val = (long)nextpos.y.val - (long)thing->mappos.y.val;
            cctrl->pos_BB.z.val = 0;
        }
        SYNCDBG(18,"Forward target set");
    }
    return 0;
}

long creature_move_to(struct Thing *thing, struct Coord3d *pos, MoveSpeed speed, unsigned char a4, TbBool backward)
{
    return creature_move_to_using_gates(thing, pos, speed, -2, a4, backward);
}

TbBool creature_move_to_using_teleport(struct Thing *thing, struct Coord3d *pos, long walk_speed)
{
    struct CreatureControl *cctrl;
    short destination_valid;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_instance_is_available(thing, CrInst_TELEPORT)
     && creature_instance_has_reset(thing, CrInst_TELEPORT)
     && (cctrl->instance_id == CrInst_NULL))
    {
        // Creature can only be teleported to a revealed location
        destination_valid = true;
        if ((thing->owner != game.hero_player_num) && (thing->owner != game.neutral_player_num)) {
            destination_valid = subtile_revealed(pos->x.stl.num, pos->y.stl.num, thing->owner);
        }
        if (destination_valid)
         {
             // Use teleport only over large enough distances
             if (get_2d_box_distance(&thing->mappos, pos) > (game.min_distance_for_teleport << 8))
             {
                 set_creature_instance(thing, CrInst_TELEPORT, 1, 0, pos);
                 return true;
             }
         }
    }
    return false;
}

short move_to_position(struct Thing *thing)
{
    Thing_State_Func callback;
    struct CreatureControl *cctrl;
    struct StateInfo *stati;
    long move_result,state_result;
    long speed;
    long i;
    SYNCDBG(18,"Starting for thing %d",(int)thing->index);
    //return _DK_move_to_position(thing);
    cctrl = creature_control_get_from_thing(thing);
    speed = get_creature_speed(thing);
    // Try teleporting the creature
    if (creature_move_to_using_teleport(thing, &cctrl->moveto_pos, speed)) {
        return 1;
    }
    move_result = creature_move_to_using_gates(thing, &cctrl->moveto_pos, speed, -2, cctrl->field_88, 0);
    state_result = 0;
    stati = get_thing_continue_state_info(thing);
    if (!state_info_invalid(stati))
    {
        callback = stati->ofsfield_C;
        if (callback != NULL)
        {
            SYNCDBG(18,"Doing callback");
            state_result = callback(thing);
        }
    }
    if (state_result == 0)
    {
        // If moving was successful
        if (move_result == 1) {
            // Back to "main state"
            internal_set_thing_state(thing, thing->continue_state);
            return 1;
        }
        // If moving failed, do a reset
        if (move_result == -1) {
            i = thing->continue_state;
            internal_set_thing_state(thing, thing->continue_state);
            set_start_state(thing);
            SYNCDBG(8,"Couldn't move %s to place required for state %d; reset to state %d",thing_model_name(thing),(int)i,(int)thing->active_state);
        }
    }
    return state_result;
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
    struct Coord3d locpos;
    unsigned short flags;
    //return _DK_get_thing_blocked_flags_at(thing, pos);
    flags = 0;
    locpos.x.val = pos->x.val;
    locpos.y.val = thing->mappos.y.val;
    locpos.z.val = thing->mappos.z.val;
    if ( thing_in_wall_at(thing, &locpos) )
        flags |= 0x01;
    locpos.x.val = thing->mappos.x.val;
    locpos.y.val = pos->y.val;
    locpos.z.val = thing->mappos.z.val;
    if ( thing_in_wall_at(thing, &locpos) )
        flags |= 0x02;
    locpos.x.val = thing->mappos.x.val;
    locpos.y.val = thing->mappos.y.val;
    locpos.z.val = pos->z.val;
    if ( thing_in_wall_at(thing, &locpos) )
        flags |= 0x04;
    switch ( flags )
    {
    case 0:
        locpos.x.val = pos->x.val;
        locpos.y.val = pos->y.val;
        locpos.z.val = pos->z.val;
        if ( thing_in_wall_at(thing, &locpos) )
            flags = 0x07;
        break;
    case 1:
        locpos.x.val = thing->mappos.x.val;
        locpos.y.val = pos->y.val;
        locpos.z.val = pos->z.val;
        if ( thing_in_wall_at(thing, &locpos) )
            flags = 0x07;
        break;
    case 2:
        locpos.x.val = pos->x.val;
        locpos.y.val = thing->mappos.y.val;
        locpos.z.val = pos->z.val;
        if ( thing_in_wall_at(thing, &locpos) )
            flags = 0x07;
        break;
    case 4:
        locpos.x.val = pos->x.val;
        locpos.y.val = pos->y.val;
        locpos.z.val = thing->mappos.z.val;
        if ( thing_in_wall_at(thing, &locpos) )
          flags = 0x07;
        break;
    }
    return flags;
}

/******************************************************************************/
