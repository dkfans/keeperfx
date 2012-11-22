/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file dungeon_stats.h
 *     Header file for dungeon_stats.c.
 * @par Purpose:
 *     Dungeon stats structures definitions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Jan 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_DNGN_STATS_H
#define DK_DNGN_STATS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct LevelStats { // sizeof = 392
  unsigned long things_researched;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned long field_14;
  unsigned long field_18;
  unsigned long field_1C;
  unsigned long field_20;
  unsigned long field_24;
  unsigned long field_28;
  unsigned long field_2C;
  unsigned long field_30;
  unsigned long field_34;
  unsigned long field_38;
  unsigned long field_3C;
  unsigned long field_40;
  unsigned long field_44;
  unsigned long field_48;
  unsigned long field_4C;
  unsigned long field_50;
  unsigned long field_54;
  unsigned long field_58;
  unsigned long field_5C;
  unsigned long field_60;
  unsigned long field_64;
  unsigned long field_68;
  unsigned long field_6C;
  unsigned long field_70;
  unsigned long field_74;
  unsigned long field_78;
  unsigned long field_7C;
  unsigned long field_80;
  unsigned long field_84;
  unsigned long field_88;
  unsigned long gold_mined;
  unsigned long field_90;
  unsigned long manufactured_doors;
  unsigned long manufactured_traps;
  unsigned long manufactured_items;
  unsigned long start_time;
  unsigned long end_time;
  unsigned long creatures_trained;
  unsigned long creatures_tortured;
  unsigned long creatures_sacrificed;
  unsigned long creatures_converted;
  unsigned long creatures_summoned;
  unsigned long num_slaps;
  unsigned long num_caveins;
  unsigned long bridges_built;
  unsigned long rock_dug_out;
  unsigned long salary_cost;
  unsigned long flies_killed_by_spiders;
  unsigned long territory_destroyed;
  unsigned long territory_lost;
  unsigned long rooms_constructed;
  unsigned long traps_used;
  unsigned long traps_armed;
  unsigned long doors_used;
  unsigned long keepers_destroyed;
  unsigned long area_claimed;
  unsigned long backs_stabbed;
  unsigned long chickens_hatched;
  unsigned long chickens_eaten;
  unsigned long chickens_wasted;
  unsigned long promises_broken;
  unsigned long ghosts_raised;
  unsigned long skeletons_raised;
  unsigned long friendly_kills;
  unsigned long lies_told;
  unsigned long creatures_annoyed;
  unsigned long graveyard_bodys;
  unsigned long vamps_created;
  unsigned long num_creatures;
  unsigned long imps_deployed;
  unsigned long battles_won;
  unsigned long battles_lost;
  unsigned long money;
  unsigned long dngn_breached_count;
  unsigned long doors_destroyed;
  unsigned long rooms_destroyed;
  unsigned long dungeon_area;
  unsigned long ideas_researched;
  unsigned long creatures_scavenged;
  unsigned long creatures_from_sacrifice;
  unsigned long spells_stolen;
  unsigned long gold_pots_stolen;
  unsigned long average_room_efficiency;
  unsigned long player_rating;
  unsigned long player_style;
  unsigned long doors_unused;
  unsigned long traps_unused;
  unsigned long num_rooms;
  unsigned long gameplay_time;
  unsigned long num_entrances;
  unsigned long hopes_dashed;
  unsigned long allow_save_score;
  unsigned long player_score;
};

#pragma pack()
/******************************************************************************/
long update_dungeons_scores(void);
TbBool load_stats_files(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
