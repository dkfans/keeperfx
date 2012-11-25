/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_panel.c
 *     Left GUI panel drawing and support functions.
 * @par Purpose:
 *     On-screen drawing of the main GUI panel.
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
#include "gui_panel.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_sprfnt.h"
#include "bflib_guibtns.h"

#include "player_data.h"
#include "game_legacy.h"
#include "creature_battle.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_pannel_map_update(long x, long y, long w, long h);
DLLIMPORT void _DK_gui_set_button_flashing(long a1, long a2);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void pannel_map_update(long x, long y, long w, long h)
{
    SYNCDBG(7,"Starting");
    _DK_pannel_map_update(x, y, w, h);
}

void gui_set_button_flashing(long btn_idx, long gameturns)
{
    game.flash_button_index = btn_idx;
    game.flash_button_gameturns = gameturns;
}

short zoom_to_fight(unsigned char a1)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_my_player();
    if (active_battle_exists(a1))
    {
        dungeon = get_players_num_dungeon(my_player_number);
        set_players_packet_action(player, 104, dungeon->field_1174, 0, 0, 0);
        step_battles_forward(a1);
        return true;
    }
    return false;
}

/******************************************************************************/
