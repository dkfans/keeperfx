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

#include <spng.h>

static int next_free_sprite = 0;

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

static unsigned char *read_png(const char *path, struct TbSprite* sprite)
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

    if(r)
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
    r = spng_decode_image(ctx, dst_buf, out_size, fmt, 0);
    if (r)
    {
        return NULL;
    }

    fclose(F);
    return dst_buf;
}

#define TRANSP_COLOR 255
static void compress_raw(struct TbSprite *sprite, unsigned char *src_buf, int x, int y, int w, int h)
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
                    *buf = -len; buf++;
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
            *buf = len; buf++;
            memcpy(buf, src_buf - len, len);
            buf += len;
        }
        *buf = 0;
        buf++;
        src_buf += tail;
    }
}

short add_custom_sprite(const char *path, int x, int y, int w, int h)
{
    short ret;
    struct TbSprite sprite;
    unsigned char *buf = read_png(path, &sprite);
    if (!buf)
        return 0;

    ret = next_free_sprite;
    next_free_sprite++;
    // This should be enough except rare cases like transparent checkerboard
    int dst_w = min(sprite.SWidth, w);
    int dst_h = min(sprite.SHeight, h);

    if (dst_w >= 255 || dst_h >= 255)
    {
        ERRORLOG("Sprites more than 255x255 are not supported");
        return 0;
    }

    size_t sz = (dst_w + 2) * (dst_h + 3);
    keepersprite_add[ret] = malloc(sz);
    sprite.Data = keepersprite_add[ret];
    compress_raw(&sprite, buf, x, y, dst_w, dst_h);
    struct KeeperSprite *ksprite = &creature_table_add[ret];

    ksprite->DataOffset = 0;
    // That is this actually?
    ksprite->SWidth = dst_w;
    ksprite->SHeight = dst_h;
    ksprite->FrameWidth = dst_w;
    ksprite->FrameHeight = dst_h;
    ksprite->Rotable = 0; // 2 need more sprite in next slot - not implemented yet
    ksprite->FramesCount = 0;
    ksprite->FrameOffsW = 0;
    ksprite->FrameOffsH = 0;
    ksprite->field_C = -dst_w/2; // Offset x
    ksprite->field_E = 1-dst_h; // Offset y

    return ret + KEEPERSPRITE_ADD_OFFSET;
}