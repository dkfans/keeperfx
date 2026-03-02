/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_landview.h
 *     Header file for front_landview.c.
 * @par Purpose:
 *     Land view, where the user can select map for campaign or multiplayer.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     16 Mar 2009 - 01 Apr 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_FRONT_LAND_H
#define DK_FRONT_LAND_H

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_sprite.h"
#include "config_campaigns.h"
#include "net_game.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define FRONTMAP_ZOOM_LENGTH 240
#define FRONTMAP_ZOOM_STEP 4
/******************************************************************************/
#pragma pack(1)

struct TbSprite;

enum MapLevelInfoFlags {
    MLInfoFlg_None            =  0x00,
    MLInfoFlg_Zooming         =  0x01,
    MLInfoFlg_SpeechAfterZoom =  0x02,
};

struct MapLevelInfo { // sizeof = 56
  unsigned char fadeflags;
  float fade_step;
  float fade_pos;
  float hotspot_imgpos_x; /**< Position of the chosen level ensign zoom area, which is either being zoomed in to or zoomed out from. Stored as land view background bitmap coordinate. */
  float hotspot_imgpos_y;
  long state_trigger;
  float screen_shift_x; /**< Shift X coordinate for top left corner of the visible land picture area. Acts as the final shift in both zoom and non-zoom modes. */
  float screen_shift_y; /**< Shift Y coordinate for top left corner of the visible land picture area. */
  float precise_scrshift_x; /**< Precise shift X for top left corner of the visible land picture area. Extended precision version, used as source for scrshift_x while zooming. */
  float precise_scrshift_y; /**< Precise shift Y for top left corner of the visible land picture area. */
  float velocity_x; /**< Velocity at which screen_shift_x is being changed. */
  float velocity_y; /**< Velocity at which screen_shift_y is being changed. */
  float hotspot_shift_x; /**< Position of the chosen level ensign zoom area, which is either being zoomed in to or zoomed out from. Set to top left corner of an area which would have the ensign in center. */
  float hotspot_shift_y;
  float screen_shift_aimed_x; /**< Shift X coordinate at which the screen_shift is aiming towards zooming. */
  float screen_shift_aimed_y;
};

struct ScreenPacket {
  unsigned char networkstatus_flags;
  char frontend_alliances;
  short stored_data1; // Can contain: VersionRelease (networking) or hand_position_x (landview)
  short stored_data2; // Can contain: VersionBuild (networking) or hand_position_y (landview)
  short param1;
  unsigned char param2;
};

/******************************************************************************/
extern TbClockMSec play_desc_speech_time;
extern unsigned long played_bad_descriptive_speech;
extern unsigned long played_good_descriptive_speech;
extern struct TbSpriteSheet *map_flag;
extern struct TbSpriteSheet *map_font;
extern struct TbSpriteSheet *map_hand;
extern long map_sound_fade;
extern unsigned char *map_screen;
extern long fe_net_level_selected;
extern long net_map_limp_time;
extern struct ScreenPacket net_screen_packet[NET_PLAYERS_COUNT];
extern long players_currently_in_session;

#pragma pack()
/******************************************************************************/
extern struct MapLevelInfo map_info;

extern long map_window_len;
/******************************************************************************/
void frontnetmap_unload(void);
TbBool frontnetmap_load(void);
void frontnetmap_input(void);
void frontnetmap_draw(void);
TbBool frontnetmap_update(void);
void frontmap_input(void);
void frontmap_draw(void);
TbBool frontmap_load(void);
void frontmap_unload(void);
long frontmap_update(void);
void frontzoom_to_point(long a1, long a2, long a3);
void compressed_window_draw(void);
void frontnet_init_level_descriptions(void);
const struct TbSprite *get_ensign_sprite_for_level(struct LevelInformation *lvinfo, int anim_frame);

TbBool initialize_description_speech(void);
TbBool play_current_description_speech(short play_good);
TbBool play_description_speech(LevelNumber lvnum, short play_good);
void check_mouse_scroll(void);
void update_velocity(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
