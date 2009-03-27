/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode.h
 *     Header file for vidmode.c.
 * @par Purpose:
 *     Video mode switching/setting function.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_VIDMODE_H
#define DK_VIDMODE_H

#include "bflib_video.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

/******************************************************************************/
DLLIMPORT unsigned short _DK_pixels_per_block;
#define pixels_per_block _DK_pixels_per_block
DLLIMPORT unsigned short _DK_units_per_pixel;
#define units_per_pixel _DK_units_per_pixel
DLLIMPORT int _DK_MinimalResolutionSetup;
#define MinimalResolutionSetup _DK_MinimalResolutionSetup

#pragma pack()
/******************************************************************************/
TbScreenMode switch_to_next_video_mode(void);
void set_game_vidmode(unsigned short i,unsigned short nmode);
TbScreenMode reenter_video_mode(void);
TbScreenMode get_next_vidmode(unsigned short mode);
TbScreenMode validate_vidmode(unsigned short mode);
TbScreenMode get_failsafe_vidmode(void);
TbScreenMode get_movies_vidmode(void);
TbScreenMode get_frontend_vidmode(void);
void set_failsafe_vidmode(unsigned short nmode);
void set_movies_vidmode(unsigned short nmode);
void set_frontend_vidmode(unsigned short nmode);
char *get_vidmode_name(unsigned short mode);

short setup_screen_mode(unsigned short nmode);
short setup_screen_mode_minimal(unsigned short nmode);
short setup_screen_mode_zero(unsigned short nmode);

short LoadMcgaData(void);
short LoadMcgaDataMinimal(void);
short update_screen_mode_data(long width, long height);
void load_pointer_file(short hi_res);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
