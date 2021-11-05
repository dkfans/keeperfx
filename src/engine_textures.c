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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
unsigned char block_mem[TEXTURE_FILES_COUNT * TEXTURE_BLOCKS_STAT_COUNT * 32 * 32];
unsigned char *block_ptrs[TEXTURE_FILES_COUNT * TEXTURE_BLOCKS_COUNT];
unsigned char slab_ext_data[85 * 85];

long block_dimension = 32;
long block_count_per_row = 8;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void setup_texture_block_mem(void)
{
    unsigned char** dst = block_ptrs;
    unsigned char* src  = block_mem;
    for (int i = 0; i < (TEXTURE_FILES_COUNT * TEXTURE_BLOCKS_COUNT); i++)
    {
        block_ptrs[i] = block_mem + block_dimension;
    }
    for (int f = 0; f < TEXTURE_FILES_COUNT; f++)
    {
        for (int i = 0; i < TEXTURE_BLOCKS_STAT_COUNT / block_count_per_row; i++)
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
    }
}

short init_animating_texture_maps(void)
{
    SYNCDBG(8,"Starting");
    //_DK_init_animating_texture_maps(); return;
    anim_counter = TEXTURE_BLOCKS_ANIM_FRAMES-1;
    return update_animating_texture_maps();
}

short update_animating_texture_maps(void)
{
  SYNCDBG(18,"Starting");
  unsigned char** dst = block_ptrs;
  short result=true;

  anim_counter = (anim_counter+1) % TEXTURE_BLOCKS_ANIM_FRAMES;
  for (int f = 0; f < TEXTURE_FILES_COUNT; f++)
  {
      for (int i = 0; i < TEXTURE_BLOCKS_ANIM_COUNT; i++)
      {
          short j = game.texture_animation[TEXTURE_BLOCKS_ANIM_FRAMES*i+anim_counter];
          if ((j>=0) && (j<TEXTURE_BLOCKS_STAT_COUNT))
          {
            dst[TEXTURE_BLOCKS_STAT_COUNT + i] = dst[j];
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

long load_texture_anim_file(void)
{
    SYNCDBG(8,"Starting");
    //return _DK_load_anim_file();
    char* fname = prepare_file_path(FGrp_StdData, "tmapanim.dat");
    SYNCDBG(0,"Reading animated tmap file \"%s\".",fname);
    if (LbFileLoadAt(fname, game.texture_animation) != sizeof(game.texture_animation))
    {
        return false;
    }
    return true;
}

static TbBool load_one_file(unsigned long tmapidx, void *dst)
{
    SYNCDBG(9,"Starting");
#ifdef SPRITE_FORMAT_V2
    fname = prepare_file_fmtpath(FGrp_StdData,"tmapa%03d-%d.dat",tmapidx,32);
#else
    char* fname = prepare_file_fmtpath(FGrp_StdData, "tmapa%03d.dat", tmapidx);
#endif
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
TbBool load_texture_map_file(unsigned long tmapidx, unsigned char n)
{
    SYNCDBG(7,"Starting");
    memset(block_mem, 130, sizeof(block_mem));
    if (!load_one_file(tmapidx, block_mem))
    {
        return false;
    }
    unsigned char *dst = block_mem + (TEXTURE_BLOCKS_STAT_COUNT * 32 * 32);
    for (int i = 0; i < TEXTURE_FILES_COUNT-1; i++, dst += (TEXTURE_BLOCKS_STAT_COUNT * 32 * 32))
    {
        if (!load_one_file(i, dst))
        {
            continue;
        }
    }
    return true;
}
/******************************************************************************/
