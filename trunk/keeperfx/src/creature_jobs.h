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

/******************************************************************************/
#pragma pack(1)

struct Thing;

#pragma pack()
/******************************************************************************/
/******************************************************************************/
TbBool set_creature_assigned_job(struct Thing *thing, CreatureJob new_job);
TbBool creature_free_for_anger_job(struct Thing *thing);
long creature_find_and_perform_anger_job(struct Thing *thing);
long attempt_job_preference(struct Thing *creatng, long jobpref);
TbBool creature_try_doing_secondary_job(struct Thing *creatng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
