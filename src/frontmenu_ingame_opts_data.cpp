/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_opts_data.cpp
 *     In-game options GUI, available under "escape" while in game.
 * @par Purpose:
 *     Structures to show and maintain option menus ingame.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 20 Apr 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "frontmenu_ingame_opts.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"

#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "frontend.h"
#include "frontmenu_saves.h"
#include "config_settings.h"
#include "frontmenu_options.h"
#include "game_legacy.h"
#include "sprites.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif


void maintain_compsetting_button(struct GuiButton* gbtn);

/******************************************************************************/
struct MsgBoxInfo MsgBox;

// Non-NULL no-op callback so that the controller snapping logic does not ignore the button
static void no_op(struct GuiButton* gbtn) {}

struct GuiButtonInit options_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_MnuOptions,          0,       {0},          0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, no_op,               NULL,        NULL,              0,  12,  36,  12,  36, 46, 64, gui_area_no_anim_button, GBS_options_button_load, GUIStr_LoadGameDesc,     &load_menu, {0},          0, maintain_loadsave },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, no_op,               NULL,        NULL,              0,  60,  36,  60,  36, 46, 64, gui_area_no_anim_button, GBS_options_button_save, GUIStr_SaveGameDesc,     &save_menu, {0},          0, maintain_loadsave },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, no_op,               NULL,        NULL,              0, 108,  36, 108,  36, 46, 64, gui_area_no_anim_button, GBS_options_button_graphc, GUIStr_GraphicsMenuDesc, &video_menu,{0},          0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, no_op,               NULL,        NULL,              0, 156,  36, 156,  36, 46, 64, gui_area_no_anim_button, GBS_options_button_sound, GUIStr_SoundMenuDesc,    &sound_menu,{0},          0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, no_op,               NULL,        NULL,              0, 204,  36, 204,  36, 46, 64, gui_area_compsetting_button, GPS_options_cassist_btn_black_a, GUIStr_ComputerAssistDesc,&autopilot_menu,{0},     0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, no_op,               NULL,        NULL,              0, 252,  36, 252,  36, 46, 64, gui_area_no_anim_button, GBS_options_button_exit, GUIStr_QuitGameDesc,     &quit_menu, {0},          0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0, 0,                       0,          {0},          0, NULL },
};

struct GuiButtonInit quit_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,210, 32, gui_area_text,                     1, GUIStr_ConfirmYouSure,   0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, no_op,               NULL,        NULL,              0,  70,  24,  72,  58, 46, 32, gui_area_normal_button, GBS_options_button_smd_no, GUIStr_ConfirmNo,        0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_quit_game,      NULL,        NULL,               0, 136,  24, 138,  58, 46, 32, gui_area_normal_button, GBS_options_button_smd_yes, GUIStr_ConfirmYes,       0,       {0},            0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit error_box_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_Error,            0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  65, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {.str = gui_error_text}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 999, 100, 999, 132, 46, 34, gui_area_normal_button, GBS_options_button_smd_yes, GUIStr_CloseWindow,      0,       {0},            0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit instance_menu_buttons[] = {
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit pause_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999, 999, 999, 999,140, 32, gui_area_text,                     0, GUIStr_PausedMsg,        0,       {0},            0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit autopilot_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_MnuComputer,      0,       {0},            0, NULL },
  {LbBtnT_RadioBtn,   BID_DEFAULT, 0, 0, gui_set_autopilot,  NULL,        NULL,               0,  12,  36,  12,  36, 46, 64, gui_area_new_normal_button, GPS_options_cassist_btn_orange, GUIStr_AggressiveAssistDesc,  0,{.ptr = &game.comp_player_aggressive},  0, maintain_compsetting_button },
  {LbBtnT_RadioBtn,   BID_DEFAULT, 0, 0, gui_set_autopilot,  NULL,        NULL,               1,  60,  36,  60,  36, 46, 64, gui_area_new_normal_button, GPS_options_cassist_btn_yellow, GUIStr_DefensiveAssistDesc,   0,{.ptr = &game.comp_player_defensive},   0, maintain_compsetting_button },
  {LbBtnT_RadioBtn,   BID_DEFAULT, 0, 0, gui_set_autopilot,  NULL,        NULL,               2, 108,  36, 108,  36, 46, 64, gui_area_new_normal_button, GPS_options_cassist_btn_pink,   GUIStr_ConstructionAssistDesc,0,{.ptr = &game.comp_player_construct},   0, maintain_compsetting_button },
  {LbBtnT_RadioBtn,   BID_DEFAULT, 0, 0, gui_set_autopilot,  NULL,        NULL,               3, 156,  36, 156,  36, 46, 64, gui_area_new_normal_button, GPS_options_cassist_btn_green,  GUIStr_MoveOnlyAssistDesc,    0,{.ptr = &game.comp_player_creatrsonly}, 0, maintain_compsetting_button },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit video_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,                          NULL,                           NULL,  0, 999,  10, 999,  10, 155, 32, gui_area_text,                     1, GUIStr_MnuGraphicsOptions,          0, {0},            0, NULL },
  {LbBtnT_ToggleBtn,  BID_DEFAULT, 0, 0, gui_video_shadows,             NULL,                           NULL,  0,  28,  38,  30,  38,  46, 64, gui_area_no_anim_button, GBS_options_button_grph_shadow0, GUIStr_OptionShadowsDesc,           0, {.ptr = &video_shadows}, 4, NULL },
  {LbBtnT_ToggleBtn,  BID_DEFAULT, 0, 0, gui_video_view_distance_level, NULL,                           NULL,  0,  76,  38,  78,  38,  46, 64, gui_area_no_anim_button, GBS_options_button_grph_range0, GUIStr_OptionViewDistanceDesc,      0, {.ptr = &video_view_distance_level}, 3, NULL },
  {LbBtnT_ToggleBtn,  BID_DEFAULT, 0, 0, gui_video_rotate_mode,         NULL,                           NULL,  0, 124,  38, 126,  38,  46, 64, gui_area_no_anim_button, GBS_options_button_grph_pers_rot, GUIStr_OptionViewTypeDesc,          0, {.ptr = &settings.video_rotate_mode}, 2, NULL },
  {LbBtnT_ToggleBtn,  BID_DEFAULT, 0, 0, gui_video_cluedo_mode,         NULL,                           NULL,  0,  28, 100,  30, 100,  46, 64, gui_area_no_anim_button, GBS_options_button_grph_wall_hi, GUIStr_OptionWallHeightDesc,        0, {.ptr = &video_cluedo_mode},1, gui_video_cluedo_maintain },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_video_gamma_correction,    NULL,                           NULL,  0,  76, 100,  78, 100,  46, 64, gui_area_no_anim_button, GBS_options_button_grph_gamma, GUIStr_OptionGammaCorrectionDesc,   0, {.ptr = &video_gamma_correction}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_switch_video_mode,         gui_display_current_resolution, NULL,  0, 124, 100, 126, 100,  46, 64, gui_area_no_anim_button, GBS_optionsbutton_resolution, GUIStr_DisplayResolution,           0, {0}, 0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,                          NULL,                           NULL,  0,   0,   0,   0,   0,   0,  0, NULL,                              0,                                     0, 0, {0},            0, NULL },
};

struct GuiButtonInit sound_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_MnuSoundOptions,  0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8,  28,  10,  28, 46, 64, gui_area_no_anim_button, GBS_options_button_snd_music, GUIStr_Empty,            0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8,  80,  10,  80, 46, 64, gui_area_no_anim_button, GBS_options_button_snd_sounds, GUIStr_Empty,            0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8, 132,  10, 132, 46, 64, gui_area_no_anim_button, GBS_optionsbutton_snd_voice, GUIStr_Empty,            0,       {0},            0, NULL },
  {LbBtnT_HorizSlider,BID_SOUND_VOL, 0, 0, gui_set_sound_volume,NULL,       NULL,               0,  66,  58,  66,  58,190, 30, gui_area_slider,                   0, GUIStr_OptionSoundFx,    0, {0}, 255, NULL },
  {LbBtnT_HorizSlider,BID_MUSIC_VOL, 0, 0, gui_set_music_volume,NULL,       NULL,               0,  66, 110,  66, 110,190, 30, gui_area_slider,                   0, GUIStr_OptionMusic,      0, {0}, 255, NULL },
  {LbBtnT_HorizSlider,BID_MENTOR_VOL, 0, 0, gui_set_mentor_volume,NULL,      NULL,               0,  66, 162,  66, 162,190, 30, gui_area_slider,                   0, GUIStr_OptionVoice,      0, {0}, 255, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit message_box_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_Empty,            0,       {.str = MsgBox.title},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  35, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {.str = MsgBox.line1}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  55, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {.str = MsgBox.line2}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  75, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {.str = MsgBox.line3}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  95, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {.str = MsgBox.line4}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999, 115, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {.str = MsgBox.line5}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 999, 115, 999, 132, 46, 34, gui_area_normal_button, GBS_options_button_smd_yes, GUIStr_CloseWindow,      0,       {0},            0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiMenu options_menu =
 { GMnu_OPTIONS,      0, 1, options_menu_buttons,       POS_GAMECTR,POS_GAMECTR,308, 120, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu instance_menu =
 { GMnu_INSTANCE,     0, 1, instance_menu_buttons,      POS_GAMECTR,POS_GAMECTR,318, 120, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu quit_menu =
 { GMnu_QUIT,         0, 1, quit_menu_buttons,          POS_GAMECTR,POS_GAMECTR,264, 116, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu error_box =
 { GMnu_ERROR_BOX,    0, 1, error_box_buttons,          POS_GAMECTR,POS_GAMECTR,280, 180, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
struct GuiMenu autopilot_menu =
 { GMnu_AUTOPILOT,    0, 4, autopilot_menu_buttons,     POS_GAMECTR,POS_GAMECTR,224, 120, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};

struct GuiMenu video_menu =
 { GMnu_VIDEO, 0, 4, video_menu_buttons,         POS_GAMECTR,POS_GAMECTR,200, 180, gui_pretty_background,       0, NULL,    init_video_menu,         0, 1, 0,};
struct GuiMenu sound_menu =
 { GMnu_SOUND, 0, 4, sound_menu_buttons,         POS_GAMECTR,POS_GAMECTR,280, 225, gui_pretty_background,       0, NULL,    init_audio_menu,         0, 1, 0,};

struct GuiMenu message_box =
{ GMnu_MSG_BOX,    0, 1, message_box_buttons,          POS_GAMECTR,POS_GAMECTR,280, 180, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
