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
 * @date     20 Sep 2020 - 20 Sep 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "packets.h"

#include "frontend.h"
#include "game_legacy.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "hist_actions.h"
#include "keeperfx.hpp"
#include "magic.h"
#include "map_blocks.h"
#include "player_instances.h"
#include "player_states.h"
#include "power_hand.h"

extern void clear_input(struct Packet* packet);

static void set_untag_mode(struct PlayerInfo* player)
{
    int i = get_subtile_number(stl_slab_center_subtile(player->field_4AB),stl_slab_center_subtile(player->field_4AD));
    if (find_from_task_list(player->id_number, i) != -1)
        player->allocflags |= PlaF_Unknown20;
    else
        player->allocflags &= ~PlaF_Unknown20;
}

TbBool process_dungeon_power_hand_state(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
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
    if (!thing_is_invalid(thing))
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
      if (can_drop_thing_here(stl_x, stl_y, player->id_number, i)
        || !can_dig_here(stl_x, stl_y, player->id_number))
      {
        tag_cursor_blocks_thing_in_hand(player->id_number, stl_x, stl_y, i, player->field_4A4);
      } else
      {
        player->field_3 |= Pf3F_Unkn02;
        tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->field_4A4);
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

    long i;
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    MapCoord x = ((unsigned short)pckt->pos_x);
    MapCoord y = ((unsigned short)pckt->pos_y);
    MapSubtlCoord stl_x = coord_subtile(x);
    MapSubtlCoord stl_y = coord_subtile(y);
    MapSubtlCoord cx = stl_slab_starting_subtile(stl_x);
    MapSubtlCoord cy = stl_slab_starting_subtile(stl_y);
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0)
      player->field_455 = P454_Unkn0;
    player->field_454 = (unsigned short)(pckt->field_10 & PCAdV_ContextMask) >> 1;

    process_dungeon_power_hand_state(plyr_idx);

    if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
    {
      if (is_my_player(player) && !game_is_busy_doing_gui())
      {
        if (player->field_454 == P454_Unkn1)
          tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->field_4A4);
      }
      if ((pckt->control_flags & PCtr_LBtnClick) != 0)
      {
        player->field_4AB = stl_x;
        player->field_4AD = stl_y;
        player->field_4AF = 1;
        player->field_455 = player->field_454;
        switch (player->field_454)
        {
        case P454_Unkn1:
          set_untag_mode(player);
          break;
        case P454_Unkn2:
          thing = get_door_for_position(player->field_4AB, player->field_4AD);
          if (thing_is_invalid(thing))
          {
            ERRORLOG("Door thing not found at map pos (%d,%d)",(int)player->field_4AB,(int)player->field_4AD);
            break;
          }
          if (thing->byte_18)
            unlock_door(thing);
          else
            lock_door(thing);
          break;
        case P454_Unkn3:
          if (player->thing_under_hand == 0)
          {
            set_untag_mode(player);
            player->field_3 |= Pf3F_Unkn01;
          }
          break;
        }
        clear_input(pckt);
      }
      if ((pckt->control_flags & PCtr_RBtnClick) != 0)
      {
        player->field_4AB = stl_x;
        player->field_4AD = stl_y;
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
          if (player->field_454 == player->field_455)
          {
            if (player->field_455 == P454_Unkn1)
            {
              if ((player->allocflags & PlaF_Unknown20) != 0)
              {
                hist_map_action(HAT_Untag, plyr_idx, cx, cy);
                untag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
              } else
              if (dungeon->task_count < 300)
              {
                if (tag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx))
                {
                  hist_map_action(HAT_Tag, plyr_idx, cx, cy);
                }
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
                hist_map_action(HAT_Untag, plyr_idx, cx, cy);
                untag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
              } else
              if (dungeon->task_count < 300)
              {
                if (can_dig_here(stl_x, stl_y, player->id_number))
                {
                  if (tag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx))
                  {
                    hist_map_action(HAT_Tag, plyr_idx, cx, cy);
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
      if (player->field_455 == P454_Unkn0)
        player->field_455 = player->field_454;
      if (player->field_4AF != 0)
      {
        thing = thing_get(player->thing_under_hand);
        if ((player->thing_under_hand != 0) && (player->input_crtr_control != 0)
          && (dungeon->things_in_hand[0] != player->thing_under_hand))
        {
            set_player_state(player, PSt_CtrlDirect, 0);
            if (magic_use_available_power_on_thing(plyr_idx, PwrK_POSSESS, 0, stl_x, stl_y, thing, PwMod_Default) == Lb_FAIL) {
                set_player_state(player, player->continue_work_state, 0);
            }
            clear_input(pckt);
        } else
        if ((player->thing_under_hand != 0) && (player->input_crtr_query != 0)
          && (dungeon->things_in_hand[0] != player->thing_under_hand)
          && can_thing_be_queried(thing, plyr_idx) )
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
        } else
        if (player->field_455 == player->field_454)
        {
          if (player->field_454 == P454_Unkn1)
          {
            if ((player->allocflags & PlaF_Unknown20) != 0)
            {
              untag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
            } else
            if (300-dungeon->task_count >= 9)
            {
              tag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
            } else
            if (is_my_player(player))
            {
              output_message(SMsg_WorkerJobsLimit, 500, true);
            }
          } else
          if (player->field_454 == P454_Unkn3)
          {
            if (player->thing_under_hand != 0) {
                // TODO SPELLS it's not a good idea to use this directly; change to magic_use_available_power_on_*()
                magic_use_power_hand(plyr_idx, stl_x, stl_y, 0);
            }
          }
        }
        player->field_4AF = 0;
        clear_input(pckt);
        player->field_455 = P454_Unkn0;
        player->field_3 &= ~Pf3F_Unkn01;
      }
    }

    if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
    {
      if (player->field_4AF != 0)
      {
        if (!power_hand_is_empty(player))
        {
          if (dump_first_held_thing_on_map(player->id_number, stl_x, stl_y, 1)) {
              player->field_4AF = 0;
              clear_input(pckt);
          }
        } else
        {
          if (player->field_454 == P454_Unkn3) {
              thing = get_nearest_thing_for_slap(plyr_idx, subtile_coord_center(stl_x), subtile_coord_center(stl_y));
              magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, 0, stl_x, stl_y, thing, PwMod_Default);
          }
          player->field_4AF = 0;
          clear_input(pckt);
        }
      }
    }
    return true;
}
