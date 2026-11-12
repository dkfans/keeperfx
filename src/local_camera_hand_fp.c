/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file local_camera_hand_fp.c
 *     Local first-person camera while carried in a co-user's hand (archon).
 * @par Purpose:
 *     Animates the possessor's view when grabbed by co-user.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "local_camera_hand_fp.h"

#include "bflib_math.h"
#include "bflib_planar.h"
#include "engine_camera.h"
#include "engine_render.h"
#include "game_legacy.h"
#include "player_data.h"
#include "power_hand_fp.h"

#include <math.h>
#include "post_inc.h"

/******************************************************************************/
static int32_t signed_angle(int32_t angle)
{
    angle &= ANGLE_MASK;
    return (angle > DEGREES_180) ? (angle - DEGREES_360) : angle;
}

static int32_t lerp_pitch(int32_t from, int32_t to, float ease)
{
    return lroundf(LbLerp(signed_angle(from), signed_angle(to), ease)) & ANGLE_MASK;
}

enum FirstPersonHandPhase get_local_fp_hand_phase(const struct Thing *ctrltng, int32_t *phase_turn)
{
    return get_fp_hand_phase(get_local_user(), ctrltng, phase_turn);
}

/** Whether the view's position comes from the hand rather than from the creature. */
TbBool fp_hand_positions_view(enum FirstPersonHandPhase phase, int32_t phase_turn)
{
    return (phase == FPHand_Squish) || (phase == FPHand_Lift) || (phase == FPHand_Held)
        || ((phase == FPHand_Drop) && (phase_turn < FP_HAND_DROP_SETTLE_TURNS));
}

/** Record view at start of hand held animation */
static void start_local_hand_camera(const struct Camera *cam, enum FirstPersonHandPhase phase)
{
    const struct UserState *ustate = get_local_user_state();
    struct LocalCameraState *lcam = &local_state.camera;
    if (phase == FPHand_Drop) {
        if (lcam->fp_hand_active && (lcam->fp_hand_drop_turn == ustate->fp_hand_drop_turn)) {
            return;
        }
        lcam->fp_hand_active = true;
        lcam->fp_hand_drop_turn = ustate->fp_hand_drop_turn;
        lcam->fp_hand_drop_start_pitch = cam->rotation_angle_y;
        lcam->first_person_look_pitch = 0;
        return;
    }
    if (lcam->fp_hand_active && (lcam->fp_hand_pickup_turn == ustate->fp_hand_pickup_turn)) {
        return;
    }
    lcam->fp_hand_active = true;
    lcam->fp_hand_pickup_turn = ustate->fp_hand_pickup_turn;
    lcam->fp_hand_start_pitch = cam->rotation_angle_y;
    lcam->first_person_look_pitch = FP_HAND_SQUISH_PITCH & ANGLE_MASK;
}

static void animate_local_hand_camera(struct Camera *cam, const struct Thing *ctrltng, enum FirstPersonHandPhase phase, int32_t phase_turn)
{
    const struct UserState *ustate = get_local_user_state();
    struct LocalCameraState *lcam = &local_state.camera;
    const int32_t true_pitch = replay.load_enable ? ctrltng->move_angle_z : lcam->first_person_look_pitch;
    switch (phase)
    {
    case FPHand_Squish:
    {
        const float t = (float)(phase_turn + 1) / (FP_HAND_SQUISH_TURNS + 1);
        cam->rotation_angle_y = lerp_pitch(lcam->fp_hand_start_pitch, FP_HAND_SQUISH_PITCH, 1.0f - (1.0f - t) * (1.0f - t));
        break;
    }
    case FPHand_Drop:
        if (phase_turn < FP_HAND_DROP_DIP_TURNS) {
            const float t = (float)(phase_turn + 1) / (FP_HAND_DROP_DIP_TURNS + 1);
            cam->rotation_angle_y = lerp_pitch(lcam->fp_hand_drop_start_pitch, FP_HAND_SQUISH_PITCH, 1.0f - (1.0f - t) * (1.0f - t));
        } else {
            const float t = min(1.0f, (float)(phase_turn - FP_HAND_DROP_DIP_TURNS) / (FP_HAND_DROP_PITCH_TURNS - FP_HAND_DROP_DIP_TURNS));
            cam->rotation_angle_y = lerp_pitch(FP_HAND_SQUISH_PITCH, true_pitch, (1.0f - cosf((float)M_PI * t)) / 2.0f);
        }
        break;
    case FPHand_Lift:
    case FPHand_Held:
    default:
        cam->rotation_angle_y = clamp_fp_hand_pitch(true_pitch);
        break;
    }
    if (fp_hand_positions_view(phase, phase_turn)) {
        view_set_camera_position(cam, ustate->fp_hand_x, ustate->fp_hand_y);
        cam->mappos.z.val = get_fp_hand_view_height(get_local_user(), ctrltng, NULL);
        cam->rotation_angle_z = 0;
    }
}

#define FP_HAND_YAW_DRIFT_DEGREES        0.3f
#define FP_HAND_YAW_DRIFT_PERIOD_SECONDS 6.0f
#define FP_HAND_PITCH_DRIFT_RATE           0.025f
#define FP_HAND_PITCH_DRIFT_DEGREES        5.0f
#define FP_HAND_PITCH_DRIFT_PERIOD_SECONDS 4.1f
#define FP_HAND_DRIFT_FADE_TURNS 8
#define ANGLE_UNITS_PER_DEGREE ((float)DEGREES_180 / 180.0f)

static float fp_hand_drift_strength(enum FirstPersonHandPhase phase, int32_t phase_turn)
{
    switch (phase)
    {
    case FPHand_Lift:
        return (float)phase_turn / FP_HAND_LIFT_TURNS;
    case FPHand_Held:
        return 1.0f;
    case FPHand_Drop:
        return max(0.0f, 1.0f - (float)phase_turn / FP_HAND_DRIFT_FADE_TURNS);
    default:
        return 0.0f;
    }
}

static void apply_fp_hand_look_drift(const struct Thing *ctrltng)
{
    struct LocalCameraState *lcam = &local_state.camera;
    int32_t phase_turn;
    const enum FirstPersonHandPhase phase = get_local_fp_hand_phase(ctrltng, &phase_turn);
    const float strength = fp_hand_drift_strength(phase, phase_turn);
    if (strength <= 0.0f) {
        lcam->fp_hand_drift_yaw_applied = 0;
        lcam->fp_hand_drift_pitch_residual = 0.0f;
        return;
    }
    const float tps = (float)max(turns_per_second, 1);
    const float turn = (float)get_gameturn();

    const float yaw_period = FP_HAND_YAW_DRIFT_PERIOD_SECONDS * tps;
    const float sway = strength * FP_HAND_YAW_DRIFT_DEGREES * ANGLE_UNITS_PER_DEGREE * yaw_period / (float)M_TAU
        * sinf((float)M_TAU * fmodf(turn, yaw_period) / yaw_period);
    const int32_t sway_units = lroundf(sway);
    lcam->first_person_look_yaw = (lcam->first_person_look_yaw + sway_units - lcam->fp_hand_drift_yaw_applied) & ANGLE_MASK;
    lcam->fp_hand_drift_yaw_applied = sway_units;
    if ((phase == FPHand_Drop) && (phase_turn < FP_HAND_DROP_DIP_TURNS)) {
        lcam->fp_hand_drift_pitch_residual = 0.0f;
        return;
    }
    const float pitch_period = FP_HAND_PITCH_DRIFT_PERIOD_SECONDS * tps;
    const float target = (float)FP_HAND_SQUISH_PITCH
        + FP_HAND_PITCH_DRIFT_DEGREES * ANGLE_UNITS_PER_DEGREE * sinf((float)M_TAU * fmodf(turn, pitch_period) / pitch_period);
    int32_t pitch = lcam->first_person_look_pitch & ANGLE_MASK;
    if (pitch > DEGREES_180) {
        pitch -= DEGREES_360;
    }
    float drifted = pitch + lcam->fp_hand_drift_pitch_residual;
    drifted += (target - drifted) * FP_HAND_PITCH_DRIFT_RATE * strength;
    const int32_t drifted_units = lroundf(drifted);
    lcam->fp_hand_drift_pitch_residual = drifted - drifted_units;
    lcam->first_person_look_pitch = drifted_units & ANGLE_MASK;
}

/* torque as possessed creature is dragged around calculated like so:
   after being displaced a distance *d* due to hand movement, consider the point *p* 2 slabs away where the user is looking. Change the user's yaw to look at the point p-d.
   (if d exceeds 1.5 in magnitude, normalize to 1.5.)
 */
#define FP_HAND_STARE_DISTANCE_SLABS 2
#define FP_HAND_STARE_MAX_SHIFT_SLABS 1.5f

static void apply_fp_hand_look_follow(const struct Thing *ctrltng)
{
    struct LocalCameraState *lcam = &local_state.camera;
    const struct UserState *ustate = get_local_user_state();
    const enum FirstPersonHandPhase phase = get_local_fp_hand_phase(ctrltng, NULL);
    if ((phase != FPHand_Squish) && (phase != FPHand_Lift) && (phase != FPHand_Held)) {
        lcam->fp_hand_track_valid = false;
        return;
    }
    const MapCoord x = ustate->fp_hand_x;
    const MapCoord y = ustate->fp_hand_y;
    if (!lcam->fp_hand_track_valid) {
        lcam->fp_hand_track_valid = true;
        lcam->fp_hand_track_x = x;
        lcam->fp_hand_track_y = y;
        return;
    }
    float dx = (float)(x - lcam->fp_hand_track_x);
    float dy = (float)(y - lcam->fp_hand_track_y);
    lcam->fp_hand_track_x = x;
    lcam->fp_hand_track_y = y;
    if ((dx == 0.0f) && (dy == 0.0f)) {
        return;
    }
    const float max_shift = FP_HAND_STARE_MAX_SHIFT_SLABS * COORD_PER_SLB;
    const float shift = sqrtf(dx * dx + dy * dy);
    if (shift > max_shift) {
        dx *= max_shift / shift;
        dy *= max_shift / shift;
    }
    const int32_t yaw = lcam->first_person_look_yaw;
    const long stare_x = move_coord_with_angle_x(x, FP_HAND_STARE_DISTANCE_SLABS * COORD_PER_SLB, yaw);
    const long stare_y = move_coord_with_angle_y(y, FP_HAND_STARE_DISTANCE_SLABS * COORD_PER_SLB, yaw);
    lcam->first_person_look_yaw = LbArcTanAngle(lroundf(stare_x - dx) - x, lroundf(stare_y - dy) - y) & ANGLE_MASK;
}

/** Starts the carried view animation when its phase changes; returns the phase. */
enum FirstPersonHandPhase start_local_fp_hand_camera(const struct Camera *cam, const struct Thing *ctrltng, int32_t *phase_turn)
{
    const enum FirstPersonHandPhase phase = get_local_fp_hand_phase(ctrltng, phase_turn);
    if (phase == FPHand_None) {
        local_state.camera.fp_hand_active = false;
    } else {
        start_local_hand_camera(cam, phase);
    }
    return phase;
}

/** Applies the carried view's pitch and position on top of the ordinary first-person camera, and sets what the renderer may skip. */
void update_local_fp_hand_camera(struct Camera *cam, const struct Thing *ctrltng, enum FirstPersonHandPhase phase, int32_t phase_turn)
{
    if (phase != FPHand_None) {
        animate_local_hand_camera(cam, ctrltng, phase, phase_turn);
    }
    // ceiling visible iff not being held in hand; the view distance is capped over the same turns
    const TbBool overhead = (phase == FPHand_Lift) || (phase == FPHand_Held)
        || ((phase == FPHand_Drop) && (phase_turn < FP_HAND_DROP_DIP_TURNS));
    engine_view_above_walls = overhead;
    engine_hide_ceiling = overhead;
    engine_view_distance_limit = overhead ? FP_HAND_VIEW_DISTANCE_SUBTILES : 0;
}

/** Drift and look-follow applied to the local look while carried. */
void apply_local_fp_hand_look(const struct Thing *ctrltng)
{
    apply_fp_hand_look_drift(ctrltng);
    apply_fp_hand_look_follow(ctrltng);
}

/******************************************************************************/
