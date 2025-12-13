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
#include "post_inc.h"

extern TbBool process_dungeon_control_packet_spell_overcharge(long plyr_idx);

extern void update_double_click_detection(long plyr_idx);

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

void remember_cursor_subtile(struct PlayerInfo *player) {
    struct Packet* pckt = get_packet_direct(player->packet_num);
    if (player->interpolated_tagging == true) {
        player->previous_cursor_subtile_x = player->cursor_subtile_x;
        player->previous_cursor_subtile_y = player->cursor_subtile_y;
        player->cursor_subtile_x = coord_subtile((pckt->pos_x));
        player->cursor_subtile_y = coord_subtile((pckt->pos_y));
    } else {
        player->cursor_subtile_x = coord_subtile((pckt->pos_x));
        player->cursor_subtile_y = coord_subtile((pckt->pos_y));
        player->previous_cursor_subtile_x = player->cursor_subtile_x;
        player->previous_cursor_subtile_y = player->cursor_subtile_y;
    }
    player->interpolated_tagging = player->mouse_on_map;
}

void set_tag_untag_mode(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    // The commented out section is the old way, this check is now performed as part of keeper_highlight_roomspace() in roomspace.cabs
    // which sets render_roomspace.untag_mode
    /*long i;
    i = get_subtile_number(stl_slab_center_subtile(stl_x),stl_slab_center_subtile(stl_y));
    if (find_from_task_list(plyr_idx,i) != -1)
        player->allocflags |= PlaF_ChosenSlabHasActiveTask;
    else
        player->allocflags &= ~PlaF_ChosenSlabHasActiveTask;*/

    if (player->render_roomspace.untag_mode)
        player->allocflags |= PlaF_ChosenSlabHasActiveTask;
    else
        player->allocflags &= ~PlaF_ChosenSlabHasActiveTask;
}

TbBool process_dungeon_control_packet_dungeon_build_room(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = (pckt->pos_x);
    MapCoord y = (pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->cursor_button_down != 0))
        {
            player->cursor_button_down = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    player->full_slab_cursor = 1; // always use full slabs, not single subtiles
    if (is_my_player(player))
    {
        gui_room_type_highlighted = player->chosen_room_kind;
    }
    get_dungeon_build_user_roomspace(&player->render_roomspace, player->id_number, player->chosen_room_kind, stl_x, stl_y, player->roomspace_mode);
    long i = tag_cursor_blocks_place_room(player->id_number, stl_x, stl_y, player->full_slab_cursor);
    if ( (player->roomspace_mode == drag_placement_mode) && (player->roomspace_drag_paint_mode == false) )
    {
       if ((pckt->control_flags & PCtr_LBtnRelease) != PCtr_LBtnRelease)
       {
           return false;
       }
    }
    if (player->roomspace_mode != drag_placement_mode) // allows the user to hold the left mouse to use "paint mode"
    {
        if ((pckt->control_flags & PCtr_LBtnClick) == 0)
        {
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->cursor_button_down != 0))
            {
                player->cursor_button_down = 0;
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            return false;
        }
    }
    else if ((pckt->control_flags & PCtr_LBtnHeld) == PCtr_LBtnHeld)
    {
        if ( (player->boxsize == 0) || (!can_build_room_at_slab(player->id_number, player->chosen_room_kind, subtile_slab(stl_x), subtile_slab(stl_y))) )
        {
            return false; //stops attempts at invalid rooms, if left mouse button held (i.e. don't repeat failure sound repeatedly in paint mode)
        }
    }
    if (i == 0)
    {
        if (can_build_room_at_slab(player->id_number, player->chosen_room_kind, subtile_slab(stl_x), subtile_slab(stl_y)))
        {
            struct Dungeon* dungeon = get_dungeon(player->id_number);
            if (player->render_roomspace.total_roomspace_cost > dungeon->total_money_owned)
            {
                if (is_my_player(player))
                {
                    output_message(SMsg_GoldNotEnough, 0);
                }
            }
        }
        else
        {
            if (is_my_player(player))
            {
                play_non_3d_sample(119);
            }
        }
        unset_packet_control(pckt, PCtr_LBtnClick);
      return false;
    }
    if (player->boxsize > 0)
    {
        keeper_build_roomspace(plyr_idx, &player->render_roomspace);
    }
    else
    {
        if (is_my_player(player))
        {
            play_non_3d_sample(119);
        }
    }
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

TbBool process_dungeon_power_hand_state(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = pckt->pos_x;
    MapCoord y = pckt->pos_y;
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);

    player->additional_flags &= ~PlaAF_ChosenSubTileIsHigh;
    if ((player->secondary_cursor_state != CSt_DefaultArrow) && (player->secondary_cursor_state != CSt_PowerHand))
    {
        if (player->instance_num != PI_Grab) {
            delete_power_hand(player->id_number);
        }
        return false;
    }
    struct Thing* thing = get_nearest_thing_for_hand_or_slap(plyr_idx, x, y);
    if (!thing_is_invalid(thing) && (!player->one_click_lock_cursor))
    {
        SYNCDBG(19,"Thing %d under hand at (%d,%d)",(int)thing->index,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
        if (player->hand_thing_idx == 0)
            create_power_hand(player->id_number);
        player->thing_under_hand = thing->index;
    }
    thing = get_first_thing_in_power_hand(player);
    if (thing_exists(thing))
    {
        if (player->hand_thing_idx == 0) {
            create_power_hand(player->id_number);
        }
        long is_digger = thing_is_creature_digger(thing);
        if ((can_drop_thing_here(stl_x, stl_y, player->id_number, is_digger)
             || !can_dig_here(stl_x, stl_y, player->id_number, true))
            && (!player->one_click_lock_cursor))
        {
            player->render_roomspace = create_box_roomspace(player->render_roomspace, 1, 1, subtile_slab(stl_x), subtile_slab(stl_y));
            player->full_slab_cursor = (player->roomspace_mode != single_subtile_mode);
            tag_cursor_blocks_thing_in_hand(plyr_idx, stl_x, stl_y, is_digger, player->full_slab_cursor);
        } else
        {
            player->additional_flags |= PlaAF_ChosenSubTileIsHigh;
            get_dungeon_highlight_user_roomspace(&player->render_roomspace, player->id_number, stl_x, stl_y);
            tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->full_slab_cursor);
            player->thing_under_hand = 0;
        }
    }
    if (player->hand_thing_idx != 0)
    {
        if ((player->instance_num != PI_Grab) && (player->instance_num != PI_Drop) &&
            (player->instance_num != PI_Whip) && (player->instance_num != PI_WhipEnd))
        {
            thing = get_first_thing_in_power_hand(player);
            if ((player->thing_under_hand != 0) || !thing_exists(thing))
            {
                set_power_hand_graphic(plyr_idx, HndA_Hover);
                if (thing_exists(thing))
                    thing->rendering_flags |= TRF_Invisible;
            } else
            if ((thing->class_id == TCls_Object) && object_is_gold_pile(thing))
            {
                set_power_hand_graphic(plyr_idx, HndA_HoldGold);
                thing->rendering_flags &= ~TRF_Invisible;
            } else
            {
                set_power_hand_graphic(plyr_idx, HndA_Hold);
                thing->rendering_flags &= ~TRF_Invisible;
            }
        }
    }
    return true;
}

TbBool process_dungeon_control_packet_dungeon_control(long plyr_idx)
{
    struct Thing *thing;
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = pckt->pos_x;
    MapCoord y = pckt->pos_y;
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    TbBool at_limit = false;
    unsigned char box_colour;
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0)
        player->secondary_cursor_state = CSt_DefaultArrow;
    player->primary_cursor_state = (unsigned short)(pckt->additional_packet_values & PCAdV_ContextMask) >> 1; // get current cursor state from pckt->additional_packet_values
    player->render_roomspace.highlight_mode = settings.highlight_mode; // reset one-click highlight mode
    player->render_roomspace.drag_mode = player->one_click_lock_cursor;
    player->pickup_all_gold = (pckt->additional_packet_values & PCAdV_RotatePressed);
    process_dungeon_power_hand_state(plyr_idx);
    if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
    {
        if ( (player->primary_cursor_state == CSt_PickAxe) || ( (player->primary_cursor_state == CSt_PowerHand) && ((player->additional_flags & PlaAF_ChosenSubTileIsHigh) != 0) ) )
        {
            player->thing_under_hand = 0;
            get_dungeon_highlight_user_roomspace(&player->render_roomspace, player->id_number, stl_x, stl_y);
            box_colour = tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->full_slab_cursor);
            at_limit = (box_colour == SLC_REDYELLOW) || (box_colour == SLC_REDFLASH);
        }
        if ((pckt->control_flags & PCtr_LBtnClick) != 0)
        {
            player->cursor_clicked_subtile_x = stl_x;
            player->cursor_clicked_subtile_y = stl_y;
            player->cursor_button_down = 1;
            player->secondary_cursor_state = player->primary_cursor_state;
            switch (player->primary_cursor_state)
            {
                case CSt_PickAxe:
                    set_tag_untag_mode(plyr_idx);
                    if (!player->render_roomspace.drag_mode)
                    {
                        if (at_limit)
                        {
                            if (is_my_player(player))
                            {
                                play_non_3d_sample(119);
                                output_message(SMsg_WorkerJobsLimit, 500); // remind the user that the task limit (MAPTASKS_COUNT) has been reached
                            }
                        }
                    }
                    break;
                case CSt_DoorKey:
                    thing = get_door_for_position(player->cursor_clicked_subtile_x, player->cursor_clicked_subtile_y);
                    if (thing_is_invalid(thing))
                    {
                        ERRORLOG("Door thing not found at map pos (%d,%d)",(int)player->cursor_clicked_subtile_x,(int)player->cursor_clicked_subtile_y);
                        break;
                    }
                    if (thing->door.is_locked)
                        unlock_door(thing);
                    else
                        lock_door(thing);
                    break;
                case CSt_PowerHand:
                    if (player->thing_under_hand == 0)
                    {
                        set_tag_untag_mode(plyr_idx);
                        if (!player->render_roomspace.drag_mode)
                        {
                            if (at_limit)
                            {
                                if (is_my_player(player))
                                {
                                    play_non_3d_sample(119);
                                    output_message(SMsg_WorkerJobsLimit, 500); // remind the user that the task limit (MAPTASKS_COUNT) has been reached
                                }
                            }
                        }
                        player->additional_flags |= PlaAF_NoThingUnderPowerHand;
                    }
                    break;
            }
            unset_packet_control(pckt, PCtr_LBtnClick);
        }
        if ((pckt->control_flags & PCtr_RBtnClick) != 0)
        {
            player->cursor_clicked_subtile_x = stl_x;
            player->cursor_clicked_subtile_y = stl_y;
            player->cursor_button_down = 1;
            unset_packet_control(pckt, PCtr_RBtnClick);
        }
    }

    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
        if (player->secondary_cursor_state == CSt_DefaultArrow)
        {
            player->secondary_cursor_state = player->primary_cursor_state;
            if (player->primary_cursor_state == CSt_PickAxe)
            {
                set_tag_untag_mode(plyr_idx);
            }
        }
        if (player->cursor_button_down != 0)
        {
            if (!player->render_roomspace.drag_mode) // allow drag and click to not place on LMB hold
            {
                if (player->primary_cursor_state == player->secondary_cursor_state)
                {
                    if ( (player->secondary_cursor_state == CSt_PickAxe) || ((player->secondary_cursor_state == CSt_PowerHand) && ((player->additional_flags & PlaAF_NoThingUnderPowerHand) != 0)) )
                    {
                        keeper_highlight_roomspace(plyr_idx, &player->render_roomspace);
                    }
                }
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
    }
    if ((pckt->control_flags & PCtr_RBtnHeld) != 0)
    {
        if (player->cursor_button_down != 0)
            unset_packet_control(pckt, PCtr_RBtnRelease);
    }
    if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
    {
        if (player->secondary_cursor_state == CSt_DefaultArrow)
            player->secondary_cursor_state = player->primary_cursor_state;
        if (player->ignore_next_PCtr_LBtnRelease)
        {
            player->ignore_next_PCtr_LBtnRelease = false;
            if ((pckt->control_flags & PCtr_RBtnHeld) == 0)
            {
                player->cursor_button_down = 0;
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        } else
        if (player->cursor_button_down != 0)
        {
            if ((player->thing_under_hand != 0) && (player->input_crtr_control != 0)
                && (dungeon->things_in_hand[0] != player->thing_under_hand))
            {
                thing = get_creature_near_for_controlling(player->id_number, x, y);
                if (!thing_is_invalid(thing))
                {
                    player->thing_under_hand = thing->index;
                }
                else
                {
                    thing = thing_get(player->thing_under_hand);
                }
                set_player_state(player, PSt_CtrlDirect, PwrK_POSSESS);
                if (magic_use_available_power_on_thing(plyr_idx, PwrK_POSSESS, 0, stl_x, stl_y, thing, PwMod_Default) == Lb_FAIL) {
                    set_player_state(player, player->continue_work_state, 0);
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            } else
            if (player->input_crtr_query != 0)
            {
                thing = get_creature_near(x, y);
                if (!can_thing_be_queried(thing, plyr_idx))
                {
                    player->thing_under_hand = 0;
                }
                else
                {
                    player->thing_under_hand = thing->index;
                }
                if (player->thing_under_hand > 0)
                {
                    if (player->thing_under_hand != player->controlled_thing_idx)
                    {
                        if (is_my_player(player))
                        {
                            turn_off_all_panel_menus();
                            turn_on_menu(GMnu_CREATURE_QUERY1);
                        }
                        player->influenced_thing_idx = player->thing_under_hand;
                        player->influenced_thing_creation = thing->creation_turn;
                        set_player_state(player, PSt_CreatrQuery, 0);
                        set_player_instance(player, PI_QueryCrtr, 0);
                    }
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            } else
            if (player->secondary_cursor_state == player->primary_cursor_state)
            {
                if ( (player->primary_cursor_state == CSt_PickAxe) || (player->primary_cursor_state == CSt_PowerHand) )
                {
                    if (player->thing_under_hand != 0) 
                    {
                        // TODO SPELLS it's not a good idea to use this directly; change to magic_use_available_power_on_*()
                        use_power_hand(plyr_idx, stl_x, stl_y, 0);
                    }
                    else if (player->render_roomspace.drag_mode)
                    {
                        if (at_limit)
                        {
                            if (is_my_player(player))
                            {
                                play_non_3d_sample(119);
                                output_message(SMsg_WorkerJobsLimit, 500); // remind the user that the task limit (MAPTASKS_COUNT) has been reached
                            }
                        }
                        else
                        {
                            keeper_highlight_roomspace(plyr_idx, &player->render_roomspace);
                        }
                    }
                }
            }
            if ((pckt->control_flags & PCtr_RBtnHeld) == 0)
            {
                player->cursor_button_down = 0;
                player->one_click_lock_cursor = false;
            }
            if (player->render_roomspace.drag_mode)
            {
                if ((pckt->control_flags & PCtr_RBtnHeld) == 0)
                {
                    player->render_roomspace.drag_mode = false;
                }
                else
                {
                    player->render_roomspace.untag_mode = !player->render_roomspace.untag_mode;
                    set_tag_untag_mode(plyr_idx);
                }
            }
            player->secondary_cursor_state = CSt_DefaultArrow;
            player->additional_flags &= ~PlaAF_NoThingUnderPowerHand;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
    }

    if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
    {
        if (player->ignore_next_PCtr_RBtnRelease && (!player->one_click_lock_cursor))
        {
            player->ignore_next_PCtr_RBtnRelease = false;
            if ((pckt->control_flags & PCtr_LBtnHeld) == 0)
            {
                player->cursor_button_down = 0;
            }
            unset_packet_control(pckt, PCtr_RBtnRelease);
        } else
        if (player->cursor_button_down != 0)
        {
            if (!power_hand_is_empty(player) && (!player->one_click_lock_cursor))
            {
                if (dump_first_held_thing_on_map(player->id_number, stl_x, stl_y, 1)) {
                    if ((pckt->control_flags & PCtr_LBtnHeld) == 0)
                    {
                        player->cursor_button_down = 0;
                    }
                    unset_packet_control(pckt, PCtr_RBtnRelease);
                }
            } else
            {
                if (player->primary_cursor_state == CSt_PowerHand && (!player->one_click_lock_cursor)) {
                    thing = get_nearest_thing_for_slap(plyr_idx, subtile_coord_center(stl_x), subtile_coord_center(stl_y));
                    magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, 0, stl_x, stl_y, thing, PwMod_Default);
                }
                if ((pckt->control_flags & PCtr_LBtnHeld) == 0)
                {
                    player->cursor_button_down = 0;
                    player->one_click_lock_cursor = false;
                }
                unset_packet_control(pckt, PCtr_RBtnRelease);
            }
        }
    }
    if ((player->cursor_button_down == 0) || (!player->one_click_lock_cursor))
    {
        //if (untag_or_tag_completed_or_cancelled)
        player->swap_to_untag_mode = 0; // no
        if ((player->cursor_button_down == 0) && ((pckt->control_flags & PCtr_LBtnHeld) == 0))
        {
            player->one_click_lock_cursor = false;
        }
    }
    return true;
}

TbBool process_dungeon_control_packet_sell_operation(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->full_slab_cursor != 0))
        {
            player->full_slab_cursor = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    MapCoord x = (pckt->pos_x);
    MapCoord y = (pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    player->full_slab_cursor = (player->roomspace_mode != single_subtile_mode);
    get_dungeon_sell_user_roomspace(&player->render_roomspace, player->id_number, stl_x, stl_y);
    tag_cursor_blocks_sell_area(plyr_idx, stl_x, stl_y, player->full_slab_cursor);
    if (player->roomspace_mode == drag_placement_mode)
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
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->full_slab_cursor != 0))
            {
                player->full_slab_cursor = 0;
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            return false;
        }
    }
    if (player->full_slab_cursor)
    {
        //Slab Mode
        if (player->render_roomspace.slab_count > 0)
        {
            keeper_sell_roomspace(plyr_idx, &player->render_roomspace);
        }
        else
        {
            struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
            if (slabmap_owner(slb) != plyr_idx)
            {
                WARNLOG("Player %d can't sell item on %s owned by player %d at subtile (%d,%d).", (int)plyr_idx, slab_code_name(slb->kind), (int)slabmap_owner(slb), (int)stl_x, (int)stl_y);
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
            if (player_sell_trap_at_subtile(plyr_idx, stl_x, stl_y))
            {
                // Nothing to do here - trap already sold
            } else
            {
                WARNLOG("Nothing to do for player %d request",(int)plyr_idx);
            }
        }
    }
    else
    {
        struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
        if (slabmap_owner(slb) != plyr_idx)
        {
            WARNLOG("Player %d can't sell item on %s owned by player %d at subtile (%d,%d).", (int)plyr_idx, slab_code_name(slb->kind), (int)slabmap_owner(slb), (int)stl_x, (int)stl_y);
            unset_packet_control(pckt, PCtr_LBtnClick);
            return false;
        }
        // Subtile Mode
        if (player_sell_trap_at_subtile(plyr_idx, stl_x, stl_y))
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
            WARNLOG("Nothing to do for player %d request",(int)plyr_idx);
        }
    }
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

TbBool process_dungeon_control_packet_dungeon_place_trap(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = (pckt->pos_x);
    MapCoord y = (pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);

    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->cursor_button_down != 0))
        {
            player->cursor_button_down = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    struct TrapConfigStats *trapst = get_trap_model_stats(player->chosen_trap_kind);
    player->full_slab_cursor = (trapst->place_on_subtile == false);
    long i = tag_cursor_blocks_place_trap(player->id_number, stl_x, stl_y, player->full_slab_cursor, player->chosen_trap_kind);
    if ((pckt->control_flags & PCtr_LBtnClick) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->cursor_button_down != 0))
        {
            player->cursor_button_down = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    if (i == 0)
    {
        if (is_my_player(player))
            play_non_3d_sample(119);
        unset_packet_control(pckt, PCtr_LBtnClick);
        return false;
    }
    if (!player_place_trap_at(stl_x, stl_y, plyr_idx, player->chosen_trap_kind))
    {
        unset_packet_control(pckt, PCtr_LBtnClick);
        return false;
    }
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

TbBool process_dungeon_control_packet_clicks(long plyr_idx)
{
    struct Thing *thing;
    PowerKind pwkind;
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6,"Starting for player %d state %s",(int)plyr_idx,player_state_code_name(player->work_state));
    player->full_slab_cursor = 1;
    packet_left_button_double_clicked[plyr_idx] = 0;
    player->mouse_on_map = is_mouse_on_map(pckt);
    remember_cursor_subtile(player);
    process_dungeon_control_packet_spell_overcharge(plyr_idx);
    if (flag_is_set(pckt->control_flags,PCtr_Gui))
        return false;
    TbBool ret = true;
    if ((pckt->control_flags & PCtr_RBtnHeld) != 0)
    {
    } else
    if ((pckt->control_flags & PCtr_RBtnRelease) == 0)
    {
        player->boxsize = 1;
    }
    if (player->id_number == my_player_number)
    {
        map_volume_box.visible = 0;
    }

    update_double_click_detection(plyr_idx);
    player->thing_under_hand = 0;
    MapCoord x = (pckt->pos_x);
    MapCoord y = (pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);

    long i;
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    pwkind = player->chosen_power_kind;

    switch (player->work_state)
    {
        case PSt_CtrlDungeon:
            process_dungeon_control_packet_dungeon_control(plyr_idx);
            break;
        case PSt_BuildRoom:
            process_dungeon_control_packet_dungeon_build_room(plyr_idx);
            break;
        case PSt_CallToArms:
        case PSt_SightOfEvil:
        case PSt_CreateDigger:
        case PSt_CastPowerOnSubtile:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                i = get_power_overcharge_level(player);
                magic_use_available_power_on_subtile(plyr_idx, pwkind, i, stl_x, stl_y, PwCast_None, PwMod_Default);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_Slap:
            thing = get_creature_near_to_be_keeper_power_target(x, y, PwrK_SLAP, plyr_idx);
            if (thing_is_invalid(thing)) {
                player->thing_under_hand = 0;
            } else {
                player->thing_under_hand = thing->index;
            }
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, 0, stl_x, stl_y, thing, PwMod_Default);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_CtrlPassngr:
        case PSt_FreeCtrlPassngr:
            if (player->work_state == PSt_CtrlPassngr)
                thing = get_creature_near_and_owned_by(x, y, plyr_idx, CREATURE_ANY);
            else
                thing = get_creature_near_and_owned_by(x, y, -1, CREATURE_ANY);
            if (thing_is_invalid(thing))
                player->thing_under_hand = 0;
            else
                player->thing_under_hand = thing->index;
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (player->thing_under_hand > 0)
                {
                    player->influenced_thing_idx = player->thing_under_hand;
                    player->influenced_thing_creation = thing->creation_turn;
                    set_player_instance(player, PI_PsngrCtrl, 0);
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
            {
                if (player->instance_num != PI_PsngrCtrl)
                {
                    set_player_state(player, player->continue_work_state, 0);
                    unset_packet_control(pckt, PCtr_RBtnRelease);
                }
            }
            break;
        case PSt_CtrlDirect:
        case PSt_FreeCtrlDirect:
            if (player->work_state == PSt_CtrlDirect)
                thing = get_creature_near_for_controlling(plyr_idx, x, y);
            else
                thing = get_creature_near(x, y);
            if (thing_is_invalid(thing))
                player->thing_under_hand = 0;
            else
                player->thing_under_hand = thing->index;
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (player->thing_under_hand > 0)
                {
                    magic_use_available_power_on_thing(plyr_idx, PwrK_POSSESS, 0, stl_x, stl_y, thing, PwMod_Default);
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
            {
                if (player->instance_num != PI_DirctCtrl)
                {
                    set_player_state(player, player->continue_work_state, 0);
                    unset_packet_control(pckt, PCtr_RBtnRelease);
                }
            }
            break;
        case PSt_CreatrQuery:
        case PSt_CreatrInfo:
            thing = get_creature_near(x, y);
            TbBool CanQuery = false;
            if (thing_is_creature(thing))
            {
                CanQuery = can_thing_be_queried(thing, plyr_idx);
            }
            if (!CanQuery)
            {
                player->thing_under_hand = 0;
            }
            else
            {
                player->thing_under_hand = thing->index;
            }
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (player->thing_under_hand > 0)
                {
                    if (thing->class_id == TCls_Creature)
                    {
                        if (player->controlled_thing_idx != player->thing_under_hand)
                        {
                            query_creature(player, player->thing_under_hand, true, false);
                        }
                    }
                    else
                    {
                        query_thing(thing);
                    }
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            if ( player->work_state == PSt_CreatrInfo )
            {
                thing = thing_get(player->controlled_thing_idx);
                if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
                {
                    if (is_my_player(player))
                    {
                        turn_off_query_menus();
                        turn_on_main_panel_menu();
                    }
                    set_player_instance(player, PI_UnqueryCrtr, 0);
                    unset_packet_control(pckt, PCtr_RBtnRelease);
                } else
                if (creature_is_dying(thing) || (thing->creation_turn != player->influenced_thing_creation))
                {
                    set_player_instance(player, PI_UnqueryCrtr, 0);
                    if (is_my_player(player))
                    {
                        turn_off_query_menus();
                        turn_on_main_panel_menu();
                    }
                }
            }
            break;
        case PSt_PlaceTrap:
            process_dungeon_control_packet_dungeon_place_trap(plyr_idx);
            break;
        case PSt_PlaceDoor:
        {
            if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
            {
                player->full_slab_cursor = 1;
                // Make the frame around active slab
                i = tag_cursor_blocks_place_door(player->id_number, stl_x, stl_y);
                if ((pckt->control_flags & PCtr_LBtnClick) != 0)
                {
                    packet_place_door(stl_x, stl_y, player->id_number, player->chosen_door_kind, i);
                }
                unset_packet_control(pckt, PCtr_LBtnClick);
            }
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (player->cursor_button_down != 0)
                {
                    player->cursor_button_down = 0;
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            break;
        }
        case PST_CastPowerOnTarget:
            thing = get_creature_near_to_be_keeper_power_target(x, y, pwkind, plyr_idx);
            if (thing_is_invalid(thing))
            {
                player->thing_under_hand = 0;
                break;
            }
            player->thing_under_hand = thing->index;
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                i = get_power_overcharge_level(player);
                magic_use_available_power_on_thing(plyr_idx, pwkind, i, stl_x, stl_y, thing, PwMod_Default);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_Sell:
            process_dungeon_control_packet_sell_operation(plyr_idx);
            break;
        default:
            if (!packets_process_cheats(plyr_idx, x, y, pckt,
                                        stl_x, stl_y, slb_x, slb_y))
            {
                ERRORLOG("Unrecognized player %d work state: %d", (int) plyr_idx, (int) player->work_state);
                ret = false;
            }
            break;
    }
    // resetting position variables - they may have been changed
    x = (pckt->pos_x);
    y = (pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);
    if (player->thing_under_hand == 0)
    {
        if ((x != 0) && (y != 0))  //originally was (y == 0), but it was probably a mistake
        {
            thing = get_queryable_object_near(x, y, plyr_idx);
            if (!thing_is_invalid(thing)) {
                player->thing_under_hand = thing->index;
            }
        }
    }
    struct PlayerStateConfigStats* plrst_cfg_stat = get_player_state_stats(player->work_state);
    if (((pckt->control_flags & PCtr_HeldAnyButton) != 0) && (plrst_cfg_stat->stop_own_units))
    {
        if (((player->secondary_cursor_state == CSt_DefaultArrow) || (player->secondary_cursor_state == CSt_PowerHand)) && (!player->one_click_lock_cursor))
            stop_creatures_around_hand(plyr_idx, stl_x, stl_y);
    }
    return ret;
}
