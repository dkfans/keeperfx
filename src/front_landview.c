/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_landview.c
 *     Land view, where the user can select map for campaign or multiplayer.
 * @par Purpose:
 *     Functions for displaying and maintaining the land view.
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
#include "pre_inc.h"
#include "front_landview.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_datetm.h"
#include "bflib_planar.h"
#include "bflib_video.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_filelst.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_mouse.h"
#include "bflib_math.h"
#include "bflib_sndlib.h"
#include "bflib_sound.h"
#include "bflib_vidraw.h"
#include "bflib_network.h"
#include "bflib_network_exchange.h"

#include "config_strings.h"
#include "config_campaigns.h"
#include "config_keeperfx.h"
#include "config_settings.h"
#include "game_lghtshdw.h"
#include "light_data.h"
#include "lvl_filesdk1.h"
#include "room_list.h"
#include "engine_textures.h"
#include "front_simple.h"
#include "front_network.h"
#include "front_lvlstats.h"
#include "frontend.h"
#include "kjm_input.h"
#include "vidmode.h"
#include "vidfade.h"
#include "game_legacy.h"
#include "front_input.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct NetMapPlayersState {
    long tmp1;
    LevelNumber lvnum;
    TbBool is_selected;
};

/******************************************************************************/
#define WINDOW_X_SIZE 960
#define WINDOW_Y_SIZE 720
TbPixel net_player_colours[] = { 251, 58, 182, 11};
const long hand_limp_xoffset[] = { 32,  31,  30,  29,  28,  27,  26,  24,  22,  19,  15,  9, };
const long hand_limp_yoffset[] = {-11, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,  0, };
struct TbSprite dummy_sprite = {0, 0, 0};

long limp_hand_x = 0;
long limp_hand_y = 0;
LevelNumber mouse_over_lvnum;
LevelNumber playing_speech_lvnum;
struct TbHugeSprite map_window;
long map_window_len = 0;

TbClockMSec play_desc_speech_time;
unsigned long played_bad_descriptive_speech;
unsigned long played_good_descriptive_speech;
struct TbSpriteSheet * map_flag = NULL;
struct TbSpriteSheet * map_font = NULL;
struct TbSpriteSheet * map_hand = NULL;
long map_sound_fade;
unsigned char *map_screen;
long fe_net_level_selected;
long net_map_limp_time;
struct ScreenPacket net_screen_packet[NET_PLAYERS_COUNT];
long players_currently_in_session;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void draw_map_screen(void)
{
    copy_raw8_image_buffer(lbDisplay.WScreen,LbGraphicsScreenWidth(),LbGraphicsScreenHeight(),
        scale_value_landview(LANDVIEW_MAP_WIDTH), scale_value_landview(LANDVIEW_MAP_HEIGHT),
        -scale_value_landview(map_info.screen_shift_x), -scale_value_landview(map_info.screen_shift_y),
        map_screen,LANDVIEW_MAP_WIDTH,LANDVIEW_MAP_HEIGHT);
}

const struct TbSprite * get_map_ensign(long idx)
{
    if (idx >= 0 && idx < num_sprites(map_flag)) {
        return get_sprite(map_flag, idx);
    }
    return &dummy_sprite;
}

/**
 * Determines if given coordinates are screen position which is over an ensign.
 * @param lvinfo Level info struct with ensign definition.
 * @param scr_x The screen point being checked, X coordinate.
 * @param scr_y The screen point being checked, Y coordinate.
 * @return True if the coords are over given ensign, false otherwise.
 */
short is_over_ensign(const struct LevelInformation *lvinfo, long scr_x, long scr_y)
{
    long map_x = map_info.screen_shift_x + scr_x * 16 / units_per_pixel_landview;
    long map_y = map_info.screen_shift_y + scr_y * 16 / units_per_pixel_landview;
    const struct TbSprite* spr = get_map_ensign(10);
    long spr_w = spr->SWidth;
    long spr_h = spr->SHeight;
    if ((map_x >= lvinfo->ensign_x-(spr_w>>1)) && (map_x < lvinfo->ensign_x+(spr_w>>1))
     && (map_y > lvinfo->ensign_y-spr_h) && (map_y < lvinfo->ensign_y-(spr_h/3)))
        return true;
    return false;
}

/**
 * Determines if given ensign is inside visible part of the map screen.
 * @param lvinfo Level info struct with ensign definition.
 * @return True if the ensign is visible, false otherwise.
 */
short is_ensign_in_screen_rect(const struct LevelInformation *lvinfo)
{
    if ((lvinfo->ensign_zoom_x >= map_info.screen_shift_x) && (lvinfo->ensign_zoom_x < map_info.screen_shift_x+lbDisplay.PhysicalScreenWidth*16/units_per_pixel_landview))
      if ((lvinfo->ensign_zoom_y >= map_info.screen_shift_y) && (lvinfo->ensign_zoom_y < map_info.screen_shift_y+lbDisplay.PhysicalScreenHeight*16/units_per_pixel_landview))
        return true;
    return false;
}

/**
 * Changes state of all land map ensigns.
 */
void set_all_ensigns_state(unsigned short nstate)
{
    struct LevelInformation* lvinfo = get_first_level_info();
    while (lvinfo != NULL)
    {
        lvinfo->state = nstate;
        lvinfo = get_next_level_info(lvinfo);
  }
}

void update_ensigns_visibility(void)
{
  struct LevelInformation *lvinfo;
  SYNCDBG(18,"Starting");
  set_all_ensigns_state(LvSt_Hidden);
  struct PlayerInfo* player = get_my_player();
  short show_all_sp = false;
  long lvnum = get_continue_level_number();
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
    long bn_lvnum = bonus_level_for_singleplayer_level(lvnum);
    if (is_bonus_level_visible(player, bn_lvnum))
    {
        lvinfo = get_level_info(bn_lvnum);
        if (lvinfo != NULL)
            lvinfo->state = LvSt_Visible;
    }
    lvnum = next_singleplayer_level(lvnum, true);
  }
  // Extra level - full moon
  lvnum = get_extra_level(ExLv_FullMoon);
  lvinfo = get_level_info(lvnum);
  if (lvinfo != NULL)
      lvinfo->state = get_extra_level_kind_visibility(ExLv_FullMoon);
  // Extra level - new moon
  lvnum = get_extra_level(ExLv_NewMoon);
  lvinfo = get_level_info(lvnum);
  if (lvinfo != NULL)
      lvinfo->state = get_extra_level_kind_visibility(ExLv_NewMoon);
}

void update_net_ensigns_visibility(void)
{
    SYNCDBG(18, "Starting");
    set_all_ensigns_state(LvSt_Hidden);
    long lvnum = first_multiplayer_level();
    while (lvnum > 0)
    {
        struct LevelInformation* lvinfo = get_level_info(lvnum);
        if (lvinfo != NULL)
            lvinfo->state = LvSt_Visible;
        lvnum = next_multiplayer_level(lvnum);
    }
}

int compute_sound_good_to_bad_factor(void)
{
    SYNCDBG(18, "Starting");
    unsigned int onscr_bad = 0;
    unsigned int onscr_good = 0;
    LevelNumber continue_lvnum = get_continue_level_number();
    short lv_beaten = (continue_lvnum != SINGLEPLAYER_NOTSTARTED);
    LevelNumber sp_lvnum = first_singleplayer_level();
    while (sp_lvnum > 0)
    {
        if (sp_lvnum == continue_lvnum)
            lv_beaten = false;
        struct LevelInformation* lvinfo = get_level_info(sp_lvnum);
        if (lvinfo != NULL)
        {
            if (is_ensign_in_screen_rect(lvinfo))
            {
                if (lv_beaten)
                    onscr_bad++;
                else
                    onscr_good++;
            }
        }
        sp_lvnum = next_singleplayer_level(sp_lvnum, true);
    }
    if ((onscr_bad + onscr_good) == 0)
        onscr_good++;
    // return a value between 0 (all bad) and 256 (all good)
    return (FULL_LOUDNESS * onscr_good) / (onscr_bad + onscr_good);
}

void update_frontmap_ambient_sound(void)
{
    // NOTE: the good / bad samples start at a volume of zero.
    SoundEmitterID emit_id = get_emitter_id(S3DGetSoundEmitter(Non3DEmitter));
    if (map_sound_fade > 0)
    {
        long lvidx = array_index_for_singleplayer_level(get_continue_level_number());
        if ((features_enabled & Ft_AdvAmbSound) != 0)
        {
            long factor = compute_sound_good_to_bad_factor();
            SetSampleVolume(emit_id, campaign.ambient_good, (map_sound_fade * (((long)settings.sound_volume * factor) / FULL_LOUDNESS)) / FULL_LOUDNESS);
            SetSampleVolume(emit_id, campaign.ambient_bad, (map_sound_fade * (((long)settings.sound_volume * (FULL_LOUDNESS - factor)) / FULL_LOUDNESS)) / FULL_LOUDNESS);
        } else
        if (lvidx > 13)
        {
            SetSampleVolume(emit_id, campaign.ambient_bad, ((long)settings.sound_volume * map_sound_fade) / FULL_LOUDNESS);
        } else
        {
        SetSampleVolume(emit_id, campaign.ambient_good, ((long)settings.sound_volume * map_sound_fade) / FULL_LOUDNESS);
        }
        set_streamed_sample_volume(((long)settings.sound_volume * map_sound_fade) / FULL_LOUDNESS);
        set_music_volume((map_sound_fade * settings.music_volume) / FULL_LOUDNESS);
    } else
    {
        if ((features_enabled & Ft_AdvAmbSound) != 0)
        {
            SetSampleVolume(emit_id, campaign.ambient_good, 0);
            SetSampleVolume(emit_id, campaign.ambient_bad, 0);
        }
        set_music_volume(0);
        set_streamed_sample_volume(0);
    }
}

int get_disabled_flag_option (unsigned short ensign, unsigned short default_ensign)
{
    int base_ensign;
    if (ensign == 0)
    {
        base_ensign = default_ensign;
    }
    else
    {
        base_ensign = ensign;
    }
    if (base_ensign == EnsTutorial)
        return EnsDisTutorial;
    else if (base_ensign == EnsFullFlag)
        return EnsDisFull;
    else if (base_ensign == EnsBonus)
        return EnsDisFull;
    else if (base_ensign == EnsFullMoon)
        return EnsDisMoonF;
    else if (base_ensign == EnsNewMoon)
        return EnsDisMoonN;

    return EnsDisFull;
}

const struct TbSprite *get_ensign_sprite_for_level(struct LevelInformation *lvinfo, int anim_frame)
{
    const struct TbSprite *spr;
    int i = lvinfo->ensign;
    if (lvinfo == NULL)
        return NULL;
    if (lvinfo->state == LvSt_Hidden)
        return NULL;

    if (lvinfo->level_type & LvKind_IsSingle)
    {
        switch (lvinfo->state)
        {
        case LvSt_Visible:
            if (i == 0)
                i = EnsFullFlag;
            if (lvinfo->lvnum == mouse_over_lvnum)
                i += 4;
            spr = get_map_ensign(i + (anim_frame & 3));
            break;
        default:
            i = get_disabled_flag_option(lvinfo->ensign, EnsFullFlag);
            spr = get_map_ensign(i);
            break;
        }
    } else
    if (lvinfo->level_type & LvKind_IsBonus)
    {
        if (i == 0)
            i = EnsBonus;
        switch (lvinfo->state)
        {
        case LvSt_Visible:
            if (lvinfo->lvnum == mouse_over_lvnum)
                i += 4;
            spr = get_map_ensign(i + (anim_frame & 3));
            break;
        default:
            i = get_disabled_flag_option(lvinfo->ensign, EnsTutorial);
            spr = get_map_ensign(i);
            break;
        }
    } else
    if (lvinfo->level_type & LvKind_IsExtra)
    {
        if (i == 0)
        {
            if (lvinfo->lvnum == get_extra_level(ExLv_NewMoon))
            {
                i = EnsNewMoon;
            }
            else
            {
                i = EnsFullMoon;
            }
        }
        switch (lvinfo->state)
        {
            case LvSt_Visible:
                if (lvinfo->lvnum == mouse_over_lvnum)
                    i += 4;
                spr = get_map_ensign(i + (anim_frame & 3));
                break;
            default:
                if (lvinfo->lvnum == get_extra_level(ExLv_NewMoon))
                {
                    i = get_disabled_flag_option(lvinfo->ensign, EnsFullMoon);
                }
                else
                {
                    i = get_disabled_flag_option(lvinfo->ensign, EnsNewMoon);
                }
                spr = get_map_ensign(i);
                break;
        }
    } else
    if (lvinfo->level_type & LvKind_IsMulti) //Note that multiplayer flags have different file
    {
        if (frontend_menu_state == FeSt_NETLAND_VIEW)
        {
            switch (lvinfo->players)
            {
            case 2:
                i = 5;
                break;
            case 3:
                i = 7;
                break;
            case 4:
                i = 9;
                break;
            default:
                i = 5;
                break;
            }
            if ((fe_net_level_selected == lvinfo->lvnum) || (net_level_hilighted == lvinfo->lvnum))
                i++;
        }
        else
        {
            switch (lvinfo->players)
            {
            case 2:
                i = EnsDisMulti2;
                break;
            case 3:
                i = EnsDisMulti3;
                break;
            case 4:
                i = EnsDisMulti4;
                break;
            default:
                i = EnsDisMulti2;
                break;
            }
        }
        spr = get_map_ensign(i);
    }
    else
    {
        spr = get_map_ensign(EnsDisFull);
    }
    if (spr == &dummy_sprite)
        ERRORLOG("Can't get Land view Ensign sprite");
    return spr;
}

/**
 * Draws the visible level ensigns on screen.
 * Note that the drawing is in reverse order than the one of reading inputs.
 */
void draw_map_level_ensigns(void)
{
    SYNCDBG(18,"Starting");
    int k = LbTimerClock() / 200;
    struct LevelInformation* lvinfo = get_last_level_info();
    while (lvinfo != NULL)
    {
      // the flag sprite
      const struct TbSprite* spr = get_ensign_sprite_for_level(lvinfo, k);
      if (spr != NULL)
      {
          long x = lvinfo->ensign_x - map_info.screen_shift_x - (int)(spr->SWidth >> 1);
          long y = lvinfo->ensign_y - map_info.screen_shift_y - (int)(spr->SHeight);
          LbSpriteDrawResized(scale_value_landview(x), scale_value_landview(y), units_per_pixel_landview, spr);
      }
      lvinfo = get_prev_level_info(lvinfo);
    }
}

/** Sets visible land picture area to have top left corner on given coordinates.
 *
 * @param map_x Shift X coordinate for top left of the visible land picture area.
 * @param map_y Shift Y coordinate for top left of the visible land picture area.
 * @note To be used only in low-level screen shift processing; use set_map_info_screen_shift() instead.
 */
void set_map_info_screen_shift_raw(long map_x, long map_y)
{
    map_info.screen_shift_x = map_x;
    map_info.screen_shift_y = map_y;
    // Make sure the hotspot will not be too close to border to not be drawn correctly at full zoom
    long delta_x;
    long delta_y;
    if ((map_info.fadeflags & MLInfoFlg_Zooming) != 0) {
        delta_x = (lbDisplay.PhysicalScreenWidth*(256 - map_info.fade_pos)*16/units_per_pixel_landview) / 256;
        delta_y = (lbDisplay.PhysicalScreenHeight*(256 - map_info.fade_pos)*16/units_per_pixel_landview) / 256;
    } else {
        delta_x = (lbDisplay.PhysicalScreenWidth*16/units_per_pixel_landview);
        delta_y = (lbDisplay.PhysicalScreenHeight*16/units_per_pixel_landview);
    }
    if (map_info.screen_shift_x > LANDVIEW_MAP_WIDTH - delta_x)
        map_info.screen_shift_x = LANDVIEW_MAP_WIDTH - delta_x;
    if (map_info.screen_shift_x < 0)
        map_info.screen_shift_x = 0;
    if (map_info.screen_shift_y > LANDVIEW_MAP_HEIGHT - delta_y)
        map_info.screen_shift_y = LANDVIEW_MAP_HEIGHT - delta_y;
    if (map_info.screen_shift_y < 0)
        map_info.screen_shift_y = 0;
}

/** Sets visible land picture area to be centered over given coordinates.
 *
 * @param map_x Shift X coordinate for center of the visible land picture area.
 * @param map_y Shift Y coordinate for center of the visible land picture area.
 */
void set_map_info_screen_shift(long map_x, long map_y)
{
    long delta_x = (lbDisplay.PhysicalScreenWidth * 16 / units_per_pixel_landview) / 2;
    long delta_y = (lbDisplay.PhysicalScreenHeight * 16 / units_per_pixel_landview) / 2;
    set_map_info_screen_shift_raw(map_x - delta_x, map_y - delta_y);
    // Reset precise shifts, which are often used for screen shift update
    // The reset is here so that new values have correct clipping applied
    map_info.precise_scrshift_x = map_info.screen_shift_x << 8;
    map_info.precise_scrshift_y = map_info.screen_shift_y << 8;
}

void step_frontmap_info_screen_shift_zoom(void)
{
    // Count the remaining shift
    long scr_x = map_info.hotspot_shift_x - map_info.screen_shift_aimed_x;
    long scr_y = map_info.hotspot_shift_y - map_info.screen_shift_aimed_y;
    if ((scr_x != 0) || (scr_y != 0))
    {
        long step = LbSinL(DEGREES_90 * map_info.fade_pos / FRONTMAP_ZOOM_LENGTH);
        map_info.precise_scrshift_x = (map_info.screen_shift_aimed_x << 8) + (scr_x * step) / 256;
        map_info.precise_scrshift_y = (map_info.screen_shift_aimed_y << 8) + (scr_y * step) / 256;
        set_map_info_screen_shift_raw(map_info.precise_scrshift_x >> 8, map_info.precise_scrshift_y >> 8);
    }
}

void set_map_info_visible_hotspot_raw(long map_x,long map_y)
{
    map_info.hotspot_shift_x = map_x;
    map_info.hotspot_shift_y = map_y;
    if (map_info.hotspot_shift_x > LANDVIEW_MAP_WIDTH - lbDisplay.PhysicalScreenWidth*16/units_per_pixel_landview)
        map_info.hotspot_shift_x = LANDVIEW_MAP_WIDTH - lbDisplay.PhysicalScreenWidth*16/units_per_pixel_landview;
    if (map_info.hotspot_shift_x < 0)
        map_info.hotspot_shift_x = 0;
    if (map_info.hotspot_shift_y > LANDVIEW_MAP_HEIGHT - lbDisplay.PhysicalScreenHeight*16/units_per_pixel_landview)
        map_info.hotspot_shift_y = LANDVIEW_MAP_HEIGHT - lbDisplay.PhysicalScreenHeight*16/units_per_pixel_landview;
    if (map_info.hotspot_shift_y < 0)
        map_info.hotspot_shift_y = 0;
}

void set_map_info_visible_hotspot(long map_x,long map_y)
{
    long delta_x = (lbDisplay.PhysicalScreenWidth * 16 / units_per_pixel_landview) / 2;
    long delta_y = (lbDisplay.PhysicalScreenHeight * 16 / units_per_pixel_landview) / 2;
    set_map_info_visible_hotspot_raw(map_x - delta_x, map_y - delta_y);
}

void frontmap_zoom_skip_init(LevelNumber lvnum)
{
    struct LevelInformation* lvinfo = get_level_info(lvnum);
    // Update hostspot to ensign position
    if (lvinfo != NULL)
    {
        map_info.hotspot_imgpos_x = lvinfo->ensign_zoom_x;
        map_info.hotspot_imgpos_y = lvinfo->ensign_zoom_y;
    } else
    {
        map_info.hotspot_imgpos_x = (LANDVIEW_MAP_WIDTH>>1);
        map_info.hotspot_imgpos_y = (LANDVIEW_MAP_HEIGHT>>1);
    }
    set_map_info_visible_hotspot(map_info.hotspot_imgpos_x, map_info.hotspot_imgpos_y);
    // Disable fade so that screen shift function fixes coords correctly
    map_info.fadeflags &= ~MLInfoFlg_Zooming;
    set_map_info_screen_shift(map_info.hotspot_imgpos_x,map_info.hotspot_imgpos_y);
    // Set aimed screen shift, shouldn't be used anyway
    map_info.screen_shift_aimed_x = map_info.hotspot_shift_x;
    map_info.screen_shift_aimed_y = map_info.hotspot_shift_y;
    // Set working parameters for zooming
    map_info.fade_pos = 1;
    map_info.fade_step = 0;
    map_info.fadeflags &= ~MLInfoFlg_SpeechAfterZoom;
    map_info.fadeflags &= ~MLInfoFlg_Zooming;
}

void frontmap_zoom_out_init(LevelNumber prev_lvnum, LevelNumber next_lvnum)
{
    struct LevelInformation* prev_lvinfo = get_level_info(prev_lvnum);
    struct LevelInformation* next_lvinfo = get_level_info(next_lvnum);
    // Update hostspot to ensign position
    if (prev_lvinfo != NULL)
    {
        map_info.hotspot_imgpos_x = prev_lvinfo->ensign_zoom_x;
        map_info.hotspot_imgpos_y = prev_lvinfo->ensign_zoom_y;
    } else
    {
        map_info.hotspot_imgpos_x = (LANDVIEW_MAP_WIDTH>>1);
        map_info.hotspot_imgpos_y = (LANDVIEW_MAP_HEIGHT>>1);
    }
    set_map_info_visible_hotspot(map_info.hotspot_imgpos_x, map_info.hotspot_imgpos_y);
    // Disable fade so that screen shift function fixes coords correctly
    map_info.fadeflags &= ~MLInfoFlg_Zooming;
    if (next_lvinfo != NULL)
    {
        // Shift towards next flag, but not too much - old flag pos must be on screen all the time
        // otherwise draw function will clip its coordinates
        long maxdelta_x = (lbDisplay.PhysicalScreenWidth * 16 / units_per_pixel_landview) / 2;
        long maxdelta_y = (lbDisplay.PhysicalScreenHeight * 16 / units_per_pixel_landview) / 2;
        long dt_x = (next_lvinfo->ensign_zoom_x - map_info.hotspot_imgpos_x) / 2;
        if (dt_x > maxdelta_x)
            dt_x = maxdelta_x;
        if (dt_x < -maxdelta_x)
            dt_x = -maxdelta_x;
        long dt_y = (next_lvinfo->ensign_zoom_y - map_info.hotspot_imgpos_y) / 2;
        if (dt_y > maxdelta_y)
            dt_y = maxdelta_y;
        if (dt_y < -maxdelta_y)
            dt_y = -maxdelta_y;
        set_map_info_screen_shift(map_info.hotspot_imgpos_x+dt_x, map_info.hotspot_imgpos_y+dt_y);
    } else
    {
        set_map_info_screen_shift(map_info.hotspot_imgpos_x,map_info.hotspot_imgpos_y);
    }
    // Set aimed screen shift, used as reference position we want to achieve
    map_info.screen_shift_aimed_x = map_info.screen_shift_x;
    map_info.screen_shift_aimed_y = map_info.screen_shift_y;
    // Set working parameters for zooming
    map_info.fade_pos = FRONTMAP_ZOOM_LENGTH;
    map_info.fade_step = -FRONTMAP_ZOOM_STEP;
    map_info.fadeflags |= MLInfoFlg_SpeechAfterZoom;
    map_info.fadeflags |= MLInfoFlg_Zooming;
}

void frontmap_zoom_in_init(LevelNumber lvnum)
{
    struct LevelInformation* lvinfo = get_level_info(lvnum);
    // Disable fade so that hostspot function fixes coords correctly
    map_info.fadeflags &= ~MLInfoFlg_Zooming;
    if (lvinfo != NULL)
    {
        map_info.hotspot_imgpos_x = lvinfo->ensign_zoom_x;
        map_info.hotspot_imgpos_y = lvinfo->ensign_zoom_y;
    } else
    {
        map_info.hotspot_imgpos_x = (LANDVIEW_MAP_WIDTH>>1);
        map_info.hotspot_imgpos_y = (LANDVIEW_MAP_HEIGHT>>1);
    }
    set_map_info_visible_hotspot(map_info.hotspot_imgpos_x, map_info.hotspot_imgpos_y);
    // We don't need to set screen_shift as it is user-made value when zooming in
    // Set aimed screen shift, used as reference position we started with
    map_info.screen_shift_aimed_x = map_info.screen_shift_x;
    map_info.screen_shift_aimed_y = map_info.screen_shift_y;
    SYNCDBG(8,"Level %ld hotspot (%d,%d) zoom (%d,%d)",(long)lvnum,(int)map_info.hotspot_shift_x,(int)map_info.hotspot_shift_y,(int)map_info.hotspot_imgpos_x,(int)map_info.hotspot_imgpos_y);
    // Set working parameters for zooming
    map_info.fade_step = FRONTMAP_ZOOM_STEP;
    map_info.fade_pos = 1;
    map_info.fadeflags |= MLInfoFlg_SpeechAfterZoom;
    map_info.fadeflags |= MLInfoFlg_Zooming;
}

TbBool frontmap_input_active_ensign(long curr_mx, long curr_my)
{
    struct LevelInformation* lvinfo = get_first_level_info();
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
    struct LevelInformation* lvinfo = get_level_info(mouse_over_lvnum);
    if (lvinfo != NULL)
    {
      set_selected_level_number(lvinfo->lvnum);
      frontmap_zoom_in_init(lvinfo->lvnum);
      map_info.state_trigger = FeSt_START_KPRLEVEL;
      return true;
    }
    return false;
}

TbBool initialize_description_speech(void)
{
    play_desc_speech_time = 0;
    playing_speech_lvnum = SINGLEPLAYER_NOTSTARTED;
    playing_good_descriptive_speech = 0;
    playing_bad_descriptive_speech = 0;
    played_good_descriptive_speech = 0;
    played_bad_descriptive_speech = 0;
    return true;
}

TbBool stop_description_speech(void)
{
    if ((playing_good_descriptive_speech) || (playing_bad_descriptive_speech))
    {
        playing_good_descriptive_speech = 0;
        playing_bad_descriptive_speech = 0;
        playing_speech_lvnum = SINGLEPLAYER_NOTSTARTED;
        stop_streamed_samples();
        return true;
    }
    return false;
}

TbBool play_current_description_speech(short play_good)
{
    LevelNumber lvnum = get_continue_level_number();
    if (!play_good)
        lvnum = prev_singleplayer_level(lvnum);
    return play_description_speech(lvnum,play_good);
}

TbBool play_description_speech(LevelNumber lvnum, short play_good)
{
    char *fname;
    if (playing_speech_lvnum == lvnum)
      return true;
    struct LevelInformation* lvinfo = get_level_info(lvnum);
    if (lvinfo == NULL)
      return false;
    if (play_good)
    {
      if (lvinfo->speech_before[0] == '\0')
        return false;
      stop_description_speech();
      if (strchr(lvinfo->speech_before, '.') == NULL)
      {
          WARNLOG("No extension specified for good speech file; defaulting to '.wav'.");
          char path[DISKPATH_SIZE];
          snprintf(path, sizeof(path), "%s.wav", lvinfo->speech_before);
          fname = prepare_file_fmtpath(FGrp_AtlSound, "%s", path);
      }
      else
      {
          fname = prepare_file_fmtpath(FGrp_AtlSound,"%s",lvinfo->speech_before);
      }
      playing_good_descriptive_speech = 1;
    } else
    {
      if (lvinfo->speech_after[0] == '\0')
        return false;
      stop_description_speech();
      if (strchr(lvinfo->speech_after, '.') == NULL)
      {
          WARNLOG("No extension specified for evil speech file; defaulting to '.wav'.");
          char path[DISKPATH_SIZE];
          snprintf(path, sizeof(path), "%s.wav", lvinfo->speech_after);
          fname = prepare_file_fmtpath(FGrp_AtlSound, "%s", path);
      }
      else
      {
          fname = prepare_file_fmtpath(FGrp_AtlSound,"%s",lvinfo->speech_after);
      }
      playing_bad_descriptive_speech = 1;
    }
    playing_speech_lvnum = lvnum;
    SYNCMSG("Playing %s", fname);
    //volume is overwritten in update_frontmap_ambient_sound
    return play_streamed_sample(fname, settings.sound_volume);
}

TbBool set_pointer_graphic_spland(long frame)
{
    const struct TbSprite* spr = get_map_ensign(1);
    if (spr == &dummy_sprite)
      ERRORLOG("Can't get Land view Mouse sprite");
    LbMouseChangeSprite(spr);
    return (spr != &dummy_sprite);
}

void frontzoom_to_point(long map_x, long map_y, long zoom)
{
    unsigned char *src;
    long bpos_x;
    long x;
    long y;
    long src_delta = (256 - zoom) * 16 / units_per_pixel_landview;
    long smap_x = scale_value_landview(map_x);
    long smap_y = scale_value_landview(map_y);
    // Initializing variables used for all quadres of screen
    // First find a quadres division place - coords bounding the quadres
    // Make sure each quadre is at least one pixel wide and high
    long scr_x = smap_x - scale_value_landview(map_info.screen_shift_x);
    if (scr_x > lbDisplay.PhysicalScreenWidth-1) scr_x = lbDisplay.PhysicalScreenWidth-1;
    if (scr_x < 1) scr_x = 1;
    long scr_y = smap_y - scale_value_landview(map_info.screen_shift_y);
    if (scr_y > lbDisplay.PhysicalScreenHeight-1) scr_y = lbDisplay.PhysicalScreenHeight-1;
    if (scr_y < 1) scr_y = 1;
    unsigned char* src_buf = &map_screen[LANDVIEW_MAP_WIDTH * map_y + map_x];
    long dst_scanln = lbDisplay.GraphicsScreenWidth;
    unsigned char* dst_buf = &lbDisplay.WScreen[dst_scanln * scr_y + scr_x];
    // Drawing first quadre
    long bpos_y = 0;
    unsigned char* dst = dst_buf;
    long dst_width = scr_x;
    long dst_height = scr_y;
    // FIXME: I'm sure there's a less convoluted way of doing this, code below is setting off lots of cppcheck alarms
    for (y=0; y <= dst_height; y++)
    {
        bpos_x = 0;
        src = &src_buf[-LANDVIEW_MAP_WIDTH*(bpos_y >> 8)];
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
    dst = dst_buf + 1;
    dst_width = -scr_x + lbDisplay.PhysicalScreenWidth - 1; // one pixel less in destination
    dst_height = scr_y;
    for (y=0; y <= dst_height; y++)
    {
        bpos_x = (1 << 8); // one pixel less in source
        src = &src_buf[-LANDVIEW_MAP_WIDTH*(bpos_y >> 8)];
        for (x=0; x < dst_width; x++)
        {
          bpos_x += src_delta;
          dst[x] = src[(bpos_x >> 8)];
        }
        dst -= dst_scanln;
        bpos_y += src_delta;
    }
    // Drawing 3rd quadre
    bpos_y = (1 << 8); // one pixel less in source
    dst = dst_buf + dst_scanln;
    dst_width = scr_x;
    dst_height = -scr_y + lbDisplay.PhysicalScreenHeight - 1; // one pixel less in destination
    for (y=0; y < dst_height; y++)
    {
        bpos_x = 0;
        src = &src_buf[LANDVIEW_MAP_WIDTH*(bpos_y >> 8)];
        for (x=0; x <= dst_width; x++)
        {
            bpos_x += src_delta;
            dst[-x] = src[-(bpos_x >> 8)];
        }
        dst += dst_scanln;
        bpos_y += src_delta;
    }
    // Drawing 4th quadre
    bpos_y = (1 << 8);
    dst = dst_buf + dst_scanln + 1;
    dst_width = -scr_x + lbDisplay.PhysicalScreenWidth - 1;
    dst_height = -scr_y + lbDisplay.PhysicalScreenHeight - 1;
    for (y=0; y < dst_height; y++)
    {
        bpos_x = (1 << 8);
        src = &src_buf[LANDVIEW_MAP_WIDTH*(bpos_y >> 8)];
        for (x=0; x < dst_width; x++)
        {
            dst[x] = src[(bpos_x >> 8)];
            bpos_x += src_delta;
        }
        dst += dst_scanln;
        bpos_y += src_delta;
    }
}
/** Draw the window frame on the campaign map (land view). */
void compressed_window_draw(void)
{
    SYNCDBG(18,"Starting");
    long default_movement_scale = 1024;
    long xshift = map_info.screen_shift_x * landview_frame_movement_scale_x / default_movement_scale / 2; // X speed is slower on aspect ratios wider than 4:3
    long yshift = map_info.screen_shift_y *landview_frame_movement_scale_y / default_movement_scale / 2; // Y speed is slower on aspect ratios taller than 4:3
    LbHugeSpriteDraw(&map_window, map_window_len,
        lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, lbDisplay.PhysicalScreenHeight,
        xshift, yshift, units_per_pixel_landview_frame);
}

void unload_map_and_window(void)
{
    clear_light_system(&game.lish);
    clear_things_and_persons_data();
    clear_mapmap();
    clear_computer();
    clear_slabs();
    clear_rooms();
    clear_dungeons();
    memcpy(frontend_palette, frontend_backup_palette, PALETTE_SIZE);
    map_window_len = 0;
}

TbBool load_map_and_window(LevelNumber lvnum)
{
    SYNCDBG(8,"Starting");
    // Select proper land view image for the level
    char* land_view = NULL;
    char* land_window = NULL;
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
        struct LevelInformation* lvinfo = get_level_info(lvnum);
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
        ERRORLOG("No Land View file names for level %d",lvnum);
        return false;
    }
    // Prepare full file name and load the image
    char* fname = prepare_file_fmtpath(FGrp_LandView, "%s.raw", land_view);
    long flen = LbFileLengthRnc(fname);
    if (flen < 1024)
    {
        ERRORLOG("Land Map background \"%s.raw\" doesn't exist or is too small",land_view);
        return false;
    }
    if (flen > 1228997)
    {
        ERRORLOG("Not enough memory in game structure for Land Map background \"%s.raw\"",land_view);
        return false;
    }
    if (LbFileLoadAt(fname, &game.land_map_start) != flen)
    {
        ERRORLOG("Unable to load Land Map background \"%s.raw\"",land_view);
        return false;
    }
    map_screen = &game.land_map_start;
    // Texture blocks memory isn't used here, so reuse it instead of allocating
    unsigned char* ptr = block_mem;
    memcpy(frontend_backup_palette, &frontend_palette, PALETTE_SIZE);
    // Now prepare window sprite file name and load the file
    fname = prepare_file_fmtpath(FGrp_LandView,"%s.dat",land_window);
    map_window_len = LbFileLoadAt(fname, ptr);
    if (map_window_len < (int32_t)(WINDOW_Y_SIZE*sizeof(int32_t)))
    {
        ERRORLOG("Unable to load Land Map Window \"%s.dat\"",land_window);
        unload_map_and_window();
        return false;
    }
    // Prepare pointer to offsets array; WINDOW_Y_SIZE entries
    map_window.Lines = (int32_t *)&ptr[0];
    // Prepare pointer to window data
    map_window.Data = &ptr[WINDOW_Y_SIZE*sizeof(int32_t)];
    // Fill the rest of huge sprite
    map_window.SWidth = WINDOW_X_SIZE;
    map_window.SHeight = WINDOW_Y_SIZE;
    // Update length, so that it corresponds to map_window pointer
    map_window_len -= WINDOW_Y_SIZE*sizeof(int32_t);
    // Load palette
    fname = prepare_file_fmtpath(FGrp_LandView,"%s.pal",land_view);
    if (LbFileLoadAt(fname, frontend_palette) != PALETTE_SIZE)
    {
        ERRORLOG("Unable to load Land Map palette \"%s.pal\"",land_view);
        unload_map_and_window();
        return false;
    }
    SYNCDBG(9,"Finished");
    return true;
}

void frontnet_init_level_descriptions(void)
{
    //TODO NETWORK Don't allow campaigns besides original - we don't have per-campaign MP yet
    //if (!is_campaign_loaded())
    {
        if (!change_campaign("")) {
            return;
        }
    }
}

void frontnetmap_unload(void)
{
    clear_light_system(&game.lish);
    clear_mapmap();
    clear_things_and_persons_data();
    clear_computer();
    clear_rooms();
    clear_dungeons();
    clear_slabs();
    memcpy(&frontend_palette, frontend_backup_palette, PALETTE_SIZE);
    free_font(&map_font);
    free_spritesheet(&map_flag);
    free_spritesheet(&map_hand);
    memcpy(&frontend_palette, frontend_backup_palette, PALETTE_SIZE);
    fe_network_active = 0;
    stop_music();
    set_music_volume(settings.music_volume);
}

static void frontmap_start_music(void)
{
    if (strlen(campaign.soundtrack_fname) > 0) {
        const int track = atoi(campaign.soundtrack_fname);
        if (track >= 1) {
            play_music_track(track);
        } else {
            const char* fname = prepare_file_fmtpath(FGrp_CmpgMedia, campaign.soundtrack_fname);
            play_music(fname);
        }
    } else {
        play_music_track(2);
    }
}

TbBool frontnetmap_load(void)
{
    SYNCDBG(8,"Starting");
    if (fe_network_active)
    {
      if (LbNetwork_EnableNewPlayers(0))
        ERRORLOG("Unable to prohibit new players joining exchange");
    }
    frontend_load_data_from_cd();
    game.selected_level_number = 0;
    if (!load_map_and_window(0))
    {
        frontend_load_data_reset();
        return false;
    }
    switch (campaign.land_markers)
    {
    case LndMk_PINPOINTS:
        map_flag = load_spritesheet("ldata/netflag_pin.dat", "ldata/netflag_pin.tab");
        break;
    default:
        ERRORLOG("Unsupported land markers type %d",(int)campaign.land_markers);
        // Fall Through
    case LndMk_ENSIGNS:
        map_flag = load_spritesheet("ldata/netflag_ens.dat", "ldata/netflag_ens.tab");
        break;
    }
    map_font = load_spritesheet("ldata/netfont.dat", "ldata/netfont.tab");
    map_hand = load_spritesheet("ldata/maphand.dat", "ldata/maphand.tab");
    if (!map_flag || !map_font || !map_hand)
    {
      ERRORLOG("Unable to load MAP SCREEN sprites");
      return false;
    }
    frontend_load_data_reset();
    frontnet_init_level_descriptions();
    frontmap_zoom_skip_init(SINGLEPLAYER_NOTSTARTED);
    fe_net_level_selected = SINGLEPLAYER_NOTSTARTED;
    net_level_hilighted = SINGLEPLAYER_NOTSTARTED;
    set_pointer_graphic_none();
    LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
    map_sound_fade = FULL_LOUDNESS;
    lbDisplay.DrawFlags = 0;
    set_music_volume(settings.music_volume);
    frontmap_start_music();
    if (fe_network_active)
    {
        net_number_of_players = 0;
        for (long i = 0; i < 4; i++)
        {
            struct ScreenPacket* nspck = &net_screen_packet[i];
            if ((nspck->networkstatus_flags & 0x01) != 0)
              net_number_of_players++;
        }
    } else
    {
      net_number_of_players = 1;
    }
    net_map_slap_frame = 0;
    net_map_limp_time = 0;
    update_net_ensigns_visibility();
    return true;
}

void process_map_zoom_in(void)
{
    step_frontmap_info_screen_shift_zoom();
    map_sound_fade = max(0, FULL_LOUDNESS + ((5 * (1 - map_info.fade_pos)) / FRONTMAP_ZOOM_STEP));
}

void process_map_zoom_out(void)
{
    step_frontmap_info_screen_shift_zoom();
}

void process_zoom_palette(void)
{
    SYNCDBG(8,"Starting");
    if (map_info.fade_step > 0)
    {
        if (map_info.fade_pos >= FRONTMAP_ZOOM_LENGTH/2)
        {
            LbPaletteFade(NULL, 29, Lb_PALETTE_FADE_OPEN);
        }
    } else
    if (map_info.fade_step < 0)
    {
        if (map_info.fade_pos > FRONTMAP_ZOOM_LENGTH/2)
        {
            if (map_info.fade_pos+map_info.fade_step > FRONTMAP_ZOOM_LENGTH/2)
            {
                LbPaletteFade(frontend_palette, 29, Lb_PALETTE_FADE_OPEN);
            } else
            {
                LbPaletteSet(frontend_palette);
            }
        }
    }
}

TbBool frontmap_update_zoom(void)
{
    SYNCDBG(8,"Starting");
    if (map_info.fade_step == FRONTMAP_ZOOM_STEP)
    {
        process_map_zoom_in();
    } else
    if (map_info.fade_step == -FRONTMAP_ZOOM_STEP)
    {
        process_map_zoom_out();
    }
    map_info.fade_pos += map_info.fade_step;
    if ((map_info.fade_pos <= 1) || (map_info.fade_pos >= FRONTMAP_ZOOM_LENGTH-1))
    {
        SYNCDBG(8,"Stopping fade");
        LbPaletteStopOpenFade();
        map_info.fadeflags &= ~MLInfoFlg_Zooming;
        if (map_info.state_trigger != FeSt_INITIAL)
        {
            frontend_set_state(map_info.state_trigger);
            LbScreenClear(0);
            LbScreenSwap();
            map_info.state_trigger = FeSt_INITIAL;
            return true;
        }
    }
    process_zoom_palette();
    return false;
}

TbBool frontmap_load(void)
{
    SYNCDBG(4,"Starting");
    memset(scratch, 0, PALETTE_SIZE);
    LbPaletteSet(scratch);
    initialize_description_speech();
    mouse_over_lvnum = SINGLEPLAYER_NOTSTARTED;
    frontend_load_data_from_cd();
    LevelNumber lvnum = get_continue_level_number();
    if (!load_map_and_window(lvnum))
    {
        frontend_load_data_reset();
        return false;
    }
    switch (campaign.land_markers) {
    case LndMk_PINPOINTS:
        map_flag = load_spritesheet("ldata/lndflag_pin.dat", "ldata/lndflag_pin.tab");
        break;
    default:
        ERRORLOG("Unsupported land markers type %d",(int)campaign.land_markers);
        // Fall through
    case LndMk_ENSIGNS:
        map_flag = load_spritesheet("ldata/lndflag_ens.dat", "ldata/lndflag_ens.tab");
        break;
    }
    if (!map_flag)
    {
        ERRORLOG("Unable to load Land View Screen sprites");
        frontend_load_data_reset();
        return false;
    }
    frontend_load_data_reset();
    struct PlayerInfo* player = get_my_player();
    lvnum = get_continue_level_number();
    if ((player->display_flags & PlaF6_PlyrHasQuit) != 0)
    {
        lvnum = get_loaded_level_number();
        frontmap_zoom_out_init(lvnum, lvnum);
    } else
    if ((lvnum == first_singleplayer_level()) || (player->victory_state == VicS_LostLevel) || (player->victory_state == VicS_State3))
    {
        frontmap_zoom_skip_init(lvnum);
        // Fading will be controlled by main frontend loop
        fade_palette_in = 1;
        play_desc_speech_time = LbTimerClock() + 1000;
    } else
    {
        frontmap_zoom_out_init(prev_singleplayer_level(lvnum), lvnum);
    }
    SYNCDBG(9,"Zoom hotspot set to (%d,%d) %s fade",(int)map_info.hotspot_imgpos_x,(int)map_info.hotspot_imgpos_y,(map_info.fadeflags & MLInfoFlg_Zooming)?"with":"without");
    map_sound_fade = FULL_LOUDNESS;
    map_info.velocity_x = 0;
    map_info.velocity_y = 0;
    set_pointer_graphic_spland(0);
    LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
    if ((features_enabled & Ft_AdvAmbSound) != 0)
    {
        // don't use play_non_3d_sample; we want looping, fading, and volume control
        SoundEmitterID emit_id = get_emitter_id(S3DGetSoundEmitter(Non3DEmitter));
        play_sample(emit_id, campaign.ambient_good, 0, 0x40, NORMAL_PITCH, -1, 2, 0);
        play_sample(emit_id, campaign.ambient_bad, 0, 0x40, NORMAL_PITCH, -1, 2, 0);
    }
    set_music_volume(settings.music_volume);
    frontmap_start_music();
    fe_computer_players = 0;
    update_ensigns_visibility();
    SYNCDBG(7,"Finished");
    return true;
}

TbBool rectangle_intersects(struct TbRect *rcta, struct TbRect *rctb)
{
    long left = rcta->left;
    if (rcta->left <= rctb->left)
      left = rctb->left;
    long top = rcta->top;
    if (top <= rctb->top)
      top = rctb->top;
    long right = rcta->right;
    if (right >= rctb->right)
      right = rctb->right;
    long bottom = rcta->bottom;
    if (bottom >= rctb->bottom)
      bottom = rctb->bottom;
    return (left < right) && (top < bottom);
}

TbBool test_hand_slap_collides(PlayerNumber plyr_idx)
{
  struct TbRect rctb;
  if (is_my_player_number(plyr_idx))
    return false;
  struct ScreenPacket* nspck = &net_screen_packet[my_player_number];
  if ((nspck->networkstatus_flags >> 3) == 0x02)
    return false;
  // Rectangle of given player
  nspck = &net_screen_packet[(int)plyr_idx];
  struct TbRect rcta;
  rcta.left = nspck->stored_data1 - 7;
  rcta.top = nspck->stored_data2 - 13;
  rcta.right = rcta.left + 30;
  rcta.bottom = rcta.top + 20;
  // Rectangle of local player
  nspck = &net_screen_packet[my_player_number];
  if ((nspck->networkstatus_flags >> 3) == 0x01)
  {
    rctb.left = nspck->stored_data1 - 31;
    rctb.top = nspck->stored_data2 - 27;
    rctb.right = get_sprite(map_hand, 9)->SWidth + rctb.left;
    rctb.bottom = rctb.top + get_sprite(map_hand, 9)->SHeight;
  } else
  if (nspck->param1 != SINGLEPLAYER_NOTSTARTED)
  {
    rctb.left = nspck->stored_data1 - 20;
    rctb.top = nspck->stored_data2 - 14;
    rctb.right = get_sprite(map_hand, 17)->SWidth + rctb.left;
    rctb.bottom = rctb.top + get_sprite(map_hand, 17)->SHeight;
  } else
  {
    rctb.left = nspck->stored_data1 - 19;
    rctb.top = nspck->stored_data2 - 25;
    rctb.right = get_sprite(map_hand, 1)->SWidth + rctb.left;
    rctb.bottom = rctb.top + get_sprite(map_hand, 1)->SHeight;
  }
  // Return if the rectangles are intersecting
  if (rectangle_intersects(&rcta, &rctb))
    return true;
  return false;
}

void frontmap_draw(void)
{
    SYNCDBG(8,"Starting");
    LbTextSetFont(map_font);
    LbTextSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
    if ((map_info.fadeflags & MLInfoFlg_Zooming) != 0)
    {
        frontzoom_to_point(map_info.hotspot_imgpos_x, map_info.hotspot_imgpos_y, map_info.fade_pos);
        compressed_window_draw();
    } else
    {
        draw_map_screen();
        draw_map_level_ensigns();
        set_pointer_graphic_spland(0);
        compressed_window_draw();
    }
}

void check_mouse_scroll(void)
{
    long mx = GetMouseX();
    if ( (mx < 8) || ( (is_game_key_pressed(Gkey_MoveLeft, NULL, false)) || (is_key_pressed(KC_LEFT,KMod_DONTCARE)) ) )
    {
        map_info.velocity_x -= 8;
        if (map_info.velocity_x < -48)
            map_info.velocity_x = -48;
        if (map_info.velocity_x > 48)
            map_info.velocity_x = 48;
  } else
  if ( (mx >= lbDisplay.PhysicalScreenWidth-8) || ( (is_game_key_pressed(Gkey_MoveRight, NULL, false)) || (is_key_pressed(KC_RIGHT,KMod_DONTCARE)) ) )
  {
    map_info.velocity_x += 8;
    if (map_info.velocity_x < -48)
      map_info.velocity_x = -48;
    if (map_info.velocity_x > 48)
      map_info.velocity_x = 48;
  }
  long my = GetMouseY();
  if ( (my < 8) || ( (is_game_key_pressed(Gkey_MoveUp, NULL, false)) || (is_key_pressed(KC_UP,KMod_DONTCARE)) ) )
  {
    map_info.velocity_y -= 8;
    if (map_info.velocity_y < -48)
      map_info.velocity_y = -48;
    if (map_info.velocity_y > 48)
      map_info.velocity_y = 48;
  } else
  if ( (my >= lbDisplay.PhysicalScreenHeight-8) || ( (is_game_key_pressed(Gkey_MoveDown, NULL, false)) || (is_key_pressed(KC_DOWN,KMod_DONTCARE)) ) )
  {
    map_info.velocity_y += 8;
    if (map_info.velocity_y < -48)
      map_info.velocity_y = -48;
    if (map_info.velocity_y > 48)
      map_info.velocity_y = 48;
  }
}

void update_velocity(void)
{
    if (map_info.velocity_x != 0)
    {
      map_info.screen_shift_x += map_info.velocity_x / 4;
      if (map_info.screen_shift_x > LANDVIEW_MAP_WIDTH - lbDisplay.PhysicalScreenWidth*16/units_per_pixel_landview)
        map_info.screen_shift_x = LANDVIEW_MAP_WIDTH - lbDisplay.PhysicalScreenWidth*16/units_per_pixel_landview;
      if (map_info.screen_shift_x < 0)
        map_info.screen_shift_x = 0;
      if (map_info.velocity_x < 0)
        map_info.velocity_x += 2;
      else
        map_info.velocity_x -= 2;
    }
    if (map_info.velocity_y != 0)
    {
      map_info.screen_shift_y += map_info.velocity_y / 4;
      if (map_info.screen_shift_y > LANDVIEW_MAP_HEIGHT - lbDisplay.PhysicalScreenHeight*16/units_per_pixel_landview)
        map_info.screen_shift_y = LANDVIEW_MAP_HEIGHT - lbDisplay.PhysicalScreenHeight*16/units_per_pixel_landview;
      if (map_info.screen_shift_y < 0)
        map_info.screen_shift_y = 0;
      if (map_info.velocity_y < 0)
        map_info.velocity_y += 2;
      else
        map_info.velocity_y -= 2;
    }
    // As we've changed non-precise coords, update the precise ones to match
    map_info.precise_scrshift_x = map_info.screen_shift_x << 8;
    map_info.precise_scrshift_y = map_info.screen_shift_y << 8;
}

/**
 * Draws player's hands.
 */
void draw_netmap_players_hands(void)
{
  struct ScreenPacket *nspck;
  const char *plyr_nam;
  const struct TbSprite *spr;
  TbPixel colr;
  long x;
  long y;
  long w;
  long h;
  long i;
  long k;
  long n;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
      nspck = &net_screen_packet[i];
      plyr_nam = network_player_name(i);
      colr = net_player_colours[i];
      if ((nspck->networkstatus_flags & 0x01) != 0)
      {
        x = 0;
        y = 0;
        n = nspck->networkstatus_flags & 0xF8;
        if (n == 8)
        {
          k = (unsigned char)nspck->param1;
          spr = get_sprite(map_hand, k);
        } else
        if (n == 16)
        {
          k = nspck->param2;
          if (k > 11)
            k = 11;
          if (k < 0)
            k = 0;
          x = hand_limp_xoffset[k];
          y = hand_limp_yoffset[k];
          spr = get_sprite(map_hand, 21);
        } else
        if (nspck->param1 == SINGLEPLAYER_NOTSTARTED)
        {
            k = LbTimerClock() / 150;
            spr = get_sprite(map_hand, 1 + (k%8));
        } else
        {
            k = LbTimerClock() / 150;
            spr = get_sprite(map_hand, 17 + (k%4));
        }
        x += nspck->stored_data1 - map_info.screen_shift_x - 18;
        y += nspck->stored_data2 - map_info.screen_shift_y - 25;
        LbSpriteDrawResized(scale_value_landview(x), scale_value_landview(y), units_per_pixel_landview, spr);
        w = LbTextStringWidth(plyr_nam);
        if (w > 0)
        {
          lbDisplay.DrawFlags = 0;
          h = LbTextHeight(level_name);
          y += 32;
          x += 32;
          LbDrawBox(scale_value_landview(x-4), scale_value_landview(y), scale_value_landview(w+8), scale_value_landview(h), colr);
          LbTextDrawResized(scale_value_landview(x), scale_value_landview(y), units_per_pixel_landview, plyr_nam);
        }
      }
  }
}

/**
 * Draws text description of active level.
 */
void draw_map_level_descriptions(void)
{
  if ((fe_net_level_selected > 0) || (net_level_hilighted > 0))
  {
    lbDisplay.DrawFlags = 0;
    LevelNumber lvnum = fe_net_level_selected;
    if (lvnum <= 0)
      lvnum = net_level_hilighted;
    struct LevelInformation* lvinfo = get_level_info(lvnum);
    if (lvinfo == NULL)
      return;
    const char* lv_name;
    if (lvinfo->name_stridx > 0)
        lv_name = get_string(lvinfo->name_stridx);
    else
      lv_name = lvinfo->name;
    if ((lv_name != NULL) && (strlen(lv_name) > 0)) {
      snprintf(level_name, sizeof(level_name), "%s %d: %s", get_string(GUIStr_MnuLevel), (int)lvinfo->lvnum, lv_name);
    } else {
      snprintf(level_name, sizeof(level_name), "%s %d", get_string(GUIStr_MnuLevel), (int)lvinfo->lvnum);
    }
    long w = LbTextStringWidth(level_name);
    long x = lvinfo->ensign_x - map_info.screen_shift_x;
    long y = lvinfo->ensign_y - map_info.screen_shift_y - 8;
    long h = LbTextHeight(level_name);
    LbDrawBox(scale_value_landview(x-4), scale_value_landview(y), scale_value_landview(w+8), scale_value_landview(h), 0);
    LbTextDrawResized(scale_value_landview(x), scale_value_landview(y), units_per_pixel_landview, level_name);
  }
}

void frontnetmap_draw(void)
{
    SYNCDBG(8,"Starting");
    LbTextSetFont(map_font);
    LbTextSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
    if ((map_info.fadeflags & MLInfoFlg_Zooming) != 0)
    {
        frontzoom_to_point(map_info.hotspot_imgpos_x, map_info.hotspot_imgpos_y, map_info.fade_pos);
        compressed_window_draw();
    } else
    {
        draw_map_screen();
        draw_map_level_ensigns();
        draw_netmap_players_hands();
        draw_map_level_descriptions();
        compressed_window_draw();
    }
}

void frontmap_input(void)
{
    SYNCDBG(8,"Starting");
    short zoom_done;
    if ((map_info.fadeflags & MLInfoFlg_SpeechAfterZoom) != 0)
    {
        if ((map_info.fadeflags & MLInfoFlg_Zooming) == 0)
        {
            map_info.fadeflags &= ~MLInfoFlg_SpeechAfterZoom;
            play_desc_speech_time = LbTimerClock() + 1000;
        }
        zoom_done = false;
    } else
    {
        zoom_done = true;
    }
    if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
    {
        clear_key_pressed(KC_ESCAPE);
        FrontendMenuState nstate = get_menu_state_when_back_from_substate(frontend_menu_state);
        frontend_set_state(nstate);
        LbPaletteStopOpenFade();
        return;
    }
    if (zoom_done)
    {
      check_mouse_scroll();
      if (is_key_pressed(KC_F11, KMod_CONTROL))
      {
        if (game.easter_eggs_enabled == true)
        {
          set_all_ensigns_state(LvSt_Visible);
          clear_key_pressed(KC_F11);
          return;
        }
      }
      if (is_key_pressed(KC_F10, KMod_CONTROL))
      {
        if (game.easter_eggs_enabled == true)
        {
          move_campaign_to_next_level();
          frontmap_unload();
          frontmap_load();
          //update_ensigns_visibility();
          clear_key_pressed(KC_F10);
          return;
        }
      }
      if (is_key_pressed(KC_F9, KMod_CONTROL))
      {
        if (game.easter_eggs_enabled == true)
        {
          move_campaign_to_prev_level();
          frontmap_unload();
          frontmap_load();
          //update_ensigns_visibility();
          clear_key_pressed(KC_F9);
          return;
        }
      }
      if (left_button_clicked)
      {
        left_button_clicked = 0;
        frontmap_input_active_ensign(left_button_clicked_x, left_button_clicked_y);
        if (clicked_map_level_ensign())
          return;
      }
      long mouse_x = GetMouseX();
      long mouse_y = GetMouseY();
      frontmap_input_active_ensign(mouse_x, mouse_y);
    }
    update_velocity();
}

void frontnetmap_input(void)
{
  if (lbKeyOn[KC_ESCAPE])
  {
      fe_net_level_selected = LEVELNUMBER_ERROR;
      lbKeyOn[KC_ESCAPE] = 0;
      SYNCLOG("Escaped from level selection");
      return;
  }

  if (net_map_limp_time == 0)
  {
      struct LevelInformation* lvinfo;
      if (right_button_clicked)
      {
          right_button_clicked = 0;
          if (fe_net_level_selected == SINGLEPLAYER_NOTSTARTED)
          {
              if ((!net_map_slap_frame) && (!net_map_limp_time))
              {
                  net_map_slap_frame = 9;
              }
          } else
        {
            lvinfo = get_level_info(fe_net_level_selected);
            if (lvinfo != NULL) {
              LbMouseSetPosition(scale_value_landview(lvinfo->ensign_x - map_info.screen_shift_x),
                  scale_value_landview(lvinfo->ensign_y - map_info.screen_shift_y));
            }
            fe_net_level_selected = SINGLEPLAYER_NOTSTARTED;
        }
    }

    if (fe_net_level_selected == SINGLEPLAYER_NOTSTARTED)
    {
      net_level_hilighted = SINGLEPLAYER_NOTSTARTED;
      frontmap_input_active_ensign(GetMouseX(), GetMouseY());
      if (mouse_over_lvnum > 0)
        net_level_hilighted = mouse_over_lvnum;
      if (net_level_hilighted > 0)
      {
        if ((net_map_slap_frame == 0) && (net_map_limp_time == 0))
        {
          if (left_button_clicked)
          {
              fe_net_level_selected = net_level_hilighted;
              left_button_clicked = 0;
              lvinfo = get_level_info(fe_net_level_selected);
              if (lvinfo != NULL) {
                snprintf(level_name, sizeof(level_name), "%s %d", get_string(GUIStr_MnuLevel), (int)lvinfo->lvnum);
              } else {
                snprintf(level_name, sizeof(level_name), "%s", get_string(GUIStr_MnuLevel));
              }
              SYNCLOG("Selected level %d with description \"%s\"",(int)fe_net_level_selected,level_name);
          }
        }
      }
      check_mouse_scroll();
      update_velocity();
    }
  }
}

void frontmap_unload(void)
{
    SYNCDBG(8,"Starting");
    set_pointer_graphic_none();
    unload_map_and_window();
    free_spritesheet(&map_flag);
    StopAllSamples();
    stop_description_speech();
    stop_music();
    set_music_volume(settings.music_volume);
}

long frontmap_update(void)
{
  SYNCDBG(8,"Starting");
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

  if ((map_info.fadeflags & MLInfoFlg_Zooming) != 0)
  {
      if (frontmap_update_zoom())
      {
          SYNCDBG(8,"Zoom end");
          return true;
      }
  }
  if (playing_good_descriptive_speech)
  {
    if (!speech_sample_playing())
    {
      playing_good_descriptive_speech = 0;
//      playing_speech_lvnum = SINGLEPLAYER_NOTSTARTED;
    }
  }
  SYNCDBG(8,"Finished");
  return 0;
}

TbBool frontmap_exchange_screen_packet(void)
{
    memset(net_screen_packet, 0, sizeof(net_screen_packet));
    struct ScreenPacket* nspck = &net_screen_packet[my_player_number];
    nspck->networkstatus_flags |= 0x01;
    nspck->param1 = fe_net_level_selected;
    if (net_map_limp_time > 0)
    {
      nspck->networkstatus_flags = (nspck->networkstatus_flags & 0x07) | 0x10;
      nspck->stored_data1 = limp_hand_x;
      nspck->stored_data2 = limp_hand_y;
      net_map_limp_time--;
      nspck->param2 = net_map_limp_time;
      if (net_map_limp_time == 1)
      {
          LbMouseSetPosition(
            limp_hand_x + hand_limp_xoffset[0] - map_info.screen_shift_x,
            limp_hand_y + hand_limp_yoffset[0] - map_info.screen_shift_y);
      }
    } else
    if (fe_net_level_selected > 0)
    {
        const struct TbSprite* spr = get_map_ensign(1);
        struct LevelInformation* lvinfo = get_level_info(fe_net_level_selected);
        if (lvinfo != NULL)
        {
            nspck->stored_data1 = lvinfo->ensign_x + my_player_number * ((long)spr->SWidth);
            nspck->stored_data2 = lvinfo->ensign_y - 48;
        }
    } else
    if (net_map_slap_frame > 0)
    {
        nspck->stored_data1 = GetMouseX()*16/units_per_pixel_landview + map_info.screen_shift_x;
        nspck->stored_data2 = GetMouseY()*16/units_per_pixel_landview + map_info.screen_shift_y;
        if (net_map_slap_frame <= 16)
        {
          nspck->networkstatus_flags = (nspck->networkstatus_flags & 0x07) | 0x08;
          nspck->param1 = net_map_slap_frame;
          net_map_slap_frame++;
        } else
        {
          net_map_slap_frame = 0;
        }
    } else
    {
        nspck->stored_data1 = GetMouseX()*16/units_per_pixel_landview + map_info.screen_shift_x;
        nspck->stored_data2 = GetMouseY()*16/units_per_pixel_landview + map_info.screen_shift_y;
    }
    if (fe_network_active)
    {
      if (LbNetwork_Exchange(NETMSG_FRONTEND, nspck, &net_screen_packet, sizeof(struct ScreenPacket)))
      {
          ERRORLOG("LbNetwork_Exchange failed");
          return false;
      }
    }
    return true;
}

TbBool frontnetmap_update_players(struct NetMapPlayersState * nmps)
{
    memset(scratch, 0, PALETTE_SIZE);
    long tmp2 = -1;
    for (long i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        struct ScreenPacket* nspck = &net_screen_packet[i];
        if ((nspck->networkstatus_flags & 0x01) == 0)
          continue;
        if (nspck->param1 == LEVELNUMBER_ERROR)
        {
            if (fe_network_active)
            {
              if (LbNetwork_EnableNewPlayers(1))
                ERRORLOG("Unable to enable new players joining exchange");
              frontend_set_state(FeSt_NET_START);
            } else
            {
              frontend_set_state(FeSt_MAIN_MENU);
            }
            return false;
        }
        if ((nspck->param1 == SINGLEPLAYER_NOTSTARTED) || ((nspck->networkstatus_flags & 0xF8) == 8))
        {
            nmps->tmp1++;
        } else
        {
            LevelNumber pckt_lvnum = nspck->param1;
            scratch[pckt_lvnum]++;
            if (scratch[pckt_lvnum] == tmp2)
            {
                nmps->is_selected = false;
            } else
            if (scratch[pckt_lvnum] > tmp2)
            {
                nmps->lvnum = pckt_lvnum;
                tmp2 = scratch[pckt_lvnum];
                nmps->is_selected = true;
            }
        }
        if (((nspck->networkstatus_flags & 0xF8) == 0x08) && (nspck->param1 == 13))
        {
            if ( test_hand_slap_collides(i) )
            {
                net_map_limp_time = 12;
                fe_net_level_selected = SINGLEPLAYER_NOTSTARTED;
                net_map_slap_frame = 0;
                limp_hand_x = nspck->stored_data1;
                limp_hand_y = nspck->stored_data2;
                nspck->networkstatus_flags = (nspck->networkstatus_flags & 7) | 0x10;
                SYNCLOG("Slapped out of level");
            }
        }
    }
    return true;
}

TbBool frontnetmap_update(void)
{
    SYNCDBG(8,"Starting");
    set_music_volume((map_sound_fade * settings.music_volume) / FULL_LOUDNESS);

    struct NetMapPlayersState nmps;
    nmps.tmp1 = 0;
    nmps.lvnum = SINGLEPLAYER_NOTSTARTED;
    nmps.is_selected = false;
    if ((map_info.fadeflags & MLInfoFlg_Zooming) != 0)
    {
        if (frontmap_update_zoom())
        {
          SYNCDBG(8,"Zoom end");
          return true;
        }
    } else
    {
        frontmap_exchange_screen_packet();
        frontnetmap_update_players(&nmps);
    }
    if ((!nmps.tmp1) && (nmps.lvnum > 0) && (nmps.is_selected))
    {
        set_selected_level_number(nmps.lvnum);
        snprintf(level_name, sizeof(level_name), "%s %d", get_string(GUIStr_MnuLevel), (int)nmps.lvnum);
        map_info.state_trigger = (fe_network_active < 1) ? FeSt_START_KPRLEVEL : FeSt_START_MPLEVEL;
        frontmap_zoom_in_init(nmps.lvnum);
        if (!fe_network_active)
            fe_computer_players = 1;
    }
    SYNCDBG(8,"Normal end");
    return false;
}


/******************************************************************************/
