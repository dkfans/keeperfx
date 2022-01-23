/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_simple.h
 *     Header file for front_simple.c.
 * @par Purpose:
 *     Simple frontend screens support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2009 - 23 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_FRONT_SIMPL_H
#define DK_FRONT_SIMPL_H

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

enum RawBitmaps {
    RBmp_None               =  0x00,
    RBmp_WaitLoading        =  0x01,
    RBmp_WaitNoCD           =  0x02,
    RBmp_SplashLegal        =  0x03,
    RBmp_SplashFx           =  0x04,
};

struct RawBitmap {
  const char *name;
  int width;
  int height;
  int bpp;
  short fgroup;
  const char *raw_fname;
  const char *pal_fname;
};

struct ActiveBitmap {
  const char *name;
  int width;
  int height;
  int bpp;
  TbClockMSec start_tm;
  TbPixel *raw_data;
  unsigned char *pal_data;
};

/******************************************************************************/
extern unsigned char palette_buf[PALETTE_SIZE];
/******************************************************************************/
DLLIMPORT extern unsigned char *_DK_palette;
#define engine_palette _DK_palette
DLLIMPORT extern unsigned char *_DK_scratch;
#define scratch _DK_scratch
/******************************************************************************/
TbBool copy_raw8_image_buffer(unsigned char *dst_buf,const int scanline,const int nlines,const int dst_width,const int dst_height,
    const int spw,const int sph,const unsigned char *src_buf,const int src_width,const int src_height);
TbBool copy_raw8_image_to_screen_center(const unsigned char *buf,const int img_width,const int img_height);
TbBool show_rawimage_screen(unsigned char *raw,unsigned char *pal,int width,int height,TbClockMSec tmdelay);
/******************************************************************************/
TbBool draw_clear_screen(void);
TbBool init_actv_bitmap_screen(int stype);
TbBool free_actv_bitmap_screen(void);
TbBool draw_actv_bitmap_screen(void);
TbBool show_actv_bitmap_screen(TbClockMSec tmdelay);
/******************************************************************************/

TbBool display_loading_screen(void);
TbBool wait_for_cd_to_be_available(void);
TbBool display_centered_message(long showTime, char *text);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
