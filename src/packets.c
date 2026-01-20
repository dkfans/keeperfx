/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets.c
 *     Packet processing routines.
 * @par Purpose:
 *     Functions for creating and executing packets.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     30 Jan 2009 - 11 Oct 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "packets.h"
#include "net_received_packets.h"
#include "net_input_lag.h"
#include "net_checksums.h"

#include <math.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_datetm.h"
#include "bflib_math.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_keybrd.h"
#include "bflib_vidraw.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_network.h"
#include "bflib_network_exchange.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_sprfnt.h"
#include "bflib_planar.h"
#include "bflib_inputctrl.h"

#include "kjm_input.h"
#include "front_input.h"
#include "front_simple.h"
#include "front_landview.h"
#include "front_network.h"
#include "frontmenu_net.h"
#include "frontend.h"
#include "vidmode.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_effects.h"
#include "config_terrain.h"
#include "config_players.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "player_instances.h"
#include "player_data.h"
#include "config_players.h"
#include "player_utils.h"
#include "engine_camera.h"
#include "engine_render.h"
#include "local_camera.h"
#include "thing_physics.h"
#include "thing_doors.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_creature.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_groups.h"
#include "console_cmd.h"
#include "dungeon_data.h"
#include "tasks_list.h"
#include "power_specials.h"
#include "power_hand.h"
#include "room_util.h"
#include "room_workshop.h"
#include "room_data.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "magic_powers.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "light_data.h"
#include "gui_draw.h"
#include "gui_topmsg.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "gui_parchment.h"
#include "gui_msgs.h"
#include "net_game.h"
#include "net_resync.h"
#include "game_legacy.h"
#include "engine_redraw.h"
#include "frontmenu_ingame_tabs.h"
#include "vidfade.h"
#include "spdigger_stack.h"
#include "frontmenu_ingame_map.h"
#include "lua_triggers.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
extern TbBool process_players_global_cheats_packet_action(PlayerNumber plyr_idx, struct Packet* pckt);
extern TbBool process_players_dungeon_control_cheats_packet_action(PlayerNumber plyr_idx, struct Packet* pckt);
extern TbBool change_campaign(const char *cmpgn_fname);
extern int total_sprite_zip_count;
/******************************************************************************/
unsigned long scheduled_unpause_time = 0;
/******************************************************************************/
void set_packet_action(struct Packet *pckt, unsigned char pcktype, long par1, long par2, unsigned short par3, unsigned short par4)
{
    pckt->actn_par1 = par1;
    pckt->actn_par2 = par2;
    pckt->actn_par3 = par3;
    pckt->actn_par4 = par4;
    pckt->action = pcktype;
}

TbBool is_packet_empty(const struct Packet *pckt) {
    if (pckt->turn != 0 ||
        pckt->checksum != 0 ||
        pckt->action != 0 ||
        pckt->actn_par1 != 0 ||
        pckt->actn_par2 != 0 ||
        pckt->pos_x != 0 ||
        pckt->pos_y != 0 ||
        pckt->control_flags != 0 ||
        pckt->additional_packet_values != 0 ||
        pckt->actn_par3 != 0 ||
        pckt->actn_par4 != 0) {
        return false;
    }
    return true;
}

void update_double_click_detection(long plyr_idx)
{
    struct Packet* pckt = get_packet(plyr_idx);
    if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
    {
        if (packet_left_button_click_space_count[plyr_idx] < 5)
            packet_left_button_double_clicked[plyr_idx] = 1;
        packet_left_button_click_space_count[plyr_idx] = 0;
  }
  if ((pckt->control_flags & (PCtr_LBtnClick|PCtr_LBtnHeld)) == 0)
  {
    if (packet_left_button_click_space_count[plyr_idx] < INT32_MAX)
      packet_left_button_click_space_count[plyr_idx]++;
  }
}

struct Room *keeper_build_room(long stl_x,long stl_y,long plyr_idx,long rkind)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    // Take top left subtile on single subtile boundbox, take center subtile on full slab boundbox
    MapCoord x = ((player->full_slab_cursor == 0) ? slab_subtile(subtile_slab(stl_x), 0) : slab_subtile_center(subtile_slab(stl_x)));
    MapCoord y = ((player->full_slab_cursor == 0) ? slab_subtile(subtile_slab(stl_y), 0) : slab_subtile_center(subtile_slab(stl_y)));
    struct Room* room = player_build_room_at(x, y, plyr_idx, rkind);
    if (!room_is_invalid(room))
    {
        if (player->boxsize > 1)
        {
            dungeon->camera_deviate_jump = 240;
        }
        else
        {
            dungeon->camera_deviate_jump = 192;
        }
        struct Coord3d pos;
        set_coords_to_slab_center(&pos, subtile_slab(stl_x), subtile_slab(stl_y));
        create_price_effect(&pos, plyr_idx, roomst->cost);
    }
    return room;
}

TbBool process_dungeon_control_packet_spell_overcharge(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    SYNCDBG(6,"Starting for player %d state %s",(int)plyr_idx,player_state_code_name(player->work_state));
    struct Packet* pckt = get_packet_direct(player->packet_num);

    while (game.conf.rules[plyr_idx].magic.allow_instant_charge_up && is_game_key_pressed(Gkey_SpeedMod, NULL, true))
    {
        struct PowerConfigStats *powerst = get_power_model_stats(player->chosen_power_kind);

        if (powerst->overcharge_check_idx == OcC_CallToArms_expand
            || powerst->overcharge_check_idx == OcC_SightOfEvil_expand
            || powerst->overcharge_check_idx == OcC_General_expand)
        {
            if (powerst->overcharge_check_idx == OcC_CallToArms_expand && player_uses_power_call_to_arms(plyr_idx))
                break;

            while(update_power_overcharge(player, player->chosen_power_kind))
            {}

            return true;
        }
        break;
    }

    if (flag_is_set(pckt->control_flags,PCtr_LBtnHeld))
    {
        struct PowerConfigStats *powerst = get_power_model_stats(player->chosen_power_kind);

        switch (powerst->overcharge_check_idx)
        {
            case OcC_CallToArms_expand:
                if (player_uses_power_call_to_arms(plyr_idx))
                    player->cast_expand_level = (dungeon->cta_power_level << 2);
                else
                    update_power_overcharge(player, player->chosen_power_kind);
                break;
            case OcC_SightOfEvil_expand:
            case OcC_General_expand:
                update_power_overcharge(player, player->chosen_power_kind);
                break;
            case OcC_do_not_expand:
            case OcC_Null:
            default:
                player->cast_expand_level++;
                break;
        }
        return true;
    }
    if ((pckt->control_flags & PCtr_LBtnRelease) == 0)
    {
        player->cast_expand_level = 0;
        return false;
    }
    return false;
}

TbBool player_sell_room_at_subtile(long plyr_idx, long stl_x, long stl_y)
{
    struct Room* room = subtile_room_get(stl_x, stl_y);
    if (room_is_invalid(room))
    {
        ERRORLOG("No room to delete at subtile (%d,%d)",(int)stl_x,(int)stl_y);
        return false;
    }
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    long revenue = compute_value_percentage(roomst->cost, game.conf.rules[plyr_idx].game.room_sale_percent);
    if (room->owner != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_players_num_dungeon(room->owner);
        dungeon->rooms_destroyed++;
        dungeon->camera_deviate_jump = 192;
    }
    delete_room_slab(subtile_slab(stl_x), subtile_slab(stl_y), 0);
    if (is_my_player_number(plyr_idx))
        play_non_3d_sample(115);
    if (revenue != 0)
    {
        struct Coord3d pos;
        set_coords_to_slab_center(&pos, subtile_slab(stl_x), subtile_slab(stl_y));
        create_price_effect(&pos, plyr_idx, revenue);
        player_add_offmap_gold(plyr_idx, revenue);
    }
    return true;
}


void process_pause_packet(long curr_pause, long new_pause)
{
  struct PlayerInfo *player;
  TbBool can = true;
  for (long i = 0; i < PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player) && (player->is_active == 1))
    {
        if ((player->allocflags & PlaF_CompCtrl) == 0)
        {
            if ((player->instance_num == PI_MapFadeTo)
             || (player->instance_num == PI_MapFadeFrom)
             || (player->instance_num == PI_CrCtrlFade)
             || (player->instance_num == PI_DirctCtrl)
             || (player->instance_num == PI_PsngrCtrl)
             || (player->instance_num == PI_DirctCtLeave)
             || (player->instance_num == PI_PsngrCtLeave))
            {
              can = false;
              break;
            }
        }
    }
  }
  if ( can )
  {
      player = get_my_player();
      set_flag_value(game.operation_flags, GOF_Paused, curr_pause);
      if ((game.operation_flags & GOF_Paused) != 0) {
          set_flag_value(game.operation_flags, GOF_WorldInfluence, new_pause);
          game.skip_initial_input_turns = game.input_lag_turns + 1;
      }
      if ( !SoundDisabled )
      {
        if ((game.operation_flags & GOF_Paused) != 0)
        {
          SetSoundMasterVolume(settings.sound_volume >> 1);
          set_music_volume(settings.music_volume >> 1);
        } else
        {
          SetSoundMasterVolume(settings.sound_volume);
          set_music_volume(settings.music_volume);
        }
      }
      if ((game.operation_flags & GOF_Paused) != 0)
      {
          if ((player->additional_flags & PlaAF_LightningPaletteIsActive) != 0)
          {
              PaletteSetPlayerPalette(player, engine_palette);
              player->additional_flags &= ~PlaAF_LightningPaletteIsActive;
          }
      }
  }
}

void process_camera_controls(struct Camera* cam, struct Packet* pckt, struct PlayerInfo* player, TbBool is_local_camera)
{
    if (cam == NULL) {
        return;
    }
    long inter_val;
    int scroll_speed = cam->zoom;
    if (scroll_speed <= 0)
        scroll_speed = 1;
    switch (cam->view_mode)
    {
    case PVM_IsoWibbleView:
    case PVM_IsoStraightView:
        if (player->roomspace_drag_paint_mode == 1)
        {
            if (scroll_speed < 4100)
            {
                scroll_speed = 4100;
            }
        }
        inter_val = 2560000 / scroll_speed;
        break;
    case PVM_FrontView:
        if (player->roomspace_drag_paint_mode == 1)
        {
            if (scroll_speed < 16384)
            {
                scroll_speed = 16384;
            }
        }
        inter_val = 12800000 / scroll_speed;
        break;
    default:
        inter_val = 256;
        break;
    }
    if (pckt->additional_packet_values & PCAdV_SpeedupPressed)
      inter_val *= 3;

    if (is_local_camera && !game.packet_load_enable)
    {
        movement_accum_x = clamp(movement_accum_x, -1.0f, 1.0f);
        movement_accum_y = clamp(movement_accum_y, -1.0f, 1.0f);
        
        // Apply same scaling as packet-based movement for consistency
        if (movement_accum_y != 0.0f) {
            long delta = (long)(movement_accum_y * inter_val / 4.0f);
            long limit = (long)(movement_accum_y * inter_val);
            view_set_camera_y_inertia(cam, delta, limit);
        }
        if (movement_accum_x != 0.0f) {
            long delta = (long)(movement_accum_x * inter_val / 4.0f);
            long limit = (long)(movement_accum_x * inter_val);
            view_set_camera_x_inertia(cam, delta, limit);
        }
        movement_accum_x = 0.0f;
        movement_accum_y = 0.0f;
    }
    else
    {
        // Packet-based movement for non-local cameras or when local camera is disabled
        if ((pckt->control_flags & PCtr_MoveUp) != 0) {
            view_set_camera_y_inertia(cam, -inter_val/4, -inter_val);
        }
        if ((pckt->control_flags & PCtr_MoveDown) != 0) {
            view_set_camera_y_inertia(cam, inter_val/4, inter_val);
        }
        if ((pckt->control_flags & PCtr_MoveLeft) != 0) {
            view_set_camera_x_inertia(cam, -inter_val/4, -inter_val);
        }
        if ((pckt->control_flags & PCtr_MoveRight) != 0) {
            view_set_camera_x_inertia(cam, inter_val/4, inter_val);
        }
    }


    if ((pckt->control_flags & PCtr_ViewRotateCCW) != 0)
    {
        switch (cam->view_mode)
        {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
             view_set_camera_rotation_inertia(cam, 16, 64);
            break;
        case PVM_FrontView:
            cam->rotation_angle_x = (cam->rotation_angle_x + DEGREES_90) & ANGLE_MASK;
            break;
        }
    }
    if ((pckt->control_flags & PCtr_ViewRotateCW) != 0)
    {
        switch (cam->view_mode)
        {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
            view_set_camera_rotation_inertia(cam, -16, -64);
            break;
        case PVM_FrontView:
            cam->rotation_angle_x = (cam->rotation_angle_x - DEGREES_90) & ANGLE_MASK;
            break;
        }
    }
    if ((pckt->control_flags & PCtr_ViewTiltUp) != 0)
    {
        switch (cam->view_mode)
        {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
            view_set_camera_tilt(cam, 1);
            break;
        }
    }
    if ((pckt->control_flags & PCtr_ViewTiltDown) != 0)
    {
        switch (cam->view_mode)
        {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
            view_set_camera_tilt(cam, 2);
            break;
        }
    }
    if ((pckt->control_flags & PCtr_ViewTiltReset) != 0)
    {
        switch (cam->view_mode)
        {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
            view_set_camera_tilt(cam, 0);
            break;
        }
    }
    unsigned long zoom_min = max(CAMERA_ZOOM_MIN, zoom_distance_setting);
    unsigned long zoom_max = CAMERA_ZOOM_MAX;
    if (pckt->control_flags & PCtr_ViewZoomIn)
    {
        switch (cam->view_mode)
        {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
            view_zoom_camera_in(cam, zoom_max, zoom_min);
            update_camera_zoom_bounds(cam, zoom_max, zoom_min);
            break;
        default:
            view_zoom_camera_in(cam, zoom_max, zoom_min);
            break;
        }
    }
    if (pckt->control_flags & PCtr_ViewZoomOut)
    {
        switch (cam->view_mode)
        {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
            view_zoom_camera_out(cam, zoom_max, zoom_min);
            update_camera_zoom_bounds(cam, zoom_max, zoom_min);
            break;
        default:
            view_zoom_camera_out(cam, zoom_max, zoom_min);
            break;
        }
    }
}

void update_box_lag_compensation(struct PlayerInfo* player) {
    box_lag_compensation_x = 0;
    box_lag_compensation_y = 0;
    if (is_my_player(player)) {
        struct Packet* auth_pckt = get_packet_direct(player->packet_num);
        struct Packet* visual_pckt = get_local_input_lag_packet_for_turn(game.play_gameturn);
        if (visual_pckt != NULL) {
            box_lag_compensation_x = coord_slab(auth_pckt->pos_x) - coord_slab(visual_pckt->pos_x);
            box_lag_compensation_y = coord_slab(auth_pckt->pos_y) - coord_slab(visual_pckt->pos_y);
            box_lag_compensation_x = slab_coord(box_lag_compensation_x);
            box_lag_compensation_y = slab_coord(box_lag_compensation_y);
        }
    }
}

void process_players_dungeon_control_packet_control(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6,"Processing player %d action %d",(int)plyr_idx,(int)pckt->action);
    struct Camera* cam = player->acamera;
    if (cam == NULL) {
        ERRORLOG("No active camera");
        return;
    }
    process_camera_controls(cam, pckt, player, false);
    if (is_my_player(player)) {
        TbBool settings_changed = false;
        if ((pckt->control_flags & (PCtr_ViewTiltUp | PCtr_ViewTiltDown | PCtr_ViewTiltReset)) != 0) {
            settings.isometric_tilt = cam->rotation_angle_y;
            settings_changed = true;
        }
        if ((pckt->control_flags & (PCtr_ViewZoomIn | PCtr_ViewZoomOut)) != 0) {
            if (cam->view_mode == PVM_IsoWibbleView || cam->view_mode == PVM_IsoStraightView) {
                settings.isometric_view_zoom_level = cam->zoom;
            } else {
                settings.frontview_zoom_level = cam->zoom;
            }
            settings_changed = true;
        }
        if (settings_changed) {
            save_settings();
        }
    }
    if (is_my_player(player)) {
        update_box_lag_compensation(player);
    }
    process_dungeon_control_packet_clicks(plyr_idx);
    set_mouse_light(player);
}

void message_text_key_add(char *message, TbKeyCode key, TbKeyMods kmodif)
{
    int chpos = strlen(message);
    if (key == KC_BACK && chpos > 0) {
        message[chpos-1] = '\0';
    } else if (chpos < PLAYER_MP_MESSAGE_LEN - 1) {
        char chr = key_to_ascii(key, kmodif);
        if (isalnum(chr) || strchr(" !:;()._'+=\\\"?/#<>^,-", chr)) {
            message[chpos] = chr;
            message[chpos+1] = '\0';
        }
    }
}

void process_chat_message_end(int player_id, const char *message)
{
    struct PlayerInfo *player = get_player(player_id);
    player->allocflags &= ~PlaF_NewMPMessage;
    if (message[0] != '\0') {
        memcpy(player->mp_message_text, message, PLAYER_MP_MESSAGE_LEN);
        memcpy(player->mp_message_text_last, message, PLAYER_MP_MESSAGE_LEN);
        lua_on_chatmsg(player_id, player->mp_message_text);
        if (message[0] != cmd_char || !cmd_exec(player_id, player->mp_message_text + 1) || (game.system_flags & GSF_NetworkActive) != 0) {
            message_add(MsgType_Player, player_id, player->mp_message_text);
        }
    }
    memset(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
}

void process_quit_packet(struct PlayerInfo *player, short complete_quit)
{
    struct PlayerInfo *swplyr;
    struct PlayerInfo* myplyr = get_my_player();
    int32_t plyr_count;
    plyr_count = 0;
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        short winning_quit = winning_player_quitting(player, &plyr_count);
        if (winning_quit)
        {
            // Set other players as losers
            for (int i = 0; i < PLAYERS_COUNT; i++)
            {
                swplyr = get_player(i);
                if (player_exists(swplyr))
                {
                    if (swplyr->is_active == 1)
                        if (swplyr->victory_state == VicS_Undecided)
                            swplyr->victory_state = VicS_WonLevel;
                }
            }
      }

      if ((player == myplyr) || (frontend_should_all_players_quit()))
      {
        if ((!winning_quit) || (plyr_count <= 1))
          LbNetwork_Stop();
        else
          myplyr->additional_flags |= PlaAF_UnlockedLordTorture;
      } else
      {
        if (!winning_quit)
        {
          if (player->victory_state != VicS_Undecided)
          {
            player->allocflags &= ~PlaF_Allocated;
          } else
          {
            player->allocflags |= PlaF_CompCtrl;
            toggle_computer_player(player->id_number);
          }
          if (player == myplyr)
          {
            quit_game = 1;
            if (complete_quit)
              exit_keeper = 1;
          }
          return;
        } else
        if (plyr_count <= 1)
          LbNetwork_Stop();
        else
          myplyr->additional_flags |= PlaAF_UnlockedLordTorture;
      }
      quit_game = 1;
      if (complete_quit)
        exit_keeper = 1;
      if (frontend_should_all_players_quit())
      {
        for (int i=0; i < PLAYERS_COUNT; i++)
        {
          swplyr = get_player(i);
          if (player_exists(swplyr))
          {
            swplyr->allocflags &= ~PlaF_Allocated;
            swplyr->display_flags |= PlaF6_PlyrHasQuit;
          }
        }
      } else
      {
        player->allocflags &= ~PlaF_Allocated;
      }
      return;
    }
    player->allocflags &= ~PlaF_Allocated;
    if (player == myplyr)
    {
        quit_game = 1;
        if (complete_quit)
          exit_keeper = 1;
    }
}

TbBool process_players_global_packet_action(PlayerNumber plyr_idx)
{
  //TODO PACKET add commands from beta
  struct PlayerInfo* player = get_player(plyr_idx);
  struct Packet* pckt = get_packet_direct(player->packet_num);
  SYNCDBG(6,"Processing player %d action %d",(int)plyr_idx,(int)pckt->action);
  struct Dungeon *dungeon;
  struct Thing *thing;
  int i;
  switch (pckt->action)
  {
  case PckA_QuitToMainMenu:
      if (is_my_player(player))
      {
        turn_off_all_menus();
        frontend_save_continue_game(true);
        free_swipe_graphic();
      }
      player->display_flags |= PlaF6_PlyrHasQuit;
      process_quit_packet(player, 0);
      return 1;
  case PckA_ForceApplicationClose:
      {
        extern unsigned char exit_keeper;
        if (is_my_player(player))
        {
          turn_off_all_menus();
          frontend_save_continue_game(true);
          free_swipe_graphic();
          // For ALT+F4, just exit directly without network cleanup
          exit_keeper = 1;
        }
        else
        {
          // Other player force-quit, just mark them as quit
          player->display_flags |= PlaF6_PlyrHasQuit;
          process_quit_packet(player, 0);
        }
        return 1;
      }
  case PckA_SaveGameAndQuit:
      if (is_my_player(player))
      {
        turn_off_all_menus();
        frontend_save_continue_game(true);
      }
      player->display_flags |= PlaF6_PlyrHasQuit;
      process_quit_packet(player, 1);
      return 1;
  case PckA_NoOperation:
      return 1;
  case PckA_FinishGame:
      if (is_my_player(player))
      {
        turn_off_all_menus();
        free_swipe_graphic();
      }
      if ((game.system_flags & GSF_NetworkActive) != 0)
      {
        process_quit_packet(player, 0);
        return 0;
      }
      switch (player->victory_state)
      {
      case VicS_WonLevel:
          complete_level(player);
          break;
      case VicS_LostLevel:
          lose_level(player);
          break;
      default:
          resign_level(player);
          break;
      }
      player->allocflags &= ~PlaF_Allocated;
      if (is_my_player(player))
      {
        frontend_save_continue_game(false);
      }
      return 0;
  case PckA_PlyrMsgEnd:
      process_chat_message_end(player->id_number, player->mp_pending_message);
      player->mp_pending_message[0] = '\0';
      return 0;
  case PckA_PlyrMsgClear:
      player->allocflags &= ~PlaF_NewMPMessage;
      memset(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
      return 0;
  case PckA_ToggleLights:
      if (is_my_player(player))
      {
          light_set_lights_on(game.lish.light_enabled == 0);
      }
      return 1;
  case PckA_SwitchScrnRes:
      if (is_my_player(player))
      {
          switch_to_next_video_mode_wrapper();
      }
      return 1;
  case PckA_TogglePause:
      process_pause_packet(pckt->actn_par1, 0);
      return 1;
  case PckA_SetCluedo:
      player->video_cluedo_mode = pckt->actn_par1;
      if (is_my_player(player))
      {
        settings.video_cluedo_mode = player->video_cluedo_mode;
        save_settings();
      }
      return 0;
  case PckA_ChangeWindowSize:
      if (is_my_player(player))
      {
        change_engine_window_relative_size(pckt->actn_par1, pckt->actn_par2);
        centre_engine_window();
      }
      return 0;
  case PckA_BookmarkLoad:
      set_player_cameras_position(player, subtile_coord_center(pckt->actn_par1), subtile_coord_center(pckt->actn_par2));
      return 0;
  case PckA_SetGammaLevel:
      if (is_my_player(player))
      {
        set_gamma(pckt->actn_par1, 1);
        save_settings();
      }
      return 0;
  case PckA_SetMinimapConf:
      player->minimap_zoom = pckt->actn_par1;
      if (is_my_player(player))
      {
        settings.minimap_zoom = player->minimap_zoom;
        save_settings();
      }
      return 0;
  case PckA_SetMapRotation:
      player->cameras[CamIV_Parchment].rotation_angle_x = pckt->actn_par1;
      player->cameras[CamIV_FrontView].rotation_angle_x = pckt->actn_par1;
      player->cameras[CamIV_Isometric].rotation_angle_x = pckt->actn_par1;
      set_local_camera_destination(player);
      return 0;
  case PckA_SetPlyrState:
      set_player_state(player, pckt->actn_par1, pckt->actn_par2);
      return 0;
  case PckA_SwitchView:
      set_engine_view(player, pckt->actn_par1);
      return 0;
  case PckA_ToggleTendency:
      toggle_creature_tendencies(player, pckt->actn_par1);
      if (is_my_player(player)) {
          dungeon = get_players_dungeon(player);
          game.creatures_tend_imprison = ((dungeon->creature_tendencies & CrTend_Imprison) != 0);
          game.creatures_tend_flee = ((dungeon->creature_tendencies & CrTend_Flee) != 0);
      }
      return 0;
  case PckA_CheatUnusedPlaceholder065:
      //TODO: remake from beta
      return 0;
  case PckA_CheatUnusedPlaceholder068:
      //TODO: remake from beta
      return 0;
  case PckA_CheatUnusedPlaceholder069:
      //TODO: remake from beta
      return 0;
  case PckA_SetViewType:
      set_player_mode(player, pckt->actn_par1);
      return 0;
  case PckA_ZoomFromMap:
      set_player_cameras_position(player, subtile_coord_center(pckt->actn_par1), subtile_coord_center(pckt->actn_par2));
      player->cameras[CamIV_Parchment].rotation_angle_x = 0;
      player->cameras[CamIV_FrontView].rotation_angle_x = 0;
      player->cameras[CamIV_Isometric].rotation_angle_x = 0;
      if (((game.system_flags & GSF_NetworkActive) != 0)
          || (lbDisplay.PhysicalScreenWidth > 320))
      {
        if (is_my_player_number(plyr_idx))
          toggle_status_menu((game.operation_flags & GOF_ShowPanel) != 0);
        set_player_mode(player, PVT_DungeonTop);
      } else
      {
        set_player_mode(player, PVT_MapFadeOut);
      }
      return 0;
  case PckA_UpdatePause:
      process_pause_packet(pckt->actn_par1, pckt->actn_par2);
      return 1;
  case PckA_ZoomToEvent:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      event_move_player_towards_event(player, pckt->actn_par1);
      return 0;
  case PckA_ZoomToRoom:
  {
      if (player->instance_num == PI_ZoomToPos) {
          return 0;
      }
      if (player->work_state == PSt_CreatrInfo)
          turn_off_query(plyr_idx);
      struct Room* room = room_get(pckt->actn_par1);
      player->zoom_to_pos_x = subtile_coord_center(room->central_stl_x);
      player->zoom_to_pos_y = subtile_coord_center(room->central_stl_y);
      set_player_instance(player, PI_ZoomToPos, 0);
      if (player->work_state == PSt_BuildRoom) {
          set_player_state(player, PSt_BuildRoom, room->kind);
      }
      return 0;
  }
  case PckA_ZoomToTrap:
      if (player->instance_num == PI_ZoomToPos) {
          return 0;
      }
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      thing = thing_get(pckt->actn_par1);
      player->zoom_to_pos_x = thing->mappos.x.val;
      player->zoom_to_pos_y = thing->mappos.y.val;
      set_player_instance(player, PI_ZoomToPos, 0);
      if ((player->work_state == PSt_PlaceTrap) || (player->work_state == PSt_PlaceDoor)) {
          set_player_state(player, PSt_PlaceTrap, thing->model);
      }
      return 0;
  case PckA_ZoomToDoor:
      if (player->instance_num == PI_ZoomToPos) {
          return 0;
      }
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      thing = thing_get(pckt->actn_par1);
      player->zoom_to_pos_x = thing->mappos.x.val;
      player->zoom_to_pos_y = thing->mappos.y.val;
      set_player_instance(player, PI_ZoomToPos, 0);
      if ((player->work_state == PSt_PlaceTrap) || (player->work_state == PSt_PlaceDoor)) {
          set_player_state(player, PSt_PlaceDoor, thing->model);
      }
      return 0;
  case PckA_ZoomToPosition:
      if (player->instance_num == PI_ZoomToPos) {
          return 0;
      }
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      player->zoom_to_pos_x = pckt->actn_par1;
      player->zoom_to_pos_y = pckt->actn_par2;
      set_player_instance(player, PI_ZoomToPos, 0);
      return 0;
  case PckA_ToggleComputerProcessing:
      game.view_mode_flags ^= (game.view_mode_flags ^ (GNFldD_ComputerPlayerProcessing * ((game.view_mode_flags & GNFldD_ComputerPlayerProcessing) == 0))) & GNFldD_ComputerPlayerProcessing;
      return 0;
  case PckA_PwrCTADis:
      turn_off_power_call_to_arms(plyr_idx);
      return 0;
  case PckA_UsePwrHandPick:
      thing = thing_get(pckt->actn_par1);
      magic_use_available_power_on_thing(plyr_idx, PwrK_HAND, 0,thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, PwMod_Default);
      return 0;
  case PckA_UsePwrHandDrop:
      dump_first_held_thing_on_map(plyr_idx, pckt->actn_par1, pckt->actn_par2, 1);
      return 0;
  case PckA_EventBoxTurnOff:
      if (game.event[pckt->actn_par1].kind == 3)
      {
        turn_off_event_box_if_necessary(plyr_idx, pckt->actn_par1);
      } else
      {
        event_delete_event(plyr_idx, pckt->actn_par1);
      }
      return 0;
  case PckA_GenericLevelPower:
      magic_use_available_power_on_level(plyr_idx, pckt->actn_par2, 0, PwMod_Default);
      return 0;
  case PckA_UsePwrObey:
      magic_use_available_power_on_level(plyr_idx, PwrK_OBEY, 0, PwMod_Default);
      return 0;
  case PckA_UsePwrArmageddon:
      magic_use_available_power_on_level(plyr_idx, PwrK_ARMAGEDDON, 0, PwMod_Default);
      return 0;
  case PckA_TurnOffQuery:
      turn_off_query(plyr_idx);
      return 0;
  case PckA_ZoomToBattle:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      battle_move_player_towards_battle(player, pckt->actn_par1);
      return 0;
  case PckA_ZoomToSpell:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      {
          struct Coord3d locpos;
          if (find_power_cast_place(plyr_idx, pckt->actn_par1, &locpos))
          {
              player->zoom_to_pos_x = locpos.x.val;
              player->zoom_to_pos_y = locpos.y.val;
              set_player_instance(player, PI_ZoomToPos, 0);
          }
      }
      if (!power_is_instinctive(pckt->actn_par1))
      {
          const struct PowerConfigStats *powerst;
          powerst = get_power_model_stats(pckt->actn_par1);
          i = get_power_index_for_work_state(player->work_state);
          if (i > 0)
            set_player_state(player, powerst->work_state, pckt->actn_par1);
      }
      return 0;
  case PckA_PlyrFastMsg:
      //show_onscreen_msg(game.num_fps, "Message from player %d", plyr_idx);
      output_message(SMsg_EnemyHarassments+pckt->actn_par1, 0);
      return 0;
  case PckA_SetComputerKind:
      set_autopilot_type(plyr_idx, pckt->actn_par1);
      return 0;
  case PckA_GoSpectator:
      level_lost_go_first_person(plyr_idx);
      return 0;
  case PckA_DumpHeldThingToOldPos:
      dungeon = get_players_num_dungeon(plyr_idx);
      if (!power_hand_is_empty(player))
      {
          thing = get_first_thing_in_power_hand(player);
          dump_first_held_thing_on_map(plyr_idx, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 1);
      }
      return false;
  case PckA_PwrSOEDis:
      turn_off_power_sight_of_evil(plyr_idx);
      return false;
  case PckA_EventBoxActivate:
      go_on_then_activate_the_event_box(plyr_idx, pckt->actn_par1);
      return false;
  case PckA_EventBoxClose:
      dungeon = get_players_num_dungeon(plyr_idx);
      turn_off_event_box_if_necessary(plyr_idx, dungeon->visible_event_idx);
      dungeon->visible_event_idx = 0;
      return false;
  case PckA_UsePwrOnThing:
      i = get_power_overcharge_level(player);
      directly_cast_spell_on_thing(plyr_idx, pckt->actn_par1, pckt->actn_par2, i);
      return 0;
  case PckA_PlyrToggleAlly:
      if (!is_player_ally_locked(plyr_idx, pckt->actn_par1))
      {
         toggle_ally_with_player(plyr_idx, pckt->actn_par1);
         if (game.conf.rules[plyr_idx].game.allies_share_vision)
         {
            panel_map_update(0, 0, game.map_subtiles_x+1, game.map_subtiles_y+1);
         }
      }
      return false;
  case PckA_SaveViewType:
      if (player->acamera != NULL)
        player->view_mode_restore = player->acamera->view_mode;
      set_player_mode(player, pckt->actn_par1);
      return false;
  case PckA_LoadViewType:
      set_player_mode(player, pckt->actn_par1);
      set_engine_view(player, player->view_mode_restore);
      return false;
  case PckA_SetRoomspaceAuto:
    {
        player->roomspace_detection_looseness = (unsigned char)pckt->actn_par1;
        player->roomspace_mode = roomspace_detection_mode;
        player->one_click_mode_exclusive = false;
        player->render_roomspace.highlight_mode = false;
        return false;
    }
   case PckA_SetRoomspaceMan:
    {
        player->user_defined_roomspace_width = pckt->actn_par1;
        player->roomspace_width = pckt->actn_par1;
        player->roomspace_height = pckt->actn_par1;
        player->roomspace_mode = box_placement_mode;
        player->one_click_mode_exclusive = false;
        player->render_roomspace.highlight_mode = false;
        player->roomspace_no_default = true;
        return false;
    }
    case PckA_SetRoomspaceDragPaint:
    {
        player->roomspace_height = 1;
        player->roomspace_width = 1;
    }
    // fall through
    case PckA_SetRoomspaceDrag:
    {
        player->roomspace_detection_looseness = DEFAULT_USER_ROOMSPACE_DETECTION_LOOSENESS;
        player->user_defined_roomspace_width = DEFAULT_USER_ROOMSPACE_WIDTH;
        player->roomspace_mode = drag_placement_mode;
        player->one_click_mode_exclusive = true; // Enable GuiLayer_OneClickBridgeBuild layer
        player->render_roomspace.highlight_mode = false;
        player->roomspace_no_default = false;
        player->roomspace_drag_paint_mode = (pckt->action == PckA_SetRoomspaceDragPaint);
        return false;
    }
    case PckA_SetRoomspaceDefault:
    {
        player->roomspace_detection_looseness = DEFAULT_USER_ROOMSPACE_DETECTION_LOOSENESS;
        player->user_defined_roomspace_width = DEFAULT_USER_ROOMSPACE_WIDTH;
        player->roomspace_width = player->roomspace_height = pckt->actn_par1;
        player->roomspace_mode = box_placement_mode;
        player->one_click_mode_exclusive = false;
        player->roomspace_no_default = false;
        return false;
    }
    case PckA_SetRoomspaceWholeRoom:
    {
        player->render_roomspace.highlight_mode = false;
        player->roomspace_mode = roomspace_detection_mode;
        return false;
    }
    case PckA_SetRoomspaceSubtile:
    {
        player->render_roomspace.highlight_mode = false;
        player->roomspace_mode = single_subtile_mode;
        return false;
    }
    case PckA_RoomspaceHighlightToggle:
    {
        if (is_my_player(player))
        {
            settings.highlight_mode = pckt->actn_par1;
            if (default_tag_mode == 3)
            {
                save_settings();
            }
        }
    }
    // fall through
    case PckA_SetRoomspaceHighlight:
    {
        player->roomspace_mode = pckt->actn_par1;
        if ( (pckt->actn_par2 == 1) || (pckt->actn_par1 == 2) )
        {
            // exit out of click and drag mode
            if (player->render_roomspace.drag_mode)
            {
                player->cursor_button_down = 0;
                player->one_click_lock_cursor = false;
                if ((pckt->control_flags & PCtr_LBtnHeld) == PCtr_LBtnHeld)
                {
                    player->ignore_next_PCtr_LBtnRelease = true;
                }
            }
            player->render_roomspace.drag_mode = false;
        }
        player->roomspace_highlight_mode = pckt->actn_par1;
        switch (pckt->actn_par1)
        {
            case 0:
            {
                reset_dungeon_build_room_ui_variables(plyr_idx);
                player->roomspace_width = player->roomspace_height = pckt->actn_par2;
                break;
            }
            case 1: // drag
            {
                if (pckt->actn_par2 == 1)
                {
                    player->roomspace_width = 1;
                    player->roomspace_height = 1;
                }
                break;
            }
        }
        player->roomspace_no_default = true;
        return false;
    }
    case PckA_PlyrQueryCreature:
    {
        query_creature(player, pckt->actn_par1, pckt->actn_par2, pckt->actn_par3);
        return false;
    }
    case PckA_SpriteZipCountSync:
    {
        process_sprite_zip_count_sync(plyr_idx, pckt->actn_par1);
        return true;
    }
    default:
      return process_players_global_cheats_packet_action(plyr_idx, pckt);
  }
}

void process_players_map_packet_control(long plyr_idx)
{
    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    // Get map coordinates
    process_map_packet_clicks(plyr_idx);
    player->cameras[CamIV_Parchment].mappos.x.val = pckt->pos_x;
    player->cameras[CamIV_Parchment].mappos.y.val = pckt->pos_y;
    set_mouse_light(player);
    SYNCDBG(8,"Finished");
}

void process_map_packet_clicks(long plyr_idx)
{
    SYNCDBG(7,"Starting");
    packet_left_button_double_clicked[plyr_idx] = 0;
    struct Packet* pckt = get_packet(plyr_idx);
    if ((pckt->control_flags & PCtr_Gui) == 0)
    {
        update_double_click_detection(plyr_idx);
    }
    SYNCDBG(8,"Finished");
}

/**
 * Process packet with input commands for given player.
 * @param plyr_idx Player to process packet for.
 */
void process_players_packet(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    if (is_packet_empty(pckt))
    {
        MULTIPLAYER_LOG("process_players_packet: Skipping empty packet for player %ld", plyr_idx);
        return;
    }
    SYNCDBG(6, "Processing player %ld packet of type %d.", plyr_idx, (int)pckt->action);
    player->input_crtr_control = ((pckt->additional_packet_values & PCAdV_CrtrContrlPressed) != 0);
    player->input_crtr_query = ((pckt->additional_packet_values & PCAdV_CrtrQueryPressed) != 0);
    if (((player->allocflags & PlaF_NewMPMessage) != 0) && (pckt->action == PckA_PlyrMsgChar) && (pckt->actn_par1 > 0))
    {
        message_text_key_add(player->mp_message_text, pckt->actn_par1, pckt->actn_par2);
  } else
  if (!process_players_global_packet_action(plyr_idx))
  {
      // Different changes to the game are possible for different views.
      // For each there can be a control change (which is view change or mouse event not translated to action),
      // and action perform (which does specific action set in packet).
      switch (player->view_type)
      {
          case PVT_DungeonTop:
            process_players_dungeon_control_packet_control(plyr_idx);
            process_players_dungeon_control_packet_action(plyr_idx);
            break;
          case PVT_CreatureContrl:
            process_players_creature_control_packet_control(plyr_idx);
            process_players_creature_control_packet_action(plyr_idx);
            break;
          case PVT_CreaturePasngr:
            //process_players_creature_passenger_packet_control(plyr_idx); -- there are no control changes in passenger mode
            process_players_creature_passenger_packet_action(plyr_idx);
            break;
          case PVT_MapScreen:
            process_players_map_packet_control(plyr_idx);
            //process_players_map_packet_action(plyr_idx); -- there are no actions to perform from map screen
            break;
          default:
            break;
      }
  }
  SYNCDBG(8,"Finished");
}

void process_players_creature_passenger_packet_action(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6,"Processing player %d action %d",(int)plyr_idx,(int)pckt->action);
    if (pckt->action == PckA_PasngrCtrlExit)
    {
        player->influenced_thing_idx = pckt->actn_par1;
        player->influenced_thing_creation = pckt->actn_par2;
        set_player_instance(player, PI_PsngrCtLeave, 0);
    }
    SYNCDBG(8,"Finished");
}

TbBool process_players_dungeon_control_packet_action(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6,"Processing player %d action %d",(int)plyr_idx,(int)pckt->action);
    switch (pckt->action)
    {
    case PckA_HoldAudience:
        magic_use_available_power_on_level(plyr_idx, PwrK_HOLDAUDNC, 0, PwMod_Default);
        break;
    case PckA_UseSpecialBox:
        activate_dungeon_special(thing_get(pckt->actn_par1), player);
        break;
    case PckA_ResurrectCrtr:
        resurrect_creature(thing_get(pckt->actn_par1), (pckt->actn_par2) & 0x0F, (pckt->actn_par2 >> 4) & 0xFF,
            (pckt->actn_par2 >> 12) & 0x0F);
        break;
    case PckA_TransferCreatr:
        transfer_creature(thing_get(pckt->actn_par1), thing_get(pckt->actn_par2), plyr_idx);
        break;
    case PckA_ToggleComputer:
        toggle_computer_player(plyr_idx);
        break;
    default:
        return process_players_dungeon_control_cheats_packet_action(plyr_idx, pckt);
    }
    return true;
}

void process_first_person_look(struct Thing *thing, struct Packet *pckt, long current_horizontal, long current_vertical, long *out_horizontal, long *out_vertical, long *out_roll)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    long maxTurnSpeed = crconf->max_turning_speed;
    if (maxTurnSpeed < 1) {
        maxTurnSpeed = 1;
    }
    long horizontalTurnSpeed = pckt->pos_x;
    if (horizontalTurnSpeed < -maxTurnSpeed) {
        horizontalTurnSpeed = -maxTurnSpeed;
    } else if (horizontalTurnSpeed > maxTurnSpeed) {
        horizontalTurnSpeed = maxTurnSpeed;
    }
    long verticalTurnSpeed = pckt->pos_y;
    if (verticalTurnSpeed < -maxTurnSpeed) {
        verticalTurnSpeed = -maxTurnSpeed;
    } else if (verticalTurnSpeed > maxTurnSpeed) {
        verticalTurnSpeed = maxTurnSpeed;
    }
    long verticalPos = (current_vertical + verticalTurnSpeed) & ANGLE_MASK;
    long lowerLimit = ANGLE_MASK - 227;
    long upperLimit = 227;
    if (verticalPos > upperLimit && verticalPos < lowerLimit) {
        if (abs(verticalPos - upperLimit) < abs(verticalPos - lowerLimit)) {
            verticalPos = upperLimit;
        } else {
            verticalPos = lowerLimit;
        }
    }
    *out_vertical = verticalPos;
    *out_horizontal = (current_horizontal + horizontalTurnSpeed) & ANGLE_MASK;
    *out_roll = 170 * horizontalTurnSpeed / maxTurnSpeed;
}

TbBool can_process_creature_input(struct Thing *thing)
{
    if (thing->class_id != TCls_Creature) {
        return false;
    }
    if (creature_is_dying(thing)) {
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->stateblock_flags != 0) || (thing->active_state == CrSt_CreatureUnconscious)) {
        return false;
    }
    return true;
}

void process_players_creature_control_packet_control(long idx)
{
    struct InstanceInfo *inst_inf;
    long i;

    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_player(idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    struct Thing* cctng = thing_get(player->controlled_thing_idx);
    if (!can_process_creature_input(cctng))
        return;
    struct CreatureControl* ccctrl = creature_control_get_from_thing(cctng);
    long speed_limit = get_creature_speed(cctng);
    if ((pckt->control_flags & PCtr_MoveUp) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->move_speed = compute_controlled_speed_increase(ccctrl->move_speed, speed_limit);
            ccctrl->creature_control_flags |= CCFlg_MoveY;
        } else
        {
            ERRORLOG("No creature to increase speed");
        }
    }
    if ((pckt->control_flags & PCtr_MoveDown) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->move_speed = compute_controlled_speed_decrease(ccctrl->move_speed, speed_limit);
            ccctrl->creature_control_flags |= CCFlg_MoveY;
        } else
        {
            ERRORLOG("No creature to decrease speed");
        }
    }
    if ((pckt->control_flags & PCtr_MoveLeft) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->orthogn_speed = compute_controlled_speed_increase(ccctrl->orthogn_speed, speed_limit);
            ccctrl->creature_control_flags |= CCFlg_MoveX;
        } else
        {
            ERRORLOG("No creature to increase speed");
        }
    }
    if ((pckt->control_flags & PCtr_MoveRight) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->orthogn_speed = compute_controlled_speed_decrease(ccctrl->orthogn_speed, speed_limit);
            ccctrl->creature_control_flags |= CCFlg_MoveX;
        } else
        {
            ERRORLOG("No creature to decrease speed");
        }
    }
    if (flag_is_set(cctng->movement_flags, TMvF_Flying))
    {
        MapCoord floor_height, ceiling_height;
        if ((pckt->control_flags & PCtr_Ascend) != 0)
        {
            if (!creature_control_invalid(ccctrl))
            {
                ccctrl->vertical_speed = compute_controlled_speed_increase(ccctrl->vertical_speed, speed_limit);
                ccctrl->creature_control_flags |= CCFlg_MoveZ;
                if (ccctrl->vertical_speed != 0)
                {
                    get_floor_and_ceiling_height_under_thing_at(cctng, &cctng->mappos, &floor_height, &ceiling_height);
                    if ( (cctng->mappos.z.val >= floor_height) && (cctng->mappos.z.val <= ceiling_height) )
                    {
                        ccctrl->moveaccel.z.val = distance_with_angle_to_coord_z(ccctrl->vertical_speed, 227);
                    }
                    else
                    {
                        ccctrl->moveaccel.z.val = 0;
                    }
                }
            } else
            {
                ERRORLOG("No creature to ascend");
            }
        }
        if ((pckt->control_flags & PCtr_Descend) != 0)
        {
            if (!creature_control_invalid(ccctrl))
            {
                // We want increase here, not decrease, because we don't want it angle-dependent
                ccctrl->vertical_speed = compute_controlled_speed_increase(ccctrl->vertical_speed, speed_limit);
                ccctrl->creature_control_flags |= CCFlg_MoveZ;
                if (ccctrl->vertical_speed != 0)
                {
                    get_floor_and_ceiling_height_under_thing_at(cctng, &cctng->mappos, &floor_height, &ceiling_height);
                    if ( (cctng->mappos.z.val >= floor_height) && (cctng->mappos.z.val <= ceiling_height) )
                    {
                        ccctrl->moveaccel.z.val = distance_with_angle_to_coord_z(ccctrl->vertical_speed, 1820);
                    }
                    else
                    {
                        ccctrl->moveaccel.z.val = 0;
                    }
                }
            } else
            {
                ERRORLOG("No creature to descend");
            }
        }
    }

    if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
    {
        i = ccctrl->active_instance_id;
        if (ccctrl->instance_id == CrInst_NULL)
        {
            if (creature_instance_is_available(cctng, i))
            {
                if (creature_instance_has_reset(cctng, i))
                {
                    if (!creature_under_spell_effect(cctng, CSAfF_Chicken))
                    {
                        inst_inf = creature_instance_info_get(i);
                        process_player_use_instance(cctng, i, pckt);
                    }
                }
            }
            else
            {
                // cheat mode
                inst_inf = creature_instance_info_get(i);
                process_player_use_instance(cctng, i, pckt);
            }
        }
    }
    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
        // Button is held down - check whether the instance has auto-repeat
        i = ccctrl->active_instance_id;
        inst_inf = creature_instance_info_get(i);
        if ((inst_inf->instance_property_flags & InstPF_RepeatTrigger) != 0)
        {
            if (ccctrl->instance_id == CrInst_NULL)
            {
                if (creature_instance_is_available(cctng, i))
                {
                    if (creature_instance_has_reset(cctng, i))
                    {
                        process_player_use_instance(cctng, i, pckt);
                    }
                }
                else
                {
                    // cheat mode
                    process_player_use_instance(cctng, i, pckt);
                }
            }
        }
    }

    long new_horizontal, new_vertical, new_roll;
    process_first_person_look(cctng, pckt, cctng->move_angle_xy, cctng->move_angle_z, &new_horizontal, &new_vertical, &new_roll);
    cctng->move_angle_xy = new_horizontal;
    cctng->move_angle_z = new_vertical;
    ccctrl->roll = new_roll;
}

void process_players_creature_control_packet_action(long plyr_idx)
{
  struct CreatureControl *cctrl;
  struct InstanceInfo *inst_inf;
  struct PlayerInfo *player;
  struct Thing *thing;
  struct Packet *pckt;
  long i;
  player = get_player(plyr_idx);
  pckt = get_packet_direct(player->packet_num);
  SYNCDBG(6,"Processing player %d action %d",(int)plyr_idx,(int)pckt->action);
  switch (pckt->action)
  {
  case PckA_DirectCtrlExit:
      player->influenced_thing_idx = pckt->actn_par1;
      player->influenced_thing_creation = pckt->actn_par2;
      thing = thing_get(player->controlled_thing_idx);
      cctrl = creature_control_get_from_thing(thing);
      struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
      if (!thing_is_invalid(dragtng))
      {
          creature_drop_dragged_object(thing, dragtng);
      }
      set_player_instance(player, PI_DirctCtLeave, 0);
      break;
  case PckA_CtrlCrtrSetInstnc:
      thing = thing_get(player->controlled_thing_idx);
      if (!thing_exists(thing))
        break;
      cctrl = creature_control_get_from_thing(thing);
      if (creature_control_invalid(cctrl))
        break;
      i = pckt->actn_par1;
      inst_inf = creature_instance_info_get(i);
      if (!inst_inf->instant || pckt->actn_par2)
      {
        cctrl->active_instance_id = i;
      } else
      if (cctrl->instance_id == CrInst_NULL)
      {
        if (creature_instance_is_available(thing,i) && creature_instance_has_reset(thing, pckt->actn_par1))
        {
          i = pckt->actn_par1;
          process_player_use_instance(thing, i, pckt);
          if (plyr_idx == my_player_number) {
              instant_instance_selected(i);
          }
        }
      }
      break;
  case PckA_CheatCtrlCrtrSetInstnc:
      thing = thing_get(player->controlled_thing_idx);
      if (!thing_exists(thing))
        break;
      cctrl = creature_control_get_from_thing(thing);
      if (creature_control_invalid(cctrl))
        break;
      i = pckt->actn_par1;
      // Cheat mode no need check any, just do/select it.
      cctrl->active_instance_id = i;
      break;
      case PckA_DirectCtrlDragDrop:
      {
         thing = thing_get(player->controlled_thing_idx);
         direct_control_pick_up_or_drop(plyr_idx, thing);
         break;
      }
    case PckA_SetFirstPersonDigMode:
    {
        player->first_person_dig_claim_mode = pckt->actn_par1;
        break;
    }
    case PckA_SwitchTeleportDest:
    {
        player->teleport_destination = pckt->actn_par1;
        break;
    }
    case PckA_SelectFPPickup:
    {
        player->selected_fp_thing_pickup = pckt->actn_par1;
        break;
    }
    case PckA_SetNearestTeleport:
    {
        player->nearest_teleport = pckt->actn_par1;
        break;
    }
  }
}

static void replace_with_ai(int old_active_players)
{
    int k = 0;
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        if (network_player_active(i))
            k++;
    }
    if (old_active_players != k)
    {
        for (int i = 0; i < NET_PLAYERS_COUNT; i++)
        {
            struct PlayerInfo *player = get_player(i);
            if (!network_player_active(player->packet_num))
            {
                message_add(MsgType_Player, player->id_number, "I am the computer now!");
                JUSTLOG("p:%d I am the computer now!", player->id_number);

                player->allocflags |= PlaF_CompCtrl;
                toggle_computer_player(i);
            }
        }
    }
}

static void load_old_packets(PlayerNumber my_packet_num) {
    GameTurn historical_turn = game.play_gameturn - game.input_lag_turns;
    const struct Packet* received_packets = get_received_packets_for_turn(historical_turn);
    const char* received_packets_status;
    if (received_packets != NULL) {
        received_packets_status = "found";
    } else {
        received_packets_status = "NULL";
    }
    MULTIPLAYER_LOG("load_input_lag_packets: current_turn=%lu historical_turn=%lu received_packets=%s", (unsigned long)game.play_gameturn, (unsigned long)historical_turn, received_packets_status);

    for (int i = 0; i < PACKETS_COUNT; i++) {
        const char* player_name;
        if (i == 0) {player_name = "Host";} else {player_name = "Client";}

        if (i == my_packet_num) {
            struct Packet* local_packet = get_local_input_lag_packet_for_turn(historical_turn);
            if (local_packet != NULL) {
                game.packets[i] = *local_packet;
            } else {
                MULTIPLAYER_LOG("load_input_lag_packets: NOT FOUND - no local packet for historical_turn=%lu", (unsigned long)historical_turn);
            }
            if (i <= 1) {
                if (is_packet_empty(&game.packets[i])) {
                    MULTIPLAYER_LOG("load_input_lag_packets: loaded local packet[%s] is EMPTY", player_name);
                } else {
                    MULTIPLAYER_LOG("load_input_lag_packets: loaded local packet[%s] turn=%lu checksum=%08lx", player_name, (unsigned long)game.packets[i].turn, (unsigned long)game.packets[i].checksum);
                }
            }
        } else {
            if (received_packets != NULL) {
                game.packets[i] = received_packets[i];
                if (i <= 1) {
                    if (is_packet_empty(&game.packets[i])) {
                        MULTIPLAYER_LOG("load_input_lag_packets: loaded packet[%s] is EMPTY", player_name);
                    } else {
                        MULTIPLAYER_LOG("load_input_lag_packets: loaded packet[%s] turn=%lu checksum=%08lx", player_name, (unsigned long)game.packets[i].turn, (unsigned long)game.packets[i].checksum);
                    }
                }
            } else {
                memset(&game.packets[i], 0, sizeof(struct Packet));
                if (i <= 1) {
                    MULTIPLAYER_LOG("load_input_lag_packets: cleared packet[%s] (no received packets)", player_name);
                }
            }
        }
    }
}

void set_local_packet_turn(void) {
    struct Packet* pckt = get_packet(my_player_number);
    pckt->turn = game.play_gameturn;
    MULTIPLAYER_LOG("set_local_packet_turn: turn=%lu checksum=%08lx", (unsigned long)game.play_gameturn, (unsigned long)pckt->checksum);
}

void check_scheduled_unpause(void) {
    if (scheduled_unpause_time > 0 && LbTimerClock() >= scheduled_unpause_time) {
        MULTIPLAYER_LOG("process_packets: Executing scheduled unpause at time=%u", LbTimerClock());
        scheduled_unpause_time = 0;
        process_pause_packet(0, 0);
    }
}

/**
 * Exchange packets if MP game, then process all packets influencing local game state.
 */
void process_packets(void)
{
    int i;
    struct PlayerInfo* player = get_my_player();
    SYNCDBG(5, "Starting");

    MULTIPLAYER_LOG("process_packets: === BEGIN turn=%lu ===", (unsigned long)game.play_gameturn);
    set_local_packet_turn();
    update_turn_checksums();
    store_local_packet_in_input_lag_queue(player->packet_num);

    check_scheduled_unpause();

    if (game.game_kind != GKind_LocalGame)
    {
        int old_active_players = 0;
        for (i = 0; i < NET_PLAYERS_COUNT; i++)
        {
            if (network_player_active(i))
                old_active_players++;
        }
        MULTIPLAYER_LOG("process_packets: About to send/receive packets, active_players=%d", old_active_players);

        if (!game.packet_load_enable || game.packet_load_initialized)
        {
            struct Packet* my_pckt = get_packet_direct(player->packet_num);
            const char* player_name;
            if (player->packet_num == 0) {player_name = "Host";} else {player_name = "Client";}
            MULTIPLAYER_LOG("process_packets: SENDING packet[%s] turn=%lu checksum=%08lx", player_name, (unsigned long)my_pckt->turn, (unsigned long)my_pckt->checksum);
            TbError exchange_result = LbNetwork_Exchange(NETMSG_GAMEPLAY, my_pckt, game.packets, sizeof(struct Packet));
            if (exchange_result != Lb_OK) {
                ERRORLOG("LbNetwork_Exchange failed");
            }
            LbNetwork_WaitForMissingPackets(game.packets, sizeof(struct Packet));
        }
        replace_with_ai(old_active_players);
    }

    MULTIPLAYER_LOG("process_packets: Loading packets from input lag queue");
    load_old_packets(player->packet_num);

    if (input_lag_skips_initial_processing())
    {
        clear_packets();
        return;
    }

    if (checksums_different()) { //Should be called directly after LbNetwork_Exchange, to see if there's anything wrong with the received packet
        // Setting checksum problem flags
        set_flag(game.system_flags, GSF_NetGameNoSync);
        clear_flag(game.system_flags, GSF_NetSeedNoSync);
    } else {
        clear_flag(game.system_flags, GSF_NetGameNoSync);
        clear_flag(game.system_flags, GSF_NetSeedNoSync);
    }

    // Write packets into file, if requested
    if ((game.packet_save_enable) && (game.packet_fopened))
        save_packets();
    //Debug code, to find packet errors
    #if DEBUG_NETWORK_PACKETS
    write_debug_packets();
    #endif
    // Process the packets
    for (i=0; i<PACKETS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
        process_players_packet(i);
    }
    // Clear all packets
    clear_packets();
    if (((game.system_flags & GSF_NetGameNoSync) != 0)
    || ((game.system_flags & GSF_NetSeedNoSync) != 0))
    {
        SYNCDBG(0,"Resyncing");
        resync_game();
    }
    get_current_slowdown_percentage();
    MULTIPLAYER_LOG("process_packets: === END turn=%lu ===", (unsigned long)game.play_gameturn);
    SYNCDBG(7,"Finished");
}

static TbBool try_starting_level_from_chat(char* message, long player_id)
{
    char *separator_pos = strchr(message, ':');
    if (!separator_pos) {
        separator_pos = strchr(message, ' ');
    }
    if (!separator_pos || separator_pos == message) {
        return false;
    }

    int campaign_len = separator_pos - message;
    if (campaign_len <= 0 || campaign_len >= 64) {
        return false;
    }

    char *level_str = separator_pos + 1;
    if (!isdigit(level_str[0])) {
        return false;
    }

    LevelNumber level_num = atoi(level_str);
    if (level_num <= 0) {
        return false;
    }

    char campaign_filename[80];
    snprintf(campaign_filename, sizeof(campaign_filename), "%.*s.cfg", campaign_len, message);

    if (!change_campaign(campaign_filename)) {
        ERRORLOG("Unable to load campaign '%.*s' for level %d", campaign_len, message, (int)level_num);
        return false;
    }

    set_selected_level_number(level_num);
    frontend_set_state(FeSt_START_MPLEVEL);
    return true;
}

static void handle_chat_message(char* message, long player_id, TbBool clear_text, char* text_to_clear)
{
    if (try_starting_level_from_chat(message, player_id)) {
        if (clear_text) {
            text_to_clear[0] = '\0';
        }
        return;
    }

    add_message(player_id, message);
    if (clear_text) {
        text_to_clear[0] = '\0';
    }
}

void process_frontend_packets(void)
{
  long i;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    net_screen_packet[i].networkstatus_flags &= ~0x01;
  }
  struct ScreenPacket* nspckt = &net_screen_packet[my_player_number];
  set_flag(nspckt->networkstatus_flags, 0x01);
  nspckt->frontend_alliances = frontend_alliances;
  set_flag(nspckt->networkstatus_flags, 0x01);
  nspckt->networkstatus_flags ^= ((nspckt->networkstatus_flags ^ (fe_computer_players << 1)) & 0x06);
  nspckt->stored_data1 = VersionRelease;
  nspckt->stored_data2 = VersionBuild;
  if (LbNetwork_Exchange(NETMSG_FRONTEND, nspckt, &net_screen_packet, sizeof(struct ScreenPacket)))
  {
      ERRORLOG("LbNetwork_Exchange failed");
      net_service_index_selected = -1;
  }
  if (frontend_should_all_players_quit())
  {
    i = frontnet_number_of_players_in_session();
    if (players_currently_in_session < i)
    {
      players_currently_in_session = i;
    }
    if (players_currently_in_session > i)
    {
      if (frontend_menu_state == FeSt_NET_SESSION)
      {
          if (LbNetwork_Stop())
          {
            ERRORLOG("LbNetwork_Stop() failed");
            return;
          }
          frontend_set_state(FeSt_MAIN_MENU);
      } else if (frontend_menu_state == FeSt_NET_START)
      {
          if (LbNetwork_Stop())
          {
            ERRORLOG("LbNetwork_Stop() failed");
            return;
          }
          if (setup_network_service(net_service_index_selected))
          {
            frontend_set_state(FeSt_NET_SESSION);
          }
          else
          {
            frontend_set_state(FeSt_MAIN_MENU);
          }
      }
    }
  }
#if DEBUG_NETWORK_PACKETS
  write_debug_screenpackets();
#endif
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    nspckt = &net_screen_packet[i];
    struct PlayerInfo* player = get_player(i);
    if ((nspckt->networkstatus_flags & 0x01) != 0)
    {
        long k;
        switch (nspckt->networkstatus_flags >> 3)
        {
        case 2:
            handle_chat_message((char*)&nspckt->param1, i, false, NULL);
            break;
        case 3:
            if (!validate_versions())
            {
                versions_different_error();
                break;
            }
            fe_network_active = 1;
            if (game_flags2 & GF2_Connect)
            {
                frontend_set_state(FeSt_START_MPLEVEL);
            }
            else
            {
                frontend_set_state(FeSt_NETLAND_VIEW);
            }
            break;
        case 4:
            frontend_set_alliance(nspckt->param1, nspckt->param2);
            break;
        case 7:
            fe_computer_players = nspckt->param1;
            break;
        case 8:
        {
            k = strlen(player->mp_message_text);
            unsigned short c;
            if (nspckt->param1 == KC_BACK)
            {
                if (k > 0)
                {
                    k--;
                    player->mp_message_text[k] = '\0';
                }
            }
            else if (nspckt->param1 == KC_RETURN)
            {
                if (k > 0)
                {
                    handle_chat_message(player->mp_message_text, i, true, player->mp_message_text);
                }
            }
            else
            {
                c = key_to_ascii(nspckt->param1, nspckt->param2);
                if ((c != 0) && (frontend_font_char_width(1, c) > 1) && (k < 62))
                {
                    player->mp_message_text[k] = c;
                    k++;
                    player->mp_message_text[k] = '\0';
                }
            }
            if (frontend_font_string_width(1, player->mp_message_text) >= 420)
            {
                if (k > 0)
                {
                    k--;
                    player->mp_message_text[k] = '\0';
                }
            }
            break;
        }
      default:
        break;
      }
      if (frontend_alliances == -1)
      {
        if (nspckt->frontend_alliances != -1)
          frontend_alliances = nspckt->frontend_alliances;
      }
      if (fe_computer_players == 2)
      {
        k = ((nspckt->networkstatus_flags & 0x06) >> 1);
        if (k != 2)
          fe_computer_players = k;
      }
      player->game_version = nspckt->stored_data2 + (nspckt->stored_data1 << 8);
    }
    nspckt->networkstatus_flags &= 0x07;
  }
  if (frontend_alliances == -1)
    frontend_alliances = 0;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    nspckt = &net_screen_packet[i];
    if ((nspckt->networkstatus_flags & 0x01) == 0)
    {
      if (frontend_is_player_allied(my_player_number, i))
        frontend_set_alliance(my_player_number, i);
    }
  }
}

void apply_default_flee_and_imprison_setting(void)
{
    struct PlayerInfo* player = get_my_player();
    if (!player_exists(player) || game.packet_load_enable) {
        return;
    }

    struct Dungeon* dungeon = get_dungeon(player->id_number);
    unsigned short tendencies_to_toggle = 0;

    TbBool current_imprison_state = (dungeon->creature_tendencies & CrTend_Imprison) != 0;
    if (IMPRISON_BUTTON_DEFAULT != current_imprison_state) {
        tendencies_to_toggle |= CrTend_Imprison;
    }

    TbBool current_flee_state = (dungeon->creature_tendencies & CrTend_Flee) != 0;
    if (FLEE_BUTTON_DEFAULT != current_flee_state) {
        tendencies_to_toggle |= CrTend_Flee;
    }

    if (tendencies_to_toggle) {
        set_players_packet_action(player, PckA_ToggleTendency, tendencies_to_toggle, 0, 0, 0);
    }
}

// Using Alt-F4, or similar operating system close requests
void force_application_close()
{
    extern unsigned char exit_keeper;
    extern int frontend_menu_state;

    // Check if we're in gameplay vs frontend
    if (frontend_menu_state == 0)
    {
        struct PlayerInfo* player = get_my_player();
        if (player != INVALID_PLAYER)
        {
            set_players_packet_action(player, PckA_ForceApplicationClose, 0, 0, 0, 0);
        }
        else
        {
            exit_keeper = 1;
        }
    }
    else
    {
        // We're in the frontend, just exit directly
        exit_keeper = 1;
    }
}

void send_sprite_zip_count_to_other_players(void)
{
    struct PlayerInfo* my_player = get_my_player();
    if (my_player != INVALID_PLAYER)
    {
        set_players_packet_action(my_player, PckA_SpriteZipCountSync, total_sprite_zip_count, 0, 0, 0);
    }
}

void process_sprite_zip_count_sync(long plyr_idx, long zip_count)
{
    if (zip_count != total_sprite_zip_count)
    {
        if (my_player_number == get_host_player_id())
        {
            message_add_fmt(MsgType_Player, 0,
                "Verify /fxdata/ is the same across both PCs.");
            message_add_fmt(MsgType_Player, 0,
                "%s has %ld .zip files, %s has %d .zip files.",
                network_player_name(plyr_idx), zip_count,
                network_player_name(my_player_number), total_sprite_zip_count);
            message_add_fmt(MsgType_Player, 0,
                "WARNING: Custom sprite files mismatch detected!");
        }
    }
}

/******************************************************************************/
