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

typedef void __fastcall FlicFunc(void);

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
DLLIMPORT unsigned char *_DK_poly_screen;
#define poly_screen _DK_poly_screen
DLLIMPORT unsigned char *_DK_vec_screen;
#define vec_screen _DK_vec_screen
DLLIMPORT unsigned char *_DK_vec_map;
#define vec_map _DK_vec_map
DLLIMPORT unsigned long _DK_vec_screen_width;
#define vec_screen_width _DK_vec_screen_width
DLLIMPORT unsigned long _DK_vec_window_width;
#define vec_window_width _DK_vec_window_width
DLLIMPORT unsigned long _DK_vec_window_height;
#define vec_window_height _DK_vec_window_height
DLLIMPORT unsigned char *_DK_dither_map;
#define dither_map _DK_dither_map
DLLIMPORT unsigned char *_DK_dither_end;
#define dither_end _DK_dither_end
DLLIMPORT unsigned char *_DK_lbSpriteReMapPtr;
#define lbSpriteReMapPtr _DK_lbSpriteReMapPtr
DLLIMPORT long _DK_scale_up;
#define scale_up _DK_scale_up
DLLIMPORT long _DK_xsteps_array[2*256];
DLLIMPORT long _DK_ysteps_array[2*320];

DLLIMPORT long _DK_alpha_scale_up;
#define alpha_scale_up _DK_alpha_scale_up
DLLIMPORT long _DK_alpha_xsteps_array[2*256];
DLLIMPORT long _DK_alpha_ysteps_array[2*320];

#pragma pack()
/******************************************************************************/
/*
extern struct PurpleDrawItem *purple_draw_list;
extern unsigned short purple_draw_index;
extern TbSprite *lbFontPtr;
extern TbPixel vec_colour;
extern unsigned char vec_tmap[];
extern StartScreenPoint proj_origin;
extern unsigned short text_window_x1, text_window_y1;
extern unsigned short text_window_x2, text_window_y2;
extern char my_line_spacing;
*/
/******************************************************************************/
//Routines to be moved into bflib_vipurp
/*
void __fastcall draw_box_purple_list(const long x, const long y, const unsigned long width, const unsigned long height, const TbPixel colour);
void __fastcall copy_box_purple_list(const long x, const long y, const unsigned long width, const unsigned long height);
void __fastcall my_set_text_window(const unsigned short x1, const unsigned short y1,
        const unsigned short width, const unsigned short height);
void __fastcall draw_text_purple_list(const long x, const long y, const char *text, const int line);
void __fastcall draw_sprite_purple_list(long x, long y, struct TbSprite *sprite);
void __fastcall draw_trig_purple_list(long x2, long y2, long x3, long y3);
void __fastcall  draw_triangle_purple_list(long x1, long y1, long x2, long y2,
        long x3, long y3, TbPixel colour);
void __fastcall draw_line_purple_list(long x1, long y1, long x2, long y2, TbPixel colour);
void __fastcall draw_flic_purple_list(FlicFunc *fn);
void __fastcall draw_hotspot_purple_list(long x, long y);
unsigned short __fastcall my_draw_text(long x, long y, const char *text, const long startline);
void __fastcall draw_purple_screen(void);
*/
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
/*
int __fastcall LbDrawBoxCoords(long xpos1, long ypos1, long xpos2, long ypos2, TbPixel colour);
void __fastcall LbDrawTriangle(long x1, long y1, long x2, long y2, long x3, long y3, TbPixel colour);
*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
