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
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct PlayerInfo;
struct Thing;
struct CompoundFilterParam;

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
    TCls_AmbientSnd   = 12,
//    TCls_CaveIn       =  x,
};


typedef long FilterParam;
typedef struct CompoundFilterParam * MaxFilterParam;

typedef long (*Thing_State_Func)(struct Thing *);
typedef long (*Thing_Class_Func)(struct Thing *);
typedef long (*Thing_Filter)(const struct Thing *, FilterParam);
typedef long (*Thing_Maximizer_Filter)(const struct Thing *, MaxFilterParam, long);

struct CompoundFilterParam {
     long plyr_idx;
     long class_id;
     long model_id;
     long num1;
     long num2;
};

struct StructureList {
     unsigned long count;
     unsigned long index;
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
      long long_13;
      struct {
      short w0;
      short w1;
      } word_13;
      struct {
        unsigned char l;
        unsigned char h;
        unsigned char f2;
        unsigned char f3;
      } byte_13;
    };
    union {
      short word_17;
      struct {
    unsigned char l;
    unsigned char h;
        } byte_17;
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
extern Thing_Class_Func class_functions[];
/******************************************************************************/
long creature_near_filter_not_imp(const struct Thing *thing, FilterParam val);
long creature_near_filter_is_enemy_of_and_not_imp(const struct Thing *thing, FilterParam val);
long creature_near_filter_is_owned_by(const struct Thing *thing, FilterParam val);

struct Thing *get_player_list_creature_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxFilterParam param);

unsigned long update_things_sounds_in_list(struct StructureList *list);
void stop_all_things_playing_samples(void);
unsigned long update_cave_in_things(void);
unsigned long update_creatures_not_in_list(void);
unsigned long update_things_in_list(struct StructureList *list);
void init_traps(void);
void init_player_start(struct PlayerInfo *player);
void setup_computer_players(void);
void init_all_creature_states(void);
long creature_of_model_in_prison(int model);
long count_player_creatures_of_model(long plyr_idx, long model);
long count_player_creatures_not_counting_to_total(long plyr_idx);
TbBool knight_in_prison(void);

void update_things(void);

struct Thing *find_hero_gate_of_number(long num);
long get_free_hero_gate_number(void);

struct Thing *thing_get(long tng_idx);
TbBool thing_exists_idx(long tng_idx);
TbBool thing_exists(const struct Thing *thing);
short thing_is_invalid(const struct Thing *thing);
long thing_get_index(const struct Thing *thing);

TbBool thing_touching_floor(const struct Thing *thing);

long thing_is_shootable_by_any_player_including_objects(struct Thing *thing);
long thing_is_shootable_by_any_player_except_own_including_objects(struct Thing *shooter, struct Thing *thing);
long thing_is_shootable_by_any_player_except_own_excluding_objects(struct Thing *shooter, struct Thing *thing);
long thing_is_shootable_by_any_player_excluding_objects(struct Thing *thing);

TbBool update_thing(struct Thing *thing);
TbBigChecksum get_thing_checksum(struct Thing *thing);
short update_thing_sound(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
