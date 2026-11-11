/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_textures.c
 *     Texture blocks support.
 * @par Purpose:
 *     Defines texture blocks, their initialization and loading.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     02 Apr 2010 - 02 May 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "engine_textures.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "engine_lenses.h"
#include "front_simple.h"
#include "config.h"
#include "game_legacy.h"
#include "filepaths.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned char block_mem[TEXTURE_VARIATIONS_COUNT * TEXTURE_BLOCKS_STAT_COUNT * 32 * 32];
unsigned char *block_ptrs[TEXTURE_VARIATIONS_COUNT * TEXTURE_BLOCKS_COUNT];

long block_dimension = 32;
long block_count_per_row = 8;
TbBool level_textures_ready = 0;
volatile unsigned int level_textures_generation = 0;

static int32_t anim_counter;
static int32_t anim_start_frame;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void setup_texture_block_mem(void)
{
    unsigned char** dst = block_ptrs;
    unsigned char* src  = block_mem;
    for (int i = 0; i < (TEXTURE_VARIATIONS_COUNT * TEXTURE_BLOCKS_COUNT); i++)
    {
        block_ptrs[i] = block_mem + block_dimension;
    }
    for (int f = 0; f < TEXTURE_VARIATIONS_COUNT; f++)
    {
        for (int i = 0; i < TEXTURE_BLOCKS_STAT_COUNT_A / block_count_per_row; i++)
        {
            for (unsigned long k = 0; k < block_count_per_row; k++)
            {
                *dst = src;
                src += block_dimension;
                dst++;
            }
            src += (block_dimension-1)*block_dimension*block_count_per_row;
        }
        dst += TEXTURE_BLOCKS_ANIM_COUNT;

        for (int i = 0; i < TEXTURE_BLOCKS_STAT_COUNT_B / block_count_per_row; i++)
        {
            for (unsigned long k = 0; k < block_count_per_row; k++)
            {
                *dst = src;
                src += block_dimension;
                dst++;
            }
            src += (block_dimension-1)*block_dimension*block_count_per_row;
        }

    }
}

short init_animating_texture_maps(void)
{
    SYNCDBG(8,"Starting");
    anim_start_frame = (get_gameturn() / 2) % TEXTURE_BLOCKS_ANIM_FRAMES;
    anim_counter = -1;
    return update_animating_texture_maps();
}

short update_animating_texture_maps(void)
{
  int32_t frame = (get_gameturn() / 2 + TEXTURE_BLOCKS_ANIM_FRAMES - anim_start_frame) % TEXTURE_BLOCKS_ANIM_FRAMES;
  if (frame == anim_counter) {
      return false;
  }
  SYNCDBG(18,"Starting");
  unsigned char** dst = block_ptrs;
  for (int f = 0; f < TEXTURE_VARIATIONS_COUNT; f++)
  {
      for (int i = 0; i < TEXTURE_BLOCKS_ANIM_COUNT; i++)
      {
          short j = game.texture_animation[TEXTURE_BLOCKS_ANIM_FRAMES*i+frame];
          if (((j>=0) && (j<TEXTURE_BLOCKS_STAT_COUNT_A)) ||
              ((j>=TEX_B_START_POINT) && (j<(TEX_B_START_POINT + TEXTURE_BLOCKS_STAT_COUNT_B))))
          {
            dst[TEXTURE_BLOCKS_STAT_COUNT_A + i] = dst[j];
          }
      }
      dst += TEXTURE_BLOCKS_COUNT;
  }
  anim_counter = frame;
  return true;
}


char *load_letter_one_file(char letter, unsigned long tmapidx, unsigned char *dst)
{
    SYNCDBG(9,"Starting");
    
    const char* fname = prepare_file_fmtpath_prio(FGrp_StdData, FGrp_CmpgConfig, true, "tmap%c%03d.dat", letter, tmapidx);
    if (!LbFileExists(fname))
    {
        SYNCDBG(10, "Texture file \"%s\" doesn't exist.",fname);
        return false;
    }

    if (LbFileLoadAt(fname, dst) < 1024)
    {
        WARNMSG("Texture file \"%s\" can't be loaded or is too small.",fname);
        return false;
    }
    SYNCDBG(6, "Texture file \"%s\" succesfully loaded.", fname);
    return true;
}

TbBool load_texture_map_file(unsigned long tmapidx)
{
    SYNCDBG(7,"Starting");
    memset(block_mem, 130, sizeof(block_mem));
    if (!load_letter_one_file('a', tmapidx, block_mem))
    {
        return false;
    }
    unsigned char *dst = block_mem + (TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32);
    load_letter_one_file('b', tmapidx, dst);
    dst += (TEXTURE_BLOCKS_STAT_COUNT_B * 32 * 32);

    for (int i = 0; i < TEXTURE_VARIATIONS_COUNT-1; i++)
    {
        load_letter_one_file('a', i, dst);

        dst += (TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32);
        load_letter_one_file('b', i, dst);
        dst += (TEXTURE_BLOCKS_STAT_COUNT_B * 32 * 32);

    }
    level_textures_ready = 1;
    level_textures_generation++;
    return true;
}
/******************************************************************************/
