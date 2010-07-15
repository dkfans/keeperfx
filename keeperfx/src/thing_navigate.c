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
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "dungeon_data.h"
#include "ariadne.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned char _DK_creature_can_navigate_to(struct Thing *thing, struct Coord3d *pos, unsigned char no_owner);
DLLIMPORT long _DK_creature_move_to_using_gates(struct Thing *thing, struct Coord3d *pos, short a3, long a4, long a5, unsigned char a6);
DLLIMPORT long _DK_creature_turn_to_face(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_creature_turn_to_face_backwards(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_creature_turn_to_face_angle(struct Thing *thing, long a2);
DLLIMPORT unsigned char _DK_get_nearest_valid_position_for_creature_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT void _DK_nearest_search(long size, long srcx, long srcy, long dstx, long dsty, long *px, long *py);
DLLIMPORT short _DK_setup_person_move_to_position(struct Thing *thing, long stl_x, long stl_y, unsigned char a4);
DLLIMPORT short _DK_move_to_position(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_can_navigate_to_with_storage(struct Thing *crtng, struct Coord3d *pos, unsigned char storage)
{
    struct CreatureControl *cctrl;
    AriadneReturn ret;
    SYNCDBG(18,"Starting");
    cctrl = creature_control_get_from_thing(crtng);
    ret = ariadne_initialise_creature_route(crtng, pos, cctrl->max_speed, storage);
    SYNCDBG(18,"Ariadne returned %d",(int)ret);
    return (ret == AridRet_OK);
}

/**
 * Returns a hero gate object to which given hero can navigate.
 * @todo It returns first hero door found, not the best one.
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
    i = game.thing_lists[2].index;
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
      if ((thing->model == 49) && creature_can_navigate_to_with_storage(herotng, &thing->mappos, 0))
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

void nearest_search(long size, long srcx, long srcy, long dstx, long dsty, long *px, long *py)
{
    _DK_nearest_search(size, srcx, srcy, dstx, dsty, px, py);
}

void get_nearest_navigable_point_for_thing(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, unsigned char a4)
{
    struct CreatureStats *crstat;
    long nav_sizexy;
    long px, py;
    crstat = creature_stats_get_from_thing(thing);
    nav_thing_can_travel_over_lava = (!crstat->hurt_by_lava) || ((thing->field_25 & 0x20) != 0);
    if ( a4 )
    {
      owner_player_navigating = -1;
    } else
    {
      owner_player_navigating = thing->owner;
    }
    nav_sizexy = actual_sizexy_to_nav_block_sizexy_table[thing->field_56] - 1;
    nearest_search(nav_sizexy, thing->mappos.x.val, thing->mappos.y.val,
      pos1->x.val, pos1->y.val, &px, &py);
    pos2->x.val = px;
    pos2->y.val = py;
    pos2->z.val = get_thing_height_at(thing, pos2);
    if (thing_in_wall_at(thing, pos2))
        get_nearest_valid_position_for_creature_at(thing, pos2);
    nav_thing_can_travel_over_lava = 0;
}

TbBool setup_person_move_to_position(struct Thing *thing, long stl_x, long stl_y, unsigned char a4)
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
        SYNCDBG(16,"Creature model %d would be trapped in wall at (%ld,%ld)",(int)thing->model,stl_x,stl_y);
        return false;
    }
    if (!creature_can_navigate_to_with_storage(thing, &pos, a4))
    {
        SYNCDBG(19,"Creature cannot reach subtile (%ld,%ld)",stl_x,stl_y);
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

TbBool creature_can_travel_over_lava(struct Thing *thing)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return (crstat->hurt_by_lava <= 0) || ((thing->field_25 & 0x20) != 0);
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
    nav_sizexy = actual_sizexy_to_nav_block_sizexy_table[thing->field_56%(MAX_SIZEXY+1)]-1;
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
    if (heartng->field_7 == 3)
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

void creature_set_speed(struct Thing *thing, short speed)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (speed < -256)
    {
        cctrl->field_C8 = -256;
    } else
    if (speed > 256)
    {
        cctrl->field_C8 = 256;
    } else
    {
        cctrl->field_C8 = speed;
    }
    cctrl->flgfield_1 |= 0x40;
}

long creature_move_to_using_gates(struct Thing *thing, struct Coord3d *pos, short a3, long a4, long a5, unsigned char backward)
{
    struct CreatureControl *cctrl;
    struct Coord3d nextpos;
    AriadneReturn follow_result;
    long i;
    SYNCDBG(18,"Starting to move thing %d into (%d,%d)",(int)thing->index,(int)pos->x.stl.num,(int)pos->y.stl.num);
    //return _DK_creature_move_to_using_gates(thing, pos, a3, a4, a5, backward);
    cctrl = creature_control_get_from_thing(thing);
    if ( backward )
    {
      i = (thing->field_52 + 1024);
      thing->field_52 = i & 0x7FF;
    }
    follow_result = creature_follow_route_to_using_gates(thing, pos, &nextpos, a3, a5);
    SYNCDBG(18,"Route result: %d, next pos (%d,%d)",(int)follow_result,(int)nextpos.x.stl.num,(int)nextpos.y.stl.num);
    if ( backward )
    {
      i = (thing->field_52 + 1024);
      thing->field_52 = i & 0x7FF;
    }
    if ((follow_result == 3) || (follow_result == 2))
    {
        creature_set_speed(thing, 0);
        return -1;
    }
    if (follow_result == 1)
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
            creature_set_speed(thing, -a3);
            cctrl->field_2 |= 0x01;
            cctrl->pos_BB.x.val = nextpos.x.val - thing->mappos.x.val;
            cctrl->pos_BB.y.val = nextpos.y.val - thing->mappos.y.val;
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
          creature_set_speed(thing, a3);
          cctrl->field_2 |= 0x01;
          cctrl->pos_BB.x.val = nextpos.x.val - thing->mappos.x.val;
          cctrl->pos_BB.y.val = nextpos.y.val - thing->mappos.y.val;
          cctrl->pos_BB.z.val = 0;
        }
        SYNCDBG(18,"Forward target set");
    }
    return 0;
}

short move_to_position(struct Thing *thing)
{
    Thing_State_Func callback;
    struct CreatureControl *cctrl;
    struct StateInfo *stati;
    long move_result,state_result;
    long speed;
    SYNCDBG(18,"Starting for thing %d",(int)thing->index);
    //return _DK_move_to_position(thing);
    cctrl = creature_control_get_from_thing(thing);
    speed = cctrl->max_speed;
    if (speed >= 256)
    {
        SYNCDBG(6,"Walk speed %d clipped",(int)speed);
        speed = 256;
    }
    state_result = 0;
    if (creature_instance_is_available(thing, 14)
     && creature_instance_has_reset(thing, 14)
     && (cctrl->field_D2 == 0))
    {
      if ((thing->owner >= game.field_14E496)
       || subtile_revealed(cctrl->moveto_pos.x.stl.num, cctrl->moveto_pos.y.stl.num, thing->owner))
       {
           if (get_2d_box_distance(&thing->mappos, &cctrl->moveto_pos) > (game.min_distance_for_teleport << 8))
           {
             set_creature_instance(thing, 14, 1, 0, &cctrl->moveto_pos);
             return 1;
           }
       }
    }
    move_result = creature_move_to_using_gates(thing, &cctrl->moveto_pos, speed, -2, cctrl->field_88, 0);
    stati = get_thing_state8_info(thing);
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
      if (move_result == 1)
      {
        internal_set_thing_state(thing, thing->field_8);
        thing->field_8 = CrSt_Unused;
        return 1;
      }
      if (move_result == -1)
        set_start_state(thing);
    }
    return state_result;
}
/******************************************************************************/
