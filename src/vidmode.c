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
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"
#include "bflib_dernc.h"
#include "bflib_sprfnt.h"
#include "bflib_filelst.h"


#include "config_spritecolors.h"
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
#include "config_keeperfx.h"
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
unsigned short units_per_pixel_menu_height;
unsigned short units_per_pixel_best;
unsigned short units_per_pixel_menu;
unsigned short units_per_pixel_landview;
unsigned short units_per_pixel_landview_frame;
unsigned short units_per_pixel_ui;
unsigned long aspect_ratio_factor_HOR_PLUS;
unsigned long aspect_ratio_factor_HOR_PLUS_AND_VERT_PLUS;
unsigned long first_person_horizontal_fov;
unsigned long first_person_vertical_fov;
unsigned long landview_frame_movement_scale_x;
unsigned long landview_frame_movement_scale_y;
long base_mouse_sensitivity = 256;

static TbBool force_video_mode_reset = true;
struct TbSpriteSheet * pointer_sprites = NULL;
struct MapLevelInfo map_info;

TbBool MinimalResolutionSetup;

struct TbColorTables pixmap;
struct TbAlphaTables alpha_sprite_table;
unsigned char white_pal[256];
unsigned char red_pal[256];
/******************************************************************************/

#if (BFDEBUG_LEVEL > 0)
// Declarations for font testing screen (debug version only)
extern struct TbLoadFiles testfont_load_files[];
#endif

extern struct TbLoadFiles gui_load_files_320[];
extern struct TbLoadFiles gui_load_files_640[];
extern struct TbLoadFiles front_load_files_minimal_640[];
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
    gui_load_files_640[1].SLength = scrbuf_size;
    // Load the files
    winfont = load_font("data/font2-64.dat", "data/font2-64.tab");
    font_sprites = load_font("data/font1-64.dat", "data/font1-64.tab");
    button_sprites = load_spritesheet("data/gui1-64.dat", "data/gui1-64.tab");
    gui_panel_sprites = load_spritesheet("data/gui2-64.dat", "data/gui2-64.tab");
    if (!winfont || !font_sprites || !button_sprites || !gui_panel_sprites || LbDataLoadAll(gui_load_files_640)) {
        return 0;
    }
    return 1;
}

void FreeVRes256Data(void)
{
    free_font(&winfont);
    free_font(&font_sprites);
    free_spritesheet(&button_sprites);
    free_spritesheet(&gui_panel_sprites);
    LbDataFreeAll(gui_load_files_640);
}

short LoadVResMinimal(void)
{
    button_sprites = load_spritesheet("data/gui1-32.dat", "data/gui1-32.tab");
#ifdef SPRITE_FORMAT_V2
    frontend_font[0] = load_font("ldata/frontft1-64.dat", "ldata/frontft1-64.tab");
    frontend_font[1] = load_font("ldata/frontft2-64.dat", "ldata/frontft2-64.tab");
    frontend_font[2] = load_font("ldata/frontft3-64.dat", "ldata/frontft3-64.tab");
    frontend_font[3] = load_font("ldata/frontft4-64.dat", "ldata/frontft4-64.tab");
#else
    frontend_font[0] = load_font("ldata/frontft1.dat", "ldata/frontft1.tab");
    frontend_font[1] = load_font("ldata/frontft2.dat", "ldata/frontft2.tab");
    frontend_font[2] = load_font("ldata/frontft3.dat", "ldata/frontft3.tab");
    frontend_font[3] = load_font("ldata/frontft4.dat", "ldata/frontft4.tab");
#endif
    return button_sprites && frontend_font[0] && frontend_font[1] && frontend_font[2] &&
        frontend_font[3] && LbDataLoadAll(front_load_files_minimal_640) == 0;
}

void FreeVResMinimal(void)
{
    for (int i = 0; i < FRONTEND_FONTS_COUNT; ++i) {
        free_font(&frontend_font[i]);
    }
    free_spritesheet(&button_sprites);
    LbDataFreeAll(front_load_files_minimal_640);
}

/**
 * Loads MCGA graphics files, for low resolution mode.
 * It is modified version of LbDataLoadAll, optimized for maximum
 * game speed on very slow machines.
 * @return Returns true if all files were loaded, false otherwise.
 */
short LoadMcgaData(void)
{
    int ferror = 0;
    int i = 0;
    struct TbLoadFiles* t_lfile = &gui_load_files_320[i];
    while (t_lfile->Start != NULL)
    {
        // Don't allow loading flags
        t_lfile->Flags = 0;
        int ret_val = LbDataLoad(t_lfile, NULL, NULL);
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
        t_lfile = &gui_load_files_320[i];
  }
  button_sprites = load_spritesheet("data/gui1-32.dat", "data/gui1-32.tab");
  winfont = load_font("data/font2-32.dat", "data/font2-32.tab");
  font_sprites = load_font("data/font1-32.dat", "data/font1-32.tab");
  gui_panel_sprites = load_spritesheet("data/gui2-32.dat", "data/gui2-32.tab");
  return button_sprites && winfont && font_sprites && gui_panel_sprites && (ferror == 0);
}

void FreeMcgaData(void)
{
    LbDataFreeAll(gui_load_files_320);
    free_font(&winfont);
    free_font(&font_sprites);
    free_spritesheet(&button_sprites);
    free_spritesheet(&gui_panel_sprites);
}

void set_game_vidmode(uint i, TbScreenMode nmode)
{
  switching_vidmodes[i%MAX_GAME_VIDMODE_COUNT]=nmode;
}

TbScreenMode get_game_vidmode(uint i)
{
  return switching_vidmodes[i%MAX_GAME_VIDMODE_COUNT];
}

TbScreenMode try_failsafe_vidmode(void)
{
  // Check the failsafe mode
  if (!LbScreenIsModeAvailable(failsafe_vidmode, display_id))
  {
      ERRORLOG("Failsafe video mode (mode %d) is invalid.",(int)failsafe_vidmode);
      return Lb_SCREEN_MODE_INVALID;
  }
  return failsafe_vidmode;
}

TbScreenMode get_failsafe_vidmode(void)
{
  return failsafe_vidmode;
}

void set_failsafe_vidmode(TbScreenMode nmode)
{
  failsafe_vidmode=nmode;
}

TbScreenMode get_movies_vidmode(void)
{
  return movies_vidmode;
}

void set_movies_vidmode(TbScreenMode nmode)
{
  movies_vidmode=nmode;
}

TbScreenMode get_frontend_vidmode(void)
{
  return frontend_vidmode;
}

void set_frontend_vidmode(TbScreenMode nmode)
{
  frontend_vidmode=nmode;
}

void load_pointer_file(short hi_res)
{
#ifdef SPRITE_FORMAT_V2
    pointer_sprites = load_spritesheet("data/pointer-64.dat", "data/pointer-64.tab");
#else
    pointer_sprites = load_spritesheet("data/pointer64.dat", "data/pointer64.tab");
#endif
    if (!pointer_sprites) ERRORLOG("Unable to load pointer sprites");
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
  LbMouseChangeSpriteAndHotspot(get_frontend_sprite(GFS_cursor_horny), 0, 0);
  return true;
}

TbBool set_pointer_graphic_spell(long spridx, long frame)
{
    long i;
    long x;
    long y;
    SYNCDBG(8, "Setting to sprite %d", (int)spridx);
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
    i = spridx + (frame%8);
  } else
  {
    y = 78;
    x = 26;
    i = spridx;
  }
  const struct TbSprite* spr = NULL;

  if (is_custom_icon(i))
  {
      spr = get_new_icon_sprite(i);
      SYNCDBG(8,"Activating pointer %ld", i);
      LbMouseChangeSpriteAndHotspot(spr, x/2, y/2);
  }
  else
  {
      SYNCDBG(8,"Activating pointer %ld", 40+i);
      if (i >= 0 && i < num_sprites(pointer_sprites))
      {
          spr = get_sprite(pointer_sprites, i);
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
  case MousePG_Pickaxe2:
    ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
      x = 12; y = 15;
      break;
  case MousePG_Sell:
      ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
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
      ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
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
      ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
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
      ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
      x = 12; y = 38;
      break;
  case  MousePG_LockMark:
  // 40..144 are spell pointers
  case  MousePG_Unkn47:
      ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
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
      ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
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
      ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
      x = 12; y = 38;
      break;
  default:
      ptr_idx = get_player_colored_pointer_icon_idx(ptr_idx,my_player_number);
      spr = get_new_icon_sprite(ptr_idx);
      if (spr != NULL)
      {
          LbMouseChangeSpriteAndHotspot(spr, spr->SWidth/2, spr->SHeight);
          return true;
      }
    WARNLOG("Unrecognized Mouse Pointer index, %ld",ptr_idx);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  if (ptr_idx >= 0 && ptr_idx < num_sprites(pointer_sprites)) {
    spr = get_sprite(pointer_sprites, ptr_idx);
    LbMouseChangeSpriteAndHotspot(spr, x, y);
  } else {
    WARNLOG("Sprite %d exceeds buffer, setting pointer to none",(int)ptr_idx);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  }
  return true;
}

void unload_pointer_file(short hi_res)
{
    set_pointer_graphic_none();
    free_spritesheet(&pointer_sprites);
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

/**
 * Set up a new screen mode suitable for playing the game.
 *
 * @param nmode The mode (index number) that we want to change to.
 * @param falisafe If TRUE the the failsafe resolution will be used if nmode is not available
 * @return Returns the mode that the screen was setup successfully with (or Lb_SCREEN_MODE_INVALID/false when the screen was not setup successfully).
 */
TbScreenMode setup_screen_mode(TbScreenMode nmode, TbBool failsafe)
{
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
  TbScreenMode old_mode = LbScreenActiveMode();
  TbScreenModeInfo* old_mdinfo = LbScreenGetModeInfo(old_mode);
  // we don't want to get the current display when using the "fill all" mode, we want to keep the old version
  if (!(old_mdinfo->VideoFlags & Lb_VF_FILLALL))
  {
    display_id = LbGetCurrentDisplayIndex(); // get current display
  }
  // Check that the desired mode is available for the current display
  if (!LbScreenIsModeAvailable(nmode, display_id))
  {
    if (failsafe)
    {
      ERRORLOG("Unable to setup screen resolution %s (mode %d), trying failsafe mode", new_mdinfo->Desc,(int)nmode);
      nmode = try_failsafe_vidmode();
      if (nmode == Lb_SCREEN_MODE_INVALID)
      {
        force_video_mode_reset = true;
        return Lb_SCREEN_MODE_INVALID;
      }
    }
    else
    {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
      return Lb_SCREEN_MODE_INVALID;
    }
    new_mdinfo = LbScreenGetModeInfo(nmode);
  }
  if (!force_video_mode_reset)
  {
    if ((nmode == old_mode) && (!MinimalResolutionSetup))
    {
      SYNCDBG(6,"Mode %d already active, no changes.",(int)nmode);
      return nmode;
    }
  }
  TbBool hi_res = ((LbGraphicsScreenHeight() < 400) ? false : true);
  long lens_mem = game.applied_lens_type;
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
      unload_pointer_file(hi_res);
    }
    if (nmode != old_mode)
        LbScreenReset(false);
    if (MinimalResolutionSetup) {
      if (hi_res) {
        FreeVResMinimal();
      }
    } else {
      if (hi_res) {
        FreeVRes256Data();
      } else {
        FreeMcgaData();
      }
    }
    if (!hi_res) ERRORLOG("MCGA Minimal not allowed (Reset)");
    MinimalResolutionSetup = false;
  }
  hi_res = ((new_mdinfo->Height < 400) ? false : true);
  if (new_mdinfo->Height < 200)
  {
      ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
      force_video_mode_reset = true;
      return Lb_SCREEN_MODE_INVALID;
  } else
  {
    SYNCDBG(6,"Entering %s mode %d, resolution %dx%d.",hi_res?"hi-res":"low-res",(int)nmode,(int)new_mdinfo->Width,(int)new_mdinfo->Height);
    if (hi_res)
    {
      if (!LoadVRes256Data((long)new_mdinfo->Width*(long)new_mdinfo->Height))
      {
        ERRORLOG("Unable to load VRes256 data files");
        force_video_mode_reset = true;
        return Lb_SCREEN_MODE_INVALID;
      }
    }
    else
    {
      if (!LoadMcgaData())
      {
        ERRORLOG("Loading Mcga files failed");
        force_video_mode_reset = true;
        return Lb_SCREEN_MODE_INVALID;
      }
    }
    if ((nmode != old_mode) || (was_minimal_res))
    {
        if (LbScreenSetup(nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, (hi_res ? 1 : 2), 0) < Lb_SUCCESS)
        {
          ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
          force_video_mode_reset = true;
          return Lb_SCREEN_MODE_INVALID;
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
  setup_heap_manager();
  force_video_mode_reset = false;
  SYNCDBG(8,"Finished");
  return nmode;
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


  // Adjust scaling factor (units per pixel) based on window resolution compared to the original DK resolutions
  // low-res - units per pixel = 8, low-res - units per pixel = 16 (or upp min is low-res = 4, high-res = 10)

  // In-game scaling (DK original: low-res - 320x200, high-res - 640x400)
  units_per_pixel = (width>height?width:height)/40;// originally was 16 for hires, 8 for lores
  units_per_pixel_min = (width>height?height:width)/40;// originally 10 for hires
  units_per_pixel_width = width/40; // 8 for low res, 16 is "kfx default"
  units_per_pixel_height = height/25; // 8 for low res, 16 is "kfx default"
  units_per_pixel_best = ((is_ar_wider_than_original(width, height)) ? units_per_pixel_height : units_per_pixel_width); // If the screen is wider than 16:10 the height is used; if the screen is narrower than 16:10 the width is used.

  // In-game scaling: UI (for the side bar menu and escape menu)
  long ui_scale = UI_NORMAL_SIZE; // UI_NORMAL_SIZE, UI_HALF_SIZE, or UI_DOUBLE_SIZE (not fully implemented yet)
  units_per_pixel_ui = resize_ui(units_per_pixel_best, ui_scale);

  // In-game scaling: Posession Mode (a 3D 1st person perspective camera)
  calculate_aspect_ratio_factor(width, height);
  first_person_vertical_fov = DEFAULT_FIRST_PERSON_VERTICAL_FOV;
  first_person_horizontal_fov = FOV_based_on_aspect_ratio();

  // Main menu scaling (DK original: 640x480)
  units_per_pixel_menu_height = height/30; // 16 is "kfx default" (640x480)
  units_per_pixel_menu = ((is_menu_ar_wider_than_original(width, height)) ? units_per_pixel_menu_height : units_per_pixel_width); // If the screen is wider than 4:3 the height is used; if the screen is narrower than 4:3 the width is used.

  // Main menu scaling: Campaign map "land view" screen (including the window frame)
  calculate_landview_upp(width, height, LANDVIEW_MAP_WIDTH, LANDVIEW_MAP_HEIGHT); // 16 is "kfx default" for 640x480 game window (1x), a 960x720 frame (1.5x), and a 1280x960 landview (2x)

  LbMouseChangeMoveRatio(base_mouse_sensitivity*units_per_pixel/16, base_mouse_sensitivity*units_per_pixel/16);
  LbMouseSetPointerHotspot(0, 0);
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbMouseSetup(NULL);
  return true;
}

/**
 * Set up a new screen mode suitable for the frontend or movie playback.
 *
 * @param nmode The mode (index number) that we want to change to.
 * @return Returns the mode that the screen was setup successfully with (or Lb_SCREEN_MODE_INVALID/false when the screen was not setup successfully).
 */
TbScreenMode setup_screen_mode_minimal(TbScreenMode nmode)
{
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
  // we don't want to get the current display when using the "fill all" mode, we want to keep the old version
  TbScreenMode old_mode = LbScreenActiveMode();
  TbScreenModeInfo* old_mdinfo = LbScreenGetModeInfo(old_mode);
  if (!(old_mdinfo->VideoFlags & Lb_VF_FILLALL))
  {
    display_id = LbGetCurrentDisplayIndex(); // get current display
  }
  // Check that the desired mode is available for the current display
  if (!LbScreenIsModeAvailable(nmode, display_id))
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d), trying failsafe mode", new_mdinfo->Desc,(int)nmode);
      nmode = try_failsafe_vidmode();
      if (nmode == Lb_SCREEN_MODE_INVALID)
      {
        force_video_mode_reset = true;
        return Lb_SCREEN_MODE_INVALID;
      }
      new_mdinfo = LbScreenGetModeInfo(nmode);
  }
  if (!force_video_mode_reset)
  {
    if ((nmode == old_mode) && (MinimalResolutionSetup))
    {
      SYNCDBG(6,"Mode %d already active, no changes.",(int)nmode);
      return nmode;
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
    }
    if ((!MinimalResolutionSetup && !hi_res) || (MinimalResolutionSetup && hi_res))
      unload_pointer_file(hi_res);
    if ((nmode != old_mode) || (force_video_mode_reset))
      LbScreenReset(false);
    if (hi_res)
    {
      if (MinimalResolutionSetup) {
        FreeVResMinimal();
      } else {
        FreeVRes256Data();
      }
    }
    else
    {
      if (!MinimalResolutionSetup) FreeMcgaData();
    }
    MinimalResolutionSetup = false;
  }
  hi_res = ((new_mdinfo->Height < 400) ? false : true);
  if (new_mdinfo->Height < 200)
  {
      ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
      force_video_mode_reset = true;
      return Lb_SCREEN_MODE_INVALID;
  } else
  {
    SYNCDBG(17,"Preparing minimal %s resolution mode",(hi_res ? "high" : "low"));
    MinimalResolutionSetup = true;
    if (hi_res)
    {
      frontend_load_data_from_cd();
      if (!LoadVResMinimal())
      {
        ERRORLOG("Unable to load VRes256 front_load minimal files");
        force_video_mode_reset = true;
        return Lb_SCREEN_MODE_INVALID;
      }
      frontend_load_data_reset();
    }

    if ((nmode != old_mode) || (force_video_mode_reset))
    {
        if (LbScreenSetup(nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, (hi_res ? 1 : 2), 0) < Lb_SUCCESS)
        {
          ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
          force_video_mode_reset = true;
          return Lb_SCREEN_MODE_INVALID;
        }
    }
  }
  LbScreenClear(0);
  LbScreenSwap();
  update_screen_mode_data(new_mdinfo->Width, new_mdinfo->Height);
  lbDisplay.DrawFlags = flg_mem;
  force_video_mode_reset = false;
  return nmode;
}

/**
 * Set up a new screen mode with a blank black screen.
 *
 * @param nmode The mode (index number) that we want to change to.
 * @return Returns the mode that the screen was setup successfully with (or Lb_SCREEN_MODE_INVALID/false when the screen was not setup successfully).
 */
TbScreenMode setup_screen_mode_zero(TbScreenMode nmode)
{
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  TbScreenModeInfo* new_mdinfo = LbScreenGetModeInfo(nmode);
  // we don't want to get the current display when using the "fill all" mode, we want to keep the old version
  TbScreenMode old_mode = LbScreenActiveMode();
  TbScreenModeInfo* old_mdinfo = LbScreenGetModeInfo(old_mode);
  if (!(old_mdinfo->VideoFlags & Lb_VF_FILLALL))
  {
    display_id = LbGetCurrentDisplayIndex(); // get current display
  }
  // Check that the desired mode is available for the current display
  if (!LbScreenIsModeAvailable(nmode, display_id))
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d), trying failsafe mode", new_mdinfo->Desc,(int)nmode);
      nmode = try_failsafe_vidmode();
      if (nmode == Lb_SCREEN_MODE_INVALID)
      {
        return Lb_SCREEN_MODE_INVALID;
      }
      new_mdinfo = LbScreenGetModeInfo(nmode);
  }
  LbPaletteDataFillBlack(engine_palette);
  if (LbScreenSetup(nmode, new_mdinfo->Width, new_mdinfo->Height, engine_palette, 2, 0) < Lb_SUCCESS)
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)", new_mdinfo->Desc,(int)nmode);
      return Lb_SCREEN_MODE_INVALID;
  }
  update_screen_mode_data(new_mdinfo->Width, new_mdinfo->Height);
  force_video_mode_reset = true;
  return nmode;
}

/**
 * Set up a the screen using the mode saved in settings (video_scrnmode).
 *
 * @return Returns the mode that the screen was setup successfully with (or Lb_SCREEN_MODE_INVALID/false when the screen was not setup successfully).
 */
TbScreenMode reenter_video_mode(void)
{
  TbScreenMode scrmode = get_game_vidmode(settings.switching_vidmodes_index);
  scrmode = setup_screen_mode(scrmode, false);
  if (scrmode == Lb_SCREEN_MODE_INVALID)
  {
    // try all of the other switchable video modes before eventually trying the failsafe
    if (!switch_to_next_video_mode())
    {
      return Lb_SCREEN_MODE_INVALID;
    }
  }
  else
  {
    SYNCLOG("set in-game video as %s (mode %d)", get_vidmode_name(scrmode),(int)scrmode);
  }
  return scrmode;
}

/**
 * Switch to the next mode in the list set by the INGAME_RES config setting (these are stored in switching_vidmodes[]).
 *
 * @return Returns the mode that the screen was setup successfully with (or Lb_SCREEN_MODE_INVALID/false when the screen was not setup successfully).
 */
TbBool switch_to_next_video_mode(void)
{
  uint current = settings.switching_vidmodes_index;
  uint i = current;
  TbBool failsafe = false;
  TbScreenMode scrmode;
  do
  {
    if ((features_enabled & Ft_HiResVideo) == 0)
    {
      // Do not allow user enter higher modes on low memory systems
      scrmode = setup_screen_mode(Lb_SCREEN_MODE_320_200_8, true);
      failsafe = ((scrmode == Lb_SCREEN_MODE_320_200_8) ? false : true);
      break;
    }
    i++;
    if (i>=MAX_GAME_VIDMODE_COUNT)
    {
      i=0;
    }
    if (i == current)
    {
      // we have done a full loop of switching_vidmodes[]
      if (get_game_vidmode(i) == LbScreenActiveMode())
      {
        SYNCLOG("No new mode to switch to; staying with %s (mode %d).", get_vidmode_name(scrmode),(int)scrmode);
        return true; // only 1 valid video mode, and we are already in it
      }
      // else there are no valid modes in the array, try the failsafe
      scrmode = setup_screen_mode(get_failsafe_vidmode(), false);
      failsafe = true;
      break;
    }
    scrmode = get_game_vidmode(i);
    if (scrmode != Lb_SCREEN_MODE_INVALID)
    {
      // try the next vidmode in switching_vidmodes[]
      scrmode = setup_screen_mode(scrmode, false);
    }
  } while (scrmode == Lb_SCREEN_MODE_INVALID);

  if (scrmode > Lb_SCREEN_MODE_INVALID)
  {
    if (failsafe)
    {
      show_onscreen_msg(game_num_fps * 6, "%s", get_string(856));
    }
    else
    {
      // we managed to switch to a new mode
      show_onscreen_msg(game_num_fps * 6, "%s", get_vidmode_name(scrmode));
      settings.switching_vidmodes_index = i;
      save_settings();
    }
  }
  else
  {
    FatalError = 1;
    exit_keeper = 1;
    return false;
  }
  SYNCLOG("Switched video to %s (mode %d)", get_vidmode_name(scrmode),(int)scrmode);
  return true;
}

/** Needed until its contents are refactored, then we can just call switch_to_next_video_mode from PckA_SwitchScrnRes. */
void switch_to_next_video_mode_wrapper(void)
{
  char percent_x = ((float)lbDisplay.MMouseX / (float)(lbDisplay.MouseWindowX + lbDisplay.MouseWindowWidth)) * 100;
  char percent_y = ((float)lbDisplay.MMouseY / (float)(lbDisplay.MouseWindowY + lbDisplay.MouseWindowHeight)) * 100;

  if (switch_to_next_video_mode() == Lb_SCREEN_MODE_INVALID)
  {
    return;
  }

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
  return;
}

#if (BFDEBUG_LEVEL > 0)
TbBool load_testfont_fonts(void)
{
    testfont[0] = load_font("ldata/frontft1.dat", "ldata/frontft1.tab");
    testfont[1] = load_font("ldata/frontft2.dat", "ldata/frontft2.tab");
    testfont[2] = load_font("ldata/frontft3.dat", "ldata/frontft3.tab");
    testfont[3] = load_font("ldata/frontft4.dat", "ldata/frontft4.tab");
    testfont[4] = load_font("data/font0-0.dat", "data/font-0-0.tab");
    testfont[5] = load_font("data/font0-1.dat", "data/font-0-1.tab");
    testfont[6] = load_font("data/font2-32.dat", "data/font2-32.tab");
    testfont[7] = load_font("data/font2-64.dat", "data/font2-64.tab");
    testfont[8] = load_font("data/font1-64.dat", "data/font1-64.tab");
    testfont[9] = load_font("data/font1-32.dat", "data/font1-32.tab");
    testfont[10] = load_font("ldata/netfont.dat", "ldata/netfont.tab");
    for (int i = 0; i < TESTFONTS_COUNT; ++i) {
        if (!testfont[i]) {
            ERRORLOG("Unable to load test font %d", i);
            return false;
        }
    }
    if (!LbDataLoadAll(testfont_load_files) )
    {
      ERRORLOG("Unable to load testfont_load_files files");
      return false;
    }
    return true;
}

void free_testfont_fonts(void)
{
    for (int i = 0; i < TESTFONTS_COUNT; ++i) {
        free_font(&testfont[i]);
    }
    LbDataFreeAll(testfont_load_files);
}
#endif

/******************************************************************************/
#ifdef __cplusplus
}
#endif
