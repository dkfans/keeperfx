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

enum CastCheckFlags {
    CastChk_Default = 0, /*< Default all flags. */
    CastChk_Final = 0x0001, /*< Final check - should be only a formality, as there were checks before. */
    CastChk_SkipThing = 0x0002, /*< While checking, skip the conditions related to specific thing. */
    CastChk_SkipAvailiabilty = 0x0004, /*< While checking, skip check if spell is availiable */
};

enum PowerModFlags {
    PwMod_Default = 0, /*< Default all flags. */
    PwMod_CastForFree = 0x0001, /*< Do not require player to pay for the spell. */
};

#pragma pack()
/******************************************************************************/
TbBool can_cast_spell_f(PlayerNumber plyr_idx, PowerKind pwmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, const struct Thing *thing, unsigned long flags, const char *func_name);
#define can_cast_spell(plyr_idx, pwmodel, stl_x, stl_y, thing, flags) can_cast_spell_f(plyr_idx, pwmodel, stl_x, stl_y, thing, flags, __func__)
TbBool can_cast_power_at_xy(PlayerNumber plyr_idx, PowerKind pwmodel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags);
TbBool can_cast_power_on_thing(PlayerNumber plyr_idx, const struct Thing *thing, PowerKind pwkind);

GoldAmount compute_power_price(PlayerNumber plyr_idx, PowerKind pwkind, long pwlevel);
GoldAmount compute_lowest_power_price(PlayerNumber plyr_idx, PowerKind pwkind, long pwlevel);
long find_spell_age_percentage(PlayerNumber plyr_idx, PowerKind pwkind);
TbBool find_power_cast_place(PlayerNumber plyr_idx, PowerKind pwkind, struct Coord3d *pos);
TbBool pay_for_spell(PlayerNumber plyr_idx, PowerKind pwkind, long pwlevel);
int get_power_overcharge_level(struct PlayerInfo *player);
TbBool update_power_overcharge(struct PlayerInfo *player, int spl_idx);
void process_dungeon_power_magic(void);

TbResult magic_use_power_chicken(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_disease(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_destroy_walls(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_imp(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long mod_flags);
TbResult magic_use_power_heal(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_conceal(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_armour(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_speed(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_lightning(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_time_bomb(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_sight(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_cave_in(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_call_to_arms(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags);
TbResult magic_use_power_slap(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long mod_flags);
TbResult magic_use_power_slap_thing(PlayerNumber plyr_idx, struct Thing *thing, unsigned long mod_flags);
TbResult magic_use_power_possess_thing(PlayerNumber plyr_idx, struct Thing *thing, unsigned long mod_flags);
TbResult magic_use_power_hold_audience(PlayerNumber plyr_idx, unsigned long mod_flags);
TbResult magic_use_power_armageddon(PlayerNumber plyr_idx, unsigned long mod_flags);
TbResult magic_use_power_obey(PlayerNumber plyr_idx, unsigned long mod_flags);

TbResult magic_use_available_power_on_thing(PlayerNumber plyr_idx, PowerKind spl_idx,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long allow_flags);
TbResult magic_use_available_power_on_subtile(PlayerNumber plyr_idx, PowerKind spl_idx,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags);
TbResult magic_use_available_power_on_level(PlayerNumber plyr_idx, PowerKind spl_idx, unsigned short splevel, unsigned long allow_flags);
void directly_cast_spell_on_thing(PlayerNumber plyr_idx, PowerKind spl_idx, ThingIndex thing_idx, long splevel);

TbResult magic_use_power_on_thing(PlayerNumber plyr_idx, PowerKind spl_idx,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long allow_flags);
TbResult magic_use_power_on_subtile(PlayerNumber plyr_idx, PowerKind spl_idx,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags);
TbResult magic_use_power_on_level(PlayerNumber plyr_idx, PowerKind spl_idx, unsigned short splevel, unsigned long allow_flags);

void slap_creature(struct PlayerInfo *player, struct Thing *thing);
void update_power_sight_explored(struct PlayerInfo *player);
TbBool update_creature_influenced_by_call_to_arms_at_pos(struct Thing *creatng, const struct Coord3d *cta_pos);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
