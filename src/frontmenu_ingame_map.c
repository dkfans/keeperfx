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
#include "thing_doors.h"
#include "vidmode.h"
#include "vidfade.h"
#include "player_instances.h"
#include "engine_render.h"
#include "gui_draw.h"
#include "local_camera.h"
#include "post_inc.h"

// Local constants
#define ANGLE_MASK_4 8188    // Angle mask rounded to multiples of 4 (0x1FFC)
#define MAP_ARROW_DISTANCE -1536    // Distance/scale factor for map arrow drawing

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
enum PanelColourIds
{
    PnC_Wall          = 1,
    PnC_Rock          = 2,
    PnC_Unexplored    = 3,
    PnC_Tagged_Gold   = 4,
    PnC_Gold          = 5,
    PnC_Lava          = 6,
    PnC_Water         = 7,
    PnC_purplePath    = 8,
    PnC_RockFloor     = 9,
    PnC_Tagged_Gems   = 10,
    PnC_Gems          = 11,
    //12-255 left free for future use
    PnC_RoomsStart    = 256,  //rooms 256-2559  (9*256 entries) TERRAIN_ITEMS_MAX
    PnC_DoorsStart    = 2560, //doors 2560-38559 (9*2000*2 entries) TRAPDOOR_TYPES_MAX
    PnC_DoorsStartLocked  = 2569,
    PnC_PathStart     = 38560,  // path (9 entries)
    PnC_End           = 38569,
};

enum TbPixelsColours
{
    Tbp_OpenDoor   = 60,
    Tbp_LockedDoor = 79,
    Tbp_DoorHighlighted = 31

};
/**************************/
/**
 * Background behind the map area.
 */
static unsigned char *MapBackground = NULL;
static int32_t *MapShapeStart = NULL;
static int32_t *MapShapeEnd = NULL;

static long PanelMapY;
static long PanelMapX;
static long NumBackColours;
static long PrevPixelSize;
static unsigned char MapBackColours[256];
static unsigned char PanelColours[16*PnC_End];
static long PrevRoomHighlight;
static long PrevDoorHighlight;
static unsigned short PanelMap[MAX_SUBTILES_X*MAX_SUBTILES_Y];
static struct InterpMinimap interp_minimap;

long clicked_on_small_map;
unsigned char grabbed_small_map;
long MapDiagonalLength = 0;
TbBool reset_all_minimap_interpolation = false;



/******************************************************************************/

void panel_map_draw_pixel(RealScreenCoord x, RealScreenCoord y, TbPixel col)
{
    if ((y >= 0) && (y < MapDiagonalLength))
    {
        if ((x >= MapShapeStart[y]) && (x < MapShapeEnd[y]))
        {
            lbDisplay.WScreen[(PanelMapY + y) * lbDisplay.GraphicsScreenWidth + (PanelMapX + x)] = col;
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
    const struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_CALL2ARMS);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(owner);
    int units_per_px;
    units_per_px = (16*status_panel_width + 140/2) / 140;
    TbPixel col;
    col = player_room_colours[get_player_color_idx(owner)];
    int i;
    i = 2*(PANEL_MAP_RADIUS*units_per_px/16) / 2;
    long center_x;
    long center_y;
    center_x = i + x2;
    center_y = i + y2;
    long long cscale;
    float circle_time;
    if ((game.operation_flags & GOF_Paused) == 0) {
        circle_time = ((game.play_gameturn + owner) & 7) + game.process_turn_time;
    } else {
        circle_time = ((game.play_gameturn + owner) & 7);
    }
    cscale = circle_time * powerst->strength[dungeon->cta_power_level];
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
          panel_map_draw_pixel(x1 + dxq1, y1 + dyq1, col);
          dxq2 = center_x + sx;
          panel_map_draw_pixel(x1 + dxq2, y1 + dyq1, col);
          dyq2 = sy + center_y;
          panel_map_draw_pixel(x1 + dxq1, y1 + dyq2, col);
          panel_map_draw_pixel(x1 + dxq2, y1 + dyq2, col);
          dxq3 = center_x - sy;
          dyq3 = center_y - sx;
          panel_map_draw_pixel(x1 + dxq3, y1 + dyq3, col);
          dxq4 = center_x + sy;
          panel_map_draw_pixel(x1 + dxq4, y1 + dyq3, col);
          dyq4 = sx + center_y;
          panel_map_draw_pixel(x1 + dxq3, y1 + dyq4, col);
          panel_map_draw_pixel(x1 + dxq4, y1 + dyq4, col);
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
        panel_map_draw_pixel(x1 + dxq1, y1 + dyq1, col);
        dxq2 = center_x + sx;
        panel_map_draw_pixel(x1 + dxq2, y1 + dyq1, col);
        dyq2 = sy + center_y;
        panel_map_draw_pixel(x1 + dxq1, y1 + dyq2, col);
        panel_map_draw_pixel(x1 + dxq2, y1 + dyq2, col);
        dxq3 = center_x - sy;
        dyq3 = center_y - sx;
        panel_map_draw_pixel(x1 + dxq3, y1 + dyq3, col);
        dxq4 = center_x + sy;
        panel_map_draw_pixel(x1 + dxq4, y1 + dyq3, col);
        dyq4 = sx + center_y;
        panel_map_draw_pixel(x1 + dxq3, y1 + dyq4, col);
        panel_map_draw_pixel(dxq4 + x1, dyq4 + y1, col);
      }
    }
}

void interpolate_minimap_thing(struct Thing *thing, struct Camera *cam)
{
    long current_minimap_x = (thing->mappos.x.val - (MapCoordDelta)cam->mappos.x.val);
    long current_minimap_y = (thing->mappos.y.val - (MapCoordDelta)cam->mappos.y.val);
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
    struct Camera *cam = get_local_camera(player->acamera);
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
                mapos_x = (zmpos_x * LbCosL(cam->rotation_angle_x) + zmpos_y * LbSinL(cam->rotation_angle_x)) >> 16;
                mapos_y = (zmpos_y * LbCosL(cam->rotation_angle_x) - zmpos_x * LbSinL(cam->rotation_angle_x)) >> 16;
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
    struct Camera *cam = get_local_camera(player->acamera);
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
            mapos_x = (zmpos_x * LbCosL(cam->rotation_angle_x) + zmpos_y * LbSinL(cam->rotation_angle_x)) >> 16;
            mapos_y = (zmpos_y * LbCosL(cam->rotation_angle_x) - zmpos_x * LbSinL(cam->rotation_angle_x)) >> 16;
            RealScreenCoord basepos;
            basepos = MapDiagonalLength/2;
            // Do the drawing
            if ((thing->trap.revealed) || (player->id_number == thing->owner))
            {
                TbPixel col;
                if ((thing->model == gui_trap_type_highlighted) && ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate)) {
                    col = player_highlight_colours[thing->owner];
                } else {
                    col = 60;
                }
                short pixels_amount = scale_pixel(basic_zoom*2);
                short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom*2);
                for (int p = 0; p < pixel_end; p++)
                {
                    // Draw a cross
                    panel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, col);
                    panel_map_draw_pixel(mapos_x + basepos + pixels_amount + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, col);
                    panel_map_draw_pixel(mapos_x + basepos - pixels_amount + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, col);
                    panel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos + pixels_amount + draw_square[p].delta_y, col);
                    panel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos - pixels_amount + draw_square[p].delta_y, col);
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
    struct Camera *cam = get_local_camera(player->acamera);
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
                mapos_x = (zmpos_x * LbCosL(cam->rotation_angle_x) + zmpos_y * LbSinL(cam->rotation_angle_x)) >> 16;
                mapos_y = (zmpos_y * LbCosL(cam->rotation_angle_x) - zmpos_x * LbSinL(cam->rotation_angle_x)) >> 16;
                RealScreenCoord basepos;
                basepos = MapDiagonalLength/2;

                // Do the drawing
                if (((game.play_gameturn % (4 * gui_blink_rate)) / gui_blink_rate) == 1) {
                    if (thing_is_special_box(thing) || thing_is_spellbook(thing))
                    {
                        short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom);
                        int p;
                        for (p = 0; p < pixel_end; p++)
                        {
                            panel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, colours[15][0][15]);
                        }
                        n++;
                    }
                    else if (thing_is_workshop_crate(thing))
                    {
                        short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom);
                        int p;
                        for (p = 0; p < pixel_end; p++)
                        {
                            panel_map_draw_pixel(mapos_x + basepos + draw_square[p].delta_x, mapos_y + basepos + draw_square[p].delta_y, colours[7][6][7]);
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

void panel_map_draw_creature_dot(long mapos_x, long mapos_y, RealScreenCoord basepos, TbPixel col, long basic_zoom, TbBool isLowRes)
{
    if (isLowRes)
    {
        // At low resolutions, we only need the single pixel
        panel_map_draw_pixel(mapos_x + basepos, mapos_y + basepos, col);
        return;
    }
    short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom);
    for (int i = 0; i < pixel_end; i++)
    {
        panel_map_draw_pixel(mapos_x + basepos + draw_square[i].delta_x, mapos_y + basepos + draw_square[i].delta_y, col);
    }
}

int draw_overlay_possessed_thing(struct PlayerInfo* player, long mapos_x, long mapos_y, RealScreenCoord basepos, TbPixel col, long basic_zoom, TbBool isLowRes)
{
    const struct Camera* cam;
    cam = get_local_camera(player->acamera);
    if (cam == NULL)
        return 0;
    if (cam->view_mode != PVM_CreatureView)
        return 0;
    if ((game.play_gameturn % (8 * gui_blink_rate)) >= 4 * gui_blink_rate)
    {
        col = colours[15][15][15];
    }
    if (isLowRes)
    {
        // At low resolutions, we only need the single pixel
        panel_map_draw_pixel(mapos_x + basepos, mapos_y + basepos, col);
        return 1;
    }
    short pixel_end = get_pixels_scaled_and_zoomed(basic_zoom * 2);
    short pixels_amount = scale_pixel(basic_zoom * 2);
    for (int i = 0; i < pixel_end; i++)
    {
        panel_map_draw_pixel(mapos_x + basepos + draw_square[i].delta_x, mapos_y + basepos + draw_square[i].delta_y, col);
        panel_map_draw_pixel(mapos_x + basepos + pixels_amount + draw_square[i].delta_x, mapos_y + basepos + draw_square[i].delta_y, col);
        panel_map_draw_pixel(mapos_x + basepos - pixels_amount + draw_square[i].delta_x, mapos_y + basepos + draw_square[i].delta_y, col);
        panel_map_draw_pixel(mapos_x + basepos + draw_square[i].delta_x, mapos_y + basepos + pixels_amount + draw_square[i].delta_y, col);
        panel_map_draw_pixel(mapos_x + basepos + draw_square[i].delta_x, mapos_y + basepos - pixels_amount + draw_square[i].delta_y, col);
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
    struct Camera *cam = get_local_camera(player->acamera);
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
                if ((game.play_gameturn % (8 * gui_blink_rate)) < 4 * gui_blink_rate)
                {
                    col1 = player_room_colours[get_player_color_idx(thing->owner)];
                    col2 = player_room_colours[get_player_color_idx(thing->owner)];
                }
                // Position of the thing on unrotated map
                // for camera, coordinates within subtile are skipped; the thing uses full resolution coordinates
                long zmpos_x = thing->interp_minimap_pos_x / zoom;
                long zmpos_y = thing->interp_minimap_pos_y / zoom;

                // Now rotate the coordinates to receive minimap points
                long mapos_x;
                long mapos_y;
                mapos_x = (zmpos_x * LbCosL(cam->rotation_angle_x) + zmpos_y * LbSinL(cam->rotation_angle_x)) >> 16;
                mapos_y = (zmpos_y * LbCosL(cam->rotation_angle_x) - zmpos_x * LbSinL(cam->rotation_angle_x)) >> 16;
                RealScreenCoord basepos;
                basepos = MapDiagonalLength/2;
                // Do the drawing
                if (thing->owner == player->id_number)
                {
                    if ((thing->model == gui_creature_type_highlighted) && ((game.play_gameturn % (4 * gui_blink_rate)) >= 2 * gui_blink_rate))
                    {
                        short pixels_amount = scale_pixel(basic_zoom * 4);
                        panel_map_draw_creature_dot(mapos_x + pixels_amount, mapos_y, basepos, col2, basic_zoom, isLowRes);
                        panel_map_draw_creature_dot(mapos_x - pixels_amount, mapos_y, basepos, col2, basic_zoom, isLowRes);
                        panel_map_draw_creature_dot(mapos_x, mapos_y + pixels_amount, basepos, col2, basic_zoom, isLowRes);
                        panel_map_draw_creature_dot(mapos_x, mapos_y - pixels_amount, basepos, col2, basic_zoom, isLowRes);
                        panel_map_draw_creature_dot(mapos_x, mapos_y, basepos, 31, basic_zoom, isLowRes);
                    } else
                    {
                        if ((is_thing_directly_controlled_by_player(thing, my_player_number)) || (is_thing_passenger_controlled_by_player(thing, my_player_number)))
                        {
                            draw_overlay_possessed_thing(player, mapos_x, mapos_y, basepos, col2, basic_zoom, isLowRes);
                        }
                        else
                        {
                            panel_map_draw_creature_dot(mapos_x, mapos_y, basepos, col2, basic_zoom, isLowRes);
                        }
                    }
                } else
                {
                    if (thing->owner == game.neutral_player_num) {
                        col = player_room_colours[get_player_color_idx(((game.play_gameturn + 1) % (4 * neutral_flash_rate)) / neutral_flash_rate)];
                    } else {
                        col = col1;
                    }
                    panel_map_draw_creature_dot(mapos_x, mapos_y, basepos, col, basic_zoom, isLowRes);
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
                    if ((game.play_gameturn % (8 * gui_blink_rate)) < 4 * gui_blink_rate)
                    {
                        col1 = player_room_colours[get_player_color_idx((int)(cctrl->party.target_plyr_idx >= 0 ? cctrl->party.target_plyr_idx : 0))];
                        col2 = player_room_colours[get_player_color_idx(thing->owner)];
                    }
                    long zmpos_x = ((stl_num_decode_x(memberpos) - (MapSubtlDelta)cam->mappos.x.stl.num) << 8);
                    long zmpos_y = ((stl_num_decode_y(memberpos) - (MapSubtlDelta)cam->mappos.y.stl.num) << 8);
                    zmpos_x += (interp_minimap.previous_x-interp_minimap.x) >> 8;
                    zmpos_y += (interp_minimap.previous_y-interp_minimap.y) >> 8;
                    zmpos_x /= zoom;
                    zmpos_y /= zoom;

                    long mapos_x;
                    long mapos_y;
                    mapos_x = (zmpos_x * LbCosL(cam->rotation_angle_x) + zmpos_y * LbSinL(cam->rotation_angle_x)) >> 16;
                    mapos_y = (zmpos_y * LbCosL(cam->rotation_angle_x) - zmpos_x * LbSinL(cam->rotation_angle_x)) >> 16;
                    RealScreenCoord basepos;
                    basepos = MapDiagonalLength/2;
                    // Do the drawing
                    if (thing->owner == player->id_number) {
                        col = col2;
                    } else {
                        col = col1;
                    }
                    panel_map_draw_creature_dot(mapos_x, mapos_y, basepos, col, basic_zoom, isLowRes);
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
    struct Camera *cam = get_local_camera(player->acamera);
    struct Thing *thing = get_player_soul_container(player->id_number);

    if (!thing_exists(thing)) {
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
    mapos_x = (zmpos_x * LbCosL(cam->rotation_angle_x) + zmpos_y * LbSinL(cam->rotation_angle_x)) >> 16;
    mapos_y = (zmpos_y * LbCosL(cam->rotation_angle_x) - zmpos_x * LbSinL(cam->rotation_angle_x)) >> 16;
    RealScreenCoord basepos;
    basepos = MapDiagonalLength/2;
    // Do the drawing
    long dist;
    long angle;
    dist = get_distance_xy(basepos, basepos, mapos_x + basepos, mapos_y + basepos);
    angle = -(LbArcTanAngle(mapos_x, mapos_y) & ANGLE_MASK) & ANGLE_MASK_4;
    int delta_x;
    int delta_y;
    delta_x = scale_ui_value(MAP_ARROW_DISTANCE) * LbSinL(angle) >> 16;
    delta_y = scale_ui_value(MAP_ARROW_DISTANCE) * LbCosL(angle) >> 16;
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
            panel_map_draw_pixel((draw_x >> 8) + draw_square[p].delta_x, (draw_y >> 8) + draw_square[p].delta_y, col);
        }
    }
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    return 1;
}

void panel_map_draw_overlay_things(long units_per_px, long scaled_zoom, long basic_zoom)
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

void panel_map_update_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    SubtlCodedCoords stl_num = get_subtile_number(stl_x, stl_y);
    struct Map *mapblk = get_map_block_at_pos(stl_num);
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    int col = 0;
    int owner_col = slabmap_owner(slb);
    if ((mapblk->flags & SlbAtFlg_Unexplored) != 0)
    {
        col = PnC_Unexplored;
    }
    else if (map_block_revealed(mapblk, plyr_idx))
    {
        if ((slb->kind == SlbT_GOLD) || (slb->kind == SlbT_DENSEGOLD))
        {
            col = PnC_Gold;
            if ((mapblk->flags & SlbAtFlg_TaggedValuable) != 0)
            {
                col--;
            }
        } else
        if (slb->kind == SlbT_GEMS)
        {
            col = PnC_Gems;
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
            col = owner_col + PLAYERS_COUNT * room->kind + PnC_RoomsStart;
        } else
        if (slb->kind == SlbT_ROCK)
        {
            col = PnC_Rock;
        } else
        if (slb->kind == SlbT_ROCK_FLOOR)
        {
            col = PnC_RockFloor;
        }
        else
        if ((mapblk->flags & SlbAtFlg_Filled) != 0)
        {
            col = PnC_Wall;
        } else
        if ((mapblk->flags & SlbAtFlg_IsDoor) != 0)
        {
            struct Thing *doortng;
            doortng = get_door_for_position(stl_x, stl_y);
            if (!thing_is_invalid(doortng)) {
                if(door_is_hidden_to_player(doortng,plyr_idx))
                {
                    col = PnC_Wall;
                }
                else
                {
                    col = owner_col + PLAYERS_COUNT * ((doortng->door.is_locked == 1) + 2 * doortng->model ) + PnC_DoorsStart;
                }
            }
        } else
        if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
        {
            if (slb->kind == SlbT_LAVA) {
                col = PnC_Lava;
            } else
            if (slb->kind == SlbT_WATER) {
                col = PnC_Water;
            }  else
            if (slb->kind == SlbT_PURPLE)
            {
                col = PnC_purplePath;
            } else {
                col = owner_col + PnC_PathStart;
            }
        }

    }
    ushort *mapptr = &PanelMap[stl_num];
    *mapptr = col;
}

void panel_map_update(long x, long y, long w, long h)
{
    SYNCDBG(17,"Starting for rect (%ld,%ld) at (%ld,%ld)",w,h,x,y);
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    for (stl_y = y; stl_y < y + h; stl_y++)
    {
        if (stl_y > game.map_subtiles_y)
            break;
        for (stl_x = x; stl_x < x + w; stl_x++)
        {
            if (stl_x > game.map_subtiles_x)
                break;
            if (subtile_has_slab(stl_x, stl_y))
            {
                panel_map_update_subtile(my_player_number, stl_x, stl_y); //player->id number is still unitialized when this function is called at level start
            }
        }
    }
}

static void do_map_rotate_stuff(long relpos_x, long relpos_y, int32_t *stl_x, int32_t *stl_y, long zoom)
{
    const struct PlayerInfo *player = get_my_player();
    const struct Camera *cam;
    cam = get_local_camera(player->acamera);
    int angle;
    angle = cam->rotation_angle_x & ANGLE_MASK_4;
    int shift_x;
    int shift_y;
    shift_x = -LbSinL(angle);
    shift_y = LbCosL(angle);
    *stl_x = (shift_y * relpos_x + shift_x * relpos_y) >> 16;
    *stl_y = (shift_y * relpos_y - shift_x * relpos_x) >> 16;
    *stl_x = zoom * (*stl_x) / 256 + cam->mappos.x.stl.num;
    *stl_y = zoom * (*stl_y) / 256 + cam->mappos.y.stl.num;
}

short do_left_map_drag(long begin_x, long begin_y, int32_t curr_x, int32_t curr_y, long zoom)
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

short do_left_map_click(long begin_x, long begin_y, int32_t curr_x, int32_t curr_y, long zoom)
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
    int32_t x;
    int32_t y;
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
    if (MapDiagonalLength != 2*(PANEL_MAP_RADIUS*units_per_px/16))
    {
        MapDiagonalLength = 2*(PANEL_MAP_RADIUS*units_per_px/16);
        free(MapBackground);
        MapBackground = calloc(MapDiagonalLength*MapDiagonalLength, sizeof(TbPixel));
        free(MapShapeStart);
        MapShapeStart = (int32_t *)calloc(MapDiagonalLength, sizeof(int32_t));
        free(MapShapeEnd);
        MapShapeEnd = (int32_t *)calloc(MapDiagonalLength, sizeof(int32_t));
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
    out = &lbDisplay.WScreen[PanelMapX + out_scanline * PanelMapY];
    int w;
    int h;
    for (h=0; h < MapDiagonalLength; h++)
    {
        for (w = MapShapeStart[h]; w < MapShapeEnd[h]; w++)
        {
            if (w < 0) continue;

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
    NumBackColours = num_colours;
}

void setup_panel_colors(void)
{
    int frame;
    frame = (game.play_gameturn % (4 * gui_blink_rate)) / gui_blink_rate;
    unsigned int frcol;
    frcol = player_room_colours[(game.play_gameturn % (4 * neutral_flash_rate)) / neutral_flash_rate];
    int bkcol_idx;
    int pncol_idx;
    pncol_idx = 0;
    for (bkcol_idx=0; bkcol_idx < NumBackColours; bkcol_idx++)
    {
        unsigned int bkcol;
        bkcol = MapBackColours[bkcol_idx];
        int n;
        n = pncol_idx;
        if (frame != 0)
        {
            PanelColours[n + PnC_Unexplored] = pixmap.ghost[bkcol + 26*256];
            PanelColours[n + PnC_Tagged_Gold] = pixmap.ghost[bkcol + 140*256];
            PanelColours[n + PnC_Gems] = 102 + (pixmap.ghost[bkcol] >> 6);
        } else //as this is during setup at gameturn 1, the else looks like it is never used.
        {
            PanelColours[n + PnC_Unexplored] = bkcol;
            PanelColours[n + PnC_Tagged_Gold] = bkcol;
            PanelColours[n + PnC_Tagged_Gems] = 104 + (pixmap.ghost[bkcol] >> 6);
        }
        PanelColours[n + 0] = bkcol;
        PanelColours[n + PnC_Wall]    = pixmap.ghost[bkcol + 16*256];
        PanelColours[n + PnC_Rock]      = 0;
        PanelColours[n + PnC_Gold]      = pixmap.ghost[bkcol + 140*256];
        PanelColours[n + PnC_Lava]      = 146;
        PanelColours[n + PnC_Water]     = 85;
        PanelColours[n + PnC_purplePath]    = 255;
        PanelColours[n + PnC_Gems]      = 102 + (pixmap.ghost[bkcol] >> 6);
        PanelColours[n + PnC_RockFloor] = 145;

        n = pncol_idx + PnC_RoomsStart;
        int i;
        int k;
        for (i=TERRAIN_ITEMS_MAX; i > 0; i--)
        {
            PanelColours[n + 0] = player_room_colours[get_player_color_idx(PLAYER0)];
            PanelColours[n + 1] = player_room_colours[get_player_color_idx(PLAYER1)];
            PanelColours[n + 2] = player_room_colours[get_player_color_idx(PLAYER2)];
            PanelColours[n + 3] = player_room_colours[get_player_color_idx(PLAYER3)];
            PanelColours[n + 4] = player_room_colours[get_player_color_idx(PLAYER_GOOD)];
            PanelColours[n + 5] = frcol;
            PanelColours[n + 6] = player_room_colours[get_player_color_idx(PLAYER4)];
            PanelColours[n + 7] = player_room_colours[get_player_color_idx(PLAYER5)];
            PanelColours[n + 8] = player_room_colours[get_player_color_idx(PLAYER6)];
            n += PLAYERS_COUNT;
        }

        n = pncol_idx + PnC_PathStart;
        {
            PanelColours[n + 0] = player_path_colours[get_player_color_idx(PLAYER0)];
            PanelColours[n + 1] = player_path_colours[get_player_color_idx(PLAYER1)];
            PanelColours[n + 2] = player_path_colours[get_player_color_idx(PLAYER2)];
            PanelColours[n + 3] = player_path_colours[get_player_color_idx(PLAYER3)];
            PanelColours[n + 4] = player_path_colours[get_player_color_idx(PLAYER_GOOD)];
            PanelColours[n + 5] = player_path_colours[PLAYER_NEUTRAL];
            PanelColours[n + 6] = player_path_colours[get_player_color_idx(PLAYER4)];
            PanelColours[n + 7] = player_path_colours[get_player_color_idx(PLAYER5)];
            PanelColours[n + 8] = player_path_colours[get_player_color_idx(PLAYER6)];
        }
        n = pncol_idx + PnC_DoorsStart;
        for (i=TRAPDOOR_TYPES_MAX; i > 0; i--)
        {
            for (k=0; k < PLAYERS_COUNT; k++)
            {
              PanelColours[n + k] = Tbp_OpenDoor;
            }
            n += PLAYERS_COUNT;
            for (k=0; k < PLAYERS_COUNT; k++)
            {
              PanelColours[n + k] = Tbp_LockedDoor;
            }
            n += PLAYERS_COUNT;
        }
        pncol_idx += PnC_End;
    }
}

void update_panel_color_player_color(PlayerNumber plyr_idx, unsigned char color_idx)
{
    int n = 0;
    int pncol_idx = 0;
    for (int bkcol_idx=0; bkcol_idx < NumBackColours; bkcol_idx++)
    {
        n = pncol_idx + PnC_RoomsStart;

        for (int i=TERRAIN_ITEMS_MAX; i > 0; i--)
        {
            PanelColours[n + plyr_idx] = player_room_colours[color_idx];
            n += PLAYERS_COUNT;
        }
        n = pncol_idx + PnC_PathStart;
        {
            PanelColours[n + plyr_idx] = player_path_colours[color_idx];
        }

        pncol_idx += PnC_End;
    }
}

void update_panel_colors(void)
{
    int frame;
    frame = (game.play_gameturn % (4 * gui_blink_rate)) / gui_blink_rate;
    unsigned int frcol;
    frcol = player_room_colours[(game.play_gameturn % (4 * neutral_flash_rate)) / neutral_flash_rate];
    int bkcol_idx;
    int pncol_idx;
    pncol_idx = 0;
    for (bkcol_idx=0; bkcol_idx < NumBackColours; bkcol_idx++)
    {
        unsigned int bkcol;
        bkcol = MapBackColours[bkcol_idx];
        int n;
        n = pncol_idx;
        if (frame != 0)
        {
            PanelColours[n + PnC_Unexplored] = pixmap.ghost[bkcol + 26*256];
            PanelColours[n + PnC_Tagged_Gold] = pixmap.ghost[bkcol + 140*256];
            PanelColours[n + PnC_Tagged_Gems] = 102 + (pixmap.ghost[bkcol] >> 6);
        } else
        {
            PanelColours[n + PnC_Unexplored] = bkcol;
            PanelColours[n + PnC_Tagged_Gold] = bkcol;
            PanelColours[n + PnC_Tagged_Gems] = 100 + (pixmap.ghost[bkcol] >> 6);
        }
        n = pncol_idx + PnC_RoomsStart;
        int i;
        for (i=TERRAIN_ITEMS_MAX; i > 0; i--)
        {
            PanelColours[n + PLAYER_NEUTRAL] = frcol;
            n += PLAYERS_COUNT;
        }
        pncol_idx += PnC_End;
    }

    int highlight;
    highlight = gui_room_type_highlighted;
    frame = game.play_gameturn % (2 * gui_blink_rate);
    if (frame >= gui_blink_rate)
        highlight = -1;
    if (PrevRoomHighlight != highlight)
    {
        if ((PrevRoomHighlight >= 0) && (NumBackColours > 0))
        {
            int i;
            int n;
            n = PLAYERS_COUNT * PrevRoomHighlight + PnC_RoomsStart;
            for (i=NumBackColours; i > 0; i--)
            {
                PanelColours[n + 0] = player_room_colours[get_player_color_idx(0)];
                PanelColours[n + 1] = player_room_colours[get_player_color_idx(1)];
                PanelColours[n + 2] = player_room_colours[get_player_color_idx(2)];
                PanelColours[n + 3] = player_room_colours[get_player_color_idx(3)];
                PanelColours[n + 4] = player_room_colours[get_player_color_idx(4)];
                PanelColours[n + 5] = frcol;
                PanelColours[n + 6] = player_room_colours[get_player_color_idx(6)];
                PanelColours[n + 7] = player_room_colours[get_player_color_idx(7)];
                PanelColours[n + 8] = player_room_colours[get_player_color_idx(8)];
                n += PnC_End;
            }
        }

        if ((highlight >= 0) && (NumBackColours > 0))
        {
            int i;
            int n;
            n = PLAYERS_COUNT * highlight + PnC_RoomsStart;
            for (i=NumBackColours; i > 0; i--)
            {
                PanelColours[n + 0] = 31;
                PanelColours[n + 1] = 31;
                PanelColours[n + 2] = 31;
                PanelColours[n + 3] = 31;
                PanelColours[n + 4] = 31;
                PanelColours[n + 5] = 31;
                PanelColours[n + 6] = 31;
                PanelColours[n + 7] = 31;
                PanelColours[n + 8] = 31;
                n += PnC_End;
            }
        }

        PrevRoomHighlight = highlight;
    }

    highlight = gui_door_type_highlighted;
    if (frame >= gui_blink_rate)
        highlight = -1;
    if (highlight != PrevDoorHighlight)
    {
        if ((PrevDoorHighlight >= 0) && (PrevDoorHighlight != TRAPDOOR_TYPES_MAX) && (NumBackColours > 0))
        {
            int i;
            int n;
            n = 2 * PLAYERS_COUNT * PrevDoorHighlight;
            for (i=NumBackColours; i > 0; i--)
            {
                int k;
                for (k=0; k < PLAYERS_COUNT; k+=2)
                {
                  PanelColours[n + PnC_DoorsStart       + k] = Tbp_OpenDoor;
                  PanelColours[n + PnC_DoorsStartLocked + k] = Tbp_LockedDoor;
                }
                n += PnC_End;
            }
        }
        if ((highlight >= 0) && (NumBackColours > 0))
        {
            int i;
            int n;
            n = 2 * PLAYERS_COUNT * highlight;
            for (i = NumBackColours; i > 0; i--)
            {
                int k;
                for (k=0; k < PLAYERS_COUNT; k+=2)
                {
                  PanelColours[n + PnC_DoorsStart       + k] = Tbp_DoorHighlighted;
                  PanelColours[n + PnC_DoorsStartLocked + k] = Tbp_DoorHighlighted;
                }
                n += PnC_End;
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
        setup_panel_colors();
    }
}

void panel_map_draw_slabs(long x, long y, long units_per_px, long zoom)
{
    PanelMapX = scale_value_for_resolution_with_upp(x,units_per_px);
    PanelMapY = scale_value_for_resolution_with_upp(y,units_per_px);
    auto_gen_tables(units_per_px);
    update_panel_colors();
    struct PlayerInfo *player = get_my_player();
    struct Camera *cam = get_local_camera(player->acamera);

    if ((cam == NULL) || (MapDiagonalLength < 1))
        return;
    if (game.play_gameturn <= 1) {reset_all_minimap_interpolation = true;} //Fixes initial minimap frame being purple

    long shift_x;
    long shift_y;
    long shift_stl_x;
    long shift_stl_y;
    {
        int angle;
        angle = cam->rotation_angle_x & ANGLE_MASK_4; //cam->rotation_angle_x
        shift_x = -LbSinL(angle) * zoom / 256;
        shift_y = LbCosL(angle) * zoom / 256;
        long current_minimap_x = (cam->mappos.x.val << 8);
        long current_minimap_y = (cam->mappos.y.val << 8);
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
    out_line = &lbDisplay.WScreen[PanelMapX + lbDisplay.GraphicsScreenWidth * PanelMapY];
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
            if ((subpos_y >= 0) && (subpos_x >= 0) && (subpos_y < (1<<16)*game.map_subtiles_x) && (subpos_x < (1<<16)*game.map_subtiles_y)) {
                break;
            }
            subpos_y -= shift_y;
            subpos_x += shift_x;
        }
        subpos_y = shift_stl_x + shift_y * start_w;
        subpos_x = shift_stl_y - shift_x * start_w;
        for (; start_w < end_w; start_w++)
        {
            if ((subpos_y >= 0) && (subpos_x >= 0) && (subpos_y < (1<<16)*game.map_subtiles_x) && (subpos_x < (1<<16)*game.map_subtiles_y)) {
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
            pnmap_idx = ((precor_x>>16)) + (((precor_y>>16)) * (game.map_subtiles_x + 1) );
            int pncol_idx;
            //TODO reenable background
            pncol_idx = PanelMap[pnmap_idx] + (*bkgnd * PnC_End);
            *out = PanelColours[pncol_idx];
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
