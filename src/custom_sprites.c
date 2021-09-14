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


#include "custom_sprites.h"
#include "creature_graphics.h"
#include "front_simple.h"
#include "engine_render.h"
#include "../deps/zlib/contrib/minizip/unzip.h"
#include "bflib_fileio.h"

#include <spng.h>
#include <json.h>
#include <json-dom.h>

// Each part of RGB tuple of palette file is 1-63 actually
#define MAX_COLOR_VALUE 64

static short next_free_sprite = 0;

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

struct PaletteRecord
{
    uint32_t color;
    unsigned char color_idx;
};

struct PaletteNode
{
    struct PaletteRecord *rec;
    int size;
};

static struct PaletteRecord pal_records[PALETTE_COLORS]; // for each color of a palette
static struct PaletteNode pal_tree[MAX_COLOR_VALUE]; // For each component of a palette
static struct NamedCommand added_sprites[KEEPERSPRITE_ADD_NUM];
static int num_added_sprite = 0;

static void init_pal_conversion();

static void compress_raw(struct TbHugeSprite *sprite, unsigned char *src_buf, int x, int y, int w, int h);

static TbBool add_custom_sprite(const char *path);

static int cmp_named_command(const void *a, const void *b);

static int pal_compare_fn(const void *a, const void *b)
{
    const struct PaletteRecord *rec_a = a;
    const struct PaletteRecord *rec_b = b;
    // FYI: minimal density is G axis (15)
    long delta = (rec_a->color & 0x00FF00) - (rec_b->color & 0x00FF00);
    if (delta != 0)  //G first
        return delta;
    delta = (rec_a->color & 0xFF0000) - (rec_b->color & 0xFF0000);
    if (delta != 0)
        return delta;
    return (rec_a->color & 0x0000FF) - (rec_b->color & 0x0000FF);
}

static int cmp_named_command(const void *a, const void *b)
{

    const struct NamedCommand *val_a = a;
    const struct NamedCommand *val_b = b;
    return strcmp(val_a->name, val_b->name);
}

static void load_system_sprites(short fgroup)
{
    struct TbFileFind fileinfo;
    int cnt = 0, cnt_ok = 0;
    char *fname = prepare_file_path(fgroup, "*.zip");
    if (0 == *fname) // No campaign
        return;
    for (int rc = LbFileFindFirst(fname, &fileinfo, 0x21u);
         rc != -1;
         rc = LbFileFindNext(&fileinfo))
    {
        if (add_custom_sprite(prepare_file_path(fgroup, fileinfo.Filename)))
        {
            cnt_ok++;
        }
        cnt++;
    }
    LbJustLog("Found %d sprite zip files, properly loaded %d.\n", cnt, cnt_ok);
}

void init_custom_sprites(LevelNumber lvnum)
{
    for (int i = 0; i < KEEPERSPRITE_ADD_NUM; i++)
    {
        if (keepersprite_add[i] != NULL)
        {
            free(keepersprite_add[i]);
            keepersprite_add[i] = NULL;
        }
    }
    for (int i = 0; i < num_added_sprite; i++)
    {
        if (added_sprites[i].name != NULL)
        {
            free((char *) added_sprites[i].name);
        }
    }
    num_added_sprite = 0;
    memset(added_sprites, 0, sizeof(added_sprites));

    memset(creature_table_add, 0, sizeof(creature_table_add));
    next_free_sprite = 0;
    if (anim_names != NULL)
    {
        free(anim_names);
    }

    init_pal_conversion();
    load_system_sprites(FGrp_FxData);
    load_system_sprites(FGrp_CmpgConfig);

    char *lvl = prepare_file_fmtpath(get_level_fgroup(lvnum), "map%05lu.zip", lvnum);
    if (add_custom_sprite(lvl))
    {
        SYNCDBG(0, "Loaded per-map sprite file");
    }
    else
    {
        SYNCDBG(0, "Unable to load per-map sprite file");
    }
}

/**
 * Setup data for rgb -> indexed conversion
 */
static void init_pal_conversion()
{
    // 1. Loading palette into pal_records
    memset(pal_records, 0, sizeof(pal_records));

    struct PaletteNode pal_tree_tmp[MAX_COLOR_VALUE] = {0}; // one color

    unsigned char *pal = engine_palette;
    for (int i = 0; i < PALETTE_COLORS; i++)
    {
        if ((pal[i * 3 + 0] > MAX_COLOR_VALUE)
            || (pal[i * 3 + 1] > MAX_COLOR_VALUE)
            || (pal[i * 3 + 2] > MAX_COLOR_VALUE)
                )
        {
            WARNLOG("Unexpected: palette file records is out of range");
        }
        pal_records[i].color = pal[i * 3 + 0] | (pal[i * 3 + 1] << 8) | (pal[i * 3 + 2] << 16);
        pal_records[i].color_idx = i;
    }
    // 2. Sorting by color
    qsort(pal_records, PALETTE_COLORS, sizeof(pal_records[0]), &pal_compare_fn);
    // 3. setting up tree
    for (int i = 0; i < PALETTE_COLORS; i++)
    {
        int idx = (pal_records[i].color & 0x00FF00) >> 8;
        struct PaletteNode *node = &pal_tree_tmp[idx];
        if (node->rec == NULL)
        {
            node->rec = &pal_records[i];
        }
        node->size++;
    }
    // 4. Expanding borders
#define NEAREST_DEPTH 5
    for (int i = 0; i < MAX_COLOR_VALUE; i++)
    {
        pal_tree[i].rec = NULL;
        pal_tree[i].size = 0;
        for (int j = 0; j < NEAREST_DEPTH; j++)
        {
            int k = i + j - NEAREST_DEPTH / 2;
            if ((k < 0) || (k >= MAX_COLOR_VALUE))
                continue;
            if (pal_tree[i].rec == NULL)
            {
                pal_tree[i].rec = pal_tree_tmp[k].rec;
            }
            pal_tree[i].size += pal_tree_tmp[k].size;
        }
    }
#undef NEAREST_DEPTH
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

    unsigned char *dst_buf = scratch;
    spng_decode_image(ctx, dst_buf, out_size, fmt, SPNG_DECODE_TRNS);

    // This should be enough except rare cases like transparent checkerboard
    int dst_w = (int) context->sprite.SWidth;
    int dst_h = (int) context->sprite.SHeight;

    if (dst_w >= 255 || dst_h >= 255)
    {
        ERRORLOG("Sprites more than 255x255 are not supported");
        return 1;
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
    ksprite->FrameWidth = dst_w;
    ksprite->FrameHeight = dst_h;
    ksprite->Rotable = context->rotatable ? 2 : 0;
    ksprite->FramesCount = 1;

    VALUE *val;
    val = value_dict_get(itm, "offset_w");
    if (value_type(val) != VALUE_NULL)
    {
        ksprite->FrameOffsW = value_uint32(val);
    }
    else if (val = value_dict_get(def, fp ? "fp_offset_w" : "td_offset_w"),
            value_type(val) != VALUE_NULL
            )
    {
        ksprite->FrameOffsW = value_uint32(val);
    }
    else
    {
        ksprite->FrameOffsW = 0;
    }

    val = value_dict_get(itm, "offset_h");
    if (value_type(val) != VALUE_NULL)
    {
        ksprite->FrameOffsH = value_uint32(val);
    }
    else if (val = value_dict_get(def, fp ? "fp_offset_h" : "td_offset_h"),
            value_type(val) != VALUE_NULL
            )
    {
        ksprite->FrameOffsH = value_uint32(val);
    }
    else
    {
        ksprite->FrameOffsH = 0;
    }

    val = value_dict_get(itm, "offset_x");
    if (value_type(val) != VALUE_NULL)
    {
        ksprite->field_C = value_int32(val);
    }
    else if (val = value_dict_get(def, fp ? "fp_offset_x" : "td_offset_x"),
            value_type(val) != VALUE_NULL
            )
    {
        ksprite->field_C = value_int32(val);
    }
    else
    {
        ksprite->field_C = -dst_w / 2;
    }

    val = value_dict_get(itm, "offset_y");
    if (value_type(val) != VALUE_NULL)
    {
        ksprite->field_E = value_int32(val);
    }
    else if (val = value_dict_get(def, fp ? "fp_offset_y" : "td_offset_y"),
            value_type(val) != VALUE_NULL
            )
    {
        ksprite->field_E = value_int32(val);
    }
    else
    {
        ksprite->field_E = 1 - dst_h;
    }

    spng_ctx_free(ctx);
    return 1;
}

static void convert_row(unsigned char *dst_buf, uint32_t *src_buf, int len)
{
#define SCALE 4
    for (int i = 0; i < len; i++, src_buf++, dst_buf++)
    {
        uint32_t data = *src_buf;
        int idx = ((data & 0x00FF00) >> 8) / SCALE;
        const struct PaletteNode *node = &pal_tree[idx];
        uint8_t max_val = 255;
        uint32_t max_dst = 3 * 64 * 64;

        for (struct PaletteRecord *rec = node->rec; rec != node->rec + node->size; rec++)
        {
            int8_t dr = (rec->color & 0x00000FF) - (data & 0x0000FF) / SCALE;
            int8_t dg = ((rec->color & 0xFF00) >> 8) - ((data & 0xFF00) >> 8) / SCALE;
            int8_t db = ((rec->color & 0xFF0000) >> 16) - ((data & 0xFF0000) >> 16) / SCALE;
            if (dr * dr + dg * dg + db * db < max_dst)
            {
                max_dst = dr * dr + dg * dg + db * db;
                max_val = rec->color_idx;
            }
        }
        *dst_buf = max_val;
    }
#undef SCALE
}

static void compress_raw(struct TbHugeSprite *sprite, unsigned char *inp_buf, int x, int y, int w, int h)
{
#define TEST_TRANSP(x) ((x & 0xFF000000u) < 0x40000000u)

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
}

struct StrBuf
{
    char *ptr;
    size_t size;
};

static int dump_callback(const char *str, size_t size, void *user_data)
{
    struct StrBuf *buf = user_data;
    buf->ptr = realloc(buf->ptr, buf->size + size + 1);
    memcpy(buf->ptr + buf->size, str, size);
    buf->size += size;
    buf->ptr[buf->size] = 0;
    return 0;
}

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

#if BFDEBUG_LEVEL > 0
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
            context->id_ptr = &context->td_id;
            context->id_sz_ptr = &context->td_sz;
        }
        else
        {
            ud_lst = value_dict_get(node, "fp");
            context->id_ptr = &context->fp_id;
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

                if (unzLocateFile(zip, name, 0))
                {
                    WARNLOG("Png '%s' not found in '%s'", name, path);
                    return 1;
                }
                if (UNZ_OK != unzOpenCurrentFile(zip))
                {
                    return 1;
                }
                read_png_data(zip, path, context, name, fp, node, itm);
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
        ERRORLOG("Should have same amount of TD and FP frames");
    }
    for (int i = context->td_sz - 1; i >= 0; i--)
    {
        short fp_id = context->fp_id + i;
        short td_id = context->td_id + i;
        td_iso_add[fp_id - KEEPERSPRITE_ADD_OFFSET] = td_id;
        iso_td_add[fp_id - KEEPERSPRITE_ADD_OFFSET] = fp_id;
        iso_td_add[td_id - KEEPERSPRITE_ADD_OFFSET] = fp_id;
        td_iso_add[td_id - KEEPERSPRITE_ADD_OFFSET] = td_id;
    }
    return 0;
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
    WARNDBG(2, "found sprite: %s", name);
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
        JUSTLOG("Overriding sprite '%s'", name);
    }
    else
    {
        spr = &added_sprites[num_added_sprite++];
        spr->name = strdup(name);
        spr->num = context.td_id;
    }

    return 1;
}

static TbBool add_custom_sprite(const char *path)
{
    unz_file_info64 zip_info = {0};
    VALUE sprites_root;
    JSON_INPUT_POS json_input_pos;
    unzFile zip = unzOpen(path);

    if (zip == NULL)
        return 0;

    if (UNZ_OK != unzLocateFile(zip, "sprites.json", 0))
    {
        unzClose(zip);
        return 0;
    }

    if (UNZ_OK != unzGetCurrentFileInfo64(zip, &zip_info, NULL, 0, NULL, 0, NULL, 0)
            )
    {
        unzClose(zip);
        return 0;
    }

    if (zip_info.uncompressed_size >= 1024 * 1024)
    {
        WARNLOG("File too big %s/sprites.json", path);
        unzClose(zip);
        return 0;
    }

    if (UNZ_OK != unzOpenCurrentFile(zip))
    {
        unzClose(zip);
        return 0;
    }

    if (unzReadCurrentFile(zip, scratch, zip_info.uncompressed_size) != zip_info.uncompressed_size)
    {
        WARNLOG("Unable to read %s/sprites.json", path);
        unzClose(zip);
        return 0;
    }
    scratch[zip_info.uncompressed_size] = 0;

    if (UNZ_OK != unzCloseCurrentFile(zip))
    {
        unzClose(zip);
        return 0;
    }

    int ret = json_dom_parse((char *) scratch, zip_info.uncompressed_size, NULL, 0, &sprites_root, &json_input_pos);
    if (ret)
    {

        WARNLOG("Incorrect %s/sprites.json line:%d col:%d", path, json_input_pos.line_number,
                json_input_pos.column_number);
        unzClose(zip);
        return 0;
    }

    if (VALUE_ARRAY != value_type(&sprites_root))
    {
        WARNLOG("%s/sprites.json should be array of dictionaries", path);
        unzClose(zip);
        return 0;
    }
    for (int i = 0; i < value_array_size(&sprites_root); i++)
    {
        VALUE *val = value_array_get(&sprites_root, i);
        if (!process_sprite_from_list(path, zip, i, val))
        {
            continue;
        }
    }

    value_fini(&sprites_root);

    unzClose(zip);

    qsort(added_sprites, num_added_sprite, sizeof(added_sprites[0]), &cmp_named_command);

    return 1;
}

short get_anim_id(const char *name)
{
    short ret = atoi(name);
    struct NamedCommand key = {name, 0};

    if (ret > 0)
        return ret;

    struct NamedCommand *val = bsearch(&key, added_sprites, num_added_sprite, sizeof(added_sprites[0]),
                                       &cmp_named_command);
    if (val)
        return val->num;
    return 0;
}