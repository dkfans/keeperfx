/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_render_data.cpp
 *     Colour arrays for drawing stripey lines.
 * @par Purpose:
 *     Provides a set of colours to use when drawing a stripey line, e.g. for a bounding box. Used by draw_stripey_line() in engine_render.c.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     07 Sep 2020 - 07 Sep 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_render.h"

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

// uses a color of the given ID from the palette: MAIN.PAL
struct stripey_line      basic_stripey_line = { { 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x00 },              0 }; // example
struct stripey_line     basic_stripey_line2 = { { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x08 },              0 }; // example
struct stripey_line        red_stripey_line = { { 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x47, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40, 0x40 },        SLC_RED };
struct stripey_line      green_stripey_line = { { 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa7, 0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0xa2, 0xa1, 0xa0, 0xa0 },      SLC_GREEN };
struct stripey_line     yellow_stripey_line = { { 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb7, 0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0, 0xb0 },     SLC_YELLOW };
struct stripey_line       grey_stripey_line = { { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x08 },       SLC_GREY };
struct stripey_line     purple_stripey_line = { { 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x6f, 0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68, 0x68 },     SLC_PURPLE };
struct stripey_line  dark_blue_stripey_line = { { 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x57, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50, 0x50 },  SLC_DARK_BLUE };
struct stripey_line dark_brown_stripey_line = { { 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x37, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30, 0x30 }, SLC_DARK_BROWN };

struct stripey_line colored_stripey_lines[STRIPEY_LINE_COLOR_COUNT] = { 
    red_stripey_line,
    green_stripey_line,
    yellow_stripey_line,
    grey_stripey_line,
    purple_stripey_line,
    dark_blue_stripey_line,
    dark_brown_stripey_line,
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
