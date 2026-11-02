/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file local_camera.h
 *     Header file for local_camera.c.
 * @par Purpose:
 *     Client-side camera rendering for multiplayer with reduced input lag.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     04 Dec 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LOCAL_CAMERA_H
#define DK_LOCAL_CAMERA_H

#include "globals.h"
#include "bflib_basics.h"
#include "engine_camera.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct Thing;
struct Packet;
struct PlayerInfo;

/******************************************************************************/
void init_local_cameras(struct PlayerInfo *player);
TbBool update_local_camera_time(void);
void update_local_cameras(void);
void update_local_first_person_view(void);
void interpolate_local_cameras(void);
void sync_local_camera(NetUserId user);
void sync_local_camera_pose(NetUserId user, const struct DungeonCamera *pose);
void update_local_dungeon_view_mode(NetUserId user);
void rotate_local_camera(int32_t angle);
void set_local_camera_destination(NetUserId user);
void set_local_camera_destination_pose(NetUserId user, const struct DungeonCamera *pose);
void step_local_possession_camera(NetUserId user, const struct Thing *thing);
void record_local_possession_start(NetUserId user);
int32_t get_local_possession_start_zoom(NetUserId user);
void carry_local_dungeon_position(NetUserId user, TbBool from_front_view);
void look_local_first_person_camera(struct Thing *ctrltng, int32_t turn_x, int32_t turn_y, int32_t *yaw, int32_t *pitch);
void move_local_camera_to_position(MapCoord x, MapCoord y);
void update_local_view_prediction(const struct Packet *pckt);
TbBool local_camera_shows_user(NetUserId user);
unsigned char get_local_view_type(void);
unsigned char get_displayed_view_type(void);
struct Camera* get_local_active_camera(void);
void set_packet_power_on_thing(struct Packet *pckt, PowerKind pwkind, ThingIndex thing_idx);
void camera_packet_set_state(struct Packet *pckt);
struct Packet *get_observer_camera_packet(void);
TbBool is_observer_camera_active(void);
void enter_observer_camera(void);
void return_to_player_camera(void);
void set_observer_camera_view(unsigned char view_type);
void fade_observer_camera_view(unsigned char view_type);
void observer_camera_jump(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
