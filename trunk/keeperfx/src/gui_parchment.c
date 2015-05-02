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
#include "config_terrain.h"
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
#ifdef SPRITE_FORMAT_V2
      fname = prepare_file_fmtpath(FGrp_StdData,"gmap-%d.raw",64);
#else
      fname = prepare_file_path(FGrp_StdData,"gmap64.raw");
#endif
      LbFileLoadAt(fname, hires_parchment);
  } else
  {
#ifdef SPRITE_FORMAT_V2
      fname = prepare_file_fmtpath(FGrp_StdData,"gmap-%d.raw",32);
#else
      fname = prepare_file_path(FGrp_StdData,"gmap32.raw");
#endif
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
    if ((((mapblk->flags & SlbAtFlg_Unk04) != 0) || ((mapblk->flags & SlbAtFlg_Unk80) != 0))
        && ((game.play_gameturn & 4) != 0))
    {
        pixval = pixmap.ghost[background + 0x1A00];
    } else
    if ((mapblk->flags & SlbAtFlg_Valuable) != 0)
    {
        pixval = pixmap.ghost[background + 0x8C00];
    } else
    if (!map_block_revealed(mapblk,plyr_idx))
    {
        pixval = background;
    } else
    if ((mapblk->flags & SlbAtFlg_IsRoom) != 0) // Room slab
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
      if ((mapblk->flags & SlbAtFlg_Filled) != 0)
      {
          pixval = pixmap.ghost[background + 0x1000];
      } else
      if ((mapblk->flags & SlbAtFlg_IsDoor) != 0) // Door slab
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
      if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
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
    int ps_units_per_px;
    {
        struct TbSprite *spr;
        spr = &gui_panel_sprites[57];
        ps_units_per_px = 32 * block_size * 4 / spr->SHeight;
    }
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
                if (rdata->medsym_sprite_idx > 0)
                {
                    struct TbSprite *spr;
                    long pos_x,pos_y;
                    spr = &gui_panel_sprites[rdata->medsym_sprite_idx];
                    pos_x = map_area->left + (block_size * room->central_stl_x / STL_PER_SLB) - (spr->SWidth*ps_units_per_px/16  / 2);
                    pos_y = map_area->top  + (block_size * room->central_stl_y / STL_PER_SLB) - (spr->SHeight*ps_units_per_px/16 / 2);
                    LbSpriteDrawResized(pos_x, pos_y, ps_units_per_px, spr);
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
        if (player_uses_power_call_to_arms(i))
        {
            struct Dungeon *dungeon;
            dungeon = get_dungeon(i);
            lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
            const struct MagicStats *pwrdynst;
            pwrdynst = get_power_dynamic_stats(PwrK_CALL2ARMS);
            long m;
            m = (4 * ((i + game.play_gameturn) & 7) * subtile_slab_fast(pwrdynst->strength[dungeon->cta_splevel]));
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
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
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
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Trap);
    i = slist->index;
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
        if (k > slist->count)
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
      if (lvinfo->name_stridx > 0)
        lv_name = get_string(lvinfo->name_stridx);
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
        int tx_units_per_px;
        tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
        LbTextDrawResized((w-LbTextStringWidth(lv_name)*units_per_pixel/16)/2, h/10 - 8*units_per_pixel/16, tx_units_per_px, lv_name);
    }
}

void draw_zoom_box_things_on_mapblk(struct Map *mapblk,unsigned short subtile_size,int scr_x,int scr_y)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    int spos_x,spos_y;
    TbPixel color;
    long spridx;
    unsigned long k;
    long i;
    int ps_units_per_px;
    {
        struct TbSprite *spr;
        spr = &gui_panel_sprites[164]; // Use dungeon special box as reference
        ps_units_per_px = (46 * units_per_pixel) / spr->SHeight;
    }
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
                spridx = get_creature_model_graphics(thing->model,CGI_HandSymbol);
                if ((game.play_gameturn & 4) != 0)
                {
                    color = get_player_path_colour(thing->owner);
                    draw_gui_panel_sprite_occentered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx, color);
                } else
                {
                    draw_gui_panel_sprite_centered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx);
                }
                draw_status_sprites(spos_x + scr_x, scr_y + spos_y - 12*units_per_pixel/16, thing, 32*256);
                break;
            case TCls_Trap:
                if ((!thing->byte_18) && (player->id_number != thing->owner))
                    break;
                manufctr = get_manufacture_data(get_manufacture_data_index_for_thing(thing->class_id, thing->model));
                spridx = manufctr->medsym_sprite_idx;
                draw_gui_panel_sprite_centered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx);
                break;
            case TCls_Object:
                if (thing_is_dungeon_heart(thing))
                {
                    spridx = 512;
                    draw_gui_panel_sprite_centered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx);
                } else
                if (object_is_gold(thing))
                {
                    spridx = 511;
                    draw_gui_panel_sprite_centered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx);
                } else
                if (thing_is_special_box(thing))
                {
                    spridx = 164;
                    draw_gui_panel_sprite_centered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx);
                } else
                if (thing_is_spellbook(thing))
                {
                    struct PowerConfigStats *powerst;
                    powerst = get_power_model_stats(book_thing_to_power_kind(thing));
                    spridx = powerst->medsym_sprite_idx;
                    draw_gui_panel_sprite_centered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx);
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

void draw_zoom_box_terrain(long scrtop_x, long scrtop_y, int stl_x, int stl_y, PlayerNumber plyr_idx, long draw_tiles_x, long draw_tiles_y, int subtile_size)
{
    int map_dx,map_dy;
    int scr_x,scr_y;
    int k;
    lbDisplay.DrawFlags = 0;
    scrtop_x += 4*units_per_pixel/16;
    scrtop_y -= 4*units_per_pixel/16;
    setup_vecs(lbDisplay.WScreen, 0, lbDisplay.GraphicsScreenWidth, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    // Draw the actual map
    scr_y = scrtop_y;
    for (map_dy=0; map_dy < draw_tiles_y; map_dy++)
    {
      scr_x = scrtop_x;
      for (map_dx=0; map_dx < draw_tiles_x; map_dx++)
      {
          struct Map *mapblk;
          mapblk = get_map_block_at(stl_x+map_dx,stl_y+map_dy);
          if (map_block_revealed(mapblk, plyr_idx))
          {
            k = element_top_face_texture(mapblk);
            draw_texture(scr_x, scr_y, subtile_size, subtile_size, k, 0, -1);
          } else
          {
            LbDrawBox(scr_x, scr_y, subtile_size, subtile_size, 1);
          }
          scr_x += subtile_size;
      }
      scr_y += subtile_size;
    }
    lbDisplay.DrawFlags |= Lb_SPRITE_OUTLINE;
    LbDrawBox(scrtop_x, scrtop_y, draw_tiles_x*subtile_size, draw_tiles_y*subtile_size, 0);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
}

void draw_zoom_box_things(long scrtop_x, long scrtop_y, int stl_x, int stl_y, PlayerNumber plyr_idx, long draw_tiles_x, long draw_tiles_y, int subtile_size)
{
    LbScreenSetGraphicsWindow(scrtop_x + 2*units_per_pixel/16, scrtop_y + 2*units_per_pixel/16,
        draw_tiles_x*subtile_size - 4*units_per_pixel/16, draw_tiles_y*subtile_size - 4*units_per_pixel/16);
    int map_dx,map_dy;
    int scr_x,scr_y;
    scr_y = 0;
    for (map_dy=0; map_dy < draw_tiles_y; map_dy++)
    {
      scr_x = 0;
      for (map_dx=0; map_dx < draw_tiles_x; map_dx++)
      {
          struct Map *mapblk;
          mapblk = get_map_block_at(stl_x+map_dx,stl_y+map_dy);
          if (map_block_revealed(mapblk, plyr_idx))
          {
            draw_zoom_box_things_on_mapblk(mapblk,subtile_size,scr_x,scr_y);
          }
          scr_x += subtile_size;
      }
      scr_y += subtile_size;
    }
}

/**
 * Draws a box near mouse with more detailed top view of map.
 * Requires screen to be locked before.
 */
void draw_zoom_box(void)
{
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

    // zoom box block size
    const int subtile_size = (8*units_per_pixel+8)/16;
    int stl_x,stl_y;

    // Drawing coordinates
    long scrtop_x,scrtop_y;
    scrtop_x = mouse_x + 24*units_per_pixel/16;
    scrtop_y = mouse_y + 24*units_per_pixel/16;
    if (scrtop_x > MyScreenWidth-draw_tiles_x*subtile_size)
      scrtop_x = MyScreenWidth-draw_tiles_x*subtile_size;
    if (scrtop_x < 0)
        scrtop_x = 0;
    if (scrtop_y > MyScreenHeight-draw_tiles_y*subtile_size)
      scrtop_y = MyScreenHeight-draw_tiles_y*subtile_size;
    if (scrtop_y < 0)
        scrtop_y = 0;
    // Source map coordinates
    stl_x = STL_PER_SLB * (mouse_x/pixel_size-map_area.left) / block_size - draw_tiles_x/2;
    stl_y = STL_PER_SLB * (mouse_y/pixel_size-map_area.top)  / block_size - draw_tiles_y/2;
    // Draw only on map area (do not allow zoom box to be empty)
    if ((stl_x < -draw_tiles_x/2) || (stl_x >= map_subtiles_x+1-draw_tiles_x/2)
     || (stl_y < -draw_tiles_y/2) || (stl_y >= map_subtiles_y+1-draw_tiles_y/2))
      return;

    draw_zoom_box_terrain(scrtop_x, scrtop_y, stl_x, stl_y, player->id_number, draw_tiles_x, draw_tiles_y, subtile_size);
    // Draw thing sprites on the map
    draw_zoom_box_things(scrtop_x, scrtop_y, stl_x, stl_y, player->id_number, draw_tiles_x, draw_tiles_y, subtile_size);
    // Draw sprites surrounding the box
    int bs_units_per_px;
    {
        struct TbSprite *spr;
        spr = &button_sprite[194];
        bs_units_per_px = (74 * units_per_pixel) / spr->SWidth;
    }
    LbScreenSetGraphicsWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    int beg_x,beg_y;
    int end_x,end_y;
    beg_x = scrtop_x-(24*units_per_pixel+8)/16;
    beg_y = scrtop_y-(20*units_per_pixel+8)/16;
    end_x = scrtop_x-(50*units_per_pixel+8)/16 + draw_tiles_x*subtile_size;
    end_y = scrtop_y-(54*units_per_pixel+8)/16 + draw_tiles_y*subtile_size;
    LbSpriteDrawResized(beg_x, beg_y, bs_units_per_px, &button_sprite[194]);
    LbSpriteDrawResized(end_x, beg_y, bs_units_per_px, &button_sprite[195]);
    LbSpriteDrawResized(beg_x, end_y, bs_units_per_px, &button_sprite[196]);
    LbSpriteDrawResized(end_x, end_y, bs_units_per_px, &button_sprite[197]);
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

void zoom_to_parchment_map(void)
{
    struct PlayerInfo *player;
    turn_off_all_window_menus();
    if ((game.status_flags & Status_ShowGui) == 0)
        set_flag_byte(&game.status_flags, Status_ShowStatusMenu, false);
    else
        set_flag_byte(&game.status_flags, Status_ShowStatusMenu, true);
    player = get_my_player();
    if (((game.system_flags & GSF_NetworkActive) != 0)
        || (lbDisplay.PhysicalScreenWidth > 320))
    {
      if (!toggle_status_menu(0))
          set_flag_byte(&game.status_flags, Status_ShowStatusMenu, false);
      set_players_packet_action(player, PckA_SaveViewType, PVT_MapScreen, 0);
      turn_off_roaming_menus();
    } else
    {
        set_players_packet_action(player, PckA_SetViewType, PVT_MapFadeIn, 0);
      turn_off_roaming_menus();
    }
}

void zoom_from_parchment_map(void)
{
    struct PlayerInfo *player;
    player = get_my_player();
    if (((game.system_flags & GSF_NetworkActive) != 0)
        || (lbDisplay.PhysicalScreenWidth > 320))
    {
        if ((game.status_flags & Status_ShowStatusMenu) != 0)
          toggle_status_menu(1);
        set_players_packet_action(player, PckA_LoadViewType, PVT_DungeonTop, 0);
    } else
    {
        set_players_packet_action(player, PckA_SetViewType, PVT_MapFadeOut, 0);
    }
}
/******************************************************************************/
