/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_mist.cpp
 *     Mist lens effect functions.
 * @par Purpose:
 *     Functions to apply Mist lens effect to the image.
 *     Mist is a effect of bitmap image moving over original 3D view.
 *     When moving, mist can change directions, but can never go upward.
 *     Mist looks like there was a layer of dirt just behind the eye.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "lens_mist.h"
#include "globals.h"
#include "bflib_basics.h"

/******************************************************************************/
class CMistFade {
  public:
    CMistFade(void);
    virtual ~CMistFade(void);
    void setup(unsigned char *lens_mem, unsigned char *fade, unsigned char *ghost);
    void animset(long a1, long a2);
    void mist(unsigned char *dstbuf, long dstwidth, unsigned char *srcbuf, long srcwidth, long width, long height);
    void animate(void);
  protected:
    /** Mist data width and height are the same and equal to this dimension */
    unsigned int lens_dim;
    unsigned char *lens_data;
    unsigned char *fade_data;
    unsigned char *ghost_data;
    unsigned char field_C;
    unsigned char field_D;
    unsigned char field_E;
    unsigned char field_F;
    long field_10;
    long field_14;
    unsigned char field_18;
    unsigned char field_19;
    unsigned char field_1A;
    unsigned char field_1B;
    };

/******************************************************************************/
CMistFade *mist = NULL;
/******************************************************************************/

void CMistFade::setup(unsigned char *lens_mem, unsigned char *fade, unsigned char *ghost)
{
  this->lens_data = lens_mem;
  this->fade_data = fade;
  this->ghost_data = ghost;
  this->lens_dim = 256;
  this->field_C = 0;
  this->field_D = 0;
  this->field_E = 50;
  this->field_F = 128;
  this->field_14 = 1024;
  this->field_10 = 0;
  this->field_18 = 2;
  this->field_19 = 1;
  this->field_1A = 253;
  this->field_1B = 3;
}

void CMistFade::animset(long a1, long a2)
{
  this->field_10 = a1;
  this->field_14 = a2;
}

void CMistFade::animate(void)
{
  this->field_C += this->field_18;
  this->field_D += this->field_19;
  this->field_E -= this->field_1A;
  this->field_10 += this->field_14;
  this->field_F += this->field_1B;
}

void CMistFade::mist(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, long width, long height)
{
    unsigned char *src;
    unsigned char *dst;
    unsigned long p2;
    unsigned long c2;
    unsigned long p1;
    unsigned long c1;
    unsigned long lens_div;
    long i;
    long k;
    long n;
    long w;
    long h;

    if ((lens_data == NULL) || (fade_data == NULL))
    {
        ERRORLOG("Can't draw Mist as it's not initialized!");
        return;
    }
    src = srcbuf;
    dst = dstbuf;
    p2 = this->field_C;
    c2 = this->field_D;
    p1 = this->field_E;
    c1 = this->field_F;
    lens_div = width/(2*lens_dim);
    if (lens_div < 1) lens_div = 1;
    for (h=height; h > 0; h--)
    {
        for (w=width; w > 0; w--)
        {
            //JUSTLOG("POS %d,%d Px %d,%d",w,h,p1,p2);
            i = lens_data[(c1 * lens_dim) + p1];
            k = lens_data[(c2 * lens_dim) + p2];
            n = (k + i) >> 3;
            if (n > 32)
              n = 32;
            else
            if (n < 0)
              n = 0;
            *dst = this->fade_data[(n << 8) + *src];
            src++;
            dst++;
            if ((w%lens_div) == 0)
            {
                c1--;
                c1 %= lens_dim;
                p2++;
                p2 %= lens_dim;
            }
        }
        // Move buffers to end of this line
        dst += (dstpitch-width);
        src += (srcpitch-width);
        // Update other counters
        if ((h%lens_div) == 0)
        {
            c1 += width;
            c1 %= lens_dim;
            p2 -= width;
            p2 %= lens_dim;
            c2++;
            c2 %= lens_dim;
            p1--;
            p1 %= lens_dim;
        }
    }
}

CMistFade::CMistFade(void)
{
  setup(NULL, NULL, NULL);
}

CMistFade::~CMistFade(void)
{
}

TbBool draw_mist(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, long width, long height)
{
    SYNCDBG(8,"Starting");
    if (mist == NULL)
    {
        WARNLOG("Tried to use uninitialized mist!");
        return false;
    }
    mist->mist(dstbuf, dstpitch, srcbuf, srcpitch, width, height);
    mist->animate();
    return true;
}

void setup_mist(unsigned char *lens_mem, unsigned char *fade, unsigned char *ghost)
{
    SYNCDBG(8,"Starting");
    if (mist == NULL)
        mist = new CMistFade();
    mist->setup(lens_mem, fade, ghost);
    mist->animset(0, 1024);
}

void free_mist(void)
{
    delete mist;
    mist = NULL;
}
/******************************************************************************/
