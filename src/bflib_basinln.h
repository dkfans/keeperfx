/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_basinln.h
 *     Inline functions for bflib_basics module.
 * @par Purpose:
 *     Define body of inline functions.
 * @par Comment:
 *     Functions defined as Inline, separated for compatibility with both Ansi-C and C++.
 * @author   Tomasz Lis
 * @date     10 Feb 2009 - 28 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_BASINLN_H
#define BFLIB_BASINLN_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

// Return the big-endian longword at p.
inline unsigned long blong (unsigned char *p)
{
    unsigned long n;
    n = p[0];
    n = (n << 8) + p[1];
    n = (n << 8) + p[2];
    n = (n << 8) + p[3];
    return n;
}

// Return the little-endian longword at p.
inline unsigned long llong (unsigned char *p)
{
    unsigned long n;
    n = p[3];
    n = (n << 8) + p[2];
    n = (n << 8) + p[1];
    n = (n << 8) + p[0];
    return n;
}

// Return the big-endian word at p.
inline unsigned long bword (unsigned char *p)
{
    unsigned long n;
    n = p[0];
    n = (n << 8) + p[1];
    return n;
}

// Return the little-endian word at p.
inline unsigned long lword (unsigned char *p)
{
    unsigned long n;
    n = p[1];
    n = (n << 8) + p[0];
    return n;
}

/*
 * Toggles a masked bit in the flags field to the value.
 * This version assumes the flag field is 1 byte long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 */
void inline toggle_flag_byte(unsigned char *flags,unsigned char mask)
{
  if ((*flags & mask) == 0)
    *flags |= mask;
  else
    *flags ^= mask;
}

/*
 * Toggles a masked bit in the flags field to the value.
 * This version assumes the flag field is 4 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 */
void inline toggle_flag_dword(unsigned long *flags,unsigned long mask)
{
  if ((*flags & mask) == 0)
    *flags |= mask;
  else
    *flags ^= mask;
}

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 2 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_word(unsigned short *flags,unsigned short mask,short value)
{
  if (value)
    *flags |= mask;
  else
    *flags ^= *flags & mask;
}

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 1 byte long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_byte(unsigned char *flags,unsigned char mask,short value)
{
  if (value)
    *flags |= mask;
  else
    *flags ^= *flags & mask;
}

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 4 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_dword(unsigned long *flags,unsigned long mask,short value)
{
  if (value)
    *flags |= mask;
  else
    *flags ^= *flags & mask;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
