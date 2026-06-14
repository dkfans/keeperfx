/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace_prediction.c
 *     Client-side dig prediction overlay for roomspace highlighting.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_legacy.h"
#include "config_sounds.h"
#include "map_data.h"
#include "cursor_tag.h"
#include "engine_render.h"
#include "frontmenu_ingame_evnt.h"
#include "player_data.h"
#include "player_utils.h"
#include "slab_data.h"
#include "packets.h"
#include "net_exchange_gameplay.h"
#include "roomspace.h"
#include "roomspace_prediction.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define LOCAL_DIG_TAG_EXPIRY 3

static struct {
    unsigned char slab_tag_modes[MAX_TILES_X * MAX_TILES_Y];
    unsigned char drag_base_slab_tag_modes[MAX_TILES_X * MAX_TILES_Y];
    GameTurn last_packet_turn;
    MapSlabCoord drag_start_slb_x;
    MapSlabCoord drag_start_slb_y;
    MapSlabCoord previous_slb_x;
    MapSlabCoord previous_slb_y;
    int predicted_task_count;
    int drag_base_task_count;
    TbBool untag_mode;
} local_dig_tag_prediction;

static struct Packet local_dig_roomspace_prediction;

static struct RoomSpace local_dig_render_roomspace;
static TbBool local_dig_render_roomspace_active;

static TbBool prevent_local_dig_prediction(const struct Packet *pckt)
{
    if (pckt == NULL) {
        return true;
    }
    if ((pckt->control_flags & (PCtr_Gui | PCtr_MapCoordsValid)) != PCtr_MapCoordsValid) {
        return true;
    }
    if ((pckt->action == PckA_UsePwrHandPick) || (pckt->action == PckA_UsePwrOnThing) || (battle_creature_over > 0)) {
        return true;
    }
    return false;
}

TbBool local_dig_prediction_is_enabled(void)
{
    return network_is_active() && !game.packet_load_enable && (game.input_lag_turns > 0);
}

struct RoomSpace *get_local_dig_prediction_render_roomspace(struct RoomSpace *roomspace)
{
    if (local_dig_render_roomspace_active) {
        return &local_dig_render_roomspace;
    }
    return roomspace;
}

static TbBool get_local_dig_prediction_roomspace(const struct Packet *pckt, struct PlayerInfo *predicted_player, struct RoomSpace *roomspace)
{
    if (!local_dig_prediction_is_enabled() || prevent_local_dig_prediction(pckt)) {
        return false;
    }
    *predicted_player = *get_my_player();
    unsigned char cursor_context = (unsigned char)((pckt->additional_packet_values & PCAdV_ContextMask) >> 1);
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    TbBool cursor_is_locked = (local_dig_tag_prediction.last_packet_turn != 0) && ((pckt->control_flags & (PCtr_LBtnHeld | PCtr_LBtnRelease)) != 0);
    if (!cursor_is_locked && (((pckt->control_flags & PCtr_LBtnAnyAction) == 0) || ((pckt->control_flags & PCtr_LBtnClick) != 0))) {
        predicted_player->swap_to_untag_mode = 0;
    }
    if ((cursor_context != CSt_PickAxe) && !cursor_is_locked) {
        if ((cursor_context != CSt_PowerHand) || (local_thing_under_hand != 0) || !can_dig_here(stl_x, stl_y, my_player_number, true)) {
            return false;
        }
    }
    if (local_dig_roomspace_prediction.action != PckA_None) {
        predicted_player->roomspace_highlight_mode = local_dig_roomspace_prediction.actn_par1;
        set_player_roomspace_size(predicted_player, local_dig_roomspace_prediction.actn_par2);
    }
    predicted_player->one_click_lock_cursor = cursor_is_locked;
    predicted_player->render_roomspace.drag_mode = cursor_is_locked;
    if (cursor_is_locked) {
        predicted_player->render_roomspace.drag_start_x = local_dig_tag_prediction.drag_start_slb_x;
        predicted_player->render_roomspace.drag_start_y = local_dig_tag_prediction.drag_start_slb_y;
        predicted_player->render_roomspace.untag_mode = local_dig_tag_prediction.untag_mode;
    }
    get_dungeon_highlight_user_roomspace(roomspace, predicted_player, pckt, stl_x, stl_y, local_dig_tag_prediction.slab_tag_modes);
    return true;
}

static TbBool update_predicted_build_or_sell_roomspace_preview(struct RoomSpace *roomspace, PlayerNumber plyr_idx, const struct Packet *pckt)
{
    if (prevent_local_dig_prediction(pckt)) {
        return false;
    }
    struct PlayerInfo *player = get_player(plyr_idx);
    if ((player->work_state != PSt_BuildRoom) && (player->work_state != PSt_Sell)) {
        return false;
    }
    struct Packet *direct_packet = get_packet_direct(player->packet_num);
    struct PlayerInfo saved_player = *player;
    struct Packet saved_packet = *direct_packet;
    *direct_packet = *pckt;
    apply_roomspace_packet_action(player, pckt);
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    if (player->work_state == PSt_BuildRoom) {
        update_dungeon_build_roomspace_preview(plyr_idx, stl_x, stl_y);
    } else {
        update_dungeon_sell_roomspace_preview(plyr_idx, stl_x, stl_y);
    }
    *roomspace = player->render_roomspace;
    *player = saved_player;
    *direct_packet = saved_packet;
    return true;
}

void update_local_dig_tag_prediction(void)
{
    struct Packet *pckt = get_packet(my_player_number);
    if (!local_dig_prediction_is_enabled()) {
        local_dig_render_roomspace_active = false;
        memset(&local_dig_tag_prediction, 0, sizeof(local_dig_tag_prediction));
        memset(&local_dig_roomspace_prediction, 0, sizeof(local_dig_roomspace_prediction));
        return;
    }
    if ((pckt->action == PckA_SetRoomspaceHighlight) || (pckt->action == PckA_RoomspaceHighlightToggle)) {
        local_dig_roomspace_prediction = *pckt;
    }
    if (((pckt->control_flags & PCtr_RBtnClick) != 0) && ((pckt->control_flags & PCtr_LBtnHeld) != 0)) {
        memset(&local_dig_tag_prediction, 0, sizeof(local_dig_tag_prediction));
        return;
    }
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0) {
        if ((local_dig_tag_prediction.last_packet_turn != 0) && (get_gameturn() - local_dig_tag_prediction.last_packet_turn > game.input_lag_turns + LOCAL_DIG_TAG_EXPIRY)) {
            memset(&local_dig_tag_prediction, 0, sizeof(local_dig_tag_prediction));
        }
        return;
    }
    MapSlabCoord slb_x = subtile_slab(coord_subtile(pckt->pos_x));
    MapSlabCoord slb_y = subtile_slab(coord_subtile(pckt->pos_y));
    TbBool start_selection = ((pckt->control_flags & PCtr_LBtnClick) != 0) || (local_dig_tag_prediction.last_packet_turn == 0);
    if (start_selection) {
        local_dig_tag_prediction.previous_slb_x = slb_x;
        local_dig_tag_prediction.previous_slb_y = slb_y;
        local_dig_tag_prediction.drag_start_slb_x = slb_x;
        local_dig_tag_prediction.drag_start_slb_y = slb_y;
    }
    struct RoomSpace roomspace;
    struct PlayerInfo predicted_player;
    if (!get_local_dig_prediction_roomspace(pckt, &predicted_player, &roomspace)) {
        memset(&local_dig_tag_prediction, 0, sizeof(local_dig_tag_prediction));
        return;
    }
    if (local_dig_tag_prediction.last_packet_turn == 0) {
        local_dig_tag_prediction.predicted_task_count = get_players_dungeon(get_my_player())->task_count;
    }
    local_dig_tag_prediction.untag_mode = roomspace.untag_mode;
    if (start_selection) {
        local_dig_tag_prediction.last_packet_turn = pckt->turn;
    }
    if ((predicted_player.roomspace_highlight_mode == drag_placement_mode) && start_selection) {
        memcpy(local_dig_tag_prediction.drag_base_slab_tag_modes, local_dig_tag_prediction.slab_tag_modes, sizeof(local_dig_tag_prediction.slab_tag_modes));
        local_dig_tag_prediction.drag_base_task_count = local_dig_tag_prediction.predicted_task_count;
    }
    if (predicted_player.roomspace_highlight_mode == drag_placement_mode) {
        memcpy(local_dig_tag_prediction.slab_tag_modes, local_dig_tag_prediction.drag_base_slab_tag_modes, sizeof(local_dig_tag_prediction.slab_tag_modes));
        local_dig_tag_prediction.predicted_task_count = local_dig_tag_prediction.drag_base_task_count;
    }
    if ((predicted_player.roomspace_highlight_mode == drag_placement_mode) && ((pckt->control_flags & PCtr_LBtnRelease) == 0)) {
        return;
    }
    int changed_slab_count = apply_roomspace_dig_tag_selection(my_player_number, &roomspace, local_dig_tag_prediction.previous_slb_x, local_dig_tag_prediction.previous_slb_y, predicted_player.roomspace_highlight_mode, local_dig_tag_prediction.slab_tag_modes, &local_dig_tag_prediction.predicted_task_count);
    local_dig_tag_prediction.previous_slb_x = slb_x;
    local_dig_tag_prediction.previous_slb_y = slb_y;
    if (changed_slab_count > 0) {
        local_dig_tag_prediction.last_packet_turn = pckt->turn;
        play_non_3d_sample(snd_tile_dig);
    }
}

unsigned char get_local_dig_prediction_render_flags(MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char base_map_flags)
{
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    if (!local_dig_prediction_is_enabled() || (local_dig_tag_prediction.last_packet_turn == 0) || slab_coords_invalid(slb_x, slb_y)) {
        return base_map_flags;
    }
    unsigned char predicted_dig_tag_mode = local_dig_tag_prediction.slab_tag_modes[get_slab_number(slb_x, slb_y)];
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

void update_local_dig_prediction_cursor_preview(void)
{
    struct PlayerInfo *player = get_my_player();
    const struct Packet *pckt = get_history_packet(player->packet_num, get_gameturn());
    const struct Packet *direct_packet = get_packet_direct(player->packet_num);
    if ((local_dig_roomspace_prediction.action != PckA_None) && ((GameTurnDelta)(direct_packet->turn - local_dig_roomspace_prediction.turn) >= 0)) {
        memset(&local_dig_roomspace_prediction, 0, sizeof(local_dig_roomspace_prediction));
    }
    struct RoomSpace roomspace;
    struct PlayerInfo predicted_player;
    if (!get_local_dig_prediction_roomspace(pckt, &predicted_player, &roomspace)) {
        if (local_dig_prediction_is_enabled() && update_predicted_build_or_sell_roomspace_preview(&local_dig_render_roomspace, player->id_number, pckt)) {
            local_dig_render_roomspace_active = true;
            box_lag_compensation_x = 0;
            box_lag_compensation_y = 0;
            return;
        }
        local_dig_render_roomspace_active = false;
        if (!local_dig_prediction_is_enabled()) {
            return;
        }
        if ((pckt != NULL) && ((player->primary_cursor_state == CSt_PickAxe) || ((player->primary_cursor_state == CSt_PowerHand) && ((player->additional_flags & PlaAF_ChosenSubTileIsHigh) != 0)))) {
            map_volume_box.visible = 0;
            box_lag_compensation_x = 0;
            box_lag_compensation_y = 0;
        }
        return;
    }
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    local_dig_render_roomspace = roomspace;
    local_dig_render_roomspace_active = true;
    tag_cursor_blocks_dig(&predicted_player, pckt, &local_dig_render_roomspace, stl_x, stl_y, true);
    box_lag_compensation_x = 0;
    box_lag_compensation_y = 0;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
