/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai.c
 *     Computer player definitions and activities.
 * @par Purpose:
 *     Experimental computer player intended to play multiplayer maps better.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "skirmish_ai.h"

#include "skirmish_ai_actions.h"
#include "skirmish_ai_planner.h"

#include "bflib_basics.h"
#include "globals.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>


#define MAX_KEEPERS             4
#define MAX_ROOM_COUNT          100
#define MAX_DIG                 200

#define PLAN_REFRESH_INTERVAL   250
#define PLAN_FRAME_BUDGET       10000
#define PLAN_INIT_BUDGET        1000000 //when initializing AI

#define ROOMFLAG_USED           0x1
#define ROOMFLAG_BEING_DUG      0x2
#define ROOMFLAG_BEING_BUILT    0x4


enum SAI_ActivityTypeAndPriority
{
    ACTIVITY_MANAGE_CREATURES,
    ACTIVITY_DIG_AREAS,
    ACTIVITY_BUILD_ROOMS,
    ACTIVITY_MANAGE_WORKSHOP_ITEMS,
    ACTIVITY_MAGIC_ATTACK,
    ACTIVITY_MANAGE_DUNGEON_SPECIALS,
    ACTIVITY_ASSIST_IMPS,
    ACTIVITY_MANAGE_BATTLES,
    ACTIVITY_START_BATTLE,

    ACTIVITY_COUNT //counter
};

struct SAI_Activity
{
    SAI_Action curr_action;
    SAI_Action action_list;
    int action_count;
    int turn_started; //when was the last turn this activity begun?
    int turn_ended; //when was the last turn this activity was ended?
};

struct SAI_ActivityInfo
{
    int max_actions;
    int min_length; //when must we change to higher priority activity that wants processing?
    int max_length; //when must we change to lower priority activity that wants processing?
};

struct SAI_Room
{
    unsigned char id;
    unsigned char kind;
    unsigned short flags;
    unsigned char x;
    unsigned char y;
    unsigned char w;
    unsigned char h;
};

struct SAI_PlayerState
{
    //stuff that connects with game
    int index;
    //player info
    //dungeon

    //ai stuff
    int turn;
    struct SAI_Activity activities[ACTIVITY_COUNT];
    enum SAI_ActivityTypeAndPriority curr_activity;
    struct SAI_Room rooms[MAX_ROOM_COUNT];
    int plan_size;
    int plan_index;
    struct SAI_PlanDecision * plan;
};

struct SAI_State
{
    struct SAI_PlayerState players[MAX_KEEPERS];
};


struct DigParams
{
    int count;

    struct
    {
        int x;
        int y;
    } pos[MAX_DIG];
};

static const SAI_ActivityInfo activity_info[ACTIVITY_COUNT] =
{
    { 20,       10, 50 },
    { MAX_DIG,  10, 50 },
    { 100,      10, 50 },
    { 20,       10, 50 },
    { 1,        10, 50 },
    { 3,        10, 50 },
    { 20,       10, 50 },
    { 256,      10, 50 }
};


static struct SAI_State state;


static int actions_left(struct SAI_PlayerState * plyrstate,
    enum SAI_ActivityTypeAndPriority activity)
{
    return activity_info[activity].max_actions - plyrstate->activities[activity].action_count;
}

static int queue_action(struct SAI_PlayerState * plyrstate,
    enum SAI_ActivityTypeAndPriority activity, SAI_Action action)
{
    struct SAI_Activity * act;

    if (actions_left(plyrstate, activity) <= 0) {
        SAI_destroy_action(action);
        return 0;
    }

    act = &plyrstate->activities[activity];
    SAI_append_action(&act->action_list, action);
    act->action_count += 1;

    return 1;
}

static void think_about_battles(struct SAI_PlayerState * plyrstate)
{

}

static void think_about_conquest(struct SAI_PlayerState * plyrstate)
{

}

static int wants_room(struct SAI_PlayerState * plyrstate)
{
    return 0;
}

static int should_dig_for_room(struct SAI_PlayerState * plyrstate,
    struct DigParams * dig_params, struct SAI_Room ** room)
{
    return 0;
}

static int should_build_room(struct SAI_PlayerState * plyrstate,
    struct SAI_Room ** room_params)
{
    return 0;
}

static void think_about_dungeon_development(struct SAI_PlayerState * plyrstate)
{
    int i, j;
    SAI_Action action;
    struct DigParams dig_params;
    struct SAI_Room * room;

    if (should_dig_for_room(plyrstate, &dig_params, &room)) {
        if (dig_params.count <= actions_left(plyrstate, ACTIVITY_DIG_AREAS)) {
            for (i = 0; i < dig_params.count; ++i) {
                action = SAI_mark_dig_action(plyrstate->index,
                    dig_params.pos[i].x, dig_params.pos[i].y);
                queue_action(plyrstate, ACTIVITY_DIG_AREAS, action);
            }

            room->flags |= ROOMFLAG_BEING_DUG;
        }
    }

    if (should_build_room(plyrstate, &room)) {
        if (room->w * room->h <= actions_left(plyrstate, ACTIVITY_BUILD_ROOMS)) {
            for (i = 0; i < room->w; ++i) {
                for (j = 0; j < room->h; ++j) {
                    action = SAI_build_room_action(plyrstate->index,
                        room->x + i, room->y + j, room->kind);
                    queue_action(plyrstate, ACTIVITY_BUILD_ROOMS, action);
                }
            }

            room->flags |= ROOMFLAG_BEING_BUILT;
        }
    }
}

static void think_about_dungeon_specials(struct SAI_PlayerState * plyrstate)
{

}

static void think_about_economy(struct SAI_PlayerState * plyrstate)
{

}

static void think_about_enemy_creatures(struct SAI_PlayerState * plyrstate)
{

}

static void think_about_own_creatures(struct SAI_PlayerState * plyrstate)
{

}

static void think_about_sacrifice(struct SAI_PlayerState * plyrstate)
{

}

static void think_about_workshop_items(struct SAI_PlayerState * plyrstate)
{

}

static void process_plan(struct SAI_PlayerState * plyrstate)
{
    AIDBG(3, "Starting");

    if (plyrstate->turn == 0) {
        SAI_begin_plan(plyrstate->index, SAI_PLAN_LEAST_RISKY);
        SAI_process_plan(plyrstate->index, PLAN_INIT_BUDGET);
    }

    if (plyrstate->turn % PLAN_REFRESH_INTERVAL == 0) { //no else if - logic interleaves with previous if
        free(plyrstate->plan);
        plyrstate->plan_index = 0;
        SAI_end_plan(plyrstate->index, &plyrstate->plan, &plyrstate->plan_size);
        SAI_begin_plan(plyrstate->index, SAI_PLAN_LEAST_RISKY);
    }
    else {
        SAI_process_plan(plyrstate->index, PLAN_FRAME_BUDGET);
    }
}

/**
 * Performs checks on what more actions it is appropriate to en-queue.
 * @param plyrstate
 */
static void think(struct SAI_PlayerState * plyrstate)
{
    process_plan(plyrstate);
    /*think_about_battles(plyrstate);
    think_about_conquest(plyrstate);
    think_about_dungeon_development(plyrstate);
    think_about_dungeon_specials(plyrstate);
    think_about_economy(plyrstate);
    think_about_enemy_creatures(plyrstate);
    think_about_own_creatures(plyrstate);
    think_about_sacrifice(plyrstate);
    think_about_workshop_items(plyrstate);*/
}


/**
 * Changes from one activity to another.
 * @param plyrstate
 * @param activity
 */
static void switch_activity(struct SAI_PlayerState * plyrstate,
    enum SAI_ActivityTypeAndPriority activity)
{
    struct SAI_Activity * act;

    //stop current
    act = &plyrstate->activities[plyrstate->curr_activity];
    act->turn_ended = plyrstate->turn;

    //start new
    plyrstate->curr_activity = activity;
    act = &plyrstate->activities[plyrstate->curr_activity];
    act->turn_started = plyrstate->turn;
}

/**
 * Higher "score" <=> preference for activity to be switch to.
 * Score is additive, where higher values => better chance of getting switched.
 * @param activity
 * @param plyrstate
 * @return
 */
static int schedule_score(enum SAI_ActivityTypeAndPriority activity,
    struct SAI_PlayerState * plyrstate)
{
    struct SAI_Activity * act;

    act = &plyrstate->activities[activity];
    return 10 * (int) activity - act->turn_ended;
}

/**
 * Looks up what next activity is due to be scheduled, and switches to that.
 * If the next activity to be scheduled is undetermined or same as current, no change is made.
 * @param plyrstate
 */
static void choose_next_activity(struct SAI_PlayerState * plyrstate)
{
    int time, i, lowest_switchable, best;
    struct SAI_Activity * act;
    const struct SAI_ActivityInfo * act_info;

    act = &plyrstate->activities[plyrstate->curr_activity];
    if (act->curr_action) {
        return; //must finish
    }

    act_info = &activity_info[plyrstate->curr_activity];
    time = plyrstate->turn - act->turn_started;
    if (act->action_list && time <= act_info->min_length) {
        return;
    }

    lowest_switchable = act->action_list && time <= act_info->max_length?
        plyrstate->curr_activity + 1 : 0;
    best = -1;
    for (i = ACTIVITY_COUNT - 1; i >= lowest_switchable; --i) {
        if (i == plyrstate->curr_activity) {
            continue; //shouldn't switch to self
        }

        if (best < 0 || schedule_score((enum SAI_ActivityTypeAndPriority) best, plyrstate) <
                schedule_score((enum SAI_ActivityTypeAndPriority) i, plyrstate)) {
            best = i;
        }
    }

    if (best >= 0) {
        switch_activity(plyrstate, (enum SAI_ActivityTypeAndPriority) best);
    }
}

/**
 * Runs the current activity.
 * @param plyrstate
 */
static void run_activity(struct SAI_PlayerState * plyrstate)
{
    struct SAI_Activity * act;

    act = &plyrstate->activities[plyrstate->curr_activity];
    if (!act->curr_action && act->action_list) {
        act->curr_action = act->action_list;
        act->action_list = SAI_get_next_action(act->action_list);
    }

    if (act->curr_action) {
        SAI_process_action(act->curr_action, plyrstate->index);

        if (SAI_is_action_done(act->curr_action)) {
            SAI_destroy_action(act->curr_action);
            act->curr_action = NULL;
            act->action_count -= 1;
        }
    }
}

void SAI_reset_for_player(int plyr)
{
    struct SAI_PlayerState * plyrstate;
    SAI_Action action;
    int i;

    SYNCDBG(3, "Starting");

    assert(plyr >= 0);
    assert(plyr < MAX_KEEPERS);
    plyrstate = &state.players[plyr];

    for (i = 0; i < ACTIVITY_COUNT; ++i) {
        while (plyrstate->activities[i].action_list) {
            action = plyrstate->activities[i].action_list;
            plyrstate->activities[i].action_list = SAI_get_next_action(action);
            SAI_destroy_action(action);
        }
    }

    free(plyrstate->plan);
    SAI_destroy_plan(plyr);

    memset(plyrstate, 0, sizeof(*plyrstate));

    plyrstate->index = plyr;
}

void SAI_run_for_player(int plyr)
{
    struct SAI_PlayerState * plyrstate;

    SYNCDBG(3, "Starting");

    assert(plyr >= 0);
    assert(plyr < MAX_KEEPERS);
    plyrstate = &state.players[plyr];

    choose_next_activity(plyrstate);
    think(plyrstate);
    run_activity(plyrstate);

    plyrstate->turn += 1;
}

