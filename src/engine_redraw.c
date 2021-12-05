/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_redraw.c
 *     Functions to redraw the engine screen.
 * @par Purpose:
 *     High level redrawing routines.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     06 Nov 2010 - 03 Jul 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_redraw.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_sprfnt.h"
#include "bflib_sound.h"
#include "bflib_mouse.h"
#include "bflib_dernc.h"
#include "lvl_script.h"
#include "engine_arrays.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "player_instances.h"
#include "player_states.h"
#include "kjm_input.h"
#include "gui_parchment.h"
#include "gui_draw.h"
#include "gui_boxmenu.h"
#include "gui_msgs.h"
#include "gui_tooltips.h"
#include "power_hand.h"
#include "power_process.h"
#include "engine_render.h"
#include "engine_lenses.h"
#include "front_simple.h"
#include "front_easter.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_map.h"
#include "creature_graphics.h"
#include "vidmode.h"
#include "config.h"
#include "config_strings.h"
#include "config_terrain.h"
#include "config_players.h"
#include "magic.h"
#include "game_merge.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void redraw_isometric_view(void);
void redraw_frontview(void);
/******************************************************************************/
long xtab[640][2];
long ytab[480][2];
/******************************************************************************/
void setup_engine_window(long x, long y, long width, long height)
{
    SYNCDBG(6,"Starting for size (%ld,%ld) at (%ld,%ld)",width,height,x,y);
    struct PlayerInfo* player = get_my_player();
    if ((game.operation_flags & GOF_ShowGui) != 0)
    {
      if (x > MyScreenWidth)
        x = MyScreenWidth;
      if (x < status_panel_width)
        x = status_panel_width;
    } else
    {
      if (x > MyScreenWidth)
        x = MyScreenWidth;
      if (x < 0)
        x = 0;
    }
    if (y > MyScreenHeight)
      y = MyScreenHeight;
    if (y < 0)
      y = 0;
    if (x+width > MyScreenWidth)
      width = MyScreenWidth-x;
    if (width < 0)
      width = 0;
    if (y+height > MyScreenHeight)
      height = MyScreenHeight-y;
    if (height < 0)
      height = 0;
    player->engine_window_x = x;
    player->engine_window_y = y;
    player->engine_window_width = width;
    player->engine_window_height = height;
}

void store_engine_window(TbGraphicsWindow *ewnd,int divider)
{
    struct PlayerInfo* player = get_my_player();
    if (divider <= 1)
    {
        ewnd->x = player->engine_window_x;
        ewnd->y = player->engine_window_y;
        ewnd->width = player->engine_window_width;
        ewnd->height = player->engine_window_height;
    } else
    {
        ewnd->x = player->engine_window_x/divider;
        ewnd->y = player->engine_window_y/divider;
        ewnd->width = player->engine_window_width/divider;
        ewnd->height = player->engine_window_height/divider;
    }
    ewnd->ptr = NULL;
}

void load_engine_window(TbGraphicsWindow *ewnd)
{
    struct PlayerInfo* player = get_my_player();
    player->engine_window_x = ewnd->x;
    player->engine_window_y = ewnd->y;
    player->engine_window_width = ewnd->width;
    player->engine_window_height = ewnd->height;
}

void map_fade(unsigned char *outbuf, unsigned char *srcbuf1, unsigned char *srcbuf2, unsigned char *fade_tbl, unsigned char *ghost_tbl, long a6, long const xmax, long const ymax, long a9)
{
    //_DK_map_fade(outbuf, srcbuf1, srcbuf2, fade_tbl, ghost_tbl, a6, xmax, ymax, a9); return;
    long ix;
    long iy;
    long x1base = 4 * a6;
    long x0base = 4 * (32 - a6);
    long* xt = xtab[0];
    int vx0 = 0;
    int vx1 = 0;
    for (ix = xmax; ix > 0; ix--)
    {
        long val = x1base + vx1 / xmax;
        long m;
        if (val >= 0)
        {
            m = min(xmax,val);
        }
        else
        {
            m = 0;
        }
        xt[1] = m;
        val = x0base + vx0 / xmax;
        if (val >= 0) {
            m = min(xmax,val);
        } else {
            m = 0;
        }
        xt[0] = m;
        xt += 2;
        vx0 += xmax - 8 * (32 - a6);
        vx1 += xmax - 8 * a6;
    }

    long y1base = 8 * ymax / xmax * x1base / 8;
    long y0base = 8 * ymax / xmax * x0base / 8;
    long* yt = ytab[0];
    int vy1 = 0;
    int vy0 = 0;
    for (iy = ymax; iy > 0; iy--)
    {
        long val = y1base + vy1 / ymax;
        long m;
        if (val >= 0)
        {
            m = min(ymax,val);
        }
        else
        {
            m = 0;
        }
        yt[1] = xmax * m;
        val = y0base + vy0 / ymax;
        if (val >= 0)
        {
            m = min(ymax,val);
        } else {
            m = 0;
        }
        yt[0] = xmax * m;
        yt += 2;
        vy0 += ymax - 2 * y0base;
        vy1 += ymax - 2 * y1base;
    }

    x0base = a6 << 8;
    y0base = (32 - a6) << 8;
    unsigned char* out = outbuf;
    yt = ytab[0];
    for (iy = ymax; iy > 0; iy--)
    {
        unsigned char* sbuf2 = &srcbuf2[yt[1]];
        unsigned char* sbuf1 = &srcbuf1[yt[0]];
        xt = xtab[0];
        for (ix = xmax; ix > 0; ix--)
        {
            int px1 = fade_tbl[x0base + sbuf1[xt[0]]];
            int px2 = fade_tbl[y0base + sbuf2[xt[1]]];
            *out = ghost_tbl[256 * px2 + px1];
            out++;
            xt += 2;
        }
        out += a9 - xmax;
        yt += 2;
    }
}

void generate_map_fade_ghost_table(const char *fname, unsigned char *palette, unsigned char *ghost_table)
{
    if (LbFileLoadAt(fname, ghost_table) != PALETTE_COLORS*PALETTE_COLORS)
    {
        unsigned char* out = ghost_table;
        for (int i = 0; i < PALETTE_COLORS; i++)
        {
            unsigned char* bpal = &palette[3 * i];
            for (int n = 0; n < PALETTE_COLORS; n++)
            {
                unsigned char* spal = &palette[3 * n];
                unsigned char r = bpal[0] + spal[0];
                unsigned char g = bpal[1] + spal[1];
                unsigned char b = bpal[2] + spal[2];
                *out = LbPaletteFindColour(palette, r, g, b);
                out++;
            }
        }
        LbFileSaveAt(fname, ghost_table, PALETTE_COLORS*PALETTE_COLORS);
    }
}

/**
 * Renders source and destination screens for map fading.
 * Stores them in given buffers.
 * @param fade_src
 * @param fade_dest
 * @param scanline Line width of the two given buffers.
 * @param height Height to be filled in given buffers.
 */
void prepare_map_fade_buffers(unsigned char *fade_src, unsigned char *fade_dest, int scanline, int height)
{
    struct PlayerInfo* player = get_my_player();
    // render the 3D screen
    if (player->view_mode_restore == PVM_IsometricView)
      redraw_isometric_view();
    else
      redraw_frontview();
    // Copy the screen to fade source temp buffer
    int i;
    int fadebuf_pos = 0;
    for (i = 0; i < height; i++)
    {
        unsigned char* src = lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth * i;
        unsigned char* dst = &fade_src[fadebuf_pos];
        fadebuf_pos += scanline;
        memcpy(dst, src, MyScreenWidth/pixel_size);
    }
    // create the parchment screen
    load_parchment_file();
    redraw_minimal_overhead_view();
    // Copy the screen to fade destination temp buffer
    fadebuf_pos = 0;
    for (i = 0; i < height; i++)
    {
        unsigned char* src = lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth * i;
        unsigned char* dst = &fade_dest[fadebuf_pos];
        fadebuf_pos += scanline;
        memcpy(dst, src, MyScreenWidth/pixel_size);
    }
}

long map_fade_in(long a)
{
    SYNCDBG(6,"Starting");
    if (a == 0)
    {
        map_fade_ghost_table = poly_pool;
        map_fade_src = poly_pool + PALETTE_COLORS*PALETTE_COLORS;
        map_fade_dest = map_fade_src + 320*200;
        prepare_map_fade_buffers(map_fade_src, map_fade_dest, 320, MyScreenHeight/pixel_size);
        generate_map_fade_ghost_table("data/mapfadeg.dat", engine_palette, map_fade_ghost_table);
    }
    map_fade(lbDisplay.WScreen, map_fade_dest, map_fade_src, pixmap.fade_tables, map_fade_ghost_table,
      a, 320, 200, lbDisplay.GraphicsScreenWidth);
    long nxamount =  a + 4;
    if (nxamount > 32)
        nxamount = 32;
    return nxamount;
}

long map_fade_out(long a)
{
    SYNCDBG(6,"Starting");
    if (a == 32)
    {
        map_fade_ghost_table = poly_pool;
        map_fade_src = poly_pool + PALETTE_COLORS*PALETTE_COLORS;
        map_fade_dest = map_fade_src + 320*200;
        prepare_map_fade_buffers(map_fade_src, map_fade_dest, 320, MyScreenHeight/pixel_size);
        generate_map_fade_ghost_table("data/mapfadeg.dat", engine_palette, map_fade_ghost_table);
    }
    map_fade(lbDisplay.WScreen, map_fade_dest, map_fade_src, pixmap.fade_tables, map_fade_ghost_table,
      a, 320, 200, lbDisplay.GraphicsScreenWidth);
    long nxamount =  a - 4;
    if (a < 0)
        nxamount = 0;
    return nxamount;
}

void set_sprite_view_3d(void)
{
    for (long i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing))
        {
            if (thing_is_creature(thing) || ((thing->field_4F & TF4F_Unknown01) == 0))
            {
                int n = straight_iso_td(thing->anim_sprite);
                if (n >= 0)
                {
                    thing->anim_sprite = n;
                    long nframes = keepersprite_frames(thing->anim_sprite);
                    if (nframes != thing->field_49)
                    {
                        ERRORLOG("No frames different between views C%d, M%d, A%d, B%d",thing->class_id,thing->model,thing->field_49,nframes);
                        thing->field_49 = nframes;
                        n = thing->field_49 - 1;
                        if (n > thing->field_48) {
                            n = thing->field_48;
                        }
                        thing->field_48 = n;
                        thing->field_40 = n << 8;
                    }
                }
            }
        }
    }
}

void set_sprite_view_isometric(void)
{
    for (long i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing))
        {
            if (thing_is_creature(thing) || ((thing->field_4F & TF4F_Unknown01) == 0))
            {
                int n = straight_td_iso(thing->anim_sprite);
                if (n >= 0)
                {
                    thing->anim_sprite = n;
                    long nframes = keepersprite_frames(thing->anim_sprite);
                    if (nframes != thing->field_49)
                    {
                        ERRORLOG("No frames different between views C%d, M%d, A%d, B%d",thing->class_id,thing->model,thing->field_49,nframes);
                        thing->field_49 = nframes;
                        n = thing->field_49 - 1;
                        if (n > thing->field_48) {
                            n = thing->field_48;
                        }
                        thing->field_48 = n;
                        thing->field_40 = n << 8;
                    }
                }
            }
        }
    }
}

long dummy_sound_line_of_sight(long a1, long a2, long a3, long a4, long a5, long a6)
{
    return 1;
}

void set_engine_view(struct PlayerInfo *player, long val)
{
    switch ( val )
    {
    case PVM_EmptyView:
        player->acamera = &player->cameras[CamIV_Isometric];
        // Allow view mode 0 only for non-local-human players
        if (!is_my_player(player))
            break;
        // If it's local human player, then setting this mode is an error
        // fall through
    default:
        ERRORLOG("Invalid view mode %d",(int)val);
        val = PVM_CreatureView;
        // fall through
    case PVM_CreatureView:
        player->acamera = &player->cameras[CamIV_FirstPerson];
        if (!is_my_player(player))
            break;
        lens_mode = 2;
        set_sprite_view_3d();
        S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
        S3DSetDeadzoneRadius(0);
        LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1,(MyScreenHeight/pixel_size) >> 1);
        break;
    case PVM_IsometricView:
        player->acamera = &player->cameras[CamIV_Isometric];
        if (!is_my_player(player))
            break;
        lens_mode = 0;
        set_sprite_view_isometric();
        S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
        S3DSetDeadzoneRadius(1280);
        break;
    case PVM_ParchmentView:
        player->acamera = &player->cameras[CamIV_Parchment];
        if (!is_my_player(player))
            break;
        S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
        S3DSetDeadzoneRadius(1280);
        break;
    case PVM_ParchFadeIn:
    case PVM_ParchFadeOut:
        // In fade states, keep the settings unchanged
        break;
    case PVM_FrontView:
        player->acamera = &player->cameras[CamIV_FrontView];
        if (!is_my_player(player))
            break;
        lens_mode = 0;
        set_sprite_view_isometric();
        S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
        S3DSetDeadzoneRadius(1280);
        break;
    }
    player->view_mode = val;
}

void draw_overlay_compass(long base_x, long base_y)
{
    unsigned short flg_mem = lbDisplay.DrawFlags;
    LbTextSetFont(winfont);
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    int units_per_px = (16 * status_panel_width + 140 / 2) / 140;
    int tx_units_per_px = (22 * units_per_px) / LbTextLineHeight();
    int w = (LbSprFontCharWidth(lbFontPtr, '/') * tx_units_per_px / 16) / 2;
    int h = (LbSprFontCharHeight(lbFontPtr, '/') * tx_units_per_px / 16) / 2 + 2 * units_per_px / 16;
    struct PlayerInfo* player = get_my_player();
    const struct Camera* cam = player->acamera;
    int center_x = base_x * units_per_px / 16 + MapDiagonalLength / 2;
    int center_y = base_y * units_per_px / 16 + MapDiagonalLength / 2;
    int shift_x = (-(MapDiagonalLength * 7 / 16) * LbSinL(cam->orient_a)) >> LbFPMath_TrigmBits;
    int shift_y = (-(MapDiagonalLength * 7 / 16) * LbCosL(cam->orient_a)) >> LbFPMath_TrigmBits;
    if (LbScreenIsLocked()) {
        LbTextDrawResized(center_x + shift_x - w, center_y + shift_y - h, tx_units_per_px, get_string(GUIStr_MapN));
    }
    shift_x = ( (MapDiagonalLength*7/16) * LbSinL(cam->orient_a)) >> LbFPMath_TrigmBits;
    shift_y = ( (MapDiagonalLength*7/16) * LbCosL(cam->orient_a)) >> LbFPMath_TrigmBits;
    if (LbScreenIsLocked()) {
        LbTextDrawResized(center_x + shift_x - w, center_y + shift_y - h, tx_units_per_px, get_string(GUIStr_MapS));
    }
    shift_x = ( (MapDiagonalLength*7/16) * LbCosL(cam->orient_a)) >> LbFPMath_TrigmBits;
    shift_y = (-(MapDiagonalLength*7/16) * LbSinL(cam->orient_a)) >> LbFPMath_TrigmBits;
    if (LbScreenIsLocked()) {
        LbTextDrawResized(center_x + shift_x - w, center_y + shift_y - h, tx_units_per_px, get_string(GUIStr_MapE));
    }
    shift_x = (-(MapDiagonalLength*7/16) * LbCosL(cam->orient_a)) >> LbFPMath_TrigmBits;
    shift_y = ( (MapDiagonalLength*7/16) * LbSinL(cam->orient_a)) >> LbFPMath_TrigmBits;
    if (LbScreenIsLocked()) {
        LbTextDrawResized(center_x + shift_x - w, center_y + shift_y - h, tx_units_per_px, get_string(GUIStr_MapW));
    }
    lbDisplay.DrawFlags = flg_mem;
}

void redraw_creature_view(void)
{
    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_my_player();
    if (player->field_45F != 2)
      player->field_45F = 2;
    update_explored_flags_for_power_sight(player);
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    TRACE_THING(thing);
    if (!thing_is_invalid(thing))
      draw_creature_view(thing);
    if (smooth_on)
    {
        TbGraphicsWindow ewnd;
        store_engine_window(&ewnd, pixel_size);
        smooth_screen_area(lbDisplay.WScreen, ewnd.x, ewnd.y,
            ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
    }
    remove_explored_flags_for_power_sight(player);
    draw_swipe_graphic();
    if ((game.operation_flags & GOF_ShowGui) != 0) {
        draw_whole_status_panel();
    }
    draw_gui();
    if ((game.operation_flags & GOF_ShowGui) != 0) {
        draw_overlay_compass(player->minimap_pos_x, player->minimap_pos_y);
    }
    message_draw();
    gui_draw_all_boxes();
    draw_tooltip();
}

void smooth_screen_area(unsigned char *scrbuf, long x, long y, long w, long h, long scanln)
{
    SYNCDBG(7,"Starting");
    unsigned char* lnbuf = scrbuf + scanln * y + x;
    for (long i = h - y - 1; i > 0; i--)
    {
        unsigned char* buf = lnbuf;
        for (long k = w - x - 1; k > 0; k--)
        {
            unsigned int ghpos = (buf[0] << 8) + buf[1];
            ghpos = (buf[scanln] << 8) + pixmap.ghost[ghpos];
            buf[0] = ghpos;
            buf++;
      }
      lnbuf += scanln;
    }
}

void make_camera_deviations(struct PlayerInfo *player,struct Dungeon *dungeon)
{
    long x = player->acamera->mappos.x.val;
    long y = player->acamera->mappos.y.val;
    if (dungeon->camera_deviate_quake != 0)
    {
      x += UNSYNC_RANDOM(80) - 40;
      y += UNSYNC_RANDOM(80) - 40;
    }
    if (dungeon->camera_deviate_jump != 0)
    {
      x += ( (dungeon->camera_deviate_jump * LbSinL(player->acamera->orient_a) >> 8) >> 8);
      y += (-(dungeon->camera_deviate_jump * LbCosL(player->acamera->orient_a) >> 8) >> 8);
    }
    if ((dungeon->camera_deviate_quake != 0) || (dungeon->camera_deviate_jump != 0))
    {
      // bounding position
      if (x < 0)
      {
        x = 0;
      } else
      if (x > 65535)
      {
        x = 65535;
      }
      if (y < 0)
      {
        y = 0;
      } else
      if (y > 65535)
      {
        y = 65535;
      }
      // setting deviated position
      player->acamera->mappos.x.val = x;
      player->acamera->mappos.y.val = y;
    }
}

void redraw_isometric_view(void)
{
    SYNCDBG(6,"Starting");

    struct PlayerInfo* player = get_my_player();
    if (player->acamera == NULL)
        return;
    struct Coord3d pos;
    memcpy(&pos, &player->acamera->mappos, sizeof(struct Coord3d));
    TbGraphicsWindow ewnd;
    LbMemorySet(&ewnd, 0, sizeof(TbGraphicsWindow));
    if (player->field_45F != 1)
      player->field_45F = 1;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    // Camera position modifications
    make_camera_deviations(player,dungeon);
    update_explored_flags_for_power_sight(player);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
        store_engine_window(&ewnd,1);
        setup_engine_window(ewnd.x, ewnd.y, ewnd.width >> 1, ewnd.height >> 1);
    }
    engine(player,&player->cameras[CamIV_Isometric]);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
        load_engine_window(&ewnd);
    }
    if (smooth_on)
    {
        store_engine_window(&ewnd,pixel_size);
        smooth_screen_area(lbDisplay.WScreen, ewnd.x, ewnd.y,
            ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
    }
    remove_explored_flags_for_power_sight(player);
    if ((game.operation_flags & GOF_ShowGui) != 0) {
        draw_whole_status_panel();
    }
    draw_gui();
    if ((game.operation_flags & GOF_ShowGui) != 0) {
        draw_overlay_compass(player->minimap_pos_x, player->minimap_pos_y);
    }
    message_draw();
    gui_draw_all_boxes();
    draw_power_hand();
    draw_tooltip();
    memcpy(&player->acamera->mappos,&pos,sizeof(struct Coord3d));
    SYNCDBG(8,"Finished");
}

void redraw_frontview(void)
{
    SYNCDBG(6,"Starting");
    long w;
    long h;
    struct PlayerInfo* player = get_my_player();
    update_explored_flags_for_power_sight(player);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
      w = player->engine_window_width;
      h = player->engine_window_height;
      setup_engine_window(player->engine_window_x, player->engine_window_y, w, h >> 1);
    } else
    {
      w = 0;
      h = 0;
    }
    draw_frontview_engine(&player->cameras[CamIV_FrontView]);
    if ((game.flags_font & FFlg_unk08) != 0)
      setup_engine_window(player->engine_window_x, player->engine_window_y, w, h);
    remove_explored_flags_for_power_sight(player);
    if ((game.operation_flags & GOF_ShowGui) != 0) {
        draw_whole_status_panel();
    }
    draw_gui();
    if ((game.operation_flags & GOF_ShowGui) != 0) {
        draw_overlay_compass(player->minimap_pos_x, player->minimap_pos_y);
    }
    message_draw();
    draw_power_hand();
    draw_tooltip();
    gui_draw_all_boxes();
}

int get_place_room_pointer_graphics(RoomKind rkind)
{
    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    return roomst->pointer_sprite_idx;
}

int get_place_trap_pointer_graphics(ThingModel trmodel)
{
    struct TrapConfigStats* trapst = get_trap_model_stats(trmodel);
    return trapst->pointer_sprite_idx;
}

int get_place_door_pointer_graphics(ThingModel drmodel)
{
    struct DoorConfigStats* doorst = get_door_model_stats(drmodel);
    return doorst->pointer_sprite_idx;
}

/**
 * Draws a cursor for given spell.
 *
 * @return Gives true if cursor spell was drawn, false if the spell wasn't available and either no cursor or block cursor was drawn.
 */
TbBool draw_spell_cursor(unsigned char wrkstate, unsigned short tng_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    long i;
    long pwkind = -1;
    if (wrkstate < PLAYER_STATES_COUNT)
      pwkind = player_state_to_power_kind[wrkstate];
    SYNCDBG(5,"Starting for power %d",(int)pwkind);
    if (pwkind <= 0)
    {
        set_pointer_graphic(MousePG_Invisible);
        return false;
    }
    struct PlayerInfo* player = get_my_player();
    struct Thing* thing = thing_get(tng_idx);
    TbBool allow_cast = false;
    const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
    allow_cast = can_cast_spell(player->id_number, pwkind, stl_x, stl_y, thing, CastChk_SkipThing);
    if (!allow_cast)
    {
        set_pointer_graphic(MousePG_DenyMark);
        return false;
    }
    Expand_Check_Func chkfunc = powerst->overcharge_check;
    if (chkfunc != NULL)
    {
        if (chkfunc())
        {
            i = get_power_overcharge_level(player);
            set_pointer_graphic(MousePG_SpellCharge0+i);
            const struct MagicStats* pwrdynst = get_power_dynamic_stats(pwkind);
            draw_spell_cost = pwrdynst->cost[i];
            return true;
        }
    }
    i = powerst->pointer_sprite_idx;
    set_pointer_graphic_spell(i, game.play_gameturn);
    return true;
}

void process_dungeon_top_pointer_graphic(struct PlayerInfo *player)
{
    struct Thing *thing;
    struct Dungeon* dungeon = get_dungeon(player->id_number);
    if (dungeon_invalid(dungeon))
    {
        set_pointer_graphic(MousePG_Invisible);
        return;
    }
    // During fade
    if (player->instance_num == PI_MapFadeFrom)
    {
        set_pointer_graphic(MousePG_Invisible);
        return;
    }
    // Mouse over panel map
    if (((game.operation_flags & GOF_ShowGui) != 0) && mouse_is_over_pannel_map(player->minimap_pos_x, player->minimap_pos_y))
    {
        if (game.small_map_state == 2) {
            set_pointer_graphic(MousePG_Invisible);
        } else {
            set_pointer_graphic(MousePG_Arrow);
        }
        return;
    }
    // Mouse over battle message box
    if (battle_creature_over > 0)
    {
        PowerKind pwkind = 0;
        if (player->work_state < PLAYER_STATES_COUNT)
            pwkind = player_state_to_power_kind[player->work_state];
        thing = thing_get(battle_creature_over);
        TRACE_THING(thing);
        if (can_cast_spell(player->id_number, pwkind, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default))
        {
            draw_spell_cursor(player->work_state, battle_creature_over, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        } else
        {
            set_pointer_graphic(MousePG_Arrow);
        }
        return;
    }
    // GUI action being processed
    if (game_is_busy_doing_gui())
    {
        set_pointer_graphic(MousePG_Arrow);
        return;
    }
    long i;
    switch (player->work_state)
    {
    case PSt_CtrlDungeon:
        if (player->secondary_cursor_state)
          i = player->secondary_cursor_state;
        else
          i = player->primary_cursor_state;
        switch (i)
        {
        case CSt_PickAxe:
            set_pointer_graphic(MousePG_Pickaxe);
            break;
        case CSt_DoorKey:
            set_pointer_graphic(MousePG_LockMark);
            break;
        case CSt_PowerHand:
            thing = thing_get(player->thing_under_hand);
            TRACE_THING(thing);
            if ((player->input_crtr_control) && (!thing_is_invalid(thing)) && (dungeon->things_in_hand[0] != player->thing_under_hand))
            {
                if (can_cast_spell(player->id_number, player_state_to_power_kind[PSt_CtrlDirect],
                  thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default)) {
                    // The condition above makes can_cast_spell() within draw_spell_cursor() to never fail; this is intentional
                    draw_spell_cursor(PSt_CtrlDirect, 0, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
                } else {
                    set_pointer_graphic(MousePG_Arrow);
                }
                player->flgfield_6 |= PlaF6_Unknown01;
            } else
            if (((player->input_crtr_query) && !thing_is_invalid(thing)) && (dungeon->things_in_hand[0] != player->thing_under_hand)
                && can_thing_be_queried(thing, player->id_number))
            {
                set_pointer_graphic(MousePG_Query);
                player->flgfield_6 |= PlaF6_Unknown01;
            } else
            {
                if ((player->additional_flags & PlaAF_ChosenSubTileIsHigh) != 0) {
                  set_pointer_graphic(MousePG_Pickaxe);
                } else {
                  set_pointer_graphic(MousePG_Invisible);
                }
            }
            break;
        default:
            if (player->hand_busy_until_turn <= game.play_gameturn)
              set_pointer_graphic(MousePG_Arrow);
            else
              set_pointer_graphic(MousePG_Invisible);
            break;
        }
        break;
    case PSt_BuildRoom:
        i = get_place_room_pointer_graphics(player->chosen_room_kind);
        set_pointer_graphic(i);
        break;
    case PSt_HoldInHand:
    case PSt_Slap:
        set_pointer_graphic(MousePG_Invisible);
        break;
    case PSt_CallToArms:
    case PSt_CaveIn:
    case PSt_SightOfEvil:
    case PSt_CtrlPassngr:
    case PSt_CtrlDirect:
    case PSt_Lightning:
    case PSt_SpeedUp:
    case PSt_Armour:
    case PSt_Conceal:
    case PSt_Heal:
    case PSt_CreateDigger:
    case PSt_DestroyWalls:
    case PSt_CastDisease:
    case PSt_TurnChicken:
    case PSt_FreeDestroyWalls:
    case PSt_FreeCastDisease:
    case PSt_FreeTurnChicken:
    case PSt_FreeCtrlPassngr:
    case PSt_FreeCtrlDirect:
        draw_spell_cursor(player->work_state, 0, game.pos_14C006.x.stl.num, game.pos_14C006.y.stl.num);
        break;
    case PSt_CreatrQuery:
    case PSt_CreatrInfo:
    case PSt_CreatrInfoAll:
    case PSt_CreatrQueryAll:
        set_pointer_graphic(MousePG_Query);
        break;
    case PSt_PlaceTrap:
        i = get_place_trap_pointer_graphics(player->chosen_trap_kind);
        set_pointer_graphic(i);
        break;
    case PSt_PlaceDoor:
        i = get_place_door_pointer_graphics(player->chosen_door_kind);
        set_pointer_graphic(i);
        break;
    case PSt_Sell:
        set_pointer_graphic(MousePG_Sell);
        break;
    default:
        set_pointer_graphic(MousePG_Arrow);
        break;
    }
}

void process_pointer_graphic(void)
{
    struct PlayerInfo* player = get_my_player();
    SYNCDBG(6,"Starting for view %d, player state %s, instance %d",(int)player->view_type,player_state_code_name(player->work_state),(int)player->instance_num);
    switch (player->view_type)
    {
    case PVT_DungeonTop:
        // This case is complicated
        process_dungeon_top_pointer_graphic(player);
        break;
    case PVT_CreatureContrl:
    case PVT_CreaturePasngr:
        if ((game.numfield_D & GNFldD_CreaturePasngr) != 0)
          set_pointer_graphic(MousePG_Arrow);
        else
          set_pointer_graphic(MousePG_Invisible);
        break;
    case PVT_MapScreen:
    case PVT_MapFadeIn:
    case PVT_MapFadeOut:
        set_pointer_graphic(MousePG_Arrow);
        break;
    case PVT_None:
        set_pointer_graphic_none();
        break;
    default:
        WARNLOG("Unsupported view type");
        set_pointer_graphic_none();
        break;
    }
}

void redraw_display(void)
{
    const char *text;
    SYNCDBG(5,"Starting");
    struct PlayerInfo* player = get_my_player();
    player->flgfield_6 &= ~PlaF6_Unknown01;
    if (game.game_kind == GKind_Unknown1)
      return;
    if (game.small_map_state == 2)
      set_pointer_graphic_none();
    else
      process_pointer_graphic();
    switch (player->view_mode)
    {
    case PVM_EmptyView:
        break;
    case PVM_CreatureView:
        redraw_creature_view();
        parchment_loaded = 0;
        break;
    case PVM_IsometricView:
        redraw_isometric_view();
        parchment_loaded = 0;
        break;
    case PVM_ParchmentView:
        redraw_parchment_view();
        break;
    case PVM_FrontView:
        redraw_frontview();
        parchment_loaded = 0;
        break;
    case PVM_ParchFadeIn:
        parchment_loaded = 0;
        player->field_4BD = map_fade_in(player->field_4BD);
        break;
    case PVM_ParchFadeOut:
        parchment_loaded = 0;
        player->field_4BD = map_fade_out(player->field_4BD);
        break;
    default:
        ERRORLOG("Unsupported drawing state, %d",(int)player->view_mode);
        break;
    }
    //LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    LbTextSetFont(winfont);
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    if ((player->allocflags & PlaF_NewMPMessage) != 0)
    {
        text = buf_sprintf( ">%s_", player->mp_message_text);
        long pos_x = 148*units_per_pixel/16;
        long pos_y = 8*units_per_pixel/16;
        if (game.armageddon_cast_turn != 0)
        {
            if ( (bonus_timer_enabled()) || (script_timer_enabled()) || display_variable_enabled() )
            {
                pos_y = ((pos_y << 3) + ((LbTextLineHeight()*units_per_pixel/16) * game.active_messages_count));
            }
        }
        LbTextDrawResized(pos_x, pos_y, tx_units_per_px, text);
    }
    if ( draw_spell_cost )
    {
        unsigned short drwflags_mem = lbDisplay.DrawFlags;
        LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
        lbDisplay.DrawFlags = 0;
        LbTextSetFont(winfont);
        text = buf_sprintf("%d", draw_spell_cost);
        long pos_y = GetMouseY() - (LbTextStringHeight(text) * units_per_pixel / 16) / 2 - 2 * units_per_pixel / 16;
        long pos_x = GetMouseX() - (LbTextStringWidth(text) * units_per_pixel / 16) / 2;
        LbTextDrawResized(pos_x, pos_y, tx_units_per_px, text);
        lbDisplay.DrawFlags = drwflags_mem;
        draw_spell_cost = 0;
    }
    if (bonus_timer_enabled())
    {
        draw_bonus_timer();
    }
    else if (script_timer_enabled())
    {
        draw_script_timer(gameadd.script_player, gameadd.script_timer_id, gameadd.script_timer_limit, gameadd.timer_real);
    }
    if (display_variable_enabled())
    {
        draw_script_variable(gameadd.script_player, gameadd.script_value_type, gameadd.script_value_id, gameadd.script_variable_target, gameadd.script_variable_target_type);
    }
    if (timer_enabled())
    {
        draw_timer();
    }
    if (((game.operation_flags & GOF_Paused) != 0) && ((game.operation_flags & GOF_WorldInfluence) == 0))
    {
          LbTextSetFont(winfont);
          text = get_string(GUIStr_PausedMsg);
          int i = LbTextCharWidth(' ') * units_per_pixel / 16;
          long w = (LbTextStringWidth(text) * units_per_pixel / 16 + 2 * i);
          i = player->view_mode;
          long pos_x;
          if ((i == PVM_IsometricView) || (i == PVM_FrontView) || (i == PVM_CreatureView))
              pos_x = player->engine_window_x + (MyScreenWidth - w - player->engine_window_x) / 2;
          else
            pos_x = (MyScreenWidth-w)/2;
          long pos_y = 16 * units_per_pixel / 16;
          i = LbTextLineHeight()*units_per_pixel/16;
          lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
          long h = i + i / 2;
          LbTextSetWindow(pos_x, pos_y, w, h);
          draw_slab64k(pos_x, pos_y, units_per_pixel, w, h);
          LbTextDrawResized(0/pixel_size, 0/pixel_size, units_per_pixel, text);
          LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    }
    if (game.armageddon_cast_turn != 0)
    {
        int i;
        if (game.armageddon.count_down + game.armageddon_cast_turn <= game.play_gameturn)
        {
            i = 0;
            if (game.armageddon_field_15035A - game.armageddon.duration <= game.play_gameturn)
                i = game.armageddon_field_15035A - game.play_gameturn;
      } else
      {
        i = game.play_gameturn - game.armageddon_cast_turn - game.armageddon.count_down;
      }
      LbTextSetFont(winfont);
      text = buf_sprintf(" %s %03d", get_string(get_power_name_strindex(PwrK_ARMAGEDDON)), i/2); // Armageddon message
      i = LbTextCharWidth(' ')*units_per_pixel/16;
      long w = LbTextStringWidth(text) * units_per_pixel / 16 + 6 * i;
      long pos_x = MyScreenWidth - w - 16 * units_per_pixel / 16;
      long pos_y = 16 * units_per_pixel / 16;
      i = LbTextLineHeight()*units_per_pixel/16;
      lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
      long h = pixel_size * i + pixel_size * i / 2;
      LbTextSetWindow(pos_x, pos_y, w, h);
      draw_slab64k(pos_x, pos_y, units_per_pixel, w, h);
      LbTextDrawResized(0/pixel_size, 0/pixel_size, tx_units_per_px, text);
      LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    }
    draw_eastegg();
  //show_onscreen_msg(8, "Physical(%d,%d) Graphics(%d,%d) Lens(%d,%d)", (int)lbDisplay.PhysicalScreenWidth, (int)lbDisplay.PhysicalScreenHeight, (int)lbDisplay.GraphicsScreenWidth, (int)lbDisplay.GraphicsScreenHeight, (int)eye_lens_width, (int)eye_lens_height);
    SYNCDBG(7,"Finished");
}

/**
 * Redraws the game display buffer.
 */
TbBool keeper_screen_redraw(void)
{
    SYNCDBG(5,"Starting");
    struct PlayerInfo* player = get_my_player();
    LbScreenClear(0);
    if (LbScreenLock() == Lb_SUCCESS)
    {
        setup_engine_window(player->engine_window_x, player->engine_window_y,
            player->engine_window_width, player->engine_window_height);
        redraw_display();
        LbScreenUnlock();
        return true;
    }
    return false;
}
/******************************************************************************/
