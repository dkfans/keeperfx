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
#include "gui_topmsg.h"
#include "engine_redraw.h"
#include "engine_textures.h"
#include "config.h"
#include "lens_api.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "creature_graphics.h"
#include "keeperfx.hpp"
#include "custom_sprites.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
TbScreenMode switching_vidmodes[] = {
  Lb_SCREEN_MODE_320_200_8,
  Lb_SCREEN_MODE_640_480_8,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  };

TbScreenMode failsafe_vidmode = Lb_SCREEN_MODE_320_200_8;
TbScreenMode movies_vidmode   = Lb_SCREEN_MODE_640_480_8;
TbScreenMode frontend_vidmode = Lb_SCREEN_MODE_640_480_8;

//struct IPOINT_2D units_per_pixel;
unsigned short units_per_pixel_min;
unsigned short units_per_pixel_width;
unsigned short units_per_pixel_height;
unsigned short units_per_pixel_best;
unsigned short units_per_pixel_ui;
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
    // Update size of the parchment buffer, as it is also used as screen buffer
    if (scrbuf_size < 640*480)
        scrbuf_size = 640*480;
    int i = LbDataFindStartIndex(gui_load_files_640, (unsigned char**)&hires_parchment);
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
    struct TbLoadFiles* load_files = gui_load_files_320;
    LbDataFreeAll(load_files);
    int ferror = 0;
    int i = 0;
    struct TbLoadFiles* t_lfile = &load_files[i];
    // Allocate some low memory, only to be sure that
    // it will be free when this function ends
    void* mem = LbMemoryAllocLow(0x10000u);
    while (t_lfile->Start != NULL)
    {
        // Don't allow loading flags
        t_lfile->Flags = 0;
        int ret_val = LbDataLoad(t_lfile);
        if (ret_val == -100)
        {
            ERRORLOG("Can't allocate memory for MCGA files element \"%s\".", t_lfile->FName);
            ferror++;
        }
        else if (ret_val == -101)
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

TbScreenMode get_next_vidmode(TbScreenMode mode)
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

/**
 * Returns first video mode with higher res than given one.
 * @param curr_mode
 * @return
 */
TbScreenMode get_higher_vidmode(TbScreenMode curr_mode)
{
    // Get size of current mode
    TbScreenModeInfo* curr_mdinfo = LbScreenGetModeInfo(curr_mode);
    unsigned long curr_size = 0;
    if (LbScreenIsModeAvailable(curr_mode)) {
        curr_size = curr_mdinfo->Width * curr_mdinfo->Height;
    }
    // Loop in search of higher res mode
    unsigned short next_mode = curr_mode;
    while (next_mode != Lb_SCREEN_MODE_INVALID)
    {
        next_mode = get_next_vidmode(next_mode);
        TbScreenModeInfo* next_mdinfo = LbScreenGetModeInfo(next_mode);
        unsigned long next_size = 0;
        if (LbScreenIsModeAvailable(next_mode)) {
            next_size = next_mdinfo->Width * next_mdinfo->Height;
        }
        // If the next mode is higher, accept it
        if (next_size > curr_size) {
            break;
        }
        // If looped through all modes, use current
        if (next_mode == curr_mode) {
            break;
        }
    }
    return next_mode;
}

void set_game_vidmode(unsigned short i,unsigned short nmode)
{
  switching_vidmodes[i]=(TbScreenMode)nmode;
}

/**
 * Returns max count of video modes which can be set.
 * @return Count,
 */
unsigned short max_game_vidmode_count(void)
{
    return sizeof(switching_vidmodes)/sizeof(switching_vidmodes[0]);
}

TbScreenMode validate_vidmode(TbScreenMode mode)
{
  int maxmodes=sizeof(switching_vidmodes)/sizeof(TbScreenMode);
  // Do not allow to enter higher modes on low memory systems
  if ((features_enabled & Ft_HiResVideo) == 0)
    return failsafe_vidmode;
  for (int i = 0; i < maxmodes; i++)
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
    long i;
    long x;
    long y;
    SYNCDBG(8, "Setting to group %d", (int)group_idx);
    if (pointer_sprites == NULL)
    {
        WARNLOG("Pointer sprites not loaded, setting to none");
        LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
        return false;
  }
  if (is_feature_on(Ft_BigPointer))
  {
    y = 32;
    x = 32;
    i = (is_custom_icon(group_idx)? group_idx : 8*group_idx) + (frame%8);
  } else
  {
    y = 78;
    x = 26;
    i = group_idx;
  }
  const struct TbSprite* spr;

  if (is_custom_icon(i))
  {
      spr = get_new_icon_sprite(i);
      SYNCDBG(8,"Activating pointer %d", i);
      LbMouseChangeSpriteAndHotspot(spr, x/2, y/2);
  }
  else
  {
      spr = &pointer_sprites[40 + i];
      SYNCDBG(8,"Activating pointer %d", 40+i);
      if ((spr >= pointer_sprites) && (spr < end_pointer_sprites))
      {
          LbMouseChangeSpriteAndHotspot(spr, x/2, y/2);
      } else
      {
          WARNLOG("Sprite %d exceeds buffer, setting pointer to none",(int)i);
          LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
      }
  }
  return true;
}

TbBool set_pointer_graphic(long ptr_idx)
{
    long x;
    long y;
    const struct TbSprite* spr;
    SYNCDBG(8, "Setting to %d", (int)ptr_idx);
    if (pointer_sprites == NULL)
    {
        WARNLOG("Pointer sprites not loaded, setting to none");
        LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
        return false;
  }
  switch (ptr_idx)
  {
  case MousePG_Invisible:
  case MousePG_Arrow:
  case MousePG_Pickaxe:
  case MousePG_Query:
  case MousePG_DenyMark:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 15;
      break;
  case MousePG_Sell:
      spr = &pointer_sprites[ptr_idx];
      x = 17; y = 29;
      break;
  case MousePG_PlaceTrap01:
  case MousePG_PlaceTrap02:
  case MousePG_PlaceTrap03:
  case MousePG_PlaceTrap04:
  case MousePG_PlaceTrap05:
  case MousePG_PlaceTrap06:
  case MousePG_PlaceDoor01:
  case MousePG_PlaceDoor02:
  case MousePG_PlaceDoor03:
  case MousePG_PlaceDoor04:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
      break;
  case  MousePG_SpellCharge0:
  case  MousePG_SpellCharge1:
  case  MousePG_SpellCharge2:
  case  MousePG_SpellCharge3:
  case  MousePG_SpellCharge4:
  case  MousePG_SpellCharge5:
  case  MousePG_SpellCharge6:
  case  MousePG_SpellCharge7:
  case  MousePG_SpellCharge8:
      spr = &pointer_sprites[ptr_idx];
      x = 20; y = 20;
      break;
  case  MousePG_PlaceRoom01:
  case  MousePG_PlaceRoom02:
  case  MousePG_PlaceRoom03:
  case  MousePG_PlaceRoom04:
  case  MousePG_PlaceRoom05:
  case  MousePG_PlaceRoom06:
  case  MousePG_PlaceRoom07:
  case  MousePG_PlaceRoom08:
  case  MousePG_PlaceRoom09:
  case  MousePG_PlaceRoom10:
  case  MousePG_PlaceRoom11:
  case  MousePG_PlaceRoom12:
  case  MousePG_PlaceRoom13:
  case  MousePG_PlaceRoom14:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
      break;
  case  MousePG_LockMark:
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
      spr = get_new_icon_sprite(ptr_idx);
      if (spr != NULL)
      {
          LbMouseChangeSpriteAndHotspot(spr, spr->SWidth/2, spr->SHeight);
          return true;
      }
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
    char* fname = prepare_file_path(FGrp_StdData, "tables.dat");
    SYNCDBG(0,"Reading fade table file \"%s\".",fname);
    if (LbFileLoadAt(fname, &pixmap) != sizeof(struct TbColorTables))
    {
        compute_fade_tables(&pixmap,engine_palette,engine_palette);
        LbFileSaveAt(fname, &pixmap, sizeof(struct TbColorTables));
    }
    lbDisplay.FadeTable = pixmap.fade_tables;
    TbPixel cblack = 144;
    // Update black color
    for (long i = 0; i < 8192; i++)
    {
        if (pixmap.fade_tables[i] == 0) {
            pixmap.fade_tables[i] = cblack;
        }
    }
    return true;
}

TbBool init_alpha_table(void)
{
    char* fname = prepare_file_path(FGrp_StdData, "alpha.col");
    SYNCDBG(0,"Reading alpha color table file \"%s\".",fname);
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
    char* fname = prepare_file_path(FGrp_StdData, "colours.col");
    SYNCDBG(0,"Reading rgb-to-index color table file \"%s\".",fname);
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
    char* fname = prepare_file_path(FGrp_StdData, "redpal.col");
    SYNCDBG(0,"Reading red-blended color table file \"%s\".",fname);
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
    char* fname = prepare_file_path(FGrp_StdData, "whitepal.col");
    SYNCDBG(0,"Reading white-blended color table file \"%s\".",fname);
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

char *get_vidmode_name(unsigned short mode)
{
    TbScreenModeInfo* curr_mdinfo = LbScreenGetModeInfo(mode);
    return curr_mdinfo->Desc;
}

TbBool setup_screen_mode(unsigned short nmode)
{
    SYNCDBG(4,"Setting up mode %d",(int)nmode);
    if (!force_video_mode_reset)
    {
      if ((nmode == lbDisplay.ScreenMode) && (!MinimalResolutionSetup))
      {
        SYNCDBG(6,"Mode %d already active, no changes.",(int)nmode);
        return true;
      }
    }
    long lens_mem = game.numfield_1B;
    unsigned int flg_mem = lbDisplay.DrawFlags;
    short was_minimal_res = (MinimalResolutionSetup || force_video_mode_reset);
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
            LbScreenReset();
          LbDataFreeAll(gui_load_files_320);
        }
    } else
    // so (LbGraphicsScreenHeight() >= 400)
    {
        if (MinimalResolutionSetup)
        {
          if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
            LbScreenReset();
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
            LbScreenReset();
          LbDataFreeAll(gui_load_files_640);
        }
    }

    TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
    if (new_mdinfo->Height < 200)
    {
        ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
        force_video_mode_reset = true;
        return false;
    } else
    if (new_mdinfo->Height < 400)
    {
        SYNCDBG(6,"Entering low-res mode %d, resolution %dx%d.",(int)nmode,(int)new_mdinfo->Width,(int)new_mdinfo->Height);
        if (!LoadMcgaData())
        {
          ERRORLOG("Loading Mcga files failed");
          force_video_mode_reset = true;
          return false;
        }
        if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
        {
            if (LbScreenSetup((TbScreenMode)nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, 2, 0) != 1)
            {
              ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                  new_mdinfo->Desc,(int)nmode);
              force_video_mode_reset = true;
              return false;
            }
        }
        load_pointer_file(0);
    } else
    // so (new_mdinfo->Height >= 400)
    {
        SYNCDBG(6,"Entering hi-res mode %d, resolution %dx%d.",(int)nmode,(int)new_mdinfo->Width,(int)new_mdinfo->Height);
        if (!LoadVRes256Data((long)new_mdinfo->Width*(long)new_mdinfo->Height))
        {
          ERRORLOG("Unable to load VRes256 data files");
          force_video_mode_reset = true;
          return false;
        }
        if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
        {
            if (LbScreenSetup((TbScreenMode)nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, 1, 0) != 1)
            {
              ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                  new_mdinfo->Desc,(int)nmode);
              force_video_mode_reset = true;
              return false;
            }
        }
        load_pointer_file(1);
    }
    LbScreenClear(0);
    LbScreenSwap();
    update_screen_mode_data(new_mdinfo->Width, new_mdinfo->Height);
    if (parchment_loaded)
      reload_parchment_file(new_mdinfo->Width >= 640);
    reinitialise_eye_lens(lens_mem);
    lbDisplay.DrawFlags = flg_mem;
    if (!setup_heap_memory())
    {
      force_video_mode_reset = true;
      return false;
    }
    setup_heap_manager();
    game.operation_flags &= ~GOF_Unkn04;
    force_video_mode_reset = false;
    SYNCDBG(8,"Finished");
    return true;
}

TbBool update_screen_mode_data(long width, long height)
{
  // if ((width >= 640) && (height >= 400))
  // {
    pixel_size = 1;
    /*
  } else
  {
    pixel_size = 2;
  }
  */
  long psize = pixel_size;
  MyScreenWidth = width * psize;
  MyScreenHeight = height * psize;
  pixels_per_block = 16 * psize;
  units_per_pixel = (width>height?width:height)/40;// originally was 16 for hires, 8 for lores
  units_per_pixel_min = (width>height?height:width)/40;// originally 10 for hires
  units_per_pixel_width = width/40; // 8 for low res, 16 is "kfx default"
  units_per_pixel_height = height/25; // 8 for low res, 16 is "kfx default"
  units_per_pixel_best = ((is_ar_wider_than_original(width, height)) ? units_per_pixel_height : units_per_pixel_width); // 8 for low res, 16 is "kfx default"
  long ui_scale = UI_NORMAL_SIZE; // UI_NORMAL_SIZE, UI_HALF_SIZE, or UI_DOUBLE_SIZE (not fully implemented yet)
  units_per_pixel_ui = resize_ui(units_per_pixel_best, ui_scale);

  if (MinimalResolutionSetup)
    LbSpriteSetupAll(setup_sprites_minimal);
  else
    LbSpriteSetupAll(setup_sprites);
  LbMouseChangeMoveRatio(base_mouse_sensitivity*units_per_pixel/16, base_mouse_sensitivity*units_per_pixel/16);
  LbMouseSetPointerHotspot(0, 0);
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbMouseSetup(NULL);
  return true;
}

short setup_screen_mode_minimal(unsigned short nmode)
{
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  if (!force_video_mode_reset)
  {
    if ((nmode == lbDisplay.ScreenMode) && (MinimalResolutionSetup))
      return 1;
  }
  unsigned int flg_mem = lbDisplay.DrawFlags;
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
          LbScreenReset();
        LbDataFreeAll(gui_load_files_320);
      }
  } else
  {
      if (MinimalResolutionSetup)
      {
        set_pointer_graphic_none();
        unload_pointer_file(1);
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(front_load_files_minimal_640);
        MinimalResolutionSetup = 0;
      } else
      {
        reset_eye_lenses();
        reset_heap_manager();
        reset_heap_memory();
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(gui_load_files_640);
      }
  }
  TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
  if (new_mdinfo->Height < 200)
  {
      ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
      force_video_mode_reset = true;
      return 0;
  } else
  if (new_mdinfo->Height < 400)
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
          if (LbScreenSetup((TbScreenMode)nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, 2, 0) != 1)
          {
            ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                new_mdinfo->Desc,(int)nmode);
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
        ERRORLOG("Unable to load VRes256 front_load minimal files");
        force_video_mode_reset = true;
        return 0;
      }
      frontend_load_data_reset();
      if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
      {
          if (LbScreenSetup((TbScreenMode)nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, 1, 0) != 1)
          {
             ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                 new_mdinfo->Desc,(int)nmode);
             force_video_mode_reset = true;
             return 0;
          }
      }
  }
  LbScreenClear(0);
  LbScreenSwap();
  update_screen_mode_data(new_mdinfo->Width, new_mdinfo->Height);
  lbDisplay.DrawFlags = flg_mem;
  force_video_mode_reset = false;
  return 1;
}

TbBool setup_screen_mode_zero(unsigned short nmode)
{
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
  LbPaletteDataFillBlack(engine_palette);
  if (LbScreenSetup((TbScreenMode)nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, 2, 0) != 1)
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)",
          new_mdinfo->Desc,(int)nmode);
      return false;
  }
  force_video_mode_reset = true;
  return true;
}

TbScreenMode reenter_video_mode(void)
{
    TbScreenMode scrmode = validate_vidmode(settings.video_scrnmode);
    if (setup_screen_mode(scrmode))
    {
        settings.video_scrnmode = scrmode;
        copy_settings_to_dk_settings();
  } else
  {
      SYNCLOG("Can't enter %s (mode %d), falling to failsafe mode",
          get_vidmode_name(scrmode),(int)scrmode);
      scrmode=get_failsafe_vidmode();
      if ( !setup_screen_mode(scrmode) )
      {
        FatalError = 1;
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
    TbScreenMode scrmode = get_next_vidmode(lbDisplay.ScreenMode);
    if ( setup_screen_mode(scrmode) )
    {
        show_onscreen_msg(game.num_fps * 6, "%s", get_vidmode_name(scrmode));
        settings.video_scrnmode = scrmode;
    } else
    {
        SYNCLOG("Can't enter %s (mode %d), falling to failsafe mode",
            get_vidmode_name(scrmode),(int)scrmode);
        show_onscreen_msg(game.num_fps * 6, "%s", get_string(856));
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
    reinit_all_menus();
    init_custom_sprites(SPRITE_LAST_LEVEL);
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
