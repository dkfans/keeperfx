/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_mood.h
 *     Header file for creature_states_mood.c.
 * @par Purpose:
 *     Creature state machine functions related to their mood.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRSTATEMOOD_H
#define DK_CRTRSTATEMOOD_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;

#pragma pack()
/******************************************************************************/
TbBool creature_can_get_angry(const struct Thing *creatng);
TbBool anger_is_creature_livid(const struct Thing *thing);
TbBool anger_is_creature_angry(const struct Thing *thing);
TbBool anger_free_for_anger_increase(struct Thing *creatng);
TbBool anger_free_for_anger_decrease(struct Thing *creatng);
void anger_increase_creature_anger_f(struct Thing *creatng, long anger, AnnoyMotive reason, const char *func_name);
#define anger_increase_creature_anger(creatng, anger, reason) anger_increase_creature_anger_f(creatng, anger, reason, __func__)
void anger_reduce_creature_anger_f(struct Thing *creatng, long anger, AnnoyMotive reason, const char *func_name);
#define anger_reduce_creature_anger(creatng, anger, reason) anger_reduce_creature_anger_f(creatng, anger, reason, __func__)
AnnoyMotive anger_get_creature_anger_type(const struct Thing *creatng);
void anger_set_creature_anger_f(struct Thing *creatng, long annoy_lv, AnnoyMotive reason, const char *func_name);
#define anger_set_creature_anger(creatng, annoy_lv, reason) anger_set_creature_anger_f(creatng, annoy_lv, reason, __func__)
void anger_apply_anger_to_creature_all_types_f(struct Thing *thing, long anger, const char *func_name);
#define anger_apply_anger_to_creature_all_types(thing, anger) anger_apply_anger_to_creature_all_types_f(thing, anger, __func__)
TbBool anger_make_creature_angry(struct Thing *thing, AnnoyMotive reason);
TbBool creature_mark_if_woken_up(struct Thing *creatng);
TbBool process_job_stress_and_going_postal(struct Thing *creatng);
TbBool any_worker_will_go_postal_on_creature_in_room(const struct Room *room, const struct Thing *victng);

short creature_moan(struct Thing *thing);
short creature_roar(struct Thing *thing);
short creature_be_happy(struct Thing *thing);
short creature_piss(struct Thing *thing);
short mad_killing_psycho(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
