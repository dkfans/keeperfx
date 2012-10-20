/******************************************************************************/
/** @file lbfileio.c
 * Library for r/w of little/big endian binary data from files.
 * @par Purpose:
 *   Little-endian and big-endian file/buffer reading functions.
 * @par Comment:
 *   Those are simple, low-level functions, so all are defined as inline.
 * @author   Tomasz Lis
 * @date     11 Mar 2005 - 22 Jul 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lbfileio.h"

/**
 * Writes 2-byte little-endian number to given FILE.
 */
inline void write_int16_le_file (FILE *fp, unsigned short x)
{
    fputc ((int) (x&255), fp);
    fputc ((int) ((x>>8)&255), fp);
}

/**
 * Writes 2-byte little-endian number into given buffer.
 */
inline void write_int16_le_buf (unsigned char *buff, unsigned short x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
}

/**
 * Writes 4-byte little-endian number to given FILE.
 */
inline void write_int32_le_file (FILE *fp, unsigned long x)
{
    fputc ((int) (x&255), fp);
    fputc ((int) ((x>>8)&255), fp);
    fputc ((int) ((x>>16)&255), fp);
    fputc ((int) ((x>>24)&255), fp);
}

/**
 * Writes 4-byte little-endian number into given buffer.
 */
inline void write_int32_le_buf (unsigned char *buff, unsigned long x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
    buff[2]=((x>>16)&255);
    buff[3]=((x>>24)&255);
}

/**
 * Reads 4-byte little-endian number from given FILE.
 */
inline long read_int32_le_file (FILE *fp)
{
    long l;
    l = fgetc(fp);
    l += fgetc(fp)<<8;
    l += fgetc(fp)<<16;
    l += fgetc(fp)<<24;
    return l;
}

/**
 * Reads 4-byte little-endian number from given buffer.
 */
inline long read_int32_le_buf (const unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    l += buff[2]<<16;
    l += buff[3]<<24;
    return l;
}

/**
 * Reads 2-byte little-endian number from given buffer.
 */
inline unsigned short read_int16_le_buf (const unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    return l;
}

/**
 * Reads 2-byte little-endian number from given FILE.
 */
inline unsigned short read_int16_le_file (FILE *fp)
{
    unsigned short l;
    l = fgetc(fp);
    l += fgetc(fp)<<8;
    return l;
}

/**
 * Reads 4-byte big-endian number from given FILE.
 */
inline long read_int32_be_file (FILE *fp)
{
    long l;
    l = fgetc(fp)<<24;
    l += fgetc(fp)<<16;
    l += fgetc(fp)<<8;
    l += fgetc(fp);
    return l;
}

/**
 * Reads 4-byte big-endian number from given buffer.
 */
inline long read_int32_be_buf (const unsigned char *buff)
{
    long l;
    l =  buff[3];
    l += buff[2]<<8;
    l += buff[1]<<16;
    l += buff[0]<<24;
    return l;
}

/**
 * Reads 2-byte big-endian number from given buffer.
 */
inline unsigned short read_int16_be_buf (const unsigned char *buff)
{
    long l;
    l = buff[1];
    l += buff[0]<<8;
    return l;
}

/**
 * Reads 2-byte big-endian number from given FILE.
 */
inline unsigned short read_int16_be_file (FILE *fp)
{
    unsigned short l;
    l = fgetc(fp)<<8;
    l += fgetc(fp);
    return l;
}

/**
 * Writes 2-byte big-endian number into given buffer.
 */
inline void write_int16_be_buf (unsigned char *buff, unsigned short x)
{
    buff[1]=(x&255);
    buff[0]=((x>>8)&255);
}

/**
 * Writes 4-byte big-endian number into given buffer.
 */
inline void write_int32_be_buf (unsigned char *buff, unsigned long x)
{
    buff[3]=(x&255);
    buff[2]=((x>>8)&255);
    buff[1]=((x>>16)&255);
    buff[0]=((x>>24)&255);
}

/**
 * Returns length of given file.
 * Value -1 means error.
 */
inline long file_length (char *path)
{
    FILE *fp;
    long length;
    
    fp = fopen (path, "rb");
    if (fp==NULL)
      return -1;
    if (fseek(fp, 0, SEEK_END) != 0)
      return -1;
    length = ftell (fp);
    fclose (fp);
    return length;
}

/**
 * Returns length of opened file.
 * Value -1 means error.
 */
inline long file_length_opened (FILE *fp)
{
    long length;
    long lastpos;
    
    if (fp==NULL)
      return -1;
    lastpos = ftell (fp);
    if (fseek(fp, 0, SEEK_END) != 0)
      return -1;
    length = ftell(fp);
    fseek(fp, lastpos, SEEK_SET);
    return length;
}

/**
 * Reads 1-byte number from given buffer.
 * Simple wrapper for use with both little and big endian files.
 */
inline unsigned char read_int8_buf (const unsigned char *buff)
{
    return buff[0];
}

/**
 * Returns the nth bit of character c.
 * @param c Source character.
 * @param n Source bit number.
 * @return The nth bit, either 1 or 0.
 */
inline short nth_bit( unsigned char c, short n )
{
    if( (n<0) || (n>7) )
        return 0;
    c = c>>n;
    return (short)(c & 1);
    
}

/**
 * Returns the nth bit of c.
 * Goes like this:
 * [31 30 29 28 27 26 25 24][...][...][7 6 5 4 3 2 1 0]
 * @param c Source characters array.
 * @param n Source bit number.
 * @return The nth bit, either 1 or 0.
 */
inline short nth_bit_fourbytes( unsigned char c[4], short n )
{
    if( (n<0) || (n>32) )
        return 0;
    
    if( n < 8 )    // bits 0 - 7
        return nth_bit( c[3], n );
    else if( n < 16 )
        return nth_bit( c[2], n%8 );
    else if( n < 24 )
        return nth_bit( c[1], n%8 );
    else
        return nth_bit( c[0], n%8 );
    
}
