/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file local_hand.c
 *     Client-side prediction of the local user's Hand of Evil.
 * @par Purpose:
 *     Replays the local inputs which the game hasn't processed yet on a copy
 *     of the local user's state and hand, so picking up and dropping things
 *     responds without input lag. Nothing here is part of the synced game state,
 *     apart from the camera shake, which only the local camera reads.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Nov 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "local_hand.h"
#include "bflib_sound.h"
#include "config_creature.h"
#include "config_magic.h"
#include "creature_control.h"
#include "creature_graphics.h"
#include "creature_instances.h"
#include "creature_states.h"
#include "dungeon_data.h"
#include "engine_camera.h"
#include "game_legacy.h"
#include "magic_powers.h"
#include "map_data.h"
#include "net_exchange_gameplay.h"
#include "net_input_lag.h"
#include "net_spectator.h"
#include "observer.h"
#include "packets.h"
#include "player_instances.h"
#include "power_hand.h"
#include "replay.h"
#include "thing_creature.h"
#include "thing_data.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "thing_physics.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct HandPrediction {
    struct UserState ustate;
    struct Thing hand;
    uint32_t pending_actions[LocalHandAction_Count];
    GameTurn turn;
    GameTurn creature_dropped_turn;
    TbBool hand_exists;
};

static struct HandPrediction prediction;
static TbBool prediction_active;
static uint32_t done_actions[LocalHandAction_Count];
static uint32_t shown_actions[LocalHandAction_Count];
static struct Thing held_view;
static GameTurn held_view_turn;
/******************************************************************************/
static TbBool claim_unshown_action(struct HandPrediction *pred, enum LocalHandAction action)
{
    pred->pending_actions[action]++;
    const uint32_t action_number = done_actions[action] + pred->pending_actions[action];
    if (action_number <= shown_actions[action]) {
        return false;
    }
    shown_actions[action] = action_number;
    return true;
}

static void show_predicted_hand(struct HandPrediction *pred)
{
    if (pred->hand_exists) {
        return;
    }
    memset(&pred->hand, 0, sizeof(pred->hand));
    pred->hand.class_id = TCls_Object;
    pred->hand.model = ObjMdl_PowerHand;
    pred->hand.owner = pred->ustate.player_id;
    pred->hand_exists = true;
    set_new_hand_graphic(&pred->ustate, &pred->hand, get_player(pred->ustate.player_id), get_first_held_thing(&pred->ustate), pred->turn);
}

static void set_predicted_hand_graphic(struct HandPrediction *pred, int32_t hand_animation)
{
    if (!pred->hand_exists || (hand_animation == HndA_None)) {
        return;
    }
    set_hand_graphic(&pred->ustate, &pred->hand, get_player(pred->ustate.player_id), hand_animation, pred->turn);
}

static void start_predicted_instance(struct HandPrediction *pred, unsigned char inum)
{
    if (begin_user_instance(&pred->ustate, inum, false)) {
        set_predicted_hand_graphic(pred, get_hand_instance_animation(inum));
    }
}

static void influence_predicted_thing(struct HandPrediction *pred, const struct Thing *thing, unsigned char inum)
{
    pred->ustate.influenced_thing_idx = thing->index;
    pred->ustate.influenced_thing_creation = thing->creation_turn;
    start_predicted_instance(pred, inum);
}

static TbBool predicted_hand_accepts(const struct UserState *ustate, const struct Thing *thing)
{
    PlayerNumber plyr_idx = ustate->player_id;
    TbBool already_held = !object_is_gold_hoard(thing) && (find_thing_in_list(ustate->things_in_hand, ustate->num_things_in_hand, thing->index) >= 0);
    return power_hand_can_take_thing(plyr_idx, get_local_user(), ustate, thing) && !already_held && !thing_is_picked_up(thing)
        && !thing_is_special_box(thing) && is_power_available(plyr_idx, PwrK_HAND);
}

static void place_predicted_thing(struct HandPrediction *pred, struct Thing *thing)
{
    struct UserState *ustate = &pred->ustate;
    if (!predicted_hand_accepts(ustate, thing)) {
        return;
    }
    insert_thing_into_hand_list(ustate, ustate->player_id, thing->index);
    if (claim_unshown_action(pred, LocalHandAction_PickUp)) {
        play_power_hand_pickup_sounds(thing, ustate->player_id, get_local_user());
    }
}

static void grab_predicted_thing(struct HandPrediction *pred, ThingIndex tng_idx)
{
    struct UserState *ustate = &pred->ustate;
    struct Thing *thing = get_thing_for_power_hand(ustate->player_id, get_local_user(), ustate, tng_idx);
    if (!predicted_hand_accepts(ustate, thing)) {
        return;
    }
    show_predicted_hand(pred);
    influence_predicted_thing(pred, thing, PI_Grab);
}

static void slap_predicted_thing(struct HandPrediction *pred, const struct Thing *thing)
{
    struct UserState *ustate = &pred->ustate;
    TbBool blocked_by_drop = ((GameTurnDelta)(pred->turn - pred->creature_dropped_turn) <= SLAP_BLOCKED_TURNS_AFTER_DROP);
    if (!thing_exists(thing) || (ustate->instance_num == PI_Whip) || blocked_by_drop) {
        return;
    }
    if (!can_cast_spell(ustate->player_id, get_local_user(), PwrK_SLAP, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default)) {
        return;
    }
    influence_predicted_thing(pred, thing, PI_Whip);
}

static void hit_predicted_thing(struct HandPrediction *pred, struct Thing *thing)
{
    TbBool is_creature = thing_is_creature(thing);
    TbBool reacts_to_slap = thing_is_object(thing) || (is_creature && !creature_under_spell_effect(thing, CSAfF_Freeze));
    if (reacts_to_slap && claim_unshown_action(pred, LocalHandAction_Slap)) {
        show_power_slap_feedback(thing);
    }
    start_predicted_instance(pred, PI_WhipEnd);
}

static TbBool drop_predicted_thing(struct HandPrediction *pred, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct UserState *ustate = &pred->ustate;
    struct Thing *thing = get_first_held_thing(ustate);
    if ((thing == NULL) || !can_place_thing_here(thing, stl_x, stl_y, ustate->player_id)) {
        return false;
    }
    enum HeldThingDropOutcome outcome = get_held_thing_drop_outcome(thing, thing_get(ustate->thing_under_hand));
    if ((outcome == HTDrop_Gold) && pred->hand_exists) {
        set_gold_dropped_hand_graphic(ustate, &pred->hand, get_player(ustate->player_id), pred->turn);
    }
    if (thing_is_creature(thing)) {
        pred->creature_dropped_turn = pred->turn;
    }
    if (ustate->num_things_in_hand == 1) {
        start_predicted_instance(pred, PI_Drop);
    }
    remove_held_thing_at(ustate->things_in_hand, &ustate->num_things_in_hand, 0);
    return true;
}

static void predict_hand_state(struct HandPrediction *pred, const struct Packet *pckt)
{
    struct UserState *ustate = &pred->ustate;
    ustate->thing_under_hand = 0;
    enum HandCursorUse hand_use = update_hand_cursor_state(ustate, pckt, true);
    if (hand_use == HandCursor_Grabbing) {
        return;
    }
    if (hand_use == HandCursor_Remove) {
        pred->hand_exists = false;
        return;
    }
    if (hand_use == HandCursor_Show) {
        show_predicted_hand(pred);
    }
    if (pred->hand_exists && (get_hand_instance_animation(ustate->instance_num) == HndA_None)) {
        set_idle_hand_graphic(ustate, &pred->hand, get_player(ustate->player_id), get_first_held_thing(ustate), pred->turn);
    }
}

static void predict_dungeon_control(struct HandPrediction *pred, const struct Packet *pckt)
{
    struct UserState *ustate = &pred->ustate;
    struct Packet button_pckt = *pckt;
    predict_hand_state(pred, pckt);
    if (flag_is_set(pckt->control_flags, PCtr_MapCoordsValid)) {
        press_cursor_buttons(ustate, pckt);
    }
    if (begin_left_release(ustate, &button_pckt)) {
        finish_left_release(ustate, &button_pckt);
    }
    enum RightReleaseHandUse release_use = begin_right_release(ustate, &button_pckt);
    TbBool dropped = false;
    if (release_use == RRelease_DropHeldThing) {
        dropped = drop_predicted_thing(pred, coord_subtile(pckt->pos_x), coord_subtile(pckt->pos_y));
    }
    finish_cursor_release(ustate, &button_pckt, release_use, dropped);
}

static void apply_predicted_hand_packet(struct HandPrediction *pred, const struct Packet *pckt)
{
    struct UserState *ustate = &pred->ustate;
    if (pckt->action == PckA_SetPlyrState) {
        ustate->work_state = pckt->actn_par1;
    }
    if (ustate->view_type != PVT_DungeonTop) {
        return;
    }
    ustate->primary_cursor_state = (pckt->additional_packet_values & PCAdV_ContextMask) >> 1;
    const TbBool over_gui = flag_is_set(pckt->control_flags, PCtr_Gui);
    if (!over_gui && (ustate->work_state == PSt_CtrlDungeon)) {
        predict_dungeon_control(pred, pckt);
    }
    struct Thing *heldtng = get_first_held_thing(ustate);
    switch (pckt->action) {
    case PckA_UsePwrHandPick:
        if (over_gui) {
            place_predicted_thing(pred, thing_get(pckt->actn_par1));
        } else {
            grab_predicted_thing(pred, pckt->actn_par1);
        }
        break;
    case PckA_UsePwrHandDrop:
        drop_predicted_thing(pred, pckt->actn_par1, pckt->actn_par2);
        break;
    case PckA_DumpHeldThingToOldPos:
        if (heldtng != NULL) {
            drop_predicted_thing(pred, heldtng->mappos.x.stl.num, heldtng->mappos.y.stl.num);
        }
        break;
    case PckA_UsePwrOnThing:
        if (pckt->actn_par1 == PwrK_SLAP) {
            slap_predicted_thing(pred, thing_get(pckt->actn_par2));
        }
        break;
    }
}

static void finish_predicted_instance(struct HandPrediction *pred, unsigned char inum)
{
    struct UserState *ustate = &pred->ustate;
    struct Thing *thing = thing_get(ustate->influenced_thing_idx);
    TbBool influenced = thing_exists(thing) && (thing->creation_turn == ustate->influenced_thing_creation);
    set_predicted_hand_graphic(pred, get_hand_instance_end_animation(inum));
    if ((inum == PI_Grab) && influenced) {
        place_predicted_thing(pred, thing);
    } else if ((inum == PI_Whip) && influenced && thing_slappable(thing, ustate->player_id)) {
        hit_predicted_thing(pred, thing);
    }
}

static void step_predicted_hand(struct HandPrediction *pred)
{
    struct UserState *ustate = &pred->ustate;
    if (ustate->instance_num != PI_Unset) {
        if (ustate->instance_remain_turns > 0) {
            ustate->instance_remain_turns--;
        }
        if (ustate->instance_remain_turns == 0) {
            const unsigned char inum = ustate->instance_num;
            ustate->instance_num = PI_Unset;
            finish_predicted_instance(pred, inum);
        }
    }
    if (pred->hand_exists) {
        advance_thing_animation(&pred->hand);
    }
    pred->turn++;
}

static void predict_hand(struct HandPrediction *pred)
{
    const NetUserId user = get_local_user();
    const struct UserState *ustate = get_user_state(user);
    const struct Thing *handtng = thing_get(ustate->hand_thing_idx);
    memset(pred->pending_actions, 0, sizeof(pred->pending_actions));
    pred->ustate = *ustate;
    pred->hand_exists = thing_exists(handtng);
    if (pred->hand_exists) {
        pred->hand = *handtng;
    }
    pred->turn = get_gameturn() + 1;
    pred->creature_dropped_turn = get_dungeon(ustate->player_id)->last_creature_dropped_gameturn;
    for (GameTurn turn = input_lag_first_pending_turn(); (GameTurnDelta)(get_gameturn() - turn) > 0; turn++) {
        const struct Packet *pckt = get_history_packet(user, turn);
        if (pckt != NULL) {
            apply_predicted_hand_packet(pred, pckt);
        }
        step_predicted_hand(pred);
    }
}

static void update_held_view(void)
{
    const struct Thing *thing = get_first_held_thing(&prediction.ustate);
    if (thing == NULL) {
        held_view.index = 0;
        return;
    }
    if ((held_view.index != thing->index) || (held_view.creation_turn != thing->creation_turn) || (held_view.model != thing->model)) {
        held_view = *thing;
        held_view_turn = get_gameturn();
        if (!thing_is_picked_up(thing)) {
            set_held_thing_draw(&held_view);
        }
    }
    while ((GameTurnDelta)(get_gameturn() - held_view_turn) > 0) {
        advance_thing_animation(&held_view);
        held_view_turn++;
    }
    set_flag_value(held_view.rendering_flags, TRF_Invisible, prediction.ustate.hand_animationId != HndA_Hold);
}

static TbBool local_hand_can_predict(void)
{
    const NetUserId user = get_local_user();
    const struct UserState *ustate = get_user_state(user);
    return network_is_active() && !replay.load_enable && !observer_is_active() && !network_user_is_spectator(user)
        && !user_state_invalid(ustate) && (ustate->player_id >= 0) && (ustate->player_id < PLAYERS_COUNT);
}

void reset_local_hand(void)
{
    prediction_active = false;
    memset(done_actions, 0, sizeof(done_actions));
    memset(shown_actions, 0, sizeof(shown_actions));
    held_view.index = 0;
}

void update_local_hand(void)
{
    if (!local_hand_can_predict()) {
        reset_local_hand();
        return;
    }
    for (int32_t action = 0; action < LocalHandAction_Count; action++) {
        shown_actions[action] = max(shown_actions[action], done_actions[action]);
    }
    predict_hand(&prediction);
    for (int32_t action = 0; action < LocalHandAction_Count; action++) {
        shown_actions[action] = min(shown_actions[action], done_actions[action] + prediction.pending_actions[action]);
    }
    prediction_active = true;
    update_held_view();
    struct UserState predicted_ustate = *get_local_user_state_prediction();
    apply_local_hand_prediction(&predicted_ustate);
    set_local_user_state_prediction(&predicted_ustate);
}

void apply_local_hand_prediction(struct UserState *ustate)
{
    if (!prediction_active) {
        return;
    }
    const struct UserState *predicted = &prediction.ustate;
    memcpy(ustate->things_in_hand, predicted->things_in_hand, sizeof(ustate->things_in_hand));
    ustate->num_things_in_hand = predicted->num_things_in_hand;
    ustate->hand_animationId = predicted->hand_animationId;
    ustate->hand_animation_until_turn = predicted->hand_animation_until_turn;
    ustate->instance_num = predicted->instance_num;
    ustate->instance_remain_turns = predicted->instance_remain_turns;
}

TbBool local_hand_takes_over_action(NetUserId user, enum LocalHandAction action)
{
    if (!prediction_active || !is_local_user(user)) {
        return false;
    }
    done_actions[action]++;
    return done_actions[action] <= shown_actions[action];
}

struct Thing *get_hand_view(NetUserId user)
{
    if (prediction_active && is_local_user(user)) {
        if (!prediction.hand_exists) {
            return NULL;
        }
        return &prediction.hand;
    }
    struct Thing *thing = thing_get(get_user_state(user)->hand_thing_idx);
    if (!thing_exists(thing)) {
        return NULL;
    }
    return thing;
}

struct Thing *get_held_thing_view(NetUserId user)
{
    if (prediction_active && is_local_user(user)) {
        if (held_view.index == 0) {
            return NULL;
        }
        return &held_view;
    }
    return get_first_held_thing(get_user_state(user));
}

TbBool local_hand_holds_thing(const struct Thing *thing)
{
    if (!prediction_active || thing_is_picked_up(thing) || object_is_gold_hoard(thing)) {
        return false;
    }
    const struct UserState *ustate = &prediction.ustate;
    return (find_thing_in_list(ustate->things_in_hand, ustate->num_things_in_hand, thing->index) >= 0);
}

TbBool local_hand_hides_thing(const struct Thing *thing)
{
    return local_hand_holds_thing(thing);
}

static void adjust_local_activity_count(const struct Thing *thing, ThingModel crmodel, int32_t *gui_job_counts, int32_t delta)
{
    TbBool counted = (thing->class_id == TCls_Creature) && (thing->model == crmodel) && (thing->owner == prediction.ustate.player_id)
        && creature_counts_in_dungeon_stats(thing);
    if (!counted) {
        return;
    }
    int32_t gui_job = get_creature_gui_job(thing);
    gui_job_counts[gui_job] = max(gui_job_counts[gui_job] + delta, 0);
}

void apply_local_hand_to_activity_counts(ThingModel crmodel, int32_t *gui_job_counts)
{
    if (!prediction_active) {
        return;
    }
    const struct UserState *ustate = &prediction.ustate;
    for (int32_t i = 0; i < ustate->num_things_in_hand; i++) {
        adjust_local_activity_count(thing_get(ustate->things_in_hand[i]), crmodel, gui_job_counts, -1);
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
