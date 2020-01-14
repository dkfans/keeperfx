/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_bufrw.c
 *     Reading/writing values in various bit formats into buffer.
 * @par Purpose:
 *     Allows writing little-endian and big-endian values.
 * @par Comment:
 *     These functions are not defined as inline because of C-to-C++
 *     transition problem with inline functions.
 * @author   Tomasz Lis
 * @date     04 Jan 2009 - 10 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_bufrw.h"

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/**
 * Writes 2-byte little-endian number into given buffer.
 */
void write_int16_le_buf (unsigned char *buff, unsigned short x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
}

/**
 * Writes 4-byte little-endian number into given buffer.
 */
void write_int32_le_buf (unsigned char *buff, unsigned long x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
    buff[2]=((x>>16)&255);
    buff[3]=((x>>24)&255);
}

/**
 * Reads 4-byte little-endian number from given buffer.
 */
long read_int32_le_buf (const unsigned char *buff)
{
    long l = buff[0];
    l += buff[1]<<8;
    l += buff[2]<<16;
    l += buff[3]<<24;
    return l;
}

/**
 * Reads 2-byte little-endian number from given buffer.
 */
unsigned short read_int16_le_buf (const unsigned char *buff)
{
    long l = buff[0];
    l += buff[1]<<8;
    return l;
}

/**
 * Reads 4-byte big-endian number from given buffer.
 */
long read_int32_be_buf (const unsigned char *buff)
{
    long l = buff[3];
    l += buff[2]<<8;
    l += buff[1]<<16;
    l += buff[0]<<24;
    return l;
}

/**
 * Reads 2-byte big-endian number from given buffer.
 */
unsigned short read_int16_be_buf (const unsigned char *buff)
{
    long l = buff[1];
    l += buff[0]<<8;
    return l;
}

/**
 * Writes 2-byte big-endian number into given buffer.
 */
void write_int16_be_buf (unsigned char *buff, unsigned short x)
{
    buff[1]=(x&255);
    buff[0]=((x>>8)&255);
}

/**
 * Writes 4-byte big-endian number into given buffer.
 */
void write_int32_be_buf (unsigned char *buff, unsigned long x)
{
    buff[3]=(x&255);
    buff[2]=((x>>8)&255);
    buff[1]=((x>>16)&255);
    buff[0]=((x>>24)&255);
}

/**
 * Reads 1-byte number from given buffer.
 * Simple wrapper for use with both little and big endian files.
 */
unsigned char read_int8_buf (const unsigned char *buff)
{
    return buff[0];
}

/**
 * Writes 1-byte number into given buffer.
 * Simple wrapper for use with both little and big endian files.
 */
void write_int8_buf (unsigned char *buff, unsigned char x)
{
    buff[0]=x;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
