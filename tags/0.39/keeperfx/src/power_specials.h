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
#ifdef __cplusplus
#pragma pack(1)
#endif

struct PlayerInfo;
struct Thing;

struct SpecialDesc {
long field_0;
long field_4;
long field_8;
};


DLLIMPORT long _DK_transfer_creature_scroll_offset;
#define transfer_creature_scroll_offset _DK_transfer_creature_scroll_offset
DLLIMPORT long _DK_resurrect_creature_scroll_offset;
#define resurrect_creature_scroll_offset _DK_resurrect_creature_scroll_offset
DLLIMPORT unsigned short _DK_dungeon_special_selected;
#define dungeon_special_selected _DK_dungeon_special_selected
DLLIMPORT struct SpecialDesc _DK_special_desc[8];
#define special_desc _DK_special_desc

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
void multiply_creatures(struct PlayerInfo *player);
void increase_level(struct PlayerInfo *player);
unsigned long steal_hero(struct PlayerInfo *player, struct Coord3d *pos);
void make_safe(struct PlayerInfo *player);
TbBool activate_bonus_level(struct PlayerInfo *player);
void activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player);
void resurrect_creature(struct Thing *thing, unsigned char a2, unsigned char a3, unsigned char a4);
void transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char a3);
void start_resurrect_creature(struct PlayerInfo *player, struct Thing *thing);
void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
