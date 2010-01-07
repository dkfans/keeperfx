/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_bufrw.h
 *     Header file for bflib_bufrw.c.
 * @par Purpose:
 *     Reading/writing values in various bit formats into buffer.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     04 Jan 2009 - 10 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_BUFRW_H
#define BFLIB_BUFRW_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

long read_int32_le_buf (const unsigned char *buff);
unsigned short read_int16_le_buf (const unsigned char *buff);

void write_int16_le_buf (unsigned char *buff, unsigned short x);
void write_int32_le_buf (unsigned char *buff, unsigned long x);

long read_int32_be_buf (const unsigned char *buff);
unsigned short read_int16_be_buf (const unsigned char *buff);

void write_int16_be_buf (unsigned char *buff, unsigned short x);
void write_int32_be_buf (unsigned char *buff, unsigned long x);

unsigned char read_int8_buf (const unsigned char *buff);
void write_int8_buf (unsigned char *buff, unsigned char x);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
