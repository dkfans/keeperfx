/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file slab_data.c
 *     Map Slabs support functions.
 * @par Purpose:
 *     Definitions and functions to maintain map slabs.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Apr 2009 - 12 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "slab_data.h"
#include "globals.h"

#include "player_instances.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/

/******************************************************************************/
void reveal_whole_map(struct PlayerInfo *player)
{
  unsigned short nflag;
  unsigned int x,y,i;
  unsigned long imax;

  nflag = (1 << player->field_2B);
  for (y=0; y<map_tiles_y; y++)
    for (x=0; x<map_tiles_x; x++)
    {
      clear_slab_dig(x, y, player->field_2B);
    }
  imax = (map_subtiles_x+1)*(map_subtiles_y+1);
  for (i=0; i<imax; i++)
  {
    x = (game.map[i].data >> 28) | nflag;
    game.map[i].data |= (x & 0x0F) << 28;
  }
  pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
