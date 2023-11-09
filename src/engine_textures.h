/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_textures.h
 *     Header file for engine_textures.c.
 * @par Purpose:
 *     Texture blocks support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     02 Apr 2010 - 02 May 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_ENGNTEXTR_H
#define DK_ENGNTEXTR_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Num of known texture files
#define TEXTURE_FILES_COUNT           16
#define TEXTURE_VARIATIONS_COUNT        16
// Static textures in tmapa
#define TEXTURE_BLOCKS_STAT_COUNT_A   544
// Static textures in tmapb
#define TEXTURE_BLOCKS_STAT_COUNT_B   544
#define TEXTURE_BLOCKS_STAT_COUNT   TEXTURE_BLOCKS_STAT_COUNT_A + TEXTURE_BLOCKS_STAT_COUNT_B
// Animated texture frames count
#define TEXTURE_BLOCKS_ANIM_FRAMES    8
// Animated textures amount
#define TEXTURE_BLOCKS_ANIM_COUNT    1000 - TEXTURE_BLOCKS_STAT_COUNT_A
#define TEXTURE_BLOCKS_COUNT         (TEXTURE_BLOCKS_STAT_COUNT_A + TEXTURE_BLOCKS_ANIM_COUNT)

#define  TEXTURE_LAND_MARKED_LAND     578
#define  TEXTURE_LAND_MARKED_GOLD     579
/******************************************************************************/

extern unsigned char block_mem[TEXTURE_FILES_COUNT * TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32];
extern unsigned char *block_ptrs[TEXTURE_FILES_COUNT * TEXTURE_BLOCKS_COUNT];
extern long block_dimension;
/******************************************************************************/
void setup_texture_block_mem(void);
short init_animating_texture_maps(void);
short update_animating_texture_maps(void);
TbBool load_texture_map_file(unsigned long tmapidx);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
