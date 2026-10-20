/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace_input.c
 *     Turns local mouse input into dig tag packet actions, and shows those tags and the cursor box before the game state catches up.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "observer.h"
#include "game_legacy.h"
#include "config_sounds.h"
#include "map_data.h"
#include "cursor_tag.h"
#include "engine_render.h"
#include "frontmenu_ingame_evnt.h"
#include "player_data.h"
#include "power_hand.h"
#include "slab_data.h"
#include "tasks_list.h"
#include "packets.h"
#include "net_exchange_gameplay.h"
#include "roomspace.h"
#include "roomspace_input.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static struct DigTagPrediction dig_tags;

static struct {
    MapSlabCoord drag_start_slb_x;
    MapSlabCoord drag_start_slb_y;
    MapSlabCoord previous_slb_x;
    MapSlabCoord previous_slb_y;
    TbBool cursor_button_down;
    TbBool cursor_locked;
    unsigned char cursor_state;
    TbBool untag_mode;
} dig_cursor;

static GameTurn cursor_release_turn;
static MapSlabCoord room_drag_start_slb_x;
static MapSlabCoord room_drag_start_slb_y;

static TbBool dig_render_roomspace_active;

void reset_dig_prediction(void)
{
    memset(&dig_tags, 0, sizeof(dig_tags));
    memset(&dig_cursor, 0, sizeof(dig_cursor));
    cursor_release_turn = 0;
    set_local_user_state_prediction(NULL);
    dig_render_roomspace_active = false;
}

static void reconcile_dig_predictions(struct DigTagPrediction *tags, GameTurn processed_turn)
{
    tags->task_count = get_players_dungeon(get_my_player())->task_count;
    for (int i = 0; i < tags->slab_count;) {
        SlabCodedCoords slb_num = tags->slabs[i];
        if ((GameTurnDelta)(tags->slab_turns[slb_num] - processed_turn) <= 0) {
            tags->slab_tag_modes[slb_num] = 0;
            tags->slabs[i] = tags->slabs[--tags->slab_count];
            continue;
        }
        TbBool tagged = find_from_task_list_by_slab(my_player_number, slb_num_decode_x(slb_num), slb_num_decode_y(slb_num)) != -1;
        tags->task_count += (tags->slab_tag_modes[slb_num] == DigTagMode_Tag) - tagged;
        i++;
    }
}

static TbBool dig_input_is_blocked(const struct Packet *pckt)
{
    if (pckt == NULL) {
        return true;
    }
    if ((pckt->control_flags & (PCtr_Gui | PCtr_MapCoordsValid)) != PCtr_MapCoordsValid) {
        return true;
    }
    switch (pckt->action)
    {
    case PckA_UsePwrHandPick:
    case PckA_UsePwrOnThing:
    case PckA_BookmarkLoad: // Dragging minimap
        return true;
    }
    if (battle_creature_over > 0) {
        return true;
    }
    return false;
}

static TbBool dig_input_is_enabled(void)
{
    return !replay.load_enable && (get_my_player() != &local_observer_player);
}

const struct RoomSpace *get_dig_prediction_render_roomspace(const struct RoomSpace *roomspace)
{
    if (dig_render_roomspace_active) {
        return &get_player_user_state_prediction(get_my_player())->render_roomspace;
    }
    return roomspace;
}

TbBool local_cursor_is_locked(void)
{
    if (dig_cursor.cursor_button_down && dig_cursor.cursor_locked) {
        return true;
    }
    if (get_gameturn() - cursor_release_turn <= game.input_lag_turns) {
        return false;
    }
    return get_local_user_state()->one_click_lock_cursor;
}

static TbBool update_dig_highlight(const struct Packet *pckt, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct UserState *ustate = get_local_user_state();
    struct PlayerInfo *player = get_my_player();
    unsigned char cursor_context = (unsigned char)((pckt->additional_packet_values & PCAdV_ContextMask) >> 1);
    TbBool cursor_is_locked = dig_cursor.cursor_button_down && dig_cursor.cursor_locked && ((pckt->control_flags & PCtr_LBtnClick) == 0) && ((pckt->control_flags & (PCtr_LBtnHeld | PCtr_LBtnRelease)) != 0);
    if ((cursor_context != CSt_PickAxe) && !cursor_is_locked) {
        if ((cursor_context != CSt_PowerHand) || power_hand_is_empty(player) || !can_dig_here(stl_x, stl_y, my_player_number, true)) {
            return false;
        }
    }
    if (!cursor_is_locked && (((pckt->control_flags & PCtr_LBtnAnyAction) == 0) || ((pckt->control_flags & PCtr_LBtnClick) != 0))) {
        ustate->swap_to_untag_mode = 0;
    }
    ustate->one_click_lock_cursor = cursor_is_locked;
    ustate->render_roomspace.drag_mode = cursor_is_locked;
    if (cursor_is_locked) {
        ustate->render_roomspace.drag_start_x = dig_cursor.drag_start_slb_x;
        ustate->render_roomspace.drag_start_y = dig_cursor.drag_start_slb_y;
        ustate->render_roomspace.untag_mode = dig_cursor.untag_mode;
    }
    get_dungeon_highlight_user_roomspace(&ustate->render_roomspace, player, get_local_user(), pckt, stl_x, stl_y, &dig_tags);
    tag_cursor_blocks_dig(player, get_local_user(), pckt, &ustate->render_roomspace, stl_x, stl_y, true);
    return true;
}

static void apply_packet_prediction(NetUserId user, const struct Packet *pckt)
{
    struct UserState *ustate = get_user_state(user);
    apply_roomspace_packet_action(user, pckt);
    ustate->primary_cursor_state = (pckt->additional_packet_values & PCAdV_ContextMask) >> 1;
    if (pckt->action == PckA_SetPlyrState) {
        ustate->work_state = pckt->actn_par1;
        set_user_work_state_kind(ustate, pckt->actn_par2);
    }
}

/**
 * Runs the singleplayer roomspace cursor code for a local packet, on the user state as it will be once the pending local packets are processed.
 * pending_turns counts the pending packets before this one; it is negative when this packet has been processed already.
 */
static TbBool get_predicted_roomspace(const struct Packet *pckt, GameTurnDelta pending_turns, struct UserState *predicted_ustate)
{
    NetUserId user = get_local_user();
    struct UserState *ustate = get_local_user_state();
    struct Packet *direct_packet = get_local_packet();
    struct UserState saved_ustate = *ustate;
    struct Packet saved_packet = *direct_packet;
    for (GameTurnDelta i = pending_turns; i > 0; i--) {
        const struct Packet *pending_pckt = get_history_packet(user, get_gameturn() - i);
        if (pending_pckt != NULL) {
            apply_packet_prediction(user, pending_pckt);
        }
    }
    TbBool has_roomspace = !dig_input_is_blocked(pckt);
    if ((pckt != NULL) && (pending_turns >= 0)) {
        apply_packet_prediction(user, pckt);
    }
    if (has_roomspace) {
        *direct_packet = *pckt;
        MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
        MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
        ustate->render_roomspace.drag_start_x = room_drag_start_slb_x;
        ustate->render_roomspace.drag_start_y = room_drag_start_slb_y;
        if (ustate->work_state == PSt_BuildRoom) {
            update_dungeon_build_roomspace_preview(user, stl_x, stl_y);
        } else if (ustate->work_state == PSt_Sell) {
            update_dungeon_sell_roomspace_preview(user, stl_x, stl_y);
        } else {
            has_roomspace = update_dig_highlight(pckt, stl_x, stl_y);
        }
    }
    *predicted_ustate = *ustate;
    *ustate = saved_ustate;
    *direct_packet = saved_packet;
    return has_roomspace;
}

static void apply_dig_tag_selection(struct Packet *pckt, struct UserState *predicted_ustate)
{
    struct RoomSpace *roomspace = &predicted_ustate->render_roomspace;
    dig_cursor.untag_mode = roomspace->untag_mode;
    TbBool apply_selection = (pckt->control_flags & PCtr_LBtnHeld) != 0;
    if (predicted_ustate->roomspace_highlight_mode == drag_placement_mode) {
        apply_selection = (pckt->control_flags & PCtr_LBtnRelease) != 0;
    }
    if (!apply_selection) {
        return;
    }
    dig_tags.turn = pckt->turn;
    int changed_slab_count = apply_roomspace_dig_tag_selection(my_player_number, roomspace, dig_cursor.previous_slb_x, dig_cursor.previous_slb_y, predicted_ustate->roomspace_highlight_mode, &dig_tags);
    if (changed_slab_count > 0) {
        uint16_t previous_slb = (uint16_t)dig_cursor.previous_slb_x | ((uint16_t)dig_cursor.previous_slb_y << 8);
        set_packet_action(pckt, PckA_ApplyRoomspaceDigTag, predicted_ustate->roomspace_highlight_mode, predicted_ustate->roomspace_width, previous_slb, roomspace->untag_mode);
        play_non_3d_sample(snd_tile_dig);
    }
}

void update_dig_tag_input(void)
{
    struct Packet *pckt = get_local_packet();
    if (!dig_input_is_enabled()) {
        reset_dig_prediction();
        return;
    }
    MapSlabCoord slb_x = coord_slab(pckt->pos_x);
    MapSlabCoord slb_y = coord_slab(pckt->pos_y);
    if (((pckt->control_flags & (PCtr_LBtnRelease | PCtr_RBtnRelease)) != 0) && ((pckt->control_flags & PCtr_HeldAnyButton) == 0)) {
        cursor_release_turn = get_gameturn();
    }
    if ((pckt->control_flags & (PCtr_Gui | PCtr_MapCoordsValid | PCtr_HeldAnyButton | PCtr_LBtnRelease)) == PCtr_MapCoordsValid) {
        room_drag_start_slb_x = slb_x;
        room_drag_start_slb_y = slb_y;
    }
    if (((pckt->control_flags & PCtr_RBtnClick) != 0) && ((pckt->control_flags & PCtr_LBtnHeld) != 0)) {
        memset(&dig_cursor, 0, sizeof(dig_cursor));
        return;
    }
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0) {
        dig_cursor.cursor_button_down = false;
        return;
    }
    TbBool ignore_packet = ((pckt->action != PckA_None) && (pckt->action != PckA_SetRoomspaceHighlight)) || (get_player_user_state_prediction(get_my_player())->work_state != PSt_CtrlDungeon) || dig_input_is_blocked(pckt);
    unsigned char cursor_context = (unsigned char)((pckt->additional_packet_values & PCAdV_ContextMask) >> 1);
    TbBool click = (pckt->control_flags & PCtr_LBtnClick) != 0;
    if (click) {
        dig_cursor.cursor_button_down = !ignore_packet;
        dig_cursor.cursor_locked = false;
        dig_cursor.cursor_state = cursor_context;
    }
    if (click || dig_input_is_blocked(get_history_packet(get_local_user(), pckt->turn - 1))) {
        dig_cursor.previous_slb_x = slb_x;
        dig_cursor.previous_slb_y = slb_y;
    }
    if (ignore_packet || !dig_cursor.cursor_button_down) {
        return;
    }
    struct Packet selection_pckt = *pckt;
    if (!dig_cursor.cursor_locked) {
        selection_pckt.control_flags |= PCtr_LBtnClick;
    }
    struct UserState predicted_ustate;
    TbBool has_roomspace = get_predicted_roomspace(&selection_pckt, game.input_lag_turns, &predicted_ustate);
    if (!dig_cursor.cursor_locked) {
        if (click && !has_roomspace && (cursor_context == CSt_PowerHand) && (local_state.local_thing_under_hand != 0)) {
            dig_cursor.cursor_button_down = false;
            return;
        }
        dig_cursor.drag_start_slb_x = slb_x;
        dig_cursor.drag_start_slb_y = slb_y;
        dig_cursor.cursor_locked = has_roomspace && (cursor_context == dig_cursor.cursor_state);
    }
    if (dig_cursor.cursor_locked) {
        apply_dig_tag_selection(pckt, &predicted_ustate);
    }
    dig_cursor.previous_slb_x = slb_x;
    dig_cursor.previous_slb_y = slb_y;
}

unsigned char get_dig_prediction_render_flags(MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char base_map_flags)
{
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    if (!dig_input_is_enabled() || slab_coords_invalid(slb_x, slb_y)) {
        return base_map_flags;
    }
    unsigned char predicted_dig_tag_mode = dig_tags.slab_tag_modes[get_slab_number(slb_x, slb_y)];
    if (predicted_dig_tag_mode == DigTagMode_Untag) {
        return base_map_flags & ~(SlbAtFlg_TaggedValuable | SlbAtFlg_Unexplored);
    }
    if (predicted_dig_tag_mode != DigTagMode_Tag) {
        return base_map_flags;
    }
    if ((base_map_flags & SlbAtFlg_Valuable) == 0) {
        return base_map_flags | SlbAtFlg_Unexplored;
    }
    if (!subtile_revealed(stl_x, stl_y, my_player_number)) {
        base_map_flags |= SlbAtFlg_Unexplored;
    }
    return base_map_flags | SlbAtFlg_TaggedValuable;
}

void update_dig_prediction_cursor_preview(void)
{
    dig_render_roomspace_active = false;
    set_local_user_state_prediction(NULL);
    box_lag_compensation_x = 0;
    box_lag_compensation_y = 0;
    if (!dig_input_is_enabled()) {
        return;
    }
    const struct Packet *direct_packet = get_local_packet();
    const struct Packet *pckt = get_history_packet(get_local_user(), get_gameturn());
    reconcile_dig_predictions(&dig_tags, direct_packet->turn);
    struct UserState predicted_ustate;
    dig_render_roomspace_active = get_predicted_roomspace(pckt, game.input_lag_turns - 1, &predicted_ustate);
    set_local_user_state_prediction(&predicted_ustate);
    if (pckt == NULL) {
        return;
    }
    if ((pckt->control_flags & PCtr_LBtnRelease) != 0) {
        dig_cursor.cursor_button_down = false;
    }
    if (dig_render_roomspace_active) {
        return;
    }
    const struct UserState *ustate = get_local_user_state();
    TbBool digging_cursor = (ustate->primary_cursor_state == CSt_PickAxe) || ((ustate->primary_cursor_state == CSt_PowerHand) && ((ustate->additional_flags & UsrAF_ChosenSubTileIsHigh) != 0));
    if (((ustate->work_state == PSt_CtrlDungeon) && digging_cursor) || (predicted_ustate.work_state != ustate->work_state)) {
        map_volume_box.visible = 0;
    } else if ((direct_packet->control_flags & PCtr_MapCoordsValid) != 0) {
        box_lag_compensation_x = slab_coord(coord_slab(direct_packet->pos_x) - coord_slab(pckt->pos_x));
        box_lag_compensation_y = slab_coord(coord_slab(direct_packet->pos_y) - coord_slab(pckt->pos_y));
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
