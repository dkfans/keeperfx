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
        int32_t X;
        int32_t Y;
};

struct EnginePoint {
        int32_t X;
        int32_t Y;
        int32_t TMapX;
        int32_t TMapY;
        int32_t Shade;
        int32_t coordinate_x_3d;
        int32_t coordinate_y_3d;
        int32_t coordinate_z_3d;
        int32_t DistSqr;
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
        short vertex_2_x;
        short vertex_2_y;
        short vertex_3_x;
        short vertex_3_y;
        TbPixel Colour;
};

struct TbDItmTriangle {
        short vertex_1_x;
        short vertex_1_y;
        short vertex_2_x;
        short vertex_2_y;
        short vertex_3_x;
        short vertex_3_y;
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
        short vertex_1_x;
        short vertex_1_y;
        short vertex_2_x;
        short vertex_2_y;
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

struct TbSourceBuffer {
        const void * data;
        uint32_t width;
        uint32_t height;
        uint32_t pitch;
};

/******************************************************************************/
extern unsigned char *poly_screen;
extern unsigned char *vec_screen;
extern unsigned char *vec_map;
extern uint32_t vec_screen_width;
extern int32_t vec_window_width;
extern int32_t vec_window_height;
extern unsigned char *dither_map;
extern unsigned char *dither_end;
extern unsigned char *lbSpriteReMapPtr;
extern int32_t scale_up;
extern int32_t xsteps_array[2*SPRITE_SCALING_XSTEPS];
extern int32_t ysteps_array[2*SPRITE_SCALING_YSTEPS];

#pragma pack()

/******************************************************************************/
TbResult LbDrawBox(int32_t x, int32_t y, uint32_t width, uint32_t height, TbPixel colour);
void LbDrawHVLine(int32_t xpos1, int32_t ypos1, int32_t xpos2, int32_t ypos2, TbPixel colour);

void LbDrawPixel(int32_t x, int32_t y, TbPixel colour);
void LbDrawCircle(int32_t x, int32_t y, int32_t radius, TbPixel colour);

void setup_vecs(unsigned char *screenbuf, unsigned char *nvec_map,
        unsigned int line_len, unsigned int width, unsigned int height);
void setup_steps(int32_t posx, int32_t posy, const struct TbSourceBuffer * src_buf, int32_t **xstep, int32_t **ystep, int *scanline);
void setup_outbuf(const int32_t *xstep, const int32_t *ystep, uchar **outbuf, int *outheight);
TbResult LbSpriteDrawUsingScalingData(int32_t posx, int32_t posy, const struct TbSourceBuffer *);
TbResult LbSpriteDrawRemapUsingScalingData(int32_t posx, int32_t posy, const struct TbSourceBuffer *, const TbPixel *cmap);
TbResult LbSpriteDrawOneColourUsingScalingData(int32_t posx, int32_t posy, const struct TbSprite *sprite, TbPixel colour);
void LbSpriteSetScalingData(int32_t x, int32_t y, int32_t swidth, int32_t sheight, int32_t dwidth, int32_t dheight);
TbResult DrawAlphaSpriteUsingScalingData(int32_t posx, int32_t posy, const struct TbSourceBuffer *);
void LbSpriteSetScalingWidthSimpleArray(int32_t * xsteps_arr, int32_t x, int32_t swidth, int32_t dwidth);
void LbSpriteSetScalingWidthClippedArray(int32_t * xsteps_arr, int32_t x, int32_t swidth, int32_t dwidth, int32_t gwidth);
void LbSpriteSetScalingHeightSimpleArray(int32_t * ysteps_arr, int32_t y, int32_t sheight, int32_t dheight);
void LbSpriteSetScalingHeightClippedArray(int32_t * ysteps_arr, int32_t y, int32_t sheight, int32_t dheight, int32_t gheight);

TbResult LbSpriteDraw(int32_t x, int32_t y, const struct TbSprite *spr);
TbResult LbSpriteDrawOneColour(int32_t x, int32_t y, const struct TbSprite *spr, const TbPixel colour);

TbResult LbSpriteDrawScaled(int32_t xpos, int32_t ypos, const struct TbSprite *sprite, int32_t dest_width, int32_t dest_height);
TbResult LbSpriteDrawScaledOneColour(int32_t xpos, int32_t ypos, const struct TbSprite *sprite, int32_t dest_width, int32_t dest_height, const TbPixel colour);
int LbSpriteDrawScaledRemap(int32_t xpos, int32_t ypos, const struct TbSprite *sprite, int32_t dest_width, int32_t dest_height, const unsigned char *cmap);
#define LbSpriteDrawResized(xpos, ypos, un_per_px, sprite) LbSpriteDrawScaled(xpos, ypos, sprite, ((sprite)->SWidth * un_per_px + 8) / 16, ((sprite)->SHeight * un_per_px + 8) / 16)
#define LbSpriteDrawResizedOneColour(xpos, ypos, un_per_px, sprite, colour) LbSpriteDrawScaledOneColour(xpos, ypos, sprite, ((sprite)->SWidth * un_per_px + 8) / 16, ((sprite)->SHeight * un_per_px + 8) / 16, colour)
#define LbSpriteDrawResizedRemap(xpos, ypos, un_per_px, sprite, cmap) LbSpriteDrawScaledRemap(xpos, ypos, sprite, ((sprite)->SWidth * un_per_px + 8) / 16, ((sprite)->SHeight * un_per_px + 8) / 16, cmap)

TbResult LbHugeSpriteDraw(const struct TbHugeSprite * spr, int32_t sp_len,
    unsigned char *r, int r_row_delta, int r_height, short xshift, short yshift, int units_per_px);
void LbTiledSpriteDraw(int32_t x, int32_t y, int32_t units_per_px, struct TiledSprite *bigspr);
int LbTiledSpriteHeight(struct TiledSprite *bigspr);

// mspointer needs this for some reason
TbResult LbSpriteDrawUsingScalingUpDataSolidLR(uchar *outbuf, int scanline, int outheight, int32_t *xstep, int32_t *ystep, const struct TbSourceBuffer * src_buf);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
