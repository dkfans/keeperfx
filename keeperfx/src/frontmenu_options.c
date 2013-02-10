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
#include "bflib_sndlib.h"
#include "player_data.h"
#include "gui_draw.h"
#include "gui_frontbtns.h"
#include "frontend.h"
#include "packets.h"
#include "config_settings.h"
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
#ifdef __cplusplus
}
#endif
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
    //_DK_gui_set_sound_volume(gbtn);
    if (gbtn->id_num == 75)
    {
      if (settings.sound_volume != sound_level)
          do_sound_menu_click();
    }
    settings.sound_volume = sound_level;
    save_settings();
    SetSoundMasterVolume(settings.sound_volume);
    SetMusicMasterVolume(settings.sound_volume);
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
    //_DK_frontend_set_mouse_sensitivity(gbtn);
    settings.first_person_move_sensitivity = fe_mouse_sensitivity;
    save_settings();
}

void frontend_invert_mouse(struct GuiButton *gbtn)
{
    //_DK_frontend_invert_mouse(gbtn);
    settings.first_person_move_invert = !settings.first_person_move_invert;
    save_settings();
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
