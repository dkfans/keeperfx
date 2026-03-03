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
#include "player_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct LevelStats {
  uint32_t things_researched;
  uint32_t creatures_attracted;
  uint32_t gold_mined;
  uint32_t manufactured_doors;
  uint32_t manufactured_traps;
  uint32_t manufactured_items;
  uint32_t start_time;
  uint32_t end_time;
  uint32_t creatures_trained;
  uint32_t creatures_tortured;
  uint32_t creatures_sacrificed;
  uint32_t creatures_converted;
  uint32_t creatures_summoned;
  uint32_t num_slaps;
  uint32_t num_caveins;
  uint32_t bridges_built;
  uint32_t rock_dug_out;
  uint32_t salary_cost;
  uint32_t flies_killed_by_spiders;
  uint32_t territory_destroyed;
  uint32_t territory_lost;
  uint32_t rooms_constructed;
  uint32_t traps_used;
  uint32_t traps_armed;
  uint32_t doors_used;
  uint32_t keepers_destroyed;
  uint32_t area_claimed;
  uint32_t backs_stabbed;
  uint32_t chickens_hatched;
  uint32_t chickens_eaten;
  uint32_t chickens_wasted;
  uint32_t promises_broken;
  uint32_t ghosts_raised;
  uint32_t skeletons_raised;
  uint32_t friendly_kills;
  uint32_t lies_told;
  uint32_t creatures_annoyed;
  uint32_t graveyard_bodys;
  uint32_t vamps_created;
  uint32_t num_creatures;
  uint32_t imps_deployed;
  uint32_t battles_won;
  uint32_t battles_lost;
  uint32_t money;
  uint32_t dngn_breached_count;
  uint32_t doors_destroyed;
  uint32_t rooms_destroyed;
  uint32_t dungeon_area;
  uint32_t ideas_researched;
  uint32_t creatures_scavenged;
  uint32_t creatures_from_sacrifice;
  uint32_t spells_stolen;
  uint32_t gold_pots_stolen;
  uint32_t average_room_efficiency;
  uint32_t player_rating;
  uint32_t player_style;
  uint32_t doors_unused;
  uint32_t traps_unused;
  uint32_t num_rooms;
  uint32_t gameplay_time;
  uint32_t num_entrances;
  uint32_t hopes_dashed;
  uint32_t allow_save_score;
  uint32_t player_score;
  uint32_t keeper_destroyed[PLAYERS_COUNT];
};

#pragma pack()
/******************************************************************************/
long update_dungeons_scores(void);
TbBool update_dungeon_scores_for_player(struct PlayerInfo *player);
TbBool load_stats_files(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
