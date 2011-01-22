/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_planner.h
 *     Header file for skirmish_ai_planner.cpp
 * @par Purpose:
 *     Planner/state tree explorer for new Skirmish AI.
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

#ifndef SKIRMISH_AI_PLANNER_H
#define SKIRMISH_AI_PLANNER_H

#include "skirmish_ai.h"

#ifdef __cplusplus
extern "C" {
#endif


enum SAI_PlanDecisionType
{
    SAI_PLAN_WAIT, //auxiliary node that means that we should wait as long as required for preconditions of children to become true
    SAI_PLAN_TAKE_ROOM, //neutral room
    SAI_PLAN_BUILD_ROOM,
    /* SAI_PLAN_STEAL_ROOM, */ //can't use right now since we cannot accurate judge effects of stealing rooms anyhow
    SAI_PLAN_HIDE, //try to delay the inevitable by avoiding digging outside dungeon
    SAI_PLAN_ASSAULT, //attack dungeon heart of opponent
    SAI_PLAN_ARMAGEDDON, //another kind of assault ;-)
    SAI_PLAN_CREATURE_PRIORITIZE,
};

enum SAI_CreaturePriority
{
    SAI_CP_TRAIN, //Default
    SAI_CP_SAVE_MONEY,
    SAI_CP_RESEARCH,
    SAI_CP_MANUFACTURE,
    SAI_CP_SCAVENGE
};

struct SAI_PlanDecision
{
    enum SAI_PlanDecisionType type;
    union
    {
        int kind;
        enum SAI_CreaturePriority cp;
    } param;
};


/**
 * Prepare for a new planning round by initializing the player's planner.
 * @param plyr Player to make a plan for.
 * @param type Type of plan desired.
 */
void SAI_begin_plan(int plyr, enum SAI_PlanType type);

/**
 *
 * @param plyr Player's plan to process.
 * @param node_budget Max nodes that may be processed this frame.
 */
void SAI_process_plan(int plyr, int node_budget);

/**
 * Stops processing, retrieves best plan, and puts used nodes into a reuse cache.
 * destroy can be called afterwards but that will flush cached nodes.
 * @param plyr Player to retrieve plan of.
 * @param decisions Pointer to pointer that will point to decision array.
 * @param num_decisions Number of decisions in plan.
 */
void SAI_end_plan(int plyr, struct SAI_PlanDecision ** decisions, int * num_decisions);

/**
 * Destroys a player plan. Can be called even if begin_plan wasn't called.
 * end_plan cannot be called after.
 * @param plyr Player whose plan to destroy.
 */
void SAI_destroy_plan(int plyr);


#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_PLANNER_H
