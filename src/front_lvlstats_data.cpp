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
    {1749, stat_return_c_slong, &frontstats_data.field_4},
    {1743, stat_return_c_slong, &frontstats_data.average_room_efficiency},
    {1746, stat_return_c_slong, &frontstats_data.gameplay_time},
    {1747, stat_return_c_slong, &frontstats_data.player_style},
    {1748, stat_return_c_slong, &frontstats_data.player_rating},
    {0, NULL, NULL}
};

struct StatsData scrolling_stats_data[] = {
    {1741, stat_return_c_slong, &frontstats_data.num_creatures},
    {1742, stat_return_c_slong, &frontstats_data.gold_mined},
    {1745, stat_return_c_slong, &frontstats_data.money},
    {1750, stat_return_c_slong, &frontstats_data.battles_won},
    {1751, stat_return_c_slong, &frontstats_data.battles_lost},
    {1752, stat_return_c_slong, &frontstats_data.dngn_breached_count},
    {1753, stat_return_c_slong, &frontstats_data.imps_deployed},
    {1755, stat_return_c_slong, &frontstats_data.doors_destroyed},
    {1756, stat_return_c_slong, &frontstats_data.rooms_destroyed},
    {1757, stat_return_c_slong, &frontstats_data.dungeon_area},
    {1758, stat_return_c_slong, &frontstats_data.ideas_researched},
    {1759, stat_return_c_slong, &frontstats_data.creatures_scavenged},
    {1760, stat_return_c_slong, &frontstats_data.creatures_from_sacrifice},
    {1761, stat_return_c_slong, &frontstats_data.creatures_sacrificed},
    {1762, stat_return_c_slong, &frontstats_data.creatures_tortured},
    {1763, stat_return_c_slong, &frontstats_data.creatures_trained},
    {1764, stat_return_c_slong, &frontstats_data.gold_pots_stolen},
    {1765, stat_return_c_slong, &frontstats_data.spells_stolen},
    {1766, stat_return_c_slong, &frontstats_data.manufactured_traps},
    {1767, stat_return_c_slong, &frontstats_data.traps_unused},
    {1768, stat_return_c_slong, &frontstats_data.manufactured_doors},
    {1769, stat_return_c_slong, &frontstats_data.doors_unused},
    {1770, stat_return_c_slong, &frontstats_data.num_rooms},
    {1771, stat_return_c_slong, &frontstats_data.num_entrances},
    {1772, stat_return_c_slong, &frontstats_data.num_slaps},
    {1773, stat_return_c_slong, &frontstats_data.num_caveins},
    {1774, stat_return_c_slong, &frontstats_data.skeletons_raised},
    {1775, stat_return_c_slong, &frontstats_data.bridges_built},
    {1776, stat_return_c_slong, &frontstats_data.rock_dug_out},
    {1777, stat_return_c_slong, &frontstats_data.salary_cost},
    {1778, stat_return_c_slong, &frontstats_data.flies_killed_by_spiders},
    {1779, stat_return_c_slong, &frontstats_data.territory_destroyed},
    {1780, stat_return_c_slong, &frontstats_data.rooms_constructed},
    {1781, stat_return_c_slong, &frontstats_data.traps_used},
    {1782, stat_return_c_slong, &frontstats_data.keepers_destroyed},
    {1783, stat_return_c_slong, &frontstats_data.area_claimed},
    {1784, stat_return_c_slong, &frontstats_data.backs_stabbed},
    {1785, stat_return_c_slong, &frontstats_data.chickens_hatched},
    {1786, stat_return_c_slong, &frontstats_data.chickens_eaten},
    {1787, stat_return_c_slong, &frontstats_data.hopes_dashed},
    {1788, stat_return_c_slong, &frontstats_data.promises_broken},
    {1789, stat_return_c_slong, &frontstats_data.ghosts_raised},
    {1790, stat_return_c_slong, &frontstats_data.doors_used},
    {1791, stat_return_c_slong, &frontstats_data.friendly_kills},
    {1792, stat_return_c_slong, &frontstats_data.things_researched},
    {1794, stat_return_c_slong, &frontstats_data.manufactured_items},
    {1795, stat_return_c_slong, &frontstats_data.creatures_converted},
    {1760, stat_return_c_slong, &frontstats_data.creatures_summoned},
    {1796, stat_return_c_slong, &frontstats_data.territory_lost},
    {1797, stat_return_c_slong, &frontstats_data.traps_armed},
    {1798, stat_return_c_slong, &frontstats_data.chickens_wasted},
    {1799, stat_return_c_slong, &frontstats_data.lies_told},
    {1800, stat_return_c_slong, &frontstats_data.creatures_annoyed},
    {1801, stat_return_c_slong, &frontstats_data.graveyard_bodys},
    {1802, stat_return_c_slong, &frontstats_data.vamps_created},
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
