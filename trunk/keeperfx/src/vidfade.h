/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidfade.h
 *     Header file for vidfade.c.
 * @par Purpose:
 *     Video fading routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     16 Jul 2010 - 05 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_VIDFADE_H
#define DK_VIDFADE_H

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct TbColorTables;
struct PlayerInfo;

#pragma pack()
/******************************************************************************/
DLLIMPORT extern unsigned char _DK_fade_palette_in;
#define fade_palette_in _DK_fade_palette_in
DLLIMPORT extern unsigned char _DK_frontend_palette[768];
#define frontend_palette _DK_frontend_palette
/******************************************************************************/
void fade_in(void);
void fade_out(void);
void compute_fade_tables(struct TbColorTables *coltbl,unsigned char *spal,unsigned char *dpal);
void ProperFadePalette(unsigned char *pal, long fade_steps, enum TbPaletteFadeFlag flg);
void ProperForcedFadePalette(unsigned char *pal, long n, enum TbPaletteFadeFlag flg);

long PaletteFadePlayer(struct PlayerInfo *player);
void PaletteApplyPainToPlayer(struct PlayerInfo *player, long intense);
void PaletteClearPainFromPlayer(struct PlayerInfo *player);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
