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
#include "../deps/centijson/src/json.h"

#include <spng.h>
#include <json.h>

static short next_free_sprite = 0;

short iso_td_add[KEEPERSPRITE_ADD_NUM];
short td_iso_add[KEEPERSPRITE_ADD_NUM];

TbSpriteData keepersprite_add[KEEPERSPRITE_ADD_NUM] = {
        0
};

struct KeeperSprite creature_table_add[KEEPERSPRITE_ADD_NUM] = {
        {0}
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
}

static unsigned char *read_png(const char *path, struct TbHugeSprite *sprite, const char **desc)
{
    unsigned char *dst_buf;
    int fmt = SPNG_FMT_PNG;
    size_t out_size;
    FILE *F = fopen(path, "rb");
    sprite->SHeight = 0;
    sprite->SWidth = 0;
    if (F == NULL)
    {
        ERRORLOG("Unable to read %s", path);
        return NULL;
    }
    spng_ctx *ctx = NULL;
    ctx = spng_ctx_new(0);
    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    size_t limit = 1024 * 1024 * 2;
    spng_set_chunk_limits(ctx, limit, limit);

    spng_set_png_file(ctx, F);
    struct spng_ihdr ihdr;
    int r = spng_get_ihdr(ctx, &ihdr);

    if (r)
    {
        ERRORLOG("spng_get_ihdr() error: %s", spng_strerror(r));
        fclose(F);
        return NULL;
    }

    if ((ihdr.bit_depth != 8) || (ihdr.color_type != SPNG_COLOR_TYPE_INDEXED))
    {
        ERRORLOG("Wrong spec: %s should be 8bit indexed Png", path);
        fclose(F);
        return NULL;
    }
    struct spng_plte plte = {0};
    r = spng_get_plte(ctx, &plte);
    // TODO: should we check palette?

    sprite->SWidth = ihdr.width;
    sprite->SHeight = ihdr.height;

    spng_decoded_image_size(ctx, fmt, &out_size);
    if (limit < out_size)
    {
        ERRORLOG("Unable to decode %s error: %s", path, spng_strerror(r));
        fclose(F);
        return NULL;
    }

    dst_buf = scratch;

    uint32_t n_text = 0;
    *desc = "";
    if (0 == spng_get_text(ctx, NULL, &n_text))
    {
        struct spng_text *text = malloc(sizeof(struct spng_text) * n_text);
        spng_get_text(ctx, text, &n_text);

        *desc = (char *) dst_buf;
        for (int i = 0; i < n_text; i++)
        {
            if (0 == strcmp(text[i].keyword, "Comment"))
            {
                strcpy((char *) dst_buf, text[i].text);
                dst_buf += text[i].length;
            }
        }
        dst_buf[0] = 0;
        dst_buf++;
        free(text);
    }

    r = spng_decode_image(ctx, dst_buf, out_size, fmt, 0);
    if (r)
    {
        return NULL;
    }

    fclose(F);
    return dst_buf;
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
    int *id_sz_ptr; // First person / Top down

    short td_id, td_sz;
    short fp_id, fp_sz;

    unsigned char *img_buf;

    int state; // 0 -> list, 1 -> anim, 2 -> list of images, 3 -> image data
    int cnt;
    TbBool  only_one;
};

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
            } else if (context->state == 1)
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

short add_custom_sprite(const char *path)
{
    short ret;
    const char *desc;
    struct SpriteContext context = {0};

    context.img_buf = read_png(path, &context.sprite, &desc);
    JSON_PARSER parser;
    JSON_CALLBACKS callbacks = {&process_json};
    if (!context.img_buf)
        return 0;

    json_init(&parser, &callbacks, NULL, &context);
    json_feed(&parser, desc, strlen(desc));
    if (json_fini(&parser, NULL))
    {
        ERRORLOG("Unable to parse JSON");
        return 0;
    }

    return context.td_id;
}