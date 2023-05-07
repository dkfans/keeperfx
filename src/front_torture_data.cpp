/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_torture_data.cpp
 *     Torture screen displaying data structures.
 * @par Purpose:
 *     Support of the bonus torture screen.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     22 Dec 2012 - 10 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "front_torture.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_filelst.h"
#include "bflib_sprite.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

struct DoorDesc doors[TORTURE_DOORS_COUNT] = {
  {102,  13, 102,  20,  97, 155, 200, NULL},
  {253,   0, 257,   0, 103, 118, 201, NULL},
  {399,   0, 413,   0, 114, 144, 202, NULL},
  {511,  65, 546,  85,  94, 160, 203, NULL},
  {149, 211, 153, 232,  55,  84, 204, NULL},
  {258, 176, 262, 178,  60,  84, 205, NULL},
  {364, 183, 375, 191,  70,  95, 206, NULL},
  {466, 257, 473, 261,  67,  94, 207, NULL},
  {254, 368, 260, 391, 128,  80, 208, NULL},
};
struct SpriteSheet * fronttor_sprites;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
