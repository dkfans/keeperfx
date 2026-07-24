/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_update.h
 *     Header file for ariadne_update.c.
 * @par Purpose:
 *     map updates for Ariadne system support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
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
int32_t update_navigation_triangulation(int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y);
int32_t init_navigation(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
