/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets_input.c
 *     Packet processing routines.
 * @par Purpose:
 *     Here we should map frontend actions and create outgoing packets
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     20 Sep 2020 - 23 Nov 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "packets.h"

#include "bflib_sound.h"
#include "config_players.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "front_input.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "game_legacy.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "hist_actions.h"
#include "keeperfx.hpp"
#include "kjm_input.h"
#include "magic.h"
#include "map_blocks.h"
#include "player_instances.h"
#include "player_states.h"
#include "player_utils.h"
#include "power_hand.h"

extern void update_double_click_detection(int plyr_idx, struct Packet* packet);
extern void clear_input(struct Packet* packet);
extern TbBool player_sell_room_at_subtile(long plyr_idx, long stl_x, long stl_y);
extern TbBool process_dungeon_control_packet_spell_overcharge(struct PlayerInfo* player, struct Packet* packet);
extern TbBool packets_process_cheats(
          long plyr_idx,
          MapCoord x, MapCoord y,
          struct Packet *packet,
          MapSubtlCoord stl_x, MapSubtlCoord stl_y,
          MapSlabCoord slb_x, MapSlabCoord slb_y,
          short *influence_own_creatures);

static void create_tag_action(struct PlayerInfo* player, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short flag)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct PlayerInfoAdd* playeradd = get_playeradd(plyr_idx);
    // The commented out section is the old way, this check is now performed as part of keeper_highlight_roomspace() in roomspace.cabs
    // which sets render_roomspace.untag_mode
    /*long i;
    i = get_subtile_number(stl_slab_center_subtile(stl_x),stl_slab_center_subtile(stl_y));
    if (find_from_task_list(plyr_idx,i) != -1)
        player->allocflags |= PlaF_ChosenSlabHasActiveTask;
    else
        player->allocflags &= ~PlaF_ChosenSlabHasActiveTask;*/

    if (playeradd->render_roomspace.untag_mode)
        player->allocflags |= PlaF_ChosenSlabHasActiveTask;
    else
        player->allocflags &= ~PlaF_ChosenSlabHasActiveTask;
}

static void set_untag_mode(struct PlayerInfo* player)
{
    int i = get_subtile_number(stl_slab_center_subtile(player->hand_stl_x), stl_slab_center_subtile(player->hand_stl_y));
    if (find_from_task_list(player->id_number, i) != -1)
        player->allocflags |= PlaF_Unknown20;
    else
        player->allocflags &= ~PlaF_Unknown20;
}

static TbBool process_dungeon_control_packet_dungeon_build_room(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct PlayerInfoAdd* playeradd = get_playeradd(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
      if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
      {
        player->field_4AF = 0;
        clear_input(pckt);
      }
      return false;
    }
    player->field_4A4 = 1;
    if (is_my_player(player))
    {
        gui_room_type_highlighted = player->chosen_room_kind;
    }
    get_dungeon_build_user_roomspace(&playeradd->render_roomspace, player->id_number, player->chosen_room_kind, stl_x, stl_y, playeradd->roomspace_mode);
    long i = tag_cursor_blocks_place_room(player->id_number, stl_x, stl_y, player->full_slab_cursor);
    if (playeradd->roomspace_mode != drag_placement_mode) // allows the user to hold the left mouse to use "paint mode"
    {
        if ((pckt->control_flags & PCtr_LBtnClick) == 0)
        {
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
            {
                player->field_4AF = 0;
                clear_input(pckt);
            }
            return false;
        }
    }
    else if ((pckt->control_flags & PCtr_LBtnHeld) == PCtr_LBtnHeld)
    {
        if ( (player->boxsize == 0) || (!can_build_room_at_slab(player->id_number, player->chosen_room_kind, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y))) )
        {
            return false; //stops attempts at invalid rooms, if left mouse button held (i.e. don't repeat failure sound repeatedly in paint mode)
        }
    }
    if (i == 0)
    {
        if (can_build_room_at_slab(player->id_number, player->chosen_room_kind, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y)))
        {
            struct Dungeon* dungeon = get_dungeon(player->id_number);
            if (playeradd->render_roomspace.total_roomspace_cost > dungeon->total_money_owned)
            {
                if (is_my_player(player))
                {
                    output_message(SMsg_GoldNotEnough, 0, true);
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
        keeper_build_roomspace(plyr_idx, &playeradd->render_roomspace);
    }
    else
    {
        if (is_my_player(player))
        {
            play_non_3d_sample(119);
        }
    }
    clear_input(pckt);
    return true;
}

static TbBool process_dungeon_power_hand_state(struct PlayerInfo* player, struct Packet* pckt)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct PlayerInfoAdd* playeradd = get_playeradd(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);

    player->field_3 &= ~Pf3F_Unkn02;
    if ((player->field_455 != P454_Unkn0) && (player->field_455 != P454_Unkn3))
    {
      if (player->instance_num != PI_Grab) {
          delete_power_hand(player->id_number);
      }
      return false;
    }
    struct Thing* thing = get_nearest_thing_for_hand_or_slap(plyr_idx, x, y);
    if (!thing_is_invalid(thing) && (!playeradd->one_click_lock_cursor))
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
            && (!playeradd->one_click_lock_cursor))
        {
            playeradd->render_roomspace = create_box_roomspace(playeradd->render_roomspace, 1, 1, subtile_slab(stl_x), subtile_slab(stl_y));
            player->full_slab_cursor = (playeradd->roomspace_mode != single_subtile_mode);
            tag_cursor_blocks_thing_in_hand(plyr_idx, stl_x, stl_y, i, player->full_slab_cursor);
        } else
        {
            player->additional_flags |= PlaAF_ChosenSubTileIsHigh;
            get_dungeon_highlight_user_roomspace(&playeradd->render_roomspace, player->id_number, stl_x, stl_y);
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
          set_power_hand_graphic(player->id_number, 782, 256);
          if (!thing_is_invalid(thing))
            thing->field_4F |= TF4F_Unknown01;
        } else
        if ((thing->class_id == TCls_Object) && object_is_gold_pile(thing))
        {
          set_power_hand_graphic(player->id_number, 781, 256);
          thing->field_4F &= ~TF4F_Unknown01;
        } else
        {
          set_power_hand_graphic(player->id_number, 784, 256);
          thing->field_4F &= ~TF4F_Unknown01;
        }
      }
    }
    return true;
}

static TbBool client_control_dungeon_build_room(struct PlayerInfo* player, struct Packet* pckt)
{
    struct Thing *thing;
    struct PlayerInfo* player = get_player(plyr_idx);
    struct PlayerInfoAdd* playeradd = get_playeradd(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0)
        player->secondary_cursor_state = CSt_DefaultArrow;
    player->primary_cursor_state = (unsigned short)(pckt->additional_packet_values & PCAdV_ContextMask) >> 1; // get current cursor state from pckt->additional_packet_values
    playeradd->render_roomspace.highlight_mode = false; // reset one-click highlight mode

    if (mode != drag_placement_mode) // allows the user to hold the left mouse to use "paint mode"
    {
        if ((pckt->control_flags & PCtr_LBtnClick) == 0)
        {
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
            {
                player->field_4AF = 0;
                clear_input(pckt);
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
        play_non_3d_sample(119);
        clear_input(pckt);
      }
      return false;
    }
    NETDBG(5, "x:%d y:%d plyr:%d room:%d", stl_x, stl_y, player->id_number, player->chosen_room_kind);
    struct BigActionPacket * big = create_packet_action_big(player, PckA_BuildRoom, 0);
    big->head.arg[0] = stl_x;
    big->head.arg[1] = stl_y;
    big->head.arg[2] = player->chosen_room_kind;

    if (player->boxsize > 0)
    {
        //keeper_build_roomspace(render_roomspace);
        if (player->boxsize > 1)
            message_add(10, "Merge required");
    }
    else
    {
        if (is_my_player(player))
        {
            play_non_3d_sample(119);
        }
    }
    clear_input(pckt);
    return true;
}

static void client_control_use_power_on_subtile(
          struct PlayerInfo* player, PowerKind pwkind, unsigned short splevel,
          MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    NETDBG(5, "x:%d y:%d plyr:%d power:%d %s level:%d",
        stl_x, stl_y, player->id_number, (int)pwkind, power_code_name(pwkind), (int)splevel);
    struct BigActionPacket * big = create_packet_action_big(player, PckA_UsePower, AP_PlusTwo);
    big->head.arg[0] = pwkind | (splevel << 8);
    big->head.arg[1] = 0;
    big->head.arg[2] = stl_x;
    big->head.arg[3] = stl_y;
}

static void client_control_use_power_on_thing(
          struct PlayerInfo* player, PowerKind pwkind, unsigned short splevel,
          MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short thing_idx)
{
    NETDBG(5, "x:%d y:%d plyr:%d power:%d %s level:%d",
        stl_x, stl_y, player->id_number, (int)pwkind, power_code_name(pwkind), (int)splevel);
    struct BigActionPacket * big = create_packet_action_big(player, PckA_UsePower, 0);
    big->head.arg[0] = pwkind | (splevel << 8);
    big->head.arg[1] = thing_idx;
    big->head.arg[2] = stl_x;
    big->head.arg[3] = stl_y;
}

static TbBool process_dungeon_control_packet_dungeon_control(struct PlayerInfo* player, struct Packet* pckt)
{
    struct Thing *thing;
    int plyr_idx = player->id_number;
    long i;
    struct Dungeon* dungeon = get_players_dungeon(player);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    MapSubtlCoord cx = stl_slab_starting_subtile(stl_x);
    MapSubtlCoord cy = stl_slab_starting_subtile(stl_y);
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0)
      player->field_455 = P454_Unkn0;
    player->field_454 = (unsigned short)(pckt->additional_packet_values & PCAdV_ContextMask) >> 1;

    process_dungeon_power_hand_state(player, pckt);

    if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
    {
      if (is_my_player(player) && !game_is_busy_doing_gui())
      {
        if (player->field_454 == P454_Unkn1)
          tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->field_4A4);
      }
      if ((pckt->control_flags & PCtr_LBtnClick) != 0)
      {
        player->hand_stl_x = stl_x;
        player->hand_stl_y = stl_y;
        player->field_4AF = 1;
        player->field_455 = player->field_454;
        switch (player->field_454)
        {
            get_dungeon_highlight_user_roomspace(&playeradd->render_roomspace, player->id_number, stl_x, stl_y);
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
                    if (thing->door.is_locked)
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
        clear_input(pckt);
      }
      if ((pckt->control_flags & PCtr_RBtnClick) != 0)
      {
        player->hand_stl_x = stl_x;
        player->hand_stl_y = stl_y;
        player->field_4AF = 1;
        clear_input(pckt);
      }
    }

    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
        if (player->field_455 == P454_Unkn0)
        {
          player->field_455 = player->field_454;
          if (player->field_454 == P454_Unkn1)
          {
            i = get_subtile_number(stl_slab_center_subtile(stl_x),stl_slab_center_subtile(stl_y));
            if (find_from_task_list(plyr_idx,i) != -1)
                player->allocflags |= PlaF_Unknown20;
            else
                player->allocflags &= ~PlaF_Unknown20;
          }
        }
        if (player->field_4AF != 0)
        {
            if (!playeradd->render_roomspace.drag_mode) // allow drag and click to not place on LMB hold
            {
              if ((player->allocflags & PlaF_Unknown20) != 0)
              {
                create_tag_action(player, cx, cy, 1);
              } else
              if (dungeon->task_count < 300)
              {
                create_tag_action(player, cx, cy, 0);
              } else
              if (is_my_player(player))
              {
                output_message(SMsg_WorkerJobsLimit, 500, true);
              }
            } else
            if ((player->field_455 == P454_Unkn3) && ((player->field_3 & Pf3F_Unkn01) != 0))
            {
              if ((player->allocflags & PlaF_Unknown20) != 0)
              {
                create_tag_action(player, cx, cy, 1);
              } else
              if (dungeon->task_count < 300)
              {
                if (can_dig_here(stl_x, stl_y, player->id_number))
                {

                    if (player->secondary_cursor_state == CSt_PickAxe)
                    {
                        keeper_highlight_roomspace(plyr_idx, &playeradd->render_roomspace, 0);
                    } else
                    if ((player->secondary_cursor_state == CSt_PowerHand) && ((player->additional_flags & PlaAF_NoThingUnderPowerHand) != 0))
                    {
                        keeper_highlight_roomspace(plyr_idx, &playeradd->render_roomspace, 0);
                    }
                }
              } else
              if (is_my_player(player))
              {
                output_message(SMsg_WorkerJobsLimit, 500, true);
              }
            }
          }
          clear_input(pckt);
        }
    }
    if ((pckt->control_flags & PCtr_RBtnHeld) != 0)
    {
      if (player->field_4AF != 0)
        clear_input(pckt);
    }
    if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
    {
        if (player->secondary_cursor_state == CSt_DefaultArrow)
            player->secondary_cursor_state = player->primary_cursor_state;
        if (playeradd->ignore_next_PCtr_LBtnRelease)
        {
            playeradd->ignore_next_PCtr_LBtnRelease = false;
            if ((pckt->control_flags & PCtr_RBtnHeld) == 0)
            {
                player->cursor_button_down = 0;
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
            clear_input(pckt);
          }
        } else
        if (player->field_455 == player->field_454)
        {
          if (player->field_454 == P454_Unkn1)
          {
            if ((player->allocflags & PlaF_Unknown20) != 0)
            {
              create_tag_action(player, cx, cy, 1);
            } else
            if (300-dungeon->task_count >= 9)
            {
              create_tag_action(player, cx, cy, 0);
            } else
            if (is_my_player(player))
            {
                if ((player->primary_cursor_state == CSt_PickAxe) || ((player->primary_cursor_state == CSt_PowerHand) && playeradd->render_roomspace.drag_mode))
                {
                    keeper_highlight_roomspace(plyr_idx, &playeradd->render_roomspace, 9);
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
                playeradd->one_click_lock_cursor = false;
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
            if (playeradd->render_roomspace.drag_mode)
            {
                if ((pckt->control_flags & PCtr_RBtnHeld) == 0)
                {
                    playeradd->render_roomspace.drag_mode = false;
                }
                else
                {
                    playeradd->render_roomspace.untag_mode = !playeradd->render_roomspace.untag_mode;
                    set_tag_untag_mode(plyr_idx, stl_x, stl_y);
                }
            }
            player->secondary_cursor_state = CSt_DefaultArrow;
            player->additional_flags &= ~PlaAF_NoThingUnderPowerHand;
        }
        player->field_4AF = 0;
        clear_input(pckt);
        player->field_455 = P454_Unkn0;
        player->field_3 &= ~Pf3F_Unkn01;
      }
    }

    if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
    {

        if (playeradd->ignore_next_PCtr_RBtnRelease && (!playeradd->one_click_lock_cursor))
        {
            playeradd->ignore_next_PCtr_RBtnRelease = false;
            if ((pckt->control_flags & PCtr_LBtnHeld) == 0)
            {
                player->cursor_button_down = 0;
            }
            unset_packet_control(pckt, PCtr_RBtnRelease);
        } else
        {

            if (!power_hand_is_empty(player) && (!playeradd->one_click_lock_cursor))
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
                if (player->primary_cursor_state == CSt_PowerHand && (!playeradd->one_click_lock_cursor)) {
                    thing = get_nearest_thing_for_slap(plyr_idx, subtile_coord_center(stl_x), subtile_coord_center(stl_y));
                    magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, 0, stl_x, stl_y, thing, PwMod_Default);
                }
                if ((pckt->control_flags & PCtr_LBtnHeld) == 0)
                {
                    player->cursor_button_down = 0;
                    playeradd->one_click_lock_cursor = false;
                }
                unset_packet_control(pckt, PCtr_RBtnRelease);
            }
        }
    }
    if ((player->cursor_button_down == 0) || (!playeradd->one_click_lock_cursor))
    {
        //if (untag_or_tag_completed_or_cancelled)
        playeradd->swap_to_untag_mode = 0; // no
        if ((player->cursor_button_down == 0) && ((pckt->control_flags & PCtr_LBtnHeld) == 0))
        {
            playeradd->one_click_lock_cursor = false;
        }
      }
    }
    return true;
}

/*
  Client side
*/
static void process_dungeon_control_packet_sell_operation(long plyr_idx, struct Packet* pckt)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    long keycode = 0;
    struct PlayerInfoAdd* playeradd = get_playeradd(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
        {
            player->field_4AF = 0;
            clear_input(pckt);
        }
        return;
    }
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    player->field_4A4 = 1;
    if (is_my_player(player))
    {
        if (!game_is_busy_doing_gui())
        {
            get_dungeon_sell_user_roomspace(stl_x, stl_y);
            tag_cursor_blocks_sell_area(player->id_number, stl_x, stl_y, player->field_4A4, (is_game_key_pressed(Gkey_SellTrapOnSubtile, &keycode, true)));
        }
    }
    player->full_slab_cursor = (playeradd->roomspace_mode != single_subtile_mode);
    get_dungeon_sell_user_roomspace(&playeradd->render_roomspace, player->id_number, stl_x, stl_y);
    tag_cursor_blocks_sell_area(plyr_idx, stl_x, stl_y, player->full_slab_cursor);
    if ((pckt->control_flags & PCtr_LBtnClick) == 0)
    {
      if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
      {
          player->field_4AF = 0;
          clear_input(pckt);
      }
      return;
    }
    if (!is_game_key_pressed(Gkey_SellTrapOnSubtile, &keycode, true))
    {
        //Slab Mode
        if (playeradd->render_roomspace.slab_count > 0)
        {
            keeper_sell_roomspace(plyr_idx, &playeradd->render_roomspace);
        }
        else
        {
            struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
            if (slabmap_owner(slb) != plyr_idx)
            {
                WARNLOG("Player %d can't sell item on %s owned by player %d at subtile (%d,%d).", (int)plyr_idx, slab_code_name(slb->kind), (int)slabmap_owner(slb), (int)stl_x, (int)stl_y);
                clear_input(pckt);
                return;
            }
            // Trying to sell room
            if (subtile_is_sellable_room(plyr_idx, stl_x, stl_y))
            {
                //player_sell_room_at_subtile(plyr_idx, stl_x, stl_y);
                struct BigActionPacket * big = create_packet_action_big(player, PckA_SellRoom, 0);
                big->head.arg[0] = plyr_idx;
                big->head.arg[1] = subtile_slab(stl_x) | (subtile_slab(stl_y) << 8);
                big->head.arg[2] = subtile_slab(stl_x) | (subtile_slab(stl_y) << 8);
            } else
            {
                create_packet_action(player, PckA_SellObject, 0,
                    stl_x | (stl_y << 8));
            }
        }
    }
    else
    {
        struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
        if (slabmap_owner(slb) != plyr_idx)
        {
            WARNLOG("Player %d can't sell item on %s owned by player %d at subtile (%d,%d).", (int)plyr_idx, slab_code_name(slb->kind), (int)slabmap_owner(slb), (int)stl_x, (int)stl_y);
            clear_input(pckt);
            return;
        }
        // Subtile Mode
        create_packet_action(player, PckA_SellObject, 0,
                             stl_x | (stl_y << 8));
    }
    clear_input(pckt);
}

static TbBool process_dungeon_control_packet_dungeon_place_trap(long plyr_idx, struct Packet* pckt)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);

    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
        {
          player->field_4AF = 0;
          clear_input(pckt);
        }
        return false;
    }
    player->field_4A4 = 1;
    long i = tag_cursor_blocks_place_trap(player->id_number, stl_x, stl_y);
    if ((pckt->control_flags & PCtr_LBtnClick) == 0)
    {
      if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
      {
        player->field_4AF = 0;
        clear_input(pckt);
      }
      return false;
    }
    if (i == 0)
    {
        if (is_my_player(player))
            play_non_3d_sample(119);
        clear_input(pckt);
        return false;
    }
    // Big packet used for addendum
    struct BigActionPacket *big = create_packet_action_big(player, PckA_PlaceTrap, 0);
    big->head.arg0 = player->chosen_trap_kind;
    big->head.arg1 = stl_x | (stl_y << 8);

    clear_input(pckt);
    return true;
}

/*
    This function should work only on client side. 
    It should emit other packets
*/
TbBool process_dungeon_control_packet_clicks(struct PlayerInfo* player, struct Packet* pckt)
{
    struct Thing *thing;
    PowerKind pwkind;
    int plyr_idx = player->id_number;
    SYNCDBG(6,"Starting for player:%d state:%s", 
                (int)player->id_number, player_state_code_name(player->work_state));

    if (pckt->control_flags & (PCtr_LBtnAnyAction | PCtr_RBtnAnyAction))
    {
        NETDBG(7, "turn:%04ld control_flags:%04x x:%03d y:%03d",
            game.play_gameturn, (int)pckt->control_flags, (int)pckt->pos_x, (int)pckt->pos_y);
    }

    player->field_4A4 = 1;
    struct PlayerInfo* player = get_player(plyr_idx);
    struct PlayerInfoAdd *playeradd = get_playeradd(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6,"Starting for player %d state %s",(int)plyr_idx,player_state_code_name(player->work_state));
    player->full_slab_cursor = 1;
    packet_left_button_double_clicked[plyr_idx] = 0;
    if ((pckt->control_flags & PCtr_Unknown4000) != 0)
      return false;
    TbBool ret = true;

    process_dungeon_control_packet_spell_overcharge(player, pckt);
    if ((pckt->control_flags & PCtr_RBtnHeld) != 0)
    {
      player->field_4D6++;
    } else
    if ((pckt->control_flags & PCtr_RBtnRelease) == 0)
    {
      player->boxsize = 1;
      player->field_4D6 = 0;
    }
    update_double_click_detection(plyr_idx, pckt);
    player->thing_under_hand = 0;
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    short influence_own_creatures = false;
    long i;
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    switch (player->work_state)
    {
    case PSt_CtrlDungeon:
        influence_own_creatures = 1;
        process_dungeon_control_packet_dungeon_control(player, pckt);
        break;
    case PSt_BuildRoom:
        client_control_dungeon_build_room(player, pckt);
        break;
    case PSt_CallToArms:
    case PSt_CaveIn:
    case PSt_SightOfEvil:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            pwkind = player_state_to_power_kind[player->work_state];
            i = get_power_overcharge_level(player);
            // magic_use_available_power_on_subtile(plyr_idx, pwkind, i, stl_x, stl_y, PwCast_None);
            client_control_use_power_on_subtile(player, pwkind, i, stl_x, stl_y);
            clear_input(pckt);
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
            // magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, 0, stl_x, stl_y, thing, PwMod_Default);
            client_control_use_power_on_thing(player, PwrK_SLAP, 0, stl_x, stl_y, thing->index);
            clear_input(pckt);
        }
        break;
    case PSt_CtrlPassngr:
    case PSt_FreeCtrlPassngr:
        influence_own_creatures = 1;
        if (player->work_state == PSt_CtrlPassngr)
            thing = get_creature_near_and_owned_by(x, y, plyr_idx);
        else
            thing = get_creature_near_and_owned_by(x, y, -1);
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
            clear_input(pckt);
          }
        }
        if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
        {
          if (player->instance_num != PI_PsngrCtrl)
          {
            set_player_state(player, player->continue_work_state, 0);
            clear_input(pckt);
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
              //magic_use_available_power_on_thing(plyr_idx, PwrK_POSSESS, 0, stl_x, stl_y, thing, PwMod_Default);
              client_control_use_power_on_thing(player, PwrK_POSSESS, 0, stl_x, stl_y, thing->index);
              clear_input(pckt);
          }
        }
        if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
        {
          if (player->instance_num != PI_DirctCtrl)
          {
            set_player_state(player, player->continue_work_state, 0);
            clear_input(pckt);
          }
        }
        break;
    case PSt_CreatrQuery:
    case PSt_CreatrInfo:
        influence_own_creatures = 1;
        thing = get_creature_near_and_owned_by(x, y, plyr_idx);
        TbBool CanQuery = can_thing_be_queried(thing, plyr_idx);
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
            clear_input(pckt);
          }
        }
        if (player->work_state == PSt_CreatrInfo)
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
            clear_input(pckt);
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
        process_dungeon_control_packet_dungeon_place_trap(plyr_idx, pckt);
        break;
    case PSt_Lightning:
        player->thing_under_hand = 0;
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            i = get_power_overcharge_level(player);
            //magic_use_available_power_on_subtile(plyr_idx, PwrK_LIGHTNING, i, stl_x, stl_y, PwCast_None);
            client_control_use_power_on_subtile(player, PwrK_LIGHTNING, i, stl_x, stl_y);
            clear_input(pckt);
        }
        break;
    case PSt_PlaceDoor:
    {
        if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
        {
            player->field_4A4 = 1;
            // Make the frame around active slab
            if ((pckt->control_flags & PCtr_LBtnClick) != 0)
            {
                // Big packet used for addendum
                struct BigActionPacket *big = create_packet_action_big(player, PckA_PlaceDoor, 0);
                big->head.arg0 = player->chosen_door_kind;
                big->head.arg1 = stl_x | (stl_y << 8);
            }
            clear_input(pckt);
        }
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
            if (player->field_4AF != 0)
            {
              player->field_4AF = 0;
              clear_input(pckt);
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
            //magic_use_available_power_on_thing(plyr_idx, pwkind, i, stl_x, stl_y, thing, PwMod_Default);
            client_control_use_power_on_thing(player, pwkind, i, stl_x, stl_y, thing->index);
            clear_input(pckt);
        }
        break;
    case PSt_Sell:
        process_dungeon_control_packet_sell_operation(plyr_idx, pckt);
        break;
    case PSt_CreateDigger:
    case PSt_DestroyWalls:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            pwkind = player_state_to_power_kind[player->work_state];
            i = get_power_overcharge_level(player);
            //magic_use_available_power_on_subtile(plyr_idx, pwkind, i, stl_x, stl_y, PwCast_None);
            client_control_use_power_on_subtile(player, pwkind, i, stl_x, stl_y);
            clear_input(pckt);
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
            //magic_use_available_power_on_thing(plyr_idx, pwkind, i, stl_x, stl_y, thing, PwMod_Default);
            client_control_use_power_on_thing(player, pwkind, i, stl_x, stl_y, thing->index);
            clear_input(pckt);
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
            //magic_use_available_power_on_thing(plyr_idx, pwkind, i, stl_x, stl_y, thing, PwMod_Default);
            client_control_use_power_on_thing(player, pwkind, i, stl_x, stl_y, thing->index);
            clear_input(pckt);
        }
        break;
    default:
        if (!packets_process_cheats(plyr_idx, x, y, pckt,
                stl_x, stl_y, slb_x, slb_y, &influence_own_creatures))
        {
            ERRORLOG("Unrecognized player %d work state: %d", (int)plyr_idx, (int)player->work_state);
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
        if (((player->secondary_cursor_state == CSt_DefaultArrow) || (player->secondary_cursor_state == CSt_PowerHand)) && (!playeradd->one_click_lock_cursor))
            stop_creatures_around_hand(plyr_idx, stl_x, stl_y);
    }
    return ret;
}
