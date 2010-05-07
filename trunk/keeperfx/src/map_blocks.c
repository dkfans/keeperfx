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

#include "slab_data.h"
#include "room_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
TbBool block_has_diggable_side(long plyr_idx, long slb_x, long slb_y)
{
  long i;
  for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
  {
    if (slab_is_safe_land(plyr_idx, slb_x + small_around[i].delta_x, slb_y + small_around[i].delta_y))
      return true;
  }
  return false;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
