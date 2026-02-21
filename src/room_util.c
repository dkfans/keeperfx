/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_util.c
 *     Generic utility and maintain functions for rooms.
 * @par Purpose:
 *     Functions to maintain rooms of various kinds.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 20 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "room_util.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "room_garden.h"
#include "map_utils.h"
#include "map_blocks.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_doors.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "room_list.h"
#include "room_workshop.h"
#include "ariadne_wallhug.h"
#include "config_terrain.h"
#include "config_creature.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "frontend.h"
#include <math.h>
#include "post_inc.h"

/******************************************************************************/
struct Thing *create_room_surrounding_flame(struct Room *room, const struct Coord3d *pos,
    unsigned short eetype, PlayerNumber owner)
{
    struct Thing* eething = create_effect_element(pos, room_effect_elements[eetype], owner);
    if (!thing_is_invalid(eething))
    {
        eething->mappos.z.val = get_thing_height_at(eething, &eething->mappos);
        eething->mappos.z.val += 10;
        // Size of the flame depends on room efficiency
        eething->sprite_size = ((eething->sprite_size - 80) * ((long)room->efficiency) / 256) + 80;
  }
  return eething;
}

void room_update_surrounding_flames(struct Room *room, const struct Coord3d *pos)
{
    long k;
    long i = room->flames_around_idx;
    MapSubtlCoord x = pos->x.stl.num + (MapSubtlCoord)small_around[i].delta_x;
    MapSubtlCoord y = pos->y.stl.num + (MapSubtlCoord)small_around[i].delta_y;
    struct Room* curoom = subtile_room_get(x, y);
    if (curoom->index != room->index)
    {
        k = (i + 1) % SMALL_AROUND_LENGTH;
        room->flames_around_idx = k;
        return;
    }
    k = (i + 3) % SMALL_AROUND_LENGTH;
    x += (MapSubtlCoord)small_around[k].delta_x;
    y += (MapSubtlCoord)small_around[k].delta_y;
    curoom = subtile_room_get(x,y);
    if (curoom->index != room->index)
    {
        room->flame_slb += game.small_around_slab[i];
        return;
    }
    room->flame_slb += game.small_around_slab[i] + game.small_around_slab[k];
    room->flames_around_idx = k;
}

void process_room_surrounding_flames(struct Room *room)
{
    SYNCDBG(19,"Starting");
    if(player_is_roaming(room->owner))
    {
        return;
    }
    MapSlabCoord x = slb_num_decode_x(room->flame_slb);
    MapSlabCoord y = slb_num_decode_y(room->flame_slb);
    long i = 3 * room->flames_around_idx + room->flame_stl;
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(slab_subtile_center(x)) + room_spark_offset[i].delta_x;
    pos.y.val = subtile_coord_center(slab_subtile_center(y)) + room_spark_offset[i].delta_y;
    pos.z.val = 0;
    // Create new element
    if (room->owner == game.neutral_player_num)
    {
      create_room_surrounding_flame(room,&pos,game.play_gameturn & 3,game.neutral_player_num);
    } else
    if (room_effect_elements[get_player_color_idx(room->owner)] != 0)
    {
      create_room_surrounding_flame(room,&pos,get_player_color_idx(room->owner),room->owner);
    }
    // Update coords for next element
    if (room->flame_stl == 2)
    {
      room_update_surrounding_flames(room,&pos);
    }
    room->flame_stl = (room->flame_stl + 1) % 3;
}

void recompute_rooms_count_in_dungeons(void)
{
    SYNCDBG(17,"Starting");
    for (long i = 0; i < DUNGEONS_COUNT; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        dungeon->total_rooms = 0;
        for (RoomKind rkind = 1; rkind < game.conf.slab_conf.room_types_count; rkind++)
        {
            if (!room_never_buildable(rkind))
            {
                dungeon->total_rooms += count_player_rooms_of_type(i, rkind);
            }
        }
    }
}

void process_rooms(void)
{
  SYNCDBG(7,"Starting");
  for (struct Room* room = start_rooms; room < end_rooms; room++)
  {
      if (!room_exists(room))
          continue;
      if (room_role_matches(room->kind, RoRoF_FoodSpawn)) {
          room_grow_food(room);
      }
      if (room_has_surrounding_flames(room->kind) && ((game.view_mode_flags & GNFldD_RoomFlameProcessing) != 0)) {
          process_room_surrounding_flames(room);
      }
  }
  recompute_rooms_count_in_dungeons();
  SYNCDBG(9,"Finished");
}

void kill_all_room_slabs_and_contents(struct Room *room)
{
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i != 0)
    {
        long slb_x = slb_num_decode_x(i);
        long slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        kill_room_slab_and_contents(room->owner, slb_x, slb_y);
        slb->next_in_room = 0;
        slb->room_index = 0;
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    room->slabs_list = 0;
    room->slabs_count = 0;
}

void sell_room_slab_when_no_free_room_structures(struct Room *room, long slb_x, long slb_y, unsigned char gnd_slab)
{
    delete_room_slab_when_no_free_room_structures(slb_x, slb_y, gnd_slab);
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    long revenue = compute_value_percentage(roomst->cost, game.conf.rules[room->owner].game.room_sale_percent);
    if (revenue != 0)
    {
        struct Coord3d pos;
        set_coords_to_slab_center(&pos, slb_x, slb_y);
        create_price_effect(&pos, room->owner, revenue);
        player_add_offmap_gold(room->owner, revenue);
    }
}

void recreate_rooms_from_room_slabs(struct Room *room, unsigned char gnd_slab)
{
    SYNCDBG(7,"Starting for %s index %d",room_code_name(room->kind),(int)room->index);
    // Clear room index in all slabs
    // This will make sure that the old room won't be returned by subtile_room_get()
    // and used as one of new rooms.
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i > 0)
    {
        struct SlabMap* slb = get_slabmap_direct(i);
        if (slabmap_block_invalid(slb))
        {
          ERRORLOG("Jump to invalid item when sweeping Slabs.");
          break;
        }
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        slb->room_index = 0;
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    // Create a new room for every slab
    struct Room* proom = INVALID_ROOM;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        long slb_x = slb_num_decode_x(i);
        long slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        struct Room* nroom = create_room(room->owner, room->kind, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (room_is_invalid(nroom)) // In case of error, sell the whole thing
        {
            ERRORLOG("Room creation failed; selling slabs");
            sell_room_slab_when_no_free_room_structures(room, slb_x, slb_y, gnd_slab);
        } else
        {
            // We may have created a new room, or just added tile to the old one
            // Integrate the room only if we're not adding tiles to old room
            if ((nroom != proom) && room_exists(proom)) {
                do_room_integration(proom);
            }
        }
        proom = nroom;
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    if (room_exists(proom)) {
        do_room_integration(proom);
    }
    // The old room no longer has any slabs
    room->slabs_list = 0;
    room->slabs_count = 0;
}

TbBool delete_room_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, TbBool is_destroyed)
{
    struct Room* room = slab_room_get(slb_x, slb_y);
    if (room_is_invalid(room))
    {
        ERRORLOG("Slab (%d,%d) is not a room",slb_x, slb_y);
        return false;
    }
    SYNCDBG(7,"Room on (%d,%d) had %d slabs",(int)slb_x,(int)slb_y,(int)room->slabs_count);
    decrease_room_area(room->owner, 1);
    kill_room_slab_and_contents(room->owner, slb_x, slb_y);
    if (room->slabs_count <= 1)
    {
        delete_room_flag(room);
        replace_room_slab(room, slb_x, slb_y, room->owner, is_destroyed);
        kill_all_room_slabs_and_contents(room);
        free_room_structure(room);
        do_slab_efficiency_alteration(slb_x, slb_y);
    } else
    {
        // Remove the slab from room tiles list
        remove_slab_from_room_tiles_list(room, slb_x, slb_y);
        replace_room_slab(room, slb_x, slb_y, room->owner, is_destroyed);
        // Create a new room from slabs left in old one
        recreate_rooms_from_room_slabs(room, is_destroyed);
        reset_creatures_rooms(room);
        free_room_structure(room);
    }
    return true;
}

TbBool replace_slab_from_script(MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char slabkind)
{
    struct Room* room = slab_room_get(slb_x, slb_y);
    struct SlabMap* slb = get_slabmap_for_subtile(slab_subtile(slb_x, 0), slab_subtile(slb_y, 0));
    short plyr_idx = slabmap_owner(slb);
    if (slab_kind_has_no_ownership(slabkind))
    {
        plyr_idx = game.neutral_player_num;
    }
    RoomKind rkind = slab_corresponding_room(slabkind);
    //When the slab to be replaced does not have a room yes, simply place the room/slab.
    if (room_is_invalid(room))
    {
        // If we're looking to place a non-room slab, simply place it.
        if (rkind == 0)
        {
            if (slab_kind_is_animated(slabkind))
            {
                place_animating_slab_type_on_map(slabkind, 0, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0), plyr_idx);
            }
            else
            {
                place_slab_type_on_map(slabkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0), plyr_idx, 0);
                set_alt_bit_on_slabs_around(slb_x, slb_y);
            }
            return true;
        }
        else
        {
            // Create the new one-slab room
            if (place_room(plyr_idx, rkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0)))
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        if (rkind == 0)
        {
            delete_room_slab(slb_x, slb_y, 0);
            if (slab_kind_is_animated(slabkind))
            {
                place_animating_slab_type_on_map(slabkind, 0, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0), plyr_idx);
            }
            else
            {
                place_slab_type_on_map(slabkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0), plyr_idx, 0);
            }
        }
        else
        {
            // Create a new one-slab room
            place_room(plyr_idx, rkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0));
        }
    }
    return true;
}

void change_slab_owner_from_script(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_owner(slb) == plyr_idx)
        return;
    if (slb->room_index)
    {
        struct Room* room = room_get(slb->room_index);
        if (room_exists(room))
        {
            take_over_room(room, plyr_idx);
        }
    } else
    {
        SlabKind slbkind = (slb->kind == SlbT_PATH) ? SlbT_CLAIMED : slb->kind;
        if (slab_kind_has_no_ownership(slbkind) == false)
        {
            if (slab_kind_is_door(slbkind))
            {
                MapSubtlCoord stl_x = slab_subtile_center(slb_x);
                MapSubtlCoord stl_y = slab_subtile_center(slb_y);
                struct Thing* doortng = get_door_for_position(stl_x, stl_y);
                if (!thing_is_invalid(doortng))
                {
                    game.dungeon[doortng->owner].total_doors--;
                    remove_key_on_door(doortng);
                    set_slab_owner(slb_x, slb_y, plyr_idx);
                    place_animating_slab_type_on_map(slbkind, doortng->door.closing_counter / 256, stl_x, stl_y, plyr_idx);
                    doortng->owner = plyr_idx;
                    game.dungeon[doortng->owner].total_doors++;
                    if (doortng->door.is_locked)
                    {
                        add_key_on_door(doortng);
                    }
                    update_navigation_triangulation(stl_x-1,  stl_y-1, stl_x+1,stl_y+1);
                }
            }
            else if (slab_kind_is_animated(slbkind))
            {
                place_animating_slab_type_on_map(slbkind, 0, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0), plyr_idx);
            }
            else
            {
                place_slab_type_on_map(slbkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0), plyr_idx, 0);
            }
            do_slab_efficiency_alteration(slb_x, slb_y);
        }
    }
}

/**
 * Updates thing interaction with rooms. Sometimes deletes the given thing.
 * @param thing Thing to be checked, and assimilated or deleted.
 * @note Used capacity of the room don't have to be updated here, as it is re-computed later.
 * @return True if the thing was either assimilated or left intact, false if it was deleted.
 */
TbBool check_and_asimilate_thing_by_room(struct Thing *thing)
{
    struct Room *room;
    if (thing_is_dragged_or_pulled(thing))
    {
        ERRORLOG("It shouldn't be possible to drag %s during initial asimilation",thing_model_name(thing));
        thing->owner = game.neutral_player_num;
        return true;
    }
    if (thing_is_gold_hoard(thing))
    {
        room = get_room_thing_is_on(thing);
        GoldAmount wealth_size_holds = game.conf.rules[room->owner].game.gold_per_hoard / get_wealth_size_types_count();
        GoldAmount gold_value = thing->valuable.gold_stored;
        if (gold_value == 0)
        {
            gold_value = wealth_size_holds* max(1, get_wealth_size_of_gold_hoard_object(thing));
        }
        else
        {
            thing->valuable.gold_stored = 0;
        }
        GoldAmount value_left;
        GoldAmount value_added;
        if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_GoldStorage))
        {
            // No room - delete it, hoard cannot exist outside treasure room
            ERRORLOG("Found %s outside of %s room; removing",thing_model_name(thing),room_role_code_name(RoRoF_GoldStorage));
            create_gold_pile(&thing->mappos, thing->owner, gold_value);
            delete_thing_structure(thing, 0);
            return false;
        }
        MapSubtlCoord stl_x = thing->mappos.x.stl.num - 1;
        MapSubtlCoord stl_y = thing->mappos.y.stl.num - 1;
        if (!((stl_x % 3) || (stl_y % 3))) // Only accept hoards on a center subtile.
        {
            thing->owner = room->owner;
            value_added = add_gold_to_hoarde(thing, room, gold_value);
            value_left = gold_value - value_added;
            if (value_left > 0)
            {
                create_gold_pile(&thing->mappos, thing->owner, value_left);
            }
            return true;
        }
        create_gold_pile(&thing->mappos, thing->owner, gold_value);
        delete_thing_structure(thing, 0);
        return false;
    }
    if (thing_is_spellbook(thing))
    {
        room = get_room_thing_is_on(thing);
        if (room->owner != game.neutral_player_num)
        {
            if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_PowersStorage) || (!player_exists(get_player(room->owner)) && (game.play_gameturn >= 10)))
            {
                // No room - oh well, leave it as free spell
                if (((game.conf.rules[room->owner].game.classic_bugs_flags & ClscBug_ClaimRoomAllThings) != 0) && !room_is_invalid(room)) {
                    // Preserve classic bug - object is claimed with the room
                    thing->owner = room->owner;
                }
                else {
                    // Make correct owner so that Imps can pick it up
                    thing->owner = game.neutral_player_num;
                    return false;
                }
                return false;
            }
            if (!add_power_to_player(book_thing_to_power_kind(thing), room->owner))
            {
                thing->owner = game.neutral_player_num;
                return false;
            }
            else
            {
                thing->owner = room->owner;
                return true;
            }
        }
        thing->owner = room->owner;
        return false;
    }
    if (thing_is_workshop_crate(thing))
    {
        room = get_room_thing_is_on(thing);
        if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_CratesStorage) || !player_exists(get_player(room->owner)))
        {
            // No room - oh well, leave it as free box
            if (((game.conf.rules[room->owner].game.classic_bugs_flags & ClscBug_ClaimRoomAllThings) != 0) && !room_is_invalid(room)) {
                // Preserve classic bug - object is claimed with the room
                thing->owner = room->owner;
            } else {
                // Make correct owner so that Imps can pick it up
                thing->owner = game.neutral_player_num;
            }
            return true;
        }
        if (!add_workshop_item_to_amounts(room->owner, crate_thing_to_workshop_item_class(thing), crate_thing_to_workshop_item_model(thing)))
        {
            thing->owner = game.neutral_player_num;
            return true;
        }
        thing->owner = room->owner;
        return true;
    }
    return false;
}

EventIndex update_cannot_find_room_of_role_wth_spare_capacity_event(PlayerNumber plyr_idx, struct Thing *creatng, RoomRole rrole)
{
    EventIndex evidx = 0;
    if (player_has_room_of_role(plyr_idx, rrole))
    {
        // Could not find room to send thing - either no capacity or not navigable
        struct Room *room;
        switch (rrole)
        {
        case RoRoF_LairStorage:
        case RoRoF_CrHealSleep:
            // Find room with lair capacity
            {
                struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
                room = find_room_of_role_with_spare_capacity(plyr_idx, rrole, crconf->lair_size);
                break;
            }
        // For Treasure rooms, the item capacity is the amount of gold, not the number of gold hoardes.
        case RoRoF_CratesStorage:
        case RoRoF_PowersStorage:
            // Find room with item capacity
            room = find_room_of_role_with_spare_room_item_capacity(plyr_idx, rrole);
            break;
        default:
            // Find room with worker capacity
            room = find_room_of_role_with_spare_capacity(plyr_idx, rrole, 1);
            break;
        }
        if (room_is_invalid(room))
        {
            SYNCDBG(5,"Player %d has %s which cannot find large enough %s",(int)plyr_idx,thing_model_name(creatng),room_role_code_name(rrole));
            switch (rrole)
            {
            case RoRoF_LairStorage:
                evidx = event_create_event_or_update_nearby_existing_event(creatng->mappos.x.val, creatng->mappos.y.val, EvKind_NoMoreLivingSet, plyr_idx, creatng->index);
                break;
            case RoRoF_GoldStorage:
                evidx = event_create_event_or_update_nearby_existing_event(0, 0, EvKind_TreasureRoomFull, plyr_idx, 0);
                break;
            default:
                evidx = 1;
                break;
            }
            if (evidx > 0) {
                output_room_message(plyr_idx, find_first_roomkind_with_role(rrole), OMsg_RoomTooSmall);
            }
        } else
        {
            SYNCDBG(5,"Player %d has %s which cannot reach %s",(int)plyr_idx,thing_model_name(creatng),room_role_code_name(rrole));
            RoomKind rkind = find_first_roomkind_with_role(rrole);
            evidx = event_create_event_or_update_nearby_existing_event(
                creatng->mappos.x.val, creatng->mappos.y.val, EvKind_WorkRoomUnreachable, plyr_idx, rkind);
            if (evidx > 0) {
                output_room_message(plyr_idx, rkind, OMsg_RoomNoRoute);
            }
        }
    } else
    {
        // We simply don't have the room of that kind
        if (rrole == RoRoF_LairStorage || rrole == RoRoF_GoldStorage || is_room_of_role_available(plyr_idx, rrole))
        {
            switch (rrole)
            {
            case RoRoF_LairStorage:
                evidx = event_create_event_or_update_nearby_existing_event(creatng->mappos.x.val, creatng->mappos.y.val, EvKind_NoMoreLivingSet, plyr_idx, creatng->index);
                break;
            case RoRoF_GoldStorage:
                evidx = event_create_event_or_update_nearby_existing_event(0, 0, EvKind_NeedTreasureRoom, plyr_idx, 0);
                break;
            default:
                evidx = 1;
                break;
            }
            if (evidx > 0) {
                output_room_message(plyr_idx, find_first_roomkind_with_role(rrole), OMsg_RoomNeeded);
            }
        }
    }
    return evidx;
}

void query_room(struct Room *room)
{
    char title[26] = "";
    const char* name = room_code_name(room->kind);
    char owner[26] = "";
    char health[26] = "";
    char capacity[26] = "";
    char efficiency[26] = "";
    snprintf(title, sizeof(title), "Room ID: %d", room->index);
    snprintf(owner, sizeof(owner), "Owner: %d", room->owner);
    snprintf(health, sizeof(health), "Health: %d", (int)room->health);
    snprintf(capacity, sizeof(capacity), "Capacity: %d/%d", room->used_capacity, room->total_capacity);
    float room_efficiency_percent = ((float)room->efficiency / (float)ROOM_EFFICIENCY_MAX) * 100;
    snprintf(efficiency, sizeof(efficiency), "Efficiency: %d", (unsigned char)round(room_efficiency_percent));
    create_message_box((const char*)&title, name, (const char*)&owner, (const char*)&health, (const char*)&capacity, (const char*)&efficiency);
}

/******************************************************************************/
