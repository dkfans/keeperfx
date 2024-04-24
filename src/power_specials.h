/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_specials.h
 *     Header file for power_specials.c.
 * @par Purpose:
 *     power_specials functions.
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
#ifndef DK_POWERSPEC_H
#define DK_POWERSPEC_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;
struct Thing;

enum SpecialKinds {
    SpcKind_Unset = 0,
    SpcKind_RevealMap,
    SpcKind_Resurrect,
    SpcKind_TrnsfrCrtr,
    SpcKind_StealHero,
    SpcKind_MultplCrtr,
    SpcKind_IncrseLvl,
    SpcKind_MakeSafe,
    SpcKind_HiddnWorld,
    SpcKind_Custom,
    SpcKind_HealAll,
    SpcKind_GetGold,
    SpcKind_MakeAngry,
    SpcKind_MakeUnsafe,
};

/******************************************************************************/
extern long transfer_creature_scroll_offset;
extern long resurrect_creature_scroll_offset;
extern unsigned short dungeon_special_selected;

#pragma pack()
/******************************************************************************/
void multiply_creatures(struct PlayerInfo *player);
void increase_level(struct PlayerInfo *player, int count);
TbBool steal_hero(struct PlayerInfo *player, struct Coord3d *pos);
void make_safe(struct PlayerInfo *player);
void make_unsafe(PlayerNumber plyr_idx);
TbBool activate_bonus_level(struct PlayerInfo *player);
void activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player);
void resurrect_creature(struct Thing *thing, PlayerNumber owner, ThingModel model, unsigned char crlevel);
void transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char plyr_idx);
void start_resurrect_creature(struct PlayerInfo *player, struct Thing *thing);
void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing);
long create_transferred_creatures_on_level(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
