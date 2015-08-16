/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_jobs.h
 *     Header file for room_jobs.c.
 * @par Purpose:
 *     List of things in room maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ROOM_JOBS_H
#define DK_ROOM_JOBS_H

#include "globals.h"
#include "bflib_basics.h"

#include "thing_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;
struct Dungeon;

#pragma pack()
/******************************************************************************/
struct Room *get_room_creature_works_in(const struct Thing *thing);
TbBool creature_is_working_in_room(const struct Thing *creatng, const struct Room *room);

TbBool add_creature_to_work_room(struct Thing *creatng, struct Room *room, CreatureJob jobpref);
TbBool add_creature_to_torture_room(struct Thing *creatng, const struct Room *room);
TbBool remove_creature_from_specific_room(struct Thing *creatng, struct Room *room, CreatureJob jobpref);
TbBool remove_creature_from_work_room(struct Thing *thing);
TbBool remove_creature_from_torture_room(struct Thing *creatng);

// Sending to rooms and moving within rooms by using CrSt_MoveToPosition substate
TbBool creature_setup_adjacent_move_for_job_within_room_f(struct Thing *creatng, struct Room *room, CreatureJob jobpref, const char *func_name);
#define creature_setup_adjacent_move_for_job_within_room(creatng, room, jobpref) creature_setup_adjacent_move_for_job_within_room_f(creatng, room, jobpref, __func__)
TbBool creature_setup_random_move_for_job_in_room_f(struct Thing *creatng, struct Room *room, CreatureJob jobpref, NaviRouteFlags nav_flags, const char *func_name);
#define creature_setup_random_move_for_job_in_room(creatng, room, jobpref, nav_flags) creature_setup_random_move_for_job_in_room_f(creatng, room, jobpref, nav_flags, __func__)

// Sending to rooms and moving within rooms - other methods
short send_creature_to_room(struct Thing *creatng, struct Room *room, CreatureJob jobpref);
TbBool setup_random_head_for_room(struct Thing *thing, struct Room *room, unsigned char flags);

struct Thing *find_object_in_room_for_creature_matching_bool_filter(struct Thing *creatng, const struct Room *room, Thing_Bool_Filter matcher_cb);

int worker_needed_in_dungeons_room_role(const struct Dungeon *dungeon, RoomRole rrole);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
