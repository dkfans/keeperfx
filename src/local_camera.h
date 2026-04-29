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
extern struct Camera local_cameras[4];
extern struct Camera previous_local_cameras[4];
extern struct Camera destination_local_cameras[4];
extern float interpolated_cam_mappos_x[4];
extern float interpolated_cam_mappos_y[4];
extern float interpolated_cam_mappos_z[4];
extern float interpolated_cam_rotation_angle_x[4];
extern float interpolated_cam_rotation_angle_y[4];
extern float interpolated_cam_rotation_angle_z[4];
extern float interpolated_camera_zoom[4];
extern TbBool local_camera_ready;
/******************************************************************************/
void init_local_cameras(struct PlayerInfo *player);
void update_local_cameras(void);
void interpolate_local_cameras(void);
void sync_local_camera(struct PlayerInfo *player);
void set_local_camera_destination(struct PlayerInfo *player);
struct Camera* get_local_camera(struct Camera* cam);
void send_camera_catchup_packets(struct PlayerInfo *player);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
