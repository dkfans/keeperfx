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

#define THINGS_COUNT         2048

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

/** Enums for thing->field_0 bit fields. */
enum ThingFlags0 {
    TF_IsInMapWho = 0x02,
};

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
    unsigned char field_0;
    unsigned char field_1;
    unsigned short field_2;
    unsigned short field_4;
    unsigned char owner;
    unsigned char field_7;
    unsigned char field_8;
    long field_9;
    struct Coord3d mappos;
    union {
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
    short field_1D;
    unsigned char class_id;
    unsigned char field_20;
unsigned char field_21;
unsigned char field_22;
    unsigned char field_23;
    unsigned char field_24;
unsigned char field_25;
    struct CoordDelta3d pos_26;
    struct CoordDelta3d pos_2C;
    struct CoordDelta3d pos_32;
    struct CoordDelta3d pos_38;
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
    unsigned short field_56;
unsigned short field_58;
    unsigned short field_5A;
    unsigned short field_5C;
    short health; //signed
unsigned short field_60;
    unsigned short field_62;
    short ccontrol_idx;
    unsigned char field_66;
    short next_of_class;
    unsigned char field_69;
    unsigned char field_6A;
};

#define INVALID_THING (game.things_lookup[0])

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
struct Thing *allocate_free_thing_structure(unsigned char a1);
short thing_create_thing(struct InitThing *itng);
unsigned char i_can_allocate_free_thing_structure(unsigned char a1);
void delete_thing_structure(struct Thing *thing, long a2);

struct Thing *thing_get(long tng_idx);
TbBool thing_exists_idx(long tng_idx);
TbBool thing_exists(const struct Thing *thing);
short thing_is_invalid(const struct Thing *thing);
long thing_get_index(const struct Thing *thing);

TbBool thing_touching_floor(const struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
