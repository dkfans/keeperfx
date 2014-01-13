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
unsigned char * swipe_sprite_data;
struct TbSprite *swipe_sprites;
unsigned char * end_swipe_sprite_data;
struct TbSprite *end_swipe_sprites;

struct TbLoadFiles swipe_load_file[] = {
  {"data/swipe??.dat", &swipe_sprite_data,               &end_swipe_sprite_data,               0, 0, 0},
  {"data/swipe??.tab", (unsigned char **)&swipe_sprites, (unsigned char **)&end_swipe_sprites, 0, 0, 0},
  {"",                 NULL,                             NULL,                                 0, 0, 0},
};

struct TbSetupSprite swipe_setup_sprites[] = {
    {&swipe_sprites,  &end_swipe_sprites, (unsigned char **)&swipe_sprite_data},
    {NULL,            NULL,               NULL},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
