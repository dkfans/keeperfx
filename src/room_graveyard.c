/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_graveyard.c
 *     Graveyard room maintain functions.
 * @par Purpose:
 *     Functions to create and use graveyards.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     01 Feb 2012 - 01 Jul 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "room_graveyard.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_corpses.h"
#include "thing_data.h"
#include "thing_physics.h"
#include "thing_stats.h"
#include "config_terrain.h"
#include "game_legacy.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
/**
 * Adds a corpse to the graveyard room capacity.
 * @param corpse The dead creature thing to be added.
 * @param room The graveyard room.
 * @return Gives true on success.
 */
TbBool add_body_to_graveyard(struct Thing *deadtng, struct Room *room)
{
    if (room->total_capacity <= room->used_capacity)
    {
        ERRORLOG("The %s has no space for another corpse",room_code_name(room->kind));
        return false;
    }
    if (corpse_laid_to_rest(deadtng))
    {
        ERRORLOG("The %s is already decomposing in %s",thing_model_name(deadtng),room_role_code_name(RoRoF_DeadStorage));
        return false;
    }
    room->used_capacity++;
    deadtng->corpse.laid_to_rest = 1;
    deadtng->health = game.conf.rules[TODO_SO_ATM_0].rooms.graveyard_convert_time;
    return true;
}

void reposition_all_bodies_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code
        if (corpse_laid_to_rest(thing))
        {
            ThingModel crkind = thing->model;
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            if (!store_creature_reposition_entry(rrepos, crkind, cctrl->exp_level)) {
                WARNLOG("Too many things to reposition in %s.",room_code_name(room->kind));
            }
            delete_thing_structure(thing, 0);
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
}

TbBool rectreate_repositioned_body_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    if ((rrepos->used < 0) || (room->used_capacity >= room->total_capacity)) {
        return false;
    }
    for (int ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] != 0)
        {
            struct Coord3d pos;
            pos.x.val = subtile_coord_center(stl_x);
            pos.y.val = subtile_coord_center(stl_y);
            pos.z.val = 0;
            struct Thing* bodytng = create_dead_creature(&pos, rrepos->models[ri], 0, room->owner, rrepos->exp_level[ri]);
            if (!thing_is_invalid(bodytng))
            {
                bodytng->corpse.laid_to_rest = 1;
                bodytng->health = game.conf.rules[TODO_SO_ATM_0].rooms.graveyard_convert_time;
                rrepos->used--;
                rrepos->models[ri] = 0;
                rrepos->exp_level[ri] = 0;
                return true;
            }
        }
    }
    return false;
}

int check_bodies_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return -2; // do nothing
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    if ((roomst->storage_height >= 0) && (get_map_floor_filled_subtiles(mapblk) != roomst->storage_height)) {
        return -1; // re-create all
    }
    int matching_things_at_subtile = 0;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code
        if (corpse_laid_to_rest(thing))
        {
            // If exceeded capacity of the room
            if (room->used_capacity >= room->total_capacity)
            {
                WARNLOG("The %s capacity %d exceeded; space used is %d",room_code_name(room->kind),(int)room->total_capacity,(int)room->used_capacity);
                return -1; // re-create all (this could save the object if there are duplicates)
            } else
            // If the thing is in wall, remove it but store to re-create later
            if (thing_in_wall_at(thing, &thing->mappos))
            {
                return -1; // re-create all
            } else
            {
                matching_things_at_subtile++;
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return matching_things_at_subtile; // Increase used capacity
}

void count_and_reposition_bodies_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    int matching_things_at_subtile = check_bodies_on_subtile_for_reposition_in_room(room, stl_x, stl_y);
    if (matching_things_at_subtile > 0) {
        // This subtile contains bodies
        SYNCDBG(19,"Got %d matching things at (%d,%d)",(int)matching_things_at_subtile,(int)stl_x,(int)stl_y);
        room->used_capacity += matching_things_at_subtile;
    } else
    {
        switch (matching_things_at_subtile)
        {
        case -2:
            // No matching things, but also cannot recreate anything on this subtile
            break;
        case -1:
            // All matching things are to be removed from the subtile and stored for re-creation
            reposition_all_bodies_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        case 0:
            // There are no matching things there, something can be re-created
            rectreate_repositioned_body_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        default:
            WARNLOG("Invalid value returned by reposition check");
            break;
        }
    }
}

void count_bodies_in_room(struct Room *room)
{
    SYNCDBG(17,"Starting for %s",room_code_name(room->kind));
    struct RoomReposition rrepos;
    init_reposition_struct(&rrepos);
    // Making two loops guarantees that no rrepos things will be lost
    for (long n = 0; n < 2; n++)
    {
        // The correct count should be taken from last sweep
        room->used_capacity = 0;
        //room->capacity_used_for_storage = 0;
        unsigned long k = 0;
        unsigned long i = room->slabs_list;
        while (i > 0)
        {
            MapSlabCoord slb_x = slb_num_decode_x(i);
            MapSlabCoord slb_y = slb_num_decode_y(i);
            // Per-slab code
            for (long dy = 0; dy < STL_PER_SLB; dy++)
            {
                for (long dx = 0; dx < STL_PER_SLB; dx++)
                {
                    count_and_reposition_bodies_in_room_on_subtile(room, slab_subtile(slb_x,dx), slab_subtile(slb_y,dy), &rrepos);
                }
            }
            // Per-slab code ends
            i = get_next_slab_number_in_room(i);
            k++;
            if (k > room->slabs_count)
            {
                ERRORLOG("Infinite loop detected when sweeping room slabs");
                break;
            }
        }
        if (rrepos.used <= 0)
            break;
        if (room->used_capacity >= room->total_capacity)
            break;
    }
    if (rrepos.used > 0) {
        ERRORLOG("The %s index %d capacity %d wasn't enough; %d items belonging to player %d dropped",
          room_code_name(room->kind),(int)room->index,(int)room->total_capacity,(int)rrepos.used,(int)room->owner);
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
