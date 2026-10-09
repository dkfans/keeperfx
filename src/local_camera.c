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
#include "observer.h"
#include "local_camera.h"
#include "engine_camera.h"
#include "engine_render.h"
#include "engine_redraw.h"
#include "net_exchange_gameplay.h"
#include "packets.h"
#include "player_data.h"
#include "config_creature.h"
#include "config_keeperfx.h"
#include "thing_creature.h"
#include "game_legacy.h"
#include "dungeon_data.h"
#include "map_data.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "frontmenu_ingame_map.h"
#include "frontend.h"
#include "gui_parchment.h"
#include "player_instances.h"
#include "bflib_datetm.h"

#include <math.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static struct Packet observer_camera_packet;
static int64_t camera_update_time;
static TbBool camera_step_due = true;
extern float interpolate_time;
/******************************************************************************/
static int get_local_active_camera_index(struct PlayerInfo *player);
static struct Camera *get_local_dungeon_camera(struct PlayerInfo *player);

static TbBool local_camera_follows_player(const struct PlayerInfo *player)
{
    if (!is_my_player(player) || !local_state.camera.ready || is_observer_camera_active()) {
        return false;
    }
    return true;
}

static TbBool get_packet_rotation_pivot(const struct Packet *pckt, MapCoord *x, MapCoord *y)
{
    if (!flag_is_set(pckt->control_flags, PCtr_MapCoordsValid))
        return false;
    *x = pckt->pos_x;
    *y = pckt->pos_y;
    return true;
}

void camera_packet_set_state(struct Packet *pckt)
{
    struct PlayerInfo* player = get_my_player();
    packet_set_camera_context(pckt, get_local_active_camera_index(player));
    local_state.camera.has_rotation_pivot = get_packet_rotation_pivot(pckt, &local_state.camera.rotation_pivot_x, &local_state.camera.rotation_pivot_y);
    if (!local_state.camera.ready || is_observer_camera_active() || (get_local_view_type(player) != PVT_DungeonTop) || (get_player_view_type(player) != PVT_DungeonTop)
     || player_instance_controls_camera(player->instance_num)) {
        // senseless to transmit camera coords during these times, and a camera instance is computed from the dungeon camera it started with
        packet_clear_camera_position(pckt);
        return;
    }
    const int cam_idx = get_local_active_camera(player) - local_state.camera.current;
    if ((cam_idx != CamIV_Isometric) && (cam_idx != CamIV_FrontView)) {
        packet_clear_camera_position(pckt);
        return;
    }
    const struct Camera *cam = &local_state.camera.destination[cam_idx];
    const TbBool front_view = (cam_idx == CamIV_FrontView);

    // Correct the dungeon camera's angle whenever the action slot is free. The local camera ignores this correction,
    // so it's safe to send mid-rotation.
    if ((pckt->action == PckA_None)
     && (cam->rotation_angle_x != get_player_user_state(player)->dungeon_camera.yaw[front_view]))
    {
        set_packet_action(pckt, PckA_SetMapRotation, cam->rotation_angle_x, 0, 0, 0);
    }
    packet_set_camera_position(pckt, cam->mappos.x.val, cam->mappos.y.val);
}

void set_packet_power_on_thing(struct Packet *pckt, PowerKind pwkind, ThingIndex thing_idx)
{
    set_packet_action(pckt, PckA_UsePwrOnThing, pwkind, thing_idx, 0, 0);
}

void rotate_local_camera(int32_t angle)
{
    local_state.camera.rotation_pending = true;
    local_state.camera.rotation_angle = angle & ANGLE_MASK;
}

static void apply_local_camera_rotation(void)
{
    if (!local_state.camera.rotation_pending) {
        return;
    }
    local_state.camera.rotation_pending = false;
    struct Packet rotation;
    memset(&rotation, 0, sizeof(rotation));
    rotation.action = PckA_SetMapRotation;
    rotation.actn_par1 = local_state.camera.rotation_angle;
    packet_set_camera_context(&rotation, get_local_active_camera_index(get_my_player()));
    process_camera_action(local_state.camera.destination, &rotation);
}

static void sync_camera_state(int cam_idx, struct Camera *cam)
{
    local_state.camera.current[cam_idx] = *cam;
    local_state.camera.destination[cam_idx] = *cam;
    local_state.camera.previous[cam_idx] = *cam;
}

static void apply_dungeon_camera_pose(struct Camera cams[], const struct DungeonCamera *pose)
{
    const struct DungeonCamera dcam = *pose;
    const int dungeon_cam_indices[] = {CamIV_Isometric, CamIV_FrontView};
    for (int i = 0; i < 2; i++) {
        struct Camera *cam = &cams[dungeon_cam_indices[i]];
        cam->mappos.x.val = dcam.x;
        cam->mappos.y.val = dcam.y;
        cam->velocity_x = 0;
        cam->velocity_y = 0;
    }
    for (int i = 0; i < 2; i++) {
        struct Camera *cam = &cams[dungeon_cam_indices[i]];
        cam->zoom = dcam.zoom[i];
        cam->rotation_angle_x = dcam.yaw[i];
    }
    cams[CamIV_Isometric].rotation_angle_y = dcam.pitch;
}

/** local camera <- canonical dungeon camera */
static void apply_dungeon_camera(struct Camera cams[], const struct PlayerInfo *player)
{
    apply_dungeon_camera_pose(cams, &get_player_user_state(player)->dungeon_camera);
}

/** local camera <- given dungeon camera */
void set_local_camera_destination_pose(struct PlayerInfo *player, const struct DungeonCamera *pose)
{
    if (!local_camera_follows_player(player) || get_local_view_type(player) == PVT_MapScreen) {
        return;
    }
    apply_dungeon_camera_pose(local_state.camera.destination, pose);
}

/** local view mode from wibble bool */
void update_local_dungeon_view_mode(struct PlayerInfo *player)
{
    if (!is_my_player(player))
        return;
    const unsigned char view_mode = (get_player_user_state(player)->prefs[UPref_Wibble] != 0) ? PVM_IsoWibbleView : PVM_IsoStraightView;
    local_state.camera.current[CamIV_Isometric].view_mode = view_mode;
    local_state.camera.previous[CamIV_Isometric].view_mode = view_mode;
    local_state.camera.destination[CamIV_Isometric].view_mode = view_mode;
}

static void update_first_person_camera(struct Camera *cam, const struct PlayerInfo *player, TbBool snap)
{
    if (player->controlled_thing_idx <= 0) {
        return;
    }
    struct Thing *ctrltng = thing_get(player->controlled_thing_idx);
    if (!thing_exists(ctrltng)) {
        return;
    }
    int eye_height = get_creature_eye_height(ctrltng);
    update_first_person_position(cam, ctrltng, eye_height);
    cam->rotation_angle_x = ctrltng->move_angle_xy;
    cam->rotation_angle_y = ctrltng->move_angle_z;
    if (snap) {
        sync_camera_state(CamIV_FirstPerson, cam);
    }
    local_state.camera.first_person_look_pending = false;
}

void init_local_cameras(struct PlayerInfo *player)
{
    if (!is_my_player(player) || (local_state.camera.ready && is_observer_camera_active())) {
        return;
    }
    local_state.camera.first_person_look_pending = false;
    camera_update_time = get_time_tick_ns();
    camera_step_due = true;
    struct Camera cams[CamIV_EndList];
    memset(cams, 0, sizeof(cams));

    struct Camera *cam = &cams[CamIV_FirstPerson];
    cam->mappos.z.val = 256;
    cam->horizontal_fov = first_person_horizontal_fov;
    cam->rotation_angle_x = ANGLE_EAST;

    cam = &cams[CamIV_Isometric];
    cam->horizontal_fov = 94;
    cam->rotation_angle_x = DEGREES_45;
    cam->view_mode = (get_player_user_state(player)->prefs[UPref_Wibble] != 0) ? PVM_IsoWibbleView : PVM_IsoStraightView;

    cam = &cams[CamIV_Parchment];
    cam->mappos.z.val = 32;
    cam->horizontal_fov = 94;

    cam = &cams[CamIV_FrontView];
    cam->mappos.z.val = 32;
    cam->horizontal_fov = 94;
    cam->view_mode = PVM_FrontView;

    apply_dungeon_camera(cams, player);
    if (player == &local_observer_player && get_player_active_camera_index(player) == CamIV_FirstPerson) {
        update_first_person_camera(&cams[CamIV_FirstPerson], get_displayed_player(), false);
    }
    for (int i = 0; i < CamIV_EndList; i++) {
        sync_camera_state(i, &cams[i]);
    }
    local_state.camera.move_cam = NULL;
    local_state.camera.ready = true;
}

void move_local_camera_to_position(MapCoord x, MapCoord y)
{
    if (!local_state.camera.ready) {
        return;
    }
    if (observer_is_active()) {
        enter_observer_camera();
        observer_camera_jump(coord_subtile(x), coord_subtile(y));
        return;
    }
    int cam_idx = get_local_active_camera(get_my_player()) - local_state.camera.current;
    struct Camera *cam = &local_state.camera.destination[cam_idx];
    local_state.camera.move_cam = cam;
    local_state.camera.move_target[0] = x;
    local_state.camera.move_target[1] = y;
    view_set_camera_move_to_position(cam->mappos.x.val, cam->mappos.y.val, x, y, &local_state.camera.move_delta[0], &local_state.camera.move_delta[1]);
}

static void process_local_camera_movement(struct Camera *cam, const struct PlayerInfo *player)
{
    const int32_t rate = camera_move_rate(cam, player, local_state.camera_speedup_pressed);
    if (local_state.camera_movement_y != 0.0f) {
        view_set_camera_y_velocity(cam, (int32_t)(local_state.camera_movement_y * rate / 4.0f), (int32_t)(local_state.camera_movement_y * rate));
    }
    if (local_state.camera_movement_x != 0.0f) {
        view_set_camera_x_velocity(cam, (int32_t)(local_state.camera_movement_x * rate / 4.0f), (int32_t)(local_state.camera_movement_x * rate));
    }
}

struct LocalCameraRotation {
    TbBool cw;
    TbBool ccw;
    TbBool around_cursor;
};

static struct LocalCameraRotation take_local_camera_rotation(void)
{
    struct LocalCameraRotation rotation;
    rotation.cw = local_state.camera_rotate_cw;
    rotation.ccw = local_state.camera_rotate_ccw;
    rotation.around_cursor = local_state.camera_rotate_around_cursor;
    local_state.camera_rotate_cw = false;
    local_state.camera_rotate_ccw = false;
    local_state.camera_rotate_around_cursor = false;
    return rotation;
}

static void process_local_camera_controls(struct Camera* cam, const struct Packet* pckt, struct PlayerInfo* player)
{
    MapCoord x;
    MapCoord y;
    const TbBool dungeon_view = (cam->view_mode == PVM_IsoWibbleView) || (cam->view_mode == PVM_IsoStraightView) || (cam->view_mode == PVM_FrontView);
    if (dungeon_view && packet_get_camera_position(pckt, &x, &y))
    {
        view_set_camera_position(cam, x, y);
        cam->velocity_x = 0;
        cam->velocity_y = 0;
    }
    process_camera_view_controls(cam, pckt, player);
}

static void process_local_camera_rotation(struct Camera *cam, TbBool has_pivot, MapCoord pivot_x, MapCoord pivot_y, const struct LocalCameraRotation *rotation)
{
    const TbBool use_pivot = rotation->around_cursor && has_pivot;
    const MapCoord rot_x = use_pivot ? pivot_x : -1;
    const MapCoord rot_y = use_pivot ? pivot_y : -1;
    const TbBool front_view = (cam->view_mode == PVM_FrontView);
    if (rotation->ccw)
    {
        if (front_view)
            cam->rotation_angle_x = (cam->rotation_angle_x + DEGREES_90) & ANGLE_MASK;
        else
            view_set_camera_rotation_velocity_around(cam, 16, 64, rot_x, rot_y);
    }
    if (rotation->cw)
    {
        if (front_view)
            cam->rotation_angle_x = (cam->rotation_angle_x - DEGREES_90) & ANGLE_MASK;
        else
            view_set_camera_rotation_velocity_around(cam, -16, -64, rot_x, rot_y);
    }
}

static TbBool local_first_person_look_frozen(struct Thing *ctrltng)
{
    return flag_is_set(game.operation_flags, GOF_Paused) || !can_process_creature_input(ctrltng);
}

// applies mouse to local first-person camera
void look_local_first_person_camera(struct Thing *ctrltng, int32_t turn_x, int32_t turn_y, int32_t *yaw, int32_t *pitch)
{
    const struct Camera *cam = &local_state.camera.destination[CamIV_FirstPerson];
    if (!thing_exists(ctrltng)) {
        *yaw = cam->rotation_angle_x;
        *pitch = cam->rotation_angle_y;
        return;
    }
    if (!local_state.camera.ready || local_first_person_look_frozen(ctrltng)) {
        *yaw = ctrltng->move_angle_xy;
        *pitch = ctrltng->move_angle_z;
        return;
    }
    process_first_person_look(ctrltng, turn_x, turn_y, cam->rotation_angle_x, cam->rotation_angle_y,
        &local_state.camera.first_person_look_yaw, &local_state.camera.first_person_look_pitch, &local_state.camera.first_person_look_roll);
    local_state.camera.first_person_look_pending = true;
    *yaw = local_state.camera.first_person_look_yaw;
    *pitch = local_state.camera.first_person_look_pitch;
}

// when not canonically able to adjust camera (e.g. frozen),
// rotate camera quickly (but smoothly) back to canonical facing
static void snap_back_first_person_look(struct Camera *cam, const struct Thing *ctrltng)
{
    const int32_t delta_yaw = get_angle_signed_difference(cam->rotation_angle_x, ctrltng->move_angle_xy);
    const int32_t delta_pitch = get_angle_signed_difference(cam->rotation_angle_y, ctrltng->move_angle_z);
    cam->rotation_angle_x = (cam->rotation_angle_x + delta_yaw / 2) & ANGLE_MASK;
    cam->rotation_angle_y = (cam->rotation_angle_y + delta_pitch / 2) & ANGLE_MASK;
}

static void update_local_first_person_camera(struct Thing *ctrltng)
{
    struct Camera* cam = &local_state.camera.destination[CamIV_FirstPerson];
    int eye_height = get_creature_eye_height(ctrltng);
    update_first_person_position(cam, ctrltng, eye_height);

    // A replay shows the recorded look as the creature took it.
    if (replay.load_enable)
    {
        local_state.camera.first_person_look_pending = false;
        cam->rotation_angle_x = ctrltng->move_angle_xy;
        cam->rotation_angle_y = ctrltng->move_angle_z;
        return;
    }
    if (local_first_person_look_frozen(ctrltng))
    {
        local_state.camera.first_person_look_pending = false;
        snap_back_first_person_look(cam, ctrltng);
        return;
    }
    if (!local_state.camera.first_person_look_pending) {
        return;
    }
    local_state.camera.first_person_look_pending = false;
    cam->rotation_angle_x = local_state.camera.first_person_look_yaw;
    cam->rotation_angle_y = local_state.camera.first_person_look_pitch;
    if ((ctrltng->movement_flags & TMvF_Flying) != 0) {
        cam->rotation_angle_z = local_state.camera.first_person_look_roll;
    }
}

TbBool update_local_camera_time(void)
{
    const int64_t now = get_time_tick_ns();
    const int64_t interval = 1000000000 / turns_per_second;
    if (game.fast_forward <= 1 || now < camera_update_time) {
        camera_update_time = now;
        camera_step_due = true;
    } else {
        const int64_t elapsed = now - camera_update_time;
        camera_step_due = elapsed >= interval;
        if (camera_step_due) {
            camera_update_time = now - min(elapsed - interval, interval);
        }
    }
    return camera_step_due;
}

void update_local_cameras(void)
{
    if (!local_state.camera.ready) {
        return;
    }
    struct PlayerInfo *player = get_my_player();
    const unsigned char previous_camera_idx = get_player_active_camera_index(player);
    if (player == &local_observer_player) {
        observer_update_view();
    }
    struct Thing *ctrltng = thing_get(player->controlled_thing_idx);
    const struct Packet *pckt = get_history_packet(get_local_user(), get_gameturn());
    const int active_cam_idx = get_local_active_camera(player) - local_state.camera.current;
    const TbBool move_camera = camera_step_due || (replay.load_enable && !is_observer_camera_active());
    const struct LocalCameraRotation rotation = take_local_camera_rotation();
    local_state.camera.previous_deviation_x = local_state.camera.destination_deviation_x;
    local_state.camera.previous_deviation_y = local_state.camera.destination_deviation_y;
    local_state.camera.destination_deviation_x = 0;
    local_state.camera.destination_deviation_y = 0;

    if (move_camera || active_cam_idx == CamIV_FirstPerson || player_instance_controls_camera(player->instance_num)) {
        memcpy(local_state.camera.previous, local_state.camera.destination, sizeof(local_state.camera.previous));
    }
    if (is_observer_camera_active()) {
        if (local_state.observer_camera_view_type == PVT_DungeonTop) {
            process_camera_action(local_state.camera.destination, &observer_camera_packet);
            apply_local_camera_rotation();
            struct Camera *cam = &local_state.camera.destination[local_state.observer_camera_idx];
            process_local_camera_movement(cam, player);
            MapCoord pivot_x = 0;
            MapCoord pivot_y = 0;
            const TbBool has_pivot = get_packet_rotation_pivot(&observer_camera_packet, &pivot_x, &pivot_y);
            process_local_camera_rotation(cam, has_pivot, pivot_x, pivot_y, &rotation);
            process_camera_view_controls(cam, &observer_camera_packet, player);
            if (move_camera) {
                view_process_camera_velocity(cam);
            }
        }
        local_state.camera_movement_x = 0.0f;
        local_state.camera_movement_y = 0.0f;
        memset(&observer_camera_packet, 0, sizeof(observer_camera_packet));
        return;
    }
    if (player == &local_observer_player) {
        apply_dungeon_camera(local_state.camera.destination, player);
        update_local_dungeon_view_mode(player);
        if (get_player_active_camera_index(player) == CamIV_FirstPerson) {
            update_first_person_camera(&local_state.camera.destination[CamIV_FirstPerson], get_displayed_player(),
                previous_camera_idx != CamIV_FirstPerson);
        }
        return;
    }
    if (replay_playback_is_paused()) {
        pckt = NULL;
    }
    if (pckt != NULL) {
        // Absolute camera rotation doesn't overwrite local camera except on replays
        if ((pckt->action != PckA_SetMapRotation) || replay.load_enable) {
            process_camera_action(local_state.camera.destination, pckt);
        }
        // Skip interpolation for parchment jumps, while retaining it for minimap dragging.
        if (pckt->action == PckA_ZoomFromMap) {
            memcpy(local_state.camera.previous, local_state.camera.destination, sizeof(local_state.camera.previous));
        }
    }
    apply_local_camera_rotation();

    if (active_cam_idx == CamIV_FirstPerson && thing_exists(ctrltng)) {
        update_local_first_person_camera(ctrltng);
        return;
    }
    if (pckt == NULL) {
        return;
    }

    struct Camera *cam = &local_state.camera.destination[active_cam_idx];
    if (active_cam_idx == CamIV_Parchment) {
        cam->mappos.x.val = pckt->pos_x;
        cam->mappos.y.val = pckt->pos_y;
    }
    if (move_camera && local_state.camera.move_cam != NULL) {
        local_state.camera.move_cam->velocity_x = 0;
        local_state.camera.move_cam->velocity_y = 0;
        if (view_move_camera_to_position(&local_state.camera.move_cam->mappos.x.val, &local_state.camera.move_cam->mappos.y.val,
                local_state.camera.move_target[0], local_state.camera.move_target[1], local_state.camera.move_delta[0], local_state.camera.move_delta[1])) {
            local_state.camera.move_cam = NULL;
        }
    }
    if (local_state.camera.move_cam != cam) {
        // Same as the packet camera: a parchment map jump ignores the packet's camera controls.
        if (pckt->action != PckA_ZoomFromMap) {
            if (!replay.load_enable && get_local_view_type(player) != PVT_MapScreen) {
                process_local_camera_movement(cam, player);
                process_local_camera_rotation(cam, local_state.camera.has_rotation_pivot,
                    local_state.camera.rotation_pivot_x, local_state.camera.rotation_pivot_y, &rotation);
                process_camera_view_controls(cam, pckt, player);
            } else {
                process_local_camera_controls(cam, pckt, player);
            }
            local_state.camera_movement_x = 0.0f;
            local_state.camera_movement_y = 0.0f;
        }
        if (move_camera) {
            view_process_camera_velocity(cam);
        }
    }
    if (active_cam_idx == CamIV_Isometric) {
        struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
        if (dungeon->camera_deviate_jump != 0) {
            int32_t angle = cam->rotation_angle_x;
            local_state.camera.destination_deviation_x += ( (dungeon->camera_deviate_jump * LbSinL(angle) >> 8) >> 8);
            local_state.camera.destination_deviation_y += (-(dungeon->camera_deviate_jump * LbCosL(angle) >> 8) >> 8);
        }
    }
}

static void interpolate_camera_deviations(void)
{
    struct PlayerInfo* my_player = get_my_player();
    if (!player_exists(my_player)) {
        return;
    }
    struct Camera *active_camera = get_local_active_camera(my_player);
    if (active_camera->view_mode != PVM_IsoWibbleView && active_camera->view_mode != PVM_IsoStraightView) {
        return;
    }
    const float interpolated_deviation_x = interpolate(local_state.camera.previous_deviation_x, local_state.camera.destination_deviation_x);
    const float interpolated_deviation_y = interpolate(local_state.camera.previous_deviation_y, local_state.camera.destination_deviation_y);
    int32_t total_deviation_x = (int32_t)interpolated_deviation_x;
    int32_t total_deviation_y = (int32_t)interpolated_deviation_y;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    if (dungeon->camera_deviate_quake != 0) {
        total_deviation_x += UNSYNC_RANDOM(80) - 40;
        total_deviation_y += UNSYNC_RANDOM(80) - 40;
    }
    if (total_deviation_x == 0 && total_deviation_y == 0) {
        return;
    }
    struct Camera* cam = &local_state.camera.current[CamIV_Isometric];
    const MapCoord max_x = (game.map_subtiles_x + 1) * COORD_PER_STL - 1;
    const MapCoord max_y = (game.map_subtiles_y + 1) * COORD_PER_STL - 1;
    cam->mappos.x.val = clamp(cam->mappos.x.val + total_deviation_x, 0, max_x);
    cam->mappos.y.val = clamp(cam->mappos.y.val + total_deviation_y, 0, max_y);
}

void interpolate_local_cameras(void)
{
    if (!local_state.camera.ready) {
        return;
    }
    float camera_fraction = 1.0f;
    if (is_feature_on(Ft_DeltaTime)) {
        camera_fraction = interpolate_time;
    }
    struct PlayerInfo *player = get_my_player();
    const int camera_idx = get_local_active_camera(player) - local_state.camera.current;
    if (game.fast_forward > 1 && (!replay.load_enable || is_observer_camera_active()) && camera_idx != CamIV_FirstPerson && !player_instance_controls_camera(player->instance_num)) {
        const float elapsed = (get_time_tick_ns() - camera_update_time) / 1e9f * turns_per_second;
        camera_fraction = clamp(elapsed, 0.0f, 1.0f);
    }
    for (int i = 0; i < CamIV_EndList; i++) {
        struct Camera* prev = &local_state.camera.previous[i];
        struct Camera* desired = &local_state.camera.destination[i];
        struct Camera* out = &local_state.camera.current[i];
        const float mappos_x = LbLerp(prev->mappos.x.val, desired->mappos.x.val, camera_fraction);
        const float mappos_y = LbLerp(prev->mappos.y.val, desired->mappos.y.val, camera_fraction);
        const float mappos_z = LbLerp(prev->mappos.z.val, desired->mappos.z.val, camera_fraction);
        const float angle_x = lerp_angle(prev->rotation_angle_x, desired->rotation_angle_x, camera_fraction);
        const float angle_y = lerp_angle(prev->rotation_angle_y, desired->rotation_angle_y, camera_fraction);
        const float angle_z = lerp_angle(prev->rotation_angle_z, desired->rotation_angle_z, camera_fraction);
        const float zoom = LbLerp(prev->zoom, desired->zoom, camera_fraction);
        out->mappos.x.val = lroundf(mappos_x);
        out->mappos.y.val = lroundf(mappos_y);
        out->mappos.z.val = lroundf(mappos_z);
        out->rotation_angle_x = lroundf(angle_x) & ANGLE_MASK;
        out->rotation_angle_y = lroundf(angle_y) & ANGLE_MASK;
        out->rotation_angle_z = lroundf(angle_z) & ANGLE_MASK;
        out->zoom = lroundf(zoom);
    }
    interpolate_camera_deviations();
}

void sync_local_camera(struct PlayerInfo *player)
{
    if (!local_camera_follows_player(player)) {
        return;
    }
    const unsigned char cam_idx = get_player_active_camera_index(player);
    if (cam_idx == CamIV_Parchment) {
        return;
    }
    if (cam_idx == CamIV_FirstPerson) {
        update_first_person_camera(&local_state.camera.destination[CamIV_FirstPerson], player, true);
        return;
    }
    sync_local_camera_pose(player, &get_player_user_state(player)->dungeon_camera);
}

/** Snaps the local dungeon cameras to a pose without any interpolation */
void sync_local_camera_pose(struct PlayerInfo *player, const struct DungeonCamera *pose)
{
    if (!local_camera_follows_player(player) || get_local_view_type(player) == PVT_MapScreen) {
        return;
    }
    struct Camera cams[CamIV_EndList];
    memcpy(cams, local_state.camera.destination, sizeof(cams));
    apply_dungeon_camera_pose(cams, pose);
    for (int i = 0; i < CamIV_EndList; i++) {
        sync_camera_state(i, &cams[i]);
    }
}

void carry_local_dungeon_position(struct PlayerInfo *player, TbBool from_front_view)
{
    if (!is_my_player(player) || !local_state.camera.ready) {
        return;
    }
    int from_idx = CamIV_Isometric;
    int to_idx = CamIV_FrontView;
    if (from_front_view) {
        from_idx = CamIV_FrontView;
        to_idx = CamIV_Isometric;
    }
    struct Camera *camera_sets[] = {local_state.camera.current, local_state.camera.previous, local_state.camera.destination};
    for (int i = 0; i < 3; i++) {
        struct Camera *cams = camera_sets[i];
        cams[to_idx].mappos.x.val = cams[from_idx].mappos.x.val;
        cams[to_idx].mappos.y.val = cams[from_idx].mappos.y.val;
        cams[to_idx].velocity_x = 0;
        cams[to_idx].velocity_y = 0;
    }
}

void record_local_possession_start(struct PlayerInfo *player)
{
    if (!is_my_player(player) || !local_state.camera.ready)
        return;
    if (is_observer_camera_active()) {
        local_state.camera.possession_start_zoom = 0;
    } else {
        local_state.camera.possession_start_zoom = local_state.camera.destination[CamIV_Isometric].zoom;
    }
}

int32_t get_local_possession_start_zoom(struct PlayerInfo *player)
{
    if (!is_my_player(player) || (local_state.camera.possession_start_zoom <= 0))
        return get_player_user_state(player)->dungeon_camera.zoom[false];
    return local_state.camera.possession_start_zoom;
}

/** possession zoom applies to local camera only */
void step_local_possession_camera(struct PlayerInfo *player, const struct Thing *thing)
{
    if (!local_camera_follows_player(player) || get_local_view_type(player) == PVT_MapScreen) {
        return;
    }
    struct Camera *cam = &local_state.camera.destination[CamIV_Isometric];
    cam->zoom = zoom_in_step(cam->zoom, 30000, 0);
    // turn toward creature's facing
    int32_t mv_a = (thing->move_angle_xy - cam->rotation_angle_x) & ANGLE_MASK;
    if (mv_a > DEGREES_180)
        mv_a -= DEGREES_360;
    mv_a = clamp(mv_a, -DEGREES_30, DEGREES_30);
    cam->rotation_angle_x = (cam->rotation_angle_x + mv_a) & ANGLE_MASK;
    // move toward a point behind the creature's eyes
    const int32_t radius = get_creature_eye_height(thing) + thing->mappos.z.val;
    const MapCoordDelta mv_x = thing->mappos.x.val + distance_with_angle_to_coord_x(radius, cam->rotation_angle_x) - (MapCoordDelta)cam->mappos.x.val;
    const MapCoordDelta mv_y = thing->mappos.y.val + distance_with_angle_to_coord_y(radius, cam->rotation_angle_x) - (MapCoordDelta)cam->mappos.y.val;
    cam->mappos.x.val += clamp(mv_x, -128, 128);
    cam->mappos.y.val += clamp(mv_y, -128, 128);
    cam->velocity_x = 0;
    cam->velocity_y = 0;
    cam->velocity_rotation = 0;
}

void set_local_camera_destination(struct PlayerInfo *player)
{
    if (!local_camera_follows_player(player) || get_local_view_type(player) == PVT_MapScreen) {
        return;
    }
    apply_dungeon_camera(local_state.camera.destination, player);
    struct Thing *ctrltng = thing_get(player->controlled_thing_idx);
    if (thing_exists(ctrltng)) {
        local_state.camera.destination[CamIV_FirstPerson].rotation_angle_x = ctrltng->move_angle_xy;
        local_state.camera.destination[CamIV_FirstPerson].rotation_angle_y = ctrltng->move_angle_z;
    }
}

static void sync_local_camera_leaving_map(struct PlayerInfo *player, const struct Packet *pckt)
{
    struct DungeonCamera pose = get_player_user_state(player)->dungeon_camera;
    if (pckt->action == PckA_ZoomFromMap)
    {
        set_view_position(&pose.x, &pose.y, subtile_coord_center(pckt->actn_par1), subtile_coord_center(pckt->actn_par2));
        pose.yaw[false] = 0;
        pose.yaw[true] = 0;
    }
    sync_local_camera_pose(player, &pose);
}

void update_local_view_prediction(const struct Packet *pckt)
{
    struct PlayerInfo *player = get_my_player();
    const TbBool on_map = (get_local_view_type(player) == PVT_MapScreen);
    if (pckt->action == PckA_ZoomFromMap) {
        local_state.camera.move_cam = NULL;
    }
    if (pckt->action == PckA_SaveViewType && pckt->actn_par1 == PVT_MapScreen) {
        local_state.view_type = PVT_MapScreen;
        toggle_status_menu(0);
    } else if ((pckt->action == PckA_LoadViewType && pckt->actn_par1 == PVT_DungeonTop)
            || (pckt->action == PckA_ZoomFromMap && pckt->actn_par3 == 0)) {
        local_state.view_type = PVT_DungeonTop;
        toggle_status_menu((game.operation_flags & GOF_ShowPanel) != 0);
        if (on_map)
            sync_local_camera_leaving_map(player, pckt);
    }
}

struct Packet *get_observer_camera_packet(void)
{
    return &observer_camera_packet;
}

TbBool is_observer_camera_active(void)
{
    return local_state.observer_camera_active;
}

void enter_observer_camera(void)
{
    if (is_observer_camera_active() || !local_state.camera.ready) {
        return;
    }
    struct PlayerInfo *player = get_my_player();
    int32_t cam_idx = CamIV_Isometric;
    if (get_player_user_state(player)->prefs[UPref_FrontView] != 0) {
        cam_idx = CamIV_FrontView;
    }
    struct Camera cams[CamIV_EndList];
    memcpy(cams, local_state.camera.destination, sizeof(cams));
    apply_dungeon_camera(cams, player);
    sync_camera_state(cam_idx, &cams[cam_idx]);
    local_state.camera.move_cam = NULL;
    memset(&observer_camera_packet, 0, sizeof(observer_camera_packet));
    local_state.observer_camera_active = true;
    local_state.observer_camera_view_type = PVT_DungeonTop;
    local_state.observer_camera_idx = cam_idx;
    observer_update_vision();
}

void return_to_player_camera(void)
{
    clicked_on_small_map = 0;
    grabbed_small_map = 0;
    if (local_state.observer_camera_active && local_state.observer_camera_view_type != PVT_DungeonTop) {
        set_observer_camera_view(PVT_DungeonTop);
    }
    local_state.observer_camera_active = false;
    if (observer_is_active()) {
        local_observer_user_state.view_type = PVT_None;
        observer_update_view();
        observer_store_packets(NULL);
    }
    init_local_cameras(get_my_player());
}

void set_observer_camera_view(unsigned char view_type)
{
    if (!is_observer_camera_active()) {
        return;
    }
    if (view_type != PVT_DungeonTop && view_type != PVT_MapScreen) {
        return;
    }
    local_state.camera.move_cam = NULL;
    memset(&observer_camera_packet, 0, sizeof(observer_camera_packet));
    local_state.camera_movement_x = 0.0f;
    local_state.camera_movement_y = 0.0f;
    local_state.observer_camera_view_type = view_type;
    toggle_status_menu(view_type == PVT_DungeonTop && (game.operation_flags & GOF_ShowGui) != 0);
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
}

void observer_camera_jump(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (!is_observer_camera_active()) {
        return;
    }
    struct Packet jump;
    memset(&jump, 0, sizeof(jump));
    jump.action = PckA_ZoomFromMap;
    jump.actn_par1 = stl_x;
    jump.actn_par2 = stl_y;
    process_camera_action(local_state.camera.destination, &jump);
    memcpy(local_state.camera.previous, local_state.camera.destination, sizeof(local_state.camera.previous));
    memcpy(local_state.camera.current, local_state.camera.destination, sizeof(local_state.camera.current));
    set_observer_camera_view(PVT_DungeonTop);
}

unsigned char get_local_view_type(const struct PlayerInfo *player)
{
    if (is_observer_camera_active() && is_my_player(player)) {
        return local_state.observer_camera_view_type;
    }
    if (!is_my_player(player) || local_state.view_type == PVT_None) {
        return get_player_view_type(player);
    }
    if (local_state.view_type == PVT_MapScreen || get_player_view_type(player) == PVT_MapScreen) {
        return local_state.view_type;
    }
    return get_player_view_type(player);
}

static int get_local_active_camera_index(struct PlayerInfo *player)
{
    if (!is_my_player(player) || !local_state.camera.ready)
        return get_player_active_camera_index(player);
    return get_local_active_camera(player) - local_state.camera.current;
}

struct Camera* get_local_active_camera(struct PlayerInfo *player)
{
    if (!is_my_player(player) || !local_state.camera.ready) {
        return &local_state.camera.current[get_player_active_camera_index(player)];
    }
    if (is_observer_camera_active()) {
        if (local_state.observer_camera_view_type == PVT_MapScreen) {
            return &local_state.camera.current[CamIV_Parchment];
        }
        return &local_state.camera.current[local_state.observer_camera_idx];
    }
    unsigned char view_type = get_local_view_type(player);
    if (view_type == PVT_MapScreen) {
        return &local_state.camera.current[CamIV_Parchment];
    }
    if (view_type == PVT_DungeonTop && get_player_view_type(player) == PVT_MapScreen) {
        return get_local_dungeon_camera(player);
    }
    return &local_state.camera.current[get_player_active_camera_index(player)];
}

static struct Camera *get_local_dungeon_camera(struct PlayerInfo *player)
{
    return &local_state.camera.current[(get_player_user_state(player)->prefs[UPref_FrontView] != 0) ? CamIV_FrontView : CamIV_Isometric];
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
