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
/******************************************************************************/
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
    RoK_GARDEN              =  13,
    RoK_LAIR                =  14,
    RoK_BARRACKS            =  12,
    RoK_BRIDGE              =  15,
    RoK_GUARDPOST           =  16,
    RoK_UNKN17              =  17,
};
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

#define ROOM_EFFICIENCY_MAX 256
#define ROOM_SELL_REVENUE_PERCENT  50

struct Room;
typedef unsigned char RoomKind;

typedef void (*Room_Update_Func)(struct Room *);

struct RoomInfo { // sizeof = 6
  unsigned short field_0;
  unsigned short field_2;
  unsigned short field_4;
};

struct Room {
    unsigned char field_0;
    unsigned short index; // index in the rooms array
    unsigned char owner;
    short prev_of_owner;
    short next_of_owner;
    unsigned char stl_x;
    unsigned char stl_y;
    unsigned short kind;
    unsigned short field_C;
    short total_capacity;
    unsigned short workers_in;
    unsigned char field_12;
    unsigned char field_13[4];
    union {
    long long_17;
    struct {
      short word_17;
      short word_19;
    };
    };
    unsigned char field_1B[28];
    unsigned short slabs_list;
    unsigned short field_39;
    unsigned short slabs_count;
    unsigned short creatures_list;
    unsigned short efficiency;
    unsigned short field_41;
    unsigned char field_43;
    unsigned char field_44;
};

struct RoomData {
      unsigned char numfield_0;
      short numfield_1;
      Room_Update_Func ofsfield_3;
      Room_Update_Func ofsfield_7;
      Room_Update_Func offfield_B;
      unsigned char field_F;
      unsigned char field_10;
      short field_11;
      short msg1str_idx;
      short msg2str_idx;
};

#define INVALID_ROOM (&game.rooms[0])
#define INVALID_ROOM_DATA (&room_data[0])

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT extern struct RoomData _DK_room_data[];
//#define room_data _DK_room_data
/******************************************************************************/
extern unsigned short const room_effect_elements[];
extern const short slab_around[];
extern const unsigned short small_around_pos[4];
extern struct Around const mid_around[9];
extern struct AroundLByte const room_spark_offset[];
extern struct Around const small_around[];
extern struct RoomData room_data[];
extern struct Around const my_around_eight[];
extern short const around_map[];
/******************************************************************************/
struct Room *room_get(long room_idx);
struct Room *subtile_room_get(long stl_x, long stl_y);
struct Room *slab_room_get(long slb_x, long slb_y);
TbBool room_is_invalid(const struct Room *room);
struct RoomData *room_data_get_for_kind(RoomKind rkind);
struct RoomData *room_data_get_for_room(const struct Room *room);
struct RoomStats *room_stats_get_for_kind(RoomKind rkind);
struct RoomStats *room_stats_get_for_room(const struct Room *room);

long get_room_look_through(RoomKind rkind);
void set_room_efficiency(struct Room *room);
void set_room_capacity(struct Room *room, long capac);
long get_room_slabs_count(long plyr_idx, unsigned short rkind);
long get_player_rooms_count(long plyr_idx, unsigned short rkind);
void reinitialise_treaure_rooms(void);
TbBool find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
struct Room *find_room_with_spare_room_item_capacity(unsigned char a1, signed char a2);
struct Room *find_room_with_spare_capacity(unsigned char owner, signed char kind, long spare);
struct Room *find_room_with_spare_capacity_starting_with(long room_idx, long spare);
struct Room *find_room_with_most_spare_capacity_starting_with(long room_idx,long *total_spare_cap);
struct Room *find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing, signed char owner, signed char kind, unsigned char nav_no_owner, long spare);
struct Room *find_random_room_creature_can_navigate_to(struct Thing *thing, unsigned char owner, signed char kind, unsigned char nav_no_owner);

void create_room_flag(struct Room *room);
void delete_room_flag(struct Room *room);
struct Room *allocate_free_room_structure(void);
unsigned short i_can_allocate_free_room_structure(void);
void delete_all_room_structures(void);
void delete_room_structure(struct Room *room);
struct Room *link_adjacent_rooms_of_type(unsigned char owner, long x, long y, unsigned char rkind);
struct Room *create_room(unsigned char owner, unsigned char rkind, unsigned short x, unsigned short y);
short room_grow_food(struct Room *room);
void update_room_efficiency(struct Room *room);
long create_workshop_object_in_workshop_room(long a1, long a2, long a3);
struct Room *get_room_of_given_kind_for_thing(struct Thing *thing, struct Dungeon *dungeon, long rkind);

TbBool initialise_map_rooms(void);
void init_room_sparks(struct Room *room);
void replace_room_slab(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char owner, unsigned char a5);
short delete_room_slab_when_no_free_room_structures(long a1, long a2, unsigned char a3);
long calculate_room_efficiency(struct Room *room);
void kill_room_slab_and_contents(unsigned char a1, unsigned char a2, unsigned char a3);
void free_room_structure(struct Room *room);
void reset_creatures_rooms(struct Room *room);

/* MOVE TO room_list.c/h */
struct Room *find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, char a2, char a3, unsigned char a4);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
