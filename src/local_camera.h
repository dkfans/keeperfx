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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct Thing;
struct Packet;
struct PlayerInfo;

/******************************************************************************/
void init_local_cameras(struct PlayerInfo *player);
void update_local_cameras(void);
void interpolate_local_cameras(void);
void sync_local_camera(struct PlayerInfo *player);
void sync_local_camera_pose(struct PlayerInfo *player, const struct DungeonCamera *pose);
void update_local_dungeon_view_mode(struct PlayerInfo *player);
void rotate_local_camera(int32_t angle);
void set_local_camera_destination(struct PlayerInfo *player);
void set_local_camera_destination_pose(struct PlayerInfo *player, const struct DungeonCamera *pose);
void step_local_possession_camera(struct PlayerInfo *player, const struct Thing *thing);
void record_local_possession_start(struct PlayerInfo *player);
int32_t get_local_possession_start_zoom(struct PlayerInfo *player);
void carry_local_dungeon_position(struct PlayerInfo *player, TbBool from_front_view);
void look_local_first_person_camera(struct Thing *ctrltng, int32_t turn_x, int32_t turn_y, int32_t *yaw, int32_t *pitch);
void move_local_camera_to_position(MapCoord x, MapCoord y);
void update_local_view_prediction(const struct Packet *pckt);
unsigned char get_local_view_type(const struct PlayerInfo *player);
struct Camera* get_local_active_camera(struct PlayerInfo *player);
struct Camera *get_local_dungeon_camera(struct PlayerInfo *player);
int get_local_active_camera_index(struct PlayerInfo *player);
void set_packet_power_on_thing(struct Packet *pckt, PowerKind pwkind, ThingIndex thing_idx);
void camera_packet_set_state(struct Packet *pckt);
struct Packet *get_freecam_packet(void);
TbBool replay_camera_detached(void);
void replay_detach(void);
void replay_attach(void);
void replay_freecam_set_map(TbBool on);
void replay_freecam_jump(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
