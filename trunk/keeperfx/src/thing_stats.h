/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_stats.h
 *     Header file for thing_stats.c.
 * @par Purpose:
 *     thing_stats functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THINGSTATS_H
#define DK_THINGSTATS_H

#include "bflib_basics.h"
#include "globals.h"

#define CREATURE_MAX_LEVEL     10

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
extern const char *blood_types[];
/******************************************************************************/
long compute_creature_max_pay(long base_pay,unsigned short crlevel);
long compute_creature_max_health(long base_health,unsigned short crlevel);
long compute_creature_attack_damage(long base_param,long luck,unsigned short crlevel);
long compute_creature_attack_range(long base_param,long luck,unsigned short crlevel);
long compute_creature_max_sparameter(long base_param,unsigned short crlevel);
long compute_creature_max_dexterity(long base_param,unsigned short crlevel);
long compute_creature_max_defence(long base_param,unsigned short crlevel);
long compute_creature_max_strength(long base_param,unsigned short crlevel);
long compute_creature_max_unaffected(long base_param,unsigned short crlevel);
#define compute_creature_max_luck compute_creature_max_unaffected
#define compute_creature_max_armour compute_creature_max_unaffected
long compute_value_percentage(long base_val, short npercent);
long compute_value_8bpercentage(long base_val, short npercent);

TbBool update_creature_health_to_max(struct Thing *thing);
void apply_damage_to_thing(struct Thing *thing, long a2, char a3);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
