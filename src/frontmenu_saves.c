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
#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
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
#include "gui_vscroll.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "sprites.h"
#include "keeperfx.hpp"
#include "kfx/save/SaveManager.h"
#include "post_inc.h"

/******************************************************************************/
int frontend_load_game_button_to_index(struct GuiButton *gbtn)
{
    long gbidx = gbtn->content.lval;
    int k = -1;
    for (int i = gbidx + load_game_scroll_offset - 45; i >= 0; i--)
    {
        struct CatalogueEntry* centry;
        do
        {
            k++;
            if (k >= SaveManager_SlotCount())
                return -1;
            centry = SaveManager_Entry(k);
        } while ((centry->flags & CEF_InUse) == 0);
  }
  return k;
}

/** Actual savegame slot shown in on-screen row btype_value (0..7), taking the
 *  scroll offset into account. */
static long loadsave_row_slot(const struct GuiButton *gbtn)
{
    long row = (gbtn != NULL) ? (gbtn->btype_value & LbBFeF_IntValueMask) : 0;
    return gui_vscroll_offset + row;
}

void gui_load_game_maintain(struct GuiButton *gbtn)
{
    long slot_num = loadsave_row_slot(gbtn);
    struct CatalogueEntry* centry = SaveManager_Entry((int)slot_num);
    if ((centry != NULL) && ((centry->flags & CEF_InUse) != 0))
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &=  ~LbBtnF_Enabled;
}

void gui_load_game(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    long slot_num = loadsave_row_slot(gbtn);
    enum SaveCheckResult why;
    enum SaveLoadOutcome outcome = SaveManager_Load((int)slot_num, &why);
    if (outcome == SvLoad_Refused)
    {
        // Nothing has been changed, so the current game carries on
        set_players_packet_action(player, PckA_TogglePause, 0, 0, 0, 0);
        create_error_box(SaveManager_CheckMessage(why));
        return;
    }
    if (outcome == SvLoad_Broken)
    {
        // Part of the save was already applied, so the current game can't carry on.
        // Use frontend_queue_message, not frontend_load_game_failed: this load
        // started in-game (mid-campaign), so the usual post-game menu logic
        // should still pick the destination (e.g. the landview), not the
        // load/save screen a frontend-started load would return to.
        set_players_packet_action(player, PckA_TogglePause, 0, 0, 0, 0);
        frontend_queue_message(GUIStr_SaveLoadFailed);
        quit_game = 1;
        return;
    }
}

void draw_load_button(struct GuiButton *gbtn)
{
    if (gbtn == NULL) return;
    int bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, GBS_frontend_button_std_c, 94);
    int width = gbtn->width;
    TbBool low_res = (MyScreenHeight < 400);
    if (low_res)
    {
        width += 32;
    }
    if ((gbtn->button_state_left_pressed) || (gbtn->button_state_right_pressed))
    {
        draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, width);
        int lit_width = gbtn->width + 6*units_per_pixel/16;
        if (low_res)
        {
            lit_width += 32;
        }
        draw_lit_bar64k(gbtn->scr_pos_x - 6*units_per_pixel/16, gbtn->scr_pos_y - 6*units_per_pixel/16, bs_units_per_px, lit_width);
    } else
    {
        draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, width);
    }
    if (gbtn->content.str != NULL)
    {
        long slot_num = loadsave_row_slot(gbtn);
        const struct CatalogueEntry* centry = SaveManager_Entry((int)slot_num);
        TbBool unloadable = (centry != NULL) && ((centry->flags & CEF_Unloadable) != 0) && (frontend_font[3] != NULL);
        const struct TbSpriteSheet *font_mem = lbFontPtr;
        if (unloadable)
            LbTextSetFont(frontend_font[3]);
        snprintf(gui_textbuf, sizeof(gui_textbuf), "%s", gbtn->content.str);
        draw_button_string(gbtn, (gbtn->width*32 + 16)/gbtn->height, gui_textbuf);
        if (unloadable)
            LbTextSetFont(font_mem);
    }
}

void gui_save_game(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    if (strcasecmp(gbtn->content.str, get_string(GUIStr_SlotUnused)) != 0)
    {
        long slot_num = loadsave_row_slot(gbtn);
        SaveManager_FillSlot((int)slot_num, gbtn->content.str);
        if (SaveManager_Save((int)slot_num))
        {
            output_message(SMsg_GameSaved, 0);
        } else
      {
          create_error_box(GUIStr_ErrorSaving);
      }
  }
  set_players_packet_action(player, PckA_UpdatePause, local_state.paused_state_restore, 0, 0, 0);
}

void update_loadsave_input_strings(void)
{
    SYNCDBG(6,"Starting");
    // Fill the 8 on-screen rows from the current scroll window [offset, offset+8).
    for (long row = 0; row < GUI_VSCROLL_VISIBLE; row++)
    {
        long slot_num = gui_vscroll_offset + row;
        const char* text;
        const struct CatalogueEntry* centry = SaveManager_Entry((int)slot_num);
        if ((centry != NULL) && ((centry->flags & CEF_InUse) != 0))
            text = centry->textname;
        else
            text = get_string(GUIStr_SlotUnused);
        snprintf(input_string[row], SAVE_TEXTNAME_LEN, "%s", text);
    }
}

void frontend_load_game(struct GuiButton *gbtn)
{
    int i = frontend_load_game_button_to_index(gbtn);
    if (i < 0)
        return;
    enum SaveCheckResult chk = SaveManager_Check(i, NULL, 0);
    if (chk == SvChk_Loadable)
    {
        frontend_start_load_game(i);
    } else
    if (chk == SvChk_Missing)
    {
        SaveManager_DisableSlot(i);
        if (!SaveManager_InitialiseSlots())
            frontend_set_state(FeSt_MAIN_MENU);
    } else
    {
        set_flag(SaveManager_Entry(i)->flags, CEF_Unloadable);
        create_frontend_error_box(get_string(SaveManager_CheckMessage(chk)));
    }
}

void frontend_draw_load_game_button(struct GuiButton *gbtn)
{
    int i = frontend_load_game_button_to_index(gbtn);
    if (i < 0)
        return;
    // Select font to draw
    int font_idx = frontend_button_caption_font(gbtn, frontend_mouse_over_button);
    const struct CatalogueEntry* centry = SaveManager_Entry(i);
    if ((centry->flags & CEF_Unloadable) != 0)
        font_idx = 3; // the grey font
    LbTextSetFont(frontend_font[font_idx]);
    RendererSetDrawFlags(Lb_TEXT_HALIGN_LEFT);
    // Set drawing window and draw the text
    int tx_units_per_px = (gbtn->height * 13 / 11) * 16 / LbTextLineHeight();
    int height = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, height);
    LbTextDrawResized(0, 0, tx_units_per_px, centry->textname);
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
    if (load_game_scroll_offset < SaveManager_SavedGamesCount()-frontend_load_menu_items_visible+1)
    {
        gbtn->flags |= LbBtnF_Enabled;
    }
    else
    {
        gbtn->flags &=  ~LbBtnF_Enabled;
    }
    if (wheel_scrolled_down || (is_key_pressed(KC_DOWN,KMod_NONE)))
    {
        if (load_game_scroll_offset < SaveManager_SavedGamesCount()-frontend_load_menu_items_visible+1)
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
    if (load_game_scroll_offset < SaveManager_SavedGamesCount()-frontend_load_menu_items_visible+1)
      load_game_scroll_offset++;
}

void frontend_load_game_scroll(struct GuiButton *gbtn)
{
    load_game_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), frontend_load_menu_items_visible-2, SaveManager_SavedGamesCount());
}

void frontend_draw_games_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, load_game_scroll_offset, frontend_load_menu_items_visible-2, SaveManager_SavedGamesCount());
}

void init_load_menu(struct GuiMenu *gmnu)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo* player = get_my_player();
  set_players_packet_action(player, PckA_UpdatePause, 1, 1, 0, 0);
  SaveManager_LoadCatalogue();
  gui_vscroll_offset = 0;   // load list starts at the top
  update_loadsave_input_strings();
}

void init_save_menu(struct GuiMenu *gmnu)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo* player = get_my_player();
  local_state.paused_state_restore = flag_is_set(game.operation_flags, GOF_Paused);
  set_players_packet_action(player, PckA_UpdatePause, 1, 1, 0, 0);
  SaveManager_LoadCatalogue();
  gui_vscroll_offset = 0;
  update_loadsave_input_strings();
}
/******************************************************************************/
