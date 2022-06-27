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
#include "thing_navigate.h"
#include "creature_control.h"
#include "config_creature.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "front_simple.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long calculate_free_lair_space(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    long cap_used = 0;
    long cap_total = 0;
    unsigned long k = 0;
    struct DungeonAdd* dungeonadd = get_dungeonadd_by_dungeon(dungeon);

    long i = dungeonadd->room_kind[RoK_LAIR];
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

static short get_lair_score(TbBool room_has_units_of_same_kind,TbBool room_has_units_of_different_kind,TbBool room_has_lair_enemy)
{
    if ( room_has_units_of_same_kind )
    {
        if ( room_has_units_of_different_kind )
        {
            if ( room_has_lair_enemy )
            {
                return 2;
            }
            else
            {
                return 5;
            }
        }
        else
        {
            return 6;
        }
    }
    else if ( room_has_units_of_different_kind )
    {
        if ( room_has_lair_enemy )
        {
            return 1;
        }
        else
        {
            return 3;
        }
    }
    else
    {
        return 4;
    }
}

struct Room *get_best_new_lair_for_creature(struct Thing *creatng)
{
    struct Room *room;
    char best_score = 0;

    const struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    struct DungeonAdd* dungeonadd = get_dungeonadd(creatng->owner);

    short *room_scores = (short *)scratch;
    memset(scratch, 0, ROOMS_COUNT);


    for (RoomKind rkind = 0; rkind < slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,RoRoF_LairStorage))
        {
            room = room_get(dungeonadd->room_kind[rkind]);
            while (!room_is_invalid(room))
            {
                if ( room_has_enough_free_capacity_for_creature_job(room, creatng, Job_TAKE_SLEEP) && creature_can_head_for_room(creatng, room, 0) )
                {
                    TbBool room_has_units_of_same_kind = false;
                    TbBool room_has_units_of_different_kind = false;
                    TbBool room_has_lair_enemy = false;
                    for ( ThingModel model = 0; model < CREATURE_TYPES_COUNT; ++model )
                    {
                        if ( room_has_units_of_same_kind && room_has_units_of_different_kind && room_has_lair_enemy )
                            break;
                        if ( room->content_per_model[model] > 0) 
                        {
                            if ( creatng->model == model )
                            {
                                room_has_units_of_same_kind = true;
                            }
                            else
                            {
                                room_has_units_of_different_kind = true;
                                if ( crstat->lair_enemy == model )
                                room_has_lair_enemy = true;
                            }
                        }
                    }
                    char lair_score = get_lair_score(room_has_units_of_same_kind,room_has_units_of_different_kind,room_has_lair_enemy);
                    room_scores[room->index] = lair_score;
                    if (best_score < lair_score)
                        best_score = lair_score;
                }
                room = room_get(room->next_of_owner);
            }
        }
    }
        
    if (best_score == 0)
    {
        return INVALID_ROOM;
    }

    struct Room *nearest_room = INVALID_ROOM;
    MapCoordDelta distance;
    MapCoordDelta min_distance = INT_MAX;
    struct Coord3d room_center_pos;

    for (RoomKind rkind = 0; rkind < slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,RoRoF_LairStorage))
        {
            room = room_get(dungeonadd->room_kind[rkind]);
            while (!room_is_invalid(room))
            {
                if ( room_scores[room->index] == best_score )
                {
                    room_center_pos.x.stl.pos = room->central_stl_x;
                    room_center_pos.y.stl.pos = room->central_stl_y;
                    room_center_pos.z.val = 256;
                    distance = get_2d_box_distance(&creatng->mappos, &room_center_pos);

                    if ( min_distance > distance )
                    {
                        min_distance = distance;
                        nearest_room = room;
                    }
                }
                room = room_get(room->next_of_owner);
            }
        }
    }
    return nearest_room;
}
/******************************************************************************/
