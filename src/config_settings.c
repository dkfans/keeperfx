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

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_keybrd.h"
#include "bflib_video.h"
#include "bflib_cpu.h"

#include "config.h"
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
     0,                         // field_0
     4,                         // video_shadows
     3,                         // view_distance
     0,                         // video_rotate_mode
     1,                         // video_textures
     0,                         // video_cluedo_mode
     127,                       // sound_volume
     90,                        // redbook_volume
     1,                         // field_8
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
          {KC_T, KMod_NONE},                 // Gkey_ZoomRoom00
          {KC_L, KMod_NONE},                 // Gkey_ZoomRoom01
          {KC_L, KMod_SHIFT},                // Gkey_ZoomRoom02
          {KC_P, KMod_SHIFT},                // Gkey_ZoomRoom03
          {KC_T, KMod_ALT},                  // Gkey_ZoomRoom04
          {KC_T, KMod_SHIFT},                // Gkey_ZoomRoom05
          {KC_H, KMod_NONE},                 // Gkey_ZoomRoom06
          {KC_W, KMod_ALT},                  // Gkey_ZoomRoom07
          {KC_S, KMod_ALT},                  // Gkey_ZoomRoom08
          {KC_T, KMod_CONTROL},              // Gkey_ZoomRoom09
          {KC_G, KMod_NONE},                 // Gkey_ZoomRoom10
          {KC_B, KMod_NONE},                 // Gkey_ZoomRoom11
          {KC_H, KMod_SHIFT},                // Gkey_ZoomRoom12
          {KC_G, KMod_SHIFT},                // Gkey_ZoomRoom13
          {KC_B, KMod_SHIFT},                // Gkey_ZoomRoom14
          {KC_P, KMod_CONTROL},              // Gkey_ZoomRoom15
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
     },                         // kbkeys
     true,                      // tooltips_on
     0,                         // first_person_move_invert
     6,                         // first_person_move_sensitivity
     256,                       // minimap_zoom
     8192,                      // isometric_view_zoom_level
     65536,                     // frontview_zoom_level
     127,                       // mentor_volume
    };
    LbMemoryCopy(&settings, &default_settings, sizeof(struct GameSettings));
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
