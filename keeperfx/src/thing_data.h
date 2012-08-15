/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_data.h
 *     Header file for thing_data.c.
 * @par Purpose:
 *     Thing struct support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THING_DATA_H
#define DK_THING_DATA_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/** Enums for thing->field_0 bit fields. */
enum ThingAllocFlags {
    TAlF_Exists        = 0x01,
    TAlF_IsInMapWho    = 0x02,
    TAlF_IsInStrucList = 0x04,
    TAlF_Unkn08        = 0x08,
    TAlF_IsInLimbo     = 0x10,
    TAlF_IsControlled  = 0x20,
    TAlF_IsInGroup     = 0x40,
    TAlF_IsDragged     = 0x80,
};

/** Enums for thing->field_1 bit fields. */
enum ThingFlags1 {
    TF1_Unkn01         = 0x01,
    TF1_InCtrldLimbo   = 0x02,
    TF1_PushdByAccel   = 0x04,
    TF1_Unkn08         = 0x08,
    TF1_Unkn10         = 0x10,
};

enum FreeThingAllocFlags {
    FTAF_Default             = 0x00,
    FTAF_FreeEffectIfNoSlots = 0x01,
    FTAF_LogFailures         = 0x80,
};

enum ThingMovementFlags {
    TMvF_Default            = 0x00,
    TMvF_IsOnWater          = 0x01,
    TMvF_IsOnLava          = 0x02,
    TMvF_Unknown04          = 0x04,
    TMvF_Unknown08          = 0x08,
    TMvF_Unknown10          = 0x10,
    TMvF_Flying             = 0x20,
    TMvF_Unknown40          = 0x40,
    TMvF_Unknown80          = 0x80,
};

/******************************************************************************/
#pragma pack(1)

struct Room;

struct InitThing { // sizeof=0x15
    struct Coord3d mappos;
    unsigned char oclass;
    unsigned char model;
    unsigned char owner;
    unsigned short range;
    unsigned short index;
    unsigned char params[8];
};

struct Thing {
    unsigned char alloc_flags;
    unsigned char field_1;
    unsigned short next_on_mapblk;
    unsigned short prev_on_mapblk;
    unsigned char owner;
    unsigned char active_state;
    unsigned char continue_state;
    long field_9;
    struct Coord3d mappos;
    union {
      struct {
        long gold_carried;
        short word_17a;
      } creature;
      struct {
        long gold_stored;
        short word_17b;
      } object;
      struct {
        unsigned char num_shots;
        unsigned char byte_14a;
        long long_15a;
      } trap;
      struct {
      long long_13;
      short word_17a;
      };
      struct {
      short word_13a;
      long long_15;
      };
      struct {
      short word_13;
      short word_15;
      short word_17;
      };
      struct {
      unsigned char byte_13b;
      short word_14;
      short word_16;
      unsigned char byte_18b;
      };
      struct {
      unsigned char byte_13a;
      long long_14;
      unsigned char byte_18a;
      };
      struct {
        unsigned char byte_13;
        unsigned char byte_14;
        unsigned char byte_15;
        unsigned char byte_16;
        unsigned char byte_17;
        unsigned char byte_18;
      };
    };
    unsigned char field_19;
    unsigned char model;
    unsigned short index;
    short parent_idx;
    unsigned char class_id;
    unsigned char field_20;
unsigned char field_21;
unsigned char field_22;
    unsigned char field_23;
    unsigned char field_24;
    unsigned char movement_flags;
    struct CoordDelta3d pos_26;
    struct CoordDelta3d pos_2C;
    struct CoordDelta3d acceleration;
    struct CoordDelta3d velocity;
unsigned short field_3E;
    long field_40;
unsigned short field_44;
unsigned short field_46;
unsigned char field_48;
unsigned char field_49;
    char field_4A;
unsigned short field_4B;
unsigned short field_4D;
    unsigned char field_4F;
    unsigned char field_50;
unsigned char field_51;
    short field_52;
    short field_54;
    unsigned short sizexy;
unsigned short field_58;
    unsigned short field_5A;
    unsigned short field_5C;
    short health; //signed
unsigned short field_60;
    unsigned short light_id;
    short ccontrol_idx;
    unsigned char snd_emitter_id;
    short next_of_class;
    short prev_of_class;
};

#define INVALID_THING (game.things.lookup[0])

/** Macro used for debugging problems related to things.
 * Should be executed in every function which changes a thing.
 * Can be defined to any SYNCLOG routine, making complete trace of usage on a thing.
 */
#define TRACE_THING(thing)

#pragma pack()
/******************************************************************************/
#define allocate_free_thing_structure(a1) allocate_free_thing_structure_f(a1, __func__)
struct Thing *allocate_free_thing_structure_f(unsigned char a1, const char *func_name);
short thing_create_thing(struct InitThing *itng);
TbBool i_can_allocate_free_thing_structure(unsigned char allocflags);
#define delete_thing_structure(thing, a2) delete_thing_structure_f(thing, a2, __func__)
void delete_thing_structure_f(struct Thing *thing, long a2, const char *func_name);
TbBool is_in_free_things_list(long tng_idx);

#define thing_get(tng_idx) thing_get_f(tng_idx, __func__)
struct Thing *thing_get_f(long tng_idx, const char *func_name);
TbBool thing_exists_idx(long tng_idx);
TbBool thing_exists(const struct Thing *thing);
short thing_is_invalid(const struct Thing *thing);
long thing_get_index(const struct Thing *thing);

TbBool thing_touching_floor(const struct Thing *thing);
struct PlayerInfo *get_player_thing_is_controlled_by(const struct Thing *thing);
unsigned char creature_remove_lair_from_room(struct Thing *thing, struct Room *room);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
