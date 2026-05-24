/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace_prediction.c
 *     Client-side dig prediction overlay for roomspace highlighting.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_legacy.h"
#include "map_data.h"
#include "cursor_tag.h"
#include "engine_render.h"
#include "player_data.h"
#include "slab_data.h"
#include "packets.h"
#include "net_exchange_gameplay.h"
#include "tasks_list.h"
#include "roomspace.h"
#include "roomspace_prediction.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
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
    enum DigTagMode dig_tag_mode;
} local_dig_tag_prediction;

static struct Packet local_dig_roomspace_prediction;

static struct RoomSpace local_dig_render_roomspace;
static TbBool local_dig_render_roomspace_active;

TbBool local_dig_prediction_is_enabled(void)
{
    return ((game.system_flags & GSF_NetworkActive) != 0) && !game.packet_load_enable && (game.input_lag_turns > 0);
}

struct RoomSpace *get_local_dig_prediction_render_roomspace(struct RoomSpace *roomspace)
{
    if (local_dig_render_roomspace_active) {
        return &local_dig_render_roomspace;
    }
    return roomspace;
}

static enum DigTagMode get_local_predicted_slab_dig_tag_mode(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    if ((local_dig_tag_prediction.last_packet_turn != 0) && !slab_coords_invalid(slb_x, slb_y)) {
        unsigned char predicted_dig_tag_mode = local_dig_tag_prediction.slab_tag_modes[get_slab_number(slb_x, slb_y)];
        if (predicted_dig_tag_mode == DigTagMode_Tag) {
            return DigTagMode_Untag;
        }
        if (predicted_dig_tag_mode == DigTagMode_Untag) {
            return DigTagMode_Tag;
        }
    }
    if (find_from_task_list_by_subtile(my_player_number, stl_x, stl_y) != -1) {
        return DigTagMode_Untag;
    }
    return DigTagMode_Tag;
}

static enum DigTagMode get_local_predicted_packet_dig_tag_mode(const struct Packet *pckt)
{
    if (((pckt->control_flags & PCtr_LBtnClick) == 0) && (local_dig_tag_prediction.last_packet_turn != 0)) {
        return local_dig_tag_prediction.dig_tag_mode;
    }
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    return get_local_predicted_slab_dig_tag_mode(stl_x, stl_y);
}

static TbBool get_local_dig_prediction_roomspace(const struct Packet *pckt, struct RoomSpace *roomspace, unsigned char *highlight_mode)
{
    if (!local_dig_prediction_is_enabled() || (pckt == NULL) || ((pckt->control_flags & PCtr_MapCoordsValid) == 0)) {
        return false;
    }
    struct PlayerInfo *player = get_my_player();
    unsigned char cursor_context = (unsigned char)((pckt->additional_packet_values & PCAdV_ContextMask) >> 1);
    if ((cursor_context != CSt_PickAxe) && ((cursor_context != CSt_PowerHand) || (local_thing_under_hand != 0))) {
        return false;
    }
    int roomspace_width = player->user_defined_roomspace_width;
    *highlight_mode = player->roomspace_highlight_mode;
    if (local_dig_roomspace_prediction.action != PckA_None) {
        *highlight_mode = local_dig_roomspace_prediction.actn_par1;
        roomspace_width = local_dig_roomspace_prediction.actn_par2;
    }
    MapSlabCoord slb_x = subtile_slab(coord_subtile(pckt->pos_x));
    MapSlabCoord slb_y = subtile_slab(coord_subtile(pckt->pos_y));
    MapSlabCoord drag_start_slb_x = slb_x;
    MapSlabCoord drag_start_slb_y = slb_y;
    if ((*highlight_mode == 1) && ((pckt->control_flags & PCtr_LBtnAnyAction) != 0) && (local_dig_tag_prediction.last_packet_turn != 0)) {
        drag_start_slb_x = local_dig_tag_prediction.drag_start_slb_x;
        drag_start_slb_y = local_dig_tag_prediction.drag_start_slb_y;
    }
    *roomspace = create_dig_highlight_roomspace(player->render_roomspace, *highlight_mode, roomspace_width, drag_start_slb_x, drag_start_slb_y, slb_x, slb_y);
    return true;
}

void update_local_dig_tag_prediction(void)
{
    local_dig_render_roomspace_active = false;
    struct Packet *pckt = get_packet(my_player_number);
    if (!local_dig_prediction_is_enabled()) {
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
        if ((local_dig_tag_prediction.last_packet_turn != 0) && (get_gameturn() - local_dig_tag_prediction.last_packet_turn > game.input_lag_turns + 3)) {
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
    unsigned char highlight_mode;
    if (!get_local_dig_prediction_roomspace(pckt, &roomspace, &highlight_mode)) {
        memset(&local_dig_tag_prediction, 0, sizeof(local_dig_tag_prediction));
        return;
    }
    if (local_dig_tag_prediction.last_packet_turn == 0) {
        local_dig_tag_prediction.predicted_task_count = get_players_dungeon(get_my_player())->task_count;
    }
    local_dig_tag_prediction.dig_tag_mode = get_local_predicted_packet_dig_tag_mode(pckt);
    if (start_selection) {
        local_dig_tag_prediction.last_packet_turn = pckt->turn;
    }
    if ((highlight_mode == 1) && start_selection) {
        memcpy(local_dig_tag_prediction.drag_base_slab_tag_modes, local_dig_tag_prediction.slab_tag_modes, sizeof(local_dig_tag_prediction.slab_tag_modes));
        local_dig_tag_prediction.drag_base_task_count = local_dig_tag_prediction.predicted_task_count;
    }
    if (highlight_mode == 1) {
        memcpy(local_dig_tag_prediction.slab_tag_modes, local_dig_tag_prediction.drag_base_slab_tag_modes, sizeof(local_dig_tag_prediction.slab_tag_modes));
        local_dig_tag_prediction.predicted_task_count = local_dig_tag_prediction.drag_base_task_count;
    }
    int changed_slab_count = apply_roomspace_dig_tag_selection(my_player_number, &roomspace, local_dig_tag_prediction.previous_slb_x, local_dig_tag_prediction.previous_slb_y, highlight_mode, local_dig_tag_prediction.dig_tag_mode, local_dig_tag_prediction.slab_tag_modes, &local_dig_tag_prediction.predicted_task_count);
    local_dig_tag_prediction.previous_slb_x = slb_x;
    local_dig_tag_prediction.previous_slb_y = slb_y;
    if ((changed_slab_count > 0) && ((highlight_mode != 1) || ((pckt->control_flags & PCtr_LBtnRelease) != 0))) {
        local_dig_tag_prediction.last_packet_turn = pckt->turn;
        play_non_3d_sample(118);
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
    struct Packet *direct_packet = get_packet_direct(player->packet_num);
    if ((local_dig_roomspace_prediction.action != PckA_None) && ((GameTurnDelta)(direct_packet->turn - local_dig_roomspace_prediction.turn) >= 0)) {
        memset(&local_dig_roomspace_prediction, 0, sizeof(local_dig_roomspace_prediction));
    }
    struct RoomSpace roomspace;
    unsigned char highlight_mode;
    if (!get_local_dig_prediction_roomspace(pckt, &roomspace, &highlight_mode)) {
        return;
    }
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    enum DigTagMode dig_tag_mode = get_local_predicted_slab_dig_tag_mode(stl_x, stl_y);
    if ((pckt->control_flags & PCtr_LBtnAnyAction) != 0) {
        dig_tag_mode = get_local_predicted_packet_dig_tag_mode(pckt);
    }
    roomspace.untag_mode = (dig_tag_mode == DigTagMode_Untag);
    local_dig_render_roomspace = check_roomspace_for_diggable_slabs(roomspace, my_player_number, local_dig_tag_prediction.slab_tag_modes);
    local_dig_render_roomspace_active = true;
    struct Packet saved_packet = *direct_packet;
    *direct_packet = *pckt;
    tag_cursor_blocks_dig(my_player_number, stl_x, stl_y, true);
    *direct_packet = saved_packet;
    box_lag_compensation_x = 0;
    box_lag_compensation_y = 0;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
