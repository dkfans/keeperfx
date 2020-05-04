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
    MousePG_Invisible = 0,
    MousePG_Arrow,
    MousePG_Pickaxe,
    MousePG_Sell,
    MousePG_Query,
    MousePG_PlaceTrap01,
    MousePG_PlaceTrap02,
    MousePG_PlaceTrap03,
    MousePG_PlaceTrap04,
    MousePG_PlaceTrap05,
    MousePG_PlaceTrap06,
    MousePG_PlaceDoor01,
    MousePG_PlaceDoor02,
    MousePG_PlaceDoor03,
    MousePG_PlaceDoor04,
    MousePG_DenyMark,
    MousePG_SpellCharge0,
    MousePG_SpellCharge1,
    MousePG_SpellCharge2,
    MousePG_SpellCharge3,
    MousePG_SpellCharge4,
    MousePG_SpellCharge5,
    MousePG_SpellCharge6,
    MousePG_SpellCharge7,
    MousePG_SpellCharge8,
    MousePG_PlaceRoom01,
    MousePG_PlaceRoom02,
    MousePG_PlaceRoom03,
    MousePG_PlaceRoom04,
    MousePG_PlaceRoom05,
    MousePG_PlaceRoom06,
    MousePG_PlaceRoom07,
    MousePG_PlaceRoom08,
    MousePG_PlaceRoom09,
    MousePG_PlaceRoom10,
    MousePG_PlaceRoom11,
    MousePG_PlaceRoom12,
    MousePG_PlaceRoom13,
    MousePG_PlaceRoom14,
    MousePG_LockMark,
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
