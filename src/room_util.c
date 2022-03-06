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
#include "room_util.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "map_utils.h"
#include "map_blocks.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
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
#include "math.h"

/******************************************************************************/
struct Thing *create_room_surrounding_flame(struct Room *room, const struct Coord3d *pos,
    unsigned short eetype, PlayerNumber owner)
{
    struct Thing* eething = create_effect_element(pos, room_effect_elements[eetype & 7], owner);
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
    long i = room->field_43;
    MapSubtlCoord x = pos->x.stl.num + (MapSubtlCoord)small_around[i].delta_x;
    MapSubtlCoord y = pos->y.stl.num + (MapSubtlCoord)small_around[i].delta_y;
    struct Room* curoom = subtile_room_get(x, y);
    if (curoom->index != room->index)
    {
        k = (i + 1) % 4;
        room->field_43 = k;
        return;
    }
    k = (i + 3) % 4;
    x += (MapSubtlCoord)small_around[k].delta_x;
    y += (MapSubtlCoord)small_around[k].delta_y;
    curoom = subtile_room_get(x,y);
    if (curoom->index != room->index)
    {
        room->field_41 += slab_around[i];
        return;
    }
    room->field_41 += slab_around[i] + slab_around[k];
    room->field_43 = k;
}

void process_room_surrounding_flames(struct Room *room)
{
    SYNCDBG(19,"Starting");
    MapSlabCoord x = slb_num_decode_x(room->field_41);
    MapSlabCoord y = slb_num_decode_y(room->field_41);
    long i = 3 * room->field_43 + room->flame_stl;
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(slab_subtile_center(x)) + room_spark_offset[i].delta_x;
    pos.y.val = subtile_coord_center(slab_subtile_center(y)) + room_spark_offset[i].delta_y;
    pos.z.val = 0;
    // Create new element
    if (room->owner == game.neutral_player_num)
    {
      create_room_surrounding_flame(room,&pos,game.play_gameturn & 3,game.neutral_player_num);
    } else
    if (room_effect_elements[room->owner] != 0)
    {
      create_room_surrounding_flame(room,&pos,room->owner,room->owner);
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
        for (RoomKind rkind = 1; rkind < ROOM_TYPES_COUNT; rkind++)
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
  TbBigChecksum sum = 0;
  for (struct Room* room = start_rooms; room < end_rooms; room++)
  {
      if (!room_exists(room))
          continue;
      if (room_role_matches(room->kind, RoRoF_FoodSpawn)) {
          room_grow_food(room);
      }
      sum += room->slabs_count + room->central_stl_x + room->central_stl_y + room->efficiency + room->used_capacity;
      if (room_has_surrounding_flames(room->kind) && ((game.numfield_D & GNFldD_Unkn40) != 0)) {
          process_room_surrounding_flames(room);
      }
  }
  player_packet_checksum_add(my_player_number, sum, "rooms");
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
    struct RoomStats* rstat = &game.room_stats[room->kind];
    long revenue = compute_value_percentage(rstat->cost, gameadd.room_sale_percent);
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

TbBool delete_room_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char is_destroyed)
{
    struct Room* room = slab_room_get(slb_x, slb_y);
    if (room_is_invalid(room))
    {
        ERRORLOG("Slab (%ld,%ld) is not a room",slb_x, slb_y);
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
    //Otherwise the old room needs modification too.
    SYNCDBG(7, "Room on (%d,%d) had %d slabs", (int)slb_x, (int)slb_y, (int)room->slabs_count);
    decrease_room_area(room->owner, 1);
    kill_room_slab_and_contents(room->owner, slb_x, slb_y);
    if (room->slabs_count <= 1)
    {
        delete_room_flag(room);
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
            }
        }
        else
        {
            // Create a new one-slab room
            place_room(plyr_idx, rkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0));
        }
        if (count_slabs_of_room_type(room->owner, room->kind) <= 1)
        {
            event_create_event_or_update_nearby_existing_event(slb_x, slb_y, EvKind_RoomLost, room->owner, room->kind);
        }
        //Clean up old room
        kill_all_room_slabs_and_contents(room);
        free_room_structure(room);
        do_slab_efficiency_alteration(slb_x, slb_y);
        return true;
    }
    else
    {
        // Remove the slab from room tiles list
        remove_slab_from_room_tiles_list(room, slb_x, slb_y);
        // check again if it's a room or other slab
        if (rkind == 0)
        {
            place_slab_type_on_map(slabkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0), plyr_idx, 0);
        }
        else
        {
            place_room(plyr_idx, rkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0));
        }
        // Create a new room from slabs left in old one
        recreate_rooms_from_room_slabs(room, 0);
        reset_creatures_rooms(room);
        free_room_structure(room);
    }
    return true;
}

void change_slab_owner_from_script(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    if (slb->room_index)
    {
        struct Room* room = room_get(slb->room_index);
        take_over_room(room, plyr_idx);
    } else
    if (slb->kind >= SlbT_WALLDRAPE && slb->kind <= SlbT_CLAIMED) //All slabs that can be owned but aren't rooms
    {
        short slbkind;
        if (slb->kind == SlbT_PATH)
        {
            slbkind = SlbT_CLAIMED;
        }
        else
        {
            slbkind = slb->kind;
        }
        place_slab_type_on_map(slbkind, slab_subtile(slb_x, 0), slab_subtile(slb_y, 0), plyr_idx, 0);
    }
}

/**
 * Updates thing interaction with rooms. Sometimes deletes the given thing.
 * @param thing Thing to be checked, and assimilated or deleted.
 * @note Used capacity of the room don't have to be updated here, as it is re-computed later.
 * @return True if the thing was either assimilated or left intact, false if it was deleted.
 */
short check_and_asimilate_thing_by_room(struct Thing *thing)
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
        if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_GoldStorage))
        {
            // No room - delete it, hoard cannot exist outside treasure room
            ERRORLOG("Found %s outside of %d room; removing",thing_model_name(thing),room_code_name(RoK_TREASURE));
            delete_thing_structure(thing, 0);
            return false;
        }
        long wealth_size_holds = gameadd.gold_per_hoard / get_wealth_size_types_count();
        unsigned long n = wealth_size_holds * (get_wealth_size_of_gold_hoard_object(thing) + 1);
        thing->owner = room->owner;
        add_gold_to_hoarde(thing, room, n);
        return true;
    }
    if (thing_is_spellbook(thing))
    {
        room = get_room_thing_is_on(thing);
        if (room->owner != game.neutral_player_num)
        {
            if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_PowersStorage) || (!player_exists(get_player(room->owner)) && (game.play_gameturn >= 10)))
            {
                // No room - oh well, leave it as free spell
                if (((gameadd.classic_bugs_flags & ClscBug_ClaimRoomAllThings) != 0) && !room_is_invalid(room)) {
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
            if (((gameadd.classic_bugs_flags & ClscBug_ClaimRoomAllThings) != 0) && !room_is_invalid(room)) {
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
    return true;
}

EventIndex update_cannot_find_room_wth_spare_capacity_event(PlayerNumber plyr_idx, struct Thing *creatng, RoomKind rkind)
{
    EventIndex evidx = 0;
    if (player_has_room(plyr_idx, rkind))
    {
        // Could not find room to send thing - either no capacity or not navigable
        struct Room *room;
        switch (rkind)
        {
        case RoK_LAIR:
            // Find room with lair capacity
            {
                struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
                room = find_room_with_spare_capacity(plyr_idx, rkind, crstat->lair_size);
                break;
            }
        case RoK_TREASURE:
        case RoK_WORKSHOP:
        case RoK_LIBRARY:
            // Find room with item capacity
            room = find_room_with_spare_room_item_capacity(plyr_idx, rkind);
            break;
        default:
            // Find room with worker capacity
            room = find_room_with_spare_capacity(plyr_idx, rkind, 1);
            break;
        }
        if (room_is_invalid(room))
        {
            SYNCDBG(5,"Player %d has %s which cannot find large enough %s",(int)plyr_idx,thing_model_name(creatng),room_code_name(rkind));
            switch (rkind)
            {
            case RoK_LAIR:
                evidx = event_create_event_or_update_nearby_existing_event(
                    creatng->mappos.x.val, creatng->mappos.y.val, EvKind_NoMoreLivingSet, plyr_idx, creatng->index);
                break;
            case RoK_TREASURE:
                evidx = event_create_event_or_update_nearby_existing_event(
                    0, 0, EvKind_TreasureRoomFull, plyr_idx, 0);
                break;
            default:
                evidx = 1;
                break;
            }
            if (evidx > 0) {
                output_message_room_related_from_computer_or_player_action(plyr_idx, rkind, OMsg_RoomTooSmall);
            }
        } else
        {
            SYNCDBG(5,"Player %d has %s which cannot reach %s",(int)plyr_idx,thing_model_name(creatng),room_code_name(rkind));
            evidx = event_create_event_or_update_nearby_existing_event(
                creatng->mappos.x.val, creatng->mappos.y.val, EvKind_WorkRoomUnreachable, plyr_idx, rkind);
            if (evidx > 0) {
                output_message_room_related_from_computer_or_player_action(plyr_idx, rkind, OMsg_RoomNoRoute);
            }
        }
    } else
    {
        // We simply don't have the room of that kind
        if ((rkind == RoK_LAIR) || (rkind == RoK_TREASURE) || is_room_available(plyr_idx, rkind))
        {
            switch (rkind)
            {
            case RoK_LAIR:
                evidx = event_create_event_or_update_nearby_existing_event(
                    creatng->mappos.x.val, creatng->mappos.y.val, EvKind_NoMoreLivingSet, plyr_idx, creatng->index);
                break;
            case RoK_TREASURE:
                evidx = event_create_event_or_update_nearby_existing_event(
                    0, 0, EvKind_NeedTreasureRoom, plyr_idx, 0);
                break;
            default:
                evidx = 1;
                break;
            }
            if (evidx > 0) {
                output_message_room_related_from_computer_or_player_action(plyr_idx, rkind, OMsg_RoomNeeded);
            }
        }
    }
    return evidx;
}

void query_room(struct Room *room)
{
    const char title[26];
    const char* name = room_code_name(room->kind);
    const char owner[26]; 
    const char health[26];
    const char capacity[26];
    const char efficiency[26] = "\0";
    sprintf((char*)title, "Room ID: %d", room->index);
    sprintf((char*)owner, "Owner: %d", room->owner);
    sprintf((char*)health, "Health: %d", room->health);
    sprintf((char*)capacity, "Capacity: %d/%d", room->used_capacity, room->total_capacity);
    float room_efficiency_percent = ((float)room->efficiency / (float)ROOM_EFFICIENCY_MAX) * 100;
    sprintf((char*)efficiency, "Efficiency: %d", (unsigned char)round(room_efficiency_percent));
    create_message_box((const char*)&title, name, (const char*)&owner, (const char*)&health, (const char*)&capacity, (const char*)&efficiency);    
}

/******************************************************************************/
