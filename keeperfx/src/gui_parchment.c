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
#include "bflib_planar.h"

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
#include "room_workshop.h"
#include "frontmenu_ingame_tabs.h"
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
      fname = prepare_file_path(FGrp_StdData,"gmap64.raw");
      LbFileLoadAt(fname, hires_parchment);
  } else
  {
      fname = prepare_file_path(FGrp_StdData,"gmap32.raw");
      LbFileLoadAt(fname, poly_pool);
  }
  parchment_loaded = 1;
}

long get_parchment_background_area_rect(struct TbRect *bkgnd_area)
{
    int img_width, img_height;
    if (LbScreenWidth() < 640)
    {
        img_width = 320;
        img_height = 200;
    } else
    {
        img_width = 640;
        img_height = 480;
    }
    int rect_w, rect_h;
    rect_w = LbScreenWidth();
    rect_h = LbScreenHeight();
    // Parchment bitmap scaling
    int units_per_px, units_per_px_max;
    units_per_px = max(16*rect_w/img_width, 16*rect_h/img_height);
    units_per_px_max = min(16*7*rect_w/(6*img_width), 16*4*rect_h/(3*img_height));
    if (units_per_px > units_per_px_max)
        units_per_px = units_per_px_max;
    // The image width can't be larger than video resolution
    if (units_per_px < 1) {
        units_per_px = 1;
    }
    // Set rectangle coords
    bkgnd_area->left = (rect_w-units_per_px*img_width/16)/2;
    bkgnd_area->top = (rect_h-units_per_px*img_height/16)/2;
    if (bkgnd_area->top < 0) bkgnd_area->top = 0;
    bkgnd_area->right = bkgnd_area->left + units_per_px*img_width/16;
    bkgnd_area->bottom = bkgnd_area->top + units_per_px*img_height/16;
    if (bkgnd_area->bottom > rect_h) bkgnd_area->bottom = rect_h;
    return units_per_px;
}

long get_parchment_map_area_rect(struct TbRect *map_area)
{
    struct TbRect bkgnd_area;
    get_parchment_background_area_rect(&bkgnd_area);
    long bkgnd_width, bkgnd_height;
    bkgnd_width = bkgnd_area.right - bkgnd_area.left;
    bkgnd_height = bkgnd_area.bottom - bkgnd_area.top;
    long block_size;
    block_size = min((bkgnd_width - bkgnd_width/3) / map_tiles_x, (bkgnd_height - bkgnd_height/8) / map_tiles_y);
    if (block_size < 1) block_size = 1;
    map_area->left = bkgnd_area.left + (bkgnd_width - block_size*map_tiles_x) / 2;
    map_area->top = bkgnd_area.top + 3 * (bkgnd_height - block_size*map_tiles_y) / 4;
    map_area->right = map_area->left + block_size*map_tiles_x;
    map_area->bottom = map_area->top + block_size*map_tiles_y;
    return block_size;
}

TbBool point_to_overhead_map(const struct Camera *camera, const long screen_x, const long screen_y, long *map_x, long *map_y)
{
    // Sizes of the parchment map on which we are
    long block_size;
    struct TbRect map_area;
    block_size = get_parchment_map_area_rect(&map_area);
    // Check if we're within coordinates with the screen position
    *map_x = 0;
    *map_y = 0;
    if ((screen_x >= map_area.left) && (screen_x < map_area.right)
      && (screen_y >= map_area.top) && (screen_y < map_area.bottom))
    {
        *map_x = COORD_PER_STL * STL_PER_SLB * (screen_x-map_area.left) / block_size + COORD_PER_STL/2;
        *map_y = COORD_PER_STL * STL_PER_SLB * (screen_y-map_area.top)  / block_size + COORD_PER_STL/2;
        return ((*map_x >= 0) && (*map_x < (map_subtiles_x+1)<<8) && (*map_y >= 0) && (*map_y < (map_subtiles_y+1)<<8));
    }
    return false;
}

TbBool parchment_copy_background_at(const struct TbRect *bkgnd_area, int units_per_px)
{
    int img_width;
    int img_height;
    unsigned char *srcbuf;
    if (LbScreenWidth() < 640)
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
    // Only 8bpp supported for now
    if (LbGraphicsScreenBPP() != 8)
        return false;
    // Do the drawing
    copy_raw8_image_buffer(lbDisplay.WScreen,LbGraphicsScreenWidth(),LbGraphicsScreenHeight(),
        img_width*units_per_px/16,img_height*units_per_px/16,bkgnd_area->left,bkgnd_area->top,srcbuf,img_width,img_height);
    // Burning candle flames
    const struct TbSprite *spr;
    spr = &button_sprite[198+(game.play_gameturn & 3)];
    LbSpriteDrawScaled(bkgnd_area->left+(36*units_per_px/(16*pixel_size)),(bkgnd_area->top+0*units_per_px/(16*pixel_size)), spr, spr->SWidth*units_per_px/16, spr->SHeight*units_per_px/16);
    spr = &button_sprite[202+(game.play_gameturn & 3)];
    LbSpriteDrawScaled(bkgnd_area->left+(574*units_per_px/(16*pixel_size)),(bkgnd_area->top+0*units_per_px/(16*pixel_size)), spr, spr->SWidth*units_per_px/16, spr->SHeight*units_per_px/16);
    return true;
}

/**
 * Draws parchment view background, used for in-game level map screen.
 */
void draw_map_parchment(void)
{
    // Get background area rectangle
    struct TbRect bkgnd_area;
    int units_per_px;
    units_per_px = get_parchment_background_area_rect(&bkgnd_area);
    // Draw it
    parchment_copy_background_at(&bkgnd_area, units_per_px);
    SYNCDBG(9,"Done");
}

TbPixel get_overhead_mapblock_color(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, TbPixel background)
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
          pixval = player_highlight_colours[owner];
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
            pixval = player_highlight_colours[owner];
          } else
          if (thing->byte_18)
          {
            pixval = 79;
          } else
          {
            pixval = 60;
          }
      } else
      if ((mapblk->flags & MapFlg_IsTall) == 0)
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

void draw_overhead_map(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    unsigned char *dstline;
    unsigned char *dstbuf;
    long cntr_h,cntr_w;
    long stl_x,stl_y;
    long line;
    long k;
    line = 0;
    stl_y = 1;
    dstline = &lbDisplay.WScreen[map_area->left + lbDisplay.GraphicsScreenWidth * map_area->top];
    for (cntr_h = map_tiles_y*block_size; cntr_h > 0; cntr_h--)
    {
        if ((line > 0) && ((line % block_size) == 0))
        {
          stl_y += STL_PER_SLB;
        }
        dstbuf = dstline;
        stl_x = 1;
        for (cntr_w=map_tiles_x; cntr_w > 0; cntr_w--)
        {
          for (k = block_size; k > 0; k--)
          {
            *dstbuf = get_overhead_mapblock_color(stl_x,stl_y,plyr_idx,*dstbuf);
            dstbuf++;
          }
          stl_x += STL_PER_SLB;
        }
        dstline += lbDisplay.GraphicsScreenWidth;
        line++;
    }
    lbDisplay.DrawFlags = 0;
}

void draw_overhead_room_icons(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    struct Room *room;
    long rkind_select;
    long room_visibility;
    //_DK_draw_overhead_room_icons(x,y);
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
            if (subtile_revealed(room->central_stl_x, room->central_stl_y, plyr_idx))
            {
                struct RoomData *rdata;
                rdata = room_data_get_for_room(room);
                if (rdata->numfield_1 > 0)
                {
                    struct TbSprite *spr;
                    long pos_x,pos_y;
                    spr = &gui_panel_sprites[rdata->numfield_1];
                    pos_x = map_area->left + (block_size * room->central_stl_x / STL_PER_SLB) - (spr->SWidth  / 2);
                    pos_y = map_area->top  + (block_size * room->central_stl_y / STL_PER_SLB) - (spr->SHeight / 2);
                    LbSpriteDraw(pos_x, pos_y, spr);
                }
            }
          }
        }
    }
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
}

int draw_overhead_call_to_arms(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    int i,n;
    n = 0;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        if (player_uses_call_to_arms(i))
        {
            struct Dungeon *dungeon;
            dungeon = get_dungeon(i);
            lbDisplay.DrawFlags = Lb_SPRITE_UNKNOWN0010;
            struct MagicStats *magstat;
            magstat = &game.keeper_power_stats[PwrK_CALL2ARMS];
            long m;
            m = (4 * ((i + game.play_gameturn) & 7) * subtile_slab_fast(magstat->strength[dungeon->cta_splevel]));
            long pos_x,pos_y,radius;
            pos_x = map_area->left + block_size * (int)dungeon->cta_stl_x / STL_PER_SLB;
            pos_y = map_area->top  + block_size * (int)dungeon->cta_stl_y / STL_PER_SLB;
            radius = (((m&7) + m) >> 3);
            LbDrawCircle(pos_x, pos_y, radius/pixel_size, player_room_colours[i]);
            n++;
        }
    }
    return n;
}

int draw_overhead_creatures(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    int i,k,n;
    n = 0;
    k = 0;
    i = game.thing_lists[TngList_Creatures].index;
    while (i != 0)
    {
        struct Thing *thing;
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
            col1 = player_highlight_colours[thing->owner];
            col2 = 1;
            if (thing_revealed(thing, plyr_idx))
            {
                if ((game.play_gameturn & 4) == 0)
                {
                    col2 = player_room_colours[thing->owner];
                    col1 = player_room_colours[thing->owner];
                }
                long pos_x,pos_y;
                pos_x = map_area->left + block_size * (int)thing->mappos.x.stl.num / STL_PER_SLB;
                pos_y = map_area->top  + block_size * (int)thing->mappos.y.stl.num / STL_PER_SLB;
                if (thing->owner == plyr_idx)
                {
                    LbDrawPixel(pos_x, pos_y, col2);
                } else
                {
                    LbDrawPixel(pos_x, pos_y, col1);
                }
                n++;
            }
            // Special tunneler code
            if (thing->model == get_players_special_digger_model(game.hero_player_num))
            {
                long m;
                m = get_creature_state_besides_move(thing);
                if ( (m == CrSt_Tunnelling) || (m == CrSt_TunnellerDoingNothing) )
                {
                    struct CreatureControl *cctrl;
                    cctrl = creature_control_get_from_thing(thing);
                    for (m=0; m < 5; m++)
                    {
                        long memberpos;
                        memberpos = cctrl->party.member_pos_stl[m];
                        if (memberpos == 0)
                            break;
                        long pos_x,pos_y;
                        pos_x = map_area->left + block_size * stl_num_decode_x(memberpos) / STL_PER_SLB;
                        pos_y = map_area->top  + block_size * stl_num_decode_y(memberpos) / STL_PER_SLB;
                        LbDrawPixel(pos_x, pos_y, col1);
                        n++;
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
    return n;
}

int draw_overhead_traps(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    struct Thing *thing;
    int i,k,n;
    n = 0;
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
            if (thing->owner == plyr_idx)
            {
                if ( (thing->byte_18) || (thing->owner == plyr_idx) )
                {
                    long pos_x,pos_y;
                    pos_x = map_area->left + block_size * (int)thing->mappos.x.stl.num / STL_PER_SLB;
                    pos_y = map_area->top  + block_size * (int)thing->mappos.y.stl.num / STL_PER_SLB;
                    LbDrawPixel(pos_x, pos_y, 60);
                    LbDrawPixel(pos_x + 1, pos_y, 60);
                    LbDrawPixel(pos_x - 1, pos_y, 60);
                    LbDrawPixel(pos_x, pos_y + 1, 60);
                    LbDrawPixel(pos_x, pos_y - 1, 60);
                    n++;
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
    return n;
}

int draw_overhead_spells(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    int i,k,n;
    n = 0;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Object);
    k = 0;
    i = slist->index;
    while (i != 0)
    {
        struct Thing *thing;
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
            if (thing_revealed(thing, plyr_idx))
            {
              if ( thing_is_special_box(thing) || thing_is_spellbook(thing) )
              {
                  long pos_x,pos_y;
                  pos_x = map_area->left + block_size * (int)thing->mappos.x.stl.num / STL_PER_SLB;
                  pos_y = map_area->top  + block_size * (int)thing->mappos.y.stl.num / STL_PER_SLB;
                  LbDrawPixel(pos_x, pos_y, colours[15][0][15]);
                  n++;
              }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return n;
}

void draw_overhead_things(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    //_DK_draw_overhead_things(x,y);
    draw_overhead_creatures(map_area, block_size, plyr_idx);
    draw_overhead_call_to_arms(map_area, block_size, plyr_idx);
    if ((game.play_gameturn & 3) == 1) {
        draw_overhead_spells(map_area, block_size, plyr_idx);
    }
    draw_overhead_traps(map_area, block_size, plyr_idx);
}

void draw_2d_map(void)
{
  struct PlayerInfo *player;
  SYNCDBG(8,"Starting");
  //_DK_draw_2d_map();
  player = get_my_player();
  // Size of the parchment map on which we're drawing
  long block_size;
  struct TbRect map_area;
  block_size = get_parchment_map_area_rect(&map_area);
  // Now draw
  draw_overhead_map(&map_area, block_size, player->id_number);
  draw_overhead_things(&map_area, block_size, player->id_number);
  draw_overhead_room_icons(&map_area, block_size, player->id_number);
}

void draw_map_level_name(void)
{
    struct LevelInformation *lvinfo;
    LevelNumber lvnum;
    const char *lv_name;
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
    struct TbRect bkgnd_area;
    get_parchment_background_area_rect(&bkgnd_area);
    // Set position
    LbTextSetFont(winfont);
    lbDisplay.DrawFlags = 0;
    int x,y,w,h;
    x = bkgnd_area.left;
    y = bkgnd_area.top;
    w = bkgnd_area.right - bkgnd_area.left;
    h = (bkgnd_area.bottom - bkgnd_area.top)/4;
    // Drawing
    if (lv_name != NULL)
    {
        LbTextSetWindow(x, y, w, h);
        LbTextDraw((w-LbTextStringWidth(lv_name))/2, (h/10 - 8), lv_name);
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
      struct ManufactureData *manufctr;
      spos_x = ((subtile_size * ((long)thing->mappos.x.stl.pos)) >> 8);
      spos_y = ((subtile_size * ((long)thing->mappos.y.stl.pos)) >> 8);
      switch (thing->class_id)
      {
      case TCls_Creature:
        spridx = get_creature_model_graphics(thing->model,CGI_GUIPanelSymbol);
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
        manufctr = get_manufacture_data_for_thing(thing->class_id, thing->model);
        spridx = manufctr->parchment_spridx;
        draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        break;
      case TCls_Object:
        if (thing_is_dungeon_heart(thing))
        {
            spridx = 512;
            draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        } else
        if (object_is_gold(thing))
        {
            spridx = 511;
            draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        } else
        if (thing_is_special_box(thing))
        {
            spridx = 164;
            draw_gui_panel_sprite_centered(scr_x+spos_x, scr_y+spos_y, spridx);
        } else
        if (thing_is_spellbook(thing))
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
    player = get_my_player();

    long draw_tiles_x,draw_tiles_y;
    draw_tiles_x = 13;
    draw_tiles_y = 13;

    // Sizes of the parchment map on which we're drawing
    // Needed only to figure out map position pointed by cursor
    long block_size;
    struct TbRect map_area;
    block_size = get_parchment_map_area_rect(&map_area);
    // Mouse coordinates
    long mouse_x,mouse_y;
    mouse_x = GetMouseX();
    mouse_y = GetMouseY();

    struct Map *mapblk;
    const int subtile_size = 8;
    int map_dx,map_dy;
    int scr_x,scr_y;
    int stl_x,stl_y;
    int k;

    lbDisplay.DrawFlags = 0;
    // Drawing coordinates
    long scrtop_x,scrtop_y;
    scrtop_x = mouse_x + 24;
    scrtop_y = mouse_y + 24;
    // Source map coordinates
    stl_x = STL_PER_SLB * (mouse_x/pixel_size-map_area.left) / block_size - draw_tiles_x/2;
    stl_y = STL_PER_SLB * (mouse_y/pixel_size-map_area.top)  / block_size - draw_tiles_y/2;
    // Draw only on map area (do not allow zoom box to be empty)
    if ((stl_x < -draw_tiles_x+4) || (stl_x >= map_subtiles_x+1-draw_tiles_x+6)
     || (stl_y < -draw_tiles_y+4) || (stl_y >= map_subtiles_x+1-draw_tiles_y+6))
      return;

    scrtop_x += 4;
    scrtop_y -= 4;
    setup_vecs(lbDisplay.WScreen, 0, lbDisplay.GraphicsScreenWidth, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    if (scrtop_y > MyScreenHeight-draw_tiles_y*subtile_size)
      scrtop_y = MyScreenHeight-draw_tiles_y*subtile_size;
    if (scrtop_y < 0)
        scrtop_y = 0;
    // Draw the actual map
    scr_y = scrtop_y;
    for (map_dy=0; map_dy < draw_tiles_y; map_dy++)
    {
      scr_x = scrtop_x;
      for (map_dx=0; map_dx < draw_tiles_x; map_dx++)
      {
        mapblk = get_map_block_at(stl_x+map_dx,stl_y+map_dy);
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
        (draw_tiles_x*subtile_size)/pixel_size, (draw_tiles_y*subtile_size)/pixel_size, 0);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_UNKNOWN0010;
    // Draw thing sprites on the map
    LbScreenSetGraphicsWindow( (scrtop_x+2)/pixel_size, (scrtop_y+2)/pixel_size,
        (draw_tiles_x*subtile_size-4)/pixel_size, (draw_tiles_y*subtile_size-4)/pixel_size);
    scr_y = 0;
    for (map_dy=0; map_dy < draw_tiles_y; map_dy++)
    {
      scr_x = 0;
      for (map_dx=0; map_dx < draw_tiles_x; map_dx++)
      {
        mapblk = get_map_block_at(stl_x+map_dx,stl_y+map_dy);
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

void redraw_minimal_overhead_view(void)
{
    draw_map_parchment();
    draw_2d_map();
    draw_gui();
    draw_tooltip();
}

void zoom_to_patchment_map(void)
{
    struct PlayerInfo *player;
    turn_off_all_window_menus();
    if ((game.numfield_C & 0x20) == 0)
      set_flag_byte(&game.numfield_C,0x40,false);
    else
      set_flag_byte(&game.numfield_C,0x40,true);
    player = get_my_player();
    if (((game.system_flags & GSF_NetworkActive) != 0)
        || (lbDisplay.PhysicalScreenWidth > 320))
    {
      if (!toggle_status_menu(0))
        set_flag_byte(&game.numfield_C,0x40,false);
      set_players_packet_action(player, PckA_Unknown119, 4, 0, 0, 0);
      turn_off_roaming_menus();
    } else
    {
      set_players_packet_action(player, PckA_Unknown080, 5, 0, 0, 0);
      turn_off_roaming_menus();
    }
}

void zoom_from_patchment_map(void)
{
    struct PlayerInfo *player;
    player = get_my_player();
    if (((game.system_flags & GSF_NetworkActive) != 0)
        || (lbDisplay.PhysicalScreenWidth > 320))
    {
        if ((game.numfield_C & 0x40) != 0)
          toggle_status_menu(1);
        set_players_packet_action(player, PckA_Unknown120,1,0,0,0);
    } else
    {
        set_players_packet_action(player, PckA_Unknown080,6,0,0,0);
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
