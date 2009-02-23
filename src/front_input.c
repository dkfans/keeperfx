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
 * @date     20 Jan 2009 - 30 Jan 2009
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

#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_sprfnt.h"
#include "bflib_datetm.h"

#include "kjm_input.h"
#include "frontend.h"
#include "scrcapt.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_input(void);
DLLIMPORT char _DK_game_is_busy_doing_gui_string_input(void);

/******************************************************************************/
char game_is_busy_doing_gui_string_input(void)
{
  return _DK_game_is_busy_doing_gui_string_input();
}

/*
 *  Reacts on a keystoke by sending text message packets.
 */
short get_players_message_inputs(void)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if (is_key_pressed(KC_RETURN,KM_NONE))
  {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, PckT_PlyrMsgEnd, 0, 0, 0, 0);
      clear_key_pressed(KC_RETURN);
      return true;
  }
  lbFontPtr = winfont;
  int msg_width = pixel_size * LbTextStringWidth(player->strfield_463);
  if ( (is_key_pressed(KC_BACK,KM_DONTCARE)) || (msg_width < 450) )
  {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt,PckT_PlyrMsgChar,lbInkey,key_modifiers,0,0);
      clear_key_pressed(lbInkey);
      return true;
  }
  return false;
}

/*
 * Captures the screen to make a gameplay movie or screenshot image.
 * @return Returns true if packet was created, false otherwise.
 */
short get_screen_capture_inputs(void)
{
  if (is_key_pressed(KC_M,KM_SHIFT))
  {
    if (game.numfield_A & 0x08)
      movie_record_stop();
    else
      movie_record_start();
    clear_key_pressed(KC_M);
  }
  if (is_key_pressed(KC_C,KM_SHIFT))
  {
    game.numfield_A |= 0x10;
    clear_key_pressed(KC_C);
  }
  return false;
}

/*
 * Handles game speed control inputs.
 * @return Returns true if packet was created, false otherwise.
 */
short get_speed_control_inputs(void)
{
  if (is_key_pressed(KC_ADD,KM_CONTROL))
  {
      game.timingvar1 += 2;
      if (game.timingvar1 < 0)
          game.timingvar1 = 0;
      else
      if ( game.timingvar1 > 64 )
          game.timingvar1 = 64;
      clear_key_pressed(KC_ADD);
  }
  if (is_key_pressed(KC_SUBTRACT,KM_CONTROL))
  {
      game.timingvar1 -= 2;
        if (game.timingvar1 < 0)
        game.timingvar1 = 0;
      else
      if ( game.timingvar1 > 64 )
        game.timingvar1 = 64;
      clear_key_pressed(KC_SUBTRACT);
  }
  return false;
}

/*
 * Handles minimap control inputs.
 * @return Returns true if packet was created, false otherwise.
 */
short get_minimap_control_inputs(void)
{
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  struct Packet *pckt;
  short packet_made;
  packet_made = false;
  if (is_key_pressed(KC_SUBTRACT,KM_NONE))
  {
      if ( player->minimap_zoom < 0x0800 )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, PckT_SetMinimapConf, 2 * player->minimap_zoom, 0, 0, 0);
        packet_made = true;
      }
      clear_key_pressed(KC_SUBTRACT);
      if (packet_made) return true;
  }
  if (is_key_pressed(KC_ADD,KM_NONE))
  {
      if ( player->minimap_zoom > 0x0080 )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, PckT_SetMinimapConf, player->minimap_zoom >> 1, 0, 0, 0);
        packet_made = true;
      }
      clear_key_pressed(KC_ADD);
      if (packet_made) return true;
  }
  return false;
}

/*
 * Handles screen control inputs.
 * @return Returns true if packet was created, false otherwise.
 */
short get_screen_control_inputs(void)
{
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  struct Packet *pckt;
  short packet_made;
  packet_made = false;
  if (is_key_pressed(KC_R,KM_ALT))
  {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, PckT_SwitchScrnRes, 0, 0, 0, 0);
      packet_made = true;
      clear_key_pressed(KC_R);
      if (packet_made) return true;
  }
  return false;
}

short get_global_inputs(void)
{
  if ( game_is_busy_doing_gui_string_input() && (!game_is_busy_doing_gui()) )
    return false;
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  struct Packet *pckt;
  long keycode;
  if ((player->field_0 & 0x04) != 0)
  {
    get_players_message_inputs();
    return true;
  }
  if ((player->field_452 == 1) && ((game.numfield_A & 0x01) != 0))
  {
      if (is_key_pressed(KC_RETURN,KM_NONE))
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, PckT_PlyrMsgBegin, 0, 0, 0, 0);
        clear_key_pressed(KC_RETURN);
        return true;
      }
  }
  // Code for debugging purposes
  if ( is_key_pressed(KC_D,KM_ALT) )
  {
    LbSyncLog("REPORT for gameturn %d\n",game.seedchk_random_used);
    // Timing report
    LbSyncLog("Now time is %d, last loop time was %d, clock is %d, requested fps is %d\n",LbTimerClock(),last_loop_time,clock(),game.num_fps);
  }

  int idx;
  for (idx=KC_F1;idx<=KC_F8;idx++)
  {
      if ( is_key_pressed(idx,KM_ALT) )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, PckT_PlyrFastMsg, idx-KC_F1, 0, 0, 0);
        clear_key_pressed(idx);
        return true;
      }
  }
  if ( (player->field_4B0 != 14) && (player->field_4B0 != 15) && (!game_is_busy_doing_gui()) )
  {
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 22, 0, 0, 0, 0);
        clear_key_pressed(keycode);
        return true;
      }
  }
  if ((game.numfield_C & 0x01) != 0)
      return true;
  if (get_speed_control_inputs())
      return true;
  if (get_minimap_control_inputs())
      return true;
  if (get_screen_control_inputs())
      return true;
  if (get_screen_capture_inputs())
      return true;
  if (is_key_pressed(KC_SPACE,KM_NONE))
  {
      if ( player->field_29 )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 5, 0, 0, 0, 0);
        clear_key_pressed(KC_SPACE);
        return true;
      }
  }
  if ( _DK_is_game_key_pressed(29, &keycode, 0) )
  {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 111, 0, 0, 0, 0);
      clear_key_pressed(keycode);
  }
  return false;
}

short get_level_lost_inputs(void)
{
  static const char *func_name="get_level_lost_inputs";
//  _DK_get_level_lost_inputs();
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  long keycode;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  if (player->field_0 & 0x04)
  {
    get_players_message_inputs();
    return true;
  }
  if ((game.numfield_A & 0x01) != 0)
  {
    if (is_key_pressed(KC_RETURN,KM_NONE))
    {
      struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 13, 0,0,0,0);
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
  if (is_key_pressed(KC_SPACE,KM_NONE))
  {
    struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
    set_packet_action(pckt, 5, 0,0,0,0);
    clear_key_pressed(KC_SPACE);
  }
  if (player->field_452 == 4)
  {
    int screen_x = GetMouseX() - 150;
    int screen_y = GetMouseY() - 56;
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      if (((game.numfield_A & 0x01) == 0) && (lbDisplay.PhysicalScreenWidth <= 320))
      {
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 80, 6,0,0,0);
      }
      else
      {
        toggle_status_menu((game.numfield_C & 0x40) != 0);
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 120, 1,0,0,0);
      }
    } else
    if ( right_button_released )
    {
        right_button_released = 0;
        if ( (game.numfield_A & 0x01) || lbDisplay.PhysicalScreenWidth > 320 )
        {
          toggle_status_menu((game.numfield_C & 0x40) != 0);
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          set_packet_action(pckt, 120, 1,0,0,0);
        }
        else
        {
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          set_packet_action(pckt, 80, 6,0,0,0);
        }
    } else
    if ( _DK_left_button_released )
    {
        int actn_x = 3*screen_x/4 + 1;
        int actn_y = 3*screen_y/4 + 1;
        if  ((actn_x >= 0) && (actn_x < map_subtiles_x) && (actn_y >= 0) && (actn_y < map_subtiles_y))
        {
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          set_packet_action(pckt, 81, actn_x,actn_y,0,0);
          _DK_left_button_released = 0;
        }
    }
  } else
  if ( player->field_452 == 1 )
  {
    if (is_key_pressed(KC_TAB,KM_DONTCARE))
    {
        if ((player->field_37 == 2) || (player->field_37 == 5))
        {
          clear_key_pressed(KC_TAB);
          toggle_gui();
        }
    } else
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      if (player->field_37 != 7)
      {
        turn_off_all_window_menus();
        game.numfield_C = (game.numfield_C ^ (unsigned __int8)(2 * game.numfield_C)) & 0x40 ^ game.numfield_C;
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        if ((game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320))
        {
              if (toggle_status_menu(0))
                game.numfield_C |= 0x40;
              else
                game.numfield_C &= 0xBF;
              set_packet_action(pckt, 119, 4,0,0,0);
        } else
        {
              set_packet_action(pckt, 80, 5,0,0,0);
        }
        _DK_turn_off_roaming_menus();
      }
    }
  }
  if (is_key_pressed(KC_ESCAPE,KM_DONTCARE))
  {
    clear_key_pressed(KC_ESCAPE);
    if ( a_menu_window_is_active() )
      turn_off_all_window_menus();
    else
      turn_on_menu(8);
  }
  struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
  struct Thing *thing;
  short inp_done=false;
  switch (player->field_452)
  {
    case 1:
      inp_done=menu_is_active(38);
      if ( !inp_done )
      {
        if ((game.numfield_C & 0x20) != 0)
        {
          _DK_initialise_tab_tags_and_menu(3);
          turn_off_all_panel_menus();
          turn_on_menu(38);
        }
      }
      inp_done=get_gui_inputs(1);
      if ( !inp_done )
      {
        if (player->field_453 == 15)
        {
          set_player_instance(player, 10, 0);
        } else
        {
          inp_done=_DK_get_small_map_inputs(player->mouse_x, player->mouse_y, player->minimap_zoom / (3-pixel_size));
          if ( !inp_done )
            _DK_get_bookmark_inputs();
          _DK_get_dungeon_control_nonaction_inputs();
        }
      }
      break;
    case 2:
      thing = game.things_lookup[player->field_2F];
      if (thing->class_id == 5)
      {
        struct CreatureControl *crctrl;
        crctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
        if ((crctrl->field_2 & 0x02) == 0)
          set_packet_action(pckt, 33, player->field_2F,0,0,0);
      } else
      {
        set_packet_action(pckt, 33, player->field_2F,0,0,0);
      }
      break;
    case 3:
      set_packet_action(pckt, 32, player->field_2F,0,0,0);
      break;
    case 4:
      if ( menu_is_active(38) )
      {
        if ((game.numfield_C & 0x20) != 0)
          turn_off_menu(38);
      }
      break;
    default:
      return false;
  }
}

long get_dungeon_control_action_inputs(void)
{
  return _DK_get_dungeon_control_action_inputs();
}

short get_creature_passenger_action_inputs(void)
{
  if ( ((game.numfield_C & 0x01)==0) || (game.numfield_C & 0x80) )
      get_gui_inputs(1);
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ( !player->field_2F )
    return false;
  if (_DK_right_button_released)
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  struct Thing *thing;
  thing = game.things_lookup[player->field_2F];
  if ((player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0) )
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  if (is_key_pressed(KC_TAB,KM_NONE))
  {
    clear_key_pressed(KC_TAB);
    toggle_gui_overlay_map();
  }
  return false;
}

short get_creature_control_action_inputs(void)
{
  static const char *func_name="get_creature_control_action_inputs";
  struct PlayerInfo *player;
  long keycode;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ( ((game.numfield_C & 0x01)==0) || (game.numfield_C & 0x80) )
    get_gui_inputs(1);
  if (is_key_pressed(KC_NUMPADENTER,KM_DONTCARE))
  {
      clear_key_pressed(KC_NUMPADENTER);
      // Toggle cheat menu
      if ( (gui_box==NULL) || (gui_box_is_not_valid(gui_box)) )
      {
        gui_box=gui_create_box(200,20,gui_instance_option_list);
/*
        player->unknownbyte  |= 0x08;
        game.unknownbyte |= 0x08;
*/
      } else
      {
        gui_delete_box(gui_box);
        gui_box=NULL;
/*
        player->unknownbyte &= 0xF7;
        game.unknownbyte &= 0xF7;
*/
      }
      return 1;
  }
  if (is_key_pressed(KC_F12,KM_DONTCARE))
  {
      clear_key_pressed(KC_F12);
      // Cheat sub-menus
      if ( (gui_cheat_box==NULL) || (gui_box_is_not_valid(gui_cheat_box)) )
      {
        gui_cheat_box=gui_create_box(150,20,gui_creature_cheat_option_list);
/*
        player->unknownbyte  |= 0x08;
*/
      } else
      {
        gui_delete_box(gui_cheat_box);
        gui_cheat_box=NULL;
/*
        player->unknownbyte &= 0xF7;
*/
      }
  }

  if ( player->field_2F )
  {
    short make_packet = _DK_right_button_released;
    if (!make_packet)
    {
      struct Thing *thing;
      thing = game.things_lookup[player->field_2F];
      if ( (player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0)
         || (thing->field_7 == 67) )
        make_packet = true;
    }
    if (make_packet)
    {
      struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_right_button_released = 0;
      set_packet_action(pckt, 33, player->field_2F,0,0,0);
    }
  }
  if ( is_key_pressed(KC_TAB,KM_NONE) )
  {
    clear_key_pressed(KC_TAB);
    toggle_gui();
  }
  int numkey;
  numkey = -1;
  {
    for (keycode=KC_1;keycode<=KC_0;keycode++)
    {
      if ( is_key_pressed(keycode,KM_NONE) )
      {
        clear_key_pressed(keycode);
        numkey = keycode-2;
        break;
      }
    }
  }
  if ( numkey != -1 )
  {
    int idx;
    int instnce;
    int num_avail;
    num_avail = 0;
    for (idx=0;idx<10;idx++)
    {
      struct Thing *thing;
      thing = game.things_lookup[player->field_2F%THINGS_COUNT];
      instnce = game.creature_stats[thing->model%CREATURE_TYPES_COUNT].field_80[idx];
      if ( creature_instance_is_available(thing,instnce) )
      {
        if ( numkey == num_avail )
        {
          struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
          set_packet_action(pckt, 39, instnce,0,0,0);
          break;
        }
        num_avail++;
      }
    }
  }
  return false;
}

short get_map_action_inputs()
{
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  long keycode;
  long mouse_x = GetMouseX();
  long mouse_y = GetMouseY();
  int mappos_x = 3 * (mouse_x - 150) / 4 + 1;
  int mappos_y = 3 * (mouse_y - 56) / 4 + 1;
  if ((mappos_x >= 0) && (mappos_x < map_subtiles_x) && (mappos_y >= 0) && (mappos_y < map_subtiles_y) )
  {
    if ( left_button_clicked )
    {
      left_button_clicked = 0;
    }
    if ( _DK_left_button_released )
    {
      struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 81,mappos_x,mappos_y,0,0);
      _DK_left_button_released = 0;
      return true;
    }
  }

  if ( _DK_right_button_clicked )
    _DK_right_button_clicked = 0;
  if ( _DK_right_button_released )
  {
    _DK_right_button_released = 0;
    zoom_from_map();
    return true;
  } else
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    if (pckt->field_5)
      return true;
    if (is_key_pressed(KC_F8,KM_NONE))
    {
      clear_key_pressed(KC_F8);
      toggle_tooltips();
    }
    if (is_key_pressed(KC_NUMPADENTER,KM_NONE))
    {
      clear_key_pressed(KC_NUMPADENTER);
      toggle_main_cheat_menu();
    }
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      turn_off_all_window_menus();
      zoom_from_map();
      return true;
    }
    return false;
  }
}

short get_inputs(void)
{
  static const char *func_name="get_inputs";
  //return _DK_get_inputs();
  long keycode;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  if (game.flags_cd & 0x01)
  {
    _DK_load_packets_for_turn(game.gameturn);
    game.gameturn++;
    if (is_key_pressed(KC_SPACE,KM_DONTCARE) ||
        is_key_pressed(KC_ESCAPE,KM_DONTCARE) ||
        is_key_pressed(KC_RETURN,KM_DONTCARE) ||
        (lbKeyOn[KC_LALT] && lbKeyOn[KC_X]) ||
        left_button_clicked)
    {
      clear_key_pressed(KC_SPACE);
      clear_key_pressed(KC_ESCAPE);
      clear_key_pressed(KC_RETURN);
      lbKeyOn[KC_X] = 0;
      left_button_clicked = 0;
      quit_game = 1;
    }
    //LbSyncLog("%s: Loading packets finished\n", func_name);
    return false;
  }
  if ( game.field_149E81 )
  {
    _DK_load_packets_for_turn(game.gameturn);
    game.gameturn++;
    if ( lbKeyOn[KC_LALT] && lbKeyOn[KC_X] )
    {
      lbKeyOn[KC_X] = 0;
      if ( game.numfield_A & 0x01 )
        _DK_LbNetwork_Stop();
      quit_game = 1;
      exit_keeper = 1;
    }
    //LbSyncLog("%s: Quit packet posted\n", func_name);
    return false;
  }
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (player->field_0 & 0x80)
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    pckt->field_A = 127;
    pckt->field_C = 127;
    if ((!game_is_busy_doing_gui()) && (game.numfield_C & 0x01))
    {
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        lbKeyOn[keycode] = 0;
        set_packet_action(pckt, 22, 0,0,0,0);
      }
    }
    return false;
  }
  if (player->field_29 == 2)
  {
    if (player->field_2C != 1)
    {
      get_level_lost_inputs();
      return true;
    }
    struct Thing *thing;
    thing = game.things_lookup[player->field_2F%THINGS_COUNT];
    if ( (thing <= game.things_lookup[0]) || (thing->class_id != 5) )
    {
      get_level_lost_inputs();
      return true;
    }
    struct CreatureControl *crctrl;
    crctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
    if ((crctrl->field_2 & 0x02) == 0)
    {
      get_level_lost_inputs();
      return true;
    }
  }
  short inp_handled = false;
  if ( !(game.numfield_C & 0x01) || (game.numfield_C & 0x80) )
    inp_handled = get_gui_inputs(1);
  if ( !inp_handled )
    inp_handled = get_global_inputs();
  if (game_is_busy_doing_gui())
    return false;
  struct Packet *pckt;
  switch ( player->field_452 )
  {
  case 1:
      if (!inp_handled)
        inp_handled=get_dungeon_control_action_inputs();
      _DK_get_dungeon_control_nonaction_inputs();
      get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 2:
      if (!inp_handled)
        inp_handled = get_creature_control_action_inputs();
      _DK_get_creature_control_nonaction_inputs();
      get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 3:
      if ( inp_handled )
      {
        get_player_gui_clicks();
        _DK_get_packet_control_mouse_clicks();
        return true;
      } else
      if ( get_creature_passenger_action_inputs() )
      {
        _DK_get_packet_control_mouse_clicks();
        return true;
      } else
      {
        get_player_gui_clicks();
        _DK_get_packet_control_mouse_clicks();
        return false;
      }
  case 4:
      if (!inp_handled)
        inp_handled = get_map_action_inputs();
      _DK_get_map_nonaction_inputs();
      get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 5:
      if (player->field_37==6)
        return false;
      if ( !(game.numfield_A & 0x01) )
        game.numfield_C &= 0xFE;
      if ( toggle_status_menu(0) )
        player->field_1 |= 0x01;
      else
        player->field_1 &= 0xFE;
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 80, 4,0,0,0);
      return false;
  case 6:
      if (player->field_37 != 7)
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 80, 1,0,0,0);
      }
      return false;
  default:
#if (BFDEBUG_LEVEL > 7)
      LbSyncLog("%s: Default exit\n", func_name);
#endif
      return false;
  }
}

void input(void)
{
  static const char *func_name="input";
#if (BFDEBUG_LEVEL > 4)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_input();return;
  update_key_modifiers();
  if ((game_is_busy_doing_gui()) && (lbInkey>0))
  {
      lbKeyOn[lbInkey] = 0;
  }

  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
  if (_DK_is_game_key_pressed(27, 0, 0) != 0)
    pckt->field_10 |= 0x20;
  else
    pckt->field_10 &= 0xDFu;
  if (_DK_is_game_key_pressed(28, 0, 0) != 0)
    pckt->field_10 |= 0x40;
  else
    pckt->field_10 &= 0xBFu;

  get_inputs();
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
