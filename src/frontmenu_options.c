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
#include "bflib_vidraw.h"
#include "bflib_sndlib.h"
#include "bflib_guibtns.h"
#include "player_data.h"
#include "gui_draw.h"
#include "config_strings.h"
#include "gui_frontbtns.h"
#include "music_player.h"
#include "frontend.h"
#include "front_input.h"
#include "kjm_input.h"
#include "packets.h"
#include "config_settings.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const long definable_key_string[] = {
    GUIStr_CtrlUp,
    GUIStr_CtrlDown,
    GUIStr_CtrlLeft,
    GUIStr_CtrlRight,
    GUIStr_CtrlRotate,
    GUIStr_CtrlSpeed,
    GUIStr_CtrlRotateLeft,
    GUIStr_CtrlRotateRight,
    GUIStr_CtrlZoomIn,
    GUIStr_CtrlZoomOut,
    CpgStr_RoomKind1+0,//TODO not GUI strings
    CpgStr_RoomKind1+1,
    CpgStr_RoomKind1+2,
    CpgStr_RoomKind1+3,
    CpgStr_RoomKind1+4,
    CpgStr_RoomKind1+5,
    CpgStr_RoomKind1+6,
    CpgStr_RoomKind1+7,
    CpgStr_RoomKind1+8,
    CpgStr_RoomKind1+9,
    CpgStr_RoomKind1+10,
    CpgStr_RoomKind1+11,
    CpgStr_RoomKind1+12,
    CpgStr_RoomKind1+13,
    CpgStr_RoomKind1+14,
    GUIStr_StateFight,
    GUIStr_StateAnnoyed,
    CpgStr_PowerKind1,//TODO not GUI string
    GUIStr_Query,
    GUIStr_UndoPickup,
    GUIStr_Pause,
    GUIStr_Map,
    GUIStr_ToggleMessage,
    GUIStr_SnapCamera,
    GUIStr_BestRoomSpace,
    GUIStr_SquareRoomSpace,
    GUIStr_RoomSpaceIncrease,
    GUIStr_RoomSpaceDecrease,
    GUIStr_SellTrapOnSubtile,
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void frontend_define_key_up_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (define_key_scroll_offset != 0)) & LbBtnF_Enabled;
		if ((wheel_scrolled_up || is_key_pressed(KC_UP,KMod_NONE)) & (define_key_scroll_offset != 0))
	{
		define_key_scroll_offset--;
	}
}

void frontend_define_key_down_maintain(struct GuiButton *gbtn)
{
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (define_key_scroll_offset < GAME_KEYS_COUNT-1)) & LbBtnF_Enabled;
	if ((wheel_scrolled_down || is_key_pressed(KC_DOWN,KMod_NONE)) & (define_key_scroll_offset < GAME_KEYS_COUNT-(frontend_define_keys_menu_items_visible-1)))
	{
		define_key_scroll_offset++;
	}
}

void frontend_define_key_maintain(struct GuiButton *gbtn)
{
    long key_id = define_key_scroll_offset - ((long)gbtn->content) - 1;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (key_id < GAME_KEYS_COUNT)) & LbBtnF_Enabled;
}

void frontend_define_key_up(struct GuiButton *gbtn)
{
    if (define_key_scroll_offset > 0) {
        define_key_scroll_offset--;
    }
}

void frontend_define_key_down(struct GuiButton *gbtn)
{
    if (define_key_scroll_offset < GAME_KEYS_COUNT-(frontend_define_keys_menu_items_visible-1)) {
        define_key_scroll_offset++;
    }
}

void frontend_define_key_scroll(struct GuiButton *gbtn)
{
    define_key_scroll_offset = frontend_scroll_tab_to_offset(gbtn, GetMouseY(), frontend_define_keys_menu_items_visible-2, GAME_KEYS_COUNT);
}

void frontend_define_key(struct GuiButton *gbtn)
{
    long key_id = define_key_scroll_offset - ((long)gbtn->content) - 1;
    defining_a_key = 1;
    defining_a_key_id = key_id;
    lbInkey = 0;
}

void frontend_draw_define_key_scroll_tab(struct GuiButton *gbtn)
{
    frontend_draw_scroll_tab(gbtn, define_key_scroll_offset, frontend_define_keys_menu_items_visible-2, GAME_KEYS_COUNT);
}

void frontend_draw_define_key(struct GuiButton *gbtn)
{
    long content = (long)gbtn->content;
    long key_id = define_key_scroll_offset - content - 1;
    if (key_id >= GAME_KEYS_COUNT) {
        return;
    }
    if (frontend_mouse_over_button == content) {
        LbTextSetFont(frontend_font[2]);
    } else if (defined_keys_that_have_been_swapped[key_id]) {
        LbTextSetFont(frontend_font[3]);
    } else {
        LbTextSetFont(frontend_font[1]);
    }
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    // This text is a bit condensed - button size is smaller than text height
    int tx_units_per_px = (gbtn->height * 13 / 11) * 16 / LbTextLineHeight();
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    int height = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextDrawResized(0, (gbtn->height - height) / 2, tx_units_per_px, get_string(definable_key_string[key_id]));
    unsigned char mods = settings.kbkeys[key_id].mods;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;

    char text[255];
    text[0] = '\0';
    if (mods & KMod_CONTROL)
    {
        strcat(text, get_string(GUIStr_KeyControl));
        strcat(text, " + ");
    }
    if (mods & KMod_ALT)
    {
        strcat(text, get_string(GUIStr_KeyAlt));
        strcat(text, " + ");
    }
    if (mods & KMod_SHIFT)
    {
        strcat(text, get_string(GUIStr_KeyShift));
        strcat(text, " + ");
    }

    unsigned char code = settings.kbkeys[key_id].code;
    const char* keytext;
    char chbuf[2];
    switch (code)
    {
      case KC_LSHIFT:
      case KC_RSHIFT:
        keytext = get_string(GUIStr_KeyShift);
        break;
      case KC_LCONTROL:
      case KC_RCONTROL:
        keytext = get_string(GUIStr_KeyControl);
        break;
      case KC_LALT:
      case KC_RALT:
        keytext = get_string(GUIStr_KeyAlt);
        break;
      case KC_MOUSE9:
      case KC_MOUSE8:
      case KC_MOUSE7:
      case KC_MOUSE6:
      case KC_MOUSE5:
      case KC_MOUSE4:
      case KC_MOUSE3:
      case KC_MOUSE2:
      case KC_MOUSE1:
      {
        char mouse_button_label[255] = "";
        const char* mouse_gui_string = get_string(key_to_string[(long)code]);
        int mouse_button_number = (KC_MOUSE1 + 1 - code);
        char mouse_button_number_string[8];
        itoa(mouse_button_number, mouse_button_number_string, 10);
        strcat(mouse_button_label, mouse_gui_string);
        strcat(mouse_button_label, " ");
        strcat(mouse_button_label, mouse_button_number_string);
        keytext = mouse_button_label;
        break;
      }
      default:
      {
        long i = key_to_string[code];
        if (i >= 0)
            keytext = get_string(i);
        else
        {
            chbuf[0] = -(char)i;
            chbuf[1] = 0;
            keytext = chbuf;
        }
        break;
      }
    }
    strcat(text, keytext);
    height = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextDrawResized(0, (gbtn->height - height) / 2, tx_units_per_px, text);
}

void gui_video_shadows(struct GuiButton *gbtn)
{
    settings.video_shadows = _DK_video_shadows;
    copy_settings_to_dk_settings();
}

void gui_video_view_distance_level(struct GuiButton *gbtn)
{
    settings.view_distance = video_view_distance_level;
    copy_settings_to_dk_settings();
}

void gui_video_rotate_mode(struct GuiButton *gbtn)
{
    struct Packet* pckt = get_packet(my_player_number);
    if (settings.video_rotate_mode) {
        set_packet_action(pckt, PckA_SwitchView, 5, 0, 0, 0);
    } else {
        set_packet_action(pckt, PckA_SwitchView, 2, 0, 0, 0);
    }
    save_settings();
}

void gui_video_cluedo_mode(struct GuiButton *gbtn)
{
    struct Packet* pckt = get_packet(my_player_number);
    set_packet_action(pckt, PckA_SetCluedo, _DK_video_cluedo_mode, 0, 0, 0);
}

void gui_video_gamma_correction(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    video_gamma_correction = (video_gamma_correction + 1) % GAMMA_LEVELS_COUNT;
    set_players_packet_action(player, PckA_SetGammaLevel, video_gamma_correction, 0, 0, 0);
}

void gui_set_sound_volume(struct GuiButton *gbtn)
{
    if (gbtn->id_num == BID_SOUND_VOL)
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
    settings.redbook_volume = music_level;
    save_settings();
    SetMusicPlayerVolume(settings.redbook_volume);
}

void gui_video_cluedo_maintain(struct GuiButton *gbtn)
{
    struct PlayerInfo* player = get_my_player();
    if (player->view_mode == PVM_FrontView)
    {
        gbtn->btype_value |= LbBFeF_NoTooltip;
        gbtn->flags &= ~LbBtnF_Enabled;
    } else
    {
        gbtn->btype_value &= LbBFeF_IntValueMask;
        gbtn->flags |= LbBtnF_Enabled;
    }
}

void frontend_set_mouse_sensitivity(struct GuiButton *gbtn)
{
    settings.first_person_move_sensitivity = fe_mouse_sensitivity;
    save_settings();
}

void frontend_invert_mouse(struct GuiButton *gbtn)
{
    settings.first_person_move_invert = !settings.first_person_move_invert;
    save_settings();
}

void frontend_draw_invert_mouse(struct GuiButton *gbtn)
{
    int font_idx = frontend_button_caption_font(gbtn, frontend_mouse_over_button);
    LbTextSetFont(frontend_font[font_idx]);
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    int tx_units_per_px = gbtn->height * 16 / LbTextLineHeight();
    const char *text;
    if (settings.first_person_move_invert) {
        text = get_string(GUIStr_On);
    } else {
        text = get_string(GUIStr_Off);
    }
    LbTextDrawResized(0, 0, tx_units_per_px, text);
}

/**
 * Initializes start state of GUI menu settings.
 * @param gmnu The GUI menu which is being initialized.
 */
void init_video_menu(struct GuiMenu *gmnu)
{
    _DK_video_shadows = settings.video_shadows;
    video_view_distance_level = settings.view_distance;
    _DK_video_textures = settings.video_textures;
    _DK_video_cluedo_mode = settings.video_cluedo_mode;
    video_gamma_correction = settings.gamma_correction;
}

/**
 * Initializes start state of GUI menu settings.
 * @param gmnu The GUI menu which is being initialized.
 */
void init_audio_menu(struct GuiMenu *gmnu)
{
    music_level = settings.redbook_volume;
    sound_level = settings.sound_volume;
}
/******************************************************************************/
