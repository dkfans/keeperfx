/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_options.h
 *     Header file for frontmenu_options.c.
 * @par Purpose:
 *     GUI menus for game options.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTMENU_OPTS_H
#define DK_FRONTMENU_OPTS_H

#include "globals.h"
#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define GAMMA_LEVELS_COUNT      5

enum OptionsButtonDesignationIDs {
    BID_SOUND_VOL = BID_DEFAULT+75,
};

struct GuiMenu;
struct GuiButton;

/******************************************************************************/
DLLIMPORT long _DK_fe_mouse_sensitivity;
#define fe_mouse_sensitivity _DK_fe_mouse_sensitivity
DLLIMPORT long _DK_sound_level;
#define sound_level _DK_sound_level
DLLIMPORT long _DK_music_level;
#define music_level _DK_music_level
DLLIMPORT char _DK_video_cluedo_mode;
DLLIMPORT char _DK_video_shadows;
DLLIMPORT char _DK_video_textures;
DLLIMPORT char _DK_video_view_distance_level;
#define video_view_distance_level _DK_video_view_distance_level

#pragma pack()
/******************************************************************************/
extern struct GuiMenu frontend_define_keys_menu;
#define frontend_define_keys_menu_items_visible  10
extern struct GuiMenu frontend_option_menu;
/******************************************************************************/
void frontend_define_key_up(struct GuiButton *gbtn);
void frontend_define_key_down(struct GuiButton *gbtn);
void frontend_define_key_scroll(struct GuiButton *gbtn);
void frontend_define_key(struct GuiButton *gbtn);
void frontend_define_key_up_maintain(struct GuiButton *gbtn);
void frontend_define_key_down_maintain(struct GuiButton *gbtn);
void frontend_define_key_maintain(struct GuiButton *gbtn);
void frontend_draw_define_key_scroll_tab(struct GuiButton *gbtn);
void frontend_draw_define_key(struct GuiButton *gbtn);
void frontend_set_mouse_sensitivity(struct GuiButton *gbtn);
void frontend_invert_mouse(struct GuiButton *gbtn);
void frontend_draw_invert_mouse(struct GuiButton *gbtn);
void gui_video_shadows(struct GuiButton *gbtn);
void gui_video_view_distance_level(struct GuiButton *gbtn);
void gui_video_rotate_mode(struct GuiButton *gbtn);
void gui_video_cluedo_mode(struct GuiButton *gbtn);
void gui_video_gamma_correction(struct GuiButton *gbtn);
void gui_video_cluedo_maintain(struct GuiButton *gbtn);
void gui_set_sound_volume(struct GuiButton *gbtn);
void gui_set_music_volume(struct GuiButton *gbtn);
void init_video_menu(struct GuiMenu *gmnu);
void init_audio_menu(struct GuiMenu *gmnu);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
