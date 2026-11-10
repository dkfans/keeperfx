/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file local_hand.h
 *     Header file for local_hand.c.
 * @par Purpose:
 *     Client-side prediction of the local user's Hand of Evil.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     10 Nov 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LOCAL_HAND_H
#define DK_LOCAL_HAND_H

#include "globals.h"
#include "bflib_basics.h"
#include "player_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct Thing;
struct UserState;

enum LocalHandAction {
    LocalHandAction_PickUp,
    LocalHandAction_Count,
};

/******************************************************************************/
void reset_local_hand(void);
void update_local_hand(void);
void apply_local_hand_prediction(struct UserState *ustate);
TbBool local_hand_takes_over_action(NetUserId user, enum LocalHandAction action);
struct Thing *get_hand_view(NetUserId user);
struct Thing *get_held_thing_view(NetUserId user);
TbBool local_hand_holds_thing(const struct Thing *thing);
TbBool local_hand_hides_thing(const struct Thing *thing);
void apply_local_hand_to_activity_counts(ThingModel crmodel, int32_t *gui_job_counts);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
