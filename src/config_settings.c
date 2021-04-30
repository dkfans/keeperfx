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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned char i_can_see_levels[] = {15, 20, 25, 30,};
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
     1,                         // tooltips_on
     0,                         // first_person_move_invert
     6,                         // first_person_move_sensitivity
     256,                       // minimap_zoom
     8192,                      // isometric_view_zoom_level
     65536,                     // frontview_zoom_level
    };
    LbMemoryCopy(&settings, &default_settings, sizeof(struct GameSettings));
    struct CPU_INFO cpu_info;
    cpu_detect(&cpu_info);
    settings.video_scrnmode = get_next_vidmode(Lb_SCREEN_MODE_INVALID);
    if ((cpu_get_family(&cpu_info) > CPUID_FAMILY_PENTIUM) && (is_feature_on(Ft_HiResVideo)))
    {
        SYNCDBG(6,"Updating to hires video mode");
        settings.video_scrnmode = get_higher_vidmode(settings.video_scrnmode);
    }
}

void copy_settings_to_dk_settings(void)
{
    _DK_settings.field_0 = settings.field_0;
    _DK_settings.video_shadows = settings.video_shadows;
    _DK_settings.view_distance = settings.view_distance;
    _DK_settings.video_rotate_mode = settings.video_rotate_mode;
    _DK_settings.video_textures = settings.video_textures;
    _DK_settings.video_cluedo_mode = settings.video_cluedo_mode;
    _DK_settings.sound_volume = settings.sound_volume;
    _DK_settings.redbook_volume = settings.redbook_volume;
    _DK_settings.roomflags_on = settings.roomflags_on;
    _DK_settings.gamma_correction = settings.gamma_correction;
    _DK_settings.video_scrnmode = settings.video_scrnmode;
    for (int i = 0; i < DK_GAME_KEYS_COUNT; i++)
    {
        _DK_settings.kbkeys[i] = settings.kbkeys[i];
    }
    _DK_settings.tooltips_on = settings.tooltips_on;
    _DK_settings.first_person_move_invert = settings.first_person_move_invert;
    _DK_settings.first_person_move_sensitivity = settings.first_person_move_sensitivity;
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
          copy_settings_to_dk_settings();
          return true;
      }
    }
    setup_default_settings();
    LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
    return false;
}

short save_settings(void)
{
    copy_settings_to_dk_settings();
    char* fname = prepare_file_path(FGrp_Save, "settings.dat");
    LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
    return true;
}

int get_creature_can_see_subtiles(void)
{
    return i_can_see_levels[settings.view_distance % 4];
}
/******************************************************************************/
