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
#pragma pack(1)

struct PlayerInfo;
struct Thing;

#pragma pack()
/******************************************************************************/
TbBool can_cast_spell_f(PlayerNumber plyr_idx, PowerKind pwmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, const struct Thing *thing, const char *func_name);
#define can_cast_spell(plyr_idx, pwmodel, stl_x, stl_y, thing) can_cast_spell_f(plyr_idx, pwmodel, stl_x, stl_y, thing, __func__)
TbBool can_cast_spell_at_xy(PlayerNumber plyr_idx, PowerKind pwmodel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags);
TbBool can_cast_spell_on_thing(PlayerNumber plyr_idx, const struct Thing *thing, PowerKind pwmodel);

void slap_creature(struct PlayerInfo *player, struct Thing *thing);
void update_power_sight_explored(struct PlayerInfo *player);
TbBool pay_for_spell(PlayerNumber plyr_idx, PowerKind spkind, long splevel);
long thing_affected_by_spell(struct Thing *thing, long spkind);

TbResult magic_use_power_chicken(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_disease(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_destroy_walls(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_imp(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbResult magic_use_power_heal(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_conceal(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_armour(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_speed(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_lightning(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_time_bomb(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_sight(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_cave_in(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel);
TbResult magic_use_power_call_to_arms(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long allow_everywhere);
TbResult magic_use_power_slap(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbResult magic_use_power_slap_thing(PlayerNumber plyr_idx, struct Thing *thing);
TbResult magic_use_power_possess_thing(PlayerNumber plyr_idx, struct Thing *thing);
TbResult magic_use_power_hold_audience(PlayerNumber plyr_idx);
TbResult magic_use_power_armageddon(PlayerNumber plyr_idx);
TbResult magic_use_power_obey(PlayerNumber plyr_idx);

TbResult magic_use_available_power_on_thing(PlayerNumber plyr_idx, PowerKind spl_idx,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing);
TbResult magic_use_available_power_on_subtile(PlayerNumber plyr_idx, PowerKind spl_idx,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags);
TbResult magic_use_available_power_on_level(PlayerNumber plyr_idx, PowerKind spl_idx, unsigned short splevel);
void directly_cast_spell_on_thing(PlayerNumber plyr_idx, PowerKind spl_idx, ThingIndex thing_idx, long splevel);

int get_power_overcharge_level(struct PlayerInfo *player);
TbBool update_power_overcharge(struct PlayerInfo *player, int spl_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
