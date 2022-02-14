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
#include "power_specials.h"
#include "room_jobs.h"
#include "room_library.h"
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
#include "magic.h"
#include "room_util.h"
#include "game_legacy.h"
#include "frontmenu_ingame_map.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned short _DK_i_can_allocate_free_room_structure(void);
/******************************************************************************/
extern void research_found_room(PlayerNumber plyr_idx, RoomKind rkind);

void count_slabs_all_only(struct Room *room);
void count_slabs_all_wth_effcncy(struct Room *room);
void count_slabs_div2_wth_effcncy(struct Room *room);
void count_gold_slabs_wth_effcncy(struct Room *room);

void count_gold_hoardes_in_room(struct Room *room);
void count_books_in_room(struct Room *room);
void count_workers_in_room(struct Room *room);
void count_crates_in_room(struct Room *room);
void count_workers_in_room(struct Room *room);
void count_bodies_in_room(struct Room *room);
void count_food_in_room(struct Room *room);
void count_lair_occupants(struct Room *room);
long find_random_valid_position_for_item_in_different_room_avoiding_object(struct Thing* thing, struct Room* skip_room, struct Coord3d* pos);
/******************************************************************************/

RoomKind look_through_rooms[] = {
    RoK_DUNGHEART, RoK_LAIR,      RoK_GARDEN,    RoK_TREASURE,
    RoK_TRAINING,  RoK_WORKSHOP,  RoK_SCAVENGER, RoK_BARRACKS,
    RoK_PRISON,    RoK_TORTURE,   RoK_GRAVEYARD, RoK_TEMPLE,
    RoK_LIBRARY,   RoK_BRIDGE,    RoK_GUARDPOST, RoK_ENTRANCE,
    RoK_TYPES_COUNT,};

struct RoomData room_data[] = {
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},//TODO the tooltip string is invalid
  { 0,  0, NULL, NULL, NULL, 1, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 1, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 1, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 1, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 1, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
  { 0,  0, NULL, NULL, NULL, 0, 0, 0, 0, 0},
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

unsigned short const room_effect_elements[] = { TngEffElm_RedFlame, TngEffElm_BlueFlame, TngEffElm_GreenFlame, TngEffElm_YellowFlame, TngEffElm_None, TngEffElm_None };
const short slab_around[] = { -85, 1, 85, -1 };
/******************************************************************************/
DLLIMPORT short _DK_delete_room_slab_when_no_free_room_structures(long a1, long plyr_idx, unsigned char a3);
DLLIMPORT void _DK_free_room_structure(struct Room *room);
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
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_block_invalid(slb))
        return INVALID_ROOM;
    return room_get(slb->room_index);
}

struct Room *slab_room_get(long slb_x, long slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb))
        return INVALID_ROOM;
    return room_get(slb->room_index);
}

struct Room *slab_number_room_get(SlabCodedCoords slab_num)
{
    MapSlabCoord slb_x = slb_num_decode_x(slab_num);
    MapSlabCoord slb_y = slb_num_decode_y(slab_num);
    return slab_room_get(slb_x, slb_y);
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
  return ((room->alloc_flags & 0x01) != 0);
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
  for (long i = 0; i < arr_length; i++)
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
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    long count = 0;
    long i = dungeon->room_kind[rkind];
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

long count_slabs_of_room_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    long nslabs = 0;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long i = dungeon->room_kind[rkind];
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
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return nslabs;
}

void get_room_kind_total_and_used_capacity(struct Dungeon *dungeon, RoomKind rkind, long *total_cap, long *used_cap)
{
    int total_capacity = 0;
    int used_capacity = 0;
    long i = dungeon->room_kind[rkind];
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

void get_room_kind_total_used_and_storage_capacity(struct Dungeon *dungeon, RoomKind rkind, long *total_cap, long *used_cap, long *storaged_cap)
{
    int total_capacity = 0;
    int used_capacity = 0;
    int storaged_capacity = 0;
    long i = dungeon->room_kind[rkind];
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
    long used_capacity;
    long total_capacity;
    get_room_kind_total_and_used_capacity(dungeon, room_kind, &total_capacity, &used_capacity);
    if (total_capacity <= 0) {
        return 0;
    }
    return (used_capacity * 256) / total_capacity;
}

void set_room_stats(struct Room *room, TbBool skip_integration)
{
    struct RoomData* rdata = room_data_get_for_room(room);
    if ((!skip_integration) || (rdata->field_F))
    {
        do_room_integration(room);
    }
}

void set_room_efficiency(struct Room *room)
{
    room->efficiency = calculate_room_efficiency(room);
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

struct Thing *treasure_room_eats_gold_piles(struct Room *room, MapSlabCoord slb_x,  MapSlabCoord slb_y, struct Thing *hoardtng)
{
    if (room->owner == game.neutral_player_num) {
        return hoardtng;
    }
    //return _DK_treasure_room_eats_gold_piles(room, slb_x, slb_y, thing);
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
    //_DK_count_gold_hoardes_in_room(room); return;
    GoldAmount all_gold_amount = 0;
    int all_wealth_size = 0;
    long wealth_size_holds = gameadd.gold_per_hoard / get_wealth_size_types_count();
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
        if (k > map_tiles_x * map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    room->capacity_used_for_storage = all_gold_amount;
    room->used_capacity = all_wealth_size;
}

void init_reposition_struct(struct RoomReposition * rrepos)
{
    rrepos->used = 0;
    for (long i = 0; i < ROOM_REPOSITION_COUNT; i++)
    {
        rrepos->models[i] = 0;
        rrepos->explevels[i] = 0;
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

TbBool store_creature_reposition_entry(struct RoomReposition * rrepos, ThingModel tngmodel, CrtrExpLevel explevel)
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
            rrepos->explevels[ri] = explevel;
            break;
        }
    }
    return true;
}

void reposition_all_books_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    struct Dungeon* dungeon;
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
        if (thing_is_spellbook(thing))
        {
            ThingModel objkind = thing->model;
            PowerKind spl_idx = book_thing_to_power_kind(thing);
            if ((spl_idx > 0) && ((thing->alloc_flags & TAlF_IsDragged) == 0))
            {
                if (game.play_gameturn > 10) //Function is used to place books in rooms before dungeons are intialized
                {
                    dungeon = get_players_num_dungeon(room->owner);
                    if (dungeon->magic_level[spl_idx] < 2)
                    {
                        if (!store_reposition_entry(rrepos, objkind)) {
                            WARNLOG("Too many things to reposition in %s.", room_code_name(room->kind));
                        }
                    }
                    if (!is_neutral_thing(thing))
                    {
                        remove_power_from_player(spl_idx, room->owner);
                        dungeon = get_dungeon(room->owner);
                        dungeon->magic_resrchable[spl_idx] = 1;
                    }
                }
                else
                {
                    if (!store_reposition_entry(rrepos, objkind))
                    {
                        WARNLOG("Too many things to reposition in %s.", room_code_name(room->kind));
                    }
                    if (!is_neutral_thing(thing))
                    {
                        remove_power_from_player(spl_idx, room->owner);
                    }
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

TbBool recreate_repositioned_book_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    if ((rrepos->used < 0) || (room->used_capacity >= room->total_capacity)) {
        return false;
    }
    for (int ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] != 0)
        {
            struct Thing* objtng = create_spell_in_library(room, rrepos->models[ri], stl_x, stl_y);
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

TbBool move_thing_to_different_room(struct Thing* thing, struct Coord3d* pos)
{
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

int position_books_in_room_with_capacity(PlayerNumber plyr_idx, RoomKind rkind, struct RoomReposition* rrepos)
{
    struct Room* room = find_room_with_spare_room_item_capacity(plyr_idx, RoK_LIBRARY);
    struct Coord3d pos;
    unsigned long k = 0;
    int i = room->index;
    int count = 0;
    while (i != 0)
    {
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        // Per-room code
        pos.x.val = subtile_coord_center(room->central_stl_x);
        pos.y.val = subtile_coord_center(room->central_stl_y);
        pos.z.val = get_floor_height_at(&pos);

        for (int ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
        {
            if (rrepos->models[ri] != 0)
            {
                struct Thing* spelltng = create_spell_in_library(room, rrepos->models[ri], room->central_stl_x, room->central_stl_y);
                if (!thing_is_invalid(spelltng))
                {
                    if (!find_random_valid_position_for_thing_in_room_avoiding_object(spelltng, room, &pos))
                    {
                        SYNCDBG(7, "Could not find position in %s for %s artifact", room_code_name(room->kind), object_code_name(spelltng->model));
                        if (!is_neutral_thing(spelltng))
                        {
                            remove_power_from_player(book_thing_to_power_kind(spelltng), plyr_idx);
                        }
                        delete_thing_structure(spelltng, 0);
                    }
                    else
                    {
                        pos.z.val = get_thing_height_at(spelltng, &pos);
                        move_thing_in_map(spelltng, &pos);
                        create_effect(&pos, TngEff_RoomSparkeLarge, spelltng->owner);
                        rrepos->used--;
                        rrepos->models[ri] = 0;
                        count++;
                    }
                }
            }
        }
        if (rrepos->used <= 0)
        {
            SYNCDBG(7,"Nothing left to reposition")
            break;
        }
        room = find_room_with_spare_room_item_capacity(plyr_idx, RoK_LIBRARY);
        if (room_is_invalid(room))
        {
            SYNCLOG("Could not find any spare %s capacity for %d remaining books", room_code_name(RoK_LIBRARY), rrepos->used);
            i = 0;
            break;
        }
        i = room->index;
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

int check_books_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return -2; // do nothing
    if (get_map_floor_filled_subtiles(mapblk) != 1) {
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
        if (thing_is_spellbook(thing))
        {
            PowerKind spl_idx = book_thing_to_power_kind(thing);
            if ((spl_idx > 0) && ((thing->alloc_flags & TAlF_IsDragged) == 0) && ((thing->owner == room->owner) || game.play_gameturn < 10))//Function is used to integrate preplaced books at map startup too.
            {
                // If exceeded capacity of the library
                if (room->used_capacity > room->total_capacity)
                {
                    SYNCDBG(7,"Room %d type %s capacity %d exceeded; space used is %d", room->index, room_code_name(room->kind), (int)room->total_capacity, (int)room->used_capacity);
                    struct Dungeon* dungeon = get_players_num_dungeon(room->owner);
                    if (dungeon->magic_level[spl_idx] <= 1)
                    { 
                        // We have a single copy, but nowhere to place it. -1 will handle the rest.
                        return -1;
                    }
                    else // We have more than one copy, so we can just delete the book.
                    {
                        if (!is_neutral_thing(thing))
                        {
                            remove_power_from_player(spl_idx, thing->owner);
                        }
                        SYNCLOG("Deleting from %s of player %d duplicate object %s", room_code_name(room->kind), (int)thing->owner, object_code_name(thing->model));
                        delete_thing_structure(thing, 0);
                    }

                } else // we have capacity to spare, so it can stay unless it's stuck
                if (thing_in_wall_at(thing, &thing->mappos)) 
                {
                    if (position_over_floor_level(thing, &thing->mappos)) //If it's inside the floors, simply move it up and count it.
                    {
                        matching_things_at_subtile++; 
                    }
                    else
                    {
                        return -1; // If it's inside the wall or cannot be moved up, recreate all items.
                    }
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
    if (matching_things_at_subtile == 0)
    {
        if (room->used_capacity == room->total_capacity) // When 0 is returned, it would try to place books at this subtile. When at capacity already, return -2 to refuse that.
        {
            return -2;
        }
    }
    return matching_things_at_subtile; // Increase used capacity
}

void count_and_reposition_books_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    int matching_things_at_subtile = check_books_on_subtile_for_reposition_in_room(room, stl_x, stl_y);
    if (matching_things_at_subtile > 0) {
        // This subtile contains spells
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
            reposition_all_books_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        case 0:
            // There are no matching things there, something can be re-created
            recreate_repositioned_book_in_room_on_subtile(room, stl_x, stl_y, rrepos);
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
    SYNCDBG(17,"Starting for %s",room_code_name(room->kind));
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
                    count_and_reposition_books_in_room_on_subtile(room, slab_subtile(slb_x,dx), slab_subtile(slb_y,dy), &rrepos);
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
    if (rrepos.used > 0) 
    {
        int move_count = position_books_in_room_with_capacity(room->owner, room->kind, &rrepos);
        if (move_count > 0)
        {
            if (rrepos.used > 0)
            {
                SYNCLOG("The %s capacity wasn't enough, %d moved, but %d items belonging to player %d dropped",
                    room_code_name(room->kind), move_count, (int)rrepos.used, (int)room->owner);
            }
            else
            {
                SYNCDBG(7,"Moved %d items belonging to player %d to different %s",
                    move_count, (int)room->owner, room_code_name(room->kind));
            }
        }
        else
        {
            SYNCLOG("No %s capacity available to move, %d items belonging to player %d dropped",
                room_code_name(room->kind), (int)rrepos.used, (int)room->owner);
        }      
    }
    room->capacity_used_for_storage = room->used_capacity;
}

void count_workers_in_room(struct Room *room)
{
    //_DK_count_workers_in_room(room);
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

void count_slabs_div2_wth_effcncy(struct Room *room)
{
    unsigned long count = room->slabs_count * ((long)room->efficiency);
    count = ((count/ROOM_EFFICIENCY_MAX) >> 1);
    if (count <= 1)
        count = 1;
    room->total_capacity = count;
}

void count_gold_slabs_wth_effcncy(struct Room *room)
{
    //_DK_count_gold_slabs_with_efficiency(room); return;
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

TbBool rectreate_repositioned_crate_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    if ((rrepos->used < 0) || (room->used_capacity >= room->total_capacity)) {
        return false;
    }
    for (int ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] != 0)
        {
            struct Thing* objtng = create_crate_in_workshop(room, rrepos->models[ri], stl_x, stl_y);
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

int check_crates_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return -2; // do nothing
    if (get_map_floor_filled_subtiles(mapblk) != 1) {
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
        if (thing_is_workshop_crate(thing) && !thing_is_dragged_or_pulled(thing) && (thing->owner == room->owner))
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

void reposition_all_crates_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
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
        if (thing_is_workshop_crate(thing) && !thing_is_dragged_or_pulled(thing) && (thing->owner == room->owner))
        {
            ThingModel objkind = thing->model;
            ThingClass tngclass = crate_thing_to_workshop_item_class(thing);
            ThingModel tngmodel = crate_thing_to_workshop_item_model(thing);
            if (!store_reposition_entry(rrepos, objkind)) {
                WARNLOG("Too many things to reposition in %s index %d",room_code_name(room->kind),(int)room->index);
            }
            if (!is_neutral_thing(thing) && player_exists(get_player(thing->owner)))
            {
                if (remove_workshop_item_from_amount_stored(thing->owner, tngclass, tngmodel, WrkCrtF_NoOffmap) > WrkCrtS_None) {
                    remove_workshop_item_from_amount_placeable(thing->owner, tngclass, tngmodel);
                }
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

void count_and_reposition_crates_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    int matching_things_at_subtile = check_crates_on_subtile_for_reposition_in_room(room, stl_x, stl_y);
    if (matching_things_at_subtile > 0) {
        // This subtile contains matching things
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
            reposition_all_crates_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        case 0:
            // There are no matching things there, something can be re-created
            rectreate_repositioned_crate_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        default:
            WARNLOG("Invalid value returned by reposition check");
            break;
        }
    }
}

/**
 * Updates count of crates (used capacity) in a workshop.
 * Also repositions crates which are in solid columns.
 * @param room The room to be recomputed and repositioned.
 */
void count_crates_in_room(struct Room *room)
{
    SYNCDBG(17,"Starting for %s",room_code_name(room->kind));
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
                    count_and_reposition_crates_in_room_on_subtile(room, STL_PER_SLB*slb_x+dx, STL_PER_SLB*slb_y+dy, &rrepos);
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
            if (!store_creature_reposition_entry(rrepos, crkind, cctrl->explevel)) {
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
            struct Thing* bodytng = create_dead_creature(&pos, rrepos->models[ri], 0, room->owner, rrepos->explevels[ri]);
            if (!thing_is_invalid(bodytng))
            {
                bodytng->corpse.laid_to_rest = 1;
                bodytng->health = game.graveyard_convert_time;
                rrepos->used--;
                rrepos->models[ri] = 0;
                rrepos->explevels[ri] = 0;
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
    if (get_map_floor_filled_subtiles(mapblk) != 1) {
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
    //_DK_count_bodies_in_room(room);
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

TbBool rectreate_repositioned_food_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
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
                rrepos->explevels[ri] = 0;
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
    if (get_map_floor_filled_subtiles(mapblk) != 0) { // Floor level for hatchery is 0
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
            rectreate_repositioned_food_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        default:
            WARNLOG("Invalid value returned by reposition check");
            break;
        }
    }
}

void count_food_in_room(struct Room *room)
{
    //_DK_count_food_in_room(room);
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

void count_lair_occupants_on_slab(struct Room *room,MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SYNCDBG(17,"Starting for %s index %d at %d,%d",room_code_name(room->kind),(int)room->index,(int)slb_x,(int)slb_y);
    for (int n = 0; n < MID_AROUND_LENGTH; n++)
    {
        MapSubtlDelta ssub_x = 1 + start_at_around[n].delta_x;
        MapSubtlDelta ssub_y = 1 + start_at_around[n].delta_y;
        struct Thing* lairtng = find_lair_totem_at(slab_subtile(slb_x, ssub_x), slab_subtile(slb_y, ssub_y));
        if (!thing_is_invalid(lairtng))
        {
            struct Thing* creatng = thing_get(lairtng->belongs_to);
            int required_cap = get_required_room_capacity_for_object(RoRoF_LairStorage, 0, creatng->model);
            if (room->used_capacity + required_cap > room->total_capacity)
            {
                create_effect(&lairtng->mappos, imp_spangle_effects[lairtng->owner], lairtng->owner);
                delete_lair_totem(lairtng);
            } else
            {
                struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
                cctrl->lair_room_id = room->index;
                room->content_per_model[creatng->model]++;
                room->used_capacity += required_cap;
            }
        }
    }
}

void count_lair_occupants(struct Room *room)
{
    //_DK_count_lair_occupants(room);
    room->used_capacity = 0;
    memset(room->content_per_model, 0, sizeof(room->content_per_model));
    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        MapSubtlCoord slb_x = slb_num_decode_x(i);
        MapSubtlCoord slb_y = slb_num_decode_y(i);
        struct SlabMap* slb = get_slabmap_direct(i);
        if (slabmap_block_invalid(slb))
        {
            ERRORLOG("Jump to invalid room slab detected");
            break;
        }
        i = get_next_slab_number_in_room(i);
        // Per slab code
        count_lair_occupants_on_slab(room, slb_x, slb_y);
        // Per slab code ends
        k++;
        if (k > map_tiles_x * map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
}


void delete_room_structure(struct Room *room)
{
    if (room_is_invalid(room))
    {
        WARNLOG("Attempt to delete invalid room");
        return;
    }
    if ((room->alloc_flags & 0x01) != 0)
    {
      // This is almost remove_room_from_players_list(room, room->owner);
      // but it doesn't change room_slabs_count and is less careful - better not use too much
      if (room->owner != game.neutral_player_num)
      {
          struct Dungeon* dungeon = get_players_num_dungeon(room->owner);
          unsigned short* wptr = &dungeon->room_kind[room->kind];
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
      LbMemorySet(room, 0, sizeof(struct Room));
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
    struct RoomData* rdata = room_data_get_for_room(room);
    Room_Update_Func cb = rdata->update_total_capacity;
    if (cb != NULL) {
        cb(room);
    }
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
        if (k >= map_tiles_x*map_tiles_y)
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
void get_room_mass_centre_coords(long *mass_x, long *mass_y, const struct Room *room)
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
        *mass_x = map_tiles_x / 2;
        *mass_y = map_tiles_y / 2;
    }
}


void update_room_central_tile_position(struct Room *room)
{
    long mass_x;
    long mass_y;
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

TbBool add_room_to_players_list(struct Room *room, PlayerNumber plyr_idx)
{
    if (plyr_idx == game.neutral_player_num) {
        return false;
    }
    if (room->kind >= ROOM_TYPES_COUNT) {
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
    long nxroom_id = dungeon->room_kind[room->kind];
    struct Room* nxroom = room_get(nxroom_id);
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

TbBool remove_room_from_players_list(struct Room *room, PlayerNumber plyr_idx)
{
    if (plyr_idx == game.neutral_player_num) {
        return false;
    }
    if (room->kind >= ROOM_TYPES_COUNT) {
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
void add_slab_list_to_room_tiles_list(struct Room *room, SlabCodedCoords slb_num)
{
    if (room->slabs_list == 0) {
        room->slabs_list = slb_num;
    } else {
        struct SlabMap* pvslb = get_slabmap_direct(room->slabs_list_tail);
        pvslb->next_in_room = slb_num;
    }
    SlabCodedCoords tail_slb_num = slb_num;
    while (1)
    {
        struct SlabMap* nxslb = get_slabmap_direct(tail_slb_num);
        nxslb->room_index = room->index;
        room->slabs_count++;
        if (nxslb->next_in_room == 0) {
            break;
        }
        tail_slb_num = nxslb->next_in_room;
    }
    room->slabs_list_tail = tail_slb_num;
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
    WARNLOG("Slab %ld couldn't be found in room tiles list.",slb_num);
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
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    add_slab_to_room_tiles_list(room, slb_x, slb_y);
    return room;
}

struct Room *create_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    SYNCDBG(7,"Starting to make %s at (%d,%d)",room_code_name(rkind),(int)stl_x,(int)stl_y);
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
    SYNCDBG(7,"Starting for %s at (%d,%d)",room_code_name(room->kind),(int)stl_x,(int)stl_y);
    if (room_can_have_ensign(room->kind))
    {
        struct Coord3d pos;
        pos.z.val = subtile_coord(2, 0);
        pos.x.val = subtile_coord(stl_x,0);
        pos.y.val = subtile_coord(stl_y,0);
        struct Thing* thing = find_base_thing_on_mapwho(TCls_Object, 25, stl_x, stl_y);
        if (thing_is_invalid(thing))
        {
            thing = create_object(&pos, 25, room->owner, -1);
        }
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Cannot create room flag");
            return;
        }
        thing->belongs_to = room->index;
    }
}

void delete_room_flag(struct Room *room)
{
    MapSubtlCoord stl_x = slab_subtile_center(slb_num_decode_x(room->slabs_list));
    MapSubtlCoord stl_y = slab_subtile_center(slb_num_decode_y(room->slabs_list));
    if (room_can_have_ensign(room->kind))
    {
        struct Thing* thing = find_base_thing_on_mapwho(TCls_Object, 25, stl_x, stl_y);
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
        if ((room->alloc_flags & 0x01) == 0)
        {
            LbMemorySet(room, 0, sizeof(struct Room));
            room->alloc_flags |= 0x01;
            room->index = i;
            return room;
        }
    }
    return INVALID_ROOM;
}

unsigned short i_can_allocate_free_room_structure(void)
{
  unsigned short ret = _DK_i_can_allocate_free_room_structure();
  if (ret == 0)
  {
      SYNCDBG(3,"No slot for next room");
  }
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
    unsigned int k = 0;
    for (unsigned int n = 0; n < DUNGEONS_COUNT; n++)
    {
        struct Dungeon* dungeon = get_dungeon(n);
        unsigned int i = dungeon->room_kind[rkind];
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
            set_room_stats(room, skip_integration);
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
    for (RoomKind rkind = 1; rkind < ROOM_TYPES_COUNT; rkind++)
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
    SYNCDBG(7,"Starting");
    for (unsigned long slb_y = 0; slb_y < map_tiles_y; slb_y++)
    {
        for (unsigned long slb_x = 0; slb_x < map_tiles_x; slb_x++)
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
                set_room_stats(room, false);
            }
        }
    }
    return true;
}

TbBool room_create_new_food_at(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    struct Thing* foodtng = create_object(&pos, 9, room->owner, -1);
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
    foodtng->belongs_to = (foodtng->field_49 << 8) / foodtng->anim_speed - 1;
    return true;
}

short room_grow_food(struct Room *room)
{
    //return _DK_room_grow_food(room);
    if (room->slabs_count < 1) {
        ERRORLOG("Room %s index %d has no slabs",room_code_name(room->kind),(int)room->index);
        return 0;
    }
    if ((room->used_capacity >= room->total_capacity)
      || game.play_gameturn % ((game.food_generation_speed / room->total_capacity) + 1)) {
        return 0;
    }
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
            struct Thing* thing = find_base_thing_on_mapwho(TCls_Object, 9, stl_x, stl_y);
            if (thing_is_invalid(thing)) {
                thing = find_base_thing_on_mapwho(TCls_Object, 4, stl_x, stl_y);
            }
            if (thing_is_invalid(thing))
            {
                if (get_floor_filled_subtiles_at(stl_x, stl_y) == 0)
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
    long nslabs = room->slabs_count;
    long i = nslabs;
    if (i >= sizeof(slabs_to_centre_peices)/sizeof(slabs_to_centre_peices[0]))
        i = sizeof(slabs_to_centre_peices)/sizeof(slabs_to_centre_peices[0]) - 1;
    long npieces = slabs_to_centre_peices[i];
    return 2 * (npieces + 4 * nslabs);
}

/** Calculates summary of surrounding-based efficiency score from all slabs in room.
 *
 * @param room Source room.
 * @return The efficiency score summary.
 */
long calculate_cummulative_room_slabs_effeciency(const struct Room *room)
{
    long score = 0;
    unsigned long k = 0;
    long i = room->slabs_list;
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
    return effic;
}

/**
 * Computes max health of a room of given size.
 */
unsigned long compute_room_max_health(unsigned short slabs_count,unsigned short efficiency)
{
  unsigned long max_health = game.hits_per_slab * slabs_count;
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
    int newhealth = linkroom->health + oldroom->health;
    int maxhealth = compute_room_max_health(linkroom->slabs_count, linkroom->efficiency);

    if ((newhealth > maxhealth) || (newhealth <= 0))
    {
        newhealth = maxhealth;
    }
    linkroom->health = newhealth;
    return false;
}

TbBool recalculate_room_health(struct Room* room)
{
    SYNCDBG(17, "Starting for %s index %d", room_code_name(room->kind), (int)room->index);
    int newhealth = (room->health + game.hits_per_slab);
    int maxhealth = compute_room_max_health(room->slabs_count, room->efficiency);
    
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
    struct RoomData* rdata = room_data_get_for_room(room);
    SYNCDBG(17,"Starting for %s index %d",room_code_name(room->kind),(int)room->index);
    Room_Update_Func cb = rdata->update_storage_in_room;
    if (cb != NULL) {
        cb(room);
    }
    cb = rdata->update_workers_in_room;
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
    long n;
    // Central slab coords - we will need it if we'll find adjacent room
    MapSlabCoord central_slb_x = subtile_slab_fast(x);
    MapSlabCoord central_slb_y = subtile_slab_fast(y);
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
                    add_slab_list_to_room_tiles_list(linkroom, room->slabs_list);
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
            room->field_43 = 1;
            room->flame_stl = 0;
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
    if (room->kind != get_room_for_job(jobpref)) {
        return false;
    }
    int required_cap = get_required_room_capacity_for_job(jobpref, creatng->model);
    if (room->used_capacity + required_cap <= room->total_capacity)
        return true;
    return false;
}

TbBool find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    //return _DK_find_random_valid_position_for_thing_in_room(thing, room, pos);
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
    long n = CREATURE_RANDOM(thing, room->slabs_count);
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
        int ssub = CREATURE_RANDOM(thing, 9);
        for (int snum = 0; snum < 9; snum++)
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
            ssub = (ssub + 1) % 9;
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
    long i = CREATURE_RANDOM(thing, room->slabs_count);
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
                pos->x.val = subtile_coord(slab_subtile(slb_x,0),CREATURE_RANDOM(thing, STL_PER_SLB*COORD_PER_STL));
                pos->y.val = subtile_coord(slab_subtile(slb_y,0),CREATURE_RANDOM(thing, STL_PER_SLB*COORD_PER_STL));
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
struct Room *find_room_with_spare_room_item_capacity(PlayerNumber plyr_idx, RoomKind rkind)
{
    if (rkind >= ROOM_TYPES_COUNT)
        return INVALID_ROOM;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return INVALID_ROOM;
    return find_nth_room_of_owner_with_spare_item_capacity_starting_with(dungeon->room_kind[rkind], 0, 1);
}

struct Room *find_room_for_thing_with_used_capacity(const struct Thing *creatng, PlayerNumber plyr_idx, RoomKind rkind, unsigned char nav_flags, long min_used_cap)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long i = dungeon->room_kind[rkind];
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
struct Room *find_room_with_spare_capacity(PlayerNumber owner, RoomKind rkind, long spare)
{
    if (rkind >= ROOM_TYPES_COUNT)
        return INVALID_ROOM;
    struct Dungeon* dungeon = get_dungeon(owner);
    if (dungeon_invalid(dungeon))
        return INVALID_ROOM;
    return find_nth_room_of_owner_with_spare_capacity_starting_with(dungeon->room_kind[rkind], 0, spare);
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

struct Room *find_room_with_most_spare_capacity_starting_with(long room_idx, long *total_spare_cap)
{
    SYNCDBG(18,"Starting");
    long loc_total_spare_cap = 0;
    struct Room* max_spare_room = INVALID_ROOM;
    long max_spare_cap = 0;
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
        pos->x.val = subtile_coord_center(map_subtiles_x/2);
        pos->y.val = subtile_coord_center(map_subtiles_y/2);
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
    pos->x.val = subtile_coord_center(map_subtiles_x/2);
    pos->y.val = subtile_coord_center(map_subtiles_y/2);
    pos->z.val = subtile_coord(1,0);
    return false;
}

struct Room *find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing, signed char owner, RoomKind rkind, unsigned char nav_flags, long spare)
{
    SYNCDBG(18,"Searching for %s with capacity for %s index %d",room_code_name(rkind),thing_model_name(thing),(int)thing->index);
    struct Dungeon* dungeon = get_dungeon(owner);
    struct Room* nearoom = INVALID_ROOM;
    long neardistance = LONG_MAX;
    unsigned long k = 0;
    int i = dungeon->room_kind[rkind];
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
        long distance = abs(thing->mappos.x.stl.num - room->central_stl_x) + abs(thing->mappos.y.stl.num - room->central_stl_y);
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
long count_rooms_with_used_capacity_creature_can_navigate_to(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = get_dungeon(owner);
    long count = 0;
    unsigned long k = 0;
    int i = dungeon->room_kind[rkind];
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
struct Room *find_nth_room_with_used_capacity_creature_can_navigate_to(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags, long n)
{
    struct Dungeon* dungeon = get_dungeon(owner);
    unsigned long k = 0;
    int i = dungeon->room_kind[rkind];
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
    for (RoomKind rkind = 1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        struct Room* room = find_nth_room_with_used_capacity_creature_can_navigate_to(thing, owner, rkind, NavRtF_Default, 0);
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
struct Room *find_random_room_with_used_capacity_creature_can_navigate_to(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    long count = count_rooms_with_used_capacity_creature_can_navigate_to(thing, owner, rkind, nav_flags);
    if (count < 1)
        return INVALID_ROOM;
    long selected = CREATURE_RANDOM(thing, count);
    return find_nth_room_with_used_capacity_creature_can_navigate_to(thing, owner, rkind, nav_flags, selected);
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
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long near_distance = LONG_MAX;
    struct Room* near_room = INVALID_ROOM;
    long i = dungeon->room_kind[rkind];
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
    long retdist = LONG_MAX;
    struct Room* retroom = INVALID_ROOM;
    for (RoomKind rkind = 0; rkind < slab_conf.room_types_count; rkind++)
    {
        if (!room_role_matches(rkind, rrole)) {
            continue;
        }
        long i = dungeon->room_kind[rkind];
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
                long dist = abs(thing->mappos.y.stl.num - (int)room->central_stl_y);
                dist += abs(thing->mappos.x.stl.num - (int)room->central_stl_x);
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
    int i = dungeon->room_kind[rkind];
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
 * Gives the n-th room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_flags
 * @param n
 * @return
 */
struct Room *find_nth_room_for_thing(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags, long n)
{
    struct Dungeon* dungeon = get_dungeon(owner);
    unsigned long k = 0;
    int i = dungeon->room_kind[rkind];
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
    return INVALID_ROOM;
}

struct Room *find_random_room_for_thing(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags)
{
    //return _DK_find_random_room_for_thing(thing, plyr_idx, rkind, a4);
    SYNCDBG(18,"Starting");
    long count = count_rooms_for_thing(thing, owner, rkind, nav_flags);
    if (count < 1)
        return INVALID_ROOM;
    long selected = CREATURE_RANDOM(thing, count);
    return find_nth_room_for_thing(thing, owner, rkind, nav_flags, selected);
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
long count_rooms_for_thing_with_spare_room_item_capacity(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = get_dungeon(owner);
    long count = 0;
    unsigned long k = 0;
    int i = dungeon->room_kind[rkind];
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
struct Room *find_nth_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags, long n)
{
    struct Dungeon* dungeon = get_dungeon(owner);
    unsigned long k = 0;
    int i = dungeon->room_kind[rkind];
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
    return INVALID_ROOM;
}

struct Room * find_random_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags)
{
    //return _DK_find_random_room_for_thing_with_spare_room_item_capacity(thing, owner, rkind, nav_flags);
    SYNCDBG(18,"Starting");
    long count = count_rooms_for_thing_with_spare_room_item_capacity(thing, owner, rkind, nav_flags);
    if (count < 1)
        return INVALID_ROOM;
    long selected = CREATURE_RANDOM(thing, count);
    return find_nth_room_for_thing_with_spare_room_item_capacity(thing, owner, rkind, nav_flags, selected);
}

short delete_room_slab_when_no_free_room_structures(long a1, long a2, unsigned char a3)
{
    SYNCDBG(8,"Starting");
    return _DK_delete_room_slab_when_no_free_room_structures(a1, a2, a3);
}

TbBool find_random_valid_position_for_thing_in_room_avoiding_object_excluding_room_slab(struct Thing *thing, struct Room *room, struct Coord3d *pos, long slbnum)
{
    // return _DK_find_random_valid_position_for_thing_in_room_avoiding_object_excluding_room_slab(thing, room, pos, a4);
    int nav_sizexy = subtile_coord(thing_nav_block_sizexy(thing), 0);
    if (room_is_invalid(room) || (room->slabs_count <= 0)) {
        ERRORLOG("Invalid room or number of slabs is zero");
        return false;
    }
    long selected = CREATURE_RANDOM(thing, room->slabs_count);  
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
            WARNLOG("Number of slabs in %s (%d) is smaller than count (%d)",room_code_name(room->kind), n, room->slabs_count);
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
            MapSubtlCoord start_stl = CREATURE_RANDOM(thing, AROUND_TILES_COUNT);
            for (nround = 0; nround < AROUND_TILES_COUNT; nround++)
            {
                MapSubtlCoord x = start_stl % 3 + stl_x;
                MapSubtlCoord y = start_stl / 3 + stl_y;
                if (get_floor_filled_subtiles_at(x, y) == 1)
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
                    if (get_floor_filled_subtiles_at(astl_x, astl_y) == 1)
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
    //return _DK_find_random_valid_position_for_item_in_different_room_avoiding_object(thing, room, pos);
    struct Dungeon* dungeon = get_players_num_dungeon(skip_room->owner);
    unsigned int matching_rooms = 0;
    long i;
    unsigned long k = 0;
    struct Room* room;
    for (i = dungeon->room_kind[skip_room->kind]; (i != 0); i = room->next_of_owner)
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
    int chosen_match_idx = CREATURE_RANDOM(thing, matching_rooms);
    int curr_match_idx = 0;
    k = 0;
    for (i = dungeon->room_kind[skip_room->kind]; (i != 0); i = room->next_of_owner)
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

struct Thing *find_lair_totem_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    //return _DK_find_lair_at(stl_x, stl_y);
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
        if (thing_is_lair_totem(thing)) {
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

void kill_room_contents_at_subtile(struct Room *room, PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, SlabCodedCoords slbnum)
{
    struct Thing *thing;
    struct Dungeon* dungeon;
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
            if (room->owner == thing->owner)
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
    switch (room->kind)
    {
    case RoK_TREASURE:
        if (get_map_floor_filled_subtiles(mapblk) == 1)
        {
            struct Thing *gldtng;
            gldtng = find_gold_hoard_at(stl_x, stl_y);
            if (!thing_is_invalid(gldtng))
            {
                room->capacity_used_for_storage -= gldtng->valuable.gold_stored;
                dungeon = get_dungeon(plyr_idx);
                if (!dungeon_invalid(dungeon)) {
                    dungeon->total_money_owned -= gldtng->valuable.gold_stored;
                }
                drop_gold_pile(gldtng->valuable.gold_stored, &gldtng->mappos);
                delete_thing_structure(gldtng, 0);
            }
        }
        break;
    case RoK_LIBRARY:
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
        break;
    case RoK_WORKSHOP:
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
        break;
    case RoK_GARDEN:
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
        break;
    case RoK_LAIR:
        thing = find_lair_totem_at(stl_x, stl_y);
        if (!thing_is_invalid(thing))
        {
            if (thing->belongs_to)
            {
                struct Thing *tmptng;
                struct CreatureControl *cctrl;
                tmptng = thing_get(thing->belongs_to);
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
        break;
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
  _DK_free_room_structure(room);
}

void reset_creatures_rooms(struct Room *room)
{
    SYNCDBG(18,"Starting");
    //_DK_reset_creatures_rooms(room);
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
                cctrl->flgfield_1 &= ~CCFlg_IsInRoomList;
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
    if (room->kind == RoK_BRIDGE)
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
    game.field_14EA4B = 1;
    if (subtile_coords_invalid(stl_x, stl_y))
        return INVALID_ROOM;
    long slb_x = subtile_slab_fast(stl_x);
    long slb_y = subtile_slab_fast(stl_y);
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
    struct RoomData* rdata = room_data_get_for_room(room);
    long i = get_slab_number(slb_x, slb_y);
    delete_room_slabbed_objects(i);
    if ((rkind == RoK_GUARDPOST) || (rkind == RoK_BRIDGE))
    {
        place_animating_slab_type_on_map(rdata->assigned_slab, 0, stl_x, stl_y, owner);
    } else
    {
        place_slab_type_on_map(rdata->assigned_slab, stl_x, stl_y, owner, 0);
    }
    SYNCDBG(7,"Updating efficiency");
    do_slab_efficiency_alteration(slb_x, slb_y);
    do_room_recalculation(room);
    if (owner != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_dungeon(owner);
        dungeon->lvstats.rooms_constructed++;
    }
    pannel_map_update(stl_x, stl_y, STL_PER_SLB, STL_PER_SLB);
    return room;
}

struct Room *find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, PlayerNumber plyr_idx, RoomKind rkind, unsigned char nav_flags)
{
    long retdist = LONG_MAX;
    struct Room* retroom = INVALID_ROOM;

    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long i = dungeon->room_kind[rkind];
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
        long dist = abs(thing->mappos.x.stl.num - room->central_stl_x) + abs(thing->mappos.y.stl.num - room->central_stl_y);
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
    return retroom;
}

struct Room * pick_random_room(PlayerNumber plyr_idx, RoomKind rkind)
{
    return _DK_pick_random_room(plyr_idx, rkind);
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
    struct Objects* objdat = get_objects_data_for_thing(thing);
    // If thing is only dragged through the room, do not interrupt
    if (thing_is_dragged_or_pulled(thing)) {
        return;
    }
    // Handle specific things in rooms for which we have a special re-creation code
    PlayerNumber oldowner;
    switch (room->kind)
    {
    case RoK_GUARDPOST:
        // Guard post owns only flag, and it must be re-created
        if (object_is_guard_flag(thing))
        {
            create_guard_flag_object(&thing->mappos, newowner, parent_idx);
            delete_thing_structure(thing, 0);
            return;
        }
        break;
    case RoK_LIBRARY:
        // Library owns books, specials and candles; only spellbooks require additional code
        if (thing_is_spellbook(thing))
        {
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
        break;
    case RoK_WORKSHOP:
        // Workshop owns trap boxes, machines and anvils; special code for boxes only
        if (thing_is_workshop_crate(thing))
        {
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
    case RoK_GARDEN:
        if (object_is_infant_food(thing) || object_is_growing_food(thing) || object_is_mature_food(thing))
        {
            thing->parent_idx = -1; // All chickens escape
        }
        break;
    case RoK_LAIR:
        // Lair - owns creature lairs
        if (objdat->related_creatr_model)
        {
            if (thing->belongs_to)
            {
                struct Thing *tmptng;
                struct CreatureControl *cctrl;
                tmptng = thing_get(thing->belongs_to);
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
    // If the object is marked to be destroyed, do it
    if ((objst->model_flags & OMF_DestroyedOnRoomClaim) != 0) {
        destroy_object(thing);
        return;
    }
    if ((gameadd.classic_bugs_flags & ClscBug_ClaimRoomAllThings) != 0) {
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
    //_DK_delete_room_slabbed_objects(slb_num);

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
    long parent_idx = get_slab_number(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
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
        struct SlabMap* slb = get_slabmap_direct(i);
        MapSlabCoord slb_x = slb_num_decode_x(i);
        MapSlabCoord slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code
        set_slab_explored(plyr_idx, slb_x, slb_y);
        slabmap_set_owner(slb, plyr_idx);
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
        pannel_map_update(start_stl_x, start_stl_y, STL_PER_SLB, STL_PER_SLB);
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
        if (k > map_tiles_x*map_tiles_y)
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

    if (oldowner == game.neutral_player_num)
    {
        room->owner = newowner;
        room->health = compute_room_max_health(room->slabs_count, room->efficiency);
        add_room_to_players_list(room, newowner);
        change_room_map_element_ownership(room, newowner);
        redraw_room_map_elements(room);
        do_room_unprettying(room, newowner);
        do_room_integration(room);
        return 1;
    } else
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
void destroy_room_leaving_unclaimed_ground(struct Room *room)
{
    unsigned long k = 0;
    long i = room->slabs_list;
    while (i != 0)
    {
        long slb_x = slb_num_decode_x(i);
        long slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        if (room->owner != game.neutral_player_num)
        {
            struct Dungeon* dungeon = get_players_num_dungeon(room->owner);
            dungeon->rooms_destroyed++;
        }
        delete_room_slab(slb_x, slb_y, 1); // Note that this function might also delete the whole room
        create_dirt_rubble_for_dug_slab(slb_x, slb_y);
        // Per room tile code ends
        k++;
        if (k > map_tiles_x*map_tiles_y) // we can't use room->slabs_count as room may be deleted
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
}

void destroy_dungeon_heart_room(PlayerNumber plyr_idx, const struct Thing *heartng)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    struct Room* room = get_room_thing_is_on(heartng);
    if (room_is_invalid(room) || (room->kind != RoK_DUNGHEART))
    {
        WARNLOG("The heart thing is not in heart room");
        long i = dungeon->room_kind[RoK_DUNGHEART];
        room = room_get(i);
    }
    if (room_is_invalid(room))
    {
        ERRORLOG("Tried to destroy heart for player who doesn't have one");
        return;
    }
    remove_room_from_players_list(room, plyr_idx);
    destroy_room_leaving_unclaimed_ground(room);
}
/******************************************************************************/
