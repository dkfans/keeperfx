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
#include "pre_inc.h"
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

#include "frontend.h"
#include "front_input.h"
#include "player_data.h"
#include "game_legacy.h"
#include "creature_states.h"
#include "creature_states_hero.h"
#include "creature_battle.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "power_hand.h"
#include "kjm_input.h"
#include "frontmenu_ingame_tabs.h"
#include "vidmode.h"
#include "vidfade.h"
#include "player_instances.h"
#include "engine_render.h"
#include "post_inc.h"

/******************************************************************************/
struct InterpMinimap
{
    long x;
    long y;
    long previous_x;
    long previous_y;
    long get_previous;
};
/******************************************************************************/
/**
 * Background behind the map area.
 */
static unsigned char *MapBackground = NULL;
static long *MapShapeStart = NULL;
static long *MapShapeEnd = NULL;
static const TbPixel RoomColours[] = {132, 92, 164, 183, 21, 132};
static long PannelMapY;
static long PannelMapX;
static long NoBackColours;
static long PrevPixelSize;
static unsigned char MapBackColours[256];
static unsigned char PannelColours[4096];
static long PrevRoomHighlight;
static long PrevDoorHighlight;
static unsigned char PannelMap[MAX_SUBTILES_X*MAX_SUBTILES_Y];//map subtiles x*y
static struct InterpMinimap interp_minimap;

long clicked_on_small_map;
unsigned char grabbed_small_map;
long MapDiagonalLength = 0;
TbBool reset_all_minimap_interpolation = false;
/******************************************************************************/

void pannel_map_draw_pixel(RealScreenCoord x, RealScreenCoord y, TbPixel col)
{
    if ((y >= 0) && (y < MapDiagonalLength))
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
    const struct MagicStats *pwrdynst;
    pwrdynst = get_power_dynamic_stats(PwrK_CALL2ARMS);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(owner);
    int units_per_px;
    units_per_px = (16*status_panel_width + 140/2) / 140;
    TbPixel col;
    col = player_room_colours[owner];
    int i;
    i = 2*(PANNEL_MAP_RADIUS*units_per_px/16) / 2;
    long center_x;
    long center_y;
    center_x = i + x2;
    center_y = i + y2;
    long long cscale;
    float circle_time;
    if ((game.operation_flags & GOF_Paused) == 0) {
        circle_time = ((game.play_gameturn + owner) & 7) + gameadd.process_turn_time;
    } else {
        circle_time = ((game.play_gameturn + owner) & 7);
    }
    cscale = circle_time * pwrdynst->strength[dungeon->cta_splevel];
    int dxq1;
    int dyq1;
    int dxq2;
    int dyq2;
    int dxq3;
    int dyq3;
    int dxq4;
    int dyq4;

    int sx;
    int sy;
    long base_y;
    base_y = ((cscale >> 3) << 8) / zoom;
    if ( base_y > 1 )
    {
      sy = base_y;
      i = 3 - 2 * base_y;
      for (sx=0; sx < sy; sx++)
      {
          dxq1 = center_x - sx;
          dyq1 = center_y - sy;
          pannel_map_draw_pixel(x1 + dxq1, y1 + dyq1, col);
          dxq2 = center_x + sx;
          pannel_map_draw_pixel(x1 + dxq2, y1 + dyq1, col);
          dyq2 = sy + center_y;
          pannel_map_draw_pixel(x1 + dxq1, y1 + dyq2, col);
          pannel_map_draw_pixel(x1 + dxq2, y1 + dyq2, col);
          dxq3 = center_x - sy;
          dyq3 = center_y - sx;
          pannel_map_draw_pixel(x1 + dxq3, y1 + dyq3, col);
          dxq4 = center_x + sy;
          pannel_map_draw_pixel(x1 + dxq4, y1 + dyq3, col);
          dyq4 = sx + center_y;
          pannel_map_draw_pixel(x1 + dxq3, y1 + dyq4, col);
          pannel_map_draw_pixel(x1 + dxq4, y1 + dyq4, col);
          if (i >= 0)
          {
              i += 4 * (sx - sy) + 10*units_per_px/16;
              sy--;
          } else
          {
              i += 4 * (sx - 1) + 10*units_per_px/16;
          }
      }

      if (sy == sx)
      {
        dxq1 = center_x - sx;
        dyq1 = center_y - sy;
        pannel_map_draw_pixel(x1 + dxq1, y1 + dyq1, col);
        dxq2 = center_x + sx;
        pannel_map_draw_pixel(x1 + dxq2, y1 + dyq1, col);
        dyq2 = sy + center_y;
        pannel_map_draw_pixel(x1 + dxq1, y1 + dyq2, col);
        pannel_map_draw_pixel(x1 + dxq2, y1 + dyq2, col);
        dxq3 = center_x - sy;
        dyq3 = center_y - sx;
        pannel_map_draw_pixel(x1 + dxq3, y1 + dyq3, col);
        dxq4 = center_x + sy;
        pannel_map_draw_pixel(x1 + dxq4, y1 + dyq3, col);
        dyq4 = sx + center_y;
        pannel_map_draw_pixel(x1 + dxq3, y1 + dyq4, col);
        pannel_map_draw_pixel(dxq4 + x1, dyq4 + y1, col);
      }
    }
}

void interpolate_minimap_thing(struct Thing *thing, struct Camera *cam)
{
    long current_minimap_x = (thing->mappos.x.val - (MapCoordDelta)subtile_coord(cam->mappos.x.stl.num,0));
    long current_minimap_y = (thing->mappos.y.val - (MapCoordDelta)subtile_coord(cam->mappos.y.stl.num,0));
    if ((reset_all_minimap_interpolation == true) || (thing->previous_minimap_pos_x == 0 && thing->previous_minimap_pos_y == 0))
    {
        thing->interp_minimap_pos_x = current_minimap_x;
        thing->interp_minimap_pos_y = current_minimap_y;
        thing->previous_minimap_pos_x = current_minimap_x;
        thing->previous_minimap_pos_y = current_minimap_y;
    } else {
        thing->interp_minimap_pos_x = interpolate(thing->interp_minimap_pos_x, thing->previous_minimap_pos_x, current_minimap_x);
        thing->interp_minimap_pos_y = interpolate(thing->interp_minimap_pos_y, thing->previous_minimap_pos_y, current_minimap_y);
    }
    if ((thing->interp_minimap_update_turn != game.play_gameturn) || (game.operation_flags & GOF_Paused) != 0) {
        thing->interp_minimap_update_turn = game.play_gameturn;
        thing->previous_minimap_pos_x = current_minimap_x;
        thing->previous_minimap_pos_y = current_minimap_y;
    }
}

/**
 * Draws all call to arms objects on minimap.
 * @param player The player for whom drawing occurs.
 * @param zoom Zoom level of the minimap.
 * @return Amount of objects drawn.
 */
int draw_overlay_call_to_arms(struct PlayerInfo *player, long units_per_px, long zoom)
{
    unsigned long k;
    int i;
    int n;
    SYNCDBG(18,"Starting");
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam = player->acamera;
    n = 0;
    const struct StructureList *slist = get_list_for_thing_class(TCls_Object);
    k = 0;
    i = slist->index;
    while (i != 0)
    {
        struct Thing *thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (!thing_is_picked_up(thing))
        {
            if (thing->model == ObjMdl_CTAEnsign)//TODO CONFIG object model dependency, move to config
            {
                // Position of the thing on unrotated map
                // for camera, coordinates within subtile are skipped; the thing uses full resolution coordinates
                interpolate_minimap_thing(thing, cam);
                long zmpos_x = thing->interp_minimap_pos_x / zoom;
                long zmpos_y = thing->interp_minimap_pos_y / zoom;

                // Now rotate the coordinates to receive minimap points
                long mapos_x;
                long mapos_y;
                mapos_x = (zmpos_x * LbCosL(interpolated_cam_orient_a) + zmpos_y * LbSinL(interpolated_cam_orient_a)) >> 16;
                mapos_y = (zmpos_y * LbCosL(interpolated_cam_orient_a) - zmpos_x * LbSinL(interpolated_cam_orient_a)) >> 16;
                draw_call_to_arms_circle(thing->owner, 0, 0, mapos_x, mapos_y, zoom);
                n++;
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

/**
 * Draws all owned traps on minimap.
 * @param player The player for whom drawing occurs.
 * @param zoom Scale between map coordinates and minimap pixels.
 * @return Amount of traps drawn.
 */
int draw_overlay_traps(struct PlayerInfo *player, long units_per_px, long scaled_zoom, long basic_zoom)
{
    unsigned long k;
    int i;
    int n;
    SYNCDBG(18,"Starting");
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam = player->acamera;
    n = 0;
    k = 0;
    const struct StructureList *slist = get_list_for_thing_class(TCls_Trap);
    i = slist->index;
    while (i != 0)
    {
        struct Thing *thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (player->id_number == thing->owner)
        {
            // Position of the thing on unrotated map
            // for camera, coordinates within subtile are skipped; the thing uses full resolution coordinates
            interpolate_minimap_thing(thing, cam);
            long zmpos_x = thing->interp_minimap_pos_x / scaled_zoom;
            long zmpos_y = thing->interp_minimap_pos_y / scaled_zoom;
            
            // Now rotate the coordinates to receive minimap points
            RealScreenCoord mapos_x;
            RealScreenCoord mapos_y;
            mapos_x = (zmpos_x * LbCosL(interpolated_cam_orient_a) + zmpos_y * LbSinL(interpolated_cam_orient_a)) >> 16;
            mapos_y = (zmpos_y * LbCosL(interpolated_cam_orient_a) - zmpos_x * LbSinL(interpolated_cam_orient_a)) >> 16;
            RealScreenCoord basepos;
            basepos = MapDiagonalLength/2;
            // Do the drawing
            if ((thing->trap.revealed) || (player->id_number == thing->owner))
            {
                TbPixel col;
                if ((thing->model == gui_trap_type_highlighted) && (game.play_gameturn & 1)) {
                    col = player_highlight_colours[thing->owner];
                } else {
                    col = 60;
                }
                short pixels_amount = scale_pixel(basic_zoom*2);
                short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom*2);
                for (int p = 0; p < pixel_end; p++)
                {
                    // Draw a cross
                    pannel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, col);
                    pannel_map_draw_pixel(mapos_x + basepos + pixels_amount + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, col);
                    pannel_map_draw_pixel(mapos_x + basepos - pixels_amount + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, col);
                    pannel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos + pixels_amount + draw_square[p].delta_y, col);
                    pannel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos - pixels_amount + draw_square[p].delta_y, col);
                }
                n++;
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

/**
 * Draws all spells and specials on minimap.
 * @param player The player for whom drawing occurs.
 * @param zoom Zoom level of the minimap.
 * @return Amount of objects drawn.
 */
int draw_overlay_spells_and_boxes(struct PlayerInfo *player, long units_per_px, long scaled_zoom, long basic_zoom)
{
    unsigned long k;
    int i;
    int n;
    SYNCDBG(18,"Starting");
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam = player->acamera;
    n = 0;
    const struct StructureList *slist = get_list_for_thing_class(TCls_Object);
    k = 0;
    i = slist->index;
    while (i != 0)
    {
        struct Thing *thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (!thing_is_picked_up(thing))
        {
            if (thing_revealed(thing, player->id_number))
            {
                // Position of the thing on unrotated map
                // for camera, coordinates within subtile are skipped; the thing uses full resolution coordinates
                interpolate_minimap_thing(thing, cam);
                long zmpos_x = thing->interp_minimap_pos_x / scaled_zoom;
                long zmpos_y = thing->interp_minimap_pos_y / scaled_zoom;
                
                long mapos_x;
                long mapos_y;
                // Now rotate the coordinates to receive minimap points
                mapos_x = (zmpos_x * LbCosL(interpolated_cam_orient_a) + zmpos_y * LbSinL(interpolated_cam_orient_a)) >> 16;
                mapos_y = (zmpos_y * LbCosL(interpolated_cam_orient_a) - zmpos_x * LbSinL(interpolated_cam_orient_a)) >> 16;
                RealScreenCoord basepos;
                basepos = MapDiagonalLength/2;
                
                // Do the drawing
                if ((game.play_gameturn & 3) == 1) {
                    if (thing_is_special_box(thing) || thing_is_spellbook(thing))
                    {
                        short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom);
                        int p;
                        for (p = 0; p < pixel_end; p++)
                        {
                            pannel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, colours[15][0][15]);
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

void pannel_map_draw_creature_dot(long mapos_x, long mapos_y, RealScreenCoord basepos, TbPixel col, long basic_zoom, TbBool isLowRes)
{
    if (isLowRes)
    {
        // At low resolutions, we only need the single pixel
        pannel_map_draw_pixel(mapos_x + basepos, mapos_y + basepos, col);
        return;
    }
    short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom);     
    for (int i = 0; i < pixel_end; i++)
    {
        pannel_map_draw_pixel(mapos_x + basepos + draw_square[i].delta_x, mapos_y + basepos + draw_square[i].delta_y, col);
    }
}

int draw_overlay_possessed_thing(struct PlayerInfo* player, long mapos_x, long mapos_y, RealScreenCoord basepos, TbPixel col, long basic_zoom, TbBool isLowRes)
{
    const struct Camera* cam;
    cam = player->acamera;
    if (cam == NULL)
        return 0;
    if (cam->view_mode != PVM_CreatureView)
        return 0;
    if (game.play_gameturn & 4)
    {
        col = colours[15][15][15];
    }
    if (isLowRes)
    {
        // At low resolutions, we only need the single pixel
        pannel_map_draw_pixel(mapos_x + basepos, mapos_y + basepos, col);
        return 1;
    }
    short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom * 2);
    short pixels_amount = scale_pixel(basic_zoom * 2);
    for (int i = 0; i < pixel_end; i++)
    {
        pannel_map_draw_pixel(mapos_x + basepos + draw_square[i].delta_x, mapos_y + basepos + draw_square[i].delta_y, col);
        pannel_map_draw_pixel(mapos_x + basepos + pixels_amount + draw_square[i].delta_x, mapos_y + basepos + draw_square[i].delta_y, col);
        pannel_map_draw_pixel(mapos_x + basepos - pixels_amount + draw_square[i].delta_x, mapos_y + basepos + draw_square[i].delta_y, col);
        pannel_map_draw_pixel(mapos_x + basepos + draw_square[i].delta_x, mapos_y + basepos + pixels_amount + draw_square[i].delta_y, col);
        pannel_map_draw_pixel(mapos_x + basepos + draw_square[i].delta_x, mapos_y + basepos - pixels_amount + draw_square[i].delta_y, col);
    }
    return 1;
}

int draw_overlay_creatures(struct PlayerInfo *player, long units_per_px, long zoom, long basic_zoom)
{
    TbBool isLowRes = 0;
    if (units_per_px < 16)
    {
       isLowRes = 1;
    }

    unsigned long k;
    int i;
    int n;
    SYNCDBG(18,"Starting");
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam = player->acamera;
    n = 0;
    k = 0;
    const struct StructureList *slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
    while (i != 0)
    {
        struct Thing *thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        TbPixel col1;
        TbPixel col2;
        TbPixel col;
        col1 = 31;
        col2 = 1;
        if (!thing_is_picked_up(thing))
        {
            interpolate_minimap_thing(thing, cam);
            if (thing_revealed(thing, player->id_number))
            {
                if ((game.play_gameturn & 4) == 0)
                {
                    col1 = player_room_colours[thing->owner];
                    col2 = player_room_colours[thing->owner];
                }
                // Position of the thing on unrotated map
                // for camera, coordinates within subtile are skipped; the thing uses full resolution coordinates
                long zmpos_x = thing->interp_minimap_pos_x / zoom;
                long zmpos_y = thing->interp_minimap_pos_y / zoom;

                // Now rotate the coordinates to receive minimap points
                long mapos_x;
                long mapos_y;
                mapos_x = (zmpos_x * LbCosL(interpolated_cam_orient_a) + zmpos_y * LbSinL(interpolated_cam_orient_a)) >> 16;
                mapos_y = (zmpos_y * LbCosL(interpolated_cam_orient_a) - zmpos_x * LbSinL(interpolated_cam_orient_a)) >> 16;
                RealScreenCoord basepos;
                basepos = MapDiagonalLength/2;
                // Do the drawing
                if (thing->owner == player->id_number)
                {
                    if ((thing->model == gui_creature_type_highlighted) && (game.play_gameturn & 2))
                    {
                        short pixels_amount = scale_pixel(basic_zoom * 4);
                        pannel_map_draw_creature_dot(mapos_x + pixels_amount, mapos_y, basepos, col2, basic_zoom, isLowRes);
                        pannel_map_draw_creature_dot(mapos_x - pixels_amount, mapos_y, basepos, col2, basic_zoom, isLowRes);
                        pannel_map_draw_creature_dot(mapos_x, mapos_y + pixels_amount, basepos, col2, basic_zoom, isLowRes);
                        pannel_map_draw_creature_dot(mapos_x, mapos_y - pixels_amount, basepos, col2, basic_zoom, isLowRes);
                        pannel_map_draw_creature_dot(mapos_x, mapos_y, basepos, 31, basic_zoom, isLowRes);
                    } else
                    {
                        if ((is_thing_directly_controlled_by_player(thing, my_player_number)) || (is_thing_passenger_controlled_by_player(thing, my_player_number)))
                        {
                            draw_overlay_possessed_thing(player, mapos_x, mapos_y, basepos, col2, basic_zoom, isLowRes);
                        }
                        else
                        {
                            pannel_map_draw_creature_dot(mapos_x, mapos_y, basepos, col2, basic_zoom, isLowRes);
                        }
                    }
                } else
                {
                    if (thing->owner == game.neutral_player_num) {
                        col = player_room_colours[(game.play_gameturn + 1) & 3];
                    } else {
                        col = col1;
                    }
                    pannel_map_draw_creature_dot(mapos_x, mapos_y, basepos, col, basic_zoom, isLowRes);
                }
            } else
            // Hero tunnelers may be visible on unrevealed terrain too (if on revealed, then they're already drawn)
            if (is_hero_tunnelling_to_attack(thing))
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
                        col1 = player_room_colours[(uchar)cctrl->party.target_plyr_idx];
                        col2 = player_room_colours[thing->owner];
                    }
                    long zmpos_x = ((stl_num_decode_x(memberpos) - (MapSubtlDelta)cam->mappos.x.stl.num) << 8);
                    long zmpos_y = ((stl_num_decode_y(memberpos) - (MapSubtlDelta)cam->mappos.y.stl.num) << 8);
                    zmpos_x += (interp_minimap.previous_x-interp_minimap.x) >> 8;
                    zmpos_y += (interp_minimap.previous_y-interp_minimap.y) >> 8;
                    zmpos_x /= zoom;
                    zmpos_y /= zoom;

                    long mapos_x;
                    long mapos_y;
                    mapos_x = (zmpos_x * LbCosL(interpolated_cam_orient_a) + zmpos_y * LbSinL(interpolated_cam_orient_a)) >> 16;
                    mapos_y = (zmpos_y * LbCosL(interpolated_cam_orient_a) - zmpos_x * LbSinL(interpolated_cam_orient_a)) >> 16;
                    RealScreenCoord basepos;
                    basepos = MapDiagonalLength/2;
                    // Do the drawing
                    if (thing->owner == player->id_number) {
                        col = col2;
                    } else {
                        col = col1;
                    }
                    pannel_map_draw_creature_dot(mapos_x, mapos_y, basepos, col, basic_zoom, isLowRes);
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
int draw_line_to_heart(struct PlayerInfo *player, long units_per_px, long zoom)
{
    if (player->acamera == NULL)
        return 0;
    struct Camera *cam = player->acamera;
    struct Thing *thing = get_player_soul_container(player->id_number);

    if (thing_is_invalid(thing)) {
        return 0;
    }
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    // Position of the thing on unrotated map
    // for camera, coordinates within subtile are skipped; the thing uses full resolution coordinates
    interpolate_minimap_thing(thing, cam);
    long zmpos_x = thing->interp_minimap_pos_x / zoom;
    long zmpos_y = thing->interp_minimap_pos_y / zoom;

    // Now rotate the coordinates to receive minimap points
    RealScreenCoord mapos_x;
    RealScreenCoord mapos_y;
    mapos_x = (zmpos_x * LbCosL(interpolated_cam_orient_a) + zmpos_y * LbSinL(interpolated_cam_orient_a)) >> 16;
    mapos_y = (zmpos_y * LbCosL(interpolated_cam_orient_a) - zmpos_x * LbSinL(interpolated_cam_orient_a)) >> 16;
    RealScreenCoord basepos;
    basepos = MapDiagonalLength/2;
    // Do the drawing
    long dist;
    long angle;
    dist = get_distance_xy(basepos, basepos, mapos_x + basepos, mapos_y + basepos);
    angle = -(LbArcTanAngle(mapos_x, mapos_y) & LbFPMath_AngleMask) & 0x1FFC;
    int delta_x;
    int delta_y;
    delta_x = scale_ui_value(-1536) * LbSinL(angle) >> 16;
    delta_y = scale_ui_value(-1536) * LbCosL(angle) >> 16;
    long frame;
    frame = (game.play_gameturn & 3) + 1;
    int draw_x;
    int draw_y;
    draw_x = -delta_x / 2 + (frame * delta_x) / 4 + (basepos << 8);
    draw_y = -delta_y / 2 + (frame * delta_y) / 4 + (basepos << 8);
    int i;
    for (i = dist - 4; i > 0; i -= 4)
    {
        if ((draw_x < 0) || (draw_x >> 8 >= MapDiagonalLength))
            break;
        if ((draw_y < 0) || (draw_y >> 8 >= MapDiagonalLength))
            break;
        draw_x += delta_x;
        draw_y += delta_y;
        short pixel_end = get_pixels_scaled_and_zoomed(zoom * 2);
        TbPixel col = 15;
        for (int p = 0; p < pixel_end; p++)
        {
            pannel_map_draw_pixel((draw_x >> 8) + draw_square[p].delta_x, (draw_y >> 8) + draw_square[p].delta_y, col);
        }
    }
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    return 1;
}

void pannel_map_draw_overlay_things(long units_per_px, long scaled_zoom, long basic_zoom)
{
    SYNCDBG(7,"Starting");
    if (scaled_zoom < 1) {
        return;
    }
    struct PlayerInfo *player = get_my_player();
    draw_overlay_call_to_arms(player, units_per_px, scaled_zoom);
    draw_overlay_traps(player, units_per_px, scaled_zoom,basic_zoom);
    draw_overlay_creatures(player, units_per_px, scaled_zoom, basic_zoom);
    draw_overlay_spells_and_boxes(player, units_per_px, scaled_zoom, basic_zoom);
    draw_line_to_heart(player, units_per_px, basic_zoom);
}

void pannel_map_update_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    SubtlCodedCoords stl_num = get_subtile_number(stl_x, stl_y);
    struct Map *mapblk = get_map_block_at_pos(stl_num);
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    int col = 0;
    int owner_col = slabmap_owner(slb);
    if (owner_col > 6) {
        owner_col -= 3;
    }
    if ((mapblk->flags & SlbAtFlg_Unexplored) != 0)
    {
        col = 3;
    }
    else if (map_block_revealed(mapblk, plyr_idx))
    {
        if (slb->kind == SlbT_GOLD)
        {
            col = 5;
            if ((mapblk->flags & SlbAtFlg_TaggedValuable) != 0)
            {
                col--;
            }
        } else
        if (slb->kind == SlbT_GEMS)
        {
            col = 178;
            if ((mapblk->flags & SlbAtFlg_TaggedValuable) != 0)
            {
                col--;
            }
        }
        else
        if ((mapblk->flags & SlbAtFlg_IsRoom) != 0)
        {
            struct Room *room;
            room = room_get(slb->room_index);
            col = owner_col + 6 * room->kind + 8;
        } else
        if (slb->kind == SlbT_ROCK)
        {
            col = 2;
        } else
        if (slb->kind == SlbT_ROCK_FLOOR)
        {
            col = 2;
        }
        else
        if ((mapblk->flags & SlbAtFlg_Filled) != 0)
        {
            col = 1;
        } else
        if ((mapblk->flags & SlbAtFlg_IsDoor) != 0)
        {
            struct Thing *doortng;
            doortng = get_door_for_position(stl_x, stl_y);
            if (!thing_is_invalid(doortng)) {
                col = owner_col + 6 * ((doortng->door.is_locked == 1) + 2 * doortng->model) + 110;
            }
        } else
        if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
        {
            if (slb->kind == SlbT_LAVA) {
                col = 6;
            } else
            if (slb->kind == SlbT_WATER) {
                col = 7;
            }  else
            if (slb->kind == SlbT_PURPLE)
            {
                col = 176;
            } else {
                col = owner_col + 170;
            }
        }

    }
    TbPixel *mapptr = &PannelMap[stl_num];
    *mapptr = col;
}

void pannel_map_update(long x, long y, long w, long h)
{
    SYNCDBG(17,"Starting for rect (%ld,%ld) at (%ld,%ld)",w,h,x,y);
    struct PlayerInfo *player;
    player = get_my_player();
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    for (stl_y = y; stl_y < y + h; stl_y++)
    {
        if (stl_y > gameadd.map_subtiles_y)
            break;
        for (stl_x = x; stl_x < x + w; stl_x++)
        {
            if (stl_x > gameadd.map_subtiles_x)
                break;
            if (subtile_has_slab(stl_x, stl_y))
            {
                pannel_map_update_subtile(player->id_number, stl_x, stl_y);
            }
        }
    }
}

static void do_map_rotate_stuff(long relpos_x, long relpos_y, long *stl_x, long *stl_y, long zoom)
{
    const struct PlayerInfo *player;
    player = get_my_player();
    const struct Camera *cam;
    cam = player->acamera;
    int angle;
    angle = interpolated_cam_orient_a & 0x1FFC;
    int shift_x;
    int shift_y;
    shift_x = -LbSinL(angle);
    shift_y = LbCosL(angle);
    *stl_x = (shift_y * relpos_x + shift_x * relpos_y) >> 16;
    *stl_y = (shift_y * relpos_y - shift_x * relpos_x) >> 16;
    *stl_x = zoom * (*stl_x) / 256 + cam->mappos.x.stl.num;
    *stl_y = zoom * (*stl_y) / 256 + cam->mappos.y.stl.num;
}

short do_left_map_drag(long begin_x, long begin_y, long curr_x, long curr_y, long zoom)
{
  SYNCDBG(17,"Starting");
  struct PlayerInfo *player;
  long x;
  long y;
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
        LbMouseSetPosition(begin_x + MapDiagonalLength/2, begin_y + MapDiagonalLength/2);
      } else
      {
        do_map_rotate_stuff(curr_x - begin_x - MapDiagonalLength/2, curr_y - begin_y - MapDiagonalLength/2, &curr_x, &curr_y, zoom);
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
    long x;
    long y;
    SYNCDBG(17,"Starting");
    struct PlayerInfo *player;
    struct Thing *thing;
    do_map_rotate_stuff(curr_mx - start_x - MapDiagonalLength/2, curr_my - start_y - MapDiagonalLength/2, &x, &y, zoom);
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
            set_players_packet_action(player, PckA_UsePwrHandDrop, x, y, 0, 0);
            return 1;
        }
    }
    return 0;
}

void setup_background(long units_per_px)
{
    if (MapDiagonalLength != 2*(PANNEL_MAP_RADIUS*units_per_px/16))
    {
        MapDiagonalLength = 2*(PANNEL_MAP_RADIUS*units_per_px/16);
        LbMemoryFree(MapBackground);
        MapBackground = LbMemoryAlloc(MapDiagonalLength*MapDiagonalLength*sizeof(TbPixel));
        LbMemoryFree(MapShapeStart);
        MapShapeStart = (long *)LbMemoryAlloc(MapDiagonalLength*sizeof(long));
        LbMemoryFree(MapShapeEnd);
        MapShapeEnd = (long *)LbMemoryAlloc(MapDiagonalLength*sizeof(long));
    }
    if ((MapBackground == NULL) || (MapShapeStart == NULL) || (MapShapeEnd == NULL)) {
        MapDiagonalLength = 0;
        return;
    }
    long radius;
    radius = MapDiagonalLength / 2;
    long quarter_area;
    quarter_area = radius * radius;
    int i;
    for (i=0; i < MapDiagonalLength; i++)
    {
        long n;
        n = (radius - i - 1) * (i - radius + 1) + quarter_area;
        MapShapeStart[i] = radius - LbSqrL(n);
        MapShapeEnd[i] = radius + LbSqrL(n);
    }

    int num_colours;
    num_colours = 0;
    long out_scanline;
    out_scanline = lbDisplay.GraphicsScreenWidth;
    long bkgnd_pos;
    bkgnd_pos = 0;
    TbPixel *out;
    out = &lbDisplay.WScreen[PannelMapX + out_scanline * PannelMapY];
    int w;
    int h;
    for (h=0; h < MapDiagonalLength; h++)
    {
        for (w = MapShapeStart[h]; w < MapShapeEnd[h]; w++)
        {
            TbPixel orig;
            orig = out[w];
            out[w] = 255;
            int colour;
            for (colour=0; colour < num_colours; colour++)
            {
                if (MapBackColours[colour] == orig) {
                    break;
                }
            }
            if (num_colours == colour)
            {
                MapBackColours[num_colours] = orig;
                num_colours++;
            }
            MapBackground[bkgnd_pos+w] = colour;
        }
        bkgnd_pos += MapDiagonalLength;
        out += out_scanline;
    }
    NoBackColours = num_colours;
}

void setup_pannel_colours(void)
{
    int frame;
    frame = game.play_gameturn & 3;
    unsigned int frcol;
    frcol = RoomColours[frame];
    int bkcol_idx;
    int pncol_idx;
    pncol_idx = 0;
    for (bkcol_idx=0; bkcol_idx < NoBackColours; bkcol_idx++)
    {
        unsigned int bkcol;
        bkcol = MapBackColours[bkcol_idx];
        int n;
        n = pncol_idx;
        if (frame != 0)
        {
            PannelColours[n + 3] = pixmap.ghost[bkcol + 26*256];
            PannelColours[n + 4] = pixmap.ghost[bkcol + 140*256];
            PannelColours[n + 177] = 102 + (pixmap.ghost[bkcol] >> 6);
        } else //as this is during setup at gameturn 1, the else looks like it is never used.
        {
            PannelColours[n + 3] = bkcol;
            PannelColours[n + 4] = bkcol;
            PannelColours[n + 177] = 104 + (pixmap.ghost[bkcol] >> 6);
        }
        PannelColours[n + 0] = bkcol;
        PannelColours[n + 1] = pixmap.ghost[bkcol + 16*256];
        PannelColours[n + 2] = 0;
        PannelColours[n + 5] = pixmap.ghost[bkcol + 140*256];
        PannelColours[n + 6] = 146;
        PannelColours[n + 7] = 85;
        PannelColours[n + 176] = 255;
        PannelColours[n + 178] = 102 + (pixmap.ghost[bkcol] >> 6);
        n = pncol_idx + 8;
        int i;
        int k;
        for (i=17; i > 0; i--)
        {
            PannelColours[n + 0] = 132;
            PannelColours[n + 1] = 92;
            PannelColours[n + 2] = 164;
            PannelColours[n + 3] = 183;
            PannelColours[n + 4] = 21;
            PannelColours[n + 5] = frcol;
            n += 6;
        }
        n = pncol_idx + 8 + 17*6 + 12*5;
        {
            PannelColours[n + 0] = 131;
            PannelColours[n + 1] = 90;
            PannelColours[n + 2] = 163;
            PannelColours[n + 3] = 181;
            PannelColours[n + 4] = 20;
            PannelColours[n + 5] = 4;
        }
        n = pncol_idx + 8 + 17*6;
        for (i=5; i > 0; i--)
        {
            for (k=0; k < 6; k++)
            {
              PannelColours[n + k] = 60;
            }
            n += 6;
            for (k=0; k < 6; k++)
            {
              PannelColours[n + k] = 79;
            }
            n += 6;
        }
        pncol_idx += 256;
    }
}

void update_pannel_colours(void)
{
    int frame;
    frame = game.play_gameturn & 3;
    unsigned int frcol;
    frcol = RoomColours[frame];
    int bkcol_idx;
    int pncol_idx;
    pncol_idx = 0;
    for (bkcol_idx=0; bkcol_idx < NoBackColours; bkcol_idx++)
    {
        unsigned int bkcol;
        bkcol = MapBackColours[bkcol_idx];
        int n;
        n = pncol_idx;
        if (frame != 0)
        {
            PannelColours[n + 3] = pixmap.ghost[bkcol + 26*256];
            PannelColours[n + 4] = pixmap.ghost[bkcol + 140*256];
            PannelColours[n + 177] = 102 + (pixmap.ghost[bkcol] >> 6);
        } else
        {
            PannelColours[n + 3] = bkcol;
            PannelColours[n + 4] = bkcol;
            PannelColours[n + 177] = 100 + (pixmap.ghost[bkcol] >> 6);
        }
        n = pncol_idx + 8;
        int i;
        for (i=17; i > 0; i--)
        {
            PannelColours[n + 5] = frcol;
            n += 6;
        }
        pncol_idx += 256;
    }

    int highlight;
    highlight = gui_room_type_highlighted;
    frame = game.play_gameturn & 1;
    if (frame != 0)
        highlight = -1;
    if (PrevRoomHighlight != highlight)
    {
        if ((PrevRoomHighlight >= 0) && (NoBackColours > 0))
        {
            int i;
            int n;
            n = 6 * PrevRoomHighlight + 8;
            for (i=NoBackColours; i > 0; i--)
            {
                PannelColours[n + 0] = RoomColours[0];
                PannelColours[n + 1] = RoomColours[1];
                PannelColours[n + 2] = RoomColours[2];
                PannelColours[n + 3] = RoomColours[3];
                PannelColours[n + 4] = RoomColours[4];
                PannelColours[n + 5] = RoomColours[5];
                n += 256;
            }
        }
        if ((highlight >= 0) && (NoBackColours > 0))
        {
            int i;
            int n;
            n = 6 * highlight + 8;
            for (i=NoBackColours; i > 0; i--)
            {
                PannelColours[n + 0] = 31;
                PannelColours[n + 1] = 31;
                PannelColours[n + 2] = 31;
                PannelColours[n + 3] = 31;
                PannelColours[n + 4] = 31;
                PannelColours[n + 5] = 31;
                n += 256;
            }
        }
        PrevRoomHighlight = highlight;
    }

    highlight = gui_door_type_highlighted;
    if (frame != 0)
        highlight = -1;
    if (highlight != PrevDoorHighlight)
    {
        if ((PrevDoorHighlight >= 0) && (PrevDoorHighlight != 5) && (NoBackColours > 0))
        {
            int i;
            int n;
            n = 12 * PrevDoorHighlight;
            for (i=NoBackColours; i > 0; i--)
            {
                int k;
                for (k=0; k < 6; k+=2)
                {
                  PannelColours[n + 110 + k] = 60;
                  PannelColours[n + 116 + k] = 79;
                }
                n += 256;
            }
        }
        if ((highlight >= 0) && (NoBackColours > 0))
        {
            int i;
            int n;
            n = 12 * highlight;
            for (i = NoBackColours; i > 0; i--)
            {
                int k;
                for (k=0; k < 6; k+=2)
                {
                  PannelColours[n + 110 + k] = 31;
                  PannelColours[n + 116 + k] = 31;
                }
                n += 256;
            }
        }
        PrevDoorHighlight = highlight;
    }
}

void auto_gen_tables(long units_per_px)
{
    if (PrevPixelSize != 256 * units_per_px / 16)
    {
        PrevPixelSize = 256 * units_per_px / 16;
        setup_background(units_per_px);
        setup_pannel_colours();
    }
}

void pannel_map_draw_slabs(long x, long y, long units_per_px, long zoom)
{
    PannelMapX = scale_value_for_resolution_with_upp(x,units_per_px);
    PannelMapY = scale_value_for_resolution_with_upp(y,units_per_px);
    auto_gen_tables(units_per_px);
    update_pannel_colours();
    struct PlayerInfo *player = get_my_player();
    struct Camera *cam = player->acamera;
    
    if ((cam == NULL) || (MapDiagonalLength < 1))
        return;
    if (game.play_gameturn <= 1) {reset_all_minimap_interpolation = true;} //Fixes initial minimap frame being purple
    
    long shift_x;
    long shift_y;
    long shift_stl_x;
    long shift_stl_y;
    {
        int angle;
        angle = interpolated_cam_orient_a & 0x1FFC; //cam->orient_a
        shift_x = -LbSinL(angle) * zoom / 256;
        shift_y = LbCosL(angle) * zoom / 256;
        long current_minimap_x = (cam->mappos.x.stl.num << 16);
        long current_minimap_y = (cam->mappos.y.stl.num << 16);
        if (reset_all_minimap_interpolation == true)
        {
            interp_minimap.x = current_minimap_x;
            interp_minimap.y = current_minimap_y;
            interp_minimap.previous_x = current_minimap_x;
            interp_minimap.previous_y = current_minimap_y;
        } else {
            interp_minimap.x = interpolate(interp_minimap.x, interp_minimap.previous_x, current_minimap_x);
            interp_minimap.y = interpolate(interp_minimap.y, interp_minimap.previous_y, current_minimap_y);
        }
        if ((interp_minimap.get_previous != game.play_gameturn) || (game.operation_flags & GOF_Paused) != 0) {
            interp_minimap.get_previous = game.play_gameturn;
            interp_minimap.previous_x = current_minimap_x;
            interp_minimap.previous_y = current_minimap_y;
        }
        shift_stl_x = interp_minimap.x - MapDiagonalLength * shift_x / 2 - MapDiagonalLength * shift_y / 2;
        shift_stl_y = interp_minimap.y - MapDiagonalLength * shift_y / 2 + MapDiagonalLength * shift_x / 2;
    }

    TbPixel *bkgnd_line;
    bkgnd_line = MapBackground;
    TbPixel *out_line;
    out_line = &lbDisplay.WScreen[PannelMapX + lbDisplay.GraphicsScreenWidth * PannelMapY];
    int h;
    for (h = 0; h < MapDiagonalLength; h++)
    {
        int start_w;
        int end_w;
        start_w = MapShapeStart[h];
        end_w = MapShapeEnd[h];
        int subpos_x;
        int subpos_y;
        subpos_y = shift_stl_x + shift_y * (end_w - 1);
        subpos_x = shift_stl_y - shift_x * (end_w - 1);
        for (; end_w > start_w; end_w--)
        {
            if ((subpos_y >= 0) && (subpos_x >= 0) && (subpos_y < (1<<16)*gameadd.map_subtiles_x) && (subpos_x < (1<<16)*gameadd.map_subtiles_y)) {
                break;
            }
            subpos_y -= shift_y;
            subpos_x += shift_x;
        }
        subpos_y = shift_stl_x + shift_y * start_w;
        subpos_x = shift_stl_y - shift_x * start_w;
        for (; start_w < end_w; start_w++)
        {
            if ((subpos_y >= 0) && (subpos_x >= 0) && (subpos_y < (1<<16)*gameadd.map_subtiles_x) && (subpos_x < (1<<16)*gameadd.map_subtiles_y)) {
                break;
            }
            subpos_y += shift_y;
            subpos_x -= shift_x;
        }
        TbPixel *bkgnd;
        bkgnd = &bkgnd_line[start_w];
        TbPixel *out;
        out = &out_line[start_w];
        unsigned int precor_y;
        unsigned int precor_x;
        precor_x = subpos_y;
        precor_y = subpos_x;
        int w;
        for (w = end_w-start_w; w > 0; w--)
        {
            int pnmap_idx;
            //formula will have to be redone if maps bigger then 256, but works for smallerAD
            pnmap_idx = ((precor_x>>16)) + (((precor_y>>16)) * (gameadd.map_subtiles_x + 1) );
            int pncol_idx;
            pncol_idx = PannelMap[pnmap_idx] | (*bkgnd << 8);
            *out = PannelColours[pncol_idx];
            precor_x += shift_y;
            precor_y -= shift_x;
            out++;
            bkgnd++;
        }
        out_line += lbDisplay.GraphicsScreenWidth;
        bkgnd_line += MapDiagonalLength;
        shift_stl_x += shift_x;
        shift_stl_y += shift_y;
    }
}
/******************************************************************************/
