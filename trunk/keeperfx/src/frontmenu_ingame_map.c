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
#include "bflib_guibtns.h"
#include "bflib_mouse.h"

#include "front_input.h"
#include "player_data.h"
#include "game_legacy.h"
#include "creature_battle.h"
#include "power_hand.h"
#include "kjm_input.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_pannel_map_update(long x, long y, long w, long h);
DLLIMPORT void _DK_gui_set_button_flashing(long a1, long a2);
DLLIMPORT void _DK_do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5);
DLLIMPORT void _DK_pannel_map_draw(long x, long y, long zoom);
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
