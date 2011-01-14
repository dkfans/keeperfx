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
#ifdef __cplusplus
#pragma pack(1)
#endif

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

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
struct Thing *create_effect(const struct Coord3d *pos, unsigned short a2, unsigned char a3);
struct Thing *create_effect_generator(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4);
struct Thing *create_effect_element(const struct Coord3d *pos, unsigned short a2, unsigned short a3);
long update_effect_element(struct Thing *thing);
long update_effect(struct Thing *thing);
long process_effect_generator(struct Thing *thing);
void process_spells_affected_by_effect_elements(struct Thing *thing);
void process_thing_spell_effects(struct Thing *thing);
TbBool destroy_effect_generator(struct Thing *thing);
long get_word_of_power_damage(const struct Thing *efftng, const struct Thing *thing);
void create_special_used_effect(const struct Coord3d *pos, long plyr_idx);
struct Thing *create_price_effect(const struct Coord3d *pos, long plyr_idx, long price);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
