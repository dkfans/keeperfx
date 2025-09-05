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
#include "pre_inc.h"
#include "lens_mist.h"
#include "globals.h"
#include "bflib_basics.h"
#include "post_inc.h"

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
    unsigned char position_offset_x;
    unsigned char position_offset_y;
    unsigned char secondary_offset_x;
    unsigned char secondary_offset_y;
    long animation_counter;
    long animation_speed;
    unsigned char position_x_step;
    unsigned char position_y_step;
    unsigned char secondary_x_step;
    unsigned char secondary_y_step;
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
  this->position_offset_x = 0;
  this->position_offset_y = 0;
  this->secondary_offset_x = 50;
  this->secondary_offset_y = 128;
  this->animation_speed = 1024;
  this->animation_counter = 0;
  this->position_x_step = 2;
  this->position_y_step = 1;
  this->secondary_x_step = 253;
  this->secondary_y_step = 3;
}

void CMistFade::animset(long a1, long a2)
{
  this->animation_counter = a1;
  this->animation_speed = a2;
}

void CMistFade::animate(void)
{
  this->position_offset_x += this->position_x_step;
  this->position_offset_y += this->position_y_step;
  this->secondary_offset_x -= this->secondary_x_step;
  this->animation_counter += this->animation_speed;
  this->secondary_offset_y += this->secondary_y_step;
}

void CMistFade::mist(unsigned char *dstbuf, long dstpitch, unsigned char *srcbuf, long srcpitch, long width, long height)
{
    unsigned char *src;
    unsigned char *dst;
    unsigned long primary_offset_x;
    unsigned long primary_offset_y;
    unsigned long local_secondary_offset_x;
    unsigned long local_secondary_offset_y;
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
    primary_offset_x = this->position_offset_x;
    primary_offset_y = this->position_offset_y;
    local_secondary_offset_x = this->secondary_offset_x;
    local_secondary_offset_y = this->secondary_offset_y;
    lens_div = width/(2*lens_dim);
    if (lens_div < 1) lens_div = 1;
    for (h=height; h > 0; h--)
    {
        for (w=width; w > 0; w--)
        {
            //JUSTLOG("POS %d,%d Px %d,%d",w,h,p1,p2);
            i = lens_data[(local_secondary_offset_y * lens_dim) + local_secondary_offset_x];
            k = lens_data[(primary_offset_y * lens_dim) + primary_offset_x];
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
                local_secondary_offset_y--;
                local_secondary_offset_y %= lens_dim;
                primary_offset_x++;
                primary_offset_x %= lens_dim;
            }
        }
        // Move buffers to end of this line
        dst += (dstpitch-width);
        src += (srcpitch-width);
        // Update other counters
        if ((h%lens_div) == 0)
        {
            local_secondary_offset_y += width;
            local_secondary_offset_y %= lens_dim;
            primary_offset_x -= width;
            primary_offset_x %= lens_dim;
            primary_offset_y++;
            primary_offset_y %= lens_dim;
            local_secondary_offset_x--;
            local_secondary_offset_x %= lens_dim;
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
