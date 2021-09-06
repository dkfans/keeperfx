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

#include <spng.h>
#include <json.h>
#include <json-dom.h>

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

struct Row
{
    int x, y;
    int w, h;
};

struct SpriteContext
{
    struct TbHugeSprite sprite;

    unsigned long w, h;
    unsigned long x, y;
    struct KeeperSprite *ksp_first;

    short *id_ptr; // First person / Top down
    short *id_sz_ptr; // First person / Top down

    short td_id, td_sz;
    short fp_id, fp_sz;

    unsigned char *img_buf;

    int state; // 0 -> list, 1 -> anim, 2 -> list of images, 3 -> image data
    int cnt;
    TbBool only_one;
};

void clear_custom_sprites()
{
    for (int i = 0; i < KEEPERSPRITE_ADD_NUM; i++)
    {
        if (keepersprite_add[i] != NULL)
        {
            free(keepersprite_add[i]);
            keepersprite_add[i] = NULL;
        }
    }
    memset(creature_table_add, 0, sizeof(creature_table_add));
    next_free_sprite = 0;
    if (anim_names != NULL)
    {
        free(anim_names);
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
 * @return
 */
static int read_png(unzFile zip, const char *path, struct SpriteContext *context, const char *blender_filename,
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

    uint32_t n_text = 0;
    TbBool found = 0;
    long frame_no = 0;

    if (0 != spng_get_text(ctx, NULL, &n_text))
    {
        spng_ctx_free(ctx);
        return 0;
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
                return 0;
            }
            frame_no--;
            found++;
        }
    }

    if (found != 3) // Scene + Camera + Frame
    {
        free(text);
        spng_ctx_free(ctx);
        return 0;
    }

    TbBool dir_type = (0 == strncasecmp(camera, "fp", 2));
    VALUE *td_dir = value_dict_get_or_add(node, dir_type ? "fp" : "td");
    if (value_type(td_dir) == VALUE_NULL)
    {
        value_init_array(td_dir);
    }

    for (int i = value_array_size(td_dir); i < 5; i++)
    {
        value_init_array(value_array_append(td_dir));
    }
    int lr_dir = dir_from_camera_name(camera);
    if (lr_dir < 0)
    {
        WARNLOG("Unknown frame: %s/%s dir:%s ", path, subpath, camera);
        free(text);
        spng_ctx_free(ctx);
        return 0;
    }
    VALUE *arr = value_array_get(td_dir, lr_dir);
    
    if (frame_no >= value_array_size(arr)) // >=
    {
        for (int i = value_array_size(arr); i <= frame_no; i++)
        {
            value_array_insert(arr, i);
        }
    }
    VALUE *dst = value_array_get(arr, frame_no);
    if (value_type(dst) != VALUE_NULL)
    {
        ERRORLOG("Duplicate frame");
    }
    value_init_string(dst, subpath);
    
    free(text);
    spng_ctx_free(ctx);
    return 1;
}

#define TRANSP_COLOR 255

static void compress_raw(struct TbHugeSprite *sprite, unsigned char *src_buf, int x, int y, int w, int h)
{
    unsigned char *buf = sprite->Data;
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
                if ((*src_buf != TRANSP_COLOR) || len == 127)
                {
                    *buf = -len;
                    buf++;
                    len = 1;
                    is_transp = (*src_buf == TRANSP_COLOR);
                }
                else
                {
                    len++;
                }
            }
            else
            {
                if ((*src_buf == TRANSP_COLOR) || len == 127)
                {
                    if (len > 0)
                    {
                        *buf = len;
                        buf++;
                        memcpy(buf, src_buf - len, len);
                        buf += len;
                    }

                    is_transp = (*src_buf == TRANSP_COLOR);
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
            memcpy(buf, src_buf - len, len);
            buf += len;
        }
        *buf = 0;
        buf++;
        src_buf += tail;
    }
}

static int process_json(JSON_TYPE json_type, const char *data, size_t data_size, void *context_)
{
    struct SpriteContext *context = context_;

    switch (json_type)
    {
        case JSON_KEY:
            if (0 == strncmp("fp", data, data_size))
            {
                context->id_ptr = &context->fp_id;
                context->id_sz_ptr = &context->fp_sz;
            }
            else if (0 == strncmp("td", data, data_size))
            {
                context->id_ptr = &context->td_id;
                context->id_sz_ptr = &context->td_sz;
            }
            else
            {
                context->id_ptr = NULL;
            }
            return 0;
        case JSON_ARRAY_BEG:
            if (context->state <= 3)
            {
                context->state++;
                return 0;
            }
            else
                return 1;
        default:
            return 0;
        case JSON_NUMBER:
            if (context->state == 4)
            {
                switch (context->cnt)
                {
                    case 0:
                        context->x = strtol(data, NULL, 10);
                        break;
                    case 1:
                        context->y = strtol(data, NULL, 10);
                        break;
                    case 2:
                        context->w = strtol(data, NULL, 10);
                        break;
                    case 3:
                        context->h = strtol(data, NULL, 10);
                        break;
                    default:
                        return 1;
                }
                context->cnt++;
                return 0;
            }
            // fallthrough
        case JSON_NULL:
        case JSON_FALSE:
        case JSON_TRUE:
            ERRORLOG("Unexpected value");
            return 1;
        case JSON_OBJECT_BEG:
            if (context->state == 0) // Only one object
            {
                context->state = 2;
                context->only_one = 1;
                return 0;
            }
            else if (context->state == 1)
            {
                context->state = 2;
                return 0;
            }
            ERRORLOG("Unexpected value");
            return 1;
        case JSON_OBJECT_END:
            context->state--;
            if (context->state == 1)
            {
                // Object defined
                if (context->fp_sz != context->td_sz)
                {
                    ERRORLOG("Different number of FP and TD frames is not supported");
                    return 1;
                }
                for (short i = 0; i < context->fp_sz; i++)
                {
                    short fp_id = context->fp_id + i;
                    short td_id = context->td_id + i;
                    td_iso_add[fp_id - KEEPERSPRITE_ADD_OFFSET] = td_id;
                    iso_td_add[fp_id - KEEPERSPRITE_ADD_OFFSET] = fp_id;
                    iso_td_add[td_id - KEEPERSPRITE_ADD_OFFSET] = fp_id;
                    td_iso_add[td_id - KEEPERSPRITE_ADD_OFFSET] = td_id;
                }
            }
            return 0;
        case JSON_ARRAY_END:
            context->state--;
            if (context->state == 2)
            {
                if (context->ksp_first == NULL)
                {
                    ERRORLOG("No frames in list");
                    return 1;
                }
                context->ksp_first = NULL;
            }
            if (context->state != 3)
            {
                return 0;
            }
            else
            {
                if (context->cnt < 4)
                {
                    ERRORLOG("[x, y, w, h] expected in list");
                    return 1;
                }
                context->cnt = 0;
            }
    }
    // This should be enough except rare cases like transparent checkerboard
    int dst_w = min(context->sprite.SWidth, context->w);
    int dst_h = min(context->sprite.SHeight, context->h);

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
    compress_raw(&context->sprite, context->img_buf, context->x, context->y, dst_w, dst_h);
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
    ksprite->Rotable = 0; // 2 need more sprite in next slot - not implemented yet
    ksprite->FramesCount = 1;
    ksprite->FrameOffsW = 0;
    ksprite->FrameOffsH = 0;
    ksprite->field_C = -dst_w / 2; // Offset x
    ksprite->field_E = 1 - dst_h; // Offset y

    return 0;
}

struct StrBuf
{
    char *ptr;
    size_t size;
};

static int dump_callback(const char* str, size_t size, void* user_data)
{
    struct StrBuf* buf = user_data;
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
static int collect_sprites(const char *path, unzFile zip, const char *blender_scene, VALUE *node)
{
    char szCurrentFileName[256];
    struct SpriteContext context = {0};
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
        read_png(zip, path, &context, blender_scene, szCurrentFileName, node);
        if (UNZ_OK != unzCloseCurrentFile(zip))
        {
            return 1;
        }
    }

    struct StrBuf buf = {0, 0};

    json_dom_dump(node, &dump_callback, &buf, 2, 0);
    fprintf(stderr, "%s", buf.ptr);
    return 0;
}

static int process_sprite_from_list(const char *path, unzFile zip, int idx, VALUE *root)
{
    VALUE *val;
    val = value_dict_get(root, "name");
    if (val == NULL)
    {
        WARNLOG("Invalid sprite %s/sprites.json[%d]: no \"name\" key", path, idx);
        return 0;
    }
    const char *name = value_string(val);
    WARNDBG(2, "found sprite: %s", name);
    val = value_dict_get(root, "blender_scene");
    if ((val != NULL) && (value_type(val) == VALUE_STRING))
    {
        collect_sprites(path, zip, value_string(val), root);
    }
    return 1;
}

short add_custom_sprite(const char *path)
{
    unz_file_info64 zip_info = {0};
    VALUE sprites_root;
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

    json_dom_parse((char *) scratch, zip_info.uncompressed_size, NULL, 0, &sprites_root, NULL);
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
    return 0;
}