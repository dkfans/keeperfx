/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_garden.c
 *     Hatchery room maintain functions.
 * @par Purpose:
 *     Functions to create and use garden rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "room_garden.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_physics.h"
#include "game_legacy.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool remove_food_from_food_room_if_possible(struct Thing *thing)
{
    struct Room *room;
    if ( thing->owner == game.neutral_player_num )
    {
        return false;
    }
    if ( thing->food.life_remaining != -1 )
    {
        return false;
    }
    room = get_room_thing_is_on(thing);
    if ( room_is_invalid(room) || !room_role_matches(room->kind,RoRoF_FoodStorage) || room->owner != thing->owner )
    {
        return false;
    }
    if ( room->used_capacity > 0 )
    {
        room->used_capacity--;
    }
    thing->food.life_remaining = game.conf.rules[thing->owner].game.food_life_out_of_hatchery;
    thing->parent_idx = -1;
    return true;
}

short room_grow_food(struct Room *room)
{
    if (room->slabs_count < 1)
    {
        ERRORLOG("Room %s index %d has no slabs",room_code_name(room->kind),(int)room->index);
        return 0;
    }
    if (room->used_capacity > room->total_capacity)
    {
        ERRORLOG("Room %s index %d has too much used capacity: %d/%d", room_code_name(room->kind), (int)room->index, room->used_capacity, room->total_capacity);
        count_food_in_room(room);
    }
    if ((room->used_capacity >= room->total_capacity)
      || game.play_gameturn % ((game.conf.rules[room->owner].rooms.food_generation_speed / room->total_capacity) + 1))
    {
        return 0;
    }
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    unsigned long k;
    long n = PLAYER_RANDOM(room->owner, room->slabs_count);
    SlabCodedCoords slbnum = room->slabs_list;
    for (k = n; k > 0; k--)
    {
        if (slbnum == 0)
            break;
        slbnum = get_next_slab_number_in_room(slbnum);
    }
    if (slbnum == 0) {
        ERRORLOG("Taking random slab (%d/%d) in %s index %d failed - internal inconsistency",(int)n,(int)room->slabs_count,room_code_name(room->kind),(int)room->index);
        slbnum = room->slabs_list;
    }
    for (k = 0; k < room->slabs_count; k++)
    {
        MapSlabCoord slb_x = slb_num_decode_x(slbnum);
        MapSlabCoord slb_y = slb_num_decode_y(slbnum);

        int m = PLAYER_RANDOM(room->owner, STL_PER_SLB * STL_PER_SLB);
        for (int i = 0; i < STL_PER_SLB * STL_PER_SLB; i++)
        {
            MapSubtlCoord stl_x = slab_subtile(slb_x, m % STL_PER_SLB);
            MapSubtlCoord stl_y = slab_subtile(slb_y, m / STL_PER_SLB);
            // Check if there is a food object already
            struct Thing* thing = find_base_thing_on_mapwho(TCls_Object, ObjMdl_ChickenGrowing, stl_x, stl_y);
            if (thing_is_invalid(thing)) {
                thing = find_base_thing_on_mapwho(TCls_Object, ObjMdl_StatueLit, stl_x, stl_y);
            }
            if (thing_is_invalid(thing))
            {
                if ((roomst->storage_height < 0) || (get_floor_filled_subtiles_at(stl_x, stl_y) == roomst->storage_height))
                {
                    return room_create_new_food_at(room, stl_x, stl_y);
                }
            }
            m = (m + 1) % (STL_PER_SLB*STL_PER_SLB);
        }

        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0) {
            slbnum = room->slabs_list;
        }
    }
    ERRORLOG("Could not find valid RANDOM point in room %s index %d",room_code_name(room->kind),(int)room->index);
    return false;
}

TbBool recreate_repositioned_food_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
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
            struct Thing* foodtng = create_object(&pos, rrepos->models[ri], room->owner, -1);
            if (!thing_is_invalid(foodtng))
            {
                rrepos->used--;
                rrepos->models[ri] = 0;
                rrepos->exp_level[ri] = 0;
                return true;
            }
        }
    }
    return false;
}

void reposition_all_food_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
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
        if (thing_is_object(thing))
        {
            ThingModel objkind = thing->model;
            if (object_is_infant_food(thing) || object_is_growing_food(thing) || object_is_mature_food(thing))
            {
                if (!store_reposition_entry(rrepos, objkind)) {
                    WARNLOG("Too many things to reposition in %s.",room_code_name(room->kind));
                }
                delete_thing_structure(thing, 0);
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
}

int check_food_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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
        if (thing_is_object(thing))
        {
            if (object_is_infant_food(thing) || object_is_growing_food(thing) || object_is_mature_food(thing))
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

void count_and_reposition_food_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    int matching_things_at_subtile = check_food_on_subtile_for_reposition_in_room(room, stl_x, stl_y);
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
            reposition_all_food_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        case 0:
            // There are no matching things there, something can be re-created
            recreate_repositioned_food_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        default:
            WARNLOG("Invalid value returned by reposition check");
            break;
        }
    }
}

void count_food_in_room(struct Room *room)
{
    SYNCDBG(17,"Starting for %s index %d",room_code_name(room->kind),(int)room->index);
    struct RoomReposition rrepos;
    init_reposition_struct(&rrepos);
    // Making two loops guarantees that no rrepos things will be lost
    for (long n = 0; n < 2; n++)
    {
        // The correct count should be taken from last sweep
        room->used_capacity = 0;
        room->capacity_used_for_storage = 0;
        unsigned long k = 0;
        unsigned long i = room->slabs_list;
        while (i > 0)
        {
            MapSubtlCoord slb_x = slb_num_decode_x(i);
            MapSubtlCoord slb_y = slb_num_decode_y(i);
            // Per-slab code
            for (long dy = 0; dy < STL_PER_SLB; dy++)
            {
                for (long dx = 0; dx < STL_PER_SLB; dx++)
                {
                    count_and_reposition_food_in_room_on_subtile(room, slab_subtile(slb_x,dx), slab_subtile(slb_y,dy), &rrepos);
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
    }
    SYNCDBG(7,"The %s index %d contains %d food",room_code_name(room->kind),(int)room->index,(int)room->used_capacity);
    if (rrepos.used > 0) {
        ERRORLOG("The %s index %d capacity %d wasn't enough; %d items belonging to player %d dropped",
          room_code_name(room->kind),(int)room->index,(int)room->total_capacity,(int)rrepos.used,(int)room->owner);
    }
    room->capacity_used_for_storage = room->used_capacity;
}

TbBool room_create_new_food_at(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    struct Thing* foodtng = create_object(&pos, ObjMdl_ChickenGrowing, room->owner, -1);
    if (thing_is_invalid(foodtng))
    {
        ERRORLOG("Cannot Create Food!");
        return false;
    }
    foodtng->mappos.z.val = get_thing_height_at(foodtng, &foodtng->mappos);
    if (thing_in_wall_at(foodtng, &foodtng->mappos)) {
        ERRORLOG("Created chicken in a wall");
    }
    int required_cap = get_required_room_capacity_for_object(RoRoF_FoodStorage, foodtng->model, 0);
    room->used_capacity += required_cap;
    foodtng->food.life_remaining = (foodtng->max_frames << 8) / foodtng->anim_speed - 1;
    return true;
}
/******************************************************************************/
