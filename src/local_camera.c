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
#include "net_exchange_gameplay.h"
#include "packets.h"
#include "player_data.h"
#include "config_creature.h"
#include "thing_creature.h"
#include "game_legacy.h"
#include "dungeon_data.h"
#include "map_data.h"
#include "bflib_math.h"
#include "frontmenu_ingame_map.h"
#include "frontend.h"
#include "gui_parchment.h"

#include <math.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static struct Camera local_cameras[CamIV_EndList];
static struct Camera previous_local_cameras[CamIV_EndList];
static struct Camera destination_local_cameras[CamIV_EndList];
static float previous_deviation_x;
static float previous_deviation_y;
static float destination_deviation_x;
static float destination_deviation_y;
static TbBool local_camera_ready;
static MapCoord local_camera_move_target[2];
static MapCoordDelta local_camera_move_delta[2];
static struct Camera *local_camera_move_cam;
static struct Packet freecam_packet;
/******************************************************************************/

static TbBool replay_is_detached(void)
{
    return game.packet_load_enable && local_state.replay_detached;
}

static void forget_camera_plan(void)
{
    local_state.camera_plan_dir_x = 0;
    local_state.camera_plan_dir_y = 0;
    local_state.camera_plan_cam_idx = -1;
}

static int32_t error_after_brake(int32_t err, int32_t vel, int32_t prev, int32_t accel)
{
    if (accel <= 0)
        return err;
    while (vel != 0) {
        const int32_t dir = (vel > 0) ? -1 : 1;
        int32_t brake = (prev == dir) ? CAMERA_AXIS_FAST_MULT * accel : accel;
        if (brake > abs(vel))
            brake = accel;
        if (brake > abs(vel))
            break;
        vel += dir * brake;
        prev = dir;
        err -= vel;
    }
    return err;
}

static int32_t plan_camera_axis(int32_t err, int32_t vel, int32_t prev, int32_t accel, TbBool *fast)
{
    static const signed char dirs[] = {1, 1, 0, -1, -1};
    static const signed char mults[] = {CAMERA_AXIS_FAST_MULT, 1, 0, 1, CAMERA_AXIS_FAST_MULT};
    const int32_t toward = (err >= 0) ? 1 : -1;
    int32_t best_dir = 0;
    int64_t best_cost = INT64_MAX;
    *fast = false;
    for (int i = 0; i < (int)(sizeof(dirs) / sizeof(dirs[0])); i++) {
        const int32_t dir = toward * dirs[i];
        const TbBool is_fast = (mults[i] == CAMERA_AXIS_FAST_MULT);
        if (is_fast && (prev != dir))
            continue;
        const int32_t new_vel = vel + dir * mults[i] * accel;
        const int32_t left = error_after_brake(err - new_vel, new_vel, dir, accel);
        const int64_t cost = (left * toward < 0) ? 2 * (int64_t)abs(left) : abs(left);
        if (cost < best_cost) {
            best_cost = cost;
            best_dir = dir;
            *fast = is_fast;
        }
    }
    return best_dir;
}

void camera_packet_plan_motion(void)
{
    // The local camera can flit about with alacrity, but we only get 4 bits
    // per turn to convey this position approximately over the network.
    //
    // The camera position only really matters for replays and a few other things.
    // 
    // We use a simple momentum-based approach and some math to prevent overshooting.

    if (game.packet_load_enable) {
        return;
    }
    if (!local_camera_ready) {
        forget_camera_plan();
        return;
    }
    struct PlayerInfo* player = get_my_player();
    if (get_local_view_type(player) != player->view_type) {
        forget_camera_plan();
        return;
    }

    int cam_idx;
    switch (get_local_active_camera(player)->view_mode)
    {
    case PVM_FrontView:
        cam_idx = CamIV_FrontView;
        break;

    case PVM_IsoStraightView:
    case PVM_IsoWibbleView:
        cam_idx = CamIV_Isometric;
        break;

    default:
        forget_camera_plan();
        return;
    }

    if (local_state.camera_plan_cam_idx != cam_idx) {
        forget_camera_plan();
    }

    struct Camera* local_cam = &destination_local_cameras[cam_idx];
    struct Camera* packet_cam = &player->cameras[cam_idx];
    struct Packet* pckt = get_local_packet();

    // if local camera has a move-destination, aim for that instead
    MapCoord aimx = local_cam->mappos.x.val;
    MapCoord aimy = local_cam->mappos.y.val;
    if (local_camera_move_cam == local_cam) {
        aimx = local_camera_move_target[0];
        aimy = local_camera_move_target[1];
    }

    int32_t dmx = aimx - packet_cam->mappos.x.val;
    int32_t dmy = aimy - packet_cam->mappos.y.val;

    int32_t angle = local_cam->rotation_angle_x;
    int32_t cos_angle = LbCosL(angle);
    int32_t sin_angle = LbSinL(angle);
    int32_t dcr = ((int64_t)dmx * cos_angle + (int64_t)dmy * sin_angle) >> 16;
    int32_t dcf = (-(int64_t)dmx * sin_angle + (int64_t)dmy * cos_angle) >> 16;

    const int32_t rate = camera_move_rate(packet_cam, player, false);

    TbBool fast_r;
    TbBool fast_f;

    // if there's a mismatch here, play it safe and use 1x
    const int32_t prev_r = (local_state.camera_plan_dir_x == packet_cam->last_move_dir_x) ? local_state.camera_plan_dir_x : 0;
    const int32_t prev_f = (local_state.camera_plan_dir_y == packet_cam->last_move_dir_y) ? local_state.camera_plan_dir_y : 0;
    const int32_t dir_r = plan_camera_axis(dcr, packet_cam->velocity_x, prev_r, rate / 4, &fast_r);
    const int32_t dir_f = plan_camera_axis(dcf, packet_cam->velocity_y, prev_f, rate / 4, &fast_f);

    if (fast_r) {
        set_packet_control(pckt, PCtr_MoveLeft | PCtr_MoveRight);
    } else if (dir_r > 0) {
        set_packet_control(pckt, PCtr_MoveRight);
    } else if (dir_r < 0) {
        set_packet_control(pckt, PCtr_MoveLeft);
    }
    if (fast_f) {
        set_packet_control(pckt, PCtr_MoveUp | PCtr_MoveDown);
    } else if (dir_f > 0) {
        set_packet_control(pckt, PCtr_MoveDown);
    } else if (dir_f < 0) {
        set_packet_control(pckt, PCtr_MoveUp);
    }
    local_state.camera_plan_dir_x = dir_r;
    local_state.camera_plan_dir_y = dir_f;
    local_state.camera_plan_cam_idx = cam_idx;
}

static void sync_camera_state(int cam_idx, struct Camera *cam)
{
    local_cameras[cam_idx] = *cam;
    destination_local_cameras[cam_idx] = *cam;
    previous_local_cameras[cam_idx] = *cam;
}

static void sync_first_person_camera(struct Camera *cam, struct PlayerInfo *player)
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
    for (int i = 0; i < CamIV_EndList; i++) {
        sync_camera_state(i, &player->cameras[i]);
    }
    local_camera_move_cam = NULL;
    local_camera_ready = true;
}

void move_local_camera_to_position(MapCoord x, MapCoord y)
{
    if (!local_camera_ready) {
        return;
    }
    int cam_idx = get_local_active_camera(get_my_player()) - local_cameras;
    struct Camera *cam = &destination_local_cameras[cam_idx];
    local_camera_move_cam = cam;
    local_camera_move_target[0] = x;
    local_camera_move_target[1] = y;
    view_set_camera_move_to_position(cam, x, y, &local_camera_move_delta[0], &local_camera_move_delta[1]);
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

static void update_local_first_person_camera(struct Thing *ctrltng, const struct Packet *pckt)
{
    struct Camera* cam = &destination_local_cameras[CamIV_FirstPerson];
    int eye_height = get_creature_eye_height(ctrltng);
    update_first_person_position(cam, ctrltng, eye_height);

    if ((flag_is_set(game.operation_flags, GOF_Paused) && game.game_kind != GKind_LocalGame)
        || ! can_process_creature_input(ctrltng))
    {
        cam->rotation_angle_x = ctrltng->move_angle_xy;
        cam->rotation_angle_y = ctrltng->move_angle_z;
        return;
    }

    if (pckt == NULL) {
        return;
    }

    const long current_horizontal = cam->rotation_angle_x;
    const long current_vertical = cam->rotation_angle_y;
    long new_horizontal, new_vertical, new_roll;
    process_first_person_look(ctrltng, pckt, current_horizontal, current_vertical, &new_horizontal, &new_vertical, &new_roll);
    cam->rotation_angle_x = new_horizontal;
    cam->rotation_angle_y = new_vertical;
    if ((ctrltng->movement_flags & TMvF_Flying) != 0) {
        cam->rotation_angle_z = new_roll;
    }
}

void update_local_cameras(void)
{
    if (!local_camera_ready) {
        return;
    }
    struct PlayerInfo *player = get_my_player();
    struct Thing *ctrltng = thing_get(player->controlled_thing_idx);
    const struct Packet *pckt = get_history_packet(get_local_user(), get_gameturn());
    previous_deviation_x = destination_deviation_x;
    previous_deviation_y = destination_deviation_y;
    destination_deviation_x = 0;
    destination_deviation_y = 0;

    memcpy(previous_local_cameras, destination_local_cameras, sizeof(previous_local_cameras));
    if (replay_is_detached()) {
        if (local_state.replay_view_type == PVT_DungeonTop) {
            struct Camera *cam = &destination_local_cameras[local_state.replay_cam_idx];
            process_local_camera_movement(cam, player);
            process_camera_view_controls(cam, &freecam_packet, player);
            view_process_camera_velocity(cam);
        }
        local_state.camera_movement_x = 0.0f;
        local_state.camera_movement_y = 0.0f;
        memset(&freecam_packet, 0, sizeof(freecam_packet));
        return;
    }
    if (pckt != NULL) {
        process_camera_action(destination_local_cameras, pckt);
        // Skip interpolation for parchment jumps, while retaining it for minimap dragging.
        if (pckt->action == PckA_ZoomFromMap) {
            memcpy(previous_local_cameras, destination_local_cameras, sizeof(previous_local_cameras));
        }
    }

    int active_cam_idx = get_local_active_camera(player) - local_cameras;
    if (active_cam_idx == CamIV_FirstPerson && thing_exists(ctrltng)) {
        update_local_first_person_camera(ctrltng, pckt);
        return;
    }
    if (pckt == NULL) {
        return;
    }

    struct Camera *cam = &destination_local_cameras[active_cam_idx];
    if (active_cam_idx == CamIV_Parchment) {
        cam->mappos.x.val = player->cameras[CamIV_Parchment].mappos.x.val;
        cam->mappos.y.val = player->cameras[CamIV_Parchment].mappos.y.val;
    }
    if (local_camera_move_cam != NULL) {
        if (view_move_camera_to_position(local_camera_move_cam, local_camera_move_target[0], local_camera_move_target[1], local_camera_move_delta[0], local_camera_move_delta[1])) {
            local_camera_move_cam = NULL;
        }
    }
    if (local_camera_move_cam != cam) {
        // Same as the packet camera: a parchment map jump ignores the packet's camera controls.
        if (pckt->action != PckA_ZoomFromMap) {
            if (!game.packet_load_enable && cam->view_mode != PVM_ParchmentView) {
                process_local_camera_movement(cam, player);
                process_camera_view_controls(cam, pckt, player);
            } else {
                process_camera_controls(cam, pckt, player);
            }
            local_state.camera_movement_x = 0.0f;
            local_state.camera_movement_y = 0.0f;
        }
        view_process_camera_velocity(cam);
    }

    if (active_cam_idx == CamIV_Isometric) {
        struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
        if (dungeon->camera_deviate_jump != 0) {
            int32_t angle = cam->rotation_angle_x;
            destination_deviation_x += ( (dungeon->camera_deviate_jump * LbSinL(angle) >> 8) >> 8);
            destination_deviation_y += (-(dungeon->camera_deviate_jump * LbCosL(angle) >> 8) >> 8);
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
    const float interpolated_deviation_x = interpolate(previous_deviation_x, destination_deviation_x);
    const float interpolated_deviation_y = interpolate(previous_deviation_y, destination_deviation_y);
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
    struct Camera* cam = &local_cameras[CamIV_Isometric];
    const MapCoord max_x = (game.map_subtiles_x + 1) * COORD_PER_STL - 1;
    const MapCoord max_y = (game.map_subtiles_y + 1) * COORD_PER_STL - 1;
    cam->mappos.x.val = clamp(cam->mappos.x.val + total_deviation_x, 0, max_x);
    cam->mappos.y.val = clamp(cam->mappos.y.val + total_deviation_y, 0, max_y);
}

void interpolate_local_cameras(void)
{
    if (!local_camera_ready) {
        return;
    }
    for (int i = 0; i < CamIV_EndList; i++) {
        struct Camera* prev = &previous_local_cameras[i];
        struct Camera* desired = &destination_local_cameras[i];
        struct Camera* out = &local_cameras[i];
        const float mappos_x = interpolate(prev->mappos.x.val, desired->mappos.x.val);
        const float mappos_y = interpolate(prev->mappos.y.val, desired->mappos.y.val);
        const float mappos_z = interpolate(prev->mappos.z.val, desired->mappos.z.val);
        const float angle_x = interpolate_angle(prev->rotation_angle_x, desired->rotation_angle_x);
        const float angle_y = interpolate_angle(prev->rotation_angle_y, desired->rotation_angle_y);
        const float angle_z = interpolate_angle(prev->rotation_angle_z, desired->rotation_angle_z);
        const float zoom = interpolate(prev->zoom, desired->zoom);
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
    if (!is_my_player(player) || !local_camera_ready) {
        return;
    }
    struct Camera *camera = get_player_active_camera(player);
    if (camera == &player->cameras[CamIV_Parchment] || player->view_mode == PVM_ParchmentView) {
        return;
    }
    if (camera == &player->cameras[CamIV_FirstPerson]) {
        sync_first_person_camera(camera, player);
        return;
    }
    for (int cam_idx = 0; cam_idx < CamIV_EndList; cam_idx++) {
        sync_camera_state(cam_idx, &player->cameras[cam_idx]);
    }
}

void set_local_camera_destination(struct PlayerInfo *player)
{
    if (!is_my_player(player) || !local_camera_ready || get_local_view_type(player) == PVT_MapScreen) {
        return;
    }
    memcpy(destination_local_cameras, player->cameras, sizeof(destination_local_cameras));
    struct Thing *ctrltng = thing_get(player->controlled_thing_idx);
    if (thing_exists(ctrltng)) {
        destination_local_cameras[CamIV_FirstPerson].rotation_angle_x = ctrltng->move_angle_xy;
        destination_local_cameras[CamIV_FirstPerson].rotation_angle_y = ctrltng->move_angle_z;
    }
}

void update_local_view_prediction(const struct Packet *pckt)
{
    if (pckt->action == PckA_ZoomFromMap) {
        local_camera_move_cam = NULL;
    }
    if (pckt->action == PckA_SaveViewType && pckt->actn_par1 == PVT_MapScreen) {
        local_state.view_type = PVT_MapScreen;
        toggle_status_menu(0);
    } else if ((pckt->action == PckA_LoadViewType && pckt->actn_par1 == PVT_DungeonTop)
            || (pckt->action == PckA_ZoomFromMap && !parchment_map_fade_enabled())) {
        local_state.view_type = PVT_DungeonTop;
        toggle_status_menu((game.operation_flags & GOF_ShowPanel) != 0);
    }
}

struct Packet *get_freecam_packet(void)
{
    return &freecam_packet;
}

TbBool replay_camera_detached(void)
{
    return replay_is_detached();
}

void replay_detach(void)
{
    if (replay_is_detached() || !local_camera_ready) {
        return;
    }
    struct PlayerInfo *player = get_my_player();
    int cam_idx = get_player_active_camera(player) - player->cameras;
    if ((cam_idx != CamIV_Isometric) && (cam_idx != CamIV_FrontView)) {
        cam_idx = (player->view_mode_restore == PVM_FrontView) ? CamIV_FrontView : CamIV_Isometric;
    }
    sync_camera_state(cam_idx, &player->cameras[cam_idx]);
    local_camera_move_cam = NULL;
    memset(&freecam_packet, 0, sizeof(freecam_packet));
    local_state.replay_detached = true;
    local_state.replay_view_type = PVT_DungeonTop;
    local_state.replay_cam_idx = cam_idx;
}

void replay_attach(void)
{
    if (!local_state.replay_detached) {
        return;
    }
    if (local_state.replay_view_type == PVT_MapScreen) {
        replay_freecam_set_map(false);
    }
    local_state.replay_detached = false;
    init_local_cameras(get_my_player());
}

void replay_freecam_set_map(TbBool on)
{
    if (!replay_is_detached()) {
        return;
    }
    local_state.replay_view_type = on ? PVT_MapScreen : PVT_DungeonTop;
    toggle_status_menu(on ? 0 : ((game.operation_flags & GOF_ShowPanel) != 0));
}

void replay_freecam_jump(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (!replay_is_detached()) {
        return;
    }
    struct Packet jump;
    memset(&jump, 0, sizeof(jump));
    jump.action = PckA_ZoomFromMap;
    jump.actn_par1 = stl_x;
    jump.actn_par2 = stl_y;
    process_camera_action(destination_local_cameras, &jump);
    memcpy(previous_local_cameras, destination_local_cameras, sizeof(previous_local_cameras));
    memcpy(local_cameras, destination_local_cameras, sizeof(local_cameras));
    replay_freecam_set_map(false);
}

unsigned char get_local_view_type(const struct PlayerInfo *player)
{
    if (replay_is_detached() && is_my_player(player)) {
        return local_state.replay_view_type;
    }
    if (!is_my_player(player) || local_state.view_type == PVT_None) {
        return player->view_type;
    }
    if (local_state.view_type == PVT_MapScreen || player->view_type == PVT_MapScreen) {
        return local_state.view_type;
    }
    return player->view_type;
}

struct Camera* get_local_active_camera(struct PlayerInfo *player)
{
    struct Camera *camera = get_player_active_camera(player);
    if (camera == NULL || !is_my_player(player) || !local_camera_ready) {
        return camera;
    }
    if (replay_is_detached()) {
        if (local_state.replay_view_type == PVT_MapScreen) {
            return &local_cameras[CamIV_Parchment];
        }
        return &local_cameras[local_state.replay_cam_idx];
    }
    unsigned char view_type = get_local_view_type(player);
    if (view_type == PVT_MapScreen) {
        return &local_cameras[CamIV_Parchment];
    }
    if (view_type == PVT_DungeonTop && player->view_type == PVT_MapScreen) {
        if (player->view_mode_restore == PVM_FrontView) {
            return &local_cameras[CamIV_FrontView];
        }
        return &local_cameras[CamIV_Isometric];
    }
    return &local_cameras[camera - player->cameras];
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
