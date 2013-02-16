/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_creature_data.cpp
 *     Creatures related data structures.
 * @par Purpose:
 *     Support of configuration files for magic spells.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     21 Dec 2012 - 10 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_creature.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_filelst.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbLoadFiles swipe_load_file[] = {
  {"data/swpe??.dat", &swipe_sprite_data[0],               &end_swipe_sprite_data[0],               0, 0, 0},
  {"data/swpe??.tab", (unsigned char **)&swipe_sprites[0], (unsigned char **)&end_swipe_sprites[0], 0, 0, 0},
  {"data/swpe??.dat", &swipe_sprite_data[1],               &end_swipe_sprite_data[1],               0, 0, 0},
  {"data/swpe??.tab", (unsigned char **)&swipe_sprites[1], (unsigned char **)&end_swipe_sprites[1], 0, 0, 0},
  {"data/swpe??.dat", &swipe_sprite_data[2],               &end_swipe_sprite_data[2],               0, 0, 0},
  {"data/swpe??.tab", (unsigned char **)&swipe_sprites[2], (unsigned char **)&end_swipe_sprites[2], 0, 0, 0},
  {"data/swpe??.dat", &swipe_sprite_data[3],               &end_swipe_sprite_data[3],               0, 0, 0},
  {"data/swpe??.tab", (unsigned char **)&swipe_sprites[3], (unsigned char **)&end_swipe_sprites[3], 0, 0, 0},
  {"data/swpe??.dat", &swipe_sprite_data[4],               &end_swipe_sprite_data[4],               0, 0, 0},
  {"data/swpe??.tab", (unsigned char **)&swipe_sprites[4], (unsigned char **)&end_swipe_sprites[4], 0, 0, 0},
  {"",                NULL,                                NULL,                                    0, 0, 0},
};

struct TbSetupSprite swipe_setup_sprites[] = {
    {&swipe_sprites[0],  &end_swipe_sprites[0], (unsigned long *)&swipe_sprite_data[0]},
    {&swipe_sprites[1],  &end_swipe_sprites[1], (unsigned long *)&swipe_sprite_data[1]},
    {&swipe_sprites[2],  &end_swipe_sprites[2], (unsigned long *)&swipe_sprite_data[2]},
    {&swipe_sprites[3],  &end_swipe_sprites[3], (unsigned long *)&swipe_sprite_data[3]},
    {&swipe_sprites[4],  &end_swipe_sprites[4], (unsigned long *)&swipe_sprite_data[4]},
    {NULL,               NULL,                  NULL},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
