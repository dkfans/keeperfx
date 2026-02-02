/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_heap.c
 *     Definition of heap, used for storing memory-expensive sounds and graphics.
 * @par Purpose:
 *     Functions to create and maintain memory heap.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     06 Apr 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef GIT_CUSTOM_SPRITES_H
#define GIT_CUSTOM_SPRITES_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
struct ObjectConfigStats;

#define SPRITE_LAST_LEVEL -1
void init_custom_sprites(LevelNumber level_no);

extern int total_sprite_zip_count;

short get_anim_id(const char *name, struct ObjectConfigStats* objst);
short get_anim_id_(const char* name);
short get_icon_id(const char *name);
const struct TbSprite *get_button_sprite_for_player(short sprite_idx, PlayerNumber plyr_idx);
const struct TbSprite *get_button_sprite(short sprite_idx);
const struct TbSprite *get_frontend_sprite(short sprite_idx);
const struct TbSprite *get_new_icon_sprite(short sprite_idx);
const struct TbSprite *get_panel_sprite(short sprite_idx);
int is_custom_icon(short icon_idx);

// Lens overlay data structure
struct LensOverlayData {
    char *name;
    unsigned char *data;
    int width;
    int height;
};

// Lens mist data structure
struct LensMistData {
    char *name;
    unsigned char *data;  // 256x256 mist texture
};

// Get lens overlay data by name (returns NULL if not found)
const struct LensOverlayData* get_lens_overlay_data(const char *name);

// Get lens mist data by name (returns NULL if not found)
const struct LensMistData* get_lens_mist_data(const char *name);

extern short bad_icon_id;
#ifdef __cplusplus
}
#endif

#endif //GIT_CUSTOM_SPRITES_H
