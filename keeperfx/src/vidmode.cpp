/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode.c
 *     Video mode switching/setting function.
 * @par Purpose:
 *     Functions to change video mode in DK way.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
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

#include "front_simple.h"
#include "front_landview.h"
#include "frontend.h"
#include "game_heap.h"
#include "gui_draw.h"
#include "gui_parchment.h"
#include "engine_redraw.h"
#include "config.h"
#include "lens_api.h"
#include "config_settings.h"
#include "game_legacy.h"
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
  Lb_SCREEN_MODE_320_200_8,
  Lb_SCREEN_MODE_640_400_8,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  };

TbScreenMode failsafe_vidmode = Lb_SCREEN_MODE_320_200_8;
TbScreenMode movies_vidmode   = Lb_SCREEN_MODE_320_200_8;
TbScreenMode frontend_vidmode = Lb_SCREEN_MODE_640_480_8;

//struct IPOINT_2D units_per_pixel;
unsigned short units_per_pixel_min;
long base_mouse_sensitivity = 256;
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
/******************************************************************************/

/**
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

/**
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

/**
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
  int i;
  int maxmodes=sizeof(switching_vidmodes)/sizeof(TbScreenMode);
  // Do not allow to enter higher modes on low memory systems
  if ((features_enabled & Ft_HiResVideo) == 0)
    return failsafe_vidmode;
  for (i=0;i<maxmodes;i++)
  {
    if (switching_vidmodes[i]==mode) break;
  }
//  SYNCMSG("SEL IDX %d ALL %d SEL %d PREV %d",i,maxmodes,switching_vidmodes[i],mode);
  i++;
  if (i>=maxmodes)
  {
    i=0;
  } else
  if (switching_vidmodes[i] == Lb_SCREEN_MODE_INVALID)
  {
    i=0;
  }
  return switching_vidmodes[i];
}

void set_game_vidmode(unsigned short i,unsigned short nmode)
{
  switching_vidmodes[i]=(TbScreenMode)nmode;
}

TbScreenMode validate_vidmode(unsigned short mode)
{
  int i;
  int maxmodes=sizeof(switching_vidmodes)/sizeof(TbScreenMode);
  // Do not allow to enter higher modes on low memory systems
  if ((features_enabled & Ft_HiResVideo) == 0)
    return failsafe_vidmode;
  for (i=0;i<maxmodes;i++)
  {
    if (switching_vidmodes[i] == mode) return switching_vidmodes[i];
  }
  return failsafe_vidmode;
}

TbScreenMode get_failsafe_vidmode(void)
{
  return failsafe_vidmode;
}

void set_failsafe_vidmode(unsigned short nmode)
{
  failsafe_vidmode=(TbScreenMode)nmode;
}

TbScreenMode get_movies_vidmode(void)
{
  return movies_vidmode;
}

void set_movies_vidmode(unsigned short nmode)
{
  movies_vidmode=(TbScreenMode)nmode;
}

TbScreenMode get_frontend_vidmode(void)
{
  return frontend_vidmode;
}

void set_frontend_vidmode(unsigned short nmode)
{
  frontend_vidmode=(TbScreenMode)nmode;
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
  TbScreenModeInfo *mdinfo;
  mdinfo = LbScreenGetModeInfo(mode);
  return   mdinfo->Desc;
}

TbBool setup_screen_mode(unsigned short nmode)
{
    TbScreenModeInfo *mdinfo;
    //return _DK_setup_screen_mode(nmode);
    unsigned int flg_mem;
    long lens_mem;
    short was_minimal_res;
    SYNCDBG(4,"Setting up mode %d",(int)nmode);
    if (!force_video_mode_reset)
    {
      if ((nmode == lbDisplay.ScreenMode) && (!MinimalResolutionSetup))
      {
        SYNCDBG(6,"Mode %d already active, no changes.",(int)nmode);
        return true;
      }
    }
    lens_mem = game.numfield_1B;
    flg_mem = lbDisplay.DrawFlags;
    was_minimal_res = (MinimalResolutionSetup || force_video_mode_reset);
    set_pointer_graphic_none();
    if (LbGraphicsScreenHeight() < 200)
    {
        WARNLOG("Unhandled previous Screen Mode %d, Reset skipped",(int)lbDisplay.ScreenMode);
    } else
    if (LbGraphicsScreenHeight() < 400)
    {
        if (MinimalResolutionSetup)
        {
          if (lbDisplay.ScreenMode != nmode)
            LbScreenReset();
          LbDataFreeAll(mcga_load_files_minimal);
          ERRORLOG("MCGA Minimal not allowed (Reset)");
          MinimalResolutionSetup = 0;
        } else
        {
          reset_eye_lenses();
          reset_heap_manager();
          reset_heap_memory();
          set_pointer_graphic_none();
          unload_pointer_file(0);
          if (lbDisplay.ScreenMode != nmode)
            LbScreenReset();
          LbDataFreeAll(mcga_load_files);
        }
    } else
    // so (LbGraphicsScreenHeight() >= 400)
    {
        if (MinimalResolutionSetup)
        {
          if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
            LbScreenReset();
          LbDataFreeAll(vres256_load_files_minimal);
          MinimalResolutionSetup = 0;
        } else
        {
          reset_eye_lenses();
          reset_heap_manager();
          reset_heap_memory();
          set_pointer_graphic_none();
          unload_pointer_file(1);
          if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
            LbScreenReset();
          LbDataFreeAll(vres256_load_files);
        }
    }

    mdinfo = LbScreenGetModeInfo(nmode);
    if (mdinfo->Height < 200)
    {
        ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
        force_video_mode_reset = true;
        return false;
    } else
    if (mdinfo->Height < 400)
    {
        SYNCDBG(6,"Entering low-res mode %d, resolution %ldx%ld.",(int)nmode,mdinfo->Width,mdinfo->Height);
        if (!LoadMcgaData())
        {
          ERRORLOG("Loading Mcga files failed");
          force_video_mode_reset = true;
          return false;
        }
        if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
        {
            if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 2, 0) != 1)
            {
              ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                  mdinfo->Desc,(int)nmode);
              force_video_mode_reset = true;
              return false;
            }
        }
        load_pointer_file(0);
    } else
    // so (mdinfo->Height >= 400)
    {
        SYNCDBG(6,"Entering hi-res mode %d, resolution %ldx%ld.",(int)nmode,mdinfo->Width,mdinfo->Height);
        if (!LoadVRes256Data((long)mdinfo->Width*(long)mdinfo->Height))
        {
          ERRORLOG("Unable to load VRes256 data files");
          force_video_mode_reset = true;
          return false;
        }
        if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
        {
            if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 1, 0) != 1)
            {
              ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                  mdinfo->Desc,(int)nmode);
              force_video_mode_reset = true;
              return false;
            }
        }
        load_pointer_file(1);
    }
    LbScreenClear(0);
    LbScreenSwap();
    update_screen_mode_data(mdinfo->Width, mdinfo->Height);
    if (parchment_loaded)
      reload_parchment_file(mdinfo->Width >= 640);
    reinitialise_eye_lens(lens_mem);
    LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1, (MyScreenHeight/pixel_size) >> 1);
    lbDisplay.DrawFlags = flg_mem;
    if (!setup_heap_memory())
    {
      force_video_mode_reset = true;
      return false;
    }
    setup_heap_manager();
    game.numfield_C &= ~0x0004;
    force_video_mode_reset = false;
    SYNCDBG(8,"Finished");
    return true;
}

TbBool update_screen_mode_data(long width, long height)
{
  if ((width >= 640) && (height >= 400))
  {
    pixel_size = 1;
  } else
  {
    pixel_size = 2;
  }
  MyScreenWidth = width * (long)pixel_size;
  MyScreenHeight = height * (long)pixel_size;
  pixels_per_block = 16 * (long)pixel_size;
  units_per_pixel = (width>height?width:height)/40;// originally was 16 for hires, 8 for lores
  units_per_pixel_min = (width>height?height:width)/40;// originally 10 for hires
  if (MinimalResolutionSetup)
    LbSpriteSetupAll(setup_sprites_minimal);
  else
    LbSpriteSetupAll(setup_sprites);
  LbMouseSetup(NULL);
  LbMouseChangeMoveRatio(base_mouse_sensitivity*units_per_pixel/16, base_mouse_sensitivity*units_per_pixel/16);
  LbMouseSetPointerHotspot(0, 0);
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  return true;
}

short setup_screen_mode_minimal(unsigned short nmode)
{
  //return _DK_setup_screen_mode_minimal(nmode);
  unsigned int flg_mem;
  TbScreenModeInfo *mdinfo;
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  if (!force_video_mode_reset)
  {
    if ((nmode == lbDisplay.ScreenMode) && (MinimalResolutionSetup))
      return 1;
  }
  flg_mem = lbDisplay.DrawFlags;
  if (LbGraphicsScreenHeight() < 200)
  {
      WARNLOG("Unhandled previous Screen Mode %d, Reset skipped",(int)lbDisplay.ScreenMode);
  } else
  if (LbGraphicsScreenHeight() < 400)
  {
      if (MinimalResolutionSetup)
      {
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(mcga_load_files_minimal);
        MinimalResolutionSetup = 0;
      } else
      {
        reset_eye_lenses();
        reset_heap_manager();
        reset_heap_memory();
        set_pointer_graphic_none();
        unload_pointer_file(0);
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(mcga_load_files);
      }
  } else
  {
      if (MinimalResolutionSetup)
      {
        set_pointer_graphic_none();
        unload_pointer_file(1);
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(vres256_load_files_minimal);
        MinimalResolutionSetup = 0;
      } else
      {
        reset_eye_lenses();
        reset_heap_manager();
        reset_heap_memory();
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(vres256_load_files);
      }
  }
  mdinfo = LbScreenGetModeInfo(nmode);
  if (mdinfo->Height < 200)
  {
      ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
      force_video_mode_reset = true;
      return 0;
  } else
  if (mdinfo->Height < 400)
  {
      SYNCDBG(17,"Preparing minimal low resolution mode");
      MinimalResolutionSetup = 1;
      if ( !LoadMcgaDataMinimal() )
      {
        ERRORLOG("Unable to load minimal MCGA files");
        return 0;
      }
      if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
      {
          if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 2, 0) != 1)
          {
            ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                mdinfo->Desc,(int)nmode);
            force_video_mode_reset = true;
            return 0;
          }
      }
  } else
  {
      SYNCDBG(17,"Preparing minimal high resolution mode");
      MinimalResolutionSetup = 1;
      frontend_load_data_from_cd();
      if ( LbDataLoadAll(vres256_load_files_minimal) )
      {
        ERRORLOG("Unable to load vres256_load_files_minimal files");
        force_video_mode_reset = true;
        return 0;
      }
      frontend_load_data_reset();
      if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
      {
          if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 1, 0) != 1)
          {
             ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                 mdinfo->Desc,(int)nmode);
             force_video_mode_reset = true;
             return 0;
          }
      }
  }
  LbScreenClear(0);
  LbScreenSwap();
  update_screen_mode_data(mdinfo->Width, mdinfo->Height);
  lbDisplay.DrawFlags = flg_mem;
  force_video_mode_reset = false;
  return 1;
}

TbBool setup_screen_mode_zero(unsigned short nmode)
{
  TbScreenModeInfo *mdinfo;
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  mdinfo = LbScreenGetModeInfo(nmode);
  LbPaletteDataFillBlack(_DK_palette);
  if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 2, 0) != 1)
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)",
          mdinfo->Desc,(int)nmode);
      return false;
  }
  force_video_mode_reset = true;
  return true;
}

TbScreenMode reenter_video_mode(void)
{
 TbScreenMode scrmode;
 scrmode=validate_vidmode(settings.video_scrnmode);
 if ( setup_screen_mode(scrmode) )
  {
      settings.video_scrnmode = scrmode;
  } else
  {
      SYNCLOG("Can't enter %s (mode %d), falling to failsafe mode",
          get_vidmode_name(scrmode),(int)scrmode);
      scrmode=get_failsafe_vidmode();
      if ( !setup_screen_mode(scrmode) )
      {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return Lb_SCREEN_MODE_INVALID;
      }
      settings.video_scrnmode = scrmode;
      save_settings();
  }
  SYNCLOG("Switched video to %s (mode %d)", get_vidmode_name(scrmode),(int)scrmode);
  return scrmode;
}

TbScreenMode switch_to_next_video_mode(void)
{
    TbScreenMode scrmode;
    scrmode = get_next_vidmode(lbDisplay.ScreenMode);
    if ( setup_screen_mode(scrmode) )
    {
        settings.video_scrnmode = scrmode;
    } else
    {
        SYNCLOG("Can't enter %s (mode %d), falling to failsafe mode",
            get_vidmode_name(scrmode),(int)scrmode);
        scrmode = get_failsafe_vidmode();
        if ( !setup_screen_mode(scrmode) )
        {
          FatalError = 1;
          exit_keeper = 1;
          return Lb_SCREEN_MODE_INVALID;
        }
        settings.video_scrnmode = scrmode;
    }
    SYNCLOG("Switched video to %s (mode %d)", get_vidmode_name(scrmode),(int)scrmode);
    save_settings();
    if (game.numfield_C & 0x20)
      setup_engine_window(status_panel_width, 0, MyScreenWidth, MyScreenHeight);
    else
      setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
//    reinit_all_menus();
    return scrmode;
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
