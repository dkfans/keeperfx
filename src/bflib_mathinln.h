/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mathinln.h
 *     Inline functions for bflib_math module.
 * @par Purpose:
 *     Define body of inline functions.
 * @par Comment:
 *     Functions defined as Inline, separated for compatibility with both Ansi-C and C++.
 * @author   Tomasz Lis
 * @date     24 Jan 2009 - 31 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_MATHINLN_H
#define BFLIB_MATHINLN_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

inline long LbSinL(long x)
{
  return lbSinTable[(unsigned long)x & 0x7FF];
}

inline long LbCosL(long x)
{
  return lbCosTable[(unsigned long)x & 0x7FF];
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
