/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_navigate.c
 *     Things movement navigation functions.
 * @par Purpose:
 *     Functions to support move and forrlowing paths by things.
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
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_can_navigate_to_with_storage(struct Thing *crtng, struct Coord3d *pos, unsigned char storage)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(crtng);
    return ariadne_initialise_creature_route(crtng, pos, cctrl->max_speed, storage) == 0;
}

/**
 * Returns a hero gate object to which given hero can navigate.
 * @todo It returns first hero door found - maybe it should find the one he will reach faster?
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

TbBool setup_person_move_to_position(struct Thing *thing, long stl_x, long stl_y, unsigned char a4)
{
    struct CreatureControl *cctrl;
    struct Coord3d pos;
    pos.x.stl.num = stl_x;
    pos.x.stl.pos = 128;
    pos.y.stl.num = stl_y;
    pos.y.stl.pos = 128;
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        WARNLOG("Creature is trapped in wall");
        return false;
    }
    if (!creature_can_navigate_to_with_storage(thing, &pos, a4))
    {
        WARNLOG("Creature cannot reach subtile (%ld,%ld)",stl_x,stl_y);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        WARNLOG("Tried to move invalid creature to (%ld,%ld)",stl_x,stl_y);
        return false;
    }
    cctrl->field_88 = a4;
    internal_set_thing_state(thing, 14);
    memcpy(&cctrl->moveto_pos,&pos,sizeof(struct Coord3d));
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
    return _DK_creature_can_navigate_to(thing, pos, no_owner);
/*
    struct Path path;
    long nav_sizexy;
    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(thing);
    if (no_owner)
      owner_player_navigating = -1;
    else
      owner_player_navigating = thing->owner;
    nav_sizexy = actual_sizexy_to_nav_block_sizexy_table[thing->field_56%(MAX_SIZEXY+1)]-1;
    path_init8_wide(&path, thing->mappos.x.val, thing->mappos.y.val,
        pos->x.val, pos->y.val, -2, nav_sizexy);
    nav_thing_can_travel_over_lava = 0;
    return path.field_10[0];*/
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
    player = get_player(plyr_idx);
    if (((player->field_0 & 0x01) == 0) || (player->field_2C != 1))
        return false;
    dungeon = get_dungeon(player->id_number);
    heartng = thing_get(dungeon->dnheart_idx);
    return  (heartng->field_7 != 3) && creature_can_navigate_to(thing, &heartng->mappos, 0);
}


/******************************************************************************/
