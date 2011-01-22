/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai.h
 *     Header file for skirmish_ai.c
 * @par Purpose:
 *     Experimental computer player intended to play multiplayer maps better.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef SKIRMISH_AI_H
#define SKIRMISH_AI_H

#ifdef __cplusplus
extern "C" {
#endif

#define SAI_MAX_KEEPERS 4

enum SAI_Skill
{
    /**
     * Uses everything but is not as fast as SAI_SKILL_PRO.
     */
    SAI_SKILL_TEST,

    /**
     * No sacrifices. No attack spells, armageddon or destroy walls.
     * No scavenging room. No use of dungeon specials.
     */
    SAI_SKILL_N00B,

    /**
     * Imp sacrifices only. No attack spells, armageddon or destroy walls.
     * Uses steal hero, resurrect creature.
     */
    SAI_SKILL_NOVICE,

    /**
     * All (useful) sacrifices excluding horny. Cave in, and destroy walls allowed.
     * Uses steal hero, resurrect creature and increase level.
     */
    SAI_SKILL_AVERAGE,

    /**
     * All useful sacrifices. All spells except far sight.
     * Uses steal hero, resurrect creature,increase level and multiply creatures.
     */
    SAI_SKILL_VETERAN,

    /**
     * Far sight + everything that SAI_SKILL_VETERAN has + some extra lame tactics
     * such as freeing prisoners with Cave in.
     */
    SAI_SKILL_PRO,


    SAI_SKILL_COUNT //counter
};

enum SAI_PlanType
{
    SAI_PLAN_LEAST_RISKY,
    SAI_PLAN_MOST_REWARDING
};


/**
 * Initializes everything that can be initialized statically after a map change.
 */
void SAI_init_for_map(void);

/**
 * Initializes Skirmish AI for a player. Calls destroy_player implicitly any
 * initialization is performed.
 * @param plyr Index of player.
 */
void SAI_init_for_player(int plyr);

/**
 * Cleans up resources used by Skirmish AI for a player.
 * @param plyr Index of player.
 */
void SAI_destroy_for_player(int plyr);

/**
 * Sets the skill level of a computer player. Skill influences reaction time in
 * particular, but it also enables/disables use of some spells, rooms and sacrifices.
 * @param plyr Index of player.
 * @param skill
 */
void SAI_set_player_skill(int plyr, enum SAI_Skill skill);

/**
 * Sets the planning strategy of a computer player.
 * @param plyr Index of player.
 * @param strat One of SAI_PLAN_LEAST_RISKY and SAI_PLAN_MOST_REWARDING
 */
void SAI_set_player_plan_strategy(int plyr, enum SAI_PlanType strat);

/**
 * Runs Skirmish AI analysis shared by all players (i.e. just call once per frame).
 */
void SAI_run_shared(void);

/**
 * Runs Skirmish AI for a player.
 * @param plyr Index of player.
 */
void SAI_run_for_player(int plyr);



#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_H
