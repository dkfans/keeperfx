/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_lvlstats_data.c
 *     Level statistics screen displaying c++ routines.
 * @par Purpose:
 *     Data and functions to show and maintain the level statistics screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     01 Jan 2012 - 29 Oct 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_strings.h"
#include "front_lvlstats.h"
#include "globals.h"
#include "bflib_basics.h"

#include "dungeon_stats.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long stat_return_c_slong(void *ptr);

struct LevelStats frontstats_data;
/******************************************************************************/
struct StatsData main_stats_data[] = {
    {GUIStr_StatisticsNames1+8, stat_return_c_slong, &frontstats_data.creatures_attracted},
    {GUIStr_StatisticsNames1+2, stat_return_c_slong, &frontstats_data.average_room_efficiency},
    {GUIStr_StatisticsNames1+5, stat_return_c_slong, &frontstats_data.gameplay_time},
    {GUIStr_StatisticsNames1+6, stat_return_c_slong, &frontstats_data.player_style},
    {GUIStr_StatisticsNames1+7, stat_return_c_slong, &frontstats_data.player_rating},
    {0, NULL, NULL}
};

struct StatsData scrolling_stats_data[] = {
    {GUIStr_StatisticsNames1, stat_return_c_slong, &frontstats_data.num_creatures},
    {GUIStr_StatisticsNames1 + 1, stat_return_c_slong, &frontstats_data.gold_mined},
    {GUIStr_StatisticsNames1 + 4, stat_return_c_slong, &frontstats_data.money},
    {GUIStr_StatisticsNames1 + 9, stat_return_c_slong, &frontstats_data.battles_won},
    {GUIStr_StatisticsNames1 + 10, stat_return_c_slong, &frontstats_data.battles_lost},
    {GUIStr_StatisticsNames1 + 11, stat_return_c_slong, &frontstats_data.dngn_breached_count},
    {GUIStr_StatisticsNames1 + 12, stat_return_c_slong, &frontstats_data.imps_deployed},
    {GUIStr_StatisticsNames1 + 14, stat_return_c_slong, &frontstats_data.doors_destroyed},
    {GUIStr_StatisticsNames1 + 15, stat_return_c_slong, &frontstats_data.rooms_destroyed},
    {GUIStr_StatisticsNames1 + 16, stat_return_c_slong, &frontstats_data.dungeon_area},
    {GUIStr_StatisticsNames1 + 17, stat_return_c_slong, &frontstats_data.ideas_researched},
    {GUIStr_StatisticsNames1 + 18, stat_return_c_slong, &frontstats_data.creatures_scavenged},
    {GUIStr_StatisticsNames1 + 19, stat_return_c_slong, &frontstats_data.creatures_from_sacrifice},
    {GUIStr_StatisticsNames1 + 20, stat_return_c_slong, &frontstats_data.creatures_sacrificed},
    {GUIStr_StatisticsNames1 + 21, stat_return_c_slong, &frontstats_data.creatures_tortured},
    {GUIStr_StatisticsNames1 + 22, stat_return_c_slong, &frontstats_data.creatures_trained},
    {GUIStr_StatisticsNames1 + 23, stat_return_c_slong, &frontstats_data.gold_pots_stolen},
    {GUIStr_StatisticsNames1 + 24, stat_return_c_slong, &frontstats_data.spells_stolen},
    {GUIStr_StatisticsNames1 + 25, stat_return_c_slong, &frontstats_data.manufactured_traps},
    {GUIStr_StatisticsNames1 + 26, stat_return_c_slong, &frontstats_data.traps_unused},
    {GUIStr_StatisticsNames1 + 27, stat_return_c_slong, &frontstats_data.manufactured_doors},
    {GUIStr_StatisticsNames1 + 28, stat_return_c_slong, &frontstats_data.doors_unused},
    {GUIStr_StatisticsNames1 + 29, stat_return_c_slong, &frontstats_data.num_rooms},
    {GUIStr_StatisticsNames1 + 30, stat_return_c_slong, &frontstats_data.num_entrances},
    {GUIStr_StatisticsNames1 + 31, stat_return_c_slong, &frontstats_data.num_slaps},
    {GUIStr_StatisticsNames1 + 32, stat_return_c_slong, &frontstats_data.num_caveins},
    {GUIStr_StatisticsNames1 + 33, stat_return_c_slong, &frontstats_data.skeletons_raised},
    {GUIStr_StatisticsNames1 + 34, stat_return_c_slong, &frontstats_data.bridges_built},
    {GUIStr_StatisticsNames1 + 35, stat_return_c_slong, &frontstats_data.rock_dug_out},
    {GUIStr_StatisticsNames1 + 36, stat_return_c_slong, &frontstats_data.salary_cost},
    {GUIStr_StatisticsNames1 + 37, stat_return_c_slong, &frontstats_data.flies_killed_by_spiders},
    {GUIStr_StatisticsNames1 + 38, stat_return_c_slong, &frontstats_data.territory_destroyed},
    {GUIStr_StatisticsNames1 + 39, stat_return_c_slong, &frontstats_data.rooms_constructed},
    {GUIStr_StatisticsNames1 + 40, stat_return_c_slong, &frontstats_data.traps_used},
    {GUIStr_StatisticsNames1 + 41, stat_return_c_slong, &frontstats_data.keepers_destroyed},
    {GUIStr_StatisticsNames1 + 42, stat_return_c_slong, &frontstats_data.area_claimed},
    {GUIStr_StatisticsNames1 + 43, stat_return_c_slong, &frontstats_data.backs_stabbed},
    {GUIStr_StatisticsNames1 + 44, stat_return_c_slong, &frontstats_data.chickens_hatched},
    {GUIStr_StatisticsNames1 + 45, stat_return_c_slong, &frontstats_data.chickens_eaten},
    {GUIStr_StatisticsNames1 + 46, stat_return_c_slong, &frontstats_data.hopes_dashed},
    {GUIStr_StatisticsNames1 + 47, stat_return_c_slong, &frontstats_data.promises_broken},
    {GUIStr_StatisticsNames1 + 48, stat_return_c_slong, &frontstats_data.ghosts_raised},
    {GUIStr_StatisticsNames1 + 49, stat_return_c_slong, &frontstats_data.doors_used},
    {GUIStr_StatisticsNames1 + 50, stat_return_c_slong, &frontstats_data.friendly_kills},
    {GUIStr_StatisticsNames1 + 51, stat_return_c_slong, &frontstats_data.things_researched},
    {GUIStr_StatisticsNames1 + 53, stat_return_c_slong, &frontstats_data.manufactured_items},
    {GUIStr_StatisticsNames1 + 54, stat_return_c_slong, &frontstats_data.creatures_converted},
    {GUIStr_StatisticsNames1 + 19, stat_return_c_slong, &frontstats_data.creatures_summoned},
    {GUIStr_StatisticsNames1 + 55, stat_return_c_slong, &frontstats_data.territory_lost},
    {GUIStr_StatisticsNames1 + 56, stat_return_c_slong, &frontstats_data.traps_armed},
    {GUIStr_StatisticsNames1 + 57, stat_return_c_slong, &frontstats_data.chickens_wasted},
    {GUIStr_StatisticsNames1 + 58, stat_return_c_slong, &frontstats_data.lies_told},
    {GUIStr_StatisticsNames1 + 59, stat_return_c_slong, &frontstats_data.creatures_annoyed},
    {GUIStr_StatisticsNames1 + 60, stat_return_c_slong, &frontstats_data.graveyard_bodys},
    {GUIStr_StatisticsNames1 + 61, stat_return_c_slong, &frontstats_data.vamps_created},
    {0, NULL, NULL}
};
/******************************************************************************/
long stat_return_c_slong(void *ptr)
{
    if (ptr == NULL) return 0;
    return *(long *)ptr;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
