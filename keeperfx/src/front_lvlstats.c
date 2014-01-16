/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_lvlstats.c
 *     High Score screen displaying routines.
 * @par Purpose:
 *     Functions to show and maintain the level statistics screen.
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
#include "gui_draw.h"
#include "config_strings.h"
#include "config_campaigns.h"
#include "config_terrain.h"
#include "config_settings.h"
#include "front_highscore.h"
#include "front_landview.h"
#include "frontend.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "room_list.h"
#include "game_merge.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
//DLLIMPORT extern struct StatsData _DK_scrolling_stats_data[];
//#define scrolling_stats_data _DK_scrolling_stats_data
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
extern struct StatsData main_stats_data[];
extern struct StatsData scrolling_stats_data[];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/** Calculates average efficiency of player's rooms.
 * @param plyr_idx Player for whom statistic is to be calculated.
 * @return Statistic value.
 */
long calculate_efficiency(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
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
            struct Room *room;
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
            struct Room *room;
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
long calculate_rating(PlayerNumber plyr_idx)
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

long calculate_doors_unused(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    long count;
    dungeon = get_dungeon(plyr_idx);
    count = 0;
    for (i=1; i < DOOR_TYPES_COUNT; i++)
    {
      count += dungeon->door_amount_stored[i];
    }
    return count;
}

long calculate_traps_unused(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    long count;
    dungeon = get_dungeon(plyr_idx);
    count = 0;
    for (i=1; i < TRAP_TYPES_COUNT; i++)
    {
      count += dungeon->trap_amount_stored[i];
    }
    return count;
}

void frontstats_initialise(void)
{
    struct Dungeon *dungeon;
    //_DK_frontstats_initialise();
    // Initialize stats in dungeon
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
    dungeon->lvstats.creatures_scavenged = dungeon->creatures_scavenge_gain;
    dungeon->lvstats.creatures_summoned = dungeon->creatures_scavenge_lost;
    dungeon->lvstats.spells_stolen = dungeon->spells_stolen;
    dungeon->lvstats.gold_pots_stolen = dungeon->gold_pots_stolen;
    dungeon->lvstats.average_room_efficiency  = calculate_efficiency(my_player_number);
    dungeon->lvstats.player_rating = calculate_rating(my_player_number);
    dungeon->lvstats.player_style = calculate_style(my_player_number);
    dungeon->lvstats.doors_unused = calculate_doors_unused(my_player_number);
    dungeon->lvstats.traps_unused = calculate_traps_unused(my_player_number);
    dungeon->lvstats.num_rooms = calculate_player_num_rooms_built(my_player_number);
    dungeon->lvstats.gameplay_time = (dungeon->lvstats.end_time - dungeon->lvstats.start_time) / 1000;
    dungeon->lvstats.num_entrances = count_player_rooms_entrances(my_player_number);
    dungeon->lvstats.hopes_dashed = game.play_gameturn;
    memcpy(&frontstats_data, &dungeon->lvstats, sizeof(struct LevelStats));
}

void frontstats_draw_main_stats(struct GuiButton *gbtn)
{
    struct StatsData *stat;
    int stat_val;
    int pos_x,pos_y;
    //_DK_frontstats_draw_main_stats(gbtn);
    draw_scroll_box(gbtn, 6);
    LbTextSetFont(frontend_font[1]);
    pos_x = gbtn->scr_pos_x;
    pos_y = LbTextLineHeight()/2 + gbtn->scr_pos_y;
    for (stat = main_stats_data; stat->name_stridx > 0; stat++)
    {
        struct TbSprite *spr;
        spr = &frontend_sprite[25];
        LbTextSetWindow( (spr->SWidth + pos_x) / pixel_size, pos_y / pixel_size,
          (gbtn->width - 2 * spr->SWidth) / pixel_size, LbTextLineHeight() / pixel_size);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        LbTextSetFont(frontend_font[1]);
        LbTextDraw(0, 0, gui_string(stat->name_stridx));
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
        if (stat->get_value != NULL) {
            stat_val = stat->get_value(stat->get_arg);
        } else {
            stat_val = -1;
        }
        LbTextDrawFmt(0, 0, "%d", stat_val);
        pos_y += LbTextLineHeight() + 1;
    }
}

void frontstats_draw_scrolling_stats(struct GuiButton *gbtn)
{
    struct StatsData *stat;
    int stat_val;
    int pos_x,pos_y;
    int ln_height;
    //_DK_frontstats_draw_scrolling_stats(gbtn);
    draw_scroll_box(gbtn, 5);
    {
        struct TbSprite *spr;
        spr = &frontend_sprite[25];
        LbTextSetWindow(spr->SWidth + gbtn->scr_pos_x, spr->SHeight + gbtn->scr_pos_y - 7,
          gbtn->width - 2 * spr->SWidth, gbtn->height + 2 * (8 - spr->SHeight));
    }
    pos_x = 0;
    pos_y = -scrolling_offset;
    for ( stat = &scrolling_stats_data[scrolling_index]; pos_y < gbtn->height; pos_y += ln_height + 4 )
    {
        lbDisplay.DrawFlags = 0x20;
        LbTextSetFont(frontend_font[1]);
        LbTextDraw(pos_x, pos_y, gui_string(stat->name_stridx));
        lbDisplay.DrawFlags = 0x80;
        if (stat->get_value != NULL) {
            stat_val = stat->get_value(stat->get_arg);
        } else {
            stat_val = -1;
        }
        LbTextDrawFmt(pos_x, pos_y, "%d", stat_val);
        stat++;
        if (!stat->name_stridx)
          stat = scrolling_stats_data;
        ln_height = LbTextLineHeight();
    }
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
    if (is_multiplayer_level(lvnum))
    {
        frontend_set_state(FeSt_NET_SERVICE);
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
    initialize_description_speech();
    frontstats_timer = LbTimerClock() + 2000;
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
        if (!scrolling_stats_data[scrolling_index].name_stridx)
            scrolling_index = 0;
    }
    lvnum = get_loaded_level_number();
    if (frontstats_timer != 0)
    {
        if (LbTimerClock() > frontstats_timer)
        {
            play_description_speech(lvnum,0);
            frontstats_timer = 0;
        }
    }
}

/******************************************************************************/
