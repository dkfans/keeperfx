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
  long field_0;
  long field_4;
};

struct DoorDesc { // sizeof = 44
  long pos_spr_x;
  long pos_spr_y;
  long pos_x;
  long pos_y;
  long width;
  long height;
  struct TbSprite *sprites;
  struct TbSprite *sprites_end;
  unsigned char *data;
  unsigned char *data_end;
  long field_28;
};

struct TortureState { // sizeof = 4
  long action;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT extern struct DoorDesc _DK_doors[TORTURE_DOORS_COUNT];
#define doors _DK_doors
DLLIMPORT extern struct TbSprite *_DK_fronttor_sprites;
#define fronttor_sprites _DK_fronttor_sprites
DLLIMPORT extern struct TbSprite *_DK_fronttor_end_sprites;
#define fronttor_end_sprites _DK_fronttor_end_sprites
DLLIMPORT extern unsigned char *_DK_fronttor_data;
#define fronttor_data _DK_fronttor_data
DLLIMPORT extern unsigned char * _DK_fronttor_end_data;
#define fronttor_end_data _DK_fronttor_end_data
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
