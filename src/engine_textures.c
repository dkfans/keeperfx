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
#include "dungeon_data.h"
#include "kfx/tilesets/TileSetScan.h"
#include "kfx/tilesets/TileSetLoader.h"
#include "kfx/tilesets/TileSetsLog.h"
#include "kfx/renderer/RendererManager.h"
#include "kfx/profiling/KfxProfilingC.h"
#include <stdlib.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
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
#if TEXTURE_VARIATIONS_COUNT != TILE_SET_COUNT
#error "TILE_SET_COUNT must match TEXTURE_VARIATIONS_COUNT"
#endif
_Static_assert(TILE_SET_LETTER_BYTES == TEXTURE_BLOCKS_STAT_COUNT_A * 32 * 32, "letter size");
_Static_assert(TILE_SET_BYTES == TEXTURE_BLOCKS_STAT_COUNT * 32 * 32, "set size");

struct TextureSetSlot {
    unsigned char *buffer;  // own tile data; null for absent sets and the alias
    unsigned char state;    // enum TileSetState
    TbBool alias_of_zero;   // same pack as set 0; shares its buffer
    uint32_t generation;    // bumped whenever the tiles this set shows change
};

static struct TextureSetSlot texture_sets[TEXTURE_VARIATIONS_COUNT];
static uint32_t texture_sets_level_serial;
static uint32_t texture_sets_files_read;
static uint32_t texture_sets_loaded_in_play;
static TbBool texture_sets_loading_level;
static TbBool texture_sets_accepting_requests;
static unsigned long texture_set0_pack;

static void point_set_at(int32_t set, unsigned char *buf)
{
    unsigned char **dst = &block_ptrs[set * TEXTURE_BLOCKS_COUNT];
    for (int i = 0; i < TEXTURE_BLOCKS_COUNT; i++)
        dst[i] = buf + block_dimension;
    unsigned char *src = buf;
    for (int i = 0; i < TEXTURE_BLOCKS_STAT_COUNT_A / block_count_per_row; i++)
    {
        for (long k = 0; k < block_count_per_row; k++)
        {
            *dst++ = src;
            src += block_dimension;
        }
        src += (block_dimension - 1) * block_dimension * block_count_per_row;
    }
    dst += TEXTURE_BLOCKS_ANIM_COUNT;
    for (int i = 0; i < TEXTURE_BLOCKS_STAT_COUNT_B / block_count_per_row; i++)
    {
        for (long k = 0; k < block_count_per_row; k++)
        {
            *dst++ = src;
            src += block_dimension;
        }
        src += (block_dimension - 1) * block_dimension * block_count_per_row;
    }
}

static void refresh_animated_slots(int32_t set, int32_t frame)
{
    unsigned char **dst = &block_ptrs[set * TEXTURE_BLOCKS_COUNT];
    for (int i = 0; i < TEXTURE_BLOCKS_ANIM_COUNT; i++)
    {
        short j = game.texture_animation[TEXTURE_BLOCKS_ANIM_FRAMES * i + frame];
        if (((j >= 0) && (j < TEXTURE_BLOCKS_STAT_COUNT_A)) ||
            ((j >= TEX_B_START_POINT) && (j < (TEX_B_START_POINT + TEXTURE_BLOCKS_STAT_COUNT_B))))
        {
            dst[TEXTURE_BLOCKS_STAT_COUNT_A + i] = dst[j];
        }
    }
}

/** Points a set at buf, re-applies the current animation frame and marks the set changed. */
static void show_set(int32_t set, unsigned char *buf)
{
    point_set_at(set, buf);
    if (anim_counter >= 0)
        refresh_animated_slots(set, anim_counter);
    texture_sets[set].generation++;
}

void texture_sets_init(void)
{
    texture_sets[0].buffer = (unsigned char *)malloc(TILE_SET_BYTES);
    if (texture_sets[0].buffer == NULL)
    {
        ERRORLOG("Can't allocate the level tile set (%u bytes)", (unsigned)TILE_SET_BYTES);
        return;
    }
    memset(texture_sets[0].buffer, TILE_SET_FILL_COLOUR, TILE_SET_BYTES);
    texture_sets[0].state = TileSet_Resident;
    for (int32_t v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
        point_set_at(v, texture_sets[0].buffer);
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
  for (int32_t f = 0; f < TEXTURE_VARIATIONS_COUNT; f++)
      refresh_animated_slots(f, frame);
  anim_counter = frame;
  return true;
}

static char *prepare_letter_one_file_path_for_mod(unsigned long tmapidx, char letter, LevelNumber lvnum, short fgroup, const struct ModConfigItem *mod_item)
{
    // Note that this is the reverse mods direction

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
    // Note that this is the reverse mods direction
    for (long i=mod_cnt-1; i>=0; i--)
    {
        const struct ModConfigItem *mod_item = mod_items + i;
        if (mod_item->state.mod_dir == 0)
            continue;

        char *fname = prepare_letter_one_file_path_for_mod(tmapidx, letter, fgroup, lvnum, mod_item);
        if (fname != NULL)
            return fname;
    }

    return NULL;
}

static char *prepare_letter_one_file_path(unsigned long tmapidx, char letter, LevelNumber lvnum, short fgroup)
{
    // Note that this is the reverse mods direction
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

static int32_t load_letter_one_file(unsigned long tmapidx, char letter, void *dst, LevelNumber lvnum, short fgroup)
{
    char *fname = prepare_letter_one_file_path(tmapidx, letter, lvnum, fgroup);
    int32_t result = (int32_t)LbFileLoadAtMax(fname, dst, TILE_SET_LETTER_BYTES);
    if (result >= 0)
        texture_sets_files_read++;
    else if (result == LbLoadAtMax_TooLarge)
        TILESETWARN("A tile set file is larger than a tile set and was not loaded. (%s, limit %u bytes)", fname, (unsigned)TILE_SET_LETTER_BYTES);
    else if (result != LbLoadAtMax_Missing)
        TILESETWARN("A tile set file could not be read. (%s, code %d)", fname, (int)result);
    return result;
}

static void set_alias(int32_t new_alias)
{
    for (int32_t v = 1; v < TEXTURE_VARIATIONS_COUNT; v++)
    {
        struct TextureSetSlot *slot = &texture_sets[v];
        if (slot->alias_of_zero && v != new_alias)
        {
            slot->alias_of_zero = false;
            slot->state = TileSet_Absent;
            slot->generation++;
        }
    }
    if (new_alias <= 0)
        return;
    struct TextureSetSlot *slot = &texture_sets[new_alias];
    free(slot->buffer);
    slot->buffer = NULL;
    slot->alias_of_zero = true;
    slot->state = TileSet_Resident;
    show_set(new_alias, texture_sets[0].buffer);
}

TbBool load_texture_map_file(unsigned long tmapidx, LevelNumber lvnum, short fgroup)
{
    SYNCDBG(7,"Starting");
    if (texture_sets[0].buffer == NULL)
        return false;
    RendererWaitIdle();
    unsigned char *buf = texture_sets[0].buffer;
    memset(buf, TILE_SET_FILL_COLOUR, TILE_SET_BYTES);
    TbBool loaded = (load_letter_one_file(tmapidx, 'a', buf, lvnum, fgroup) >= 0);
    if (loaded)
        load_letter_one_file(tmapidx, 'b', buf + TILE_SET_LETTER_BYTES, lvnum, fgroup);
    else
        TILESETWARN("The level's own tile set has no files, so the map draws as plain colour. (pack %lu)", tmapidx);
    texture_set0_pack = tmapidx;
    // Every set without its own buffer shows set 0's tiles, so they all changed.
    for (int32_t v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
        if ((v == 0) || (texture_sets[v].buffer == NULL))
            texture_sets[v].generation++;
    set_alias(tile_set_alias_index((uint32_t)tmapidx));
    return loaded;
}

static void log_level_stats(void)
{
    uint32_t count = 0;
    for (int32_t v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
        if (texture_sets[v].buffer != NULL)
            count++;
    TILESETLOG("Tile sets in memory: %u of %d (%.1f MiB). Files read for this level: %u. Sets loaded during play: %u.",
        (unsigned)count, TEXTURE_VARIATIONS_COUNT, count * (double)TILE_SET_BYTES / (1024.0 * 1024.0),
        (unsigned)texture_sets_files_read, (unsigned)texture_sets_loaded_in_play);
}

void texture_sets_release_level(void)
{
    TileSetLoader_WaitIdle();
    struct TileSetResult res;
    while (TileSetLoader_TakeDone(&res))
        free(res.buffer);
    if (level_textures_ready)
        log_level_stats();
    RendererWaitIdle();
    for (int32_t v = 1; v < TEXTURE_VARIATIONS_COUNT; v++)
    {
        struct TextureSetSlot *slot = &texture_sets[v];
        free(slot->buffer);
        slot->buffer = NULL;
        slot->state = TileSet_Absent;
        slot->alias_of_zero = false;
        if (texture_sets[0].buffer != NULL)
            show_set(v, texture_sets[0].buffer);
    }
    texture_sets_level_serial++;
    texture_sets_files_read = 0;
    texture_sets_loaded_in_play = 0;
    texture_sets_accepting_requests = false;
    level_textures_ready = 0;
}

static uint32_t scan_current_level(void)
{
    unsigned char packs[DUNGEONS_COUNT];
    for (int i = 0; i < DUNGEONS_COUNT; i++)
        packs[i] = get_dungeon(i)->texture_pack;
    size_t slabs = (size_t)game.map_tiles_x * game.map_tiles_y;
    return tile_sets_scan(game.slab_ext_data, game.slab_ext_data_initial, slabs, packs, DUNGEONS_COUNT);
}

void texture_sets_load_for_level(void)
{
    LevelNumber lvnum = get_loaded_level_number();
    short fgroup = get_level_fgroup(lvnum);
    texture_sets_release_level();
    load_texture_map_file(game.texture_id, lvnum, fgroup);
    uint32_t mask = scan_current_level();
    texture_sets_accepting_requests = true;
    texture_sets_loading_level = true;
    for (int32_t v = 1; v < TEXTURE_VARIATIONS_COUNT; v++)
        if (mask & (1u << v))
            texture_set_request(v);
    TileSetLoader_WaitIdle();
    texture_sets_publish_ready();
    texture_sets_loading_level = false;
    level_textures_ready = 1;
    level_textures_generation++;
}

void texture_sets_reload_level_set(void)
{
    LevelNumber lvnum = get_loaded_level_number();
    int32_t old_alias = -1;
    for (int32_t v = 1; v < TEXTURE_VARIATIONS_COUNT; v++)
        if (texture_sets[v].alias_of_zero)
            old_alias = v;
    load_texture_map_file(game.texture_id, lvnum, get_level_fgroup(lvnum));
    if ((old_alias > 0) && (scan_current_level() & (1u << old_alias)))
        texture_set_request(old_alias);
}

void texture_sets_reconcile(void)
{
    if (!texture_sets_accepting_requests)
        return;
    if (texture_set0_pack != game.texture_id)
        texture_sets_reload_level_set();
    uint32_t mask = scan_current_level();
    for (int32_t v = 1; v < TEXTURE_VARIATIONS_COUNT; v++)
        if (mask & (1u << v))
            texture_set_request(v);
}

void texture_sets_shutdown(void)
{
    TileSetLoader_Shutdown();
    RendererWaitIdle();
    for (int32_t v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
    {
        free(texture_sets[v].buffer);
        texture_sets[v].buffer = NULL;
        texture_sets[v].state = TileSet_Absent;
    }
}

void texture_set_request(int32_t set)
{
    if (set <= 0 || set >= TEXTURE_VARIATIONS_COUNT || !texture_sets_accepting_requests)
        return;
    struct TextureSetSlot *slot = &texture_sets[set];
    if (!tile_set_needs_request(set, slot->state, slot->alias_of_zero))
        return;
    LevelNumber lvnum = get_loaded_level_number();
    short fgroup = get_level_fgroup(lvnum);
    static struct TileSetJob job;
    memset(&job, 0, sizeof(job));
    job.set = set;
    job.level_serial = texture_sets_level_serial;
    snprintf(job.path_a, sizeof(job.path_a), "%s", prepare_letter_one_file_path(set - 1, 'a', lvnum, fgroup));
    snprintf(job.path_b, sizeof(job.path_b), "%s", prepare_letter_one_file_path(set - 1, 'b', lvnum, fgroup));
    slot->state = TileSet_Pending;
    TileSetLoader_Queue(&job);
}

void texture_sets_publish_ready(void)
{
    struct TileSetResult res;
    TbBool waited = false;
    KFX_C_ZONE_BEGIN(ctx_publish, "TileSets::Publish");
    while (TileSetLoader_TakeDone(&res))
    {
        struct TextureSetSlot *slot = &texture_sets[res.set];
        if ((res.level_serial != texture_sets_level_serial) || (slot->state != TileSet_Pending))
        {
            free(res.buffer);
            continue;
        }
        if (res.buffer == NULL)
        {
            slot->state = TileSet_Absent;
            TILESETWARN("Not enough memory to load tile set %d; slabs that use it keep the level's own style.", (int)res.set);
            continue;
        }
        if (!waited)
        {
            RendererWaitIdle();
            waited = true;
        }
        slot->buffer = res.buffer;
        slot->state = TileSet_Resident;
        show_set(res.set, res.buffer);
        texture_sets_files_read += (res.result_a >= 0) + (res.result_b >= 0);
        if (!texture_sets_loading_level)
        {
            texture_sets_loaded_in_play++;
            TILESETLOG("Loaded tile set %d while playing. Slabs that use it now draw in that style.", (int)res.set);
        }
        if (res.result_a == LbLoadAtMax_Missing)
            TILESETWARN("Tile set %d has no files, so slabs that use it draw as plain colour. (pack %d)", (int)res.set, (int)res.set - 1);
        if ((res.result_a == LbLoadAtMax_TooLarge) || (res.result_b == LbLoadAtMax_TooLarge))
            TILESETWARN("A tile set file is larger than a tile set and was not loaded. (set %d, limit %u bytes)", (int)res.set, (unsigned)TILE_SET_LETTER_BYTES);
    }
    KFX_C_ZONE_END(ctx_publish);
}

TbBool texture_set_is_resident(int32_t set)
{
    if (set < 0 || set >= TEXTURE_VARIATIONS_COUNT)
        return false;
    return texture_sets[set].state == TileSet_Resident;
}

uint32_t texture_set_generation(int32_t set)
{
    if (set < 0 || set >= TEXTURE_VARIATIONS_COUNT)
        return 0;
    return texture_sets[set].generation;
}

void set_slab_texture_set(SlabCodedCoords slb_num, unsigned char value, TbBool also_initial)
{
    game.slab_ext_data[slb_num] = value;
    if (also_initial)
        game.slab_ext_data_initial[slb_num] = value;
    texture_set_request(value & TILE_SET_INDEX_MASK);
}
/******************************************************************************/
