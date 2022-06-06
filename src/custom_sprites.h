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
struct Objects;

#define SPRITE_LAST_LEVEL -1
void init_custom_sprites(LevelNumber level_no);

short get_anim_id(const char *name, struct Objects* objdat);
short get_icon_id(const char *name);
const struct TbSprite *get_button_sprite(short sprite_idx);
const struct TbSprite *get_frontend_sprite(short sprite_idx);
const struct TbSprite *get_new_icon_sprite(short sprite_idx);
int is_custom_icon(short icon_idx);

extern short bad_icon_id;
#ifdef __cplusplus
}
#endif

#endif //GIT_CUSTOM_SPRITES_H
