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
#include "pre_inc.h"
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
#include "sprites.h"
#include "post_inc.h"

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

static TbScreenMode failsafe_vidmode = Lb_SCREEN_MODE_320_200_8;
static TbScreenMode movies_vidmode   = Lb_SCREEN_MODE_640_480_8;
static TbScreenMode frontend_vidmode = Lb_SCREEN_MODE_640_480_8;

//struct IPOINT_2D units_per_pixel;
unsigned short units_per_pixel_min;
unsigned short units_per_pixel_width;
unsigned short units_per_pixel_height;
unsigned short units_per_pixel_best;
unsigned short units_per_pixel_ui;
unsigned long aspect_ratio_factor_HOR_PLUS;
unsigned long aspect_ratio_factor_HOR_PLUS_AND_VERT_PLUS;
unsigned long first_person_horizontal_fov;
unsigned long first_person_vertical_fov;
long base_mouse_sensitivity = 256;

static TbBool force_video_mode_reset = true;

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

TbBool MinimalResolutionSetup;

struct TbColorTables pixmap;
struct TbAlphaTables alpha_sprite_table;
unsigned char white_pal[256];
unsigned char red_pal[256];
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
    return Lb_SCREEN_MODE_320_200_8;
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

void set_game_vidmode(unsigned short i, TbScreenMode nmode)
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
    return Lb_SCREEN_MODE_320_200_8;
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

void set_failsafe_vidmode(TbScreenMode nmode)
{
  failsafe_vidmode=(TbScreenMode)nmode;
}

TbScreenMode get_movies_vidmode(void)
{
  return movies_vidmode;
}

void set_movies_vidmode(TbScreenMode nmode)
{
  movies_vidmode=(TbScreenMode)nmode;
}

TbScreenMode get_frontend_vidmode(void)
{
  return frontend_vidmode;
}

void set_frontend_vidmode(TbScreenMode nmode)
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
  LbMouseChangeSpriteAndHotspot(&frontend_sprite[GFS_cursor_horny], 0, 0);
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
  case MousePG_PlaceTrap07:
  case MousePG_PlaceTrap08:
  case MousePG_PlaceTrap09:
  case MousePG_PlaceTrap10:
  case MousePG_PlaceTrap11:
  case MousePG_PlaceTrap12:
  case MousePG_PlaceTrap13:
  case MousePG_PlaceTrap14:
  case MousePG_PlaceDoor01:
  case MousePG_PlaceDoor02:
  case MousePG_PlaceDoor03:
  case MousePG_PlaceDoor04:
  case MousePG_Mystery:
      // 166..181 are place trap pointers with spell icons
  case 166:
  case 167:
  case 168:
  case 169:
  case 170:
  case 171:
  case 172:
  case 173:
  case 174:
  case 175:
  case 176:
  case 177:
  case 178:
  case 179:
  case 180:
  case 181:
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
  case  MousePG_PlaceRoom15:
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
  case MousePG_PlaceImpRock:
  case MousePG_PlaceGold:
  case MousePG_PlaceEarth:
  case MousePG_PlaceWall:
  case MousePG_PlacePath:
  case MousePG_PlaceClaimed:
  case MousePG_PlaceLava:
  case MousePG_PlaceWater:
  case MousePG_PlaceGems:
  case MousePG_MkDigger:
  case MousePG_MkCreature:
  case MousePG_MvCreature:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
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

char *get_vidmode_name(TbScreenMode mode)
{
    TbScreenModeInfo* curr_mdinfo = LbScreenGetModeInfo(mode);
    return curr_mdinfo->Desc;
}

TbBool setup_screen_mode(TbScreenMode nmode)
{
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  TbScreenMode old_mode = LbScreenActiveMode();
  if (!force_video_mode_reset)
  {
    if ((nmode == old_mode) && (!MinimalResolutionSetup))
    {
      SYNCDBG(6,"Mode %d already active, no changes.",(int)nmode);
      return true;
    }
  }
  TbBool hi_res = ((LbGraphicsScreenHeight() < 400) ? false : true);
  long lens_mem = game.numfield_1B;
  unsigned int flg_mem = lbDisplay.DrawFlags;
  TbBool was_minimal_res = (MinimalResolutionSetup || force_video_mode_reset);
  set_pointer_graphic_none();
  if (LbGraphicsScreenHeight() < 200)
  {
      WARNLOG("Unhandled previous Screen Mode %d, Reset skipped",(int)old_mode);
  } else
  {
    if (!MinimalResolutionSetup)
    {
      reset_eye_lenses();
      reset_heap_manager();
      reset_heap_memory();
      unload_pointer_file(hi_res);
    }
    if (nmode != old_mode)
        LbScreenReset();
    if (MinimalResolutionSetup)
      LbDataFreeAll(hi_res ? front_load_files_minimal_640 : front_load_files_minimal_320);
    else
      LbDataFreeAll(hi_res ? gui_load_files_640 : gui_load_files_320);
    if (!hi_res) ERRORLOG("MCGA Minimal not allowed (Reset)");
    MinimalResolutionSetup = false;
  }
  TbScreenModeInfo* old_mdinfo = LbScreenGetModeInfo(LbScreenActiveMode());
  if (!(old_mdinfo->VideoFlags & Lb_VF_FILLALL))
  {
      display_id = LbGetCurrentDisplayIndex();
  }
  TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
  // Check that the desired mode is available for the current display
  if (!LbScreenIsModeAvailable(nmode, display_id))
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
      force_video_mode_reset = true;
      return false;
  }
  hi_res = ((new_mdinfo->Height < 400) ? false : true);
  if (new_mdinfo->Height < 200)
  {
      ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
      force_video_mode_reset = true;
      return false;
  } else
  {
    SYNCDBG(6,"Entering %s mode %d, resolution %dx%d.",hi_res?"hi-res":"low-res",(int)nmode,(int)new_mdinfo->Width,(int)new_mdinfo->Height);
    if (hi_res)
    {
      if (!LoadVRes256Data((long)new_mdinfo->Width*(long)new_mdinfo->Height))
      {
        ERRORLOG("Unable to load VRes256 data files");
        force_video_mode_reset = true;
        return false;
      }
    }
    else
    {
      if (!LoadMcgaData())
      {
        ERRORLOG("Loading Mcga files failed");
        force_video_mode_reset = true;
        return false;
      }
    }
    if ((nmode != old_mode) || (was_minimal_res))
    {
        if (LbScreenSetup(nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, (hi_res ? 1 : 2), 0) < Lb_SUCCESS)
        {
          ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
          force_video_mode_reset = true;
          return false;
        }
    }
    load_pointer_file(hi_res);
  }
  LbScreenClear(0);
  LbScreenSwap();
  update_screen_mode_data(new_mdinfo->Width, new_mdinfo->Height);
  if (parchment_loaded)
    reload_parchment_file(hi_res);
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
  calculate_aspect_ratio_factor(width, height);
  first_person_vertical_fov = DEFAULT_FIRST_PERSON_VERTICAL_FOV;
  first_person_horizontal_fov = FOV_based_on_aspect_ratio();

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

TbBool setup_screen_mode_minimal(TbScreenMode nmode)
{
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  TbScreenMode old_mode = LbScreenActiveMode();
  if (!force_video_mode_reset)
  {
    if ((nmode == old_mode) && (MinimalResolutionSetup))
    {
      SYNCDBG(6,"Mode %d already active, no changes.",(int)nmode);
      return true;
    }
  }
  TbBool hi_res = ((LbGraphicsScreenHeight() < 400) ? false : true);
  ushort flg_mem = lbDisplay.DrawFlags;
  if (LbGraphicsScreenHeight() < 200)
  {
    WARNLOG("Unhandled previous Screen Mode %d, Reset skipped",(int)old_mode);
  } else
  {
    if (!MinimalResolutionSetup)
    {
      reset_eye_lenses();
      reset_heap_manager();
      reset_heap_memory();
    }
    if ((!MinimalResolutionSetup && !hi_res) || (MinimalResolutionSetup && hi_res))
      unload_pointer_file(hi_res);
    if ((nmode != old_mode) || (force_video_mode_reset))
      LbScreenReset();
    if (hi_res)
    {
      if (MinimalResolutionSetup)
        LbDataFreeAll(front_load_files_minimal_640);
      else
        LbDataFreeAll(gui_load_files_640);
    }
    else
    {
      if (MinimalResolutionSetup)
        LbDataFreeAll(front_load_files_minimal_320);
      else
        LbDataFreeAll(gui_load_files_320);
    }
    MinimalResolutionSetup = false;
  }
  TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
  // Check that the desired mode is available for the current display
  if (!LbScreenIsModeAvailable(nmode, display_id))
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
      force_video_mode_reset = true;
      return false;
  }
  // we don't want to get the current display when using the "fill all" mode, we want to keep the old version
  TbScreenModeInfo* old_mdinfo = LbScreenGetModeInfo(old_mode);
  if (!(old_mdinfo->VideoFlags & Lb_VF_FILLALL))
  {
      display_id = LbGetCurrentDisplayIndex(); // get current display
  }
  hi_res = ((new_mdinfo->Height < 400) ? false : true);
  if (new_mdinfo->Height < 200)
  {
      ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
      force_video_mode_reset = true;
      return false;
  } else
  {
    SYNCDBG(17,"Preparing minimal %s resolution mode",(hi_res ? "high" : "low"));
    MinimalResolutionSetup = true;
    if (hi_res)
    {
      frontend_load_data_from_cd();
      if ( LbDataLoadAll(front_load_files_minimal_640) )
      {
        ERRORLOG("Unable to load VRes256 front_load minimal files");
        force_video_mode_reset = true;
        return false;
      }
      frontend_load_data_reset();
    }
    else
    {
      if ( !LoadMcgaDataMinimal() )
      {
        ERRORLOG("Unable to load minimal MCGA files");
        return false;
      }
    }
    
    if ((nmode != old_mode) || (force_video_mode_reset))
    {
        if (LbScreenSetup(nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, (hi_res ? 1 : 2), 0) < Lb_SUCCESS)
        {
          ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
          force_video_mode_reset = true;
          return false;
        }
    }
  }
  LbScreenClear(0);
  LbScreenSwap();
  update_screen_mode_data(new_mdinfo->Width, new_mdinfo->Height);
  lbDisplay.DrawFlags = flg_mem;
  force_video_mode_reset = false;
  return true;
}

/**
 * Set up a new screen mode with a blank black screen.
 * 
 * @param nmode The mode (index number) that we want to change to.
 * @return Returns TRUE if the screen was setup successfully. 
 */
TbBool setup_screen_mode_zero(TbScreenMode nmode)
{
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
  // Check that the desired mode is available for the current display
  if (!LbScreenIsModeAvailable(nmode, display_id))
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
      return false;
  }
  LbPaletteDataFillBlack(engine_palette);
  if (LbScreenSetup(nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, 2, 0) < Lb_SUCCESS)
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
      return false;
  }
  update_screen_mode_data(new_mdinfo->Width, new_mdinfo->Height);
  force_video_mode_reset = true;
  return true;
}

TbScreenMode reenter_video_mode(void)
{
    TbScreenMode scrmode = validate_vidmode(settings.video_scrnmode);
    if (setup_screen_mode(scrmode))
    {
        settings.video_scrnmode = scrmode;
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
    char percent_x = ((float)lbDisplay.MMouseX / (float)(lbDisplay.MouseWindowX + lbDisplay.MouseWindowWidth)) * 100;
    char percent_y = ((float)lbDisplay.MMouseY / (float)(lbDisplay.MouseWindowY + lbDisplay.MouseWindowHeight)) * 100;
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
    TbBool reload_video = (menu_is_active(GMnu_VIDEO));
    if (menu_is_active(GMnu_CREATURE_QUERY1))
    {
        vid_change_query_menu = GMnu_CREATURE_QUERY1;
    }
    else if (menu_is_active(GMnu_CREATURE_QUERY2))
    {
        vid_change_query_menu = GMnu_CREATURE_QUERY2;
    }
    else if (menu_is_active(GMnu_CREATURE_QUERY3))
    {
        vid_change_query_menu = GMnu_CREATURE_QUERY3;
    }
    else if (menu_is_active(GMnu_CREATURE_QUERY4))
    {
        vid_change_query_menu = GMnu_CREATURE_QUERY4;
    }
    reinit_all_menus();
    if (reload_video)
    {
        turn_on_menu(GMnu_VIDEO);
    }
    LbMouseSetPosition(((lbDisplay.MouseWindowX + lbDisplay.MouseWindowWidth) / 100) * percent_x, ((lbDisplay.MouseWindowY + lbDisplay.MouseWindowHeight) / 100) * percent_y);
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
