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
#include "room_library.h"
#include "room_workshop.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
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
DLLIMPORT struct Room * _DK_find_random_room_for_thing(struct Thing *thing, signed char plyr_idx, signed char rkind, unsigned char a4);
DLLIMPORT struct Room * _DK_find_random_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, signed char newowner, signed char rkind, unsigned char a4);
DLLIMPORT long _DK_claim_room(struct Room *room,struct Thing *claimtng);
DLLIMPORT long _DK_claim_enemy_room(struct Room *room,struct Thing *claimtng);
DLLIMPORT struct Room *_DK_get_room_thing_is_on(struct Thing *thing);
DLLIMPORT void _DK_change_room_map_element_ownership(struct Room *room, unsigned char plyr_idx);
DLLIMPORT void _DK_copy_block_with_cube_groups(short a1, unsigned char plyr_idx, unsigned char a3);
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
  {50,  0, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 201, 201},
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

unsigned char const slabs_to_centre_peices[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  2,  2,  2,  3,  4,  4,
  4,  5,  6,  6,  6,  7,  8,  9,  9,
  9, 10, 11, 12, 12, 12, 13, 14, 15,
 16, 16, 16, 17, 18, 19, 20, 20, 20,
 21, 22, 23, 24, 25,
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
DLLIMPORT void _DK_set_room_capacity(struct Room *room, long skip_integration);
DLLIMPORT void _DK_set_room_efficiency(struct Room *room);
DLLIMPORT struct Room *_DK_link_adjacent_rooms_of_type(unsigned char owner, long stl_x, long stl_y, unsigned char rkind);
DLLIMPORT struct Room *_DK_create_room(unsigned char a1, unsigned char plyr_idx, unsigned short a3, unsigned short a4);
DLLIMPORT void _DK_create_room_flag(struct Room *room);
DLLIMPORT struct Room *_DK_allocate_free_room_structure(void);
DLLIMPORT unsigned short _DK_i_can_allocate_free_room_structure(void);
DLLIMPORT struct Room *_DK_find_room_with_spare_room_item_capacity(unsigned char a1, signed char plyr_idx);
DLLIMPORT long _DK_create_workshop_object_in_workshop_room(long a1, long plyr_idx, long a3);
DLLIMPORT unsigned char _DK_find_first_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT struct Room* _DK_find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing,
    signed char plyr_idx, signed char a3, unsigned char a4, long a5);
DLLIMPORT struct Room* _DK_find_room_with_spare_capacity(unsigned char a1, signed char plyr_idx, long a3);
DLLIMPORT short _DK_delete_room_slab_when_no_free_room_structures(long a1, long plyr_idx, unsigned char a3);
DLLIMPORT long _DK_calculate_room_efficiency(struct Room *room);
DLLIMPORT void _DK_kill_room_slab_and_contents(unsigned char a1, unsigned char plyr_idx, unsigned char a3);
DLLIMPORT void _DK_free_room_structure(struct Room *room);
DLLIMPORT void _DK_reset_creatures_rooms(struct Room *room);
DLLIMPORT void _DK_replace_room_slab(struct Room *room, long plyr_idx, long a3, unsigned char a4, unsigned char a5);
DLLIMPORT struct Room *_DK_place_room(unsigned char a1, unsigned char plyr_idx, unsigned short a3, unsigned short a4);
DLLIMPORT struct Room *_DK_find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, char plyr_idx, char a3, unsigned char a4);
DLLIMPORT struct Room * _DK_pick_random_room(PlayerNumber newowner, int rkind);
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

struct Room *subtile_room_get(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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

/**
 * Recomputes the amount of slabs the player has.
 * @param plyr_idx
 * @param rkind
 */
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

void get_room_kind_total_and_used_capacity(struct Dungeon *dungeon, RoomKind room_kind, long *total_cap, long *used_cap)
{
    struct Room * room;
    int used_capacity;
    int total_capacity;
    long i;
    unsigned long k;
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
    *total_cap = total_capacity;
    *used_cap = used_capacity;
}

long get_room_kind_used_capacity_fraction(PlayerNumber plyr_idx, RoomKind room_kind)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    long used_capacity;
    long total_capacity;
    get_room_kind_total_and_used_capacity(dungeon, room_kind, &total_capacity, &used_capacity);
    if (total_capacity <= 0) {
        return 0;
    }
    return (used_capacity * 256) / total_capacity;
}

void set_room_capacity(struct Room *room, TbBool skip_integration)
{
    struct RoomData *rdata;
    //_DK_set_room_capacity(room, capac); return;
    rdata = room_data_get_for_room(room);
    if ((!skip_integration) || (rdata->field_F))
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

void init_reposition_struct(struct RoomReposition * rrepos)
{
    long i;
    rrepos->used = 0;
    for (i=0; i < ROOM_REPOSITION_COUNT; i++)
        rrepos->models[i] = 0;
}

TbBool store_reposition_entry(struct RoomReposition * rrepos, ThingModel tngmodel)
{
    rrepos->used++;
    if (rrepos->used > ROOM_REPOSITION_COUNT) {
        rrepos->used = ROOM_REPOSITION_COUNT;
        return false;
    }
    int ri;
    // Don't store the same entry two times
    for (ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] == tngmodel) {
            return true;
        }
    }
    for (ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] == 0) {
            rrepos->models[ri] = tngmodel;
            break;
        }
    }
    return true;
}

void reposition_all_books_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    long i;
    unsigned long k;
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code
        if (thing->class_id == TCls_Object)
        {
            PowerKind spl_idx;
            ThingModel objkind;
            objkind = thing->model;
            spl_idx = book_thing_to_magic(thing);
            if ((spl_idx > 0) && ((thing->alloc_flags & 0x80) == 0))
            {
                if (!store_reposition_entry(rrepos, objkind)) {
                    WARNLOG("Too many things to reposition in %s.",room_code_name(room->kind));
                }
                if (!is_neutral_thing(thing))
                    remove_spell_from_player(spl_idx, room->owner);
                delete_thing_structure(thing, 0);
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

TbBool rectreate_repositioned_book_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    if ((rrepos->used < 0) || (room->used_capacity >= room->total_capacity)) {
        return false;
    }
    int ri;
    for (ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] != 0)
        {
            struct Thing *objtng;
            objtng = create_spell_in_library(room, rrepos->models[ri], stl_x, stl_y);
            if (!thing_is_invalid(objtng))
            {
                rrepos->used--;
                rrepos->models[ri] = 0;
                return true;
            }
        }
    }
    return false;
}

int check_books_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return -2; // do nothing
    if (get_map_floor_filled_subtiles(mapblk) != 1) {
        return -1; // re-create all
    }
    long i;
    unsigned long k;
    int books_at_subtile;
    books_at_subtile = 0;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code
        if (thing->class_id == TCls_Object)
        {
            PowerKind spl_idx;
            spl_idx = book_thing_to_magic(thing);
            if ((spl_idx > 0) && ((thing->alloc_flags & 0x80) == 0))
            {
                // If exceeded capacity of the library
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
                    books_at_subtile++;
                }
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return books_at_subtile; // Increase used capacity
}

void count_and_reposition_books_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    int books_at_subtile;
    books_at_subtile = check_books_on_subtile_for_reposition_in_room(room, stl_x, stl_y);
    if (books_at_subtile > 0) {
        // This subtile contains spells
        SYNCDBG(19,"Got %d books at (%d,%d)",(int)books_at_subtile,(int)stl_x,(int)stl_y);
        room->used_capacity += books_at_subtile;
    } else
    {
        switch (books_at_subtile)
        {
        case -2:
            // No spells, but also cannot recreate anything on this subtile
            break;
        case -1:
            // All spells are to be removed from the subtile and stored for re-creation
            reposition_all_books_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        case 0:
            // There are no spells there, something can be re-created
            rectreate_repositioned_book_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        default:
            WARNLOG("Invalid value returned by reposition check");
            break;
        }
    }
}

/**
 * Updates count of books (used capacity) in a library.
 * Also repositions spellbooks which are in solid rock.
 * @param room The room to be recomputed and repositioned.
 */
void count_books_in_room(struct Room *room)
{
    long n;
    SYNCDBG(17,"Starting for %s",room_code_name(room->kind));
    //_DK_count_books_in_room(room); return;
    struct RoomReposition rrepos;
    init_reposition_struct(&rrepos);
    // Making two loops guarantees that no rrepos things will be lost
    for (n=0; n < 2; n++)
    {
        // The correct count should be taken from last sweep
        room->used_capacity = 0;
        room->capacity_used_for_storage = 0;
        unsigned long i;
        unsigned long k;
        k = 0;
        i = room->slabs_list;
        while (i > 0)
        {
            MapSubtlCoord slb_x,slb_y;
            slb_x = slb_num_decode_x(i);
            slb_y = slb_num_decode_y(i);
            // Per-slab code
            long dx,dy;
            for (dy=0; dy < STL_PER_SLB; dy++)
            {
                for (dx=0; dx < STL_PER_SLB; dx++)
                {
                    count_and_reposition_books_in_room_on_subtile(room, 3*slb_x+dx, 3*slb_y+dy, &rrepos);
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
    if (rrepos.used > 0) {
        ERRORLOG("The %s index %d capacity %d wasn't enough; %d items belonging to player %d dropped",
          room_code_name(room->kind),(int)room->index,(int)room->total_capacity,(int)rrepos.used,(int)room->owner);
    }
    room->capacity_used_for_storage = room->used_capacity;
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
          struct Dungeon *dungeon;
          dungeon = get_players_num_dungeon(room->owner);
          wptr = &dungeon->room_kind[room->kind];
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
    long non_creature;
    non_creature = 0;
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
        stl_x = x + STL_PER_SLB * (long)small_around[n].delta_x;
        stl_y = y + STL_PER_SLB * (long)small_around[n].delta_y;
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
    WARNLOG("Cannot find position in %s index %d to place an ensign.",room_code_name(room->kind),(int)room->index);
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

void create_room_flag(struct Room *room)
{
    struct Thing *thing;
    struct Coord3d pos;
    MapSubtlCoord stl_x,stl_y;
    //_DK_create_room_flag(room);
    stl_x = slab_subtile_center(slb_num_decode_x(room->slabs_list));
    stl_y = slab_subtile_center(slb_num_decode_y(room->slabs_list));
    SYNCDBG(7,"Starting for %s at (%d,%d)",room_code_name(room->kind),(int)stl_x,(int)stl_y);
    if (room_can_have_ensign(room->kind))
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
    if (room_can_have_ensign(room->kind))
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

/**
 * Re-initializes all players rooms of specific kind.
 * @param rkind
 * @param skip_integration
 * @return Total amount of rooms which were reinitialized.
 */
long reinitialise_rooms_of_kind(RoomKind rkind, TbBool skip_integration)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned int i,k,n;
    k = 0;
    for (n=0; n < DUNGEONS_COUNT; n++)
    {
        dungeon = get_dungeon(n);
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
            // Per-room code starts
            set_room_capacity(room, skip_integration);
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
    RoomKind rkind;
    for (rkind=1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        reinitialise_rooms_of_kind(rkind, false);
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
            rkind = slab_corresponding_room(slb->kind);
            if (rkind > 0)
                room = create_room(slabmap_owner(slb), rkind, slab_subtile_center(slb_x), slab_subtile_center(slb_y));
            else
                room = INVALID_ROOM;
            if (!room_is_invalid(room))
            {
                set_room_efficiency(room);
                set_room_capacity(room, false);
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

TbBool update_room_total_capacities(struct Room *room)
{
    struct RoomData *rdata;
    Room_Update_Func cb;
    SYNCDBG(17,"Starting for %s index %d",room_code_name(room->kind),(int)room->index);
    room->field_C = (long)game.hits_per_slab * (long)room->slabs_count;
    rdata = room_data_get_for_room(room);
    cb = rdata->update_total_capacity;
    if (cb != NULL)
        cb(room);
    return true;
}

TbBool update_room_contents(struct Room *room)
{
    struct RoomData *rdata;
    Room_Update_Func cb;
    rdata = room_data_get_for_room(room);
    SYNCDBG(17,"Starting for %s index %d",room_code_name(room->kind),(int)room->index);
    cb = rdata->update_used_capacity;
    if (cb != NULL)
        cb(room);
    cb = rdata->offfield_B;
    if (cb != NULL)
        cb(room);
    return true;
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

/**
 * Returns if given slab lies at outer border of an area.
 * Only slabs which are surrounded by the same slab kind from 4 sides are not at outer border of an area.
 * @param slb_x The slab to be checked, X coordinate.
 * @param slb_y The slab to be checked, y coordinate.
 * @return True if the slab lies at outer border of an area, false otherwise.
 */
TbBool slab_is_area_outer_border(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    PlayerNumber plyr_idx;
    SlabKind slbkind;
    struct SlabMap *slb;
    // Store kind and owner of the slab
    slb = get_slabmap_block(slb_x,slb_y);
    slbkind = slb->kind;
    plyr_idx = slabmap_owner(slb);
    long n;
    for (n=0; n < SMALL_AROUND_LENGTH; n++)
    {
        long aslb_x,aslb_y;
        aslb_x = slb_x + (long)small_around[n].delta_x;
        aslb_y = slb_y + (long)small_around[n].delta_y;
        struct SlabMap *slb;
        slb = get_slabmap_block(aslb_x,aslb_y);
        if ((slb->kind != slbkind) || (slabmap_owner(slb) != plyr_idx)) {
            return true;
        }
    }
    return false;
}

TbBool find_random_position_at_border_of_room(struct Coord3d *pos, const struct Room *room)
{
    long i;
    unsigned long n;
    // Find a random slab in the room to be used as our starting point
    i = ACTION_RANDOM(room->slabs_count);
    n = room->slabs_list;
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
        MapSlabCoord slb_x, slb_y;
        slb_x = subtile_slab_fast(stl_num_decode_x(n));
        slb_y = subtile_slab_fast(stl_num_decode_y(n));
        if (slab_is_area_outer_border(slb_x, slb_y))
        {
            pos->x.val = subtile_coord(slab_subtile(slb_x,0),ACTION_RANDOM(STL_PER_SLB*256));
            pos->y.val = subtile_coord(slab_subtile(slb_y,0),ACTION_RANDOM(STL_PER_SLB*256));
            pos->z.val = subtile_coord(1,0);
            return true;
        }
        n = get_next_slab_number_in_room(n);
        i--;
    }
    return false;
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

struct Room *find_room_for_thing_with_used_capacity(const struct Thing *creatng, PlayerNumber plyr_idx, RoomKind rkind, unsigned char nav_no_owner, long min_used_cap)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long i;
    unsigned long k;
    SYNCDBG(18,"Starting");
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
        struct Coord3d pos;
        if ((room->used_capacity >= min_used_cap) && find_first_valid_position_for_thing_in_room(creatng, room, &pos))
        {
            if (thing_is_creature(creatng))
            {
                if (creature_can_navigate_to(creatng, &pos, nav_no_owner)) {
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
struct Room *find_room_with_spare_capacity(unsigned char owner, signed char rkind, long spare)
{
    struct Dungeon *dungeon;
    if ((rkind < 0) || (rkind >= ROOM_TYPES_COUNT))
        return INVALID_ROOM;
    dungeon = get_dungeon(owner);
    if (dungeon_invalid(dungeon))
        return INVALID_ROOM;
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
    return INVALID_ROOM;
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

/**
 * Retrieves a position in room which could be used as a place for thing to enter that room.
 * Returns first valid position found.
 * @todo For Temple and Entrance, this should return border slabs only.
 * @param thing
 * @param room
 * @param pos
 * @return
 */
TbBool find_first_valid_position_for_thing_in_room(const struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    long block_radius;
    //return _DK_find_first_valid_position_for_thing_in_room(thing, room, pos);
    if (!room_exists(room))
    {
        ERRORLOG("Tried to find position in non-existing room");
        pos->x.val = subtile_coord_center(map_subtiles_x/2);
        pos->y.val = subtile_coord_center(map_subtiles_y/2);
        pos->z.val = subtile_coord(1,0);
        return false;
    }
    block_radius = subtile_coord(thing_nav_block_sizexy(thing),0) / 2;

    unsigned long i;
    unsigned long k;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        MapSubtlCoord slb_x,slb_y;
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        // Per-slab code
        long dx,dy;
        for (dy=0; dy < 3; dy++)
        {
            for (dx=0; dx < 3; dx++)
            {
                MapSubtlCoord stl_x,stl_y;
                struct Map *mapblk;
                stl_x = 3*slb_x+dx;
                stl_y = 3*slb_y+dy;
                mapblk = get_map_block_at(stl_x,stl_y);
                if (((mapblk->flags & 0x10) == 0) && ((get_navigation_map(stl_x,stl_y) & 0x0F) < 4))
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
    pos->x.val = subtile_coord_center(map_subtiles_x/2);
    pos->y.val = subtile_coord_center(map_subtiles_y/2);
    pos->z.val = subtile_coord(1,0);
    return false;
}

struct Room *find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing, signed char owner, RoomKind rkind, unsigned char nav_no_owner, long spare)
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
long count_rooms_creature_can_navigate_to(struct Thing *thing, unsigned char owner, RoomKind rkind, unsigned char nav_no_owner)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    long count;
    SYNCDBG(18,"Starting");
    dungeon = get_dungeon(owner);
    count = 0;
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
        struct Coord3d pos;
        if (find_first_valid_position_for_thing_in_room(thing, room, &pos) && (room->used_capacity > 0))
        {
            if (creature_can_navigate_to(thing, &pos, nav_no_owner))
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
 * Returns if a creature can navigate to any kind of players rooms.
 * Only rooms which have used capacity are taken into account.
 * @param thing
 * @param owner
 * @return
 */
TbBool creature_can_get_to_any_of_players_rooms(struct Thing *thing, PlayerNumber owner)
{
    RoomKind rkind;
    for (rkind=1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        if (count_rooms_creature_can_navigate_to(thing, owner, rkind, 0) > 0)
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
 * @param nav_no_owner
 * @return
 */
struct Room *find_random_room_creature_can_navigate_to(struct Thing *thing, unsigned char owner, RoomKind rkind, unsigned char nav_no_owner)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    long count,selected;
    SYNCDBG(18,"Starting");
    count = count_rooms_creature_can_navigate_to(thing, owner, rkind, nav_no_owner);
    if (count < 1)
        return NULL;
    dungeon = get_dungeon(owner);
    selected = ACTION_RANDOM(count);
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
        struct Coord3d pos;
        if (find_first_valid_position_for_thing_in_room(thing, room, &pos) && (room->used_capacity > 0))
        {
            if (creature_can_navigate_to(thing, &pos, nav_no_owner))
            {
                if (selected > 0)
                {
                    selected--;
                } else
                {
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
    MapCoordDelta delta_x,delta_y;
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
        delta_x = (room->central_stl_x << 8) - (MapCoordDelta)pos->x.val;
        delta_y = (room->central_stl_y << 8) - (MapCoordDelta)pos->y.val;
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
    long retdist,pay;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
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
        long attractiveness; // Says how attractive is a specific room, based on some room-specific code below
        attractiveness = 10; // Default attractiveness
        switch (room->kind)
        {
        case RoK_TREASURE:
            pay = calculate_correct_creature_pay(thing);
            if (room->capacity_used_for_storage > pay) {
                // This room isn't attractive at all - creature won't get salary there
                attractiveness = 0;
            }
            break;
        case RoK_LAIR:
            if (room->index == cctrl->lairtng_idx) {
                // A room where we already have a lair is a few times more attractive
                attractiveness += 70;
            }
            break;
        }
        if (attractiveness > 0)
        {
            struct Thing *enmtng;
            enmtng = get_creature_in_range_who_is_enemy_of_able_to_attack_and_not_specdigger(thing->mappos.x.val, thing->mappos.y.val, 10, thing->owner);
            if (!thing_is_invalid(enmtng)) {
                // A room with enemies inside is very unattractive, but still possible to select
                attractiveness = 1;
            }
        }
        if (attractiveness > 0)
        {
            long dist;
            dist =  abs(thing->mappos.y.stl.num - (int)room->central_stl_y);
            dist += abs(thing->mappos.x.stl.num - (int)room->central_stl_x);
            dist = (dist*100)/attractiveness;
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
    return retroom;
}

struct Room *find_random_room_for_thing(struct Thing *thing, signed char plyr_idx, signed char rkind, unsigned char a4)
{
    return _DK_find_random_room_for_thing(thing, plyr_idx, rkind, a4);
}

struct Room * find_random_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, signed char plyr_idx, signed char rkind, unsigned char a4)
{
    return _DK_find_random_room_for_thing_with_spare_room_item_capacity(thing, plyr_idx, rkind, a4);
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

struct Room *place_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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
        place_animating_slab_type_on_map(rdata->assigned_slab, 0, stl_x, stl_y, owner);
    } else
    {
        delete_room_slabbed_objects(i);
        place_slab_type_on_map(rdata->assigned_slab, stl_x, stl_y, owner, 0);
    }
    SYNCDBG(7,"Updating efficiency");
    do_slab_efficiency_alteration(slb_x, slb_y);
    update_room_efficiency(room);
    set_room_capacity(room,false);
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
        if (thing_is_door_or_trap_box(thing) && !thing_is_dragged_or_pulled(thing))
        {
            ThingClass tngclass;
            ThingModel tngmodel;
            oldowner = thing->owner;
            thing->owner = newowner;
            tngclass = crate_thing_to_workshop_item_class(thing);
            tngmodel = crate_thing_to_workshop_item_model(thing);
            remove_workshop_item_from_amount_stored(oldowner, tngclass, tngmodel);
            remove_workshop_item_from_amount_placeable(oldowner, tngclass, tngmodel);
            add_workshop_item_to_amounts(newowner, tngclass, tngmodel);
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
    SYNCDBG(7,"Starting for %s index %d owned by player %d",room_code_name(room->kind),(int)room->index,(int)room->owner);
    update_room_efficiency(room);
    update_room_total_capacities(room);
    update_room_contents(room);
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
long claim_room(struct Room *room, struct Thing *claimtng)
{
    PlayerNumber oldowner;
    SYNCDBG(7,"Starting for %s index %d claimed by player %d",room_code_name(room->kind),(int)room->index,(int)claimtng->owner);
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
long claim_enemy_room(struct Room *room, struct Thing *claimtng)
{
    PlayerNumber oldowner;
    SYNCDBG(7,"Starting for %s index %d claimed by player %d",room_code_name(room->kind),(int)room->index,(int)claimtng->owner);
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
