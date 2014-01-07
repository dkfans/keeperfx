/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_map.c
 *     Map on in-game GUI panel drawing and support functions.
 * @par Purpose:
 *     On-screen drawing of the map area in GUI panel.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 23 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontmenu_ingame_map.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_sprfnt.h"
#include "bflib_math.h"
#include "bflib_guibtns.h"
#include "bflib_mouse.h"
#include "bflib_planar.h"

#include "front_input.h"
#include "player_data.h"
#include "game_legacy.h"
#include "creature_states.h"
#include "creature_battle.h"
#include "config_creature.h"
#include "power_hand.h"
#include "kjm_input.h"
#include "frontmenu_ingame_tabs.h"
#include "vidfade.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_pannel_map_update(long x, long y, long w, long h);
DLLIMPORT void _DK_gui_set_button_flashing(long a1, long a2);
DLLIMPORT void _DK_do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5);
DLLIMPORT void _DK_pannel_map_draw(long x, long y, long zoom);
DLLIMPORT void _DK_draw_overlay_things(long zoom);
DLLIMPORT void _DK_draw_call_to_arms_circle(unsigned char owner, long x1, long y1, long x2, long y2, long zoom);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void pannel_map_draw_pixel(long x, long y, long col)
{
    if ((y >= 0) && (y < 116))
    {
        if ((x >= MapShapeStart[y]) && (x < MapShapeEnd[y]))
        {
            lbDisplay.WScreen[(PannelMapY + y) * lbDisplay.GraphicsScreenWidth + (PannelMapX + x)] = col;
        }
    }
}

/**
 * Draws single call to arms overlay on minimap.
 * @param owner
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param zoom
 */
void draw_call_to_arms_circle(unsigned char owner, long x1, long y1, long x2, long y2, long zoom)
{
    _DK_draw_call_to_arms_circle(owner, x1, y1, x2, y2, zoom); return;
}

/**
 * Draws all call to arms objects on minimap.
 * @param player The player for whom drawing occurs.
 * @param zoom Zoom level of the minimap.
 * @return Amount of objects drawn.
 */
int draw_overlay_call_to_arms(struct PlayerInfo *player, long zoom)
{
    unsigned long k;
    int i,n;
    SYNCDBG(18,"Starting");
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam;
    cam = player->acamera;
    n = 0;
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
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
        if (((thing->alloc_flags & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
        {
            if (thing->model == 24)
            {
                long zmpos_x, zmpos_y;
                zmpos_x = ((thing->mappos.x.stl.num - (MapSubtlDelta)cam->mappos.x.stl.num) << 8) / zoom;
                zmpos_y = ((thing->mappos.y.stl.num - (MapSubtlDelta)cam->mappos.y.stl.num) << 8) / zoom;
                long mapos_x, mapos_y;
                mapos_x = (zmpos_x * LbCosL(cam->orient_a) + zmpos_y * LbSinL(cam->orient_a)) >> 16;
                mapos_y = (zmpos_y * LbCosL(cam->orient_a) - zmpos_x * LbSinL(cam->orient_a)) >> 16;
                draw_call_to_arms_circle(thing->owner, 0, 0, mapos_x, mapos_y, zoom);
                n++;
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

/**
 * Draws all owned traps on minimap.
 * @param player The player for whom drawing occurs.
 * @param zoom Zoom level of the minimap.
 * @return Amount of traps drawn.
 */
int draw_overlay_traps(struct PlayerInfo *player, long zoom)
{
    unsigned long k;
    int i,n;
    SYNCDBG(18,"Starting");
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam;
    cam = player->acamera;
    n = 0;
    k = 0;
    i = game.thing_lists[TngList_Traps].index;
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
        if (player->id_number == thing->owner)
        {
            long zmpos_x, zmpos_y;
            zmpos_x = ((thing->mappos.x.stl.num - (MapSubtlDelta)cam->mappos.x.stl.num) << 8) / zoom;
            zmpos_y = ((thing->mappos.y.stl.num - (MapSubtlDelta)cam->mappos.y.stl.num) << 8) / zoom;
            long mapos_x, mapos_y;
            mapos_x = (zmpos_x * LbCosL(cam->orient_a) + zmpos_y * LbSinL(cam->orient_a)) >> 16;
            mapos_y = (zmpos_y * LbCosL(cam->orient_a) - zmpos_x * LbSinL(cam->orient_a)) >> 16;
            long basepos;
            basepos = 116/pixel_size/2;
            if (((mapos_x+basepos) * pixel_size >= 0) && ((mapos_x+basepos) * pixel_size < 116))
            {
                if (((mapos_y+basepos) * pixel_size >= 0) && ((mapos_y+basepos) * pixel_size < 116))
                {
                    if ((thing->byte_18) || (player->id_number == thing->owner))
                    {
                        TbPixel col;
                        if ((thing->model == gui_trap_type_highlighted) && (game.play_gameturn & 1)) {
                            col = 31;
                        } else {
                            col = 60;
                        }
                        pannel_map_draw_pixel(mapos_x+basepos,   mapos_y+basepos,   col);
                        pannel_map_draw_pixel(mapos_x+basepos-1, mapos_y+basepos,   col);
                        pannel_map_draw_pixel(mapos_x+basepos+1, mapos_y+basepos,   col);
                        pannel_map_draw_pixel(mapos_x+basepos,   mapos_y+basepos+1, col);
                        pannel_map_draw_pixel(mapos_x+basepos,   mapos_y+basepos-1, col);
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

/**
 * Draws all spells and specials on minimap.
 * @param player The player for whom drawing occurs.
 * @param zoom Zoom level of the minimap.
 * @return Amount of objects drawn.
 */
int draw_overlay_spells_and_boxes(struct PlayerInfo *player, long zoom)
{
    unsigned long k;
    int i,n;
    SYNCDBG(18,"Starting");
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam;
    cam = player->acamera;
    n = 0;
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
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
        if (((thing->alloc_flags & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
        {
            if (subtile_revealed(thing->mappos.x.stl.num, thing->mappos.y.stl.num, player->id_number))
            {
                long zmpos_x, zmpos_y;
                zmpos_x = ((thing->mappos.x.stl.num - (MapSubtlDelta)cam->mappos.x.stl.num) << 8) / zoom;
                zmpos_y = ((thing->mappos.y.stl.num - (MapSubtlDelta)cam->mappos.y.stl.num) << 8) / zoom;
                long mapos_x, mapos_y;
                mapos_x = (zmpos_x * LbCosL(cam->orient_a) + zmpos_y * LbSinL(cam->orient_a)) >> 16;
                mapos_y = (zmpos_y * LbCosL(cam->orient_a) - zmpos_x * LbSinL(cam->orient_a)) >> 16;
                long basepos;
                basepos = 116/pixel_size/2;
                if (((mapos_x+basepos) * pixel_size >= 0) && ((mapos_x+basepos) * pixel_size < 116))
                {
                    if (((mapos_y+basepos) * pixel_size >= 0) && ((mapos_y+basepos) * pixel_size < 116))
                    {
                        if ((thing->byte_18) || (player->id_number == thing->owner))
                        {
                            if (thing_is_special_box(thing) || thing_is_spellbook(thing)) {
                                pannel_map_draw_pixel(mapos_x+basepos, mapos_y+basepos, colours[15][0][15]);
                                n++;
                            }
                        }
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

int draw_overlay_creatures(struct PlayerInfo *player, long zoom)
{
    unsigned long k;
    int i,n;
    SYNCDBG(18,"Starting");
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam;
    cam = player->acamera;
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
        TbPixel col1, col2, col;
        col1 = 31;
        col2 = 1;
        if (((thing->alloc_flags & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
        {
            if (subtile_revealed(thing->mappos.x.stl.num, thing->mappos.y.stl.num, player->id_number))
            {
                if ((game.play_gameturn & 4) == 0)
                {
                    col1 = player_room_colours[thing->owner];
                    col2 = player_room_colours[thing->owner];
                }
                long zmpos_x, zmpos_y;
                zmpos_x = ((thing->mappos.x.stl.num - (MapSubtlDelta)cam->mappos.x.stl.num) << 8) / zoom;
                zmpos_y = ((thing->mappos.y.stl.num - (MapSubtlDelta)cam->mappos.y.stl.num) << 8) / zoom;
                long mapos_x, mapos_y;
                mapos_x = (zmpos_x * LbCosL(cam->orient_a) + zmpos_y * LbSinL(cam->orient_a)) >> 16;
                mapos_y = (zmpos_y * LbCosL(cam->orient_a) - zmpos_x * LbSinL(cam->orient_a)) >> 16;
                long basepos;
                basepos = 116/pixel_size/2;
                if (((mapos_x+basepos) * pixel_size >= 0) && ((mapos_x+basepos) * pixel_size < 116))
                {
                    if (((mapos_y+basepos) * pixel_size >= 0) && ((mapos_y+basepos) * pixel_size < 116))
                    {
                        if (thing->owner == player->id_number)
                        {
                            if ((thing->model == gui_creature_type_highlighted) && (game.play_gameturn & 1))
                            {
                                pannel_map_draw_pixel(mapos_x+basepos,   mapos_y+basepos,   31);
                                pannel_map_draw_pixel(mapos_x+basepos-1, mapos_y+basepos,   col2);
                                pannel_map_draw_pixel(mapos_x+basepos+1, mapos_y+basepos,   col2);
                                pannel_map_draw_pixel(mapos_x+basepos,   mapos_y+basepos,   col2);
                                pannel_map_draw_pixel(mapos_x+basepos,   mapos_y+basepos-1, col2);
                            } else
                            {
                                pannel_map_draw_pixel(mapos_x+basepos, mapos_y+basepos, col2);
                            }
                        } else
                        {
                            if (thing->owner == game.neutral_player_num) {
                                col = player_room_colours[(game.play_gameturn + 1) & 3];
                            } else {
                                col = col1;
                            }
                            pannel_map_draw_pixel(mapos_x+basepos, mapos_y+basepos, col);
                        }
                    }
                }
            }
            // Hero tunnelers may be visible even on unrevealed terrain
            if (thing->model == get_players_special_digger_breed(game.hero_player_num))
            {
                CrtrStateId crstat;
                crstat = get_creature_state_besides_move(thing);
                if ((crstat == CrSt_Tunnelling) || (crstat == CrSt_TunnellerDoingNothing))
                {
                    struct CreatureControl *cctrl;
                    cctrl = creature_control_get_from_thing(thing);
                    int m;
                    for (m=0; m < 5; m++)
                    {
                        long memberpos;
                        memberpos = cctrl->party.member_pos_stl[m];
                        if (memberpos == 0)
                            break;
                        if ((game.play_gameturn & 4) == 0)
                        {
                            col1 = player_room_colours[thing->owner];
                            col2 = player_room_colours[thing->owner];
                        }
                        long zmpos_x, zmpos_y;
                        zmpos_x = ((stl_num_decode_x(memberpos) - (MapSubtlDelta)cam->mappos.x.stl.num) << 8) / zoom;
                        zmpos_y = ((stl_num_decode_y(memberpos) - (MapSubtlDelta)cam->mappos.y.stl.num) << 8) / zoom;
                        long mapos_x, mapos_y;
                        mapos_x = (zmpos_x * LbCosL(cam->orient_a) + zmpos_y * LbSinL(cam->orient_a)) >> 16;
                        mapos_y = (zmpos_y * LbCosL(cam->orient_a) - zmpos_x * LbSinL(cam->orient_a)) >> 16;
                        long basepos;
                        basepos = 116/pixel_size/2;
                        if (((mapos_x+basepos) * pixel_size >= 0) && ((mapos_x+basepos) * pixel_size < 116))
                        {
                            if (((mapos_y+basepos) * pixel_size >= 0) && ((mapos_y+basepos) * pixel_size < 116))
                            {
                                if (thing->owner == player->id_number) {
                                    col = col2;
                                } else {
                                    col = col1;
                                }
                                pannel_map_draw_pixel(mapos_x+basepos, mapos_y+basepos, col);
                            }
                        }
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

/**
 * Draws own dungeon heart line on minimap.
 * @param player The player for whom drawing occurs.
 * @param zoom Zoom level of the minimap.
 * @return Amount of hearts drawn, either 0 or 1.
 */
int draw_line_to_heart(struct PlayerInfo *player, long zoom)
{
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam;
    cam = player->acamera;
    struct Thing *heartng;
    heartng = get_player_soul_container(player->id_number);
    if (thing_is_invalid(heartng)) {
        return 0;
    }
    lbDisplay.DrawFlags |= 0x0004;
    long zmpos_x, zmpos_y;
    zmpos_x = ((heartng->mappos.x.stl.num - (MapSubtlDelta)cam->mappos.x.stl.num) << 8) / zoom;
    zmpos_y = ((heartng->mappos.y.stl.num - (MapSubtlDelta)cam->mappos.y.stl.num) << 8) / zoom;
    long mapos_x, mapos_y;
    mapos_x = (zmpos_x * LbCosL(cam->orient_a) + zmpos_y * LbSinL(cam->orient_a)) >> 16;
    mapos_y = (zmpos_y * LbCosL(cam->orient_a) - zmpos_x * LbSinL(cam->orient_a)) >> 16;
    long basepos;
    basepos = 116/pixel_size/2;
    long dist, angle;
    dist = get_distance_xy(basepos, basepos, mapos_x + basepos, mapos_y + basepos);
    angle = -(LbArcTanAngle(mapos_x, mapos_y) & LbFPMath_AngleMask) & 0x1FFC;
    int delta_x, delta_y;
    delta_x = -1024 * LbSinL(angle) >> 16;
    delta_y = -1024 * LbCosL(angle) >> 16;
    long frame;
    frame = (game.play_gameturn & 3) + 1;
    int draw_x, draw_y;
    draw_x = delta_x / 2 + (frame * delta_x) / 4 + (basepos << 8);
    draw_y = delta_y / 2 + (frame * delta_y) / 4 + (basepos << 8);
    int i;
    for (i = dist - 4; i > 0; i -= 4)
    {
        if ((pixel_size * draw_x < 0) || (pixel_size * draw_x >= 29696))
            break;
        if ((pixel_size * draw_y < 0) || (pixel_size * draw_y >= 29696))
            break;
        draw_x += delta_x;
        draw_y += delta_y;
        pannel_map_draw_pixel(draw_x >> 8, draw_y >> 8, 15);
    }
    lbDisplay.DrawFlags &= ~0x0004;
    return 1;
}

void draw_overlay_things(long zoom)
{
    SYNCDBG(7,"Starting");
    //_DK_draw_overlay_things(zoom); return;
    struct PlayerInfo *player;
    player = get_my_player();
    draw_overlay_call_to_arms(player, zoom);
    draw_overlay_traps(player, zoom);
    draw_overlay_creatures(player, zoom);
    if ((game.play_gameturn & 3) == 1) {
        draw_overlay_spells_and_boxes(player, zoom);
    }
    draw_line_to_heart(player, zoom);
    if ((game.play_gameturn & 1) && (player->acamera->field_6 == 1))
    {
        pannel_map_draw_pixel(116/pixel_size/2,   116/pixel_size/2,   colours[15][15][15]);
        pannel_map_draw_pixel(116/pixel_size/2-1, 116/pixel_size/2,   colours[15][15][15]);
        pannel_map_draw_pixel(116/pixel_size/2+1, 116/pixel_size/2,   colours[15][15][15]);
        pannel_map_draw_pixel(116/pixel_size/2,   116/pixel_size/2+1, colours[15][15][15]);
        pannel_map_draw_pixel(116/pixel_size/2,   116/pixel_size/2-1, colours[15][15][15]);
    }
}

void pannel_map_update(long x, long y, long w, long h)
{
    SYNCDBG(7,"Starting");
    _DK_pannel_map_update(x, y, w, h);
}

void do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5)
{
    _DK_do_map_rotate_stuff(a1, a2, a3, a4, a5);
}

short do_left_map_drag(long begin_x, long begin_y, long curr_x, long curr_y, long zoom)
{
  SYNCDBG(17,"Starting");
  struct PlayerInfo *player;
  long x,y;
  if (!clicked_on_small_map)
  {
    grabbed_small_map = 0;
    return 0;
  }
  x = (curr_x - (MyScreenWidth >> 1)) / 2;
  y = (curr_y - (MyScreenHeight >> 1)) / 2;
  if ((abs(curr_x - old_mx) < 2) && (abs(curr_y - old_my) < 2))
    return 0;
  if (!grabbed_small_map)
  {
    grabbed_small_map = 1;
    x = 0;
    y = 0;
  }
  do_map_rotate_stuff(x, y, &curr_x, &curr_y, zoom);
  player = get_my_player();
  game.hand_over_subtile_x = curr_x;
  game.hand_over_subtile_y = curr_y;
  if (subtile_has_slab(curr_x, curr_y))
  {
    set_players_packet_action(player, PckA_BookmarkLoad, curr_x, curr_y, 0, 0);
  }
  return 1;
}

short do_left_map_click(long begin_x, long begin_y, long curr_x, long curr_y, long zoom)
{
  SYNCDBG(17,"Starting");
  struct PlayerInfo *player;
  short result;
  result = 0;
  player = get_my_player();
  if ((left_button_released) && (clicked_on_small_map))
  {
      if (grabbed_small_map)
      {
        game.small_map_state = 2;
        LbMouseSetPosition((begin_x+58)/pixel_size, (begin_y+58)/pixel_size);
      } else
      {
        do_map_rotate_stuff(curr_x-begin_x-58, curr_y-begin_y-58, &curr_x, &curr_y, zoom);
        game.hand_over_subtile_x = curr_x;
        game.hand_over_subtile_y = curr_y;
        if (subtile_has_slab(curr_x, curr_y))
        {
          result = 1;
          set_players_packet_action(player, PckA_BookmarkLoad, curr_x, curr_y, 0, 0);
        }
      }
    grabbed_small_map = 0;
    clicked_on_small_map = 0;
    left_button_released = 0;
  }
  return result;
}

short do_right_map_click(long start_x, long start_y, long curr_mx, long curr_my, long zoom)
{
    long x,y;
    SYNCDBG(17,"Starting");
    struct PlayerInfo *player;
    struct Thing *thing;
    do_map_rotate_stuff(curr_mx-start_x-58, curr_my-start_y-58, &x, &y, zoom);
    game.hand_over_subtile_x = x;
    game.hand_over_subtile_y = y;
    player = get_my_player();
    thing = get_first_thing_in_power_hand(player);
    if (!thing_is_invalid(thing))
    {
        if (can_place_thing_here(thing, x, y, player->id_number))
          game.small_map_state = 1;
    }
    if (right_button_clicked)
      right_button_clicked = 0;
    if (right_button_released)
    {
        right_button_released = 0;
        if (subtile_has_slab(x, y))
        {
          set_players_packet_action(player, PckA_DumpHeldThings, x, y, 0, 0);
          return 1;
        }
    }
    return 0;
}

void pannel_map_draw(long x, long y, long zoom)
{
  _DK_pannel_map_draw(x, y, zoom);
}
/******************************************************************************/
