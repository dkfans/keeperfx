/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_vidraw.h
 *     Header file for bflib_vidraw.c.
 * @par Purpose:
 *     Graphics canvas drawing library.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 10 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_VIDRAW_H
#define BFLIB_VIDRAW_H

#include "bflib_video.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MAX_SUPPORTED_SPRITE_DIM 256

#define NUM_DRAWITEMS 238
#define SPRITE_SCALING_XSTEPS max(MAX_SUPPORTED_SPRITE_DIM,MAX_SUPPORTED_SCREEN_WIDTH)
#define SPRITE_SCALING_YSTEPS max(MAX_SUPPORTED_SPRITE_DIM,MAX_SUPPORTED_SCREEN_HEIGHT)
/******************************************************************************/
#pragma pack(1)

struct TiledSprite;
struct TbSprite;
struct TbHugeSprite;

typedef void FlicFunc(void);

struct StartScreenPoint {
        short X;
        short Y;
};

//Note: this name is incorrect! (not from game)
struct LongPoint {
        long X;
        long Y;
};

struct EnginePoint {
        long X;
        long Y;
        long TMapX;
        long TMapY;
        long Shade;
        long X3d;
        long Y3d;
        long Z3d;
        long DistSqr;
        unsigned short padw;
        unsigned char Flags;
        unsigned char padb;
};

struct TbDItmHotspot {
        short X;
        short Y;
};

struct TbDItmFlic {
        FlicFunc *Function;
        TbPixel Colour;
};

struct TbDItmText {
        short WindowX;
        short WindowY;
        short Width;
        short Height;
        short X;
        short Y;
        const char *Text;
        struct TbSprite *Font;
        unsigned short Line;
        TbPixel Colour;
};

struct TbDItmSprite {
        short X;
        short Y;
        struct TbSprite *Sprite;
        TbPixel Colour;
};

struct TbDItmTrig {
        short X2;
        short Y2;
        short X3;
        short Y3;
        TbPixel Colour;
};

struct TbDItmTriangle {
        short X1;
        short Y1;
        short X2;
        short Y2;
        short X3;
        short Y3;
        TbPixel Colour;
};

struct TbDItmBox {
        short X;
        short Y;
        short Width;
        short Height;
        TbPixel Colour;
};

struct TbDItmLine {
        short X1;
        short Y1;
        short X2;
        short Y2;
        TbPixel Colour;
};

union TbDItmU {
        struct TbDItmTrig Trig;
        struct TbDItmTriangle Triangle;
        struct TbDItmBox Box;
        struct TbDItmLine Line;
        struct TbDItmSprite Sprite;
        struct TbDItmText Text;
        struct TbDItmFlic Flic;
        struct TbDItmHotspot Hotspot;
};

//Original size (incl. any padding) = 26 bytes
struct PurpleDrawItem {
        union TbDItmU U;
        // pos=23d
        unsigned char Type;
        // pos=24d
        unsigned short Flags;
};

/******************************************************************************/
extern unsigned char *poly_screen;
extern unsigned char *vec_screen;
extern unsigned char *vec_map;
extern unsigned long vec_screen_width;
extern long vec_window_width;
extern long vec_window_height;
extern unsigned char *dither_map;
extern unsigned char *dither_end;
extern unsigned char *lbSpriteReMapPtr;
extern long scale_up;
extern long alpha_scale_up;

#pragma pack()

/******************************************************************************/
TbResult LbDrawBox(long x, long y, unsigned long width, unsigned long height, TbPixel colour);
void LbDrawHVLine(long xpos1, long ypos1, long xpos2, long ypos2, TbPixel colour);

void LbDrawPixel(long x, long y, TbPixel colour);
void LbDrawCircle(long x, long y, long radius, TbPixel colour);
TbResult LbDrawLine(long x1, long y1, long x2, long y2, TbPixel colour);

void setup_vecs(unsigned char *screenbuf, unsigned char *nvec_map,
        unsigned int line_len, unsigned int width, unsigned int height);
TbResult LbSpriteDrawUsingScalingData(long posx, long posy, const struct TbSprite *sprite);
TbResult LbSpriteDrawRemapUsingScalingData(long posx, long posy, const struct TbSprite *sprite, const TbPixel *cmap);
TbResult LbSpriteDrawOneColourUsingScalingData(long posx, long posy, const struct TbSprite *sprite, TbPixel colour);
void LbSpriteSetScalingData(long x, long y, long swidth, long sheight, long dwidth, long dheight);
TbResult DrawAlphaSpriteUsingScalingData(long posx, long posy, struct TbSprite *sprite);
void SetAlphaScalingData(long a1, long a2, long a3, long a4, long a5, long a6);
void LbSpriteSetScalingWidthSimpleArray(long * xsteps_arr, long x, long swidth, long dwidth);
void LbSpriteSetScalingWidthClippedArray(long * xsteps_arr, long x, long swidth, long dwidth, long gwidth);
void LbSpriteSetScalingHeightSimpleArray(long * ysteps_arr, long y, long sheight, long dheight);
void LbSpriteSetScalingHeightClippedArray(long * ysteps_arr, long y, long sheight, long dheight, long gheight);

TbResult LbSpriteDraw(long x, long y, const struct TbSprite *spr);
TbResult LbSpriteDrawOneColour(long x, long y, const struct TbSprite *spr, const TbPixel colour);
int LbSpriteDrawRemap(long x, long y, const struct TbSprite *spr,const unsigned char *cmap);

TbResult LbSpriteDrawScaled(long xpos, long ypos, const struct TbSprite *sprite, long dest_width, long dest_height);
TbResult LbSpriteDrawScaledOneColour(long xpos, long ypos, const struct TbSprite *sprite, long dest_width, long dest_height, const TbPixel colour);
int LbSpriteDrawScaledRemap(long xpos, long ypos, const struct TbSprite *sprite, long dest_width, long dest_height, const unsigned char *cmap);
#define LbSpriteDrawResized(xpos, ypos, un_per_px, sprite) LbSpriteDrawScaled(xpos, ypos, sprite, ((sprite)->SWidth * un_per_px + 8) / 16, ((sprite)->SHeight * un_per_px + 8) / 16)
#define LbSpriteDrawResizedOneColour(xpos, ypos, un_per_px, sprite, colour) LbSpriteDrawScaledOneColour(xpos, ypos, sprite, ((sprite)->SWidth * un_per_px + 8) / 16, ((sprite)->SHeight * un_per_px + 8) / 16, colour)
#define LbSpriteDrawResizedRemap(xpos, ypos, un_per_px, sprite, cmap) LbSpriteDrawScaledRemap(xpos, ypos, sprite, ((sprite)->SWidth * un_per_px + 8) / 16, ((sprite)->SHeight * un_per_px + 8) / 16, cmap)

TbResult LbHugeSpriteDraw(const struct TbHugeSprite * spr, long sp_len,
    unsigned char *r, int r_row_delta, int r_height, short xshift, short yshift, int units_per_px);
void LbTiledSpriteDraw(long x, long y, long units_per_px, struct TiledSprite *bigspr, struct TbSprite *sprite);
int LbTiledSpriteHeight(struct TiledSprite *bigspr, struct TbSprite *sprite);

void LbSpriteSetScalingWidthClipped(long x, long swidth, long dwidth, long gwidth);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
