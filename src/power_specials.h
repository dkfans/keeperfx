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
    SpcKind_Unknown6,
    SpcKind_Unknown7,
    SpcKind_Unknown8,
    SpcKind_Unknown9,
};

struct SpecialDesc {
long field_0;
long field_4;
    /** Speech message index, from TbSpeechMessages enum */
    long speech_msg;
};

/******************************************************************************/
DLLIMPORT long _DK_transfer_creature_scroll_offset;
#define transfer_creature_scroll_offset _DK_transfer_creature_scroll_offset
DLLIMPORT long _DK_resurrect_creature_scroll_offset;
#define resurrect_creature_scroll_offset _DK_resurrect_creature_scroll_offset
DLLIMPORT unsigned short _DK_dungeon_special_selected;
#define dungeon_special_selected _DK_dungeon_special_selected
DLLIMPORT struct SpecialDesc _DK_special_desc[8];
#define special_desc _DK_special_desc

#pragma pack()
/******************************************************************************/
void multiply_creatures(struct PlayerInfo *player);
void increase_level(struct PlayerInfo *player, int count);
TbBool steal_hero(struct PlayerInfo *player, struct Coord3d *pos);
void make_safe(struct PlayerInfo *player);
TbBool activate_bonus_level(struct PlayerInfo *player);
void activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player);
void resurrect_creature(struct Thing *thing, PlayerNumber owner, ThingModel model, unsigned char crlevel);
void transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char a3);
void start_resurrect_creature(struct PlayerInfo *player, struct Thing *thing);
void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing);
TbBool create_transferred_creature_on_level(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
