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
};

struct SAI_PlanDecision
{
    enum SAI_PlanDecisionType type;
    union
    {
        int kind;
    } param;
};


/**
 *
 * @param plyr Player to make a plan for.
 * @param next_decision
 */
void SAI_make_plan(int plyr, struct SAI_PlanDecision ** decisions, int * num_decisions);

#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_PLANNER_H
