/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_input.c
 *     Front-end user keyboard and mouse input.
 * @par Purpose:
 *     Reacts on events by creating packets or directly modifying various parameters.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 09 Aug 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "front_input.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_planar.h"

#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_sprfnt.h"
#include "bflib_datetm.h"
#include "bflib_fileio.h"
#include "bflib_network.h"
#include "bflib_network_exchange.h"
#include "bflib_inputctrl.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "kjm_input.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_map.h"
#include "frontmenu_ingame_evnt.h"
#include "scrcapt.h"
#include "player_instances.h"
#include "config_players.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "creature_instances.h"
#include "creature_states.h"
#include "gui_boxmenu.h"
#include "gui_frontmenu.h"
#include "gui_frontbtns.h"
#include "gui_tooltips.h"
#include "gui_topmsg.h"
#include "gui_parchment.h"
#include "power_hand.h"
#include "thing_creature.h"
#include "thing_shots.h"
#include "thing_traps.h"
#include "room_workshop.h"
#include "kjm_input.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "game_legacy.h"
#include "spdigger_stack.h"
#include "room_graveyard.h"
#include "gui_soundmsgs.h"
#include "creature_states_spdig.h"
#include "room_data.h"
#include "map_blocks.h"
#include "local_camera.h"
#include "packets.h"
#include "console_cmd.h"

#include "keeperfx.hpp"
#include "KeeperSpeech.h"

#include <math.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

unsigned short const zoom_key_room_order[] =
    {RoK_TREASURE, RoK_LIBRARY, RoK_LAIR, RoK_PRISON,
     RoK_TORTURE, RoK_TRAINING, RoK_DUNGHEART, RoK_WORKSHOP,
     RoK_SCAVENGER, RoK_TEMPLE, RoK_GRAVEYARD, RoK_BARRACKS,
     RoK_GARDEN, RoK_GUARDPOST, RoK_BRIDGE, RoK_ENTRANCE, RoK_NONE,};

KEEPERSPEECH_EVENT last_speech_event;

// define the current GUI layer as the default
struct GuiLayer gui_layer = {GuiLayer_Default};

TbBool first_person_see_item_desc = false;

long old_mx;
long old_my;

//arbitrary state machine, not deserving own enum
int synthetic_left = 0;
int synthetic_right = 0;

/******************************************************************************/
void get_dungeon_control_nonaction_inputs(void);
void get_creature_control_nonaction_inputs(void);
short zoom_shortcuts(void);
short get_bookmark_inputs(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long get_current_gui_layer()
{
    return gui_layer.current_gui_layer;
}

TbBool set_current_gui_layer(long layer_id)
{
    gui_layer.current_gui_layer = layer_id;
    return true;
}

TbBool check_current_gui_layer(long layer_id)
{
    // Check the current gui layer against the one passed as a parameter
    // Also checks if the passed layer_id is the parent of the current gui layer

    // This is just a basic example, GuiLayer objects should be created with parent properties etc etc

    if (gui_layer.current_gui_layer == layer_id)
    {
        return true;
    }
    else
    {
        // check children
        switch (layer_id)
        {
            case GuiLayer_OneClick:
                switch (gui_layer.current_gui_layer)
                {
                    case GuiLayer_OneClickBridgeBuild:
                        return true;
                    default:
                        return false;
                }
                break;
            default:
                return false;
        }
    }
}

void update_gui_layer()
{
    // Determine the current/correct GUI Layer to use at this moment

    if ((game.system_flags & GSF_NetworkActive) == 1) // no one click on multiplayer.
    {
        //todo Make multiplayer work with 1-click
        set_current_gui_layer(GuiLayer_Default);
        return;
    }

    struct PlayerInfo* player = get_my_player();
    if ( ((player->work_state == PSt_Sell) || (player->work_state == PSt_BuildRoom) || (player->render_roomspace.highlight_mode))  &&
         (is_game_key_pressed(Gkey_BestRoomSpace, NULL, true) || is_game_key_pressed(Gkey_SquareRoomSpace, NULL, true)) )
    {
        if (player->render_roomspace.one_click_mode_exclusive)
        {
            // Is the user in "one-click bridge building" mode
            set_current_gui_layer(GuiLayer_OneClickBridgeBuild);
        }
        else
        {
            // Is the user in "one-click" mode (i.e. they are in the build/sell player state, and are pressing the square/automagic button)
            set_current_gui_layer(GuiLayer_OneClick);
        }
    }
    else
    {
        // For now this is equivilent to "old input behaviour"
        set_current_gui_layer(GuiLayer_Default);
    }
}

short game_is_busy_doing_gui_string_input(void)
{
  return (input_button != NULL);
}

int is_game_key_pressed(long key_id, int32_t *val, TbBool ignore_mods)
{
  int result;
  int i;
  if ((key_id < 0) || (key_id >= GAME_KEYS_COUNT))
    return 0;
  if (val != NULL)
  {
    *val = settings.kbkeys[key_id].code;
  }
  if (get_current_gui_layer() == GuiLayer_OneClickBridgeBuild)
  {
    if ( (key_id == Gkey_RotateMod) && (
         (settings.kbkeys[Gkey_RotateMod].code == settings.kbkeys[Gkey_BestRoomSpace].code) ||
         (settings.kbkeys[Gkey_RotateMod].code == settings.kbkeys[Gkey_SquareRoomSpace].code) ) )
    {
      return 0;
    }
    if ( (key_id == Gkey_SpeedMod) && (
         (settings.kbkeys[Gkey_SpeedMod].code == settings.kbkeys[Gkey_BestRoomSpace].code) ||
         (settings.kbkeys[Gkey_SpeedMod].code == settings.kbkeys[Gkey_SquareRoomSpace].code) ) )
    {
      return 0;
    }
    if ( (key_id == Gkey_CrtrContrlMod) && (
         (settings.kbkeys[Gkey_CrtrContrlMod].code == settings.kbkeys[Gkey_BestRoomSpace].code) ||
         (settings.kbkeys[Gkey_CrtrContrlMod].code == settings.kbkeys[Gkey_SquareRoomSpace].code) ) )
    {
      return 0;
    }
  }
  if ((key_id == Gkey_RotateMod) || (key_id == Gkey_SpeedMod) || (key_id == Gkey_CrtrContrlMod)
      || (key_id == Gkey_CrtrQueryMod) || (key_id == Gkey_BestRoomSpace) || (key_id == Gkey_SquareRoomSpace)
      || (key_id == Gkey_SellTrapOnSubtile))
  {
      i = settings.kbkeys[key_id].code;
      switch (i)
      {
        case KC_LSHIFT:
        case KC_RSHIFT:
          result = key_modifiers & KMod_SHIFT;
          break;
        case KC_LCONTROL:
        case KC_RCONTROL:
          result = key_modifiers & KMod_CONTROL;
          break;
        case KC_LALT:
        case KC_RALT:
          result = key_modifiers & KMod_ALT;
          break;
        default:
          result = lbKeyOn[i];
          break;
      }
  } else
  {
      if ((ignore_mods) || (key_modifiers == settings.kbkeys[key_id].mods)) {
          i = settings.kbkeys[key_id].code;
          result = lbKeyOn[i];
      } else {
          result = 0;
      }
  }
  return result;
}

/**
 *  Reacts on a keystoke by sending text message packets.
 */
short get_players_message_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    if (is_key_pressed(KC_RETURN, KMod_NONE)) {
        memcpy(player->mp_pending_message, player->mp_message_text, PLAYER_MP_MESSAGE_LEN);
        set_players_packet_action(player, PckA_PlyrMsgEnd, 0, 0, 0, 0);
        if ((game.system_flags & GSF_NetworkActive) != 0) {
            LbNetwork_SendChatMessageImmediate(player->id_number, player->mp_message_text);
        }
        player->allocflags &= ~PlaF_NewMPMessage;
        memset(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
        clear_key_pressed(KC_RETURN);
    } else if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE)) {
        set_players_packet_action(player, PckA_PlyrMsgClear, 0, 0, 0, 0);
        clear_key_pressed(KC_ESCAPE);
    } else if (is_key_pressed(KC_TAB, KMod_NONE) && player->mp_message_text[0] == cmd_char) {
        cmd_auto_completion(player->id_number, player->mp_message_text + 1, PLAYER_MP_MESSAGE_LEN - 1);
        clear_key_pressed(KC_TAB);
    } else if (is_key_pressed(KC_UP, KMod_NONE)) {
        memcpy(player->mp_message_text, player->mp_message_text_last, PLAYER_MP_MESSAGE_LEN);
        clear_key_pressed(KC_UP);
    } else {
        LbTextSetFont(winfont);
        if (is_key_pressed(KC_BACK,KMod_DONTCARE) || pixel_size * LbTextStringWidth(player->mp_message_text) < 450) {
            message_text_key_add(player->mp_message_text, lbInkey, key_modifiers);
            clear_key_pressed(lbInkey);
            return true;
        }
        return false;
    }
    return true;
}

/**
 * Captures the screen to make a gameplay movie or screenshot image.
 * @return Returns true if packet was created, false otherwise.
 */
short get_screen_capture_inputs(void)
{
  if (is_key_pressed(KC_M,KMod_SHIFT))
  {
      if ((game.system_flags & GSF_CaptureMovie) != 0)
        movie_record_stop();
      else
        movie_record_start();
      clear_key_pressed(KC_M);
  }
  if (is_key_pressed(KC_C,KMod_SHIFT))
  {
      set_flag(game.system_flags, GSF_CaptureSShot);
      clear_key_pressed(KC_C);
  }
  return false;
}

/**
 * Checks if mouse pointer is currently over a specific button.
 * @param gbtn The button which position is to be verified.
 * @return Returns true it mouse is over the button.
 */
TbBool check_if_mouse_is_over_button(const struct GuiButton *gbtn)
{
    if ((gbtn->flags & LbBtnF_Visible) == 0)
        return false;
    return check_if_pos_is_over_button(gbtn, GetMouseX(), GetMouseY());
}

void clip_frame_skip(void)
{
  if (game.frame_skip > 512)
    game.frame_skip = 512;
  if (game.frame_skip < 0)
    game.frame_skip = 0;
}

void increaseFrameskip(void)
{
    // Default no longer using frame_skip=1, which will not change the logic frame rate but the makes the game will less smooth. But it can still be passed in through parameters
    int level = 16;
    for (int i=0; i<10; i++) {
        if (game.frame_skip < level)
            break;
        level <<= 1;
    }
    int adj = level/8;
    game.frame_skip += adj;
    clip_frame_skip();
    char speed_txt[256] = "normal";
    if (game.frame_skip > 0)
        sprintf(speed_txt, "x%d", game.frame_skip);
    show_onscreen_msg(game_num_fps*(game.frame_skip+1), "Fast Forward %s", speed_txt);
}

void decreaseFrameskip(void)
{
    // Defaul no longer using frame_skip=1, which will not change the logic frame rate but the makes the game will less smooth. But it can still be passed in through parameters
    int level = 16;
    for (int i=0; i<10; i++) {
        if (game.frame_skip <= level)
            break;
        level <<= 1;
    }
    int adj = level/8;
    game.frame_skip -= adj;
    clip_frame_skip();
    char speed_txt[256] = "normal";
    if (game.frame_skip > 0)
        sprintf(speed_txt, "x%d", game.frame_skip);
    show_onscreen_msg(game_num_fps*(game.frame_skip+1), "Fast Forward %s", speed_txt);
}

/**
 * Handles game speed control inputs.
 * @return Returns true if packet was created, false otherwise.
 */
short get_speed_control_inputs(void)
{
  if (is_key_pressed(KC_ADD,KMod_CONTROL))
  {
      increaseFrameskip();
      clear_key_pressed(KC_ADD);
  }
  if (is_key_pressed(KC_EQUALS,KMod_CONTROL))
  {
      increaseFrameskip();
      clear_key_pressed(KC_EQUALS);
  }
  if (is_key_pressed(KC_SUBTRACT,KMod_CONTROL))
  {
      decreaseFrameskip();
      clear_key_pressed(KC_SUBTRACT);
  }
  if (is_key_pressed(KC_MINUS,KMod_CONTROL))
  {
      decreaseFrameskip();
      clear_key_pressed(KC_MINUS);
  }
  return false;
}

/**
 * Handles control inputs in PacketLoad mode.
 */
short get_packet_load_game_control_inputs(void)
{
  if (lbKeyOn[KC_LALT] && lbKeyOn[KC_X])
  {
    clear_key_pressed(KC_X);
    if ((game.system_flags & GSF_NetworkActive) != 0)
      LbNetwork_Stop();
    quit_game = 1;
    exit_keeper = 1;
    return true;
  }
  if (is_key_pressed(KC_T,KMod_ALT))
  {
    clear_key_pressed(KC_T);
    disable_packet_mode();
    return true;
  }
  return false;
}

long get_small_map_inputs(long x, long y, long zoom)
{
  SYNCDBG(7,"Starting");
  short result = 0;
  long curr_mx = GetMouseX();
  long curr_my = GetMouseY();
  if (!grabbed_small_map)
    game.small_map_state = 0;
  if (((game.operation_flags & GOF_ShowGui) != 0) && (mouse_is_over_panel_map(x,y) || grabbed_small_map))
  {
    if (left_button_clicked)
    {
      clicked_on_small_map = 1;
      left_button_clicked = 0;
    }
    if ( do_left_map_click(x, y, curr_mx, curr_my, zoom)
      || do_right_map_click(x, y, curr_mx, curr_my, zoom)
      || do_left_map_drag(x, y, curr_mx, curr_my, zoom) )
      result = 1;
  } else
  {
    clicked_on_small_map = 0;
  }
  if (grabbed_small_map)
  {
    LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1, (MyScreenHeight/pixel_size) >> 1);
  }
  old_mx = curr_mx;
  old_my = curr_my;
  if (grabbed_small_map)
    game.small_map_state = 2;
  SYNCDBG(8,"Finished");
  return result;
}

short get_bookmark_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    for (int i = 0; i < BOOKMARKS_COUNT; i++)
    {
        struct Bookmark* bmark = &game.bookmark[i];
        int kcode = KC_1 + i;
        // Store bookmark check
        if (is_key_pressed(kcode, KMod_CONTROL))
        {
            clear_key_pressed(kcode);
            if (player->acamera != NULL)
            {
                bmark->x = player->acamera->mappos.x.stl.num;
                bmark->y = player->acamera->mappos.y.stl.num;
                bmark->flags |= 0x01;
                show_onscreen_msg(game_num_fps, "Bookmark %d stored", i + 1);
            }
            return true;
        }
        // Load bookmark check
        if (is_key_pressed(kcode, KMod_SHIFT))
        {
            clear_key_pressed(kcode);
            if ((bmark->flags & 0x01) != 0)
            {
                set_players_packet_action(player, PckA_BookmarkLoad, bmark->x, bmark->y, 0, 0);
                return true;
            }
        }
  }
  return false;
}

short zoom_shortcuts(void)
{
    for (int i = 0; i <= ZOOM_KEY_ROOMS_COUNT; i++)
    {
        int32_t val;
        if (is_game_key_pressed(Gkey_ZoomRoomTreasure + i, &val, false))
        {
            clear_key_pressed(val);
            go_to_my_next_room_of_type(zoom_key_room_order[i]);
            return true;
        }
  }
  return false;
}

/**
 * Handles minimap control inputs.
 * @return Returns true if packet was created, false otherwise.
 */
short get_minimap_control_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    short packet_made = false;
    if (is_key_pressed(KC_SUBTRACT, KMod_NONE))
    {
        if (menu_is_active(GMnu_MAIN))
        {
            fake_button_click(BID_MAP_ZOOM_OU);
        }
        if (player->minimap_zoom < 2048)
        {
            set_players_packet_action(player, PckA_SetMinimapConf, 2 * (long)player->minimap_zoom, 0, 0, 0);
            packet_made = true;
        }
        clear_key_pressed(KC_SUBTRACT);
        if (packet_made)
            return true;
  }
  if (is_key_pressed(KC_ADD,KMod_NONE))
  {
      if (menu_is_active(GMnu_MAIN))
      {
          fake_button_click(BID_MAP_ZOOM_IN);
      }
      if ( player->minimap_zoom > 128 )
      {
          set_players_packet_action(player, PckA_SetMinimapConf, player->minimap_zoom >> 1, 0, 0, 0);
          packet_made = true;
      }
      clear_key_pressed(KC_ADD);
      if (packet_made) return true;
  }
  return false;
}

/**
 * Handles screen control inputs.
 * @return Returns true if packet was created, false otherwise.
 */
short get_screen_control_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    short packet_made = false;
    if (is_key_pressed(KC_R, KMod_ALT))
    {
        set_players_packet_action(player, PckA_SwitchScrnRes, 0, 0, 0, 0);
        packet_made = true;
        clear_key_pressed(KC_R);
        if (packet_made)
            return true;
  }
  return false;
}

short get_global_inputs(void)
{
  if (game_is_busy_doing_gui_string_input())
    return false;
  struct PlayerInfo* player = get_my_player();
  int32_t keycode;
  if ((player->allocflags & PlaF_NewMPMessage) != 0)
  {
    get_players_message_inputs();
    return true;
  }
  if (((player->view_type == PVT_DungeonTop) || (player->view_type == PVT_CreatureContrl))
  && (((game.system_flags & GSF_NetworkActive) != 0) ||
     ((game.flags_gui & GGUI_SoloChatEnabled) != 0)))
  {
      if (is_key_pressed(KC_RETURN,KMod_NONE))
      {
          if (menu_is_active(GMnu_QUIT))
          {
              set_players_packet_action(player, PckA_QuitToMainMenu, 0, 0, 0, 0);
              clear_key_pressed(KC_RETURN);
              return true;
          }
        player->allocflags |= PlaF_NewMPMessage;
        clear_key_pressed(KC_RETURN);
        return true;
      }
  }
  // Code for debugging purposes
  if ( is_key_pressed(KC_D,KMod_ALT) )
  {
    JUSTMSG("REPORT. gameturn is %u, requested fps is %d",game.play_gameturn, game_num_fps);
  }

  for (int idx = KC_F1; idx <= KC_F8; idx++)
  {
      if ( is_key_pressed(idx,KMod_CONTROL) )
      {
        set_players_packet_action(player, PckA_PlyrFastMsg, idx-KC_F1, 0, 0, 0);
        clear_key_pressed(idx);
        return true;
      }
  }
  if ((player->instance_num != PI_MapFadeTo) &&
      (player->instance_num != PI_MapFadeFrom) &&
      (!game_is_busy_doing_gui_string_input()))
  {
      if ( is_game_key_pressed(Gkey_TogglePause, &keycode, false) )
      {
        long grab_check_flags = (((game.operation_flags & GOF_Paused) == 0) ? MG_OnPauseEnter : MG_OnPauseLeave);// the paused flag is currently set to the current pause state, not the state we are about to enter
        LbGrabMouseCheck(grab_check_flags);
        if (pause_music_when_game_paused())
        {
            // only pause music, rather than pause all audio, because otherwise announcer messages will be lost (it continues to play while muted, it needs a new feature)
            if (((grab_check_flags & MG_OnPauseEnter) != 0)) {
                pause_music();
            } else {
                resume_music();
            }
        }
        if (((grab_check_flags & MG_OnPauseEnter) != 0))
        {
            for (int i = 0; i < PLAYER_NEUTRAL; i++)
            {
                stop_thing_playing_sample(find_players_dungeon_heart(i), 93);
            }
        }
        set_packet_pause_toggle();
        clear_key_pressed(keycode);
        return true;
      }
      else if( flag_is_set(game.operation_flags, GOF_Paused) && flag_is_set(start_params.debug_flags, DFlg_FrameStep) )
      {
        if( is_key_pressed(KC_PERIOD, KMOD_NONE) )
        {
            game.frame_step = true;
            set_packet_pause_toggle();
            clear_key_pressed(KC_PERIOD);
        }
      }
  }
  if ((game.operation_flags & GOF_Paused) != 0)
      return true;
  if (get_speed_control_inputs())
      return true;
  if (get_minimap_control_inputs())
      return true;
  if (get_screen_control_inputs())
      return true;
  if (get_screen_capture_inputs())
      return true;
  if (is_key_pressed(KC_SPACE,KMod_NONE))
  {
      if (player->victory_state != VicS_Undecided)
      {
        if ( timer_enabled() )
        {
            update_time();
            struct GameTime GameT = get_game_time(game.play_gameturn, game_num_fps);
            SYNCMSG("Finished level %d. Total turns taken: %u (%02u:%02u:%02u at %d fps). Real time elapsed: %02u:%02u:%02u:%03u.",
                game.loaded_level_number, game.play_gameturn, GameT.Hours, GameT.Minutes, GameT.Seconds, game_num_fps, Timer.Hours, Timer.Minutes, Timer.Seconds, Timer.MSeconds);
        }
        set_players_packet_action(player, PckA_FinishGame, 0, 0, 0, 0);
        clear_key_pressed(KC_SPACE);
        return true;
      }
  }
  if ( is_game_key_pressed(Gkey_DumpToOldPos, &keycode, false) )
  {
      set_players_packet_action(player, PckA_DumpHeldThingToOldPos, 0, 0, 0, 0);
      clear_key_pressed(keycode);
  }
  if (is_key_pressed(KC_GRAVE, KMod_DONTCARE)) {
    debug_display_consolelog = !debug_display_consolelog;
    clear_key_pressed(KC_GRAVE);
  }
  return false;
}

TbBool get_level_lost_inputs(void)
{
    int32_t keycode;
    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_my_player();
    if ((player->allocflags & PlaF_NewMPMessage) != 0)
    {
      get_players_message_inputs();
      return true;
    }
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
      if (is_key_pressed(KC_RETURN,KMod_NONE))
      {
        player->allocflags |= PlaF_NewMPMessage;
        clear_key_pressed(KC_RETURN);
        return true;
      }
    }
    if (get_speed_control_inputs())
        return true;
    if (get_minimap_control_inputs())
        return true;
    if (get_screen_control_inputs())
        return true;
    if (get_screen_capture_inputs())
        return true;
    if (is_key_pressed(KC_SPACE,KMod_NONE))
    {
        set_players_packet_action(player, PckA_FinishGame, 0,0,0,0);
        clear_key_pressed(KC_SPACE);
    }
    if (player->view_type == PVT_MapScreen)
    {
        long mouse_x = GetMouseX();
        long mouse_y = GetMouseY();
        // Position on the parchment map on which we're doing action
        int32_t map_x;
        int32_t map_y;
        TbBool map_valid = point_to_overhead_map(get_local_camera(player->acamera), mouse_x / pixel_size, mouse_y / pixel_size, &map_x, &map_y);
        if (is_game_key_pressed(Gkey_SwitchToMap, &keycode, false))
        {
            lbKeyOn[keycode] = 0;
            zoom_from_parchment_map();
        } else
        if ( right_button_released )
        {
            right_button_released = 0;
            zoom_from_parchment_map();
        } else
        if ( left_button_released )
        {
            if  ( map_valid ) {
                MapSubtlCoord stl_x = coord_subtile(map_x);
                MapSubtlCoord stl_y = coord_subtile(map_y);
                set_players_packet_action(player, PckA_ZoomFromMap, stl_x, stl_y, 0, 0);
                left_button_released = 0;
            }
        }
    } else
    if (player->view_type == PVT_DungeonTop)
    {
      if (is_key_pressed(KC_TAB,KMod_DONTCARE))
      {
          if (
            player->view_mode == PVM_IsoWibbleView ||
            player->view_mode == PVM_FrontView ||
            player->view_mode == PVM_IsoStraightView
          ) {
            clear_key_pressed(KC_TAB);
            toggle_gui();
          }
      } else
      if (is_game_key_pressed(Gkey_SwitchToMap, &keycode, false))
      {
        lbKeyOn[keycode] = 0;
        if (player->view_mode != PVM_ParchFadeOut)
        {
          turn_off_all_window_menus();
          set_flag_value(game.operation_flags, GOF_ShowPanel, (game.operation_flags & GOF_ShowGui) != 0);
          if (((game.system_flags & GSF_NetworkActive) != 0)
            || (lbDisplay.PhysicalScreenWidth > 320))
          {
                if (toggle_status_menu(0))
                  set_flag(game.operation_flags, GOF_ShowPanel);
                else
                  clear_flag(game.operation_flags, GOF_ShowPanel);
                set_players_packet_action(player, PckA_SaveViewType, PVT_MapScreen, 0,0,0);
          } else
          {
                set_players_packet_action(player, PckA_SetViewType, PVT_MapFadeIn, 0,0,0);
          }
          turn_off_roaming_menus();
        }
      }
    }
    if (is_key_pressed(KC_ESCAPE,KMod_DONTCARE))
    {
      clear_key_pressed(KC_ESCAPE);
      if ( a_menu_window_is_active() )
      {
        turn_off_all_window_menus();
      }
      else
      {
          if (menu_is_active(GMnu_MAIN))
          {
            fake_button_click(BID_OPTIONS);
          }
        turn_on_menu(GMnu_OPTIONS);
      }
    }
    TbBool inp_done=false;
    switch (player->view_type)
    {
      case PVT_DungeonTop:
        inp_done = menu_is_active(GMnu_SPELL_LOST);
        if ( !inp_done )
        {
          if ((game.operation_flags & GOF_ShowGui) != 0)
          {
            initialise_tab_tags_and_menu(GMnu_SPELL);
            turn_off_all_panel_menus();
            turn_on_menu(GMnu_SPELL_LOST);
          }
        }
        inp_done = get_gui_inputs(1);
        if ( !inp_done )
        {
          if ( (player->work_state == PSt_CreatrInfo) || (player->work_state == PSt_CreatrInfoAll) )
          {
              set_player_instance(player, PI_UnqueryCrtr, 0);
          } else
          {
              int mm_units_per_px;
              {
                  int mnu_num;
                  mnu_num = menu_id_to_number(GMnu_MAIN);
                  struct GuiMenu *gmnu;
                  gmnu = get_active_menu(mnu_num);
                  mm_units_per_px = (gmnu->width * 16 + 140/2) / 140;
                  if (mm_units_per_px < 1)
                      mm_units_per_px = 1;
              }
              long mmzoom;
              if (16/mm_units_per_px < 3)
                  mmzoom = (player->minimap_zoom) / (3-16/mm_units_per_px);
              else
                  mmzoom = (player->minimap_zoom);
              inp_done = get_small_map_inputs(player->minimap_pos_x*mm_units_per_px/16, player->minimap_pos_y*mm_units_per_px/16, mmzoom);
              if ( !inp_done )
                get_bookmark_inputs();
              get_dungeon_control_nonaction_inputs();
          }
        }
        break;
      case PVT_CreatureContrl:
      {
          struct Thing* thing = thing_get(player->controlled_thing_idx);
          TRACE_THING(thing);
          if (thing->class_id == TCls_Creature)
          {
              struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
              if ((cctrl->creature_state_flags & TF2_Spectator) == 0)
              {
                  set_players_packet_action(player, PckA_DirectCtrlExit, player->controlled_thing_idx, 0, 0, 0);
                  inp_done = true;
              }
          } else
        {
          set_players_packet_action(player, PckA_DirectCtrlExit, player->controlled_thing_idx,0,0,0);
          inp_done = true;
        }
        break;
      }
      case PVT_CreaturePasngr:
        set_players_packet_action(player, PckA_PasngrCtrlExit, player->controlled_thing_idx,0,0,0);
        break;
      case PVT_MapScreen:
        if (menu_is_active(GMnu_SPELL_LOST))
        {
          if ((game.operation_flags & GOF_ShowGui) != 0)
            turn_off_menu(GMnu_SPELL_LOST);
        }
        break;
      default:
          break;
    }
    return inp_done;
}

short get_status_panel_keyboard_action_inputs(void)
{
  struct PlayerInfo* player = get_my_player();
  if ( (player->work_state == PSt_PlaceTerrain) || (player->work_state == PSt_MkDigger) || (player->work_state == PSt_MkGoodCreatr) || (player->work_state == PSt_MkBadCreatr) )
  {
      return false;
  }
  if (is_key_pressed(KC_1, KMod_NONE))
  {
    clear_key_pressed(KC_1);
    struct GuiButton* gbtn = get_gui_button(BID_QUERY_2);
    if (gbtn != NULL)
    {
        if (flag_is_set(gbtn->flags,(LbBtnF_Visible | LbBtnF_Enabled)))
        {
            if (menu_is_active(GMnu_QUERY))
            {
                gui_switch_players_visible(gbtn);
                fake_button_click(BID_QUERY_2);
            }
            else
            {
                fake_button_click(BID_INFO_TAB);
            }
        }
    }
    else
    {
        fake_button_click(BID_INFO_TAB);
    }
  }
  if (is_key_pressed(KC_2, KMod_NONE))
  {
    clear_key_pressed(KC_2);
    struct GuiButton *gbtn = get_gui_button(BID_ROOM_NXPG);
    if (gbtn != NULL)
    {
        if ((gbtn->flags & (LbBtnF_Visible|LbBtnF_Enabled)) != 0)
        {
            if (menu_is_active(GMnu_ROOM))
            {
                turn_off_menu(GMnu_ROOM);
                turn_on_menu(GMnu_ROOM2);
                fake_button_click(BID_ROOM_NXPG);
            }
            else if (menu_is_active(GMnu_ROOM2))
            {
                turn_off_menu(GMnu_ROOM2);
                turn_on_menu(GMnu_ROOM);
                fake_button_click(BID_ROOM_NXPG);
            }
            else
            {
                fake_button_click(BID_ROOM_TAB);
            }
        }
        else
        {
            fake_button_click(BID_ROOM_TAB);
        }
    }
    else
    {
        fake_button_click(BID_ROOM_TAB);
    }
  }
  if (is_key_pressed(KC_3, KMod_NONE))
  {
    clear_key_pressed(KC_3);
    struct GuiButton *gbtn = get_gui_button(BID_POWER_NXPG);
    if (gbtn != NULL)
    {
        if ((gbtn->flags & (LbBtnF_Visible|LbBtnF_Enabled)) != 0)
        {
            if (menu_is_active(GMnu_SPELL))
            {
                turn_off_menu(GMnu_SPELL);
                turn_on_menu(GMnu_SPELL2);
                fake_button_click(BID_POWER_NXPG);
            }
            else if (menu_is_active(GMnu_SPELL2))
            {
                turn_off_menu(GMnu_SPELL2);
                turn_on_menu(GMnu_SPELL);
                fake_button_click(BID_POWER_NXPG);
            }
            else
            {
                fake_button_click(BID_SPELL_TAB);
            }
        }
        else
        {
            fake_button_click(BID_SPELL_TAB);
        }
    }
    else
    {
        fake_button_click(BID_SPELL_TAB);
    }
  }
  if (is_key_pressed(KC_4, KMod_NONE))
  {
    clear_key_pressed(KC_4);
    struct GuiButton *gbtn = get_gui_button(BID_MNFCT_NXPG);
    if (gbtn != NULL)
    {
        if ((gbtn->flags & (LbBtnF_Visible|LbBtnF_Enabled)) != 0)
        {
            if (menu_is_active(GMnu_TRAP))
            {
                turn_off_menu(GMnu_TRAP);
                turn_on_menu(GMnu_TRAP2);
                fake_button_click(BID_MNFCT_NXPG);
            }
            else if (menu_is_active(GMnu_TRAP2))
            {
                turn_off_menu(GMnu_TRAP2);
                turn_on_menu(GMnu_TRAP);
                fake_button_click(BID_MNFCT_NXPG);
            }
            else
            {
                fake_button_click(BID_MNFCT_TAB);
            }
        }
        else
        {
            fake_button_click(BID_MNFCT_TAB);
        }
    }
    else
    {
        fake_button_click(BID_MNFCT_TAB);
    }
  }
  if (is_key_pressed(KC_5, KMod_NONE))
  {
    clear_key_pressed(KC_5);
    fake_button_click(BID_CREATR_TAB);
  }
  return false;
}

TbBool get_dungeon_control_pausable_action_inputs(void)
{
    int32_t val;
    struct PlayerInfo* player = get_my_player();
    if (get_players_packet_action(player) != PckA_None)
      return true;

    if (get_bookmark_inputs())
      return true;

    if (is_key_pressed(KC_F8, KMod_NONE))
    {
        clear_key_pressed(KC_F8);
        toggle_tooltips();
    }
    if (is_key_pressed(KC_NUMPADENTER,KMod_NONE))
    {
        if (close_instance_cheat_menu())
        {
            clear_key_pressed(KC_NUMPADENTER);
        }
        else
        if (toggle_main_cheat_menu())
        {
            clear_key_pressed(KC_NUMPADENTER);
        }
    }
    // also use the main keyboard enter key (while holding shift) for cheat menu
    if (is_key_pressed(KC_RETURN,KMod_SHIFT))
        {
            if (close_instance_cheat_menu())
            {
                clear_key_pressed(KC_RETURN);
            }
        else
            if (toggle_main_cheat_menu())
            {
                clear_key_pressed(KC_RETURN);
            }
        }
    if (is_key_pressed(KC_F12,KMod_DONTCARE))
    {
        // Note that we're using "close", not "toggle". Menu can't be opened here.
        if (close_creature_cheat_menu())
        {
            clear_key_pressed(KC_F12);
        }
    }
    if (player->view_mode == PVM_IsoWibbleView || player->view_mode == PVM_IsoStraightView)
    {
      if (is_key_pressed(KC_TAB, !KMod_CONTROL))
      {
          clear_key_pressed(KC_TAB);
      }
      if (is_key_pressed(KC_TAB, KMod_CONTROL))
      {
          clear_key_pressed(KC_TAB);
          toggle_gui();
      }
      // Middle mouse camera actions for IsometricView
      if (is_game_key_pressed(Gkey_SnapCamera, &val, true))
      {
          struct Camera* cam = &player->cameras[CamIV_Isometric];
          struct Packet* pckt = get_packet(my_player_number);
          int angle = cam->rotation_angle_x;
          if (key_modifiers & KMod_CONTROL)
          {
              if ((angle >= ANGLE_NORTH && angle < ANGLE_NORTHEAST) || angle == DEGREES_360)
              {
                  angle = ANGLE_NORTHEAST;
              }
              else if (angle >= ANGLE_NORTHEAST && angle < ANGLE_EAST)
              {
                  angle = ANGLE_EAST;
              }
              else if (angle >= ANGLE_EAST && angle < ANGLE_SOUTHEAST)
              {
                  angle = ANGLE_SOUTHEAST;
              }
              else if (angle >= ANGLE_SOUTHEAST && angle < ANGLE_SOUTH)
              {
                  angle = ANGLE_SOUTH;
              }
              else if (angle >= ANGLE_SOUTH && angle < ANGLE_SOUTHWEST)
              {
                  angle = ANGLE_SOUTHWEST;
              }
              else if (angle >= ANGLE_SOUTHWEST && angle < ANGLE_WEST)
              {
                  angle = ANGLE_WEST;
              }
              else if (angle >= ANGLE_WEST && angle < ANGLE_NORTHWEST)
              {
                  angle = ANGLE_NORTHWEST;
              }
              else if (angle >= ANGLE_NORTHWEST && angle < DEGREES_360)
              {
                  angle = ANGLE_NORTH;
              }
        }
        else if (key_modifiers & KMod_SHIFT)
        {
            if (angle > ANGLE_NORTH && angle <= ANGLE_NORTHEAST)
            {angle = DEGREES_360;}
            else if (angle > ANGLE_NORTHEAST && angle <= ANGLE_EAST)
            {angle = ANGLE_NORTHEAST;}
            else if (angle > ANGLE_EAST && angle <= ANGLE_SOUTHEAST)
            {angle = ANGLE_EAST;}
            else if (angle > ANGLE_SOUTHEAST && angle <= ANGLE_SOUTH)
            {angle = ANGLE_SOUTHEAST;}
            else if (angle > ANGLE_SOUTH && angle <= ANGLE_SOUTHWEST)
            {angle = ANGLE_SOUTH;}
            else if (angle > ANGLE_SOUTHWEST && angle <= ANGLE_WEST)
            {angle = ANGLE_SOUTHWEST;}
            else if (angle > ANGLE_WEST && angle <= ANGLE_NORTHWEST)
            {angle = ANGLE_WEST;}
            else if ((angle > ANGLE_NORTHWEST && angle <= DEGREES_360) || angle == ANGLE_NORTH)
            {angle = ANGLE_NORTHWEST;}
        }
        else if (angle == ANGLE_NORTH || angle == DEGREES_360)
        {
            (angle = ANGLE_SOUTH);
        }
        else if (angle == ANGLE_EAST)
        {
            (angle = ANGLE_WEST);
        }
        else if (angle == ANGLE_WEST)
        {
            (angle = ANGLE_EAST);
        }
        else
        {
            (angle = ANGLE_NORTH);
        }
        set_packet_action(pckt,PckA_SetMapRotation,angle,0,0,0);
        clear_key_pressed(val);
        return true;
      }
    }
    if (player->view_mode == PVM_FrontView)
    {
      if (is_key_pressed(KC_TAB, !KMod_CONTROL))
      {
          clear_key_pressed(KC_TAB);
      }
      if (is_key_pressed(KC_TAB, KMod_CONTROL))
      {
          clear_key_pressed(KC_TAB);
          toggle_gui();
      }
      // Middle mouse camera actions for FrontView
      if (is_game_key_pressed(Gkey_SnapCamera, &val, true))
      {
          struct Camera* cam = &player->cameras[CamIV_FrontView];
          struct Packet* pckt = get_packet(my_player_number);
          int angle = cam->rotation_angle_x;
          if (key_modifiers & KMod_CONTROL)
          {
              set_packet_control(pckt, PCtr_ViewRotateCW);
        }
        else if (key_modifiers & KMod_SHIFT)
        {
            set_packet_control(pckt, PCtr_ViewRotateCCW);
        }
        else
        {
            if (angle == ANGLE_NORTH || angle == DEGREES_360)
            {
                (angle = ANGLE_SOUTH);
            }
            else
            {
                (angle = ANGLE_NORTH);
            }
        set_packet_action(pckt,PckA_SetMapRotation,angle,0,0,0);
        }
        clear_key_pressed(val);
        return true;
      }
    }

    if ((player->work_state == PSt_PlaceTerrain) || (player->work_state == PSt_MkDigger) || (player->work_state == PSt_MkBadCreatr) || (player->work_state == PSt_MkGoodCreatr)
        || (player->work_state == PSt_KillPlayer) || (player->work_state == PSt_HeartHealth) || (player->work_state == PSt_StealRoom) ||
        (player->work_state == PSt_StealSlab) || (player->work_state == PSt_ConvertCreatr))
    {
        process_cheat_mode_selection_inputs();
    }
    if (is_game_key_pressed(Gkey_SwitchToMap, &val, false))
    {
      clear_key_pressed(val);
      if ((player->view_mode != PVM_ParchFadeOut) && (game.small_map_state != 2))
      {
          turn_off_all_window_menus();
          zoom_to_parchment_map();
      }
      return true;
    }
    if (is_key_pressed(KC_F, KMod_ALT))
    {
        clear_key_pressed(KC_F);
        toggle_hero_health_flowers();
    }
    return false;
}

TbBool get_dungeon_control_action_inputs(void)
{
    int32_t val;
    struct PlayerInfo* player = get_my_player();
    if (get_players_packet_action(player) != PckA_None)
        return true;
    int mm_units_per_px;
    {
        int mnu_num = menu_id_to_number(GMnu_MAIN);
        struct GuiMenu* gmnu = get_active_menu(mnu_num);
        mm_units_per_px = (gmnu->width * 16 + 140 / 2) / 140;
        if (mm_units_per_px < 1)
        {
            mm_units_per_px = 1;
        }
    }
    long mmzoom;
    if (16 / mm_units_per_px < 3)
    {
        mmzoom = (player->minimap_zoom) / scale_value_for_resolution_with_upp(2, mm_units_per_px);
    }
    else
        mmzoom = (player->minimap_zoom);
    if (get_small_map_inputs(player->minimap_pos_x * mm_units_per_px / 16, player->minimap_pos_y * mm_units_per_px / 16, mmzoom))
        return 1;

    if (player->work_state == PSt_CtrlDungeon)
    {
        if ((player->primary_cursor_state == CSt_PickAxe) || (player->primary_cursor_state == CSt_PowerHand))
        {
            process_highlight_roomspace_inputs(player->id_number);
        }
    }
    else if (player->work_state == PSt_BuildRoom)
    {
        process_build_roomspace_inputs(player->id_number);
    }
    else if (player->work_state == PSt_Sell)
    {
        process_sell_roomspace_inputs(player->id_number);
    }

    // Zooming cannot be done paused because it's a player instance.
    if (is_game_key_pressed(Gkey_ZoomToFight, &val, false))
    {
        clear_key_pressed(val);
        zoom_to_fight(player->id_number);
        return true;
    }
    if (is_game_key_pressed(Gkey_ZoomCrAnnoyed, &val, false))
    {
        clear_key_pressed(val);
        zoom_to_next_annoyed_creature();
        return true;
    }
    if (zoom_shortcuts())
    {
        return true;
    }
    get_status_panel_keyboard_action_inputs();
    return false;
}

short get_creature_passenger_action_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    if (get_players_packet_action(player) != PckA_None)
        return 1;
    if (((game.operation_flags & GOF_Paused) == 0) || ((game.operation_flags & GOF_WorldInfluence) != 0))
        get_gui_inputs(1);
    if (player->controlled_thing_idx == 0)
        return false;
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    TRACE_THING(thing);
    if (thing_is_creature(thing))
    {
        if (menu_is_active(GMnu_CREATURE_QUERY1))
        {
          if (wheel_scrolled_down)
          {
            turn_off_menu(GMnu_CREATURE_QUERY1);
            turn_on_menu(GMnu_CREATURE_QUERY2);
            fake_button_click(0);
            update_wheel_scrolled();
          }
          else if (wheel_scrolled_up)
          {
            turn_off_menu(GMnu_CREATURE_QUERY1);
            turn_on_menu(GMnu_CREATURE_QUERY4);
            fake_button_click(0);
            update_wheel_scrolled();
          }
        }
        else if (menu_is_active(GMnu_CREATURE_QUERY2))
        {
          if (wheel_scrolled_down)
          {
            turn_off_menu(GMnu_CREATURE_QUERY2);
            turn_on_menu(GMnu_CREATURE_QUERY3);
            fake_button_click(0);
            update_wheel_scrolled();
          }
          else if (wheel_scrolled_up)
          {
            turn_off_menu(GMnu_CREATURE_QUERY2);
            turn_on_menu(GMnu_CREATURE_QUERY1);
            fake_button_click(0);
            update_wheel_scrolled();
          }
        }
        else if (menu_is_active(GMnu_CREATURE_QUERY3))
        {
          if (wheel_scrolled_down)
          {
            turn_off_menu(GMnu_CREATURE_QUERY3);
            turn_on_menu(GMnu_CREATURE_QUERY4);
            fake_button_click(0);
            update_wheel_scrolled();
          }
          else if (wheel_scrolled_up)
          {
            turn_off_menu(GMnu_CREATURE_QUERY3);
            turn_on_menu(GMnu_CREATURE_QUERY2);
            fake_button_click(0);
            update_wheel_scrolled();
          }
        }
        else if (menu_is_active(GMnu_CREATURE_QUERY4))
        {
          if (wheel_scrolled_down)
          {
            turn_off_menu(GMnu_CREATURE_QUERY4);
            turn_on_menu(GMnu_CREATURE_QUERY1);
            fake_button_click(0);
            update_wheel_scrolled();
          }
          else if (wheel_scrolled_up)
          {
            turn_off_menu(GMnu_CREATURE_QUERY4);
            turn_on_menu(GMnu_CREATURE_QUERY3);
            fake_button_click(0);
            update_wheel_scrolled();
          }
        }
    }
    if (right_button_released)
    {
        set_players_packet_action(player, PckA_PasngrCtrlExit, player->controlled_thing_idx, 0, 0, 0);
        return true;
    }
    if (!thing_exists(thing) || (player->controlled_thing_creatrn != thing->creation_turn))
    {
        set_players_packet_action(player, PckA_PasngrCtrlExit, player->controlled_thing_idx,0,0,0);
        return true;
    }
    if (is_key_pressed(KC_TAB, KMod_CONTROL))
    {
        clear_key_pressed(KC_TAB);
        toggle_gui();
    }
    return false;
}

static void set_possession_instance(struct PlayerInfo* player, struct Thing* thing, int direction)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);

    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    int current_pos = 0;
    for (int pos = 0; pos < LEARNED_INSTANCES_COUNT; pos++)
    {
        if (cctrl->active_instance_id == crconf->learned_instance_id[pos])
        {
            current_pos = pos;
            break;
        }
    }

    int final_pos = -1;
    for (int i = 0; i < LEARNED_INSTANCES_COUNT; i++)
    {
        int pos = current_pos + direction * (i + 1);
        
        pos += LEARNED_INSTANCES_COUNT;
        pos %= LEARNED_INSTANCES_COUNT;
        
        if (creature_instance_is_available(thing, crconf->learned_instance_id[pos]))
        {
            int inst_id = crconf->learned_instance_id[pos];
            final_pos = pos;
            set_players_packet_action(player, PckA_CtrlCrtrSetInstnc, inst_id, 1, 0, 0);
            break;
        }
    }


    if (menu_is_active(GMnu_CREATURE_QUERY1) && final_pos > 5)
    {
        turn_off_menu(GMnu_CREATURE_QUERY1);
        turn_on_menu(GMnu_CREATURE_QUERY2);
    }
    else if (menu_is_active(GMnu_CREATURE_QUERY2) && final_pos <= 5)
    {
        turn_off_menu(GMnu_CREATURE_QUERY2);
        turn_on_menu(GMnu_CREATURE_QUERY1);
    }
}

short get_creature_control_action_inputs(void)
{
    int32_t keycode;
    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_my_player();
    if (get_players_packet_action(player) != PckA_None)
        return 1;
    if ( ((game.operation_flags & GOF_Paused) == 0) || ((game.operation_flags & GOF_WorldInfluence) != 0))
        get_gui_inputs(1);
    if (is_key_pressed(KC_NUMPADENTER,KMod_DONTCARE))
    {
        // Note that we're using "close", not "toggle". Menu can't be opened here.
        if (close_main_cheat_menu())
        {
            clear_key_pressed(KC_NUMPADENTER);
        } else
        if (toggle_instance_cheat_menu())
        {
            clear_key_pressed(KC_NUMPADENTER);
        }
    }
    // also use the main keyboard enter key (while holding shift) for cheat menu
    if (is_key_pressed(KC_RETURN,KMod_SHIFT))
    {
        // Note that we're using "close", not "toggle". Menu can't be opened here.
        if (close_main_cheat_menu())
        {
            clear_key_pressed(KC_RETURN);
        }
        else
        {
            toggle_instance_cheat_menu();
            clear_key_pressed(KC_RETURN);
        }
    }
    if (is_key_pressed(KC_F12,KMod_DONTCARE))
    {
        toggle_creature_cheat_menu();
        clear_key_pressed(KC_F12);
    }
    if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
    {
        if (a_menu_window_is_active())
        {
            clear_key_pressed(KC_ESCAPE);
            turn_off_all_window_menus();
        }
    }
    if (is_key_pressed(KC_ESCAPE, KMod_SHIFT))
    {
        clear_key_pressed(KC_ESCAPE);
        if (menu_is_active(GMnu_MAIN))
        {
            fake_button_click(BID_OPTIONS);
        }
        turn_on_menu(GMnu_OPTIONS);
    }
    if (player->controlled_thing_idx != 0)
    {
        short make_packet = right_button_released || is_key_pressed(KC_ESCAPE, KMod_DONTCARE);
        struct Thing* thing = thing_get(player->controlled_thing_idx);
        if (!make_packet)
        {
            TRACE_THING(thing);
            if ((player->controlled_thing_creatrn != thing->creation_turn) || (!flag_is_set(thing->alloc_flags, TAlF_Exists)) || (thing->active_state == CrSt_CreatureUnconscious))
            {
                make_packet = true;
            }
        }
        if (make_packet)
        {
            right_button_released = 0;
            clear_key_pressed(KC_ESCAPE);
            if ((player->possession_lock == true) && thing_is_creature(thing))
            {
                if (is_my_player(player))
                    play_non_3d_sample(119); //refusal
            }
            else
            {
                set_players_packet_action(player, PckA_DirectCtrlExit, player->controlled_thing_idx, 0, 0, 0);
            }
        }
    }
    // Use the Query/Message keys and mouse wheel to scroll through query pages and go to correct query page when selecting an instance.
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    if (!thing_is_creature(thing))
    {
        return false;
    }
    if (menu_is_active(GMnu_CREATURE_QUERY1))
    {
      if ( ( (is_key_pressed(KC_7,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD7,KMod_DONTCARE))) && (creature_instance_get_available_id_for_pos(thing,6) > 0) ) ||
           ( (is_key_pressed(KC_8,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD8,KMod_DONTCARE))) && (creature_instance_get_available_id_for_pos(thing,7) > 0) ) ||
           ( (is_key_pressed(KC_9,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD9,KMod_DONTCARE))) && (creature_instance_get_available_id_for_pos(thing,8) > 0) ) ||
           ( (is_key_pressed(KC_0,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD0,KMod_DONTCARE))) && (creature_instance_get_available_id_for_pos(thing,9) > 0) )  )
      {
        turn_off_menu(GMnu_CREATURE_QUERY1);
        turn_on_menu(GMnu_CREATURE_QUERY2);
      }
      if (is_game_key_pressed(Gkey_ToggleMessage, &keycode, false) || wheel_scrolled_down)
      {
        turn_off_menu(GMnu_CREATURE_QUERY1);
        if (creature_instance_get_available_id_for_pos(thing,6) > 0)
        {
            turn_on_menu(GMnu_CREATURE_QUERY2);
        } else
        {
            turn_on_menu(GMnu_CREATURE_QUERY3);
        }
        clear_key_pressed(keycode);
        fake_button_click(0);
        update_wheel_scrolled();
      }
      if (is_game_key_pressed(Gkey_CrtrQueryMod, &keycode, false) || wheel_scrolled_up)
      {
        turn_off_menu(GMnu_CREATURE_QUERY1);
        turn_on_menu(GMnu_CREATURE_QUERY4);
        clear_key_pressed(keycode);
        fake_button_click(0);
        update_wheel_scrolled();
      }
    }
    if (menu_is_active(GMnu_CREATURE_QUERY2))
    {
      if ( ( (is_key_pressed(KC_1,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD1,KMod_DONTCARE))) ) ||
           ( (is_key_pressed(KC_2,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD2,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,1) > 0) ) ||
           ( (is_key_pressed(KC_3,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD3,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,2) > 0) ) ||
           ( (is_key_pressed(KC_4,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD4,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,3) > 0) )  )
      {
        turn_off_menu(GMnu_CREATURE_QUERY2);
        turn_on_menu(GMnu_CREATURE_QUERY1);
      }
      if (is_game_key_pressed(Gkey_ToggleMessage, &keycode, false) || wheel_scrolled_down)
      {
        turn_off_menu(GMnu_CREATURE_QUERY2);
        turn_on_menu(GMnu_CREATURE_QUERY3);
        clear_key_pressed(keycode);
        fake_button_click(0);
        update_wheel_scrolled();
      }
      if (is_game_key_pressed(Gkey_CrtrQueryMod, &keycode, false) || wheel_scrolled_up)
      {
        turn_off_menu(GMnu_CREATURE_QUERY2);
        turn_on_menu(GMnu_CREATURE_QUERY1);
        clear_key_pressed(keycode);
        fake_button_click(0);
        update_wheel_scrolled();
      }
    }
    if (menu_is_active(GMnu_CREATURE_QUERY3))
    {
      if ( ( (is_key_pressed(KC_1,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD1,KMod_DONTCARE))) ) ||
           ( (is_key_pressed(KC_2,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD2,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,1) > 0) ) ||
           ( (is_key_pressed(KC_3,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD3,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,2) > 0) ) ||
           ( (is_key_pressed(KC_4,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD4,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,3) > 0) )  )
      {
        turn_off_menu(GMnu_CREATURE_QUERY3);
        turn_on_menu(GMnu_CREATURE_QUERY1);
      }
      if ( ( (is_key_pressed(KC_7,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD7,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,6) > 0) ) ||
           ( (is_key_pressed(KC_8,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD8,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,7) > 0) ) ||
           ( (is_key_pressed(KC_9,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD9,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,8) > 0) ) ||
           ( (is_key_pressed(KC_0,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD0,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,9) > 0) )  )
      {
        turn_off_menu(GMnu_CREATURE_QUERY3);
        turn_on_menu(GMnu_CREATURE_QUERY2);
      }
      if (is_game_key_pressed(Gkey_ToggleMessage, &keycode, false) || wheel_scrolled_down)
      {
        turn_off_menu(GMnu_CREATURE_QUERY3);
        turn_on_menu(GMnu_CREATURE_QUERY4);
        clear_key_pressed(keycode);
        fake_button_click(0);
        update_wheel_scrolled();
      }
      if (is_game_key_pressed(Gkey_CrtrQueryMod, &keycode, false) || wheel_scrolled_up)
      {
        turn_off_menu(GMnu_CREATURE_QUERY3);
        if (creature_instance_get_available_id_for_pos(thing,6) > 0)
        {
            turn_on_menu(GMnu_CREATURE_QUERY2);
        } else
        {
            turn_on_menu(GMnu_CREATURE_QUERY1);
        }
        clear_key_pressed(keycode);
        fake_button_click(0);
        update_wheel_scrolled();
      }
    }
    if (menu_is_active(GMnu_CREATURE_QUERY4))
    {
      if ( ( (is_key_pressed(KC_1,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD1,KMod_DONTCARE))) ) ||
           ( (is_key_pressed(KC_2,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD2,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,1) > 0) ) ||
           ( (is_key_pressed(KC_3,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD3,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,2) > 0) ) ||
           ( (is_key_pressed(KC_4,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD4,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,3) > 0) )  )
      {
        turn_off_menu(GMnu_CREATURE_QUERY4);
        turn_on_menu(GMnu_CREATURE_QUERY1);
      }
      if ( ( (is_key_pressed(KC_7,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD7,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,6) > 0) ) ||
           ( (is_key_pressed(KC_8,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD8,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,7) > 0) ) ||
           ( (is_key_pressed(KC_9,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD9,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,8) > 0) ) ||
           ( (is_key_pressed(KC_0,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD0,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,9) > 0) )  )
      {
        turn_off_menu(GMnu_CREATURE_QUERY4);
        turn_on_menu(GMnu_CREATURE_QUERY2);
      }
      if (is_game_key_pressed(Gkey_ToggleMessage, &keycode, false) || wheel_scrolled_down)
      {
        turn_off_menu(GMnu_CREATURE_QUERY4);
        turn_on_menu(GMnu_CREATURE_QUERY1);
        clear_key_pressed(keycode);
        fake_button_click(0);
        update_wheel_scrolled();
      }
      if (is_game_key_pressed(Gkey_CrtrQueryMod, &keycode, false) || wheel_scrolled_up)
      {
        turn_off_menu(GMnu_CREATURE_QUERY4);
        turn_on_menu(GMnu_CREATURE_QUERY3);
        clear_key_pressed(keycode);
        fake_button_click(0);
        update_wheel_scrolled();
      }
    }

    if (is_key_pressed(KC_TAB, !KMod_CONTROL))
    {
        clear_key_pressed(KC_TAB);
    }
    if (is_key_pressed(KC_TAB, KMod_CONTROL))
    {
        clear_key_pressed(KC_TAB);
        toggle_gui();
    }
    int numkey = -1;
    for (keycode=KC_1; keycode <= KC_0; keycode++)
    {
        if (is_key_pressed(keycode,KMod_DONTCARE))
        {
            clear_key_pressed(keycode);
            numkey = keycode-KC_1;
            break;
        }
    }
    if (numkey == -1)
    {
        if (is_key_pressed(KC_NUMPAD1,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD1);
            numkey = 0;
        } else
        if (is_key_pressed(KC_NUMPAD2,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD2);
            numkey = 1;
        } else
        if (is_key_pressed(KC_NUMPAD3,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD3);
            numkey = 2;
        } else
        if (is_key_pressed(KC_NUMPAD4,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD4);
            numkey = 3;
        } else
        if (is_key_pressed(KC_NUMPAD5,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD5);
            numkey = 4;
        } else
        if (is_key_pressed(KC_NUMPAD6,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD6);
            numkey = 5;
        } else
        if (is_key_pressed(KC_NUMPAD7,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD7);
            numkey = 6;
        } else
        if (is_key_pressed(KC_NUMPAD8,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD8);
            numkey = 7;
        } else
        if (is_key_pressed(KC_NUMPAD9,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD9);
            numkey = 8;
        } else
        if (is_key_pressed(KC_NUMPAD0,KMod_DONTCARE))
        {
            clear_key_pressed(KC_NUMPAD0);
            numkey = 9;
        }
    }
        int32_t val;
        TextStringId StrID = 0;
        for (int i = 0; i <= 15; i++)
        {
            if (is_game_key_pressed(Gkey_ZoomRoomTreasure + i, &val, false))
            {
                clear_key_pressed(val);
                set_players_packet_action(player, PckA_SwitchTeleportDest, i, 0, 0, 0);
                struct RoomConfigStats* roomst = get_room_kind_stats(zoom_key_room_order[i]);
                StrID = roomst->name_stridx;
            }

        }
        if (is_game_key_pressed(Gkey_ZoomToFight, &val, false))
        {
            clear_key_pressed(val);
            set_players_packet_action(player, PckA_SwitchTeleportDest, 16, 0, 0, 0);
            StrID = 567;
        }
        else if (is_key_pressed(KC_SEMICOLON,KMod_DONTCARE))
        {
            clear_key_pressed(KC_SEMICOLON);
            set_players_packet_action(player, PckA_SwitchTeleportDest, 17, 0, 0, 0);; // Last work room
        }
        else if (is_key_pressed(KC_SLASH,KMod_DONTCARE))
        {
            clear_key_pressed(KC_SLASH);
            set_players_packet_action(player, PckA_SwitchTeleportDest, 18, 0, 0, 0);; // Call to Arms
            struct PowerConfigStats *powerst = get_power_model_stats(PwrK_CALL2ARMS);
            StrID = powerst->name_stridx;
        }
        else if (is_key_pressed(KC_COMMA,KMod_DONTCARE))
        {
            clear_key_pressed(KC_COMMA);
            set_players_packet_action(player, PckA_SwitchTeleportDest, 19, 0, 0, 0); // default behaviour
            StrID = 609;
        }
        struct Thing* creatng = thing_get(player->controlled_thing_idx);
        if ( (StrID != 0) && (creature_instance_is_available(creatng, CrInst_TELEPORT)) )
        {
            if (game.active_messages_count > 0)
            {
                clear_messages_from_player(MsgType_CreatureInstance, CrInst_TELEPORT);
            }
            message_add(MsgType_CreatureInstance, CrInst_TELEPORT, get_string(StrID));
        }
        if (is_game_key_pressed(Gkey_CrtrContrlMod, &val, false))
        {
            if (!player->first_person_dig_claim_mode)
            {
                set_players_packet_action(player, PckA_SetFirstPersonDigMode, true, 0, 0, 0);
            }
        }
        else
        {
            if (player->first_person_dig_claim_mode)
            {
                set_players_packet_action(player, PckA_SetFirstPersonDigMode, false, 0, 0, 0);
            }
        }
        if (is_key_pressed(KC_LALT,KMod_DONTCARE))
        {
            if (!player->nearest_teleport)
            {
                set_players_packet_action(player, PckA_SetNearestTeleport, true, 0, 0, 0);
            }
        }
        else
        {
            if (player->nearest_teleport)
            {
                set_players_packet_action(player, PckA_SetNearestTeleport, false, 0, 0, 0);
            }
        }
        player->thing_under_hand = 0;
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (cctrl->active_instance_id == CrInst_FIRST_PERSON_DIG)
        {
            if (is_game_key_pressed(Gkey_SellTrapOnSubtile, &val, true))
            {
                first_person_see_item_desc = true;
            }
            else
            {
                first_person_see_item_desc = false;
            }
            struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
            if (thing_is_trap_crate(dragtng))
            {
                struct Thing* traptng = controlled_get_trap_to_rearm(thing);
                if (!thing_is_invalid(traptng))
                {
                    player->thing_under_hand = traptng->index;
                }
            }
            else if (!thing_exists(dragtng))
            {
                struct ShotConfigStats* shotst = get_shot_model_stats(ShM_Dig);
                TbBool diggable_subtile = false;
                MapSubtlCoord stl_x = thing->mappos.x.stl.num;
                MapSubtlCoord stl_y = thing->mappos.y.stl.num;
                for (unsigned char range = 0; range < shotst->health; range++)
                {
                    controlled_continue_looking_excluding_diagonal(thing, &stl_x, &stl_y);
                    diggable_subtile = subtile_is_diggable_for_player(thing->owner, stl_x, stl_y, true);
                    if (diggable_subtile)
                    {
                        break;
                    }
                }
                if (!diggable_subtile)
                {
                    struct Thing* picktng = controlled_get_thing_to_pick_up(thing);
                    if (!thing_is_invalid(picktng))
                    {
                        player->thing_under_hand = picktng->index;
                        if (first_person_see_item_desc)
                        {
                            display_controlled_pick_up_thing_name(picktng, 1, player->id_number);
                        }
                    }
                }
            }
            if (player->selected_fp_thing_pickup != player->thing_under_hand)
            {
                set_players_packet_action(player, PckA_SelectFPPickup, player->thing_under_hand, 0, 0, 0);
            }
        }
        if (!creature_under_spell_effect(thing, CSAfF_Chicken))
        {
            if (numkey != -1)
            {
                int num_avail = 0;
                for (int idx = 0; idx < LEARNED_INSTANCES_COUNT; idx++)
                {
                    struct Thing* cthing = thing_get(player->controlled_thing_idx);
                    TRACE_THING(cthing);
                    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(cthing);
                    int inst_id = crconf->learned_instance_id[idx];
                    if (creature_instance_is_available(cthing, inst_id))
                    {
                        if (numkey == num_avail)
                        {
                            set_players_packet_action(player, PckA_CtrlCrtrSetInstnc, inst_id, 0, 0, 0);
                            break;
                        }
                        num_avail++;
                    }
                }
            }
            
            // Next/Previous instance switching
            if (menu_is_active(GMnu_CREATURE_QUERY1) || menu_is_active(GMnu_CREATURE_QUERY2))
            {
                struct Thing* cthing = thing_get(player->controlled_thing_idx);
                
                if (is_key_pressed(KC_GAMEPAD_RIGHTSHOULDER, KMod_DONTCARE))
                {
                    clear_key_pressed(KC_GAMEPAD_RIGHTSHOULDER);
                    set_possession_instance(player, cthing, 1);
                    
                }
                else if (is_key_pressed(KC_GAMEPAD_LEFTSHOULDER, KMod_DONTCARE))
                {
                    clear_key_pressed(KC_GAMEPAD_LEFTSHOULDER);
                    set_possession_instance(player, cthing, -1);
                }
            }
        }
    return false;
}

void get_packet_control_mouse_clicks(void)
{
    SYNCDBG(8,"Starting");

    if (flag_is_set(game.operation_flags, GOF_Paused))
    {
        return;
    }

    struct PlayerInfo* player = get_my_player();

    if ( left_button_held || synthetic_left == 1)
    {
      set_players_packet_control(player, PCtr_LBtnHeld);
      synthetic_left = 2;
    }

    if ( right_button_held || synthetic_right == 1 )
    {
      set_players_packet_control(player, PCtr_RBtnHeld);
      synthetic_right = 2;
    }

    if ( left_button_clicked || last_speech_event.type == KS_HAND_CHOOSE )
    {
      set_players_packet_control(player, PCtr_LBtnClick);

      if ( last_speech_event.type == KS_HAND_CHOOSE ) {
        synthetic_left = 1;
      }
      else {
        synthetic_left = 0; //good idea to cancel current pick up, mouse takes precedence
      }
    }

    if ( right_button_clicked || last_speech_event.type == KS_HAND_ACTION )
    {
      set_players_packet_control(player, PCtr_RBtnClick);

      if ( last_speech_event.type == KS_HAND_ACTION ) {
        synthetic_right = 1;
      }
      else {
        synthetic_right = 0; //good idea to cancel current slap
      }
    }

    if ( left_button_released || synthetic_left == 3)
    {
      set_players_packet_control(player, PCtr_LBtnRelease);
      synthetic_left = 0;
    }

    if ( right_button_released || synthetic_right == 3 )
    {
      set_players_packet_control(player, PCtr_RBtnRelease);
      synthetic_right = 0;
    }

    if (synthetic_right == 2) {
        synthetic_right = 3;
    }
    if (synthetic_left == 2) {
        synthetic_left = 3;
    }
}

short get_map_action_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    long mouse_x = GetMouseX();
    long mouse_y = GetMouseY();
    // Get map coordinates from mouse position on parchment screen
    int32_t map_x;
    int32_t map_y;
    TbBool map_valid = point_to_overhead_map(get_local_camera(player->acamera), mouse_x / pixel_size, mouse_y / pixel_size, &map_x, &map_y);
    if  (map_valid)
    {
        MapSubtlCoord stl_x = coord_subtile(map_x);
        MapSubtlCoord stl_y = coord_subtile(map_y);
        if (left_button_clicked) {
            left_button_clicked = 0;
        }
        if (left_button_released) {
            left_button_released = 0;
            set_players_packet_action(player, PckA_ZoomFromMap, stl_x, stl_y, 0, 0);
            return true;
        }
    }

    if (right_button_clicked) {
        right_button_clicked = 0;
    }
    if (right_button_released) {
        right_button_released = 0;
        zoom_from_parchment_map();
        return true;
    }
    if (get_players_packet_action(player) != PckA_None)
        return true;
    {
      if (is_key_pressed(KC_F8,KMod_NONE))
      {
          toggle_tooltips();
          clear_key_pressed(KC_F8);
      }
      if (is_key_pressed(KC_NUMPADENTER,KMod_NONE))
      {
          if (toggle_main_cheat_menu())
            clear_key_pressed(KC_NUMPADENTER);
      }
      // also use the main keyboard enter key (while holding shift) for cheat menu
      if (is_key_pressed(KC_RETURN,KMod_SHIFT))
      {
          if (toggle_main_cheat_menu())
            clear_key_pressed(KC_RETURN);
      }
      int32_t keycode;
      if (is_game_key_pressed(Gkey_SwitchToMap, &keycode, false))
      {
          clear_key_pressed(keycode);
          turn_off_all_window_menus();
          zoom_from_parchment_map();
          return true;
      }
      return false;
    }
}

// TODO: Might want to initiate this in main() and pass a reference to it
// rather than using this global variable. But this works.
int global_frameskipTurn = 0;

void get_isometric_or_front_view_mouse_inputs(struct Packet *pckt,int rotate_pressed,TbBool mods_used)
{
    // Reserve the scroll wheel for the resurrect and transfer creature specials
    if ((menu_is_active(GMnu_RESURRECT_CREATURE) || menu_is_active(GMnu_TRANSFER_CREATURE) || rotate_pressed || mods_used) == 0)
    {
        // mouse scroll zoom unaffected by frameskip
        if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
        {
            if (wheel_scrolled_up)
            {
                set_packet_control(pckt, PCtr_ViewZoomIn);
            }
            if (wheel_scrolled_down)
            {
                set_packet_control(pckt, PCtr_ViewZoomOut);
            }
        }
    }
    if (menu_is_active(GMnu_BATTLE) && (rotate_pressed))
    {
        if (wheel_scrolled_up)
        {
            gui_previous_battle(0);
        }
        if (wheel_scrolled_down)
        {
            gui_next_battle(0);
        }
    }
    // Only pan the camera as often as normal despite frameskip
    if (game.frame_skip > 0)
    {
        int frameskipMax = 1;
        if (game.frame_skip < 4)
        {
            frameskipMax = game.frame_skip;
        }
        else if (game.frame_skip == 4)
        {
            frameskipMax = 3;
        }
        else if (game.frame_skip > 20 && game.frame_skip < 50)
        {
            frameskipMax = (game.frame_skip) / ( log(game.frame_skip) / log(17) );
        }
        else if (game.frame_skip < 200)
        {
            frameskipMax = game.frame_skip / ( log(game.frame_skip) / log(10) );
        }
        else if (game.frame_skip == 512) // max frameskip
        {
            frameskipMax = 60;
        }
        else // more than 200 but less than 512
        {
            frameskipMax = game.frame_skip / ( log(game.frame_skip) / log(4) );
        }
        TbBool moveTheCamera = (global_frameskipTurn == 0);
        //Checking for evenly distributed camera movement for the various frameskip amounts
        //JUSTMSG("moveTheCamera: %d", moveTheCamera);
        global_frameskipTurn++;
        if (global_frameskipTurn > frameskipMax) global_frameskipTurn = 0;
        if (!moveTheCamera) return;
    }
    // Camera Panning : mouse at window edge scrolling feature
    if (!LbIsMouseActive())
    {
        return; // don't pan the camera if the mouse has left the window
    }
    if (is_feature_on(Ft_DisableCursorCameraPanning) == false)
    {
        long mx = my_mouse_x;
        long my = my_mouse_y;
        long edge_scrolling_border = max(4, scale_fixed_DK_value(4));
        if (mx <= edge_scrolling_border)
        {
            if ( is_game_key_pressed(Gkey_MoveLeft, NULL, false) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
            {
              if (!rotate_pressed)
                pckt->additional_packet_values |= PCAdV_SpeedupPressed;
            }
            movement_accum_x = -1.0f;
        }
        if (mx >= MyScreenWidth-edge_scrolling_border)
        {
            if ( is_game_key_pressed(Gkey_MoveRight, NULL, false) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
            {
              if (!rotate_pressed)
                pckt->additional_packet_values |= PCAdV_SpeedupPressed;
            }
            movement_accum_x = 1.0f;
        }
        if (my <= edge_scrolling_border)
        {
            if ( is_game_key_pressed(Gkey_MoveUp, NULL, false) || is_key_pressed(KC_UP,KMod_DONTCARE) )
            {
              if (!rotate_pressed)
                pckt->additional_packet_values |= PCAdV_SpeedupPressed;
            }
            movement_accum_y = -1.0f;
        }
        if (my >= MyScreenHeight-edge_scrolling_border)
        {
            if ( is_game_key_pressed(Gkey_MoveDown, NULL, false) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
            {
              if (!rotate_pressed)
                pckt->additional_packet_values |= PCAdV_SpeedupPressed;
            }
            movement_accum_y = 1.0f;
        }
    }
}

void get_isometric_view_nonaction_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    struct Packet* packet = get_packet(my_player_number);
    int rotate_pressed = is_game_key_pressed(Gkey_RotateMod, NULL, true);
    int speed_pressed = is_game_key_pressed(Gkey_SpeedMod, NULL, true);
    if ((player->allocflags & PlaF_KeyboardInputDisabled) != 0)
      return;
    if (speed_pressed != 0)
        packet->additional_packet_values |= PCAdV_SpeedupPressed;
    TbBool no_mods = ((rotate_pressed != 0) || (speed_pressed != 0) || (check_current_gui_layer(GuiLayer_OneClick)));

    get_isometric_or_front_view_mouse_inputs(packet, rotate_pressed, no_mods);

    if ( rotate_pressed )
    {
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, no_mods) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
            set_packet_control(packet, PCtr_ViewRotateCW);
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, no_mods) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
            set_packet_control(packet, PCtr_ViewRotateCCW);
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, no_mods) || is_key_pressed(KC_UP,KMod_DONTCARE) )
            set_packet_control(packet, PCtr_ViewZoomIn);
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, no_mods) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
            set_packet_control(packet, PCtr_ViewZoomOut);
    } else
    {
        if ( is_game_key_pressed(Gkey_RotateCW, NULL, false) )
            set_packet_control(packet, PCtr_ViewRotateCW);
        if ( is_game_key_pressed(Gkey_RotateCCW, NULL, false) )
            set_packet_control(packet, PCtr_ViewRotateCCW);
        if ( is_game_key_pressed(Gkey_ZoomIn, NULL, false) )
            set_packet_control(packet, PCtr_ViewZoomIn);
        if ( is_game_key_pressed(Gkey_ZoomOut, NULL, false) )
            set_packet_control(packet, PCtr_ViewZoomOut);
        if ( is_game_key_pressed(Gkey_TiltUp, NULL, false) )
            set_packet_control(packet, PCtr_ViewTiltUp);
        if ( is_game_key_pressed(Gkey_TiltDown, NULL, false) )
            set_packet_control(packet, PCtr_ViewTiltDown);
        if ( is_game_key_pressed(Gkey_TiltReset, NULL, false) )
            set_packet_control(packet, PCtr_ViewTiltReset);
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, no_mods) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
        {
            if(player->work_state == PSt_FreeCtrlDirect || player->work_state == PSt_CtrlDirect)
            {
                set_packet_control(packet, PCtr_MoveLeft);
            }
            else
            {
                 movement_accum_x = -1.0f;
            }
        }
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, no_mods) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
        {
            if(player->work_state == PSt_FreeCtrlDirect || player->work_state == PSt_CtrlDirect)
            {
                set_packet_control(packet, PCtr_MoveRight);
            }
            else
            {
                 movement_accum_x = 1.0f;
            }
        }
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, no_mods) || is_key_pressed(KC_UP,KMod_DONTCARE) )
        {
            if(player->work_state == PSt_FreeCtrlDirect || player->work_state == PSt_CtrlDirect)
            {
                set_packet_control(packet, PCtr_MoveUp);
            }
            else
            {
                 movement_accum_y = -1.0f;
            }
        }
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, no_mods) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
        {
            if(player->work_state == PSt_FreeCtrlDirect || player->work_state == PSt_CtrlDirect)
            {
                set_packet_control(packet, PCtr_MoveDown);
            }
            else
            {
                 movement_accum_y = 1.0f;
            }
        }
        // Packets will be sent by send_camera_catchup_packets() based on position difference
    }
}

void get_overhead_view_nonaction_inputs(void)
{
    SYNCDBG(19,"Starting");
    struct PlayerInfo* player = get_my_player();
    struct Packet* pckt = get_packet(my_player_number);
    long my = my_mouse_y;
    long mx = my_mouse_x;
    int rotate_pressed = is_game_key_pressed(Gkey_RotateMod, NULL, true);
    int speed_pressed = is_game_key_pressed(Gkey_SpeedMod, NULL, true);
    if ((player->allocflags & PlaF_KeyboardInputDisabled) == 0)
    {
        if (speed_pressed)
          pckt->additional_packet_values |= PCAdV_SpeedupPressed;
        if (rotate_pressed)
        {
          if ( is_game_key_pressed(Gkey_MoveUp, NULL, speed_pressed!=0) )
            set_packet_control(pckt, PCtr_ViewZoomIn);
          if ( is_game_key_pressed(Gkey_MoveDown, NULL, speed_pressed!=0) )
            set_packet_control(pckt, PCtr_ViewZoomOut);
        }
        if (my <= 4)
          set_packet_control(pckt, PCtr_MoveUp);
        if (my >= MyScreenHeight-4)
          set_packet_control(pckt, PCtr_MoveDown);
        if (mx <= 4)
          set_packet_control(pckt, PCtr_MoveLeft);
        if (mx >= MyScreenWidth-4)
          set_packet_control(pckt, PCtr_MoveRight);
    }
    set_local_camera_destination(player);
}

void get_front_view_nonaction_inputs(void)
{
    static TbClockMSec last_rotate_left_time = 0;
    static TbClockMSec last_rotate_right_time = 0;
    struct PlayerInfo* player = get_my_player();
    struct Packet* pckt = get_packet(my_player_number);
    int rotate_pressed = is_game_key_pressed(Gkey_RotateMod, NULL, true);
    int speed_pressed = is_game_key_pressed(Gkey_SpeedMod, NULL, true);
    TbBool no_mods = ((rotate_pressed != 0) || (speed_pressed != 0) || (check_current_gui_layer(GuiLayer_OneClick)));

    if ((player->allocflags & PlaF_KeyboardInputDisabled) != 0)
      return;
    if (speed_pressed != 0)
      pckt->additional_packet_values |= PCAdV_SpeedupPressed;

    get_isometric_or_front_view_mouse_inputs(pckt,rotate_pressed,no_mods);

    if ( rotate_pressed )
    {
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, no_mods) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
        {
          if ( LbTimerClock() > last_rotate_left_time+250 )
          {
            set_packet_control(pckt, PCtr_ViewRotateCW);
            last_rotate_left_time = LbTimerClock();
          }
        }
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, no_mods) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
        {
          if ( LbTimerClock() > last_rotate_right_time+250 )
          {
            set_packet_control(pckt, PCtr_ViewRotateCCW);
            last_rotate_right_time = LbTimerClock();
          }
        }
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, no_mods) || is_key_pressed(KC_UP,KMod_DONTCARE) )
            set_packet_control(pckt, PCtr_ViewZoomIn);
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, no_mods) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
            set_packet_control(pckt, PCtr_ViewZoomOut);
    } else
    {
        if ( is_game_key_pressed(Gkey_RotateCW, NULL, false) )
        {
          if ( LbTimerClock() > last_rotate_left_time+250 )
          {
            set_packet_control(pckt, PCtr_ViewRotateCW);
            last_rotate_left_time = LbTimerClock();
          }
        }
        if ( is_game_key_pressed(Gkey_RotateCCW, NULL, false) )
        {
          if ( LbTimerClock() > last_rotate_right_time+250 )
          {
            set_packet_control(pckt, PCtr_ViewRotateCCW);
            last_rotate_right_time = LbTimerClock();
          }
        }
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, no_mods) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
            set_packet_control(pckt, PCtr_MoveLeft);
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, no_mods) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
            set_packet_control(pckt, PCtr_MoveRight);
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, no_mods) || is_key_pressed(KC_UP,KMod_DONTCARE) )
            set_packet_control(pckt, PCtr_MoveUp);
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, no_mods) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
            set_packet_control(pckt, PCtr_MoveDown);
    }
    if ( is_game_key_pressed(Gkey_ZoomIn, NULL, false) )
        set_packet_control(pckt, PCtr_ViewZoomIn);
    if ( is_game_key_pressed(Gkey_ZoomOut, NULL, false) )
        set_packet_control(pckt, PCtr_ViewZoomOut);
}

/**
 * Updates given position and context variables.
 * Makes no changes to the Game or Packets.
 */
TbBool get_player_coords_and_context(struct Coord3d *pos, unsigned char *context)
{
  unsigned long x;
  unsigned long y;
  struct PlayerInfo* player = get_my_player();
  if ((pointer_x < 0) || (pointer_y < 0)
   || (pointer_x >= player->engine_window_width/pixel_size)
   || (pointer_y >= player->engine_window_height/pixel_size))
      return false;
  if (top_pointed_at_x <= game.map_subtiles_x)
    x = top_pointed_at_x;
  else
    x = game.map_subtiles_x;
  if (top_pointed_at_y <= game.map_subtiles_y)
    y = top_pointed_at_y;
  else
    y = game.map_subtiles_y;
  unsigned int slb_x = subtile_slab(x);
  unsigned int slb_y = subtile_slab(y);

  struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
  struct SlabConfigStats* slabst = get_slab_stats(slb);
  if (slab_kind_is_door(slb->kind) && (slabmap_owner(slb) == player->id_number) && (!player->one_click_lock_cursor))
  {
    *context = CSt_DoorKey;
    pos->x.val = (x<<8) + top_pointed_at_frac_x;
    pos->y.val = (y<<8) + top_pointed_at_frac_y;
  } else
  if (!power_hand_is_empty(player))
  {
    *context = CSt_PowerHand;
    pos->x.val = (x<<8) + top_pointed_at_frac_x;
    pos->y.val = (y<<8) + top_pointed_at_frac_y;
  } else
  if (!subtile_revealed(x,y,player->id_number))
  {
    *context = CSt_PickAxe;
    pos->x.val = (x<<8) + top_pointed_at_frac_x;
    pos->y.val = (y<<8) + top_pointed_at_frac_y;
  } else
  if (((slb_x >= game.map_tiles_x) || (slb_y >= game.map_tiles_y)) && (!player->one_click_lock_cursor))
  {
    *context = CSt_DefaultArrow;
    pos->x.val = (block_pointed_at_x<<8) + pointed_at_frac_x;
    pos->y.val = (block_pointed_at_y<<8) + pointed_at_frac_y;
  } else
  if (((slabst->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) || (player->one_click_lock_cursor))
  {
    *context = CSt_PickAxe;
    pos->x.val = (x<<8) + top_pointed_at_frac_x;
    pos->y.val = (y<<8) + top_pointed_at_frac_y;
  } else
  {
    pos->x.val = (block_pointed_at_x<<8) + pointed_at_frac_x;
    pos->y.val = (block_pointed_at_y<<8) + pointed_at_frac_y;
    struct Thing* thing = get_nearest_thing_for_hand_or_slap(player->id_number, pos->x.val, pos->y.val);
    if (!thing_is_invalid(thing))
      *context = CSt_PowerHand;
    else
      *context = CSt_DefaultArrow;
  }
  if (pos->x.val >= (game.map_subtiles_x << 8))
    pos->x.val = (game.map_subtiles_x << 8)-1;
  if (pos->y.val >= (game.map_subtiles_y << 8))
    pos->y.val = (game.map_subtiles_y << 8)-1;
  return true;
}

/**
 * Fill packet structure with non action user input in dungeon view.
 */
void get_dungeon_control_nonaction_inputs(void)
{
  struct Coord3d pos;
  my_mouse_x = GetMouseX();
  my_mouse_y = GetMouseY();
  struct PlayerInfo* player = get_my_player();
  struct Packet* pckt = get_packet(my_player_number);
  unset_packet_control(pckt, PCtr_MapCoordsValid);
  if (player->work_state == PSt_CtrlDungeon)
  {
      unsigned char context;
      if (get_player_coords_and_context(&pos, &context))
      {
          set_players_packet_position(pckt, pos.x.val, pos.y.val, context);
    }
  } else
  if (screen_to_map(get_local_camera(player->acamera), my_mouse_x, my_mouse_y, &pos))
  {
      set_players_packet_position(pckt, pos.x.val, pos.y.val, 0);
      pckt->additional_packet_values &= ~PCAdV_ContextMask; // reset cursor states to 0 (CSt_DefaultArrow)
  }
  if (lbKeyOn[KC_LALT] && lbKeyOn[KC_X])
  {
    clear_key_pressed(KC_X);
    turn_on_menu(GMnu_QUIT);
  }
  if (lbKeyOn[KC_ESCAPE])
  {
      lbKeyOn[KC_ESCAPE] = 0;
      if (a_menu_window_is_active())
      {
          turn_off_all_window_menus();
      }
      else
      {
          if (menu_is_active(GMnu_MAIN))
          {
              fake_button_click(BID_OPTIONS);
          }
          turn_on_menu(GMnu_OPTIONS);
      }
  }
  if ((player->allocflags & PlaF_NewMPMessage) == 0)
  {
      switch (player->view_mode)
      {
      case PVM_IsoWibbleView:
      case PVM_IsoStraightView:
          get_isometric_view_nonaction_inputs();
          break;
      case PVM_ParchmentView:
          get_overhead_view_nonaction_inputs();
          break;
      case PVM_FrontView:
          get_front_view_nonaction_inputs();
          break;
      }
  }
}

void get_map_nonaction_inputs(void)
{
    SYNCDBG(9,"Starting");
    struct Coord3d pos;
    pos.x.val = 0;
    pos.y.val = 0;
    pos.z.val = 0;
    struct PlayerInfo* player = get_my_player();
    TbBool coords_valid = screen_to_map(get_local_camera(player->acamera), GetMouseX(), GetMouseY(), &pos);
    set_players_packet_position(get_packet(my_player_number), pos.x.val, pos.y.val, 0);
    struct Packet* pckt = get_packet(my_player_number);
    if (coords_valid) {
        set_packet_control(pckt, PCtr_MapCoordsValid);
    } else {
        unset_packet_control(pckt, PCtr_MapCoordsValid);
    }
    if (((game.operation_flags & GOF_Paused) == 0) && (player->view_mode == PVM_ParchmentView))
    {
        get_overhead_view_nonaction_inputs();
    }
}

short get_packet_load_game_inputs(void)
{
    load_packets_for_turn(game.pckt_gameturn);
    game.pckt_gameturn++;
    get_packet_load_game_control_inputs();
    if (get_speed_control_inputs())
        return false;
    if (get_screen_control_inputs())
        return false;
    if (get_screen_capture_inputs())
        return false;
    return false;
}

/**
 * Inputs for demo mode. In this mode, the only control keys
 * should take the game back into main menu.
 */
TbBool get_packet_load_demo_inputs(void)
{
  if (is_key_pressed(KC_SPACE,KMod_DONTCARE) ||
      is_key_pressed(KC_ESCAPE,KMod_DONTCARE) ||
      is_key_pressed(KC_RETURN,KMod_DONTCARE) ||
      (lbKeyOn[KC_LALT] && lbKeyOn[KC_X]) ||
      left_button_clicked)
  {
      clear_key_pressed(KC_SPACE);
      clear_key_pressed(KC_ESCAPE);
      clear_key_pressed(KC_RETURN);
      lbKeyOn[KC_X] = 0;
      left_button_clicked = 0;
      quit_game = 1;
      return true;
  }
  return false;
}

void get_creature_control_nonaction_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    if ((player->allocflags & PlaF_CreaturePassengerMode) != 0)
    {
        return;
    }
    struct Packet* pckt = get_packet(my_player_number);
    long x = GetMouseX();
    long y = GetMouseY();
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    TRACE_THING(thing);
    TbBool cheat_menu_active = cheat_menu_is_active();
    if (((MyScreenWidth >> 1) != x) || ((MyScreenHeight >> 1) != y))
    {
        if (!cheat_menu_active && !a_menu_window_is_active())
        {
            LbMouseSetPositionInitial((MyScreenWidth / pixel_size) >> 1, (MyScreenHeight / pixel_size) >> 1);
        }
    }
    if (!cheat_menu_active && !a_menu_window_is_active())
    {
        long centerX = MyScreenWidth / 2;
        long centerY = MyScreenHeight / 2;
        long deltaX = x - centerX;
        long deltaY = y - centerY;
        long k;

        // Map to the range -255 to 255
        pckt->pos_x = 255 * deltaX / centerX;
        if (settings.first_person_move_invert) {
            pckt->pos_y = 255 * deltaY / centerY;
        } else {
            pckt->pos_y = -255 * deltaY / centerY;
        }

        long i = settings.first_person_move_sensitivity + 1;
        x = pckt->pos_x;
        y = pckt->pos_y;

        if (i < 6) {
            k = 5 - settings.first_person_move_sensitivity;
            pckt->pos_x = x / k;
            pckt->pos_y = y / k;
        } else if (i > 6) {
            k = settings.first_person_move_sensitivity - 5;
            pckt->pos_x = k * x;
            pckt->pos_y = k * y;
        }

        if (pckt->pos_x < -255) {
            pckt->pos_x = -255;
        } else if (pckt->pos_x > 255) {
            pckt->pos_x = 255;
        }
        if (pckt->pos_y < -255) {
            pckt->pos_y = -255;
        } else if (pckt->pos_y > 255) {
            pckt->pos_y = 255;
        }
    }
    // Now do user actions
    if (thing_is_creature(thing))
    {
        if (left_button_clicked)
        {
            left_button_clicked = 0;
            left_button_released = 0;
        }
        if (right_button_clicked)
        {
            right_button_clicked = 0;
            right_button_released = 0;
        }
        if (is_game_key_pressed(Gkey_MoveLeft, NULL, true) || is_key_pressed(KC_LEFT, KMod_DONTCARE))
            set_packet_control(pckt, PCtr_MoveLeft);
        if (is_game_key_pressed(Gkey_MoveRight, NULL, true) || is_key_pressed(KC_RIGHT, KMod_DONTCARE))
            set_packet_control(pckt, PCtr_MoveRight);
        if (is_game_key_pressed(Gkey_MoveUp, NULL, true) || is_key_pressed(KC_UP, KMod_DONTCARE))
            set_packet_control(pckt, PCtr_MoveUp);
        if (is_game_key_pressed(Gkey_MoveDown, NULL, true) || is_key_pressed(KC_DOWN, KMod_DONTCARE))
            set_packet_control(pckt, PCtr_MoveDown);
        if (flag_is_set(thing->movement_flags, TMvF_Flying))
        {
            if (is_game_key_pressed(Gkey_Ascend, NULL, true))
                set_packet_control(pckt, PCtr_Ascend);
            if (is_game_key_pressed(Gkey_Descend, NULL, true))
                set_packet_control(pckt, PCtr_Descend);
        }
    }
    if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
    {
        clear_key_pressed(KC_ESCAPE);
        if (a_menu_window_is_active())
        {
            turn_off_all_window_menus();
        }
        else
        {
            if (menu_is_active(GMnu_MAIN))
            {
                fake_button_click(BID_OPTIONS);
            }
            turn_on_menu(GMnu_OPTIONS);
        }
    }
}

static void speech_pickup_of_gui_job(int job_idx)
{
    SYNCDBG(7, "Picking up creature of breed %s for job of type %d",
        last_speech_event.u.creature.model_name, job_idx);
    int kind = creature_model_id(last_speech_event.u.creature.model_name);
    if (kind < 0) {
        SYNCDBG(0, "No such creature");
        return;
    }

    unsigned short pick_flags = TPF_PickableCheck;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL] || (job_idx == -1))
        pick_flags |= TPF_OrderedPick;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        pick_flags |= TPF_ReverseOrder;
    pick_up_creature_of_model_and_gui_job(kind, job_idx, my_player_number, pick_flags);
}

/**
 * Processes speech inputs that can be handled separately without interfacing with
 * mouse/keyboard code.
 */
static void get_dungeon_speech_inputs(void)
{
    SYNCDBG(8,"Starting");

    int id;
    switch (last_speech_event.type)
    {
    case KS_PICKUP_IDLE:
        speech_pickup_of_gui_job(CrGUIJob_Wandering);
        break;
    case KS_PICKUP_WORKING:
        speech_pickup_of_gui_job(CrGUIJob_Working);
        break;
    case KS_PICKUP_FIGHTING:
        speech_pickup_of_gui_job(CrGUIJob_Fighting);
        break;
    case KS_PICKUP_ANY:
        speech_pickup_of_gui_job(CrGUIJob_Any);
        break;
    case KS_SELECT_ROOM:
    {
        struct RoomConfigStats* room_stats = get_room_kind_stats(last_speech_event.u.room.id);
        activate_room_build_mode(last_speech_event.u.room.id, room_stats->tooltip_stridx);
        break;
    }
    case KS_SELECT_POWER:
        id = power_model_id(last_speech_event.u.power.model_name);
        if (id < 0) {
            WARNLOG("Bad power string %s", last_speech_event.u.power.model_name);
        }
        else {
            choose_spell(id, 2); //TODO: see what happens with tool tip
        }
        break;
    case KS_SELECT_TRAP:
        id = trap_model_id(last_speech_event.u.trapdoor.model_name);
        if (id < 0) {
            WARNLOG("Bad trap string %s", last_speech_event.u.trapdoor.model_name);
        }
        else if ((id = get_manufacture_data_index_for_thing(TCls_Trap, id)) > 0) {
            choose_workshop_item(id, 2); //TODO: see what happens with tool tip
        }
        else {
            WARNLOG("Trap %s is not in trap data array", last_speech_event.u.trapdoor.model_name);
        }
        break;
    case KS_SELECT_DOOR:
        id = door_model_id(last_speech_event.u.trapdoor.model_name);
        if (id < 0) {
            WARNLOG("Bad door string %s", last_speech_event.u.trapdoor.model_name);
        }
        else if ((id = get_manufacture_data_index_for_thing(TCls_Door, id)) > 0) {
            choose_workshop_item(id, 2); //TODO: see what happens with tool tip
        }
        else {
            WARNLOG("Door %s is not in trap data array", last_speech_event.u.trapdoor.model_name);
        }
        break;
    case KS_VIEW_INFO:
        set_menu_mode(BID_INFO_TAB); //TODO SPEECH not working for some reason, debug
        break;
    case KS_VIEW_ROOMS:
        set_menu_mode(BID_ROOM_TAB);
        break;
    case KS_VIEW_POWERS:
        set_menu_mode(BID_SPELL_TAB);
        break;
    case KS_VIEW_TRAPS:
        set_menu_mode(BID_MNFCT_TAB);
        break;
    case KS_VIEW_CREATURES:
        set_menu_mode(BID_CREATR_TAB);
        break;
    default:
        break; //don't care
    }
}

TbBool active_menu_functions_while_paused()
{
    return (menu_is_active(GMnu_QUIT) || menu_is_active(GMnu_OPTIONS) || menu_is_active(GMnu_LOAD) || menu_is_active(GMnu_SAVE)
         || menu_is_active(GMnu_VIDEO) || menu_is_active(GMnu_SOUND) || menu_is_active(GMnu_ERROR_BOX) || menu_is_active(GMnu_AUTOPILOT));
}


/** Fill packet struct with game action information.
 */
short get_inputs(void)
{
    if ((game.mode_flags & MFlg_IsDemoMode) != 0)
    {
        SYNCDBG(5,"Starting for demo mode");
        load_packets_for_turn(game.pckt_gameturn);
        game.pckt_gameturn++;
        get_packet_load_demo_inputs();
        return false;
    }
    if (game.packet_load_enable)
    {
        SYNCDBG(5,"Loading packet inputs");
        return get_packet_load_game_inputs();
    }
    struct PlayerInfo* player = get_my_player();
    if ((player->allocflags & PlaF_MouseInputDisabled) != 0)
    {
        SYNCDBG(5,"Starting for creature fade");
        set_players_packet_position(get_packet(my_player_number), 0, 0 , 0);
        if ((!game_is_busy_doing_gui_string_input()) && ((game.operation_flags & GOF_Paused) != 0))
        {
            int32_t keycode;
            if (is_game_key_pressed(Gkey_TogglePause, &keycode, false))
            {
                lbKeyOn[keycode] = 0;
                set_packet_pause_toggle();
            }
            else if( flag_is_set(start_params.debug_flags, DFlg_FrameStep) )
            {
                if( is_key_pressed(KC_PERIOD, KMOD_NONE) )
                {
                    game.frame_step = true;
                    set_packet_pause_toggle();
                    clear_key_pressed(KC_PERIOD);
                }
            }
        }
        return false;
    }
    SYNCDBG(5,"Starting");
    gui_process_inputs();
    if (player->victory_state == VicS_LostLevel)
    {
        if (player->is_active != 1)
        {
            get_level_lost_inputs();
            return true;
        }
        struct Thing* thing = thing_get(player->controlled_thing_idx);
        TRACE_THING(thing);
        if (!thing_is_creature(thing))
        {
            get_level_lost_inputs();
            return true;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if ((cctrl->creature_state_flags & TF2_Spectator) == 0)
        {
            get_level_lost_inputs();
            return true;
        }
    }
    TbBool inp_handled = false;
    if (!flag_is_set(game.operation_flags,GOF_Paused) || active_menu_functions_while_paused() || flag_is_set(game.operation_flags,GOF_WorldInfluence))
        inp_handled = get_gui_inputs(1);
    if (!inp_handled)
        inp_handled = get_global_inputs();
    if (game_is_busy_doing_gui_string_input())
      return false;
    get_screen_capture_inputs();
    SYNCDBG(7,"Getting inputs for view %d",(int)player->view_type);
    switch (player->view_type)
    {
    case PVT_DungeonTop:
        get_dungeon_control_pausable_action_inputs();
        if (!inp_handled)
            inp_handled = get_dungeon_control_action_inputs();
        get_dungeon_control_nonaction_inputs();
        get_player_gui_clicks();
        get_packet_control_mouse_clicks();
        get_dungeon_speech_inputs();
        return inp_handled;
    case PVT_CreatureContrl:
        if (!inp_handled)
            inp_handled = get_creature_control_action_inputs();
        get_creature_control_nonaction_inputs();
        get_player_gui_clicks();
        get_packet_control_mouse_clicks();
        return inp_handled;
    case PVT_CreaturePasngr:
        if (!inp_handled)
            inp_handled = get_creature_passenger_action_inputs();
        get_player_gui_clicks();
        get_packet_control_mouse_clicks();
        return inp_handled;
    case PVT_MapScreen:
        inp_handled = get_map_action_inputs();
        get_map_nonaction_inputs();
        get_player_gui_clicks();
        get_packet_control_mouse_clicks();
        // Unset button release events if we're going to do an action; this is to avoid casting
        // spells or doing other actions just after switch from parchment to dungeon view
        if (get_players_packet_action(player) != PckA_None)
            unset_players_packet_control(player, PCtr_LBtnRelease|PCtr_RBtnRelease);
        return inp_handled;
    case PVT_MapFadeIn:
        if (player->view_mode != PVM_ParchFadeIn)
        {
          if ((game.system_flags & GSF_NetworkActive) == 0)
            game.operation_flags &= ~GOF_Paused;
          player->status_menu_restore = toggle_status_menu(0); // store current status menu visibility, and hide the status menu (when the map is visible) [duplicate? unneeded?]
          set_players_packet_action(player, PckA_SetViewType, PVT_MapScreen, 0,0,0);
        }
        return false;
    case PVT_MapFadeOut:
        if (player->view_mode != PVM_ParchFadeOut)
        {
          set_players_packet_action(player, PckA_SetViewType, PVT_DungeonTop, 0,0,0);
        }
        return false;
    default:
        SYNCDBG(7,"Default exit");
        return false;
    }
}

void input(void)
{
    SYNCDBG(4,"Starting");

    update_mouse();
    update_key_modifiers();
    update_gui_layer();

    if (KeeperSpeechPopEvent(&last_speech_event)) {
      last_speech_event.type = KS_UNUSED;
    }

    if ((game_is_busy_doing_gui_string_input()) && (lbInkey>0))
    {
      lbKeyOn[lbInkey] = 0;
    }
    struct Packet* pckt = get_packet(my_player_number);
    if (is_game_key_pressed(Gkey_CrtrContrlMod, NULL, false) != 0)
      pckt->additional_packet_values |= PCAdV_CrtrContrlPressed;
    else
      pckt->additional_packet_values &= ~PCAdV_CrtrContrlPressed;
    if (is_game_key_pressed(Gkey_CrtrQueryMod, NULL, false) != 0)
      pckt->additional_packet_values |= PCAdV_CrtrQueryPressed;
    else
      pckt->additional_packet_values &= ~PCAdV_CrtrQueryPressed;
    if (is_game_key_pressed(Gkey_RotateMod, NULL, false) != 0)
        pckt->additional_packet_values |= PCAdV_RotatePressed;
    else
        pckt->additional_packet_values &= ~PCAdV_RotatePressed;

    get_inputs();

    SYNCDBG(7,"Finished");
}

short get_gui_inputs(short gameplay_on)
{
  static ActiveButtonID over_slider_button = -1;
  SYNCDBG(7,"Starting");
  battle_creature_over = 0;
  gui_room_type_highlighted = -1;
  gui_door_type_highlighted = -1;
  gui_trap_type_highlighted = -1;
  gui_creature_type_highlighted = -1;
  if (gameplay_on) {
      update_creatr_model_activities_list(0);
      maintain_my_battle_list();
  }
  if (!lbDisplay.MLeftButton)
  {
      drag_menu_x = -999;
      drag_menu_y = -999;
      for (int idx = 0; idx < ACTIVE_BUTTONS_COUNT; idx++)
      {
        struct GuiButton *gbtn = &active_buttons[idx];
        if ((gbtn->flags & LbBtnF_Active) && (gbtn->gbtype == LbBtnT_Hotspot))
            gbtn->button_state_left_pressed = 0;
      }
  }
  update_busy_doing_gui_on_menu();
  int fmmenu_idx = first_monopoly_menu();
  struct PlayerInfo* player = get_my_player();
  int gmbtn_idx = -1;
  ActiveButtonID nx_over_slider_button = -1;
  struct GuiButton *gbtn;
  // Sweep through buttons
  for (int gidx = 0; gidx < ACTIVE_BUTTONS_COUNT; gidx++)
  {
      gbtn = &active_buttons[gidx];
      if ((gbtn->flags & LbBtnF_Active) == 0)
          continue;
      if (!get_active_menu(gbtn->gmenu_idx)->is_turned_on)
          continue;
      Gf_Btn_Callback callback = gbtn->maintain_call;
      if (callback != NULL)
          callback(gbtn);
      if ((gbtn->btype_value & LbBFeF_NoMouseOver) != 0)
          continue;
      // TODO GUI Introduce circular buttons instead of specific condition for pannel map
      if ((menu_id_to_number(GMnu_MAIN) >= 0) && mouse_is_over_panel_map(player->minimap_pos_x,player->minimap_pos_y))
          continue;
      if ( (check_if_mouse_is_over_button(gbtn) && !game_is_busy_doing_gui_string_input())
        || ((gbtn->gbtype == LbBtnT_Hotspot) && (gbtn->button_state_left_pressed != 0)) )
      {
          if ((fmmenu_idx == -1) || (gbtn->gmenu_idx == fmmenu_idx))
          {
            gmbtn_idx = gidx;
            gbtn->flags |= LbBtnF_MouseOver;
            busy_doing_gui = 1;
            callback = gbtn->ptover_event;
            if (callback != NULL)
                callback(gbtn);
            if (gbtn->gbtype == LbBtnT_Hotspot)
                break;
            if (gbtn->gbtype == LbBtnT_HorizSlider)
                nx_over_slider_button = gidx;
          } else
          {
            gbtn->flags &= ~LbBtnF_MouseOver;
          }
      } else
      {
          gbtn->flags &= ~LbBtnF_MouseOver;
      }
      if (gbtn->gbtype == LbBtnT_HorizSlider)
      {
          if (gui_slider_button_mouse_over_slider_tracker(gidx))
          {
              if ( left_button_clicked )
              {
                left_button_clicked = 0;
                nx_over_slider_button = gidx;
                over_slider_button = gidx;
                do_sound_menu_click();
              }
          }
      }
  }  // end for

  // Reset slider button if we were not really over it
  if (over_slider_button != nx_over_slider_button)
      over_slider_button = -1;

  TbBool result = false;
  if (game_is_busy_doing_gui_string_input())
  {
    busy_doing_gui = 1;
    if (get_button_area_input(input_button,input_button->id_num) != 0)
        result = true;
  }
  if ((over_slider_button != -1) && (left_button_released))
  {
      left_button_released = 0;
      if (gmbtn_idx != -1) {
          gbtn = &active_buttons[gmbtn_idx];
          gbtn->button_state_left_pressed = 0;
      }
      over_slider_button = -1;
      do_sound_menu_click();
  }
  clear_flag(tool_tip_box.flags, TTip_Visible);
  gui_button_tooltip_update(gmbtn_idx);
  if (gui_slider_button_inputs(over_slider_button))
      return true;
  result |= gui_button_click_inputs(gmbtn_idx);
  gui_clear_buttons_not_over_mouse(gmbtn_idx);
  result |= gui_button_release_inputs(gmbtn_idx);
  input_gameplay_tooltips(gameplay_on);
  SYNCDBG(8,"Finished");
  return result;
}

void process_cheat_mode_selection_inputs()
{
    struct PlayerInfo *player = get_my_player();
    unsigned char new_value;
    struct CreatureModelConfig* crconf;
    // player selection
    if (player->work_state == PSt_PlaceTerrain)
    {
        if (slab_kind_has_no_ownership(player->cheatselection.chosen_terrain_kind))
        {
            goto INPUTS;
        }
    }
    if (is_key_pressed(KC_NUMPAD0, KMod_DONTCARE))
    {
        new_value = PLAYER0;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD0);
    }
    else if (is_key_pressed(KC_NUMPAD1, KMod_DONTCARE))
    {
        new_value = PLAYER1;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD1);
    }
    else if (is_key_pressed(KC_NUMPAD2, KMod_DONTCARE))
    {
        new_value = PLAYER2;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD2);
    }
    else if (is_key_pressed(KC_NUMPAD3, KMod_DONTCARE))
    {
        new_value = PLAYER3;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD3);
    }
    else if (is_key_pressed(KC_NUMPAD4, KMod_DONTCARE))
    {
        new_value = PLAYER4;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD4);
    }
    else if (is_key_pressed(KC_NUMPAD5, KMod_DONTCARE))
    {
        new_value = PLAYER5;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD5);
    }
    else if (is_key_pressed(KC_NUMPAD6, KMod_DONTCARE))
    {
        new_value = PLAYER6;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD6);
    }
    else if (is_key_pressed(KC_NUMPAD7, KMod_DONTCARE))
    {
        new_value = PLAYER_GOOD;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD7);
    }
    else if (is_key_pressed(KC_NUMPAD8, KMod_DONTCARE))
    {
        new_value = PLAYER_NEUTRAL;
        set_players_packet_action(player, PckA_CheatSwitchPlayer, new_value, 0, 0, 0);
        clear_key_pressed(KC_NUMPAD8);
    }

    INPUTS:
    // state-specific inputs
    switch (player->work_state)
    {
        case PSt_MkBadCreatr:
        case PSt_MkGoodCreatr:
        case PSt_MkDigger:
        {
            if (is_key_pressed(KC_1, KMod_NONE))
            {
                new_value = 0;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_1);
            }
            else if (is_key_pressed(KC_2, KMod_NONE))
            {
                new_value = 1;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_2);
            }
            else if (is_key_pressed(KC_3, KMod_NONE))
            {
                new_value = 2;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_3);
            }
            else if (is_key_pressed(KC_4, KMod_NONE))
            {
                new_value = 3;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_4);
            }
            else if (is_key_pressed(KC_5, KMod_NONE))
            {
                new_value = 4;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_5);
            }
            else if (is_key_pressed(KC_6, KMod_NONE))
            {
                new_value = 5;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_6);
            }
            else if (is_key_pressed(KC_7, KMod_NONE))
            {
                new_value = 6;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_7);
            }
            else if (is_key_pressed(KC_8, KMod_NONE))
            {
                new_value = 7;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_8);
            }
            else if (is_key_pressed(KC_9, KMod_NONE))
            {
                new_value = 8;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_9);
            }
            else if (is_key_pressed(KC_0, KMod_NONE))
            {
                new_value = 9;
                set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                clear_key_pressed(KC_0);
            }
            else if (is_key_pressed(KC_EQUALS, KMod_DONTCARE))
            {

                new_value = player->cheatselection.chosen_experience_level;
                if (new_value < 9)
                {
                    new_value++;
                    set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                }
                clear_key_pressed(KC_EQUALS);
            }
            else if (is_key_pressed(KC_MINUS, KMod_DONTCARE))
            {
                new_value = player->cheatselection.chosen_experience_level;
                if (new_value > 0)
                {
                    new_value--;
                    set_players_packet_action(player, PckA_CheatSwitchExperience, new_value, 0, 0, 0);
                }
                clear_key_pressed(KC_MINUS);
            }
            else if (is_key_pressed(KC_LSHIFT, KMod_DONTCARE))
            {
                if (player->work_state == PSt_MkGoodCreatr)
                {
                    new_value = player->cheatselection.chosen_hero_kind;
                    do
                    {
                        new_value++;
                        if (new_value >= game.conf.crtr_conf.model_count)
                        {
                            new_value = 0;
                            break;
                        }
                        crconf = &game.conf.crtr_conf.model[new_value];
                    }
                    while ( ((crconf->model_flags & CMF_IsEvil) != 0) || ((crconf->model_flags & CMF_IsSpectator) != 0) );
                    set_players_packet_action(player, PckA_CheatSwitchHero, new_value, 0, 0, 0);
                }
                else if (player->work_state == PSt_MkBadCreatr)
                {
                    new_value = player->cheatselection.chosen_creature_kind;
                    do
                    {
                        new_value++;
                        if (new_value >= game.conf.crtr_conf.model_count)
                        {
                            new_value = 0;
                            break;
                        }
                        crconf = &game.conf.crtr_conf.model[new_value];
                    }
                    while ( ((crconf->model_flags & CMF_IsEvil) == 0) || ((crconf->model_flags & CMF_IsSpectator) != 0) );
                    set_players_packet_action(player, PckA_CheatSwitchCreature, new_value, 0, 0, 0);
                }
                clear_key_pressed(KC_LSHIFT);
            }
            else if (is_key_pressed(KC_LCONTROL, KMod_DONTCARE))
            {
                if (player->work_state == PSt_MkGoodCreatr)
                {
                    new_value = player->cheatselection.chosen_hero_kind;
                    do
                    {
                        if (new_value == 0)
                        {
                            new_value = game.conf.crtr_conf.model_count - 1;
                        }
                        else
                        {
                           new_value--;
                           if (new_value == 0)
                           {
                               break;
                           }
                        }
                        crconf = &game.conf.crtr_conf.model[new_value];
                    }
                    while ( ((crconf->model_flags & CMF_IsEvil) != 0) || ((crconf->model_flags & CMF_IsSpectator) != 0) );
                    set_players_packet_action(player, PckA_CheatSwitchHero, new_value, 0, 0, 0);
                }
                else if (player->work_state == PSt_MkBadCreatr)
                {
                    new_value = player->cheatselection.chosen_creature_kind;
                    do
                    {
                        if (new_value == 0)
                        {
                            new_value = game.conf.crtr_conf.model_count - 1;
                        }
                        else
                        {
                            new_value--;
                            if (new_value == 0)
                            {
                                break;
                            }
                        }
                        crconf = &game.conf.crtr_conf.model[new_value];
                    }
                    while ( ((crconf->model_flags & CMF_IsEvil) == 0) || ((crconf->model_flags & CMF_IsSpectator) != 0) );
                    set_players_packet_action(player, PckA_CheatSwitchCreature, new_value, 0, 0, 0);
                }
                clear_key_pressed(KC_LCONTROL);
            }
            break;
        }
        case PSt_PlaceTerrain:
        {
            new_value = player->cheatselection.chosen_terrain_kind;
            if (is_key_pressed(KC_0, KMod_NONE))
            {
                new_value = SlbT_ROCK;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_0);
            }
            else if (is_key_pressed(KC_1, KMod_NONE))
            {
                new_value = SlbT_GOLD;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_1);
            }
            else if (is_key_pressed(KC_2, KMod_NONE))
            {
                new_value = SlbT_GEMS;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_2);
            }
            else if (is_key_pressed(KC_3, KMod_NONE))
            {
                new_value = SlbT_EARTH;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_3);
            }
            else if (is_key_pressed(KC_4, KMod_NONE))
            {
                new_value = SlbT_TORCHDIRT;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_4);
            }
            else if (is_key_pressed(KC_5, KMod_NONE))
            {
                new_value = SlbT_PATH;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_5);
            }
            else if (is_key_pressed(KC_6, KMod_NONE))
            {
                new_value = SlbT_CLAIMED;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_6);
            }
            else if (is_key_pressed(KC_7, KMod_NONE))
            {
                new_value = SlbT_LAVA;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_7);
            }
            else if (is_key_pressed(KC_8, KMod_NONE))
            {
                new_value = SlbT_WATER;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_8);
            }
            else if (is_key_pressed(KC_9, KMod_NONE))
            {
                new_value = rand() % (5) + 4;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_9);
            }
            else if (is_key_pressed(KC_MINUS, KMod_NONE))
            {
                new_value = SlbT_DAMAGEDWALL;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_MINUS);
            }
            else if (is_key_pressed(KC_EQUALS, KMod_NONE))
            {
                new_value = SlbT_SLAB50;
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_EQUALS);
            }
            else if (is_key_pressed(KC_LSHIFT, KMod_DONTCARE))
            {
                new_value++;
                if (new_value >= game.conf.slab_conf.slab_types_count)
                {
                    new_value = SlbT_ROCK;
                }
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_LSHIFT);
            }
            else if (is_key_pressed(KC_LCONTROL, KMod_DONTCARE))
            {
                if (new_value == SlbT_ROCK)
                {
                    new_value = game.conf.slab_conf.slab_types_count - 1;
                }
                else
                {
                    new_value--;
                }
                set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                clear_key_pressed(KC_LCONTROL);
            }
            if ( (player->cheatselection.chosen_terrain_kind >= SlbT_WALLDRAPE) && (player->cheatselection.chosen_terrain_kind <= SlbT_WALLPAIRSHR) )
            {
                if (is_key_pressed(KC_LALT, KMod_DONTCARE))
                {
                    struct Coord3d pos;
                    if (screen_to_map(get_local_camera(player->acamera), GetMouseX(), GetMouseY(), &pos))
                    {
                        MapSlabCoord slb_x = subtile_slab(pos.x.stl.num);
                        MapSlabCoord slb_y = subtile_slab(pos.y.stl.num);
                        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
                        PlayerNumber id;
                        if ( (slb->kind == SlbT_CLAIMED) || ( (slb->kind >= SlbT_WALLDRAPE) && (slb->kind <= SlbT_DAMAGEDWALL) ) )
                        {
                            id = slabmap_owner(slb);
                        }
                        else
                        {
                            id = player->cheatselection.chosen_player;
                        }
                        new_value = choose_pretty_type(id, slb_x, slb_y);
                        set_players_packet_action(player, PckA_CheatSwitchTerrain, new_value, 0, 0, 0);
                    }
                    clear_key_pressed(KC_LALT);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

TbBool process_cheat_heart_health_inputs(HitPoints *value, HitPoints max_health)
{
   HitPoints new_health = *value;
   if ( (is_key_pressed(KC_ADD, KMod_ALT)) || (is_key_pressed(KC_EQUALS, KMod_SHIFT)) || (is_key_pressed(KC_EQUALS, KMod_NONE)) )
   {
        if (new_health < max_health)
        {
            new_health++;
            *value = new_health;
            clear_key_pressed(KC_ADD);
            clear_key_pressed(KC_EQUALS);
            return true;
        }
    }
    else if ( (is_key_pressed(KC_PERIOD, KMod_SHIFT)) || (is_key_pressed(KC_PERIOD, KMod_NONE)) )
    {
        new_health += 100;
        *value = new_health;
        clear_key_pressed(KC_PERIOD);
        return true;
    }
    else if ( (is_key_pressed(KC_COMMA, KMod_SHIFT)) || (is_key_pressed(KC_COMMA, KMod_NONE)) )
    {
        new_health -= 100;
        *value = new_health;
        clear_key_pressed(KC_COMMA);
        return true;
    }
    else if ( (is_key_pressed(KC_SUBTRACT, KMod_ALT)) || (is_key_pressed(KC_MINUS, KMod_NONE)) )
    {
        new_health--;
        *value = new_health;
        clear_key_pressed(KC_SUBTRACT);
        clear_key_pressed(KC_MINUS);
        return true;
    }
    return false;
}

void disable_packet_mode()
{
    close_packet_file();
    game.packet_load_enable = false;
    game.packet_save_enable = false;
    show_onscreen_msg(2*game_num_fps, "Packet mode disabled");
    set_gui_visible(true);
}

/******************************************************************************/
