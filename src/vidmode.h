/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode.h
 *     Header file for vidmode.c.
 *     Note that this file is a C header, while its code is CPP.
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

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_video.h"
#include "bflib_filelst.h"

#ifdef __cplusplus
extern "C" {
#endif

enum MousePointerGraphics {
    MousePG_Unkn00 = 0,
    MousePG_Unkn01,
    MousePG_Unkn02,
    MousePG_Unkn03,
    MousePG_Unkn04,
    MousePG_Unkn05,
    MousePG_Unkn06,
    MousePG_Unkn07,
    MousePG_Unkn08,
    MousePG_Unkn09,
    MousePG_Unkn10,
    MousePG_Unkn11,
    MousePG_Unkn12,
    MousePG_Unkn13,
    MousePG_Unkn14,
    MousePG_Unkn15,
    MousePG_Unkn16,
    MousePG_Unkn17,
    MousePG_Unkn18,
    MousePG_Unkn19,
    MousePG_Unkn20,
    MousePG_Unkn21,
    MousePG_Unkn22,
    MousePG_Unkn23,
    MousePG_Unkn24,
    MousePG_Unkn25,
    MousePG_Unkn26,
    MousePG_Unkn27,
    MousePG_Unkn28,
    MousePG_Unkn29,
    MousePG_Unkn30,
    MousePG_Unkn31,
    MousePG_Unkn32,
    MousePG_Unkn33,
    MousePG_Unkn34,
    MousePG_Unkn35,
    MousePG_Unkn36,
    MousePG_Unkn37,
    MousePG_Unkn38,
    MousePG_Unkn39,
    MousePG_Unkn40,
    MousePG_Unkn41,
    MousePG_Unkn42,
    MousePG_Unkn43,
    MousePG_Unkn44,
    MousePG_Unkn45,
    MousePG_Unkn46,
    MousePG_Unkn47,
    MousePG_Unkn48,
    MousePG_Unkn49,
};
/******************************************************************************/
#pragma pack(1)

struct TbColorTables {
  unsigned char fade_tables[64*256];
  unsigned char ghost[256*256];
  unsigned char flat_colours_tl[2*256];
  unsigned char flat_colours_tr[2*256];
  unsigned char flat_colours_br[2*256];
  unsigned char flat_colours_bl[2*256];
  unsigned char robs_bollocks[256];
};

struct TbAlphaTables {
    unsigned char black[256];
    unsigned char grey[8*256];
    unsigned char orange[8*256];
    unsigned char red[8*256];
    unsigned char blue[8*256];
    unsigned char green[8*256];
    // This is to force the array to have 256x256 size
    //unsigned char unused[215*256];
};
/******************************************************************************/
DLLIMPORT int _DK_MinimalResolutionSetup;
#define MinimalResolutionSetup _DK_MinimalResolutionSetup

DLLIMPORT struct TbColorTables _DK_pixmap;
#define pixmap _DK_pixmap
DLLIMPORT struct TbAlphaTables _DK_alpha_sprite_table;
#define alpha_sprite_table _DK_alpha_sprite_table
DLLIMPORT unsigned char _DK_white_pal[256];
#define white_pal _DK_white_pal
DLLIMPORT unsigned char _DK_red_pal[256];
#define red_pal _DK_red_pal

#pragma pack()
/******************************************************************************/
extern struct TbSprite *pointer_sprites;
extern struct TbLoadFiles legal_load_files[];
extern struct TbLoadFiles map_flag_load_files[];
extern struct TbLoadFiles netmap_flag_load_files[];
extern struct TbLoadFiles game_load_files[];
extern unsigned short units_per_pixel_min;
extern long base_mouse_sensitivity;
/******************************************************************************/
TbScreenMode switch_to_next_video_mode(void);
void set_game_vidmode(unsigned short i,unsigned short nmode);
unsigned short max_game_vidmode_count(void);
TbScreenMode reenter_video_mode(void);
TbScreenMode get_next_vidmode(TbScreenMode mode);
TbScreenMode get_higher_vidmode(TbScreenMode curr_mode);
TbScreenMode validate_vidmode(TbScreenMode mode);
TbScreenMode get_failsafe_vidmode(void);
TbScreenMode get_movies_vidmode(void);
TbScreenMode get_frontend_vidmode(void);
void set_failsafe_vidmode(unsigned short nmode);
void set_movies_vidmode(unsigned short nmode);
void set_frontend_vidmode(unsigned short nmode);
char *get_vidmode_name(unsigned short mode);

TbBool setup_screen_mode(unsigned short nmode);
short setup_screen_mode_minimal(unsigned short nmode);
TbBool setup_screen_mode_zero(unsigned short nmode);

short LoadMcgaData(void);
short LoadMcgaDataMinimal(void);
TbBool update_screen_mode_data(long width, long height);
void load_pointer_file(short hi_res);
TbBool load_testfont_fonts(void);
void free_testfont_fonts(void);

TbBool init_fades_table(void);
TbBool init_alpha_table(void);
void init_colours(void);

TbBool set_pointer_graphic_none(void);
TbBool set_pointer_graphic_menu(void);
TbBool set_pointer_graphic_spell(long group_idx, long frame);
TbBool set_pointer_graphic(long ptr_idx);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
