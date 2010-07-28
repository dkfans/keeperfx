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
