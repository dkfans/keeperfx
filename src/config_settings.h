/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_settings.h
 *     Header file for config_settings.c.
 * @par Purpose:
 *     List of language-specific strings support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 31 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGSETTINGS_H
#define DK_CFGSETTINGS_H

#include "globals.h"
#include "bflib_basics.h"

#define DK_GAME_KEYS_COUNT     32
#define GAME_KEYS_COUNT        39

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct GameKey { // sizeof = 2
  unsigned char code;
  unsigned char mods;
};

struct _DK_GameSettings { // sizeof = 0x52 (82)
    unsigned char field_0;
    unsigned char video_shadows;
    unsigned char view_distance;
    unsigned char video_rotate_mode;
    unsigned char video_textures;
    unsigned char video_cluedo_mode;
    unsigned char sound_volume;
    unsigned char redbook_volume;
    unsigned char roomflags_on;
    unsigned short gamma_correction;
    int video_scrnmode;
    struct GameKey kbkeys[DK_GAME_KEYS_COUNT];
    unsigned char tooltips_on;
    unsigned char first_person_move_invert;
    unsigned char first_person_move_sensitivity;
    };

struct GameSettings { // KFX settings
    unsigned char field_0;
    unsigned char video_shadows;
    unsigned char view_distance;
    unsigned char video_rotate_mode;
    unsigned char video_textures;
    unsigned char video_cluedo_mode;
    unsigned char sound_volume;
    unsigned char redbook_volume;
    unsigned char roomflags_on;
    unsigned short gamma_correction;
    int video_scrnmode;
    struct GameKey kbkeys[GAME_KEYS_COUNT];
    unsigned char tooltips_on;
    unsigned char first_person_move_invert;
    unsigned char first_person_move_sensitivity;
    unsigned int minimap_zoom;
    unsigned long isometric_view_zoom_level;
    unsigned long frontview_zoom_level;
    };
#pragma pack()
/******************************************************************************/
DLLIMPORT extern struct _DK_GameSettings _DK_settings; // DK settings
extern struct GameSettings settings; // KFX settings
/******************************************************************************/
void copy_settings_to_dk_settings(void);
TbBool load_settings(void);
short save_settings(void);

int get_creature_can_see_subtiles(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
