/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_shots.h
 *     Header file for thing_shots.c.
 * @par Purpose:
 *     Shots support functions.
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
#ifndef DK_THING_SHOTS_H
#define DK_THING_SHOTS_H

#include "bflib_basics.h"
#include "globals.h"
#include "room_workshop.h"
#include "thing_list.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum ShotModels {
    ShM_Null = 0,
    ShM_Fireball,
    ShM_Firebomb,
    ShM_Freeze,
    ShM_Lightning,
    ShM_PoisonCloud, // 5
    ShM_NaviMissile,
    ShM_FlameBreathe,
    ShM_Wind,
    ShM_Missile,
    ShM_Slow, // 10
    ShM_Grenade,
    ShM_Drain,
    ShM_Hail_storm,
    ShM_Arrow,
    ShM_Boulder, // 15
    ShM_GodLightning,
    ShM_Spike,
    ShM_Lizard,
    ShM_Alarm,
    ShM_SolidBoulder, // 20
    ShM_SwingSword,
    ShM_SwingFist,
    ShM_Dig,
    ShM_GodLightBall,
    ShM_Group, // 25
    ShM_Disease,
    ShM_Chicken,
    ShM_TimeBomb,
    ShM_TrapLightning, // 29
};

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Coord3d;

#pragma pack()
/******************************************************************************/
/******************************************************************************/
struct Thing *create_shot(struct Coord3d *pos, unsigned short model, unsigned short owner);
TngUpdateRet update_shot(struct Thing *thing);
TbBool thing_is_shot(const struct Thing *thing);

long get_damage_of_melee_shot(struct Thing *shotng, const struct Thing *target);
long project_damage_of_melee_shot(long shot_dexterity, long shot_damage, const struct Thing *target);
void create_relevant_effect_for_shot_hitting_thing(struct Thing *shotng, struct Thing *target);

TbBool shot_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx);
TbBool shot_model_is_navigable(long tngmodel);
TbBool shot_model_makes_flesh_explosion(long shot_model);
TbBool detonate_shot(struct Thing *shotng);
TbBool shot_is_boulder(const struct Thing *shotng);

struct Thing *get_thing_collided_with_at_satisfying_filter(struct Thing *thing, struct Coord3d *pos, Thing_Collide_Func filter, long a4, long a5);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
