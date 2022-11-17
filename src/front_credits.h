/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_credits.h
 *     Header file for front_credits.c.
 * @par Purpose:
 *     Credits and story screen displaying routines.
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
#ifndef DK_FRONT_CREDITS_H
#define DK_FRONT_CREDITS_H

#include "globals.h"

#include "bflib_basics.h"
#include "bflib_sprite.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

DKEXTERN unsigned char * _DK_frontstory_background;
#define frontstory_background _DK_frontstory_background
DKEXTERN struct TbSprite *_DK_frontstory_font;
#define frontstory_font _DK_frontstory_font
DKEXTERN struct TbSprite *_DK_frontstory_end_font;
#define frontstory_end_font _DK_frontstory_end_font
DKEXTERN unsigned char * _DK_frontstory_font_data;
#define frontstory_font_data _DK_frontstory_font_data
DKEXTERN unsigned char * _DK_frontstory_end_font_data;
#define frontstory_end_font_data _DK_frontstory_end_font_data
DKEXTERN long _DK_frontstory_text_no;
#define frontstory_text_no _DK_frontstory_text_no
DKEXTERN struct TbSetupSprite _DK_frontstory_setup_sprites[2];
#define frontstory_setup_sprites _DK_frontstory_setup_sprites

DKEXTERN long _DK_credits_scroll_speed;
#define credits_scroll_speed _DK_credits_scroll_speed
DKEXTERN long _DK_credits_offset;
#define credits_offset _DK_credits_offset
DKEXTERN int _DK_credits_end;
#define credits_end _DK_credits_end

#pragma pack()
/******************************************************************************/
void frontstory_load(void);
void frontstory_unload(void);
void frontstory_draw(void);
short frontstory_input(void);

void frontcredits_draw(void);
TbBool frontcredits_input(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
