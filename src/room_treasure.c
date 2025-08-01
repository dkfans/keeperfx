/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_garden.c
 *     Treasure room maintain functions.
 * @par Purpose:
 *     Functions to create and use Treasure Rooms.
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

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
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
void count_gold_slabs_wth_effcncy(struct Room *room)
{
    // Compute max size of gold hoard stored on one slab
    long subefficiency = (get_wealth_size_types_count() * (long)room->efficiency) / ROOM_EFFICIENCY_MAX;
    // Every slab is always capable of storing at least smallest hoard
    if (subefficiency < 1)
        subefficiency = 1;
    unsigned long count = room->slabs_count * subefficiency;
    if (count < 1)
        count = 1;
    room->total_capacity = count;
}

void count_gold_slabs_full(struct Room *room)
{
    room->total_capacity = room->slabs_count * get_wealth_size_types_count();
}

void count_gold_slabs_div2(struct Room* room)
{
    room->total_capacity = room->slabs_count * get_wealth_size_types_count() / 2;
}

struct Thing *find_gold_hoarde_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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
        if (thing_is_object(thing) && object_is_gold_hoard(thing))
        {
            return thing;
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

struct Thing *treasure_room_eats_gold_piles(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y, struct Thing *hoardtng)
{
    if (room->owner == game.neutral_player_num) {
        return hoardtng;
    }
    GoldAmount gold_gathered = 0;
    // Find gold objects around, delete them and gather sum of the gold they had
    for (long k = 0; k < AROUND_TILES_COUNT; k++)
    {
        MapSubtlCoord stl_x = slab_subtile(slb_x, around[k].delta_x + 1);
        MapSubtlCoord stl_y = slab_subtile(slb_y, around[k].delta_y + 1);
        struct Map* mapblk = get_map_block_at(stl_x, stl_y);
        unsigned long j = 0;
        for (int i = get_mapwho_thing_index(mapblk); i != 0;)
        {
            struct Thing* gldtng = thing_get(i);
            i = gldtng->next_on_mapblk;
            if (!thing_is_invalid(gldtng) && object_is_gold_pile(gldtng))
            {
                gold_gathered += gldtng->valuable.gold_stored; 
                delete_thing_structure(gldtng, 0);
            }
            j++;
            if (j > THINGS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping things list");
                break_mapwho_infinite_chain(mapblk);
                break;
            }
        }
    }
    if (gold_gathered <= 0) {
        return hoardtng;
    }
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
    pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
    pos.z.val = get_floor_height_at(&pos);
    // Either create a hoard or add gold to existing one
    if (!thing_is_invalid(hoardtng))
    {
        gold_gathered -= add_gold_to_hoarde(hoardtng, room, gold_gathered);
    } else
    {
        hoardtng = create_gold_hoarde(room, &pos, gold_gathered);
        if (!thing_is_invalid(hoardtng)) {
            gold_gathered -= hoardtng->valuable.gold_stored;
        }
    }
    // If there's still gold left, just drop it as pile
    if (gold_gathered > 0)
    {
        drop_gold_pile(gold_gathered, &pos);
    }
    return hoardtng;
}

void count_gold_hoardes_in_room(struct Room *room)
{
    GoldAmount all_gold_amount = 0;
    int all_wealth_size = 0;
    long wealth_size_holds = game.conf.rules[plyr_idx].game.gold_per_hoard / get_wealth_size_types_count();
    GoldAmount max_hoard_size_in_room = wealth_size_holds * room->total_capacity / room->slabs_count;
    // First, set the values to something big; this will prevent logging warnings on add/remove_gold_from_hoarde()
    room->used_capacity = room->total_capacity;
    room->capacity_used_for_storage = room->used_capacity * wealth_size_holds;
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x = slb_num_decode_x(i);
        MapSlabCoord slb_y = slb_num_decode_y(i);
        struct Thing* gldtng = find_gold_hoarde_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        GoldAmount gold_amount;
        if (!thing_is_invalid(gldtng) && (gldtng->valuable.gold_stored > max_hoard_size_in_room))
        {
            struct Coord3d pos;
            pos.x.val = gldtng->mappos.x.val;
            pos.y.val = gldtng->mappos.y.val;
            pos.z.val = gldtng->mappos.z.val;
            long drop_amount = remove_gold_from_hoarde(gldtng, room, gldtng->valuable.gold_stored - max_hoard_size_in_room);
            drop_gold_pile(drop_amount, &pos);
            gold_amount = gldtng->valuable.gold_stored;
        } else
        {
            gldtng = treasure_room_eats_gold_piles(room, slb_x, slb_y, gldtng);
            if (!thing_is_invalid(gldtng))
            {
                gold_amount = gldtng->valuable.gold_stored;
            } else {
                gold_amount = 0;
            }
        }
        if (gold_amount > 0) {
            all_gold_amount += gold_amount;
            all_wealth_size += get_wealth_size_of_gold_amount(gold_amount);
        }

        i = get_next_slab_number_in_room(i);
        k++;
        if (k > game.map_tiles_x * game.map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    room->capacity_used_for_storage = all_gold_amount;
    room->used_capacity = all_wealth_size;
}
/******************************************************************************/
