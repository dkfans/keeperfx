/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_saves.c
 *     GUI menus for saved games support (save and load screens).
 * @par Purpose:
 *     Functions to show and maintain menus used for saving and loading.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontmenu_saves.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "config_strings.h"
#include "game_saves.h"
#include "gui_draw.h"
#include "gui_frontbtns.h"
#include "gui_soundmsgs.h"
#include "player_data.h"
#include "packets.h"
#include "frontend.h"
#include "front_input.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "keeperfx.hpp"

/******************************************************************************/
int frontend_load_game_button_to_index(struct GuiButton *gbtn)
{
    long gbidx = (unsigned long)gbtn->content;
    int k = -1;
    for (int i = gbidx + load_game_scroll_offset - 45; i >= 0; i--)
    {
        struct CatalogueEntry* centry;
        do
        {
            k++;
            if (k >= TOTAL_SAVE_SLOTS_COUNT)
                return -1;
            centry = &save_game_catalogue[k];
        } while ((centry->flags & CEF_InUse) == 0);
  }
  return k;
}

void gui_load_game_maintain(struct GuiButton *gbtn)
{
  long slot_num;
  if (gbtn != NULL)
      slot_num = gbtn->btype_value & LbBFeF_IntValueMask;
  else
      slot_num = 0;
  struct CatalogueEntry* centry = &save_game_catalogue[slot_num];
  if ((centry->flags & CEF_InUse) != 0)
      gbtn->flags |= LbBtnF_Enabled;
  else
      gbtn->flags &=  ~LbBtnF_Enabled;
}

void gui_load_game(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    long slot_num = gbtn->btype_value & LbBFeF_IntValueMask;
    if (!load_game(slot_num))
    {
        ERRORLOG("Loading game %d failed; quitting.", (int)slot_num);
        // Even on quit, we still should unpause the game
        set_players_packet_action(player, PckA_TogglePause, 0, 0, 0, 0);
        quit_game = 1;
        return;
  }
  set_players_packet_action(player, PckA_TogglePause, 0, 0, 0, 0);
}

void draw_load_button(struct GuiButton *gbtn)
{
    if (gbtn == NULL) return;
    int bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, 2, 94);
    if ((gbtn->gbactn_1) || (gbtn->gbactn_2))
    {
        draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, gbtn->width);
        draw_lit_bar64k(gbtn->scr_pos_x - 6*bs_units_per_px/16, gbtn->scr_pos_y - 6*bs_units_per_px/16, bs_units_per_px, gbtn->width + 6*bs_units_per_px/16);
    } else
    {
        draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, gbtn->width);
    }
    if (gbtn->content != NULL)
    {
        sprintf(gui_textbuf, "%s", (const char *)gbtn->content);
        draw_button_string(gbtn, 300, gui_textbuf);
    }
}

void gui_save_game(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    if (strcasecmp((char*)gbtn->content, get_string(GUIStr_SlotUnused)) != 0)
    {
        long slot_num = (gbtn->btype_value & LbBFeF_IntValueMask) % TOTAL_SAVE_SLOTS_COUNT;
        fill_game_catalogue_slot(slot_num, (char*)gbtn->content);
        if (save_game(slot_num))
        {
            output_message(SMsg_GameSaved, 0, true);
        } else
      {
          ERRORLOG("Error in save!");
          create_error_box(GUIStr_ErrorSaving);
      }
  }
  set_players_packet_action(player, PckA_TogglePause, 0, 0, 0, 0);
}

void update_loadsave_input_strings(struct CatalogueEntry *game_catalg)
{
    SYNCDBG(6,"Starting");
    for (long slot_num = 0; slot_num < TOTAL_SAVE_SLOTS_COUNT; slot_num++)
    {
        struct CatalogueEntry* centry = &game_catalg[slot_num];
        const char* text;
        if ((centry->flags & CEF_InUse) != 0)
            text = centry->textname;
        else
          text = get_string(GUIStr_SlotUnused);
        strncpy(input_string[slot_num], text, SAVE_TEXTNAME_LEN);
    }
}

void frontend_load_game(struct GuiButton *gbtn)
{
    int i = frontend_load_game_button_to_index(gbtn);
    if (i < 0)
        return;
    game.numfield_15 = i;
    if (is_save_game_loadable(i))
    {
        frontend_set_state(FeSt_LOAD_GAME);
  } else
  {
    save_catalogue_slot_disable(i);
    if (!initialise_load_game_slots())
      frontend_set_state(FeSt_MAIN_MENU);
  }
}

void frontend_draw_load_game_button(struct GuiButton *gbtn)
{
    int i = frontend_load_game_button_to_index(gbtn);
    if (i < 0)
        return;
    // Select font to draw
    int font_idx = frontend_button_caption_font(gbtn, frontend_mouse_over_button);
    LbTextSetFont(frontend_font[font_idx]);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    // Set drawing window and draw the text
    int tx_units_per_px = (gbtn->height * 13 / 11) * 16 / LbTextLineHeight();
    int height = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, height);
    LbTextDrawResized(0, 0, tx_units_per_px, save_game_catalogue[i].textname);
}

void frontend_load_game_up_maintain(struct GuiButton *gbtn)
{
    if (load_game_scroll_offset != 0)
    {
        gbtn->flags |= LbBtnF_Enabled;
    }
    else
    {
        gbtn->flags &=  ~LbBtnF_Enabled;
    }
    if (wheel_scrolled_up || (is_key_pressed(KC_UP,KMod_NONE)))
    {
        if (load_game_scroll_offset > 0)
        {
            load_game_scroll_offset--;
        }
    }
}

void frontend_load_game_down_maintain(struct GuiButton *gbtn)
{
    if (load_game_scroll_offset < number_of_saved_games-frontend_load_menu_items_visible+1)
    {
        gbtn->flags |= LbBtnF_Enabled;
    }
    else
    {
        gbtn->flags &=  ~LbBtnF_Enabled;
    }
    if (wheel_scrolled_down || (is_key_pressed(KC_DOWN,KMod_NONE)))
    {
        if (load_game_scroll_offset < number_of_saved_games-frontend_load_menu_items_visible+1)
        {
            load_game_scroll_offset++;
        }
	}
}

void frontend_load_game_up(struct GuiButton *gbtn)
{
  if (load_game_scroll_offset > 0)
    load_game_scroll_offset--;
}

void frontend_load_game_down(struct GuiButton *gbtn)
{
    if (load_game_scroll_offset < number_of_saved_games-frontend_load_menu_items_visible+1)
      load_game_scroll_offset++;
}

void frontend_load_game_scroll(struct GuiButton *gbtn)
{
    load_game_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), frontend_load_menu_items_visible-2, number_of_saved_games);
}

void frontend_draw_games_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, load_game_scroll_offset, frontend_load_menu_items_visible-2, number_of_saved_games);
}

void init_load_menu(struct GuiMenu *gmnu)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo* player = get_my_player();
  set_players_packet_action(player, PckA_UpdatePause, 1, 1, 0, 0);
  load_game_save_catalogue();
  update_loadsave_input_strings(save_game_catalogue);
}

void init_save_menu(struct GuiMenu *gmnu)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo* player = get_my_player();
  set_players_packet_action(player, PckA_UpdatePause, 1, 1, 0, 0);
  load_game_save_catalogue();
  update_loadsave_input_strings(save_game_catalogue);
}
/******************************************************************************/
