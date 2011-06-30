/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lens_flyeye.h
 *     Header file for lens_flyeye.cpp.
 * @par Purpose:
 *     lens_flyeye functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LENSFLYEYE_H
#define DK_LENSFLYEYE_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define CSCAN_STRIPS 26

struct CScan {
  long strips_num;
  unsigned short strip_len[CSCAN_STRIPS];
  short strip_w[CSCAN_STRIPS];
  short strip_h[CSCAN_STRIPS];
};

DLLIMPORT struct CScan *_DK_ScanBuffer;
//#define ScanBuffer _DK_ScanBuffer
extern struct CScan *ScanBuffer;

#pragma pack()
/******************************************************************************/
void flyeye_blitsec(unsigned char *srcbuf, long srcpitch, unsigned char *dstbuf, long dstpitch, long start_h, long end_h);
void flyeye_setup(long width, long height);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
