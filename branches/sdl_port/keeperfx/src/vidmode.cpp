/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode.c
 *     Video mode switching/setting function.
 * @par Purpose:
 *     Functions to change video mode in DK way.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis, KeeperFX Team
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "vidmode.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"
#include "bflib_dernc.h"
#include "bflib_sprfnt.h"
#include "bflib_filelst.h"
#include "bflib_drawsdk.hpp"

#include "front_simple.h"
#include "front_landview.h"
#include "frontend.h"
#include "gui_draw.h"
#include "config.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
//DLLIMPORT struct TbLoadFiles _DK_hi_res_small_pointer_load_files[3];
//DLLIMPORT struct TbLoadFiles _DK_low_res_small_pointer_load_files[3];
//DLLIMPORT struct TbLoadFiles _DK_hi_res_pointer_load_files[3];
//DLLIMPORT struct TbLoadFiles _DK_low_res_pointer_load_files[3];
//DLLIMPORT struct TbLoadFiles _DK_mcga_load_files[8];
//DLLIMPORT struct TbLoadFiles _DK_mcga_load_files_minimal[1];
//DLLIMPORT struct TbLoadFiles _DK_vres256_load_files[9];
//DLLIMPORT struct TbLoadFiles _DK_vres256_load_files_minimal[11];
//DLLIMPORT struct TbSetupSprite _DK_setup_sprites_minimal[5];
//DLLIMPORT struct TbSetupSprite _DK_setup_sprites[8];
//DLLIMPORT struct TbSprite *_DK_pointer_sprites;
//DLLIMPORT struct TbSprite *_DK_end_pointer_sprites;
//DLLIMPORT unsigned long _DK_pointer_data;


DLLIMPORT int _DK_setup_screen_mode(short nmode);
DLLIMPORT int _DK_setup_screen_mode_minimal(short nmode);
/******************************************************************************/
TbScreenMode switching_vidmodes[] = {
  { 320, 200, 8 },
  { 640, 400, 8 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 }
  };

TbScreenMode failsafe_vidmode = { 320, 200, 8 };
TbScreenMode movies_vidmode   = { 320, 200, 8 };
TbScreenMode frontend_vidmode = { 640, 480, 8 };

//struct IPOINT_2D units_per_pixel;
/******************************************************************************/
struct TbSprite *pointer_sprites;
struct TbSprite *end_pointer_sprites;
unsigned long pointer_data;

struct TbSetupSprite setup_sprites_minimal[] = {
  {&frontend_font[0],     &frontend_end_font[0],  &frontend_font_data[0]},
  {&frontend_font[1],     &frontend_end_font[1],  &frontend_font_data[1]},
  {&frontend_font[2],     &frontend_end_font[2],  &frontend_font_data[2]},
  {&frontend_font[3],     &frontend_end_font[3],  &frontend_font_data[3]},
  {NULL,                  NULL,                   NULL},
};

struct TbSetupSprite setup_sprites[] = {
  {&pointer_sprites,      &end_pointer_sprites,   &pointer_data},
  {&font_sprites,         &end_font_sprites,      &font_data},
  {&edit_icon_sprites,    &end_edit_icon_sprites, &edit_icon_data},
  {&winfont,              &end_winfonts,          &winfont_data},
  {&button_sprite,        &end_button_sprites,    &button_sprite_data},
  {&port_sprite,          &end_port_sprites,      &port_sprite_data},
  {&gui_panel_sprites,    &end_gui_panel_sprites, &gui_panel_sprite_data},
  {NULL,                  NULL,                   NULL},
};

#if (BFDEBUG_LEVEL > 0)
// Declarations for font testing screen (debug version only)
struct TbSetupSprite setup_testfont[] = {
  {&testfont[0],          &testfont_end[0],       &testfont_data[0]},
  {&testfont[1],          &testfont_end[1],       &testfont_data[1]},
  {&testfont[2],          &testfont_end[2],       &testfont_data[2]},
  {&testfont[3],          &testfont_end[3],       &testfont_data[3]},
  {&testfont[4],          &testfont_end[4],       &testfont_data[4]},
  {&testfont[5],          &testfont_end[5],       &testfont_data[5]},
  {&testfont[6],          &testfont_end[6],       &testfont_data[6]},
  {&testfont[7],          &testfont_end[7],       &testfont_data[7]},
  {&testfont[8],          &testfont_end[8],       &testfont_data[8]},
  {&testfont[9],          &testfont_end[9],       &testfont_data[9]},
  {&testfont[10],         &testfont_end[10],      &testfont_data[10]},
  {NULL,                  NULL,                   NULL},
};

struct TbLoadFiles testfont_load_files[] = {
  {"ldata/frontft1.dat", (unsigned char **)&testfont_data[0], NULL,                                          0, 0, 0},
  {"ldata/frontft1.tab", (unsigned char **)&testfont[0],      (unsigned char **)&testfont_end[0],            0, 0, 0},
  {"ldata/frontft2.dat", (unsigned char **)&testfont_data[1], NULL,                                          0, 0, 0},
  {"ldata/frontft2.tab", (unsigned char **)&testfont[1],      (unsigned char **)&testfont_end[1],            0, 0, 0},
  {"ldata/frontft3.dat", (unsigned char **)&testfont_data[2], NULL,                                          0, 0, 0},
  {"ldata/frontft3.tab", (unsigned char **)&testfont[2],      (unsigned char **)&testfont_end[2],            0, 0, 0},
  {"ldata/frontft4.dat", (unsigned char **)&testfont_data[3], NULL,                                          0, 0, 0},
  {"ldata/frontft4.tab", (unsigned char **)&testfont[3],      (unsigned char **)&testfont_end[3],            0, 0, 0},
  {"data/font0-0.dat",   (unsigned char **)&testfont_data[4], NULL,                                          0, 0, 0},
  {"data/font0-0.tab",   (unsigned char **)&testfont[4],      (unsigned char **)&testfont_end[4],            0, 0, 0},
  {"data/font0-1.dat",   (unsigned char **)&testfont_data[5], NULL,                                          0, 0, 0},
  {"data/font0-1.tab",   (unsigned char **)&testfont[5],      (unsigned char **)&testfont_end[5],            0, 0, 0},
  {"data/font2-0.dat",   (unsigned char **)&testfont_data[6], NULL,                                          0, 0, 0},
  {"data/font2-0.tab",   (unsigned char **)&testfont[6],      (unsigned char **)&testfont_end[6],            0, 0, 0},
  {"data/font2-1.dat",   (unsigned char **)&testfont_data[7], NULL,                                          0, 0, 0},
  {"data/font2-1.tab",   (unsigned char **)&testfont[7],      (unsigned char **)&testfont_end[7],            0, 0, 0},
  {"data/hifont.dat",    (unsigned char **)&testfont_data[8], NULL,                                          0, 0, 0},
  {"data/hifont.tab",    (unsigned char **)&testfont[8],      (unsigned char **)&testfont_end[8],            0, 0, 0},
  {"data/lofont.dat",    (unsigned char **)&testfont_data[9], NULL,                                          0, 0, 0},
  {"data/lofont.tab",    (unsigned char **)&testfont[9],      (unsigned char **)&testfont_end[9],            0, 0, 0},
  {"ldata/netfont.dat",  (unsigned char **)&testfont_data[10],NULL,                                          0, 0, 0},
  {"ldata/netfont.tab",  (unsigned char **)&testfont[10],     (unsigned char **)&testfont_end[10],           0, 0, 0},
  {"data/frontend.pal",  (unsigned char **)&testfont_palette[0],NULL,                                        0, 0, 0},
  {"data/palette.dat",   (unsigned char **)&testfont_palette[1],NULL,                                        0, 0, 0},
  {"",                    NULL,                               NULL,                                          0, 0, 0},
};
#endif

struct TbLoadFiles mcga_load_files[] = {
  {"data/gui.dat",       (unsigned char **)&button_sprite_data,    (unsigned char **)&end_button_sprite_data,      0, 0, 0},
  {"data/gui2-0-0.dat",  (unsigned char **)&gui_panel_sprite_data, (unsigned char **)&end_gui_panel_sprite_data,   0, 0, 0},
  {"data/gui.tab",       (unsigned char **)&button_sprite,         (unsigned char **)&end_button_sprites,          0, 0, 0},
  {"data/font2-0.dat",   (unsigned char **)&winfont_data,          (unsigned char **)&end_winfont_data,            0, 0, 0},
  {"data/font2-0.tab",   (unsigned char **)&winfont,               (unsigned char **)&end_winfonts,                0, 0, 0},
  {"data/lofont.dat",    (unsigned char **)&font_data,             NULL,                                           0, 0, 0},
  {"data/lofont.tab",    (unsigned char **)&font_sprites,          (unsigned char **)&end_font_sprites,            0, 0, 0},
  {"data/slab0-0.dat",   (unsigned char **)&gui_slab,              NULL,                                           0, 0, 0},
  {"data/gui2-0-0.tab",  (unsigned char **)&gui_panel_sprites,     (unsigned char **)&end_gui_panel_sprites,       0, 0, 0},
  {"",                    NULL,                                     NULL,                                           0, 0, 0},
};

struct TbLoadFiles vres256_load_files[] = {
  {"data/guihi.dat",     (unsigned char **)&button_sprite_data,    (unsigned char **)&end_button_sprite_data,      0, 0, 0},
  {"data/guihi.tab",     (unsigned char **)&button_sprite,         (unsigned char **)&end_button_sprites,          0, 0, 0},
  {"data/font2-1.dat",   (unsigned char **)&winfont_data,          (unsigned char **)&end_winfont_data,            0, 0, 0},
  {"data/font2-1.tab",   (unsigned char **)&winfont,               (unsigned char **)&end_winfonts,                0, 0, 0},
  {"data/hifont.dat",    (unsigned char **)&font_data,             NULL,                                           0, 0, 0},
  {"data/hifont.tab",    (unsigned char **)&font_sprites,          (unsigned char **)&end_font_sprites,            0, 0, 0},
  {"data/slab0-1.dat",   (unsigned char **)&gui_slab,              NULL,                                           0, 0, 0},
  {"data/gui2-0-1.dat",  (unsigned char **)&gui_panel_sprite_data, (unsigned char **)&end_gui_panel_sprite_data,   0, 0, 0},
  {"data/gui2-0-1.tab",  (unsigned char **)&gui_panel_sprites,     (unsigned char **)&end_gui_panel_sprites,       0, 0, 0},
  {"*B_SCREEN",           (unsigned char **)&hires_parchment,       NULL,                                     640*480, 0, 0},
  {"",                    NULL,                                     NULL,                                           0, 0, 0},
};

struct TbLoadFiles mcga_load_files_minimal[] = {
  {"",                    NULL,                                     NULL,                                          0, 0, 0},
};

struct TbLoadFiles vres256_load_files_minimal[] = {
  {"ldata/frontft1.dat", (unsigned char **)&frontend_font_data[0], (unsigned char **)&frontend_end_font_data[0],  0, 0, 0},
  {"ldata/frontft1.tab", (unsigned char **)&frontend_font[0],      (unsigned char **)&frontend_end_font[0],       0, 0, 0},
  {"ldata/frontft2.dat", (unsigned char **)&frontend_font_data[1], (unsigned char **)&frontend_end_font_data[1],  0, 0, 0},
  {"ldata/frontft2.tab", (unsigned char **)&frontend_font[1],      (unsigned char **)&frontend_end_font[1],       0, 0, 0},
  {"ldata/frontft3.dat", (unsigned char **)&frontend_font_data[2], (unsigned char **)&frontend_end_font_data[2],  0, 0, 0},
  {"ldata/frontft3.tab", (unsigned char **)&frontend_font[2],      (unsigned char **)&frontend_end_font[2],       0, 0, 0},
  {"ldata/frontft4.dat", (unsigned char **)&frontend_font_data[3], (unsigned char **)&frontend_end_font_data[3],  0, 0, 0},
  {"ldata/frontft4.tab", (unsigned char **)&frontend_font[3],      (unsigned char **)&frontend_end_font[3],       0, 0, 0},
//  {"levels/levels.txt",  (unsigned char **)&level_names_data,      (unsigned char **)&end_level_names_data,       0, 0, 0},
  {"*FE_BACKUP_PAL",      (unsigned char **)&frontend_backup_palette,NULL,                                       768, 0, 0},
  {"",                    NULL,                                     NULL,                                          0, 0, 0},
};

struct TbLoadFiles low_res_pointer_load_files[] = {
  {"data/lpointer.dat",   (unsigned char **)&pointer_data,          NULL,                                          0, 0, 0},
  {"data/lpointer.tab",   (unsigned char **)&pointer_sprites,       (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                    NULL,                                     NULL,                                          0, 0, 0},
};

struct TbLoadFiles low_res_small_pointer_load_files[] = {
  {"data/lpoints.dat",    (unsigned char **)&pointer_data,          NULL,                                          0, 0, 0},
  {"data/lpoints.tab",    (unsigned char **)&pointer_sprites,      (unsigned char **)&end_pointer_sprites,         0, 0, 0},
  {"",                    NULL,                                     NULL,                                          0, 0, 0},
};

struct TbLoadFiles hi_res_pointer_load_files[] = {
  {"data/hpointer.dat",   (unsigned char **)&pointer_data,          NULL,                                          0, 0, 0},
  {"data/hpointer.tab",   (unsigned char **)&pointer_sprites,       (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                    NULL,                                     NULL,                                          0, 0, 0},
};

struct TbLoadFiles hi_res_small_pointer_load_files[] = {
  {"data/hpoints.dat",    (unsigned char **)&pointer_data,          NULL,                                          0, 0, 0},
  {"data/hpoints.tab",    (unsigned char **)&pointer_sprites,       (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                    NULL,                                     NULL,                                          0, 0, 0},
};

//DLLIMPORT extern struct TbLoadFiles _DK_legal_load_files[];
struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE", &_DK_palette, NULL, PALETTE_SIZE, 0, 0},
    {"*SCRATCH", &scratch, NULL, 0x10000, 1, 0},
    {"", NULL, NULL, 0, 0, 0}, };

/*
unsigned char *nocd_raw;
unsigned char *nocd_pal;

struct TbLoadFiles nocd_load_files[] = {
    {"data/nocd.raw", &nocd_raw, NULL, 0, 0, 0},
    {"data/nocd.pal", &nocd_pal, NULL, 0, 0, 0},
    {"", NULL, NULL, 0, 0, 0}, };
*/

struct TbLoadFiles map_flag_load_files[] = {
  {"ldata/dkflag00.dat", (unsigned char **)&map_flag_data,(unsigned char **)&end_map_flag_data, 0, 0, 0},
  {"ldata/dkflag00.tab", (unsigned char **)&map_flag,     (unsigned char **)&end_map_flag,      0, 0, 0},
  {"",                   NULL,                            NULL,                                 0, 0, 0},
};
/*
struct TbSetupSprite map_flag_setup_sprites[] = {
  {&map_flag, &end_map_flag, &map_flag_data},
  {NULL,      NULL,          NULL,},
};

struct TbSetupSprite netmap_flag_setup_sprites[] = {
  {&map_flag, &end_map_flag, &map_flag_data},
  {&map_font, &end_map_font, &map_font_data},
  {&map_hand, &end_map_hand, &map_hand_data},
  {NULL,      NULL,          NULL,},
};
*/

short force_video_mode_reset = true;
static TbScreenMode activeScreenMode; //new screen mode tracking variable, replacement for lbDisplay.ScreenMode


/******************************************************************************/


/*
 * Loads VGA 256 graphics files, for high resolution modes.
 * @return Returns true if all files were loaded, false otherwise.
 */
short LoadVRes256Data(long scrbuf_size)
{
  int i;
  // Update size of the parchment buffer, as it is also used as screen buffer
  if (scrbuf_size < 640*480)
    scrbuf_size = 640*480;
  i = LbDataFindStartIndex(vres256_load_files,(unsigned char **)&hires_parchment);
  if (i>=0)
  {
    vres256_load_files[i].SLength = scrbuf_size;
  }
  // Load the files
  if (LbDataLoadAll(vres256_load_files))
    return 0;
  return 1;
}

/*
 * Loads MCGA graphics files, for low resolution mode.
 * It is modified version of LbDataLoadAll, optimized for maximum
 * game speed on very slow machines.
 * @return Returns true if all files were loaded, false otherwise.
 */
short LoadMcgaData(void)
{
  //return _DK_LoadMcgaData();
  struct TbLoadFiles *load_files;
  void *mem;
  struct TbLoadFiles *t_lfile;
  int ferror;
  int ret_val;
  int i;
  load_files = mcga_load_files;
  LbDataFreeAll(load_files);
  ferror = 0;
  i = 0;
  t_lfile = &load_files[i];
  // Allocate some low memory, only to be sure that
  // it will be free when this function ends
  mem = LbMemoryAllocLow(0x10000u);
  while (t_lfile->Start != NULL)
  {
    // Don't allow loading flags
    t_lfile->Flags = 0;
    ret_val = LbDataLoad(t_lfile);
    if (ret_val == -100)
    {
      ERRORLOG("Can't allocate memory for MCGA files element \"%s\".", t_lfile->FName);
      ferror++;
    } else
    if ( ret_val == -101 )
    {
      ERRORLOG("Can't load MCGA file \"%s\".", t_lfile->FName);
      ferror++;
    }
    i++;
    t_lfile = &load_files[i];
  }
  if (mem != NULL)
    LbMemoryFree(mem);
  return (ferror == 0);
}

/*
 * Loads MCGA graphics files, for low resolution mode.
 * Loads only most importand files, where no GUI is needed.
 * @return Returns true if all files were loaded, false otherwise.
 */
short LoadMcgaDataMinimal(void)
{
  // Load the files
  if (LbDataLoadAll(mcga_load_files_minimal))
    return 0;
  return 1;
}

TbScreenMode get_next_vidmode(unsigned short mode)
{
	static const int maxModes = sizeof(switching_vidmodes) / sizeof(TbScreenMode);
	static int curr = 0; //TODO: remember this?

	// Do not allow to enter higher modes on low memory systems
	//TODO: remove this? should be useless on modern systems and only asks for screw up where user can't pick a mode because system detection failed
	if ((features_enabled & Ft_HiResVideo) == 0) {
		return failsafe_vidmode;
	}

	curr = (curr + 1) % maxModes;
	if (switching_vidmodes[curr].width < 1 ||
			switching_vidmodes[curr].height < 1 ||
			switching_vidmodes[curr].bpp < 1) {
		curr = 0;
	}

	return switching_vidmodes[curr];
}

void set_game_vidmode(unsigned short i, int w, int h, int bpp)
{
	switching_vidmodes[i].width = w;
	switching_vidmodes[i].height = h;
	switching_vidmodes[i].bpp = bpp;
}

bool validate_vidmode(TbScreenMode * mode)
{
	return TDDrawSdk::is_mode_possible(mode);
}

TbScreenMode get_failsafe_vidmode(void)
{
  return failsafe_vidmode;
}

void set_failsafe_vidmode(int w, int h, int bpp)
{
	failsafe_vidmode.width = w;
	failsafe_vidmode.height = h;
	failsafe_vidmode.bpp = bpp;
}

TbScreenMode get_movies_vidmode(void)
{
  return movies_vidmode;
}

void set_movies_vidmode(int w, int h, int bpp)
{
	movies_vidmode.width = w;
	movies_vidmode.height = h;
	movies_vidmode.bpp = bpp;
}

TbScreenMode get_frontend_vidmode(void)
{
  return frontend_vidmode;
}

void set_frontend_vidmode(int w, int h, int bpp)
{
	frontend_vidmode.width = w;
	frontend_vidmode.height = h;
	frontend_vidmode.bpp = bpp;
}

void load_pointer_file(short hi_res)
{
  struct TbLoadFiles *ldfiles;
  if ((features_enabled & Ft_BigPointer) == 0)
  {
    if (hi_res)
      ldfiles = hi_res_small_pointer_load_files;
    else
      ldfiles = low_res_small_pointer_load_files;
  } else
  {
    if (hi_res)
      ldfiles = hi_res_pointer_load_files;
    else
      ldfiles = low_res_pointer_load_files;
  }
  if ( LbDataLoadAll(ldfiles) )
    ERRORLOG("Unable to load pointer files");
  LbSpriteSetup(pointer_sprites, end_pointer_sprites, (unsigned long)pointer_data);
}

TbBool set_pointer_graphic_none(void)
{
  LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  return true;
}

TbBool set_pointer_graphic_menu(void)
{
  if (frontend_sprite == NULL)
  {
    WARNLOG("Frontend sprites not loaded, setting pointer to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  LbMouseChangeSpriteAndHotspot(&frontend_sprite[1], 0, 0);
  return true;
}

TbBool set_pointer_graphic_spell(long group_idx, long frame)
{
  long i,x,y;
  struct TbSprite *spr;
  SYNCDBG(8,"Setting to group %ld",group_idx);
  if (pointer_sprites == NULL)
  {
    WARNLOG("Pointer sprites not loaded, setting to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  if ((group_idx < 0) || (group_idx >= SPELL_POINTER_GROUPS))
  {
    WARNLOG("Group index out of range, setting pointer to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  if (is_feature_on(Ft_BigPointer))
  {
    y = 32;
    x = 32;
    i = 8*group_idx + (frame%8);
  } else
  {
    y = 78;
    x = 26;
    i = group_idx;
  }
  spr = &pointer_sprites[40+i];
  SYNCDBG(8,"Activating pointer %d",40+i);
  if ((spr >= pointer_sprites) && (spr < end_pointer_sprites))
  {
    LbMouseChangeSpriteAndHotspot(spr, x/2, y/2);
  } else
  {
    WARNLOG("Sprite %ld exceeds buffer, setting pointer to none",i);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  }
  return true;
}

TbBool set_pointer_graphic(long ptr_idx)
{
  long x,y;
  struct TbSprite *spr;
  SYNCDBG(8,"Setting to %ld",ptr_idx);
  if (pointer_sprites == NULL)
  {
    WARNLOG("Pointer sprites not loaded, setting to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  switch (ptr_idx)
  {
  case   0:
  case   1:
  case   2:
  case   4:
  case  15:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 15;
      break;
  case   3:
      spr = &pointer_sprites[ptr_idx];
      x = 17; y = 29;
      break;
  case   5:
  case   6:
  case   7:
  case   8:
  case   9:
  case  10:
  case  11:
  case  12:
  case  13:
  case  14:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
      break;
  case  16:
  case  17:
  case  18:
  case  19:
  case  20:
  case  21:
  case  22:
  case  23:
  case  24:
      spr = &pointer_sprites[ptr_idx];
      x = 20; y = 20;
      break;
  case  25:
  case  26:
  case  27:
  case  28:
  case  29:
  case  30:
  case  31:
  case  32:
  case  33:
  case  34:
  case  35:
  case  36:
  case  37:
  case  38:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
      break;
  case  39:
  // 40..144 are spell pointers
  case  47:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 15;
      break;
  case  96:
  case  97:
  case  98:
  case  99:
  case 100:
  case 101:
  case 102:
  case 103:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 15;
      break;
  default:
    WARNLOG("Unrecognized Mouse Pointer index, %d",ptr_idx);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  if ((spr >= pointer_sprites) && (spr < end_pointer_sprites))
  {
    LbMouseChangeSpriteAndHotspot(spr, x, y);
  } else
  {
    WARNLOG("Sprite %ld exceeds buffer, setting pointer to none",ptr_idx);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  }
  return true;
}

void unload_pointer_file(short hi_res)
{
  struct TbLoadFiles *ldfiles;
  set_pointer_graphic_none();
  if ((features_enabled & Ft_BigPointer) == 0)
  {
    if (hi_res)
      ldfiles = hi_res_small_pointer_load_files;
    else
      ldfiles = low_res_small_pointer_load_files;
  } else
  {
    if (hi_res)
      ldfiles = hi_res_pointer_load_files;
    else
      ldfiles = low_res_pointer_load_files;
  }
  LbDataFreeAll(ldfiles);
}

char *get_vidmode_name(unsigned short mode)
{
  return "this is a video mode"; //TODO: see if function is needed
}

short setup_screen_mode(TbScreenMode * mode, bool minimal) //minimal = true emulates _DK_setup_screen_mode_minimal
{
	unsigned int flg_mem;
	long lens_mem;
	short was_minimal_res;

	SYNCDBG(4,"Setting up mode %ix%ix%i", mode->width, mode->height, mode->bpp);

	// Reset old video mode.

	if (!force_video_mode_reset) {
		if ((mode->width == activeScreenMode.width &&
				mode->height == activeScreenMode.height &&
				mode->bpp == activeScreenMode.bpp) &&
				!MinimalResolutionSetup) {
			SYNCDBG(6,"Mode %ix%ix%i already active, no changes.", mode->width, mode->height, mode->bpp);
			return 1;
		}
	}

	flg_mem = lbDisplay.DrawFlags;

	if (!minimal) {
		lens_mem = game.numfield_1B;
		was_minimal_res = (MinimalResolutionSetup || force_video_mode_reset);
		set_pointer_graphic_none();
	}

	if (activeScreenMode.width <= 512) {
		if (MinimalResolutionSetup) {
			if (mode->width != activeScreenMode.width ||
					mode->height != activeScreenMode.height ||
					mode->bpp != activeScreenMode.bpp ||
					force_video_mode_reset) {
				LbScreenReset();
			}
			LbDataFreeAll(mcga_load_files_minimal);
			if (!minimal) ERRORLOG("MCGA Minimal not allowed (Reset)"); //wut?
			MinimalResolutionSetup = 0;
		}
		else {
			reset_eye_lenses();
			reset_heap_manager();
			reset_heap_memory();
			set_pointer_graphic_none();
			unload_pointer_file(0);
			if (mode->width != activeScreenMode.width ||
					mode->height != activeScreenMode.height ||
					mode->bpp != activeScreenMode.bpp ||
					force_video_mode_reset) {
				LbScreenReset();
			}
			LbDataFreeAll(mcga_load_files);
		}
	}
	else {
		if (MinimalResolutionSetup) {
			if (minimal) {
				set_pointer_graphic_none();
				unload_pointer_file(1);
			}
			if (mode->width != activeScreenMode.width ||
					mode->height != activeScreenMode.height ||
					mode->bpp != activeScreenMode.bpp ||
					force_video_mode_reset) {
				LbScreenReset();
			}
			LbDataFreeAll(vres256_load_files_minimal);
			MinimalResolutionSetup = 0;
		}
		else {
			reset_eye_lenses();
			reset_heap_manager();
			reset_heap_memory();
			if (!minimal) {
				set_pointer_graphic_none();
				unload_pointer_file(1);
			}
			if (mode->width != activeScreenMode.width ||
					mode->height != activeScreenMode.height ||
					mode->bpp != activeScreenMode.bpp ||
					force_video_mode_reset) {
				LbScreenReset();
			}
			LbDataFreeAll(vres256_load_files);
		}
	}

	// Set new video mode.

	if (mode->width <= 512) {
		SYNCDBG(6,"Entering low-res mode %ix%ix%i", mode->width, mode->height, mode->bpp);
		if (minimal) {
			MinimalResolutionSetup = 1;

			if (!LoadMcgaDataMinimal()) {
				ERRORLOG("Unable to load minimal MCGA files");
				return 0;
			}
		}
		else {
			if (!LoadMcgaData()) {
				ERRORLOG("Loading Mcga files failed");
				force_video_mode_reset = true;
				return 0;
			}
		}

		if (mode->width != activeScreenMode.width ||
				mode->height != activeScreenMode.height ||
				mode->bpp != activeScreenMode.bpp ||
				was_minimal_res) {
			if (LbScreenSetup(mode, _DK_palette, 2, 0) != 1) {
				ERRORLOG("Unable to setup screen resolution %ix%ix%i", mode->width, mode->height, mode->bpp);
				force_video_mode_reset = true;
				return 0;
			}
		}

		if (!minimal) {
			load_pointer_file(0);
		}
	}
	else {
		SYNCDBG(6,"Entering hi-res mode %ix%ix%i.", mode->width, mode->height, mode->bpp);
		if (minimal) {
			MinimalResolutionSetup = 1;
			frontend_load_data_from_cd();
			if (LbDataLoadAll(vres256_load_files_minimal)) {
			  ERRORLOG("Unable to load vres256_load_files_minimal files");
			  force_video_mode_reset = true;
			  return 0;
			}
			frontend_load_data_reset();
		}
		else {
			if (!LoadVRes256Data(mode->width * mode->height)) {
				ERRORLOG("Unable to load VRes256 data files");
				force_video_mode_reset = true;
				return 0;
			}
		}

		if (mode->width != activeScreenMode.width ||
				mode->height != activeScreenMode.height ||
				mode->bpp != activeScreenMode.bpp ||
				was_minimal_res) {
			if (LbScreenSetup(mode, _DK_palette, 1, 0) != 1) {
				ERRORLOG("Unable to setup screen resolution %ix%ix%i", mode->width, mode->height, mode->bpp);
				force_video_mode_reset = true;
				return 0;
			}
		}

		if (!minimal) {
			load_pointer_file(1);
		}
	}

	LbScreenClear(0);
	LbScreenSwap();
	update_screen_mode_data(mode->width, mode->height);

	if (!minimal) {
		if (parchment_loaded) {
			reload_parchment_file(mode->width >= 640);
		}

		reinitialise_eye_lens(lens_mem);
		LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1, (MyScreenHeight/pixel_size) >> 1);
		lbDisplay.DrawFlags = flg_mem;
		if (!setup_heap_memory()) {
			force_video_mode_reset = true;
			return 0;
		}
		setup_heap_manager();

		game.numfield_C &= 0xFFFB;
	}
	else {
		lbDisplay.DrawFlags = flg_mem;
	}

	force_video_mode_reset = false;
	SYNCDBG(8,"Finished");
	return 1;
}

TbBool update_screen_mode_data(long width, long height)
{
  if (width >= 640)
  {
    pixel_size = 1;
  } else
  {
    pixel_size = 2;
  }
  MyScreenWidth = width * pixel_size;
  MyScreenHeight = height * pixel_size;
  pixels_per_block = 16 * pixel_size;
  units_per_pixel = width/40;// originally was 16 for hires, 8 for lores
  if (MinimalResolutionSetup)
    LbSpriteSetupAll(setup_sprites_minimal);
  else
    LbSpriteSetupAll(setup_sprites);
  LbMouseSetup(NULL);
  LbMouseSetPointerHotspot(0, 0);
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  return true;
}

TbBool setup_screen_mode_zero(TbScreenMode * mode)
{
  SYNCDBG(4,"Setting up mode %ix%ix%i", mode->width, mode->height, mode->bpp);
  memset(_DK_palette, 0, PALETTE_SIZE);
  if (LbScreenSetup(mode, _DK_palette, 2, 0) != 1)
  {
      ERRORLOG("Unable to setup screen mode %ix%ix%i", mode->width, mode->height, mode->bpp);
      return false;
  }
  force_video_mode_reset = true;
  return true;
}

void reenter_video_mode(void)
{
	TbScreenMode mode = /*&settings.video_scrnmode*/ switching_vidmodes[0]; //TODO: load from some settings file, can't use original
	if (!validate_vidmode(&mode)) {
		ERRORLOG("Video mode %ix%ix%i not possible", mode.width, mode.height, mode.bpp);
	}

	if (setup_screen_mode(&mode, false)) {
      //settings.video_scrnmode = mode;
	}
	else {
		SYNCLOG("Can't enter %ix%ix%i, falling to failsafe mode", mode.width, mode.height, mode.bpp);
		mode = get_failsafe_vidmode();
		if ( !setup_screen_mode(&mode, false)) {
			_DK_FatalError = 1;
			exit_keeper = 1;
		}
		//settings.video_scrnmode = scrmode;
		save_settings();
  }
  SYNCLOG("Switched video to %ix%ix%i", mode.width, mode.height, mode.bpp);
}

TbScreenMode switch_to_next_video_mode(void)
{
  TbScreenMode mode;
  unsigned long prev_units_per_pixel_size;
  prev_units_per_pixel_size = units_per_pixel*pixel_size;

  mode = get_next_vidmode(lbDisplay.ScreenMode);
  if (setup_screen_mode(&mode, false)) {
      //settings.video_scrnmode = mode;
  }
  else {
	  SYNCLOG("Can't enter %ix%ix%i, falling to failsafe mode", mode.width, mode.height, mode.bpp);
      mode = get_failsafe_vidmode();
      if (!setup_screen_mode(&mode, false)) {
        _DK_FatalError = 1;
        exit_keeper = 1;
      }
      //settings.video_scrnmode = mode;
  }
  SYNCLOG("Switched video to %ix%ix%i", mode.width, mode.height, mode.bpp);
  LbScreenClear(0);
  LbScreenSwap();
  save_settings();
  if (game.numfield_C & 0x20)
    setup_engine_window(140, 0, MyScreenWidth, MyScreenHeight);
  else
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  keep_local_camera_zoom_level(prev_units_per_pixel_size);
//  reinit_all_menus();
  return mode;
}

TbScreenMode * getActiveScreenMode() {
	return &activeScreenMode;
}

#if (BFDEBUG_LEVEL > 0)
TbBool load_testfont_fonts(void)
{
  if ( LbDataLoadAll(testfont_load_files) )
  {
    ERRORLOG("Unable to load testfont_load_files files");
    return false;
  }
  LbSpriteSetupAll(setup_testfont);
  return true;
}

void free_testfont_fonts(void)
{
  LbDataFreeAll(testfont_load_files);
}
#endif

/******************************************************************************/
#ifdef __cplusplus
}
#endif
