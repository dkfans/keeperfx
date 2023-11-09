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
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "engine_lenses.h"
#include "front_simple.h"
#include "config.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned char block_mem[TEXTURE_VARIATIONS_COUNT * TEXTURE_BLOCKS_STAT_COUNT * 32 * 32];
unsigned char *block_ptrs[TEXTURE_VARIATIONS_COUNT * TEXTURE_BLOCKS_COUNT];

long block_dimension = 32;
long block_count_per_row = 8;

static long anim_counter;
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
        /*
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
        */
    }
}

short init_animating_texture_maps(void)
{
    SYNCDBG(8,"Starting");
    anim_counter = TEXTURE_BLOCKS_ANIM_FRAMES-1;
    return update_animating_texture_maps();
}

short update_animating_texture_maps(void)
{
  SYNCDBG(18,"Starting");
  unsigned char** dst = block_ptrs;
  short result=true;

  anim_counter = (anim_counter+1) % TEXTURE_BLOCKS_ANIM_FRAMES;
  for (int f = 0; f < TEXTURE_VARIATIONS_COUNT; f++)
  {
      for (int i = 0; i < TEXTURE_BLOCKS_ANIM_COUNT; i++)
      {
          short j = game.texture_animation[TEXTURE_BLOCKS_ANIM_FRAMES*i+anim_counter];
          if ((j>=0) && (j<TEXTURE_BLOCKS_STAT_COUNT_A))
          {
            dst[TEXTURE_BLOCKS_STAT_COUNT_A + i] = dst[j];
          }
          else
          {
            result=false;
          }
      }
      dst += TEXTURE_BLOCKS_COUNT;
  }
  return result;
}

static TbBool load_one_file(unsigned long tmapidx,char letter, void *dst)
{
    SYNCDBG(9,"Starting");

    char* fname = prepare_file_fmtpath(FGrp_CmpgConfig, "tmap%c%03d.dat",letter, tmapidx);
    if (!LbFileExists(fname))
    {
        fname = prepare_file_fmtpath(FGrp_StdData, "tmap%c%03d.dat",letter, tmapidx);
    }

    if (!wait_for_cd_to_be_available())
        return false;
    if (!LbFileExists(fname))
    {
        WARNMSG("Texture file \"%s\" doesn't exist.",fname);
        return false;
    }
    // The texture file has always over 500kb
    if (LbFileLoadAt(fname, dst) < 65536)
    {
        WARNMSG("Texture file \"%s\" can't be loaded or is too small.",fname);
        return false;
    }
    return true;
}
TbBool load_texture_map_file(unsigned long tmapidx)
{
    SYNCDBG(7,"Starting");
    memset(block_mem, 130, sizeof(block_mem));
    if (!load_one_file(tmapidx,'a', block_mem))
    {
        return false;
    }
    unsigned char *dst = block_mem + (TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32);
    for (int i = 0; i < TEXTURE_VARIATIONS_COUNT-1; i++, dst += (TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32))
    /*
    load_one_file(tmapidx,'b', dst);
    dst += (TEXTURE_BLOCKS_STAT_COUNT_B * 32 * 32);

    for (int i = 0; i < TEXTURE_VARIATIONS_COUNT-1; i++)
    */
    {
        load_one_file(i,'a', dst);
        /*
        dst += (TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32);
        load_one_file(i,'b', dst);
        dst += (TEXTURE_BLOCKS_STAT_COUNT_B * 32 * 32);
        */
    }
    return true;
}
/******************************************************************************/
