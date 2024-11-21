/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_string.h
 *     Header file for bflib_string.c.
 * @par Purpose:
 *     Text strings support - functions for string manipulation.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Feb 2012 - 20 Jun 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_STRING_H
#define BFLIB_STRING_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** Locate dependent char definition.
 *  Redefines char so that it's easier to know whether a string should
 *  be manipulated with LbLocText*() routines.
 */
typedef char TbLocChar;
typedef int TbCharCount;
/******************************************************************************/
TbCharCount LbLocTextStringLength(const TbLocChar *s);
TbSize LbLocTextStringSize(const TbLocChar *s);
TbSize LbLocTextPosToLength(const TbLocChar *s, TbCharCount pos);
TbLocChar *LbLocTextStringConcat(TbLocChar *str, const TbLocChar *catstr, TbSize maxlen);
TbLocChar *LbLocTextStringInsert(TbLocChar *str, const TbLocChar *catstr, TbCharCount pos, TbSize maxlen);
TbLocChar *LbLocTextStringDelete(TbLocChar *str, TbCharCount pos, TbCharCount count);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
