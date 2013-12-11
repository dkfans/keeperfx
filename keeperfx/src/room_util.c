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
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "room_list.h"
#include "room_workshop.h"
#include "config_terrain.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned long gold_per_hoarde = 2000; //TODO CONFIG place into any config struct
/******************************************************************************/
DLLIMPORT void _DK_process_rooms(void);
DLLIMPORT short _DK_delete_room_slab(long x, long y, unsigned char gnd_slab);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Thing *create_room_surrounding_flame(struct Room *room, const struct Coord3d *pos,
    unsigned short eetype, PlayerNumber owner)
{
  struct Thing *eething;
  eething = create_effect_element(pos, room_effect_elements[eetype & 7], owner);
  if (!thing_is_invalid(eething))
  {
    eething->mappos.z.val = get_thing_height_at(eething, &eething->mappos);
    eething->mappos.z.val += 10;
    // Size of the flame depends on room efficiency
    eething->field_46 = ((eething->field_46 - 80) * ((long)room->efficiency) / 256) + 80;
  }
  return eething;
}

void room_update_surrounding_flames(struct Room *room, const struct Coord3d *pos)
{
    struct Room *curoom;
    MapSubtlCoord x,y;
    long i,k;
    i = room->field_43;
    x = pos->x.stl.num + (MapSubtlCoord)small_around[i].delta_x;
    y = pos->y.stl.num + (MapSubtlCoord)small_around[i].delta_y;
    curoom = subtile_room_get(x,y);
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
    struct Coord3d pos;
    long x,y;
    long i;
    SYNCDBG(19,"Starting");
    x = 3 * slb_num_decode_x(room->field_41);
    y = 3 * slb_num_decode_y(room->field_41);
    i = 3 * room->field_43 + room->field_44;
    pos.x.val = 256 * (x+1) + room_spark_offset[i].delta_x + 128;
    pos.y.val = 256 * (y+1) + room_spark_offset[i].delta_y + 128;
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
    if (room->field_44 == 2)
    {
      room_update_surrounding_flames(room,&pos);
    }
    room->field_44 = (room->field_44 + 1) % 3;
}

void recompute_rooms_count_in_dungeons(void)
{
  SYNCDBG(17,"Starting");
  struct Dungeon *dungeon;
  long i,k;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = get_dungeon(i);
    dungeon->buildable_rooms_count = 0;
    for (k = 1; k < 17; k++)
    {
      if ((k != RoK_ENTRANCE) && (k != RoK_DUNGHEART))
      {
        dungeon->buildable_rooms_count += get_player_rooms_count(i, k);
      }
    }
  }
}

void process_rooms(void)
{
  SYNCDBG(7,"Starting");
  struct Room *room;
  struct Packet *pckt;
  //_DK_process_rooms(); return;
  for (room = start_rooms; room < end_rooms; room++)
  {
    if ((room->field_0 & 0x01) == 0)
      continue;
    if (room->kind == RoK_GARDEN)
      room_grow_food(room);
    pckt = get_packet(my_player_number);
    pckt->chksum += (room->slabs_count & 0xFF) + room->central_stl_x + room->central_stl_y;
    if (((game.numfield_D & 0x40) == 0) || (room->kind == RoK_DUNGHEART))
      continue;
    process_room_surrounding_flames(room);
  }
  recompute_rooms_count_in_dungeons();
  SYNCDBG(9,"Finished");
}

void kill_all_room_slabs_and_contents(struct Room *room)
{
    struct SlabMap *slb;
    long slb_x, slb_y;
    unsigned long k;
    long i;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        slb = get_slabmap_block(slb_x, slb_y);
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

void remove_slab_from_room_tiles_list(struct Room *room, long rslb_num)
{
    struct SlabMap *slb,*rslb;
    unsigned long k;
    long i;
    rslb = get_slabmap_direct(rslb_num);
    if (slabmap_block_invalid(rslb))
    {
        ERRORLOG("Non-existing slab %d.",(int)rslb_num);
        return;
    }
    // If the slab to remove is first in room slabs list - it's simple
    // In this case we need to re-put a flag on first slab
    if (room->slabs_list == rslb_num)
    {
        delete_room_flag(room);
        room->slabs_list = rslb->next_in_room;
        rslb->next_in_room = 0;
        rslb->room_index = 0;
        room->slabs_count--;
        create_room_flag(room);
        return;
    }
    // If the slab to remove is not first, we have to sweep the list
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb = get_slabmap_direct(i);
        if (slabmap_block_invalid(slb))
        {
          ERRORLOG("Jump to invalid item when sweeping Slabs.");
          break;
        }
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        if (slb->next_in_room == rslb_num)
        {
            // When the item was found, replace its reference with next item
            slb->next_in_room = rslb->next_in_room;
            rslb->next_in_room = 0;
            rslb->room_index = 0;
            room->slabs_count--;
            return;
        }
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    WARNLOG("Slab %ld couldn't be found in room tiles list.",rslb_num);
    rslb->next_in_room = 0;
    rslb->room_index = 0;
}

void sell_room_slab_when_no_free_room_structures(struct Room *room, long slb_x, long slb_y, unsigned char gnd_slab)
{
    struct RoomStats *rstat;
    struct Coord3d pos;
    long revenue;
    delete_room_slab_when_no_free_room_structures(slb_x, slb_y, gnd_slab);
    rstat = &game.room_stats[room->kind];
    revenue = compute_value_percentage(rstat->cost, ROOM_SELL_REVENUE_PERCENT);
    if (revenue != 0)
    {
        set_coords_to_slab_center(&pos,slb_x,slb_y);
        create_price_effect(&pos, room->owner, revenue);
        player_add_offmap_gold(room->owner, revenue);
    }
}

void recreate_rooms_from_room_slabs(struct Room *room, unsigned char gnd_slab)
{
    struct SlabMap *slb;
    long slb_x, slb_y;
    unsigned long k;
    long i;
    SYNCDBG(7,"Starting for %s index %d",room_code_name(room->kind),(int)room->index);
    // Clear room index in all slabs
    // This will make sure that the old room won't be returned by subtile_room_get()
    // and used as one of new rooms.
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb = get_slabmap_direct(i);
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
    struct Room *proom;
    proom = INVALID_ROOM;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        struct Room *nroom;
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        nroom = create_room(room->owner, room->kind, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
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

TbBool delete_room_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char gnd_slab)
{
    struct Room *room;
    SlabCodedCoords slb_num;
    //return _DK_delete_room_slab(slb_x, slb_y, gnd_slab);
    room = slab_room_get(slb_x,slb_y);
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
        replace_room_slab(room, slb_x, slb_y, room->owner, gnd_slab);
        kill_all_room_slabs_and_contents(room);
        free_room_structure(room);
        do_slab_efficiency_alteration(slb_x, slb_y);
    } else
    {
        slb_num = get_slab_number(slb_x, slb_y);
        // Remove the slab from room tiles list
        remove_slab_from_room_tiles_list(room, slb_num);
        replace_room_slab(room, slb_x, slb_y, room->owner, gnd_slab);
        // Create a new room from slabs left in old one
        recreate_rooms_from_room_slabs(room, gnd_slab);
        reset_creatures_rooms(room);
        free_room_structure(room);
    }
    return true;
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
    unsigned long n;
    if ((thing->field_1 & TF1_IsDragged1) != 0)
    {
        ERRORLOG("It shouldn't be possible to drag %s during initial asimilation",thing_model_name(thing));
        thing->owner = game.neutral_player_num;
        return true;
    }
    if (thing_is_gold_hoard(thing))
    {
        room = get_room_thing_is_on(thing);
        if (room_is_invalid(room) || (room->kind != RoK_TREASURE))
        {
            // No room - delete it, hoard cannot exist outside treasure room
            ERRORLOG("Found %s outside of Treasure Room; removing",thing_model_name(thing));
            delete_thing_structure(thing, 0);
            return false;
        }
        n = (gold_per_hoarde/5)*(((long)thing->model)-51);
        thing->owner = room->owner;
        add_gold_to_hoarde(thing, room, n);
        return true;
    }
    if (thing_is_spellbook(thing))
    {
        room = get_room_thing_is_on(thing);
        if (room_is_invalid(room) || (room->kind != RoK_LIBRARY))
        {
            // No room - oh well, leave it as free spell
            if (((gameadd.classic_bugs_flags & ClscBug_ClaimRoomAllThings) != 0) && !room_is_invalid(room)) {
                // Preserve classic bug - object is claimed with the room
                thing->owner = room->owner;
            } else {
                // Make correct owner so that Imps can pick it up
                thing->owner = game.neutral_player_num;
            }
            return true;
        }
        if (!add_spell_to_player(book_thing_to_magic(thing), room->owner))
        {
            thing->owner = game.neutral_player_num;
            return true;
        }
        thing->owner = room->owner;
        return true;
    }
    if (thing_is_door_or_trap_box(thing))
    {
        room = get_room_thing_is_on(thing);
        if (room_is_invalid(room) || (room->kind != RoK_WORKSHOP))
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
        if (!add_workshop_item(room->owner, box_thing_to_workshop_object_class(thing), box_thing_to_door_or_trap(thing)))
        {
            thing->owner = game.neutral_player_num;
            return true;
        }
        thing->owner = room->owner;
        return true;
    }
    return true;
}
/******************************************************************************/
