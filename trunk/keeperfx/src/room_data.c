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
#include "room_data.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "config_creature.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_traps.h"
#include "thing_effects.h"
#include "room_jobs.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "magic.h"
#include "room_util.h"
#include "game_legacy.h"
#include "frontmenu_ingame_map.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_delete_room_structure(struct Room *room);
DLLIMPORT struct Room * _DK_find_random_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, signed char newowner, signed char rkind, unsigned char a4);
DLLIMPORT long _DK_claim_room(struct Room *room,struct Thing *claimtng);
DLLIMPORT long _DK_claim_enemy_room(struct Room *room,struct Thing *claimtng);
DLLIMPORT struct Room *_DK_get_room_thing_is_on(struct Thing *thing);
DLLIMPORT void _DK_do_unprettying(unsigned char a1, long plyr_idz, long a3);
DLLIMPORT void _DK_change_room_map_element_ownership(struct Room *room, unsigned char plyr_idz);
DLLIMPORT void _DK_copy_block_with_cube_groups(short a1, unsigned char plyr_idz, unsigned char a3);
/******************************************************************************/
void count_slabs(struct Room *room);
void count_gold_slabs_with_efficiency(struct Room *room);
void count_gold_hoardes_in_room(struct Room *room);
void count_slabs_div2(struct Room *room);
void count_books_in_room(struct Room *room);
void count_workers_in_room(struct Room *room);
void count_slabs_with_efficiency(struct Room *room);
void count_crates_in_room(struct Room *room);
void count_workers_in_room(struct Room *room);
void count_bodies_in_room(struct Room *room);
void count_capacity_in_garden(struct Room *room);
void count_food_in_room(struct Room *room);
void count_lair_occupants(struct Room *room);
/******************************************************************************/

RoomKind look_through_rooms[] = {
    RoK_DUNGHEART, RoK_TREASURE, RoK_LAIR,      RoK_GARDEN,
    RoK_LIBRARY,   RoK_TRAINING, RoK_WORKSHOP,  RoK_SCAVENGER,
    RoK_PRISON,    RoK_TEMPLE,   RoK_TORTURE,   RoK_GRAVEYARD,
    RoK_BARRACKS,  RoK_BRIDGE,   RoK_GUARDPOST, RoK_ENTRANCE,
    RoK_DUNGHEART, RoK_UNKN17,};

struct RoomData room_data[] = {
  { 0,  0, NULL,                    NULL,                   NULL,                  0, 0, 0, 201, 201},
  {14,  0, count_slabs,             NULL,                   NULL,                  0, 0, 0, 598, 614},
  {16, 57, count_gold_slabs_with_efficiency, count_gold_hoardes_in_room, NULL,     1, 0, 0, 599, 615},
  {18, 61, count_slabs_div2,        count_books_in_room,    count_workers_in_room, 0, 0, 0, 600, 616},
  {20, 65, count_slabs_with_efficiency, NULL,               NULL,                  1, 0, 0, 601, 617},
  {22, 63, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 602, 619},
  {24, 67, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 603, 618},
  {26,  0, NULL,                    NULL,                   NULL,                  0, 0, 0, 604, 620},
  {28, 75, count_slabs_div2,        count_crates_in_room,   count_workers_in_room, 0, 0, 0, 605, 621},
  {30, 77, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 613, 629},
  {32, 73, count_slabs_div2,        NULL,                   NULL,                  1, 0, 0, 612, 628},
  {34, 71, count_slabs_div2,        count_bodies_in_room,   NULL,                  0, 0, 0, 606, 622},
  {40, 69, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 607, 623},
  {36, 59, count_capacity_in_garden, count_food_in_room,    NULL,                  1, 0, 0, 608, 624},
  {38, 79, count_slabs_with_efficiency, count_lair_occupants, NULL,                1, 0, 0, 609, 625},
  {51, 81, NULL,                    NULL,                   NULL,                  0, 0, 0, 610, 626},
  {53, 83, count_slabs,             NULL,                   NULL,                  0, 0, 0, 611, 627},
};

struct RoomInfo room_info[] = {
  { 0,  0,  0},
  { 0,  0,  0},
  {29, 57,  0},
  {33, 61,  0},
  {37, 65,  0},
  {35, 63,  0},
  {39, 67, 85},
  { 0,  0,  0},
  {47, 75, 86},
  {49, 77,156},
  {45, 73,155},
  {43, 71, 45},
  {41, 69,  0},
  {31, 59,  0},
  {51, 79,  0},
  {53, 81,  0},
  {55, 83,  0},
};

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

struct Around const small_around[] = {
  { 0,-1},
  { 1, 0},
  { 0, 1},
  {-1, 0},
};

struct Around const my_around_eight[] = {
  { 0,-1},
  { 1,-1},
  { 1, 0},
  { 1, 1},
  { 0, 1},
  {-1, 1},
  {-1, 0},
  {-1,-1},
};

short const around_map[] = {-257, -256, -255, -1, 0, 1, 255, 256, 257};

unsigned char const slabs_to_centre_peices[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  2,  2,  2,  3,  4,  4,
  4,  5,  6,  6,  6,  7,  8,  9,  9,
  9, 10, 11, 12, 12, 12, 13, 14, 15,
 16, 16, 16, 17, 18, 19, 20, 20, 20,
 21, 22, 23, 24, 25,
};

/**
 * Should contain values encoded with get_subtile_number(). */
const unsigned short small_around_pos[] = {
  0xFF00, 0x0001, 0x0100, 0xFFFF,
};

struct Around const mid_around[] = {
  { 0,  0},
  { 0, -1},
  { 1,  0},
  { 0,  1},
  {-1,  0},
  {-1, -1},
  { 1, -1},
  {-1,  1},
  { 1,  1},
};

unsigned short const room_effect_elements[] = { 55, 56, 57, 58, 0, 0 };
const short slab_around[] = { -85, 1, 85, -1 };
/******************************************************************************/
DLLIMPORT unsigned char _DK_find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT void _DK_count_gold_slabs_with_efficiency(struct Room *room);
DLLIMPORT void _DK_count_gold_hoardes_in_room(struct Room *room);
DLLIMPORT void _DK_count_books_in_room(struct Room *room);
DLLIMPORT void _DK_count_workers_in_room(struct Room *room);
DLLIMPORT void _DK_count_crates_in_room(struct Room *room);
DLLIMPORT void _DK_count_workers_in_room(struct Room *room);
DLLIMPORT void _DK_count_bodies_in_room(struct Room *room);
DLLIMPORT void _DK_count_food_in_room(struct Room *room);
DLLIMPORT void _DK_count_lair_occupants(struct Room *room);
DLLIMPORT short _DK_room_grow_food(struct Room *room);
DLLIMPORT void _DK_set_room_capacity(struct Room *room, long capac);
DLLIMPORT void _DK_set_room_efficiency(struct Room *room);
DLLIMPORT struct Room *_DK_link_adjacent_rooms_of_type(unsigned char owner, long stl_x, long stl_y, unsigned char rkind);
DLLIMPORT struct Room *_DK_create_room(unsigned char a1, unsigned char plyr_idz, unsigned short a3, unsigned short a4);
DLLIMPORT void _DK_create_room_flag(struct Room *room);
DLLIMPORT struct Room *_DK_allocate_free_room_structure(void);
DLLIMPORT unsigned short _DK_i_can_allocate_free_room_structure(void);
DLLIMPORT struct Room *_DK_find_room_with_spare_room_item_capacity(unsigned char a1, signed char plyr_idz);
DLLIMPORT long _DK_create_workshop_object_in_workshop_room(long a1, long plyr_idz, long a3);
DLLIMPORT unsigned char _DK_find_first_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT struct Room* _DK_find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing,
    signed char plyr_idz, signed char a3, unsigned char a4, long a5);
DLLIMPORT struct Room* _DK_find_room_with_spare_capacity(unsigned char a1, signed char plyr_idz, long a3);
DLLIMPORT short _DK_delete_room_slab_when_no_free_room_structures(long a1, long plyr_idz, unsigned char a3);
DLLIMPORT long _DK_calculate_room_efficiency(struct Room *room);
DLLIMPORT void _DK_kill_room_slab_and_contents(unsigned char a1, unsigned char plyr_idz, unsigned char a3);
DLLIMPORT void _DK_free_room_structure(struct Room *room);
DLLIMPORT void _DK_reset_creatures_rooms(struct Room *room);
DLLIMPORT void _DK_replace_room_slab(struct Room *room, long plyr_idz, long a3, unsigned char a4, unsigned char a5);
DLLIMPORT struct Room *_DK_place_room(unsigned char a1, unsigned char plyr_idz, unsigned short a3, unsigned short a4);
DLLIMPORT struct Room *_DK_find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, char plyr_idz, char a3, unsigned char a4);
DLLIMPORT struct Room * _DK_pick_random_room(PlayerNumber newowner, int kind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Room *room_get(long room_idx)
{
  if ((room_idx < 1) || (room_idx > ROOMS_COUNT))
    return &game.rooms[0];
  return &game.rooms[room_idx];
}

struct Room *subtile_room_get(long stl_x, long stl_y)
{
  struct SlabMap *slb;
  slb = get_slabmap_for_subtile(stl_x,stl_y);
  if (slabmap_block_invalid(slb))
    return INVALID_ROOM;
  return room_get(slb->room_index);
}

struct Room *slab_room_get(long slb_x, long slb_y)
{
  struct SlabMap *slb;
  slb = get_slabmap_block(slb_x,slb_y);
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
  return ((room->field_0 & 0x01) != 0);
}

struct RoomData *room_data_get_for_kind(RoomKind rkind)
{
  if ((rkind < 1) || (rkind > ROOM_TYPES_COUNT))
    return &room_data[0];
  return &room_data[rkind];
}

struct RoomData *room_data_get_for_room(const struct Room *room)
{
  if ((room->kind < 1) || (room->kind > ROOM_TYPES_COUNT))
    return &room_data[0];
  return &room_data[room->kind];
}

struct RoomStats *room_stats_get_for_kind(RoomKind rkind)
{
    if ((rkind < 1) || (rkind > ROOM_TYPES_COUNT))
        return &game.room_stats[0];
    return &game.room_stats[rkind];
}

struct RoomStats *room_stats_get_for_room(const struct Room *room)
{
    if ((room->kind < 1) || (room->kind > ROOM_TYPES_COUNT))
        return &game.room_stats[0];
    return &game.room_stats[room->kind];
}

long get_room_look_through(RoomKind rkind)
{
  const long arr_length = sizeof(look_through_rooms)/sizeof(look_through_rooms[0]);
  long i;
  for (i=0; i < arr_length; i++)
  {
    if (look_through_rooms[i] == rkind)
      return i;
  }
  return -1;
}

long get_room_slabs_count(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    long i;
    long count;
    dungeon = get_players_num_dungeon(plyr_idx);
    count = 0;
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
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

long count_slabs_of_room_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long nslabs;
    long i;
    unsigned long k;
    nslabs = 0;
    dungeon = get_dungeon(plyr_idx);
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
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
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return nslabs;
}

long get_room_kind_used_capacity_fraction(PlayerNumber plyr_idx, RoomKind room_kind)
{
    struct Dungeon * dungeon;
    struct Room * room;
    int used_capacity;
    int total_capacity;
    long i;
    unsigned long k;
    dungeon = get_dungeon(plyr_idx);
    total_capacity = 0;
    used_capacity = 0;
    i = dungeon->room_kind[room_kind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
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
    if (total_capacity <= 0) {
        return 0;
    }
    return (used_capacity * 256) / total_capacity;
}

long get_player_rooms_count(PlayerNumber plyr_idx, RoomKind rkind)
{
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned long k;
  long i;
  // note that we can't get_players_num_dungeon() because players
  // may be uninitialized yet when this is called.
  dungeon = get_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
      return 0;
  i = dungeon->room_kind[rkind];
  k = 0;
  while (i != 0)
  {
    room = room_get(i);
    if (room_is_invalid(room))
    {
      ERRORLOG("Jump to invalid room detected");
      break;
    }
    i = room->next_of_owner;
    k++;
    if (k > ROOMS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping rooms list");
      break;
    }
  }
  return k;
}

void set_room_capacity(struct Room *room, long capac)
{
    struct RoomData *rdata;
    SYNCDBG(7,"Starting");
    //_DK_set_room_capacity(room, capac); return;
    rdata = room_data_get_for_room(room);
    if ((capac == 0) || (rdata->field_F))
    {
        do_room_integration(room);
    }
}

void set_room_efficiency(struct Room *room)
{
    //_DK_set_room_efficiency(room);
    room->efficiency = calculate_room_efficiency(room);
}

void count_slabs(struct Room *room)
{
  room->total_capacity = room->slabs_count;
}

void count_gold_slabs_with_efficiency(struct Room *room)
{
  _DK_count_gold_slabs_with_efficiency(room);
}

void count_gold_hoardes_in_room(struct Room *room)
{
  _DK_count_gold_hoardes_in_room(room);
}

void count_slabs_div2(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = ((count/256) >> 1);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;
}

void count_books_in_room(struct Room *room)
{
  _DK_count_books_in_room(room);
}

void count_workers_in_room(struct Room *room)
{
  _DK_count_workers_in_room(room);
}

void count_slabs_with_efficiency(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = (count/256);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;
}

void count_crates_in_room(struct Room *room)
{
  _DK_count_crates_in_room(room);
}

void count_bodies_in_room(struct Room *room)
{
  _DK_count_bodies_in_room(room);
}

void count_capacity_in_garden(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = (count/256);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;
}

void count_food_in_room(struct Room *room)
{
  _DK_count_food_in_room(room);
}

void count_lair_occupants(struct Room *room)
{
  _DK_count_lair_occupants(room);
}


void delete_room_structure(struct Room *room)
{
    struct Dungeon *dungeon;
    struct Room *secroom;
    unsigned short *wptr;
    //_DK_delete_room_structure(room); return;
    if (room_is_invalid(room))
    {
        WARNLOG("Attempt to delete invalid room");
        return;
    }
    if ((room->field_0 & 0x01) != 0)
    {
      if (room->owner != game.neutral_player_num)
      {
          dungeon = get_players_num_dungeon(room->owner);
          wptr = &dungeon->room_kind[room->kind];
          if (room->index == *wptr)
          {
              *wptr = room->next_of_owner;
              secroom = room_get(room->next_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->prev_of_owner = 0;
          }
          else
          {
              secroom = room_get(room->next_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->prev_of_owner = room->prev_of_owner;
              secroom = room_get(room->prev_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->next_of_owner = room->next_of_owner;
          }
      }
      LbMemorySet(room, 0, sizeof(struct Room));
    }
}

void delete_all_room_structures(void)
{
    struct Room *room;
    long i;
    for (i=1; i < ROOMS_COUNT; i++)
    {
        room = &game.rooms[i];
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
    unsigned long k;
    k = 0;
    while (wrkroom->creatures_list != 0)
    {
        struct Thing *thing;
        thing = thing_get(wrkroom->creatures_list);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid creature %d detected",(int)wrkroom->creatures_list);
            break;
        }
        // Per creature code
        remove_creature_from_specific_room(thing, wrkroom);
        add_creature_to_work_room(thing, newroom);
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
    unsigned long k;
    k = 0;
    while (wrkroom->creatures_list != 0)
    {
        struct Thing *thing;
        thing = thing_get(wrkroom->creatures_list);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid creature %d detected",(int)wrkroom->creatures_list);
            break;
        }
        // Per creature code
        set_start_state(thing);
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
}

void update_room_total_capacity(struct Room *room)
{
    struct RoomData *rdata;
    rdata = room_data_get_for_room(room);
    if (rdata->update_total_capacity != NULL)
    {
        rdata->update_total_capacity(room);
    }
}

/**
 * Counts slabs making up the room and stores them in the room.
 * Also, updates room index in all the slabs.
 * @param room
 */
void count_room_slabs(struct Room *room)
{
    struct SlabMap *slb;
    unsigned long k;
    long i;
    long n;
    n = 0;
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
        n++;
        slb->room_index = room->index;
        // Per room tile code ends
        k++;
        if (k >= map_tiles_x*map_tiles_y)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    room->slabs_count = n;
}

/**
 * Checks if a room slab of given kind at given subtile could link to any of rooms at adjacent slabs.
 * @param owner The owning player index of the new room slab.
 * @param x The x subtile of the new room slab.
 * @param y The y subtile of the new room slab.
 * @param rkind  Kind of the new room slab.
 * @return
 */
struct Room *link_adjacent_rooms_of_type(PlayerNumber owner, MapSubtlCoord x, MapSubtlCoord y, RoomKind rkind)
{
    SlabCodedCoords central_slbnum;
    struct SlabMap *slb;
    struct Room *linkroom;
    struct Room *room;
    MapSubtlCoord stl_x,stl_y;
    long n;
    // Encoded slab coords - we will need it if we'll find adjacent room
    central_slbnum = get_slab_number(subtile_slab_fast(x),subtile_slab_fast(y));
    //return _DK_link_adjacent_rooms_of_type(owner, x, y, rkind);
    // Localize the room to be merged with other rooms
    linkroom = INVALID_ROOM;
    for (n = 0; n < 4; n++)
    {
        stl_x = x + 3 * (long)small_around[n].delta_x;
        stl_y = y + 3 * (long)small_around[n].delta_y;
        room = subtile_room_get(stl_x,stl_y);
        if ( !room_is_invalid(room) )
        {
          if ( (room->owner == owner) && (room->kind == rkind) )
          {
              // Add the central slab to room which was found
              room->total_capacity = 0;
              slb = get_slabmap_direct(room->slabs_list_tail);
              slb->next_in_room = central_slbnum;
              slb = get_slabmap_direct(central_slbnum);
              slb->next_in_room = 0;
              room->slabs_list_tail = central_slbnum;
              linkroom = room;
              break;
          }
        }
    }
    if ( room_is_invalid(linkroom) )
    {
        return INVALID_ROOM;
    }
    // If slab was added to the room, check if more rooms now have to be linked together
    for (n++; n < 4; n++)
    {
        stl_x = x + 3 * (long)small_around[n].delta_x;
        stl_y = y + 3 * (long)small_around[n].delta_y;
        room = subtile_room_get(stl_x,stl_y);
        if ( !room_is_invalid(room) )
        {
          if ( (room->owner == owner) && (room->kind == rkind) )
          {
              if (room != linkroom)
              {
                  // We have a room to merge
                  slb = get_slabmap_direct(linkroom->slabs_list_tail);
                  // Link together lists of slabs
                  slb->next_in_room = room->slabs_list;
                  linkroom->slabs_list_tail = room->slabs_list_tail;
                  // Update slabs in the new list
                  count_room_slabs(linkroom);
                  update_room_total_capacity(linkroom);
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

/** Returns coordinates of slab at mass centre of given room.
 *  Note that the slab returned may not be pat of the room - it is possible
 *   that the room is just surrounding the spot.
 * @param mass_x
 * @param mass_y
 * @param room
 */
void get_room_mass_centre_coords(long *mass_x, long *mass_y, const struct Room *room)
{
    struct SlabMap *slb;
    unsigned long tot_x,tot_y;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    tot_x = 0;
    tot_y = 0;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        slb = get_slabmap_block(slb_x,slb_y);
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
        *mass_x = map_tiles_x / 2;
        *mass_y = map_tiles_y / 2;
    }
}


void update_room_central_tile_position(struct Room *room)
{
    struct MapOffset *sstep;
    struct SlabMap *slb;
    long mass_x,mass_y;
    long cx,cy;
    long i;
    get_room_mass_centre_coords(&mass_x, &mass_y, room);
    for (i=0; i < 256; i++)
    {
        sstep = &spiral_step[i];
        cx = 3 * (mass_x + (long)sstep->h) + 1;
        cy = 3 * (mass_y + (long)sstep->v) + 1;
        slb = get_slabmap_for_subtile(cx,cy);
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
    ERRORLOG("Cannot find position in %s index %d to place an ensign.",room_code_name(room->kind),(int)room->index);
}

void add_room_to_global_list(struct Room *room)
{
    struct Room *nxroom;
    // There is only one global list of rooms - the list of entrances
    if (room->kind == RoK_ENTRANCE)
    {
      if ((game.entrance_room_id > 0) && (game.entrance_room_id < ROOMS_COUNT))
      {
        room->next_of_kind = game.entrance_room_id;
        nxroom = room_get(game.entrance_room_id);
        nxroom->prev_of_kind = room->index;
      }
      game.entrance_room_id = room->index;
      game.entrances_count++;
    }
}

TbBool add_room_to_players_list(struct Room *room, long plyr_idx)
{
    struct Dungeon *dungeon;
    struct Room *nxroom;
    long nxroom_id;
    if (plyr_idx == game.neutral_player_num)
        return false;
    if (room->kind >= ROOM_TYPES_COUNT)
    {
        ERRORLOG("Room no %d has invalid kind",(int)room->index);
        return false;
    }
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    dungeon = get_dungeon(plyr_idx);
    nxroom_id = dungeon->room_kind[room->kind];
    nxroom = room_get(nxroom_id);
    if (room_is_invalid(nxroom))
    {
        room->next_of_owner = 0;
    } else
    {
        room->next_of_owner = nxroom_id;
        nxroom->prev_of_owner = room->index;
    }
    dungeon->room_kind[room->kind] = room->index;
    dungeon->room_slabs_count[room->kind]++;
    return true;
}

TbBool remove_room_from_players_list(struct Room *room, long plyr_idx)
{
    struct Dungeon *dungeon;
    struct Room *nxroom;
    struct Room *pvroom;
    if (plyr_idx == game.neutral_player_num)
        return false;
    if (room->kind >= ROOM_TYPES_COUNT)
    {
        ERRORLOG("Room no %d has invalid kind",(int)room->index);
        return false;
    }
    dungeon = get_dungeon(plyr_idx);
    pvroom = room_get(room->prev_of_owner);
    nxroom = room_get(room->next_of_owner);
    if (!room_is_invalid(pvroom)) {
        pvroom->next_of_owner = room->next_of_owner;
    } else {
        dungeon->room_kind[room->kind] = room->next_of_owner;
    }
    if (!room_is_invalid(nxroom)) {
        nxroom->prev_of_owner = room->prev_of_owner;
    }
    room->next_of_owner = 0;
    room->prev_of_owner = 0;
    dungeon->room_slabs_count[room->kind]--;
    return true;
}

struct Room *prepare_new_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct SlabMap *slb;
    struct Room *room;
    MapSlabCoord slb_x,slb_y;
    long i;
    if (!subtile_has_slab(stl_x, stl_y))
    {
        ERRORLOG("Attempt to create room on invalid coordinates.");
        return NULL;
    }
    if ( !i_can_allocate_free_room_structure() )
    {
        ERRORDBG(2,"Cannot allocate any more rooms.");
        erstat_inc(ESE_NoFreeRooms);
        return NULL;
    }
    room = allocate_free_room_structure();
    room->owner = owner;
    room->kind = rkind;
    add_room_to_global_list(room);
    add_room_to_players_list(room, owner);
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    i = get_slab_number(slb_x, slb_y);
    room->slabs_list = i;
    room->slabs_list_tail = i;
    slb = get_slabmap_direct(i);
    slb->next_in_room = 0;
    return room;
}

struct Room *create_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Room *room;
    SYNCDBG(7,"Starting to make %s at (%d,%d)",room_code_name(rkind),(int)stl_x,(int)stl_y);
    //room = _DK_create_room(owner, rkind, stl_x, stl_y); return room;
    // Try linking the new room slab to existing room
    room = link_adjacent_rooms_of_type(owner, stl_x, stl_y, rkind);
    if (room_is_invalid(room))
    {
        room = prepare_new_room(owner, rkind, stl_x, stl_y);
        if (room_is_invalid(room))
            return INVALID_ROOM;
        count_room_slabs(room);
        update_room_central_tile_position(room);
        create_room_flag(room);
    } else
    {
        count_room_slabs(room);
        update_room_central_tile_position(room);
    }
    SYNCDBG(7,"Done");
    return room;
}

TbBool room_can_have_flag(const struct Room *room)
{
    return ( (room->kind != RoK_DUNGHEART) && (room->kind != RoK_ENTRANCE)
      && (room->kind != RoK_GUARDPOST) && (room->kind != RoK_BRIDGE) );
}

void create_room_flag(struct Room *room)
{
    struct Thing *thing;
    struct Coord3d pos;
    MapSubtlCoord stl_x,stl_y;
    //_DK_create_room_flag(room);
    stl_x = slab_subtile_center(slb_num_decode_x(room->slabs_list));
    stl_y = slab_subtile_center(slb_num_decode_y(room->slabs_list));
    SYNCDBG(7,"Starting for %s at (%ld,%ld)",room_code_name(room->kind),stl_x,stl_y);
    if (room_can_have_flag(room))
    {
        pos.z.val = 2 << 8;
        pos.x.val = stl_x << 8;
        pos.y.val = stl_y << 8;
        thing = find_base_thing_on_mapwho(TCls_Object, 25, stl_x, stl_y);
        if (thing_is_invalid(thing))
        {
            thing = create_object(&pos, 25, room->owner, -1);
        }
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Cannot create room flag");
            return;
        }
        thing->word_13 = room->index;
    }
}

void delete_room_flag(struct Room *room)
{
    struct Thing *thing;
    MapSubtlCoord stl_x,stl_y;
    stl_x = slab_subtile_center(slb_num_decode_x(room->slabs_list));
    stl_y = slab_subtile_center(slb_num_decode_y(room->slabs_list));
    if (room_can_have_flag(room))
    {
        thing = find_base_thing_on_mapwho(TCls_Object, 25, stl_x, stl_y);
        if (!thing_is_invalid(thing)) {
            delete_thing_structure(thing, 0);
        }
    }
}

struct Room *allocate_free_room_structure(void)
{
  return _DK_allocate_free_room_structure();
}

unsigned short i_can_allocate_free_room_structure(void)
{
  unsigned short ret = _DK_i_can_allocate_free_room_structure();
  if (ret == 0)
      SYNCDBG(3,"No slot for next room");
  return ret;
}

RoomKind slab_to_room_type(SlabKind slbkind)
{
  switch (slbkind)
  {
  case SlbT_ENTRANCE:
      return RoK_ENTRANCE;
  case SlbT_TREASURE:
      return RoK_TREASURE;
  case SlbT_LIBRARY:
      return RoK_LIBRARY;
  case SlbT_PRISON:
      return RoK_PRISON;
  case SlbT_TORTURE:
      return RoK_TORTURE;
  case SlbT_TRAINING:
      return RoK_TRAINING;
  case SlbT_DUNGHEART:
      return RoK_DUNGHEART;
  case SlbT_WORKSHOP:
      return RoK_WORKSHOP;
  case SlbT_SCAVENGER:
      return RoK_SCAVENGER;
  case SlbT_TEMPLE:
      return RoK_TEMPLE;
  case SlbT_GRAVEYARD:
      return RoK_GRAVEYARD;
  case SlbT_GARDEN:
      return RoK_GARDEN;
  case SlbT_LAIR:
      return RoK_LAIR;
  case SlbT_BARRACKS:
      return RoK_BARRACKS;
  case SlbT_BRIDGE:
      return RoK_BRIDGE;
  case SlbT_GUARDPOST:
      return RoK_GUARDPOST;
  default:
      return RoK_NONE;
  }
}

void reinitialise_treaure_rooms(void)
{
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned int i,k,n;
  for (n=0; n < DUNGEONS_COUNT; n++)
  {
    dungeon = get_dungeon(n);
    i = dungeon->room_kind[RoK_TREASURE];
    k = 0;
    while (i != 0)
    {
      room = room_get(i);
      if (room_is_invalid(room))
      {
        ERRORLOG("Jump to invalid room detected");
        break;
      }
      i = room->next_of_owner;
      set_room_capacity(room, 1);
      k++;
      if (k > ROOMS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping rooms list");
        break;
      }
    }
  }
}

TbBool initialise_map_rooms(void)
{
    unsigned long slb_x,slb_y;
    SYNCDBG(7,"Starting");
    for (slb_y=0; slb_y < map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < map_tiles_x; slb_x++)
        {
            struct SlabMap *slb;
            struct Room *room;
            RoomKind rkind;
            slb = get_slabmap_block(slb_x, slb_y);
            rkind = slab_to_room_type(slb->kind);
            if (rkind > 0)
              room = create_room(slabmap_owner(slb), rkind, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
            else
              room = INVALID_ROOM;
            if (!room_is_invalid(room))
            {
              set_room_efficiency(room);
              set_room_capacity(room, 0);
            }
        }
    }
    return true;
}

short room_grow_food(struct Room *room)
{
  return _DK_room_grow_food(room);
}

long calculate_room_widespread_factor(const struct Room *room)
{
    long nslabs,npieces;
    long i;
    nslabs = room->slabs_count;
    i = nslabs;
    if (i >= sizeof(slabs_to_centre_peices)/sizeof(slabs_to_centre_peices[0]))
        i = sizeof(slabs_to_centre_peices)/sizeof(slabs_to_centre_peices[0]) - 1;
    npieces = slabs_to_centre_peices[i];
    return 2 * (npieces + 4 * nslabs);
}

/** Calculates summary of efficiency score from all slabs in room.
 *
 * @param room Source room.
 * @return The efficiency score summary.
 */
long calculate_cummulative_room_slabs_effeciency(const struct Room *room)
{
    long score;
    long i;
    unsigned long k;
    score = 0;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        // Per room tile code
        score += calculate_effeciency_score_for_room_slab(i, room->owner);
        // Per room tile code ends
        i = get_next_slab_number_in_room(i); // It would be better to have this before per-tile block, but we need old value
        k++;
        if (k > room->slabs_count)
        {
          ERRORLOG("Room slabs list length exceeded when sweeping");
          break;
        }
    }
    return score;
}

long calculate_room_efficiency(const struct Room *room)
{
    long nslabs,score,widespread,effic;
    long expected_base;
    //return _DK_calculate_room_efficiency(room);
    nslabs = room->slabs_count;
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
    widespread = calculate_room_widespread_factor(room);
    score = calculate_cummulative_room_slabs_effeciency(room);
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
    return effic;
}

void update_room_efficiency(struct Room *room)
{
    room->efficiency = calculate_room_efficiency(room);
}

TbBool update_room_contents(struct Room *room)
{
  struct RoomData *rdata;
  rdata = room_data_get_for_room(room);
  if (rdata->ofsfield_7 != NULL)
  {
    rdata->ofsfield_7(room);
  }
  if (rdata->offfield_B != NULL)
  {
    rdata->offfield_B(room);
  }
  return true;
}

TbBool thing_is_on_own_room_tile(const struct Thing *thing)
{
    return subtile_is_player_room(thing->owner, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

struct Room *get_room_thing_is_on(const struct Thing *thing)
{
    //return _DK_get_room_thing_is_on(thing);
    return subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

void init_room_sparks(struct Room *room)
{
    struct SlabMap *slb;
    struct SlabMap *sibslb;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    if (room->kind == RoK_DUNGHEART) {
        return;
    }
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        // Per room tile code
        slb = get_slabmap_block(slb_x, slb_y);
        sibslb = get_slabmap_block(slb_x, slb_y-1);
        if (sibslb->room_index != slb->room_index)
        {
            room->field_43 = 1;
            room->field_44 = 0;
            room->field_41 = i;
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
    long slb_x,slb_y;
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
        struct Coord3d pos;
        long effect_kind;
        pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
        pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
        pos.z.val = subtile_coord_center(1);
        effect_kind = effkind;
        if (effrange > 0)
            effect_kind += ACTION_RANDOM(effrange);
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
    long slb_x,slb_y;
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

TbBool room_has_enough_free_capacity_for_creature(const struct Room *room, const struct Thing *creatng)
{
    if (room->kind == RoK_LAIR)
    {
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(creatng);
        if (room->used_capacity + crstat->lair_size <= room->total_capacity)
            return true;
    } else
    {
        if (room->used_capacity + 1 <= room->total_capacity)
            return true;
    }
    return false;
}

TbBool find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_find_random_valid_position_for_thing_in_room(thing, room, pos);
}

struct Room *find_room_with_spare_room_item_capacity(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    //return _DK_find_room_with_spare_room_item_capacity(a1, a2);
    if ((rkind < 0) || (rkind >= ROOM_TYPES_COUNT))
        return NULL;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return NULL;
    k = 0;
    i = dungeon->room_kind[rkind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->capacity_used_for_storage < room->total_capacity) {
            return room;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return NULL;
}

/**
 * Searches for room of given kind and owner which has no less than given spare capacity.
 * @param owner
 * @param rkind
 * @param spare
 * @return
 * @note Function find_room_with_spare_room_capacity() should also redirect to this one.
 */
struct Room *find_room_with_spare_capacity(unsigned char owner, signed char rkind, long spare)
{
    struct Dungeon *dungeon;
    if ((rkind < 0) || (rkind >= ROOM_TYPES_COUNT))
        return NULL;
    dungeon = get_dungeon(owner);
    if (dungeon_invalid(dungeon))
        return NULL;
    return find_room_with_spare_capacity_starting_with(dungeon->room_kind[rkind], spare);
}

struct Room *find_room_with_spare_capacity_starting_with(long room_idx, long spare)
{
    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    k = 0;
    i = room_idx;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->used_capacity + spare <= room->total_capacity)
        {
            return room;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return NULL;
}

struct Room *find_room_with_most_spare_capacity_starting_with(long room_idx,long *total_spare_cap)
{
    long max_spare_cap,loc_total_spare_cap,delta;
    struct Room *max_spare_room;
    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    loc_total_spare_cap = 0;
    max_spare_room = INVALID_ROOM;
    max_spare_cap = 0;
    k = 0;
    i = room_idx;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->total_capacity > room->used_capacity)
        {
          delta = room->total_capacity - room->used_capacity;
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
    if (total_spare_cap != NULL)
       (*total_spare_cap) = loc_total_spare_cap;
    return max_spare_room;
}

TbBool find_first_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_find_first_valid_position_for_thing_in_room(thing, room, pos);
}

struct Room *find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing, signed char owner, signed char rkind, unsigned char nav_no_owner, long spare)
{
    struct Dungeon *dungeon;
    struct Room *nearoom;
    long neardistance,distance;
    struct Coord3d pos;

    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Searching for %s with capacity for %s index %d",room_code_name(rkind),thing_model_name(thing),(int)thing->index);
    dungeon = get_dungeon(owner);
    nearoom = INVALID_ROOM;
    neardistance = LONG_MAX;
    k = 0;
    i = dungeon->room_kind[rkind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        // Compute simplified distance - without use of mul or div
        distance = abs(thing->mappos.x.stl.num - room->central_stl_x)
                 + abs(thing->mappos.y.stl.num - room->central_stl_y);
        if ((neardistance > distance) && (room->used_capacity + spare <= room->total_capacity))
        {
            if (find_first_valid_position_for_thing_in_room(thing, room, &pos))
            {
                if ((thing->class_id != TCls_Creature)
                  || creature_can_navigate_to(thing, &pos, nav_no_owner))
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
    return nearoom;
}

/**
 * Counts all room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_no_owner
 * @return
 */
long count_rooms_creature_can_navigate_to(struct Thing *thing, unsigned char owner, signed char kind, unsigned char nav_no_owner)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    struct Coord3d pos;
    long count;
    SYNCDBG(18,"Starting");
    dungeon = get_dungeon(owner);
    count = 0;
    k = 0;
    i = dungeon->room_kind[kind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        pos.x.val = get_subtile_center_pos(room->central_stl_x);
        pos.y.val = get_subtile_center_pos(room->central_stl_y);
        pos.z.val = 256;
        if ((room->used_capacity > 0) && creature_can_navigate_to(thing, &pos, nav_no_owner))
        {
            count++;
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
 * Gives a room of given kind and owner where the creature can navigate to.
 * Counts all possible rooms, then selects one and returns it.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_no_owner
 * @return
 */
struct Room *find_random_room_creature_can_navigate_to(struct Thing *thing, unsigned char owner, signed char kind, unsigned char nav_no_owner)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    struct Coord3d pos;
    long count,selected;
    SYNCDBG(18,"Starting");
    count = count_rooms_creature_can_navigate_to(thing, owner, kind, nav_no_owner);
    if (count < 1)
        return NULL;
    dungeon = get_dungeon(owner);
    selected = ACTION_RANDOM(count);
    k = 0;
    i = dungeon->room_kind[kind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        pos.x.val = get_subtile_center_pos(room->central_stl_x);
        pos.y.val = get_subtile_center_pos(room->central_stl_y);
        pos.z.val = 256;
        if ((room->used_capacity > 0) && creature_can_navigate_to(thing, &pos, nav_no_owner))
        {
            if (selected > 0)
            {
                selected--;
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
    return NULL;
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
struct Room *find_room_nearest_to_position(PlayerNumber plyr_idx, RoomKind rkind, const struct Coord3d *pos, long *room_distance)
{
    struct Dungeon *dungeon;
    struct Room *room;
    struct Room *near_room;
    long i;
    unsigned long k;
    long distance,near_distance;
    long delta_x,delta_y;
    dungeon = get_dungeon(plyr_idx);
    near_distance = LONG_MAX;
    near_room = INVALID_ROOM;
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        delta_x = (room->central_stl_x << 8) - (long)pos->x.val;
        delta_y = (room->central_stl_y << 8) - (long)pos->y.val;
        distance = LbDiagonalLength(abs(delta_x), abs(delta_y));
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


//TODO CREATURE_AI try to make sure the creature will do proper activity in the room.
//TODO CREATURE_AI try to select lair far away from CTA and enemies
struct Room *get_room_of_given_kind_for_thing(struct Thing *thing, struct Dungeon *dungeon, RoomKind rkind)
{
  struct Room *room;
  struct Room *retroom;
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  long retdist,dist,pay;
  unsigned long k;
  long i;
  retdist = LONG_MAX;
  retroom = INVALID_ROOM;
  i = dungeon->room_kind[rkind];
  k = 0;
  while (i != 0)
  {
    room = room_get(i);
    if (room_is_invalid(room))
    {
      ERRORLOG("Jump to invalid room detected");
      break;
    }
    i = room->next_of_owner;
    // Per-room code
    if (room->kind == RoK_TREASURE)
    {
        cctrl = creature_control_get_from_thing(thing);
        crstat = creature_stats_get_from_thing(thing);
        dungeon = get_dungeon(thing->owner);
        if (dungeon->tortured_creatures[thing->model] )
        {
          pay = compute_creature_max_pay(crstat->pay,cctrl->explevel) / 2;
        } else
        {
          pay = compute_creature_max_pay(crstat->pay,cctrl->explevel);
        }
        if (room->capacity_used_for_storage > pay)
          continue;
    }
    dist =  abs(thing->mappos.y.stl.num - room->central_stl_y);
    dist += abs(thing->mappos.x.stl.num - room->central_stl_x);
    if (retdist > dist)
    {
      retdist = dist;
      retroom = room;
    }
    // Per-room code ends
    k++;
    if (k > ROOMS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping rooms list");
      break;
    }
  }
  return retroom;
}

struct Room * find_random_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, signed char plyr_idx, signed char rkind, unsigned char a4)
{
    return _DK_find_random_room_for_thing_with_spare_room_item_capacity(thing, plyr_idx, rkind, a4);
}

long create_workshop_object_in_workshop_room(long plyr_idx, long tng_class, long tng_kind)
{
    struct Coord3d pos;
    struct Thing *thing;
    struct Room *room;
    struct Dungeon *dungeon;
    //return _DK_create_workshop_object_in_workshop_room(plyr_idx, tng_class, tng_kind);
    pos.x.val = 0;
    pos.y.val = 0;
    pos.z.val = 0;
    switch (tng_class)
    {
    case TCls_Trap:
        thing = create_object(&pos, trap_to_object[tng_kind], plyr_idx, -1);
        break;
    case TCls_Door:
        thing = create_object(&pos, door_to_object[tng_kind], plyr_idx, -1);
        break;
    default:
        thing = INVALID_THING;
        ERRORLOG("No known workshop crate can represent %s model %d",thing_class_code_name(tng_class),(int)tng_kind);
        break;
    }
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Could not create workshop crate thing");
        return 0;
    }
    room = find_random_room_for_thing_with_spare_room_item_capacity(thing, plyr_idx, RoK_WORKSHOP, 0);
    if (room_is_invalid(room))
    {
        ERRORLOG("No room for thing");
        delete_thing_structure(thing, 0);
        return 0;
    }
    if ( !find_random_valid_position_for_thing_in_room_avoiding_object(thing, room, &pos) )
    {
        ERRORLOG("Could not find room for thing");
        delete_thing_structure(thing, 0);
        return 0;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    move_thing_in_map(thing, &pos);
    room->used_capacity++;
    room->capacity_used_for_storage++;
    dungeon = get_players_num_dungeon(plyr_idx);
    switch (tng_class)
    {
    case TCls_Trap:
        if ( !dungeon->trap_placeable[tng_kind] ) {
            event_create_event(thing->mappos.x.val, thing->mappos.y.val, EvKind_NewTrap, plyr_idx, tng_kind);
        }
        break;
    case TCls_Door:
        if ( !dungeon->door_placeable[tng_kind] ) {
          event_create_event(thing->mappos.x.val, thing->mappos.y.val, EvKind_NewDoor, plyr_idx, tng_kind);
        }
        break;
    default:
        break;
    }
    create_effect(&pos, TngEff_Unknown56, thing->owner);
    thing_play_sample(thing, 89, 100, 0, 3, 0, 2, 256);
    return 1;
}

short delete_room_slab_when_no_free_room_structures(long a1, long a2, unsigned char a3)
{
    SYNCDBG(8,"Starting");
    return _DK_delete_room_slab_when_no_free_room_structures(a1, a2, a3);
}

void kill_room_slab_and_contents(unsigned char a1, unsigned char a2, unsigned char a3)
{
  _DK_kill_room_slab_and_contents(a1, a2, a3);
}

void free_room_structure(struct Room *room)
{
  _DK_free_room_structure(room);
}

void reset_creatures_rooms(struct Room *room)
{
  _DK_reset_creatures_rooms(room);
}

void replace_room_slab(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char owner, unsigned char a5)
{
    struct SlabMap *slb;
    //_DK_replace_room_slab(room, slb_x, slb_y, owner, a5);
    if (room->kind == RoK_BRIDGE)
    {
        slb = get_slabmap_block(slb_x, slb_y);
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
        if ( a5 )
        {
            place_slab_type_on_map(SlbT_PATH, slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 0);
        } else
        {
            place_slab_type_on_map(SlbT_CLAIMED, slab_subtile(slb_x,0), slab_subtile(slb_y,0), owner, 0);
            increase_dungeon_area(owner, 1);
        }
    }
}

struct Room *place_room(unsigned char owner, RoomKind rkind, unsigned short stl_x, unsigned short stl_y)
{
    struct Room *room;
    struct RoomData *rdata;
    struct Dungeon *dungeon;
    struct SlabMap *slb;
    long slb_x, slb_y;
    long i;
    //return _DK_place_room(owner, rkind, stl_x, stl_y);
    game.field_14EA4B = 1;
    if (subtile_coords_invalid(stl_x, stl_y))
        return INVALID_ROOM;
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    slb = get_slabmap_block(slb_x,slb_y);
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
    room = create_room(owner, rkind, stl_x, stl_y);
    if (room_is_invalid(room))
        return INVALID_ROOM;
    // Make sure we have first subtile
    stl_x = stl_slab_starting_subtile(stl_x);
    stl_y = stl_slab_starting_subtile(stl_y);
    // Update slab type on map
    rdata = room_data_get_for_room(room);
    i = get_slab_number(slb_x, slb_y);
    if ((rkind == RoK_GUARDPOST) || (rkind == RoK_BRIDGE))
    {
        delete_room_slabbed_objects(i);
        place_animating_slab_type_on_map(rdata->numfield_0, 0, stl_x, stl_y, owner);
    } else
    {
        delete_room_slabbed_objects(i);
        place_slab_type_on_map(rdata->numfield_0, stl_x, stl_y, owner, 0);
    }
    SYNCDBG(7,"Updating efficiency");
    do_slab_efficiency_alteration(slb_x, slb_y);
    update_room_efficiency(room);
    set_room_capacity(room,0);
    if (owner != game.neutral_player_num)
    {
        dungeon = get_dungeon(owner);
        dungeon->lvstats.rooms_constructed++;
    }
    pannel_map_update(stl_x, stl_y, 3, 3);
    return room;
}

struct Room *find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, char a2, char a3, unsigned char a4)
{
    return _DK_find_nearest_room_for_thing_with_spare_item_capacity(thing, a2, a3, a4);
}

struct Room * pick_random_room(PlayerNumber plyr_idx, RoomKind rkind)
{
    return _DK_pick_random_room(plyr_idx, rkind);
}

TbBool remove_item_from_room_capacity(struct Room *room)
{
    if ( (room->used_capacity <= 0) || (room->capacity_used_for_storage <= 0) )
    {
        ERRORLOG("Room %s index %d does not contain item to remove",room_code_name(room->kind),(int)room->index);
        return false;
    }
    room->used_capacity--;
    room->capacity_used_for_storage--;
    return true;
}

TbBool add_item_to_room_capacity(struct Room *room)
{
    //TODO ROOMS think if we really want to compare it with used_capacity and not capacity_used_for_storage
    if (room->used_capacity >= room->total_capacity)
    {
        return false;
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
void change_ownership_or_delete_object_thing_in_room(struct Room *room, struct Thing *thing, long parent_idx, PlayerNumber newowner)
{
    PlayerNumber oldowner;
    struct Objects *objdat;
    objdat = get_objects_data_for_thing(thing);
    // Handle specific things in rooms for which we have a special re-creation code
    switch (room->kind)
    {
    case RoK_GUARDPOST:
        // Guard post owns only flag
        if (object_is_guard_flag(thing))
        {
            create_guard_flag_object(&thing->mappos, newowner, parent_idx);
            delete_thing_structure(thing, 0);
            return;
        }
        break;
    case RoK_LIBRARY:
        // Library owns books and candles
        if (thing_is_spellbook(thing) && ((thing->alloc_flags & TAlF_IsDragged) == 0))
        {
            oldowner = thing->owner;
            thing->owner = newowner;
            if (oldowner != game.neutral_player_num) {
                remove_spell_from_player(book_thing_to_magic(thing), oldowner);
            }
            if (newowner != game.neutral_player_num) {
                add_spell_to_player(book_thing_to_magic(thing), newowner);
            }
            return;
        }
        break;
    case RoK_WORKSHOP:
        // Workshop owns trap boxes, machines and anvils
        if (thing_is_door_or_trap_box(thing) && ((thing->field_1 & TF1_Unkn01) == 0) )
        {
            oldowner = thing->owner;
            thing->owner = newowner;
            remove_workshop_item(oldowner, workshop_object_class[thing->model], box_thing_to_door_or_trap(thing));
            add_workshop_item(newowner, workshop_object_class[thing->model], box_thing_to_door_or_trap(thing));
            return;
        }
        break;
    case RoK_TREASURE:
        if (thing_is_gold_hoard(thing))
        {
            oldowner = thing->owner;
            struct Dungeon *dungeon;
            {
                dungeon = get_dungeon(newowner);
                dungeon->total_money_owned += thing->long_13;
            }
            if (oldowner != game.neutral_player_num)
            {
                dungeon = get_dungeon(oldowner);
                dungeon->total_money_owned -= thing->long_13;
            }
            thing->owner = newowner;
            return;
        }
        break;
    case RoK_LAIR:
        // Lair - owns creature lairs
        if (objdat->related_creatr_model)
        {
            if (thing->word_13)
            {
                struct Thing *tmptng;
                struct CreatureControl *cctrl;
                tmptng = thing_get(thing->word_13);
                cctrl = creature_control_get_from_thing(tmptng);
                if (cctrl->lairtng_idx == thing->index) {
                    creature_remove_lair_from_room(tmptng, room);
                } else {
                    ERRORLOG("Lair thing thinks it belongs to a creature, but the creature disagrees.");
                }
            } else
            {
                ERRORLOG("Lair thing %d has no owner!",(int)thing->index);
            }
            return;
        }
        break;
    default:
        break;
    }
    // If an object has parent slab, then it should change owner with that slab
    if (thing->parent_idx != -1)
    {
        if (parent_idx == thing->parent_idx) {
            thing->owner = newowner;
        }
        return;
    }
    // Otherwise, changing owner depends on what the object represents
    switch ( objdat->field_14 )
    {
      case 0:
      case 1:
      case 3:
        thing->owner = newowner;
        break;
      case 2:
        destroy_object(thing);
        break;
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
TbBool change_room_subtile_things_ownership(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    struct Thing *thing;
    struct Map *mapblk;
    long parent_idx;
    long i;
    unsigned long k;
    mapblk = get_map_block_at(stl_x,stl_y);
    parent_idx = get_slab_number(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
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
        if (thing->class_id == TCls_Object)
        {
            change_ownership_or_delete_object_thing_in_room(room, thing, parent_idx, plyr_idx);
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return true;
}

/**
 * Changes ownership of things in a room while it's being claimed.
 * @param room The room to be affected.
 * @param plyr_idx The new owner.
 */
void change_room_map_element_ownership(struct Room *room, PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    //_DK_change_room_map_element_ownership(room, plyr_idx); return;
    dungeon = get_dungeon(plyr_idx);
    {
        struct SlabMap *slb;
        PlayerNumber owner;
        slb = get_slabmap_direct(room->slabs_list);
        owner = slabmap_owner(slb);
        if (owner != game.neutral_player_num) {
            decrease_room_area(owner, room->slabs_count);
        }
        increase_room_area(dungeon->owner, room->slabs_count);
    }
    unsigned long i;
    unsigned long k;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x,slb_y;
        struct SlabMap *slb;
        slb = get_slabmap_direct(i);
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code
        MapSubtlCoord start_stl_x,start_stl_y;
        MapSubtlCoord end_stl_x,end_stl_y;
        set_slab_explored(plyr_idx, slb_x, slb_y);
        slabmap_set_owner(slb, plyr_idx);
        start_stl_x = slab_subtile(slb_x,0);
        start_stl_y = slab_subtile(slb_y,0);
        end_stl_x = slab_subtile(slb_x+1,0);
        end_stl_y = slab_subtile(slb_y+1,0);
        // Do the loop
        MapSubtlCoord stl_x,stl_y;
        for (stl_y=start_stl_y; stl_y < end_stl_y; stl_y++)
        {
            for (stl_x=start_stl_x; stl_x < end_stl_x; stl_x++)
            {
                change_room_subtile_things_ownership(room, stl_x, stl_y, plyr_idx);
            }
        }
        pannel_map_update(start_stl_x, start_stl_y, 3, 3);
        // Per-slab code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
}

void copy_block_with_cube_groups(short a1, unsigned char a2, unsigned char a3)
{
    _DK_copy_block_with_cube_groups(a1, a2, a3);
}

void redraw_slab_map_elements(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    // Prepare start and end subtiles
    MapSubtlCoord start_stl_x,start_stl_y;
    MapSubtlCoord end_stl_x,end_stl_y;
    start_stl_x = slab_subtile(slb_x,0);
    start_stl_y = slab_subtile(slb_y,0);
    end_stl_x = slab_subtile(slb_x,3);
    end_stl_y = slab_subtile(slb_y,3);
    // Do the loop
    MapSubtlCoord stl_x,stl_y;
    for (stl_y=start_stl_y; stl_y < end_stl_y; stl_y++)
    {
        for (stl_x=start_stl_x; stl_x < end_stl_x; stl_x++)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x,stl_y);
            if (!map_block_invalid(mapblk))
            {
                copy_block_with_cube_groups(-get_mapblk_column_index(mapblk), stl_x, stl_y);
            }
        }
    }
}

void redraw_room_map_elements(struct Room *room)
{
    unsigned long i;
    unsigned long k;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x,slb_y;
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code start
        redraw_slab_map_elements(slb_x, slb_y);
        // Per-slab code end
        k++;
        if (k > map_tiles_x*map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
}

void do_unprettying(unsigned char a1, long a2, long a3)
{
    _DK_do_unprettying(a1, a2, a3);
}

void do_room_unprettying(struct Room *room, PlayerNumber plyr_idx)
{
    unsigned long i;
    unsigned long k;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        MapSlabCoord slb_x,slb_y;
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code start
        do_unprettying(plyr_idx, slb_x, slb_y);
        // Per-slab code end
        k++;
        if (k > map_tiles_x*map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
}

void do_room_integration(struct Room *room)
{
    struct RoomData *rdata;
    Room_Update_Func cb;
    update_room_efficiency(room);
    rdata = room_data_get_for_room(room);
    room->field_C = (long)game.hits_per_slab * (long)room->slabs_count;
    cb = rdata->update_total_capacity;
    if (cb != NULL)
        cb(room);
    cb = rdata->ofsfield_7;
    if (cb != NULL)
        cb(room);
    cb = rdata->offfield_B;
    if (cb != NULL)
        cb(room);
    init_room_sparks(room);
}

void output_room_takeover_message(struct Room *room, PlayerNumber oldowner, PlayerNumber newowner)
{
    if (room->kind == RoK_ENTRANCE)
    {
        if (is_my_player_number(oldowner)) {
            output_message(SMsg_EntranceLost, 0, 1);
        } else
        if (is_my_player_number(newowner))
        {
            output_message(SMsg_EntranceClaimed, 0, 1);
        }
    } else
    if (is_my_player_number(newowner))
    {
        if (oldowner == game.neutral_player_num) {
            output_message(SMsg_NewRoomTakenOver, 0, 1);
        } else {
            output_message(SMsg_EnemyRoomTakeOver, 0, 1);
        }
    }
}

/**
 * Function used for claiming unowned room by a creature.
 * @param room The room to be claimed.
 * @param claimtng The creature which claimed it.
 * @return
 */
long claim_room(struct Room *room,struct Thing *claimtng)
{
    PlayerNumber oldowner;
    //return _DK_claim_room(room,claimtng);
    oldowner = room->owner;
    if ((oldowner != game.neutral_player_num) || (claimtng->owner == game.neutral_player_num))
    {
        return 0;
    }
    room->owner = claimtng->owner;
    room->field_C = room->slabs_count * game.hits_per_slab;
    add_room_to_players_list(room, claimtng->owner);
    change_room_map_element_ownership(room, claimtng->owner);
    redraw_room_map_elements(room);
    do_room_unprettying(room, claimtng->owner);
    event_create_event(subtile_coord_center(room->central_stl_x), subtile_coord_center(room->central_stl_y),
        EvKind_RoomTakenOver, claimtng->owner, room->kind);
    do_room_integration(room);
    thing_play_sample(claimtng, 116, 100, 0, 3, 0, 4, 256);
    output_room_takeover_message(room, oldowner, claimtng->owner);
    return 1;
}

/**
 * Function used for claiming room owned by another keeper by a creature.
 * @param room The room to be claimed.
 * @param claimtng The creature which claimed it.
 * @return
 */
long claim_enemy_room(struct Room *room,struct Thing *claimtng)
{
    PlayerNumber oldowner;
    //return _DK_claim_enemy_room(room,claimtng);
    oldowner = room->owner;
    if ((oldowner == claimtng->owner) || (claimtng->owner == game.neutral_player_num))
    {
        return 0;
    }
    reset_state_of_creatures_working_in_room(room);
    remove_room_from_players_list(room,oldowner);
    room->owner = claimtng->owner;
    room->field_C = (long)game.hits_per_slab * (long)room->slabs_count;
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

/******************************************************************************/
