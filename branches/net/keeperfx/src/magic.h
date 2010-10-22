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
#include "map_data.h"
#include "player_data.h"

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
TbBool pay_for_spell(PlayerNumber plyr_idx, long spkind, long splevel);
long thing_affected_by_spell(struct Thing *thing, long spkind);

TbResult magic_use_power_chicken(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
void magic_use_power_disease(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
TbResult magic_use_power_destroy_walls(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_imp(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbResult magic_use_power_heal(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_conceal(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_armour(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_speed(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_lightning(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
long magic_use_power_sight(unsigned char a1, long a2, long a3, long a4);
void magic_use_power_cave_in(unsigned char a1, long a2, long a3, long a4);
long magic_use_power_call_to_arms(unsigned char a1, long a2, long a3, long a4, long a5);
short magic_use_power_slap(unsigned short plyr_idx, unsigned short stl_x, unsigned short stl_y);
short magic_use_power_slap_thing(unsigned short plyr_idx, struct Thing *thing);
void magic_use_power_hold_audience(unsigned char idx);
void magic_use_power_armageddon(unsigned int plridx);
short magic_use_power_obey(unsigned short plridx);

int get_spell_overcharge_level(struct PlayerInfo *player);
TbBool update_spell_overcharge(struct PlayerInfo *player, int spl_idx);
long can_cast_spell_on_creature(long a1, struct Thing *thing, long a3);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
