/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_hero.h
 *     Header file for creature_states_hero.c.
 * @par Purpose:
 *     Creature state machine functions for heroes.
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
#ifndef DK_CRTRSTATEHERO_H
#define DK_CRTRSTATEHERO_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

enum CreatureHeroTasks {
    CHeroTsk_Default         = 0,
    CHeroTsk_AttackRooms     = 1,
    CHeroTsk_AttackEnemies   = 2,
    CHeroTsk_AttackDnHeart   = 3,
    CHeroTsk_StealGold       = 4,
    CHeroTsk_StealSpells     = 5,
    CHeroTsk_DefendParty     = 6,
};

struct Thing;

#pragma pack()
/******************************************************************************/
short good_attack_room(struct Thing *thing);
short good_arrived_at_attack_room(struct Thing *thing);
short good_back_at_start(struct Thing *thing);
short good_doing_nothing(struct Thing *thing);
short good_drops_gold(struct Thing *thing);
short good_leave_through_exit_door(struct Thing *thing);
short good_returns_to_start(struct Thing *thing);
short good_wait_in_exit_door(struct Thing *thing);
short creature_hero_entering(struct Thing *thing);
short tunneller_doing_nothing(struct Thing *creatng);
short tunnelling(struct Thing *creatng);

TbBool good_setup_wander_to_exit(struct Thing *creatng);
short setup_person_tunnel_to_position(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char a4);
TbBool send_tunneller_to_point_in_dungeon(struct Thing *creatng, PlayerNumber plyr_idx, struct Coord3d *pos);
TbBool is_hero_tunnelling_to_attack(struct Thing *creatng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
