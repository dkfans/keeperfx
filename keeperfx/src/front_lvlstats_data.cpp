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
#include "front_lvlstats.h"
#include "globals.h"
#include "bflib_basics.h"

#include "dungeon_stats.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long stat_return_c_slong(void *ptr);

DLLIMPORT extern struct LevelStats _DK_frontstats_data;
#define frontstats_data _DK_frontstats_data
/******************************************************************************/
struct StatsData main_stats_data[] = {
    {749, stat_return_c_slong, &frontstats_data.field_4},
    {743, stat_return_c_slong, &frontstats_data.average_room_efficiency},
    {746, stat_return_c_slong, &frontstats_data.gameplay_time},
    {747, stat_return_c_slong, &frontstats_data.player_style},
    {748, stat_return_c_slong, &frontstats_data.player_rating},
    {0}
};

struct StatsData scrolling_stats_data[] = {
    {741, stat_return_c_slong, &frontstats_data.num_creatures},
    {742, stat_return_c_slong, &frontstats_data.gold_mined},
    {745, stat_return_c_slong, &frontstats_data.money},
    {750, stat_return_c_slong, &frontstats_data.battles_won},
    {751, stat_return_c_slong, &frontstats_data.battles_lost},
    {752, stat_return_c_slong, &frontstats_data.dngn_breached_count},
    {753, stat_return_c_slong, &frontstats_data.imps_deployed},
    {755, stat_return_c_slong, &frontstats_data.doors_destroyed},
    {756, stat_return_c_slong, &frontstats_data.rooms_destroyed},
    {757, stat_return_c_slong, &frontstats_data.dungeon_area},
    {758, stat_return_c_slong, &frontstats_data.ideas_researched},
    {759, stat_return_c_slong, &frontstats_data.creatures_scavenged},
    {760, stat_return_c_slong, &frontstats_data.creatures_from_sacrifice},
    {761, stat_return_c_slong, &frontstats_data.creatures_sacrificed},
    {762, stat_return_c_slong, &frontstats_data.creatures_tortured},
    {763, stat_return_c_slong, &frontstats_data.creatures_trained},
    {764, stat_return_c_slong, &frontstats_data.gold_pots_stolen},
    {765, stat_return_c_slong, &frontstats_data.spells_stolen},
    {766, stat_return_c_slong, &frontstats_data.manufactured_traps},
    {767, stat_return_c_slong, &frontstats_data.traps_unused},
    {768, stat_return_c_slong, &frontstats_data.manufactured_doors},
    {769, stat_return_c_slong, &frontstats_data.doors_unused},
    {770, stat_return_c_slong, &frontstats_data.num_rooms},
    {771, stat_return_c_slong, &frontstats_data.num_entrances},
    {772, stat_return_c_slong, &frontstats_data.num_slaps},
    {773, stat_return_c_slong, &frontstats_data.num_caveins},
    {774, stat_return_c_slong, &frontstats_data.skeletons_raised},
    {775, stat_return_c_slong, &frontstats_data.bridges_built},
    {776, stat_return_c_slong, &frontstats_data.rock_dug_out},
    {777, stat_return_c_slong, &frontstats_data.salary_cost},
    {778, stat_return_c_slong, &frontstats_data.flies_killed_by_spiders},
    {779, stat_return_c_slong, &frontstats_data.territory_destroyed},
    {780, stat_return_c_slong, &frontstats_data.rooms_constructed},
    {781, stat_return_c_slong, &frontstats_data.traps_used},
    {782, stat_return_c_slong, &frontstats_data.keepers_destroyed},
    {783, stat_return_c_slong, &frontstats_data.area_claimed},
    {784, stat_return_c_slong, &frontstats_data.backs_stabbed},
    {785, stat_return_c_slong, &frontstats_data.chickens_hatched},
    {786, stat_return_c_slong, &frontstats_data.chickens_eaten},
    {787, stat_return_c_slong, &frontstats_data.hopes_dashed},
    {788, stat_return_c_slong, &frontstats_data.promises_broken},
    {789, stat_return_c_slong, &frontstats_data.ghosts_raised},
    {790, stat_return_c_slong, &frontstats_data.doors_used},
    {791, stat_return_c_slong, &frontstats_data.friendly_kills},
    {792, stat_return_c_slong, &frontstats_data.things_researched},
    {794, stat_return_c_slong, &frontstats_data.manufactured_items},
    {795, stat_return_c_slong, &frontstats_data.creatures_converted},
    {760, stat_return_c_slong, &frontstats_data.creatures_summoned},
    {796, stat_return_c_slong, &frontstats_data.territory_lost},
    {797, stat_return_c_slong, &frontstats_data.traps_armed},
    {798, stat_return_c_slong, &frontstats_data.chickens_wasted},
    {799, stat_return_c_slong, &frontstats_data.lies_told},
    {800, stat_return_c_slong, &frontstats_data.creatures_annoyed},
    {801, stat_return_c_slong, &frontstats_data.graveyard_bodys},
    {802, stat_return_c_slong, &frontstats_data.vamps_created},
    {0}
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
