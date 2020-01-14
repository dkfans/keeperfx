/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_string.c
 *     Text strings support - functions for string manipulation.
 * @par Purpose:
 *     Enables support of locale-dependant strings.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Feb 2012 - 20 Jun 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_string.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
/**
 * Returns amount of characters in a locale-dependant string.
 * @param s The source string.
 * @return Amount of characters in the string.
 */
TbCharCount LbLocTextStringLength(const TbLocChar *s)
{
    TbCharCount j = 0;
    TbSize i = 0;
    while (s[i] != 0)
    {
      //if ((s[i] & 0xc0) != 0x80) // enable when/if UTF-8 is supported
      {
          j++;
      }
      i++;
    }
    return j;
}

/**
 * Returns size of a locale-dependant string.
 * @param s The source string.
 * @return Amount of bytes in the string.
 */
TbSize LbLocTextStringSize(const TbLocChar *s)
{
    TbSize i = 0;
    while (s[i] != 0) {
      i++;
    }
    return i;
}

/**
 * Converts string char count to position in specific string.
 * @param s The source string.
 * @return Amount of bytes in the string.
 */
TbSize LbLocTextPosToLength(const TbLocChar *s, TbCharCount pos)
{
    TbCharCount j = 0;
    TbSize i = 0;
    while ((s[i] != 0) && (j < pos))
    {
      //if ((s[i] & 0xc0) != 0x80) // enable when/if UTF-8 is supported
      {
          j++;
      }
      i++;
    }
    return i;
}

/**
 * Apples the second string at end of first string.
 */
TbLocChar *LbLocTextStringConcat(TbLocChar *str,const TbLocChar *catstr, TbSize maxlen)
{
   return strncat(str,catstr,maxlen);
}

/**
 * Inserts the second string to the first string.
 */
TbLocChar *LbLocTextStringInsert(TbLocChar *str,const TbLocChar *catstr, TbCharCount pos, TbSize maxlen)
{
    TbSize spos = LbLocTextPosToLength(str, pos);
    TbSize slen = LbLocTextStringSize(str);
    TbSize clen = LbLocTextStringSize(catstr);
    // Check if we're ok
    if (slen < spos)
        spos = slen;
    if (slen+clen >= maxlen)
        return NULL;
    // Make place for the string to insert
    for (TbSize i = slen - spos; i > 0; i--)
    {
        str[spos+clen+i-1] = str[spos+i-1];
    }
    str[slen+clen] = '\0';
    // And fill the string
    memcpy(str+spos,catstr,clen);
    return str;
}

/**
 * Deletes part of a string.
 */
TbLocChar *LbLocTextStringDelete(TbLocChar *str, TbCharCount pos, TbCharCount count)
{
    TbSize spos = LbLocTextPosToLength(str, pos);
    TbSize slen = LbLocTextStringSize(str);
    TbSize clen = LbLocTextPosToLength(str, pos + count) - spos;
    // Check if we're ok
    if (slen < spos)
        return NULL;
    if (spos+clen > slen)
        clen = slen-spos;
    // Remove the chars
    TbSize lim = slen - spos - clen;
    for (TbSize i = 0; i < lim; i++)
    {
        str[spos+i] = str[spos+clen+i];
    }
    str[slen-clen] = '\0';
    return str;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
