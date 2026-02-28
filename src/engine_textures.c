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
          if (((j>=0) && (j<TEXTURE_BLOCKS_STAT_COUNT_A)) ||
              ((j>=TEX_B_START_POINT) && (j<(TEX_B_START_POINT + TEXTURE_BLOCKS_STAT_COUNT_B))))
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

static char *prepare_letter_one_file_path_for_mod_one(unsigned long tmapidx, char letter, LevelNumber lvnum, short fgroup, const struct ModConfigItem *mod_item)
{
    // Note that this is the reverse direction

    const struct ModExistState *mod_state = &mod_item->state;
    char* fname = NULL;
    char mod_dir[256] = {0};
    sprintf(mod_dir, "%s/%s", MODS_DIR_NAME, mod_item->name);

    if (mod_state->cmpg_lvls)
    {
        fname = prepare_file_fmtpath_mod(mod_dir, FGrp_CmpgLvls, "map%05lu.tmap%c%03d.dat", (unsigned long)lvnum, letter, tmapidx);
        if (fname[0] != 0 && LbFileExists(fname))
            return fname;
    }

    if (mod_state->cmpg_config)
    {
        fname = prepare_file_fmtpath_mod(mod_dir, FGrp_CmpgConfig, "tmap%c%03d.dat", letter, tmapidx);
        if (fname[0] != 0 && LbFileExists(fname))
            return fname;
    }

    if (mod_state->std_data)
    {
        fname = prepare_file_fmtpath_mod(mod_dir, FGrp_StdData, "tmap%c%03d.dat", letter, tmapidx);
        if (fname[0] != 0 && LbFileExists(fname))
            return fname;
    }

    return NULL;
}

static char *prepare_letter_one_file_path_for_mod_list(unsigned long tmapidx, char letter, LevelNumber lvnum, short fgroup, const struct ModConfigItem *mod_items, long mod_cnt)
{
    // Note that this is the reverse direction
    for (long i=mod_cnt-1; i>=0; i--)
    {
        const struct ModConfigItem *mod_item = mod_items + i;
        if (mod_item->state.mod_dir == 0)
            continue;

        char *fname = prepare_letter_one_file_path_for_mod_one(tmapidx, letter, fgroup, lvnum, mod_item);
        if (fname != NULL)
            return fname;
    }

    return NULL;
}

static char *prepare_letter_one_file_path(unsigned long tmapidx, char letter, LevelNumber lvnum, short fgroup)
{
    char* fname = NULL;
    if (mods_conf.after_map_cnt > 0)
    {
        fname = prepare_letter_one_file_path_for_mod_list(tmapidx, letter, lvnum, fgroup, mods_conf.after_map_item, mods_conf.after_map_cnt);
        if (fname != NULL)
            return fname;
    }

    fname = prepare_file_fmtpath(fgroup, "map%05lu.tmap%c%03d.dat",(unsigned long)lvnum, letter, tmapidx);
    if (LbFileExists(fname))
        return fname;

    if (mods_conf.after_campaign_cnt > 0)
    {
        fname = prepare_letter_one_file_path_for_mod_list(tmapidx, letter, lvnum, fgroup, mods_conf.after_campaign_item, mods_conf.after_campaign_cnt);
        if (fname != NULL)
            return fname;
    }

    fname = prepare_file_fmtpath(FGrp_CmpgConfig, "tmap%c%03d.dat", letter, tmapidx);
    if (LbFileExists(fname))
        return fname;

    if (mods_conf.after_base_cnt > 0)
    {
        fname = prepare_letter_one_file_path_for_mod_list(tmapidx, letter, lvnum, fgroup, mods_conf.after_base_item, mods_conf.after_base_cnt);
        if (fname != NULL)
            return fname;
    }

    fname = prepare_file_fmtpath(FGrp_StdData, "tmap%c%03d.dat", letter, tmapidx);
    return fname;
}

static TbBool load_letter_one_file(unsigned long tmapidx, char letter, void *dst, LevelNumber lvnum, short fgroup)
{
    SYNCDBG(9,"Starting");

    char* fname = prepare_letter_one_file_path(tmapidx, letter, lvnum, fgroup);
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

TbBool load_texture_map_file(unsigned long tmapidx, LevelNumber lvnum, short fgroup)
{
    SYNCDBG(7,"Starting");
    memset(block_mem, 130, sizeof(block_mem));
    if (!load_letter_one_file(tmapidx,'a', block_mem,lvnum,fgroup))
    {
        return false;
    }
    unsigned char *dst = block_mem + (TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32);
    load_letter_one_file(tmapidx,'b', dst, lvnum, fgroup);
    dst += (TEXTURE_BLOCKS_STAT_COUNT_B * 32 * 32);

    for (int i = 0; i < TEXTURE_VARIATIONS_COUNT-1; i++)

    {
        load_letter_one_file(i,'a', dst, lvnum, fgroup);

        dst += (TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32);
        load_letter_one_file(i,'b', dst, lvnum, fgroup);
        dst += (TEXTURE_BLOCKS_STAT_COUNT_B * 32 * 32);

    }
    return true;
}
/******************************************************************************/
