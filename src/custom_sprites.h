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
#define CUSTOM_ENSIGN_BASE 50

static const char * const required_sprite_zips[] = {
    "colored_sprites.zip",
    "creatures.zip",
    "decorative_objects.zip",
    "druid.zip",
    "effects.zip",
    "maiden.zip",
    "natural_features.zip",
    "time_mage.zip",
    "trapsdoors.zip",
};

#define REQUIRED_SPRITE_ZIP_COUNT (sizeof(required_sprite_zips) / sizeof(required_sprite_zips[0]))

void init_custom_sprites(LevelNumber level_no);
void init_custom_campaign_sprites(const char *dir_path, const char *dir_desc);
void show_ignored_fxdata_zip_messages(void);
void load_sprites_for_multi_front(LevelNumber lvnum);

extern TbBigChecksum required_sprite_zip_checksums[REQUIRED_SPRITE_ZIP_COUNT];

short get_anim_id(const char *name, struct ObjectConfigStats* objst);
short get_anim_id_(const char* name);
short get_icon_id(const char *name);
short get_ensign_id(const char *name);
struct TbSpriteSheet *load_custom_ensigns_into_sheet(struct TbSpriteSheet *sheet, const unsigned char *palette);
const struct TbSprite *get_custom_ensign_sprite(struct TbSpriteSheet *sheet, short ensign_id, int frame);
const struct TbSprite *get_button_sprite_for_player(short sprite_idx, PlayerNumber plyr_idx);
const struct TbSprite *get_button_sprite(short sprite_idx);
const struct TbSprite *get_frontend_sprite(short sprite_idx);
const struct TbSprite *get_new_icon_sprite(short sprite_idx);
const struct TbSprite *get_panel_sprite(short sprite_idx);
struct TbSpriteSheet *load_custom_sheet_from_zip(const char *path, const unsigned char *palette);
int is_custom_icon(short icon_idx);
int get_custom_icon_frame_count(short icon_idx);
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
