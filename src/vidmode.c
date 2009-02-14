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
DLLIMPORT struct TbLoadFiles _DK_mcga_load_files[8];
#define mcga_load_files _DK_mcga_load_files
DLLIMPORT struct TbLoadFiles _DK_mcga_load_files_minimal[1];
#define mcga_load_files_minimal _DK_mcga_load_files_minimal
DLLIMPORT struct TbLoadFiles _DK_vres256_load_files[9];
#define vres256_load_files _DK_vres256_load_files
DLLIMPORT struct TbLoadFiles _DK_vres256_load_files_minimal[11];
#define vres256_load_files_minimal _DK_vres256_load_files_minimal
DLLIMPORT struct TbSetupSprite _DK_setup_sprites_minimal[5];
#define setup_sprites_minimal _DK_setup_sprites_minimal
DLLIMPORT struct TbSetupSprite _DK_setup_sprites[8];
#define setup_sprites _DK_setup_sprites
DLLIMPORT int _DK_MinimalResolutionSetup;
#define MinimalResolutionSetup _DK_MinimalResolutionSetup
DLLIMPORT struct TbSprite *_DK_pointer_sprites;
#define pointer_sprites _DK_pointer_sprites
DLLIMPORT struct TbSprite *_DK_end_pointer_sprites;
#define end_pointer_sprites _DK_end_pointer_sprites
DLLIMPORT unsigned long _DK_pointer_data;
#define pointer_data _DK_pointer_data

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

TbScreenMode get_next_vidmode(unsigned short mode)
{
  int i;
  int maxmodes=sizeof(switching_vidmodes)/sizeof(TbScreenMode);
//HACK to make the mode switching work - 13 really enters 10
//  if (mode==10) mode=13; (disabled - not needed anymore)
  // Do not allow to enter higher modes on low memory systems
  if (mem_size < 16)
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
  if (mem_size < 16)
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
  if (mem_size < 16)
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
  LbSpriteSetup(pointer_sprites, end_pointer_sprites, (const char *)pointer_data);
}

void unload_pointer_file(short hi_res)
{
  struct TbLoadFiles *ldfiles;
  LbMouseChangeSpriteAndHotspot(0, 0, 0);
  if (mem_size < 16)
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
  if ((nmode == lbDisplay.ScreenMode) && (!MinimalResolutionSetup))
    return 1;
  lens_mem = game.numfield_1B;
  flg_mem = lbDisplay.DrawFlags;
  was_minimal_res=MinimalResolutionSetup;
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
    if (!LoadMcgaData())
    {
      error(func_name, 689, "Load Mcga failed");
      return 0;
    }
    if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
    {
      if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 2, 0) != 1)
      {
        error(func_name, 904, "Unable to setup screen resolution");
        return 0;
      }
    }
    load_pointer_file(0);
    pixel_size = 2;
    MyScreenWidth = mdinfo->Width * pixel_size;
    MyScreenHeight = mdinfo->Height * pixel_size;
    pixels_per_block = 32;
    units_per_pixel = 8;
    if (parchment_loaded)
    {
      fname=prepare_file_path(FGrp_StdData,"gmap.raw");
      LbFileLoadAt(fname, poly_pool);
    }
    break;
  case Lb_SCREEN_MODE_640_400_8:
  case Lb_SCREEN_MODE_640_480_8:
  case Lb_SCREEN_MODE_800_600_8:
  case Lb_SCREEN_MODE_1024_768_8:
  case Lb_SCREEN_MODE_1200_1024_8:
  case Lb_SCREEN_MODE_1600_1200_8:
    if (LbDataLoadAll(vres256_load_files))
    {
      error(func_name, 727, "Unable to load vres256_load_files");
      return 0;
    }
    if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
    {
/*
    if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
    {
      if (LbScreenIsModeAvailable(Lb_SCREEN_MODE_640_400_8))
      {
        if (LbScreenSetup(Lb_SCREEN_MODE_640_400_8, mdinfo->Width, 400, _DK_palette, 1, 0) != 1)
          return 0;
      } else
*/
      if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 1, 0) != 1)
      {
        error(func_name, 904, "Unable to setup screen resolution");
        return 0;
      }
    }
    load_pointer_file(1);
    pixel_size = 1;
    MyScreenWidth = mdinfo->Width * pixel_size;
    MyScreenHeight = mdinfo->Height * pixel_size;
    pixels_per_block = 16;
    units_per_pixel = mdinfo->Width/40;// originally was 16
    if (parchment_loaded)
    {
      fname=prepare_file_path(FGrp_StdData,"gmaphi.raw");
      LbFileLoadAt(fname, hires_parchment);
    }
    break;
  default:
    error(func_name, 779, "Unhandled Screen Mode (Setup)");
    return 0;
  }
  LbScreenClear(0);
  LbScreenSwap();
  reinitialise_eye_lens(lens_mem);
  LbSpriteSetupAll(setup_sprites);
  LbMouseSetup(0);
  LbMouseSetPointerHotspot(0, 0);
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1, (MyScreenHeight/pixel_size) >> 1);
  lbDisplay.DrawFlags = flg_mem;
  if (setup_heap_memory())
  {
    setup_heap_manager();
    game.numfield_C &= 0xFFFB;
    return 1;
  }
  return 0;
}

short setup_screen_mode_minimal(unsigned short nmode)
{
  static const char *func_name="setup_screen_mode_minimal";
  //return _DK_setup_screen_mode_minimal(nmode);
  unsigned int flg_mem;
  struct TbScreenModeInfo *mdinfo;

  if ((nmode == lbDisplay.ScreenMode) && (MinimalResolutionSetup))
    return 1;
  flg_mem = lbDisplay.DrawFlags;
  switch (lbDisplay.ScreenMode)
  {
  case Lb_SCREEN_MODE_320_200_8:
  case Lb_SCREEN_MODE_320_240_8:
  case Lb_SCREEN_MODE_512_384_8:
    if (MinimalResolutionSetup)
    {
      if (nmode != lbDisplay.ScreenMode)
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
      if (nmode != lbDisplay.ScreenMode)
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
      if (nmode != lbDisplay.ScreenMode)
        LbScreenReset();
      LbDataFreeAll(vres256_load_files_minimal);
      MinimalResolutionSetup = 0;
    } else
    {
      reset_eye_lenses();
      reset_heap_manager();
      reset_heap_memory();
      if (nmode != lbDisplay.ScreenMode)
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
    if ( LbDataLoadAll(mcga_load_files_minimal) )
    {
      error(func_name, 895, "Unable to load mcga_load_files_minimal");
      return 0;
    }
    if (nmode != lbDisplay.ScreenMode)
    {
      if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 2, 0) != 1)
      {
        error(func_name, 904, "Unable to setup screen resolution");
        return 0;
      }
    }
    pixel_size = 2;
    MyScreenWidth = mdinfo->Width * pixel_size;
    MyScreenHeight = mdinfo->Height * pixel_size;
    pixels_per_block = 32;
    units_per_pixel = 8;
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
      return 0;
    }
    frontend_load_data_reset();
    if (nmode != lbDisplay.ScreenMode)
    {
     if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, _DK_palette, 1, 0) != 1)
     {
        error(func_name, 904, "Unable to setup screen resolution");
        return 0;
     }
    }
    pixel_size = 1;
    MyScreenWidth = mdinfo->Width * pixel_size;
    MyScreenHeight = mdinfo->Height * pixel_size;
    pixels_per_block = 16;
    units_per_pixel = mdinfo->Width/40;// originally was 16
    LbSpriteSetupAll(setup_sprites_minimal);
    break;
  default:
    error(func_name, 948, "Unhandled Screen Mode (Setup)");
    return 0;
  }
  LbScreenClear(0);
  LbScreenSwap();
  LbMouseSetup(0);
  LbMouseSetPointerHotspot(0, 0);
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  lbDisplay.DrawFlags = flg_mem;
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
