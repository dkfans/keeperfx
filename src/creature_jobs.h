/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_jobs.h
 *     Header file for creature_jobs.c.
 * @par Purpose:
 *     Creature job assign and verify functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     27 Aug 2013 - 07 Oct 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRJOBS_H
#define DK_CRTRJOBS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Flags for selecting creature_can_do_job_near_position() behavior.
 */
enum CreatureJobCheckFlagValues {
    JobChk_None = 0x0000,          //**< Do not make any changes
    JobChk_SetStateOnFail = 0x0001,//**< If check fails, update the creature state accordingly
    JobChk_PlayMsgOnFail = 0x0002, //**< If check fails, prepare corresponding message for the player
};

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;
struct Dungeon;

#pragma pack()
/******************************************************************************/
TbBool set_creature_assigned_job(struct Thing *thing, CreatureJob new_job);
TbBool creature_has_job(const struct Thing *thing, CreatureJob job_kind);
TbBool creature_free_for_anger_job(struct Thing *thing);
TbBool creature_find_and_perform_anger_job(struct Thing *thing);
TbBool attempt_job_preference(struct Thing *creatng, long jobpref);
TbBool creature_try_doing_secondary_job(struct Thing *creatng);

// Assigning jobs by making creature go to the workplace, or dropping the creature at it
TbBool creature_can_do_job_for_player(const struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job, unsigned long flags);
TbBool creature_can_do_job_near_position(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job, unsigned long flags);
TbBool send_creature_to_job_for_player(struct Thing *creatng, PlayerNumber plyr_idx, CreatureJob new_job);
TbBool send_creature_to_job_near_position(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CreatureJob new_job);

// Assigning jobs by computer player
TbBool creature_can_do_job_for_computer_player_in_room_role(const struct Thing *creatng, PlayerNumber plyr_idx, RoomRole rrole);
TbBool get_drop_position_for_creature_job_in_dungeon(struct Coord3d *pos, const struct Dungeon *dungeon, struct Thing *creatng, CreatureJob new_job, unsigned long drop_kind_flags);
TbBool get_drop_position_for_creature_job_in_room(struct Coord3d *pos, const struct Room *room, CreatureJob jobpref, struct Thing *creatng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
