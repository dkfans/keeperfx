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
#define LOCAL_HAND_FEEDBACK_MARGIN_TURNS 4
#define MAX_HEALTH_FLASHES 8
#define DAMAGE_FLASH_TURNS 1

struct HandDrop {
    ThingIndex index;
    GameTurn turn;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    TbBool lands_on_ground;
};

struct HandPrediction {
    struct UserState ustate;
    struct Thing hand;
    struct HandDrop drops[MAX_THINGS_IN_HAND];
    int32_t drop_count;
    uint32_t pending_actions[LocalHandAction_Count];
    GameTurn packet_turn;
    GameTurn turn;
    GameTurn creature_dropped_turn;
    TbBool hand_exists;
};

struct DroppedView {
    struct Thing thing;
    struct CreatureControl cctrl;
    GameTurn packet_turn;
    GameTurn turn;
    TbBool landed;
};

struct HealthFlash {
    ThingIndex index;
    GameTurn creation_turn;
    GameTurn slap_turn;
    GameTurn hidden_damage_turn;
};

static struct HandPrediction prediction;
static TbBool prediction_active;
static uint32_t done_actions[LocalHandAction_Count];
static uint32_t shown_actions[LocalHandAction_Count];
static struct DroppedView dropped_views[MAX_THINGS_IN_HAND];
static int32_t dropped_view_count;
static struct HealthFlash health_flashes[MAX_HEALTH_FLASHES];
static int32_t next_health_flash;
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

static struct HealthFlash *find_health_flash(const struct Thing *thing)
{
    const GameTurnDelta lifetime = HEALTH_BAR_FLASH_TURNS + game.input_lag_turns + LOCAL_HAND_FEEDBACK_MARGIN_TURNS;
    for (int32_t i = 0; i < MAX_HEALTH_FLASHES; i++) {
        struct HealthFlash *flash = &health_flashes[i];
        TbBool same_thing = (flash->index == thing->index) && (flash->creation_turn == thing->creation_turn);
        if (same_thing && ((GameTurnDelta)(get_gameturn() - flash->slap_turn) <= lifetime)) {
            return flash;
        }
    }
    return NULL;
}

static void flash_predicted_health(const struct Thing *thing)
{
    const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    TbBool slap_hurts = (calculate_slap_damage(thing) > 0) && (thing->health >= 0) && !flag_is_set(cctrl->creature_control_flags, CCFlg_PreventDamage);
    if (!slap_hurts) {
        return;
    }
    struct HealthFlash *flash = find_health_flash(thing);
    if (flash == NULL) {
        flash = &health_flashes[next_health_flash];
        next_health_flash = (next_health_flash + 1) % MAX_HEALTH_FLASHES;
    }
    flash->index = thing->index;
    flash->creation_turn = thing->creation_turn;
    flash->slap_turn = get_gameturn();
    flash->hidden_damage_turn = thing->last_turn_damaged;
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
    return power_hand_can_take_thing(plyr_idx, ustate, thing) && !already_held && !thing_is_picked_up(thing)
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
        play_power_hand_pickup_sounds(thing, ustate->player_id);
    }
}

static void grab_predicted_thing(struct HandPrediction *pred, ThingIndex tng_idx)
{
    struct UserState *ustate = &pred->ustate;
    struct Thing *thing = get_thing_for_power_hand(ustate->player_id, ustate, tng_idx);
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
        if (is_creature) {
            flash_predicted_health(thing);
        }
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
    if (pred->drop_count < MAX_THINGS_IN_HAND) {
        struct HandDrop *drop = &pred->drops[pred->drop_count];
        drop->index = thing->index;
        drop->turn = pred->packet_turn;
        drop->stl_x = stl_x;
        drop->stl_y = stl_y;
        drop->lands_on_ground = (outcome == HTDrop_Ground);
        pred->drop_count++;
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
    enum HandCursorUse hand_use = begin_dungeon_control_input(ustate, pckt, true);
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
        highlight_cursor_roomspace(ustate, get_player(ustate->player_id), pckt);
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
    const TbBool over_gui = flag_is_set(pckt->control_flags, PCtr_Gui);
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
    ustate->primary_cursor_state = (pckt->additional_packet_values & PCAdV_ContextMask) >> 1;
    if (!over_gui && (ustate->work_state == PSt_CtrlDungeon)) {
        predict_dungeon_control(pred, pckt);
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
    pred->drop_count = 0;
    pred->turn = get_gameturn() + 1;
    pred->creature_dropped_turn = get_dungeon(ustate->player_id)->last_creature_dropped_gameturn;
    for (GameTurn turn = input_lag_first_pending_turn(); (GameTurnDelta)(get_gameturn() - turn) > 0; turn++) {
        const struct Packet *pckt = get_history_packet(user, turn);
        pred->packet_turn = turn;
        if (pckt != NULL) {
            apply_predicted_hand_packet(pred, pckt);
        }
        step_predicted_hand(pred);
    }
}

static TbBool get_dropped_view_next_position(struct Thing *thing, struct Coord3d *pos)
{
    if (thing->class_id == TCls_Creature) {
        return get_creature_move_position(thing, pos, false);
    }
    return get_thing_next_position(pos, thing) && !thing_in_wall_at(thing, pos);
}

static void step_dropped_view(struct DroppedView *view)
{
    struct Thing *thing = &view->thing;
    thing->previous_mappos = thing->mappos;
    thing->previous_floor_height = thing->floor_height;
    apply_thing_pushes_to_velocity(thing);
    struct Coord3d pos;
    if (get_dropped_view_next_position(thing, &pos)) {
        thing->mappos = pos;
    }
    thing->floor_height = get_thing_height_at(thing, &thing->mappos);
    if (thing->class_id == TCls_Creature) {
        update_creature_graphic_anim(thing, &view->cctrl);
        view->cctrl.anim_time = thing->anim_time;
        if (!view->landed && thing_has_landed(thing)) {
            view->landed = true;
            if (flag_is_set(get_creature_model_flags(thing), CMF_Trembling) && !creature_under_spell_effect(thing, CSAfF_Chicken)) {
                get_dungeon(prediction.ustate.player_id)->camera_deviate_jump = TREMBLING_LANDING_CAMERA_DEVIATE_JUMP;
            }
        }
    }
    apply_thing_drag_and_gravity(thing);
    advance_thing_animation(thing);
    update_thing_sound(thing);
    view->turn++;
}

static void create_dropped_view(const struct HandDrop *drop)
{
    const struct Thing *thing = thing_get(drop->index);
    TbBool droppable = thing_is_creature(thing) || (thing_is_object(thing) && !object_is_gold_hoard(thing));
    if (!droppable || (dropped_view_count >= MAX_THINGS_IN_HAND)) {
        return;
    }
    struct DroppedView *view = &dropped_views[dropped_view_count];
    dropped_view_count++;
    view->thing = *thing;
    view->packet_turn = drop->turn;
    view->turn = drop->turn;
    view->landed = false;
    struct Thing *viewtng = &view->thing;
    struct Coord3d pos;
    get_held_thing_drop_position(viewtng, drop->stl_x, drop->stl_y, &pos);
    set_dropped_thing_position(viewtng, &pos);
    viewtng->floor_height = get_thing_height_at(viewtng, &viewtng->mappos);
    viewtng->snd_emitter_id = 0;
    clear_thing_velocity(viewtng);
    clear_flag(viewtng->alloc_flags, TAlF_IsInLimbo);
    set_flag_value(viewtng->rendering_flags, TRF_Invisible, !drop->lands_on_ground);
    if (thing_is_object(thing)) {
        set_dropped_object_draw(viewtng);
        viewtng->active_state = ObSt_BeingDropped;
        return;
    }
    view->cctrl = *creature_control_get_from_thing(thing);
    view->cctrl.instance_id = CrInst_NULL;
    view->cctrl.distance_to_destination = 0;
    viewtng->active_state = CrSt_CreatureBeingDropped;
    play_creature_sound(viewtng, CrSnd_Drop, 3, 0);
}

static void release_dropped_view(int32_t view_idx)
{
    struct DroppedView *view = &dropped_views[view_idx];
    if (view->thing.snd_emitter_id != 0) {
        S3DDestroySoundEmitter(view->thing.snd_emitter_id);
    }
    dropped_view_count--;
    dropped_views[view_idx] = dropped_views[dropped_view_count];
}

static const struct DroppedView *find_dropped_view(const struct Thing *thing)
{
    for (int32_t i = 0; i < dropped_view_count; i++) {
        if ((dropped_views[i].thing.index == thing->index) && (dropped_views[i].thing.creation_turn == thing->creation_turn)) {
            return &dropped_views[i];
        }
    }
    return NULL;
}

static const struct DroppedView *find_view_for_drop(const struct HandDrop *drop)
{
    for (int32_t i = 0; i < dropped_view_count; i++) {
        if ((dropped_views[i].thing.index == drop->index) && (dropped_views[i].packet_turn == drop->turn)) {
            return &dropped_views[i];
        }
    }
    return NULL;
}

static TbBool dropped_view_is_pending(const struct DroppedView *view)
{
    for (int32_t i = 0; i < prediction.drop_count; i++) {
        if (find_view_for_drop(&prediction.drops[i]) == view) {
            return true;
        }
    }
    return false;
}

static TbBool dropped_view_is_falling(const struct DroppedView *view)
{
    const struct Thing *thing = thing_get(view->thing.index);
    if (!thing_exists(thing) || (thing->creation_turn != view->thing.creation_turn) || thing_is_picked_up(thing)) {
        return false;
    }
    TbBool still_descending = (thing->previous_mappos.z.val != thing->mappos.z.val);
    return (thing->active_state == view->thing.active_state) && (!thing_has_landed(thing) || still_descending);
}

static void update_dropped_views(void)
{
    for (int32_t i = dropped_view_count - 1; i >= 0; i--) {
        if (!dropped_view_is_pending(&dropped_views[i]) && !dropped_view_is_falling(&dropped_views[i])) {
            release_dropped_view(i);
        }
    }
    for (int32_t i = 0; i < prediction.drop_count; i++) {
        if (find_view_for_drop(&prediction.drops[i]) == NULL) {
            create_dropped_view(&prediction.drops[i]);
        }
    }
    for (int32_t i = 0; i < dropped_view_count; i++) {
        while ((GameTurnDelta)(get_gameturn() - dropped_views[i].turn) > 0) {
            step_dropped_view(&dropped_views[i]);
        }
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
    while (dropped_view_count > 0) {
        release_dropped_view(0);
    }
    prediction_active = false;
    memset(done_actions, 0, sizeof(done_actions));
    memset(shown_actions, 0, sizeof(shown_actions));
    memset(health_flashes, 0, sizeof(health_flashes));
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
    update_dropped_views();
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
    if (local_hand_holds_thing(thing)) {
        return true;
    }
    return local_hand_predicted_drop(thing);
}

struct Thing *find_local_dropped_view(MapSubtlCoord stl_x, MapSubtlCoord stl_y, int32_t *view_idx)
{
    while (*view_idx < dropped_view_count) {
        struct Thing *thing = &dropped_views[*view_idx].thing;
        (*view_idx)++;
        if ((thing->mappos.x.stl.num == stl_x) && (thing->mappos.y.stl.num == stl_y) && !flag_is_set(thing->rendering_flags, TRF_Invisible)) {
            return thing;
        }
    }
    return NULL;
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
    for (int32_t i = 0; i < dropped_view_count; i++) {
        const struct Thing *viewtng = &dropped_views[i].thing;
        if (thing_is_picked_up(thing_get(viewtng->index))) {
            adjust_local_activity_count(viewtng, crmodel, gui_job_counts, 1);
        }
    }
}

TbBool local_hand_shows_flash(const struct Thing *thing, enum LocalHandFlash flash)
{
    TbBool server_flashes = (thing->creature.health_bar_turns > 0);
    GameTurnDelta predicted_turns = HEALTH_BAR_FLASH_TURNS;
    if (flash == LocalHandFlash_Damage) {
        server_flashes = (thing->last_turn_damaged == get_gameturn());
        predicted_turns = DAMAGE_FLASH_TURNS;
    }
    const struct HealthFlash *health_flash = find_health_flash(thing);
    if (health_flash == NULL) {
        return server_flashes;
    }
    TbBool predicted = ((GameTurnDelta)(get_gameturn() - health_flash->slap_turn) < predicted_turns);
    TbBool from_confirmed_slap = (thing->last_turn_damaged == health_flash->hidden_damage_turn);
    return predicted || (server_flashes && !from_confirmed_slap);
}

void local_hand_hide_damage_flash(const struct Thing *thing)
{
    struct HealthFlash *flash = find_health_flash(thing);
    if (flash != NULL) {
        flash->hidden_damage_turn = thing->last_turn_damaged;
    }
}

TbBool local_hand_predicted_drop(const struct Thing *thing)
{
    return (find_dropped_view(thing) != NULL);
}

TbBool local_hand_showed_landing(const struct Thing *thing)
{
    const struct DroppedView *view = find_dropped_view(thing);
    return (view != NULL) && view->landed;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
