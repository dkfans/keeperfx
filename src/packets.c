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
#include "packets.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_keybrd.h"
#include "bflib_vidraw.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_network.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_planar.h"

#include "kjm_input.h"
#include "front_input.h"
#include "front_simple.h"
#include "front_landview.h"
#include "front_network.h"
#include "frontmenu_net.h"
#include "frontend.h"
#include "vidmode.h"
#include "config.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_effects.h"
#include "config_terrain.h"
#include "config_players.h"
#include "config_settings.h"
#include "player_instances.h"
#include "player_data.h"
#include "player_states.h"
#include "player_utils.h"
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
#include "magic.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "light_data.h"
#include "gui_draw.h"
#include "gui_topmsg.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "gui_parchment.h"
#include "net_game.h"
#include "net_sync.h"
#include "game_legacy.h"
#include "engine_redraw.h"
#include "frontmenu_ingame_tabs.h"
#include "vidfade.h"
#include "spdigger_stack.h"

#include "keeperfx.hpp"

#include "music_player.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/******************************************************************************/
void set_packet_action(struct Packet *pckt, unsigned char pcktype, unsigned short par1, unsigned short par2, unsigned short par3, unsigned short par4)
{
    pckt->actn_par1 = par1;
    pckt->actn_par2 = par2;
    pckt->action = pcktype;
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
    if (packet_left_button_click_space_count[plyr_idx] < LONG_MAX)
      packet_left_button_click_space_count[plyr_idx]++;
  }
}

struct Room *keeper_build_room(long stl_x,long stl_y,long plyr_idx,long rkind)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct RoomStats* rstat = room_stats_get_for_kind(rkind);
    // Take top left subtile on single subtile boundbox, take center subtile on full slab boundbox
    MapCoord x = ((player->full_slab_cursor == 0) ? slab_subtile(subtile_slab_fast(stl_x), 0) : slab_subtile_center(subtile_slab_fast(stl_x)));
    MapCoord y = ((player->full_slab_cursor == 0) ? slab_subtile(subtile_slab_fast(stl_y), 0) : slab_subtile_center(subtile_slab_fast(stl_y)));
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
        set_coords_to_slab_center(&pos, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
        create_price_effect(&pos, plyr_idx, rstat->cost);
    }
    return room;
}

TbBool process_dungeon_control_packet_spell_overcharge(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    SYNCDBG(6,"Starting for player %d state %s",(int)plyr_idx,player_state_code_name(player->work_state));
    struct Packet* pckt = get_packet_direct(player->packet_num);
    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
      switch (player->work_state)
      {
      case PSt_CallToArms:
          if (player_uses_power_call_to_arms(plyr_idx))
            player->cast_expand_level = (dungeon->cta_splevel << 2);
          else
            update_power_overcharge(player, PwrK_CALL2ARMS);
          break;
      case PSt_CaveIn:
          update_power_overcharge(player, PwrK_CAVEIN);
          break;
      case PSt_SightOfEvil:
          update_power_overcharge(player, PwrK_SIGHT);
          break;
      case PSt_Lightning:
          update_power_overcharge(player, PwrK_LIGHTNING);
          break;
      case PSt_SpeedUp:
          update_power_overcharge(player, PwrK_SPEEDCRTR);
          break;
      case PSt_Armour:
          update_power_overcharge(player, PwrK_PROTECT);
          break;
      case PSt_Conceal:
          update_power_overcharge(player, PwrK_CONCEAL);
          break;
      case PSt_Heal:
          update_power_overcharge(player, PwrK_HEALCRTR);
          break;
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
    struct RoomStats* rstat = room_stats_get_for_room(room);
    long revenue = compute_value_percentage(rstat->cost, gameadd.room_sale_percent);
    if (room->owner != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_players_num_dungeon(room->owner);
        dungeon->rooms_destroyed++;
        dungeon->camera_deviate_jump = 192;
    }
    delete_room_slab(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y), 0);
    if (is_my_player_number(plyr_idx))
        play_non_3d_sample(115);
    if (revenue != 0)
    {
        struct Coord3d pos;
        set_coords_to_slab_center(&pos, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
        create_price_effect(&pos, plyr_idx, revenue);
        player_add_offmap_gold(plyr_idx, revenue);
    }
    return true;
}

TbBigChecksum get_thing_simple_checksum(const struct Thing *tng)
{
    return (ulong)tng->mappos.x.val + (ulong)tng->mappos.y.val + (ulong)tng->mappos.z.val
         + (ulong)tng->move_angle_xy + (ulong)tng->owner;
}

  TbBigChecksum get_packet_save_checksum(void)
  {
      TbBigChecksum sum = 0;
      for (long tng_idx = 0; tng_idx < THINGS_COUNT; tng_idx++)
      {
          struct Thing* tng = thing_get(tng_idx);
          if ((tng->alloc_flags & TAlF_Exists) != 0)
          {
              // It would be nice to completely ignore effects, but since
              // thing indices are used in packets, lack of effect may cause desync too.
              if ((tng->class_id != TCls_AmbientSnd) && (tng->class_id != TCls_EffectElem))
              {
                  sum += get_thing_simple_checksum(tng);
              }
          }
      }
      return sum;
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
      set_flag_byte(&game.operation_flags, GOF_Paused, curr_pause);
      if ((game.operation_flags & GOF_Paused) != 0)
          set_flag_byte(&game.operation_flags, GOF_WorldInfluence, new_pause);
      else
          set_flag_byte(&game.operation_flags, GOF_Paused, false);
      if ( !SoundDisabled )
      {
        if ((game.operation_flags & GOF_Paused) != 0)
        {
          SetSoundMasterVolume(settings.sound_volume >> 1);
          SetMusicPlayerVolume(settings.redbook_volume >> 1);
          SetMusicMasterVolume(settings.sound_volume >> 1);
        } else
        {
          SetSoundMasterVolume(settings.sound_volume);
          SetMusicPlayerVolume(settings.redbook_volume);
          SetMusicMasterVolume(settings.sound_volume);
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

void process_players_dungeon_control_packet_control(long plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6,"Processing player %d action %d",(int)plyr_idx,(int)pckt->action);
    struct Camera* cam = player->acamera;
    long inter_val;
    switch (cam->view_mode)
    {
    case PVM_IsometricView:
        inter_val = 2560000 / cam->zoom;
        break;
    case PVM_FrontView:
        inter_val = 12800000 / cam->zoom;
        break;
    default:
        inter_val = 256;
        break;
    }
    if (pckt->additional_packet_values & PCAdV_SpeedupPressed)
      inter_val *= 3;

    if ((pckt->control_flags & PCtr_MoveUp) != 0)
        view_set_camera_y_inertia(cam, -inter_val/4, -inter_val);
    if ((pckt->control_flags & PCtr_MoveDown) != 0)
        view_set_camera_y_inertia(cam, inter_val/4, inter_val);
    if ((pckt->control_flags & PCtr_MoveLeft) != 0)
        view_set_camera_x_inertia(cam, -inter_val/4, -inter_val);
    if ((pckt->control_flags & PCtr_MoveRight) != 0)
        view_set_camera_x_inertia(cam, inter_val/4, inter_val);
    if ((pckt->control_flags & PCtr_ViewRotateCCW) != 0)
    {
        switch (cam->view_mode)
        {
        case PVM_IsometricView:
             view_set_camera_rotation_inertia(cam, 16, 64);
            break;
        case PVM_FrontView:
            cam->orient_a = (cam->orient_a + LbFPMath_PI/2) & LbFPMath_AngleMask;
            break;
        }
    }
    if ((pckt->control_flags & PCtr_ViewRotateCW) != 0)
    {
        switch (cam->view_mode)
        {
        case PVM_IsometricView:
            view_set_camera_rotation_inertia(cam, -16, -64);
            break;
        case PVM_FrontView:
            cam->orient_a = (cam->orient_a - LbFPMath_PI/2) & LbFPMath_AngleMask;
            break;
        }
    }
    unsigned long zoom_min = adjust_min_camera_zoom(cam, game.operation_flags & GOF_ShowGui); // = CAMERA_ZOOM_MIN (+300 if gui is hidden in Isometric view)
    unsigned long zoom_max = CAMERA_ZOOM_MAX;
    if (pckt->control_flags & PCtr_ViewZoomIn)
    {
        switch (cam->view_mode)
        {
        case PVM_IsometricView:
            view_zoom_camera_in(cam, zoom_max, zoom_min);
            update_camera_zoom_bounds(cam, zoom_max, zoom_min);
            if (is_my_player(player))
            {
                settings.isometric_view_zoom_level = cam->zoom;
                save_settings();
            }
            break;
        default:
            view_zoom_camera_in(cam, zoom_max, zoom_min);
            break;
        }
        if (is_my_player(player))
        {
            settings.frontview_zoom_level = cam->zoom;
            save_settings();
        }
    }
    if (pckt->control_flags & PCtr_ViewZoomOut)
    {
        switch (cam->view_mode)
        {
        case PVM_IsometricView:
            view_zoom_camera_out(cam, zoom_max, zoom_min);
            update_camera_zoom_bounds(cam, zoom_max, zoom_min);
            if (is_my_player(player))
            {
                settings.isometric_view_zoom_level = cam->zoom;
                save_settings();
            }
            break;
        default:
            view_zoom_camera_out(cam, zoom_max, zoom_min);
            if (is_my_player(player))
            {
                settings.frontview_zoom_level = cam->zoom;
                save_settings();
            }
            break;
        }
    }
    process_dungeon_control_packet_clicks(plyr_idx);
    set_mouse_light(player);
}

/**
 * Modifies a message string according to a key pressed.
 * @param message The message buffer.
 * @param maxlen Max length of the string (message buffer size - 1).
 * @param key The key which will affect the message.
 * @param kmodif Modifier keys pressed with the key.
 * @return Gives true if the message was modified by that key, false if it stayed without change.
 * @note We shouldn't use regional settings in this function, otherwise players playing different language
 * versions may get different messages from each other.
 */
TbBool message_text_key_add(char * message, long maxlen, TbKeyCode key, TbKeyMods kmodif)
{
    char chr = key_to_ascii(key, kmodif);
    int chpos = strlen(message);
    if (key == KC_BACK)
    {
      if (chpos>0) {
          message[chpos-1] = '\0';
          return true;
      }
    } else
    if (((chr >= 'a') && (chr <= 'z')) ||
        ((chr >= 'A') && (chr <= 'Z')) ||
        ((chr >= '0') && (chr <= '9')) ||
        (chr == ' ')  || (chr == '!') || (chr == ':') || (chr == ';')
        || (chr == '(') || (chr == ')') || (chr == '.') || (chr == '_') 
        || (chr == '\'') || (chr == '+') || (chr == '=') || (chr == '-')
        || (chr == '"') || (chr == '?') || (chr == '/') || (chr == '#')
        || (chr == '<') || (chr == '>') || (chr == '^'))
    {
        if (chpos < maxlen)
        {
            message[chpos] = chr;
            message[chpos+1] = '\0';
            return true;
        }
    }
    return false;
}

void process_players_message_character(struct PlayerInfo *player)
{
    struct Packet* pcktd = get_packet(player->id_number);
    if (pcktd->actn_par1 > 0)
    {
        message_text_key_add(player->mp_message_text, PLAYER_MP_MESSAGE_LEN, pcktd->actn_par1, pcktd->actn_par2);
    }
}

void process_quit_packet(struct PlayerInfo *player, short complete_quit)
{
    struct PlayerInfo *swplyr;
    struct PlayerInfo* myplyr = get_my_player();
    long plyr_count;
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
            swplyr->flgfield_6 |= PlaF6_PlyrHasQuit;
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
  case PckA_Unknown001:
      if (is_my_player(player))
      {
        turn_off_all_menus();
        frontend_save_continue_game(true);
        free_swipe_graphic();
      }
      player->flgfield_6 |= PlaF6_PlyrHasQuit;
      process_quit_packet(player, 0);
      return 1;
  case PckA_Unknown003:
      if (is_my_player(player))
      {
        turn_off_all_menus();
        frontend_save_continue_game(true);
      }
      player->flgfield_6 |= PlaF6_PlyrHasQuit;
      process_quit_packet(player, 1);
      return 1;
  case PckA_Unknown004:
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
  case PckA_PlyrMsgBegin:
      player->allocflags |= PlaF_NewMPMessage;
      return 0;
  case PckA_PlyrMsgEnd:
      player->allocflags &= ~PlaF_NewMPMessage;
      if (player->mp_message_text[0] == '!')
      {
          if (!cmd_exec(player->id_number, player->mp_message_text))
              message_add(player->id_number, player->mp_message_text);
      }
      else if (player->mp_message_text[0] != '\0')
          message_add(player->id_number, player->mp_message_text);
      LbMemorySet(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
      return 0;
  case PckA_PlyrMsgClear:
      player->allocflags &= ~PlaF_NewMPMessage;
      LbMemorySet(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
      return 0;
  case PckA_ToggleLights:
      if (is_my_player(player))
      {
          light_set_lights_on(game.lish.field_4614D == 0);
      }
      return 1;
  case PckA_SwitchScrnRes:
      if (is_my_player(player))
      {
          switch_to_next_video_mode();
      }
      return 1;
  case PckA_TogglePause:
      process_pause_packet((game.operation_flags & GOF_Paused) == 0, pckt->actn_par1);
      return 1;
  case PckA_SetCluedo:
      player->video_cluedo_mode = pckt->actn_par1;
      if (is_my_player(player))
      {
        settings.video_cluedo_mode = player->video_cluedo_mode;
        save_settings();
      }
      return 0;
  case PckA_Unknown025:
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
      player->cameras[CamIV_Parchment].orient_a = pckt->actn_par1;
      player->cameras[CamIV_FrontView].orient_a = pckt->actn_par1;
      player->cameras[CamIV_Isometric].orient_a = pckt->actn_par1;
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
          game.creatures_tend_imprison = ((dungeon->creature_tendencies & 0x01) != 0);
          game.creatures_tend_flee = ((dungeon->creature_tendencies & 0x02) != 0);
      }
      return 0;
  case PckA_CheatEnter:
//      game.???[my_player_number].cheat_mode = 1;
      show_onscreen_msg(2*game.num_fps, "Cheat mode activated by player %d", plyr_idx);
      return 1;
  case PckA_CheatAllFree:
      make_all_creatures_free();
      make_all_rooms_free();
      make_all_powers_cost_free();
      return 1;
  case PckA_CheatCrtSpells:
      //TODO: remake from beta
      return 0;
  case PckA_CheatRevealMap:
  {
      struct PlayerInfo* myplyr = get_my_player();
      reveal_whole_map(myplyr);
      return 0;
  }
  case PckA_CheatCrAllSpls:
      //TODO: remake from beta
      return 0;
  case PckA_Unknown065:
      //TODO: remake from beta
      return 0;
  case PckA_CheatAllMagic:
      make_available_all_researchable_powers(my_player_number);
      return 0;
  case PckA_CheatAllRooms:
      make_available_all_researchable_rooms(my_player_number);
      return 0;
  case PckA_Unknown068:
      //TODO: remake from beta
      return 0;
  case PckA_Unknown069:
      //TODO: remake from beta
      return 0;
  case PckA_CheatAllResrchbl:
      make_all_powers_researchable(my_player_number);
      make_all_rooms_researchable(my_player_number);
      return 0;
  case PckA_SetViewType:
      set_player_mode(player, pckt->actn_par1);
      return 0;
  case PckA_ZoomFromMap:
      set_player_cameras_position(player, subtile_coord_center(pckt->actn_par1), subtile_coord_center(pckt->actn_par2));
      player->cameras[CamIV_Parchment].orient_a = 0;
      player->cameras[CamIV_FrontView].orient_a = 0;
      player->cameras[CamIV_Isometric].orient_a = 0;
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
  case PckA_Unknown083:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      event_move_player_towards_event(player, pckt->actn_par1);
      return 0;
  case PckA_ZoomToRoom:
  {
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
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      player->zoom_to_pos_x = pckt->actn_par1;
      player->zoom_to_pos_y = pckt->actn_par2;
      set_player_instance(player, PI_ZoomToPos, 0);
      return 0;
  case PckA_Unknown088:
      game.numfield_D ^= (game.numfield_D ^ (GNFldD_Unkn04 * ((game.numfield_D & GNFldD_Unkn04) == 0))) & GNFldD_Unkn04;
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
  case PckA_Unknown092:
      if (game.event[pckt->actn_par1].kind == 3)
      {
        turn_off_event_box_if_necessary(plyr_idx, pckt->actn_par1);
      } else
      {
        event_delete_event(plyr_idx, pckt->actn_par1);
      }
      return 0;
  case PckA_UsePwrObey:
      magic_use_available_power_on_level(plyr_idx, PwrK_OBEY, 0, PwMod_Default);
      return 0;
  case PckA_UsePwrArmageddon:
      magic_use_available_power_on_level(plyr_idx, PwrK_ARMAGEDDON, 0, PwMod_Default);
      return 0;
  case PckA_Unknown099:
      turn_off_query(plyr_idx);
      return 0;
  case PckA_Unknown104:
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
            set_player_state(player, powerst->work_state, 0);
      }
      return 0;
  case PckA_PlyrFastMsg:
      //show_onscreen_msg(game.num_fps, "Message from player %d", plyr_idx);
      output_message(SMsg_EnemyHarassments+pckt->actn_par1, 0, true);
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
      toggle_ally_with_player(plyr_idx, pckt->actn_par1);
      return 0;
  case PckA_SaveViewType:
      if (player->acamera != NULL)
        player->view_mode_restore = player->acamera->view_mode;
      set_player_mode(player, pckt->actn_par1);
      return false;
  case PckA_LoadViewType:
      set_player_mode(player, pckt->actn_par1);
      set_engine_view(player, player->view_mode_restore);
      return false;
    default:
      return false;
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
    if ((pckt->control_flags & PCtr_Unknown4000) == 0)
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
    SYNCDBG(6, "Processing player %d packet of type %d.", plyr_idx, (int)pckt->action);
    player->input_crtr_control = ((pckt->additional_packet_values & PCAdV_CrtrContrlPressed) != 0);
    player->input_crtr_query = ((pckt->additional_packet_values & PCAdV_CrtrQueryPressed) != 0);
    if (((player->allocflags & PlaF_NewMPMessage) != 0) && (pckt->action == PckA_PlyrMsgChar))
    {
        process_players_message_character(player);
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
        return false;
    }
    return true;
}

void process_players_creature_control_packet_control(long idx)
{
    struct InstanceInfo *inst_inf;
    long i;
    long n;

    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_player(idx);
    struct Packet* pckt = get_packet_direct(player->packet_num);
    struct Thing* cctng = thing_get(player->controlled_thing_idx);
    if (cctng->class_id != TCls_Creature)
        return;
    struct CreatureControl* ccctrl = creature_control_get_from_thing(cctng);
    if (creature_is_dying(cctng))
        return;
    if ((ccctrl->stateblock_flags != 0) || (cctng->active_state == CrSt_CreatureUnconscious))
        return;
    long speed_limit = get_creature_speed(cctng);
    if ((pckt->control_flags & PCtr_MoveUp) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->move_speed = compute_controlled_speed_increase(ccctrl->move_speed, speed_limit);
            ccctrl->flgfield_1 |= CCFlg_Unknown40;
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
            ccctrl->flgfield_1 |= CCFlg_Unknown40;
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
            ccctrl->flgfield_1 |= CCFlg_Unknown80;
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
            ccctrl->flgfield_1 |= CCFlg_Unknown80;
        } else
        {
            ERRORLOG("No creature to decrease speed");
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
                    if (!creature_affected_by_spell(cctng, SplK_Chicken))
                    {
                        inst_inf = creature_instance_info_get(i);
                        n = get_human_controlled_creature_target(cctng, inst_inf->field_1D);
                        set_creature_instance(cctng, i, 1, n, 0);
                    }
                }
            }
        }
    }
    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
        // Button is held down - check whether the instance has auto-repeat
        i = ccctrl->active_instance_id;
        inst_inf = creature_instance_info_get(i);
        if ((inst_inf->flags & InstPF_RepeatTrigger) != 0)
        {
            if (ccctrl->instance_id == CrInst_NULL)
            {
                if (creature_instance_is_available(cctng, i))
                {
                    if (creature_instance_has_reset(cctng, i))
                    {
                        n = get_human_controlled_creature_target(cctng, inst_inf->field_1D);
                        set_creature_instance(cctng, i, 1, n, 0);
                    }
                }
            }
        }
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(cctng);
    i = pckt->pos_y;
    if (i < 5)
      i = 5;
    else
    if (i > 250)
      i = 250;
    long k = i - 127;
    long angle = (pckt->pos_x - 127) / player->field_14;
    if (angle != 0)
    {
      if (angle < -32)
          angle = -32;
      else
      if (angle > 32)
          angle = 32;
      ccctrl->field_6C += 56 * angle / 32;
    }
    long angle_limit = crstat->max_angle_change;
    if (angle_limit < 1)
        angle_limit = 1;
    angle = ccctrl->field_6C;
    if (angle < -angle_limit)
        angle = -angle_limit;
    else
    if (angle > angle_limit)
        angle = angle_limit;
    cctng->move_angle_xy = (cctng->move_angle_xy + angle) & LbFPMath_AngleMask;
    cctng->move_angle_z = (227 * k / 127) & LbFPMath_AngleMask;
    ccctrl->field_CC = 170 * angle / angle_limit;
    ccctrl->field_6C = 4 * angle / 8;
}

void process_players_creature_control_packet_action(long plyr_idx)
{
  struct CreatureControl *cctrl;
  struct InstanceInfo *inst_inf;
  struct PlayerInfo *player;
  struct Thing *thing;
  struct Packet *pckt;
  long i;
  long k;
  player = get_player(plyr_idx);
  pckt = get_packet_direct(player->packet_num);
  SYNCDBG(6,"Processing player %d action %d",(int)plyr_idx,(int)pckt->action);
  switch (pckt->action)
  {
  case PckA_DirectCtrlExit:
      player->influenced_thing_idx = pckt->actn_par1;
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
      if (thing_is_invalid(thing))
        break;
      cctrl = creature_control_get_from_thing(thing);
      if (creature_control_invalid(cctrl))
        break;
      i = pckt->actn_par1;
      inst_inf = creature_instance_info_get(i);
      if (!inst_inf->instant)
      {
        cctrl->active_instance_id = i;
      } else
      if (cctrl->instance_id == CrInst_NULL)
      {
        if (creature_instance_is_available(thing,i) && creature_instance_has_reset(thing, pckt->actn_par1))
        {
          i = pckt->actn_par1;
          inst_inf = creature_instance_info_get(i);
          k = get_human_controlled_creature_target(thing, inst_inf->field_1D);
          set_creature_instance(thing, i, 1, k, 0);
          if (plyr_idx == my_player_number) {
              instant_instance_selected(i);
          }
        }
      }
      break;
      case PckA_DirectCtrlDragDrop:
      {
         direct_control_pick_up_or_drop(player);
         break;
      }
  }
}

/**
 * Exchange packets if MP game, then process all packets influencing local game state.
 */
void process_packets(void)
{
    int i;
    struct PlayerInfo* player;
    SYNCDBG(5, "Starting");
    // Do the network data exchange
    lbDisplay.DrawColour = colours[15][15][15];
    // Exchange packets with the network
    if (game.game_kind != GKind_LocalGame)
    {
        player = get_my_player();
        int j = 0;
        for (i = 0; i < 4; i++)
        {
            if (network_player_active(i))
                j++;
        }
        if (!game.packet_load_enable || game.numfield_149F47)
        {
            struct Packet* pckt = get_packet_direct(player->packet_num);
            if (LbNetwork_Exchange(pckt) != 0)
            {
                ERRORLOG("LbNetwork_Exchange failed");
            }
        }
        int k = 0;
        for (i = 0; i < 4; i++)
        {
            if (network_player_active(i))
                k++;
        }
        if (j != k)
        {
            for (i = 0; i < 4; i++)
            {
                player = get_player(i);
                if (!network_player_active(player->packet_num))
                {
                    player->allocflags |= PlaF_CompCtrl;
                    toggle_computer_player(i);
                }
            }
        }
  }
  // Setting checksum problem flags
  switch (checksums_different())
  {
  case 1:
      set_flag_byte(&game.system_flags,GSF_NetGameNoSync,true);
      set_flag_byte(&game.system_flags,GSF_NetSeedNoSync,false);
    break;
  case 2:
      set_flag_byte(&game.system_flags,GSF_NetGameNoSync,false);
      set_flag_byte(&game.system_flags,GSF_NetSeedNoSync,true);
    break;
  case 3:
      set_flag_byte(&game.system_flags,GSF_NetGameNoSync,true);
      set_flag_byte(&game.system_flags,GSF_NetSeedNoSync,true);
    break;
  default:
      set_flag_byte(&game.system_flags,GSF_NetGameNoSync,false);
      set_flag_byte(&game.system_flags,GSF_NetSeedNoSync,false);
    break;
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
  SYNCDBG(7,"Finished");
}

void process_frontend_packets(void)
{
  long i;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    net_screen_packet[i].field_4 &= ~0x01;
  }
  struct ScreenPacket* nspckt = &net_screen_packet[my_player_number];
  set_flag_byte(&nspckt->field_4, 0x01, true);
  nspckt->field_5 = frontend_alliances;
  set_flag_byte(&nspckt->field_4, 0x01, true);
  nspckt->field_4 ^= ((nspckt->field_4 ^ (fe_computer_players << 1)) & 0x06);
  nspckt->field_6 = VersionMajor;
  nspckt->field_8 = VersionMinor;
  if (LbNetwork_Exchange(nspckt))
    ERRORLOG("LbNetwork_Exchange failed");
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
      } else
      if (frontend_menu_state == FeSt_NET_START)
      {
          if (LbNetwork_Stop())
          {
            ERRORLOG("LbNetwork_Stop() failed");
            return;
          }
          if (setup_network_service(net_service_index_selected))
          {
            frontend_set_state(FeSt_NET_SESSION);
          } else
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
    if ((nspckt->field_4 & 0x01) != 0)
    {
        long k;
        switch (nspckt->field_4 >> 3)
        {
        case 2:
            add_message(i, (char*)&nspckt->param1);
            break;
        case 3:
            if (!validate_versions())
            {
                versions_different_error();
                break;
            }
            fe_network_active = 1;
            frontend_set_state(FeSt_NETLAND_VIEW);
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
                    add_message(i, player->mp_message_text);
                    k = 0;
                    player->mp_message_text[k] = '\0';
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
        if (nspckt->field_5 != -1)
          frontend_alliances = nspckt->field_5;
      }
      if (fe_computer_players == 2)
      {
        k = ((nspckt->field_4 & 0x06) >> 1);
        if (k != 2)
          fe_computer_players = k;
      }
      player->field_4E7 = nspckt->field_8 + (nspckt->field_6 << 8);
    }
    nspckt->field_4 &= 0x07;
  }
  if (frontend_alliances == -1)
    frontend_alliances = 0;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    nspckt = &net_screen_packet[i];
    if ((nspckt->field_4 & 0x01) == 0)
    {
      if (frontend_is_player_allied(my_player_number, i))
        frontend_set_alliance(my_player_number, i);
    }
  }
}

/******************************************************************************/
