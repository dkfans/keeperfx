/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_data.c
 *     Rooms support functions.
 * @par Purpose:
 *     Functions to create and maintain the game rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Apr 2009 - 14 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "room_data.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "config_creature.h"
#include "power_specials.h"
#include "room_garden.h"
#include "room_graveyard.h"
#include "room_jobs.h"
#include "room_lair.h"
#include "room_library.h"
#include "room_treasure.h"
#include "room_workshop.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_traps.h"
#include "thing_corpses.h"
#include "thing_effects.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "config_terrain.h"
#include "config_effects.h"
#include "creature_states.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "magic_powers.h"
#include "room_util.h"
#include "game_legacy.h"
#include "frontmenu_ingame_map.h"
#include "keeperfx.hpp"
#include "config_spritecolors.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void count_slabs_all_only(struct Room *room);
void count_slabs_all_wth_effcncy(struct Room *room);
void count_slabs_no_min_wth_effcncy(struct Room *room);
void count_slabs_div2_wth_effcncy(struct Room *room);
void count_slabs_div2_nomin_effcncy(struct Room *room);
void count_slabs_mul2_wth_effcncy(struct Room *room);
void count_slabs_pow2_wth_effcncy(struct Room *room);
void count_workers_in_room(struct Room *room);
long find_random_valid_position_for_item_in_different_room_avoiding_object(struct Thing* thing, struct Room* skip_room, struct Coord3d* pos);
/******************************************************************************/

struct AroundLByte const room_spark_offset[] = {
  {-256,  256},
  {-256,    0},
  {-256, -256},
  {-256, -256},
  {   0, -256},
  { 256, -256},
  { 256, -256},
  { 256,    0},
  { 256,  256},
  { 256,  256},
  {   0,  256},
  {-256,  256},
};

unsigned char const slabs_to_centre_pieces[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  2,  2,  2,  3,  4,  4,
  4,  5,  6,  6,  6,  7,  8,  9,  9,
  9, 10, 11, 12, 12, 12, 13, 14, 15,
 16, 16, 16, 17, 18, 19, 20, 20, 20,
 21, 22, 23, 24, 25,
};

unsigned short const room_effect_elements[] = { TngEffElm_RedFlame, TngEffElm_BlueFlame, TngEffElm_GreenFlame, TngEffElm_YellowFlame, TngEffElm_WhiteFlame,
                                                TngEffElm_None, TngEffElm_PurpleFlame, TngEffElm_BlackFlame, TngEffElm_OrangeFlame };
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Room *room_get(RoomIndex room_idx)
{
  if ((room_idx < 1) || (room_idx > ROOMS_COUNT))
    return &game.rooms[0];
  return &game.rooms[room_idx];
}

struct Room *subtile_room_get(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_block_invalid(slb))
        return INVALID_ROOM;
    return room_get(slb->room_index);
}

struct Room *slab_room_get(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb))
        return INVALID_ROOM;
    return room_get(slb->room_index);
}

TbBool room_is_invalid(const struct Room *room)
{
  if (room == NULL)
    return true;
  if (room == INVALID_ROOM)
    return true;
  return (room <= &game.rooms[0]);
}

TbBool room_exists(const struct Room *room)
{
  if (room_is_invalid(room))
    return false;
  return ((room->alloc_flags & RoF_Allocated) != 0);
}

/**
 * Recomputes the amount of slabs the player has.
 * @param plyr_idx
 * @param rkind
 */
long get_room_slabs_count(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long count = 0;
    long i = dungeon->room_list_start[rkind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        count += room->slabs_count;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return count;
}

/**
 * Recomputes the amount of slabs the player has.
 * @param plyr_idx
 * @param rrole
 */
long get_room_of_role_slabs_count(PlayerNumber plyr_idx, RoomRole rrole)
{
    if (plyr_idx == game.neutral_player_num)
        return -1;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long count = 0;

    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(!room_role_matches(rkind,rrole))
        {
            continue;
        }
        long i = dungeon->room_list_start[rkind];
        unsigned long k = 0;
        while (i != 0)
        {
            struct Room* room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            count += room->slabs_count;
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }
    return count;
}

long count_slabs_of_room_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    if (plyr_idx == game.neutral_player_num)
        return -1;
    long nslabs = 0;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long i = dungeon->room_list_start[rkind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        nslabs += room->slabs_count;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping %s rooms list.", player_code_name(plyr_idx));
            break;
        }
    }
    return nslabs;
}

void get_room_kind_total_and_used_capacity(struct Dungeon *dungeon, RoomKind rkind, int32_t *total_cap, int32_t *used_cap)
{
    unsigned int total_capacity = 0;
    unsigned int used_capacity = 0;
    long i = dungeon->room_list_start[rkind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        used_capacity += room->used_capacity;
        total_capacity += room->total_capacity;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    *total_cap = total_capacity;
    *used_cap = used_capacity;
}

void get_room_kind_total_used_and_storage_capacity(struct Dungeon *dungeon, RoomKind rkind, int32_t *total_cap, int32_t *used_cap, int32_t *storaged_cap)
{
    unsigned int total_capacity = 0;
    unsigned int used_capacity = 0;
    int storaged_capacity = 0;
    long i = dungeon->room_list_start[rkind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        used_capacity += room->used_capacity;
        total_capacity += room->total_capacity;
        storaged_capacity += room->capacity_used_for_storage;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    *total_cap = total_capacity;
    *used_cap = used_capacity;
    *storaged_cap = storaged_capacity;
}

long get_room_kind_used_capacity_fraction(PlayerNumber plyr_idx, RoomKind room_kind)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    int32_t used_capacity;
    int32_t total_capacity;
    get_room_kind_total_and_used_capacity(dungeon, room_kind, &total_capacity, &used_capacity);
    if (total_capacity <= 0) {
        return 0;
    }
    return (used_capacity * 256) / total_capacity;
}

void set_room_efficiency(struct Room *room)
{
    room->efficiency = calculate_room_efficiency(room);
}

void init_reposition_struct(struct RoomReposition * rrepos)
{
    rrepos->used = 0;
    for (long i = 0; i < ROOM_REPOSITION_COUNT; i++)
    {
        rrepos->models[i] = 0;
        rrepos->exp_level[i] = 0;
    }
}

TbBool store_reposition_entry(struct RoomReposition * rrepos, ThingModel tngmodel)
{
    int ri;
    // Don't store the same entry two times
    for (ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] == tngmodel) {
            return true;
        }
    }
    if (rrepos->used > ROOM_REPOSITION_COUNT)
    {
        ERRORLOG("Reposition entries to store (%d) exceed maximum %d", rrepos->used,ROOM_REPOSITION_COUNT);
        rrepos->used = ROOM_REPOSITION_COUNT;
        return false;
    }
    for (ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] == 0) {
            rrepos->models[ri] = tngmodel;
            rrepos->used++;
            break;
        }
    }
    return true;
}

TbBool store_creature_reposition_entry(struct RoomReposition * rrepos, ThingModel tngmodel, CrtrExpLevel exp_level)
{
    rrepos->used++;
    if (rrepos->used > ROOM_REPOSITION_COUNT)
    {
        ERRORLOG("Creature reposition entries to store (%d) exceed maximum %d", rrepos->used, ROOM_REPOSITION_COUNT);
        rrepos->used = ROOM_REPOSITION_COUNT;
        return false;
    }
    for (int ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] == 0) {
            rrepos->models[ri] = tngmodel;
            rrepos->exp_level[ri] = exp_level;
            break;
        }
    }
    return true;
}

TbBool move_thing_to_different_room(struct Thing* thing, struct Coord3d* pos)
{
    if (!thing_exists(thing))
    {
        ERRORLOG("Attempt to move non-existing thing to different room.");
        return false;
    }
    pos->z.val = get_thing_height_at(thing, pos);
    move_thing_in_map(thing, pos);
    create_effect(pos, TngEff_RoomSparkeLarge, thing->owner);
    struct Room* nxroom;
    nxroom = get_room_thing_is_on(thing);
    if (room_exists(nxroom))
    {
        update_room_contents(nxroom);
        return true;
    }
    return false;
}

void count_workers_in_room(struct Room *room)
{
    int count = 0;
    long i = room->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (!creature_control_exists(cctrl))
        {
            ERRORLOG("Jump to invalid creature %d detected",(int)i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        count++;
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    room->used_capacity += count;
}

void count_slabs_all_only(struct Room *room)
{
    room->total_capacity = room->slabs_count;
}

void count_slabs_all_wth_effcncy(struct Room *room)
{
    unsigned long count = room->slabs_count * ((long)room->efficiency);
    count = (count/ROOM_EFFICIENCY_MAX);
    if (count <= 1)
        count = 1;
    room->total_capacity = count;
}

void count_slabs_no_min_wth_effcncy(struct Room *room)
{
    unsigned long count = room->slabs_count * ((long)room->efficiency);
    count = (count/ROOM_EFFICIENCY_MAX);
    if (count < 1)
        count = 0;
    room->total_capacity = count;
}

void count_slabs_div2_wth_effcncy(struct Room *room)
{
    unsigned long count = room->slabs_count * ((long)room->efficiency);
    count = ((count/ROOM_EFFICIENCY_MAX) >> 1);
    if (count <= 1)
        count = 1;
    room->total_capacity = count;
}

void count_slabs_div2_nomin_effcncy(struct Room *room)
{
    unsigned long count = room->slabs_count * ((long)room->efficiency);
    count = ((count/ROOM_EFFICIENCY_MAX) >> 1);
    if (count < 1)
        count = 0;
    room->total_capacity = count;
}

void count_slabs_mul2_wth_effcncy(struct Room *room)
{
    unsigned long count = room->slabs_count * ((long)room->efficiency);
    count = ((count/ROOM_EFFICIENCY_MAX) << 1);
    if (count <= 1)
        count = 1;
    room->total_capacity = count;
}

void count_slabs_pow2_wth_effcncy(struct Room *room)
{
    unsigned long count = room->slabs_count * ((long)room->efficiency) * ((long)room->efficiency);
    count = (count/ROOM_EFFICIENCY_MAX/ROOM_EFFICIENCY_MAX);
    if (count <= 1)
        count = 1;
    room->total_capacity = count;
}

void delete_room_structure(struct Room *room)
{
    if (room_is_invalid(room))
    {
        WARNLOG("Attempt to delete invalid room");
        return;
    }
    if ((room->alloc_flags & RoF_Allocated) != 0)
    {
      // This is almost remove_room_from_players_list(room, room->owner);
      // but it doesn't change room_slabs_count and is less careful - better not use too much
      if (room->owner != game.neutral_player_num)
      {
          struct Dungeon* dungeon = get_dungeon(room->owner);
          unsigned short* wptr = &dungeon->room_list_start[room->kind];
          struct Room* secroom;
          if (room->index == *wptr)
          {
              *wptr = room->next_of_owner;
              secroom = room_get(room->next_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->prev_of_owner = 0;
          } else
          {
              secroom = room_get(room->next_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->prev_of_owner = room->prev_of_owner;
              secroom = room_get(room->prev_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->next_of_owner = room->next_of_owner;
          }
      }
      memset(room, 0, sizeof(struct Room));
    }
}

void delete_all_room_structures(void)
{
    for (long i = 1; i < ROOMS_COUNT; i++)
    {
        struct Room* room = &game.rooms[i];
        delete_room_structure(room);
    }
}

/**
 * Changes work room of all creatures working in specific room.
 * Used for replacing room structures.
 * @param wrkroom The room creatures are working in.
 * @param newroom The new room for the creatures to work.
 */
void change_work_room_of_creatures_working_in_room(struct Room *wrkroom, struct Room *newroom)
{
    unsigned long k = 0;
    while (wrkroom->creatures_list != 0)
    {
        struct Thing* thing = thing_get(wrkroom->creatures_list);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid creature %d detected",(int)wrkroom->creatures_list);
            break;
        }
        // Per creature code
        CreatureJob jobpref = get_job_for_creature_state(get_creature_state_besides_interruptions(thing));
        remove_creature_from_specific_room(thing, wrkroom, jobpref);
        add_creature_to_work_room(thing, newroom, jobpref);
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
}

/**
 * Changes active state of all creatures working in specific room.
 * @param wrkroom The room creatures are working in.
 */
void reset_state_of_creatures_working_in_room(struct Room *wrkroom)
{
    long non_creature = 0;
    long i = wrkroom->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (!creature_control_exists(cctrl))
        {
            ERRORLOG("Jump to invalid creature %d detected",(int)i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        if (thing_is_creature(thing)) {
            set_start_state(thing);
        } else {
            non_creature++;
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    if (non_creature > 0) {
        // For some reasons, gravestones are also on this list; we should check whether this makes sense or it's just a mistake
        WARNLOG("The %s contained %d things which were not creatures",room_code_name(wrkroom->kind),(int)non_creature);
    }
}

void update_room_total_capacity(struct Room *room)
{
    SYNCDBG(7, "Starting for %s index %d owned by player %d", room_code_name(room->kind), (int)room->index, (int)room->owner);
    const struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    Room_Update_Func cb = terrain_room_total_capacity_func_list[roomst->update_total_capacity_idx];
    if (cb != NULL) {
        cb(room);
    }
    SYNCDBG(7, "Finished");
}

/**
 * Counts slabs making up the room and stores them in the room.
 * Also, updates room index in all the slabs.
 * @param room
 * @note was named count_room_slabs()
 */
void recount_and_reassociate_room_slabs(struct Room *room)
{
    long n = 0;
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
        n++;
        slb->room_index = room->index;
        // Per room tile code ends
        k++;
        if (k >= game.map_tiles_x*game.map_tiles_y)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    room->slabs_count = n;
}

/** Returns coordinates of slab at mass centre of given room.
 *  Note that the slab returned may not be pat of the room - it is possible
 *   that the room is just surrounding the spot.
 * @param mass_x
 * @param mass_y
 * @param room
 */
void get_room_mass_centre_coords(int32_t *mass_x, int32_t *mass_y, const struct Room *room)
{
    unsigned long tot_x = 0;
    unsigned long tot_y = 0;
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i > 0)
    {
        long slb_x = slb_num_decode_x(i);
        long slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        if (slabmap_block_invalid(slb))
        {
            ERRORLOG("Jump to invalid item when sweeping room %s index %d Slabs.",room_code_name(room->kind),(int)room->index);
            break;
        }
        // Per room tile code
        tot_x += slb_x;
        tot_y += slb_y;
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room %s index %d slabs list length exceeded when sweeping.",room_code_name(room->kind),(int)room->index);
            break;
        }
    }
    if (room->slabs_count > 1) {
        *mass_x = tot_x / room->slabs_count;
        *mass_y = tot_y / room->slabs_count;
    } else
    if (room->slabs_count > 0) {
        *mass_x = tot_x;
        *mass_y = tot_y;
    } else {
        ERRORLOG("Room %s index %d has no slabs.",room_code_name(room->kind),(int)room->index);
        *mass_x = game.map_tiles_x / 2;
        *mass_y = game.map_tiles_y / 2;
    }
}


void update_room_central_tile_position(struct Room *room)
{
    int32_t mass_x;
    int32_t mass_y;
    get_room_mass_centre_coords(&mass_x, &mass_y, room);
    for (long i = 0; i < 16 * 16; i++)
    {
        struct MapOffset* sstep = &spiral_step[i];
        MapSubtlCoord cx = slab_subtile_center(mass_x + sstep->h);
        MapSubtlCoord cy = slab_subtile_center(mass_y + sstep->v);
        struct SlabMap* slb = get_slabmap_for_subtile(cx, cy);
        if (slabmap_block_invalid(slb))
            continue;
        if (slb->room_index == room->index)
        {
            room->central_stl_x = cx;
            room->central_stl_y = cy;
            return;
        }
    }
    room->central_stl_x = mass_x;
    room->central_stl_y = mass_y;
    WARNLOG("Cannot find position in %s index %d to place an ensign.",room_code_name(room->kind),(int)room->index);
}

void add_room_to_global_list(struct Room *room)
{
    // There is only one global list of rooms - the list of entrances
    if (room->kind == RoK_ENTRANCE)
    {
      if ((game.entrance_room_id > 0) && (game.entrance_room_id < ROOMS_COUNT))
      {
        room->next_of_kind = game.entrance_room_id;
        struct Room* nxroom = room_get(game.entrance_room_id);
        nxroom->prev_of_kind = room->index;
      }
      game.entrance_room_id = room->index;
      game.entrances_count++;
    }
}

void remove_room_from_global_list(struct Room* room)
{
    // There is only one global list of rooms - the list of entrances
    if (room->kind != RoK_ENTRANCE)
        return;

    struct Room* pvroom = room_get(room->prev_of_kind);
    struct Room* nxroom = room_get(room->next_of_kind);

    if (!room_is_invalid(pvroom)) {
        pvroom->next_of_kind = room->next_of_kind;
    }
    else {
        game.entrance_room_id = room->next_of_kind;
    }

    if (!room_is_invalid(nxroom)) {
        nxroom->prev_of_kind = room->prev_of_kind;
    }

    room->next_of_kind = 0;
    room->prev_of_kind = 0;
    game.entrances_count--;
}

TbBool add_room_to_players_list(struct Room *room, PlayerNumber plyr_idx)
{
    if (room->kind >= game.conf.slab_conf.room_types_count) {
        ERRORLOG("Room index %d has invalid kind %d",(int)room->index,(int)room->kind);
        return false;
    }
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    long nxroom_id = dungeon->room_list_start[room->kind];
    struct Room* nxroom = room_get(nxroom_id);
    if (room_is_invalid(nxroom))
    {
        room->next_of_owner = 0;
    } else
    {
        room->next_of_owner = nxroom_id;
        nxroom->prev_of_owner = room->index;
    }
    dungeon->room_list_start[room->kind] = room->index;
    dungeon->room_discrete_count[room->kind]++;
    return true;
}

TbBool remove_room_from_players_list(struct Room *room, PlayerNumber plyr_idx)
{
    if (room->kind >= game.conf.slab_conf.room_types_count) {
        ERRORLOG("Room index %d has invalid kind %d",(int)room->index,(int)room->kind);
        return false;
    }
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Player %d has no dungeon",(int)plyr_idx);
        room->next_of_owner = 0;
        room->prev_of_owner = 0;
        return false;
    }
    struct Room* pvroom = room_get(room->prev_of_owner);
    struct Room* nxroom = room_get(room->next_of_owner);
    if (!room_is_invalid(pvroom)) {
        pvroom->next_of_owner = room->next_of_owner;
    } else {
        dungeon->room_list_start[room->kind] = room->next_of_owner;
    }
    if (!room_is_invalid(nxroom)) {
        nxroom->prev_of_owner = room->prev_of_owner;
    }
    room->next_of_owner = 0;
    room->prev_of_owner = 0;
    dungeon->room_discrete_count[room->kind]--;
    return true;
}

void add_slab_to_room_tiles_list(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SlabCodedCoords slb_num = get_slab_number(slb_x, slb_y);
    if (room->slabs_list == 0) {
        room->slabs_list = slb_num;
    } else {
        struct SlabMap* pvslb = get_slabmap_direct(room->slabs_list_tail);
        pvslb->next_in_room = slb_num;
    }
    {
        struct SlabMap* nxslb = get_slabmap_direct(slb_num);
        nxslb->room_index = room->index;
        room->slabs_count++;
        nxslb->next_in_room = 0;
    }
    room->slabs_list_tail = slb_num;
}

/**
 * Adds slab list starting with given slab number to given room.
 *
 * @param room
 * @param slb_num
 */
TbBool add_slab_list_to_room_tiles_list(struct Room *room, SlabCodedCoords slb_num)
{
    if (room->slabs_list == 0) {
        room->slabs_list = slb_num;
    } else {
        struct SlabMap* pvslb = get_slabmap_direct(room->slabs_list_tail);
        pvslb->next_in_room = slb_num;
    }
    SlabCodedCoords tail_slb_num = slb_num;
    unsigned short k = 0;
    while (1)
    {
        struct SlabMap* nxslb = get_slabmap_direct(tail_slb_num);
        nxslb->room_index = room->index;
        room->slabs_count++;
        if (nxslb->next_in_room == 0) {
            break;
        }
        tail_slb_num = nxslb->next_in_room;
        // Per room tile code ends
        k++;
        if (k > (MAX_TILES_X * MAX_TILES_Y))
        {
            ERRORLOG("Room slabs list length exceeded when sweeping Room (%d) '%s' at stl (%d,%d)",room->index,room_code_name(room->kind),room->central_stl_x,room->central_stl_y);
            return false;
        }
    }
    room->slabs_list_tail = tail_slb_num;
    return true;
}

void remove_slab_from_room_tiles_list(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SlabCodedCoords slb_num = get_slab_number(slb_x, slb_y);

    struct SlabMap* rmslb = get_slabmap_direct(slb_num);
    if (slabmap_block_invalid(rmslb))
    {
        ERRORLOG("Non-existing slab (%d,%d).",(int)slb_x,(int)slb_y);
        return;
    }
    // If the slab to remove is first in room slabs list - it's simple
    // In this case we need to re-put a flag on first slab
    if (room->slabs_list == slb_num)
    {
        delete_room_flag(room);
        room->slabs_list = rmslb->next_in_room;
        room->slabs_count--;
        rmslb->next_in_room = 0;
        rmslb->room_index = 0;
        create_room_flag(room);
        return;
    }
    // If the slab to remove is not first, we have to sweep the list
    unsigned short k = 0;
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
        if (slb->next_in_room == slb_num)
        {
            // When the item was found, replace its reference with next item
            slb->next_in_room = rmslb->next_in_room;
            room->slabs_count--;
            rmslb->next_in_room = 0;
            rmslb->room_index = 0;
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
    WARNLOG("Slab %d couldn't be found in room tiles list.",slb_num);
    rmslb->next_in_room = 0;
    rmslb->room_index = 0;
}

struct Room *prepare_new_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (!subtile_has_slab(stl_x, stl_y))
    {
        ERRORLOG("Attempt to create room on invalid coordinates.");
        return INVALID_ROOM;
    }
    if ( !i_can_allocate_free_room_structure() )
    {
        ERRORDBG(2,"Cannot allocate any more rooms.");
        erstat_inc(ESE_NoFreeRooms);
        return INVALID_ROOM;
    }
    struct Room* room = allocate_free_room_structure();
    room->owner = owner;
    room->kind = rkind;
    add_room_to_global_list(room);
    add_room_to_players_list(room, owner);
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    add_slab_to_room_tiles_list(room, slb_x, slb_y);
    return room;
}

struct Room *create_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    SYNCDBG(7,"Starting to make %s (%d) at (%d,%d)",room_code_name(rkind), rkind,(int)stl_x,(int)stl_y);
    // Try linking the new room slab to existing room
    struct Room* room = link_adjacent_rooms_of_type(owner, stl_x, stl_y, rkind);
    if (room_is_invalid(room))
    {
        room = prepare_new_room(owner, rkind, stl_x, stl_y);
        if (room_is_invalid(room)) {
            return INVALID_ROOM;
        }
        recount_and_reassociate_room_slabs(room);
        update_room_central_tile_position(room);
        create_room_flag(room);
    } else
    {
        recount_and_reassociate_room_slabs(room);
        update_room_central_tile_position(room);
    }
    SYNCDBG(7,"Done");
    return room;
}

void create_room_flag(struct Room *room)
{
    MapSubtlCoord stl_x = slab_subtile_center(slb_num_decode_x(room->slabs_list));
    MapSubtlCoord stl_y = slab_subtile_center(slb_num_decode_y(room->slabs_list));
    SYNCDBG(7,"Starting for %s (%d) at (%d,%d)",room_code_name(room->kind), room->kind,(int)stl_x,(int)stl_y);
    if (room_can_have_ensign(room->kind))
    {
        struct Coord3d pos;
        pos.z.val = subtile_coord(2, 0);
        pos.x.val = subtile_coord(stl_x,0);
        pos.y.val = subtile_coord(stl_y,0);
        struct Thing* thing = find_base_thing_on_mapwho(TCls_Object, ObjMdl_RoomFlag, stl_x, stl_y);
        if (thing_is_invalid(thing))
        {
            thing = create_object(&pos, ObjMdl_RoomFlag, room->owner, -1);
        }
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Cannot create room flag");
            return;
        }
        thing->lair.belongs_to = room->index;
    }
}

void delete_room_flag(struct Room *room)
{
    MapSubtlCoord stl_x = slab_subtile_center(slb_num_decode_x(room->slabs_list));
    MapSubtlCoord stl_y = slab_subtile_center(slb_num_decode_y(room->slabs_list));
    if (room_can_have_ensign(room->kind))
    {
        struct Thing* thing = find_base_thing_on_mapwho(TCls_Object, ObjMdl_RoomFlag, stl_x, stl_y);
        if (!thing_is_invalid(thing)) {
            delete_thing_structure(thing, 0);
        }
    }
}

struct Room *allocate_free_room_structure(void)
{
    for (int i = 1; i < ROOMS_COUNT; i++)
    {
        struct Room* room = &game.rooms[i];
        if ((room->alloc_flags & RoF_Allocated) == 0)
        {
            memset(room, 0, sizeof(struct Room));
            room->alloc_flags |= RoF_Allocated;
            room->index = i;
            room->creation_turn = game.play_gameturn;
            return room;
        }
    }
    return INVALID_ROOM;
}

unsigned short i_can_allocate_free_room_structure(void)
{
    for ( int i = 1; i < ROOMS_COUNT; ++i )
    {
        struct Room* room = &game.rooms[i];
        if ((room->alloc_flags & RoF_Allocated) == 0)
        {
            return i;
        }
    }
    SYNCDBG(3,"No slot for next room");
    return 0;
}



/**
 * Recalculates all players rooms of specific kind.
 * @param rkind
 * @return Total amount of rooms which were reinitialized.
 */
long recalculate_effeciency_for_rooms_of_kind(RoomKind rkind)
{
    unsigned int k = 0;
    for (unsigned int n = 0; n < DUNGEONS_COUNT; n++)
    {
        struct Dungeon* dungeon = get_dungeon(n);
        unsigned int i = dungeon->room_list_start[rkind];
        while (i != 0)
        {
            struct Room* room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code starts
            set_room_efficiency(room);
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }
    return k;
}

/**
 * Re-initializes all players rooms of specific kind.
 * @param rkind
 * @param skip_integration
 * @return Total amount of rooms which were reinitialized.
 */
long reinitialise_rooms_of_kind(RoomKind rkind)
{
    unsigned int k = 0;
    for (unsigned int n = 0; n < DUNGEONS_COUNT; n++)
    {
        struct Dungeon* dungeon = get_dungeon(n);
        unsigned int i = dungeon->room_list_start[rkind];
        while (i != 0)
        {
            struct Room* room = room_get(i);
            if (room_is_invalid(room))
            {
              ERRORLOG("Jump to invalid room detected");
              break;
            }
            i = room->next_of_owner;
            // Per-room code starts
            do_room_integration(room);
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
              ERRORLOG("Infinite loop detected when sweeping rooms list");
              break;
            }
        }
    }
    return k;
}

/**
 * Does re-initialization of level rooms after the level is completely loaded.
 * @see initialise_map_rooms()
 * @return
 */
void reinitialise_map_rooms(void)
{
    for (RoomKind rkind = 1; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        reinitialise_rooms_of_kind(rkind);
    }
}

/**
 * Does basic initialization of level rooms after reading SLB file.
 * @note While this is executed, things may not be loaded yet.
 * @see reinitialise_map_rooms()
 * @return
 */
TbBool initialise_map_rooms(void)
{
    SYNCDBG(7,"Starting");
    for (unsigned long slb_y = 0; slb_y < game.map_tiles_y; slb_y++)
    {
        for (unsigned long slb_x = 0; slb_x < game.map_tiles_x; slb_x++)
        {
            struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
            RoomKind rkind = slab_corresponding_room(slb->kind);
            struct Room* room;
            if (rkind > 0)
                room = create_room(slabmap_owner(slb), rkind, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
            else
                room = INVALID_ROOM;
            if (!room_is_invalid(room))
            {
                do_room_integration(room);
            }
        }
    }
    return true;
}

MapCoordDelta get_distance_to_room(const struct Coord3d *pos, const struct Room *room)
{
    MapCoordDelta dist_x = abs(pos->x.val - subtile_coord_center(room->central_stl_x));
    MapCoordDelta dist_y = abs(pos->y.val - subtile_coord_center(room->central_stl_y));
    return max(dist_x,dist_y);
}

/** Calculates shape-based efficiency score from all slabs in room.
 *
 * @param room Source room.
 * @return The efficiency score summary.
 */
long calculate_room_widespread_factor(const struct Room *room)
{
    SYNCDBG(7, "Starting for %s index %d owned by player %d", room_code_name(room->kind), (int)room->index, (int)room->owner);
    long nslabs = room->slabs_count;
    long i = nslabs;
    if (i >= sizeof(slabs_to_centre_pieces)/sizeof(slabs_to_centre_pieces[0]))
        i = sizeof(slabs_to_centre_pieces)/sizeof(slabs_to_centre_pieces[0]) - 1;
    long npieces = slabs_to_centre_pieces[i];
    return 2 * (npieces + 4 * nslabs);
}

/** Calculates summary of surrounding-based efficiency score from all slabs in room.
 *
 * @param room Source room.
 * @return The efficiency score summary.
 */
long calculate_cummulative_room_slabs_effeciency(const struct Room *room)
{
    SYNCDBG(7, "Starting for %s index %d owned by player %d", room_code_name(room->kind), (int)room->index, (int)room->owner);
    long score = 0;
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i != 0)
    {
        // Per room tile code
        score += calculate_effeciency_score_for_room_slab(i, room->owner, get_room_kind_stats(room->kind)->synergy_slab);
        // Per room tile code ends
        i = get_next_slab_number_in_room(i); // It would be better to have this before per-tile block, but we need old value
        k++;
        if (k > room->slabs_count)
        {
          ERRORLOG("Room slabs list length exceeded when sweeping");
          break;
        }
    }
    SYNCDBG(7, "Finished");
    return score;
}

long calculate_room_efficiency(const struct Room *room)
{
    SYNCDBG(7, "Starting for %s index %d owned by player %d", room_code_name(room->kind), (int)room->index, (int)room->owner);
    long effic;
    long expected_base;
    long nslabs = room->slabs_count;
    if (nslabs <= 0)
    {
        ERRORLOG("Room %s index %d seems to have no slabs.",room_code_name(room->kind),(int)room->index);
        return 0;
    }
    if (nslabs == 1) {
        expected_base = 0;
    } else {
        expected_base = 4 * (nslabs - 1);
    }
    long widespread = calculate_room_widespread_factor(room);
    long score = calculate_cummulative_room_slabs_effeciency(room);
    if (score <= expected_base) {
        effic = 0;
    } else
    if (widespread <= expected_base) {
        effic = ROOM_EFFICIENCY_MAX;
    } else
    {
        effic = ((score - expected_base) << 8) / (widespread - expected_base);
    }
    if (effic > ROOM_EFFICIENCY_MAX)
        effic = ROOM_EFFICIENCY_MAX;
    SYNCDBG(7, "Finished");
    return effic;
}

/**
 * Computes max health of a room of given size.
 */
unsigned long compute_room_max_health(unsigned short slabs_count,unsigned short efficiency)
{
  HitPoints max_health = game.conf.rules[0].workers.hits_per_slab * slabs_count;
  return saturate_set_unsigned(max_health, 16);
}

TbBool update_room_total_health(struct Room *room)
{
    SYNCDBG(17,"Starting for %s index %d",room_code_name(room->kind),(int)room->index);
    room->health = compute_room_max_health(room->slabs_count, room->efficiency);
    return true;
}

TbBool link_room_health(struct Room* linkroom, struct Room* oldroom)
{
    HitPoints newhealth = linkroom->health + oldroom->health;
    HitPoints maxhealth = compute_room_max_health(linkroom->slabs_count, linkroom->efficiency);

    if ((newhealth > maxhealth) || (newhealth <= 0))
    {
        newhealth = maxhealth;
    }
    linkroom->health = newhealth;
    return false;
}

TbBool recalculate_room_health(struct Room* room)
{
    SYNCDBG(7, "Starting for %s index %d", room_code_name(room->kind), (int)room->index);
    HitPoints newhealth = (room->health + game.conf.rules[room->owner].workers.hits_per_slab);
    HitPoints maxhealth = compute_room_max_health(room->slabs_count, room->efficiency);

    if ((newhealth <= maxhealth) && (newhealth >= 0))
    {
        room->health = newhealth;
        return true;
    }
    room->health = maxhealth;
    return false;
}

TbBool update_room_contents(struct Room *room)
{
    const struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    SYNCDBG(17,"Starting for %s index %d",room_code_name(room->kind),(int)room->index);
    Room_Update_Func cb = terrain_room_used_capacity_func_list[roomst->update_storage_in_room_idx];
    if (cb != NULL) {
        cb(room);
    }
    cb = terrain_room_used_capacity_func_list[roomst->update_workers_in_room_idx];
    if (cb != NULL) {
        cb(room);
    }
    return true;
}


/**
 * Checks if a room slab of given kind at given subtile could link to any of rooms at adjacent slabs.
 * @param owner The owning player index of the new room slab.
 * @param x The x subtile of the new room slab.
 * @param y The y subtile of the new room slab.
 * @param rkind  Kind of the new room slab.
 * @return
 */
struct Room* link_adjacent_rooms_of_type(PlayerNumber owner, MapSubtlCoord x, MapSubtlCoord y, RoomKind rkind)
{
    struct Room* room;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    short n;
    // Central slab coords - we will need it if we'll find adjacent room
    MapSlabCoord central_slb_x = subtile_slab(x);
    MapSlabCoord central_slb_y = subtile_slab(y);
    // Localize the room to be merged with other rooms
    struct Room* linkroom = INVALID_ROOM;
    for (n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        stl_x = x + STL_PER_SLB * (long)small_around[n].delta_x;
        stl_y = y + STL_PER_SLB * (long)small_around[n].delta_y;
        room = subtile_room_get(stl_x, stl_y);
        if (!room_is_invalid(room))
        {
            if ((room->owner == owner) && (room->kind == rkind))
            {
                // Add the central slab to room which was found
                room->total_capacity = 0;
                add_slab_to_room_tiles_list(room, central_slb_x, central_slb_y);
                linkroom = room;
                break;
            }
        }
    }
    if (room_is_invalid(linkroom))
    {
        return INVALID_ROOM;
    }
    // If slab was added to the room, check if more rooms now have to be linked together
    for (n++; n < SMALL_AROUND_LENGTH; n++)
    {
        stl_x = x + STL_PER_SLB * (long)small_around[n].delta_x;
        stl_y = y + STL_PER_SLB * (long)small_around[n].delta_y;
        room = subtile_room_get(stl_x, stl_y);
        if (!room_is_invalid(room))
        {
            if ((room->owner == owner) && (room->kind == rkind))
            {
                if (room != linkroom)
                {
                    if (!add_slab_list_to_room_tiles_list(linkroom, room->slabs_list))
                    {
                        return INVALID_ROOM;
                    }
                    // Update slabs in the new list
                    recount_and_reassociate_room_slabs(linkroom);
                    update_room_total_capacity(linkroom);
                    link_room_health(linkroom,room);
                    // Make sure creatures working in the room won't leave
                    change_work_room_of_creatures_working_in_room(room, linkroom);
                    // Get rid of the old room ensign
                    delete_room_flag(room);
                    // Clear list of slabs in the old room
                    room->slabs_count = 0;
                    room->slabs_list = 0;
                    room->slabs_list_tail = 0;
                    // Delete the old room
                    free_room_structure(room);
                }
            }
        }
    }
    return linkroom;
}

TbBool thing_is_on_any_room_tile(const struct Thing *thing)
{
    return subtile_is_room(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

TbBool thing_is_on_own_room_tile(const struct Thing *thing)
{
    return subtile_is_player_room(thing->owner, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

struct Room *get_room_thing_is_on(const struct Thing *thing)
{
    return subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

struct Room *get_room_at_pos(struct Coord3d *pos)
{
     return subtile_room_get(pos->x.stl.num, pos->y.stl.num);
}

void init_room_sparks(struct Room *room)
{
    if (room->kind == RoK_DUNGHEART) {
        return;
    }
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i != 0)
    {
        long slb_x = slb_num_decode_x(i);
        long slb_y = slb_num_decode_y(i);
        // Per room tile code
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        struct SlabMap* sibslb = get_slabmap_block(slb_x, slb_y - 1);
        if (sibslb->room_index != slb->room_index)
        {
            room->flames_around_idx = 1;
            room->flame_stl = 0;
            room->flame_slb = i;
            break;
        }
        // Per room tile code ends
        i = get_next_slab_number_in_room(i);
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
}

TbBool create_effects_on_room_slabs(struct Room *room, ThingModel effkind, long effrange, PlayerNumber effowner)
{
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i != 0)
    {
        long slb_x = slb_num_decode_x(i);
        long slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
        pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
        pos.z.val = subtile_coord_center(1);
        long effect_kind = effkind;
        if (effrange > 0) // TODO: always zero?
            effect_kind += UNSYNC_RANDOM(effrange);
        create_effect(&pos, effect_kind, effowner);
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    return true;
}

/**
 * Clears digging operations for given player on slabs making up given room.
 *
 * @param room The room whose slabs are to be affected.
 * @param plyr_idx Player index whose dig tag shall be cleared.
 */
TbBool clear_dig_on_room_slabs(struct Room *room, PlayerNumber plyr_idx)
{
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i != 0)
    {
        long slb_x = slb_num_decode_x(i);
        long slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        clear_slab_dig(slb_x, slb_y, plyr_idx);
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    return true;
}

TbBool room_has_enough_free_capacity_for_creature_job(const struct Room *room, const struct Thing *creatng, CreatureJob jobpref)
{
    if (!room_role_matches(room->kind,get_room_role_for_job(jobpref))) {
        return false;
    }
    int required_cap = get_required_room_capacity_for_job(jobpref, creatng->model);
    if (room->used_capacity + required_cap <= room->total_capacity)
        return true;
    return false;
}

TbBool find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    if (!room_exists(room)) {
        ERRORLOG("Received nonexisting room for creature");
        return false;
    }
    if (room->slabs_count < 1) {
        ERRORLOG("Number of slabs %d for %s is not positive",(int)room->slabs_count,room_code_name(room->kind));
        return false;
    }
    int navi_radius = abs(thing_nav_block_sizexy(thing) << 8) >> 1;
    unsigned long k;
    long n = THING_RANDOM(thing, room->slabs_count);
    SlabCodedCoords slbnum = room->slabs_list;
    for (k = n; k > 0; k--)
    {
        if (slbnum == 0)
            break;
        slbnum = get_next_slab_number_in_room(slbnum);
    }
    if (slbnum == 0) {
        ERRORLOG("Taking random slab (%d/%d) in %s index %d failed - internal inconsistency.",(int)n,(int)room->slabs_count,room_code_name(room->kind),(int)room->index);
        slbnum = room->slabs_list;
    }
    for (k = 0; k < room->slabs_count; k++)
    {
        MapSlabCoord slb_x = slb_num_decode_x(slbnum);
        MapSlabCoord slb_y = slb_num_decode_y(slbnum);
        int ssub = THING_RANDOM(thing, AROUND_TILES_COUNT);
        for (int snum = 0; snum < AROUND_TILES_COUNT; snum++)
        {
            MapSubtlCoord stl_x = slab_subtile(slb_x, ssub % 3);
            MapSubtlCoord stl_y = slab_subtile(slb_y, ssub / 3);
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if (((mapblk->flags & SlbAtFlg_Blocking) == 0) && (get_navigation_map_floor_height(stl_x,stl_y) < 4))
            {
                if (!terrain_toxic_for_creature_at_position(thing, stl_x, stl_y) && !subtile_has_sacrificial_on_top(stl_x, stl_y))
                {
                    pos->x.val = subtile_coord_center(stl_x);
                    pos->y.val = subtile_coord_center(stl_y);
                    pos->z.val = get_thing_height_at_with_radius(thing, pos, navi_radius);
                    if (!thing_in_wall_at_with_radius(thing, pos, navi_radius)) {
                        return true;
                    }
                }
            }
            ssub = (ssub + 1) % AROUND_TILES_COUNT;
        }
        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0) {
            slbnum = room->slabs_list;
        }
    }
    ERRORLOG("Could not find valid RANDOM point in %s for creature",room_code_name(room->kind));
    return false;
}

/**
 * Returns if given slab lies at outer border of an area.
 * Only slabs which are surrounded by the same slab kind from 4 sides are not at outer border of an area.
 * @param slb_x The slab to be checked, X coordinate.
 * @param slb_y The slab to be checked, y coordinate.
 * @return True if the slab lies at outer border of an area, false otherwise.
 */
TbBool slab_is_area_outer_border(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    // Store kind and owner of the slab
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    SlabKind slbkind = slb->kind;
    PlayerNumber plyr_idx = slabmap_owner(slb);
    for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        long aslb_x = slb_x + (long)small_around[n].delta_x;
        long aslb_y = slb_y + (long)small_around[n].delta_y;
        slb = get_slabmap_block(aslb_x,aslb_y);
        if ((slb->kind != slbkind) || (slabmap_owner(slb) != plyr_idx)) {
            return true;
        }
    }
    return false;
}

/**
 * Returns if given slab lies at inner fill of an area.
 * Only slabs which are surrounded by the same slab kind from all 8 sides are inner fill of an area.
 * @param slb_x The slab to be checked, X coordinate.
 * @param slb_y The slab to be checked, y coordinate.
 * @return True if the slab lies at inner fill of an area, false otherwise.
 */
TbBool slab_is_area_inner_fill(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    // Store kind and owner of the slab
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    SlabKind slbkind = slb->kind;
    PlayerNumber plyr_idx = slabmap_owner(slb);
    for (long n = 0; n < AROUND_EIGHT_LENGTH; n++)
    {
        long aslb_x = slb_x + (long)my_around_eight[n].delta_x;
        long aslb_y = slb_y + (long)my_around_eight[n].delta_y;
        struct SlabMap* aslb = get_slabmap_block(aslb_x, aslb_y);
        if ((aslb->kind != slbkind) || (slabmap_owner(aslb) != plyr_idx)) {
            return false;
        }
    }
    return true;
}

TbBool find_random_position_at_area_of_room(struct Coord3d *pos, const struct Room *room, unsigned char room_area,
        struct Thing *thing)
{
    // Find a random slab in the room to be used as our starting point
    long i = THING_RANDOM(thing, room->slabs_count);
    unsigned long n = room->slabs_list;
    while (i > 0)
    {
        n = get_next_slab_number_in_room(n);
        i--;
    }
    // Now loop starting from that point
    i = room->slabs_count;
    while (i > 0)
    {
        // Loop the slabs list
        if (n <= 0) {
            n = room->slabs_list;
        }
        MapSlabCoord slb_x = slb_num_decode_x(n);
        MapSlabCoord slb_y = slb_num_decode_y(n);
        if  ((room_area == RoArC_ANY)
         || ((room_area == RoArC_BORDER) && slab_is_area_outer_border(slb_x, slb_y))
         || ((room_area == RoArC_CENTER) && slab_is_area_inner_fill(slb_x, slb_y)))
        {
            // In case we will select a column on that subtile, do 3 tries
            for (int k = 0; k < 3; k++)
            {
                pos->x.val = subtile_coord(slab_subtile(slb_x,0),THING_RANDOM(thing, COORD_PER_SLB));
                pos->y.val = subtile_coord(slab_subtile(slb_y,0),THING_RANDOM(thing, COORD_PER_SLB));
                pos->z.val = subtile_coord(1,0);
                struct Map* mapblk = get_map_block_at(pos->x.stl.num, pos->y.stl.num);
                if (((mapblk->flags & SlbAtFlg_Blocking) == 0) && ((mapblk->flags & SlbAtFlg_IsDoor) == 0)
                    && (get_navigation_map_floor_height(pos->x.stl.num, pos->y.stl.num) < 4)) {
                    return true;
                }
            }
        }
        n = get_next_slab_number_in_room(n);
        i--;
    }
    return false;
}

/**
 * Finds a room with space item slot for storage.
 * Note that this function may return a room filled to its full by workers. Only item storage
 * is used to determine whether the room is full.
 * @param plyr_idx
 * @param rkind
 * @return
 */
struct Room *find_room_of_role_with_spare_room_item_capacity(PlayerNumber plyr_idx, RoomRole rrole)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    struct Room* room;
    if (dungeon_invalid(dungeon))
        return INVALID_ROOM;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            room = find_nth_room_of_owner_with_spare_item_capacity_starting_with(dungeon->room_list_start[rkind], 0, 1);
            if(room != INVALID_ROOM){
                return room;
            }
        }
    }
    return INVALID_ROOM;
}

struct Room *find_room_of_role_for_thing_with_used_capacity(const struct Thing *creatng, PlayerNumber plyr_idx, RoomRole rrole, unsigned char nav_flags, long min_used_cap)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    unsigned long k = 0;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(!room_role_matches(rkind,rrole))
        {
            continue;
        }
        long i = dungeon->room_list_start[rkind];
        while (i != 0)
        {
            struct Room* room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            struct Coord3d pos;
            if ((room->used_capacity >= min_used_cap) && find_first_valid_position_for_thing_anywhere_in_room(creatng, room, &pos))
            {
                if (thing_is_creature(creatng))
                {
                    if (creature_can_navigate_to(creatng, &pos, nav_flags)) {
                        return room;
                    }
                } else
                {
                    return room;
                }
            }
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }

    return INVALID_ROOM;
}

/**
 * Searches for room of given kind and owner which has no less than given spare capacity.
 * @param owner
 * @param rkind
 * @param spare
 * @return
 * @note Function find_room_with_spare_room_capacity() should also redirect to this one.
 */
struct Room *find_room_of_role_with_spare_capacity(PlayerNumber owner, RoomRole rrole, long spare)
{
    struct Room *room;
    struct Dungeon* dungeon = get_dungeon(owner);
    if (dungeon_invalid(dungeon))
        return INVALID_ROOM;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            room = find_nth_room_of_owner_with_spare_capacity_starting_with(dungeon->room_list_start[rkind], 0, spare);
            if(room != INVALID_ROOM)
                return room;
        }
    }
    return INVALID_ROOM;
}

struct Room *find_nth_room_of_owner_with_spare_capacity_starting_with(long room_idx, long n, long spare)
{
    SYNCDBG(18,"Starting");
    unsigned long k = 0;
    int i = room_idx;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->used_capacity + spare <= room->total_capacity)
        {
            if (n > 0) {
                n--;
            } else {
                return room;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return INVALID_ROOM;
}

struct Room *find_nth_room_of_owner_with_spare_item_capacity_starting_with(long room_idx, long n, long spare)
{
    SYNCDBG(18,"Starting");
    unsigned long k = 0;
    int i = room_idx;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->capacity_used_for_storage + spare <= room->total_capacity)
        {
            if (n > 0) {
                n--;
            } else {
                return room;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return INVALID_ROOM;
}

struct Room *find_room_of_role_with_most_spare_capacity(const struct Dungeon *dungeon,RoomRole rrole, int32_t *total_spare_cap)
{
    SYNCDBG(18,"Starting");
    long loc_total_spare_cap = 0;
    struct Room* max_spare_room = INVALID_ROOM;
    long max_spare_cap = 0;
    unsigned long k = 0;

    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            int i = dungeon->room_list_start[rkind];
            while (i != 0)
            {
                struct Room* room = room_get(i);
                if (room_is_invalid(room))
                {
                    ERRORLOG("Jump to invalid room detected");
                    break;
                }
                i = room->next_of_owner;
                // Per-room code
                if (room->total_capacity > room->used_capacity)
                {
                    long delta = room->total_capacity - room->used_capacity;
                    loc_total_spare_cap += delta;
                    if (max_spare_cap < delta)
                    {
                        max_spare_cap = delta;
                        max_spare_room = room;
                    }
                }
                // Per-room code ends
                k++;
                if (k > ROOMS_COUNT)
                {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
                }
            }
        }
    }


    if (total_spare_cap != NULL)
    (*total_spare_cap) = loc_total_spare_cap;
    return max_spare_room;
}

/**
 * Retrieves a position in room which could be used as a place for thing to enter that room.
 * Returns first valid position found.
 * @param thing
 * @param room
 * @param pos
 * @return
 * @note originally find_first_valid_position_for_thing_in_room()
 * @see creature_move_to_place_in_room() an alternative which can return only border or inner slabs when needed
 */
TbBool find_first_valid_position_for_thing_anywhere_in_room(const struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    if (!room_exists(room))
    {
        ERRORLOG("Tried to find position in non-existing room");
        pos->x.val = subtile_coord_center(game.map_subtiles_x/2);
        pos->y.val = subtile_coord_center(game.map_subtiles_y/2);
        pos->z.val = subtile_coord(1,0);
        return false;
    }
    long block_radius = subtile_coord(thing_nav_block_sizexy(thing), 0) / 2;

    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        MapSubtlCoord slb_x = slb_num_decode_x(i);
        MapSubtlCoord slb_y = slb_num_decode_y(i);
        // Per-slab code
        for (long dy = 0; dy < 3; dy++)
        {
            for (long dx = 0; dx < 3; dx++)
            {
                MapSubtlCoord stl_x = 3 * slb_x + dx;
                MapSubtlCoord stl_y = 3 * slb_y + dy;
                struct Map* mapblk = get_map_block_at(stl_x, stl_y);
                // Check if the position isn't filled with solid block
                if (((mapblk->flags & SlbAtFlg_Blocking) == 0) && (get_navigation_map_floor_height(stl_x,stl_y) < 4))
                {
                    if (!terrain_toxic_for_creature_at_position(thing, stl_x, stl_y) && !subtile_has_sacrificial_on_top(stl_x, stl_y))
                    {
                        pos->x.val = subtile_coord_center(stl_x);
                        pos->y.val = subtile_coord_center(stl_y);
                        pos->z.val = get_thing_height_at_with_radius(thing, pos, block_radius);
                        if ( !thing_in_wall_at_with_radius(thing, pos, block_radius) ) {
                          return true;
                        }
                    }
                }
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
    ERRORLOG("Could not find valid FIRST point in %s for %s",room_code_name(room->kind),thing_model_name(thing));
    pos->x.val = subtile_coord_center(game.map_subtiles_x/2);
    pos->y.val = subtile_coord_center(game.map_subtiles_y/2);
    pos->z.val = subtile_coord(1,0);
    return false;
}

struct Room *find_nearest_room_of_role_for_thing_with_spare_capacity(struct Thing *thing, signed char owner, RoomRole rrole, unsigned char nav_flags, long spare)
{
    SYNCDBG(18,"Searching for %s with capacity for %s index %d",room_role_code_name(rrole),thing_model_name(thing),(int)thing->index);
    struct Dungeon* dungeon = get_dungeon(owner);
    struct Room* nearoom = INVALID_ROOM;
    long neardistance = INT32_MAX;

    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            unsigned long k = 0;
            int i = dungeon->room_list_start[rkind];
            while (i != 0)
            {
                struct Room* room = room_get(i);
                if (room_is_invalid(room))
                {
                    ERRORLOG("Jump to invalid room detected");
                    break;
                }
                i = room->next_of_owner;
                // Per-room code
                // Compute simplified distance - without use of mul or div
                long distance = grid_distance(thing->mappos.x.stl.num, thing->mappos.y.stl.num, room->central_stl_x, room->central_stl_y);
                if ((neardistance > distance) && (room->used_capacity + spare <= room->total_capacity))
                {
                    struct Coord3d pos;
                    if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos))
                    {
                        if ((thing->class_id != TCls_Creature)
                        || creature_can_navigate_to(thing, &pos, nav_flags))
                        {
                            neardistance = distance;
                            nearoom = room;
                        }
                    }
                }
                // Per-room code ends
                k++;
                if (k > ROOMS_COUNT)
                {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
                }
            }
        }
    }

    return nearoom;
}

/**
 * Counts all room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_flags
 * @return
 */
static long count_rooms_with_used_capacity_creature_can_navigate_to(struct Thing *thing, PlayerNumber owner, RoomKind rrole, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = get_dungeon(owner);
    long count = 0;
    unsigned long k = 0;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            int i = dungeon->room_list_start[rkind];
            while (i != 0)
            {
                struct Room* room = room_get(i);
                if (room_is_invalid(room))
                {
                    ERRORLOG("Jump to invalid room detected");
                    break;
                }
                i = room->next_of_owner;
                // Per-room code
                struct Coord3d pos;
                if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos) && (room->used_capacity > 0))
                {
                    if (creature_can_navigate_to(thing, &pos, nav_flags))
                    {
                        count++;
                    }
                }
                // Per-room code ends
                k++;
                if (k > ROOMS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping rooms list");
                    break;
                }
            }
        }
    }

    return count;
}

/**
 * Gives the n-th room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_flags
 * @param n
 * @return
 */
struct Room* find_room_of_kind_creature_can_navigate_to(struct Thing* thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags)
{
    struct Dungeon* dungeon = get_dungeon(owner);
    unsigned long k = 0;

    int i = dungeon->room_list_start[rkind];
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        struct Coord3d pos;
        if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos))
        {
            if (creature_can_navigate_to(thing, &pos, nav_flags))
            {
                return room;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return INVALID_ROOM;
}

/**
 * Gives the n-th room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param role
 * @param nav_flags
 * @param n
 * @return
 */
struct Room *find_nth_room_with_used_capacity_creature_can_navigate_to(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags, long n)
{
    struct Dungeon* dungeon = get_dungeon(owner);
    unsigned long k = 0;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            int i = dungeon->room_list_start[rkind];
            while (i != 0)
            {
                struct Room* room = room_get(i);
                if (room_is_invalid(room))
                {
                    ERRORLOG("Jump to invalid room detected");
                    break;
                }
                i = room->next_of_owner;
                // Per-room code
                struct Coord3d pos;
                if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos) && (room->used_capacity > 0))
                {
                    if (creature_can_navigate_to(thing, &pos, nav_flags))
                    {
                        if (n > 0) {
                            n--;
                        } else {
                            return room;
                        }
                    }
                }
                // Per-room code ends
                k++;
                if (k > ROOMS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping rooms list");
                    break;
                }
            }
        }
    }

    return INVALID_ROOM;
}

/**
 * Returns if a creature can navigate to any kind of players rooms.
 * Only rooms which have used capacity are taken into account.
 * @param thing
 * @param owner
 * @return
 */
TbBool creature_can_get_to_any_of_players_rooms(struct Thing *thing, PlayerNumber owner)
{
    for (RoomKind rkind = 1; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        struct Room* room = find_room_of_kind_creature_can_navigate_to(thing, owner, rkind, NavRtF_NoOwner);
        if (!room_is_invalid(room))
            return true;
    }
    return false;
}

/**
 * Gives a room of given kind and owner where the creature can navigate to.
 * Counts all possible rooms, then selects one and returns it.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_flags
 * @return
 */
struct Room *find_random_room_of_role_with_used_capacity_creature_can_navigate_to(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    long count = count_rooms_with_used_capacity_creature_can_navigate_to(thing, owner, rrole, nav_flags);
    if (count < 1)
        return INVALID_ROOM;
    long selected = THING_RANDOM(thing, count);
    return find_nth_room_with_used_capacity_creature_can_navigate_to(thing, owner, rrole, nav_flags, selected);
}

/**
 * Searches for players room of given kind which center is closest to given position.
 * Computes geometric distance - does not include any map obstacles in computations.
 *
 * @param plyr_idx Player of which room distance we want.
 * @param rkind Room kind of which all rooms are to be checked.
 * @param pos Position to be closest to.
 * @param room_distance Output variable which returns the closest distance, in map coords.
 * @return
 */
struct Room *find_room_nearest_to_position(PlayerNumber plyr_idx, RoomKind rkind, const struct Coord3d *pos, int32_t *room_distance)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long near_distance = INT32_MAX;
    struct Room* near_room = INVALID_ROOM;
    long i = dungeon->room_list_start[rkind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        MapCoordDelta delta_x = subtile_coord_center(room->central_stl_x) - (MapCoordDelta)pos->x.val;
        MapCoordDelta delta_y = subtile_coord_center(room->central_stl_y) - (MapCoordDelta)pos->y.val;
        long distance = LbDiagonalLength(abs(delta_x), abs(delta_y));
        if (distance < near_distance)
        {
            near_room = room;
            near_distance = distance;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    *room_distance = near_distance;
    return near_room;
}

long get_room_attractiveness_for_thing(const struct Dungeon *dungeon, const struct Room *room, const struct Thing *thing, RoomRole rrole, int needed_capacity)
{
    // Says how attractive is a specific room, based on some room-specific code below
    long attractiveness = 16; // Default attractiveness
    if ((rrole & RoRoF_GoldStorage) != 0)
    {
        long salary = calculate_correct_creature_pay(thing);
        if (room->capacity_used_for_storage + dungeon->offmap_money_owned < salary) {
            // This room isn't attractive at all - creature won't get salary there
            attractiveness = 0;
        } else {
            attractiveness += 2 * min(room->capacity_used_for_storage/(salary+1),16);
        }
    }
    if ((rrole & (RoRoF_CrHealSleep|RoRoF_LairStorage)) != 0)
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (room->index == cctrl->lairtng_idx) {
            // A room where we already have a lair is a few times more attractive
            attractiveness += 64;
        } else {
            attractiveness += 2 * min(max(room->total_capacity - (int)room->used_capacity,0),16);
        }
    }
    if ((rrole & RoRoF_FoodStorage) != 0)
    {
        attractiveness += min(room->used_capacity,16);
    }
    if ((rrole & (RoRoF_CratesStorage|RoRoF_PowersStorage|RoRoF_DeadStorage|RoRoF_Prison|RoRoF_Torture|RoRoF_CrHappyPray|RoRoF_CrMakeGroup|RoRoF_CrPurifySpell)) != 0)
    {
        if (room->used_capacity+needed_capacity > room->total_capacity) {
            // This room isn't attractive at all - creature won't get job there
            attractiveness = 0;
        } else {
            attractiveness += min(max(room->total_capacity - (int)room->used_capacity,0),16);
        }
    }
    if ((rrole & (RoRoF_CrScavenge|RoRoF_CrTrainExp|RoRoF_Research|RoRoF_CratesManufctr|RoRoF_CrGuard)) != 0)
    {
        if (room->used_capacity+needed_capacity > room->total_capacity) {
            // This room isn't attractive at all - creature won't get job there
            attractiveness = 0;
        } else {
            // There is no need to consider work value of the creature, as creature and room role are already chosen
            attractiveness += min(max(room->total_capacity - (int)room->used_capacity,0),16);
            // But specific room is still to be selected, so efficiency might help; allow it to increase attractiveness
            attractiveness += room->efficiency / (ROOM_EFFICIENCY_MAX/16);
        }
    }
    if (attractiveness > 0)
    {
        struct Thing* enmtng = get_creature_in_range_who_is_enemy_of_able_to_attack_and_not_specdigger(thing->mappos.x.val, thing->mappos.y.val, 10, thing->owner);
        if (!thing_is_invalid(enmtng)) {
            // A room with enemies inside is very unattractive, but still possible to select
            attractiveness = 1;
        }
    }
    return attractiveness;
}

struct Room *get_room_of_given_role_for_thing(const struct Thing *thing, const struct Dungeon *dungeon, RoomRole rrole, int needed_capacity)
{
    long retdist = INT32_MAX;
    struct Room* retroom = INVALID_ROOM;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if (!room_role_matches(rkind, rrole)) {
            continue;
        }
        long i = dungeon->room_list_start[rkind];
        unsigned long k = 0;
        while (i != 0)
        {
            struct Room* room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            long attractiveness = get_room_attractiveness_for_thing(dungeon, room, thing, rrole & get_room_roles(room->kind), needed_capacity);
            if (attractiveness > 0)
            {
                long dist = grid_distance(thing->mappos.x.stl.num, thing->mappos.y.stl.num, room->central_stl_x, room->central_stl_y);
                dist = (dist*128)/attractiveness;
                if (retdist > dist)
                {
                    retdist = dist;
                    retroom = room;
                }
            }
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }
    return retroom;
}

/**
 * Counts all room of given kind and owner where the creature can navigate to.
 * Works only for rooms which store items.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_flags
 * @return
 */
long count_rooms_for_thing(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = get_dungeon(owner);
    long count = 0;
    unsigned long k = 0;
    int i = dungeon->room_list_start[rkind];
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        struct Coord3d pos;
        if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos))
        {
            if (!thing_is_creature(thing) || creature_can_navigate_to(thing, &pos, nav_flags))
            {
                count++;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return count;
}

/**
 * Counts all room of given role and owner where the creature can navigate to.
 * Works only for rooms which store items.
 * @param thing
 * @param owner
 * @param role
 * @param nav_flags
 * @return
 */
long count_rooms_of_role_for_thing(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags)
{
    long result = 0;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            result += count_rooms_for_thing(thing, owner, rkind, nav_flags);
        }
    }
    return result;
}

struct Room* find_first_room_of_role(PlayerNumber owner, RoomRole rrole)
{
    struct Dungeon* dungeon = get_dungeon(owner);
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if (room_role_matches(rkind, rrole))
        {
            int i = dungeon->room_list_start[rkind];
            if (i != 0)
            {
                return room_get(i);
            }
        }
    }
    return INVALID_ROOM;
}

/**
 * Gives the n-th room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_flags
 * @param n
 * @return
 */
struct Room *find_nth_room_of_role_for_thing(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags, long n)
{
    struct Dungeon* dungeon = get_dungeon(owner);
    unsigned long k = 0;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            int i = dungeon->room_list_start[rkind];
            while (i != 0)
            {
                struct Room* room = room_get(i);
                if (room_is_invalid(room))
                {
                    ERRORLOG("Jump to invalid room detected");
                    break;
                }
                i = room->next_of_owner;
                // Per-room code
                struct Coord3d pos;
                if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos))
                {
                    if (!thing_is_creature(thing) || creature_can_navigate_to(thing, &pos, nav_flags))
                    {
                        if (n > 0) {
                            n--;
                        } else {
                            return room;
                        }
                    }
                }
                // Per-room code ends
                k++;
                if (k > ROOMS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping rooms list");
                    break;
                }
            }
        }
    }
    return INVALID_ROOM;
}

struct Room *find_random_room_of_role_for_thing(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    long count = count_rooms_of_role_for_thing(thing, owner, rrole, nav_flags);
    if (count < 1)
        return INVALID_ROOM;
    long selected = THING_RANDOM(thing, count);
    return find_nth_room_of_role_for_thing(thing, owner, rrole, nav_flags, selected);
}

/**
 * Counts all room of given kind and owner where the creature can navigate to.
 * Works only for rooms which store items.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_flags
 * @return
 */
static long count_rooms_of_role_for_thing_with_spare_room_item_capacity(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = get_dungeon(owner);
    long count = 0;
    unsigned long k = 0;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(!room_role_matches(rkind,rrole))
        {
            continue;
        }
        int i = dungeon->room_list_start[rkind];
        while (i != 0)
        {
            struct Room* room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            struct Coord3d pos;
            if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos) && (room->total_capacity > room->capacity_used_for_storage))
            {
                if (!thing_is_creature(thing) || creature_can_navigate_to(thing, &pos, nav_flags))
                {
                    count++;
                }
            }
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }

    return count;
}

/**
 * Gives the n-th room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_flags
 * @param n
 * @return
 */
static struct Room *find_nth_room_of_role_for_thing_with_spare_room_item_capacity(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags, long n)
{
    struct Dungeon* dungeon = get_dungeon(owner);
    unsigned long k = 0;
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(!room_role_matches(rkind,rrole))
        {
            continue;
        }
        int i = dungeon->room_list_start[rkind];
        while (i != 0)
        {
            struct Room* room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            struct Coord3d pos;
            if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos) && (room->total_capacity > room->capacity_used_for_storage))
            {
                if (!thing_is_creature(thing) || creature_can_navigate_to(thing, &pos, nav_flags))
                {
                    if (n > 0) {
                        n--;
                    } else {
                        return room;
                    }
                }
            }
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }
    return INVALID_ROOM;
}

struct Room * find_random_room_of_role_for_thing_with_spare_room_item_capacity(struct Thing *thing, PlayerNumber owner, RoomRole rrole, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    long count = count_rooms_of_role_for_thing_with_spare_room_item_capacity(thing, owner, rrole, nav_flags);
    if (count < 1)
        return INVALID_ROOM;
    long selected = THING_RANDOM(thing, count);
    return find_nth_room_of_role_for_thing_with_spare_room_item_capacity(thing, owner, rrole, nav_flags, selected);
}



void delete_room_slab_when_no_free_room_structures(MapCoord slb_x, MapCoord slb_y, unsigned char gnd_slab)
{
    SYNCDBG(8,"Starting");

    struct Room *room;
    SlabCodedCoords room_slab;


    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    room = room_get(slb->room_index);

    SlabCodedCoords slb_num = get_slab_number(slb_x, slb_y);

    if ( room_is_invalid(room) )
    {
        ERRORLOG("This is not a room slab");
        return;
    }

    decrease_room_area(room->owner, 1);
    kill_room_slab_and_contents(room->owner, slb_x, slb_y);
    if ( room->slabs_count == 1 )
    {
        delete_room_flag(room);
        replace_room_slab(room, slb_x, slb_y, room->owner, gnd_slab);
        room_slab = room->slabs_list;
        if ( room_slab )
        {
            do
            {
                MapCoord roomslb_x = slb_num_decode_x(room_slab);
                MapCoord roomslb_y = slb_num_decode_y(room_slab);
                struct SlabMap* roomslb = get_slabmap_block(roomslb_x, roomslb_y);
                room_slab = roomslb->next_in_room;
                kill_room_slab_and_contents(room->owner, roomslb_x, roomslb_y);
                roomslb->next_in_room = 0;
            }
            while ( room_slab );
        }
        delete_room_flag(room);
        free_room_structure(room);
        do_slab_efficiency_alteration(slb_x, slb_y);
    }
    else
    {
        room_slab = room->slabs_list;
        if ( slb_num == room_slab )
        {
            delete_room_flag(room);
            room->slabs_list = get_next_slab_number_in_room(slb_num);
            create_room_flag(room);
        }
        else if ( room->slabs_list )
        {
            MapCoord roomslb_x = slb_num_decode_x(room_slab);
            MapCoord roomslb_y = slb_num_decode_y(room_slab);
            struct SlabMap* roomslb = get_slabmap_block(roomslb_x, roomslb_y);
            do
            {
                roomslb_x = slb_num_decode_x(room_slab);
                roomslb_y = slb_num_decode_y(room_slab);
                roomslb = get_slabmap_block(roomslb_x, roomslb_y);
                room_slab = roomslb->next_in_room;
                if ( roomslb->next_in_room == slb_num )
                    roomslb->next_in_room = slb->next_in_room;
                room_slab = roomslb->next_in_room;
            }
            while ( roomslb->next_in_room != 0 );
        }
        replace_room_slab(room, slb_x, slb_y, room->owner, gnd_slab);
        slb->next_in_room = 0;
    }
}

TbBool find_random_valid_position_for_thing_in_room_avoiding_object_excluding_room_slab(struct Thing *thing, struct Room *room, struct Coord3d *pos, long slbnum)
{
    int nav_sizexy = subtile_coord(thing_nav_block_sizexy(thing), 0);
    if (room_is_invalid(room) || (room->slabs_count <= 0)) {
        ERRORLOG("Invalid room or number of slabs is zero");
        return false;
    }
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    long selected = THING_RANDOM(thing, room->slabs_count);
    unsigned long n = 0;
    long i = room->slabs_list;
    // Get the selected index
    while (i != 0)
    {
        // Per room tile code
        if (n >= selected)
        {
            break;
        }
        // Per room tile code ends
        do
        {
            i = get_next_slab_number_in_room(i);
        }
        while (i == slbnum);
        n++;
    }
    if (i == 0)
    {
        if (n < room->slabs_count)
        {
            WARNLOG("Number of slabs in %s (%lu) is smaller than count (%u)",room_code_name(room->kind), n, room->slabs_count);
        }
        n = 0;
        i = room->slabs_list;
        while (i == slbnum)
        {
            i = get_next_slab_number_in_room(i);
        }
    }
    // Sweep rooms starting on that index
    unsigned long k = 0;
    long nround;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    while (i != 0)
    {
        if (i != slbnum)
        {
            stl_x = slab_subtile(slb_num_decode_x(i), 0);
            stl_y = slab_subtile(slb_num_decode_y(i), 0);
            // Per room tile code
            MapSubtlCoord start_stl = THING_RANDOM(thing, AROUND_TILES_COUNT);
            for (nround = 0; nround < AROUND_TILES_COUNT; nround++)
            {
                MapSubtlCoord x = start_stl % 3 + stl_x;
                MapSubtlCoord y = start_stl / 3 + stl_y;
                if ((roomst->storage_height < 0) || (get_floor_filled_subtiles_at(x, y) == roomst->storage_height))
                {
                    struct Thing* objtng = find_base_thing_on_mapwho(TCls_Object, 0, x, y);
                    if (thing_is_invalid(objtng))
                    {
                        pos->x.val = subtile_coord_center(x);
                        pos->y.val = subtile_coord_center(y);
                        pos->z.val = get_thing_height_at_with_radius(thing, pos, nav_sizexy);
                        if (!thing_in_wall_at_with_radius(thing, pos, nav_sizexy)) {
                            return true;
                        }
                    }
                }
                start_stl = (start_stl + 1) % 9;
            }
        }
        // Per room tile code ends
        if (n+1 >= room->slabs_count)
        {
            n = 0;
            i = room->slabs_list;
            while (i == slbnum)
            {
                i = get_next_slab_number_in_room(i);
            }
        } else {
            n++;
            do
            {
                i = get_next_slab_number_in_room(i);
            }
            while (i == slbnum);
        }
        k++;
        if (k > room->slabs_count) {
            ERRORLOG("Infinite loop detected when sweeping room slabs list");
            break;
        }
    }
    // if the above fails, do a more thorough check
    if (room->used_capacity <= room->total_capacity)
    {
        SYNCLOG("Could not find valid random point in %s %d for %s. Attempting thorough check.",room_code_name(room->kind),room->index,thing_model_name(thing));
        k = 0;
        for (i = room->slabs_list; (i != 0); i = get_next_slab_number_in_room(i))
        {
            if (i != slbnum)
            {
                stl_x = slab_subtile_center(slb_num_decode_x(i));
                stl_y = slab_subtile_center(slb_num_decode_y(i));
                for (nround = 0; nround < (AROUND_TILES_COUNT - 1); nround++)
                {
                    MapSubtlCoord astl_x = stl_x + around[nround].delta_x;
                    MapSubtlCoord astl_y = stl_y + around[nround].delta_y;
                    if ((roomst->storage_height < 0) || (get_floor_filled_subtiles_at(astl_x, astl_y) == roomst->storage_height))
                    {
                        struct Thing* objtng = find_base_thing_on_mapwho(TCls_Object, 0, astl_x, astl_y);
                        if (thing_is_invalid(objtng))
                        {
                            pos->x.val = subtile_coord_center(astl_x);
                            pos->y.val = subtile_coord_center(astl_y);
                            pos->z.val = get_thing_height_at_with_radius(thing, pos, nav_sizexy);
                            if (!thing_in_wall_at_with_radius(thing, pos, nav_sizexy)) {
                                SYNCLOG("Thorough check succeeded where random check failed.");
                                return true;
                            }
                        }
                    }
                }
            }
            k++;
            if (k > room->slabs_count)
            {
                ERRORLOG("Infinite loop detected when sweeping room slabs list");
                break;
            }
        }
    }
    SYNCLOG("Could not find any valid point in %s %d for %s",room_code_name(room->kind),room->index,thing_model_name(thing));
    return false;
}

long find_random_valid_position_for_item_in_different_room_avoiding_object(struct Thing *thing, struct Room *skip_room, struct Coord3d *pos)
{
    struct Dungeon* dungeon = get_dungeon(skip_room->owner);
    unsigned int matching_rooms = 0;
    long i;
    unsigned long k = 0;
    struct Room* room;
    for (i = dungeon->room_list_start[skip_room->kind]; (i != 0); i = room->next_of_owner)
    {
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
        room = room_get(i);
        if (room_is_invalid(room))
        {
          ERRORLOG("Jump to invalid room detected");
          break;
        }
        if (room->index == skip_room->index)
        {
            continue;
        }
        // Per-room code
        if (room->total_capacity > room->capacity_used_for_storage)
        {
            matching_rooms++;
        }
        // Per-room code ends
    }
    if (matching_rooms == 0)
    {
        return 0;
    }
    int chosen_match_idx = THING_RANDOM(thing, matching_rooms);
    int curr_match_idx = 0;
    k = 0;
    for (i = dungeon->room_list_start[skip_room->kind]; (i != 0); i = room->next_of_owner)
    {
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
        room = room_get(i);
        if (room_is_invalid(room))
        {
          ERRORLOG("Jump to invalid room detected");
          break;
        }
        if (room->index == skip_room->index)
        {
            continue;
        }
        if (room->total_capacity > room->capacity_used_for_storage)
        {
            if (curr_match_idx >= chosen_match_idx)
            {
                if (find_random_valid_position_for_thing_in_room_avoiding_object(thing, room, pos)) {
                    return 1;
                }
            }
            curr_match_idx++;
            if (curr_match_idx >= matching_rooms) {
                // All rooms which were matched are checked
                break;
            }
        }
    }
    ERRORLOG("Found %d matching rooms but couldn't find position within any",(int)matching_rooms);
    return 0;
}

void kill_room_contents_at_subtile(struct Room *room, PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, SlabCodedCoords slbnum)
{
    struct Thing *thing;
    struct Dungeon* dungeon;
    struct RoomConfigStats* roomst;
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing_is_creature(thing))
        {
            if (creature_is_being_sacrificed(thing))
            {
                kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects);
            } else
            if (creature_is_working_in_room(thing, room))
            {
                set_start_state(thing);
            }
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

    if (room_role_matches(room->kind, RoRoF_GoldStorage))
    {
        roomst = get_room_kind_stats(room->kind);
        if ((roomst->storage_height < 0) || (get_map_floor_filled_subtiles(mapblk) == roomst->storage_height))
        {
            struct Thing *gldtng;
            gldtng = find_gold_hoard_at(stl_x, stl_y);
            while (!thing_is_invalid(gldtng)) //Normally there is just a single hoard at a slab, but mapmakers may place more.
            {
                room->capacity_used_for_storage -= gldtng->valuable.gold_stored;
                dungeon = get_dungeon(plyr_idx);
                if (!dungeon_invalid(dungeon)) {
                    dungeon->total_money_owned -= gldtng->valuable.gold_stored;
                }
                drop_gold_pile(gldtng->valuable.gold_stored, &gldtng->mappos);
                delete_thing_structure(gldtng, 0);
                gldtng = find_gold_hoard_at(stl_x, stl_y);
            }
        }
    }
    if (room_role_matches(room->kind, RoRoF_PowersStorage))
    {
        k = 0;
        i = get_mapwho_thing_index(mapblk);
        while (i != 0)
        {
            thing = thing_get(i);
            TRACE_THING(thing);
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Jump to invalid thing detected");
                break;
            }
            i = thing->next_on_mapblk;
            // Per thing code start
            if (thing_is_spellbook(thing) && ((thing->alloc_flags & TAlF_IsDragged) == 0))
            {
                PowerKind spl_idx = book_thing_to_power_kind(thing);
                dungeon = get_dungeon(plyr_idx);
                if (thing->owner == room->owner)
                {
                    struct Coord3d pos;
                    // Try to move spellbook within the room
                    if (find_random_valid_position_for_thing_in_room_avoiding_object_excluding_room_slab(thing, room, &pos, slbnum))
                    {
                        pos.z.val = get_thing_height_at(thing, &pos);
                        move_thing_in_map(thing, &pos);
                        create_effect(&pos, TngEff_RoomSparkeLarge, thing->owner);
                    } else
                    // Try to move spellbook to another library, if it's the only copy
                    if ((dungeon->magic_level[spl_idx] < 2) && (find_random_valid_position_for_item_in_different_room_avoiding_object(thing, room, &pos)))
                    {
                        move_thing_to_different_room(thing, &pos);
                    } else
                    // Cannot store the spellbook anywhere - remove the spell
                    {
                        if (!is_neutral_thing(thing))
                        {
                            if (dungeon->magic_level[spl_idx] >= 2)
                            {
                                SYNCLOG("Deleting duplicate object %s from %s of player %d ", object_code_name(thing->model), room_code_name(room->kind), (int)thing->owner);
                            }
                            else
                            {
                                SYNCLOG("Found no new location for object %s in %s for player %d, deleting object", object_code_name(thing->model), room_code_name(room->kind), (int)thing->owner);
                                dungeon->magic_resrchable[spl_idx] = 1;
                            }
                            remove_power_from_player(spl_idx, thing->owner);
                        }
                        delete_thing_structure(thing, 0);
                    }
                }
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
    }
    if (room_role_matches(room->kind, RoRoF_CratesStorage))
    {
        k = 0;
        i = get_mapwho_thing_index(mapblk);
        while (i != 0)
        {
            thing = thing_get(i);
            TRACE_THING(thing);
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Jump to invalid thing detected");
                break;
            }
            i = thing->next_on_mapblk;
            // Per thing code start
            if (thing_is_workshop_crate(thing) && !thing_is_dragged_or_pulled(thing) && (thing->owner == room->owner))
            {
                struct Coord3d pos;
                // Try to move crate within the room
                if (find_random_valid_position_for_thing_in_room_avoiding_object_excluding_room_slab(thing, room, &pos, slbnum))
                {
                    pos.z.val = get_thing_height_at(thing, &pos);
                    move_thing_in_map(thing, &pos);
                    create_effect(&pos, TngEff_RoomSparkeLarge, thing->owner);
                } else
                // Try to move crate to another workshop
                if (find_random_valid_position_for_item_in_different_room_avoiding_object(thing, room, &pos))
                {
                    pos.z.val = get_thing_height_at(thing, &pos);
                    move_thing_in_map(thing, &pos);
                    create_effect(&pos, TngEff_RoomSparkeLarge, thing->owner);
                    struct Room *nxroom;
                    nxroom = get_room_thing_is_on(thing);
                    update_room_contents(nxroom);
                } else
                // Cannot store the crate anywhere - remove it
                {
                    ThingClass tngclass;
                    ThingModel tngmodel;
                    tngclass = crate_thing_to_workshop_item_class(thing);
                    tngmodel = crate_thing_to_workshop_item_model(thing);
                    if (remove_workshop_item_from_amount_stored(thing->owner, tngclass, tngmodel, WrkCrtF_NoOffmap) > WrkCrtS_None) {
                        remove_workshop_item_from_amount_placeable(thing->owner, tngclass, tngmodel);
                    }
                    delete_thing_structure(thing, 0);
                }
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
    }
    if (room_role_matches(room->kind, RoRoF_FoodStorage))
    {
        k = 0;
        i = get_mapwho_thing_index(mapblk);
        while (i != 0)
        {
            thing = thing_get(i);
            TRACE_THING(thing);
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Jump to invalid thing detected");
                break;
            }
            i = thing->next_on_mapblk;
            // Per thing code start
            if (thing_is_object(thing))
            {
                if (object_is_infant_food(thing) || object_is_mature_food(thing) || object_is_growing_food(thing)) {
                    delete_thing_structure(thing, 0);
                }
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
    }
    if (room_role_matches(room->kind, RoRoF_LairStorage))
    {
        thing = find_lair_totem_at(stl_x, stl_y);
        if (!thing_is_invalid(thing))
        {
            if (thing->lair.belongs_to)
            {
                struct Thing *tmptng;
                struct CreatureControl *cctrl;
                tmptng = thing_get(thing->lair.belongs_to);
                cctrl = creature_control_get_from_thing(tmptng);
                if (cctrl->lairtng_idx == thing->index) {
                    creature_remove_lair_totem_from_room(tmptng, room);
                } else {
                    ERRORLOG("Lair thing thinks it belongs to a creature, but the creature disagrees.");
                }
            } else
            {
                ERRORLOG("Lair thing %d has no owner!",(int)thing->index);
            }
        }
    }
}

void kill_room_slab_and_contents(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    // Get the room, and clear room index
    struct Room* room = room_get(slb->room_index);
    slb->room_index = 0;
    SlabCodedCoords slbnum = get_slab_number(slb_x, slb_y);
    for (MapSubtlCoord sstl_y = slab_subtile(slb_y, 0); sstl_y <= slab_subtile(slb_y, 2); sstl_y++)
    {
        for (MapSubtlCoord sstl_x = slab_subtile(slb_x, 0); sstl_x <= slab_subtile(slb_x, 2); sstl_x++)
        {
            kill_room_contents_at_subtile(room, plyr_idx, sstl_x, sstl_y, slbnum);
        }
    }
}

void free_room_structure(struct Room *room)
{
    PlayerNumber owner = room->owner;

    struct Dungeon *dungeon = get_dungeon(owner);

    if ( room->index == dungeon->room_list_start[room->kind] )
    {
        dungeon->room_list_start[room->kind] = room->next_of_owner;
        struct Room *next_room = room_get(room->next_of_owner);
        next_room->prev_of_owner = 0;
    }
    else
    {
        struct Room *next_room = room_get(room->next_of_owner);
        next_room->prev_of_owner = room->prev_of_owner;
        struct Room *prev_room = room_get(room->prev_of_owner);
        prev_room->next_of_owner = room->next_of_owner;
    }
    --dungeon->room_discrete_count[room->kind];

    remove_room_from_global_list(room);
    delete_room_structure(room);
}

void reset_creatures_rooms(struct Room *room)
{
    SYNCDBG(18,"Starting");
    if (room->owner == game.neutral_player_num) {
        return;
    }
    // Clear work room id for all creatures, so that they won't be stored as last_work_room_id
    long i = room->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (!creature_control_exists(cctrl)) {
            ERRORLOG("Jump to invalid creature %d detected",(int)i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        cctrl->work_room_id = -1;
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    // Switch the room for all creatures
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    k = 0;
    i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (cctrl->work_room_id == -1)
        {
            struct Room* nroom = get_room_thing_is_on(thing);
            if (room_is_invalid(nroom))
            {
                cctrl->creature_control_flags &= ~CCFlg_IsInRoomList;
                cctrl->work_room_id = 0;
                set_start_state(thing);
            } else
            {
                CreatureJob jobpref = get_job_for_creature_state(get_creature_state_besides_interruptions(thing));
                remove_creature_from_specific_room(thing, room, jobpref);
                add_creature_to_work_room(thing, nroom, jobpref);
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

void replace_room_slab(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char owner, unsigned char is_destroyed)
{
    if (room_role_matches(room->kind,RoRoF_PassWater|RoRoF_PassLava))
    {
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        switch (slabmap_wlb(slb))
        {
        case 1:
            place_slab_type_on_map(SlbT_LAVA,  slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 0);
            break;
        case 2:
            place_slab_type_on_map(SlbT_WATER, slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 0);
            break;
        default:
            ERRORLOG("WLB flags seem damaged for slab (%ld,%ld).",(long)slb_x,(long)slb_y);
            place_slab_type_on_map(SlbT_PATH,  slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 0);
            break;
        }
    } else
    {
        if ( is_destroyed )
        {
            place_slab_type_on_map(SlbT_PATH, slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 0);
        } else
        {
            place_slab_type_on_map(SlbT_CLAIMED, slab_subtile(slb_x,0), slab_subtile(slb_y,0), owner, 0);
            increase_dungeon_area(owner, 1);
        }
    }
}

struct Room *place_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    game.map_changed_for_nagivation = 1;
    if (subtile_coords_invalid(stl_x, stl_y))
        return INVALID_ROOM;
    long slb_x = subtile_slab(stl_x);
    long slb_y = subtile_slab(stl_y);
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    // If there already was a room, delete it; also, update area statistics
    if (slb->room_index > 0)
    {
        delete_room_slab(slb_x, slb_y, 0);
    } else
    {
        decrease_dungeon_area(owner, 1);
        increase_room_area(owner, 1);
    }
    // Create the new room; possibly merge adjacent rooms
    struct Room* room = create_room(owner, rkind, stl_x, stl_y);
    if (room_is_invalid(room))
        return INVALID_ROOM;
    // Make sure we have first subtile
    stl_x = stl_slab_starting_subtile(stl_x);
    stl_y = stl_slab_starting_subtile(stl_y);
    // Update slab type on map
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    SlabCodedCoords i = get_slab_number(slb_x, slb_y);
    delete_room_slabbed_objects(i);
    if ((rkind == RoK_GUARDPOST) || (rkind == RoK_BRIDGE)) //todo Make configurable
    {
        place_animating_slab_type_on_map(roomst->assigned_slab, 0, stl_x, stl_y, owner);
    } else
    {
        place_slab_type_on_map(roomst->assigned_slab, stl_x, stl_y, owner, 0);
    }
    SYNCDBG(7,"Updating efficiency");
    do_slab_efficiency_alteration(slb_x, slb_y);
    do_room_recalculation(room);
    if (owner != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_dungeon(owner);
        dungeon->lvstats.rooms_constructed++;
    }
    panel_map_update(stl_x, stl_y, STL_PER_SLB, STL_PER_SLB);
    return room;
}

struct Room *find_nearest_room_of_role_for_thing_with_spare_item_capacity(struct Thing *thing, PlayerNumber plyr_idx, RoomRole rrole, unsigned char nav_flags)
{
    long retdist = INT32_MAX;
    struct Room* retroom = INVALID_ROOM;

    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            long i = dungeon->room_list_start[rkind];
            unsigned long k = 0;
            while (i != 0)
            {
                struct Room* room = room_get(i);
                if (room_is_invalid(room))
                {
                    ERRORLOG("Jump to invalid room detected");
                    break;
                }
                i = room->next_of_owner;
                // Per-room code
                long dist = grid_distance(thing->mappos.x.stl.num, thing->mappos.y.stl.num, room->central_stl_x, room->central_stl_y);
                if ((dist < retdist) && (room->total_capacity > room->capacity_used_for_storage))
                {
                    struct Coord3d pos;
                    if (find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos))
                    {
                        if (!thing_is_creature(thing) || creature_can_navigate_to(thing, &pos, nav_flags))
                        {
                            retdist = dist;
                            retroom = room;
                        }
                    }
                }
                // Per-room code ends
                k++;
                if (k > ROOMS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping rooms list");
                    break;
                }
            }
        }
    }
    return retroom;
}

struct Room * pick_random_room_of_role(PlayerNumber plyr_idx, RoomRole rrole)
{
    struct Room *room = INVALID_ROOM;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);


    if ( !player_has_room_of_role(plyr_idx,rrole) )
        return INVALID_ROOM;

    int rand = PLAYER_RANDOM(plyr_idx, count_player_discrete_rooms_with_role(plyr_idx, rrole));

    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if (room_role_matches(rkind, rrole))
        {

            room = room_get(dungeon->room_list_start[rkind]);
            while ( !room_is_invalid(room) )
            {
                if ( rand == 0 )
                    return room;
                --rand;
                room = room_get(room->next_of_owner);
            }
        }
    }

    if ( room_is_invalid(room) )
    {
        ERRORLOG("Room not found");
        return INVALID_ROOM;
    }
    return INVALID_ROOM;
}

TbBool remove_item_from_room_capacity(struct Room *room)
{
    if ((room->used_capacity == 0) || (room->capacity_used_for_storage == 0))
    {
        ERRORLOG("Room %s index %d does not contain item to remove",room_code_name(room->kind),(int)room->index);
        return false;
    }
    room->used_capacity--;
    room->capacity_used_for_storage--;
    return true;
}

/**
 * Adds one item to room capacity. To be used for all rooms which store items, but not gold.
 */
TbBool add_item_to_room_capacity(struct Room *room, TbBool force)
{
    if (!force)
    {
        if (room->used_capacity >= room->total_capacity) {
            return false;
        }
    } else
    {
        if (room->capacity_used_for_storage >= room->total_capacity) {
            return false;
        }
    }
    room->used_capacity++;
    room->capacity_used_for_storage++;
    return true;
}

/**
 * Changes ownership of object thing in a room while it's being claimed.
 * @param room The room being claimed.
 * @param thing The thing to be checked, and possibly changed.
 * @param parent_idx The new thing parent. Parent for objects is a slab number. Not all objects have the parent set.
 * @param newowner
 */
static void change_ownership_or_delete_object_thing_in_room(struct Room *room, struct Thing *thing, long parent_idx, PlayerNumber newowner)
{
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    struct Dungeon* dungeon;
    // If thing is only dragged through the room, do not interrupt
    if (thing_is_dragged_or_pulled(thing)) {
        return;
    }
    // Handle specific things in rooms for which we have a special re-creation code
    PlayerNumber oldowner;

    // coloured objects must be recreated when owner changes, and it must be re-created
    ThingModel base_model = get_coloured_object_base_model(thing->model);
    if (base_model != 0)
    {
        create_coloured_object(&thing->mappos, newowner, parent_idx,base_model);
        delete_thing_structure(thing, 0);
        return;
    }
    else if (room_role_matches(room->kind,RoRoF_PowersStorage) && thing_is_spellbook(thing) )
    {
        // Library owns books, specials and candles; only spellbooks require additional code

        oldowner = thing->owner;
        thing->owner = newowner;
        if (oldowner != game.neutral_player_num) {
            remove_power_from_player(book_thing_to_power_kind(thing), oldowner);
        }
        if (newowner != game.neutral_player_num) {
            add_power_to_player(book_thing_to_power_kind(thing), newowner);
        }
        return;

    }
    else if (room_role_matches(room->kind,RoRoF_CratesStorage) && thing_is_workshop_crate(thing))
    {
        // Workshop owns trap boxes, machines and anvils; special code for boxes only
        ThingClass tngclass;
        ThingModel tngmodel;
        oldowner = thing->owner;
        thing->owner = newowner;
        tngclass = crate_thing_to_workshop_item_class(thing);
        tngmodel = crate_thing_to_workshop_item_model(thing);
        remove_workshop_item_from_amount_stored(oldowner, tngclass, tngmodel, WrkCrtF_NoOffmap);
        remove_workshop_item_from_amount_placeable(oldowner, tngclass, tngmodel);
        add_workshop_item_to_amounts(newowner, tngclass, tngmodel);
        return;
     }
     else if (room_role_matches(room->kind,RoRoF_GoldStorage) && object_is_gold_hoard(thing))
     {
         oldowner = thing->owner;
         {
             dungeon = get_dungeon(newowner);
             dungeon->total_money_owned += thing->valuable.gold_stored;
         }
         if (oldowner != game.neutral_player_num)
         {
             dungeon = get_dungeon(oldowner);
             dungeon->total_money_owned -= thing->valuable.gold_stored;
         }
         thing->owner = newowner;
         return;
    }
    else if (room_role_matches(room->kind,RoRoF_FoodStorage) && (object_is_infant_food(thing) || object_is_growing_food(thing) || object_is_mature_food(thing)))
    {
        thing->parent_idx = -1; // All chickens escape
    }
    else if (room_role_matches(room->kind,RoRoF_LairStorage) && thing_is_lair_totem(thing))
    {
        // Lair - owns creature lairs
        if (objst->related_creatr_model)
        {
            if (thing->lair.belongs_to)
            {
                struct Thing *tmptng;
                struct CreatureControl *cctrl;
                tmptng = thing_get(thing->lair.belongs_to);
                cctrl = creature_control_get_from_thing(tmptng);
                if (cctrl->lairtng_idx == thing->index) {
                    creature_remove_lair_totem_from_room(tmptng, room);
                } else {
                    ERRORLOG("Lair totem thinks it belongs to a creature, but the creature disagrees.");
                }
            } else
            {
                ERRORLOG("Lair totem %d has no owner!",(int)thing->index);
            }
            return;
        }
    }
    else if (room_role_matches(room->kind, RoRoF_KeeperStorage) && thing_is_dungeon_heart(thing))
    {
        dungeon = get_dungeon(newowner);
        if (dungeon->backup_heart_idx == 0)
        {
            dungeon->backup_heart_idx = thing->index;
        }
    }


    // If an object has parent slab, then it should change owner with that slab
    if (thing->parent_idx != -1)
    {
        if (parent_idx == thing->parent_idx) {
            thing->owner = newowner;
        }
        return;
    }
    // If the object is marked to be destroyed, do it
    if ((objst->model_flags & OMF_DestroyedOnRoomClaim) != 0) {
        destroy_object(thing);
        return;
    }
    if ((game.conf.rules[room->owner].game.classic_bugs_flags & ClscBug_ClaimRoomAllThings) != 0) {
        // Preserve classic bug - object is claimed with the room
        thing->owner = newowner;
        return;
    }
    // For some object types we've already executed code before; being here means they're on wrong room kind
    if (thing_is_workshop_crate(thing) || thing_is_gold_hoard(thing) || thing_is_spellbook(thing)) {
        return;
    }
    // Otherwise, changing owner depends on object properties
    if ((objst->model_flags & OMF_ChOwnedOnRoomClaim) != 0) {
        thing->owner = newowner;
        return;
    }
    // No need to change owner - leave the object intact
}

void delete_room_slabbed_objects(SlabCodedCoords slb_num)
{
    SYNCDBG(17,"Starting");

    MapSubtlCoord slb_x = slb_num_decode_x(slb_num);
    MapSubtlCoord slb_y = slb_num_decode_y(slb_num);
    for (int ssub_y = 0; ssub_y < STL_PER_SLB; ssub_y++)
    {
        for (int ssub_x = 0; ssub_x < STL_PER_SLB; ssub_x++)
        {
            MapSubtlCoord stl_x = slab_subtile(slb_x, ssub_x);
            MapSubtlCoord stl_y = slab_subtile(slb_y, ssub_y);
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
                // Per-thing code start
                if ((thing->parent_idx != slb_num) && (thing->class_id == TCls_Object))
                {
                    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
                    if ((objst->model_flags & OMF_DestroyedOnRoomPlace) != 0) {
                        destroy_object(thing);
                    }
                }
                // Per-thing code end
                k++;
                if (k > THINGS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping things list");
                    break_mapwho_infinite_chain(mapblk);
                    break;
                }
            }
        }
    }
}

/**
 * Changes ownership of things on a room subtile while it's being claimed.
 * @param room The room being claimed.
 * @param stl_x The subtile to be affected, X coord.
 * @param stl_y The subtile to be affected, Y coord.
 * @param plyr_idx The player which is claiming the room subtile.
 * @return True on success, false otherwise.
 */
static TbBool change_room_subtile_things_ownership(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    long parent_idx = get_slab_number(subtile_slab(stl_x), subtile_slab(stl_y));
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
        switch (thing->class_id)
        {
        case TCls_Object:
            change_ownership_or_delete_object_thing_in_room(room, thing, parent_idx, plyr_idx);
            break;
        case TCls_Trap:
            // Destroy any traps place on the room
            destroy_trap(thing);
            break;
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
    return true;
}

/**
 * Changes ownership of things in a room while it's being claimed.
 * Deletes things which shouldn't be in a room which is changing owner.
 * @param room The room to be affected.
 * @param plyr_idx The new owner.
 */
static void change_room_map_element_ownership(struct Room *room, PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    {
        struct SlabMap* slb = get_slabmap_direct(room->slabs_list);
        PlayerNumber owner = slabmap_owner(slb);
        if (owner != game.neutral_player_num) {
            decrease_room_area(owner, room->slabs_count);
        }
        increase_room_area(dungeon->owner, room->slabs_count);
    }
    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x = slb_num_decode_x(i);
        MapSlabCoord slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code
        set_slab_explored(plyr_idx, slb_x, slb_y);
        set_slab_owner(slb_x,slb_y, plyr_idx);
        MapSubtlCoord start_stl_x = slab_subtile(slb_x, 0);
        MapSubtlCoord start_stl_y = slab_subtile(slb_y, 0);
        MapSubtlCoord end_stl_x = slab_subtile(slb_x + 1, 0);
        MapSubtlCoord end_stl_y = slab_subtile(slb_y + 1, 0);
        // Do the loop
        for (MapSubtlCoord stl_y = start_stl_y; stl_y < end_stl_y; stl_y++)
        {
            for (MapSubtlCoord stl_x = start_stl_x; stl_x < end_stl_x; stl_x++)
            {
                change_room_subtile_things_ownership(room, stl_x, stl_y, plyr_idx);
            }
        }
        panel_map_update(start_stl_x, start_stl_y, STL_PER_SLB, STL_PER_SLB);
        // Per-slab code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
}

void redraw_slab_map_elements(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    // Prepare start and end subtiles
    MapSubtlCoord start_stl_x = slab_subtile(slb_x, 0);
    MapSubtlCoord start_stl_y = slab_subtile(slb_y, 0);
    MapSubtlCoord end_stl_x = slab_subtile(slb_x, 3);
    MapSubtlCoord end_stl_y = slab_subtile(slb_y, 3);
    // Do the loop
    for (MapSubtlCoord stl_y = start_stl_y; stl_y < end_stl_y; stl_y++)
    {
        for (MapSubtlCoord stl_x = start_stl_x; stl_x < end_stl_x; stl_x++)
        {
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if (!map_block_invalid(mapblk))
            {
                copy_block_with_cube_groups(-get_mapblk_column_index(mapblk), stl_x, stl_y);
            }
        }
    }
}

void redraw_room_map_elements(struct Room *room)
{
    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x = slb_num_decode_x(i);
        MapSlabCoord slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code start
        redraw_slab_map_elements(slb_x, slb_y);
        // Per-slab code end
        k++;
        if (k > game.map_tiles_x*game.map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
}

void do_room_unprettying(struct Room *room, PlayerNumber plyr_idx)
{
    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x = slb_num_decode_x(i);
        MapSlabCoord slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code start
        do_unprettying(plyr_idx, slb_x, slb_y);
        // Per-slab code end
        k++;
        if (k > game.map_tiles_x*game.map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
}

void do_room_integration(struct Room *room)
{
    SYNCDBG(7,"Starting for %s index %d owned by player %d",room_code_name(room->kind),(int)room->index,(int)room->owner);
    set_room_efficiency(room);
    update_room_total_health(room);
    update_room_total_capacity(room);
    update_room_contents(room);
    init_room_sparks(room);
}

void do_room_recalculation(struct Room* room)
{
    SYNCDBG(7, "Starting for %s index %d owned by player %d", room_code_name(room->kind), (int)room->index, (int)room->owner);
    set_room_efficiency(room);
    recalculate_room_health(room);
    update_room_total_capacity(room);
    update_room_contents(room);
    init_room_sparks(room);
}

void output_room_takeover_message(struct Room *room, PlayerNumber oldowner, PlayerNumber newowner)
{
    if (room->kind == RoK_ENTRANCE)
    {
        if (is_my_player_number(oldowner)) {
            output_message(SMsg_EntranceLost, 0);
        } else
        if (is_my_player_number(newowner))
        {
            output_message(SMsg_EntranceClaimed, 0);
        }
    } else
    if (is_my_player_number(newowner))
    {
        if (oldowner == game.neutral_player_num) {
            output_message(SMsg_NewRoomTakenOver, 0);
        } else {
            output_message(SMsg_EnemyRoomTakeOver, 0);
        }
    }
}

/**
 * Function used for claiming unowned room by a creature.
 * @param room The room to be claimed.
 * @param claimtng The creature which claimed it.
 * @return
 */
long claim_room(struct Room *room, struct Thing *claimtng)
{
    SYNCDBG(7,"Starting for %s index %d claimed by player %d",room_code_name(room->kind),(int)room->index,(int)claimtng->owner);
    PlayerNumber oldowner = room->owner;
    if ((oldowner != game.neutral_player_num) || (claimtng->owner == game.neutral_player_num))
    {
        return 0;
    }
    research_found_room(claimtng->owner, room->kind);
    room->owner = claimtng->owner;
    room->health = compute_room_max_health(room->slabs_count, room->efficiency);
    add_room_to_players_list(room, claimtng->owner);
    change_room_map_element_ownership(room, claimtng->owner);
    redraw_room_map_elements(room);
    do_room_unprettying(room, claimtng->owner);
    event_create_event(subtile_coord_center(room->central_stl_x), subtile_coord_center(room->central_stl_y),
        EvKind_RoomTakenOver, claimtng->owner, room->kind);
    do_room_integration(room);
    thing_play_sample(claimtng, 116, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    output_room_takeover_message(room, oldowner, claimtng->owner);
    return 1;
}

/**
 * Function used for claiming room owned by another keeper by a creature.
 * @param room The room to be claimed.
 * @param claimtng The creature which claimed it.
 * @return
 */
long claim_enemy_room(struct Room *room, struct Thing *claimtng)
{
    SYNCDBG(7,"Starting for %s index %d claimed by player %d",room_code_name(room->kind),(int)room->index,(int)claimtng->owner);
    PlayerNumber oldowner = room->owner;
    if ((oldowner == claimtng->owner) || (claimtng->owner == game.neutral_player_num))
    {
        return 0;
    }
    research_found_room(claimtng->owner, room->kind);
    reset_state_of_creatures_working_in_room(room);
    remove_room_from_players_list(room,oldowner);
    room->owner = claimtng->owner;
    room->health = compute_room_max_health(room->slabs_count, room->efficiency);
    add_room_to_players_list(room, claimtng->owner);
    change_room_map_element_ownership(room, claimtng->owner);
    redraw_room_map_elements(room);
    do_room_unprettying(room, claimtng->owner);
    event_create_event(subtile_coord_center(room->central_stl_x), subtile_coord_center(room->central_stl_y),
        EvKind_RoomTakenOver, claimtng->owner, room->kind);
    do_room_integration(room);
    output_room_takeover_message(room, oldowner, claimtng->owner);
    return 1;
}

/**
 * Function used for claiming room owned by another player by the level script.
 * @param room The room to be claimed.
 * @param newowner The player which will receive the room.
 * @return
 */
long take_over_room(struct Room* room, PlayerNumber newowner)
{
    SYNCDBG(7, "Starting for %s index %d claimed by player %d", room_code_name(room->kind), (int)room->index, newowner);
    PlayerNumber oldowner = room->owner;

    // Mark that player 'knows' about such room
    research_found_room(newowner, room->kind);

    if (oldowner != newowner)
    {
        reset_state_of_creatures_working_in_room(room);
        remove_room_from_players_list(room, oldowner);
        room->owner = newowner;
        room->health = compute_room_max_health(room->slabs_count, room->efficiency);
        add_room_to_players_list(room, newowner);
        change_room_map_element_ownership(room, newowner);
        redraw_room_map_elements(room);
        do_room_unprettying(room, newowner);
        do_room_integration(room);
        MapCoord ccor_x = subtile_coord_center(room->central_stl_x);
        MapCoord ccor_y = subtile_coord_center(room->central_stl_y);
        event_create_event_or_update_nearby_existing_event(ccor_x, ccor_y, EvKind_RoomLost, oldowner, room->kind);
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * Destroys all slabs of given room, creating gold rubble effect in the place.
 * @param room The room structure which slabs are to be destroyed.
 * @note The room structure is freed before this function end.
 */
void destroy_room_leaving_unclaimed_ground(struct Room *room, TbBool create_rubble)
{
    unsigned long k = 0;
    unsigned long count = room->slabs_count;
    SlabCodedCoords* slbs = malloc(count * sizeof(SlabCodedCoords));
    SlabCodedCoords i = room->slabs_list;
    while (i != 0)
    {
        slbs[k] = i;
        i = get_next_slab_number_in_room(i);
        k++;
    }
    for (k = 0; k < count; k++)
    {
        if (room->owner != game.neutral_player_num)
        {
            struct Dungeon* dungeon = get_players_num_dungeon(room->owner);
            dungeon->rooms_destroyed++;
        }
        MapSlabCoord slb_x = slb_num_decode_x(slbs[k]);
        MapSlabCoord slb_y = slb_num_decode_y(slbs[k]);
        if (create_rubble)
        {
            create_dirt_rubble_for_dug_slab(slb_x, slb_y);
        }
        delete_room_slab(slb_x, slb_y, 1); // Note that this function might also delete the whole room
    }
    free(slbs);
}

void destroy_dungeon_heart_room(PlayerNumber plyr_idx, const struct Thing *heartng)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    struct Room* room = get_room_thing_is_on(heartng);
    if (room_is_invalid(room) || (!room_role_matches(room->kind,RoRoF_KeeperStorage)))
    {
        WARNLOG("The heart thing is not in heart room");
        if (dungeon->backup_heart_idx == 0)
        {
            room = find_first_room_of_role(plyr_idx, RoRoF_KeeperStorage);
        }
    }
    if (room_is_invalid(room))
    {
        ERRORLOG("Tried to destroy heart for player who doesn't have one");
        return;
    }
    remove_room_from_players_list(room, plyr_idx);
    destroy_room_leaving_unclaimed_ground(room, true);
}
/******************************************************************************/
