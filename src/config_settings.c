/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_settings.c
 *     List of language-specific strings support.
 * @par Purpose:
 *     Support of configuration files for game strings.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Nov 2011 - 01 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_settings.h"
#include "globals.h"
#include "sounds.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_keybrd.h"
#include "bflib_video.h"
#include "frontmenu_options.h"
#include "config.h"
#include "engine_camera.h"
#include "game_merge.h"
#include "vidmode.h"
#include "value_util.h"
#include <ctype.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned char i_can_see_levels[] = {30, 45, 60, 254,};
struct GameSettings settings;
/******************************************************************************/

static const struct { unsigned char code; const char *name; } keycode_table[] = {
    { KC_UNASSIGNED,       "UNASSIGNED" },
    { KC_ESCAPE,           "ESCAPE" },
    { KC_1,                "1" },
    { KC_2,                "2" },
    { KC_3,                "3" },
    { KC_4,                "4" },
    { KC_5,                "5" },
    { KC_6,                "6" },
    { KC_7,                "7" },
    { KC_8,                "8" },
    { KC_9,                "9" },
    { KC_0,                "0" },
    { KC_MINUS,            "MINUS" },
    { KC_EQUALS,           "EQUALS" },
    { KC_BACK,             "BACK" },
    { KC_TAB,              "TAB" },
    { KC_Q,                "Q" },
    { KC_W,                "W" },
    { KC_E,                "E" },
    { KC_R,                "R" },
    { KC_T,                "T" },
    { KC_Y,                "Y" },
    { KC_U,                "U" },
    { KC_I,                "I" },
    { KC_O,                "O" },
    { KC_P,                "P" },
    { KC_LBRACKET,         "LBRACKET" },
    { KC_RBRACKET,         "RBRACKET" },
    { KC_RETURN,           "RETURN" },
    { KC_LCONTROL,         "LCONTROL" },
    { KC_A,                "A" },
    { KC_S,                "S" },
    { KC_D,                "D" },
    { KC_F,                "F" },
    { KC_G,                "G" },
    { KC_H,                "H" },
    { KC_J,                "J" },
    { KC_K,                "K" },
    { KC_L,                "L" },
    { KC_SEMICOLON,        "SEMICOLON" },
    { KC_APOSTROPHE,       "APOSTROPHE" },
    { KC_GRAVE,            "GRAVE" },
    { KC_LSHIFT,           "LSHIFT" },
    { KC_BACKSLASH,        "BACKSLASH" },
    { KC_Z,                "Z" },
    { KC_X,                "X" },
    { KC_C,                "C" },
    { KC_V,                "V" },
    { KC_B,                "B" },
    { KC_N,                "N" },
    { KC_M,                "M" },
    { KC_COMMA,            "COMMA" },
    { KC_PERIOD,           "PERIOD" },
    { KC_SLASH,            "SLASH" },
    { KC_RSHIFT,           "RSHIFT" },
    { KC_MULTIPLY,         "MULTIPLY" },
    { KC_LALT,             "LALT" },
    { KC_SPACE,            "SPACE" },
    { KC_CAPITAL,          "CAPITAL" },
    { KC_F1,               "F1" },
    { KC_F2,               "F2" },
    { KC_F3,               "F3" },
    { KC_F4,               "F4" },
    { KC_F5,               "F5" },
    { KC_F6,               "F6" },
    { KC_F7,               "F7" },
    { KC_F8,               "F8" },
    { KC_F9,               "F9" },
    { KC_F10,              "F10" },
    { KC_NUMLOCK,          "NUMLOCK" },
    { KC_SCROLL,           "SCROLL" },
    { KC_NUMPAD7,          "NUMPAD7" },
    { KC_NUMPAD8,          "NUMPAD8" },
    { KC_NUMPAD9,          "NUMPAD9" },
    { KC_SUBTRACT,         "SUBTRACT" },
    { KC_NUMPAD4,          "NUMPAD4" },
    { KC_NUMPAD5,          "NUMPAD5" },
    { KC_NUMPAD6,          "NUMPAD6" },
    { KC_ADD,              "ADD" },
    { KC_NUMPAD1,          "NUMPAD1" },
    { KC_NUMPAD2,          "NUMPAD2" },
    { KC_NUMPAD3,          "NUMPAD3" },
    { KC_NUMPAD0,          "NUMPAD0" },
    { KC_DECIMAL,          "DECIMAL" },
    { KC_F11,              "F11" },
    { KC_F12,              "F12" },
    { KC_NUMPADENTER,      "NUMPADENTER" },
    { KC_RCONTROL,         "RCONTROL" },
    { KC_DIVIDE,           "DIVIDE" },
    { KC_RALT,             "RALT" },
    { KC_HOME,             "HOME" },
    { KC_UP,               "UP" },
    { KC_PGUP,             "PGUP" },
    { KC_LEFT,             "LEFT" },
    { KC_RIGHT,            "RIGHT" },
    { KC_END,              "END" },
    { KC_DOWN,             "DOWN" },
    { KC_PGDOWN,           "PGDOWN" },
    { KC_INSERT,           "INSERT" },
    { KC_DELETE,           "DELETE" },
    { KC_MOUSE9,           "MOUSE9" },
    { KC_MOUSE8,           "MOUSE8" },
    { KC_MOUSE7,           "MOUSE7" },
    { KC_MOUSE6,           "MOUSE6" },
    { KC_MOUSE5,           "MOUSE5" },
    { KC_MOUSE4,           "MOUSE4" },
    { KC_MOUSE3,           "MOUSE3" },
    { KC_MOUSE2,           "MOUSE2" },
    { KC_MOUSE1,           "MOUSE1" },
    { KC_MOUSEWHEEL_DOWN,  "MOUSEWHEEL_DOWN" },
    { KC_MOUSEWHEEL_UP,    "MOUSEWHEEL_UP" },
};
#define KEYCODE_TABLE_SIZE ((int)(sizeof(keycode_table)/sizeof(keycode_table[0])))

static const char *game_key_names[GAME_KEYS_COUNT] = {
    "MoveUp",                   // Gkey_MoveUp
    "MoveDown",                 // Gkey_MoveDown
    "MoveLeft",                 // Gkey_MoveLeft
    "MoveRight",                // Gkey_MoveRight
    "RotateMod",                // Gkey_RotateMod
    "SpeedMod",                 // Gkey_SpeedMod
    "RotateCW",                 // Gkey_RotateCW
    "RotateCCW",                // Gkey_RotateCCW
    "ZoomIn",                   // Gkey_ZoomIn
    "ZoomOut",                  // Gkey_ZoomOut
    "ZoomRoomTreasure",         // Gkey_ZoomRoomTreasure
    "ZoomRoomLibrary",          // Gkey_ZoomRoomLibrary
    "ZoomRoomLair",             // Gkey_ZoomRoomLair
    "ZoomRoomPrison",           // Gkey_ZoomRoomPrison
    "ZoomRoomTorture",          // Gkey_ZoomRoomTorture
    "ZoomRoomTraining",         // Gkey_ZoomRoomTraining
    "ZoomRoomHeart",            // Gkey_ZoomRoomHeart
    "ZoomRoomWorkshop",         // Gkey_ZoomRoomWorkshop
    "ZoomRoomScavenger",        // Gkey_ZoomRoomScavenger
    "ZoomRoomTemple",           // Gkey_ZoomRoomTemple
    "ZoomRoomGraveyard",        // Gkey_ZoomRoomGraveyard
    "ZoomRoomBarracks",         // Gkey_ZoomRoomBarracks
    "ZoomRoomHatchery",         // Gkey_ZoomRoomHatchery
    "ZoomRoomGuardPost",        // Gkey_ZoomRoomGuardPost
    "ZoomRoomBridge",           // Gkey_ZoomRoomBridge
    "ZoomRoomPortal",           // Gkey_ZoomRoomPortal
    "ZoomToFight",              // Gkey_ZoomToFight
    "ZoomCrAnnoyed",            // Gkey_ZoomCrAnnoyed
    "CrtrContrlMod",            // Gkey_CrtrContrlMod
    "CrtrQueryMod",             // Gkey_CrtrQueryMod
    "DumpToOldPos",             // Gkey_DumpToOldPos
    "TogglePause",              // Gkey_TogglePause
    "SwitchToMap",              // Gkey_SwitchToMap
    "ToggleMessage",            // Gkey_ToggleMessage
    "SnapCamera",               // Gkey_SnapCamera
    "BestRoomSpace",            // Gkey_BestRoomSpace
    "SquareRoomSpace",          // Gkey_SquareRoomSpace
    "RoomSpaceIncSize",         // Gkey_RoomSpaceIncSize
    "RoomSpaceDecSize",         // Gkey_RoomSpaceDecSize
    "SellTrapOnSubtile",        // Gkey_SellTrapOnSubtile
    "TiltUp",                   // Gkey_TiltUp
    "TiltDown",                 // Gkey_TiltDown
    "TiltReset",                // Gkey_TiltReset
    "Ascend",                   // Gkey_Ascend
    "Descend",                  // Gkey_Descend
};

static const char *keycode_to_name(unsigned char code)
{
    for (int i = 0; i < KEYCODE_TABLE_SIZE; i++)
        if (keycode_table[i].code == code)
            return keycode_table[i].name;
    return "UNASSIGNED";
}

static unsigned char name_to_keycode(const char *name)
{
    for (int i = 0; i < KEYCODE_TABLE_SIZE; i++)
        if (strcmp(keycode_table[i].name, name) == 0)
            return keycode_table[i].code;
    return KC_UNASSIGNED;
}

static void kmod_to_name(unsigned char mods, char *buf, size_t buflen)
{
    int len = 0;
    unsigned char flags = mods & (KMod_SHIFT | KMod_CONTROL | KMod_ALT);

    if (buflen == 0)
        return;

    if (flags == KMod_NONE)
    {
        snprintf(buf, buflen, "NONE");
        return;
    }

    buf[0] = '\0';
    if ((flags & KMod_SHIFT) != 0)
        len += snprintf(buf + len, buflen - len, "%sSHIFT", (len > 0) ? "|" : "");
    if ((flags & KMod_CONTROL) != 0)
        len += snprintf(buf + len, buflen - len, "%sCTRL", (len > 0) ? "|" : "");
    if ((flags & KMod_ALT) != 0)
        snprintf(buf + len, buflen - len, "%sALT", (len > 0) ? "|" : "");
}

static unsigned char name_to_kmod(const char *name)
{
    unsigned char mods = KMod_NONE;
    char token[16];
    int token_len = 0;

    if (name == NULL)
        return mods;

    for (const char *p = name;; p++)
    {
        unsigned char c = (unsigned char)(*p);
        TbBool token_end = (c == '\0' || c == '|' || c == '+' || c == ',' || isspace(c));

        if (!token_end)
        {
            if (token_len < (int)sizeof(token) - 1)
                token[token_len++] = (char)toupper(c);
            continue;
        }

        if (token_len > 0)
        {
            token[token_len] = '\0';
            if (strcmp(token, "SHIFT") == 0)
                mods |= KMod_SHIFT;
            else if (strcmp(token, "CTRL") == 0)
                mods |= KMod_CONTROL;
            else if (strcmp(token, "ALT") == 0)
                mods |= KMod_ALT;
            token_len = 0;
        }

        if (c == '\0')
            break;
    }

    return mods;
}

#ifdef __cplusplus
}
#endif
/******************************************************************************/
void setup_default_settings(void)
{
    // CPU status variable
    const struct GameSettings default_settings = {
     0,                         // unusedfield_0
     4,                         // video_shadows
     3,                         // view_distance
     0,                         // video_rotate_mode
     1,                         // video_textures
     0,                         // video_cluedo_mode
     127,                       // sound_volume
     90,                        // music_volume
     1,                         // unusedfield_8
     0,                         // gamma_correction
     Lb_SCREEN_MODE_INVALID,    // Screen mode, set to correct value below
     {
          {KC_W, KMod_NONE},                 // Gkey_MoveUp
          {KC_S, KMod_NONE},                 // Gkey_MoveDown
          {KC_A, KMod_NONE},                 // Gkey_MoveLeft
          {KC_D, KMod_NONE},                 // Gkey_MoveRight
          {KC_LCONTROL, KMod_NONE},          // Gkey_RotateMod
          {KC_LSHIFT, KMod_NONE},            // Gkey_SpeedMod
          {KC_DELETE, KMod_NONE},            // Gkey_RotateCW
          {KC_PGDOWN, KMod_NONE},            // Gkey_RotateCCW
          {KC_HOME, KMod_NONE},              // Gkey_ZoomIn
          {KC_END, KMod_NONE},               // Gkey_ZoomOut
          {KC_T, KMod_NONE},                 // Gkey_ZoomRoomTreasure
          {KC_L, KMod_NONE},                 // Gkey_ZoomRoomLibrary
          {KC_L, KMod_SHIFT},                // Gkey_ZoomRoomLair
          {KC_P, KMod_SHIFT},                // Gkey_ZoomRoomPrison
          {KC_T, KMod_ALT},                  // Gkey_ZoomRoomTorture
          {KC_T, KMod_SHIFT},                // Gkey_ZoomRoomTraining
          {KC_H, KMod_NONE},                 // Gkey_ZoomRoomHeart
          {KC_W, KMod_ALT},                  // Gkey_ZoomRoomWorkshop
          {KC_S, KMod_ALT},                  // Gkey_ZoomRoomScavenger
          {KC_T, KMod_CONTROL},              // Gkey_ZoomRoomTemple
          {KC_G, KMod_NONE},                 // Gkey_ZoomRoomGraveyard
          {KC_B, KMod_NONE},                 // Gkey_ZoomRoomBarracks
          {KC_H, KMod_SHIFT},                // Gkey_ZoomRoomHatchery
          {KC_G, KMod_SHIFT},                // Gkey_ZoomRoomGuardPost
          {KC_B, KMod_SHIFT},                // Gkey_ZoomRoomBridge
          {KC_P, KMod_CONTROL},              // Gkey_ZoomRoomPortal
          {KC_F, KMod_NONE},                 // Gkey_ZoomToFight
          {KC_A, KMod_ALT},                  // Gkey_ZoomCrAnnoyed
          {KC_LSHIFT, KMod_NONE},            // Gkey_CrtrContrlMod
          {KC_Q, KMod_NONE},                 // Gkey_CrtrQueryMod
          {KC_BACK, KMod_NONE},              // Gkey_DumpToOldPos
          {KC_P, KMod_NONE},                 // Gkey_TogglePause
          {KC_M, KMod_NONE},                 // Gkey_SwitchToMap
          {KC_E, KMod_NONE},                 // Gkey_ToggleMessage
          {KC_MOUSE3, KMod_NONE},            // Gkey_SnapCamera
          {KC_LSHIFT, KMod_NONE},            // Gkey_BestRoomSpace
          {KC_LCONTROL, KMod_NONE},          // Gkey_SquareRoomSpace
          {KC_MOUSEWHEEL_DOWN, KMod_NONE},   // Gkey_RoomSpaceIncSize
          {KC_MOUSEWHEEL_UP, KMod_NONE},     // Gkey_RoomSpaceDecSize
          {KC_LALT, KMod_NONE},              // Gkey_SellTrapOnSubtile
          {KC_PGUP, KMod_SHIFT},             // Gkey_TiltUp
          {KC_PGDOWN, KMod_SHIFT},           // Gkey_TiltDown
          {KC_INSERT, KMod_SHIFT},           // Gkey_TiltReset
          {KC_X, KMod_NONE},                 // Gkey_Ascend
          {KC_Z, KMod_NONE},                 // Gkey_Descend
     },                         // kbkeys
     true,                      // tooltips_on
     0,                         // first_person_move_invert
     6,                         // first_person_move_sensitivity
     256,                       // minimap_zoom
     8192,                      // isometric_view_zoom_level
     FRONTVIEW_CAMERA_ZOOM_MAX, // frontview_zoom_level
     127,                       // mentor_volume
     CAMERA_TILT_DEFAULT,       // isometric_tilt
     false,                     // highlight_mode
    };
    memcpy(&settings, &default_settings, sizeof(struct GameSettings));
    settings.switching_vidmodes_index = 0;
}

TbBool load_settings(void)
{
    SYNCDBG(6,"Starting");
    setup_default_settings();

    char *fname = prepare_file_path(FGrp_Save, "settings.toml");
    VALUE root;
    if (!load_toml_file(fname, &root, CnfLd_IgnoreErrors))
    {
        save_settings();
        return false;
    }

    VALUE *vsec;
    VALUE *val;

    /* [video] */
    vsec = value_dict_get(&root, "video");
    if (vsec)
    {
        val = value_dict_get(vsec, "detail_level");
        if (val && value_type(val) == VALUE_INT32) settings.video_detail_level = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "shadows");
        if (val && value_type(val) == VALUE_INT32) settings.video_shadows = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "view_distance");
        if (val && value_type(val) == VALUE_INT32) settings.view_distance = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "rotate_mode");
        if (val && value_type(val) == VALUE_INT32) settings.video_rotate_mode = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "textures");
        if (val && value_type(val) == VALUE_INT32) settings.video_textures = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "cluedo_mode");
        if (val && value_type(val) == VALUE_INT32) settings.video_cluedo_mode = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "gamma_correction");
        if (val && value_type(val) == VALUE_INT32) settings.gamma_correction = (unsigned short)value_int32(val);
        val = value_dict_get(vsec, "roomflags_on");
        if (val && value_type(val) == VALUE_INT32) settings.roomflags_on = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "switching_vidmodes_index");
        if (val && value_type(val) == VALUE_INT32) settings.switching_vidmodes_index = value_int32(val);
    }

    /* [audio] */
    vsec = value_dict_get(&root, "audio");
    if (vsec)
    {
        val = value_dict_get(vsec, "sound_volume");
        if (val && value_type(val) == VALUE_INT32) settings.sound_volume = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "music_volume");
        if (val && value_type(val) == VALUE_INT32) settings.music_volume = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "mentor_volume");
        if (val && value_type(val) == VALUE_INT32) settings.mentor_volume = (long)value_int32(val);
    }

    /* [display] */
    vsec = value_dict_get(&root, "display");
    if (vsec)
    {
        val = value_dict_get(vsec, "minimap_zoom");
        if (val && value_type(val) == VALUE_INT32) settings.minimap_zoom = (unsigned int)value_int32(val);
        val = value_dict_get(vsec, "isometric_view_zoom_level");
        if (val && value_type(val) == VALUE_INT32) settings.isometric_view_zoom_level = (unsigned long)value_int32(val);
        val = value_dict_get(vsec, "frontview_zoom_level");
        if (val && value_type(val) == VALUE_INT32) settings.frontview_zoom_level = (unsigned long)value_int32(val);
        val = value_dict_get(vsec, "isometric_tilt");
        if (val && value_type(val) == VALUE_INT32) settings.isometric_tilt = value_int32(val);
        val = value_dict_get(vsec, "tooltips_on");
        if (val) settings.tooltips_on = (TbBool)value_coerce_bool(val);
        val = value_dict_get(vsec, "highlight_mode");
        if (val) settings.highlight_mode = (TbBool)value_coerce_bool(val);
    }

    /* [gameplay] */
    vsec = value_dict_get(&root, "gameplay");
    if (vsec)
    {
        val = value_dict_get(vsec, "first_person_move_invert");
        if (val && value_type(val) == VALUE_INT32) settings.first_person_move_invert = (unsigned char)value_int32(val);
        val = value_dict_get(vsec, "first_person_move_sensitivity");
        if (val && value_type(val) == VALUE_INT32) settings.first_person_move_sensitivity = (unsigned char)value_int32(val);
    }

    /* [keys] */
    vsec = value_dict_get(&root, "keys");
    if (vsec)
    {
        for (int i = 0; i < GAME_KEYS_COUNT; i++)
        {
            val = value_dict_get(vsec, game_key_names[i]);
            if (val && value_type(val) == VALUE_DICT)
            {
                VALUE *vcode = value_dict_get(val, "code");
                VALUE *vmods = value_dict_get(val, "mods");
                if (vcode && value_type(vcode) == VALUE_STRING)
                    settings.kbkeys[i].code = name_to_keycode(value_string(vcode));
                if (vmods && value_type(vmods) == VALUE_STRING)
                    settings.kbkeys[i].mods = name_to_kmod(value_string(vmods));
            }
        }
    }

    value_fini(&root);

    // sanity checks
    settings.video_shadows = clamp(settings.video_shadows, 0, 3);
    settings.view_distance = clamp(settings.view_distance, 0, 3);
    settings.video_rotate_mode = clamp(settings.video_rotate_mode, 0, 2);
    settings.video_textures = clamp(settings.video_textures, 0, 1);
    settings.video_cluedo_mode = clamp(settings.video_cluedo_mode, 0, 1);
    settings.sound_volume = clamp(settings.sound_volume, 0, FULL_LOUDNESS);
    settings.music_volume = clamp(settings.music_volume, 0, FULL_LOUDNESS);
    settings.mentor_volume = clamp(settings.mentor_volume, 0, FULL_LOUDNESS);
    settings.gamma_correction = clamp(settings.gamma_correction, 0, GAMMA_LEVELS_COUNT);
    settings.switching_vidmodes_index = clamp(settings.switching_vidmodes_index, 0, MAX_GAME_VIDMODE_COUNT);
    settings.first_person_move_sensitivity = clamp(settings.first_person_move_sensitivity, 0, 1000);
    settings.minimap_zoom = clamp(settings.minimap_zoom, 256, 2048);
    settings.isometric_view_zoom_level = clamp(settings.isometric_view_zoom_level, CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX);
    settings.frontview_zoom_level = clamp(settings.frontview_zoom_level, FRONTVIEW_CAMERA_ZOOM_MIN, FRONTVIEW_CAMERA_ZOOM_MAX);
    settings.isometric_tilt = clamp(settings.isometric_tilt, CAMERA_TILT_MIN, CAMERA_TILT_MAX);
    settings.highlight_mode = clamp(settings.highlight_mode, false, true);
    return true;
}

short save_settings(void)
{
    char *fname = prepare_file_path(FGrp_Save, "settings.toml");

    char *buf = (char *)malloc(16384);
    if (!buf) return false;
    int len = 0;
    int maxlen = 16384;
#define TOSAVE(...) len += snprintf(buf + len, maxlen - len, __VA_ARGS__)

    TOSAVE("# This file gets written by the game automatically.\n");
    TOSAVE("# Do not edit manually unless you know what you're doing.\n");
    TOSAVE("[video]\n");
    TOSAVE("detail_level = %d\n", (int)settings.video_detail_level);
    TOSAVE("shadows = %d\n", (int)settings.video_shadows);
    TOSAVE("view_distance = %d\n", (int)settings.view_distance);
    TOSAVE("rotate_mode = %d\n", (int)settings.video_rotate_mode);
    TOSAVE("textures = %d\n", (int)settings.video_textures);
    TOSAVE("cluedo_mode = %d\n", (int)settings.video_cluedo_mode);
    TOSAVE("gamma_correction = %d\n", (int)settings.gamma_correction);
    TOSAVE("roomflags_on = %d\n", (int)settings.roomflags_on);
    TOSAVE("switching_vidmodes_index = %d\n", settings.switching_vidmodes_index);
    TOSAVE("\n[audio]\n");
    TOSAVE("sound_volume = %d\n", (int)settings.sound_volume);
    TOSAVE("music_volume = %d\n", (int)settings.music_volume);
    TOSAVE("mentor_volume = %ld\n", settings.mentor_volume);
    TOSAVE("\n[display]\n");
    TOSAVE("minimap_zoom = %u\n", settings.minimap_zoom);
    TOSAVE("isometric_view_zoom_level = %lu\n", settings.isometric_view_zoom_level);
    TOSAVE("frontview_zoom_level = %lu\n", settings.frontview_zoom_level);
    TOSAVE("isometric_tilt = %d\n", settings.isometric_tilt);
    TOSAVE("tooltips_on = %s\n", settings.tooltips_on ? "true" : "false");
    TOSAVE("highlight_mode = %s\n", settings.highlight_mode ? "true" : "false");
    TOSAVE("\n[gameplay]\n");
    TOSAVE("first_person_move_invert = %d\n", (int)settings.first_person_move_invert);
    TOSAVE("first_person_move_sensitivity = %d\n", (int)settings.first_person_move_sensitivity);
    TOSAVE("\n[keys]\n");
    for (int i = 0; i < GAME_KEYS_COUNT; i++)
    {
        char mods_buf[32];
        kmod_to_name(settings.kbkeys[i].mods, mods_buf, sizeof(mods_buf));
        TOSAVE("%s = { code = \"%s\", mods = \"%s\" }\n",
            game_key_names[i],
            keycode_to_name(settings.kbkeys[i].code),
            mods_buf);
    }
#undef TOSAVE

    LbFileSaveAt(fname, buf, len);
    free(buf);
    return true;
}

int get_max_i_can_see_from_settings(void)
{
    return i_can_see_levels[settings.view_distance % 4];
}
/******************************************************************************/
