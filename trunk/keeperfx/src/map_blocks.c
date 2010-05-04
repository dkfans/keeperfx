/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_blocks.c
 *     Map blocks support functions.
 * @par Purpose:
 *     Functions to manage map blocks.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_blocks.h"

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_block_has_diggable_side(long a1, long a2, long a3);

/******************************************************************************/
long block_has_diggable_side(long a1, long a2, long a3)
{
  return _DK_block_has_diggable_side(a1, a2, a3);
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
