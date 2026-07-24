/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_torture.h
 *     Header file for front_torture.c.
 * @par Purpose:
 *     Torture screen displaying routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONT_TORTURE_H
#define DK_FRONT_TORTURE_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TORTURE_DOORS_COUNT 9
/******************************************************************************/
#pragma pack(1)

struct DoorSoundState { // sizeof = 8
  int32_t current_volume;
  int32_t volume_step; // how much to add / subtract
};

struct DoorDesc {
  int32_t pos_spr_x;
  int32_t pos_spr_y;
  int32_t pos_x;
  int32_t pos_y;
  int32_t width;
  int32_t height;
  struct TbSpriteSheet * sprites;
  int32_t smptbl_id;
};

struct TortureState { // sizeof = 4
  int32_t action;
};

#pragma pack()
/******************************************************************************/
void fronttorture_unload(void);
void fronttorture_load(void);
void fronttorture_clear_state(void);
void fronttorture_input(void);
TbBool fronttorture_draw(void);
void fronttorture_update(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
