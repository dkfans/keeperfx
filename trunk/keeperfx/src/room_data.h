/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_data.h
 *     Header file for room_data.c.
 * @par Purpose:
 *     Rooms support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Apr 2009 - 14 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOMDATA_H
#define DK_ROOMDATA_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ROOM_TYPES_COUNT      17
#define SLAB_AROUND_COUNT      4
#define ROOMS_COUNT          150
/******************************************************************************/
enum RoomFlags {
    RoF_Allocated           = 0x01,
};

enum RoomKinds {
    RoK_NONE                =   0,
    RoK_ENTRANCE            =   1,
    RoK_TREASURE            =   2,
    RoK_LIBRARY             =   3,
    RoK_PRISON              =   4,
    RoK_TORTURE             =   5,
    RoK_TRAINING            =   6,
    RoK_DUNGHEART           =   7,
    RoK_WORKSHOP            =   8,
    RoK_SCAVENGER           =   9,
    RoK_TEMPLE              =  10,
    RoK_GRAVEYARD           =  11,
    RoK_BARRACKS            =  12,
    RoK_GARDEN              =  13,
    RoK_LAIR                =  14,
    RoK_BRIDGE              =  15,
    RoK_GUARDPOST           =  16,
    RoK_UNKN17              =  17,
};

#define ROOM_EFFICIENCY_MAX 256
#define ROOM_SELL_REVENUE_PERCENT  50
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Coord3d;
struct Room;
struct Dungeon;

typedef void (*Room_Update_Func)(struct Room *);

struct RoomInfo { // sizeof = 6
  unsigned short field_0;
  unsigned short field_2;
  unsigned short ambient_snd_smp_id;
};

struct Room {
    unsigned char alloc_flags;
    unsigned short index; // index in the rooms array
    unsigned char owner;
    short prev_of_owner;
    short next_of_owner;
    unsigned char central_stl_x;
    unsigned char central_stl_y;
    unsigned short kind;
    unsigned short health;
    short total_capacity;
    unsigned short used_capacity;
    /* Informs whether players are interested in that room.
     * Usually used for neutral rooms, set if a player is starting to dig to that room. */
    unsigned char player_interested[5];
    union {
    /** For rooms which can store things, amount of storage space, or sum of gold, used by them.
     *  Rooms which can store things are workshops, libraries, treasure rooms etc. */
    struct {
      long capacity_used_for_storage;
      short hatchfield_1B;
      unsigned char field_1D[26];
    };
    /** For rooms which are often browsed for various reasons, list of all rooms of given kind.
     *  Rooms which have such list are entrances (only?). */
    struct {
      short prev_of_kind;
      short next_of_kind;
      unsigned char field_1Bx[28];
    };
    struct {
      /** For rooms which store creatures, amount of each model.
       * Rooms which have such lists are lairs. */
      unsigned char content_per_model[32];
    };
    /* For hatchery; integrate with something else, if possible */
    struct {
      long hatch_gameturn;
      unsigned char field_1Bh[28];
    };
    };
    unsigned short slabs_list;
    unsigned short slabs_list_tail;
    unsigned short slabs_count;
    unsigned short creatures_list;
    unsigned short efficiency;
    unsigned short field_41;
    unsigned char field_43;
    unsigned char flame_stl;
};

struct RoomData {
      unsigned char assigned_slab;
      short medsym_sprite_idx;
      Room_Update_Func update_total_capacity;
      Room_Update_Func update_storage_in_room;
      Room_Update_Func update_workers_in_room;
      unsigned char field_F; //< Originally was long, but the only used values are 1 and 0
      unsigned char field_10; //< Unused, originally was part of field_F
      short field_11; //< Unused, originally was part of field_F
      short name_stridx;
      short iTooltipString;
};

#define INVALID_ROOM (&game.rooms[0])
#define INVALID_ROOM_DATA (&room_data[0])

struct RoomStats {
  short cost;
  unsigned short health;
};

/** Max. amount of items to be repositioned in a room */
#define ROOM_REPOSITION_COUNT 16

/**
 * Structure used for repositioning things in rooms so that they're not placed in solid columns.
 */
struct RoomReposition {
    int used;
    ThingModel models[ROOM_REPOSITION_COUNT];
    CrtrExpLevel explevels[ROOM_REPOSITION_COUNT];
};

#pragma pack()
/******************************************************************************/
DLLIMPORT extern struct RoomData _DK_room_data[];
//#define room_data _DK_room_data
/******************************************************************************/
extern unsigned short const room_effect_elements[];
extern struct AroundLByte const room_spark_offset[];
extern struct RoomData room_data[];
extern RoomKind look_through_rooms[18];
/******************************************************************************/
struct Room *room_get(long room_idx);
struct Room *subtile_room_get(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Room *slab_room_get(long slb_x, long slb_y);
TbBool room_is_invalid(const struct Room *room);
TbBool room_exists(const struct Room *room);
struct RoomData *room_data_get_for_kind(RoomKind rkind);
struct RoomData *room_data_get_for_room(const struct Room *room);
struct RoomStats *room_stats_get_for_kind(RoomKind rkind);
struct RoomStats *room_stats_get_for_room(const struct Room *room);

long get_room_look_through(RoomKind rkind);
long compute_room_max_health(long slabs_count,unsigned short efficiency);
void set_room_efficiency(struct Room *room);
void set_room_capacity(struct Room *room, TbBool skip_integration);
long get_room_slabs_count(PlayerNumber plyr_idx, RoomKind rkind);
long get_room_kind_used_capacity_fraction(PlayerNumber plyr_idx, RoomKind room_kind);
void get_room_kind_total_and_used_capacity(struct Dungeon *dungeon, RoomKind room_kind, long *total_cap, long *used_cap);
TbBool thing_is_on_any_room_tile(const struct Thing *thing);
TbBool thing_is_on_own_room_tile(const struct Thing *thing);
struct Room *get_room_thing_is_on(const struct Thing *thing);
void reinitialise_map_rooms(void);

// Finding position within room
TbBool find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
TbBool find_first_valid_position_for_thing_anywhere_in_room(const struct Thing *thing, struct Room *room, struct Coord3d *pos);
TbBool find_random_position_at_border_of_room(struct Coord3d *pos, const struct Room *room);

// Finding a room for a thing
TbBool creature_can_get_to_any_of_players_rooms(struct Thing *thing, PlayerNumber owner);
struct Room *find_room_with_spare_room_item_capacity(PlayerNumber plyr_idx, RoomKind rkind);
struct Room *find_nth_room_of_owner_with_spare_item_capacity_starting_with(long room_idx, long n, long spare);
struct Room *find_room_with_spare_capacity(PlayerNumber owner, signed char rkind, long spare);
struct Room *find_nth_room_of_owner_with_spare_capacity_starting_with(long room_idx, long n, long spare);
struct Room *find_room_with_most_spare_capacity_starting_with(long room_idx, long *total_spare_cap);
struct Room *find_room_nearest_to_position(PlayerNumber plyr_idx, RoomKind rkind, const struct Coord3d *pos, long *room_distance);
// Finding a navigable room for a thing
struct Room *find_room_for_thing_with_used_capacity(const struct Thing *creatng, PlayerNumber plyr_idx, RoomKind rkind, unsigned char nav_flags, long min_used_cap);
struct Room *find_random_room_with_used_capacity_creature_can_navigate_to(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags);
struct Room *find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing, signed char owner, RoomKind rkind, unsigned char nav_flags, long spare);

void create_room_flag(struct Room *room);
void delete_room_flag(struct Room *room);
struct Room *allocate_free_room_structure(void);
unsigned short i_can_allocate_free_room_structure(void);
void add_slab_to_room_tiles_list(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y);
void remove_slab_from_room_tiles_list(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y);
void add_slab_list_to_room_tiles_list(struct Room *room, SlabCodedCoords slb_num);
void delete_all_room_structures(void);
void delete_room_structure(struct Room *room);
void delete_room_slabbed_objects(SlabCodedCoords slb_num);
struct Room *link_adjacent_rooms_of_type(PlayerNumber owner, MapSubtlCoord x, MapSubtlCoord y, RoomKind rkind);
struct Room *create_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
short room_grow_food(struct Room *room);
void update_room_efficiency(struct Room *room);
struct Room *get_room_of_given_kind_for_thing(struct Thing *thing, struct Dungeon *dungeon, RoomKind rkind);
struct Thing *find_lair_totem_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Room *place_room(PlayerNumber owner, RoomKind rkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool slab_is_area_outer_border(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool slab_is_area_inner_fill(MapSlabCoord slb_x, MapSlabCoord slb_y);
MapCoordDelta get_distance_to_room(const struct Coord3d *pos, const struct Room *room);

TbBool initialise_map_rooms(void);
void init_room_sparks(struct Room *room);
void replace_room_slab(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char owner, unsigned char a5);
short delete_room_slab_when_no_free_room_structures(long a1, long a2, unsigned char a3);
long calculate_room_efficiency(const struct Room *room);
void kill_room_slab_and_contents(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
void free_room_structure(struct Room *room);
void reset_creatures_rooms(struct Room *room);

TbBool remove_item_from_room_capacity(struct Room *room);
TbBool add_item_to_room_capacity(struct Room *room, TbBool force);
TbBool room_has_enough_free_capacity_for_creature(const struct Room *room, const struct Thing *creatng);

long count_slabs_of_room_type(PlayerNumber plyr_idx, RoomKind rkind);
long claim_enemy_room(struct Room *room,struct Thing *claimtng);
long claim_room(struct Room *room,struct Thing *claimtng);
TbBool create_effects_on_room_slabs(struct Room *room, ThingModel effkind, long effrange, PlayerNumber effowner);
TbBool clear_dig_on_room_slabs(struct Room *room, PlayerNumber plyr_idx);
void do_room_integration(struct Room *room);
void destroy_dungeon_heart_room(PlayerNumber plyr_idx, const struct Thing *heartng);

/* MOVE TO room_list.c/h */
struct Room *find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, PlayerNumber plyr_idx, RoomKind rkind, unsigned char nav_flags);
struct Room *find_random_room_for_thing(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags);
struct Room * find_random_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, PlayerNumber owner, RoomKind rkind, unsigned char nav_flags);
struct Room * pick_random_room(PlayerNumber plyr_idx, RoomKind rkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
