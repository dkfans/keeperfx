/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_ceiling.h
 *     Header file for map_ceiling.c.
 * @par Purpose:
 *     Map ceiling support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   keeperFx Team
 * @date     12 Nov 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_MAP_CEILING_H
#define DK_MAP_CEILING_H


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey);
long ceiling_init(unsigned long a1, unsigned long a2);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
