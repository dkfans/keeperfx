/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_options.c
 *     GUI menus for game options.
 * @par Purpose:
 *     Functions to show and maintain options screens.
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
#include "frontmenu_options.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "player_data.h"
#include "frontend.h"
#include "packets.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_frontend_define_key_up(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_down(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_up_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_down_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_define_key_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_define_key_scroll_tab(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_define_key(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_shadows(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_view_distance_level(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_rotate_mode(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_cluedo_mode(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_gamma_correction(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_video_cluedo_maintain(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_sound_volume(struct GuiButton *gbtn);
DLLIMPORT void _DK_gui_set_music_volume(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_set_mouse_sensitivity(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_invert_mouse(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontend_draw_invert_mouse(struct GuiButton *gbtn);
DLLIMPORT void _DK_init_video_menu(struct GuiMenu *gmnu);
DLLIMPORT void _DK_init_audio_menu(struct GuiMenu *gmnu);
/******************************************************************************/
struct GuiButtonInit frontend_define_keys_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {92},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 117,  82, 117,450,246, frontend_draw_scroll_box,          0, 201,  0,      {94},            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontend_define_key_up,NULL, frontend_over_button,   0, 532, 116, 532, 116, 26, 14, frontend_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontend_define_key_up_maintain },
  { 1,  0, 0, 0, 0, frontend_define_key_down,NULL,frontend_over_button,  0, 532, 350, 532, 350, 26, 14, frontend_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontend_define_key_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 130, 536, 130, 10,220, frontend_draw_define_key_scroll_tab,0,201,  0,      {40},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 130,  95, 130,424, 22, frontend_draw_define_key,          0, 201,  0,      {-1},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 152,  95, 152,424, 22, frontend_draw_define_key,          0, 201,  0,      {-2},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 174,  95, 174,424, 22, frontend_draw_define_key,          0, 201,  0,      {-3},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 196,  95, 196,424, 22, frontend_draw_define_key,          0, 201,  0,      {-4},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 218,  95, 218,424, 22, frontend_draw_define_key,          0, 201,  0,      {-5},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 240,  95, 240,424, 22, frontend_draw_define_key,          0, 201,  0,      {-6},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 262,  95, 262,424, 22, frontend_draw_define_key,          0, 201,  0,      {-7},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 284,  95, 284,424, 22, frontend_draw_define_key,          0, 201,  0,      {-8},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 306,  95, 306,424, 22, frontend_draw_define_key,          0, 201,  0,      {-9},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 328,  95, 328,424, 22, frontend_draw_define_key,          0, 201,  0,     {-10},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,     frontend_over_button,27,999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {98},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit frontend_option_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {96},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 107,  95, 107,220, 26, frontend_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 133,  95, 133,  2, 88, frontend_draw_scroll_box,          0, 201,  0,      {89},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 115, 108, 115, 108,220, 26, frontend_draw_text,                0, 201,  0,      {99},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 146, 142, 146, 142, 48, 32, frontend_draw_icon,               90, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 146, 182, 146, 182, 48, 32, frontend_draw_icon,               89, 201,  0,       {0},            0, 0, NULL },
  { 4, 75, 0, 0, 0, gui_set_sound_volume,NULL,       NULL,               0, 194, 147, 194, 147,300, 22, frontend_draw_slider,              0, 201,  0,{(long)&sound_level}, 127, 0, NULL },
  { 4,  0, 0, 0, 0, gui_set_music_volume,NULL,       NULL,               0, 194, 187, 194, 187,300, 22, frontend_draw_slider,              0, 201,  0,{(long)&music_level}, 127, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 231,  95, 231,220, 26, frontend_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 257,  95, 257,  0, 88, frontend_draw_scroll_box,          0, 201,  0,      {89},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 115, 232, 115, 232,220, 26, frontend_draw_text,                0, 201,  0,     {100},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 271, 102, 271,190, 22, frontend_draw_text,                0, 201,  0,     {101},            0, 0, NULL },
  { 4,  0, 0, 0, 0, frontend_set_mouse_sensitivity,NULL,NULL,            0, 304, 271, 304, 271,190, 22, frontend_draw_small_slider,        0, 201,  0,{(long)&fe_mouse_sensitivity}, 7, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_invert_mouse,NULL,     frontend_over_button,0, 102, 303, 102, 303,380, 22, frontend_draw_text,                0, 201,  0,     {102},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 320, 303,   0,   0,100, 22, frontend_draw_invert_mouse,        0, 201,  0,     {102},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,    frontend_over_button,26, 999, 357, 999, 357,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {95},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,    frontend_over_button, 1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit video_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 717,  0,       {0},            0, 0, NULL },
  { 2,  0, 0, 0, 0, gui_video_shadows,  NULL,        NULL,               0,   8,  38,  10,  38, 46, 64, gui_area_no_anim_button,          27, 313,  0,{(long)&video_shadows}, 4, 0, NULL },
  { 2,  0, 0, 0, 0, gui_video_view_distance_level,NULL,NULL,             0,  56,  38,  58,  38, 46, 64, gui_area_no_anim_button,          36, 316,  0,{(long)&video_view_distance_level}, 3, 0, NULL },
  { 2,  0, 0, 0, 0, gui_video_rotate_mode,NULL,      NULL,               0, 104,  38, 106,  38, 46, 64, gui_area_no_anim_button,          32, 314,  0,{(long)&settings.field_3}, 1, 0, NULL },
  { 2,  0, 0, 0, 0, gui_video_cluedo_mode,NULL,      NULL,               0,  32,  90,  32,  90, 46, 64, gui_area_no_anim_button,          42, 315,  0,{(long)&_DK_video_cluedo_mode},1, 0, gui_video_cluedo_maintain },
  { 0,  0, 0, 0, 0, gui_video_gamma_correction,NULL, NULL,               0,  80,  90,  80,  90, 46, 64, gui_area_no_anim_button,          44, 317,  0,{(long)&video_gamma_correction}, 0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit sound_menu_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, 718,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8,  28,  10,  28, 46, 64, gui_area_no_anim_button,          41, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   8,  80,  10,  80, 46, 64, gui_area_no_anim_button,          40, 201,  0,       {0},            0, 0, NULL },
  { 4,  0, 0, 0, 0, gui_set_sound_volume,NULL,       NULL,               0,  66,  58,  66,  58,190, 32, gui_area_slider,                   0, 340,  0,{(long)&sound_level}, 127, 0, NULL },
  { 4,  0, 0, 0, 0, gui_set_music_volume,NULL,       NULL,               0,  66, 110,  66, 110,190, 32, gui_area_slider,                   0, 341,  0,{(long)&music_level}, 127, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiMenu frontend_define_keys_menu =
 { 36, 0, 1, frontend_define_keys_buttons,        0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_option_menu =
 { 39, 0, 1, frontend_option_buttons,             0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    frontend_init_options_menu,0,0,0,};
struct GuiMenu video_menu =
 { 13, 0, 4, video_menu_buttons,         POS_GAMECTR,POS_GAMECTR,160, 170, gui_pretty_background,       0, NULL,    init_video_menu,         0, 1, 0,};
struct GuiMenu sound_menu =
 { 14, 0, 4, sound_menu_buttons,         POS_GAMECTR,POS_GAMECTR,280, 170, gui_pretty_background,       0, NULL,    init_audio_menu,         0, 1, 0,};
/******************************************************************************/
void frontend_define_key_up_maintain(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_up_maintain(gbtn);
}

void frontend_define_key_down_maintain(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_down_maintain(gbtn);
}

void frontend_define_key_maintain(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_maintain(gbtn);
}

void frontend_define_key_up(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_up(gbtn);
}

void frontend_define_key_down(struct GuiButton *gbtn)
{
  _DK_frontend_define_key_down(gbtn);
}

void frontend_define_key(struct GuiButton *gbtn)
{
  _DK_frontend_define_key(gbtn);
/*
  long key_id;
  key_id = define_key_scroll_offset - ((long)gbtn->field_33) - 1;
  defining_a_key = 1;
  defining_a_key_id = key_id;
  lbInkey = 0;
*/
}

void frontend_draw_define_key_scroll_tab(struct GuiButton *gbtn)
{
  _DK_frontend_draw_define_key_scroll_tab(gbtn);
}

void frontend_draw_define_key(struct GuiButton *gbtn)
{
  _DK_frontend_draw_define_key(gbtn);
}

void gui_video_shadows(struct GuiButton *gbtn)
{
  _DK_gui_video_shadows(gbtn);
}

void gui_video_view_distance_level(struct GuiButton *gbtn)
{
  _DK_gui_video_view_distance_level(gbtn);
}

void gui_video_rotate_mode(struct GuiButton *gbtn)
{
  _DK_gui_video_rotate_mode(gbtn);
}

void gui_video_cluedo_mode(struct GuiButton *gbtn)
{
  _DK_gui_video_cluedo_mode(gbtn);
}

void gui_video_gamma_correction(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  video_gamma_correction = (video_gamma_correction + 1) % GAMMA_LEVELS_COUNT;
  set_players_packet_action(player, PckA_SetGammaLevel, video_gamma_correction, 0, 0, 0);
}

void gui_set_sound_volume(struct GuiButton *gbtn)
{
  _DK_gui_set_sound_volume(gbtn);
}

void gui_set_music_volume(struct GuiButton *gbtn)
{
  _DK_gui_set_music_volume(gbtn);
}

void gui_video_cluedo_maintain(struct GuiButton *gbtn)
{
  _DK_gui_video_cluedo_maintain(gbtn);
}

void frontend_set_mouse_sensitivity(struct GuiButton *gbtn)
{
  _DK_frontend_set_mouse_sensitivity(gbtn);
}

void frontend_invert_mouse(struct GuiButton *gbtn)
{
  _DK_frontend_invert_mouse(gbtn);
}

void frontend_draw_invert_mouse(struct GuiButton *gbtn)
{
  _DK_frontend_draw_invert_mouse(gbtn);
}

void init_video_menu(struct GuiMenu *gmnu)
{
  _DK_init_video_menu(gmnu);
}

void init_audio_menu(struct GuiMenu *gmnu)
{
  _DK_init_audio_menu(gmnu);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
