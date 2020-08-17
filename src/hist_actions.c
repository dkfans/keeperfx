/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file hist_action.c
 *     Accounting of game events for improved multiplayer
 * @par Purpose:
 *     Allows to sync present with events in past in multiplayer to compensate lag.
 * @par Comment:
 *     None.
 * @author   TheSim
 * @date     17 Aug 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */

#include "hist_actions.h"

#include "config_crtrstates.h"
#include "game_legacy.h"

#define TOTAL_ITEMS     128
#define TOTAL_THINGS    32
#define TIME_DEPTH      200
/* 
    This is a ringbuffer of actions that player on this client is performed.
    AND events that depend on it. 
    So we can divide timeline into stable and unstable parts
    `STABLE_PAST <= UNSTABLE_PAST <= NOW`
    
    STABLE_PAST contains all events that have been already synced between all players.
    UNSTABLE_PAST contains events that are not yet synced to some player.
    NOW is current _game.play_gameturn_
*/
static int next_free_record = 0;
static int used_records = 0;
struct HistActionRecord records[TOTAL_ITEMS] = {0};

struct ThingHistoryRecord
{
    int       idx;
    long      gameturn;
};

// creature lists
static int next_creature_record = 0;
static int total_creature_records = 0;
static struct ThingHistoryRecord creature_records[TOTAL_THINGS] = {0};

void hist_init()
{
    next_free_record = 0;
    used_records = 0;
    next_creature_record = 0;
    total_creature_records = 0;
}

static void creature_add_record(long gameturn, int creature_idx)
{
    int idx = next_creature_record++;
    if (next_creature_record >= TOTAL_THINGS)
        next_creature_record = 0;

    if (total_creature_records < TOTAL_THINGS)
        total_creature_records++;

    struct ThingHistoryRecord *rec = &creature_records[idx];
    rec->idx = creature_idx;
    rec->gameturn = gameturn;
}

void hist_take_task(PlayerNumber plyr_idx, int task_id, int creature_idx)
{
    if (plyr_idx != my_player_number)
        return;
    struct HistActionRecord *rec;

    int idx = next_free_record - 1;
    for (int i = 0; i < used_records; i++)
    {
        rec = &records[idx];
        idx--;
        if (idx < 0) idx += TOTAL_ITEMS;
        if (rec->task_id == task_id)
        {
            creature_add_record(rec->gameturn, creature_idx);
            break;
        }
    }

}

void hist_map_action(enum HistActionType type, PlayerNumber plyr_idx, MapSubtlCoord cx, MapSubtlCoord cy)
{
    if (plyr_idx != my_player_number)
        return;
    int prev_id = next_free_record - 1;
    if (prev_id < 0)
        prev_id += TOTAL_ITEMS;
    struct HistActionRecord *prev_rec = &records[prev_id];
    struct HistActionRecord *rec = &records[next_free_record++];
    int i, task_id;
    MapSubtlCoord x, y;
    if (used_records < TOTAL_ITEMS)
        used_records++;
    if (next_free_record >= TOTAL_ITEMS)
        next_free_record = 0;
    rec->type = type;
    rec->gameturn = game.play_gameturn;
    rec->stl_num = get_subtile_number(cx, cy);

    switch (type)
    {
    case HAT_Tag:
        x = STL_PER_SLB * (cx/STL_PER_SLB);
        y = STL_PER_SLB * (cy/STL_PER_SLB);
        i = get_subtile_number(x + 1,y + 1);
        rec->task_id = find_from_task_list(plyr_idx, i);
        rec->creature_idx = -1;
        break;
    case HAT_Untag:
        if (rec->task_id == -1)
        {
            
        }
        x = STL_PER_SLB * (cx/STL_PER_SLB);
        y = STL_PER_SLB * (cy/STL_PER_SLB);
        i = get_subtile_number(x + 1,y + 1);
        break;
    default:
        rec->task_id = -1;
        rec->creature_idx = -1;
        break;
    }
}

/*
    To make a visible list of events
*/
void hist_get_string(int order, char *str_left, char *str_right)
{
    int idx;
    /*
    // all action records
    idx = next_free_record - 1 - order;
    if (idx < 0) idx += TOTAL_ITEMS;
    if (order >= used_records)
    {
        strcpy(str_left, "");
        strcpy(str_right, "");
        return;
    }
    struct HistActionRecord *rec = &records[idx];
    switch(rec->type)
    {
    case HAT_Tag:
        sprintf(str_left, "tag %3ld, %3ld", 
            stl_num_decode_x(rec->stl_num), stl_num_decode_y(rec->stl_num));
        break;
    default:
        sprintf(str_left, "unknown");
    }
    sprintf(str_right, "%4ld", rec->gameturn - game.play_gameturn);
    */
    idx = next_creature_record - 1 - order;
    if (idx < 0) idx += TOTAL_ITEMS;
    if (order >= total_creature_records)
    {
        strcpy(str_left, "");
        strcpy(str_right, "");
        return;
    }
    struct ThingHistoryRecord *rec = &creature_records[idx];
    struct Thing *creatng = thing_get(rec->idx);
    if (game.play_gameturn - rec->gameturn > TIME_DEPTH)
    {
        strcpy(str_left, "");
        strcpy(str_right, "");
        return;
    }
    sprintf(str_left, "%3d %s %s", rec->idx, thing_model_name(creatng), creature_state_code_name(creatng->active_state));
    sprintf(str_right, "%4ld", rec->gameturn - game.play_gameturn);
}
