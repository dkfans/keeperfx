/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets_input.c
 *     Packet processing routines.
 * @par Purpose:
 *     Functions for creating and executing packets.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     30 Jan 2009 - 10 Mar 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "net_game.h"
#include "pre_inc.h"
#include "config_players.h"
#include "config_powerhands.h"
#include "packets.h"
#include "player_data.h"
#include "map_data.h"
#include "slab_data.h"
#include "frontmenu_ingame_tabs.h"
#include "front_input.h"
#include "game_loop.h"
#include "bflib_sound.h"
#include "config_sounds.h"
#include "gui_soundmsgs.h"
#include "player_instances.h"
#include "power_hand.h"
#include "frontend.h"
#include "config_players.h"
#include "magic_powers.h"
#include "player_utils.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "room_util.h"
#include "creature_states.h"
#include "kjm_input.h"
#include "config_effects.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "room_workshop.h"
#include "cursor_tag.h"
#include "engine_render.h"
#include "config_settings.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#define HAND_DROP_TARGET_MAX_SUBTILES 3

extern void update_double_click_detection(NetUserId user);

// Returns false if mouse is on map edges or on GUI
TbBool is_mouse_on_map(struct Packet* pckt)
{
    int x = (pckt->pos_x >> 8) / 3;
    int y = (pckt->pos_y >> 8) / 3;
    if (x == 0) {return false;}
    if (y == 0) {return false;}
    if (x == game.map_tiles_x-1) {return false;}
    if (y == game.map_tiles_y-1) {return false;}
    return true;
}

static void remember_cursor_subtile(NetUserId user)
{
    struct Packet* pckt = get_packet(user);
    struct UserState* ustate = get_user_state(user);
    ustate->previous_cursor_subtile_x = ustate->cursor_subtile_x;
    ustate->previous_cursor_subtile_y = ustate->cursor_subtile_y;
    ustate->cursor_subtile_x = coord_subtile(pckt->pos_x);
    ustate->cursor_subtile_y = coord_subtile(pckt->pos_y);
}

struct Thing *get_thing_under_hand(NetUserId user, MapCoord x, MapCoord y)
{
    struct PlayerInfo *player = get_user_player(user);
    struct UserState* ustate = get_user_state(user);
    struct Thing *thing;

    switch (ustate->work_state) {
    case PSt_Slap:
        return get_creature_near_to_be_keeper_power_target(x, y, PwrK_SLAP, player->id_number);
    case PSt_CtrlPassngr:
        return get_creature_near_and_owned_by(x, y, player->id_number, CREATURE_ANY);
    case PSt_FreeCtrlPassngr:
        return get_creature_near_and_owned_by(x, y, -1, CREATURE_ANY);
    case PSt_CtrlDirect:
        return get_creature_near_for_controlling(player->id_number, user, x, y);
    case PSt_FreeCtrlDirect:
        return get_creature_near(x, y);
    case PSt_CreatrQuery:
    case PSt_CreatrInfo:
        thing = get_creature_near(x, y);
        if (thing_is_creature(thing) && can_thing_be_queried(thing, player->id_number)) {
            return thing;
        }
        break;
    case PST_CastPowerOnTarget:
        return get_creature_near_to_be_keeper_power_target(x, y, ustate->chosen_power_kind, player->id_number);
    }
    return INVALID_THING;
}

TbBool process_dungeon_control_packet_dungeon_build_room(NetUserId user)
{
    struct PlayerInfo* player = get_player(get_net_user_player_number(user));
    struct UserState* ustate = get_user_state(user);
    struct Packet* pckt = get_packet(user);
    MapCoord x = (pckt->pos_x);
    MapCoord y = (pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (ustate->cursor_button_down != 0))
        {
            ustate->cursor_button_down = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    TbBool can_place_room = update_dungeon_build_roomspace_preview(user, stl_x, stl_y);
    if ( (ustate->roomspace_mode == drag_placement_mode) && (ustate->roomspace_drag_paint_mode == false) )
    {
       if ((pckt->control_flags & PCtr_LBtnRelease) != PCtr_LBtnRelease)
       {
           return false;
       }
    }
    if (ustate->roomspace_mode != drag_placement_mode) // allows the user to hold the left mouse to use "paint mode"
    {
        if ((pckt->control_flags & PCtr_LBtnClick) == 0)
        {
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (ustate->cursor_button_down != 0))
            {
                ustate->cursor_button_down = 0;
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            return false;
        }
    }
    else if ((pckt->control_flags & PCtr_LBtnHeld) == PCtr_LBtnHeld)
    {
        if ( (ustate->boxsize == 0) || (!can_build_room_at_slab(player->id_number, ustate->chosen_room_kind, subtile_slab(stl_x), subtile_slab(stl_y))) )
        {
            return false; //stops attempts at invalid rooms, if left mouse button held (i.e. don't repeat failure sound repeatedly in paint mode)
        }
    }
    if (!can_place_room)
    {
        if (can_build_room_at_slab(player->id_number, ustate->chosen_room_kind, subtile_slab(stl_x), subtile_slab(stl_y)))
        {
            struct Dungeon* dungeon = get_dungeon(player->id_number);
            if (ustate->render_roomspace.total_roomspace_cost > dungeon->total_money_owned)
            {
                output_player_message(player->id_number, SMsg_GoldNotEnough, 0);
            }
        }
        else
        {
            play_player_sample(player->id_number, snd_refusal);
        }
        unset_packet_control(pckt, PCtr_LBtnClick);
      return false;
    }
    if (ustate->boxsize > 0)
    {
        keeper_start_roomspace(user, &ustate->render_roomspace, ustate->chosen_room_kind);
    }
    else
    {
        play_player_sample(player->id_number, snd_refusal);
    }
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

static enum RightReleaseHandUse get_right_release_hand_use(const struct UserState *ustate)
{
    if (ustate->ignore_next_PCtr_RBtnRelease && !ustate->one_click_lock_cursor) {
        return RRelease_Ignored;
    }
    if (ustate->cursor_button_down == 0) {
        return RRelease_Unused;
    }
    if ((ustate->num_things_in_hand != 0) && !ustate->one_click_lock_cursor) {
        return RRelease_DropHeldThing;
    }
    return RRelease_Slap;
}

static void apply_packet_hand_drop_target(struct UserState *ustate, const struct Packet *pckt)
{
    if ((pckt->action != PckA_SetHandDropTarget) || flag_is_set(ustate->additional_flags, UsrAF_ChosenSubTileIsHigh)) {
        return;
    }
    ustate->thing_under_hand = 0;
    struct Thing *thing = thing_get(pckt->actn_par1);
    if (!thing_exists(thing) || !thing_is_ready_for_hand_or_slap(thing, ustate->player_id)) {
        return;
    }
    int32_t distance_x = abs((MapSubtlCoord)thing->mappos.x.stl.num - coord_subtile(pckt->pos_x));
    int32_t distance_y = abs((MapSubtlCoord)thing->mappos.y.stl.num - coord_subtile(pckt->pos_y));
    if (max(distance_x, distance_y) <= HAND_DROP_TARGET_MAX_SUBTILES) {
        ustate->thing_under_hand = thing->index;
    }
}

enum HandCursorUse update_hand_cursor_state(struct UserState *ustate, const struct Packet *pckt, TbBool local_prediction)
{
    PlayerNumber plyr_idx = ustate->player_id;
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0) {
        ustate->secondary_cursor_state = CSt_DefaultArrow;
    }
    clear_flag(ustate->additional_flags, UsrAF_ChosenSubTileIsHigh);
    if ((ustate->secondary_cursor_state != CSt_DefaultArrow) && (ustate->secondary_cursor_state != CSt_PowerHand)) {
        if (ustate->instance_num == PI_Grab) {
            return HandCursor_Grabbing;
        }
        return HandCursor_Remove;
    }
    enum HandCursorUse hand_use = HandCursor_Idle;
    struct Thing *thing = get_nearest_thing_for_hand_or_slap(plyr_idx, pckt->pos_x, pckt->pos_y, local_prediction);
    if (!thing_is_invalid(thing) && !ustate->one_click_lock_cursor) {
        SYNCDBG(19,"Thing %d under hand at (%d,%d)",(int)thing->index,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
        ustate->thing_under_hand = thing->index;
        hand_use = HandCursor_Show;
    }
    struct Thing *heldtng = get_first_held_thing(ustate);
    if (heldtng == NULL) {
        return hand_use;
    }
    if (ustate->one_click_lock_cursor || (!can_drop_thing_here(stl_x, stl_y, plyr_idx, is_creature_droppable_on_path(heldtng)) && can_dig_here(stl_x, stl_y, plyr_idx, true))) {
        set_flag(ustate->additional_flags, UsrAF_ChosenSubTileIsHigh);
        ustate->thing_under_hand = 0;
    }
    return HandCursor_Show;
}

void press_cursor_buttons(struct UserState *ustate, const struct Packet *pckt)
{
    if ((pckt->control_flags & (PCtr_LBtnClick | PCtr_RBtnClick)) == 0) {
        return;
    }
    if ((pckt->control_flags & PCtr_LBtnClick) != 0) {
        ustate->secondary_cursor_state = ustate->primary_cursor_state;
    }
    ustate->cursor_clicked_subtile_x = coord_subtile(pckt->pos_x);
    ustate->cursor_clicked_subtile_y = coord_subtile(pckt->pos_y);
    ustate->cursor_button_down = 1;
}

TbBool begin_left_release(struct UserState *ustate, struct Packet *pckt)
{
    TbBool left_held = flag_is_set(pckt->control_flags, PCtr_LBtnHeld);
    TbBool right_held = flag_is_set(pckt->control_flags, PCtr_RBtnHeld);
    if ((left_held || flag_is_set(pckt->control_flags, PCtr_LBtnRelease)) && (ustate->secondary_cursor_state == CSt_DefaultArrow)) {
        ustate->secondary_cursor_state = ustate->primary_cursor_state;
    }
    if (ustate->cursor_button_down != 0) {
        if (left_held) {
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        if (right_held) {
            unset_packet_control(pckt, PCtr_RBtnRelease);
        }
    }
    if (!flag_is_set(pckt->control_flags, PCtr_LBtnRelease)) {
        return false;
    }
    if (ustate->ignore_next_PCtr_LBtnRelease) {
        ustate->ignore_next_PCtr_LBtnRelease = false;
        if (!right_held) {
            ustate->cursor_button_down = 0;
        }
        unset_packet_control(pckt, PCtr_LBtnRelease);
        return false;
    }
    return (ustate->cursor_button_down != 0);
}

void finish_left_release(struct UserState *ustate, struct Packet *pckt)
{
    TbBool right_held = flag_is_set(pckt->control_flags, PCtr_RBtnHeld);
    if (ustate->render_roomspace.drag_mode && right_held) {
        ustate->render_roomspace.untag_mode = !ustate->render_roomspace.untag_mode;
    }
    if (!right_held) {
        ustate->cursor_button_down = 0;
        ustate->one_click_lock_cursor = false;
        ustate->render_roomspace.drag_mode = false;
    }
    ustate->secondary_cursor_state = CSt_DefaultArrow;
    unset_packet_control(pckt, PCtr_LBtnRelease);
}

enum RightReleaseHandUse begin_right_release(struct UserState *ustate, const struct Packet *pckt)
{
    if (!flag_is_set(pckt->control_flags, PCtr_RBtnRelease)) {
        return RRelease_Unused;
    }
    enum RightReleaseHandUse release_use = get_right_release_hand_use(ustate);
    if (release_use == RRelease_Ignored) {
        ustate->ignore_next_PCtr_RBtnRelease = false;
    }
    if (release_use == RRelease_DropHeldThing) {
        apply_packet_hand_drop_target(ustate, pckt);
    }
    return release_use;
}

void finish_cursor_release(struct UserState *ustate, struct Packet *pckt, enum RightReleaseHandUse release_use, TbBool dropped)
{
    TbBool left_held = flag_is_set(pckt->control_flags, PCtr_LBtnHeld);
    TbBool release_handled = (release_use != RRelease_Unused) && ((release_use != RRelease_DropHeldThing) || dropped);
    if (release_handled) {
        if (!left_held) {
            if (release_use == RRelease_Slap) {
                ustate->one_click_lock_cursor = false;
            }
            ustate->cursor_button_down = 0;
        }
        unset_packet_control(pckt, PCtr_RBtnRelease);
    }
    if ((ustate->cursor_button_down == 0) || !ustate->one_click_lock_cursor) {
        ustate->swap_to_untag_mode = 0;
    }
    if ((ustate->cursor_button_down == 0) && !left_held) {
        ustate->one_click_lock_cursor = false;
    }
}

static void process_dungeon_power_hand_state(NetUserId user)
{
    struct PlayerInfo* player = get_player(get_net_user_player_number(user));
    struct UserState* ustate = get_user_state(user);
    struct Packet* pckt = get_packet(user);
    enum HandCursorUse hand_use = update_hand_cursor_state(ustate, pckt, false);
    if (hand_use == HandCursor_Grabbing) {
        return;
    }
    if (hand_use == HandCursor_Remove) {
        delete_power_hand(user);
        return;
    }
    if ((hand_use == HandCursor_Show) && (ustate->hand_thing_idx == 0)) {
        create_power_hand(user);
    }
    struct Thing *heldtng = get_first_held_thing(ustate);
    if ((heldtng != NULL) && !flag_is_set(ustate->additional_flags, UsrAF_ChosenSubTileIsHigh)) {
        tag_cursor_blocks_thing_in_hand(user, coord_subtile(pckt->pos_x), coord_subtile(pckt->pos_y));
    }
    if ((ustate->hand_thing_idx == 0) || (get_hand_instance_animation(ustate->instance_num) != HndA_None)) {
        return;
    }
    TbBool held_shown = set_idle_hand_graphic(ustate, thing_get(ustate->hand_thing_idx), player, heldtng, get_gameturn());
    if (heldtng != NULL) {
        set_flag_value(heldtng->rendering_flags, TRF_Invisible, !held_shown);
    }
}

TbBool process_dungeon_control_packet_dungeon_control(NetUserId user)
{
    struct Thing *thing;
    struct PlayerInfo* player = get_player(get_net_user_player_number(user));
    struct UserState* ustate = get_user_state(user);
    PlayerNumber plyr_idx = player->id_number;
    struct Packet* pckt = get_packet(user);
    MapCoord x = pckt->pos_x;
    MapCoord y = pckt->pos_y;
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    TbBool at_limit = false;
    TbBool apply_roomspace_tag = pckt->action == PckA_ApplyRoomspaceDigTag;
    TbBool thing_target_action = apply_roomspace_tag || (pckt->action == PckA_UsePwrHandPick) || (pckt->action == PckA_UsePwrOnThing);
    unsigned char box_colour;
    ustate->render_roomspace.highlight_mode = (ustate->prefs[UPref_StartingHighlightMode] != 0);
    ustate->render_roomspace.drag_mode = ustate->one_click_lock_cursor;
    ustate->pickup_all_gold = (pckt->additional_packet_values & PCAdV_RotatePressed);
    process_dungeon_power_hand_state(user);
    if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
    {
        if (apply_roomspace_tag || (ustate->primary_cursor_state == CSt_PickAxe) || ((ustate->primary_cursor_state == CSt_PowerHand) && ((ustate->additional_flags & UsrAF_ChosenSubTileIsHigh) != 0))) {
            ustate->thing_under_hand = 0;
            get_dungeon_highlight_user_roomspace(&ustate->render_roomspace, player, user, pckt, stl_x, stl_y, NULL);
            if (apply_roomspace_tag) {
                ustate->render_roomspace.untag_mode = pckt->actn_par4;
                ustate->render_roomspace = check_roomspace_for_diggable_slabs(ustate->render_roomspace, plyr_idx, NULL);
            }
            box_colour = tag_cursor_blocks_dig(player, user, pckt, &ustate->render_roomspace, stl_x, stl_y, ustate->full_slab_cursor);
            at_limit = (box_colour == SLC_REDYELLOW) || (box_colour == SLC_REDFLASH);
            if (apply_roomspace_tag) {
                MapSlabCoord previous_slb_x = (uint16_t)pckt->actn_par3 & 0xFF;
                MapSlabCoord previous_slb_y = (uint16_t)pckt->actn_par3 >> 8;
                if (apply_roomspace_dig_tag_selection(plyr_idx, &ustate->render_roomspace, previous_slb_x, previous_slb_y, ustate->roomspace_highlight_mode, NULL) > 0) {
                    ustate->last_dig_order_turn = get_gameturn();
                    ustate->last_dig_order_was_removal = ustate->render_roomspace.untag_mode;
                }
            }
        }
        press_cursor_buttons(ustate, pckt);
        if ((pckt->control_flags & PCtr_LBtnClick) != 0)
        {
            switch (ustate->primary_cursor_state)
            {
                case CSt_PowerHand:
                    if (ustate->thing_under_hand != 0) {
                        break;
                    }
                    // fall through
                case CSt_PickAxe:
                    if (!ustate->render_roomspace.drag_mode && at_limit) {
                        play_player_sample(player->id_number, snd_refusal);
                        output_player_message(player->id_number, SMsg_WorkerJobsLimit, 500); // remind the user that the task limit (MAPTASKS_COUNT) has been reached
                    }
                    break;
                case CSt_DoorKey:
                    thing = get_door_for_position(ustate->cursor_clicked_subtile_x, ustate->cursor_clicked_subtile_y);
                    if (thing_is_invalid(thing))
                    {
                        ERRORLOG("Door thing not found at map pos (%d,%d)",(int)ustate->cursor_clicked_subtile_x,(int)ustate->cursor_clicked_subtile_y);
                        break;
                    }
                    if (thing->door.is_locked)
                        unlock_door(thing);
                    else
                        lock_door(thing);
                    break;
            }
            unset_packet_control(pckt, PCtr_LBtnClick);
        }
        unset_packet_control(pckt, PCtr_RBtnClick);
    }

    if (begin_left_release(ustate, pckt)) {
        TbBool direct_control_target = !thing_target_action && (ustate->thing_under_hand != 0) && (ustate->input_crtr_control != 0);
        if (direct_control_target && (ustate->things_in_hand[0] != ustate->thing_under_hand)) {
            thing = get_creature_near_for_controlling(player->id_number, user, x, y);
            if (!thing_is_invalid(thing)) {
                ustate->thing_under_hand = thing->index;
            } else {
                thing = thing_get(ustate->thing_under_hand);
            }
            set_user_work_state(user, PSt_CtrlDirect, PwrK_POSSESS);
            if (magic_use_available_power_on_thing(plyr_idx, user,PwrK_POSSESS, 0, stl_x, stl_y, thing, PwMod_Default) == Lb_FAIL) {
                set_user_work_state(user, ustate->continue_work_state, 0);
            }
        } else if (!thing_target_action && ustate->input_crtr_query != 0) {
            thing = get_creature_near(x, y);
            if (!can_thing_be_queried(thing, plyr_idx)) {
                ustate->thing_under_hand = 0;
            } else {
                ustate->thing_under_hand = thing->index;
            }
            if ((ustate->thing_under_hand > 0) && (ustate->thing_under_hand != get_user_state(user)->controlled_thing_idx)) {
                if (is_local_user(user)) {
                    turn_off_all_panel_menus();
                    turn_on_menu(GMnu_CREATURE_QUERY1);
                }
                ustate->influenced_thing_idx = ustate->thing_under_hand;
                ustate->influenced_thing_creation = thing->creation_turn;
                set_user_work_state(user, PSt_CreatrQuery, 0);
                set_user_instance(user, PI_QueryCrtr, 0);
            }
        } else if (!thing_target_action && ustate->secondary_cursor_state == ustate->primary_cursor_state) {
            if ((ustate->primary_cursor_state == CSt_PickAxe) || (ustate->primary_cursor_state == CSt_PowerHand)) {
                if (ustate->thing_under_hand != 0) {
                    // TODO SPELLS it's not a good idea to use this directly; change to magic_use_available_power_on_*()
                    use_power_hand(plyr_idx, user, stl_x, stl_y, 0);
                } else if (ustate->render_roomspace.drag_mode && at_limit) {
                    play_player_sample(player->id_number, snd_refusal);
                    output_player_message(player->id_number, SMsg_WorkerJobsLimit, 500); // remind the user that the task limit (MAPTASKS_COUNT) has been reached
                }
            }
        }
        finish_left_release(ustate, pckt);
    }

    enum RightReleaseHandUse release_use = begin_right_release(ustate, pckt);
    TbBool dropped = false;
    if (release_use == RRelease_DropHeldThing) {
        dropped = dump_first_held_thing_on_map(player->id_number, user, stl_x, stl_y, 1);
    }
    if ((release_use == RRelease_Slap) && !thing_target_action && (ustate->primary_cursor_state == CSt_PowerHand) && !ustate->one_click_lock_cursor) {
        thing = get_nearest_thing_for_slap(plyr_idx, subtile_coord_center(stl_x), subtile_coord_center(stl_y));
        if (!thing_is_invalid(thing)) {
            magic_use_available_power_on_thing(plyr_idx, user, PwrK_SLAP, 0, stl_x, stl_y, thing, PwMod_Default);
        }
    }
    finish_cursor_release(ustate, pckt, release_use, dropped);
    return true;
}

TbBool process_dungeon_control_packet_sell_operation(NetUserId user)
{
    struct PlayerInfo* player = get_player(get_net_user_player_number(user));
    struct UserState* ustate = get_user_state(user);
    PlayerNumber plyr_idx = player->id_number;
    struct Packet* pckt = get_packet(user);
    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (ustate->full_slab_cursor != 0))
        {
            ustate->full_slab_cursor = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    MapCoord x = (pckt->pos_x);
    MapCoord y = (pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    update_dungeon_sell_roomspace_preview(user, stl_x, stl_y);
    if (ustate->roomspace_mode == drag_placement_mode)
    {
       if ((pckt->control_flags & PCtr_LBtnRelease) != PCtr_LBtnRelease)
       {
           return false;
       }
    }
    else
    {
        if ((pckt->control_flags & PCtr_LBtnClick) == 0)
        {
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (ustate->full_slab_cursor != 0))
            {
                ustate->full_slab_cursor = 0;
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            return false;
        }
    }
    if (ustate->full_slab_cursor)
    {
        //Slab Mode
        if (ustate->render_roomspace.slab_count > 0)
        {
            keeper_start_roomspace(user, &ustate->render_roomspace, RoK_SELL);
        }
        else
        {
            struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
            if (slabmap_owner(slb) != plyr_idx)
            {
                SYNCDBG(7,"Player %d can't sell item on %s owned by player %d at subtile (%d,%d).", (int)plyr_idx, slab_code_name(slb->kind), (int)slabmap_owner(slb), (int)stl_x, (int)stl_y);
                unset_packet_control(pckt, PCtr_LBtnClick);
                return false;
            }
            // Trying to sell room
            if (subtile_is_sellable_room(plyr_idx, stl_x, stl_y))
            {
                player_sell_room_at_subtile(plyr_idx, stl_x, stl_y);
            } else
                // Trying to sell door
            if (player_sell_door_at_subtile(plyr_idx, stl_x, stl_y))
            {
                // Nothing to do here - door already sold
            } else
                // Trying to sell trap
            if (player_sell_trap_at_subtile(plyr_idx, stl_x, stl_y, ustate->full_slab_cursor != 0))
            {
                // Nothing to do here - trap already sold
            } else
            {
                SYNCDBG(7,"Nothing to sell for player %d request",(int)plyr_idx);
            }
        }
    }
    else
    {
        struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
        if (slabmap_owner(slb) != plyr_idx)
        {
            SYNCDBG(7,"Player %d can't sell item on %s owned by player %d at subtile (%d,%d).", (int)plyr_idx, slab_code_name(slb->kind), (int)slabmap_owner(slb), (int)stl_x, (int)stl_y);
            unset_packet_control(pckt, PCtr_LBtnClick);
            return false;
        }
        // Subtile Mode
        if (player_sell_trap_at_subtile(plyr_idx, stl_x, stl_y, ustate->full_slab_cursor != 0))
        {
            // Nothing to do here - trap already sold
        } else
        if (player_sell_door_at_subtile(plyr_idx, stl_x, stl_y))
        {
            // Nothing to do here - door already sold
        } else
        if (subtile_is_sellable_room(plyr_idx, stl_x, stl_y))
        {
            player_sell_room_at_subtile(plyr_idx, stl_x, stl_y);
        }
        else
        {
            SYNCDBG(7,"Nothing to sell for player %d request",(int)plyr_idx);
        }
    }
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

TbBool process_dungeon_control_packet_dungeon_place_trap(NetUserId user)
{
    struct PlayerInfo* player = get_player(get_net_user_player_number(user));
    struct UserState* ustate = get_user_state(user);
    PlayerNumber plyr_idx = player->id_number;
    struct Packet* pckt = get_packet(user);
    MapCoord x = (pckt->pos_x);
    MapCoord y = (pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);

    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (ustate->cursor_button_down != 0))
        {
            ustate->cursor_button_down = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    TbBool can_place = tag_cursor_blocks_place_trap(user, stl_x, stl_y, ustate->chosen_trap_kind);
    if ((pckt->control_flags & PCtr_LBtnClick) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (ustate->cursor_button_down != 0))
        {
            ustate->cursor_button_down = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    if (can_place == 0)
    {
        play_player_sample(player->id_number, snd_refusal);
        unset_packet_control(pckt, PCtr_LBtnClick);
        return false;
    }
    if (!player_place_trap_at(stl_x, stl_y, plyr_idx, ustate->chosen_trap_kind))
    {
        unset_packet_control(pckt, PCtr_LBtnClick);
        return false;
    }
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

TbBool process_dungeon_control_packet_clicks(NetUserId user)
{
    struct Thing *thing;
    PowerKind pwkind;
    struct PlayerInfo* player = get_player(get_net_user_player_number(user));
    struct UserState* ustate = get_user_state(user);
    const PlayerNumber plyr_idx = player->id_number;
    struct Packet* pckt = get_packet(user);
    SYNCDBG(6,"Starting for user %d state %s",user,player_state_code_name(ustate->work_state));
    TbBool thing_target_action = (pckt->action == PckA_UsePwrHandPick) || (pckt->action == PckA_UsePwrOnThing);
    ustate->full_slab_cursor = 1;
    ustate->primary_cursor_state = (pckt->additional_packet_values & PCAdV_ContextMask) >> 1;
    packet_left_button_double_clicked[user] = 0;
    ustate->mouse_on_map = is_mouse_on_map(pckt);
    remember_cursor_subtile(user);
    process_dungeon_control_packet_spell_overcharge(user, pckt);
    if (user == get_displayed_user()) {
        map_volume_box.visible = 0;
    }
    if (flag_is_set(pckt->control_flags,PCtr_Gui))
        return false;
    TbBool ret = true;
    if ((pckt->control_flags & PCtr_RBtnHeld) != 0)
    {
    } else
    if ((pckt->control_flags & PCtr_RBtnRelease) == 0)
    {
        ustate->boxsize = 1;
    }
    update_double_click_detection(user);
    ustate->thing_under_hand = 0;
    MapCoord x = (pckt->pos_x);
    MapCoord y = (pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);

    long i;
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    pwkind = ustate->chosen_power_kind;
    thing = get_thing_under_hand(user, x, y);
    if (!thing_is_invalid(thing)) {
        ustate->thing_under_hand = thing->index;
    }

    switch (ustate->work_state)
    {
        case PSt_CtrlDungeon:
            process_dungeon_control_packet_dungeon_control(user);
            break;
        case PSt_BuildRoom:
            process_dungeon_control_packet_dungeon_build_room(user);
            break;
        case PSt_CallToArms:
        case PSt_SightOfEvil:
        case PSt_CreateDigger:
        case PSt_CastPowerOnSubtile:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                i = get_power_overcharge_level(ustate);
                magic_use_available_power_on_subtile(plyr_idx, user,pwkind, i, stl_x, stl_y, PwCast_None, PwMod_Default);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_Slap:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (((pckt->control_flags & PCtr_MapCoordsValid) != 0) || thing_target_action)) {
                if (!thing_target_action) {
                    magic_use_available_power_on_thing(plyr_idx, user,PwrK_SLAP, 0, stl_x, stl_y, thing, PwMod_Default);
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_CtrlPassngr:
        case PSt_FreeCtrlPassngr:
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (ustate->thing_under_hand > 0)
                {
                    ustate->influenced_thing_idx = ustate->thing_under_hand;
                    ustate->influenced_thing_creation = thing->creation_turn;
                    set_user_instance(user, PI_PsngrCtrl, 0);
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
            {
                if (ustate->instance_num != PI_PsngrCtrl)
                {
                    set_user_work_state(user, ustate->continue_work_state, 0);
                    unset_packet_control(pckt, PCtr_RBtnRelease);
                }
            }
            break;
        case PSt_CtrlDirect:
        case PSt_FreeCtrlDirect:
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (thing_target_action) {
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                } else if (ustate->thing_under_hand > 0) {
                    magic_use_available_power_on_thing(plyr_idx, user,PwrK_POSSESS, 0, stl_x, stl_y, thing, PwMod_Default);
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
            {
                if (ustate->instance_num != PI_DirctCtrl)
                {
                    set_user_work_state(user, ustate->continue_work_state, 0);
                    unset_packet_control(pckt, PCtr_RBtnRelease);
                }
            }
            break;
        case PSt_CreatrQuery:
        case PSt_CreatrInfo:
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (ustate->thing_under_hand > 0)
                {
                    if (thing->class_id == TCls_Creature)
                    {
                        if (get_user_state(user)->controlled_thing_idx != ustate->thing_under_hand)
                        {
                            query_creature(user, ustate->thing_under_hand, true, false);
                        }
                    }
                    else
                    {
                        query_thing(thing);
                    }
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            if ( ustate->work_state == PSt_CreatrInfo )
            {
                thing = thing_get(get_user_state(user)->controlled_thing_idx);
                if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
                {
                    if (is_local_user(user))
                    {
                        turn_off_query_menus();
                        turn_on_main_panel_menu();
                    }
                    set_user_instance(user, PI_UnqueryCrtr, 0);
                    unset_packet_control(pckt, PCtr_RBtnRelease);
                } else
                if (creature_is_dying(thing) || (thing->creation_turn != ustate->influenced_thing_creation))
                {
                    set_user_instance(user, PI_UnqueryCrtr, 0);
                    if (is_local_user(user))
                    {
                        turn_off_query_menus();
                        turn_on_main_panel_menu();
                    }
                }
            }
            break;
        case PSt_PlaceTrap:
            process_dungeon_control_packet_dungeon_place_trap(user);
            break;
        case PSt_PlaceDoor:
        {
            if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
            {
                ustate->full_slab_cursor = 1;
                // Make the frame around active slab
                i = tag_cursor_blocks_place_door(player->id_number, user, stl_x, stl_y);
                if ((pckt->control_flags & PCtr_LBtnClick) != 0)
                {
                    packet_place_door(stl_x, stl_y, player->id_number, ustate->chosen_door_kind, i);
                }
                unset_packet_control(pckt, PCtr_LBtnClick);
            }
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (ustate->cursor_button_down != 0)
                {
                    ustate->cursor_button_down = 0;
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            break;
        }
        case PST_CastPowerOnTarget:
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (!thing_target_action) {
                    if (thing_is_invalid(thing)) {
                        break;
                    }
                    i = get_power_overcharge_level(ustate);
                    magic_use_available_power_on_thing(plyr_idx, user,pwkind, i, stl_x, stl_y, thing, PwMod_Default);
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_Sell:
            process_dungeon_control_packet_sell_operation(user);
            break;
        default:
            if (!packets_process_cheats(user, plyr_idx, x, y, pckt,
                                        stl_x, stl_y, slb_x, slb_y))
            {
                ERRORLOG("Unrecognized player %d work state: %d", (int) plyr_idx, (int) ustate->work_state);
                ret = false;
            }
            break;
    }
    // resetting position variables - they may have been changed
    x = (pckt->pos_x);
    y = (pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);
    struct PlayerStateConfigStats* plrst_cfg_stat = get_player_state_stats(ustate->work_state);
    if (((pckt->control_flags & PCtr_HeldAnyButton) != 0) && (plrst_cfg_stat->stop_own_units))
    {
        if (((ustate->secondary_cursor_state == CSt_DefaultArrow) || (ustate->secondary_cursor_state == CSt_PowerHand)) && (!ustate->one_click_lock_cursor))
            stop_creatures_around_hand(plyr_idx, stl_x, stl_y);
    }
    return ret;
}
