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
struct Thing;
struct PlayerInfo;
typedef long (*InstncInfo_Func)(struct PlayerInfo *player, long *n);

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
    PSt_None           =    0,
    PSt_BuildRoom      =    2,
    PSt_MkGoodWorker   =    3,
    PSt_MkGoodCreatr   =    4,
    PSt_CtrlPassngr    =   10,
    PSt_CtrlDirect     =   11,
    PSt_OrderCreatr    =   13,
    PSt_MkBadCreatr    =   14,
    PSt_SplDstrWalls   =   25,
    PSt_SplDisease     =   26,
    PSt_SplChicken     =   27,
};

#ifdef __cplusplus
#pragma pack(1)
#endif

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

void set_player_instance(struct PlayerInfo *player, long ninum, short force);
void process_player_instance(struct PlayerInfo *player);
void process_player_instances(void);

void leave_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing);
void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
TbBool set_selected_creature(struct PlayerInfo *player, struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
