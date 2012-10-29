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
#include "skirmish_ai_map.h"
#include "skirmish_ai_planner.h"
#include "skirmish_ai_rooms.h"
#include "skirmish_ai_values.h"

#include "bflib_basics.h"
#include "config_terrain.h"
#include "globals.h"
#include "room_data.h"
#include "slab_data.h"

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>


#define MAX_ROOM_COUNT          100
#define MAX_DIG                 200

#define PLAN_REFRESH_INTERVAL   250
#define PLAN_FRAME_BUDGET       10000
#define PLAN_INIT_BUDGET        1000000 //when initializing AI


enum SAI_ActivityTypeAndPriority
{
    ACTIVITY_MANAGE_CREATURES,
    ACTIVITY_DIG_AREAS,
    ACTIVITY_MODIFY_ROOMS,
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

struct SAI_PlayerState
{
    //stuff that connects with game
    int index;
    //player info
    //dungeon

    //ai settings
    enum SAI_PlanType plan_type;
    enum SAI_Skill skill;

    //ai state
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
    struct SAI_PlayerState players[SAI_MAX_KEEPERS];
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

static void owned_dug_tile_counter(int x, int y, struct SlabMap * slab, int * count)
{
    switch (slab->kind) {
    case SlbT_ROCK:
    case SlbT_GOLD:
    case SlbT_EARTH:
    case SlbT_TORCHDIRT:
    case SlbT_WALLDRAPE:
    case SlbT_WALLTORCH:
    case SlbT_WALLWTWINS:
    case SlbT_WALLWWOMAN:
    case SlbT_WALLPAIRSHR:
    case SlbT_GEMS:
    case SlbT_PATH:
        break;
    default:
        *count += 1;
    }
}

static void think_about_digging_for_rooms(struct SAI_PlayerState * plyrstate)
{
    int x, y;
    int w, h;
    int i;
    SAI_Action action;
    struct SAI_Room * room;
    struct SAI_Point * dig_coords;
    int count;
    int required;
    int available;
    int num_tiles_done;

    AIDBG(4, "Starting");

    //dig new rooms
    for (room = SAI_get_rooms_of_state(plyrstate->index, SAI_ROOM_UNDUG);
            room != NULL; room = room->next_of_state) {
        if (room->kind == RoK_NONE) {
            continue;
        }

        count = SAI_find_path_to_room(plyrstate->index, room->id, &dig_coords);
        w = SAI_rect_width(room->rect);
        h = SAI_rect_height(room->rect);

        required = count + w * h;
        available = actions_left(plyrstate, ACTIVITY_DIG_AREAS);
        if (required <= available) {
            AIDBG(6, "Deciding to dig using %i actions", required);

            for (i = 0; i < count; ++i) {
                action = SAI_mark_dig_action(plyrstate->index,
                    dig_coords[i].x, dig_coords[i].y);
                queue_action(plyrstate, ACTIVITY_DIG_AREAS, action);
            }

            for (y = room->rect.t; y <= room->rect.b; ++y) {
                for (x = room->rect.l; x <= room->rect.r; ++x) {
                    action = SAI_mark_dig_action(plyrstate->index, x, y);
                    queue_action(plyrstate, ACTIVITY_DIG_AREAS, action);
                }
            }

            SAI_set_room_state(plyrstate->index, room->id, SAI_ROOM_BEING_DUG);
        }
        else {
            AIDBG(6, "Deciding to wait with dig: %i actions required, actions available: %i",
                required, available);
        }

        free(dig_coords);
    }

    //check for finished digs
    for (room = SAI_get_rooms_of_state(plyrstate->index, SAI_ROOM_BEING_DUG);
            room != NULL; room = room->next_of_state) {
        w = SAI_rect_width(room->rect);
        h = SAI_rect_height(room->rect);
        num_tiles_done = 0;
        SAI_tiles_in_rect(room->rect,
            (SAI_TileFunc) owned_dug_tile_counter, &num_tiles_done);

        if (w * h == num_tiles_done) {
            SAI_set_room_state(plyrstate->index, room->id, SAI_ROOM_EMPTY);
        }
    }
}

static void think_about_modifying_rooms(struct SAI_PlayerState * plyrstate)
{
    int x, y;
    int w, h;
    int real_width;
    int real_height;
    int count;
    SAI_Action action;
    struct SAI_Room * room;

    for (room = SAI_get_rooms_of_state(plyrstate->index, SAI_ROOM_EMPTY);
            room != NULL; room = room->next_of_state) {
        if (room->num_tiles_in_use <= actions_left(plyrstate, ACTIVITY_MODIFY_ROOMS)) {
            count = 0;
            real_width = SAI_rect_width(room->rect);
            real_height = SAI_rect_height(room->rect);
            w = h = (int) sqrt(room->num_tiles_in_use);
            w = min(w, real_width);
            h = min(h, real_height);
            if (w * h < room->num_tiles_in_use && w < real_width) {
                w += 1;
            }
            if (w * h < room->num_tiles_in_use) {
                h += 1;
            }

            assert(w * h >= room->num_tiles_in_use); //in case I'm mistaken...

            for (y = room->rect.t; y < room->rect.t + h && count < room->num_tiles_in_use; ++y) {
                for (x = room->rect.l; x < room->rect.l + w && count < room->num_tiles_in_use; ++x) {
                    action = SAI_build_room_action(plyrstate->index,
                        x, y, room->kind);
                    queue_action(plyrstate, ACTIVITY_MODIFY_ROOMS, action);

                    count += 1;
                }
            }

            SAI_set_room_state(plyrstate->index, room->id, SAI_ROOM_BEING_BUILT);
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

/**
 * Processes an AI plan decision (part of plan).
 * @param plyrstate
 * @param item Current plan item.
 * @param parent "Parent" of current item, i.e. earlier plan item.
 * @return Non-zero if the item could be processed.
 */
static int try_process_plan_item(struct SAI_PlayerState * plyrstate,
    struct SAI_PlanDecision * item, struct SAI_PlanDecision * parent)
{
    enum RoomKinds room_kind;
    int id;
    int tiles;

    AIDBG(4, "Starting for plan item of type %i", (int) item->type);

    switch (item->type) {
    case SAI_PLAN_WAIT:
        return 1;
    case SAI_PLAN_BUILD_ROOM:
        room_kind = (enum RoomKinds) item->param.kind;
        if (!is_room_available(plyrstate->index, room_kind)) {
            //goto next item if parent node was auxiliary wait
            return parent == NULL || parent->type != SAI_PLAN_WAIT;
        }

        tiles = SAI_tiles_for_next_room_of_kind(plyrstate->index, room_kind);
        //TODO AI: more sophisticated room sizing scheme?
        id = SAI_request_room(plyrstate->index, room_kind, tiles);
        if (id >= 0) {
            return 1;
        }
        break;
    default:
        break;
    }

    return 0;
}

/**
 * Processes the next plan (and swaps it if finished);
 *  then processes the current plan item in queue.
 * @param plyrstate
 */
static void process_plan(struct SAI_PlayerState * plyrstate)
{
    struct SAI_PlanDecision * parent;

    AIDBG(3, "Starting");

    if (plyrstate->turn == 0) {
        SAI_begin_plan(plyrstate->index, plyrstate->plan_type);
        SAI_process_plan(plyrstate->index, PLAN_INIT_BUDGET);
    }

    if (plyrstate->turn % PLAN_REFRESH_INTERVAL == 0) { //no else if - logic interleaves with previous if
        free(plyrstate->plan);
        plyrstate->plan_index = 0;
        SAI_end_plan(plyrstate->index, &plyrstate->plan, &plyrstate->plan_size);
        SAI_begin_plan(plyrstate->index, plyrstate->plan_type);
    }
    else {
        SAI_process_plan(plyrstate->index, PLAN_FRAME_BUDGET);
    }

    if (plyrstate->plan_index < plyrstate->plan_size) {
        parent = plyrstate->plan_index <= 0? NULL :
            plyrstate->plan + plyrstate->plan_index - 1;
        if (try_process_plan_item(plyrstate, plyrstate->plan + plyrstate->plan_index,
                parent)) {
            plyrstate->plan_index += 1;
        }
    }
}

/**
 * Performs checks on what more actions it is appropriate to en-queue.
 * @param plyrstate
 */
static void think(struct SAI_PlayerState * plyrstate)
{
    process_plan(plyrstate);
    think_about_battles(plyrstate);
    think_about_conquest(plyrstate);
    think_about_digging_for_rooms(plyrstate);
    think_about_modifying_rooms(plyrstate);
    think_about_dungeon_specials(plyrstate);
    think_about_economy(plyrstate);
    think_about_enemy_creatures(plyrstate);
    think_about_own_creatures(plyrstate);
    think_about_sacrifice(plyrstate);
    think_about_workshop_items(plyrstate);
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

    AIDBG(4, "Switching to activity %i", (int) activity);

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

    AIDBG(3, "Starting");

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

        if (!plyrstate->activities[i].action_list) {
            continue; //no point switching to empty activity
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

    AIDBG(3, "Starting for activity of type %i", (int) plyrstate->curr_activity);

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

static struct SAI_PlayerState * player_state(int plyr)
{
    assert(plyr >= 0);
    assert(plyr < SAI_MAX_KEEPERS);
    return &state.players[plyr];
}

void SAI_init_for_map(void)
{
    SAI_init_map_analysis();
    SAI_run_map_analysis(); //required for room layout
}

void SAI_destroy_for_player(int plyr)
{
    struct SAI_PlayerState * plyrstate;
    SAI_Action action;
    int i;

    SYNCDBG(3, "Starting");
    plyrstate = player_state(plyr);

    for (i = 0; i < ACTIVITY_COUNT; ++i) {
        while (plyrstate->activities[i].action_list) {
            action = plyrstate->activities[i].action_list;
            plyrstate->activities[i].action_list = SAI_get_next_action(action);
            SAI_destroy_action(action);
        }
    }

    free(plyrstate->plan);
    SAI_destroy_plan(plyr);

    LbMemorySet(plyrstate, 0, sizeof(*plyrstate));
}

void SAI_init_for_player(int plyr)
{
    struct SAI_PlayerState * plyrstate;

    SAI_destroy_for_player(plyr);

    SYNCDBG(3, "Starting");
    plyrstate = player_state(plyr);
    plyrstate->index = plyr;

    SAI_init_room_layout_for_player(plyr);
}

void SAI_set_player_skill(int plyr, enum SAI_Skill skill)
{
    struct SAI_PlayerState * plyrstate;

    SYNCDBG(4, "Starting");
    plyrstate = player_state(plyr);
    plyrstate->skill = skill;
}

void SAI_set_player_plan_strategy(int plyr, enum SAI_PlanType strat)
{
    struct SAI_PlayerState * plyrstate;

    SYNCDBG(4, "Starting");
    plyrstate = player_state(plyr);
    plyrstate->plan_type = strat;
}

void SAI_run_shared(void)
{
    SAI_run_map_analysis();
}

void SAI_run_for_player(int plyr)
{
    struct SAI_PlayerState * plyrstate;

    SYNCDBG(3, "Starting");
    plyrstate = player_state(plyr);

    choose_next_activity(plyrstate);
    think(plyrstate);
    run_activity(plyrstate);

    plyrstate->turn += 1;
}

