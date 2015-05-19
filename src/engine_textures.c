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
DLLIMPORT long _DK_load_texture_map_file(unsigned long lv_num, unsigned char n);
DLLIMPORT void _DK_init_animating_texture_maps(void);
DLLIMPORT long _DK_load_anim_file(void);
/******************************************************************************/
long block_dimension = 32;
long block_count_per_row = 8;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void setup_texture_block_mem(void)
{
    unsigned char **dst;
    unsigned char *src;
    unsigned long i,k,n;
    dst = block_ptrs;
    n = 0;
    for (i=0; i < TEXTURE_BLOCKS_STAT_COUNT/block_count_per_row; i++)
    {
        src = block_mem + n;
        for (k=0; k < block_count_per_row; k++)
        {
            *dst = src;
            src += block_dimension;
            dst++;
        }
        n += block_dimension*block_dimension*block_count_per_row;
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
  int i;
  SYNCDBG(18,"Starting");
  anim_counter = (anim_counter+1) % TEXTURE_BLOCKS_ANIM_FRAMES;
  short result=true;
  for (i=0; i<TEXTURE_BLOCKS_ANIM_COUNT; i++)
  {
        short j = game.texture_animation[TEXTURE_BLOCKS_ANIM_FRAMES*i+anim_counter];
        if ((j>=0) && (j<TEXTURE_BLOCKS_STAT_COUNT))
        {
          block_ptrs[TEXTURE_BLOCKS_STAT_COUNT+i] = block_ptrs[j];
        } else
        {
          result=false;
        }
  }
  return result;
}

long load_texture_anim_file(void)
{
    SYNCDBG(8,"Starting");
    //return _DK_load_anim_file();
    char *fname;
    static const char textname[] = "animated tmap";
    fname = prepare_file_path(FGrp_StdData,"tmapanim.dat");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    if (LbFileLoadAt(fname, game.texture_animation) != sizeof(game.texture_animation))
    {
        return false;
    }
    return true;
}

TbBool load_texture_map_file(unsigned long tmapidx, unsigned char n)
{
    char *fname;
    SYNCDBG(7,"Starting");
#ifdef SPRITE_FORMAT_V2
    fname = prepare_file_fmtpath(FGrp_StdData,"tmapa%03d-%d.dat",tmapidx,32);
#else
    fname = prepare_file_fmtpath(FGrp_StdData,"tmapa%03d.dat",tmapidx);
#endif
    if (!wait_for_cd_to_be_available())
        return false;
    if (!LbFileExists(fname))
    {
        WARNMSG("Texture file \"%s\" doesn't exits.",fname);
        return false;
    }
    // The texture file has always over 500kb
    if (LbFileLoadAt(fname, block_mem) < 65536)
    {
        WARNMSG("Texture file \"%s\" can't be loaded or is too small.",fname);
        return false;
    }
    return true;
}
/******************************************************************************/
