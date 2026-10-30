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
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct DungeonCamera;

enum PlayerNames {
    PLAYER0          =  0,//red
    PLAYER1          =  1,//blue
    PLAYER2          =  2,//green
    PLAYER3          =  3,//yellow
    PLAYER_GOOD      =  4,//white
    PLAYER_NEUTRAL   =  5,
    PLAYER4          =  6,//purple
    PLAYER5          =  7,//black
    PLAYER6          =  8,//orange
    ALL_PLAYERS      =  9,
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
    PI_UnusedSlot14,
    PI_UnusedSlot15,
    PI_ZoomToPos,
    PI_UnusedSlot17,
    PI_UnusedSlot18,
};
/******************************************************************************/
#define ANY_USER -2

enum UserControllingThing {
    UCT_Direct           = 0x01, /* user is possessing the creature normally */
    UCT_Passenger        = 0x02, /* possession, but unable to control */
    UCT_Other            = 0x04, /* mainly for intro heart zoom-in and post-defeat spectating in multiplayer */
    UCT_PossessionEntryZoom = 0x08, /* the possession-start animation instance. (Without this flag, only counts if already in first-person.) */
    UCT_Selected         = 0x10, /* misc. for querying, cheat menu, etc. */
};

#pragma pack(1)

struct Thing;
struct UserState;
struct PlayerInfo;

typedef int32_t (*InstncInfo_Func)(NetUserId user, int32_t *n);

struct PlayerInstanceInfo { // sizeof = 44
  long length_turns;
  long instance_state;
  InstncInfo_Func start_cb;
  InstncInfo_Func maintain_cb;
  InstncInfo_Func end_cb;
  int32_t start_callback_parameters[2];
  unsigned char extra_callback_data[8];
  int32_t maintain_end_callback_parameter;
  int32_t reserved_callback_parameter;
};

#define PLAYER_INSTANCES_COUNT 19
#define PLAYER_INSTANCES_COUNT_OLD 17

/******************************************************************************/

#pragma pack()
/******************************************************************************/
extern struct PlayerInstanceInfo player_instance_info[PLAYER_INSTANCES_COUNT];
/******************************************************************************/
void set_user_instance(NetUserId user, int32_t ninum, TbBool force);
TbBool slap_thing(struct PlayerInfo *player, struct Thing *thing);
TbBool player_instance_controls_camera(unsigned char inum);
void process_user_instance(NetUserId user);
void process_player_instances(void);

void leave_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing);
void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
#define set_selected_creature(ustate,thing) set_selected_creature_f(ustate, thing, __func__)
TbBool set_selected_creature_f(struct UserState *ustate, struct Thing *thing, const char *func_name);
#define set_selected_thing(ustate,thing) set_selected_thing_f(ustate, thing, __func__)
TbBool set_selected_thing_f(struct UserState *ustate, struct Thing *thing, const char *func_name);
TbBool clear_selected_thing(struct UserState *ustate);

TbBool user_controls_thing(NetUserId user, const struct Thing *thing, uint32_t uct_flags);
NetUserId get_user_controlling_thing(const struct Thing *thing, uint32_t uct_flags);

void set_player_zoom_to_position(struct PlayerInfo *player,struct Coord3d *pos);

struct Room *player_build_room_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, RoomKind rkind, int slabs_left);
TbBool player_place_trap_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel);
TbBool player_place_trap_without_check_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel, TbBool free);
TbBool player_place_door_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel);
TbBool player_place_door_without_check_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel, TbBool free);
long packet_place_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel, TbBool allowed);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
