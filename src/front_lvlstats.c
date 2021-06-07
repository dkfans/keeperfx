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
#include "frontmenu_ingame_evnt.h"
#include "frontend.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "room_list.h"
#include "game_merge.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

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
extern struct StatsData main_stats_data[];
extern struct StatsData scrolling_stats_data[];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/** Calculates average efficiency of player's rooms.
 * @param plyr_idx Player for whom statistic is to be calculated.
 * @return Statistic value, scaled 0..100.
 */
long calculate_efficiency(PlayerNumber plyr_idx)
{
    long count = 0;
    long efficiency = 0;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    for (long rkind = 1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        long i = dungeon->room_kind[rkind];
        unsigned long k = 0;
        while (i != 0)
        {
            struct Room* room = room_get(i);
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
    return 100 * efficiency / (count * ROOM_EFFICIENCY_MAX);
}

long calculate_style(long plyr_idx)
{
    long area = 0;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    for (long rkind = 1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        long i = dungeon->room_kind[rkind];
        unsigned long k = 0;
        while (i != 0)
        {
            struct Room* room = room_get(i);
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
    long half_area = (dungeon->total_area >> 1);
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
    long rating = calculate_style(plyr_idx) * calculate_efficiency(plyr_idx) / 100;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    rating += 100 * dungeon->lvstats.player_score / 800;
    long btlost = dungeon->lvstats.battles_lost;
    long btwon = dungeon->lvstats.battles_won;
    // Find scoring ratio
    long ratio = 100;
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
    struct DungeonAdd* dungeonadd = get_dungeonadd(plyr_idx);
    long count = 0;
    for (long i = 1; i < gameadd.trapdoor_conf.door_types_count; i++)
    {
      count += dungeonadd->mnfct_info.door_amount_stored[i];
    }
    return count;
}

long calculate_traps_unused(PlayerNumber plyr_idx)
{
    struct DungeonAdd* dungeon = get_dungeonadd(plyr_idx);
    long count = 0;
    for (long i = 1; i < gameadd.trapdoor_conf.trap_types_count; i++)
    {
      count += dungeon->mnfct_info.trap_amount_stored[i];
    }
    return count;
}

void frontstats_initialise(void)
{
    // Initialize stats in dungeon
    struct Dungeon* dungeon = get_my_dungeon();
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
    int fs_units_per_px = scroll_box_get_units_per_px(gbtn);
    draw_scroll_box(gbtn, fs_units_per_px, 6);
    LbTextSetFont(frontend_font[1]);
    // The GUI item height should be 6 lines of text
    int tx_units_per_px = gbtn->height * 16 / (6 * (LbTextLineHeight() + 1));
    int ln_height = LbTextLineHeight() * tx_units_per_px / 16;
    int pos_x = gbtn->scr_pos_x;
    int pos_y = gbtn->scr_pos_y + ln_height / 2;
    for (struct StatsData* stat = main_stats_data; stat->name_stridx > 0; stat++)
    {
        int border;
        {
            struct TbSprite* spr = &frontend_sprite[25];
            border = spr->SWidth * fs_units_per_px / 16;
        }
        LbTextSetWindow(pos_x + border, pos_y, gbtn->width - 2 * border, ln_height);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        LbTextDrawResized(0, 0, tx_units_per_px, get_string(stat->name_stridx));
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
        int stat_val;
        if (stat->get_value != NULL)
        {
            stat_val = stat->get_value(stat->get_arg);
        }
        else
        {
            stat_val = -1;
        }
        if ( (timer_enabled()) && (stat->name_stridx == 1746) && (!TimerGame) )
        {
            LbTextDrawResizedFmt(0, 0, tx_units_per_px, "%02ld:%02ld:%02ld:%03ld", Timer.Hours, Timer.Minutes, Timer.Seconds, Timer.MSeconds);       
        }
        else
        {    
            LbTextDrawResizedFmt(0, 0, tx_units_per_px, "%d", stat_val);
        }
        pos_y += ln_height + 1 * units_per_pixel / 16;
    }
}

void frontstats_draw_scrolling_stats(struct GuiButton *gbtn)
{
    int fs_units_per_px = scroll_box_get_units_per_px(gbtn);
    draw_scroll_box(gbtn, fs_units_per_px, 5);
    LbTextSetFont(frontend_font[1]);
    {
        struct TbSprite* spr = &frontend_sprite[25];
        LbTextSetWindow(gbtn->scr_pos_x + spr->SWidth * fs_units_per_px / 16, gbtn->scr_pos_y + (spr->SHeight-7) * fs_units_per_px / 16,
          gbtn->width - 2 * (spr->SWidth * fs_units_per_px / 16), gbtn->height + 2 * (8 - spr->SHeight) * fs_units_per_px / 16);
    }
    // The GUI item height should be 5 lines of text
    int tx_units_per_px = gbtn->height * 16 / (5 * (LbTextLineHeight() + 1));
    int ln_height = LbTextLineHeight() * tx_units_per_px / 16;
    int pos_x = 0;
    int pos_y = -scrolling_offset * tx_units_per_px / 16;
    for (struct StatsData* stat = &scrolling_stats_data[scrolling_index]; pos_y < gbtn->height; pos_y += ln_height + 4 * units_per_pixel / 16)
    {
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
        LbTextDrawResized(pos_x, pos_y, tx_units_per_px, get_string(stat->name_stridx));
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
        int stat_val;
        if (stat->get_value != NULL)
        {
            stat_val = stat->get_value(stat->get_arg);
        }
        else
        {
            stat_val = -1;
        }
        LbTextDrawResizedFmt(pos_x, pos_y, tx_units_per_px, "%d", stat_val);
        stat++;
        if (!stat->name_stridx)
          stat = scrolling_stats_data;
    }
}

void init_menu_state_on_net_stats_exit(void)
{
    FrontendMenuState nstate = get_menu_state_when_back_from_substate(FeSt_LEVEL_STATS);
    if (nstate == FeSt_NET_SESSION)
    {
        // If the parent state is network session state, try to stay in net service
        if (!setup_old_network_service()) {
            nstate = get_menu_state_when_back_from_substate(nstate);
        }
    }
    frontend_set_state(nstate);
    fe_high_score_table_from_main_menu = false;
}

/**
 * Loads next menu state after leaving frontstats.
 */
void frontstats_leave(struct GuiButton *gbtn)
{
    init_menu_state_on_net_stats_exit();
}

void frontstats_set_timer(void)
{
    initialize_description_speech();
    frontstats_timer = LbTimerClock() + 2000;
}

void frontstats_update(void)
{
    scrolling_offset++;
    LbTextSetFont(frontend_font[1]);
    int h = LbTextLineHeight();
    if (h+4 < scrolling_offset)
    {
        scrolling_offset -= h+4;
        scrolling_index++;
        if (!scrolling_stats_data[scrolling_index].name_stridx)
            scrolling_index = 0;
    }
    if (frontstats_timer != 0)
    {
        LevelNumber lvnum = get_loaded_level_number();
        if (LbTimerClock() > frontstats_timer)
        {
            play_description_speech(lvnum,0);
            frontstats_timer = 0;
        }
    }
}

/******************************************************************************/
