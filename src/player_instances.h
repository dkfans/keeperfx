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
    PLAYER_NEUTRAL   =  5,
    ALL_PLAYERS      =  8,
};

enum PlayerInstanceNum {
    PI_Unset = 0,
    PI_Grab,
    PI_Drop,
    PI_Whip,
    PI_WhipEnd,
    PI_DirctCtrl,
    PI_PsngrCtrl,
    PI_DirctCtLeave,
    PI_PsngrCtLeave,
    PI_QueryCrtr,
    PI_UnqueryCrtr,
    PI_HeartZoom,
    PI_HeartZoomOut,
    PI_CrCtrlFade,
    PI_MapFadeTo,
    PI_MapFadeFrom,
    PI_ZoomToPos,
    PI_Unknown17,
    PI_Unknown18,
};
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct PlayerInfo;

typedef long (*InstncInfo_Func)(struct PlayerInfo *player, long *n);

struct PlayerInstanceInfo { // sizeof = 44
  long length_turns;
  long field_4;
  InstncInfo_Func start_cb;
  InstncInfo_Func maintain_cb;
  InstncInfo_Func end_cb;
  long field_14[2];
  unsigned char field_1C[8];
  long field_24;
  long field_28;
};

/******************************************************************************/
DLLIMPORT struct PlayerInstanceInfo _DK_player_instance_info[];
//#define player_instance_info _DK_player_instance_info

#pragma pack()
/******************************************************************************/
extern struct PlayerInstanceInfo player_instance_info[];
/******************************************************************************/
void set_player_instance(struct PlayerInfo *player, long ninum, TbBool force);
void process_player_instance(struct PlayerInfo *player);
void process_player_instances(void);

void leave_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing);
void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
#define set_selected_creature(player,thing) set_selected_creature_f(player, thing, __func__)
TbBool set_selected_creature_f(struct PlayerInfo *player, struct Thing *thing, const char *func_name);
#define set_selected_thing(player,thing) set_selected_thing_f(player, thing, __func__)
TbBool set_selected_thing_f(struct PlayerInfo *player, struct Thing *thing, const char *func_name);
TbBool clear_selected_thing(struct PlayerInfo *player);
TbBool is_thing_directly_controlled(const struct Thing *thing);
TbBool is_thing_passenger_controlled(const struct Thing *thing);
TbBool is_thing_query_controlled(const struct Thing *thing);
TbBool is_thing_some_way_controlled(const struct Thing *thing);
TbBool is_thing_directly_controlled_by_player(const struct Thing *thing, PlayerNumber plyr_idx);
TbBool is_thing_passenger_controlled_by_player(const struct Thing *thing, PlayerNumber plyr_idx);

struct Room *player_build_room_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, RoomKind rkind);
TbBool player_place_trap_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel);
TbBool player_place_door_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
