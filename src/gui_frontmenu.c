/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_frontmenu.c
 *     GUI Menus support functions.
 * @par Purpose:
 *     Functions to manage GUI Menus in the game.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     28 May 2010 - 12 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_frontmenu.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_guibtns.h"
#include "kjm_input.h"
#include "frontend.h"
#include "gui_frontbtns.h"
#include "front_input.h"
#include "config_settings.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

/******************************************************************************/
struct GuiMenu *get_active_menu(MenuNumber num)
{
    if (num < 0)
        num = 0;
    if (num >= ACTIVE_MENUS_COUNT)
        num = 0;
    return &active_menus[num];
}

int first_monopoly_menu(void)
{
    for (int idx = 0; idx < ACTIVE_MENUS_COUNT; idx++)
    {
        struct GuiMenu* gmnu = &active_menus[idx];
        if ((gmnu->visual_state != 0) && (gmnu->is_monopoly_menu != 0))
            return idx;
  }
  return -1;
}

MenuNumber menu_id_to_number(MenuID menu_id)
{
    for (MenuNumber idx = 0; idx < ACTIVE_MENUS_COUNT; idx++)
    {
        struct GuiMenu* gmnu = &active_menus[idx];
        //SYNCDBG(8,"ID %d use %d",(int)gmnu->ident,(int)gmnu->field_1);
        if ((gmnu->visual_state != 0) && (gmnu->ident == menu_id))
            return idx;
    }
    return MENU_INVALID_ID;
}

/**
 * Checks if the given screen point is over a gui menu.
 * @param x,y Screen coordinates to check.
 * @return Returns index of the menu, or -1 if there's no menu on this point.
 */
int point_is_over_gui_menu(long x, long y)
{
    int gidx = MENU_INVALID_ID;
    for (int idx = 0; idx < ACTIVE_MENUS_COUNT; idx++)
    {
        struct GuiMenu* gmnu = &active_menus[idx];
        if (gmnu->visual_state != 2)
            continue;
        if (gmnu->is_turned_on == 0)
            continue;
        short gx = gmnu->pos_x;
        if ((x >= gx) && (x < gx + gmnu->width))
        {
            short gy = gmnu->pos_y;
            if ((y >= gy) && (y < gy + gmnu->height))
                gidx = idx;
      }
    }
    return gidx;
}

void update_busy_doing_gui_on_menu(void)
{
    int gidx = point_is_over_gui_menu(GetMouseX(), GetMouseY());
    if (gidx == -1)
        busy_doing_gui = 0;
    else
        busy_doing_gui = 1;
}

void turn_off_menu(MenuID mnu_idx)
{
    SYNCDBG(8,"Menu ID %d",(int)mnu_idx);
    if ((mnu_idx == GMnu_VIDEO) || (mnu_idx == GMnu_SOUND))
        save_settings();
    long menu_num = menu_id_to_number(mnu_idx);
    SYNCDBG(8,"Menu number %d",(int)menu_num);
    if (menu_num >= 0)
    {
        if (game_is_busy_doing_gui_string_input())
        {
            if (input_button->gmenu_idx == menu_num)
                kill_button_area_input();
        }
        struct GuiMenu* gmnu = get_active_menu(menu_num);
        gmnu->visual_state = 3;
        if (update_menu_fade_level(gmnu) == -1)
        {
            kill_menu(gmnu);
            remove_from_menu_stack(gmnu->ident);
        }
    }
}

void turn_off_roaming_menus(void)
{
    turn_off_menu(GMnu_VIDEO);
    turn_off_menu(GMnu_SOUND);
    turn_off_menu(GMnu_QUIT);
    turn_off_menu(GMnu_HOLD_AUDIENCE);
    turn_off_menu(GMnu_ARMAGEDDON);
    turn_off_menu(GMnu_TEXT_INFO);
}

void turn_off_query_menus(void)
{
    turn_off_menu(GMnu_CREATURE_QUERY1);
    turn_off_menu(GMnu_CREATURE_QUERY2);
    turn_off_menu(GMnu_CREATURE_QUERY3);
    turn_off_menu(GMnu_CREATURE_QUERY4);
}

void turn_off_all_panel_menus(void)
{
    int mnu_num = menu_id_to_number(GMnu_MAIN);
    if (mnu_num >= 0)
    {
        struct GuiMenu* gmnu = get_active_menu(mnu_num);
        setup_radio_buttons(gmnu);
  }
  if ( menu_is_active(GMnu_ROOM) )
  {
    turn_off_menu(GMnu_ROOM);
  }
  if ( menu_is_active(GMnu_SPELL) )
  {
    turn_off_menu(GMnu_SPELL);
  }
  if ( menu_is_active(GMnu_TRAP) )
  {
    turn_off_menu(GMnu_TRAP);
  }
  if ( menu_is_active(GMnu_QUERY) )
  {
    turn_off_menu(GMnu_QUERY);
  }
  if ( menu_is_active(GMnu_CREATURE) )
  {
    turn_off_menu(GMnu_CREATURE);
  }
  if ( menu_is_active(GMnu_CREATURE_QUERY1) )
  {
    turn_off_menu(GMnu_CREATURE_QUERY1);
  }
  if ( menu_is_active(GMnu_CREATURE_QUERY2) )
  {
    turn_off_menu(GMnu_CREATURE_QUERY2);
  }
  if ( menu_is_active(GMnu_CREATURE_QUERY3) )
  {
    turn_off_menu(GMnu_CREATURE_QUERY3);
  }
  if ( menu_is_active(GMnu_CREATURE_QUERY4) )
  {
    turn_off_menu(GMnu_CREATURE_QUERY4);
  }
  if ( menu_is_active(GMnu_SPELL_LOST) )
  {
    turn_off_menu(GMnu_SPELL_LOST);
  }
}

void set_menu_mode(long mnu_idx)
{
  if (!menu_is_active(mnu_idx))
  {
    turn_off_all_panel_menus();
    turn_on_menu(mnu_idx);
  }
}

short turn_off_all_window_menus(void)
{
    short result = false;
    if (menu_is_active(GMnu_QUIT))
    {
        result = true;
        turn_off_menu(GMnu_QUIT);
  }
  if (menu_is_active(GMnu_LOAD))
  {
    result = true;
    set_packet_pause_toggle();
    turn_off_menu(GMnu_LOAD);
  }
  if (menu_is_active(GMnu_SAVE))
  {
    result = true;
    set_packet_pause_toggle();
    turn_off_menu(GMnu_SAVE);
  }
  if (menu_is_active(GMnu_OPTIONS))
  {
    result = true;
    turn_off_menu(GMnu_OPTIONS);
  }
  if (menu_is_active(GMnu_VIDEO))
  {
    result = true;
    turn_off_menu(GMnu_VIDEO);
  }
  if (menu_is_active(GMnu_SOUND))
  {
    result = true;
    turn_off_menu(GMnu_SOUND);
  }
  if (menu_is_active(GMnu_ERROR_BOX))
  {
    result = true;
    turn_off_menu(GMnu_ERROR_BOX);
  }
  if (menu_is_active(GMnu_INSTANCE))
  {
    result = true;
    turn_off_menu(GMnu_INSTANCE);
  }
  if (menu_is_active(GMnu_RESURRECT_CREATURE))
  {
    result = true;
    turn_off_menu(GMnu_RESURRECT_CREATURE);
  }
  if (menu_is_active(GMnu_TRANSFER_CREATURE))
  {
    result = true;
    turn_off_menu(GMnu_TRANSFER_CREATURE);
  }
  if (menu_is_active(GMnu_ARMAGEDDON))
  {
    result = true;
    turn_off_menu(GMnu_ARMAGEDDON);
  }
  if (menu_is_active(GMnu_AUTOPILOT))
  {
    result = true;
    turn_off_menu(GMnu_AUTOPILOT);
  }
  if (menu_is_active(GMnu_SPELL_LOST))
  {
    result = true;
    turn_off_menu(GMnu_SPELL_LOST);
  }
  return result;
}

void turn_on_main_panel_menu(void)
{
  if (menu_id_to_number(GMnu_MAIN) == MENU_INVALID_ID)
  {
    turn_on_menu(GMnu_MAIN);
  }
  if (info_tag != 0)
  {
    turn_on_menu(GMnu_QUERY);
  } else
  if (room_tag != 0)
  {
    turn_on_menu(GMnu_ROOM);
  } else
  if (spell_tag != 0)
  {
    turn_on_menu(GMnu_SPELL);
  } else
  if (trap_tag != 0)
  {
    turn_on_menu(GMnu_TRAP);
  } else
  if (creature_tag != 0)
  {
    turn_on_menu(GMnu_CREATURE);
  }
}

short turn_off_all_bottom_menus(void)
{
    short result = false;
    if (menu_is_active(GMnu_TEXT_INFO))
    {
        result = true;
        turn_off_menu(GMnu_TEXT_INFO);
    }
    if (menu_is_active(GMnu_BATTLE))
    {
        result = true;
        turn_off_menu(GMnu_BATTLE);
    }
    if (menu_is_active(GMnu_DUNGEON_SPECIAL))
    {
        result = true;
        turn_off_menu(GMnu_DUNGEON_SPECIAL);
    }
    return result;
}

void turn_off_all_menus(void)
{
  turn_off_all_panel_menus();
  turn_off_all_window_menus();
  turn_off_all_bottom_menus();
}

void turn_on_menu(MenuID mnu_idx)
{
    SYNCDBG(8,"Menu ID %d",(int)mnu_idx);
    struct GuiMenu* gmnu = menu_list[mnu_idx];
    if (create_menu(gmnu) >= 0)
    {
      if (gmnu->field_1F)
        game.active_panel_mnu_idx = mnu_idx;
    }
}

void set_menu_visible_on(MenuID menu_id)
{
    long menu_num = menu_id_to_number(menu_id);
    if (menu_num < 0)
      return;
    get_active_menu(menu_num)->is_turned_on = 1;
    for (int idx = 0; idx < ACTIVE_BUTTONS_COUNT; idx++)
    {
      struct GuiButton *gbtn = &active_buttons[idx];
      if (gbtn->flags & LbBtnF_Active)
      {
          Gf_Btn_Callback callback = gbtn->maintain_call;
          if ((gbtn->gmenu_idx == menu_num) && (callback != NULL))
              callback(gbtn);
      }
    }
}

void set_menu_visible_off(MenuID menu_id)
{
    MenuNumber menu_num = menu_id_to_number(menu_id);
    if (menu_num < 0)
      return;
    get_active_menu(menu_num)->is_turned_on = 0;
}

void kill_menu(struct GuiMenu *gmnu)
{
    if (gmnu->visual_state != 0)
    {
      gmnu->visual_state = 0;
      for (int i = 0; i < ACTIVE_BUTTONS_COUNT; i++)
      {
          struct GuiButton* gbtn = &active_buttons[i];
          if ((gbtn->flags & LbBtnF_Active) && (gbtn->gmenu_idx == gmnu->number)) {
              kill_button(gbtn);
          }
      }
    }
}

void remove_from_menu_stack(short mnu_id)
{
    unsigned short i;
    for (i=0; i<no_of_active_menus; i++)
    {
        if (menu_stack[i] == mnu_id)
        {
            while (i < no_of_active_menus-1)
            {
                menu_stack[i] = menu_stack[i+1];
                i++;
            }
            break;
        }
    }
    if (i < no_of_active_menus)
      no_of_active_menus--;
}

void add_to_menu_stack(unsigned char mnu_idx)
{
    if (no_of_active_menus >= ACTIVE_MENUS_COUNT)
    {
      ERRORLOG("No more room on menu stack");
      return;
    }

    for (short i = 0; i < no_of_active_menus; i++)
    {
      if (menu_stack[i] == mnu_idx)
      { // If already in stack, move it at end of the stack.
        while (i < no_of_active_menus-1)
        {
          menu_stack[i] = menu_stack[i+1];
          i++;
        }
        menu_stack[(int)no_of_active_menus-1] = mnu_idx;
        //SYNCMSG("Menu %d moved to end of stack, at position %d.",mnu_idx,no_of_active_menus-1);
        return;
      }
    }
    // If not in stack, add at end
    menu_stack[(unsigned char)no_of_active_menus] = mnu_idx;
    no_of_active_menus++;
    SYNCDBG(9,"Menu %d put on stack, at position %d.",mnu_idx,no_of_active_menus-1);
}

long first_available_menu(void)
{
    for (short i = 0; i < ACTIVE_MENUS_COUNT; i++)
    {
        if (active_menus[i].visual_state == 0)
            return i;
    }
    return -1;
}

void turn_off_event_box_if_necessary(PlayerNumber plyr_idx, unsigned char event_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon->visible_event_idx != event_idx) {
        return;
    }
    dungeon->visible_event_idx = 0;
    if (is_my_player_number(plyr_idx))
    {
        turn_off_menu(GMnu_TEXT_INFO);
        turn_off_menu(GMnu_BATTLE);
        turn_off_menu(GMnu_DUNGEON_SPECIAL);
    }
}

/******************************************************************************/
