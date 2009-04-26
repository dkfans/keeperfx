/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode.c
 *     Video mode switching/setting function.
 * @par Purpose:
 *     Functions to change video mode in DK way.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
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
#include "frontend.h"
#include "config.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct TbLoadFiles _DK_hi_res_small_pointer_load_files[3];
#define hi_res_small_pointer_load_files _DK_hi_res_small_pointer_load_files
DLLIMPORT struct TbLoadFiles _DK_low_res_small_pointer_load_files[3];
#define low_res_small_pointer_load_files _DK_low_res_small_pointer_load_files
DLLIMPORT struct TbLoadFiles _DK_hi_res_pointer_load_files[3];
#define hi_res_pointer_load_files _DK_hi_res_pointer_load_files
DLLIMPORT struct TbLoadFiles _DK_low_res_pointer_load_files[3];
#define low_res_pointer_load_files _DK_low_res_pointer_load_files
//DLLIMPORT struct TbLoadFiles _DK_mcga_load_files[8];
//#define mcga_load_files _DK_mcga_load_files
//DLLIMPORT struct TbLoadFiles _DK_mcga_load_files_minimal[1];
//#define mcga_load_files_minimal _DK_mcga_load_files_minimal
//DLLIMPORT struct TbLoadFiles _DK_vres256_load_files[9];
//#define vres256_load_files _DK_vres256_load_files
//DLLIMPORT struct TbLoadFiles _DK_vres256_load_files_minimal[11];
//#define vres256_load_files_minimal _DK_vres256_load_files_minimal
//DLLIMPORT struct TbSetupSprite _DK_setup_sprites_minimal[5];
//#define setup_sprites_minimal _DK_setup_sprites_minimal
//DLLIMPORT struct TbSetupSprite _DK_setup_sprites[8];
//#define setup_sprites _DK_setup_sprites

DLLIMPORT __cdecl int _DK_setup_screen_mode(short nmode);
DLLIMPORT __cdecl int _DK_setup_screen_mode_minimal(short nmode);
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
/******************************************************************************/
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

struct TbLoadFiles mcga_load_files[] = {
  {"data\\gui.dat",       (unsigned char **)&button_sprite_data,    (unsigned char **)&end_button_sprite_data,      0, 0, 0},
  {"data\\gui2-0-0.dat",  (unsigned char **)&gui_panel_sprite_data, (unsigned char **)&end_gui_panel_sprite_data,   0, 0, 0},
  {"data\\gui.tab",       (unsigned char **)&button_sprite,         (unsigned char **)&end_button_sprites,          0, 0, 0},
  {"data\\font2-0.dat",   (unsigned char **)&winfont_data,          (unsigned char **)&end_winfont_data,            0, 0, 0},
  {"data\\font2-0.tab",   (unsigned char **)&winfont,               (unsigned char **)&end_winfonts,                0, 0, 0},
  {"data\\lofont.dat",    (unsigned char **)&font_data,             NULL,                                           0, 0, 0},
  {"data\\lofont.tab",    (unsigned char **)&font_sprites,          (unsigned char **)&end_font_sprites,            0, 0, 0},
  {"data\\slab0-0.dat",   (unsigned char **)&gui_slab,              NULL,                                           0, 0, 0},
  {"data\\gui2-0-0.tab",  (unsigned char **)&gui_panel_sprites,     (unsigned char **)&end_gui_panel_sprites,       0, 0, 0},
  {"",                    NULL,                                     NULL,                                           0, 0, 0},
};

struct TbLoadFiles vres256_load_files[] = {
  {"data\\guihi.dat",     (unsigned char **)&button_sprite_data,    (unsigned char **)&end_button_sprite_data,      0, 0, 0},
  {"data\\guihi.tab",     (unsigned char **)&button_sprite,         (unsigned char **)&end_button_sprites,          0, 0, 0},
  {"data\\font2-1.dat",   (unsigned char **)&winfont_data,          (unsigned char **)&end_winfont_data,            0, 0, 0},
  {"data\\font2-1.tab",   (unsigned char **)&winfont,               (unsigned char **)&end_winfonts,                0, 0, 0},
  {"data\\hifont.dat",    (unsigned char **)&font_data,             NULL,                                           0, 0, 0},
  {"data\\hifont.tab",    (unsigned char **)&font_sprites,          (unsigned char **)&end_font_sprites,            0, 0, 0},
  {"data\\slab0-1.dat",   (unsigned char **)&gui_slab,              NULL,                                           0, 0, 0},
  {"data\\gui2-0-1.dat",  (unsigned char **)&gui_panel_sprite_data, (unsigned char **)&end_gui_panel_sprite_data,   0, 0, 0},
  {"data\\gui2-0-1.tab",  (unsigned char **)&gui_panel_sprites,     (unsigned char **)&end_gui_panel_sprites,       0, 0, 0},
  {"*B_SCREEN",           (unsigned char **)&hires_parchment,       NULL,                                     640*480, 0, 0},
  {"",                    NULL,                                     NULL,                                           0, 0, 0},
};

struct TbLoadFiles mcga_load_files_minimal[] = {
  {"",                    NULL,                                     NULL,                                          0, 0, 0},
};

struct TbLoadFiles vres256_load_files_minimal[] = {
  {"ldata\\frontft1.dat", (unsigned char **)&frontend_font_data[0], (unsigned char **)&frontend_end_font_data[0],  0, 0, 0},
  {"ldata\\frontft1.tab", (unsigned char **)&frontend_font[0],      (unsigned char **)&frontend_end_font[0],       0, 0, 0},
  {"ldata\\frontft2.dat", (unsigned char **)&frontend_font_data[1], (unsigned char **)&frontend_end_font_data[1],  0, 0, 0},
  {"ldata\\frontft2.tab", (unsigned char **)&frontend_font[1],      (unsigned char **)&frontend_end_font[1],       0, 0, 0},
  {"ldata\\frontft3.dat", (unsigned char **)&frontend_font_data[2], (unsigned char **)&frontend_end_font_data[2],  0, 0, 0},
  {"ldata\\frontft3.tab", (unsigned char **)&frontend_font[2],      (unsigned char **)&frontend_end_font[2],       0, 0, 0},
  {"ldata\\frontft4.dat", (unsigned char **)&frontend_font_data[3], (unsigned char **)&frontend_end_font_data[3],  0, 0, 0},
  {"ldata\\frontft4.tab", (unsigned char **)&frontend_font[3],      (unsigned char **)&frontend_end_font[3],       0, 0, 0},
  {"levels\\levels.txt",  (unsigned char **)&level_names_data,      (unsigned char **)&end_level_names_data,       0, 0, 0},
  {"*FE_BACKUP_PAL",      (unsigned char **)&frontend_backup_palette,NULL,                                       768, 0, 0},
  {"",                    NULL,                                     NULL,                                          0, 0, 0},
};

short force_video_mode_reset = true;
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
      LbErrorLog("Can't allocate memory for MCGA files element \"%s\".\n", t_lfile->FName);
      ferror++;
    } else
    if ( ret_val == -101 )
    {
      LbErrorLog("Can't load MCGA file \"%s\".\n", t_lfile->FName);
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
  int i;
  int maxmodes=sizeof(switching_vidmodes)/sizeof(TbScreenMode);
  // Do not allow to enter higher modes on low memory systems
  if ((features_enabled & Ft_HiResVideo) == 0)
    return failsafe_vidmode;
  for (i=0;i<maxmodes;i++)
  {
    if (switching_vidmodes[i]==mode) break;
  }
//  LbSyncLog("SEL IDX %d ALL %d SEL %d PREV %d\n",i,maxmodes,switching_vidmodes[i],mode);
  i++;
  if (i>=maxmodes)
  {
    i=0;
  } else
  if (switching_vidmodes[i]==Lb_SCREEN_MODE_INVALID)
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
    if (switching_vidmodes[i]==mode) return switching_vidmodes[i];
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
  static const char *func_name="load_pointer_file";
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
    error(func_name, 1105, "Unable to load pointer files");
  LbSpriteSetup(pointer_sprites, end_pointer_sprites, (unsigned long)pointer_data);
}

void unload_pointer_file(short hi_res)
{
  struct TbLoadFiles *ldfiles;
  LbMouseChangeSpriteAndHotspot(0, 0, 0);
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
  struct TbScreenModeInfo *mdinfo;
  mdinfo = LbScreenGetModeInfo(mode);
  return   mdinfo->Desc;
}

short setup_screen_mode(unsigned short nmode)
{
  static const char *func_name="setup_screen_mode";
  char *fname;
  struct TbScreenModeInfo *mdinfo;
  //return _DK_setup_screen_mode(nmode);
  unsigned int flg_mem;
  long lens_mem;
  short was_minimal_res;
  if (!force_video_mode_reset)
  {
    if ((nmode == lbDisplay.ScreenMode) && (!MinimalResolutionSetup))
    {
  #if (BFDEBUG_LEVEL > 6)
      LbSyncLog("%s: Mode %d already active, no changes.\n",func_name,(int)nmode);
  #endif
      return 1;
    }
  }
  lens_mem = game.numfield_1B;
  flg_mem = lbDisplay.DrawFlags;
  was_minimal_res = (MinimalResolutionSetup || force_video_mode_reset);
  LbMouseChangeSpriteAndHotspot(0, 0, 0);
  switch (lbDisplay.ScreenMode)
  {
  case Lb_SCREEN_MODE_320_200_8:
  case Lb_SCREEN_MODE_320_240_8:
  case Lb_SCREEN_MODE_512_384_8:
    if (MinimalResolutionSetup)
    {
      if (lbDisplay.ScreenMode != nmode)
        LbScreenReset();
      LbDataFreeAll(mcga_load_files_minimal);
      error(func_name, 625, "MCGA Minimal not allowed (Reset)");
      MinimalResolutionSetup = 0;
    } else
    {
      reset_eye_lenses();
      reset_heap_manager();
      reset_heap_memory();
      LbMouseChangeSpriteAndHotspot(0, 0, 0);
      unload_pointer_file(0);
      if (lbDisplay.ScreenMode != nmode)
        LbScreenReset();
      LbDataFreeAll(mcga_load_files);
    }
    break;
  case Lb_SCREEN_MODE_640_400_8:
  case Lb_SCREEN_MODE_640_480_8:
  case Lb_SCREEN_MODE_800_600_8:
  case Lb_SCREEN_MODE_1024_768_8:
  case Lb_SCREEN_MODE_1200_1024_8:
  case Lb_SCREEN_MODE_1600_1200_8:
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
      LbMouseChangeSpriteAndHotspot(0, 0, 0);
      unload_pointer_file(1);
      if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
        LbScreenReset();
      LbDataFreeAll(vres256_load_files);
    }
    break;
  default:
    error(func_name, 677, "Unhandled previous Screen Mode (Reset)");
    return 0;
  }
  
  mdinfo = LbScreenGetModeInfo(nmode);
  switch (nmode)
  {
  case Lb_SCREEN_MODE_320_200_8:
  case Lb_SCREEN_MODE_320_240_8:
  case Lb_SCREEN_MODE_512_384_8:
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Entering low-res mode %d, resolution %ldx%ld.\n",func_name,(int)nmode,mdinfo->Width,mdinfo->Height);
#endif
    if (!LoadMcgaData())
    {
      error(func_name, 689, "Loading Mcga files failed");
      force_video_mode_reset = true;
      return 0;
    }
    if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
    {
      if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 2, 0) != 1)
      {
        error(func_name, 904, "Unable to setup screen resolution");
        force_video_mode_reset = true;
        return 0;
      }
    }
    load_pointer_file(0);
    break;
  case Lb_SCREEN_MODE_640_400_8:
  case Lb_SCREEN_MODE_640_480_8:
  case Lb_SCREEN_MODE_800_600_8:
  case Lb_SCREEN_MODE_1024_768_8:
  case Lb_SCREEN_MODE_1200_1024_8:
  case Lb_SCREEN_MODE_1600_1200_8:
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Entering hi-res mode %d, resolution %ldx%ld.\n",func_name,(int)nmode,mdinfo->Width,mdinfo->Height);
#endif
    if (!LoadVRes256Data(mdinfo->Width*mdinfo->Height))
    {
      error(func_name, 727, "Unable to load vres256_load_files");
      force_video_mode_reset = true;
      return 0;
    }
    if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
    {
      if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 1, 0) != 1)
      {
        error(func_name, 904, "Unable to setup screen resolution");
        force_video_mode_reset = true;
        return 0;
      }
    }
    load_pointer_file(1);
    break;
  default:
    error(func_name, 779, "Unhandled Screen Mode (Setup)");
    force_video_mode_reset = true;
    return 0;
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
    return 0;
  }
  setup_heap_manager();
  game.numfield_C &= 0xFFFB;
  force_video_mode_reset = false;
  return 1;
}

short update_screen_mode_data(long width, long height)
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
  LbMouseSetup(0);
  LbMouseSetPointerHotspot(0, 0);
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  return true;
}

short setup_screen_mode_minimal(unsigned short nmode)
{
  static const char *func_name="setup_screen_mode_minimal";
  //return _DK_setup_screen_mode_minimal(nmode);
  unsigned int flg_mem;
  struct TbScreenModeInfo *mdinfo;

  if (!force_video_mode_reset)
  {
    if ((nmode == lbDisplay.ScreenMode) && (MinimalResolutionSetup))
      return 1;
  }
  flg_mem = lbDisplay.DrawFlags;
  switch (lbDisplay.ScreenMode)
  {
  case Lb_SCREEN_MODE_320_200_8:
  case Lb_SCREEN_MODE_320_240_8:
  case Lb_SCREEN_MODE_512_384_8:
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
      LbMouseChangeSpriteAndHotspot(0, 0, 0);
      unload_pointer_file(0);
      if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
        LbScreenReset();
      LbDataFreeAll(mcga_load_files);
    }
    break;
  case Lb_SCREEN_MODE_640_400_8:
  case Lb_SCREEN_MODE_640_480_8:
  case Lb_SCREEN_MODE_800_600_8:
  case Lb_SCREEN_MODE_1024_768_8:
  case Lb_SCREEN_MODE_1200_1024_8:
  case Lb_SCREEN_MODE_1600_1200_8:
    if (MinimalResolutionSetup)
    {
      LbMouseChangeSpriteAndHotspot(0, 0, 0);
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
    break;
  default:
    error(func_name, 884, "Unhandled previous Screen Mode (Reset)");
    break;
  }
  mdinfo = LbScreenGetModeInfo(nmode);
  switch (nmode)
  {
  case Lb_SCREEN_MODE_320_200_8:
  case Lb_SCREEN_MODE_320_240_8:
  case Lb_SCREEN_MODE_512_384_8:
    MinimalResolutionSetup = 1;
    if ( !LoadMcgaDataMinimal() )
    {
      error(func_name, 895, "Unable to load minimal MCGA files");
      return 0;
    }
    if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
    {
      if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 2, 0) != 1)
      {
        error(func_name, 904, "Unable to setup screen resolution");
        force_video_mode_reset = true;
        return 0;
      }
    }
    break;
  case Lb_SCREEN_MODE_640_400_8:
  case Lb_SCREEN_MODE_640_480_8:
  case Lb_SCREEN_MODE_800_600_8:
  case Lb_SCREEN_MODE_1024_768_8:
  case Lb_SCREEN_MODE_1200_1024_8:
  case Lb_SCREEN_MODE_1600_1200_8:
    MinimalResolutionSetup = 1;
    frontend_load_data_from_cd();
    if ( LbDataLoadAll(vres256_load_files_minimal) )
    {
      error(func_name, 924, "Unable to load vres256_load_files_minimal");
      force_video_mode_reset = true;
      return 0;
    }
    frontend_load_data_reset();
    if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
    {
     if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 1, 0) != 1)
     {
        error(func_name, 904, "Unable to setup screen resolution");
        force_video_mode_reset = true;
        return 0;
     }
    }
    break;
  default:
    error(func_name, 948, "Unhandled Screen Mode (Setup)");
    force_video_mode_reset = true;
    return 0;
  }
  LbScreenClear(0);
  LbScreenSwap();
  update_screen_mode_data(mdinfo->Width, mdinfo->Height);
  lbDisplay.DrawFlags = flg_mem;
  force_video_mode_reset = false;
  return 1;
}

short setup_screen_mode_zero(unsigned short nmode)
{
  static const char *func_name="setup_screen_mode_zero";
  struct TbScreenModeInfo *mdinfo;
  mdinfo = LbScreenGetModeInfo(nmode);
  memset(_DK_palette, 0, PALETTE_SIZE);
  if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 2, 0) != 1)
  {
        error(func_name, 904, "Unable to setup screen resolution");
        return 0;
  }
  force_video_mode_reset = true;
  return 1;
}

TbScreenMode reenter_video_mode(void)
{
  static const char *func_name="reenter_video_mode";
 TbScreenMode scrmode;
 scrmode=validate_vidmode(_DK_settings.field_B);
 if ( setup_screen_mode(scrmode) )
  {
      _DK_settings.field_B = scrmode;
  } else
  {
      LbSyncLog("%s: Can't enter %s (mode %d), falling to failsafe mode\n",
          func_name,get_vidmode_name(scrmode),(int)scrmode);
      scrmode=get_failsafe_vidmode();
      if ( !setup_screen_mode(scrmode) )
      {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return Lb_SCREEN_MODE_INVALID;
      }
      _DK_settings.field_B = scrmode;
      save_settings();
  }
  LbSyncLog("%s: Switched to video mode %d\n",func_name,(int)scrmode);
  return scrmode;
}

TbScreenMode switch_to_next_video_mode(void)
{
  static const char *func_name="switch_to_next_video_mode";
  TbScreenMode scrmode;
  unsigned long prev_units_per_pixel_size;
  prev_units_per_pixel_size = units_per_pixel*pixel_size;
  scrmode = get_next_vidmode(lbDisplay.ScreenMode);
  if ( setup_screen_mode(scrmode) )
  {
      _DK_settings.field_B = scrmode;
  } else
  {
      LbSyncLog("%s: Can't enter %s (mode %d), falling to failsafe mode\n",
          func_name,get_vidmode_name(scrmode),(int)scrmode);
      scrmode = get_failsafe_vidmode();
      if ( !setup_screen_mode(scrmode) )
      {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return Lb_SCREEN_MODE_INVALID;
      }
      _DK_settings.field_B = scrmode;
  }
  LbSyncLog("%s: Switched to video mode %d\n",func_name,(int)scrmode);
  int x1,y1,x2,y2;
  LbScreenClear(0);
  LbScreenSwap();
  save_settings();
  if (game.numfield_C & 0x20)
    setup_engine_window(140, 0, MyScreenWidth, MyScreenHeight);
  else
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  keep_local_camera_zoom_level(prev_units_per_pixel_size);
//  reinit_all_menus();
  return scrmode;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
