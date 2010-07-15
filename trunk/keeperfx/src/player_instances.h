/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_instances.h
 *     Header file for player_instances.c.
 * @par Purpose:
 *     Player instances support and switching code.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PLYR_INSTNC_H
#define DK_PLYR_INSTNC_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum PlayerNames {
    PLAYER0          =  0,
    PLAYER1          =  1,
    PLAYER2          =  2,
    PLAYER3          =  3,
    PLAYER_GOOD      =  4,
    ALL_PLAYERS      =  8,
};

enum PlayerInstanceNum {
    PI_Unset        =  0,
    PI_Grab         =  1,
    PI_Drop         =  2,
    PI_Whip         =  3,
    PI_WhipEnd      =  4,
    PI_DirctCtrl    =  5,
    PI_PsngrCtrl    =  6,
    PI_DirctCtLeave =  7,
    PI_PsngrCtLeave =  8,
    PI_QueryCrtr    =  9,
    PI_UnqueryCrtr  = 10,
    PI_HeartZoom    = 11,
    PI_HeartZoomOut = 12,
    PI_CrCtrlFade   = 13,
    PI_MapFadeTo    = 14,
    PI_MapFadeFrom  = 15,
    PI_ZoomToPos    = 16,
};

enum PlayerStates {
    PSt_None                =   0,
    PSt_CtrlDungeon         =   1,
    PSt_BuildRoom           =   2,
    PSt_MkGoodWorker        =   3,
    PSt_MkGoodCreatr        =   4,
    PSt_CallToArms          =   6,
    PSt_CaveIn              =   7,
    PSt_SightOfEvil         =   8,
    PSt_Slap                =   9,
    PSt_CtrlPassngr         =  10,
    PSt_CtrlDirect          =  11,
    PSt_OrderCreatr         =  13,
    PSt_MkBadCreatr         =  14,
    PSt_PlaceTrap           =  16,
    PSt_Lightning           =  17,
    PSt_PlaceDoor           =  18,
    PSt_SpeedUp             =  19,
    PSt_Armour              =  20,
    PSt_Conceal             =  21,
    PSt_Heal                =  22,
    PSt_Sell                =  23,
    PSt_CreateDigger        =  24,
    PSt_DestroyWalls        =  25,
    PSt_CastDisease         =  26,
    PSt_TurnChicken         =  27,
    PSt_MkGoldPot           =  28,
};

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;
struct PlayerInfo;

typedef long (*InstncInfo_Func)(struct PlayerInfo *player, long *n);

struct PlayerInstanceInfo { // sizeof = 44
  long field_0;
  long field_4;
  InstncInfo_Func start_cb;
  InstncInfo_Func maintain_cb;
  InstncInfo_Func end_cb;
  long field_14[2];
  unsigned char field_1C[8];
  long field_24;
  long field_28;
};

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
extern struct PlayerInstanceInfo player_instance_info[];
/******************************************************************************/
DLLIMPORT struct PlayerInstanceInfo _DK_player_instance_info[];
//#define player_instance_info _DK_player_instance_info
/******************************************************************************/
void set_player_instance(struct PlayerInfo *player, long ninum, TbBool force);
void process_player_instance(struct PlayerInfo *player);
void process_player_instances(void);

void leave_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing);
void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
TbBool set_selected_creature(struct PlayerInfo *player, struct Thing *thing);

struct Room *player_build_room_at(long stl_x, long stl_y, long plyr_idx, long rkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
