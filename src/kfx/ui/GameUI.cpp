/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file GameUI.cpp
 *     Owner/orchestrator of the composed in-game UI.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/ui/GameUI.h"

#include "frontmenu_ingame_tabs.h" /* draw_whole_status_panel */
#include "frontend.h"              /* draw_gui */
#include "engine_redraw.h"         /* draw_overlay_compass */
#include "gui_msgs.h"              /* message_draw */
#include "gui_tooltips.h"          /* draw_tooltip */
#include "power_hand.h"            /* draw_power_hand */
#include "gui_boxmenu.h"           /* gui_draw_all_boxes */
#include "player_data.h"           /* struct PlayerInfo, PVT_MapScreen/MapFadeIn/MapFadeOut */
#include "game_legacy.h"           /* game, GOF_ShowGui */
#include "bflib_basics.h"          /* flag_is_set, TbBool */

#include "post_inc.h"

/******************************************************************************/

GameUI& GameUI::Get()
{
    static GameUI s_instance;
    return s_instance;
}

bool GameUI::IsActiveForCurrentView(const struct PlayerInfo* player) const
{
    if (!player)
        return false;
    if (player->view_type == PVT_MapScreen
        || player->view_type == PVT_MapFadeIn
        || player->view_type == PVT_MapFadeOut)
        return false;
    return true;
}

void GameUI::DrawFrame(struct PlayerInfo* player)
{
    if (!IsActiveForCurrentView(player))
        return;

    TbBool show_sidebar = flag_is_set(game.operation_flags, GOF_ShowGui);

    if (show_sidebar)
    {
        draw_whole_status_panel();
    }
    draw_gui();
    if (show_sidebar)
    {
        draw_overlay_compass(local_state.minimap_pos_x, local_state.minimap_pos_y);
    }
    
    message_draw();
    draw_tooltip();
    draw_power_hand();
    gui_draw_all_boxes();
}

/******************************************************************************/
// C shim

extern "C" {

void GameUI_DrawFrame(struct PlayerInfo* player)
{
    GameUI::Get().DrawFrame(player);
}

} // extern "C"
