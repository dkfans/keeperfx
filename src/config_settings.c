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
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned char i_can_see_levels[] = {30, 45, 60, 254,};
struct GameSettings settings;
/******************************************************************************/
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
    char* fname = prepare_file_path(FGrp_Save, "settings.dat");
    long len = LbFileLengthRnc(fname);
    if (len == sizeof(struct GameSettings))
    {
      if (LbFileLoadAt(fname, &settings) == sizeof(struct GameSettings))
      {
          // sanity checks
          settings.video_shadows = clamp(settings.video_shadows, 0, 3);
          settings.view_distance = clamp(settings.view_distance, 0, 3);
          settings.video_rotate_mode = clamp(settings.video_rotate_mode, 0, 2);
          settings.video_textures = clamp(settings.video_textures, 0, 1);
          settings.video_cluedo_mode = clamp(settings.video_cluedo_mode, 0, 1);
          settings.sound_volume = clamp(settings.sound_volume, 0, FULL_LOUDNESS);
          settings.music_volume = clamp(settings.music_volume, 0, FULL_LOUDNESS);
          settings.gamma_correction = clamp(settings.gamma_correction, 0, GAMMA_LEVELS_COUNT);
          settings.switching_vidmodes_index = clamp(settings.switching_vidmodes_index, 0, MAX_GAME_VIDMODE_COUNT);
          settings.first_person_move_sensitivity = clamp(settings.first_person_move_sensitivity, 0, 1000);
          settings.minimap_zoom = clamp(settings.minimap_zoom, 256, 2048);
          settings.isometric_view_zoom_level = clamp(settings.isometric_view_zoom_level, CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX);
          settings.frontview_zoom_level = clamp(settings.frontview_zoom_level, FRONTVIEW_CAMERA_ZOOM_MIN, FRONTVIEW_CAMERA_ZOOM_MAX);
          settings.mentor_volume = clamp(settings.mentor_volume, 0, 127);
          settings.isometric_tilt = clamp(settings.isometric_tilt, CAMERA_TILT_MIN, CAMERA_TILT_MAX);
          settings.highlight_mode = clamp(settings.highlight_mode, false, true);
          return true;
      }
    }
    setup_default_settings();
    LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
    return false;
}

short save_settings(void)
{
    char* fname = prepare_file_path(FGrp_Save, "settings.dat");
    LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
    return true;
}

int get_max_i_can_see_from_settings(void)
{
    return i_can_see_levels[settings.view_distance % 4];
}
/******************************************************************************/
