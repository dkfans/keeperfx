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

#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_filelst.h"
#include "bflib_dernc.h"
#include "bflib_keybrd.h"
#include "bflib_vidraw.h"
#include "bflib_mouse.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "config.h"
#include "engine_render.h"
#include "player_data.h"
#include "kjm_input.h"
#include "front_simple.h"
#include "frontend.h"
#include "vidfade.h"

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
DLLIMPORT extern struct DoorDesc _DK_doors[TORTURE_DOORS_COUNT];
#define doors _DK_doors
DLLIMPORT extern TortureState _DK_torture_state;
#define torture_state _DK_torture_state
DLLIMPORT extern unsigned char *_DK_torture_background;
#define torture_background _DK_torture_background
DLLIMPORT extern unsigned char *_DK_torture_palette;
#define torture_palette _DK_torture_palette
DLLIMPORT extern struct TbSprite *_DK_fronttor_sprites;
#define fronttor_sprites _DK_fronttor_sprites
DLLIMPORT extern struct TbSprite *_DK_fronttor_end_sprites;
#define fronttor_end_sprites _DK_fronttor_end_sprites
DLLIMPORT extern unsigned long _DK_fronttor_data;
#define fronttor_data _DK_fronttor_data
DLLIMPORT extern unsigned long _DK_fronttor_end_data;
#define fronttor_end_data _DK_fronttor_end_data
/******************************************************************************/
long torture_doors_available = TORTURE_DOORS_COUNT;

struct TbLoadFiles torture_load_files[] = {
  {"ldata/fronttor.tab", (unsigned char **)&fronttor_sprites, (unsigned char **)&fronttor_end_sprites, 0, 0, 0},
  {"ldata/fronttor.dat", (unsigned char **)&fronttor_data,    (unsigned char **)&fronttor_end_data,    0, 0, 0},
  {"",                    NULL,                                NULL,                                   0, 0, 0},
};

struct TbSetupSprite setup_torture_sprites[] = {
  {&doors[0].sprites, &doors[0].sprites_end, &doors[0].data},
  {&doors[1].sprites, &doors[1].sprites_end, &doors[1].data},
  {&doors[2].sprites, &doors[2].sprites_end, &doors[2].data},
  {&doors[3].sprites, &doors[3].sprites_end, &doors[3].data},
  {&doors[4].sprites, &doors[4].sprites_end, &doors[4].data},
  {&doors[5].sprites, &doors[5].sprites_end, &doors[5].data},
  {&doors[6].sprites, &doors[6].sprites_end, &doors[6].data},
  {&doors[7].sprites, &doors[7].sprites_end, &doors[7].data},
  {&doors[8].sprites, &doors[8].sprites_end, &doors[8].data},
  {&fronttor_sprites, &fronttor_end_sprites, &fronttor_data},
  {NULL,              NULL,                  NULL,}
};

/******************************************************************************/
DLLIMPORT void _DK_fronttorture_update(void);
DLLIMPORT void _DK_fronttorture_draw(void);
DLLIMPORT void _DK_fronttorture_unload(void);
DLLIMPORT void _DK_fronttorture_load(void);
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
  struct DoorDesc *door;
  long i;
  for (i=0; i < torture_doors_available; i++)
  {
    door = &doors[i];
    if ((x >= door->pos_x) && (x < door->pos_x+door->width))
      if ((y >= door->pos_y) && (y < door->pos_y+door->height))
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
  clear_light_system();
  clear_computer();
  clear_things_and_persons_data();
  clear_mapmap();
  clear_slabs();
  clear_rooms();
  clear_dungeons();
}

void fronttorture_load(void)
{
  struct PlayerInfo *player;
  char *fname;
  unsigned char *ptr;
  long i,k;
  wait_for_cd_to_be_available();
  frontend_load_data_from_cd();
  memcpy(frontend_backup_palette, &frontend_palette, PALETTE_SIZE);
  ptr = block_mem;
  // Load RAW/PAL background
  fname = prepare_file_path(FGrp_LoData,"torture.raw");
  torture_background = ptr;
  i = LbFileLoadAt(fname, ptr);
  ptr += i;
  fname = prepare_file_path(FGrp_LoData,"torture.pal");
  torture_palette = ptr;
  i = LbFileLoadAt(fname, ptr);
  ptr += i;

  // Load DAT/TAB sprites for doors
  k = 0;
  {
    fname = prepare_file_fmtpath(FGrp_LoData,"door%02d.dat",k+1);
    i = LbFileLoadAt(fname, ptr);
    doors[k].data = (unsigned long)ptr;
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
    doors[k].data = (unsigned long)ptr;
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
    doors[k].data = (unsigned long)ptr;
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
  torture_state = 0;
  torture_door_selected = -1;
  torture_end_sprite = -1;
  torture_sprite_direction = 0;
  memset(door_sound_state, 0, 0x48u);

  player = get_my_player();
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
  struct TbSprite *spr;
  const int img_width = 640;
  const int img_height = 480;
  int w,h,m,i;
  int spx,spy;
  // Only 8bpp supported for now
  if (LbGraphicsScreenBPP() != 8)
    return false;
  w=0;
  h=0;
  m=1;
  {
    w+=img_width;
    h+=img_height;
  }
  // Starting point coords
  //TODO: temporarely set to top left corner because input function is not rewritten
  spx = 0;//(mdinfo->Width-m*img_width)>>1;
  spy = 0;//(mdinfo->Height-m*img_height)>>1;
  copy_raw8_image_buffer(lbDisplay.WScreen,LbGraphicsScreenWidth(),LbGraphicsScreenHeight(),
      spx,spy,torture_background,img_width,img_height,m);

  for (i=0; i < torture_doors_available; i++)
  {
    if (i == torture_door_selected)
    {
      spr = &doors[i].sprites[torture_sprite_frame];
    } else
    {
      spr = &doors[i].sprites[1];
    }
    LbSpriteDraw(spx+doors[i].field_0, spy+doors[i].field_4, spr);
  }
  return true;
}

void fronttorture_input(void)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  long x,y;
  long plyr_idx,door_id;
  clear_packets();
  player = get_my_player();
  pckt = get_packet(my_player_number);
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
    pckt->field_6 = GetMouseX();
    pckt->field_8 = GetMouseY();
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
    x = pckt->field_6;
    y = pckt->field_8;
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
  door_id = torture_door_over_point(x,y);
  if ((torture_door_selected != -1) && (torture_door_selected != door_id))
    door_id = -1;
  // Make the action
  if (door_id == -1)
    torture_left_button = 0;
  switch (torture_state)
  {
  case 0:
      if (door_id != -1)
      {
        torture_state = 1;
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
          torture_state = 2;
          torture_sprite_frame = 8;
          torture_end_sprite = 4;
          torture_sprite_direction = -1;
        } else
        if ((pckt->action & 6) != 0)
        {
          torture_state = 3;
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
        torture_state = 0;
        torture_door_selected = -1;
      }
      break;
  case 3:
      if (torture_sprite_frame == torture_end_sprite)
      {
        if (((pckt->action & 0x04) == 0) || (door_id == -1))
        {
          torture_state = 4;
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
        torture_state = 1;
        torture_sprite_frame = 7;
        torture_end_sprite = 7;
      }
      break;
  }
}

void fronttorture_update(void)
{
  _DK_fronttorture_update();
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
