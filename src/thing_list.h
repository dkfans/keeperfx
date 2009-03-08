/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_list.h
 *     Header file for thing_list.c.
 * @par Purpose:
 *     Things list support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THINGLIST_H
#define DK_THINGLIST_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;

enum ThingClass {
    TCls_Empty        =  0,
    TCls_Object       =  1,
    TCls_Shot         =  2,
    TCls_EffectElem   =  3,
    TCls_DeadCreature =  4,
    TCls_Creature     =  5,
    TCls_EffectGen    =  7,
    TCls_Trap         =  8,
    TCls_Door         =  9,
//    TCls_AmbientSnd   =  x,
//    TCls_CaveIn       =  x,
};

struct StructureList {
     unsigned long field_0;
     unsigned long field_4;
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
    unsigned char field_4;
    unsigned char field_5;
    unsigned char owner;
    unsigned char field_7;
    unsigned char field_8;
    long field_9;
    struct Coord3d mappos;
    unsigned char field_13;
    unsigned char field_14;
    unsigned char field_15;
    unsigned char field_16;
    union {
      short word_17;
      struct {
    unsigned char l;
    unsigned char h;
        } byte_17;
    };
    unsigned char field_19;
    unsigned char model;
    unsigned short field_1B;
    unsigned short field_1D;
    unsigned char class_id;
    unsigned char field_20;
unsigned char field_21[2];
    unsigned char field_23;
    unsigned char field_24;
unsigned char field_25;
    struct CoordDelta3d pos_26;
    struct CoordDelta3d pos_2C;
    struct CoordDelta3d pos_32;
    struct CoordDelta3d pos_38;
unsigned short field_3E;
unsigned long field_40;
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
    unsigned short field_52;
    unsigned short field_54;
unsigned char field_56[2];
unsigned short field_58;
unsigned char field_5A[4];
    short field_5E; //signed
unsigned short field_60;
    unsigned short field_62;
    short field_64;
    unsigned char field_66;
    short next_of_class;
    unsigned char field_69;
    unsigned char field_6A;
};

#pragma pack()
/******************************************************************************/
unsigned long update_things_sounds_in_list(struct StructureList *list);
unsigned long update_cave_in_things(void);
void update_creatures_not_in_list(void);
unsigned long update_things_in_list(struct StructureList *list);
void init_traps(void);
void init_player_start(struct PlayerInfo *player);
void setup_computer_players(void);
void init_all_creature_states(void);

void update_things(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
