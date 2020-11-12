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
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define  TEXTURE_LAND_MARKED_LAND     578
#define  TEXTURE_LAND_MARKED_GOLD     579
/******************************************************************************/
DLLIMPORT unsigned char *_DK_block_mem;
#define block_mem _DK_block_mem
DLLIMPORT unsigned char *_DK_block_ptrs[TEXTURE_BLOCKS_COUNT];
#define block_ptrs _DK_block_ptrs
DLLIMPORT long _DK_anim_counter;
#define anim_counter _DK_anim_counter
/******************************************************************************/
extern long block_dimension;
/******************************************************************************/
void setup_texture_block_mem(void);
short init_animating_texture_maps(void);
short update_animating_texture_maps(void);
TbBool load_texture_map_file(unsigned long tmapidx, unsigned char n);
long load_texture_anim_file(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
