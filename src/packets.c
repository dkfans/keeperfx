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
#include "bflib_dernc.h"
#include "bflib_network.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_planar.h"

#include "kjm_input.h"
#include "front_simple.h"
#include "front_network.h"
#include "frontend.h"
#include "frontmenu_net.h"
#include "vidmode.h"
#include "config.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_effects.h"
#include "config_terrain.h"
#include "config_players.h"
#include "config_settings.h"
#include "hist_actions.h"
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
#include "net_remap.h"
#include "net_sync.h"
#include "game_legacy.h"
#include "engine_redraw.h"
#include "frontmenu_ingame_tabs.h"
#include "vidfade.h"

#include "keeperfx.hpp"

#include "music_player.h"

TbBool packets_first_resync = true;

struct PacketContext
{
    // Here server will get movement controls of each player
    struct PacketEx last_packet_ex[NET_PLAYERS_COUNT];
};

/******************************************************************************/

static void process_players_dungeon_control_packet_action(
    struct PlayerInfo* player, enum TbPacketAction action, struct SmallActionPacket *packet);
static void process_players_creature_passenger_packet_action(
        struct PlayerInfo* player, enum TbPacketAction action, struct SmallActionPacket *packet);
static void process_players_creature_control_packet_action(
          struct PlayerInfo* player, enum TbPacketAction action, struct SmallActionPacket *packet);

/******************************************************************************/
static void loss_wait()
{
    if ((game.operation_flags & GOF_Paused) == 0)
    {
        NETDBG(3, "micro wait turn:%lu ui_turn:%lu", (unsigned long)game.play_gameturn, ui_turn);
        game.operation_flags |= GOF_Paused;
        game_flags2 |= GF2_ClearPauseOnPacket;
    }
}
/******************************************************************************/

extern struct Thing *create_gold_pile(struct Coord3d *pos, long value);
extern TbBool process_dungeon_control_packet_clicks(struct PlayerInfo* player, struct Packet* pckt);
/******************************************************************************/
static void set_mouse_light(struct PlayerInfo *player, struct Packet *pckt)
{
    SYNCDBG(7,"Starting");
    if (player->field_460 != 0)
    {
        if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
        {
            struct Coord3d pos;
            pos.x.val = pckt->pos_x;
            pos.y.val = pckt->pos_y;
            pos.z.val = get_floor_height_at(&pos);
            if (is_my_player(player)) {
                game.pos_14C006 = pos;
            }
            light_turn_light_on(player->field_460);
            light_set_light_position(player->field_460, &pos);
        }
        else
        {
            light_turn_light_off(player->field_460);
        }
    }
}

void update_double_click_detection(int plyr_idx, struct Packet* packet)
{
  // TODO: move to player?
  if ((packet->control_flags & PCtr_LBtnRelease) != 0)
  {
        if (packet_left_button_click_space_count[plyr_idx] < 5)
            packet_left_button_double_clicked[plyr_idx] = 1;
        packet_left_button_click_space_count[plyr_idx] = 0;
  }
  if ((packet->control_flags & (PCtr_LBtnClick|PCtr_LBtnHeld)) == 0)
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
    MapCoord x = ((player->field_4A4 + 1) / 2) + slab_subtile(subtile_slab_fast(stl_x), 0);
    MapCoord y = ((player->field_4A4 + 1) / 2) + slab_subtile(subtile_slab_fast(stl_y), 0);
    struct Room* room = player_build_room_at(x, y, plyr_idx, rkind);
    if (!room_is_invalid(room))
    {
        dungeon->camera_deviate_jump = 192;
        struct Coord3d pos;
        set_coords_to_slab_center(&pos, subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
        create_price_effect(&pos, plyr_idx, rstat->cost);
    }
    return room;
}

// Fake packet
TbBool process_dungeon_control_packet_spell_overcharge(struct PlayerInfo* player, struct Packet* pckt)
{
    struct Dungeon* dungeon = get_players_dungeon(player);
    SYNCDBG(6,"Starting for player %d state %s", (int)player->id_number, player_state_code_name(player->work_state));
    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
      switch (player->work_state)
      {
      case PSt_CallToArms:
          if (player_uses_power_call_to_arms(player->id_number))
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

/*
  this function remove any input flags from incoming packet
  That should be removed later because we dont want raw input from clients
*/
void clear_input(struct Packet* packet)
{
    packet->control_flags &= ~(PCtr_LBtnRelease | PCtr_LBtnClick | PCtr_RBtnRelease);
    NETDBG(7, "turn:%04ld", game.play_gameturn);
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
          if ((player->field_3 & Pf3F_Unkn08) != 0)
          {
              PaletteSetPlayerPalette(player, engine_palette);
              player->field_3 &= ~Pf3F_Unkn08;
          }
      }
  }
}

static void process_players_dungeon_control_packet_control(struct PlayerInfo* player, struct Packet *pckt)
{
    SYNCDBG(6,"Processing player %d action %d",(int)player->id_number,(int)pckt->action);
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
    if (pckt->field_10 & PCAdV_SpeedupPressed)
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
        case PVM_IsometricView:
            view_zoom_camera_out(cam, zoom_max, zoom_min);
            update_camera_zoom_bounds(cam, zoom_max, zoom_min);
            break;
        default:
            view_zoom_camera_out(cam, zoom_max, zoom_min);
            break;
        }
    }
    // TODO: all reasonable things should be extracted from that function
    // process_dungeon_control_packet_clicks(player, pckt);
    set_mouse_light(player, pckt);
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
        (chr == ' ')  || (chr == '!') || (chr == ':') || (chr == ';') ||
        (chr == '(') || (chr == ')') || (chr == '.'))
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

static void process_players_message_character(struct PlayerInfo *player, struct SmallActionPacket* packet)
{
    NETDBG(8, "action:%d", packet->action);
    assert(packet->action == PckA_PlyrMsgChar);
    if (packet->arg0 > 0)
    {
        message_text_key_add(player->mp_message_text, PLAYER_MP_MESSAGE_LEN, packet->arg0, packet->arg1);
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
          myplyr->field_3 |= Pf3F_Unkn10;
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
          myplyr->field_3 |= Pf3F_Unkn10;
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

static TbBool process_players_global_packet_action(
      struct PlayerInfo* player, unsigned char kind, struct SmallActionPacket* pckt)
{
  SYNCDBG(6,"Processing player:%d action:%d",(int)player->id_number,(int)kind);
  struct Dungeon *dungeon;
  Thingid thing_id;
  struct Thing *thing;
  struct ThingAdd *thingadd;
  struct Coord3d pos;
  int i;

  switch (kind)
  {
  case PckA_None:
      return true; // Nothing to do
  case PckA_Quitgame:
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
      return 1;
  case PckA_PlyrMsgEnd:
      player->allocflags &= ~PlaF_NewMPMessage;
      if (player->mp_message_text[0] == '!')
      {
          if (!cmd_exec(player->id_number, player->mp_message_text))
              message_add(player->id_number, player->mp_message_text);
          else
          {
              JUSTLOG("CMD player:%d cmd:%s", player->id_number, player->mp_message_text);
          }
      }
      else if (player->mp_message_text[0] != '\0')
          message_add(player->id_number, player->mp_message_text);
      LbMemorySet(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
      return 1;
  case PckA_PlyrMsgClear:
      player->allocflags &= ~PlaF_NewMPMessage;
      LbMemorySet(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
      return 1;
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
      process_pause_packet((game.operation_flags & GOF_Paused) == 0, pckt->arg0);
      return 1;
  case PckA_SetCluedo:
      if (is_my_player(player))
      {
          settings.video_cluedo_mode = pckt->arg0;
          save_settings();
      }
      player->video_cluedo_mode = pckt->arg0;
      return 0;
  case PckA_Unknown025:
      if (is_my_player(player))
      {
        change_engine_window_relative_size(pckt->arg0, pckt->arg1);
        centre_engine_window();
      }
      return 0;
  case PckA_BookmarkLoad:
      set_player_cameras_position(player, subtile_coord_center(pckt->arg0), subtile_coord_center(pckt->arg1));
      return 0;
  case PckA_SetGammaLevel:
      if (is_my_player(player))
      {
        set_gamma(pckt->arg0, 1);
        save_settings();
      }
      return 0;
  case PckA_SetMinimapConf:
      player->minimap_zoom = pckt->arg0;
      return 0;
  case PckA_SetMapRotation:
      player->cameras[CamIV_Parchment].orient_a = pckt->arg0;
      player->cameras[CamIV_FrontView].orient_a = pckt->arg0;
      player->cameras[CamIV_Isometric].orient_a = pckt->arg0;
      return 0;
  case PckA_SetPlyrState:
      set_player_state(player, pckt->arg0, pckt->arg1);
      return 1;
  case PckA_SwitchView:
      set_engine_view(player, pckt->arg0);
      return 0;
  case PckA_ToggleTendency:
      toggle_creature_tendencies(player, pckt->arg0);
      if (is_my_player(player)) {
          dungeon = get_players_dungeon(player);
          game.creatures_tend_imprison = ((dungeon->creature_tendencies & 0x01) != 0);
          game.creatures_tend_flee = ((dungeon->creature_tendencies & 0x02) != 0);
      }
      return 0;
  case PckA_CheatEnter:
//      game.???[my_player_number].cheat_mode = 1;
      show_onscreen_msg(2*game.num_fps, "Cheat mode activated by player %d", player->id_number);
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
      set_player_mode(player, pckt->arg0);
      return 0;
  case PckA_ZoomFromMap:
      //TODO: is it actually required on other end?
      set_player_cameras_position(player, subtile_coord_center(pckt->arg0), subtile_coord_center(pckt->arg1));
      player->cameras[CamIV_Parchment].orient_a = 0;
      player->cameras[CamIV_FrontView].orient_a = 0;
      player->cameras[CamIV_Isometric].orient_a = 0;
      if (((game.system_flags & GSF_NetworkActive) != 0)
          || (lbDisplay.PhysicalScreenWidth > 320))
      {
        if (is_my_player_number(player->id_number))
          toggle_status_menu((game.operation_flags & GOF_ShowPanel) != 0);
        set_player_mode(player, PVT_DungeonTop);
      } else
      {
        set_player_mode(player, PVT_MapFadeOut);
      }
      return true;
  case PckA_UpdatePause:
      process_pause_packet(pckt->arg0, pckt->arg1);
      return 1;
  case PckA_Unknown083:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(player->id_number);
      event_move_player_towards_event(player, pckt->arg0);
      return 0;
  case PckA_ZoomToRoom:
  {
      if (player->work_state == PSt_CreatrInfo)
          turn_off_query(player->id_number);
      struct Room* room = room_get(pckt->arg0);
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
        turn_off_query(player->id_number);
      thing_id = net_remap_thingid(player_to_client(player->id_number), pckt->arg0);
      thing = thing_get(thing_id);
      player->zoom_to_pos_x = thing->mappos.x.val;
      player->zoom_to_pos_y = thing->mappos.y.val;
      set_player_instance(player, PI_ZoomToPos, 0);
      if ((player->work_state == PSt_PlaceTrap) || (player->work_state == PSt_PlaceDoor)) {
          set_player_state(player, PSt_PlaceTrap, thing->model);
      }
      return 0;
  case PckA_ZoomToDoor:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(player->id_number);
      thing_id = net_remap_thingid(player_to_client(player->id_number), pckt->arg0);
      thing = thing_get(thing_id);
      player->zoom_to_pos_x = thing->mappos.x.val;
      player->zoom_to_pos_y = thing->mappos.y.val;
      set_player_instance(player, PI_ZoomToPos, 0);
      if ((player->work_state == PSt_PlaceTrap) || (player->work_state == PSt_PlaceDoor)) {
          set_player_state(player, PSt_PlaceDoor, thing->model);
      }
      return 0;
  case PckA_ZoomToPosition:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(player->id_number);
      player->zoom_to_pos_x = pckt->arg0;
      player->zoom_to_pos_y = pckt->arg1;
      set_player_instance(player, PI_ZoomToPos, 0);
      return 0;
  case PckA_Unknown088:
      game.numfield_D ^= (game.numfield_D ^ (GNFldD_Unkn04 * ((game.numfield_D & GNFldD_Unkn04) == 0))) & GNFldD_Unkn04;
      return 0;
  case PckA_PwrCTADis:
      turn_off_power_call_to_arms(player->id_number);
      return 0;
  case PckA_UsePwrHandPick:
      thing_id = net_remap_thingid(player_to_client(player->id_number), pckt->arg0);
      thing = thing_get(thing_id);
      if (magic_use_available_power_on_thing(player->id_number, PwrK_HAND, 0,thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, PwMod_Default))
      {
          if (player == get_my_player())
          {
              // Update sprites for the creature in hand, and power hand itself
              set_power_hand_offset(player, get_first_thing_in_power_hand(player));
              if (!thing_is_invalid(thing))
              {
                  set_power_hand_graphic(player->id_number, 784, 256);
              }
          }
      }
      return true;
  case PckA_UsePwrHandDrop:
      if (!dump_first_held_thing_on_map(player->id_number, pckt->arg0, pckt->arg1, 1))
      {
          if (player->id_number == my_player_number)
          {
              //TODO: revert to "FULL HAND" state?
              // player->field_4AF = !0;
          }
      }
      return true;
  case PckA_HandPreGrab: // Maybe not necessary action
      thing = thing_get(pckt->arg0);
      if (thing_is_creature(thing))
      {
        clear_creature_instance(thing);
      }
      return true; // processed
  case PckA_Unknown092:
      if (game.event[pckt->arg0].kind == 3)
      {
        turn_off_event_box_if_necessary(player->id_number, pckt->arg0);
      } else
      {
        event_delete_event(player->id_number, pckt->arg0);
      }
      return 0;
  case PckA_UsePwrObey:
      magic_use_available_power_on_level(player->id_number, PwrK_OBEY, 0, PwMod_Default);
      return 0;
  case PckA_UsePwrArmageddon:
      magic_use_available_power_on_level(player->id_number, PwrK_ARMAGEDDON, 0, PwMod_Default);
      return 0;
  case PckA_Unknown099:
      turn_off_query(player->id_number);
      return 0;
  case PckA_Unknown104:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(player->id_number);
      battle_move_player_towards_battle(player, pckt->arg0);
      return 0;
  case PckA_ZoomToSpell:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(player->id_number);
      {
          struct Coord3d locpos;
          if (find_power_cast_place(player->id_number, pckt->arg0, &locpos))
          {
              player->zoom_to_pos_x = locpos.x.val;
              player->zoom_to_pos_y = locpos.y.val;
              set_player_instance(player, PI_ZoomToPos, 0);
          }
      }
      if (!power_is_instinctive(pckt->arg0))
      {
          const struct PowerConfigStats *powerst;
          powerst = get_power_model_stats(pckt->arg0);
          i = get_power_index_for_work_state(player->work_state);
          if (i > 0)
            set_player_state(player, powerst->work_state, 0);
      }
      return 0;
  case PckA_PlyrFastMsg:
      //show_onscreen_msg(game.num_fps, "Message from player %d", player->id_number);
      output_message(SMsg_EnemyHarassments+pckt->arg0, 0, true);
      return 0;
  case PckA_SetComputerKind:
      set_autopilot_type(player->id_number, pckt->arg0);
      return 0;
  case PckA_GoSpectator:
      level_lost_go_first_person(player->id_number);
      return 0;
  case PckA_DumpHeldThingToOldPos:
      dungeon = get_players_num_dungeon(player->id_number);
      if (!power_hand_is_empty(player))
      {
          thing = get_first_thing_in_power_hand(player);
          dump_first_held_thing_on_map(player->id_number, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 1);
      }
      return false;
  case PckA_PwrSOEDis:
      turn_off_power_sight_of_evil(player->id_number);
      return false;
  case PckA_EventBoxActivate:
      //TODO: is it actually required on net?
      go_on_then_activate_the_event_box(player->id_number, pckt->arg0);
      return true;
  case PckA_EventBoxClose:
      dungeon = get_players_num_dungeon(player->id_number);
      turn_off_event_box_if_necessary(player->id_number, dungeon->visible_event_idx);
      dungeon->visible_event_idx = 0;
      return false;
  case PckA_UsePwrOnThing:
      i = get_power_overcharge_level(player);
      directly_cast_spell_on_thing(player->id_number, pckt->arg0, pckt->arg1, i);
      return 0;
  case PckA_PlyrToggleAlly:
      toggle_ally_with_player(player->id_number, pckt->arg0);
      return 0;
  case PckA_SaveViewType:
      if (player->acamera != NULL)
        player->view_mode_restore = player->acamera->view_mode;
      set_player_mode(player, pckt->arg0);
      return false;
  case PckA_LoadViewType:
      set_player_mode(player, pckt->arg0);
      set_engine_view(player, player->view_mode_restore);
      return false;
  case PckA_CreateGoldPile:
      unpackpos_2d(&pos, pckt->arg0);
      thing = create_gold_pile(&pos, pckt->arg[2] | (pckt->arg[3] << 8));
      net_remap_update(player_to_client(player->id_number), pckt->arg[1], thing->index);
      NETDBG(4, "PckA_CreateGoldPile their:%d, mine:%d", (int)pckt->arg[1], (int)thing->index);
      pckt->arg[1] = thing->index;
      return true;
  default:
      return false;
  }
}

static void process_map_packet_clicks(struct PlayerInfo* player, struct Packet *pckt)
{
    SYNCDBG(7,"Starting");
    packet_left_button_double_clicked[player->id_number] = 0;
    if ((pckt->control_flags & PCtr_Unknown4000) == 0)
    {
        update_double_click_detection(player->id_number, pckt);
    }
    SYNCDBG(8,"Finished");
}

static void process_players_map_packet_control(struct PlayerInfo* player, struct Packet *pckt)
{
    SYNCDBG(6,"Starting");
    // Get map coordinates
    process_map_packet_clicks(player, pckt);
    player->cameras[CamIV_Parchment].mappos.x.val = pckt->pos_x;
    player->cameras[CamIV_Parchment].mappos.y.val = pckt->pos_y;
    set_mouse_light(player, pckt);
    SYNCDBG(8,"Finished");
}

/**
 * Process packet with input commands for given player.
 * @param plyr_idx Player to process packet for.
 */
static void process_players_packet(
    struct PlayerInfo* player, unsigned char kind, struct SmallActionPacket* packet_short)
{
    SYNCDBG(6, "Processing player %d packet of type %d.", player->id_number, (int)kind);

    if (kind == PckA_PlyrMsgChar)
    {
        if ((player->allocflags & PlaF_NewMPMessage) != 0)
        {
            process_players_message_character(player, packet_short);
        }
        else
        {
            WARNLOG("MsgChar without PlaF_NewMPMessage player:%d", player->id_number);
        }
    }
    else if (!process_players_global_packet_action(player, kind, packet_short))
    {
      // Different changes to the game are possible for different views.
      // For each there can be a control change (which is view change or mouse event not translated to action),
      // and action perform (which does specific action set in packet).
      switch (player->view_type)
      {
      case PVT_DungeonTop:
        process_players_dungeon_control_packet_action(player, (enum TbPacketAction)kind, packet_short);
        break;
      case PVT_CreatureContrl:
        process_players_creature_control_packet_action(player, (enum TbPacketAction)kind, packet_short);
        break;
      case PVT_CreaturePasngr:
        //process_players_creature_passenger_packet_control(player->id_number); -- there are no control changes in passenger mode
        process_players_creature_passenger_packet_action(player, (enum TbPacketAction)kind, packet_short);
        break;
      case PVT_MapScreen:
        //process_players_map_packet_action(player->id_number); -- there are no actions to perform from map screen
        break;
      default:
        break;
      }
  }
  SYNCDBG(8,"Finished");
}

static void process_players_creature_passenger_packet_action(
        struct PlayerInfo* player, enum TbPacketAction action, struct SmallActionPacket *packet)
{
    SYNCDBG(6,"Processing player %d action %d",(int)player->id_number,(int)packet->action);
    if (packet->action == PckA_PasngrCtrlExit)
    {
        player->influenced_thing_idx = packet->arg0;
        set_player_instance(player, PI_PsngrCtLeave, 0);
    }
    SYNCDBG(8,"Finished");
}

static void process_players_dungeon_control_packet_action(
    struct PlayerInfo* player, enum TbPacketAction action, struct SmallActionPacket *packet)
{

    struct BigActionPacket *big = (struct BigActionPacket *)packet;
    int plyr_idx = player->id_number;
    Thingid thing_id;
    SYNCDBG(6,"Processing player %d action %d",(int)plyr_idx,(int)action);
    switch (action)
    {
    case PckA_HoldAudience:
        magic_use_available_power_on_level(plyr_idx, PwrK_HOLDAUDNC, 0, PwMod_Default);
        break;
    case PckA_UseSpecialBox:
        activate_dungeon_special(thing_get(packet->arg0), player);
        break;
    case PckA_ResurrectCrtr:
        resurrect_creature(thing_get(packet->arg0), 
            (packet->arg1) & 0x0F, 
            (packet->arg1 >> 4) & 0xFF,
            (packet->arg1 >> 12) & 0x0F);
        break;
    case PckA_TransferCreatr:
        transfer_creature(thing_get(packet->arg0), thing_get(packet->arg1), plyr_idx);
        break;
    case PckA_ToggleComputer:
        toggle_computer_player(plyr_idx);
        break;
    case PckA_BuildRoom:
        NETDBG(5, "packet_kind:%d player_kind:%d", big->head.arg[2], player->chosen_room_kind);

        // TODO: we should track these events sometimes
        assert(big->head.arg[2] == player->chosen_room_kind);

        keeper_build_room(
            big->head.arg[0], //stl_x
            big->head.arg[1], //stl_y
            player->id_number,
            big->head.arg[2] //player->chosen_room_kind
            );
        break;
    case PckA_UsePower:
        thing_id = net_remap_thingid(player_to_client(player->id_number), big->head.arg[1]);
        NETDBG(5, "plyr:%d power:%d %s level:%d x:%d y:%d thing:%d(%d)",
            player->id_number,
            big->head.arg[0] & 255, // pwkind
            power_code_name(big->head.arg[0] & 255),
            big->head.arg[0] >> 8,  // powerlevel,
            big->head.arg[2], // stl_x
            big->head.arg[3], // stl_y
            thing_id, big->head.arg[1] // thing
              );
        if (big->head.arg[1] != 0)
        {
            magic_use_available_power_on_thing(player->id_number,
                big->head.arg[0] & 255, // pwkind
                big->head.arg[0] >> 8,  // powerlevel
                big->head.arg[2], // stl_x
                big->head.arg[3], // stl_y
                thing_get(thing_id), // thing
                PwMod_Default);
        }
        else
        {
            magic_use_available_power_on_subtile(player->id_number,
                big->head.arg[0] & 255, // pwkind
                big->head.arg[0] >> 8,  // powerlevel
                big->head.arg[2], // stl_x
                big->head.arg[3], // stl_y
                PwCast_None);
        }
        break;
    case PckA_TagUntag:
        NETDBG(5, "%s plyr:%d x:%d y:%d flags:%d",
            (big->head.arg[2] == 1)? "untag" : ((big->head.arg[2] == 0)? "tag" : "?" ),
            player->id_number,
            big->head.arg[0], // stl_x
            big->head.arg[1], // stl_y
            big->head.arg[2] // flags
            );
        struct Dungeon* dungeon = get_players_dungeon(player);
        if (big->head.arg[2] == 1)
        {
            NETDBG(6, "untagged");
            hist_map_action(HAT_Untag, player->id_number, big->head.arg[0], big->head.arg[1]);
            untag_blocks_for_digging_in_rectangle_around(big->head.arg[0], big->head.arg[1], player->id_number);
        }
        else if (big->head.arg[2] == 0)
        {
            if (dungeon->task_count < 300 - 9)
            {
                if (tag_blocks_for_digging_in_rectangle_around(big->head.arg[0], big->head.arg[1], player->id_number))
                {
                    NETDBG(6, "tagged");
                    hist_map_action(HAT_Tag, player->id_number, big->head.arg[0], big->head.arg[1]);
                }
                else
                {
                    NETDBG(6, "already tagged");
                }
            }
            else
            {
                NETDBG(6, "unable to tag - too many tasks for player:%d ", player->id_number);
            }
        }
        break;
    default:
        NETLOG("unexpected action: %d", (int) action);
    }
}

void process_players_creature_control_packet_control(struct PlayerInfo* player, struct Packet *pckt)
{
    struct InstanceInfo *inst_inf;
    long i;
    long n;

    SYNCDBG(6,"Starting");
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
                    inst_inf = creature_instance_info_get(i);
                    n = get_human_controlled_creature_target(cctng, inst_inf->field_1D);
                    set_creature_instance(cctng, i, 1, n, 0);
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

static void process_players_creature_control_packet_action(
          struct PlayerInfo* player, enum TbPacketAction action, struct SmallActionPacket *packet)
{
  struct CreatureControl *cctrl;
  struct InstanceInfo *inst_inf;
  struct Thing *thing;
  long i;
  long k;
  SYNCDBG(6,"Processing player %d action %d",(int)player->id_number, (int)packet->action);
  switch (packet->action)
  {
  case PckA_DirectCtrlExit:
      player->influenced_thing_idx = packet->arg0;
      set_player_instance(player, PI_DirctCtLeave, 0);
      break;
  case PckA_CtrlCrtrSetInstnc:
      thing = thing_get(player->controlled_thing_idx);
      if (thing_is_invalid(thing))
        break;
      cctrl = creature_control_get_from_thing(thing);
      if (creature_control_invalid(cctrl))
        break;
      i = packet->arg0;
      inst_inf = creature_instance_info_get(i);
      if (!inst_inf->field_0)
      {
        cctrl->active_instance_id = i;
      } else
      if (cctrl->instance_id == CrInst_NULL)
      {
        if (creature_instance_is_available(thing,i) && creature_instance_has_reset(thing, packet->arg0))
        {
          i = packet->arg0;
          inst_inf = creature_instance_info_get(i);
          k = get_human_controlled_creature_target(thing, inst_inf->field_1D);
          set_creature_instance(thing, i, 1, k, 0);
          if (player->id_number == my_player_number) {
              instant_instance_selected(i);
          }
        }
      }
      break;
  }
}

static TbBool process_packet_cb(
        void *context_ptr, unsigned long turn, int plyr_idx, unsigned char kind, void *data, short size)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    
    struct PacketContext *context = (struct PacketContext *)context_ptr;
    struct SmallActionPacket* packet_short = (struct SmallActionPacket*)data;
    struct PacketEx *packet_ex = (struct PacketEx*)data;

    switch (kind)
    {
    case PckA_RemapNotify:
        return net_remap_packet_cb(turn, plyr_idx, kind, data, size);
    case PckA_ForceResync:
        return net_sync_process_force_packet(turn, plyr_idx, kind, data, size);
    default:
        break;
    }

    net_remap_start(plyr_idx, kind, data, size);

    if (kind == PckA_PacketEx)
    {   // process First packet (with mouse coords and without action)
    
        if (size != sizeof(struct PacketEx))
        {
            NETLOG("WTF?! %d != %d", size, sizeof(struct PacketEx));
        }
        assert (size == sizeof(struct PacketEx));
        player->input_crtr_control = ((packet_ex->packet.field_10 & PCAdV_CrtrContrlPressed) != 0);
        player->input_crtr_query = ((packet_ex->packet.field_10 & PCAdV_CrtrQueryPressed) != 0);

        assert((packet_ex->packet.action == PckA_None) 
            || (packet_ex->packet.action == PckA_Invalid));

        context->last_packet_ex[player->id_number] = *packet_ex;
        
        switch (player->view_type)
        {
        case PVT_DungeonTop:
            //TODO: we should store valid packet_ex structure somewhere
            process_players_dungeon_control_packet_control(player, &packet_ex->packet);
            break;
        case PVT_CreaturePasngr:
            process_players_creature_control_packet_control(player, &packet_ex->packet);
            break;
        case PVT_MapScreen:
            process_players_map_packet_control(player, &packet_ex->packet);
            break;
        }
    }
    else
    {
        process_players_packet(player, kind, packet_short);
    }
    net_remap_finish();
    return true;
}
/**
 * Exchange packets if MP game, then process all packets influencing local game state.
 */
void process_packets(void)
{
    int i;
    int player_status;
    struct PlayerInfo* player;
    struct PacketContext context = { 0 };
    SYNCDBG(9, "Starting");
    // Do the network data exchange
    lbDisplay.DrawColour = colours[15][15][15];

    {
        player = get_my_player();
        player_status = 0;
        for (i = 0; i < 4; i++)
        {
            if (network_player_active(i))
                player_status ^= (1 << i);
        }
        if (!game.packet_load_enable || game.numfield_149F47)
        {
            if (check_resync_turn())
            {
                JUSTLOG("Time to resync! turn:%ld", game.play_gameturn);
                set_flag_byte(&game.system_flags,GSF_NetGameNoSync,true);
                set_flag_byte(&game.system_flags,GSF_NetSeedNoSync,true);
            }
            switch(LbNetwork_Exchange(&context, &process_packet_cb))
            {
            case NR_FAIL:
                ERRORLOG("LbNetwork_Exchange failed");
                loss_wait();
                return;
            case NR_RESYNC:
                if ((game.system_flags & GSF_NetGameNoSync) == 0)
                {
                    NETDBG(3, "got resync packet");
                    set_flag_byte(&game.system_flags,GSF_NetGameNoSync,true);
                    set_flag_byte(&game.system_flags,GSF_NetSeedNoSync,true);
                }
                break;
            case NR_OK:
                break;
            case NR_DISCONNECT:
                ERRORLOG("LbNetwork_Exchange says: NR_DISCONNECT");
                quit_game = 1;
                return;
            }
        }

        for (i = 0; i < 4; i++)
        {
            if (network_player_active(i))
                player_status ^= (1 << i);
        }
        if (player_status != 0) // Someone connected or disconnected
        {
            for (i = 0; i < 4; i++)
            {
                if ((player_status & (1 << i)) == 0)
                    continue;
                player = get_player(i);
                if (!network_player_active(player->packet_num))
                {
                    EVM_GLOBAL_EVENT("mp.disconnect,plyr=%d cnt=1", i);

                    game.operation_flags |= GOF_Paused;
                    //message_add_fmt(i, "AI in control!");
                    //player->allocflags |= PlaF_CompCtrl;
                    //toggle_computer_player(i);
                }
            }
        }
  }
  // Setting checksum problem flags
  if (((game.system_flags & GSF_NetGameNoSync) == 0) && checksums_different())
  {
      set_flag_byte(&game.system_flags,GSF_NetGameNoSync,true);
      set_flag_byte(&game.system_flags,GSF_NetSeedNoSync,true);
  }
  // Write packets into file, if requested
  if ((game.packet_save_enable) && (game.packet_fopened))
    save_packets();
//Debug code, to find packet errors
#if DEBUG_NETWORK_PACKETS
  if (((game.system_flags & GSF_NetGameNoSync) == 0)) || packets_first_resync)
  {
      write_debug_packets();
  }
#endif
  if (((game.system_flags & GSF_NetGameNoSync) != 0)
   || ((game.system_flags & GSF_NetSeedNoSync) != 0))

  {
      if (packets_first_resync)
      {
          game.operation_flags |= GOF_Paused;
          game_flags2 |= GF2_ClearPauseOnSync;
          SYNCDBG(0, "Resyncing");
          EVM_GLOBAL_EVENT("mp.resync,system_flags=0x%0x cnt=1", game.system_flags);
      }
      if (resync_game(packets_first_resync))
      {
          set_flag_byte(&game.system_flags, GSF_NetGameNoSync, false);
          set_flag_byte(&game.system_flags, GSF_NetSeedNoSync, false);
          packets_first_resync = true;
          if (game_flags2 & GF2_ClearPauseOnSync)
          {
              NETDBG(0, "Done resyncing, unpausing");
              game_flags2 &= ~GF2_ClearPauseOnSync;
              game.operation_flags &= ~GOF_Paused;
          }
          else
          {
              NETDBG(0, "Done resyncing");
          }
          EVM_GLOBAL_EVENT("mp.done_resync cnt=1");
      }
      else
      {
          packets_first_resync = false;
      }
  }
  else
  {
      // Clear all packets
      clear_packets();
  }
  SYNCDBG(13,"Finished");
}

void set_my_packet_action(struct PlayerInfo *player, enum TbPacketAction action, short arg0, short arg1)
{
    assert(is_my_player(player));
    create_packet_action(player, action, arg0, arg1);
}

struct PacketEx *create_outgoing_input_packet()
{
    struct PacketEx *ret = LbNetwork_AddPacket(PckA_PacketEx, game.play_gameturn, sizeof(struct PacketEx));
    return ret;
}
/******************************************************************************/
