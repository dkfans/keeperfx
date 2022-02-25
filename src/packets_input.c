
#include "config_players.h"
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
#include "player_states.h"
#include "magic.h"
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

extern TbBool process_dungeon_control_packet_spell_overcharge(long plyr_idx);
extern TbBool packets_process_cheats(
          long plyr_idx,
          MapCoord x, MapCoord y,
          struct Packet *packet,
          MapSubtlCoord stl_x, MapSubtlCoord stl_y,
          MapSlabCoord slb_x, MapSlabCoord slb_y,
          short *influence_own_creatures);

extern void update_double_click_detection(long plyr_idx);

void set_tag_untag_mode(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
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

    if (render_roomspace.untag_mode)
        player->allocflags |= PlaF_ChosenSlabHasActiveTask;
    else
        player->allocflags &= ~PlaF_ChosenSlabHasActiveTask;
}

TbBool process_dungeon_control_packet_dungeon_build_room(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    int mode = box_placement_mode;
    long keycode = 0;
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
    TbBool drag_check = ((is_game_key_pressed(Gkey_BestRoomSpace, &keycode, true) || is_game_key_pressed(Gkey_SquareRoomSpace, &keycode, true)) && ((pckt->control_flags & PCtr_LBtnHeld) == PCtr_LBtnHeld));
    if (game.system_flags & GSF_NetworkActive)
    {
        drag_check = false; // Disable due to lack of network support
    }
    get_dungeon_build_user_roomspace(player->id_number, player->chosen_room_kind, stl_x, stl_y, &mode, drag_check);
    long i = tag_cursor_blocks_place_room(player->id_number, stl_x, stl_y, player->full_slab_cursor);

    if (mode != drag_placement_mode) // allows the user to hold the left mouse to use "paint mode"
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
        if (player->boxsize == 0)
        {
            return false; //stops attempts at invalid rooms, if left mouse button held (i.e. don't repeat failure sound repeatedly in paint mode)
        }
    }
    if (i == 0)
    {
        if (is_my_player(player))
        {
            if (can_build_room_at_slab(player->id_number, player->chosen_room_kind, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)))
            {
                struct Dungeon* dungeon = get_dungeon(player->id_number);
                if (render_roomspace.total_roomspace_cost > dungeon->total_money_owned)
                {
                    output_message(SMsg_GoldNotEnough, 0, true);
                }
            }
            else
            {
                play_non_3d_sample(119);
            }
            unset_packet_control(pckt, PCtr_LBtnClick);
        }
        return false;
    }
    if (player->boxsize > 0)
    {
        keeper_build_roomspace(&render_roomspace);
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
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
    if (!thing_is_invalid(thing) && (!dungeonadd->one_click_lock_cursor))
    {
        SYNCDBG(19,"Thing %d under hand at (%d,%d)",(int)thing->index,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
        if (player->hand_thing_idx == 0)
            create_power_hand(player->id_number);
        player->thing_under_hand = thing->index;
    }
    thing = get_first_thing_in_power_hand(player);
    if (!thing_is_invalid(thing))
    {
        if (player->hand_thing_idx == 0) {
            create_power_hand(player->id_number);
        }
        long i = thing_is_creature_special_digger(thing);
        if ((can_drop_thing_here(stl_x, stl_y, player->id_number, i)
             || !can_dig_here(stl_x, stl_y, player->id_number, true))
            && (!dungeonadd->one_click_lock_cursor))
        {
            render_roomspace = create_box_roomspace(render_roomspace, 1, 1, subtile_slab(stl_x), subtile_slab(stl_y));
            long keycode;
            tag_cursor_blocks_thing_in_hand(player->id_number, stl_x, stl_y, i, (!is_game_key_pressed(Gkey_SellTrapOnSubtile, &keycode, true)));
        } else
        {
            player->additional_flags |= PlaAF_ChosenSubTileIsHigh;
            get_dungeon_highlight_user_roomspace(player->id_number, stl_x, stl_y);
            tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->full_slab_cursor);
        }
    }
    if (player->hand_thing_idx != 0)
    {
        if ((player->instance_num != PI_Grab) && (player->instance_num != PI_Drop) &&
            (player->instance_num != PI_Whip) && (player->instance_num != PI_WhipEnd))
        {
            thing = get_first_thing_in_power_hand(player);
            if ((player->thing_under_hand != 0) || thing_is_invalid(thing))
            {
                set_power_hand_graphic(plyr_idx, 782, 256);
                if (!thing_is_invalid(thing))
                    thing->field_4F |= TF4F_Unknown01;
            } else
            if ((thing->class_id == TCls_Object) && object_is_gold_pile(thing))
            {
                set_power_hand_graphic(plyr_idx, 781, 256);
                thing->field_4F &= ~TF4F_Unknown01;
            } else
            {
                set_power_hand_graphic(plyr_idx, 784, 256);
                thing->field_4F &= ~TF4F_Unknown01;
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0)
        player->secondary_cursor_state = CSt_DefaultArrow;
    player->primary_cursor_state = (unsigned short)(pckt->additional_packet_values & PCAdV_ContextMask) >> 1; // get current cursor state from pckt->additional_packet_values
    render_roomspace.highlight_mode = false; // reset one-click highlight mode

    process_dungeon_power_hand_state(plyr_idx);

    if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
    {
        if (player->primary_cursor_state == CSt_PickAxe)
        {
            get_dungeon_highlight_user_roomspace(player->id_number, stl_x, stl_y);
            tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->full_slab_cursor);
        }
        if ((pckt->control_flags & PCtr_LBtnClick) != 0)
        {
            player->cursor_stl_x = stl_x;
            player->cursor_stl_y = stl_y;
            player->cursor_button_down = 1;
            player->secondary_cursor_state = player->primary_cursor_state;
            switch (player->primary_cursor_state)
            {
                case CSt_PickAxe:
                    set_tag_untag_mode(plyr_idx, stl_x, stl_y);
                    break;
                case CSt_DoorKey:
                    thing = get_door_for_position(player->cursor_stl_x, player->cursor_stl_y);
                    if (thing_is_invalid(thing))
                    {
                        ERRORLOG("Door thing not found at map pos (%d,%d)",(int)player->cursor_stl_x,(int)player->cursor_stl_y);
                        break;
                    }
                    if (thing->trap_door_active_state)
                        unlock_door(thing);
                    else
                        lock_door(thing);
                    break;
                case CSt_PowerHand:
                    if (player->thing_under_hand == 0)
                    {
                        set_tag_untag_mode(plyr_idx, stl_x, stl_y);
                        player->additional_flags |= PlaAF_NoThingUnderPowerHand;
                    }
                    break;
            }
            unset_packet_control(pckt, PCtr_LBtnClick);
        }
        if ((pckt->control_flags & PCtr_RBtnClick) != 0)
        {
            player->cursor_stl_x = stl_x;
            player->cursor_stl_y = stl_y;
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
                set_tag_untag_mode(plyr_idx, stl_x, stl_y);
            }
        }
        if (player->cursor_button_down != 0)
        {
            if (!render_roomspace.drag_mode) // allow drag and click to not place on LMB hold
            {
                if (player->primary_cursor_state == player->secondary_cursor_state)
                {
                    if (player->secondary_cursor_state == CSt_PickAxe)
                    {
                        keeper_highlight_roomspace(plyr_idx, &render_roomspace, 0);
                    } else
                    if ((player->secondary_cursor_state == CSt_PowerHand) && ((player->additional_flags & PlaAF_NoThingUnderPowerHand) != 0))
                    {
                        keeper_highlight_roomspace(plyr_idx, &render_roomspace, 0);
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
        if (dungeonadd->ignore_next_PCtr_LBtnRelease)
        {
            dungeonadd->ignore_next_PCtr_LBtnRelease = false;
            if ((pckt->control_flags & PCtr_RBtnHeld) == 0)
            {
                player->cursor_button_down = 0;
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        } else
        if (player->cursor_button_down != 0)
        {
            thing = thing_get(player->thing_under_hand);
            if ((player->thing_under_hand != 0) && (player->input_crtr_control != 0)
                && (dungeon->things_in_hand[0] != player->thing_under_hand))
            {
                set_player_state(player, PSt_CtrlDirect, 0);
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
                        set_player_state(player, PSt_CreatrQuery, 0);
                        set_player_instance(player, PI_QueryCrtr, 0);
                    }
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            } else
            if (player->secondary_cursor_state == player->primary_cursor_state)
            {
                if ((player->primary_cursor_state == CSt_PickAxe) || ((player->primary_cursor_state == CSt_PowerHand) && render_roomspace.drag_mode))
                {
                    keeper_highlight_roomspace(plyr_idx, &render_roomspace, 9);
                } else
                if (player->primary_cursor_state == CSt_PowerHand)
                {
                    if (player->thing_under_hand != 0) {
                        // TODO SPELLS it's not a good idea to use this directly; change to magic_use_available_power_on_*()
                        magic_use_power_hand(plyr_idx, stl_x, stl_y, 0);
                    }
                }
            }
            if ((pckt->control_flags & PCtr_RBtnHeld) == 0)
            {
                player->cursor_button_down = 0;
                dungeonadd->one_click_lock_cursor = false;
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
            if (render_roomspace.drag_mode)
            {
                if ((pckt->control_flags & PCtr_RBtnHeld) == 0)
                {
                    render_roomspace.drag_mode = false;
                }
                else
                {
                    render_roomspace.untag_mode = !render_roomspace.untag_mode;
                    set_tag_untag_mode(plyr_idx, stl_x, stl_y);
                }
            }
            player->secondary_cursor_state = CSt_DefaultArrow;
            player->additional_flags &= ~PlaAF_NoThingUnderPowerHand;
        }
    }

    if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
    {
        if (dungeonadd->ignore_next_PCtr_RBtnRelease && (!dungeonadd->one_click_lock_cursor))
        {
            dungeonadd->ignore_next_PCtr_RBtnRelease = false;
            if ((pckt->control_flags & PCtr_LBtnHeld) == 0)
            {
                player->cursor_button_down = 0;
            }
            unset_packet_control(pckt, PCtr_RBtnRelease);
        } else
        if (player->cursor_button_down != 0)
        {
            if (!power_hand_is_empty(player) && (!dungeonadd->one_click_lock_cursor))
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
                if (player->primary_cursor_state == CSt_PowerHand && (!dungeonadd->one_click_lock_cursor)) {
                    thing = get_nearest_thing_for_slap(plyr_idx, subtile_coord_center(stl_x), subtile_coord_center(stl_y));
                    magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, 0, stl_x, stl_y, thing, PwMod_Default);
                }
                if ((pckt->control_flags & PCtr_LBtnHeld) == 0)
                {
                    player->cursor_button_down = 0;
                    dungeonadd->one_click_lock_cursor = false;
                }
                unset_packet_control(pckt, PCtr_RBtnRelease);
            }
        }
    }
    if ((player->cursor_button_down == 0) || (!dungeonadd->one_click_lock_cursor))
    {
        //if (untag_or_tag_completed_or_cancelled)
        dungeonadd->swap_to_untag_mode = 0; // no
        if ((player->cursor_button_down == 0) && ((pckt->control_flags & PCtr_LBtnHeld) == 0))
        {
            dungeonadd->one_click_lock_cursor = false;
        }
    }
    return true;
}

TbBool process_dungeon_control_packet_sell_operation(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    long keycode = 0;
    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->full_slab_cursor != 0))
        {
            player->full_slab_cursor = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    player->full_slab_cursor = (!is_game_key_pressed(Gkey_SellTrapOnSubtile, &keycode, true));
    if (is_my_player(player))
    {
        if (!game_is_busy_doing_gui())
        {
            get_dungeon_sell_user_roomspace(player->id_number, stl_x, stl_y);
            tag_cursor_blocks_sell_area(player->id_number, stl_x, stl_y, player->full_slab_cursor);
        }
    }
    if ((pckt->control_flags & PCtr_LBtnClick) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->full_slab_cursor != 0))
        {
            player->full_slab_cursor = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    if (player->full_slab_cursor)
    {
        //Slab Mode
        if (render_roomspace.slab_count > 0)
        {
            keeper_sell_roomspace(&render_roomspace);
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
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
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
    player->full_slab_cursor = ((player->chosen_trap_kind == TngTrp_Boulder) || (!gameadd.place_traps_on_subtiles));
    long i = tag_cursor_blocks_place_trap(player->id_number, stl_x, stl_y, player->full_slab_cursor);
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
    struct DungeonAdd *dungeonadd = get_dungeonadd(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6,"Starting for player %d state %s",(int)plyr_idx,player_state_code_name(player->work_state));
    player->full_slab_cursor = 1;
    packet_left_button_double_clicked[plyr_idx] = 0;
    if ((pckt->control_flags & PCtr_Unknown4000) != 0)
        return false;
    TbBool ret = true;

    process_dungeon_control_packet_spell_overcharge(plyr_idx);
    if ((pckt->control_flags & PCtr_RBtnHeld) != 0)
    {
        player->field_4D6++;
    } else
    if ((pckt->control_flags & PCtr_RBtnRelease) == 0)
    {
        player->boxsize = 1;
        player->field_4D6 = 0;
    }
    update_double_click_detection(plyr_idx);
    player->thing_under_hand = 0;
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    short influence_own_creatures = false;
    struct SlabMap *slb;
    long i;
    struct Room* room = NULL;
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    switch (player->work_state)
    {
        case PSt_CtrlDungeon:
            influence_own_creatures = 1;
            process_dungeon_control_packet_dungeon_control(plyr_idx);
            break;
        case PSt_BuildRoom:
            process_dungeon_control_packet_dungeon_build_room(plyr_idx);
            break;
        case PSt_CallToArms:
        case PSt_CaveIn:
        case PSt_SightOfEvil:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                pwkind = player_state_to_power_kind[player->work_state];
                i = get_power_overcharge_level(player);
                magic_use_available_power_on_subtile(plyr_idx, pwkind, i, stl_x, stl_y, PwCast_None);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_Slap:
            influence_own_creatures = 1;
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
            influence_own_creatures = 1;
            if (player->work_state == PSt_CtrlPassngr)
                thing = get_creature_near_and_owned_by(x, y, plyr_idx, -1);
            else
                thing = get_creature_near_and_owned_by(x, y, -1, -1);
            if (thing_is_invalid(thing))
                player->thing_under_hand = 0;
            else
                player->thing_under_hand = thing->index;
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (player->thing_under_hand > 0)
                {
                    player->influenced_thing_idx = player->thing_under_hand;
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
            influence_own_creatures = 1;
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
            influence_own_creatures = 1;
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
                            if (is_my_player(player))
                            {
                                turn_off_all_panel_menus();
                                initialise_tab_tags_and_menu(GMnu_CREATURE_QUERY1);
                                turn_on_menu(GMnu_CREATURE_QUERY1);
                            }
                            player->influenced_thing_idx = player->thing_under_hand;
                            set_player_instance(player, PI_QueryCrtr, 0);
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
                if (creature_is_dying(thing))
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
        case PSt_Lightning:
            player->thing_under_hand = 0;
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                i = get_power_overcharge_level(player);
                magic_use_available_power_on_subtile(plyr_idx, PwrK_LIGHTNING, i, stl_x, stl_y, PwCast_None);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_PlaceDoor:
        {
            long k;
            if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
            {
                player->full_slab_cursor = 1;
                // Make the frame around active slab
                i = tag_cursor_blocks_place_door(player->id_number, stl_x, stl_y);
                if ((pckt->control_flags & PCtr_LBtnClick) != 0)
                {
                    k = get_slab_number(slb_x, slb_y);
                    delete_room_slabbed_objects(k);
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
        case PSt_SpeedUp:
        case PSt_Armour:
        case PSt_Conceal:
        case PSt_Heal:
            influence_own_creatures = true;
            pwkind = player_state_to_power_kind[player->work_state];
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
        case PSt_CreateDigger:
        case PSt_DestroyWalls:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                pwkind = player_state_to_power_kind[player->work_state];
                i = get_power_overcharge_level(player);
                magic_use_available_power_on_subtile(plyr_idx, pwkind, i, stl_x, stl_y, PwCast_None);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_CastDisease:
            pwkind = player_state_to_power_kind[player->work_state];
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
        case PSt_TurnChicken:
            influence_own_creatures = true;
            pwkind = player_state_to_power_kind[player->work_state];
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
        default:
            if (!packets_process_cheats(plyr_idx, x, y, pckt,
                                        stl_x, stl_y, slb_x, slb_y, &influence_own_creatures))
            {
                ERRORLOG("Unrecognized player %d work state: %d", (int) plyr_idx, (int) player->work_state);
                ret = false;
            }
            break;
    }
    // resetting position variables - they may have been changed
    x = ((unsigned short)pckt->pos_x);
    y = ((unsigned short)pckt->pos_y);
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
    if (((pckt->control_flags & PCtr_HeldAnyButton) != 0) && (influence_own_creatures))
    {
        if (((player->secondary_cursor_state == CSt_DefaultArrow) || (player->secondary_cursor_state == CSt_PowerHand)) && (!dungeonadd->one_click_lock_cursor))
            stop_creatures_around_hand(plyr_idx, stl_x, stl_y);
    }
    return ret;
}
