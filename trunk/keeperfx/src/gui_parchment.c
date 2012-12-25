/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_parchment.c
 *     The map parchment screen support functions.
 * @par Purpose:
 *     Functions to display and maintain Parchment view (map view) during
 *     the gameplay.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     23 May 2010 - 10 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_parchment.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_vidraw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_dernc.h"
#include "frontend.h"
#include "front_simple.h"
#include "config.h"
#include "gui_boxmenu.h"
#include "gui_tooltips.h"
#include "gui_draw.h"
#include "kjm_input.h"
#include "engine_render.h"
#include "map_data.h"
#include "map_blocks.h"
#include "player_data.h"
#include "config_strings.h"
#include "config_campaigns.h"
#include "config_creature.h"
#include "thing_data.h"
#include "thing_objects.h"
#include "thing_traps.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "power_hand.h"
#include "game_legacy.h"
#include "room_list.h"
#include "vidfade.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_draw_map_parchment(void);
DLLIMPORT void _DK_draw_2d_map(void);
DLLIMPORT void _DK_draw_overhead_room_icons(long x, long y);
DLLIMPORT void _DK_draw_overhead_things(long x, long y);
DLLIMPORT void _DK_draw_zoom_box(void);

/******************************************************************************/
void load_parchment_file(void)
{
  if ( !parchment_loaded )
  {
    reload_parchment_file(lbDisplay.PhysicalScreenWidth >= 640);
  }
}

void reload_parchment_file(TbBool hires)
{
  char *fname;
  if (hires)
  {
    fname=prepare_file_path(FGrp_StdData,"gmaphi.raw");
    LbFileLoadAt(fname, hires_parchment);
  } else
  {
    fname=prepare_file_path(FGrp_StdData,"gmap.raw");
    LbFileLoadAt(fname, poly_pool);
  }
  parchment_loaded = 1;
}

void parchment_copy_background_at(int rect_x,int rect_y,int rect_w,int rect_h)
{
  int img_width;
  int img_height;
  unsigned char *srcbuf;
  if (lbDisplay.GraphicsScreenWidth < 640)
  {
    img_width = 320;
    img_height = 200;
    srcbuf = poly_pool;
  } else
  {
    img_width = 640;
    img_height = 480;
    srcbuf = hires_parchment;
  }
  TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(LbScreenActiveMode());
  int m;
  int spx,spy;
  // Only 8bpp supported for now
  if (LbGraphicsScreenBPP() != 8)
    return;
  if (rect_w == POS_AUTO)
    rect_w = mdinfo->Width-rect_x;
  if (rect_h == POS_AUTO)
    rect_h = mdinfo->Height-rect_y;
  if (rect_w<0) rect_w=0;
  if (rect_h<0) rect_h=0;
  // Parchment bitmap can't be scaled
  m = 1;
  // Starting point coords
  spx=0;spy=0; // disabled, for now
/*
  spx = rect_x + ((rect_w-m*img_width)>>1);
  spy = rect_y + ((rect_h-m*img_height)>>1);
  if (spy<0) spy=0;
*/
  // Do the drawing
  copy_raw8_image_buffer(lbDisplay.WScreen,LbGraphicsScreenWidth(),LbGraphicsScreenHeight(),
      spx,spy,srcbuf,img_width,img_height,m);
  // Burning candle flames
  LbSpriteDraw(spx+(36/pixel_size),(spy+0/pixel_size), &button_sprite[198+(game.play_gameturn & 3)]);
  LbSpriteDraw(spx+(574/pixel_size),(spy+0/pixel_size), &button_sprite[202+(game.play_gameturn & 3)]);
}

/**
 * Draws parchment view background, used for in-game level map screen.
 */
void draw_map_parchment(void)
{
  parchment_copy_background_at(0,0,POS_AUTO,POS_AUTO);
}

TbPixel get_overhead_mapblock_color(long stl_x,long stl_y,long plyr_idx,TbPixel background)
{
  struct Thing *thing;
  struct SlabMap *slb;
  struct Room *room;
  struct Map *mapblk;
  long owner;
  TbPixel pixval;
  mapblk = get_map_block_at(stl_x, stl_y);
  slb = get_slabmap_for_subtile(stl_x,stl_y);
  owner = slabmap_owner(slb);
  if ((((mapblk->flags & MapFlg_Unkn04) != 0) || ((mapblk->flags & MapFlg_Unkn80) != 0))
      && ((game.play_gameturn & 4) != 0))
  {
    pixval = pixmap.ghost[background + 0x1A00];
  } else
  if ((mapblk->flags & MapFlg_Unkn01) != 0)
  {
    pixval = pixmap.ghost[background + 0x8C00];
  } else
  if (!map_block_revealed(mapblk,plyr_idx))
  {
    pixval = background;
  } else
  if ((mapblk->flags & MapFlg_IsRoom) != 0) // Room slab
  {
    room = subtile_room_get(stl_x, stl_y);
    if (((game.play_gameturn & 1) != 0) && (room->kind == gui_room_type_highlighted))
    {
      pixval = 31;
    } else
    if (owner == game.neutral_player_num)
    {
      pixval = player_room_colours[game.play_gameturn & 3];
    } else
    {
      pixval = player_room_colours[owner];
    }
  } else
  {
    if (slb->kind == SlbT_ROCK)
    {
      pixval = 0;
    } else
    if ((mapblk->flags & MapFlg_Unkn20) != 0)
    {
      pixval = pixmap.ghost[background + 0x1000];
    } else
    if ((mapblk->flags & MapFlg_IsDoor) != 0) // Door slab
    {
      thing = get_door_for_position(stl_x, stl_y);
      if (thing_is_invalid(thing))
      {
        pixval = 60;
      } else
      if ((game.play_gameturn & 1) && (thing->model == gui_door_type_highlighted))
      {
        pixval = 31;
      } else
      if (thing->byte_18)
      {
        pixval = 79;
      } else
      {
        pixval = 60;
      }
    } else
    if ((mapblk->flags & MapFlg_Unkn10) == 0)
    {
      if (slb->kind == SlbT_LAVA)
      {
        pixval = 146;
      } else
      if (slb->kind == SlbT_WATER)
      {
        pixval = 85;
      } else
      if (owner == game.neutral_player_num)
      {
        pixval = 4;
      } else
      {
        pixval = player_path_colours[owner];
      }
    } else
    {
      pixval = background;
    }
  }
  return pixval;
}

void draw_overhead_map(long plyr_idx)
{
  long block_size;
  unsigned char *dstline;
  unsigned char *dstbuf;
  long cntr_h,cntr_w;
  long stl_x,stl_y;
  long line;
  long k;
  block_size = 4 / pixel_size;
  if (block_size < 1) block_size = 1;
  line = 0;
  stl_y = 1;
  dstline = &lbDisplay.WScreen[150/pixel_size + lbDisplay.GraphicsScreenWidth * 56/pixel_size];
  for (cntr_h = 85*block_size; cntr_h > 0; cntr_h--)
  {
    if ((line > 0) && ((line % block_size) == 0))
    {
      stl_y += 3;
    }
    dstbuf = dstline;
    stl_x = 1;
    for (cntr_w=85; cntr_w > 0; cntr_w--)
    {
      for (k = block_size; k > 0; k--)
      {
        *dstbuf = get_overhead_mapblock_color(stl_x,stl_y,plyr_idx,*dstbuf);
        dstbuf++;
      }
      stl_x += 3;
    }
    dstline += lbDisplay.GraphicsScreenWidth;
    line++;
  }
  lbDisplay.DrawFlags = 0;
}

void draw_overhead_room_icons(long x, long y)
{
    struct Room *room;
    long rkind_select;
    long room_visibility;
    struct RoomData *rdata;
    struct PlayerInfo *player;
    //_DK_draw_overhead_room_icons(x,y);
    player = get_my_player();
    rkind_select = (game.play_gameturn >> 1) % ROOM_TYPES_COUNT;
    for (room = start_rooms; room < end_rooms; room++)
    {
      if (room_exists(room))
      {
          room_visibility = abs(rkind_select - room->kind);
          if ((room_visibility < 2) || (room_visibility >= 4))
            lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
          else
              lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
          if (room_visibility < 4)
          {
            if (subtile_revealed(room->central_stl_x, room->central_stl_y, player->id_number))
            {
                rdata = room_data_get_for_room(room);
                if (rdata->numfield_1 > 0)
                {
                    struct TbSprite *spr;
                    long pos_x,pos_y;
                    spr = &gui_panel_sprites[rdata->numfield_1];
                    pos_x = (4 * room->central_stl_x / 3) - (pixel_size * spr->SWidth  / 2) + x;
                    pos_y = (4 * room->central_stl_y / 3) - (pixel_size * spr->SHeight / 2) + y;
                    LbSpriteDraw(pos_x/pixel_size, pos_y/pixel_size, spr);
                }
            }
          }
        }
    }
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
}

void draw_overhead_things(long x, long y)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    long pos_x,pos_y,radius;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    int i,k;
    long n;
    //_DK_draw_overhead_things(x,y);
    player = get_my_player();

    k = 0;
    i = game.thing_lists[TngList_Creatures].index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (!thing_is_picked_up(thing))
        {
            TbPixel col1,col2;
            col1 = 31;
            col2 = 1;
            if (subtile_revealed(thing->mappos.x.stl.num, thing->mappos.y.stl.num, player->id_number))
            {
                if ((game.play_gameturn & 4) == 0)
                {
                    col2 = player_room_colours[thing->owner];
                    col1 = player_room_colours[thing->owner];
                }
                pos_x = x + (4 * (long)thing->mappos.x.stl.num / 3);
                pos_y = y + (4 * (long)thing->mappos.y.stl.num / 3);
                if (player->id_number == thing->owner)
                {
                    LbDrawPixel(pos_x/pixel_size, pos_y/pixel_size, col2);
                } else
                {
                    LbDrawPixel(pos_x/pixel_size, pos_y/pixel_size, col1);
                }
            }
            // Special tunneler code
            if (thing->model == get_players_special_digger_breed(game.hero_player_num))
            {
                n = get_creature_state_besides_move(thing);
                if ( (n == CrSt_Tunnelling) || (n == CrSt_TunnellerDoingNothing) )
                {
                    for (n=0; n < 5; n++)
                    {
                        long memberpos;
                        cctrl = creature_control_get_from_thing(thing);
                        memberpos = cctrl->party.word_90[n];
                        if (memberpos == 0)
                            break;
                        pos_x = x + 4 * stl_num_decode_x(memberpos) / 3;
                        pos_y = y + 4 * stl_num_decode_y(memberpos) / 3;
                        LbDrawPixel(pos_x/pixel_size, pos_y/pixel_size, col1);
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        dungeon = get_dungeon(i);
        if (dungeon->field_884 > 0)
        {
            struct MagicStats *magstat;
            lbDisplay.DrawFlags = Lb_SPRITE_UNKNOWN0010;
            magstat = &game.magic_stats[PwrK_CALL2ARMS];
            k = (4 * ((i + game.play_gameturn) & 7) * subtile_slab_fast(magstat->power[dungeon->field_883]));
            pos_x = x + (4 * subtile_slab_fast(dungeon->field_881));
            pos_y = y + (4 * subtile_slab_fast(dungeon->field_882));
            radius = (((k&7) + k) >> 3);
            LbDrawCircle(pos_x/pixel_size, pos_y/pixel_size, radius/pixel_size, player_room_colours[i]);
        }
    }
    if ((game.play_gameturn & 3) == 1)
    {
        k = 0;
        i = game.thing_lists[TngList_Objects].index;
        while (i != 0)
        {
            thing = thing_get(i);
            if (thing_is_invalid(thing))
            {
              ERRORLOG("Jump to invalid thing detected");
              break;
            }
            i = thing->next_of_class;
            // Per-thing code
            if (!thing_is_picked_up(thing))
            {
                if (subtile_revealed(thing->mappos.x.stl.num, thing->mappos.y.stl.num, player->id_number))
                {
                  if ( thing_is_special_box(thing) || thing_is_spellbook(thing) )
                  {
                      pos_x = x + (4 * (long)thing->mappos.x.stl.num / 3);
                      pos_y = y + (4 * (long)thing->mappos.y.stl.num / 3);
                      LbDrawPixel(pos_x/pixel_size, pos_y/pixel_size, colours[15][0][15]);
                  }
                }
            }
            // Per-thing code ends
            k++;
            if (k > THINGS_COUNT)
            {
              ERRORLOG("Infinite loop detected when sweeping things list");
              break;
            }
        }
    }
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (!thing_is_picked_up(thing))
        {
            if (player->id_number == thing->owner)
            {
                if ( (thing->byte_18) || (thing->owner == player->id_number) )
                {
                    pos_x = x + (4 * (long)thing->mappos.x.stl.num / 3);
                    pos_y = y + (4 * (long)thing->mappos.y.stl.num / 3);
                    LbDrawPixel(pos_x/pixel_size, pos_y/pixel_size, 60);
                    LbDrawPixel(pos_x/pixel_size + 1, pos_y/pixel_size, 60);
                    LbDrawPixel(pos_x/pixel_size - 1, pos_y/pixel_size, 60);
                    LbDrawPixel(pos_x/pixel_size, pos_y/pixel_size + 1, 60);
                    LbDrawPixel(pos_x/pixel_size, pos_y/pixel_size - 1, 60);
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
}

void draw_2d_map(void)
{
  struct PlayerInfo *player;
  SYNCDBG(8,"Starting");
  //_DK_draw_2d_map();
  player = get_my_player();
  draw_overhead_map(player->id_number);
  draw_overhead_things(150, 56);
  draw_overhead_room_icons(150, 56);
}

void draw_map_level_name(void)
{
  struct LevelInformation *lvinfo;
  LevelNumber lvnum;
  const char *lv_name;
  int x,y,w,h;
  // Retrieving name
  lv_name = NULL;
  lvnum = get_loaded_level_number();
  lvinfo = get_level_info(lvnum);
  if (lvinfo != NULL)
  {
    if (lvinfo->name_id > 0)
      lv_name = cmpgn_string(lvinfo->name_id);
    else
      lv_name = lvinfo->name;
  } else
  if (is_multiplayer_level(lvnum))
  {
    lv_name = level_name;
  }
  // Retrieving position
  x = 0;
  y = 0;
  w = 640;//MyScreenWidth;
  h = MyScreenHeight;
  // Drawing
  if (lv_name != NULL)
  {
    LbTextSetFont(winfont);
    lbDisplay.DrawFlags = 0;
    LbTextSetWindow(x/pixel_size, y/pixel_size, w/pixel_size, h/pixel_size);
    LbTextDraw((w-pixel_size*LbTextStringWidth(lv_name))/2 / pixel_size, 32 / pixel_size, lv_name);
  }
}

void draw_zoom_box_things_on_mapblk(struct Map *mapblk,unsigned short subtile_size,int scr_x,int scr_y)
{
  struct PlayerInfo *player;
  struct SpellData *pwrdata;
  struct Thing *thing;
  int spos_x,spos_y;
  TbPixel color;
  long spridx;
  unsigned long k;
  long i;
  player = get_my_player();
  k = 0;
  i = get_mapwho_thing_index(mapblk);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      WARNLOG("Jump out of things array");
      break;
    }
    i = thing->next_on_mapblk;
    if (!thing_is_picked_up(thing))
    {
      spos_x = ((subtile_size * ((long)thing->mappos.x.stl.pos)) >> 8);
      spos_y = ((subtile_size * ((long)thing->mappos.y.stl.pos)) >> 8);
      switch (thing->class_id)
      {
      case TCls_Creature:
        spridx = get_creature_breed_graphics(thing->model,CGI_HandSymbol);
        if ((game.play_gameturn & 0x04) != 0)
        {
          color = get_player_path_colour(thing->owner);
          draw_gui_panel_sprite_occentered(scr_x+spos_x, scr_y+spos_y, spridx, color);
        } else
        {
          draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        }
        draw_status_sprites((spos_x+scr_x)/pixel_size - 10, (spos_y+scr_y-20)/pixel_size, thing, 4096);
        break;
      case TCls_Trap:
        if ((!thing->byte_18) && (player->id_number != thing->owner))
          break;
        spridx = trap_data[thing->model].parchment_spridx;
        draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        break;
      case TCls_Object:
        if (thing->model == 5)
        {
          spridx = 512;
          draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        } else
        if (object_is_gold(thing))
        {
          spridx = 511;
          draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        } else
        if ( thing_is_special_box(thing) )
        {
          spridx = 164;
          draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        } else
        if ( thing_is_spellbook(thing) )
        {
          pwrdata = get_power_data(book_thing_to_magic(thing));
          spridx = pwrdata->field_B;
          draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        }
        break;
      default:
        break;
      }
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
}

/**
 * Draws a box near mouse with more detailed top view of map.
 * Requires screen to be locked before.
 */
void draw_zoom_box(void)
{
    //_DK_draw_zoom_box(); return;
    struct PlayerInfo *player;
    struct Map *mapblk;
    const int subtile_size = 8;
    long map_tiles_x,map_tiles_y;
    long scrtop_x,scrtop_y;
    long mouse_x,mouse_y;
    int map_dx,map_dy;
    int scr_x,scr_y;
    int map_x,map_y;
    int k;

    map_tiles_x = 13;
    map_tiles_y = 13;

    mouse_x = GetMouseX();
    mouse_y = GetMouseY();

    lbDisplay.DrawFlags = 0;
    scrtop_x = mouse_x + 24;
    scrtop_y = mouse_y + 24;
    map_x = (3*mouse_x-450) / 4 - 6;
    map_y = (3*mouse_y-168) / 4 - 6;

    // Draw only on map area
    if ((map_x < -map_tiles_x+4) || (map_x >= map_subtiles_x+1-map_tiles_x+6)
     || (map_y < -map_tiles_y+4) || (map_y >= map_subtiles_x+1-map_tiles_y+6))
      return;

    scrtop_x += 4;
    scrtop_y -= 4;
    setup_vecs(lbDisplay.WScreen, 0, lbDisplay.GraphicsScreenWidth, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    if (scrtop_y > MyScreenHeight-map_tiles_y*subtile_size)
      scrtop_y = MyScreenHeight-map_tiles_y*subtile_size;
    if (scrtop_y < 0)
        scrtop_y = 0;
    player = get_my_player();
    // Draw the actual map
    scr_y = scrtop_y;
    for (map_dy=0; map_dy < map_tiles_y; map_dy++)
    {
      scr_x = scrtop_x;
      for (map_dx=0; map_dx < map_tiles_x; map_dx++)
      {
        mapblk = get_map_block_at(map_x+map_dx,map_y+map_dy);
        if (map_block_revealed(mapblk, player->id_number))
        {
          k = element_top_face_texture(mapblk);
          draw_texture(scr_x, scr_y, subtile_size, subtile_size, k, 0, -1);
        } else
        {
          LbDrawBox(scr_x/pixel_size, scr_y/pixel_size, 8/pixel_size, 8/pixel_size, 1);
        }
        scr_x += subtile_size;
      }
      scr_y += subtile_size;
    }
    lbDisplay.DrawFlags |= Lb_SPRITE_UNKNOWN0010;
    LbDrawBox(scrtop_x/pixel_size, scrtop_y/pixel_size,
        (map_tiles_x*subtile_size)/pixel_size, (map_tiles_y*subtile_size)/pixel_size, 0);
    set_flag_word(&lbDisplay.DrawFlags,Lb_SPRITE_UNKNOWN0010,false);
    // Draw thing sprites on the map
    LbScreenSetGraphicsWindow( (scrtop_x+2)/pixel_size, (scrtop_y+2)/pixel_size,
        (map_tiles_x*subtile_size-4)/pixel_size, (map_tiles_y*subtile_size-4)/pixel_size);
    scr_y = 0;
    for (map_dy=0; map_dy < map_tiles_y; map_dy++)
    {
      scr_x = 0;
      for (map_dx=0; map_dx < map_tiles_x; map_dx++)
      {
        mapblk = get_map_block_at(map_x+map_dx,map_y+map_dy);
        if (map_block_revealed(mapblk, player->id_number))
        {
          draw_zoom_box_things_on_mapblk(mapblk,subtile_size,scr_x,scr_y);
        }
        scr_x += subtile_size;
      }
      scr_y += subtile_size;
    }
    // Draw sprites surrounding the box
    LbScreenSetGraphicsWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    LbSpriteDraw((scrtop_x-24)/pixel_size, (scrtop_y-20)/pixel_size, &button_sprite[194]);
    LbSpriteDraw((scrtop_x+54)/pixel_size, (scrtop_y-20)/pixel_size, &button_sprite[195]);
    LbSpriteDraw((scrtop_x-24)/pixel_size, (scrtop_y+50)/pixel_size, &button_sprite[196]);
    LbSpriteDraw((scrtop_x+54)/pixel_size, (scrtop_y+50)/pixel_size, &button_sprite[197]);
    // Finish
    LbScreenSetGraphicsWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}

void redraw_parchment_view(void)
{
  SYNCDBG(5,"Starting");
  // Load and draw background
  load_parchment_file();
  draw_map_parchment();
  // Draw top view of the map
  draw_2d_map();
  // Draw on-screen GUIs and boxes
  draw_gui();
  gui_draw_all_boxes();
  // Put zoom box, map name and tooltips
  draw_zoom_box();
  draw_map_level_name();
  draw_tooltip();
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
