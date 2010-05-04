/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file magic.h
 *     Header file for magic.c.
 * @par Purpose:
 *     magic functions.
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
#ifndef DK_MAGIC_H
#define DK_MAGIC_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct PlayerInfo;
struct Thing;

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
void slap_creature(struct PlayerInfo *player, struct Thing *thing);
TbBool can_cast_spell_at_xy(unsigned char plyr_idx, unsigned char spl_id, unsigned char stl_x, unsigned char stl_y, long a5);
void update_power_sight_explored(struct PlayerInfo *player);

void magic_use_power_chicken(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
void magic_use_power_disease(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
void magic_use_power_destroy_walls(unsigned char a1, long a2, long a3, long a4);
short magic_use_power_imp(unsigned short a1, unsigned short a2, unsigned short a3);
void magic_use_power_heal(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
void magic_use_power_conceal(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
void magic_use_power_armour(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
void magic_use_power_speed(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
void magic_use_power_lightning(unsigned char a1, long a2, long a3, long a4);
long magic_use_power_sight(unsigned char a1, long a2, long a3, long a4);
void magic_use_power_cave_in(unsigned char a1, long a2, long a3, long a4);
long magic_use_power_call_to_arms(unsigned char a1, long a2, long a3, long a4, long a5);
short magic_use_power_slap(unsigned short plyr_idx, unsigned short stl_x, unsigned short stl_y);
void magic_use_power_hold_audience(unsigned char idx);
void magic_use_power_armageddon(unsigned int plridx);
short magic_use_power_obey(unsigned short plridx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
