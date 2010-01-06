/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_camera.c
 *     Camera move, maintain and support functions.
 * @par Purpose:
 *     Defines and maintains cameras.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_camera.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"

#include "engine_lenses.h"
#include "engine_render.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long get_3d_box_distance(struct Coord3d *pos1, struct Coord3d *pos2)
{
  long dx;
  long dy;
  long dz;
  dy = abs(pos2->y.val - pos1->y.val);
  dx = abs(pos2->x.val - pos1->x.val);
  if (dy <= dx)
    dy = dx;
  dz = abs(pos2->z.val - pos1->z.val);
  if (dy <= dz)
    dy = dz;
  return dy;
}

/******************************************************************************/

/******************************************************************************/



/******************************************************************************/
#ifdef __cplusplus
}
#endif
