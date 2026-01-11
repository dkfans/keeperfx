/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file local_camera.c
 *     Client-side camera rendering for multiplayer with reduced input lag.
 * @par Purpose:
 *     Renders the local player's camera based on more up-to-date packets
 *     instead of input-lagged packets for smoother multiplayer experience.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     04 Dec 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "local_camera.h"
#include "engine_camera.h"
#include "engine_render.h"
#include "packets.h"
#include "player_data.h"
#include "net_input_lag.h"
#include "config_creature.h"
#include "thing_creature.h"
#include "game_legacy.h"
#include "map_data.h"
#include "bflib_math.h"
#include "frontmenu_ingame_map.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// In update(), camera code runs after turns are incremented
#define CURRENT_TURN_FOR_CAMERA (game.play_gameturn-1)
/******************************************************************************/
struct Camera local_cameras[4];
struct Camera previous_local_cameras[4];
struct Camera destination_local_cameras[4];
float interpolated_cam_mappos_x[4];
float interpolated_cam_mappos_y[4];
float interpolated_cam_mappos_z[4];
float interpolated_cam_rotation_angle_x[4];
float interpolated_cam_rotation_angle_y[4];
float interpolated_cam_rotation_angle_z[4];
float interpolated_camera_zoom[4];
TbBool local_camera_ready;
/******************************************************************************/

void send_camera_catchup_packets(struct PlayerInfo *player)
{
    // Threshold distance before sending catchup packets (in map coordinates)
    #define CAMERA_DESYNC_THRESHOLD 512

    if (!is_my_player(player) || !local_camera_ready) {
        return;
    }
    
    // Determine which camera to compare based on view mode
    int cam_idx = (player->view_mode == PVM_FrontView) ? CamIV_FrontView : CamIV_Isometric;
    
    struct Camera* local_cam = &destination_local_cameras[cam_idx];
    struct Camera* packet_cam = &player->cameras[cam_idx];
    struct Packet* pckt = get_packet(player->id_number);
    
    long diff_map_x = local_cam->mappos.x.val - packet_cam->mappos.x.val;
    long diff_map_y = local_cam->mappos.y.val - packet_cam->mappos.y.val;
    
    long angle = local_cam->rotation_angle_x;
    long cos_angle = LbCosL(angle);
    long sin_angle = LbSinL(angle);
    long diff_cam_right = (diff_map_x * cos_angle + diff_map_y * sin_angle) >> 16;
    long diff_cam_forward = (-diff_map_x * sin_angle + diff_map_y * cos_angle) >> 16;

    // Send catchup packets if position has drifted too far in camera space
    if (diff_cam_right > CAMERA_DESYNC_THRESHOLD) {
        set_packet_control(pckt, PCtr_MoveRight);
    } else if (diff_cam_right < -CAMERA_DESYNC_THRESHOLD) {
        set_packet_control(pckt, PCtr_MoveLeft);
    }
    if (diff_cam_forward > CAMERA_DESYNC_THRESHOLD) {
        set_packet_control(pckt, PCtr_MoveDown);
    } else if (diff_cam_forward < -CAMERA_DESYNC_THRESHOLD) {
        set_packet_control(pckt, PCtr_MoveUp);
    }
}

void sync_camera_state(int cam_idx, struct Camera *cam)
{
    local_cameras[cam_idx] = *cam;
    destination_local_cameras[cam_idx] = *cam;
    previous_local_cameras[cam_idx] = *cam;
    interpolated_cam_mappos_x[cam_idx] = cam->mappos.x.val;
    interpolated_cam_mappos_y[cam_idx] = cam->mappos.y.val;
    interpolated_cam_mappos_z[cam_idx] = cam->mappos.z.val;
    interpolated_cam_rotation_angle_x[cam_idx] = cam->rotation_angle_x;
    interpolated_cam_rotation_angle_y[cam_idx] = cam->rotation_angle_y;
    interpolated_cam_rotation_angle_z[cam_idx] = cam->rotation_angle_z;
    interpolated_camera_zoom[cam_idx] = cam->zoom;
}

void sync_first_person_camera(struct Camera *cam, struct PlayerInfo *player)
{
    if (player->controlled_thing_idx <= 0) {
        return;
    }
    struct Thing *ctrltng = thing_get(player->controlled_thing_idx);
    if (!thing_exists(ctrltng)) {
        return;
    }
    struct Camera corrected_cam = *cam;
    int eye_height = get_creature_eye_height(ctrltng);
    update_first_person_position(&corrected_cam, ctrltng, eye_height);
    corrected_cam.rotation_angle_x = ctrltng->move_angle_xy;
    corrected_cam.rotation_angle_y = ctrltng->move_angle_z;
    sync_camera_state(CamIV_FirstPerson, &corrected_cam);
}

void init_local_cameras(struct PlayerInfo *player)
{
    if (!is_my_player(player)) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        sync_camera_state(i, &player->cameras[i]);
    }
    local_camera_ready = true;
}

void process_local_minimap_click(struct Packet* packet) {
    if (packet != NULL && packet->action == PckA_BookmarkLoad) {
        long pos_x = subtile_coord_center(packet->actn_par1);
        long pos_y = subtile_coord_center(packet->actn_par2);
        for (int i = CamIV_Isometric; i <= CamIV_FrontView; i++) {
            if (i != CamIV_FirstPerson) {
                destination_local_cameras[i].mappos.x.val = pos_x;
                destination_local_cameras[i].mappos.y.val = pos_y;
            }
        }
    }
}

void update_local_first_person_camera(struct Thing *ctrltng)
{
    struct Camera* cam = &destination_local_cameras[CamIV_FirstPerson];
    int eye_height = get_creature_eye_height(ctrltng);
    update_first_person_position(cam, ctrltng, eye_height);

    long current_horizontal = destination_local_cameras[CamIV_FirstPerson].rotation_angle_x;
    long current_vertical = destination_local_cameras[CamIV_FirstPerson].rotation_angle_y;
    struct Packet* latest_packet = get_local_input_lag_packet_for_turn(CURRENT_TURN_FOR_CAMERA);
    if (latest_packet != NULL) {
        long new_horizontal, new_vertical, new_roll;
        process_first_person_look(ctrltng, latest_packet, current_horizontal, current_vertical, &new_horizontal, &new_vertical, &new_roll);
        current_horizontal = new_horizontal;
        current_vertical = new_vertical;
        if ((ctrltng->movement_flags & TMvF_Flying) != 0) {
            cam->rotation_angle_z = new_roll;
        }
    }
    cam->rotation_angle_x = current_horizontal;
    cam->rotation_angle_y = current_vertical;
}

void update_local_cameras(void)
{
    if (!local_camera_ready) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        previous_local_cameras[i] = destination_local_cameras[i];
    }
    struct PlayerInfo* my_player = get_my_player();
    struct Thing *ctrltng = thing_get(my_player->controlled_thing_idx);
    TbBool in_first_person = ( (thing_exists(ctrltng)) && (my_player->view_mode == PVM_CreatureView) );
    if (in_first_person) {
        update_local_first_person_camera(ctrltng);
    } else {
        struct Packet* local_packet = get_local_input_lag_packet_for_turn(CURRENT_TURN_FOR_CAMERA);
        if (local_packet == NULL) {
            return;
        }
        process_local_minimap_click(local_packet);
        // Only process camera controls for the currently active camera view
        int active_cam_idx = (my_player->view_mode == PVM_FrontView) ? CamIV_FrontView : CamIV_Isometric;
        process_camera_controls(&destination_local_cameras[active_cam_idx], local_packet, my_player, true);
        view_process_camera_inertia(&destination_local_cameras[active_cam_idx]);
        
        // Send catchup packets if local camera has drifted too far from packet-based camera
        send_camera_catchup_packets(my_player);
    }
}

void interpolate_local_cameras(void)
{
    if (!local_camera_ready) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        struct Camera* prev = &previous_local_cameras[i];
        struct Camera* desired = &destination_local_cameras[i];
        struct Camera* out = &local_cameras[i];
        interpolated_cam_mappos_x[i] = interpolate(interpolated_cam_mappos_x[i], prev->mappos.x.val, desired->mappos.x.val);
        interpolated_cam_mappos_y[i] = interpolate(interpolated_cam_mappos_y[i], prev->mappos.y.val, desired->mappos.y.val);
        interpolated_cam_mappos_z[i] = interpolate(interpolated_cam_mappos_z[i], prev->mappos.z.val, desired->mappos.z.val);
        interpolated_cam_rotation_angle_x[i] = interpolate_angle(interpolated_cam_rotation_angle_x[i], prev->rotation_angle_x, desired->rotation_angle_x);
        interpolated_cam_rotation_angle_y[i] = interpolate_angle(interpolated_cam_rotation_angle_y[i], prev->rotation_angle_y, desired->rotation_angle_y);
        interpolated_cam_rotation_angle_z[i] = interpolate_angle(interpolated_cam_rotation_angle_z[i], prev->rotation_angle_z, desired->rotation_angle_z);
        interpolated_camera_zoom[i] = interpolate(interpolated_camera_zoom[i], prev->zoom, desired->zoom);
        out->mappos.x.val = (long)interpolated_cam_mappos_x[i];
        out->mappos.y.val = (long)interpolated_cam_mappos_y[i];
        out->mappos.z.val = (long)interpolated_cam_mappos_z[i];
        out->rotation_angle_x = (int)interpolated_cam_rotation_angle_x[i] & ANGLE_MASK;
        out->rotation_angle_y = (int)interpolated_cam_rotation_angle_y[i] & ANGLE_MASK;
        out->rotation_angle_z = (int)interpolated_cam_rotation_angle_z[i] & ANGLE_MASK;
        out->zoom = (int)interpolated_camera_zoom[i];
    }
}

void sync_local_camera(struct PlayerInfo *player)
{
    if (!is_my_player(player) || !local_camera_ready) {
        return;
    }
    if (player->acamera == &player->cameras[CamIV_FirstPerson]) {
        sync_first_person_camera(player->acamera, player);
        return;
    }
    for (int cam_idx = CamIV_Isometric; cam_idx <= CamIV_FrontView; cam_idx++) {
        sync_camera_state(cam_idx, &player->cameras[cam_idx]);
    }
    if (player->view_mode == PVM_ParchmentView) {
        reset_all_minimap_interpolation = true;
    }
}

void set_local_camera_destination(struct PlayerInfo *player)
{
    if (!is_my_player(player) || !local_camera_ready) {
        return;
    }
    for (int cam_idx = CamIV_Isometric; cam_idx <= CamIV_FrontView; cam_idx++) {
        destination_local_cameras[cam_idx] = player->cameras[cam_idx];
    }
    struct Thing *ctrltng = thing_get(player->controlled_thing_idx);
    if (thing_exists(ctrltng)) {
        destination_local_cameras[CamIV_FirstPerson].rotation_angle_x = ctrltng->move_angle_xy;
        destination_local_cameras[CamIV_FirstPerson].rotation_angle_y = ctrltng->move_angle_z;
    }
}

struct Camera* get_local_camera(struct Camera* cam)
{
    if (!local_camera_ready) {
        return cam;
    }
    struct PlayerInfo *player = get_my_player();
    for (int cam_idx = CamIV_Isometric; cam_idx <= CamIV_FrontView; cam_idx++) {
        if (cam == &player->cameras[cam_idx]) {
            return &local_cameras[cam_idx];
        }
    }
    return cam;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
