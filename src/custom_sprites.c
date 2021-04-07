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

static int next_free_sprite = 0;

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

struct TgaSpec
{
    unsigned short ofsX;
    unsigned short ofsY;
    unsigned short width;
    unsigned short height;
    char pixDepth;
    char desc;
};
static unsigned char *read_tga(const char *path, struct TbSprite* sprite)
{
    unsigned char *dst_buf;
    FILE *F = fopen(path, "rb");
    sprite->SHeight = 0;
    sprite->SWidth = 0;
    if (F == NULL)
    {
        ERRORLOG("Unable to read %s", path);
        return NULL;
    }
    if (1 != fread(scratch, 8, 1, F))
    {
        ERRORLOG("Unable to read %s", path);
        fclose(F);
        return NULL;
    }
    if (memcmp(scratch, "\0\1\1\0\0\0\1\x18", 8) != 0)
    {
        ERRORLOG("%s should be uncompressed 8bit TGA file with palette", path);
        fclose(F);
        return NULL;
    }
    struct TgaSpec *spec = (struct TgaSpec *)scratch;
    if (1 != fread(spec, sizeof(struct TgaSpec), 1, F))
    {
        ERRORLOG("Unable to read %s", path);
        fclose(F);
        return NULL;
    }
    if (spec->pixDepth != 8)
    {
        ERRORLOG("wrong spec: %s should be uncompressed 8bit TGA file with palette", path);
        fclose(F);
        return NULL;
    }
    if (spec->desc != 0x20)
    {
        WARNLOG("image origin should be top-left(%s)", path);
    }
    sprite->SWidth = spec->width;
    sprite->SHeight = spec->height;
    fseek(F, 18 + 256 * 3,SEEK_SET);
    dst_buf = scratch;
    if (1 != fread(dst_buf, sprite->SWidth * sprite->SHeight, 1, F))
    {
        ERRORLOG("Unable to read %s", path);
        fclose(F);
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
    for (int j = 0; j < h; j++)
    {
        is_transp = false;
        len = 0;
        src_buf += x;
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
    unsigned char *buf = read_tga(path, &sprite);
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