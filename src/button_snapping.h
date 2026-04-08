/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file button_snapping.h
 *     Header file for button_snapping.c.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BUTTON_SNAPPING_H
#define BUTTON_SNAPPING_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Exported functions
void snap_to_direction(long mouse_x, long mouse_y, float dx, float dy);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
