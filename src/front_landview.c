/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_landview.c
 *     Land view, where the user can select map for campaign or multiplayer.
 * @par Purpose:
 *     Functions for displaying and maintaininfg the land view.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     16 Mar 2009 - 01 Apr 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "front_landview.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_datetm.h"
#include "bflib_video.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_filelst.h"
#include "bflib_sprite.h"
#include "bflib_mouse.h"
#include "bflib_sndlib.h"
#include "bflib_sound.h"
#include "bflib_vidraw.h"

#include "config.h"
#include "front_simple.h"
#include "frontend.h"
#include "kjm_input.h"

#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
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

LevelNumber mouse_over_lvnum;
LevelNumber playing_speech_lvnum;
/******************************************************************************/
DLLIMPORT long _DK_frontnetmap_update(void);
DLLIMPORT void _DK_frontnetmap_draw(void);
DLLIMPORT void _DK_frontnetmap_input(void);
DLLIMPORT void _DK_frontnetmap_unload(void);
DLLIMPORT void _DK_frontnetmap_load(void);
DLLIMPORT long _DK_frontmap_update(void);
DLLIMPORT void _DK_frontmap_draw(void);
DLLIMPORT void _DK_frontmap_input(void);
DLLIMPORT void _DK_frontmap_unload(void);
DLLIMPORT int _DK_frontmap_load(void);
DLLIMPORT long _DK_load_map_and_window(unsigned long lv_num);
DLLIMPORT void _DK_frontzoom_to_point(long a1, long a2, long a3);
DLLIMPORT void _DK_compressed_window_draw(void);
DLLIMPORT void _DK_check_mouse_scroll(void);
DLLIMPORT void _DK_update_velocity(void);
/******************************************************************************/
void draw_map_screen(void)
{
  static const char *func_name="draw_map_screen";
  unsigned char *dstbuf;
  unsigned char *srcbuf;
  long i;
  long w;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  dstbuf = lbDisplay.WScreen;
  w = lbDisplay.PhysicalScreenWidth;
  if (map_info.field_16+w > MAP_SCREEN_WIDTH)
    w = MAP_SCREEN_WIDTH-map_info.field_16;
  srcbuf = &map_screen[MAP_SCREEN_WIDTH*map_info.field_1A + map_info.field_16];
  for (i=0; i < lbDisplay.PhysicalScreenHeight; i++)
  {
    if (map_info.field_1A+i >= MAP_SCREEN_HEIGHT)
      break;
    memcpy(dstbuf, srcbuf, w);
    srcbuf += MAP_SCREEN_WIDTH;
    dstbuf += lbDisplay.GraphicsScreenWidth;
  }
}

short is_over_ensign(struct LevelInformation *lvinfo, long scr_x, long scr_y)
{
  long map_x,map_y,spr_w,spr_h;
  map_x = map_info.field_16+scr_x;
  map_y = map_info.field_1A+scr_y;
  spr_w = map_flag[10].SWidth;
  spr_h = map_flag[10].SHeight;
  if ((map_x >= lvinfo->ensign_x-(spr_w>>1)) && (map_x < lvinfo->ensign_x+(spr_w>>1))
   && (map_y > lvinfo->ensign_y-spr_h) && (map_y < lvinfo->ensign_y-(spr_h/3)))
      return true;
  return false;
}

short is_ensign_in_screen_rect(struct LevelInformation *lvinfo)
{
  if ((lvinfo->ensign_zoom_x >= map_info.field_16) && (lvinfo->ensign_zoom_x < map_info.field_16+lbDisplay.PhysicalScreenWidth))
    if ((lvinfo->ensign_zoom_y >= map_info.field_1A) && (lvinfo->ensign_zoom_y < map_info.field_1A+lbDisplay.PhysicalScreenHeight))
      return true;
  return false;
}

/*
 * Chanes state of all land map ensigns.
 */
void set_all_ensigns_state(unsigned short nstate)
{
  static const char *func_name="set_all_ensigns_state";
  struct LevelInformation *lvinfo;
  lvinfo = get_first_level_info();
  while (lvinfo != NULL)
  {
    lvinfo->state = nstate;
    lvinfo = get_next_level_info(lvinfo);
  }
}

void update_ensigns_visibility(void)
{
  static const char *func_name="update_ensigns_visibility";
  struct LevelInformation *lvinfo;
  struct PlayerInfo *player;
  long lvnum,bn_lvnum;
  short show_all_sp;
  int i,k;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  set_all_ensigns_state(LvSt_Hidden);
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  show_all_sp = false;
  lvnum = get_continue_level_number();
  if (lvnum > 0)
  {
    lvinfo = get_level_info(lvnum);
    if (lvinfo != NULL)
      lvinfo->state = LvSt_Visible;
  } else
  if (lvnum == SINGLEPLAYER_FINISHED)
  {
    show_all_sp = true;
  }
  lvnum = first_singleplayer_level();
  while (lvnum > 0)
  {
    if (show_all_sp)
    {
      lvinfo = get_level_info(lvnum);
      if (lvinfo != NULL)
        lvinfo->state = LvSt_Visible;
    }
    bn_lvnum = bonus_level_for_singleplayer_level(lvnum);
    if (is_bonus_level_visible(player, bn_lvnum))
    {
      lvinfo = get_level_info(bn_lvnum);
      if (lvinfo != NULL)
        lvinfo->state = LvSt_Visible;
    }
    lvnum = next_singleplayer_level(lvnum);
  }
  lvnum = get_extra_level(ExLv_FullMoon);
  lvinfo = get_level_info(lvnum);
  if (lvinfo != NULL)
    lvinfo->state = get_extra_level_kind_visibility(ExLv_FullMoon);
  lvnum = get_extra_level(ExLv_NewMoon);
  lvinfo = get_level_info(lvnum);
  if (lvinfo != NULL)
    lvinfo->state = get_extra_level_kind_visibility(ExLv_NewMoon);
}

int compute_sound_good_to_bad_factor(void)
{
  static const char *func_name="compute_sound_good_to_bad_factor";
  struct LevelInformation *lvinfo;
  unsigned int onscr_bad,onscr_good;
  LevelNumber sp_lvnum,continue_lvnum;
  short lv_beaten;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  onscr_bad = 0;
  onscr_good = 0;
  continue_lvnum = get_continue_level_number();
  lv_beaten = (continue_lvnum != SINGLEPLAYER_NOTSTARTED);
  sp_lvnum = first_singleplayer_level();
  while (sp_lvnum > 0)
  {
    if (sp_lvnum == continue_lvnum)
      lv_beaten = false;
    lvinfo = get_level_info(sp_lvnum);
    if (is_ensign_in_screen_rect(lvinfo))
    {
      if (lv_beaten)
        onscr_bad++;
      else
        onscr_good++;
    }
    sp_lvnum = next_singleplayer_level(sp_lvnum);
  }
  if ((onscr_bad+onscr_good) == 0)
    onscr_good++;
  return (127*onscr_good)/(onscr_bad+onscr_good);
}

void update_frontmap_ambient_sound(void)
{
  static const char *func_name="update_frontmap_ambient_sound";
  long lvidx;
  long i;
  if (map_sound_fade)
  {
    lvidx = array_index_for_singleplayer_level(get_continue_level_number());
    if ((features_enabled & Ft_AdvAmbSonud) != 0)
    {
      i = compute_sound_good_to_bad_factor();
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Volume factor is %ld\n",func_name,i);
#endif
      SetSampleVolume(0, campaign.ambient_good, map_sound_fade*(i)/256, 0);
      SetSampleVolume(0, campaign.ambient_bad, map_sound_fade*(127-i)/256, 0);
    } else
    if (lvidx > 13)
    {
      SetSampleVolume(0, campaign.ambient_bad, 127*map_sound_fade/256, 0);
    } else
    {
      SetSampleVolume(0, campaign.ambient_good, 127*map_sound_fade/256, 0);
    }
    SetStreamedSampleVolume(127*map_sound_fade/256);
    if ( !(game.flags_cd & 0x10) )
      SetRedbookVolume(map_sound_fade*settings.redbook_volume/256);
  } else
  {
    if ((features_enabled & Ft_AdvAmbSonud) != 0)
    {
      SetSampleVolume(0, campaign.ambient_good, 0, 0);
      SetSampleVolume(0, campaign.ambient_bad, 0, 0);
    }
    if ((game.flags_cd & 0x10) == 0)
      SetRedbookVolume(0);
    SetStreamedSampleVolume(0);
  }
}

struct TbSprite *get_ensign_sprite_for_level(struct LevelInformation *lvinfo, int anim_frame)
{
  static const char *func_name="get_ensign_sprite_for_level";
  struct TbSprite *spr;
  int i;
  if (lvinfo == NULL)
    return NULL;
  if (lvinfo->state == LvSt_Hidden)
    return NULL;
  if (lvinfo->options & LvOp_IsSingle)
  {
    switch (lvinfo->state)
    {
    case LvSt_Visible:
        if (lvinfo->lvnum > 5)
          i = 10; // full red flag
        else
          i = 2; // 'T' flag - tutorial
        if (lvinfo->lvnum == mouse_over_lvnum)
          i += 4;
        spr = &map_flag[i+(anim_frame & 3)];
        break;
    default:
        spr = &map_flag[10];
        break;
    }
  } else
  if (lvinfo->options & LvOp_IsBonus)
  {
    switch (lvinfo->state)
    {
    case LvSt_Visible:
        i = 18;
        if (lvinfo->lvnum == mouse_over_lvnum)
          i += 4;
        spr = &map_flag[i+(anim_frame & 3)];
        break;
    default:
        spr = &map_flag[18];
        break;
    }
  } else
  if (lvinfo->options & LvOp_IsExtra)
  {
    switch (lvinfo->state)
    {
    case LvSt_Visible:
        i = 26;
        if (lvinfo->lvnum == mouse_over_lvnum)
          i += 4;
        spr = &map_flag[i+(anim_frame & 3)];
        break;
    default:
        spr = &map_flag[34];
        break;
    }
  } else
  if (lvinfo->options & LvOp_IsMulti) //TODO: multiplayer flags should be treated differently
  {
    spr = &map_flag[34];
  } else
  {
    spr = &map_flag[34];
  }
  if (spr < end_map_flag)
    return spr;
  return NULL;
}

/*
 * Draws the visible level ensigns on screen.
 * Note that the drawing is in reverse order than the one of reading inputs.
 */
void draw_map_level_ensigns(void)
{
  static const char *func_name="draw_map_level_ensigns";
  struct LevelInformation *lvinfo;
  struct TbSprite *spr;
  long x,y;
  int k;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  k = LbTimerClock()/200;
  lvinfo = get_last_level_info();
  while (lvinfo != NULL)
  {
    // the flag sprite
    spr = get_ensign_sprite_for_level(lvinfo,k);
    if (spr != NULL)
    {
      x = lvinfo->ensign_x - map_info.field_16 - (spr->SWidth>>1);
      y = lvinfo->ensign_y - map_info.field_1A -  spr->SHeight;
      LbSpriteDraw(x, y, spr);
    }
    lvinfo = get_prev_level_info(lvinfo);
  }
}

void set_map_info_visible_hotspot(long map_x,long map_y)
{
  map_info.field_2E = map_x - lbDisplay.PhysicalScreenWidth/2;
  map_info.field_32 = map_y - lbDisplay.PhysicalScreenHeight/2;
  if (map_info.field_2E > MAP_SCREEN_WIDTH-lbDisplay.PhysicalScreenWidth)
    map_info.field_2E = MAP_SCREEN_WIDTH-lbDisplay.PhysicalScreenWidth;
  if (map_info.field_2E < 0)
    map_info.field_2E = 0;
  if (map_info.field_32 > MAP_SCREEN_HEIGHT-lbDisplay.PhysicalScreenHeight)
    map_info.field_32 = MAP_SCREEN_HEIGHT-lbDisplay.PhysicalScreenHeight;
  if (map_info.field_32 < 0)
    map_info.field_32 = 0;
}

void set_map_info_draw_hotspot(long map_x,long map_y)
{
  map_info.field_16 = map_x - lbDisplay.PhysicalScreenWidth/2;
  map_info.field_1A = map_y - lbDisplay.PhysicalScreenHeight/2;
  if (map_info.field_16 > MAP_SCREEN_WIDTH-lbDisplay.PhysicalScreenWidth)
    map_info.field_16 = MAP_SCREEN_WIDTH-lbDisplay.PhysicalScreenWidth;
  if (map_info.field_16 < 0)
    map_info.field_16 = 0;
  if (map_info.field_1A > MAP_SCREEN_HEIGHT-lbDisplay.PhysicalScreenHeight)
    map_info.field_1A = MAP_SCREEN_HEIGHT-lbDisplay.PhysicalScreenHeight;
  if (map_info.field_1A < 0)
    map_info.field_1A = 0;
}

void set_map_info_draw_hotspot_raw(long map_x,long map_y)
{
  map_info.field_16 = map_x;
  map_info.field_1A = map_y;
  if (map_info.field_16 > MAP_SCREEN_WIDTH-lbDisplay.PhysicalScreenWidth)
    map_info.field_16 = MAP_SCREEN_WIDTH-lbDisplay.PhysicalScreenWidth;
  if (map_info.field_16 < 0)
    map_info.field_16 = 0;
  if (map_info.field_1A > MAP_SCREEN_HEIGHT-lbDisplay.PhysicalScreenHeight)
    map_info.field_1A = MAP_SCREEN_HEIGHT-lbDisplay.PhysicalScreenHeight;
  if (map_info.field_1A < 0)
    map_info.field_1A = 0;
}

void update_frontmap_info_draw_hotspot(void)
{
  long scr_x,scr_y;
  long i,k;
  scr_x = map_info.field_2E - map_info.field_16;
  scr_y = map_info.field_32 - map_info.field_1A;
  if ((scr_x != 0) || (scr_y != 0))
  {
      i = abs(scr_y);
      k = abs(scr_x);
      if ( i <= k )
        i = k;
      if (i > 59)
        i = 59;
      if (i < 0)
        i = 0;
      if (i <= 1)
      {
        set_map_info_draw_hotspot_raw(map_info.field_2E,map_info.field_32);
      } else
      {
        map_info.field_1E += (scr_x << 8) / i;
        map_info.field_22 += (scr_y << 8) / i;
        set_map_info_draw_hotspot_raw(map_info.field_1E >> 8, map_info.field_22 >> 8);
      }
  }
}

short frontmap_input_active_ensign(long curr_mx, long curr_my)
{
  struct LevelInformation *lvinfo;
  lvinfo = get_first_level_info();
  while (lvinfo != NULL)
  {
    if (lvinfo->state == LvSt_Visible)
      if (is_over_ensign(lvinfo, curr_mx, curr_my))
      {
        mouse_over_lvnum = lvinfo->lvnum;
        return true;
      }
    lvinfo = get_next_level_info(lvinfo);
  }
  mouse_over_lvnum = SINGLEPLAYER_NOTSTARTED;
  return false;
}

short clicked_map_level_ensign(void)
{
  struct LevelInformation *lvinfo;
  lvinfo = get_level_info(mouse_over_lvnum);
  if (lvinfo != NULL)
  {
    set_selected_level_number(lvinfo->lvnum);
    map_info.field_0 = 1;
    map_info.field_1 = 1;
    map_info.field_6 = 1;
    map_info.field_A = lvinfo->ensign_zoom_x;
    map_info.field_E = lvinfo->ensign_zoom_y;
    map_info.field_2 = 4;
    map_info.field_12 = 7;
    set_map_info_visible_hotspot(map_info.field_A, map_info.field_E);
    map_info.field_1E = map_info.field_16 << 8;
    map_info.field_22 = map_info.field_1A << 8;
    return true;
  }
  return false;
}

short stop_description_speech(void)
{
  if ((playing_good_descriptive_speech) || (playing_bad_descriptive_speech))
  {
    playing_good_descriptive_speech = 0;
    playing_bad_descriptive_speech = 0;
    playing_speech_lvnum = SINGLEPLAYER_NOTSTARTED;
    StopStreamedSample();
    return true;
  }
  return false;
}

short play_current_description_speech(short play_good)
{
  LevelNumber lvnum;
  lvnum = get_continue_level_number();
  if (!play_good)
    lvnum = prev_singleplayer_level(lvnum);
  return play_description_speech(lvnum,play_good);
}

short play_description_speech(LevelNumber lvnum, short play_good)
{
  struct LevelInformation *lvinfo;
  char *fname;
  if (playing_speech_lvnum == lvnum)
    return true;
  lvinfo = get_level_info(lvnum);
  if (lvinfo == NULL)
    return false;
  if (play_good)
  {
    if (lvinfo->speech_before[0] == '\0')
      return false;
    stop_description_speech();
    fname = prepare_file_fmtpath(FGrp_AtlSound,"%s.wav",lvinfo->speech_before);
    playing_good_descriptive_speech = 1;
  } else
  {
    if (lvinfo->speech_after[0] == '\0')
      return false;
    stop_description_speech();
    fname = prepare_file_fmtpath(FGrp_AtlSound,"%s.wav",lvinfo->speech_after);
    playing_bad_descriptive_speech = 1;
  }
  playing_speech_lvnum = lvnum;
  SetStreamedSampleVolume(127);
  PlayStreamedSample(fname, 1622, 0, 1);
  return true;
}

void frontzoom_to_point(long map_x, long map_y, long zoom)
{
  unsigned char *dst_buf;
  unsigned char *src_buf;
  unsigned char *dst;
  unsigned char *src;
  long scr_x,scr_y;
  long src_delta,bpos_x,bpos_y;
  long dst_width,dst_height,dst_scanln;
  long x,y;
  //_DK_frontzoom_to_point(map_x, map_y, zoom);
  src_delta = 256 - zoom;
  // Restricting coordinates - to make sure we won't go outside of the buffer
  if (map_x >= (MAP_SCREEN_WIDTH-(lbDisplay.GraphicsScreenWidth>>1)))
    map_x = (MAP_SCREEN_WIDTH-(lbDisplay.GraphicsScreenWidth>>1)-1);
  if (map_x < (lbDisplay.GraphicsScreenWidth>>1))
    map_x = (lbDisplay.GraphicsScreenWidth>>1);
  if (map_y >= (MAP_SCREEN_HEIGHT-(lbDisplay.GraphicsScreenHeight>>1)))
    map_y = (MAP_SCREEN_HEIGHT-(lbDisplay.GraphicsScreenHeight>>1)-1);
  if (map_y < (lbDisplay.GraphicsScreenHeight>>1))
    map_y = (lbDisplay.GraphicsScreenHeight>>1);
  // Initializing variables used for all quadres of screen
  scr_x = map_x - map_info.field_16;
  if (scr_x > lbDisplay.GraphicsScreenWidth) scr_x = lbDisplay.GraphicsScreenWidth;
  if (scr_x < 0) scr_x = 0;
  scr_y = map_y - map_info.field_1A;
  if (scr_y > lbDisplay.GraphicsScreenHeight) scr_y = lbDisplay.GraphicsScreenHeight;
  if (scr_y < 0) scr_y = 0;
  src_buf = &map_screen[MAP_SCREEN_WIDTH * map_y + map_x];
  dst_buf = &lbDisplay.WScreen[lbDisplay.GraphicsScreenWidth * scr_y + scr_x];
  dst_scanln = lbDisplay.GraphicsScreenWidth;
  // Drawing first quadre
  bpos_y = 0;
  dst = dst_buf;
  dst_width = scr_x;
  dst_height = scr_y;
  for (y=0; y <= dst_height; y++)
  {
      bpos_x = 0;
      src = &src_buf[-5*(bpos_y & 0xFFFFFF00)];
      for (x=0; x <= dst_width; x++)
      {
        bpos_x += src_delta;
        dst[-x] = src[-(bpos_x >> 8)];
      }
      dst -= dst_scanln;
      bpos_y += src_delta;
  }
  // Drawing 2nd quadre
  bpos_y = 0;
  dst = dst_buf;
  dst_width = -scr_x + lbDisplay.PhysicalScreenWidth;
  dst_height = scr_y;
  for (y=0; y <= dst_height; y++)
  {
      bpos_x = 0;
      src = &src_buf[-5*(bpos_y & 0xFFFFFF00)];
      for (x=0; x < dst_width; x++)
      {
        bpos_x += src_delta;
        dst[x] = src[(bpos_x >> 8)];
      }
      dst -= dst_scanln;
      bpos_y += src_delta;
  }
  // Drawing 3rd quadre
  bpos_y = 0;
  dst = dst_buf + dst_scanln;
  dst_width = scr_x;
  dst_height = -scr_y + lbDisplay.PhysicalScreenHeight - 1;
  for (y=0; y < dst_height; y++)
  {
      bpos_x = 0;
      src = &src_buf[5*(bpos_y & 0xFFFFFF00)];
      for (x=0; x <= dst_width; x++)
      {
        bpos_x += src_delta;
        dst[-x] = src[-(bpos_x >> 8)];
      }
      dst += dst_scanln;
      bpos_y += src_delta;
  }
  // Drawing 4th quadre
  bpos_y = 0;
  dst = dst_buf + dst_scanln;
  dst_width = -scr_x + lbDisplay.PhysicalScreenWidth;
  dst_height = -scr_y + lbDisplay.PhysicalScreenHeight - 1;
  for (y=0; y < dst_height; y++)
  {
      bpos_x = 0;
      src = &src_buf[5*(bpos_y & 0xFFFFFF00)];
      for (x=0; x < dst_width; x++)
      {
        dst[x] = src[(bpos_x >> 8)];
        bpos_x += src_delta;
      }
      dst += dst_scanln;
      bpos_y += src_delta;
  }
}

void compressed_window_draw(void)
{
  _DK_compressed_window_draw();
}

void unload_map_and_window(void)
{
  clear_light_system();
  clear_things_and_persons_data();
  clear_mapmap();
  clear_computer();
  clear_slabs();
  clear_rooms();
  clear_dungeons();
  LbMemoryCopy(frontend_palette, frontend_backup_palette, PALETTE_SIZE);
}

short load_map_and_window(LevelNumber lvnum)
{
  static const char *func_name="load_map_and_window";
  struct LevelInformation *lvinfo;
  char *land_view;
  char *land_window;
  char *fname;
  long flen;
//  return _DK_load_map_and_window(cmpgidx);
  land_view = NULL;
  land_window = NULL;
  if (lvnum == SINGLEPLAYER_NOTSTARTED)
  {
    land_view = campaign.land_view_start;
    land_window = campaign.land_window_start;
  } else
  if (lvnum == SINGLEPLAYER_FINISHED)
  {
    land_view = campaign.land_view_end;
    land_window = campaign.land_window_end;
  } else
  {
    lvinfo = get_level_info(lvnum);
    if (lvinfo != NULL)
    {
      land_view = lvinfo->land_view;
      land_window = lvinfo->land_window;
    } else
    {
      land_view = campaign.land_view_start;
      land_window = campaign.land_window_start;
    }
  }
  if ((land_view == NULL) || (land_window == NULL))
  {
    LbErrorLog("No land View file names for level %d\n",lvnum);
    return false;
  }
  fname = prepare_file_fmtpath(FGrp_LoData,"%s.raw",land_view);
  flen = LbFileLengthRnc(fname);
  if (flen < 1024)
  {
    error(func_name, 1923, "land Map file doesn't exist or is too small");
    return false;
  }
  if (flen > 1228997)
  {
    error(func_name, 1916, "Not enough memory in game structure for land Map");
    return false;
  }
  if (LbFileLoadAt(fname, &game.land_map_start) != flen)
  {
    error(func_name, 1970, "Unable to load land Map background");
    return false;
  }
  map_screen = &game.land_map_start;
  memcpy(frontend_backup_palette, frontend_palette, PALETTE_SIZE);
  fname = prepare_file_fmtpath(FGrp_LoData,"%s.dat",land_window);
  wait_for_cd_to_be_available();
  if (LbFileLoadAt(fname, block_mem) == -1)
  {
    error(func_name, 2007, "Unable to load Land Map Window");
    unload_map_and_window();
    return false;
  }
  window_y_offset = (long *)block_mem;
  fname = prepare_file_fmtpath(FGrp_LoData,"%s.pal",land_view);
  wait_for_cd_to_be_available();
  if (LbFileLoadAt(fname, frontend_palette) != PALETTE_SIZE)
  {
    error(func_name, 2020, "Unable to load Land Map screen palette");
    unload_map_and_window();
    return 0;
  }
  map_window = (unsigned char *)(window_y_offset + 720);
  return true;
}

void frontnetmap_unload(void)
{
  static const char *func_name="frontnetmap_unload";
  _DK_frontnetmap_unload();
}

void frontnetmap_load(void)
{
  static const char *func_name="frontnetmap_load";
  _DK_frontnetmap_load();
}

int frontmap_load(void)
{
  static const char *func_name="frontmap_load";
  struct PlayerInfo *player;
  struct LevelInformation *lvinfo;
  struct TbSprite *spr;
  LevelNumber lvnum;
  long lvidx;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
//  return _DK_frontmap_load();
  memset(scratch, 0, PALETTE_SIZE);
  LbPaletteSet(scratch);
  play_desc_speech_time = 0;
  playing_good_descriptive_speech = 0;
  playing_bad_descriptive_speech = 0;
  played_good_descriptive_speech = 0;
  played_bad_descriptive_speech = 0;
  playing_speech_lvnum = SINGLEPLAYER_NOTSTARTED;
  mouse_over_lvnum = SINGLEPLAYER_NOTSTARTED;
  wait_for_cd_to_be_available();
  frontend_load_data_from_cd();
  lvnum = get_continue_level_number();
  if (!load_map_and_window(lvnum))
  {
    frontend_load_data_reset();
    return 0;
  }
  if (LbDataLoadAll(map_flag_load_files))
  {
    error(func_name, 373, "Unable to load Land View Screen sprites");
    frontend_load_data_reset();
    return 0;
  }
  LbSpriteSetupAll(map_flag_setup_sprites);
  frontend_load_data_reset();
  if ((game.flags_cd & 0x10) == 0)
    PlayRedbookTrack(2);
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  lvnum = get_continue_level_number();
  if ((player->field_6 & 0x02) != 0)
  {
    lvnum = get_loaded_level_number();
    lvinfo = get_level_info(lvnum);
    if (lvinfo != NULL)
    {
      map_info.field_A = lvinfo->ensign_zoom_x;
      map_info.field_E = lvinfo->ensign_zoom_y;
      set_map_info_draw_hotspot(lvinfo->ensign_zoom_x,lvinfo->ensign_zoom_y);
    } else
    {
      map_info.field_A = (MAP_SCREEN_WIDTH>>1);
      map_info.field_E = (MAP_SCREEN_HEIGHT>>1);
      set_map_info_draw_hotspot((MAP_SCREEN_WIDTH>>1),(MAP_SCREEN_HEIGHT>>1));
    }
    map_info.field_6 = 240;
    map_info.field_2 = -4;
    map_info.field_0 = 1;
    map_info.field_1 = 1;
  } else
  if ((lvnum == first_singleplayer_level()) || (player->victory_state == VicS_LostLevel) || (player->victory_state == VicS_State3))
  {
    lvinfo = get_level_info(lvnum);
    if (lvinfo != NULL)
    {
      set_map_info_draw_hotspot(lvinfo->ensign_zoom_x,lvinfo->ensign_zoom_y);
      set_map_info_visible_hotspot(lvinfo->ensign_zoom_x, lvinfo->ensign_zoom_y);
    } else
    {
      set_map_info_draw_hotspot((MAP_SCREEN_WIDTH>>1),(MAP_SCREEN_HEIGHT>>1));
      set_map_info_visible_hotspot((MAP_SCREEN_WIDTH>>1),(MAP_SCREEN_HEIGHT>>1));
    }
    map_info.field_0 = 0;
    play_desc_speech_time = LbTimerClock() + 1000;
  } else
  {
    lvnum = prev_singleplayer_level(lvnum);
    lvinfo = get_level_info(lvnum);
    if (lvinfo != NULL)
    {
      map_info.field_A = lvinfo->ensign_zoom_x;
      map_info.field_E = lvinfo->ensign_zoom_y;
      set_map_info_draw_hotspot(lvinfo->ensign_zoom_x,lvinfo->ensign_zoom_y);
    } else
    {
      map_info.field_A = (MAP_SCREEN_WIDTH>>1);
      map_info.field_E = (MAP_SCREEN_HEIGHT>>1);
      set_map_info_draw_hotspot((MAP_SCREEN_WIDTH>>1),(MAP_SCREEN_HEIGHT>>1));
    }
    map_info.field_6 = 240;
    map_info.field_2 = -4;
    map_info.field_0 = 1;
    map_info.field_1 = 1;
  }
  map_sound_fade = 256;
  map_info.field_26 = 0;
  map_info.field_1E = map_info.field_16 << 8;
  map_info.field_2A = 0;
  map_info.field_22 = map_info.field_1A << 8;
  spr = &map_flag[1];
  if (spr < end_map_flag)
    LbMouseChangeSprite(spr);
  LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
    if ((features_enabled & Ft_AdvAmbSonud) != 0)
  {
    play_sample_using_heap(0, campaign.ambient_good, 0, 0x40, 100, -1, 2, 0);
    play_sample_using_heap(0, campaign.ambient_bad, 0, 0x40, 100, -1, 2, 0);
  }
  if ((game.flags_cd & 0x10) == 0)
    SetRedbookVolume(settings.redbook_volume);
  fe_computer_players = 0;
  update_ensigns_visibility();
  return 1;
}

void frontmap_draw(void)
{
  static const char *func_name="frontmap_draw";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_frontmap_draw(); return;
  if (map_info.field_1)
  {
    frontzoom_to_point(map_info.field_A, map_info.field_E, map_info.field_6);
    compressed_window_draw();
  }
  else
  {
    draw_map_screen();
    draw_map_level_ensigns();
    compressed_window_draw();
  }
}

void check_mouse_scroll(void)
{
  _DK_check_mouse_scroll();
}

void update_velocity(void)
{
  static const char *func_name="update_velocity";
  _DK_update_velocity();
}

void frontnetmap_draw(void)
{
  static const char *func_name="frontnetmap_draw";
  _DK_frontnetmap_draw();
}

void frontmap_input(void)
{
  static const char *func_name="frontmap_input";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  short zoom_done;
  long mouse_x,mouse_y;
  //_DK_frontmap_input(); return;
  if (map_info.field_0)
  {
    if (!map_info.field_1)
    {
      map_info.field_0 = 0;
      play_desc_speech_time = LbTimerClock() + 1000;
    }
    zoom_done = false;
  } else
  {
    zoom_done = true;
  }
  if (zoom_done)
  {
    if (is_key_pressed(KC_ESCAPE, KM_DONTCARE))
    {
      clear_key_pressed(KC_ESCAPE);
      frontend_set_state(1);
      LbPaletteStopOpenFade();
      return;
    }
    check_mouse_scroll();
    if (left_button_clicked)
    {
      left_button_clicked = 0;
      frontmap_input_active_ensign(left_button_clicked_x, left_button_clicked_y);
      if (clicked_map_level_ensign())
        return;
    }
    mouse_x = GetMouseX();
    mouse_y = GetMouseY();
    frontmap_input_active_ensign(mouse_x, mouse_y);
  }
  update_velocity();
}

void frontnetmap_input(void)
{
  _DK_frontnetmap_input();
}

void frontmap_unload(void)
{
  static const char *func_name="frontmap_unload";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_frontmap_unload(); return;
  LbMouseChangeSprite(NULL);
  unload_map_and_window();
  LbDataFreeAll(map_flag_load_files);
  StopAllSamples();
  stop_description_speech();
  if ((game.flags_cd & 0x10) == 0)
  {
    StopRedbookTrack();
    SetRedbookVolume(settings.redbook_volume);
  }
}

long frontmap_update(void)
{
  static const char *func_name="frontmap_update";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_frontmap_update();
  if ((mouse_over_lvnum > 0) && (playing_speech_lvnum != mouse_over_lvnum))
  {
    play_desc_speech_time = 0;
    play_description_speech(mouse_over_lvnum,1);
  }
  if ((play_desc_speech_time != 0) && (LbTimerClock() > play_desc_speech_time))
  {
    play_desc_speech_time = 0;
    play_current_description_speech(1);
  }
  update_frontmap_ambient_sound();

  if ( map_info.field_1 )
  {
    if (map_info.field_2 == 4)
    {
      update_frontmap_info_draw_hotspot();
      if (map_sound_fade > 0)
      {
        map_sound_fade = 256 + (5*(1-map_info.field_6))/4;
        if (map_sound_fade < 0)
          map_sound_fade = 0;
      }
    }
    map_info.field_6 += map_info.field_2;
    if ((map_info.field_6 <= 1) || (map_info.field_6 >= 240))
    {
      LbPaletteStopOpenFade();
      map_info.field_1 = 0;
      if (map_info.field_12)
      {
        frontend_set_state(map_info.field_12);
        LbScreenClear(0);
        LbScreenSwap();
        map_info.field_12 = 0;
        return 1;
      }
    }
    if ( map_info.field_2 >= 0 )
    {
      if (map_info.field_6 >= 120)
        LbPaletteFade(NULL, 29, Lb_PALETTE_FADE_OPEN);
    } else
    if (map_info.field_6 > 120)
    {
      if ( map_info.field_2+map_info.field_6 <= 120)
        LbPaletteSet(frontend_palette);
      else
        LbPaletteFade(frontend_palette, 29, Lb_PALETTE_FADE_OPEN);
    }
  }
  if (playing_good_descriptive_speech)
  {
    if (StreamedSampleFinished())
    {
      playing_good_descriptive_speech = 0;
//      playing_speech_lvnum = SINGLEPLAYER_NOTSTARTED;
    }
  }
  if ((game.flags_cd & 0x10) == 0)
    PlayRedbookTrack(2);
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
  return 0;
}

long frontnetmap_update(void)
{
  return _DK_frontnetmap_update();
}




/******************************************************************************/
#ifdef __cplusplus
}
#endif
