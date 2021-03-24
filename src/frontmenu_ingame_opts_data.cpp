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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct MsgBoxInfo MsgBox;

struct GuiButtonInit options_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_MnuOptions,          0,       {0},          0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  12,  36,  12,  36, 46, 64, gui_area_no_anim_button,          23, GUIStr_LoadGameDesc,     &load_menu, {0},          0, maintain_loadsave },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  60,  36,  60,  36, 46, 64, gui_area_no_anim_button,          22, GUIStr_SaveGameDesc,     &save_menu, {0},          0, maintain_loadsave },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 108,  36, 108,  36, 46, 64, gui_area_no_anim_button,          25, GUIStr_GraphicsMenuDesc, &video_menu,{0},          0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 156,  36, 156,  36, 46, 64, gui_area_no_anim_button,          24, GUIStr_SoundMenuDesc,    &sound_menu,{0},          0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 204,  36, 204,  36, 46, 64, gui_area_compsetting_button,     501, GUIStr_ComputerAssistDesc,&autopilot_menu,{0},     0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 252,  36, 252,  36, 46, 64, gui_area_no_anim_button,          26, GUIStr_QuitGameDesc,     &quit_menu, {0},          0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0, 0,                       0,          {0},          0, NULL },
};

struct GuiButtonInit quit_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,210, 32, gui_area_text,                     1, GUIStr_ConfirmYouSure,   0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0,  70,  24,  72,  58, 46, 32, gui_area_normal_button,           46, GUIStr_ConfirmNo,        0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_quit_game,      NULL,        NULL,               0, 136,  24, 138,  58, 46, 32, gui_area_normal_button,           48, GUIStr_ConfirmYes,       0,       {0},            0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit error_box_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_Error,            0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  65, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {(long)&gui_error_text}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 999, 100, 999, 132, 46, 34, gui_area_normal_button,           48, GUIStr_CloseWindow,      0,       {0},            0, NULL },
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
  {LbBtnT_RadioBtn,   BID_DEFAULT, 0, 0, gui_set_autopilot,  NULL,        NULL,               0,  12,  36,  12,  36, 46, 64, gui_area_new_normal_button,      503, GUIStr_AggressiveAssistDesc,0,{(long)&game.comp_player_aggressive}, 0, NULL },
  {LbBtnT_RadioBtn,   BID_DEFAULT, 0, 0, gui_set_autopilot,  NULL,        NULL,               0,  60,  36,  60,  36, 46, 64, gui_area_new_normal_button,      505, GUIStr_DefensiveAssistDesc,0,{(long)&game.comp_player_defensive}, 0, NULL },
  {LbBtnT_RadioBtn,   BID_DEFAULT, 0, 0, gui_set_autopilot,  NULL,        NULL,               0, 108,  36, 108,  36, 46, 64, gui_area_new_normal_button,      507, GUIStr_ConstructionAssistDesc,0,{(long)&game.comp_player_construct}, 0, NULL },
  {LbBtnT_RadioBtn,   BID_DEFAULT, 0, 0, gui_set_autopilot,  NULL,        NULL,               0, 156,  36, 156,  36, 46, 64, gui_area_new_normal_button,      509, GUIStr_MoveOnlyAssistDesc,0,{(long)&game.comp_player_creatrsonly}, 0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit video_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_MnuGraphicsOptions,0,      {0},            0, NULL },
  {LbBtnT_ToggleBtn,  BID_DEFAULT, 0, 0, gui_video_shadows,  NULL,        NULL,               0,   8,  38,  10,  38, 46, 64, gui_area_no_anim_button,          27, GUIStr_OptionShadowsDesc, 0,{(long)&_DK_video_shadows}, 4, NULL },
  {LbBtnT_ToggleBtn,  BID_DEFAULT, 0, 0, gui_video_view_distance_level,NULL,NULL,             0,  56,  38,  58,  38, 46, 64, gui_area_no_anim_button,          36, GUIStr_OptionViewDistanceDesc,0,{(long)&video_view_distance_level}, 3, NULL },
  {LbBtnT_ToggleBtn,  BID_DEFAULT, 0, 0, gui_video_rotate_mode,NULL,      NULL,               0, 104,  38, 106,  38, 46, 64, gui_area_no_anim_button,          32, GUIStr_OptionViewTypeDesc,0,{(long)&settings.video_rotate_mode}, 1, NULL },
  {LbBtnT_ToggleBtn,  BID_DEFAULT, 0, 0, gui_video_cluedo_mode,NULL,      NULL,               0,  32,  90,  32,  90, 46, 64, gui_area_no_anim_button,          42, GUIStr_OptionWallHeightDesc,0,{(long)&_DK_video_cluedo_mode},1, gui_video_cluedo_maintain },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_video_gamma_correction,NULL, NULL,               0,  80,  90,  80,  90, 46, 64, gui_area_no_anim_button,          44, GUIStr_OptionGammaCorrectionDesc,0,{(long)&video_gamma_correction}, 0, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit sound_menu_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_MnuSoundOptions,  0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8,  28,  10,  28, 46, 64, gui_area_no_anim_button,          41, GUIStr_Empty,            0,       {0},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   8,  80,  10,  80, 46, 64, gui_area_no_anim_button,          40, GUIStr_Empty,            0,       {0},            0, NULL },
  {LbBtnT_HorizSlider,BID_DEFAULT, 0, 0, gui_set_sound_volume,NULL,       NULL,               0,  66,  58,  66,  58,190, 30, gui_area_slider,                   0, GUIStr_OptionSoundFx,    0,{(long)&sound_level},127, NULL },
  {LbBtnT_HorizSlider,BID_DEFAULT, 0, 0, gui_set_music_volume,NULL,       NULL,               0,  66, 110,  66, 110,190, 30, gui_area_slider,                   0, GUIStr_OptionMusic,      0,{(long)&music_level},127, NULL },
  {              -1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                     0,       {0},            0, NULL },
};

struct GuiButtonInit message_box_buttons[] = {
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  10, 999,  10,155, 32, gui_area_text,                     1, GUIStr_Empty,            0,       {(long)&MsgBox.title},            0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  35, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {(long)&MsgBox.line1}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  55, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {(long)&MsgBox.line2}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  75, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {(long)&MsgBox.line3}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999,  95, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {(long)&MsgBox.line4}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 999, 115, 999,  0, 250, 32, gui_area_text,                     0, GUIStr_Empty,            0,       {(long)&MsgBox.line5}, 0, NULL },
  {LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, NULL,               NULL,        NULL,               0, 999, 115, 999, 132, 46, 34, gui_area_normal_button,           48, GUIStr_CloseWindow,      0,       {0},            0, NULL },
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
 { GMnu_VIDEO, 0, 4, video_menu_buttons,         POS_GAMECTR,POS_GAMECTR,160, 170, gui_pretty_background,       0, NULL,    init_video_menu,         0, 1, 0,};
struct GuiMenu sound_menu =
 { GMnu_SOUND, 0, 4, sound_menu_buttons,         POS_GAMECTR,POS_GAMECTR,280, 170, gui_pretty_background,       0, NULL,    init_audio_menu,         0, 1, 0,};
 
struct GuiMenu message_box =
{ GMnu_MSG_BOX,    0, 1, message_box_buttons,          POS_GAMECTR,POS_GAMECTR,280, 180, gui_pretty_background,       0, NULL,    NULL,                    0, 1, 0,};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
