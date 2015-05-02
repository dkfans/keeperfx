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

#include "vidfade.h"
#include "front_simple.h"
#include "front_landview.h"
#include "frontend.h"
#include "game_heap.h"
#include "gui_draw.h"
#include "gui_parchment.h"
#include "engine_redraw.h"
#include "engine_textures.h"
#include "config.h"
#include "lens_api.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "creature_graphics.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
TbScreenMode switching_vidmodes[] = {
    Lb_SCREEN_MODE_640_480,
    Lb_SCREEN_MODE_INVALID,
    Lb_SCREEN_MODE_INVALID,
    Lb_SCREEN_MODE_INVALID,
    Lb_SCREEN_MODE_INVALID,
    };

TbScreenMode failsafe_vidmode = Lb_SCREEN_MODE_640_480;
TbScreenMode movies_vidmode = Lb_SCREEN_MODE_320_200;
TbScreenMode frontend_vidmode = Lb_SCREEN_MODE_640_480;

//struct IPOINT_2D units_per_pixel;
unsigned short units_per_pixel_min;
long base_mouse_sensitivity = 256;

short force_video_mode_reset = true;

struct TbSprite *pointer_sprites;
struct TbSprite *end_pointer_sprites;
unsigned char * pointer_data;

struct TbSprite *end_map_font;
struct TbSprite *end_map_hand;
TbSpriteData map_font_data;
TbSpriteData end_map_font_data;
TbSpriteData map_hand_data;
TbSpriteData end_map_hand_data;
struct MapLevelInfo map_info;
/******************************************************************************/

extern struct TbSetupSprite setup_sprites_minimal[];
extern struct TbSetupSprite setup_sprites[];
#if (BFDEBUG_LEVEL > 0)
// Declarations for font testing screen (debug version only)
extern struct TbSetupSprite setup_testfont[];
extern struct TbLoadFiles testfont_load_files[];
#endif

extern struct TbLoadFiles gui_load_files_320[];
extern struct TbLoadFiles gui_load_files_640[];
extern struct TbLoadFiles front_load_files_minimal_320[];
extern struct TbLoadFiles front_load_files_minimal_640[];
extern struct TbLoadFiles pointer_load_files_320[];
extern struct TbLoadFiles pointer_small_load_files_320[];
extern struct TbLoadFiles pointer_load_files_640[];
extern struct TbLoadFiles pointer_small_load_files_640[];
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
    i = LbDataFindStartIndex(gui_load_files_640,(unsigned char **)&hires_parchment);
    if (i>=0) {
        gui_load_files_640[i].SLength = scrbuf_size;
    }
    // Load the files
    if (LbDataLoadAll(gui_load_files_640)) {
        return 0;
    }
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
  struct TbLoadFiles *load_files;
  void *mem;
  struct TbLoadFiles *t_lfile;
  int ferror;
  int ret_val;
  int i;
  load_files = gui_load_files_320;
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
  if (LbDataLoadAll(front_load_files_minimal_320))
    return 0;
  return 1;
}

// Find the index of next video mode registered in switching_vidmodes list, may loop back to original place.
TbScreenMode get_next_vidmode_for_switching(unsigned short currentModeIndex)
{
    int i = 0;
    int modeCount = sizeof(switching_vidmodes) / sizeof(TbScreenMode);

    // TODO HeM need to check if there is any good mode exists in the list.

    // Do not allow to enter higher modes on low memory systems
    if ((features_enabled & Ft_HiResVideo) == 0)
    {
        SYNCLOG("Hi Res feature not enabled, failsafing.");
        return failsafe_vidmode;
    }

    // Finding index of current active mode in switching_vidmodes list.
    while ((i < modeCount) && (switching_vidmodes[i] != currentModeIndex))
    {
        i++;
    }
    i++;

    if ((i >= modeCount) ||
        (switching_vidmodes[i] == Lb_SCREEN_MODE_INVALID))
    {
        i = 0;
    }

    return switching_vidmodes[i];
}

// Register index of video mode for switching
void register_vidmode_index_for_switching(unsigned short i,unsigned short nmode)
{
    switching_vidmodes[i] = (TbScreenMode)nmode;
}

// Search for given mode in switching mode list,
// In case it is not in the list or is not supported, return failsafe mode.
TbScreenMode validate_vidmode_in_switching_list(unsigned short mode)
{
    int i;
    int modeCount = sizeof(switching_vidmodes) / sizeof(TbScreenMode);

    // Do not allow to enter higher modes on low memory systems
    if ((features_enabled & Ft_HiResVideo) == 0)
    {
        return failsafe_vidmode;
    }

    for (i = 0; i < modeCount; i++)
    {
        if (switching_vidmodes[i] == mode)
        {
            return switching_vidmodes[i];
        }
    }

    // Return first mode in list, not failsafe mode.
    return switching_vidmodes[0];
}

TbScreenMode get_failsafe_vidmode(void)
{
    return failsafe_vidmode;
}

TbScreenMode get_movies_vidmode(void)
{
    return movies_vidmode;
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
      ldfiles = pointer_small_load_files_640;
    else
      ldfiles = pointer_small_load_files_320;
  } else
  {
    if (hi_res)
      ldfiles = pointer_load_files_640;
    else
      ldfiles = pointer_load_files_320;
  }
  if ( LbDataLoadAll(ldfiles) )
    ERRORLOG("Unable to load pointer files");
  LbSpriteSetup(pointer_sprites, end_pointer_sprites, pointer_data);
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
  SYNCDBG(8,"Setting to group %d",(int)group_idx);
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
    WARNLOG("Sprite %d exceeds buffer, setting pointer to none",(int)i);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  }
  return true;
}

TbBool set_pointer_graphic(long ptr_idx)
{
  long x,y;
  struct TbSprite *spr;
  SYNCDBG(8,"Setting to %d",(int)ptr_idx);
  if (pointer_sprites == NULL)
  {
    WARNLOG("Pointer sprites not loaded, setting to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  switch (ptr_idx)
  {
  case MousePG_None:
  case MousePG_Cursor:
  case MousePG_Dig:
  case MousePG_Query:
  case MousePG_Unkn15:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 15;
      break;
  case MousePG_Sell:
      spr = &pointer_sprites[ptr_idx];
      x = 17; y = 29;
      break;
  case MousePG_Unkn05:
  case MousePG_Unkn06:
  case MousePG_Unkn07:
  case MousePG_Unkn08:
  case MousePG_Unkn09:
  case MousePG_Unkn10:
  case MousePG_Unkn11:
  case MousePG_Unkn12:
  case MousePG_Unkn13:
  case MousePG_Unkn14:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
      break;
  case  MousePG_Unkn16:
  case  MousePG_Unkn17:
  case  MousePG_Unkn18:
  case  MousePG_Unkn19:
  case  MousePG_Unkn20:
  case  MousePG_Unkn21:
  case  MousePG_Unkn22:
  case  MousePG_Unkn23:
  case  MousePG_Unkn24:
      spr = &pointer_sprites[ptr_idx];
      x = 20; y = 20;
      break;
  case  MousePG_Unkn25:
  case  MousePG_Unkn26:
  case  MousePG_Unkn27:
  case  MousePG_Unkn28:
  case  MousePG_Unkn29:
  case  MousePG_Unkn30:
  case  MousePG_Unkn31:
  case  MousePG_Unkn32:
  case  MousePG_Unkn33:
  case  MousePG_Unkn34:
  case  MousePG_Unkn35:
  case  MousePG_Unkn36:
  case  MousePG_Unkn37:
  case  MousePG_Unkn38:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
      break;
  case  MousePG_Key:
  // 40..144 are spell pointers
  case  MousePG_Unkn47:
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
    WARNLOG("Sprite %d exceeds buffer, setting pointer to none",(int)ptr_idx);
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
      ldfiles = pointer_small_load_files_640;
    else
      ldfiles = pointer_small_load_files_320;
  } else
  {
    if (hi_res)
      ldfiles = pointer_load_files_640;
    else
      ldfiles = pointer_load_files_320;
  }
  LbDataFreeAll(ldfiles);
}

TbBool init_fades_table(void)
{
    char *fname;
    long i;
    static const char textname[] = "fade table";
    fname = prepare_file_path(FGrp_StdData,"tables.dat");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    if (LbFileLoadAt(fname, &pixmap) != sizeof(struct TbColorTables))
    {
        compute_fade_tables(&pixmap,engine_palette,engine_palette);
        LbFileSaveAt(fname, &pixmap, sizeof(struct TbColorTables));
    }
    lbDisplay.FadeTable = pixmap.fade_tables;
    TbPixel cblack = 144;
    // Update black color
    for (i=0; i < 8192; i++)
    {
        if (pixmap.fade_tables[i] == 0) {
            pixmap.fade_tables[i] = cblack;
        }
    }
    return true;
}

TbBool init_alpha_table(void)
{
    char *fname;
    static const char textname[] = "alpha color table";
    fname = prepare_file_path(FGrp_StdData,"alpha.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &alpha_sprite_table) != sizeof(struct TbAlphaTables))
    {
        compute_alpha_tables(&alpha_sprite_table,engine_palette,engine_palette);
        LbFileSaveAt(fname, &alpha_sprite_table, sizeof(struct TbAlphaTables));
    }
    return true;
}

TbBool init_rgb2idx_table(void)
{
    char *fname;
    static const char textname[] = "rgb-to-index color table";
    fname = prepare_file_path(FGrp_StdData,"colours.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &colours) != sizeof(TbRGBColorTable))
    {
        compute_rgb2idx_table(colours,engine_palette);
        LbFileSaveAt(fname, &colours, sizeof(TbRGBColorTable));
    }
    return true;
}

TbBool init_redpal_table(void)
{
    char *fname;
    static const char textname[] = "red-blended color table";
    fname = prepare_file_path(FGrp_StdData,"redpal.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &red_pal) != 256)
    {
        compute_shifted_palette_table(red_pal, engine_palette, engine_palette, 20, -10, -10);
        LbFileSaveAt(fname, &red_pal, 256);
    }
    return true;
}

TbBool init_whitepal_table(void)
{
    char *fname;
    static const char textname[] = "white-blended color table";
    fname = prepare_file_path(FGrp_StdData,"whitepal.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &white_pal) != 256)
    {
        compute_shifted_palette_table(white_pal, engine_palette, engine_palette, 48, 48, 48);
        LbFileSaveAt(fname, &white_pal, 256);
    }
    return true;
}

void init_colours(void)
{
    init_rgb2idx_table();
    init_redpal_table();
    init_whitepal_table();
}

char *get_vidmode_description(unsigned short mode)
{
  TbScreenModeInfo *mdinfo;
  mdinfo = LbScreenGetModeInfo(mode);
  return   mdinfo->Desc;
}

TbBool setup_screen_mode(unsigned short nmode)
{
    TbScreenModeInfo *mdinfo;
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
            LbScreenReset(false);
          LbDataFreeAll(front_load_files_minimal_320);
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
            LbScreenReset(false);
          LbDataFreeAll(gui_load_files_320);
        }
    } else
    // so (LbGraphicsScreenHeight() >= 400)
    {
        if (MinimalResolutionSetup)
        {
          if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
            LbScreenReset(false);
          LbDataFreeAll(front_load_files_minimal_640);
          MinimalResolutionSetup = 0;
        } else
        {
          reset_eye_lenses();
          reset_heap_manager();
          reset_heap_memory();
          set_pointer_graphic_none();
          unload_pointer_file(1);
          if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
            LbScreenReset(false);
          LbDataFreeAll(gui_load_files_640);
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
        SYNCDBG(6,"Entering low-res mode %d, resolution %dx%d.",(int)nmode,(int)mdinfo->Width,(int)mdinfo->Height);
        if (!LoadMcgaData())
        {
          ERRORLOG("Loading Mcga files failed");
          force_video_mode_reset = true;
          return false;
        }
        if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
        {
            if (LbScreenSetup((TbScreenMode)nmode, engine_palette, 2, 0) != 1)
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
        SYNCDBG(6,"Entering hi-res mode %d, resolution %dx%d.",(int)nmode,(int)mdinfo->Width,(int)mdinfo->Height);
        if (!LoadVRes256Data((long)mdinfo->Width*(long)mdinfo->Height))
        {
          ERRORLOG("Unable to load VRes256 data files");
          force_video_mode_reset = true;
          return false;
        }
        if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
        {
            if (LbScreenSetup((TbScreenMode)nmode, engine_palette, 1, 0) != 1)
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
    LbScreenRender();
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
    game.status_flags &= ~Status_Unknown;
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
          LbScreenReset(false);
        LbDataFreeAll(front_load_files_minimal_320);
        MinimalResolutionSetup = 0;
      } else
      {
        reset_eye_lenses();
        reset_heap_manager();
        reset_heap_memory();
        set_pointer_graphic_none();
        unload_pointer_file(0);
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset(false);
        LbDataFreeAll(gui_load_files_320);
      }
  } else
  {
      if (MinimalResolutionSetup)
      {
        set_pointer_graphic_none();
        unload_pointer_file(1);
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset(false);
        LbDataFreeAll(front_load_files_minimal_640);
        MinimalResolutionSetup = 0;
      } else
      {
        reset_eye_lenses();
        reset_heap_manager();
        reset_heap_memory();
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset(false);
        LbDataFreeAll(gui_load_files_640);
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
          if (LbScreenSetup((TbScreenMode)nmode, engine_palette, 2, 0) != 1)
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
      if ( LbDataLoadAll(front_load_files_minimal_640) )
      {
        ERRORLOG("Unable to load vres256_load_files_minimal files");
        force_video_mode_reset = true;
        return 0;
      }
      frontend_load_data_reset();
      if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
      {
          if (LbScreenSetup((TbScreenMode)nmode, engine_palette, 1, 0) != 1)
          {
             ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                 mdinfo->Desc,(int)nmode);
             force_video_mode_reset = true;
             return 0;
          }
      }
  }
  LbScreenClear(0);
  LbScreenRender();
  update_screen_mode_data(mdinfo->Width, mdinfo->Height);
  lbDisplay.DrawFlags = flg_mem;
  force_video_mode_reset = false;
  return 1;
}

// Setup initial screen mode for displaying legal screen.
TbBool setup_screen_mode_zero(unsigned short nmode)
{
  TbScreenModeInfo *mdinfo;
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  mdinfo = LbScreenGetModeInfo(nmode);
  LbPaletteDataFillBlack(engine_palette);
  if (LbScreenSetup((TbScreenMode)nmode, engine_palette, 2, 0) != 1)
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
    scrmode = validate_vidmode_in_switching_list(settings.video_scrnmode);
    SYNCDBG(8, "reentering video %d -> %d .", settings.video_scrnmode, (int)scrmode);

    // In game resolution may be different with the frontend resolution, so we recreate window.
    // TODO HeM Mefisto: Use unified resolution for everything except movie when the Config UI
    // is updated, then we eliminate methods related to 'frontend_vidmode' and use same video
    // mode list with game play. Then we can remove this line.
    LbScreenReset(true);

    if (setup_screen_mode(scrmode))
    {
        settings.video_scrnmode = scrmode;
    }
    else
    {
        SYNCLOG("Can't enter %s (mode %d), falling to failsafe mode.", get_vidmode_description(scrmode), (int)scrmode);
        scrmode = get_failsafe_vidmode();
        if (!setup_screen_mode(scrmode))
        {
            SYNCLOG("Can't enter failsafe mode, exiting game.");
            FatalError = 1;
            exit_keeper = 1;
            return Lb_SCREEN_MODE_INVALID;
        }
        settings.video_scrnmode = scrmode;
        save_settings();
    }

    SYNCLOG("reentered video to %s (mode %d).", get_vidmode_description(scrmode), (int)scrmode);
    return scrmode;
}

TbScreenMode switch_to_next_video_mode(void)
{
    TbScreenMode scrmode;
    scrmode = get_next_vidmode_for_switching(lbDisplay.ScreenMode);
    SYNCDBG(8, "get_next_vidmode_for_switching %d -> %d", lbDisplay.ScreenMode, scrmode);

    // Don't do anything if there is only one screen mode registered, or failed to get any valid mode.
    if ((scrmode != lbDisplay.ScreenMode) &&
        (scrmode != Lb_SCREEN_MODE_INVALID))
    {
        // Destory current window.
        LbScreenReset(true);

        if (setup_screen_mode(scrmode))
        {
            settings.video_scrnmode = scrmode;
        }
        else
        {
            SYNCLOG("Can't enter %s (mode %d), falling to failsafe mode",
                get_vidmode_description(scrmode), (int)scrmode);
            scrmode = get_failsafe_vidmode();
            if (!setup_screen_mode(scrmode))
            {
                SYNCLOG("Can't enter failsafe mode, exiting game.");
                FatalError = 1;
                exit_keeper = 1;
                return Lb_SCREEN_MODE_INVALID;
            }
            settings.video_scrnmode = scrmode;
        }
        SYNCLOG("Switched video to %s (mode %d)", get_vidmode_description(scrmode), (int)scrmode);
        save_settings();
        reinit_all_menus();
    }

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
