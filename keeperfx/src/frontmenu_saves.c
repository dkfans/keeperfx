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
#include "game_saves.h"
#include "gui_draw.h"
#include "gui_frontbtns.h"
#include "gui_soundmsgs.h"
#include "player_data.h"
#include "packets.h"
#include "frontend.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_gui_load_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_load_game_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_draw_load_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_load_game_button(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_load_game_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_games_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_save_game(struct GuiButton *gbtn);
DLLIMPORT void _DK_init_load_menu(struct GuiMenu *gmnu);
DLLIMPORT void _DK_init_save_menu(struct GuiMenu *gmnu);
/******************************************************************************/
struct GuiButtonInit load_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 719,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               0, 999,  58, 999,  58,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[0]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               1, 999,  90, 999,  90,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[1]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               2, 999, 122, 999, 122,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[2]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               3, 999, 154, 999, 154,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[3]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               4, 999, 186, 999, 186,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[4]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               5, 999, 218, 999, 218,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[5]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               6, 999, 250, 999, 250,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[6]}, 0, 0, gui_load_game_maintain },
  { 0,  0, 0, 0, 1, gui_load_game,      NULL,        NULL,               7, 999, 282, 999, 282,300, 32, draw_load_button,                  1, 201,  0,{(long)&input_string[7]}, 0, 0, gui_load_game_maintain },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit save_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 720,  0,       {0},            0, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               0, 999,  58, 999,  58,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[0]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               1, 999,  90, 999,  90,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[1]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               2, 999, 122, 999, 122,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[2]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               3, 999, 154, 999, 154,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[3]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               4, 999, 186, 999, 186,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[4]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               5, 999, 218, 999, 218,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[5]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               6, 999, 250, 999, 250,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[6]},15, 0, NULL },
  { 5, -2,-1,-1, 1, gui_save_game,      NULL,        NULL,               7, 999, 282, 999, 282,300, 32, gui_area_text,                     1, 201,  0,{(long)&input_string[7]},15, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

#define frontend_load_menu_items_visible  6
struct GuiButtonInit frontend_load_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {7},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 124,  82, 124,220, 26, frontend_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 150,  82, 150,450,180, frontend_draw_scroll_box,          0, 201,  0,      {26},            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontend_load_game_up,NULL,frontend_over_button,     0, 532, 149, 532, 149, 26, 14, frontend_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontend_load_game_up_maintain },
  { 1,  0, 0, 0, 0, frontend_load_game_down,NULL,frontend_over_button,   0, 532, 317, 532, 317, 26, 14, frontend_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontend_load_game_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 163, 536, 163, 10,154, frontend_draw_games_scroll_tab,    0, 201,  0,      {40},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 125, 102, 125,220, 26, frontend_draw_text,                0, 201,  0,      {30},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 157,  95, 157,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {45},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 185,  95, 185,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {46},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 213,  95, 213,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {47},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 241,  95, 241,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {48},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 269,  95, 269,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {49},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_load_game,NULL,frontend_over_button,        0,  95, 297,  95, 297,424, 26, frontend_draw_load_game_button,    0, 201,  0,      {50},            0, 0, frontend_load_game_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,frontend_over_button,     1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiMenu load_menu =
 { 11, 0, 4, load_menu_buttons,          POS_GAMECTR,POS_GAMECTR,436, 350, gui_pretty_background,       0, NULL,    init_load_menu,          0, 1, 0,};
struct GuiMenu save_menu =
 { 12, 0, 4, save_menu_buttons,          POS_GAMECTR,POS_GAMECTR,436, 350, gui_pretty_background,       0, NULL,    init_save_menu,          0, 1, 0,};
struct GuiMenu frontend_load_menu =
 { 19, 0, 1, frontend_load_menu_buttons,          0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
/******************************************************************************/
int frontend_load_game_button_to_index(struct GuiButton *gbtn)
{
  struct CatalogueEntry *centry;
  long gbidx;
  int i,k;
  gbidx = (unsigned long)gbtn->content;
  k = -1;
  for (i=gbidx+load_game_scroll_offset-45; i >= 0; i--)
  {
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
  struct CatalogueEntry *centry;
  if (gbtn != NULL)
      slot_num = gbtn->field_1B;
  else
      slot_num = 0;
  centry = &save_game_catalogue[slot_num];
  set_flag_byte(&gbtn->field_0, 0x08, ((centry->flags & CEF_InUse) != 0));
}

void gui_load_game(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player=get_my_player();
  if (!load_game(gbtn->field_1B))
  {
      ERRORLOG("Error in load!");
      quit_game = 1;
      return;
  }
  set_players_packet_action(player, 22, 0, 0, 0, 0);
}

void draw_load_button(struct GuiButton *gbtn)
{
  if (gbtn == NULL) return;
  gbtn->height = 32;
  if ((gbtn->field_1) || (gbtn->field_2))
  {
    draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width);
    draw_lit_bar64k(gbtn->scr_pos_x - 6, gbtn->scr_pos_y - 6, gbtn->width + 6);
  } else
  {
    draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width);
  }
  if (gbtn->content != NULL)
  {
    sprintf(gui_textbuf, "%s", (const char *)gbtn->content);
    draw_button_string(gbtn, gui_textbuf);
  }
}

void gui_save_game(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  long slot_num;
  player = get_my_player();
  if (strcasecmp((char *)gbtn->content, gui_strings[342]) != 0) // "UNUSED"
  {
      slot_num = gbtn->field_1B%TOTAL_SAVE_SLOTS_COUNT;
      fill_game_catalogue_slot(slot_num,(char *)gbtn->content);
      if (save_game(slot_num))
      {
        output_message(103, 0, 1);
      } else
      {
        ERRORLOG("Error in save!");
        create_error_box(536);
      }
  }
  set_players_packet_action(player, PckA_TogglePause, 0, 0, 0, 0);
}

void update_loadsave_input_strings(struct CatalogueEntry *game_catalg)
{
    struct CatalogueEntry *centry;
    long slot_num;
    char *text;
    SYNCDBG(6,"Starting");
    for (slot_num=0; slot_num < TOTAL_SAVE_SLOTS_COUNT; slot_num++)
    {
        centry = &game_catalg[slot_num];
        if ((centry->flags & CEF_InUse) != 0)
          text = centry->textname;
        else
          text = gui_strings[342]; // UNUSED
        strncpy(input_string[slot_num], text, SAVE_TEXTNAME_LEN);
    }
}

void frontend_load_game(struct GuiButton *gbtn)
{
  int i;
  i = frontend_load_game_button_to_index(gbtn);
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
  int nfont;
  long gbidx;
  int i,h;
  gbidx = (unsigned long)gbtn->content;
  nfont = frontend_button_info[gbidx%FRONTEND_BUTTON_INFO_COUNT].font_index;
  if ((gbidx != 0) && (frontend_mouse_over_button == gbidx))
      nfont = 2;
  lbDisplay.DrawFlags = 0x20;
  LbTextSetFont(frontend_font[nfont]);
  h = LbTextLineHeight();
  LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, h);
  i = frontend_load_game_button_to_index(gbtn);
  if (i < 0)
    return;
  LbTextDraw(0, 0, save_game_catalogue[i].textname);
}

void frontend_load_game_up_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (load_game_scroll_offset != 0));
}

void frontend_load_game_down_maintain(struct GuiButton *gbtn)
{
  set_flag_byte(&gbtn->field_0, 0x08, (load_game_scroll_offset < number_of_saved_games-frontend_load_menu_items_visible+1));
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

void frontend_draw_games_scroll_tab(struct GuiButton *gbtn)
{
  frontend_draw_scroll_tab(gbtn, load_game_scroll_offset, frontend_load_menu_items_visible-2, number_of_saved_games);
}

void init_load_menu(struct GuiMenu *gmnu)
{
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  player = get_my_player();
  set_players_packet_action(player, 82, 1, 1, 0, 0);
  load_game_save_catalogue();
  update_loadsave_input_strings(save_game_catalogue);
}

void init_save_menu(struct GuiMenu *gmnu)
{
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  player = get_my_player();
  set_players_packet_action(player, 82, 1, 1, 0, 0);
  load_game_save_catalogue();
  update_loadsave_input_strings(save_game_catalogue);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
