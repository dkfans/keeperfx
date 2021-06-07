/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file dungeon_stats.c
 *     Dungeons statistics handling.
 * @par Purpose:
 *     Acquisition and maintaining of dungeon-related statistics.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     01 Jan 2012 - 22 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "dungeon_stats.h"
#include "globals.h"
#include "bflib_basics.h"

#include "config.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_objects.h"
#include "config_lenses.h"
#include "config_trapdoor.h"
#include "config_effects.h"
#include "config_terrain.h"
#include "room_library.h"
#include "game_legacy.h"

/******************************************************************************/
TbBool load_stats_files(void)
{
    TbBool result = true;
    clear_research_for_all_players();
    if (!load_creaturetypes_config(keeper_creaturetp_file,CnfLd_ListOnly))
      result = false;
    if (!load_terrain_config(keeper_terrain_file,CnfLd_ListOnly))
      result = false;
    if (!load_objects_config(keeper_objects_file,CnfLd_ListOnly))
      result = false;
    if (!load_trapdoor_config(keeper_trapdoor_file,CnfLd_ListOnly))
      result = false;
    if (!load_effects_config(keeper_effects_file,CnfLd_ListOnly))
      result = false;
    if (!load_lenses_config(keeper_lenses_file,CnfLd_ListOnly))
      result = false;
    if (!load_magic_config(keeper_magic_file,CnfLd_ListOnly))
      result = false;
    if (!load_creaturestates_config(creature_states_file,CnfLd_ListOnly))
      result = false;
    if (!load_terrain_config(keeper_terrain_file,CnfLd_Standard))
      result = false;
    if (!load_objects_config(keeper_objects_file,CnfLd_Standard))
      result = false;
    if (!load_trapdoor_config(keeper_trapdoor_file,CnfLd_Standard))
      result = false;
    if (!load_effects_config(keeper_effects_file,CnfLd_Standard))
      result = false;
    if (!load_lenses_config(keeper_lenses_file,CnfLd_Standard))
      result = false;
    if (!load_magic_config(keeper_magic_file,CnfLd_Standard))
      result = false;
    if (!load_creaturetypes_config(keeper_creaturetp_file,CnfLd_Standard))
      result = false;
    if (!load_creaturestates_config(creature_states_file,CnfLd_Standard))
      result = false;
    // note that rules file requires definitions of magic and creature types
    if (!load_rules_config(keeper_rules_file,CnfLd_Standard))
      result = false;
    for (int i = 1; i < gameadd.crtr_conf.model_count; i++)
    {
      if (!load_creaturemodel_config(i,0))
        result = false;
    }
    game.field_149E7B = game.tile_strength;
//  LbFileSaveAt("!stat11", &game, sizeof(struct Game));
//  LbFileSaveAt("!stat12", &shot_stats, sizeof(shot_stats));
//  LbFileSaveAt("!stat13", &instance_info, sizeof(instance_info));
    SYNCDBG(3,"Finished");
    return result;
}

/**
 * Compute points related to room slabs and entrances.
 * @param num_entrances
 * @param rooms_area
 * @param entrance_gen
 * @return
 */
unsigned long compute_dungeon_rooms_attraction_score(long num_entrance_slbs, long rooms_area, long entrance_gen)
{
    if (num_entrance_slbs > 27) // That would be approx. 3 entrances
        num_entrance_slbs = 27;
    if (rooms_area > 128) // more than 5 rooms of size 5x5
        rooms_area = 128;
    if (entrance_gen >= 10)
        entrance_gen = 10;
    return 100 * entrance_gen + 100 * rooms_area + 400 * num_entrance_slbs;
}

/**
 * Computes efficiency of destroying and taking over enemy creatures.
 * @param battles_won
 * @param battles_lost
 * @param scavenge_gain
 * @param scavenge_lost
 * @return
 */
unsigned long compute_dungeon_creature_tactics_score(long battles_won, long battles_lost, long scavenge_gain, long scavenge_lost)
{
    long battle_efficiency = battles_won - battles_lost;
    if (battle_efficiency < 0)
        battle_efficiency = 0;
    if (battle_efficiency > 40)
        battle_efficiency = 40;
    long battle_total = battles_won + battles_lost;
    if (battle_total < 0)
        battle_total = 0;
    if (battle_total > 80)
        battle_total = 80;
    long scavenge_efficiency = scavenge_gain - scavenge_lost;
    if (scavenge_efficiency < 0)
        scavenge_efficiency = 0;
    if (scavenge_efficiency > 40)
        scavenge_efficiency = 40;
    return 25 * scavenge_efficiency + 25 * battle_efficiency + 2 * battle_total;
}

/**
 * Compute point related to acquired territory.
 * @param room_types
 * @param total_area
 * @return
 */
unsigned long compute_dungeon_rooms_variety_score(long room_types, long total_area)
{
    if (room_types < 0)
        room_types = 0;
    if (room_types > ROOM_TYPES_COUNT)
        room_types = ROOM_TYPES_COUNT;
    if (total_area < 0)
        total_area = 0;
    if (total_area >= 512)
        total_area = 512;
    return 3 * total_area + 25 * room_types;
}

unsigned long compute_dungeon_train_research_manufctr_wealth_score(long total_train, long total_research, long total_manufctr, long total_wealth)
{
    long pt_total_train = total_train / 256;
    if (pt_total_train < 0)
        pt_total_train = 0;
    if (pt_total_train >= 96000)
        pt_total_train = 96000;
    long pt_total_research = total_research / 256;
    if (pt_total_research < 0)
        pt_total_research = 0;
    if (pt_total_research >= 96000)
        pt_total_research = 96000;
    long pt_total_manufctr = total_manufctr / 256;
    if (pt_total_manufctr < 0)
        pt_total_manufctr = 0;
    if (pt_total_manufctr >= 96000)
        pt_total_manufctr = 96000;
    long pt_total_wealth = total_wealth;
    if (pt_total_wealth < 0)
        pt_total_wealth = 0;
    if (pt_total_wealth >= 30000)
        pt_total_wealth = 30000;
    return pt_total_train / 2 + pt_total_research / 2 + pt_total_manufctr / 2 + 3 * pt_total_wealth;
}

unsigned long compute_dungeon_creature_amount_score(long total_creatrs)
{
    if (total_creatrs < 0)
        total_creatrs = 0;
    if (total_creatrs > 160)
        total_creatrs = 160;
    return 512 * total_creatrs;
}

unsigned long compute_dungeon_creature_mood_score(long survived_creatrs, long annoyed_creatrs)
{
    if (survived_creatrs <= 0)
        return 0;
    if (survived_creatrs > 160)
        survived_creatrs = 160;
    if (annoyed_creatrs < 0)
        annoyed_creatrs = 0;
    if (annoyed_creatrs > 160)
        annoyed_creatrs = 160;
    // Don't allow more angry creatures than total creatures
    if (survived_creatrs < annoyed_creatrs)
        survived_creatrs = annoyed_creatrs;
    return (annoyed_creatrs << 8) / survived_creatrs;
}

/**
 * Updates gameplay score for a dungeon belonging to given player.
 * @return
 */
TbBool update_dungeon_scores_for_player(struct PlayerInfo *player)
{
    int i;
    int k;
    struct Dungeon* dungeon = get_players_dungeon(player);
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    unsigned long manage_efficiency = 0;
    unsigned long max_manage_efficiency = 0;
    {
        manage_efficiency += 40 * compute_dungeon_rooms_attraction_score(dungeon->room_slabs_count[RoK_ENTRANCE],
            dungeon->room_manage_area, dungeon->portal_scavenge_boost);
        max_manage_efficiency += 40 * compute_dungeon_rooms_attraction_score(LONG_MAX, LONG_MAX, LONG_MAX);
    }
    {
        manage_efficiency += 40 * compute_dungeon_creature_tactics_score(dungeon->battles_won, dungeon->battles_lost,
            dungeon->creatures_scavenge_gain, dungeon->creatures_scavenge_lost);
        max_manage_efficiency += 40 * compute_dungeon_creature_tactics_score(LONG_MAX, LONG_MAX, LONG_MAX, LONG_MAX);
    }
    {
        // Compute amount of different types of rooms built
        unsigned long room_types = 0;
        for (i=0; i < ROOM_TYPES_COUNT; i++)
        {
            if (dungeon->room_slabs_count[i] > 0)
                room_types++;
        }
        manage_efficiency += 40 * compute_dungeon_rooms_variety_score(room_types, dungeon->total_area);
        max_manage_efficiency += 40 * compute_dungeon_rooms_variety_score(ROOM_TYPES_COUNT, LONG_MAX);
    }
    {
        manage_efficiency += compute_dungeon_train_research_manufctr_wealth_score(dungeon->total_experience_creatures_gained,
            dungeon->total_research_points, dungeon->total_manufacture_points, dungeon->total_money_owned);
        max_manage_efficiency += compute_dungeon_train_research_manufctr_wealth_score(LONG_MAX, LONG_MAX, LONG_MAX, LONG_MAX);
    }
    unsigned long creatures_efficiency;
    unsigned long creatures_mood;
    unsigned long max_creatures_efficiency;
    unsigned long max_creatures_mood;
    {
        creatures_efficiency = compute_dungeon_creature_amount_score(dungeon->num_active_creatrs);
        max_creatures_efficiency = compute_dungeon_creature_amount_score(LONG_MAX);
        creatures_mood = compute_dungeon_creature_mood_score(dungeon->num_active_creatrs,dungeon->creatures_annoyed);
        max_creatures_mood = compute_dungeon_creature_mood_score(LONG_MAX,LONG_MAX);
    }
    { // Compute total score for this turn
        i = manage_efficiency + creatures_efficiency;
        k = max_manage_efficiency + max_creatures_efficiency;
        dungeon->total_score = 1000 * i / k;
    }
    { // Compute managing efficiency score
        i = manage_efficiency - creatures_efficiency;
        k = max_manage_efficiency - max_creatures_efficiency;
        if (i < 0)
            i = 0;
        long raw_score = 1000 * i / k;
        // Angry creatures may degrade the score by up to 50%
        raw_score = raw_score / 2 + raw_score * (max_creatures_mood - creatures_mood) / (2*max_creatures_mood);
        dungeon->manage_score = raw_score;
    }
    {
        unsigned long gameplay_score = dungeon->total_score;
        if (gameplay_score <= 1) {
            WARNLOG("Player %d total score for turn is too low.", (int)player->id_number);
            gameplay_score = 1;
        }
        dungeon->total_score = gameplay_score;
    }
    {
        unsigned long gameplay_score = dungeon->manage_score;
        if (gameplay_score <= 1) {
            WARNLOG("Player %d managing score for turn is too low.", (int)player->id_number);
            gameplay_score = 1;
        }
        dungeon->manage_score = gameplay_score;
    }
    { // Check to update max score
        unsigned long gameplay_score = dungeon->total_score;
        if (dungeon->max_gameplay_score < gameplay_score)
            dungeon->max_gameplay_score = gameplay_score;
    }
    return true;
}

/**
 * Updates scores for existing players.
 * @return Gives the amount of players for which stats were set successfully.
 */
long update_dungeons_scores(void)
{
    int k = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (!player_exists(player))
            continue;
        if (player->is_active == 1)
        {
            if (update_dungeon_scores_for_player(player)) {
                k++;
            }
        }
    }
    return k;
}
/******************************************************************************/
