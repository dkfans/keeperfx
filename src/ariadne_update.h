/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_tringls.h
 *     Header file for ariadne_tringls.c.
 * @par Purpose:
 *     Triangles array for Ariadne system support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 22 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_UPDATE_H
#define DK_ARIADNE_UPDATE_H

#include "bflib_basics.h"
#include "globals.h"


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

/******************************************************************************/
long update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y);
long init_navigation(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
