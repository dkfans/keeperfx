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
#include "pre_inc.h"
#include "kfx_memory.h"
#include "kfx/renderer/RendererManager.h"
#include "gui_parchment.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_vidraw.h"
#include "vidmode.h" // pixmap.ghost, ghost_table_blend() -- shared with the minimap's identical blend
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_dernc.h"
#include "bflib_planar.h"
#include "custom_sprites.h"
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
#include "config_spritecolors.h"
#include "thing_data.h"
#include "thing_objects.h"
#include "thing_traps.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "creature_states_hero.h"
#include "power_hand.h"
#include "game_legacy.h"
#include "room_list.h"
#include "room_workshop.h"
#include "frontmenu_ingame_tabs.h"
#include "vidfade.h"
#include "sprites.h"
#include "player_instances.h"

#include "keeperfx.hpp"
#include "post_inc.h"

/******************************************************************************/
unsigned short engine_remap_texture_blocks(long stl_x, long stl_y, unsigned short tex_id);
/******************************************************************************/
int parchment_loaded;
unsigned char *hires_parchment;
static unsigned char *lores_parchment = NULL;
/******************************************************************************/
void load_parchment_file(void)
{
    if ( !parchment_loaded )
    {
      reload_parchment_file(RendererPhysicalWidth() >= 640);
    }
}

void reload_parchment_file(TbBool hires)
{
  char *fname;
  long result;
  if (hires)
  {
#ifdef SPRITE_FORMAT_V2
      fname = prepare_file_fmtpath(FGrp_StdData,"gmap-%d.raw",64);
#else
      fname = prepare_file_path(FGrp_StdData,"gmap64.raw");
#endif
      result = LbFileLoadAt(fname, hires_parchment);
  } else
  {
#ifdef SPRITE_FORMAT_V2
      fname = prepare_file_fmtpath(FGrp_StdData,"gmap-%d.raw",32);
#else
      fname = prepare_file_path(FGrp_StdData,"gmap32.raw");
#endif
      if (lores_parchment == NULL)
          lores_parchment = (unsigned char *)KfxAlloc(320*200);
      result = LbFileLoadAt(fname, lores_parchment);
  }
  // Only latch "loaded" on success -- LbFileLoadAt() already ERRORLOGs the
  // failure with the filename; leaving parchment_loaded at 0 lets the next
  // load_parchment_file() call (re-entering the map screen) retry, instead
  // of permanently serving whatever the buffer held before this call.
  if (result != -1)
      parchment_loaded = 1;
}

long get_parchment_background_area_rect(struct TbRect *bkgnd_area)
{
    int img_width;
    int img_height;
    if (RendererPhysicalWidth() < 640)
    {
        img_width = 320;
        img_height = 200;
    } else
    {
        img_width = 640;
        img_height = 480;
    }
    int rect_w = RendererPhysicalWidth();
    int rect_h = RendererPhysicalHeight();
    // Parchment bitmap scaling
    int units_per_px = max(16 * rect_w / img_width, 16 * rect_h / img_height);
    int units_per_px_max = min(16 * 7 * rect_w / (6 * img_width), 16 * 4 * rect_h / (3 * img_height));
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
    long bkgnd_width = bkgnd_area.right - bkgnd_area.left;
    long bkgnd_height = bkgnd_area.bottom - bkgnd_area.top;
    long block_size = min((bkgnd_width - bkgnd_width / 3) / game.map_tiles_x, (bkgnd_height - bkgnd_height / 8) / game.map_tiles_y);
    if (block_size < 1) block_size = 1;
    map_area->left = bkgnd_area.left + (bkgnd_width - block_size*game.map_tiles_x) / 2;
    map_area->top = bkgnd_area.top + 3 * (bkgnd_height - block_size*game.map_tiles_y) / 4;
    map_area->right = map_area->left + block_size*game.map_tiles_x;
    map_area->bottom = map_area->top + block_size*game.map_tiles_y;
    return block_size;
}

TbBool point_to_overhead_map(const struct Camera *camera, const long screen_x, const long screen_y, int32_t *map_x, int32_t *map_y)
{
    // Sizes of the parchment map on which we are
    struct TbRect map_area;
    long block_size = get_parchment_map_area_rect(&map_area);
    // Check if we're within coordinates with the screen position
    *map_x = 0;
    *map_y = 0;
    if ((screen_x >= map_area.left) && (screen_x < map_area.right)
      && (screen_y >= map_area.top) && (screen_y < map_area.bottom))
    {
        *map_x = COORD_PER_STL * STL_PER_SLB * (screen_x-map_area.left) / block_size + COORD_PER_STL/2;
        *map_y = COORD_PER_STL * STL_PER_SLB * (screen_y-map_area.top)  / block_size + COORD_PER_STL/2;
        return ((*map_x >= 0) && (*map_x < (game.map_subtiles_x+1)<<8) && (*map_y >= 0) && (*map_y < (game.map_subtiles_y+1)<<8));
    }
    return false;
}

TbBool parchment_copy_background_at(const struct TbRect *bkgnd_area, int units_per_px)
{
    int img_width;
    int img_height;
    unsigned char *srcbuf;
    unsigned char shift;
    if (RendererPhysicalWidth() < 640)
    {
        img_width = 320;
        img_height = 200;
        srcbuf = lores_parchment;
        shift = 5;
    } else
    {
        img_width = 640;
        img_height = 480;
        srcbuf = hires_parchment;
        shift = 4;
    }
    // Only 8bpp supported for now
    if (LbGraphicsScreenBPP() != 8)
    {
        WARNLOG("Parchment background/candles skipped -- screen is %dbpp, not 8", (int)LbGraphicsScreenBPP());
        return false;
    }
    if (srcbuf == NULL)
    {
        WARNLOG("Parchment background skipped -- source buffer not loaded (parchment_loaded=%d)", parchment_loaded);
        return false;
    }
    // Do the drawing
    struct RendererPresentImageDesc d = {0};
    d.format  = PRESENT_FORMAT_INDEXED8;
    d.palette = PRESENT_PALETTE_GAME;
    d.kind    = PRESENT_KIND_OPAQUE;
    d.dst_w = img_width*units_per_px/16; d.dst_h = img_height*units_per_px/16;
    d.dst_x = bkgnd_area->left;          d.dst_y = bkgnd_area->top;
    d.src   = srcbuf; d.src_w = img_width; d.src_h = img_height;
    RendererPresentImage(&d);
    // Burning candle flames
    const struct TbSprite* spr = get_button_sprite(GBS_parchment_map_screen_flame_1 + (get_gameturn() & 3));
    UIRenderer_SubmitScaledSprite(bkgnd_area->left+(36*units_per_px/(pixel_size << shift)),(bkgnd_area->top+0*units_per_px/(16*pixel_size)), spr->SWidth*units_per_px/16, spr->SHeight*units_per_px/16, spr, 0);
    spr = get_button_sprite(GBS_parchment_map_screen_flame_5+(get_gameturn() & 3));
    UIRenderer_SubmitScaledSprite(bkgnd_area->left+(574*units_per_px/(pixel_size << shift)),(bkgnd_area->top+0*units_per_px/(16*pixel_size)), spr->SWidth*units_per_px/16, spr->SHeight*units_per_px/16, spr, 0);
    return true;
}

/**
 * Draws parchment view background, used for in-game level map screen.
 */
void draw_map_parchment(void)
{
    // Get background area rectangle
    struct TbRect bkgnd_area;
    int units_per_px = get_parchment_background_area_rect(&bkgnd_area);
    // Draw it
    parchment_copy_background_at(&bkgnd_area, units_per_px);
    SYNCDBG(9,"Done");
}

enum OverheadMapStyle {
    OMapSt_Unchanged = 256,
    OMapSt_Tagged,
    OMapSt_TaggedGems,
    OMapSt_Gold,
    OMapSt_Gems,
    OMapSt_Wall,
    OMapSt_Abyss,
};

static TbPixel get_player_path_colour(unsigned short owner)
{
  return player_path_colours[get_player_color_idx(owner % PLAYERS_COUNT)];
}

static int get_overhead_mapblock_style(const struct Map* mapblk, const struct SlabMap* slb, MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx, int gui_frame, TbPixel neutral_colour)
{
    PlayerNumber owner = slb->owner;
    if ((((mapblk->flags & SlbAtFlg_Unexplored) != 0) || ((mapblk->flags & SlbAtFlg_TaggedValuable) != 0)) && (gui_frame >= 4)) {
        if (slb->kind == SlbT_GEMS) {
            return OMapSt_TaggedGems;
        }
        return OMapSt_Tagged;
    }
    if (!map_block_revealed(mapblk, plyr_idx)) {
        return OMapSt_Unchanged;
    }
    if ((slb->kind == SlbT_GOLD) || (slb->kind == SlbT_DENSEGOLD)) {
        return OMapSt_Gold;
    }
    if (slb->kind == SlbT_GEMS) {
        return OMapSt_Gems;
    }
    if ((mapblk->flags & SlbAtFlg_IsRoom) != 0) {
        struct Room* room = room_get(slb->room_index);
        if (((gui_frame & 1) != 0) && (room->kind == gui_room_type_highlighted)) {
            return player_highlight_colours[owner];
        }
        unsigned char color_idx = get_player_color_idx(owner);
        if (color_idx == PLAYER_NEUTRAL) {
            return neutral_colour;
        }
        return player_room_colours[color_idx];
    }
    if (slb->kind == SlbT_ROCK) {
        return 0;
    }
    if (slb->kind == SlbT_ROCK_FLOOR) {
        return pixmap.ghost[3];
    }
    if (subtile_has_abyss_on_top(slab_subtile_center(slb_x), slab_subtile_center(slb_y))) {
        return OMapSt_Abyss;
    }
    if ((mapblk->flags & SlbAtFlg_Filled) != 0) {
        return OMapSt_Wall;
    }
    if ((mapblk->flags & SlbAtFlg_IsDoor) != 0) {
        struct Thing* thing = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (thing_is_invalid(thing)) {
            return 60;
        }
        if (((gui_frame & 1) != 0) && (thing->model == gui_door_type_highlighted)) {
            return player_highlight_colours[owner];
        }
        if (door_is_hidden_to_player(thing, plyr_idx)) {
            return OMapSt_Wall;
        }
        if (thing->door.is_locked) {
            return 79;
        }
        return 60;
    }
    if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
        return OMapSt_Unchanged;
    }
    if (slb->kind == SlbT_LAVA) {
        return 146;
    }
    if (slb->kind == SlbT_WATER) {
        return 85;
    }
    if (slb->kind == SlbT_PURPLE) {
        return 255;
    }
    return get_player_path_colour(owner);
}

/** Seeds an overhead-map scratch buffer with the parchment background it
 *  will be composited over, sampled 1:1 with map_area's screen pixels. Lets
 *  the ghost-remap styles below tint real backing pixels instead of the
 *  live framebuffer (absent under GL), and unrevealed tiles need no special
 *  case -- they're simply never overwritten. */
static void fill_overhead_scratch_with_parchment_background(unsigned char* omap_scratch, int map_w, int map_h,
    const struct TbRect *map_area)
{
    struct TbRect bkgnd_area;
    int bg_units_per_px = get_parchment_background_area_rect(&bkgnd_area);
    int img_width, img_height;
    const unsigned char* srcbuf;
    if (RendererPhysicalWidth() < 640) {
        img_width = 320; img_height = 200; srcbuf = lores_parchment;
    } else {
        img_width = 640; img_height = 480; srcbuf = hires_parchment;
    }
    if (srcbuf == NULL) {
        memset(omap_scratch, 0, (size_t)map_w * (size_t)map_h);
        return;
    }
    long base_rel_x = map_area->left - bkgnd_area.left;
    long base_rel_y = map_area->top - bkgnd_area.top;
    for (int y = 0; y < map_h; y++) {
        long src_y = (base_rel_y + y) * 16 / bg_units_per_px;
        if (src_y < 0) src_y = 0;
        if (src_y >= img_height) src_y = img_height - 1;
        const unsigned char* srcrow = &srcbuf[src_y * img_width];
        unsigned char* dstrow = &omap_scratch[y * map_w];
        for (int x = 0; x < map_w; x++) {
            long src_x = (base_rel_x + x) * 16 / bg_units_per_px;
            if (src_x < 0) src_x = 0;
            if (src_x >= img_width) src_x = img_width - 1;
            dstrow[x] = srcrow[src_x];
        }
    }
}

void draw_overhead_map(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    int map_w = (int)(map_area->right - map_area->left);
    int map_h = (int)(map_area->bottom - map_area->top);
    if ((map_w <= 0) || (map_h <= 0))
        return;
    unsigned char* omap_scratch = (unsigned char*)KfxAlloc((size_t)map_w * (size_t)map_h);
    if (omap_scratch == NULL) {
        WARNLOG("Overhead map scratch buffer allocation failed; map not drawn");
        return;
    }
    fill_overhead_scratch_with_parchment_background(omap_scratch, map_w, map_h, map_area);

    GameTurn turn = get_gameturn();
    int gui_frame = (turn / gui_blink_rate) & 7;
    TbPixel neutral_colour = player_room_colours[(turn / neutral_flash_rate) & 3];
    int32_t block_stride = map_w * block_size;
    int styles[MAX_TILES_X];
    const struct SlabMap* slb = get_slabmap_block(0, 0);
    unsigned char* dstrow = omap_scratch;
    for (MapSlabCoord slb_y = 0; slb_y < game.map_tiles_y; slb_y++, dstrow += block_stride) {
        const struct Map* mapblk = get_map_block_at(slab_subtile_center(0), slab_subtile_center(slb_y));
        for (MapSlabCoord slb_x = 0; slb_x < game.map_tiles_x; slb_x++, slb++, mapblk += STL_PER_SLB) {
            styles[slb_x] = get_overhead_mapblock_style(mapblk, slb, slb_x, slb_y, plyr_idx, gui_frame, neutral_colour);
        }
        unsigned char* dstblock = dstrow;
        for (MapSlabCoord slb_x = 0; slb_x < game.map_tiles_x;) {
            int style = styles[slb_x];
            MapSlabCoord run = 1;
            while ((slb_x + run < game.map_tiles_x) && (styles[slb_x + run] == style)) {
                run++;
            }
            int32_t run_width = run * block_size;
            if (style == OMapSt_Unchanged) {
                slb_x += run;
                dstblock += run_width;
                continue;
            }
            // ghost_table_blend()'s table_offset for each style, or -1 for a
            // literal colour write -- same table/formula the minimap uses
            // (vidmode.h) for the identical categories. Abyss uses its own
            // direct 256-entry LUT (pixmap.map_abyss) instead, since it isn't
            // an offset into pixmap.ghost.
            int table_offset = -1;
            int shift = 0;
            int add = 0;
            const unsigned char* abyss_remap = NULL;
            if (style == OMapSt_Tagged || style == OMapSt_TaggedGems) {
                table_offset = 0x1A00;
                if (style == OMapSt_TaggedGems) {
                    add = 2;
                }
            } else if (style == OMapSt_Gold) {
                table_offset = 0x8C00;
            } else if (style == OMapSt_Gems) {
                table_offset = 0;
                shift = 6;
                add = 102;
            } else if (style == OMapSt_Wall) {
                table_offset = 0x1000;
            } else if (style == OMapSt_Abyss) {
                abyss_remap = pixmap.map_abyss;
            }
            unsigned char* dstline = dstblock;
            for (int32_t y = 0; y < block_size; y++) {
                if (abyss_remap != NULL) {
                    for (int32_t x = 0; x < run_width; x++) {
                        dstline[x] = abyss_remap[dstline[x]];
                    }
                } else if (table_offset < 0) {
                    if (run_width >= 16) {
                        memset(dstline, style, run_width);
                    } else {
                        volatile unsigned char* dstpixel = dstline;
                        for (int32_t x = 0; x < run_width; x++) {
                            dstpixel[x] = style;
                        }
                    }
                } else {
                    for (int32_t x = 0; x < run_width; x++) {
                        dstline[x] = ghost_table_blend(dstline[x], table_offset, shift, add);
                    }
                }
                dstline += map_w;
            }
            slb_x += run;
            dstblock += run_width;
        }
    }

    struct RendererPresentImageDesc d = {0};
    d.format = PRESENT_FORMAT_INDEXED8;
    d.palette = PRESENT_PALETTE_GAME;
    d.kind = PRESENT_KIND_OPAQUE;
    d.dst_x = map_area->left;   d.dst_y = map_area->top;
    d.dst_w = map_w;            d.dst_h = map_h;
    d.src = omap_scratch;            d.src_pitch = map_w;
    d.src_w = map_w;            d.src_h = map_h;
    RendererPresentImage(&d);

    KfxFree(omap_scratch);
    RendererSetDrawFlags(0);
}

void draw_overhead_room_icons(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    int ps_units_per_px;
    {
        const struct TbSprite* spr = get_panel_sprite(GPS_room_treasury_std_s);//only for size, room irrelevant
        ps_units_per_px = 32 * block_size * 4 / spr->SHeight;
    }
    long rkind_select = (get_gameturn() >> 1) % game.conf.slab_conf.room_types_count;
    for (struct Room* room = start_rooms; room < end_rooms; room++)
    {
      if (room_exists(room))
      {
          long room_visibility = abs(rkind_select - room->kind);
          TbBool dimmed = (room_visibility >= 2) && (room_visibility < 4);
          if (room_visibility < 4)
          {
            if (subtile_revealed(room->central_stl_x, room->central_stl_y, plyr_idx))
            {
                const struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
                if (roomst->medsym_sprite_idx > 0)
                {
                    long sprite_idx = get_player_colored_icon_idx(roomst->medsym_sprite_idx,room->owner);
                    const struct TbSprite* spr = get_panel_sprite(sprite_idx);
                    long pos_x = map_area->left + (block_size * room->central_stl_x / STL_PER_SLB) - (spr->SWidth * ps_units_per_px / 16 / 2);
                    long pos_y = map_area->top + (block_size * room->central_stl_y / STL_PER_SLB) - (spr->SHeight * ps_units_per_px / 16 / 2);
                    if (RendererPointInZoomBoxScreenRect((int)pos_x, (int)pos_y)) continue;
                    UIRenderer_SubmitPanelSpriteRaw(pos_x, pos_y, ps_units_per_px, spr, dimmed ? Lb_SPRITE_TRANSPAR4 : 0);
                }
            }
          }
        }
    }
}

int draw_overhead_call_to_arms(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    int n = 0;
    for (int i = 0; i < DUNGEONS_COUNT; i++)
    {
        if (player_uses_power_call_to_arms(i))
        {
            struct Dungeon* dungeon = get_dungeon(i);
            const struct PowerConfigStats *powerst = get_power_model_stats(PwrK_CALL2ARMS);
            long m = (4 * ((i + get_gameturn()) & 7) * subtile_slab(powerst->strength[dungeon->cta_power_level]));
            long pos_x = map_area->left + block_size * (int)dungeon->cta_stl_x / STL_PER_SLB;
            long pos_y = map_area->top + block_size * (int)dungeon->cta_stl_y / STL_PER_SLB;
            long radius = (((m & 7) + m) >> 3) / pixel_size;
            unsigned char col = player_room_colours[get_player_color_idx(i)];
            if (RendererPointInZoomBoxScreenRect((int)pos_x, (int)pos_y)) { n++; continue; }
            // GPU path has no circle primitive -- approximate as an outline
            // square (matches develop's UIRenderer_SubmitOutlineBox use here).
            UIRenderer_SubmitOutlineBox(pos_x - radius, pos_y - radius, radius * 2, radius * 2, col);
            n++;
        }
    }
    return n;
}

int draw_overhead_creatures(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    TbPixel col;
    short pixel_end;
    int p;
    int n = 0;
    int k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (!thing_is_picked_up(thing))
        {
            unsigned char color_idx = get_player_color_idx(thing->owner);
            TbPixel col1 = player_highlight_colours[color_idx];
            TbPixel col2 = 1;
            if (thing_revealed(thing, plyr_idx))
            {
                if (color_idx == game.neutral_player_num)
                {
                    col1 = player_room_colours[(((get_gameturn() + neutral_flash_rate) % (4 * neutral_flash_rate)) / neutral_flash_rate)];
                } else
                if ((get_gameturn() % (8 * gui_blink_rate)) < 4 * gui_blink_rate)
                {
                    col2 = player_room_colours[color_idx];
                    col1 = player_room_colours[color_idx];
                }
                long pos_x = map_area->left + block_size * (int)thing->mappos.x.stl.num / STL_PER_SLB;
                long pos_y = map_area->top + block_size * (int)thing->mappos.y.stl.num / STL_PER_SLB;
                if (thing->owner == plyr_idx)
                {
                    col = col2;

                }
                else
                {
                    col = col1;
                }
                if (!RendererPointInZoomBoxScreenRect((int)pos_x, (int)pos_y))
                {
                    pixel_end = get_pixels_scaled_and_zoomed(TWO_PIXELS);
                    for (p = 0; p < pixel_end; p++)
                    {
                        UIRenderer_SubmitSolidBox(pos_x + draw_square[p].delta_x, pos_y+draw_square[p].delta_y, 1, 1, col);
                    }
                    n++;
                }
            } else
            // Special tunneler code
            if (is_hero_tunnelling_to_attack(thing))
            {
                struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
                if ((get_gameturn() % (8 * gui_blink_rate)) < 4 * gui_blink_rate)
                {
                    col1 = player_room_colours[get_player_color_idx((int)(cctrl->party.target_plyr_idx>=0?cctrl->party.target_plyr_idx:0))];
                    col2 = player_room_colours[get_player_color_idx(thing->owner)];
                }
                if (thing->owner == plyr_idx)
                {
                    col = col2;
                }
                else
                {
                    col = col1;
                }
                for (int m = 0; m < 5; m++)
                {
                    long memberpos = cctrl->party.member_pos_stl[m];
                    if (memberpos == 0)
                        break;
                    long pos_x = map_area->left + block_size * stl_num_decode_x(memberpos) / STL_PER_SLB;
                    long pos_y = map_area->top + block_size * stl_num_decode_y(memberpos) / STL_PER_SLB;
                    if (RendererPointInZoomBoxScreenRect((int)pos_x, (int)pos_y)) { n++; continue; }
                    pixel_end = get_pixels_scaled_and_zoomed(TWO_PIXELS);
                    for (p = 0; p < pixel_end; p++)
                    {
                        UIRenderer_SubmitSolidBox(pos_x + draw_square[p].delta_x, pos_y + draw_square[p].delta_y, 1, 1, col);
                    }
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

int draw_overhead_traps(const struct TbRect *map_area, long block_size, PlayerNumber plyr_idx)
{
    int n = 0;
    int k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Trap);
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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
                if ( (thing->trap.revealed) || (thing->owner == plyr_idx) )
                {
                    long pos_x = map_area->left + (block_size * (int)thing->mappos.x.stl.num / STL_PER_SLB) + ((block_size + 1)/5);
                    long pos_y = map_area->top + (block_size * (int)thing->mappos.y.stl.num / STL_PER_SLB) + ((block_size + 1)/5);
                    if (!RendererPointInZoomBoxScreenRect((int)pos_x, (int)pos_y))
                    {
                    short pixels_amount = scale_pixel(ONE_PIXEL);
                    short pixel_end = get_pixels_scaled_and_zoomed(ONE_PIXEL);
                    short colour = 60;
                    for (int p = 0; p < pixel_end; p++)
                    {
                        // Draw a cross
                        UIRenderer_SubmitSolidBox(pos_x + draw_square[p].delta_x, pos_y + draw_square[p].delta_y, 1, 1, colour);
                        UIRenderer_SubmitSolidBox(pos_x + pixels_amount + draw_square[p].delta_x, pos_y + draw_square[p].delta_y, 1, 1, colour);
                        UIRenderer_SubmitSolidBox(pos_x - pixels_amount + draw_square[p].delta_x, pos_y + draw_square[p].delta_y, 1, 1, colour);
                        UIRenderer_SubmitSolidBox(pos_x + draw_square[p].delta_x, pos_y + pixels_amount + draw_square[p].delta_y, 1, 1, colour);
                        UIRenderer_SubmitSolidBox(pos_x + draw_square[p].delta_x, pos_y - pixels_amount + draw_square[p].delta_y, 1, 1, colour);
                    }
                    n++;
                    }
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
    int n = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    int k = 0;
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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
                  long pos_x = map_area->left + block_size * (int)thing->mappos.x.stl.num / STL_PER_SLB  + ((block_size + 1)/5);
                  long pos_y = map_area->top + block_size * (int)thing->mappos.y.stl.num / STL_PER_SLB + ((block_size + 1)/5);
                  if (!RendererPointInZoomBoxScreenRect((int)pos_x, (int)pos_y)) {
                  short pixel_end = get_pixels_scaled_and_zoomed(TWO_PIXELS);
                  for (int p = 0; p < pixel_end; p++)
                  {
                      UIRenderer_SubmitSolidBox(pos_x + draw_square[p].delta_x, pos_y + draw_square[p].delta_y, 1, 1, colours[15][0][15]);
                  }
                  }
              }
              else if ( thing_is_workshop_crate(thing) )
              {
                  long pos_x = map_area->left + block_size * (int)thing->mappos.x.stl.num / STL_PER_SLB  + ((block_size + 1)/5);
                  long pos_y = map_area->top + block_size * (int)thing->mappos.y.stl.num / STL_PER_SLB + ((block_size + 1)/5);
                  if (!RendererPointInZoomBoxScreenRect((int)pos_x, (int)pos_y)) {
                  short pixel_end = get_pixels_scaled_and_zoomed(TWO_PIXELS);
                  for (int p = 0; p < pixel_end; p++)
                  {
                      UIRenderer_SubmitSolidBox(pos_x + draw_square[p].delta_x, pos_y + draw_square[p].delta_y, 1, 1, colours[7][6][7]);
                  }
                  }
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
    if ((((get_gameturn()) % (4 * gui_blink_rate)) / gui_blink_rate) == 1) {
        draw_overhead_spells(map_area, block_size, plyr_idx);
    }
    draw_overhead_traps(map_area, block_size, plyr_idx);
}

void draw_2d_map(void)
{
    SYNCDBG(8, "Starting");
    if (!render_fade_tables)
    {
        render_fade_tables = pixmap.fade_tables;
    }
    struct PlayerInfo* player = get_my_player();
    // Size of the parchment map on which we're drawing
    struct TbRect map_area;
    long block_size = get_parchment_map_area_rect(&map_area);
    // Now draw
    draw_overhead_map(&map_area, block_size, player->id_number);
    draw_overhead_things(&map_area, block_size, player->id_number);
    draw_overhead_room_icons(&map_area, block_size, player->id_number);
}

void draw_map_level_name(void)
{
    // Retrieving name
    const char* lv_name = NULL;
    LevelNumber lvnum = get_loaded_level_number();
    struct LevelInformation* lvinfo = get_level_info(lvnum);
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
    RendererSetDrawFlags(0);
    int x = bkgnd_area.left;
    int y = bkgnd_area.top;
    int w = bkgnd_area.right - bkgnd_area.left;
    int h = (bkgnd_area.bottom - bkgnd_area.top) / 4;
    // Drawing
    if (lv_name != NULL)
    {
        LbTextSetWindow(x, y, w, h);
        int tx_units_per_px = ( (MyScreenHeight < 400) && (dbc_initialized && dbc_enabled) ) ? scale_ui_value(32) : (22 * units_per_pixel) / LbTextLineHeight();
        LbTextDrawResized((w-LbTextStringWidth(lv_name)*units_per_pixel/16)/2, h/10 - 8*units_per_pixel/16, tx_units_per_px, lv_name);
    }
}

void draw_zoom_box_things_on_mapblk(struct Map *mapblk,unsigned short subtile_size,int scr_x,int scr_y)
{
    int ps_units_per_px;
    {
        const struct TbSprite* spr = get_panel_sprite(GPS_trapdoor_bonus_box_std_s); // Use dungeon special box as reference
        ps_units_per_px = (46 * units_per_pixel) / spr->SHeight;
    }
    struct PlayerInfo* player = get_my_player();
    unsigned long k = 0;
    struct ObjectConfigStats* objst;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        if (!thing_is_picked_up(thing))
        {
            int spos_x = ((subtile_size * ((long)thing->mappos.x.stl.pos)) >> 8);
            int spos_y = ((subtile_size * ((long)thing->mappos.y.stl.pos)) >> 8);
            long spridx;
            switch (thing->class_id)
            {
            case TCls_Creature:
            {
                spridx = get_creature_model_graphics(thing->model, CGI_HandSymbol);
                if ((get_gameturn() % (8 * gui_blink_rate)) >= 4 * gui_blink_rate)
                {
                    TbPixel color = get_player_path_colour(thing->owner);
                    draw_gui_panel_sprite_occentered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx, color, 0);
                } else
                {
                    draw_gui_panel_sprite_centered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx, 0);
                }
                draw_status_sprites(spos_x + scr_x, scr_y + spos_y - 12*units_per_pixel/16, thing);
                break;
            }
            case TCls_Trap:
            {
                struct TrapConfigStats* trapst = get_trap_model_stats(thing->model);
                if ((!thing->trap.revealed) && (player->id_number != thing->owner) && (trapst->hidden != 0))
                    break;
                struct ManufactureData* manufctr = get_manufacture_data(get_manufacture_data_index_for_thing(thing->class_id, thing->model));
                spridx = manufctr->medsym_sprite_idx;
                //This line and all cases below used to be: draw_gui_panel_sprite_centered(scr_x + spos_x, scr_y + spos_y - 13*units_per_pixel/16, ps_units_per_px, spridx);
                draw_gui_panel_sprite_centered(scr_x + (spos_x * 3 / 2), scr_y - (spos_y /2), ps_units_per_px, spridx, 0);
                break;
            }
            case TCls_Object:
                //get spridx from config
                objst = get_object_model_stats(thing->model);
                spridx = objst->map_icon;
                if (spridx < 0)
                {
                    if (thing_is_spellbook(thing))
                    {
                        struct PowerConfigStats* powerst;
                        powerst = get_power_model_stats(book_thing_to_power_kind(thing));
                        spridx = powerst->medsym_sprite_idx;
                    }
                }
                if (spridx > 0)
                {
                    draw_gui_panel_sprite_centered(spos_x + scr_x, scr_y + spos_y - 6 * units_per_pixel / 16, ps_units_per_px, spridx, 0);
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
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
}

static void scale_tmap2(long texture_block_index, long flags, long fade_level, long screen_x, long screen_y, long scaled_width, long scaled_height)
{
    if ((scaled_width == 0) || (scaled_height == 0)) {
        return;
    }
    long xstart;
    long ystart;
    long xend;
    long yend;
    char orient;
    switch (flags)
    {
    case 0:
        xstart = 0;
        ystart = 0;
        xend = 2097151 / scaled_width;
        yend = 2097151 / scaled_height;
        orient = 0;
        break;
    case 0x10:
        xstart = 2097151;
        ystart = 0;
        xend = -2097151 / scaled_width;
        yend = 2097151 / scaled_height;
        orient = 0;
        break;
    case 0x20:
        xstart = 0;
        ystart = 2097151;
        xend = 2097151 / scaled_width;
        yend = -2097151 / scaled_height;
        orient = 0;
        break;
    case 0x30:
        xstart = 2097151;
        ystart = 2097151;
        xend = -2097151 / scaled_width;
        yend = -2097151 / scaled_height;
        orient = 0;
        break;
    case 0x40:
        ystart = 0;
        xstart = 0;
        yend = 2097151 / scaled_height;
        xend = 2097151 / scaled_width;
        orient = 1;
        break;
    case 0x50:
        ystart = 0;
        xstart = 2097151;
        yend = 2097151 / scaled_height;
        xend = -2097151 / scaled_width;
        orient = 1;
        break;
    case 0x60:
        ystart = 2097151;
        xstart = 0;
        yend = -2097151 / scaled_height;
        xend = 2097151 / scaled_width;
        orient = 1;
        break;
    case 0x70:
        xstart = 2097151;
        ystart = 2097151;
        yend = -2097151 / scaled_height;
        xend = -2097151 / scaled_width;
        orient = 1;
        break;
    default:
          return;
    }
    long local_screen_x;
    long local_screen_y;
    local_screen_x = screen_x;
    if (local_screen_x < 0)
    {
        scaled_width += local_screen_x;
        if (scaled_width < 0) {
            return;
        }
        xstart -= xend * local_screen_x;
        local_screen_x = 0;
    }
    if (local_screen_x + scaled_width > vec_window_width)
    {
        scaled_width = vec_window_width - local_screen_x;
        if (scaled_width < 0) {
            return;
        }
    }
    local_screen_y = screen_y;
    if (local_screen_y < 0)
    {
        scaled_height += local_screen_y;
        if (scaled_height < 0) {
            return;
        }
        ystart -= local_screen_y * yend;
        local_screen_y = 0;
    }
    if (local_screen_y + scaled_height > vec_window_height)
    {
        scaled_height = vec_window_height - local_screen_y;
        if (scaled_height < 0) {
            return;
        }
    }
    int i;
    int32_t hlimits[480];
    int32_t wlimits[640];
    int32_t *xlim;
    int32_t *ylim;
    unsigned char *dbuf;
    unsigned char *block;
    if (!orient)
    {
        xlim = wlimits;
        for (i = scaled_width; i > 0; i--)
        {
            *xlim = xstart;
            xlim++;
            xstart += xend;
        }
        ylim = hlimits;
        for (i = scaled_height; i > 0; i--)
        {
            *ylim = ystart;
            ylim++;
            ystart += yend;
        }
        dbuf = &vec_screen[local_screen_x + local_screen_y * vec_screen_width];
        block = block_ptrs[texture_block_index];
        ylim = hlimits;
        long px;
        long py;
        int srcx;
        int srcy;
        unsigned char *d;
        if ( fade_level >= 0 )
        {
          for (py = scaled_height; py > 0; py--)
          {
              xlim = wlimits;
              d = dbuf;
              srcy = (((*ylim) & 0xFF0000u) >> 16);
              for (px = scaled_width; px > 0; px--)
              {
                srcx = (((*xlim) & 0xFF0000u) >> 16);
                xlim++;
                *d = pixmap.fade_tables[256 * fade_level + block[(srcy << 8) + srcx]];
                ++d;
              }
              dbuf += vec_screen_width;
              ylim++;
          }
        } else
        {
          for (py = scaled_height; py > 0; py--)
          {
            xlim = wlimits;
            d = dbuf;
            srcy = (((*ylim) & 0xFF0000u) >> 16);
            for (px = scaled_width; px > 0; px--)
            {
              srcx = (((*xlim) & 0xFF0000u) >> 16);
              xlim++;
              *d = block[(srcy << 8) + srcx];
              ++d;
            }
            dbuf += vec_screen_width;
            ylim++;
          }
        }
    } else
    {
        ylim = wlimits;
        for (i = scaled_height; i > 0; i--)
        {
          *ylim = ystart;
          ylim++;
          ystart += yend;
        }
        xlim = hlimits;
        for (i = scaled_width; i > 0; i--)
        {
          *xlim = xstart;
          xlim++;
          xstart += xend;
        }
        dbuf = &vec_screen[local_screen_x + local_screen_y * vec_screen_width];
        block = block_ptrs[texture_block_index];
        ylim = wlimits;
        long px;
        long py;
        int srcx;
        int srcy;
        unsigned char *d;
        if ( fade_level >= 0 )
        {
          for (py = scaled_height; py > 0; py--)
          {
              xlim = hlimits;
              d = dbuf;
              srcy = (((*ylim) & 0xFF0000u) >> 16);
              for (px = scaled_width; px > 0; px--)
              {
                srcx = (((*xlim) & 0xFF0000u) >> 16);
                xlim++;
                *d = pixmap.fade_tables[256 * fade_level + block[(srcx << 8) + srcy]];
                ++d;
              }
              dbuf += vec_screen_width;
              ylim++;
          }
        } else
        {
          for (py = scaled_height; py > 0; py--)
          {
            xlim = hlimits;
            d = dbuf;
            srcy = (((*ylim) & 0xFF0000u) >> 16);
            for (px = scaled_width; px > 0; px--)
            {
              srcx = (((*xlim) & 0xFF0000u) >> 16);
              xlim++;
              *d = block[(srcx << 8) + srcy];
              ++d;
            }
            dbuf += vec_screen_width;
            ylim++;
          }
        }
    }
}

static void draw_texture(int32_t texture_x, int32_t texture_y, int32_t texture_width, int32_t texture_height, int32_t texture_block_index, int32_t flags, int32_t fade_level)
{
    scale_tmap2(texture_block_index, flags, fade_level, texture_x / pixel_size, texture_y / pixel_size, texture_width / pixel_size, texture_height / pixel_size);
}

void draw_zoom_box_terrain(long scrtop_x, long scrtop_y, int stl_x, int stl_y, PlayerNumber plyr_idx, long draw_tiles_x, long draw_tiles_y, int subtile_size)
{
    RendererSetDrawFlags(0);
    scrtop_x += 4*units_per_pixel/16;
    scrtop_y -= 4*units_per_pixel/16;

    TbBool submitted = 0;
    unsigned short* tile_buf = (unsigned short*)KfxAlloc((size_t)draw_tiles_x * (size_t)draw_tiles_y * sizeof(unsigned short));
    if (tile_buf)
    {
        for (int dy = 0; dy < draw_tiles_y; dy++)
        {
            for (int dx = 0; dx < draw_tiles_x; dx++)
            {
                struct Map* mapblk = get_map_block_at(stl_x + dx, stl_y + dy);
                if (map_block_revealed(mapblk, plyr_idx))
                {
                    int k = element_top_face_texture(mapblk);
                    k = engine_remap_texture_blocks(stl_x + dx, stl_y + dy, k);
                    tile_buf[dy * draw_tiles_x + dx] = (unsigned short)k;
                } else
                {
                    tile_buf[dy * draw_tiles_x + dx] = 0xFFFF;
                }
            }
        }
        submitted = RendererSubmitZoomBoxTiles(tile_buf, draw_tiles_x, draw_tiles_y,
            scrtop_x, scrtop_y, subtile_size, subtile_size);
        if (submitted)
        {
            // GPU path skips unrevealed (0xFFFF) tiles -- fill them in here
            // so both backends look the same.
            int scr_y = scrtop_y;
            for (int dy = 0; dy < draw_tiles_y; dy++, scr_y += subtile_size)
            {
                int scr_x = scrtop_x;
                for (int dx = 0; dx < draw_tiles_x; dx++, scr_x += subtile_size)
                {
                    if (tile_buf[dy * draw_tiles_x + dx] == 0xFFFF)
                        UIRenderer_SubmitSolidBox(scr_x, scr_y, subtile_size, subtile_size, 1);
                }
            }
        }
        KfxFree(tile_buf);
    }
    else
    {
        WARNLOG("Zoom box tile buffer allocation failed; terrain not drawn");
    }

    // Software (or no GPU shaders compiled): draw directly into WScreen, the
    // only path that can write there at all.
    if (!submitted && lbDisplay.WScreen != NULL)
    {
        setup_vecs(lbDisplay.WScreen, 0, RendererScreenWidth(), MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
        int scr_y = scrtop_y;
        for (int map_dy = 0; map_dy < draw_tiles_y; map_dy++)
        {
            int scr_x = scrtop_x;
            for (int map_dx = 0; map_dx < draw_tiles_x; map_dx++)
            {
                struct Map* mapblk = get_map_block_at(stl_x + map_dx, stl_y + map_dy);
                if (map_block_revealed(mapblk, plyr_idx))
                {
                    int k = element_top_face_texture(mapblk);
                    k = engine_remap_texture_blocks(stl_x + map_dx, stl_y + map_dy, k);
                    draw_texture(scr_x, scr_y, subtile_size, subtile_size, k, 0, -1);
                } else
              {
                LbDrawBox(scr_x, scr_y, subtile_size, subtile_size, 1);
              }
              scr_x += subtile_size;
          }
          scr_y += subtile_size;
        }
    }
    RendererAddDrawFlags(Lb_SPRITE_OUTLINE);
    LbDrawBox(scrtop_x, scrtop_y, draw_tiles_x*subtile_size, draw_tiles_y*subtile_size, 0);
    RendererClearDrawFlags(Lb_SPRITE_OUTLINE);
}

void draw_zoom_box_things(long scrtop_x, long scrtop_y, int stl_x, int stl_y, PlayerNumber plyr_idx, long draw_tiles_x, long draw_tiles_y, int subtile_size)
{
    scrtop_x += 4 * units_per_pixel / 16;
    scrtop_y -= 4 * units_per_pixel / 16;
    UIRenderer_BeginZoomBoxOverlay(scrtop_x, scrtop_y,
        draw_tiles_x*subtile_size, draw_tiles_y*subtile_size);
    // Software gets its (scrtop_x, scrtop_y) origin from the graphics window
    // set just above (SwTargetWindowX()/Y(), applied inside the immediate
    // blitters) -- that mechanism is software-only, so GL's IR command
    // recording (raw x/y, no reference to lbDisplay.GraphicsWindowX/Y) needs
    // the same origin baked into the coordinates directly instead, matching
    // how draw_zoom_box_terrain() already submits absolute screen coords.
    long origin_x = 0, origin_y = 0;
    if (RendererGetActiveType() == RENDERER_OPENGL)
    {
        origin_x = scrtop_x;
        origin_y = scrtop_y;
    }
    int scr_y = origin_y;
    for (int map_dy = 0; map_dy < draw_tiles_y; map_dy++)
    {
        int scr_x = origin_x;
        for (int map_dx = 0; map_dx < draw_tiles_x; map_dx++)
        {
            struct Map* mapblk = get_map_block_at(stl_x + map_dx, stl_y + map_dy);
            if (map_block_revealed(mapblk, plyr_idx))
            {
                draw_zoom_box_things_on_mapblk(mapblk, subtile_size, scr_x, scr_y);
            }
            scr_x += subtile_size;
      }
      scr_y += subtile_size;
    }
    UIRenderer_EndZoomBoxOverlay(scrtop_x, scrtop_y,
        draw_tiles_x*subtile_size, draw_tiles_y*subtile_size);
}

/**
 * Draws a box near mouse with more detailed top view of map.
 * Requires screen to be locked before.
 */
void draw_zoom_box(void)
{
    struct PlayerInfo* player = get_my_player();

    long draw_tiles = 13;
    long subtile_unscaled = 8;
    if (local_state.minimap_zoom == 128)
    {
        draw_tiles = 6;
        subtile_unscaled = 18;
    } else
    if (local_state.minimap_zoom == 256)
    {
        draw_tiles = 9;
        subtile_unscaled = 12;
    } else
    if (local_state.minimap_zoom == 512)
    {
        draw_tiles = 12;
        subtile_unscaled = 9;
    } else
    if (local_state.minimap_zoom == 1024)
    {
        draw_tiles = 18;
        subtile_unscaled = 6;
    } else
    if (local_state.minimap_zoom == 2048)
    {
        draw_tiles = 36;
        subtile_unscaled = 3;
    }
    long draw_tiles_x = draw_tiles;
    long draw_tiles_y = draw_tiles;

    // Sizes of the parchment map on which we're drawing
    // Needed only to figure out map position pointed by cursor
    struct TbRect map_area;
    long block_size = get_parchment_map_area_rect(&map_area);
    // Mouse coordinates
    long mouse_x = GetMouseX();
    long mouse_y = GetMouseY();

    // zoom box block size
    const int subtile_size = scale_value_for_resolution(subtile_unscaled);

    // Drawing coordinates
    long scrtop_x = mouse_x + scale_value_for_resolution(24);
    long scrtop_y = mouse_y + scale_value_for_resolution(24);
    if (scrtop_x > MyScreenWidth-draw_tiles_x*subtile_size)
      scrtop_x = MyScreenWidth-draw_tiles_x*subtile_size;
    if (scrtop_x < 0)
        scrtop_x = 0;
    if (scrtop_y > MyScreenHeight-draw_tiles_y*subtile_size)
      scrtop_y = MyScreenHeight-draw_tiles_y*subtile_size;
    if (scrtop_y < 0)
        scrtop_y = 0;
    // Source map coordinates
    int stl_x = STL_PER_SLB * (mouse_x / pixel_size - map_area.left) / block_size - draw_tiles_x / 2;
    int stl_y = STL_PER_SLB * (mouse_y / pixel_size - map_area.top) / block_size - draw_tiles_y / 2;
    // Draw only on map area (do not allow zoom box to be empty)
    if ((stl_x < -draw_tiles_x/2) || (stl_x >= game.map_subtiles_x+1-draw_tiles_x/2)
     || (stl_y < -draw_tiles_y/2) || (stl_y >= game.map_subtiles_y+1-draw_tiles_y/2))
      return;

    RendererSetZoomBoxScreenRect(scrtop_x, scrtop_y, scrtop_x + draw_tiles_x*subtile_size, scrtop_y + draw_tiles_y*subtile_size);
    draw_zoom_box_terrain(scrtop_x, scrtop_y, stl_x, stl_y, player->id_number, draw_tiles_x, draw_tiles_y, subtile_size);
    // Draw thing sprites on the map
    draw_zoom_box_things(scrtop_x, scrtop_y, stl_x, stl_y, player->id_number, draw_tiles_x, draw_tiles_y, subtile_size);
    RendererClearZoomBoxScreenRect();
    // Draw sprites surrounding the box, dead-last so they're never covered
    // by the terrain/things content above.
    int bs_units_per_px;
    {
        const struct TbSprite* spr = get_button_sprite(GBS_parchment_map_frame_deco_b_tl);
        bs_units_per_px = (74 * units_per_pixel) / spr->SWidth;
    }
    int beg_x = scrtop_x - scale_value_for_resolution(20);
    int beg_y = scrtop_y - scale_value_for_resolution(24);
    int end_x = scrtop_x - scale_value_for_resolution(46) + draw_tiles_x * subtile_size;
    int end_y = scrtop_y - scale_value_for_resolution(58) + draw_tiles_y * subtile_size;
    UIRenderer_BeginTopOverlay();
    UIRenderer_SubmitButtonSprite(beg_x, beg_y, bs_units_per_px, get_button_sprite(GBS_parchment_map_frame_deco_b_tl));
    UIRenderer_SubmitButtonSprite(end_x, beg_y, bs_units_per_px, get_button_sprite(GBS_parchment_map_frame_deco_b_tr));
    UIRenderer_SubmitButtonSprite(beg_x, end_y, bs_units_per_px, get_button_sprite(GBS_parchment_map_frame_deco_b_bl));
    UIRenderer_SubmitButtonSprite(end_x, end_y, bs_units_per_px, get_button_sprite(GBS_parchment_map_frame_deco_b_br));
    UIRenderer_EndTopOverlay();
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
    turn_off_all_window_menus();
    if ((game.operation_flags & GOF_ShowGui) == 0)
      clear_flag(game.operation_flags, GOF_ShowPanel);
    else
      set_flag(game.operation_flags, GOF_ShowPanel);
    struct PlayerInfo* player = get_my_player();
    // GL removes the >320px instant-cut fallback entirely (P5.8b) -- that
    // cap is a software-only per-pixel-LUT performance limit, not a
    // fundamental one; see MapFadeSupportsNativeResolution()'s own comment.
    if (network_is_active()
        || (!MapFadePass_SupportsNativeResolution() && (RendererPhysicalWidth() > 320)))
    {
      set_players_packet_action(player, PckA_SaveViewType, PVT_MapScreen, 0, 0, 0);
      turn_off_roaming_menus();
    } else
    {
      set_players_packet_action(player, PckA_SetViewType, PVT_MapFadeIn, 0, 0, 0);
      turn_off_roaming_menus();
    }
}

void zoom_from_parchment_map(void)
{
    struct PlayerInfo* player = get_my_player();
    if (network_is_active()
        || (!MapFadePass_SupportsNativeResolution() && (RendererPhysicalWidth() > 320)))
    {
        set_players_packet_action(player, PckA_LoadViewType, PVT_DungeonTop, 0,0,0);
    } else
    {
        set_players_packet_action(player, PckA_SetViewType, PVT_MapFadeOut, 0,0,0);
    }
}
/******************************************************************************/
