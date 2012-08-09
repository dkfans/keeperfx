/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_effects.h
 *     Header file for thing_effects.c.
 * @par Purpose:
 *     Effect generators and effect elements support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     01 Jan 2010 - 12 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_TNGEFFECT_H
#define DK_TNGEFFECT_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define HIT_TYPES_COUNT 9

enum ThingEffectKind {
    TngEff_None = 0,
    TngEff_Unknown01,
    TngEff_Unknown02,
    TngEff_Unknown03,
    TngEff_Unknown04,
    TngEff_Unknown05,
    TngEff_Unknown06,
    TngEff_Unknown07,
    TngEff_Unknown08,
    TngEff_Unknown09,
    TngEff_Unknown10,
    TngEff_Unknown11,
    TngEff_Unknown12,
    TngEff_Unknown13,
    TngEff_Unknown14,
    TngEff_Unknown15,
    TngEff_Unknown16,
    TngEff_Unknown17,
    TngEff_Unknown18,
    TngEff_Unknown19,
    TngEff_Unknown20,
    TngEff_Unknown21,
    TngEff_Unknown22,
    TngEff_Unknown23,
    TngEff_Unknown24,
    TngEff_Unknown25,
    TngEff_Unknown26,
    TngEff_Unknown27,
    TngEff_Unknown28,
    TngEff_Unknown29,
    TngEff_Unknown30,
    TngEff_Unknown31,
    TngEff_Unknown32,
    TngEff_Unknown33,
    TngEff_Unknown34,
    TngEff_Unknown35,
    TngEff_Unknown36,
    TngEff_Unknown37,
    TngEff_Unknown38,
    TngEff_Unknown39,
    TngEff_Unknown40,
    TngEff_Unknown41,
    TngEff_Unknown42,
    TngEff_Unknown43,
    TngEff_Unknown44,
    TngEff_Unknown45,
    TngEff_Unknown46,
    TngEff_Unknown47,
    TngEff_Unknown48,
    TngEff_Unknown49,
    TngEff_Unknown50,
    TngEff_Unknown51,
    TngEff_Unknown52,
    TngEff_Unknown53,
    TngEff_Unknown54,
    TngEff_Unknown55,
    TngEff_Unknown56,
    TngEff_Unknown57,
    TngEff_Unknown58,
    TngEff_Unknown59,
    TngEff_Unknown60,
    TngEff_Unknown61,
    TngEff_Unknown62,
    TngEff_Unknown63,
    TngEff_Unknown64,
    TngEff_Unknown65,
    TngEff_Unknown66,
    TngEff_Unknown67,
    TngEff_Unknown68,
    TngEff_Unknown69,
};

/******************************************************************************/
#pragma pack(1)

struct InitEffect;
struct Thing;

struct EffectGeneratorStats { // sizeof = 57
    long genation_delay_min;
    long genation_delay_max;
    long genation_amount;
    long field_C;
    unsigned char field_10;
    long field_11;
    long acc_x_min;
    long acc_x_max;
    long acc_y_min;
    long acc_y_max;
    long acc_z_min;
    long acc_z_max;
    long sound_sample_idx;
    long sound_sample_rng;
    long sound_sample_sec;
};

struct EffectElementStats { // sizeof = 79
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2;
  short numfield_3;
  short numfield_5;
  short numfield_7;
  short numfield_9;
  short numfield_B;
  unsigned char field_D;
  unsigned short field_E;
  unsigned short field_10;
  unsigned char field_12;
  unsigned char field_13;
  unsigned char field_14;
  unsigned char field_15;
  unsigned char field_16;
  unsigned char field_17;
  unsigned char field_18;
  unsigned char field_19;
  unsigned char field_1A;
  unsigned char field_1B;
  unsigned char field_1C;
  unsigned char field_1D;
  unsigned short subeffect_model;
  unsigned short subeffect_delay;
  unsigned char field_22;
  unsigned short field_23;
  unsigned short field_25;
  unsigned short field_27;
  unsigned char field_29;
  unsigned short field_2A;
  unsigned short field_2C;
  unsigned short field_2E;
  unsigned char field_30;
  unsigned short field_31;
  unsigned short field_33;
  unsigned short field_35;
  unsigned char field_37;
  unsigned short transform_model;
  unsigned short field_3A;
  unsigned char field_3C;
  long field_3D;
  long field_41;
  long field_45;
  long field_49;
  unsigned char field_4D;
  unsigned char field_4E;
};

#pragma pack()
/******************************************************************************/
struct InitEffect *get_effect_info(ThingModel effmodel);
struct InitEffect *get_effect_info_for_thing(const struct Thing *thing);

struct Thing *create_effect(const struct Coord3d *pos, ThingModel effmodel, PlayerNumber owner);
struct Thing *create_effect_generator(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4);
struct Thing *create_effect_element(const struct Coord3d *pos, unsigned short a2, unsigned short a3);
long update_effect_element(struct Thing *thing);
long update_effect(struct Thing *thing);
long process_effect_generator(struct Thing *thing);
void process_spells_affected_by_effect_elements(struct Thing *thing);
TbBool destroy_effect_thing(struct Thing *thing);
long get_word_of_power_damage(const struct Thing *efftng, const struct Thing *thing, long range);
void create_special_used_effect(const struct Coord3d *pos, long plyr_idx);
struct Thing *create_price_effect(const struct Coord3d *pos, long plyr_idx, long price);

void explosion_affecting_area(struct Thing *tngsrc, const struct Coord3d *pos,
    MapSubtlCoord range, HitPoints max_damage, long blow_strength, unsigned char hit_type);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
