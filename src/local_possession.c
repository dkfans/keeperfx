/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file local_possession.c
 *     Client-side prediction of the local player's possessed creature.
 * @par Purpose:
 *     Replays the local inputs which the game hasn't processed yet on a local
 *     copy of the possessed creature, so possession responds without input lag.
 *     The copy is never part of the synced game state.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     20 Oct 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "local_possession.h"
#include "creature_control.h"
#include "creature_graphics.h"
#include "creature_instances.h"
#include "creature_states.h"
#include "engine_render.h"
#include "game_legacy.h"
#include "net_exchange_gameplay.h"
#include "net_input_lag.h"
#include "observer.h"
#include "packets.h"
#include "player_data.h"
#include "sounds.h"
#include "thing_creature.h"
#include "thing_data.h"
#include "thing_physics.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define POSSESSION_CORRECTION_RETAIN_PERCENT 75
#define POSSESSION_CORRECTION_MIN_SNAP_DISTANCE (2 * COORD_PER_STL)
#define POSSESSION_CORRECTION_SNAP_DISTANCE_PER_LAG_TURN (COORD_PER_STL / 2)
#define POSSESSION_SHOT_OFFSETS_COUNT (MAXIMUM_INPUT_LAG_TURNS + 1)

struct PossessionPrediction {
    struct Thing thing;
    struct CreatureControl cctrl;
    GameTurn turn;
    GameTurn applied_packet_turn;
    GameTurn shot_turn;
    GameTurn footstep_turn;
    ThingModel shot_model;
    TbBool valid;
};

struct PossessionSchedule {
    GameTurn first_packet_turn;
    GameTurn last_packet_turn;
    GameTurn end_turn;
    int32_t lag;
};

struct PossessionShotOffset {
    int32_t turn;
    int32_t fade_turns;
    struct CoordDelta3d offset;
};

static struct PossessionPrediction prediction;
static struct PossessionPrediction continuation;
static struct CoordDelta3d correction;
static struct PossessionShotOffset shot_offsets[POSSESSION_SHOT_OFFSETS_COUNT];
static struct Thing possession_view;
static TbBool possession_pause_pending;
static GameTurn sounded_shot_turn;
static GameTurn sounded_footstep_turn;
/******************************************************************************/
static TbBool turn_pauses_game(GameTurn turn)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        const struct Packet *pckt = get_history_packet(user, turn);
        if ((pckt != NULL) && ((pckt->action == PckA_TogglePause) || (pckt->action == PckA_UpdatePause)) && (pckt->actn_par1 != 0)) {
            return true;
        }
    }
    return false;
}

static void get_possession_schedule(struct PossessionSchedule *schedule)
{
    const GameTurn next_turn = get_gameturn();
    possession_pause_pending = false;
    schedule->lag = game.input_lag_turns;
    schedule->first_packet_turn = input_lag_first_pending_turn();
    schedule->last_packet_turn = next_turn - 1;
    if (flag_is_set(game.operation_flags, GOF_Paused)) {
        schedule->last_packet_turn = next_turn;
    }
    schedule->end_turn = schedule->last_packet_turn + schedule->lag + 1;
    for (GameTurn turn = schedule->first_packet_turn; (GameTurnDelta)(turn - schedule->last_packet_turn) <= 0; turn++) {
        if (turn_pauses_game(turn)) {
            schedule->last_packet_turn = turn - 1;
            schedule->end_turn = turn + 2 * schedule->lag + 1;
            possession_pause_pending = true;
            return;
        }
    }
}

static void start_predicted_instance(struct PossessionPrediction *pred, CrInstance inst_id, const struct Packet *pckt, TbBool allow_unavailable)
{
    if (!possession_instance_can_start(&pred->thing, &pred->cctrl, inst_id, pred->turn, pckt, allow_unavailable)) {
        return;
    }
    const struct InstanceInfo *inst_inf = creature_instance_info_get(inst_id);
    const ThingIndex target_idx = get_human_controlled_creature_target(&pred->thing, inst_id, pckt);
    if (creature_instance_info_invalid(inst_inf) || (inst_inf->time == -1)
     || (flag_is_set(inst_inf->instance_property_flags, InstPF_NeedsTarget) && (target_idx == 0))) {
        return;
    }
    int32_t itime;
    int32_t aitime;
    get_creature_instance_times(&pred->thing, inst_id, &itime, &aitime);
    pred->cctrl.instance_id = inst_id;
    pred->cctrl.targtng_idx = target_idx;
    pred->cctrl.inst_turn = 0;
    pred->cctrl.inst_total_turns = itime;
    pred->cctrl.inst_action_turns = aitime;
}

static void advance_predicted_instance(struct PossessionPrediction *pred)
{
    struct CreatureControl *cctrl = &pred->cctrl;
    if ((cctrl->inst_turn >= cctrl->inst_total_turns) && !cctrl->inst_repeat) {
        cctrl->instance_id = CrInst_NULL;
        return;
    }
    cctrl->inst_repeat = 0;
    if (cctrl->instance_id == CrInst_NULL) {
        return;
    }
    cctrl->inst_turn++;
    if (cctrl->inst_turn != cctrl->inst_action_turns) {
        return;
    }
    if (instance_action_starts_cooldown(cctrl->instance_id)) {
        cctrl->instance_use_turn[cctrl->instance_id] = pred->turn;
    }
    const ThingModel shot_model = get_instance_shot_model(cctrl->instance_id);
    if (shot_model != 0) {
        pred->shot_turn = pred->turn;
        pred->shot_model = shot_model;
    }
}

static void apply_predicted_packet(struct PossessionPrediction *pred, const struct Packet *pckt)
{
    struct CreatureControl *cctrl = &pred->cctrl;
    apply_creature_movement_controls(&pred->thing, cctrl, pckt);
    start_predicted_instance(pred, get_packet_triggered_instance(&pred->thing, cctrl, pckt), pckt, true);
    const CrInstance instant_id = get_packet_instant_instance(pckt);
    if ((pckt->action == PckA_CtrlCrtrSetInstnc) && (instant_id == CrInst_NULL)) {
        cctrl->active_instance_id = pckt->actn_par1;
    }
    start_predicted_instance(pred, instant_id, pckt, false);
}

static TbBool step_possession_prediction(struct PossessionPrediction *pred, const struct Packet *pckt)
{
    struct Thing *thing = &pred->thing;
    struct CreatureControl *cctrl = &pred->cctrl;
    if (flag_is_set(thing->state_flags, TF1_FallingIntoAbyss) || flag_is_set(thing->movement_flags, TMvF_Immobile)) {
        return false;
    }
    if (pckt != NULL) {
        apply_predicted_packet(pred, pckt);
    }
    apply_thing_pushes_to_velocity(thing);
    if ((cctrl->stopped_for_hand_turns == 0) || (cctrl->instance_id == CrInst_EAT)) {
        advance_predicted_instance(pred);
    }
    if (((cctrl->stateblock_flags == 0) || creature_state_cannot_be_blocked(thing)) && (cctrl->stopped_for_hand_turns > 0)) {
        cctrl->stopped_for_hand_turns--;
    }
    if (cctrl->stateblock_flags != 0) {
        memset(&cctrl->moveaccel, 0, sizeof(cctrl->moveaccel));
        cctrl->move_speed = 0;
    } else {
        update_controlled_creature_movement(thing, cctrl);
    }
    thing->velocity.x.val += cctrl->moveaccel.x.val;
    thing->velocity.y.val += cctrl->moveaccel.y.val;
    thing->velocity.z.val += cctrl->moveaccel.z.val;
    const struct Coord3d prevpos = thing->mappos;
    struct Coord3d pos;
    if (get_creature_move_position(thing, &pos, false)) {
        thing->mappos = pos;
    }
    update_creature_move_distance(thing, cctrl, &prevpos);
    slow_down_controlled_creature(cctrl);
    thing->floor_height = get_thing_height_at(thing, &thing->mappos);
    update_creature_graphic_anim(thing, cctrl);
    cctrl->anim_time = thing->anim_time;
    advance_thing_animation(thing);
    memset(&cctrl->moveaccel, 0, sizeof(cctrl->moveaccel));
    clear_flag(cctrl->creature_control_flags, CCFlg_MoveX | CCFlg_MoveY | CCFlg_MoveZ);
    pred->turn++;
    if (thing_starts_falling_into_abyss(thing)) {
        set_flag(thing->state_flags, TF1_FallingIntoAbyss);
        return false;
    }
    apply_thing_drag_and_gravity(thing);
    if (thing_touching_floor(thing) && (cctrl->distance_to_destination != 0) && (get_foot_creature_has_down(thing, cctrl) != 0)) {
        pred->footstep_turn = pred->turn - 1;
    }
    return true;
}

static void advance_possession_prediction(struct PossessionPrediction *pred, const struct PossessionSchedule *schedule)
{
    while ((GameTurnDelta)(schedule->end_turn - pred->turn) > 0) {
        const GameTurn packet_turn = pred->turn - schedule->lag;
        const struct Packet *pckt = NULL;
        if (((GameTurnDelta)(packet_turn - pred->applied_packet_turn) > 0) && ((GameTurnDelta)(packet_turn - schedule->last_packet_turn) <= 0)) {
            pckt = get_history_packet(get_local_user(), packet_turn);
            pred->applied_packet_turn = packet_turn;
        }
        if (!step_possession_prediction(pred, pckt)) {
            return;
        }
    }
}

static TbBool possession_is_predicted(const struct Thing *thing)
{
    return prediction.valid && (prediction.thing.index == thing->index) && (prediction.thing.creation_turn == thing->creation_turn);
}

static void update_possession_correction(TbBool continuing)
{
    if (!continuing) {
        memset(&correction, 0, sizeof(correction));
        return;
    }
    if (!flag_is_set(game.operation_flags, GOF_Paused)) {
        correction.x.val = correction.x.val * POSSESSION_CORRECTION_RETAIN_PERCENT / 100;
        correction.y.val = correction.y.val * POSSESSION_CORRECTION_RETAIN_PERCENT / 100;
        correction.z.val = correction.z.val * POSSESSION_CORRECTION_RETAIN_PERCENT / 100;
    }
    correction.x.val += continuation.thing.mappos.x.val - prediction.thing.mappos.x.val;
    correction.y.val += continuation.thing.mappos.y.val - prediction.thing.mappos.y.val;
    correction.z.val += continuation.thing.mappos.z.val - prediction.thing.mappos.z.val;
    const int32_t snap_distance = max(POSSESSION_CORRECTION_MIN_SNAP_DISTANCE, game.input_lag_turns * POSSESSION_CORRECTION_SNAP_DISTANCE_PER_LAG_TURN);
    if ((abs(correction.x.val) > snap_distance) || (abs(correction.y.val) > snap_distance) || (abs(correction.z.val) > snap_distance)) {
        memset(&correction, 0, sizeof(correction));
    }
}

static void record_possession_shot_offset(const struct Thing *thing, TbBool continuing)
{
    if (!continuing) {
        memset(shot_offsets, 0, sizeof(shot_offsets));
    }
    if (flag_is_set(game.operation_flags, GOF_Paused) || (game.input_lag_turns <= 0)) {
        return;
    }
    const int32_t turn = get_gameturn() - 1;
    struct PossessionShotOffset *shot_offset = &shot_offsets[turn % POSSESSION_SHOT_OFFSETS_COUNT];
    shot_offset->turn = turn;
    shot_offset->fade_turns = game.input_lag_turns;
    shot_offset->offset.x.val = possession_view.mappos.x.val - thing->mappos.x.val;
    shot_offset->offset.y.val = possession_view.mappos.y.val - thing->mappos.y.val;
    shot_offset->offset.z.val = possession_view.mappos.z.val - thing->mappos.z.val;
}

static TbBool advance_sounded_turn(GameTurn *sounded_turn, GameTurn turn)
{
    if ((GameTurnDelta)(turn - *sounded_turn) <= 0) {
        return false;
    }
    *sounded_turn = turn;
    return true;
}

static void play_predicted_sounds(struct Thing *thing)
{
    if (advance_sounded_turn(&sounded_shot_turn, prediction.shot_turn)) {
        play_shot_firing_sound(thing, prediction.shot_model);
    }
    if (advance_sounded_turn(&sounded_footstep_turn, prediction.footstep_turn)) {
        play_creature_footstep(thing, FULL_LOUDNESS);
    }
    advance_sounded_turn(&sounded_footstep_turn, prediction.turn - 1);
}

void update_local_possession(void)
{
    struct PlayerInfo *player = get_my_player();
    struct Thing *thing = thing_get(get_local_user_state()->controlled_thing_idx);
    if (!network_is_active() || observer_is_active() || (get_player_view_type(player) != PVT_CreatureContrl) || !thing_is_creature(thing)
     || !flag_is_set(thing->alloc_flags, TAlF_IsControlled)) {
        prediction.valid = false;
        possession_pause_pending = false;
        return;
    }
    struct PossessionSchedule schedule;
    get_possession_schedule(&schedule);
    const TbBool continuing = possession_is_predicted(thing);
    if (continuing) {
        continuation = prediction;
        advance_possession_prediction(&continuation, &schedule);
    } else {
        sounded_shot_turn = get_gameturn() - 1;
        sounded_footstep_turn = get_gameturn() - 1;
    }
    prediction.thing = *thing;
    prediction.cctrl = *creature_control_get_from_thing(thing);
    prediction.turn = get_gameturn();
    prediction.applied_packet_turn = schedule.first_packet_turn - 1;
    prediction.shot_turn = sounded_shot_turn;
    prediction.footstep_turn = sounded_footstep_turn;
    prediction.valid = true;
    advance_possession_prediction(&prediction, &schedule);
    play_predicted_sounds(thing);
    update_possession_correction(continuing);
    possession_view = prediction.thing;
    possession_view.mappos.x.val += correction.x.val;
    possession_view.mappos.y.val += correction.y.val;
    possession_view.mappos.z.val += correction.z.val;
    possession_view.floor_height += correction.z.val;
    record_possession_shot_offset(thing, continuing);
}

struct Thing *get_local_possession_view(struct Thing *thing)
{
    if (!possession_is_predicted(thing)) {
        return thing;
    }
    return &possession_view;
}

const struct CreatureControl *get_local_creature_control(const struct Thing *thing)
{
    if (!possession_is_predicted(thing)) {
        return creature_control_get_from_thing(thing);
    }
    return &prediction.cctrl;
}

TbBool local_possession_look_frozen(struct Thing *thing)
{
    return flag_is_set(game.operation_flags, GOF_Paused) || possession_pause_pending || !can_process_creature_input(thing);
}

GameTurn get_local_creature_turn(const struct Thing *thing)
{
    if (!possession_is_predicted(thing)) {
        return get_gameturn();
    }
    return prediction.turn;
}

TbBool local_possession_sounded_shot(const struct Thing *firing)
{
    if (!possession_is_predicted(firing)) {
        return false;
    }
    const struct CreatureControl *cctrl = creature_control_get_from_thing(firing);
    return (get_instance_shot_model(cctrl->instance_id) != 0) && ((GameTurnDelta)(get_gameturn() - sounded_shot_turn) <= 0);
}

TbBool local_possession_sounded_footstep(const struct Thing *thing)
{
    if (!possession_is_predicted(thing)) {
        return false;
    }
    return (GameTurnDelta)(get_gameturn() - sounded_footstep_turn) <= 0;
}

void offset_local_possession_shot(const struct Thing *shotng, struct Coord3d *pos)
{
    if ((shotng->class_id != TCls_Shot) || !prediction.valid || (shotng->parent_idx != prediction.thing.index)) {
        return;
    }
    const struct PossessionShotOffset *shot_offset = &shot_offsets[shotng->creation_turn % POSSESSION_SHOT_OFFSETS_COUNT];
    const int32_t remaining_turns = shot_offset->fade_turns + 1 - (GameTurnDelta)(get_gameturn() - shotng->creation_turn);
    if ((shot_offset->turn != shotng->creation_turn) || (remaining_turns < 0)) {
        return;
    }
    float fade = 1.0f;
    if (remaining_turns < shot_offset->fade_turns) {
        const float linear_fade = interpolate_synced(remaining_turns + 1, remaining_turns) / shot_offset->fade_turns;
        fade = linear_fade * linear_fade * (3.0f - 2.0f * linear_fade);
    }
    pos->x.val += shot_offset->offset.x.val * fade;
    pos->y.val += shot_offset->offset.y.val * fade;
    pos->z.val += shot_offset->offset.z.val * fade;
}

void set_packet_possession_target(struct Packet *pckt)
{
    struct Thing *thing = thing_get(get_local_user_state()->controlled_thing_idx);
    if ((packet_camera_context(pckt) != CamIV_FirstPerson) || !thing_is_creature(thing)) {
        return;
    }
    CrInstance inst_id = get_packet_triggered_instance(thing, get_local_creature_control(thing), pckt);
    if (inst_id == CrInst_NULL) {
        inst_id = get_packet_instant_instance(pckt);
    }
    if (inst_id == CrInst_NULL) {
        return;
    }
    packet_set_possession_target(pckt, find_possession_target(get_local_possession_view(thing), inst_id, pckt->pos_x & ANGLE_MASK));
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
