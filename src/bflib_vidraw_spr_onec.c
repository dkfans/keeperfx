/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_vidraw_spr_onec.c
 *     Graphics canvas drawing library, scaled sprite drawing with one color.
 * @par Purpose:
 *    Screen drawing routines; draws rescaled sprite.
 * @par Comment:
 *     Medium level library, draws on screen buffer used in bflib_video.
 *     Used for drawing screen components.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 01 Aug 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_vidraw.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"

#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_mouse.h"
#include "bflib_render.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern long xsteps_array[2*SPRITE_SCALING_XSTEPS];
extern long ysteps_array[2*SPRITE_SCALING_YSTEPS];
extern long alpha_xsteps_array[2*SPRITE_SCALING_XSTEPS];
extern long alpha_ysteps_array[2*SPRITE_SCALING_YSTEPS];
/******************************************************************************/
void LbPixelBlockCopyForward(TbPixel * dst, const TbPixel * src, long len);
/******************************************************************************/
/**
 * Draws a scaled up sprite on given buffer, with transparency mapping and one colour, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingUpDataTrans1RL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            unsigned char *prevdata;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            prevdata = sprdata;
            while (ydup > 0)
            {
                sprdata = prevdata;
                xcurstep = xstep;
                TbPixel *out_end;
                out_end = outbuf;
                while ( 1 )
                {
                    long pxlen;
                    pxlen = (signed char)*sprdata;
                    sprdata++;
                    if (pxlen == 0)
                        break;
                    if (pxlen < 0)
                    {
                        pxlen = -pxlen;
                        out_end -= xcurstep[0] + xcurstep[1];
                        xcurstep -= 2 * pxlen;
                        out_end += xcurstep[0] + xcurstep[1];
                    }
                    else
                    {
                        for (;pxlen > 0; pxlen--)
                        {
                            xdup = xcurstep[1];
                            if (xcurstep[0]+xdup > abs(scanline))
                                xdup = abs(scanline)-xcurstep[0];
                            if (xdup > 0)
                            {
                                unsigned int pxmap;
                                pxmap = ((colour) << 8);
                                for (;xdup > 0; xdup--)
                                {
                                    pxmap = (pxmap & ~0x00ff) | ((*out_end));
                                    *out_end = transmap[pxmap];
                                    out_end--;
                                }
                            }
                            sprdata++;
                            xcurstep -= 2;
                        }
                    }
                }
                outbuf += scanline;
                ydup--;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with transparency mapping and one colour, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @param transmap The transparency mapping table to be used. Should have a size of 256x256 to avoid invalid memory reads.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingUpDataTrans1LR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            unsigned char *prevdata;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            prevdata = sprdata;
            while (ydup > 0)
            {
                sprdata = prevdata;
                xcurstep = xstep;
                TbPixel *out_end;
                out_end = outbuf;
                while ( 1 )
                {
                    long pxlen;
                    pxlen = (signed char)*sprdata;
                    sprdata++;
                    if (pxlen == 0)
                        break;
                    if (pxlen < 0)
                    {
                        pxlen = -pxlen;
                        out_end -= xcurstep[0];
                        xcurstep += 2 * pxlen;
                        out_end += xcurstep[0];
                    }
                    else
                    {
                        for (;pxlen > 0; pxlen--)
                        {
                            xdup = xcurstep[1];
                            if (xcurstep[0]+xdup > abs(scanline))
                                xdup = abs(scanline)-xcurstep[0];
                            if (xdup > 0)
                            {
                                unsigned int pxmap;
                                pxmap = ((colour) << 8);
                                for (;xdup > 0; xdup--)
                                {
                                    pxmap = (pxmap & ~0x00ff) | ((*out_end));
                                    *out_end = transmap[pxmap];
                                    out_end++;
                                }
                            }
                            sprdata++;
                            xcurstep += 2;
                        }
                    }
                }
                outbuf += scanline;
                ydup--;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with reversed transparency mapping and one colour, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingUpDataTrans2RL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            unsigned char *prevdata;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            prevdata = sprdata;
            while (ydup > 0)
            {
                sprdata = prevdata;
                xcurstep = xstep;
                TbPixel *out_end;
                out_end = outbuf;
                while ( 1 )
                {
                    long pxlen;
                    pxlen = (signed char)*sprdata;
                    sprdata++;
                    if (pxlen == 0)
                        break;
                    if (pxlen < 0)
                    {
                        pxlen = -pxlen;
                        out_end -= xcurstep[0] + xcurstep[1];
                        xcurstep -= 2 * pxlen;
                        out_end += xcurstep[0] + xcurstep[1];
                    }
                    else
                    {
                        for (;pxlen > 0; pxlen--)
                        {
                            xdup = xcurstep[1];
                            if (xcurstep[0]+xdup > abs(scanline))
                                xdup = abs(scanline)-xcurstep[0];
                            if (xdup > 0)
                            {
                                unsigned int pxmap;
                                pxmap = (colour);
                                for (;xdup > 0; xdup--)
                                {
                                    pxmap = (pxmap & ~0xff00) | ((*out_end) << 8);
                                    *out_end = transmap[pxmap];
                                    out_end--;
                                }
                            }
                            sprdata++;
                            xcurstep -= 2;
                        }
                    }
                }
                outbuf += scanline;
                ydup--;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with reversed transparency mapping and one colour, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingUpDataTrans2LR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            unsigned char *prevdata;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            prevdata = sprdata;
            while (ydup > 0)
            {
                sprdata = prevdata;
                xcurstep = xstep;
                TbPixel *out_end;
                out_end = outbuf;
                while ( 1 )
                {
                    long pxlen;
                    pxlen = (signed char)*sprdata;
                    sprdata++;
                    if (pxlen == 0)
                        break;
                    if (pxlen < 0)
                    {
                        pxlen = -pxlen;
                        out_end -= xcurstep[0];
                        xcurstep += 2 * pxlen;
                        out_end += xcurstep[0];
                    }
                    else
                    {
                        for (;pxlen > 0; pxlen--)
                        {
                            xdup = xcurstep[1];
                            if (xcurstep[0]+xdup > abs(scanline))
                                xdup = abs(scanline)-xcurstep[0];
                            if (xdup > 0)
                            {
                                unsigned int pxmap;
                                pxmap = (colour);
                                for (;xdup > 0; xdup--)
                                {
                                    pxmap = (pxmap & ~0xff00) | ((*out_end) << 8);
                                    *out_end = transmap[pxmap];
                                    out_end++;
                                }
                            }
                            sprdata++;
                            xcurstep += 2;
                        }
                    }
                }
                outbuf += scanline;
                ydup--;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with one colour, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingUpDataSolidRL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            int ycur;
            int solid_len;
            TbPixel * out_line;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0] + xcurstep[1];
                    xcurstep -= 2 * pxlen;
                    out_end += xcurstep[0] + xcurstep[1];
                }
                else
                {
                    TbPixel *out_start;
                    out_start = out_end;
                    for(;pxlen > 0; pxlen--)
                    {
                        xdup = xcurstep[1];
                        if (xcurstep[0]+xdup > abs(scanline))
                            xdup = abs(scanline)-xcurstep[0];
                        if (xdup > 0)
                        {
                            unsigned char pxval;
                            pxval = (colour);
                            for (;xdup > 0; xdup--)
                            {
                                *out_end = pxval;
                                out_end--;
                            }
                        }
                        sprdata++;
                        xcurstep -= 2;
                    }
                    ycur = ydup - 1;
                    if (ycur > 0)
                    {
                        solid_len = out_start - out_end;
                        out_start = out_end;
                        solid_len++;
                        out_line = out_start + scanline;
                        for (;ycur > 0; ycur--)
                        {
                            if (solid_len > 0) {
                                LbPixelBlockCopyForward(out_line, out_start, solid_len);
                            }
                            out_line += scanline;
                        }
                    }
                }
            }
            outbuf += scanline;
            ycur = ydup - 1;
            for (;ycur > 0; ycur--)
            {
                outbuf += scanline;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with one colour, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingUpDataSolidLR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            int ycur;
            int solid_len;
            TbPixel * out_line;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0];
                    xcurstep += 2 * pxlen;
                    out_end += xcurstep[0];
                }
                else
                {
                    TbPixel *out_start;
                    out_start = out_end;
                    for(;pxlen > 0; pxlen--)
                    {
                        xdup = xcurstep[1];
                        if (xcurstep[0]+xdup > abs(scanline))
                            xdup = abs(scanline)-xcurstep[0];
                        if (xdup > 0)
                        {
                            unsigned char pxval;
                            pxval = (colour);
                            for (;xdup > 0; xdup--)
                            {
                                *out_end = pxval;
                                out_end++;
                            }
                        }
                        sprdata++;
                        xcurstep += 2;
                    }
                    ycur = ydup - 1;
                    if (ycur > 0)
                    {
                        solid_len = out_end - out_start;
                        out_line = out_start + scanline;
                        for (;ycur > 0; ycur--)
                        {
                            if (solid_len > 0) {
                                LbPixelBlockCopyForward(out_line, out_start, solid_len);
                            }
                            out_line += scanline;
                        }
                    }
                }
            }
            outbuf += scanline;
            ycur = ydup - 1;
            for (;ycur > 0; ycur--)
            {
                outbuf += scanline;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with transparency mapping and one colour, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingDownDataTrans1RL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0] + xcurstep[1];
                    xcurstep -= 2 * pxlen;
                    out_end += xcurstep[0] + xcurstep[1];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned int pxmap;
                            pxmap = ((colour) << 8);
                            {
                                pxmap = (pxmap & ~0x00ff) | ((*out_end));
                                *out_end = transmap[pxmap];
                                out_end--;
                            }
                        }
                        sprdata++;
                        xcurstep -= 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with transparency mapping and one colour, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingDownDataTrans1LR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0];
                    xcurstep += 2 * pxlen;
                    out_end += xcurstep[0];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned int pxmap;
                            pxmap = ((colour) << 8);
                            {
                                pxmap = (pxmap & ~0x00ff) | ((*out_end));
                                *out_end = transmap[pxmap];
                                out_end++;
                            }
                        }
                        sprdata++;
                        xcurstep += 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with reverse transparency mapping and one colour, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingDownDataTrans2RL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0] + xcurstep[1];
                    xcurstep -= 2 * pxlen;
                    out_end += xcurstep[0] + xcurstep[1];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned int pxmap;
                            pxmap = ((colour) << 8);
                            {
                                pxmap = (pxmap & ~0xff00) | ((*out_end) << 8);
                                *out_end = transmap[pxmap];
                                out_end--;
                            }
                        }
                        sprdata++;
                        xcurstep -= 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with reverse transparency mapping and one colour, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingDownDataTrans2LR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0];
                    xcurstep += 2 * pxlen;
                    out_end += xcurstep[0];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned int pxmap;
                            pxmap = (colour);
                            {
                                pxmap = (pxmap & ~0xff00) | ((*out_end) << 8);
                                *out_end = transmap[pxmap];
                                out_end++;
                            }
                        }
                        sprdata++;
                        xcurstep += 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with one colour, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingDownDataSolidRL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0] + xcurstep[1];
                    xcurstep -= 2 * pxlen;
                    out_end += xcurstep[0] + xcurstep[1];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned char pxval;
                            pxval = (colour);
                            {
                                *out_end = pxval;
                                out_end--;
                            }
                        }
                        sprdata++;
                        xcurstep -= 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with one colour, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawOneColourUsingScalingDownDataSolidLR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSprite *sprite, TbPixel colour)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    unsigned char *sprdata;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    sprdata = sprite->Data;
    ycurstep = ystep;

    int h;
    for (h=sprite->SHeight; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0];
                    xcurstep += 2 * pxlen;
                    out_end += xcurstep[0];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned char pxval;
                            pxval = (colour);
                            {
                                *out_end = pxval;
                                out_end++;
                            }
                        }
                        sprdata++;
                        xcurstep += 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled sprite with one colour on current graphics window at given position.
 * Requires LbSpriteSetScalingData() to be called before.
 *
 * @param posx The X coord within current graphics window.
 * @param posy The Y coord within current graphics window.
 * @param sprite The source sprite.
 * @param colour The colour to be used for drawing.
 * @return Gives 0 on success.
 * @see LbSpriteSetScalingData()
 */
TbResult LbSpriteDrawOneColourUsingScalingData(long posx, long posy, const struct TbSprite *sprite, TbPixel colour)
{
    SYNCDBG(17,"Drawing at (%ld,%ld)",posx,posy);
    long *xstep;
    long *ystep;
    int scanline;
    {
        long sposx;
        long sposy;
        sposx = posx;
        sposy = posy;
        scanline = lbDisplay.GraphicsScreenWidth;
        if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0) {
            sposx = sprite->SWidth + posx - 1;
        }
        if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_VERTIC) != 0) {
            sposy = sprite->SHeight + posy - 1;
            scanline = -lbDisplay.GraphicsScreenWidth;
        }
        xstep = &xsteps_array[2 * sposx];
        ystep = &ysteps_array[2 * sposy];
    }
    uchar *outbuf;
    int outheight;
    {
        int gspos_x;
        int gspos_y;
        gspos_y = ystep[0];
        if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_VERTIC) != 0)
            gspos_y += ystep[1] - 1;
        gspos_x = xstep[0];
        if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
            gspos_x += xstep[1] - 1;
        outbuf = &lbDisplay.GraphicsWindowPtr[gspos_x + lbDisplay.GraphicsScreenWidth * gspos_y];
        outheight = lbDisplay.GraphicsScreenHeight;
    }
    if ( scale_up )
    {
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawOneColourUsingScalingUpDataTrans1RL(outbuf, scanline, outheight, xstep, ystep, sprite, colour, render_ghost);
          }
          else
          {
              return LbSpriteDrawOneColourUsingScalingUpDataTrans1LR(outbuf, scanline, outheight, xstep, ystep, sprite, colour, render_ghost);
          }
        }
        else
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8) != 0)
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawOneColourUsingScalingUpDataTrans2RL(outbuf, scanline, outheight, xstep, ystep, sprite, colour, render_ghost);
          }
          else
          {
              return LbSpriteDrawOneColourUsingScalingUpDataTrans2LR(outbuf, scanline, outheight, xstep, ystep, sprite, colour, render_ghost);
          }
        }
        else
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawOneColourUsingScalingUpDataSolidRL(outbuf, scanline, outheight, xstep, ystep, sprite, colour);
          }
          else
          {
              return LbSpriteDrawOneColourUsingScalingUpDataSolidLR(outbuf, scanline, outheight, xstep, ystep, sprite, colour);
          }
        }
    }
    else
    {
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawOneColourUsingScalingDownDataTrans1RL(outbuf, scanline, outheight, xstep, ystep, sprite, colour, render_ghost);
          }
          else
          {
              return LbSpriteDrawOneColourUsingScalingDownDataTrans1LR(outbuf, scanline, outheight, xstep, ystep, sprite, colour, render_ghost);
          }
        }
        else
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8) != 0)
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawOneColourUsingScalingDownDataTrans2RL(outbuf, scanline, outheight, xstep, ystep, sprite, colour, render_ghost);
          }
          else
          {
              return LbSpriteDrawOneColourUsingScalingDownDataTrans2LR(outbuf, scanline, outheight, xstep, ystep, sprite, colour, render_ghost);
          }
        }
        else
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawOneColourUsingScalingDownDataSolidRL(outbuf, scanline, outheight, xstep, ystep, sprite, colour);
          }
          else
          {
              return LbSpriteDrawOneColourUsingScalingDownDataSolidLR(outbuf, scanline, outheight, xstep, ystep, sprite, colour);
          }
        }
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
