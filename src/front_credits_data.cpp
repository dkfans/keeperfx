/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_credits_data.cpp
 *     Credits and story screen displaying routines.
 * @par Purpose:
 *     Support of the credits and story screens.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     22 Dec 2014 - 01 May 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "front_credits.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_filelst.h"
#include "bflib_sprite.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef SPRITE_FORMAT_V2
struct TbLoadFiles frontstory_load_files_640[] = {
  {"ldata/front-64.raw", &frontstory_background,                 NULL,                                          0, 0, 0},
  {"ldata/frontft1-64.dat",&frontstory_font_data,                &frontstory_end_font_data,                     0, 0, 0},
  {"ldata/frontft1-64.tab",(unsigned char **)&frontstory_font,   (unsigned char **)&frontstory_end_font,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};
#else
struct TbLoadFiles frontstory_load_files_640[] = {
  {"ldata/front.raw",    &frontstory_background,                 NULL,                                          0, 0, 0},
  {"ldata/frontft1.dat", &frontstory_font_data,                  &frontstory_end_font_data,                     0, 0, 0},
  {"ldata/frontft1.tab", (unsigned char **)&frontstory_font,     (unsigned char **)&frontstory_end_font,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
