/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_torture.c
 *     Torture screen displaying routines.
 * @par Purpose:
 *     Functions to show and maintain the torture screen.
 *     Torture screen is a bonus, available after the game which has finished
 *     with imprisoning Lord of the Land.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 May 2009 - 20 Jun 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "front_torture.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_filelst.h"
#include "bflib_dernc.h"
#include "bflib_keybrd.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_mouse.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "config.h"
#include "engine_render.h"
#include "engine_textures.h"
#include "game_lghtshdw.h"
#include "player_data.h"
#include "room_list.h"
#include "kjm_input.h"
#include "front_simple.h"
#include "frontend.h"
#include "vidmode.h"
#include "vidfade.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT extern long _DK_torture_left_button;
#define torture_left_button _DK_torture_left_button
DLLIMPORT extern long _DK_torture_sprite_direction;
#define torture_sprite_direction _DK_torture_sprite_direction
DLLIMPORT extern long _DK_torture_end_sprite;
#define torture_end_sprite _DK_torture_end_sprite
DLLIMPORT extern long _DK_torture_sprite_frame;
#define torture_sprite_frame _DK_torture_sprite_frame
DLLIMPORT extern long _DK_torture_door_selected;
#define torture_door_selected _DK_torture_door_selected
DLLIMPORT extern struct DoorSoundState _DK_door_sound_state[TORTURE_DOORS_COUNT];
#define door_sound_state _DK_door_sound_state
DLLIMPORT extern struct TortureState _DK_torture_state;
#define torture_state _DK_torture_state
DLLIMPORT extern unsigned char *_DK_torture_background;
#define torture_background _DK_torture_background
DLLIMPORT extern unsigned char *_DK_torture_palette;
#define torture_palette _DK_torture_palette
/******************************************************************************/
extern struct TbLoadFiles torture_load_files[];
extern struct TbSetupSprite setup_torture_sprites[];

long torture_doors_available = TORTURE_DOORS_COUNT;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void torture_play_sound(long door_id, TbBool state)
{
  if ((door_id < 0) || (door_id >= TORTURE_DOORS_COUNT))
    return;
  if (state)
  {
    play_sample_using_heap(0, doors[door_id].field_28, 0, 64, 100, -1, 2, 0);
    door_sound_state[door_id].field_0 = 0;
    door_sound_state[door_id].field_4 = 16;
  }
  else
  {
    door_sound_state[door_id].field_4 = -16;
  }
}

long torture_door_over_point(long x,long y)
{
    int units_per_px = min(units_per_pixel, units_per_pixel_min * 16 / 10);
    const int img_width = 640;
    const int img_height = 480;
    int w = img_width * units_per_px / 16;
    int h = img_height * units_per_px / 16;
    // Starting point coords
    int spx = (LbScreenWidth() - w) >> 1;
    int spy = (LbScreenHeight() - h) >> 1;
    for (long i = 0; i < torture_doors_available; i++)
    {
        struct DoorDesc* door = &doors[i];
        if ((x >= spx + door->pos_x * units_per_px / 16) && (x < spx + door->pos_x * units_per_px / 16 + door->width * units_per_px / 16))
            if ((y >= spy + door->pos_y * units_per_px / 16) && (y < spy + door->pos_y * units_per_px / 16 + door->height * units_per_px / 16))
                return i;
    }
    return -1;
}

void fronttorture_unload(void)
{
  LbDataFreeAll(torture_load_files);
  memcpy(&frontend_palette, frontend_backup_palette, PALETTE_SIZE);
  StopAllSamples();
  // Clearing the space used for torture graphics
  clear_light_system(&game.lish);
  clear_computer();
  clear_things_and_persons_data();
  clear_mapmap();
  clear_slabs();
  clear_rooms();
  clear_dungeons();
}

void fronttorture_load(void)
{
    wait_for_cd_to_be_available();
    frontend_load_data_from_cd();
    memcpy(frontend_backup_palette, &frontend_palette, PALETTE_SIZE);
    // Texture blocks memory isn't used here, so reuse it instead of allocating
    unsigned char* ptr = block_mem;
    // Load RAW/PAL background
    char* fname = prepare_file_path(FGrp_LoData, "torture.raw");
    torture_background = ptr;
    long i = LbFileLoadAt(fname, ptr);
    ptr += i;
    fname = prepare_file_path(FGrp_LoData,"torture.pal");
    torture_palette = ptr;
    i = LbFileLoadAt(fname, ptr);
    ptr += i;

    // Load DAT/TAB sprites for doors
    long k = 0;
    {
        fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.dat",k+1);
        i = LbFileLoadAt(fname, ptr);
        doors[k].data = ptr;
        ptr += i;
        doors[k].data_end = ptr;

        fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.tab",k+1);
        i = LbFileLoadAt(fname, ptr);
        doors[k].sprites = (struct TbSprite *)ptr;
        ptr += i;
        doors[k].sprites_end =(struct TbSprite *) ptr;
    }
    ptr = &game.land_map_start;
    for (k=1; k < 8; k++)
    {
        fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.dat",k+1);
        i = LbFileLoadAt(fname, ptr);
        doors[k].data = ptr;
        ptr += i;
        doors[k].data_end = ptr;
        fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.tab",k+1);
        i = LbFileLoadAt(fname, ptr);
        doors[k].sprites = (struct TbSprite *)ptr;
        ptr += i;
        doors[k].sprites_end = (struct TbSprite *)ptr;
    }
    ptr = poly_pool;
    k = 8;
    {
        fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.dat",k+1);
        i = LbFileLoadAt(fname, ptr);
        doors[k].data = ptr;
        ptr += i;
        doors[k].data_end = ptr;
        fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.tab",k+1);
        i = LbFileLoadAt(fname, ptr);
        doors[k].sprites = (struct TbSprite *)ptr;
        ptr += i;
        doors[k].sprites_end = (struct TbSprite *)ptr;
    }

    if ( LbDataLoadAll(torture_load_files) )
        ERRORLOG("Unable to load torture load files");
    LbSpriteSetupAll(setup_torture_sprites);
    frontend_load_data_reset();
    memcpy(&frontend_palette, torture_palette, PALETTE_SIZE);
    torture_state.action = 0;
    torture_door_selected = -1;
    torture_end_sprite = -1;
    torture_sprite_direction = 0;
    LbMemorySet(door_sound_state, 0, TORTURE_DOORS_COUNT*sizeof(struct DoorSoundState));

    struct PlayerInfo* player = get_my_player();
    if (player->victory_state == VicS_WonLevel)
    {
        LbMouseChangeSpriteAndHotspot(&fronttor_sprites[1], 0, 0);
    } else
    {
        LbMouseChangeSpriteAndHotspot(0, 0, 0);
    }
    torture_left_button = 0;
}

TbBool fronttorture_draw(void)
{
  const int img_width = 640;
  const int img_height = 480;
  // Only 8bpp supported for now
  if (LbGraphicsScreenBPP() != 8)
    return false;
  int units_per_px = min(units_per_pixel, units_per_pixel_min * 16 / 10);
  int w = img_width * units_per_px / 16;
  int h = img_height * units_per_px / 16;
  // Starting point coords
  int spx = (LbScreenWidth() - w) >> 1;
  int spy = (LbScreenHeight() - h) >> 1;
  copy_raw8_image_buffer(lbDisplay.WScreen,LbGraphicsScreenWidth(),LbGraphicsScreenHeight(),
      w,h,spx,spy,torture_background,img_width,img_height);

  for (int i = 0; i < torture_doors_available; i++)
  {
      struct TbSprite* spr;
      if (i == torture_door_selected)
      {
          spr = &doors[i].sprites[torture_sprite_frame];
    } else
    {
      spr = &doors[i].sprites[1];
    }
    LbSpriteDrawResized(spx + doors[i].pos_spr_x*units_per_px/16, spy + doors[i].pos_spr_y*units_per_px/16, units_per_px, spr);
  }
  return true;
}

void fronttorture_clear_state(void)
{
    torture_state.action = 0;
    torture_door_selected = -1;
}

void fronttorture_input(void)
{
    long x;
    long y;
    PlayerNumber plyr_idx;
    clear_packets();
    struct PlayerInfo* player = get_my_player();
    struct Packet* pckt = get_packet(my_player_number);
    // Get inputs and create packet
    if (player->victory_state == VicS_WonLevel)
    {
        if (left_button_clicked)
        {
            torture_left_button = 1;
            left_button_clicked = 0;
        }
        if ((lbKeyOn[KC_SPACE]) || (lbKeyOn[KC_RETURN]) || (lbKeyOn[KC_ESCAPE]))
        {
            lbKeyOn[KC_SPACE] = 0;
            lbKeyOn[KC_RETURN] = 0;
            lbKeyOn[KC_ESCAPE] = 0;
            pckt->action |= 0x01;
        }
        if (torture_left_button)
            pckt->action |= 0x02;
        if (left_button_held)
            pckt->action |= 0x04;
        pckt->actn_par1 = GetMouseX();
        pckt->actn_par2 = GetMouseY();
    }
    // Exchange packet with other players
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        if (LbNetwork_Exchange(pckt))
            ERRORLOG("LbNetwork_Exchange failed");
    }
    // Determine the controlling player and get his mouse coords
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        player = get_player(plyr_idx);
        pckt = get_packet(plyr_idx);
        if ((pckt->action != 0) && (player->victory_state == VicS_WonLevel))
            break;
    }
    if (plyr_idx < PLAYERS_COUNT)
    {
        x = pckt->actn_par1;
        y = pckt->actn_par2;
    } else
    {
        plyr_idx = my_player_number;
        player = get_player(plyr_idx);
        pckt = get_packet(plyr_idx);
        x = 0;
        y = 0;
    }
    if ((pckt->action & 0x01) != 0)
    {
        frontend_set_state(FeSt_LEVEL_STATS);
        if ((game.system_flags & GSF_NetworkActive) != 0)
            LbNetwork_Stop();
        return;
    }
    // Get active door
    long door_id = torture_door_over_point(x, y);
    if ((torture_door_selected != -1) && (torture_door_selected != door_id))
        door_id = -1;
    // Make the action
    if (door_id == -1)
      torture_left_button = 0;
    switch (torture_state.action)
    {
    case 0:
        if (door_id != -1)
        {
          torture_state.action = 1;
          torture_sprite_direction = 1;
          torture_door_selected = door_id;
          torture_sprite_frame = 3;
          torture_end_sprite = 7;
        }
        break;
    case 1:
        if (torture_sprite_frame == torture_end_sprite)
        {
          if (door_id == -1)
          {
            torture_state.action = 2;
            torture_sprite_frame = 8;
            torture_end_sprite = 4;
            torture_sprite_direction = -1;
          } else
          if ((pckt->action & (0x02|0x04)) != 0)
          {
            torture_state.action = 3;
            torture_left_button = 0;
            torture_sprite_frame = 7;
            torture_end_sprite = 11;
            torture_sprite_direction = 1;
            torture_play_sound(torture_door_selected, true);
          }
        }
        break;
    case 2:
        if (torture_sprite_frame == torture_end_sprite)
        {
          torture_state.action = 0;
          torture_door_selected = -1;
        }
        break;
    case 3:
        if (torture_sprite_frame == torture_end_sprite)
        {
          if (((pckt->action & 0x04) == 0) || (door_id == -1))
          {
            torture_state.action = 4;
            torture_sprite_frame = 12;
            torture_end_sprite = 8;
            torture_sprite_direction = -1;
            torture_play_sound(torture_door_selected, false);
          }
        }
        break;
    case 4:
        if (torture_sprite_frame == torture_end_sprite)
        {
          torture_state.action = 1;
          torture_sprite_frame = 7;
          torture_end_sprite = 7;
        }
        break;
    }
}

void fronttorture_update(void)
{
    if (torture_state.action != 0)
    {
      if ( torture_sprite_frame != torture_end_sprite )
        torture_sprite_frame += torture_sprite_direction;
    }
    for (int i = 0; i < TORTURE_DOORS_COUNT; i++)
    {
        struct DoorDesc* door = &doors[i];
        struct DoorSoundState* doorsnd = &door_sound_state[i];
        if ( doorsnd->field_4 )
        {
            int volume = doorsnd->field_4 + doorsnd->field_0;
            if (volume <= 0)
            {
                volume = 0;
                doorsnd->field_4 = 0;
                StopSample(0, door->field_28);
            } else
            if (volume >= 127)
            {
                volume = 127;
                doorsnd->field_4 = 0;
            }
            doorsnd->field_0 = volume;
            if (volume > 0)
            {
              SetSampleVolume(0, door->field_28, volume, 0);
            }
        }
    }
}
/******************************************************************************/
