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
#include "kfx/modding/mod_api.h"
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

/* -----------------------------------------------------------------------
 * Unified texture find-first via kfx/modding walker
 * --------------------------------------------------------------------- */

/**
 * Per-map tier locations (CmpgLvls): map%05lu.tmap<letter><idx>.dat
 * Both the PerMap base and AfterMap mods are in this walker.
 */
static const ModLocation s_texture_permap_locs[] = {
    { ModTier_PerMap,   (short)FGrp_CmpgLvls, ModLifetime_Level, SIZE_MAX,                                  ModRes_File, NULL },
    { ModTier_AfterMap, (short)FGrp_CmpgLvls, ModLifetime_Level, offsetof(struct ModExistState, cmpg_lvls), ModRes_File, NULL },
};

/**
 * Campaign / base tier locations: tmap<letter><idx>.dat
 * Check after_campaign mods and campaign first, then after_base mods and base StdData.
 */
static const ModLocation s_texture_base_locs[] = {
    { ModTier_Base,          (short)FGrp_StdData,    ModLifetime_Startup,  SIZE_MAX,                                       ModRes_File, NULL },
    { ModTier_AfterBase,     (short)FGrp_StdData,    ModLifetime_Startup,  offsetof(struct ModExistState, std_data),        ModRes_File, NULL },
    { ModTier_Campaign,      (short)FGrp_CmpgConfig, ModLifetime_Campaign, SIZE_MAX,                                       ModRes_File, NULL },
    { ModTier_AfterCampaign, (short)FGrp_CmpgConfig, ModLifetime_Campaign, offsetof(struct ModExistState, cmpg_config),     ModRes_File, NULL },
};

static KfxModHandle s_texture_permap_walker = NULL;
static KfxModHandle s_texture_base_walker   = NULL;

static char *prepare_letter_one_file_path(unsigned long tmapidx, char letter, LevelNumber lvnum, short fgroup)
{
    static char out_path[512];

    /* Lazy init */
    if (s_texture_permap_walker == NULL)
        s_texture_permap_walker = kfx_mod_create_walker(s_texture_permap_locs,
            sizeof(s_texture_permap_locs) / sizeof(s_texture_permap_locs[0]));
    if (s_texture_base_walker == NULL)
        s_texture_base_walker = kfx_mod_create_walker(s_texture_base_locs,
            sizeof(s_texture_base_locs) / sizeof(s_texture_base_locs[0]));

    /* Per-map name: highest priority (after_map mods > per-map base) */
    char map_fname[80];
    snprintf(map_fname, sizeof(map_fname), "map%05lu.tmap%c%03d.dat", (unsigned long)lvnum, letter, (int)tmapidx);
    if (kfx_mod_find(s_texture_permap_walker, map_fname, out_path, sizeof(out_path)))
        return out_path;

    /* Campaign/base name: (after_campaign mods > campaign > after_base mods > base) */
    char base_fname[40];
    snprintf(base_fname, sizeof(base_fname), "tmap%c%03d.dat", letter, (int)tmapidx);
    if (kfx_mod_find(s_texture_base_walker, base_fname, out_path, sizeof(out_path)))
        return out_path;

    /* Nothing found — return empty path so caller can detect failure */
    out_path[0] = '\0';
    return out_path;
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
