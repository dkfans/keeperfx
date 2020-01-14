/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_lair.c
 *     Lair room and creature lairs maintain functions.
 * @par Purpose:
 *     Functions to create and use lairs.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 20 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_lair.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "creature_control.h"
#include "config_creature.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct Room *_DK_get_best_new_lair_for_creature(struct Thing *creatng);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long calculate_free_lair_space(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    //return _DK_calculate_free_lair_space(dungeon);
    long cap_used = 0;
    long cap_total = 0;
    unsigned long k = 0;
    long i = dungeon->room_kind[RoK_LAIR];
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        cap_total += room->total_capacity;
        cap_used += room->used_capacity;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    long cap_required = 0;
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->lair_room_id == 0)
        {
            struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
            cap_required += crstat->lair_size;
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(9,"Total lair capacity %d, used %d, will need %d more",(int)cap_total,(int)cap_used,(int)cap_required);
    return cap_total - cap_used - cap_required;
}

struct Room *get_best_new_lair_for_creature(struct Thing *thing)
{
    return _DK_get_best_new_lair_for_creature(thing);
}
/******************************************************************************/
