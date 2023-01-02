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
  {102,  13, 102,  20,  97, 155, 0, 0, 0, 0, 200},
  {253,   0, 257,   0, 103, 118, 0, 0, 0, 0, 201},
  {399,   0, 413,   0, 114, 144, 0, 0, 0, 0, 202},
  {511,  65, 546,  85,  94, 160, 0, 0, 0, 0, 203},
  {149, 211, 153, 232,  55,  84, 0, 0, 0, 0, 204},
  {258, 176, 262, 178,  60,  84, 0, 0, 0, 0, 205},
  {364, 183, 375, 191,  70,  95, 0, 0, 0, 0, 206},
  {466, 257, 473, 261,  67,  94, 0, 0, 0, 0, 207},
  {254, 368, 260, 391, 128,  80, 0, 0, 0, 0, 208},
};
struct TbSprite *fronttor_sprites;
struct TbSprite *fronttor_end_sprites;
unsigned char *fronttor_data;
unsigned char * fronttor_end_data;

/******************************************************************************/
struct TbLoadFiles torture_load_files[] = {
  {"ldata/fronttor.tab", (unsigned char **)&fronttor_sprites, (unsigned char **)&fronttor_end_sprites, 0, 0, 0},
  {"ldata/fronttor.dat", (unsigned char **)&fronttor_data,    (unsigned char **)&fronttor_end_data,    0, 0, 0},
  {"",                    NULL,                                NULL,                                   0, 0, 0},
};

struct TbSetupSprite setup_torture_sprites[] = {
  {&doors[0].sprites, &doors[0].sprites_end, &doors[0].data},
  {&doors[1].sprites, &doors[1].sprites_end, &doors[1].data},
  {&doors[2].sprites, &doors[2].sprites_end, &doors[2].data},
  {&doors[3].sprites, &doors[3].sprites_end, &doors[3].data},
  {&doors[4].sprites, &doors[4].sprites_end, &doors[4].data},
  {&doors[5].sprites, &doors[5].sprites_end, &doors[5].data},
  {&doors[6].sprites, &doors[6].sprites_end, &doors[6].data},
  {&doors[7].sprites, &doors[7].sprites_end, &doors[7].data},
  {&doors[8].sprites, &doors[8].sprites_end, &doors[8].data},
  {&fronttor_sprites, &fronttor_end_sprites, &fronttor_data},
  {NULL,              NULL,                  NULL,}
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
