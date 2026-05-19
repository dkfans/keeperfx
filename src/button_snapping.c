/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file button_snapping.c
 *     routines related to button snapping with a controller.
 * @par Comment:
 *     None.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_guibtns.h"
#include "bflib_basics.h"
#include "bflib_planar.h"
#include "bflib_mouse.h"
#include "kjm_input.h"
#include <math.h>
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "front_landview.h"
#include "frontmenu_select.h"
#include "front_input.h"
#include "creature_battle.h"
#include "map_data.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Forward declarations
static void snap_cursor_to_button(long *snap_to_x, long *snap_to_y);
extern void gui_get_creature_in_battle(struct GuiButton *gbtn);
extern void gui_setup_friend_over(struct GuiButton *gbtn);
/******************************************************************************/

static TbBool try_scroll_select_list_edge(long mouse_x, long mouse_y, float dx, float dy)
{
    if (fabsf(dy) < fabsf(dx)) {
        return false;
    }

    const TbBool is_down = (dy > 0.0f);
    const TbBool is_up = (dy < 0.0f);

    for (int i = 0; i < ACTIVE_BUTTONS_COUNT; i++) {
        struct GuiButton* gbtn = &active_buttons[i];

        if (!(gbtn->flags & LbBtnF_Active)) continue;
        if (!(gbtn->flags & LbBtnF_Visible)) continue;
        if (!(gbtn->flags & LbBtnF_Enabled)) continue;
        if (gbtn->click_event == NULL) continue;

        if (mouse_x < gbtn->scr_pos_x || mouse_x >= (gbtn->scr_pos_x + gbtn->width) ||
            mouse_y < gbtn->scr_pos_y || mouse_y >= (gbtn->scr_pos_y + gbtn->height)) {
            continue;
        }

        if (is_down && gbtn->content.lval != 51) {
            return false;
        }
        if (is_up && gbtn->content.lval != 45) {
            return false;
        }

        if (gbtn->click_event == frontend_level_select) {
            if (is_down) {
                frontend_level_select_down(NULL);
            } else {
                frontend_level_select_up(NULL);
            }
            return true;
        }

        if (gbtn->click_event == frontend_campaign_select) {
            if (is_down) {
                frontend_campaign_select_down(NULL);
            } else {
                frontend_campaign_select_up(NULL);
            }
            return true;
        }

        if (gbtn->click_event == frontend_mappack_select) {
            if (is_down) {
                frontend_mappack_select_down(NULL);
            } else {
                frontend_mappack_select_up(NULL);
            }
            return true;
        }

        return false;
    }

    return false;
}

static float get_button_score(long mouse_x, long mouse_y, long btn_center_x, long btn_center_y, float dx, float dy, float MIN_DOT)
{
    // Vector from mouse to button
    float to_btn_x = (float)(btn_center_x - mouse_x);
    float to_btn_y = (float)(btn_center_y - mouse_y);
    float dist = sqrtf(to_btn_x * to_btn_x + to_btn_y * to_btn_y);
    
    if (dist < 5.0f) return -1.0f; // Skip if already on button
    
    // Normalize
    to_btn_x /= dist;
    to_btn_y /= dist;
    
    // Check alignment with desired direction
    float dot = dx * to_btn_x + dy * to_btn_y;
    if (dot < MIN_DOT) return -1.0f;
    
    // Score based on alignment and distance (prefer close + aligned)
    float score = dot / (1.0f + dist / 200.0f);
    return score;
}

static float get_battle_buttons_top_score(const struct GuiButton* gbtn, long *btn_center_x, long *btn_center_y, long mouse_x, long mouse_y, float dx, float dy, float MIN_DOT)
{
    int visbtl_id = gbtn->btype_value & LbBFeF_IntValueMask;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    BattleIndex battle_id = dungeon->visible_battles[visbtl_id];
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle)) {
        return -1.0f;
    }
    if (battle->fighters_num <= 0) {
        return -1.0f;
    }

    long btn_y = gbtn->scr_pos_y + gbtn->height / 2;
    float best_score = -1.0f;

    TbBool friendly = (gbtn->ptover_event == gui_setup_friend_over);

    for (int i = 0; i < MESSAGE_BATTLERS_COUNT; i++) {
        struct Thing* thing;
        if (friendly)
            thing = thing_get(friendly_battler_list[(MESSAGE_BATTLERS_COUNT * visbtl_id) + i]);
        else
            thing = thing_get(enemy_battler_list[(MESSAGE_BATTLERS_COUNT * visbtl_id) + i]);
        
        if (thing_exists(thing) && thing_revealed(thing, my_player_number))
        {
            const int slot_w = gbtn->width / 7;
            int btn_x;
            if (friendly)
                btn_x = gbtn->scr_pos_x + (6 - i) * slot_w + slot_w / 2;
            else
                btn_x = gbtn->scr_pos_x + i * slot_w + slot_w / 2;
            
            float score = get_button_score(mouse_x, mouse_y, btn_x, btn_y, dx, dy, MIN_DOT);
            if (score > best_score) {
                *btn_center_x = btn_x;
                *btn_center_y = btn_y;
                
                best_score = score;
            }
        }
    }

    return best_score;
}

static TbBool find_nearest_button_in_direction(long mouse_x, long mouse_y, float dx, float dy, long *snap_to_x, long *snap_to_y)
{
    float best_score = -1.0f;
    const float MIN_DOT = 0.3f; // Minimum alignment required
    *snap_to_x = 0;
    *snap_to_y = 0;

    TbBool btn_found = false;
    
    // Normalize direction
    float mag = sqrtf(dx * dx + dy * dy);
    if (mag < 0.01f) return false;
    dx /= mag;
    dy /= mag;
    
    for (int i = 0; i < ACTIVE_BUTTONS_COUNT; i++) {
        struct GuiButton* gbtn = &active_buttons[i];
        
        // Skip inactive, invisible, or disabled buttons
        if (!(gbtn->flags & LbBtnF_Active)) continue; 
        if (!(gbtn->flags & LbBtnF_Visible)) continue;
        if (!(gbtn->flags & LbBtnF_Enabled)) continue;
        if (gbtn->click_event == NULL) continue;

        
        // Calculate button center
        long btn_center_x = gbtn->pos_x + gbtn->width / 2;
        long btn_center_y = gbtn->pos_y + gbtn->height / 2;

        float score;

        if (gbtn->click_event == gui_get_creature_in_battle)
        {
            score = get_battle_buttons_top_score(gbtn,&btn_center_x,&btn_center_y, mouse_x, mouse_y, dx, dy, MIN_DOT);
        }
        else
        {
            score = get_button_score(mouse_x, mouse_y, btn_center_x, btn_center_y, dx, dy, MIN_DOT);
        }
        
        if (score > best_score) {
            *snap_to_x = btn_center_x;
            *snap_to_y = btn_center_y;
            
            best_score = score;
            btn_found = true;
        }
    }

    return btn_found;
}

static TbBool find_nearest_landview_flag_in_direction(long mouse_x, long mouse_y, float dx, float dy, long *snap_to_x, long *snap_to_y)
{
    const float MIN_DOT = 0.3f;
    float best_score = -1.0f;
    *snap_to_x = 0;
    *snap_to_y = 0;

    float mag = sqrtf(dx * dx + dy * dy);
    if (mag < 0.01f) return false;
    dx /= mag;
    dy /= mag;

    long screen_max_x = map_info.screen_shift_x + lbDisplay.PhysicalScreenWidth * 16 / units_per_pixel_landview;
    long screen_max_y = map_info.screen_shift_y + lbDisplay.PhysicalScreenHeight * 16 / units_per_pixel_landview;

    TbBool flag_found = false;
    struct LevelInformation* lvinfo = get_first_level_info();
    while (lvinfo != NULL && lvinfo->lvnum != 0)
    {
        if ((frontend_menu_state == FeSt_LAND_VIEW && lvinfo->level_type & LvKind_IsMulti)
         || (frontend_menu_state == FeSt_NETLAND_VIEW && (lvinfo->level_type & (LvKind_IsSingle|LvKind_IsExtra|LvKind_IsBonus))))
        {
            lvinfo = get_next_level_info(lvinfo);
            continue;
        }
            
        if (lvinfo->state == LvSt_Visible)
        {
            if ((lvinfo->ensign_zoom_x >= map_info.screen_shift_x) && (lvinfo->ensign_zoom_x < screen_max_x)
             && (lvinfo->ensign_zoom_y >= map_info.screen_shift_y) && (lvinfo->ensign_zoom_y < screen_max_y))
            {
                long btn_center_x = scale_value_landview(lvinfo->ensign_x - (long)map_info.screen_shift_x);
                long btn_center_y = scale_value_landview(lvinfo->ensign_y - (long)map_info.screen_shift_y);
                const struct TbSprite *spr = get_ensign_sprite_for_level(lvinfo, 0);
                if (spr != NULL)
                {
                    btn_center_y = scale_value_landview(lvinfo->ensign_y - (long)map_info.screen_shift_y - (long)((spr->SHeight * 2) / 3));
                }
                float score = get_button_score(mouse_x, mouse_y, btn_center_x, btn_center_y, dx, dy, MIN_DOT);
                if (score > best_score)
                {
                    *snap_to_x = btn_center_x;
                    *snap_to_y = btn_center_y;
                    best_score = score;
                    flag_found = true;
                }
            }
        }
        lvinfo = get_next_level_info(lvinfo);
    }

    return flag_found;

}

static void snap_cursor_to_button(long *snap_to_x, long *snap_to_y)
{
    if (snap_to_x == NULL || snap_to_y == NULL) return;
    
    long btn_center_x = *snap_to_x;
    long btn_center_y = *snap_to_y;
    
    struct TbPoint delta;
    delta.x = btn_center_x - GetMouseX();
    delta.y = btn_center_y - GetMouseY();
    
    mouseControl(MActn_MOUSEMOVE, &delta);
}

void snap_to_direction(long mouse_x, long mouse_y, float dx, float dy)
{
    if (try_scroll_select_list_edge(mouse_x, mouse_y, dx, dy)) {
        return;
    }

    long snap_to_x, snap_to_y;
    TbBool found;
    if ((frontend_menu_state == FeSt_LAND_VIEW) || (frontend_menu_state == FeSt_NETLAND_VIEW)) {
        found = find_nearest_landview_flag_in_direction(mouse_x, mouse_y, dx, dy, &snap_to_x, &snap_to_y);
    }
    else {
        found = find_nearest_button_in_direction(mouse_x, mouse_y, dx, dy, &snap_to_x, &snap_to_y);
    }

    if (found)
        snap_cursor_to_button(&snap_to_x, &snap_to_y);
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
