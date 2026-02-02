/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_heap.c
 *     Definition of heap, used for storing memory-expensive sounds and graphics.
 * @par Purpose:
 *     Functions to create and maintain memory heap.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     06 Apr 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "custom_sprites.h"
#include "creature_graphics.h"
#include "front_simple.h"
#include "engine_render.h"
#include "bflib_fileio.h"
#include "gui_draw.h"
#include "frontend.h"
#include "bflib_dernc.h"
#include "sprites.h"
#include "config_spritecolors.h"
#include <spng.h>
#include <json.h>
#include <json-dom.h>
#include <minizip/unzip.h>
#include "post_inc.h"

// Performance tests
// #define OUTER
// #define INNER
#if defined(OUTER) || defined(INNER)
#include <SDL2/SDL.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Each part of RGB tuple of palette file is 1-63 actually
#define MAX_COLOR_VALUE 64
static uint8_t * rgb_to_pal_table = NULL;
static short next_free_sprite = 0;
static short next_free_icon = 0;

struct TbSpriteSheet * gui_panel_sprites = NULL;
struct TbSpriteSheet * custom_sprites = NULL;
struct NamedCommand *anim_names = NULL;

short iso_td_add[KEEPERSPRITE_ADD_NUM];
short td_iso_add[KEEPERSPRITE_ADD_NUM];

TbSpriteData keepersprite_add[KEEPERSPRITE_ADD_NUM] = {
        0
};

struct KeeperSprite creature_table_add[KEEPERSPRITE_ADD_NUM] = {
        {0}
};

struct SpriteContext
{
    struct TbHugeSprite sprite;

    unsigned long x, y;
    struct KeeperSprite *ksp_first;

    short *id_ptr; // First person / Top down
    short *id_sz_ptr; // First person / Top down

    short td_id, td_sz;
    short fp_id, fp_sz;

    TbBool rotatable;
};

static struct NamedCommand added_sprites[KEEPERSPRITE_ADD_NUM];
static struct NamedCommand added_icons[GUI_PANEL_SPRITES_NEW];
static int num_added_sprite = 0;
static int num_added_icons = 0;

#define MAX_LENS_OVERLAYS 64
static struct LensOverlayData added_lens_overlays[MAX_LENS_OVERLAYS];
static int num_added_lens_overlays = 0;

#define MAX_LENS_MISTS 64
static struct LensMistData added_lens_mists[MAX_LENS_MISTS];
static int num_added_lens_mists = 0;

unsigned char base_pal[PALETTE_SIZE];

int total_sprite_zip_count = 0;

// Indicates what custom assets to load
enum CustomLoadFlags {
    /// @brief Custom sprites
    CLF_Sprites = 0x1,
    /// @brief Custom icons
    CLF_Icons = 0x2,
    /// @brief Lens overlays
    CLF_LensOverlays = 0x4,
    /// @brief Lens mists
    CLF_LensMists = 0x8
};

static unsigned char big_scratch_data[1024*1024*16] = {0};
unsigned char *big_scratch = big_scratch_data;

static void compress_raw(struct TbHugeSprite *sprite, unsigned char *src_buf, int x, int y, int w, int h);

static TbBool add_custom_sprite(const char *path);

static TbBool
add_custom_json(const char *path, const char *name, TbBool (*process)(const char *path, unzFile zip, VALUE *root));

static TbBool process_lens_overlay(const char *path, unzFile zip, VALUE *root);
static TbBool process_lens_mist(const char *path, unzFile zip, VALUE *root);

static TbBool process_icon(const char *path, unzFile zip, VALUE *root);

static int cmp_named_command(const void *a, const void *b);

static unsigned char bad_icon_data[] = // 16x16
        {
                16, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 0,
                16, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 0,
                16, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 0,
                16, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 0,
                16, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 0,
                16, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 0,
                16, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 0,
                16, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 0,
                16, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 0,
                16, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 0,
                16, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 0,
                16, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 0,
                16, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 0,
                16, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 0,
                16, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 0,
                16, 17, 17, 17, 17, 255, 255, 255, 255, 17, 17, 17, 17, 255, 255, 255, 255, 0,
        };

const struct TbSprite bad_icon = { bad_icon_data, 16, 16 };
short bad_icon_id = INT16_MAX;

/*
 * Speedup zip stuff
 * We postulate only one zip file loaded at once
 */
static VALUE zip_cache_v;
static VALUE *zip_cache = &zip_cache_v;

static int fastUnzLocateFile(unzFile zip, const char *szFileName, int iCaseSensitivity)
{
    //return unzLocateFile(file, szFileName, iCaseSensitivity);
    char seek_for[PATH_MAX];
    strncpy(seek_for, szFileName, PATH_MAX - 1);
    make_lowercase(seek_for);
    VALUE *rec = value_dict_get(zip_cache, seek_for);
    if (rec == NULL)
        return UNZ_END_OF_LIST_OF_FILE;
    unz64_file_pos file_pos = {
            .pos_in_zip_directory = value_int64(value_array_get(rec, 0)),
            .num_of_file = value_int64(value_array_get(rec, 1))
    };
    return unzGoToFilePos64(zip, &file_pos);
}

/*
 * Construct a cache for files.
 * Also if there is no indexFile just return instead
 * */
static int fastUnzConstructCache(unzFile zip)
{
    char szCurrentFileName[PATH_MAX];
    if (value_type(zip_cache) != VALUE_NULL)
    {
        ERRORLOG("Zip cache is not clear!");
    }
    value_init_dict(zip_cache);

    for (int err = unzGoToFirstFile(zip);
         err == UNZ_OK;
         err = unzGoToNextFile(zip))
    {
        if (UNZ_OK != unzGetCurrentFileInfo64(zip, NULL,
                                              szCurrentFileName, sizeof(szCurrentFileName) - 1,
                                              NULL, 0, NULL, 0)
                )
        {
            continue;
        }
        make_lowercase(szCurrentFileName);

        unz64_file_pos file_pos;
        unzGetFilePos64(zip, &file_pos);

        VALUE *rec = value_dict_add(zip_cache, szCurrentFileName);
        value_init_array(rec);
        value_init_int64(value_array_append(rec), file_pos.pos_in_zip_directory);
        value_init_int64(value_array_append(rec), file_pos.num_of_file);
    }
    return UNZ_OK;
}

static int fastUnzClearCache()
{
    value_fini(zip_cache);
    return 0;
}

/* end of zip stuff */

static int cmp_named_command(const void *a, const void *b)
{

    const struct NamedCommand *val_a = a;
    const struct NamedCommand *val_b = b;
    return strcasecmp(val_a->name, val_b->name);
}


static int load_file_sprites(const char *path, const char *file_desc)
{
    SYNCDBG(8, "Starting");
    int add_flag = 0;
    if (add_custom_sprite(path))
    {
        add_flag |= CLF_Sprites;
    }

    if (add_custom_json(path, "icons.json", &process_icon))
    {
        add_flag |= CLF_Icons;
    }

    if (add_custom_json(path, "lenses.json", &process_lens_overlay))
    {
        add_flag |= CLF_LensOverlays;
    }

    if (add_custom_json(path, "mists.json", &process_lens_mist))
    {
        add_flag |= CLF_LensMists;
    }

    if (file_desc != NULL)
    {
        if (add_flag & CLF_Sprites)
        {
            JUSTLOG("Loaded per-map sprites from %s", file_desc);
        }
        else
        {
            SYNCDBG(0, "Unable to load per-map sprites from %s", file_desc);
        }

        if (add_flag & CLF_Icons)
        {
            JUSTLOG("Loaded per-map icons from %s", file_desc);
        }
        else
        {
            SYNCDBG(0, "Unable to load per-map icons from %s", file_desc);
        }

        if (add_flag & CLF_LensOverlays)
        {
            JUSTLOG("Loaded lens overlays from %s", file_desc);
        }
        else
        {
            SYNCDBG(0, "Unable to load lens overlays from %s", file_desc);
        }
        if (add_flag & CLF_LensMists)
        {
            JUSTLOG("Loaded lens mists from %s", file_desc);
        }
        else
        {
            SYNCDBG(0, "Unable to load lens mists from %s", file_desc);
        }
        total_sprite_zip_count++;
    }

    return add_flag;
}

static void load_dir_sprites(const char *dir_path, const char *dir_desc)
{
    SYNCDBG(8, "Starting");
    if (dir_path == NULL || dir_path[0] == 0)
        return;
    char full_path[1024] = {0};
    sprintf(full_path, "%s/%s", dir_path, "*.zip");
    struct TbFileEntry fe;
    struct TbFileFind * ff = LbFileFindFirst(full_path, &fe);
    int cnt_zip = 0, cnt_sprite = 0, cnt_icon = 0;
    if (ff) {
        do {
            sprintf(full_path, "%s/%s", dir_path, fe.Filename);
            int add_flag = load_file_sprites(full_path, NULL);
            if (add_flag & CLF_Sprites)
                cnt_sprite++;
            if (add_flag & CLF_Icons)
                cnt_icon++;
            cnt_zip++;
        } while (LbFileFindNext(ff, &fe) >= 0);
        LbFileFindEnd(ff);

        if (dir_desc != NULL)
            LbJustLog("Found %d sprite zip file(s) from %s, loaded %d with animations and %d with icons. Used %d/%d sprite slots.\n", cnt_zip, dir_desc, cnt_sprite, cnt_icon, next_free_sprite, KEEPERSPRITE_ADD_NUM);
        total_sprite_zip_count += cnt_zip;
    }
}

/* @comment
 *     The loading items of init_custom_sprites and load_sprites_for_mod_one need to be consistent.
 */
static void load_sprites_for_mod_one(LevelNumber lvnum, const struct ModConfigItem *mod_item)
{

    const struct ModExistState *mod_state = &mod_item->state;
    char* fname = NULL, *dname = NULL;
    char mod_dir[256] = {0}, desc[256] = {0};;
    sprintf(mod_dir, "%s/%s", MODS_DIR_NAME, mod_item->name);

    if (mod_state->fx_data)
    {
        dname = prepare_file_path_mod(mod_dir, FGrp_FxData, NULL);
        if (strlen(dname) > 0)
        {
            sprintf(desc, "Mod[%s] FxData dir", mod_item->name);
            load_dir_sprites(dname, desc);
        }
    }

    if (mod_state->cmpg_config)
    {
        dname = prepare_file_path_mod(mod_dir, FGrp_CmpgConfig, NULL);
        if (strlen(dname) > 0)
        {
            sprintf(desc, "Mod[%s] CmpgConfig dir", mod_item->name);
            load_dir_sprites(dname, desc);
        }
    }

    if (mod_state->cmpg_lvls)
    {
        fname = prepare_file_fmtpath_mod(mod_dir, FGrp_CmpgLvls, "map%05lu.zip", lvnum);
        if (strlen(fname) > 0 && LbFileExists(fname))
        {
            sprintf(desc, "Mod[%s] CmpgLvls file", mod_item->name);
            load_file_sprites(fname, desc);
        }
    }
}

static void load_sprites_for_mod_list(LevelNumber lvnum, const struct ModConfigItem *mod_items, long mod_cnt)
{
    for (long i=0; i<mod_cnt; i++)
    {
        const struct ModConfigItem *mod_item = mod_items + i;
        if (mod_item->state.mod_dir == 0)
            continue;

        load_sprites_for_mod_one(lvnum, mod_item);
    }
}

/* @comment
 *     The loading items of init_custom_sprites and load_sprites_for_mod_one need to be consistent.
 */
void init_custom_sprites(LevelNumber lvnum)
{
    SYNCDBG(8, "Starting");
    free_spritesheet(&custom_sprites);
    custom_sprites = create_spritesheet();
    total_sprite_zip_count = 0;
    // This is a workaround because get_selected_level_number is zeroed on res change
    if (lvnum == SPRITE_LAST_LEVEL)
    {
        lvnum = game.last_level;
    }
    else if (lvnum > 0)
    {
        game.last_level = lvnum;
    }
    else
    {
        ERRORLOG("Invalid level number %d for loading custom sprites", lvnum);
    }
    // Clear sprite data
    for (int i = 0; i < KEEPERSPRITE_ADD_NUM; i++)
    {
        if (keepersprite_add[i] != NULL)
        {
            free(keepersprite_add[i]);
            keepersprite_add[i] = NULL;
        }
    }
    // Clear added sprites
    for (int i = 0; i < num_added_sprite; i++)
    {
        if (added_sprites[i].name != NULL)
        {
            free((char *) added_sprites[i].name);
        }
    }
    num_added_sprite = 0;
    memset(added_sprites, 0, sizeof(added_sprites));

    // Clear added icons
    for (int i = 0; i < num_added_icons; i++)
    {
        if (added_icons[i].name != NULL)
        {
            free((char *) added_icons[i].name);
            added_icons[i].name = NULL;
        }
    }
    num_added_icons = 0;
    memset(added_icons, 0, sizeof(added_icons));
    next_free_icon = 0;

    // Clear creature table (there sprites live)
    memset(creature_table_add, 0, sizeof(creature_table_add));
    next_free_sprite = 0;

    if (anim_names != NULL)
    {
        free(anim_names);
    }


    char *dname = prepare_file_path(FGrp_FxData, NULL);
    load_dir_sprites(dname, "Main FxData dir");

    if (mods_conf.after_base_cnt > 0)
    {
        load_sprites_for_mod_list(lvnum, mods_conf.after_base_item, mods_conf.after_base_cnt);
    }

    dname = prepare_file_path(FGrp_CmpgConfig, NULL);
    load_dir_sprites(dname, "Main CmpgConfig dir");

    if (mods_conf.after_campaign_cnt > 0)
    {
        load_sprites_for_mod_list(lvnum, mods_conf.after_campaign_item, mods_conf.after_campaign_cnt);
    }

    char *fname = prepare_file_fmtpath(get_level_fgroup(lvnum), "map%05lu.zip", lvnum);
    if (LbFileExists(fname))
        load_file_sprites(fname, "Main CmpgLvls file");

    if (mods_conf.after_map_cnt > 0)
    {
        load_sprites_for_mod_list(lvnum, mods_conf.after_map_item, mods_conf.after_map_cnt);
    }

}

/**
 * Read from current file in zip archive (it should be opened already)
 * @param ctx
 * @param user
 * @param dst_src
 * @param length
 * @return
 */
static int zip_read_fn(spng_ctx *ctx, void *user, void *dst_src, size_t length)
{
    unzFile zip = user;

    return unzReadCurrentFile(zip, dst_src, length) != length;
}

/**
 * Convert camera name (i.e. fprr) to camera #
 * @param camera_name
 * @return index of camera direction
 */
static int dir_from_camera_name(const char *camera_name)
{
    if (camera_name[2] == 0)
        return 0;
    if (0 == strcasecmp(camera_name + 2, "rff"))
        return 0;
    if (0 == strcasecmp(camera_name + 2, "rf"))
        return 1;
    if (0 == strcasecmp(camera_name + 2, "r"))
        return 2;
    if (0 == strcasecmp(camera_name + 2, "br"))
        return 3;
    if (0 == strcasecmp(camera_name + 2, "b"))
        return 4;
    return -1;
}

/**
 *
 * @param zip
 * @param path
 * @param context
 * @param blender_filename
 * @param subpath
 * @param node
 * @return 1 if error
 */
static int read_png_info(unzFile zip, const char *path, struct SpriteContext *context, const char *blender_filename,
                         const char *subpath, VALUE *node)
{
    struct TbHugeSprite *sprite = &context->sprite;
    const char *camera = NULL;
    size_t out_size;
    sprite->SHeight = 0;
    sprite->SWidth = 0;

    spng_ctx *ctx = NULL;
    ctx = spng_ctx_new(0);
    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    size_t limit = 1024 * 1024 * 2;
    spng_set_chunk_limits(ctx, limit, limit);

    spng_set_png_stream(ctx, zip_read_fn, (void *) zip);
    struct spng_ihdr ihdr;
    int r = spng_get_ihdr(ctx, &ihdr);

    if (r)
    {
        ERRORLOG("spng_get_ihdr() error: %s", spng_strerror(r));
        spng_ctx_free(ctx);
        return 1;
    }

    if (ihdr.bit_depth != 8)
    {
        ERRORLOG("Wrong spec: %s/%s should be 8bit truecolor or indexed .png", path, subpath);
        spng_ctx_free(ctx);
        return 1;
    }
    struct spng_plte plte = {0};
    r = spng_get_plte(ctx, &plte);
    // TODO: should we check palette?

    sprite->SWidth = ihdr.width;
    sprite->SHeight = ihdr.height;

    int fmt = SPNG_FMT_RGBA8; // for indexed should be SPNG_FMT_PNG

    spng_decoded_image_size(ctx, fmt, &out_size);
    if (limit < out_size) // Image is too big
    {
        ERRORLOG("Unable to decode %s error: %s", path, spng_strerror(r));
        spng_ctx_free(ctx);
        return 1;
    }

    uint32_t n_text = 0;
    TbBool found = 0;
    long frame_no = 0;

    if (0 != spng_get_text(ctx, NULL, &n_text))
    {
        spng_ctx_free(ctx);
        return 1;
    }
    struct spng_text *text = malloc(sizeof(struct spng_text) * n_text);
    spng_get_text(ctx, text, &n_text);

    for (int i = 0; i < n_text; i++)
    {
        const char *keyword = text[i].keyword;
        const char *value = text[i].text;
        if (0 == strcmp(keyword, "Scene"))
        {
            if (0 == strncmp(value, blender_filename, text[i].length)) // Not our scene?
            {
                found++;
            }
        }
        else if (0 == strcmp(keyword, "Camera"))
        {
            camera = value;
            found++;
        }
        else if (0 == strcmp(keyword, "Frame"))
        {
            char *endl = NULL;
            frame_no = strtol(value, &endl, 10);
            if (endl == value)
            {
                WARNLOG("Invalid Frame metadata at %s/%s", path, subpath);
                free(text);
                spng_ctx_free(ctx);
                return 1;
            }
            frame_no--;
            found++;
        }
    }

    if (found != 3) // Scene + Camera + Frame
    {
        free(text);
        spng_ctx_free(ctx);
        return 0; // File without metadata is not a problem
    }

    TbBool dir_type = (0 == strncasecmp(camera, "fp", 2));
    VALUE *td_dir = value_dict_get_or_add(node, dir_type ? "fp" : "td");
    if (value_type(td_dir) == VALUE_NULL)
    {
        value_init_array(td_dir);
    }

    // At least one image direction should be present
    for (int i = value_array_size(td_dir); i < 1; i++)
    {
        value_init_array(value_array_append(td_dir));
    }
    int lr_dir = dir_from_camera_name(camera);
    if (lr_dir < 0)
    {
        WARNLOG("Unknown frame: %s/%s dir:%s ", path, subpath, camera);
        free(text);
        spng_ctx_free(ctx);
        return 1;
    }

    if ((!context->rotatable) && (lr_dir > 1))
    {
        VALUE *rotated = value_dict_get_or_add(node, "rotatable");
        switch (value_type(rotated))
        {
            case VALUE_NULL:
                value_init_bool(rotated, true);
                break;
            case VALUE_BOOL:
                if (!value_bool(rotated))
                {
                    WARNLOG("Too many frames and Rotated is false");
                    free(text);
                    spng_ctx_free(ctx);
                    return 1;
                }
                break;
            case VALUE_INT32:
            case VALUE_UINT32:
            case VALUE_INT64:
            case VALUE_UINT64:
            case VALUE_FLOAT:
            case VALUE_DOUBLE:
                if (!value_int32(rotated))
                {
                    WARNLOG("Too many frames and Rotated is false");
                    free(text);
                    spng_ctx_free(ctx);
                    return 1;
                }
                break;
            default:
            {
                WARNLOG("'rotatable' has unexpected value");
                free(text);
                spng_ctx_free(ctx);
                return 1;
            }
        }
        context->rotatable = true;
    }
    if (context->rotatable)
    {
        for (int i = value_array_size(td_dir); i < 5; i++)
        {
            value_init_array(value_array_append(td_dir));
        }
    }

    VALUE *arr = value_array_get(td_dir, lr_dir);

    if (frame_no >= value_array_size(arr)) // >=
    {
        for (int i = value_array_size(arr); i <= frame_no; i++)
        {
            value_array_insert(arr, i);
        }
    }
    VALUE *row = value_array_get(arr, frame_no);
    if (value_type(row) == VALUE_NULL)
    {
        value_init_dict(row);
    }
    else if (value_type(row) != VALUE_DICT)
    {
        ERRORLOG("Invalid frame record");
        free(text);
        spng_ctx_free(ctx);
        return 1;
    }

    VALUE *dst = value_dict_get_or_add(row, "file");
    if (value_type(dst) != VALUE_NULL)
    {
        WARNLOG("Overriding frame %s/%s", path, subpath);
        value_fini(dst);
    }
    value_init_string(dst, subpath);

    free(text);
    spng_ctx_free(ctx);
    return 0;
}

static int read_png_icon(unzFile zip, const char *path, const char *subpath, int *icon_ptr)
{
    struct TbHugeSprite sprite = {0};
    size_t out_size;

    spng_ctx *ctx = NULL;
    ctx = spng_ctx_new(0);
    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    size_t limit = 1024 * 1024 * 2;
    spng_set_chunk_limits(ctx, limit, limit);

    spng_set_png_stream(ctx, zip_read_fn, (void *) zip);
    struct spng_ihdr ihdr;
    int r = spng_get_ihdr(ctx, &ihdr);

    if (r)
    {
        ERRORLOG("spng_get_ihdr() error: %s", spng_strerror(r));
        spng_ctx_free(ctx);
        return 0;
    }

    if (ihdr.bit_depth != 8)
    {
        ERRORLOG("Wrong spec: %s/%s should be 8bit truecolor or indexed .png", path, subpath);
        spng_ctx_free(ctx);
        return 0;
    }
    struct spng_plte plte = {0};
    r = spng_get_plte(ctx, &plte);

    sprite.SWidth = ihdr.width;
    sprite.SHeight = ihdr.height;

    int fmt = SPNG_FMT_RGBA8; // for indexed should be SPNG_FMT_PNG

    spng_decoded_image_size(ctx, fmt, &out_size);
    if (limit < out_size) // Image is too big
    {
        ERRORLOG("Unable to decode %s error: %s", path, spng_strerror(r));
        spng_ctx_free(ctx);
        return 0;
    }

    unsigned char *dst_buf = big_scratch;
    spng_decode_image(ctx, dst_buf, out_size, fmt, SPNG_DECODE_TRNS);

    if (sprite.SWidth >= 255 || sprite.SHeight >= 255)
    {
        ERRORLOG("Sprites more than 255x255 are not supported");
        return 0;
    }

    size_t sz = (sprite.SWidth + 2) * (sprite.SHeight + 3);
    sprite.Data = malloc(sz);

    compress_raw(&sprite, dst_buf, 0, 0, sprite.SWidth, sprite.SHeight);

    spng_ctx_free(ctx);

    if (next_free_icon >= GUI_PANEL_SPRITES_NEW)
    {
        ERRORLOG("Too many custom icons allocated");
        return 0;
    }

    add_sprite(custom_sprites, sprite.SWidth, sprite.SHeight, sz, sprite.Data);
    free(sprite.Data);
    *icon_ptr = next_free_icon + GUI_PANEL_SPRITES_COUNT;
    next_free_icon++;

    return 1;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-branch-clone"
static int read_png_data(unzFile zip, const char *path, struct SpriteContext *context, const char *subpath,
                         int fp, VALUE *def, VALUE *itm)
{
    struct TbHugeSprite *sprite = &context->sprite;
    size_t out_size;
    sprite->SHeight = 0;
    sprite->SWidth = 0;

    spng_ctx *ctx = NULL;
    ctx = spng_ctx_new(0);
    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    size_t limit = 1024 * 1024 * 2;
    spng_set_chunk_limits(ctx, limit, limit);

    spng_set_png_stream(ctx, zip_read_fn, (void *) zip);
    struct spng_ihdr ihdr;
    int r = spng_get_ihdr(ctx, &ihdr);

    if (r)
    {
        ERRORLOG("spng_get_ihdr() error: %s", spng_strerror(r));
        spng_ctx_free(ctx);
        return 0;
    }

    if (ihdr.bit_depth != 8)
    {
        ERRORLOG("Wrong spec: %s/%s should be 8bit truecolor or indexed .png", path, subpath);
        spng_ctx_free(ctx);
        return 0;
    }
    struct spng_plte plte = {0};
    r = spng_get_plte(ctx, &plte);
    // TODO: should we check palette?

    sprite->SWidth = ihdr.width;
    sprite->SHeight = ihdr.height;

    int fmt = SPNG_FMT_RGBA8; // for indexed should be SPNG_FMT_PNG

    spng_decoded_image_size(ctx, fmt, &out_size);
    if (limit < out_size) // Image is too big
    {
        ERRORLOG("Unable to decode %s error: %s", path, spng_strerror(r));
        spng_ctx_free(ctx);
        return 0;
    }

    unsigned char *dst_buf = big_scratch;
    if (spng_decode_image(ctx, dst_buf, out_size, fmt, SPNG_DECODE_TRNS))
    {
        ERRORLOG("Unable to decode %s/%s", path, subpath);
        spng_ctx_free(ctx);
        return 0;
    }

    // This should be enough except rare cases like transparent checkerboard
    int dst_w = (int) context->sprite.SWidth;
    int dst_h = (int) context->sprite.SHeight;

    if (dst_w >= 255 || dst_h >= 255)
    {
        ERRORLOG("Sprites more than 255x255 are not supported");
        return 0;
    }

    if (next_free_sprite >= KEEPERSPRITE_ADD_NUM)
    {
        ERRORLOG("Too many custom sprites allocated");
        return 0;
    }
    short sprite_idx = next_free_sprite;
    next_free_sprite++;
    if (*context->id_ptr == 0) // First sprite for current view (FP/TD)
        *context->id_ptr = sprite_idx + KEEPERSPRITE_ADD_OFFSET;
    (*context->id_sz_ptr)++; // Add new sprite for current view (FP/TD)

    size_t sz = (dst_w + 2) * (dst_h + 3);
    keepersprite_add[sprite_idx] = malloc(sz);
    context->sprite.Data = keepersprite_add[sprite_idx];
    compress_raw(&context->sprite, dst_buf, context->x, context->y, dst_w, dst_h);
    struct KeeperSprite *ksprite = &creature_table_add[sprite_idx];

    if (context->ksp_first == NULL)
    {
        context->ksp_first = ksprite;
    }
    else
    {
        context->ksp_first->FramesCount++;
    }

    ksprite->DataOffset = 0;
    // That is this actually?
    ksprite->SWidth = dst_w;
    ksprite->SHeight = dst_h;
    ksprite->Rotable = context->rotatable ? 2 : 0;
    ksprite->FramesCount = 1;

    VALUE *val;

#define READ_WITH_DEFAULT(dst, field, fp_def, td_def, def_val, fn) \
    val = value_dict_get(itm, field); \
    if (value_type(val) != VALUE_NULL) \
    { \
        ksprite-> dst = fn(val); \
    } \
    else if (val = value_dict_get(def, fp ? fp_def : td_def), \
            value_type(val) != VALUE_NULL \
            ) \
    {                                                              \
        ksprite-> dst = fn(val); \
    } \
    else \
    { \
        ksprite-> dst = def_val; \
    }

    READ_WITH_DEFAULT(FrameWidth, "frame_w", "fp_frame_w", "td_frame_w", dst_w, value_uint32)
    READ_WITH_DEFAULT(FrameHeight, "frame_h", "fp_frame_h", "td_frame_h", dst_h, value_uint32)
    READ_WITH_DEFAULT(FrameOffsW, "offset_w", "fp_offset_w", "td_offset_w", 0, value_int32)
    READ_WITH_DEFAULT(FrameOffsH, "offset_h", "fp_offset_h", "td_offset_h", 0, value_int32)
    READ_WITH_DEFAULT(offset_x, "offset_x", "fp_offset_x", "td_offset_x", -dst_w / 2, -value_int32)
    READ_WITH_DEFAULT(offset_y, "offset_y", "fp_offset_y", "td_offset_y", 1 - dst_h, -value_int32)

    READ_WITH_DEFAULT(shadow_offset, "shadow_offset", "fp_shadow_offset", "td_shadow_offset", (dst_h + ksprite->offset_y), value_int32)

    READ_WITH_DEFAULT(frame_flags, "frame_flags", "fp_frame_flags", "td_frame_flags", 0, value_uint32)

#undef READ_WITH_DEFAULT

    spng_ctx_free(ctx);
    return 1;
}
#pragma clang diagnostic pop

static void convert_row(unsigned char *dst_buf, uint32_t *src_buf, int len)
{
    for (int i = 0; i < len; i++)
    {
        const uint32_t color = *src_buf++;
        const uint32_t key =
            (((color >> 2) & (MAX_COLOR_VALUE - 1)) << 0) +
            (((color >> 10) & (MAX_COLOR_VALUE - 1)) << 6) +
            (((color >> 18) & (MAX_COLOR_VALUE - 1)) << 12);
        *dst_buf++ = rgb_to_pal_table[key];
    }
}

static uint8_t nearest_color(uint32_t value, const uint8_t * palette)
{
    // naive approach
    uint8_t nearest = 0;
    uint32_t nearest_delta = UINT32_MAX;
    const uint8_t vr = value & (MAX_COLOR_VALUE - 1);
    const uint8_t vg = (value >> 6) & (MAX_COLOR_VALUE - 1);
    const uint8_t vb = (value >> 12) & (MAX_COLOR_VALUE - 1);
    for (int i = 0; i < 256; ++i) {
        const uint8_t pr = palette[(i * 3) + 0];
        const uint8_t pg = palette[(i * 3) + 1];
        const uint8_t pb = palette[(i * 3) + 2];
        const uint32_t delta =
            (max(pr, vr) - min(pr, vr)) +
            (max(pg, vg) - min(pg, vg)) +
            (max(pb, vb) - min(pb, vb));
        if (delta < nearest_delta) {
            nearest = i;
            nearest_delta = delta;
        }
    }
    return nearest;
}

static void load_rgb_to_pal_table()
{
    if (rgb_to_pal_table) {
        return; // already done, skip
    }
    // load palette
    const uint32_t table_size = MAX_COLOR_VALUE * MAX_COLOR_VALUE * MAX_COLOR_VALUE;
    rgb_to_pal_table = calloc(table_size, 1);
    if (!rgb_to_pal_table) {
        ERRORLOG("Cannot allocate rgb conversion table");
        return;
    }
    const char * fname = prepare_file_fmtpath(FGrp_StdData, "png_conv_pal.dat");
    if (!LbFileExists(fname))
    {
        WARNMSG("Palette file \"%s\" doesn't exist.", fname);
        return;
    }
    uint8_t palette[768];
    if (LbFileLoadAt(fname, palette) < sizeof(palette)) {
        ERRORLOG("Can't load palette file.");
        return;
    }
    // populate table
    for (uint32_t i = 0; i < table_size; ++i) {
        rgb_to_pal_table[i] = nearest_color(i, palette);
    }
}

static void compress_raw(struct TbHugeSprite *sprite, unsigned char *inp_buf, int x, int y, int w, int h)
{
    #define TEST_TRANSP(x) ((x & 0xFF000000u) < 0x40000000u)
    load_rgb_to_pal_table();
    unsigned char *buf = sprite->Data;
    uint32_t *src_buf = (uint32_t *) inp_buf;
    TbBool is_transp;
    int len;
    int tail = sprite->SWidth - w;
    src_buf += y * sprite->SWidth;
    src_buf += x;
    for (int j = 0; j < h; j++)
    {
        is_transp = false;
        len = 0;
        for (int i = 0; i < w; i++, src_buf++)
        {
            if (is_transp)
            {
                if (!TEST_TRANSP(*src_buf) || len == 127)
                {
                    *buf = -len;
                    buf++;
                    len = 1;
                    is_transp = TEST_TRANSP(*src_buf);
                }
                else
                {
                    len++;
                }
            }
            else
            {
                if (TEST_TRANSP(*src_buf) || len == 127)
                {
                    if (len > 0)
                    {
                        *buf = len;
                        buf++;
                        convert_row(buf, src_buf - len, len);
                        buf += len;
                    }

                    is_transp = TEST_TRANSP(*src_buf);
                    len = 1;
                }
                else
                {
                    len++;
                }
            }
        }
        if ((len > 0) && !is_transp)
        {
            *buf = len;
            buf++;
            convert_row(buf, src_buf - len, len);
            buf += len;
        }
        *buf = 0;
        buf++;
        src_buf += tail;
    }
    #undef TEST_TRANSP
}

#if BFDEBUG_LEVEL > 0
struct StrBuf
{
    char *ptr;
    size_t size;
};
#endif

#if BFDEBUG_LEVEL > 10
static int dump_callback(const char *str, size_t size, void *user_data)
{
    struct StrBuf *buf = user_data;
    buf->ptr = realloc(buf->ptr, buf->size + size + 1);
    memcpy(buf->ptr + buf->size, str, size);
    buf->size += size;
    buf->ptr[buf->size] = 0;
    return 0;
}
#endif

/**
 * Collect sprites from zipfile with specific blender_scene
 * @param zip - opened zip file
 */
static int
collect_sprites(const char *path, unzFile zip, const char *blender_scene, struct SpriteContext *context, VALUE *node)
{
    char szCurrentFileName[256];

    if (blender_scene != NULL) // Collect sprites by blender_scene
    {
        for (int err = unzGoToFirstFile(zip);
             err == UNZ_OK;
             err = unzGoToNextFile(zip))
        {
            if (UNZ_OK != unzGetCurrentFileInfo64(zip, NULL,
                                                  szCurrentFileName, sizeof(szCurrentFileName) - 1,
                                                  NULL, 0, NULL, 0)
                    )
            {
                continue;
            }
            char *term = strrchr(szCurrentFileName, '.');
            if (term == NULL)
                continue;
            if (strcasecmp(term, ".png") != 0)
                continue;
            if (UNZ_OK != unzOpenCurrentFile(zip))
            {
                return 1;
            }
            err = read_png_info(zip, path, context, blender_scene, szCurrentFileName, node);
            if (UNZ_OK != unzCloseCurrentFile(zip))
            {
                return 1;
            }
            if (err)
            {
                return err;
            }
        }
    }

#if BFDEBUG_LEVEL > 10
    struct StrBuf buf = {0, 0};

    json_dom_dump(node, &dump_callback, &buf, 2, 0);

    fprintf(stderr, "%s", buf.ptr);
#endif
    context->rotatable = (value_bool(value_dict_get(node, "rotatable")) > 0);

    int prev_sz;
    VALUE *ud_lst;
    for (int fp = 0; fp < 2; fp++)
    {
        if (fp == 0)
        {
            ud_lst = value_dict_get(node, "td");
            prev_sz = value_array_size(value_array_get(ud_lst, 0));
            // first good loaded sprite will be loaded at this pointers (topdown in this case)
            context->id_ptr = &context->td_id;
            context->id_sz_ptr = &context->td_sz;
        }
        else
        {
            ud_lst = value_dict_get(node, "fp");
            context->id_ptr = &context->fp_id; // First person case
            context->id_sz_ptr = &context->fp_sz;
        }
        for (int lr = 0; lr < (context->rotatable ? 5 : 1); lr++) // If sprite is rotatable
        {
            VALUE *lr_list = value_array_get(ud_lst, lr);
            // Each frame should keep valid frames count
            if (context->ksp_first != NULL)
            {
                for (int i = 1; i < context->ksp_first->FramesCount; i++)
                {
                    context->ksp_first[i].FramesCount = context->ksp_first->FramesCount;
                }
            }
            context->ksp_first = NULL;

            for (int frame = 0; frame < value_array_size(lr_list); frame++)
            {
                VALUE *itm = value_array_get(lr_list, frame);
                const char *name = value_string(value_dict_get(itm, "file"));
                if (name == NULL)
                {
                    WARNLOG("Invalid sprite file record in '%s/sprites.json'", path);
                    return 1;
                }
                if (fastUnzLocateFile(zip, name, 0))
                {
                    WARNLOG("Png '%s' not found in '%s'", name, path);
                    return 1;
                }
                if (UNZ_OK != unzOpenCurrentFile(zip))
                {
                    WARNLOG("Unable to open '%s/%s'", path, name);
                    return 1;
                }
                short store_p = *context->id_ptr;
                short store_sz = *context->id_sz_ptr;
                struct KeeperSprite *store_ksp = context->ksp_first;
                unsigned char store_ksp_fc = 0;
                if (store_ksp)
                    store_ksp_fc = context->ksp_first->FramesCount;
#ifdef INNER
                fprintf(stderr, "F:%s/%s\n", path, name);
                fprintf(stderr, "A:%d\n", SDL_GetTicks());
#endif
                if (!read_png_data(zip, path, context, name, fp, node, itm))
                {
                    // Reverting possible changes
                    *context->id_ptr = store_p;
                    *context->id_sz_ptr = store_sz;
                    context->ksp_first = store_ksp;
                    if (store_ksp)
                        context->ksp_first->FramesCount = store_ksp_fc;

                    unzCloseCurrentFile(zip);
                    WARNLOG("Unable to read '%s/%s'", path, name);
                    return 1;
                }
#ifdef INNER
                fprintf(stderr, "B:%d\n", SDL_GetTicks());
#endif
                if (UNZ_OK != unzCloseCurrentFile(zip))
                {
                    return 1;
                }
            }
        }
    }
    // Each frame should keep valid frames count
    if (context->ksp_first != NULL)
    {
        for (int i = 1; i < context->ksp_first->FramesCount; i++)
        {
            context->ksp_first[i].FramesCount = context->ksp_first->FramesCount;
        }
    }

    if (prev_sz != value_array_size(value_array_get(ud_lst, 0)))
    {
        ERRORLOG("%s/sprite.json should have same amount of TD and FP frames", path);
        return 1;
    }
    if (context->fp_sz != context->td_sz)
    {
        ERRORLOG("%s/sprite.json should have same amount of TD and FP frames (2)", path);
        return 1;
    }
    // Installing frames into arrays ()
    for (int i = context->td_sz - 1; i >= 0; i--)
    {
        short fp_id = context->fp_id + i;
        short td_id = context->td_id + i;
        td_iso_add[fp_id - KEEPERSPRITE_ADD_OFFSET] = td_id;
        iso_td_add[fp_id - KEEPERSPRITE_ADD_OFFSET] = fp_id;
        iso_td_add[td_id - KEEPERSPRITE_ADD_OFFSET] = fp_id;
        td_iso_add[td_id - KEEPERSPRITE_ADD_OFFSET] = td_id;
    }
    return context->td_sz <= 0;
}

static int process_sprite_from_list(const char *path, unzFile zip, int idx, VALUE *root)
{
    VALUE *val;
    struct SpriteContext context = {0};

    val = value_dict_get(root, "name");
    if (val == NULL)
    {
        WARNLOG("Invalid sprite %s/sprites.json[%d]: no \"name\" key", path, idx);
        return 0;
    }
    const char *name = value_string(val);
    const char *blend_scene = NULL;
    SYNCDBG(2, "found sprite: '%s/%s'", path,name);
    val = value_dict_get(root, "blender_scene");
    if ((val != NULL) && (value_type(val) == VALUE_STRING))
    {
        blend_scene = value_string(val);
    }

    if (collect_sprites(path, zip, blend_scene, &context, root))
    {
        WARNLOG("Unable to collect sprites from %s", path);
        return 0;
    }

    struct NamedCommand key = {name, 0};
    struct NamedCommand *spr = bsearch(&key, added_sprites, num_added_sprite, sizeof(added_sprites[0]),
                                       &cmp_named_command);
    if (spr)
    {
        // TODO: remove old spr->num (all of them are removed on each map load)
        spr->num = context.td_id;
        JUSTLOG("Sprite '%s/%s' overwrites sprite with same name.", path,name);
    }
    else
    {
        if (num_added_sprite >= KEEPERSPRITE_ADD_NUM)
        {
            ERRORLOG("Too many custom sprites");
            return 0;
        }
        spr = &added_sprites[num_added_sprite++];
        spr->name = strdup(name);
        spr->num = context.td_id;
    }

    return 1;
}

static TbBool
add_custom_json(const char *path, const char *name, TbBool (*process)(const char *path, unzFile zip, VALUE *root))
{
    SYNCDBG(8, "Starting");
    unz_file_info64 zip_info = {0};
    VALUE root;
    JSON_INPUT_POS json_input_pos;
    unzFile zip = unzOpen(path);

    if (zip == NULL)
        return 0;

    if (UNZ_OK != fastUnzConstructCache(zip))
    {
        goto end;
    }

    if (UNZ_OK != fastUnzLocateFile(zip, name, 0))
    {
        goto end;
    }

    if (UNZ_OK != unzGetCurrentFileInfo64(zip, &zip_info, NULL, 0, NULL, 0, NULL, 0)
            )
    {
        goto end;
    }

    if (zip_info.uncompressed_size >= 1024 * 1024)
    {
        WARNLOG("File too big %s/%s", path, name);
        goto end;
    }

    if (UNZ_OK != unzOpenCurrentFile(zip))
    {
        goto end;
    }

    if (unzReadCurrentFile(zip, big_scratch, zip_info.uncompressed_size) != zip_info.uncompressed_size)
    {
        WARNLOG("Unable to read %s/%s", path, name);
        goto end;
    }
    big_scratch[zip_info.uncompressed_size] = 0;

    if (UNZ_OK != unzCloseCurrentFile(zip))
    {
        goto end;
    }

    int ret = json_dom_parse((char *) big_scratch, zip_info.uncompressed_size, NULL, 0, &root, &json_input_pos);
    if (ret)
    {

        WARNLOG("Incorrect %s/%s line:%d col:%d", path, name, json_input_pos.line_number,
                json_input_pos.column_number);
        goto end;
    }

    if (VALUE_ARRAY != value_type(&root))
    {
        WARNLOG("%s/%s should be array of dictionaries", path, name);
        goto end;
    }
    TbBool ret_ok = process(path, zip, &root);

    value_fini(&root);

    fastUnzClearCache();
    unzClose(zip);

    return ret_ok;
end:
    fastUnzClearCache();
    unzClose(zip);
    return 0;
}

// Forward declaration for internal PNG decoder
static unsigned char* decode_png_to_indexed_internal(unzFile zip, const char *file, const char *path,
                                                      int *out_width, int *out_height,
                                                      unz_file_info64 *zip_info, TbBool use_palette_conversion);

// Helper function to decode PNG from ZIP file to indexed palette format
// Returns indexed data on success, NULL on failure
// Caller must free the returned data
static unsigned char* decode_png_to_indexed(unzFile zip, const char *file, const char *path, 
                                             int *out_width, int *out_height, 
                                             unz_file_info64 *zip_info)
{
    return decode_png_to_indexed_internal(zip, file, path, out_width, out_height, zip_info, true);
}

static unsigned char* decode_png_to_indexed_no_palette(unzFile zip, const char *file, const char *path,
                                                        int *out_width, int *out_height,
                                                        unz_file_info64 *zip_info)
{
    return decode_png_to_indexed_internal(zip, file, path, out_width, out_height, zip_info, false);
}

static unsigned char* decode_png_to_indexed_internal(unzFile zip, const char *file, const char *path, 
                                             int *out_width, int *out_height, 
                                             unz_file_info64 *zip_info, TbBool use_palette_conversion)
{
    // Only load RGB to palette conversion table if needed for color images
    if (use_palette_conversion) {
        load_rgb_to_pal_table();
    }
    
    if (zip_info->uncompressed_size > 1024 * 1024 * 4)
    {
        WARNLOG("PNG file too large: '%s' in '%s'", file, path);
        return NULL;
    }

    unsigned char *png_buffer = malloc(zip_info->uncompressed_size);
    if (png_buffer == NULL)
    {
        ERRORLOG("Failed to allocate memory for PNG buffer");
        return NULL;
    }

    if (unzReadCurrentFile(zip, png_buffer, zip_info->uncompressed_size) != zip_info->uncompressed_size)
    {
        WARNLOG("Failed to read '%s' from '%s'", file, path);
        free(png_buffer);
        return NULL;
    }

    // Decode PNG using spng
    spng_ctx *ctx = spng_ctx_new(0);
    if (ctx == NULL)
    {
        ERRORLOG("Failed to create spng context");
        free(png_buffer);
        return NULL;
    }

    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);
    size_t limit = 1024 * 1024 * 4;
    spng_set_chunk_limits(ctx, limit, limit);

    if (spng_set_png_buffer(ctx, png_buffer, zip_info->uncompressed_size))
    {
        ERRORLOG("Failed to set PNG buffer for '%s'", file);
        spng_ctx_free(ctx);
        free(png_buffer);
        return NULL;
    }

    struct spng_ihdr ihdr;
    int r = spng_get_ihdr(ctx, &ihdr);
    if (r)
    {
        ERRORLOG("spng_get_ihdr() error: %s for '%s'", spng_strerror(r), file);
        spng_ctx_free(ctx);
        free(png_buffer);
        return NULL;
    }

    if (ihdr.width <= 0 || ihdr.height <= 0 || ihdr.width > 4096 || ihdr.height > 4096)
    {
        WARNLOG("Invalid image dimensions (%dx%d) in '%s'", ihdr.width, ihdr.height, file);
        spng_ctx_free(ctx);
        free(png_buffer);
        return NULL;
    }

    // Decode to RGBA8
    size_t out_size;
    int fmt = SPNG_FMT_RGBA8;
    if (spng_decoded_image_size(ctx, fmt, &out_size))
    {
        ERRORLOG("Failed to get decoded image size for '%s'", file);
        spng_ctx_free(ctx);
        free(png_buffer);
        return NULL;
    }

    unsigned char *rgba_buffer = malloc(out_size);
    if (rgba_buffer == NULL)
    {
        ERRORLOG("Failed to allocate memory for decoded image");
        spng_ctx_free(ctx);
        free(png_buffer);
        return NULL;
    }

    if (spng_decode_image(ctx, rgba_buffer, out_size, fmt, SPNG_DECODE_TRNS))
    {
        ERRORLOG("Failed to decode PNG '%s'", file);
        free(rgba_buffer);
        spng_ctx_free(ctx);
        free(png_buffer);
        return NULL;
    }

    spng_ctx_free(ctx);
    free(png_buffer);

    // Convert RGBA to indexed palette format
    size_t indexed_size = ihdr.width * ihdr.height;
    unsigned char *indexed_data = malloc(indexed_size);
    if (indexed_data == NULL)
    {
        ERRORLOG("Failed to allocate memory for indexed image data");
        free(rgba_buffer);
        return NULL;
    }



    // Convert RGBA to palette indices using rgb_to_pal_table
    int transparent_count = 0, opaque_count = 0;
    for (size_t i = 0; i < indexed_size; i++)
    {
        unsigned char red = rgba_buffer[i * 4 + 0];
        unsigned char green = rgba_buffer[i * 4 + 1];
        unsigned char blue = rgba_buffer[i * 4 + 2];
        unsigned char alpha = rgba_buffer[i * 4 + 3];
        
        if (use_palette_conversion)
        {
            // Color image - convert RGB to palette index
            if (alpha < 128) {
                // Transparent pixel - use palette index 255 as transparency marker
                indexed_data[i] = 255;
                transparent_count++;
            } else if (rgb_to_pal_table != NULL) {
                // Use lookup table for color conversion
                indexed_data[i] = rgb_to_pal_table[
                    ((red >> 2) << 12) | ((green >> 2) << 6) | (blue >> 2)
                ];
                opaque_count++;
            } else {
                // Fallback: simple grayscale conversion
                indexed_data[i] = (red + green + blue) / 3;
                opaque_count++;
            }
        }
        else
        {
            // Data image (mist/displacement) - preserve grayscale values as-is
            // Use the red channel as the index value (assuming grayscale PNG)
            indexed_data[i] = red;
        }
    }

    free(rgba_buffer);

    *out_width = ihdr.width;
    *out_height = ihdr.height;
    return indexed_data;
}

static int process_lens_overlay_from_list(const char *path, unzFile zip, int idx, VALUE *root)
{
    VALUE *val;

    val = value_dict_get(root, "name");
    if (val == NULL)
    {
        WARNLOG("Invalid lens overlay %s/lenses.json[%d]: no \"name\" key", path, idx);
        return 0;
    }
    const char *name = value_string(val);
    SYNCDBG(2, "found lens overlay: '%s/%s'", path, name);

    VALUE *file_value = value_dict_get(root, "file");
    if (file_value == NULL)
    {
        WARNLOG("Invalid lens overlay %s/lenses.json[%d]: no \"file\" key", path, idx);
        return 0;
    }

    const char *file = NULL;
    if (value_type(file_value) == VALUE_STRING)
    {
        file = value_string(file_value);
    }
    else if (value_type(file_value) == VALUE_ARRAY && value_array_size(file_value) > 0)
    {
        file = value_string(value_array_get(file_value, 0));
    }
    else
    {
        WARNLOG("Invalid lens overlay %s/lenses.json[%d]: invalid \"file\" value", path, idx);
        return 0;
    }

    if (fastUnzLocateFile(zip, file, 0))
    {
        WARNLOG("File '%s' not found in '%s'", file, path);
        return 0;
    }

    unz_file_info64 zip_info = {0};
    if (UNZ_OK != unzGetCurrentFileInfo64(zip, &zip_info, NULL, 0, NULL, 0, NULL, 0))
    {
        WARNLOG("Failed to get file info for '%s' in '%s'", file, path);
        return 0;
    }

    if (UNZ_OK != unzOpenCurrentFile(zip))
    {
        return 0;
    }

    // Check if this is a RAW file (256x256 = 65536 bytes)
    const size_t raw_size = 256 * 256;
    const char *ext = strrchr(file, '.');
    TbBool is_raw = (zip_info.uncompressed_size == raw_size) && ext && (strcasecmp(ext, ".raw") == 0);

    if (is_raw && zip_info.uncompressed_size == raw_size)
    {
        // Load RAW format directly (256x256 8-bit indexed palette data)
        unsigned char *indexed_data = malloc(raw_size);
        if (indexed_data == NULL)
        {
            ERRORLOG("Failed to allocate memory for RAW overlay");
            unzCloseCurrentFile(zip);
            return 0;
        }

        if (unzReadCurrentFile(zip, indexed_data, raw_size) != raw_size)
        {
            WARNLOG("Failed to read RAW file '%s' from '%s'", file, path);
            free(indexed_data);
            unzCloseCurrentFile(zip);
            return 0;
        }

        unzCloseCurrentFile(zip);

        // Check if overlay with this name already exists
        struct LensOverlayData *existing = NULL;
        for (int i = 0; i < num_added_lens_overlays; i++)
        {
            if (strcasecmp(added_lens_overlays[i].name, name) == 0)
            {
                existing = &added_lens_overlays[i];
                break;
            }
        }

        if (existing)
        {
            // Override existing overlay
            free(existing->data);
            existing->data = indexed_data;
            existing->width = 256;
            existing->height = 256;
        }
        else
        {
            // Add new overlay
            if (num_added_lens_overlays >= MAX_LENS_OVERLAYS)
            {
                ERRORLOG("Too many lens overlays (max %d)", MAX_LENS_OVERLAYS);
                free(indexed_data);
                return 0;
            }

            added_lens_overlays[num_added_lens_overlays].name = strdup(name);
            added_lens_overlays[num_added_lens_overlays].data = indexed_data;
            added_lens_overlays[num_added_lens_overlays].width = 256;
            added_lens_overlays[num_added_lens_overlays].height = 256;
            num_added_lens_overlays++;
            SYNCDBG(8, "Added RAW lens overlay '%s' (256x256)", name);
        }

        return 1;
    }

    // PNG format handling - use shared helper
    int width, height;
    unsigned char *indexed_data = decode_png_to_indexed(zip, file, path, &width, &height, &zip_info);
    if (indexed_data == NULL)
    {
        WARNLOG("Failed to decode PNG '%s' from '%s'", file, path);
        return 0;
    }

    if (width <= 0 || height <= 0 || width > 4096 || height > 4096)
    {
        WARNLOG("Invalid lens overlay dimensions (%dx%d) in '%s'", width, height, file);
        free(indexed_data);
        return 0;
    }

    // Check if overlay with this name already exists
    struct LensOverlayData *existing = NULL;
    for (int i = 0; i < num_added_lens_overlays; i++)
    {
        if (strcasecmp(added_lens_overlays[i].name, name) == 0)
        {
            existing = &added_lens_overlays[i];
            break;
        }
    }

    if (existing)
    {
        // Override existing overlay
        free(existing->data);
        existing->data = indexed_data;
        existing->width = width;
        existing->height = height;
    }
    else
    {
        // Add new overlay
        if (num_added_lens_overlays >= MAX_LENS_OVERLAYS)
        {
            ERRORLOG("Too many lens overlays (max %d)", MAX_LENS_OVERLAYS);
            free(indexed_data);
            return 0;
        }
        
        added_lens_overlays[num_added_lens_overlays].name = strdup(name);
        added_lens_overlays[num_added_lens_overlays].data = indexed_data;
        added_lens_overlays[num_added_lens_overlays].width = width;
        added_lens_overlays[num_added_lens_overlays].height = height;
        num_added_lens_overlays++;
        SYNCDBG(8, "Added PNG lens overlay '%s' (%dx%d)", name, width, height);
    }

    return 1;
}

static int process_icon_from_list(const char *path, unzFile zip, int idx, VALUE *root)
{
    VALUE *val;

    val = value_dict_get(root, "name");
    if (val == NULL)
    {
        WARNLOG("Invalid sprite %s/icons.json[%d]: no \"name\" key", path, idx);
        return 0;
    }
    const char *name = value_string(val);
    SYNCDBG(2, "found icon: '%s/%s'", path,name);

    TbBool is_lowres = (lbDisplay.PhysicalScreenWidth <= LOWRES_SCREEN_SIZE);
    const char *file_key = is_lowres ? "lowres" : "file";

    VALUE *file_value = value_dict_get(root, file_key);
    if ((file_value == NULL) && (is_lowres))
    {
        WARNLOG("No lowres icons for '%s' in '%s'", name, path);
        //no lowres -> use hires
        file_value = value_dict_get(root, "file");
    }

    if (value_type(file_value) == VALUE_STRING)
    {
        // convert "String" to ["String"]
        char *tmp = strdup(value_string(file_value));
        value_init_array(file_value);
        value_init_string(value_array_append(file_value), tmp);
        free(tmp);
    }
    else if (value_type(file_value) != VALUE_ARRAY)
    {
        WARNLOG("Invalid sprite %s/icons.json[%d]: invalid value for %s", path, idx, file_key);
        return 0;
    }

    int first_icon = 0;
    int icons_count = value_array_size(file_value);
    for (int i = 0; i < icons_count; i++)
    {
        const char *file = value_string(value_array_get(file_value, i));


        if (fastUnzLocateFile(zip, file, 0))
        {
            WARNLOG("Png '%s' not found in '%s'", file, path);
            return 0;
        }
        if (UNZ_OK != unzOpenCurrentFile(zip))
        {
            return 0;
        }

        int icon;
        if (!read_png_icon(zip, path, file, &icon))
        {
            unzCloseCurrentFile(zip);
            return 0;
        }
        if (first_icon == 0)
            first_icon = icon;

        if (UNZ_OK != unzCloseCurrentFile(zip))
        {
            return 0;
        }
    }

    struct NamedCommand key = {name, 0};
    struct NamedCommand *spr = bsearch(&key, added_icons, num_added_icons, sizeof(added_icons[0]),
                                       &cmp_named_command);
    if (spr)
    {
        spr->num = first_icon;
        JUSTLOG("Overriding icon '%s/%s'", path,name);
    }
    else
    {
        if (num_added_icons >= GUI_PANEL_SPRITES_NEW)
        {
            ERRORLOG("Too many custom icons");
            return 0;
        }
        spr = &added_icons[num_added_icons++];
        spr->name = strdup(name);
        spr->num = first_icon;
    }

    return 1;
}

static TbBool process_lens_overlay(const char *path, unzFile zip, VALUE *root)
{
    int array_size = value_array_size(root);
    TbBool ret_ok = true;
    for (int i = 0; i < array_size; i++)
    {
        VALUE *val = value_array_get(root, i);
        if (!process_lens_overlay_from_list(path, zip, i, val))
        {
            ret_ok = false;
            continue;
        }
    }
    return ret_ok;
}

static int process_lens_mist_from_list(const char *path, unzFile zip, int idx, VALUE *root)
{
    VALUE *val;

    val = value_dict_get(root, "name");
    if (val == NULL)
    {
        WARNLOG("Invalid lens mist %s/mists.json[%d]: no \"name\" key", path, idx);
        return 0;
    }
    const char *name = value_string(val);
    SYNCDBG(2, "found lens mist: '%s/%s'", path, name);

    VALUE *file_value = value_dict_get(root, "file");
    if (file_value == NULL)
    {
        WARNLOG("Invalid lens mist %s/mists.json[%d]: no \"file\" key", path, idx);
        return 0;
    }

    const char *file = NULL;
    if (value_type(file_value) == VALUE_STRING)
    {
        file = value_string(file_value);
    }
    else if (value_type(file_value) == VALUE_ARRAY && value_array_size(file_value) > 0)
    {
        file = value_string(value_array_get(file_value, 0));
    }
    else
    {
        WARNLOG("Invalid lens mist %s/mists.json[%d]: invalid \"file\" value", path, idx);
        return 0;
    }

    if (fastUnzLocateFile(zip, file, 0))
    {
        WARNLOG("File '%s' not found in '%s'", file, path);
        return 0;
    }

    unz_file_info64 zip_info = {0};
    if (UNZ_OK != unzGetCurrentFileInfo64(zip, &zip_info, NULL, 0, NULL, 0, NULL, 0))
    {
        WARNLOG("Failed to get file info for '%s' in '%s'", file, path);
        return 0;
    }

    if (UNZ_OK != unzOpenCurrentFile(zip))
    {
        return 0;
    }

    unsigned char *mist_data = NULL;
    const size_t mist_size = 256 * 256;

    // Try PNG format first
    if (zip_info.uncompressed_size != mist_size)
    {
        // Not RAW format, try PNG
        int width, height;
        mist_data = decode_png_to_indexed_no_palette(zip, file, path, &width, &height, &zip_info);
        if (mist_data == NULL)
        {
            // Already closed by decode function on failure
            return 0;
        }

        // Validate mist dimensions (must be 256x256)
        if (width != 256 || height != 256)
        {
            WARNLOG("Invalid mist dimensions for '%s' in '%s': expected 256x256, got %dx%d", 
                    file, path, width, height);
            free(mist_data);
            return 0;
        }

        SYNCDBG(7, "Loaded PNG mist '%s' from '%s'", file, path);
    }
    else
    {
        // RAW format (256x256 = 65536 bytes)
        mist_data = malloc(mist_size);
        if (mist_data == NULL)
        {
            ERRORLOG("Failed to allocate memory for mist data");
            unzCloseCurrentFile(zip);
            return 0;
        }

        if (unzReadCurrentFile(zip, mist_data, mist_size) != mist_size)
        {
            WARNLOG("Failed to read mist file '%s' from '%s'", file, path);
            free(mist_data);
            unzCloseCurrentFile(zip);
            return 0;
        }

        unzCloseCurrentFile(zip);
        SYNCDBG(7, "Loaded RAW mist '%s' from '%s'", file, path);
    }

    // Check if mist with this name already exists
    struct LensMistData *existing = NULL;
    for (int i = 0; i < num_added_lens_mists; i++)
    {
        if (strcasecmp(added_lens_mists[i].name, name) == 0)
        {
            existing = &added_lens_mists[i];
            break;
        }
    }

    if (existing)
    {
        // Override existing mist
        free(existing->data);
        existing->data = mist_data;
        JUSTLOG("Overriding lens mist '%s/%s'", path, name);
    }
    else
    {
        // Add new mist
        if (num_added_lens_mists >= MAX_LENS_MISTS)
        {
            ERRORLOG("Too many lens mists (max %d)", MAX_LENS_MISTS);
            free(mist_data);
            return 0;
        }

        added_lens_mists[num_added_lens_mists].name = strdup(name);
        added_lens_mists[num_added_lens_mists].data = mist_data;
        num_added_lens_mists++;
        SYNCDBG(8, "Added lens mist '%s' (256x256)", name);
    }

    return 1;
}

static TbBool process_lens_mist(const char *path, unzFile zip, VALUE *root)
{
    TbBool ret_ok = true;
    for (int i = 0; i < value_array_size(root); i++)
    {
        VALUE *val = value_array_get(root, i);
        if (!process_lens_mist_from_list(path, zip, i, val))
        {
            ret_ok = false;
            continue;
        }
    }
    return ret_ok;
}

static TbBool process_icon(const char *path, unzFile zip, VALUE *root)
{
    TbBool ret_ok = true;
    for (int i = 0; i < value_array_size(root); i++)
    {
        VALUE *val = value_array_get(root, i);
        if (!process_icon_from_list(path, zip, i, val))
        {
            ret_ok = false;
            continue;
        }
    }

    qsort(added_icons, num_added_icons, sizeof(added_icons[0]), &cmp_named_command);
    return ret_ok;
}

static TbBool process_sprite(const char *path, unzFile zip, VALUE *root)
{
    TbBool ret_ok = true;
    for (int i = 0; i < value_array_size(root); i++)
    {
        VALUE *val = value_array_get(root, i);
        if (!process_sprite_from_list(path, zip, i, val))
        {
            ret_ok = false;
            continue;
        }
    }

    qsort(added_sprites, num_added_sprite, sizeof(added_sprites[0]), &cmp_named_command);
    return ret_ok;
}

static TbBool add_custom_sprite(const char *path)
{
    return add_custom_json(path, "sprites.json", &process_sprite);
}

short get_icon_id(const char *name)
{
    short ret = atoi(name);
    struct NamedCommand key = {name, 0};

    if (ret != 0)
        return ret;

    struct NamedCommand *val = bsearch(&key, added_icons, num_added_icons, sizeof(added_icons[0]),
                                       &cmp_named_command);
    if (val)
        return (short) val->num;

    if (0 == strcmp(name, "0"))
        return 0;

    return bad_icon_id; // -1 is used by SPELLBOOK_POSS etc
}

short get_anim_id(const char *name, struct ObjectConfigStats *objst)
{
    short ret = atoi(name);
    struct NamedCommand key = {name, 0};

    if (ret > 0)
        return ret;

    struct NamedCommand *val = bsearch(&key, added_sprites, num_added_sprite, sizeof(added_sprites[0]),
                                       &cmp_named_command);
    if (val)
        return (short) val->num;

    if (0 == strcmp(name, "0"))
        return 0;

    char *P = strrchr(name, ':');
    if (P != NULL)
    {
        char *name2 = strdup(name);
        P = strchr(name2, ':');
        *P = 0; // removing :
        P++;
        key.name = name2;

        val = bsearch(&key, added_sprites, num_added_sprite, sizeof(added_sprites[0]),
                      &cmp_named_command);
        if (!val)
        {
            ERRORLOG("Unable to find sprite %s", name);
            free(name2);
            return 0;
        }
        if (0 == strcmp(P, "NORTH"))
        {
            objst->rotation_flag = 0;
        }
        else if (0 == strcmp(P, "NORTHEAST"))
        {
            objst->rotation_flag = 1;
        }
        else if (0 == strcmp(P, "EAST"))
        {
            objst->rotation_flag = 2;
        }
        else if (0 == strcmp(P, "SOUTHEAST"))
        {
            objst->rotation_flag = 3;
        }
        else if (0 == strcmp(P, "SOUTH"))
        {
            objst->rotation_flag = 4;
        }
        else if (0 == strcmp(P, "SOUTHWEST"))
        {
            objst->rotation_flag = 5;
        }
        else if (0 == strcmp(P, "WEST"))
        {
            objst->rotation_flag = 6;
        }
        else if (0 == strcmp(P, "NORTHWEST"))
        {
            objst->rotation_flag = 7;
        }
        else
        {
            ERRORLOG("Unexpected Anim direction: %s", P);
        }

        free(name2);
        return (short) val->num;
    }
    return 0;
}

short get_anim_id_(const char* word_buf)
{
    struct ObjectConfigStats obj_tmp;
    return get_anim_id(word_buf, &obj_tmp);
}

const struct TbSprite *get_button_sprite_for_player(short sprite_idx, PlayerNumber plyr_idx)
{
    return get_button_sprite(get_player_colored_button_sprite_idx(sprite_idx, plyr_idx));
}

const struct TbSprite *get_button_sprite(short sprite_idx)
{
    if ((sprite_idx >= 0) && (sprite_idx < GUI_BUTTON_SPRITES_COUNT)) {
        return get_sprite(button_sprites, sprite_idx);
    }
    sprite_idx -= GUI_PANEL_SPRITES_COUNT;
    if ((sprite_idx >= 0) && (sprite_idx < num_sprites(custom_sprites))) {
        return get_sprite(custom_sprites, sprite_idx);
    }
    return &bad_icon;
}

const struct TbSprite *get_frontend_sprite(short sprite_idx)
{
    if ((sprite_idx >= 0) && (sprite_idx < num_sprites(frontend_sprite))) {
        return get_sprite(frontend_sprite, sprite_idx);
    }
    sprite_idx -= GUI_PANEL_SPRITES_COUNT;
    if ((sprite_idx >= 0) && (sprite_idx < num_sprites(custom_sprites))) {
        return get_sprite(custom_sprites, sprite_idx);
    }
    return &bad_icon;
}

const struct TbSprite *get_new_icon_sprite(short sprite_idx)
{
    sprite_idx -= GUI_PANEL_SPRITES_COUNT;
    if ((sprite_idx >= 0) && (sprite_idx < num_sprites(custom_sprites))) {
        return get_sprite(custom_sprites, sprite_idx);
    }
    return &bad_icon;
}

const struct TbSprite *get_panel_sprite(short sprite_idx)
{
    if ((sprite_idx >= 0) && (sprite_idx < num_sprites(gui_panel_sprites))) {
        return get_sprite(gui_panel_sprites, sprite_idx);
    }
    sprite_idx -= GUI_PANEL_SPRITES_COUNT;
    if ((sprite_idx >= 0) && (sprite_idx < num_sprites(custom_sprites))) {
        return get_sprite(custom_sprites, sprite_idx);
    }
    return &bad_icon;
}

int is_custom_icon(short icon_idx)
{
    icon_idx -= GUI_PANEL_SPRITES_COUNT;
    return (icon_idx >= 0) && (icon_idx < num_sprites(custom_sprites));
}

const struct LensOverlayData* get_lens_overlay_data(const char *name)
{
    if (name == NULL || name[0] == '\0')
        return NULL;

    for (int i = 0; i < num_added_lens_overlays; i++)
    {
        if (strcasecmp(added_lens_overlays[i].name, name) == 0)
        {
            return &added_lens_overlays[i];
        }
    }
    return NULL;
}

const struct LensMistData* get_lens_mist_data(const char *name)
{
    if (name == NULL || name[0] == '\0')
        return NULL;

    for (int i = 0; i < num_added_lens_mists; i++)
    {
        if (strcasecmp(added_lens_mists[i].name, name) == 0)
        {
            return &added_lens_mists[i];
        }
    }
    return NULL;
}
