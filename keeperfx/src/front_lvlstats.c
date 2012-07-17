/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_lvlstats.c
 *     High Score screen displaying routines.
 * @par Purpose:
 *     Functions to show and maintain the high scores screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     01 Jan 2012 - 23 Jun 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "front_lvlstats.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_sprite.h"
#include "bflib_guibtns.h"
#include "bflib_vidraw.h"
#include "bflib_sprfnt.h"
#include "kjm_input.h"
#include "config_campaigns.h"
#include "front_highscore.h"
#include "front_landview.h"
#include "frontend.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "game_merge.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT extern struct StatsData _DK_scrolling_stats_data[];
#define scrolling_stats_data _DK_scrolling_stats_data
DLLIMPORT extern struct LevelStats _DK_frontstats_data;
#define frontstats_data _DK_frontstats_data
DLLIMPORT extern TbClockMSec _DK_frontstats_timer;
#define frontstats_timer _DK_frontstats_timer
/******************************************************************************/
DLLIMPORT void _DK_frontstats_update(void);
DLLIMPORT void _DK_frontstats_set_timer(void);
DLLIMPORT void _DK_frontstats_initialise(void);
DLLIMPORT void _DK_frontstats_draw_main_stats(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontstats_draw_scrolling_stats(struct GuiButton *gbtn);
DLLIMPORT void _DK_frontstats_leave(struct GuiButton *gbtn);
/******************************************************************************/
/** Calculates average efficiency of player's rooms.
 * @param plyr_idx Player for whom statistic is to be calculated.
 * @return Statistic value.
 */
long calculate_efficiency(long plyr_idx)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long i,rkind;
    long count,efficiency;
    unsigned long k;
    count = 0;
    efficiency = 0;
    dungeon = get_dungeon(plyr_idx);
    for (rkind=1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        i = dungeon->room_kind[rkind];
        k = 0;
        while (i != 0)
        {
            room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            count++;
            efficiency += room->efficiency;
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }
    if (count < 1)
        return 0;
    return 100 * efficiency / (count << 8);
}

long calculate_style(long plyr_idx)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long i,rkind;
    long area,half_area;
    unsigned long k;
    area = 0;
    dungeon = get_dungeon(plyr_idx);
    for (rkind=1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        i = dungeon->room_kind[rkind];
        k = 0;
        while (i != 0)
        {
          room = room_get(i);
          if (room_is_invalid(room))
          {
            ERRORLOG("Jump to invalid room detected");
            break;
          }
          i = room->next_of_owner;
          // Per-room code
          area += room->slabs_count;
          // Per-room code ends
          k++;
          if (k > ROOMS_COUNT)
          {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
          }
        }
    }
    half_area = (dungeon->total_area >> 1);
    if ((area < half_area) && (half_area > 0))
        return 100 * area / half_area;
    else
        return 100;
}

/** Calculates rating, a level summary statistic parameter.
 *  Rating is based on other level parameters, like style, efficiency, score and amount of won/lost battles.
 * @param plyr_idx Player for whom statistic is to be calculated.
 * @return Statistic value.
 */
long calculate_rating(long plyr_idx)
{
    struct Dungeon *dungeon;
    long btlost,btwon,ratio;
    long rating;
    rating = calculate_style(plyr_idx) * calculate_efficiency(plyr_idx) / 100;
    dungeon = get_dungeon(plyr_idx);
    rating += 100 * dungeon->lvstats.player_score / 800;
    btlost = dungeon->lvstats.battles_lost;
    btwon = dungeon->lvstats.battles_won;
    // Find scoring ratio
    ratio = 100;
    if ( (btlost < btwon) && (btlost > 0) )
    {
        ratio = btwon / btlost;
        if (ratio < 10) {
          ratio = 10;
        } else
        if (ratio > 100) {
            ratio = 100;
        }
    }
    rating += rating * ratio / 100;
    return 75 * rating;
}

long calculate_doors_unused(long plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    long count;
    dungeon = get_dungeon(plyr_idx);
    count = 0;
    for (i=1; i < DOOR_TYPES_COUNT; i++)
    {
      count += dungeon->door_amount[i];
    }
    return count;
}

long calculate_traps_unused(long plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    long count;
    dungeon = get_dungeon(plyr_idx);
    count = 0;
    for (i=1; i < TRAP_TYPES_COUNT; i++)
    {
      count += dungeon->trap_amount[i];
    }
    return count;
}

long count_rooms_of_type(long plyr_idx, long rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long i;
    unsigned long k;
    dungeon = get_dungeon(plyr_idx);
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
      room = room_get(i);
      if (room_is_invalid(room))
      {
        ERRORLOG("Jump to invalid room detected");
        break;
      }
      i = room->next_of_owner;
      // No Per-room code - we only want count
      k++;
      if (k > ROOMS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping rooms list");
        break;
      }
    }
    return k;
}

long calculate_num_rooms(long plyr_idx)
{
    struct PlayerInfo *player;
    long rkind;
    long count;
    count = 0;
    player = get_player(plyr_idx);
    for (rkind=1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        if ((rkind != RoK_ENTRANCE) && (rkind != RoK_DUNGHEART))
        {
            count += count_rooms_of_type(player->id_number, rkind);
        }
    }
    return count;
}

long calculate_entrances(long plyr_idx)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long i;
    unsigned long k;
    long count;
    dungeon = get_dungeon(plyr_idx);
    count = 0;
    i = dungeon->room_kind[game.entrance_room_id];
    k = 0;
    while (i != 0)
    {
      room = room_get(i);
      if (room_is_invalid(room))
      {
        ERRORLOG("Jump to invalid room detected");
        break;
      }
      i = room->next_of_kind;
      // Per-room code
      if (room->owner == plyr_idx)
          count++;
      // Per-room code ends
      k++;
      if (k > ROOMS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping rooms list");
        break;
      }
    }
    return count;
}

void frontstats_initialise(void)
{
    struct Dungeon *dungeon;
    //_DK_frontstats_initialise();
    dungeon = get_my_dungeon();
    dungeon->lvstats.end_time = LbTimerClock();
    dungeon->lvstats.num_creatures = dungeon->num_active_creatrs;
    dungeon->lvstats.imps_deployed = dungeon->num_active_diggers;
    dungeon->lvstats.battles_won = dungeon->battles_won;
    dungeon->lvstats.battles_lost = dungeon->battles_lost;
    dungeon->lvstats.money = dungeon->total_money_owned;
    dungeon->lvstats.dngn_breached_count = dungeon->times_broken_into;
    dungeon->lvstats.doors_destroyed = dungeon->doors_destroyed;
    dungeon->lvstats.rooms_destroyed = dungeon->rooms_destroyed;
    dungeon->lvstats.dungeon_area = dungeon->total_area;
    dungeon->lvstats.ideas_researched = (dungeon->total_research_points >> 8);
    dungeon->lvstats.creatures_scavenged = dungeon->creatures_scavenged;
    dungeon->lvstats.creatures_summoned = dungeon->creatures_summoned;
    dungeon->lvstats.spells_stolen = dungeon->spells_stolen;
    dungeon->lvstats.gold_pots_stolen = dungeon->gold_pots_stolen;
    dungeon->lvstats.average_room_efficiency  = calculate_efficiency(my_player_number);
    dungeon->lvstats.player_rating = calculate_rating(my_player_number);
    dungeon->lvstats.player_style = calculate_style(my_player_number);
    dungeon->lvstats.doors_unused = calculate_doors_unused(my_player_number);
    dungeon->lvstats.traps_unused = calculate_traps_unused(my_player_number);
    dungeon->lvstats.num_rooms = calculate_num_rooms(my_player_number);
    dungeon->lvstats.gameplay_time = (dungeon->lvstats.end_time - dungeon->lvstats.start_time) / 1000;
    dungeon->lvstats.num_entrances = calculate_entrances(my_player_number);
    dungeon->lvstats.hopes_dashed = game.play_gameturn;
    memcpy(&frontstats_data, &dungeon->lvstats, sizeof(struct LevelStats));
}

void frontstats_draw_main_stats(struct GuiButton *gbtn)
{
  _DK_frontstats_draw_main_stats(gbtn);
}

void frontstats_draw_scrolling_stats(struct GuiButton *gbtn)
{
  _DK_frontstats_draw_scrolling_stats(gbtn);
}

/**
 * Loads next menu state after leaving frontstats.
 */
void frontstats_leave(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  LevelNumber lvnum;
  if ((game.system_flags & GSF_NetworkActive) != 0)
  {
    if ( setup_old_network_service() )
    {
      frontend_set_state(FeSt_NET_SESSION);
      fe_high_score_table_from_main_menu = false;
    } else
    {
      frontend_set_state(FeSt_MAIN_MENU);
    }
  } else
  {
    player = get_my_player();
    lvnum = get_loaded_level_number();
    if (player->victory_state == VicS_WonLevel)
    {
      frontend_set_state(FeSt_HIGH_SCORES);
      fe_high_score_table_from_main_menu = false;
    } else
    if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum) || is_extra_level(lvnum))
    {
      frontend_set_state(FeSt_LAND_VIEW);
    } else
    if (is_freeplay_level(lvnum))
    {
      frontend_set_state(FeSt_LEVEL_SELECT);
    } else
    {
      frontend_set_state(FeSt_MAIN_MENU);
    }
  }
}

void frontstats_set_timer(void)
{
  frontstats_timer = LbTimerClock() + 3000;
}

void frontstats_update(void)
{
  LevelNumber lvnum;
  int h;
  scrolling_offset++;
  LbTextSetFont(frontend_font[1]);
  h = LbTextLineHeight();
  if (h+4 < scrolling_offset)
  {
    scrolling_offset -= h+4;
    scrolling_index++;
    if (!scrolling_stats_data[scrolling_index].field_0)
      scrolling_index = 0;
  }
  lvnum = get_loaded_level_number();
  if (frontstats_timer != 0)
    if (LbTimerClock() > frontstats_timer)
    {
      play_description_speech(lvnum,0);
      frontstats_timer = 0;
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
