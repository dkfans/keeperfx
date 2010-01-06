/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_mist.h
 *     Header file for lens_mist.cpp.
 * @par Purpose:
 *     Mist lens effect functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LENSMIST_H
#define DK_LENSMIST_H

#include "globals.h"

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
extern CMistFade *Mist;
/******************************************************************************/
#endif
