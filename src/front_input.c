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
#include "bflib_memory.h"
#include "bflib_network.h"

#include "kjm_input.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_map.h"
#include "frontmenu_ingame_evnt.h"
#include "scrcapt.h"
#include "player_instances.h"
#include "player_states.h"
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
#include "thing_traps.h"
#include "room_workshop.h"
#include "kjm_input.h"
#include "config_settings.h"
#include "game_legacy.h"

#include "keeperfx.hpp"
#include "KeeperSpeech.h"

#include <math.h>
#include "front_simple.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

unsigned short const zoom_key_room_order[] =
    {RoK_TREASURE, RoK_LIBRARY, RoK_LAIR, RoK_PRISON,
     RoK_TORTURE, RoK_TRAINING, RoK_DUNGHEART, RoK_WORKSHOP,
     RoK_SCAVENGER, RoK_TEMPLE, RoK_GRAVEYARD, RoK_BARRACKS,
     RoK_GARDEN, RoK_GUARDPOST, RoK_BRIDGE, RoK_NONE,};

KEEPERSPEECH_EVENT last_speech_event;

static TbBool map_coords_valid = false;
/******************************************************************************/
static void get_dungeon_control_nonaction_inputs(struct PacketEx *packet);
static void get_creature_control_nonaction_inputs(struct PacketEx *packet);
short zoom_shortcuts(void);
short get_bookmark_inputs(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
static inline void check_set_packet_action(
    struct PlayerInfo* player, struct PacketEx *packet,  enum TbPacketAction action, short arg0, short arg1)
{
    assert(packet->packet.action == PckA_None);
    packet->packet.action = PckA_Invalid;
    create_packet_action(player, action, arg0, arg1);
}

short game_is_busy_doing_gui_string_input(void)
{
  return (input_button != NULL);
}

short current_view_supports_status_menu()
{
    struct PlayerInfo* player = get_my_player();
    return (player->view_type != PVT_MapScreen);
}

int is_game_key_pressed(long key_id, long *val, TbBool ignore_mods)
{
  int result;
  int i;
  if ((key_id < 0) || (key_id >= GAME_KEYS_COUNT))
    return 0;
  if (val != NULL)
  {
    *val = settings.kbkeys[key_id].code;
  }
  if ((key_id == Gkey_RotateMod) || (key_id == Gkey_SpeedMod) || (key_id == Gkey_CrtrContrlMod) || (key_id == Gkey_CrtrQueryMod))
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
static TbBool get_players_message_inputs(void)
{
    struct PlayerInfo* player = get_my_player();
    struct PacketEx* packet = LbNetwork_AddPacket(
        PckA_PlyrMsgEnd, game.play_gameturn, sizeof(struct PacketEx*));

    if (is_key_pressed(KC_RETURN, KMod_NONE))
    {
        packet->packet.action = PckA_PlyrMsgEnd;
        clear_key_pressed(KC_RETURN);
        return true;
    } else if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
    {
        packet->packet.action = PckA_PlyrMsgClear;
        clear_key_pressed(KC_ESCAPE);
        return true;
    }
    LbTextSetFont(winfont);
    int msg_width = pixel_size * LbTextStringWidth(player->mp_message_text);
    if ( (is_key_pressed(KC_BACK,KMod_DONTCARE)) || (msg_width < 450) )
    {
        packet->packet.action = PckA_PlyrMsgChar;
        packet->packet.actn_par1 = lbInkey;
        packet->packet.actn_par2 = key_modifiers;
        clear_key_pressed(lbInkey);
        return true;
    }
    return false;
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
      set_flag_byte(&game.system_flags,GSF_CaptureSShot,true);
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
    if (game.frame_skip < 2)
    {
        game.frame_skip ++;
    }
    else if (game.frame_skip < 16)
    {
        game.frame_skip += 2;
    }
    else
    {
        game.frame_skip += (game.frame_skip/3);
    }
    clip_frame_skip();
    show_onscreen_msg(game.num_fps+game.frame_skip, "Frame skip %d",game.frame_skip);
}

void decreaseFrameskip(void)
{
    if (game.frame_skip <= 2)
    {
        game.frame_skip --;
    }
    else if (game.frame_skip <= 16)
    {
        game.frame_skip -= 2;
    }
    else
    {
        game.frame_skip -= (game.frame_skip/4);
    }
    clip_frame_skip();
    show_onscreen_msg(game.num_fps+game.frame_skip, "Frame skip %d",game.frame_skip);
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
    close_packet_file();
    game.packet_load_enable = false;
    game.packet_save_enable = false;
    show_onscreen_msg(2*game.num_fps, "Packet mode disabled");
    set_gui_visible(true);
    return true;
  }
  return false;
}

long get_small_map_inputs(long x, long y, long zoom)
{
  SYNCDBG(15,"Starting");
  short result = 0;
  long curr_mx = GetMouseX();
  long curr_my = GetMouseY();
  dummy_x = curr_mx;
  dummy_y = curr_my;
  dummy = 1;
  if (!grabbed_small_map)
    game.small_map_state = 0;
  if (((game.operation_flags & GOF_ShowGui) != 0) && (mouse_is_over_pannel_map(x,y) || grabbed_small_map))
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
  SYNCDBG(15,"Finished");
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
                show_onscreen_msg(game.num_fps, "Bookmark %d stored", i + 1);
            }
            return true;
        }
        // Load bookmark check
        if (is_key_pressed(kcode, KMod_SHIFT))
        {
            clear_key_pressed(kcode);
            if ((bmark->flags & 0x01) != 0)
            {
                create_packet_action(player, PckA_BookmarkLoad, bmark->x, bmark->y);
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
        long val;
        if (is_game_key_pressed(Gkey_ZoomRoom00 + i, &val, false))
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
        if (player->minimap_zoom < 0x0800)
        {
            create_packet_action(player, PckA_SetMinimapConf, 2 * (long)player->minimap_zoom, 0);
            packet_made = true;
        }
        clear_key_pressed(KC_SUBTRACT);
        if (packet_made)
            return true;
  }
  if (is_key_pressed(KC_ADD,KMod_NONE))
  {
      if ( player->minimap_zoom > 0x0080 )
      {
        create_packet_action(player, PckA_SetMinimapConf, player->minimap_zoom >> 1, 0);
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
        //TODO: is it should be an action
        create_packet_action(player, PckA_SwitchScrnRes, 0, 0);
        packet_made = true;
        clear_key_pressed(KC_R);
        if (packet_made)
            return true;
  }
  return false;
}

static TbBool get_global_inputs(struct PacketEx *packet)
{
  if (game_is_busy_doing_gui_string_input())
    return false;
  struct PlayerInfo* player = get_my_player();
  long keycode;
  if ((player->allocflags & PlaF_NewMPMessage) != 0)
  {
    get_players_message_inputs(); // We may create other packets while typing
    return true;
  }
  if ((player->view_type == PVT_DungeonTop)
  && (((game.system_flags & GSF_NetworkActive) != 0) ||
     ((game.flags_gui & GGUI_SoloChatEnabled) != 0)))
  {
      if (is_key_pressed(KC_RETURN,KMod_NONE))
      {
        create_packet_action(player, PckA_PlyrMsgBegin, 0, 0);
        clear_key_pressed(KC_RETURN);
        return true;
      }
  }
  // Code for debugging purposes
  if ( is_key_pressed(KC_D,KMod_ALT) )
  {
    JUSTMSG("REPORT for gameturn %d",game.play_gameturn);
    // Timing report
    JUSTMSG("Now time is %d, last loop time was %d, clock is %d, requested fps is %d",LbTimerClock(),last_loop_time,clock(),game.num_fps);
    test_variable = !test_variable;
  }

  for (int idx = KC_F1; idx <= KC_F8; idx++)
  {
      if ( is_key_pressed(idx,KMod_ALT) )
      {
        create_packet_action(player, PckA_PlyrFastMsg, idx-KC_F1, 0);
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
        set_packet_pause_toggle(packet);
        clear_key_pressed(keycode);
        return true;
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
        create_packet_action(player, PckA_FinishGame, 0, 0);
        clear_key_pressed(KC_SPACE);
        return true;
      }
  }
  if ( is_game_key_pressed(Gkey_DumpToOldPos, &keycode, false) )
  {
      create_packet_action(player, PckA_DumpHeldThingToOldPos, 0, 0);
      clear_key_pressed(keycode);
  }
  return false;
}

static TbBool get_level_lost_inputs(struct PacketEx *packet)
{
    long keycode;
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
        create_packet_action(player, PckA_PlyrMsgBegin, 0, 0);
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
        create_packet_action(player, PckA_FinishGame, 0, 0);
        clear_key_pressed(KC_SPACE);
    }
    if (player->view_type == PVT_MapScreen)
    {
        long mouse_x = GetMouseX();
        long mouse_y = GetMouseY();
        // Position on the parchment map on which we're doing action
        long map_x;
        long map_y;
        TbBool map_valid = point_to_overhead_map(player->acamera, mouse_x / pixel_size, mouse_y / pixel_size, &map_x, &map_y);
        if (is_game_key_pressed(Gkey_SwitchToMap, &keycode, false))
        {
            lbKeyOn[keycode] = 0;
            zoom_from_patchment_map();
        } else
        if ( right_button_released )
        {
            right_button_released = 0;
            zoom_from_patchment_map();
        } else
        if ( left_button_released )
        {
            if  ( map_valid ) {
                MapSubtlCoord stl_x = coord_subtile(map_x);
                MapSubtlCoord stl_y = coord_subtile(map_y);
                check_set_packet_action(player, packet, PckA_ZoomFromMap, stl_x, stl_y);
                left_button_released = 0;
            }
        }
    } else
    if (player->view_type == PVT_DungeonTop)
    {
      if (is_key_pressed(KC_TAB,KMod_DONTCARE))
      {
          if ((player->view_mode == PVM_IsometricView) || (player->view_mode == PVM_FrontView))
          {
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
          set_flag_byte(&game.operation_flags, GOF_ShowPanel, (game.operation_flags & GOF_ShowGui) != 0);
          if (((game.system_flags & GSF_NetworkActive) != 0)
            || (lbDisplay.PhysicalScreenWidth > 320))
          {
                if (toggle_status_menu(0))
                  set_flag_byte(&game.operation_flags,GOF_ShowPanel,true);
                else
                  set_flag_byte(&game.operation_flags,GOF_ShowPanel,false);
                check_set_packet_action(player, packet, PckA_SaveViewType, PVT_MapScreen, 0);
          } else
          {
                check_set_packet_action(player, packet, PckA_SetViewType, PVT_MapFadeIn, 0);
          }
          turn_off_roaming_menus();
        }
      }
    }
    if (is_key_pressed(KC_ESCAPE,KMod_DONTCARE))
    {
      clear_key_pressed(KC_ESCAPE);
      if ( a_menu_window_is_active() )
        turn_off_all_window_menus();
      else
        turn_on_menu(GMnu_OPTIONS);
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
          if (player->work_state == PSt_CreatrInfo)
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
              get_dungeon_control_nonaction_inputs(packet);
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
              if ((cctrl->flgfield_2 & TF2_Spectator) == 0)
              {
                  check_set_packet_action(player, packet, PckA_DirectCtrlExit, player->controlled_thing_idx, 0);
                  inp_done = true;
              }
          } else
        {
          check_set_packet_action(player, packet, PckA_DirectCtrlExit, player->controlled_thing_idx, 0);
          inp_done = true;
        }
        break;
      }
      case PVT_CreaturePasngr:
        check_set_packet_action(player, packet, PckA_PasngrCtrlExit, player->controlled_thing_idx, 0);
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
  if (is_key_pressed(KC_1, KMod_NONE))
  {
    clear_key_pressed(KC_1);
    fake_button_click(1);
  }
  if (is_key_pressed(KC_2, KMod_NONE))
  {
    clear_key_pressed(KC_2);
    fake_button_click(2);
  }
  if (is_key_pressed(KC_3, KMod_NONE))
  {
    clear_key_pressed(KC_3);
    fake_button_click(3);
  }
  if (is_key_pressed(KC_4, KMod_NONE))
  {
    clear_key_pressed(KC_4);
    fake_button_click(4);
  }
  if (is_key_pressed(KC_5, KMod_NONE))
  {
    clear_key_pressed(KC_5);
    fake_button_click(5);
  }
  return false;
}

static TbBool get_dungeon_control_action_inputs(struct PacketEx *packet)
{
    long val;
    struct PlayerInfo* player = get_my_player();

    //This code forces only one control action per turn?
    if (packet->packet.action != PckA_None)
      return 1;
    int mm_units_per_px;
    {
        int mnu_num = menu_id_to_number(GMnu_MAIN);
        struct GuiMenu* gmnu = get_active_menu(mnu_num);
        mm_units_per_px = (gmnu->width * 16 + 140/2) / 140;
        if (mm_units_per_px < 1)
        {
            mm_units_per_px = 1;
        }
    }
    long mmzoom;
    if (16/mm_units_per_px < 3)
    {
        mmzoom = (player->minimap_zoom) / (3-16/mm_units_per_px);
    }
    else
        mmzoom = (player->minimap_zoom);
    if (get_small_map_inputs(player->minimap_pos_x*mm_units_per_px/16, player->minimap_pos_y*mm_units_per_px/16, mmzoom))
      return 1;

    if (get_bookmark_inputs())
      return 1;

    if (is_key_pressed(KC_F8, KMod_NONE))
    {
        clear_key_pressed(KC_F8);
        toggle_tooltips();
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
    if (is_key_pressed(KC_F12,KMod_DONTCARE))
    {
        // Note that we're using "close", not "toggle". Menu can't be opened here.
        if (close_creature_cheat_menu())
            clear_key_pressed(KC_F12);
    }
    if (player->view_mode == PVM_IsometricView)
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
      if (lbDisplay.MiddleButton >= 1)
      {
          struct Camera* cam = &player->cameras[CamIV_Isometric];
          int angle = cam->orient_a;
          if (is_game_key_pressed(Gkey_RotateMod, NULL, false))
          {
              if ((angle >= 0 && angle < 256) || angle == 2048)
              {
                  angle = 256;
              }
              else if (angle >= 256 && angle < 512)
              {
                  angle = 512;
              }
              else if (angle >= 512 && angle < 768)
              {
                  angle = 768;
              }
              else if (angle >= 768 && angle < 1024)
              {
                  angle = 1024;
              }
              else if (angle >= 1024 && angle < 1280)
              {
                  angle = 1280;
              }
              else if (angle >= 1280 && angle < 1536)
              {
                  angle = 1536;
              }
              else if (angle >= 1536 && angle < 1792)
              {
                  angle = 1792;
              }
              else if (angle >= 1792 && angle < 2048)
              {
                  angle = 0;
              }
        }
        else if (lbKeyOn[KC_LSHIFT])
        {
            if (angle > 0 && angle <= 256)
            {angle = 2048;}
            else if (angle > 256 && angle <= 512)
            {angle = 256;}
            else if (angle > 512 && angle <= 768)
            {angle = 512;}
            else if (angle > 768 && angle <= 1024)
            {angle = 768;}
            else if (angle > 1024 && angle <= 1280)
            {angle = 1024;}
            else if (angle > 1280 && angle <= 1536)
            {angle = 1280;}
            else if (angle > 1536 && angle <= 1792)
            {angle = 1536;}
            else if ((angle > 1792 && angle <= 2048) || angle == 0)
            {angle = 1792;}
        }
        else if (angle == 0 || angle == 2048)
        {
            (angle = 1024);
        }
        else if (angle == 512)
        {
            (angle = 1536);
        }
        else if (angle == 1536)
        {
            (angle = 512);
        }
        else
        {
            (angle = 0);
        }
        packet->packet.action = PckA_SetMapRotation;
        packet->packet.actn_par1 = angle;
        lbDisplay.MiddleButton = 0;
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
      if (lbDisplay.MiddleButton >= 1)
      {
        struct Camera* cam = &player->cameras[CamIV_FrontView];
        int angle = cam->orient_a;
        if (is_game_key_pressed(Gkey_RotateMod, NULL, false))
        {
              packet->packet.control_flags |= PCtr_ViewRotateCW;
        }
        else if (lbKeyOn[KC_LSHIFT])
        {
            packet->packet.control_flags |= PCtr_ViewRotateCCW;
        }
        else
        {
            if (angle == 0 || angle == 2048)
            {
                (angle = 1024);
            }
            else if (angle == 512)
            {
                (angle = 1536);
            }
            else if (angle == 1536)
            {
                (angle = 512);
            }
            else
            {
                (angle = 0);
            }
        packet->packet.action = PckA_SetMapRotation;
        packet->packet.actn_par1 = angle;
        }
      lbDisplay.MiddleButton = 0;
      }
    }

    if (is_game_key_pressed(Gkey_ZoomToFight, &val, false))
    {
        clear_key_pressed(val);
        zoom_to_fight(player->id_number);
        return 1;
    }
    if ( is_game_key_pressed(Gkey_ZoomCrAnnoyed, &val, false) )
    {
        clear_key_pressed(val);
        zoom_to_next_annoyed_creature();
        return 1;
    }
    if (zoom_shortcuts())
    {
        return 1;
    }
    if (is_game_key_pressed(Gkey_SwitchToMap, &val, false))
    {
      clear_key_pressed(val);
      if ((player->view_mode != PVM_ParchFadeOut) && (game.small_map_state != PVM_IsometricView))
      {
          turn_off_all_window_menus();
          zoom_to_parchment_map();
      }
      return 1;
    }
    if (is_key_pressed(KC_F, KMod_ALT))
    {
        clear_key_pressed(KC_F);
        toggle_hero_health_flowers();
    }
    get_status_panel_keyboard_action_inputs();
    return 0;
}

static TbBool get_creature_passenger_action_inputs(struct PacketEx *packet)
{
    struct PlayerInfo* player = get_my_player();

    if (((game.operation_flags & GOF_Paused) == 0) || ((game.operation_flags & GOF_WorldInfluence) != 0))
        get_gui_inputs(1);
    if (player->controlled_thing_idx == 0)
        return false;
    if (right_button_released)
    {
        check_set_packet_action(player, packet, PckA_PasngrCtrlExit, player->controlled_thing_idx, 0);
        return true;
  }
  struct Thing* thing = thing_get(player->controlled_thing_idx);
  TRACE_THING(thing);
  if (!thing_exists(thing) || (player->controlled_thing_creatrn != thing->creation_turn))
  {
    check_set_packet_action(player, packet, PckA_PasngrCtrlExit, player->controlled_thing_idx, 0);
    return true;
  }
  if (is_key_pressed(KC_TAB,KMod_NONE))
  {
    clear_key_pressed(KC_TAB);
    toggle_gui_overlay_map();
  }
  return false;
}

static TbBool get_creature_control_action_inputs(struct PacketEx *packet)
{
    long keycode;
    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_my_player();
    if ( ((game.operation_flags & GOF_Paused) == 0) || ((game.operation_flags & GOF_WorldInfluence) != 0))
        get_gui_inputs(1);
    if (is_key_pressed(KC_NUMPADENTER,KMod_DONTCARE))
    {
        if (toggle_instance_cheat_menu())
            clear_key_pressed(KC_NUMPADENTER);
    }
    // also use the main keyboard enter key (while holding shift) for cheat menu
    if (is_key_pressed(KC_RETURN,KMod_SHIFT))
        {
            if (toggle_instance_cheat_menu())
                clear_key_pressed(KC_RETURN);
        }
    if (is_key_pressed(KC_F12,KMod_DONTCARE))
    {
        if (toggle_creature_cheat_menu())
            clear_key_pressed(KC_F12);
    }

    if (player->controlled_thing_idx != 0)
    {
        short make_packet = right_button_released || is_key_pressed(KC_ESCAPE, KMod_DONTCARE);
        if (!make_packet)
        {
            struct Thing* thing = thing_get(player->controlled_thing_idx);
            TRACE_THING(thing);
            if ((player->controlled_thing_creatrn != thing->creation_turn) || ((thing->alloc_flags & TAlF_Exists) == 0) || (thing->active_state == CrSt_CreatureUnconscious))
                make_packet = true;
        }
        if (make_packet)
        {
            right_button_released = 0;
            clear_key_pressed(KC_ESCAPE);
            check_set_packet_action(player, packet, PckA_DirectCtrlExit, player->controlled_thing_idx, 0);
        }
    }
    // Use the Query/Message keys and mouse wheel to scroll through query pages and go to correct query page when selecting an instance.
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    if (menu_is_active(GMnu_CREATURE_QUERY1))
    {
      if ( ( (is_key_pressed(KC_7,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD7,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,6) > 0) ) ||
           ( (is_key_pressed(KC_8,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD8,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,7) > 0) ) ||
           ( (is_key_pressed(KC_9,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD9,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,8) > 0) ) ||
           ( (is_key_pressed(KC_0,KMod_DONTCARE) || (is_key_pressed(KC_NUMPAD0,KMod_DONTCARE))) & (creature_instance_get_available_id_for_pos(thing,9) > 0) )  )
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
        if (is_key_pressed(79,KMod_DONTCARE))
        {
            clear_key_pressed(79);
            numkey = 0;
        } else 
        if (is_key_pressed(80,KMod_DONTCARE))
        {
            clear_key_pressed(80);
            numkey = 1;
        } else 
        if (is_key_pressed(81,KMod_DONTCARE))
        {
            clear_key_pressed(81);
            numkey = 2;
        } else 
        if (is_key_pressed(75,KMod_DONTCARE))
        {
            clear_key_pressed(75);
            numkey = 3;
        } else 
        if (is_key_pressed(76,KMod_DONTCARE))
        {
            clear_key_pressed(76);
            numkey = 4;
        } else 
        if (is_key_pressed(77,KMod_DONTCARE))
        {
            clear_key_pressed(77);
            numkey = 5;
        } else 
        if (is_key_pressed(71,KMod_DONTCARE))
        {
            clear_key_pressed(71);
            numkey = 6;
        } else 
        if (is_key_pressed(72,KMod_DONTCARE))
        {
            clear_key_pressed(72);
            numkey = 7;
        } else 
        if (is_key_pressed(73,KMod_DONTCARE))
        {
            clear_key_pressed(73);
            numkey = 8;
        } else 
        if (is_key_pressed(82,KMod_DONTCARE))
        {
            clear_key_pressed(82);
            numkey = 9;
        }
    }
        long val;
        for (int i = 0; i <= 15; i++)
        {
            if (is_game_key_pressed(Gkey_ZoomRoom00 + i, &val, false))
            {
                clear_key_pressed(val);
                teleport_destination = i;
            }
        
        }
        if (is_key_pressed(KC_SEMICOLON,KMod_DONTCARE))
        {
            teleport_destination = 16; // Last work room
        }
        else if (is_key_pressed(KC_SLASH,KMod_DONTCARE))
        {
            teleport_destination = 17; // Call to Arms
        }
        else if (is_key_pressed(KC_COMMA,KMod_DONTCARE))
        {
            teleport_destination = 18;
        }
    // In possession sets the screen blue when frozen, and to default when not.
    if (creature_affected_by_spell(thing, SplK_Freeze)) 
    {
        PaletteSetPlayerPalette(player, blue_palette);
    } else
    {
        PaletteSetPlayerPalette(player, engine_palette); 
    }
    if (numkey != -1)
    {
        int num_avail = 0;
        for (int idx = 0; idx < LEARNED_INSTANCES_COUNT; idx++)
        {
            struct Thing* cthing = thing_get(player->controlled_thing_idx);
            TRACE_THING(cthing);
            struct CreatureStats* crstat = creature_stats_get_from_thing(cthing);
            int inst_id = crstat->learned_instance_id[idx];
            if (creature_instance_is_available(cthing, inst_id))
            {
                if (numkey == num_avail)
                {
                    check_set_packet_action(player, packet, PckA_CtrlCrtrSetInstnc, inst_id, 0);
                    break;
                }
                num_avail++;
            }
      }
    }
    return false;
}

void get_packet_control_mouse_clicks(struct PacketEx *packet)
{
    static int synthetic_left = 0; //arbitrary state machine, not deserving own enum
    static int synthetic_right = 0;
    SYNCDBG(15,"Starting");

    if ((game.operation_flags & GOF_Paused) != 0)
    {
        return;
    }

    if ( left_button_held || synthetic_left == 1)
    {
      packet->packet.control_flags |= PCtr_LBtnHeld;
      synthetic_left = 2;
    }

    if ( right_button_held || synthetic_right == 1 )
    {
      packet->packet.control_flags |= PCtr_RBtnHeld;
      synthetic_right = 2;
    }

    if ( left_button_clicked || last_speech_event.type == KS_HAND_CHOOSE )
    {
      packet->packet.control_flags |= PCtr_LBtnClick;

      if ( last_speech_event.type == KS_HAND_CHOOSE ) {
        synthetic_left = 1;
      }
      else {
        synthetic_left = 0; //good idea to cancel current pick up, mouse takes precedence
      }
    }

    if ( right_button_clicked || last_speech_event.type == KS_HAND_ACTION )
    {
      packet->packet.control_flags |= PCtr_RBtnClick;

      if ( last_speech_event.type == KS_HAND_ACTION ) {
        synthetic_right = 1;
      }
      else {
        synthetic_right = 0; //good idea to cancel current slap
      }
    }

    if ( left_button_released || synthetic_left == 3)
    {
      packet->packet.control_flags |= PCtr_LBtnRelease;
      synthetic_left = 0;
    }

    if ( right_button_released || synthetic_right == 3 )
    {
      packet->packet.control_flags |= PCtr_RBtnRelease;
      synthetic_right = 0;
    }

    if (synthetic_right == 2) {
        synthetic_right = 3;
    }
    if (synthetic_left == 2) {
        synthetic_left = 3;
    }
}

static TbBool get_map_action_inputs(struct PacketEx *packet)
{
    struct PlayerInfo* player = get_my_player();
    long mouse_x = GetMouseX();
    long mouse_y = GetMouseY();
    // Get map coordinates from mouse position on parchment screen
    long map_x;
    long map_y;
    TbBool map_valid = point_to_overhead_map(player->acamera, mouse_x / pixel_size, mouse_y / pixel_size, &map_x, &map_y);
    if  (map_valid)
    {
        MapSubtlCoord stl_x = coord_subtile(map_x);
        MapSubtlCoord stl_y = coord_subtile(map_y);
        if (left_button_clicked) {
            left_button_clicked = 0;
        }
        if (left_button_released) {
            left_button_released = 0;
            check_set_packet_action(player, packet, PckA_ZoomFromMap, stl_x, stl_y);
            return true;
        }
    }

    if (right_button_clicked) {
        right_button_clicked = 0;
    }
    if (right_button_released) {
        right_button_released = 0;
        zoom_from_patchment_map();
        return true;
    }
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
      long keycode;
      if (is_game_key_pressed(Gkey_SwitchToMap, &keycode, false))
      {
          clear_key_pressed(keycode);
          turn_off_all_window_menus();
          zoom_from_patchment_map();
          return true;
      }
      return false;
    }
}

// TODO: Might want to initiate this in main() and pass a reference to it
// rather than using this global variable. But this works.
int global_frameskipTurn = 0;

static void get_isometric_or_front_view_mouse_inputs(
    struct PacketEx* packet, int rotate_pressed, int speed_pressed)
{
    // Reserve the scroll wheel for the resurrect and transfer creature specials
    if ((menu_is_active(GMnu_RESURRECT_CREATURE) || menu_is_active(GMnu_TRANSFER_CREATURE) || lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_LCONTROL]) == 0)
    {
        // mouse scroll zoom unaffected by frameskip
        if (map_coords_valid)
        {
            if (wheel_scrolled_up)
            {
                packet->packet.control_flags |= PCtr_ViewZoomIn;
            }
            if (wheel_scrolled_down)
            {
                packet->packet.control_flags |= PCtr_ViewZoomOut;
            }
        }
    }
    if (menu_is_active(GMnu_BATTLE) && lbKeyOn[KC_LCONTROL])
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

    long mx = my_mouse_x;
    long my = my_mouse_y;
    if (mx <= 4)
    {
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, false) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
        {
          if (!rotate_pressed)
            packet->packet.field_10 |= PCAdV_SpeedupPressed;
        }
        packet->packet.control_flags |= PCtr_MoveLeft;
    }
    if (mx >= MyScreenWidth-4)
    {
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, false) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
        {
          if (!rotate_pressed)
            packet->packet.field_10 |= PCAdV_SpeedupPressed;
        }
        packet->packet.control_flags |= PCtr_MoveRight;
    }
    if (my <= 4)
    {
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, false) || is_key_pressed(KC_UP,KMod_DONTCARE) )
        {
          if (!rotate_pressed)
            packet->packet.field_10 |= PCAdV_SpeedupPressed;
        }
        packet->packet.control_flags |= PCtr_MoveUp;
    }
    if (my >= MyScreenHeight-4)
    {
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, false) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
        {
          if (!rotate_pressed)
            packet->packet.field_10 |= PCAdV_SpeedupPressed;
        }
        packet->packet.control_flags |= PCtr_MoveDown;
    }
}

static void get_isometric_view_nonaction_inputs(struct PacketEx* packet)
{
    struct PlayerInfo* player = get_my_player();
    int rotate_pressed = is_game_key_pressed(Gkey_RotateMod, NULL, true);
    int speed_pressed = is_game_key_pressed(Gkey_SpeedMod, NULL, true);
    if ((player->allocflags & PlaF_Unknown10) != 0)
      return;
    if (speed_pressed != 0)
      packet->packet.field_10 |= PCAdV_SpeedupPressed;
    TbBool no_mods = false;
    if ((rotate_pressed != 0) || (speed_pressed != 0))
      no_mods = true;

    get_isometric_or_front_view_mouse_inputs(packet, rotate_pressed,speed_pressed);

    if ( rotate_pressed )
    {
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, no_mods) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_ViewRotateCW;
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, no_mods) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_ViewRotateCCW;
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, no_mods) || is_key_pressed(KC_UP,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_ViewZoomIn;
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, no_mods) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_ViewZoomOut;
    } else
    {
        if ( is_game_key_pressed(Gkey_RotateCW, NULL, false) )
            packet->packet.control_flags |= PCtr_ViewRotateCW;
        if ( is_game_key_pressed(Gkey_RotateCCW, NULL, false) )
            packet->packet.control_flags |= PCtr_ViewRotateCCW;
        if ( is_game_key_pressed(Gkey_ZoomIn, NULL, false) )
            packet->packet.control_flags |= PCtr_ViewZoomIn;
        if ( is_game_key_pressed(Gkey_ZoomOut, NULL, false) )
            packet->packet.control_flags |= PCtr_ViewZoomOut;
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, no_mods) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_MoveLeft;
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, no_mods) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_MoveRight;
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, no_mods) || is_key_pressed(KC_UP,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_MoveUp;
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, no_mods) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_MoveDown;
    }
}

static void get_overhead_view_nonaction_inputs(struct PacketEx* packet)
{
    SYNCDBG(19,"Starting");
    struct PlayerInfo* player = get_my_player();
    long my = my_mouse_y;
    long mx = my_mouse_x;
    int rotate_pressed = is_game_key_pressed(Gkey_RotateMod, NULL, true);
    int speed_pressed = is_game_key_pressed(Gkey_SpeedMod, NULL, true);
    if ((player->allocflags & PlaF_Unknown10) == 0)
    {
        if (speed_pressed)
          packet->packet.field_10 |= PCAdV_SpeedupPressed;
        if (rotate_pressed)
        {
          if ( is_game_key_pressed(Gkey_MoveUp, NULL, speed_pressed!=0) )
            packet->packet.control_flags |= PCtr_ViewZoomIn;
          if ( is_game_key_pressed(Gkey_MoveDown, NULL, speed_pressed!=0) )
            packet->packet.control_flags |= PCtr_ViewZoomOut;
        }
        if (my <= 4)
          packet->packet.control_flags |= PCtr_MoveUp;
        if (my >= MyScreenHeight-4)
          packet->packet.control_flags |= PCtr_MoveDown;
        if (mx <= 4)
          packet->packet.control_flags |= PCtr_MoveLeft;
        if (mx >= MyScreenWidth-4)
          packet->packet.control_flags |= PCtr_MoveRight;
    }
}

static void get_front_view_nonaction_inputs(struct PacketEx *packet)
{
    static TbClockMSec last_rotate_left_time = 0;
    static TbClockMSec last_rotate_right_time = 0;
    struct PlayerInfo* player = get_my_player();
    int rotate_pressed = is_game_key_pressed(Gkey_RotateMod, NULL, true);
    int speed_pressed = is_game_key_pressed(Gkey_SpeedMod, NULL, true);
    TbBool no_mods = false;
    if ((rotate_pressed != 0) || (speed_pressed != 0))
      no_mods = true;

    if ((player->allocflags & PlaF_Unknown10) != 0)
      return;
    if (speed_pressed != 0)
      packet->packet.field_10 |= PCAdV_SpeedupPressed;

    get_isometric_or_front_view_mouse_inputs(packet, rotate_pressed,speed_pressed);

    if ( rotate_pressed )
    {
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, no_mods) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
        {
          if ( LbTimerClock() > last_rotate_left_time+250 )
          {
            packet->packet.control_flags |= PCtr_ViewRotateCW;
            last_rotate_left_time = LbTimerClock();
          }
        }
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, no_mods) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
        {
          if ( LbTimerClock() > last_rotate_right_time+250 )
          {
            packet->packet.control_flags |= PCtr_ViewRotateCCW;
            last_rotate_right_time = LbTimerClock();
          }
        }
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, no_mods) || is_key_pressed(KC_UP,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_ViewZoomIn;
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, no_mods) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_ViewZoomOut;
    } else
    {
        if ( is_game_key_pressed(Gkey_RotateCW, NULL, false) )
        {
          if ( LbTimerClock() > last_rotate_left_time+250 )
          {
            packet->packet.control_flags |= PCtr_ViewRotateCW;
            last_rotate_left_time = LbTimerClock();
          }
        }
        if ( is_game_key_pressed(Gkey_RotateCCW, NULL, false) )
        {
          if ( LbTimerClock() > last_rotate_right_time+250 )
          {
            packet->packet.control_flags |= PCtr_ViewRotateCCW;
            last_rotate_right_time = LbTimerClock();
          }
        }
        if ( is_game_key_pressed(Gkey_MoveLeft, NULL, no_mods) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_MoveLeft;
        if ( is_game_key_pressed(Gkey_MoveRight, NULL, no_mods) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_MoveRight;
        if ( is_game_key_pressed(Gkey_MoveUp, NULL, no_mods) || is_key_pressed(KC_UP,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_MoveUp;
        if ( is_game_key_pressed(Gkey_MoveDown, NULL, no_mods) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
            packet->packet.control_flags |= PCtr_MoveDown;
    }
    if ( is_game_key_pressed(Gkey_ZoomIn, NULL, false) )
        packet->packet.control_flags |= PCtr_ViewZoomIn;
    if ( is_game_key_pressed(Gkey_ZoomOut, NULL, false) )
        packet->packet.control_flags |= PCtr_ViewZoomOut;
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
  if (top_pointed_at_x <= map_subtiles_x)
    x = top_pointed_at_x;
  else
    x = map_subtiles_x;
  if (top_pointed_at_y <= map_subtiles_y)
    y = top_pointed_at_y;
  else
    y = map_subtiles_y;
  unsigned int slb_x = subtile_slab_fast(x);
  unsigned int slb_y = subtile_slab_fast(y);
  struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
  struct SlabAttr* slbattr = get_slab_attrs(slb);
  if (slab_kind_is_door(slb->kind) && (slabmap_owner(slb) == player->id_number))
  {
    *context = 2;
    pos->x.val = (x<<8) + top_pointed_at_frac_x;
    pos->y.val = (y<<8) + top_pointed_at_frac_y;
  } else
  if (!power_hand_is_empty(player))
  {
    *context = 3;
    pos->x.val = (x<<8) + top_pointed_at_frac_x;
    pos->y.val = (y<<8) + top_pointed_at_frac_y;
  } else
  if (!subtile_revealed(x,y,player->id_number))
  {
    *context = 1;
    pos->x.val = (x<<8) + top_pointed_at_frac_x;
    pos->y.val = (y<<8) + top_pointed_at_frac_y;
  } else
  if ((slb_x >= map_tiles_x) || (slb_y >= map_tiles_y))
  {
    *context = 0;
    pos->x.val = (block_pointed_at_x<<8) + pointed_at_frac_x;
    pos->y.val = (block_pointed_at_y<<8) + pointed_at_frac_y;
  } else
  if ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
  {
    *context = 1;
    pos->x.val = (x<<8) + top_pointed_at_frac_x;
    pos->y.val = (y<<8) + top_pointed_at_frac_y;
  } else
  {
    pos->x.val = (block_pointed_at_x<<8) + pointed_at_frac_x;
    pos->y.val = (block_pointed_at_y<<8) + pointed_at_frac_y;
    struct Thing* thing = get_nearest_thing_for_hand_or_slap(player->id_number, pos->x.val, pos->y.val);
    if (!thing_is_invalid(thing))
      *context = 3;
    else
      *context = 0;
  }
  if (pos->x.val >= (map_subtiles_x << 8))
    pos->x.val = (map_subtiles_x << 8)-1;
  if (pos->y.val >= (map_subtiles_y << 8))
    pos->y.val = (map_subtiles_y << 8)-1;
  return true;
}

/**
 * Fill packet structure with non action user input in dungeon view.
 */
static void get_dungeon_control_nonaction_inputs(struct PacketEx *packet)
{
  struct Coord3d pos;
  my_mouse_x = GetMouseX();
  my_mouse_y = GetMouseY();
  struct PlayerInfo* player = get_my_player();
  map_coords_valid = false;
  if (player->work_state == PSt_CtrlDungeon)
  {
      unsigned char context;
      if (get_player_coords_and_context(&pos, &context))
      {
          // TODO: server should not care about player mouse state except light from hand
          /*
          set_players_packet_position(player, pos.x.val, pos.y.val);
          unset_players_add_flag(player, PCAdV_ContextMask);
          set_players_add_flag(player, (context << 1));
          */
          map_coords_valid = true;
    }
  }
  else if (screen_to_map(player->acamera, my_mouse_x, my_mouse_y, &pos))
  {
      /*
      set_players_packet_position(player,pos.x.val,pos.y.val);
      unset_players_add_flag(player, PCAdV_ContextMask);
      */
      map_coords_valid = true;
  }
  if (lbKeyOn[KC_LALT] && lbKeyOn[KC_X])
  {
    clear_key_pressed(KC_X);
    turn_on_menu(GMnu_QUIT);
  }
  switch (player->view_mode)
  {
  case PVM_IsometricView:
      get_isometric_view_nonaction_inputs(packet);
      break;
  case PVM_ParchmentView:
      get_overhead_view_nonaction_inputs(packet);
      break;
  case PVM_FrontView:
      get_front_view_nonaction_inputs(packet);
      break;
  }
}

static void get_map_nonaction_inputs(struct PacketEx *packet)
{
    SYNCDBG(9,"Starting");
    struct Coord3d pos;
    pos.x.val = 0;
    pos.y.val = 0;
    pos.z.val = 0;
    struct PlayerInfo* player = get_my_player();
    TbBool coords_valid = screen_to_map(player->acamera, GetMouseX(), GetMouseY(), &pos);

    if (coords_valid) {
        map_coords_valid = true;
        // Should be used only for light from player hand
        set_players_packet_position(packet, pos.x.val, pos.y.val);
    } else {
        map_coords_valid = false;
    }
    if (((game.operation_flags & GOF_Paused) == 0) && (player->view_mode == PVM_ParchmentView))
    {
        get_overhead_view_nonaction_inputs(packet);
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

static void get_creature_control_nonaction_inputs(struct PacketEx *packet)
{
  long k;
  struct PlayerInfo* player = get_my_player();

  long x = GetMouseX();
  long y = GetMouseY();
  struct Thing* thing = thing_get(player->controlled_thing_idx);
  TRACE_THING(thing);
  long pos_x = 127;
  long pos_y = 127;
  if ((player->allocflags & PlaF_Unknown8) != 0)
    return;

  // TODO: Should it be here?
  while (((MyScreenWidth >> 1) != GetMouseX()) || (GetMouseY() != y))
    LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1, y/pixel_size);

  // Set pos_x and pos_y
  if (settings.first_person_move_invert)
    pos_y = 255 * ((long)MyScreenHeight - y) / MyScreenHeight;
  else
    pos_y = 255 * y / MyScreenHeight;
  pos_x = 255 * x / MyScreenWidth;
  // Update the position based on current settings
  long i = settings.first_person_move_sensitivity + 1;
  x = pos_x - 127;
  y = pos_y - 127;
  if (i < 6)
  {
      k = 5 - settings.first_person_move_sensitivity;
      pos_x = x/k + 127;
      pos_y = y/k + 127;
  } else
  if (i > 6)
  {
      k = settings.first_person_move_sensitivity - 5;
      pos_x = k*x + 127;
      pos_y = k*y + 127;
  }
  // Bound posx and pos_y
  if (pos_x > map_subtiles_x)
    pos_x = map_subtiles_x;
  if (pos_y > map_subtiles_y)
    pos_y = map_subtiles_y;

  packet->packet.pos_x = pos_x;
  packet->packet.pos_y = pos_y;
  // Now do user actions
  if (thing_is_invalid(thing))
    return;
  if (thing->class_id == TCls_Creature)
  {
      if ( left_button_clicked )
      {
          left_button_clicked = 0;
          left_button_released = 0;
      }
      if ( right_button_clicked )
      {
          right_button_clicked = 0;
          right_button_released = 0;
      }
      if ( is_game_key_pressed(Gkey_MoveLeft, NULL, true) || is_key_pressed(KC_LEFT,KMod_DONTCARE) )
          packet->packet.control_flags |= PCtr_MoveLeft;
      if ( is_game_key_pressed(Gkey_MoveRight, NULL, true) || is_key_pressed(KC_RIGHT,KMod_DONTCARE) )
          packet->packet.control_flags |= PCtr_MoveRight;
      if ( is_game_key_pressed(Gkey_MoveUp, NULL, true) || is_key_pressed(KC_UP,KMod_DONTCARE) )
          packet->packet.control_flags |= PCtr_MoveUp;
      if ( is_game_key_pressed(Gkey_MoveDown, NULL, true) || is_key_pressed(KC_DOWN,KMod_DONTCARE) )
          packet->packet.control_flags |= PCtr_MoveDown;
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
    SYNCDBG(14,"Starting");

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

/** Fill packet struct with game action information.
 */
short get_inputs(void)
{
    if ((game.flags_cd & MFlg_IsDemoMode) != 0)
    {
        SYNCDBG(15,"Starting for demo mode");
        load_packets_for_turn(game.pckt_gameturn);
        game.pckt_gameturn++;
        get_packet_load_demo_inputs();
        return false;
    }
    if (game.packet_load_enable)
    {
        SYNCDBG(15,"Loading packet inputs");
        return get_packet_load_game_inputs();
    }
    struct PlayerInfo* player = get_my_player();
    struct PacketEx* packet = create_outgoing_input_packet();
    if ((player->allocflags & PlaF_Unknown80) != 0)
    {
        SYNCDBG(15,"Starting for creature fade");
        packet->packet.pos_x = 127;
        packet->packet.pos_y = 127;

        if ((!game_is_busy_doing_gui_string_input()) && ((game.operation_flags & GOF_Paused) != 0))
        {
            long keycode;
            if (is_game_key_pressed(Gkey_TogglePause, &keycode, false))
            {
                lbKeyOn[keycode] = 0;
                set_packet_pause_toggle(packet);
          }
        }
        return false;
    }
    SYNCDBG(15,"Starting");
    if (gui_process_inputs())
    {
        return true;
    }
    if (player->victory_state == VicS_LostLevel)
    {
        if (player->is_active != 1)
        {
            get_level_lost_inputs(packet);
            return true;
        }
        struct Thing* thing = thing_get(player->controlled_thing_idx);
        TRACE_THING(thing);
        if (!thing_is_creature(thing))
        {
            get_level_lost_inputs(packet);
            return true;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if ((cctrl->flgfield_2 & TF2_Spectator) == 0)
        {
            get_level_lost_inputs(packet);
            return true;
        }
    }
    TbBool inp_handled = false;
    if (((game.operation_flags & GOF_Paused) == 0) || ((game.operation_flags & GOF_WorldInfluence) != 0))
        inp_handled = get_gui_inputs(1);
    if (!inp_handled)
        inp_handled = get_global_inputs(packet);
    if (game_is_busy_doing_gui_string_input())
      return false;
    SYNCDBG(15,"Getting inputs for view %d",(int)player->view_type);
    switch (player->view_type)
    {
    case PVT_DungeonTop:
        if (!inp_handled)
            inp_handled = get_dungeon_control_action_inputs(packet);
        get_dungeon_control_nonaction_inputs(packet);
        get_player_gui_clicks();
        get_packet_control_mouse_clicks(packet);
        get_dungeon_speech_inputs();
        return inp_handled;
    case PVT_CreatureContrl:
        if (!inp_handled)
            inp_handled = get_creature_control_action_inputs(packet);
        get_creature_control_nonaction_inputs(packet);
        get_player_gui_clicks();
        get_packet_control_mouse_clicks(packet);
        return inp_handled;
    case PVT_CreaturePasngr:
        if (!inp_handled)
            inp_handled = get_creature_passenger_action_inputs(packet);
        get_player_gui_clicks();
        get_packet_control_mouse_clicks(packet);
        return inp_handled;
    case PVT_MapScreen:
        if (!inp_handled)
            inp_handled = get_map_action_inputs(packet);
        get_map_nonaction_inputs(packet);
        get_player_gui_clicks(); // Does nothing
        get_packet_control_mouse_clicks(packet);

        /* This will not work anymore
        // Unset button release events if we're going to do an action; this is to avoid casting
        // spells or doing other actions just after switch from parchment to dungeon view
        if (get_players_packet_action(player) != PckA_None)
            unset_players_packet_control(player, PCtr_LBtnRelease|PCtr_RBtnRelease);
        */
        return inp_handled;
    case PVT_MapFadeIn:
        if (player->view_mode != PVM_ParchFadeIn)
        {
          if ((game.system_flags & GSF_NetworkActive) == 0)
            game.operation_flags &= ~GOF_Paused;
          if (toggle_status_menu(0))
            player->field_1 |= 0x01;
          else
            player->field_1 &= ~0x01;
          check_set_packet_action(player, packet, PckA_SetViewType, PVT_MapScreen, 0);
        }
        return false;
    case PVT_MapFadeOut:
        if (player->view_mode != PVM_ParchFadeOut)
        {
          check_set_packet_action(player, packet, PckA_SetViewType, PVT_DungeonTop, 0);
        }
        return false;
    default:
        SYNCDBG(7,"Default exit");
        return false;
    }
}

void input(void)
{
    SYNCDBG(15,"Starting");

    update_mouse();
    update_key_modifiers();

    if (KeeperSpeechPopEvent(&last_speech_event)) {
      last_speech_event.type = KS_UNUSED;
    }

    if ((game_is_busy_doing_gui_string_input()) && (lbInkey>0))
    {
      lbKeyOn[lbInkey] = 0;
    }

    // TODO: Do not care about that now
    /*
        struct PlayerInfo* player = get_my_player();
        if (is_game_key_pressed(Gkey_CrtrContrlMod, NULL, false) != 0)
        {
            set_players_add_flag(player, PCAdV_CrtrContrlPressed);
        }
        else
        {
            unset_players_add_flag(player, PCAdV_CrtrContrlPressed);
        }
        if (is_game_key_pressed(Gkey_CrtrQueryMod, NULL, false) != 0)
        {
            set_players_add_flag(player, PCAdV_CrtrQueryPressed);
        }
        else
        {
            unset_players_add_flag(player, PCAdV_CrtrQueryPressed);
        }
    }
    */
    get_inputs();

    SYNCDBG(15,"Finished");
}

short get_gui_inputs(short gameplay_on)
{
  static ActiveButtonID over_slider_button = -1;
  SYNCDBG(15,"Starting");
  battle_creature_over = 0;
  gui_room_type_highlighted = -1;
  gui_door_type_highlighted = -1;
  gui_trap_type_highlighted = -1;
  gui_creature_type_highlighted = -1;
  if (gameplay_on) {
      update_creatr_model_activities_list();
      maintain_my_battle_list();
  }
  if (!lbDisplay.MLeftButton)
  {
      drag_menu_x = -999;
      drag_menu_y = -999;
      for (int idx = 0; idx < ACTIVE_BUTTONS_COUNT; idx++)
      {
        struct GuiButton *gbtn = &active_buttons[idx];
        if ((gbtn->flags & LbBtnF_Unknown01) && (gbtn->gbtype == LbBtnT_Unknown6))
            gbtn->gbactn_1 = 0;
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
      if ((gbtn->flags & LbBtnF_Unknown01) == 0)
          continue;
      if (!get_active_menu(gbtn->gmenu_idx)->is_turned_on)
          continue;
      Gf_Btn_Callback callback = gbtn->maintain_call;
      if (callback != NULL)
          callback(gbtn);
      if ((gbtn->btype_value & LbBFeF_NoMouseOver) != 0)
          continue;
      // TODO GUI Introduce circular buttons instead of specific condition for pannel map
      if ((menu_id_to_number(GMnu_MAIN) >= 0) && mouse_is_over_pannel_map(player->minimap_pos_x,player->minimap_pos_y))
          continue;
      if ( (check_if_mouse_is_over_button(gbtn) && !game_is_busy_doing_gui_string_input())
        || ((gbtn->gbtype == LbBtnT_Unknown6) && (gbtn->gbactn_1 != 0)) )
      {
          if ((fmmenu_idx == -1) || (gbtn->gmenu_idx == fmmenu_idx))
          {
            gmbtn_idx = gidx;
            gbtn->flags |= LbBtnF_Unknown10;
            busy_doing_gui = 1;
            callback = gbtn->ptover_event;
            if (callback != NULL)
                callback(gbtn);
            if (gbtn->gbtype == LbBtnT_Unknown6)
                break;
            if (gbtn->gbtype == LbBtnT_HorizSlider)
                nx_over_slider_button = gidx;
          } else
          {
            gbtn->flags &= ~LbBtnF_Unknown10;
          }
      } else
      {
          gbtn->flags &= ~LbBtnF_Unknown10;
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
          gbtn->gbactn_1 = 0;
      }
      over_slider_button = -1;
      do_sound_menu_click();
  }

  gui_button_tooltip_update(gmbtn_idx);
  if (gui_slider_button_inputs(over_slider_button))
      return true;
  result |= gui_button_click_inputs(gmbtn_idx);
  gui_clear_buttons_not_over_mouse(gmbtn_idx);
  result |= gui_button_release_inputs(gmbtn_idx);
  input_gameplay_tooltips(gameplay_on);
  SYNCDBG(15, "Finished");
  return result;
}

/******************************************************************************/
