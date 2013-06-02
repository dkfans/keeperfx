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

#define COLOUR_TABLE_BITS_PER_VALUE 4
#define COLOUR_TABLE_DIMENSION (1<<COLOUR_TABLE_BITS_PER_VALUE)

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct TbColorTables;
struct TbAlphaTables;
struct PlayerInfo;
typedef unsigned char TbRGBColorTable[COLOUR_TABLE_DIMENSION][COLOUR_TABLE_DIMENSION][COLOUR_TABLE_DIMENSION];

/******************************************************************************/
DLLIMPORT extern unsigned char _DK_fade_palette_in;
#define fade_palette_in _DK_fade_palette_in
DLLIMPORT extern unsigned char _DK_frontend_palette[768];
#define frontend_palette _DK_frontend_palette
DLLIMPORT TbRGBColorTable _DK_colours;
#define colours _DK_colours

#pragma pack()
/******************************************************************************/
void fade_in(void);
void fade_out(void);
void compute_fade_tables(struct TbColorTables *coltbl,unsigned char *spal,unsigned char *dpal);
void ProperFadePalette(unsigned char *pal, long fade_steps, enum TbPaletteFadeFlag flg);
void ProperForcedFadePalette(unsigned char *pal, long n, enum TbPaletteFadeFlag flg);

void compute_alpha_tables(struct TbAlphaTables *alphtbls,unsigned char *spal,unsigned char *dpal);
void compute_rgb2idx_table(TbRGBColorTable ctab,unsigned char *spal);
void compute_shifted_palette_table(TbPixel *ocol, const unsigned char *spal,
    const unsigned char *dpal, int shiftR, int shiftG, int shiftB);


long PaletteFadePlayer(struct PlayerInfo *player);
void PaletteApplyPainToPlayer(struct PlayerInfo *player, long intense);
void PaletteClearPainFromPlayer(struct PlayerInfo *player);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
