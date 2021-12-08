/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_combt.h
 *     Header file for creature_states_combt.c.
 * @par Purpose:
 *     Creature state machine functions related to combat.
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
#ifndef DK_CRTRSTATECOMBT_H
#define DK_CRTRSTATECOMBT_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;

/** How often to check for possible battles, in game turns */
#define BATTLE_CHECK_INTERVAL 8

typedef void (*CombatState)(struct Thing *);

struct CombatWeapon {
    long inst_id;
    long range_min;
    long range_max;
};

extern struct CombatWeapon offensive_weapon[];

#pragma pack()
/******************************************************************************/
extern const CombatState combat_state[];
/******************************************************************************/
short cleanup_combat(struct Thing *thing);
short cleanup_door_combat(struct Thing *thing);
short cleanup_object_combat(struct Thing *thing);
short cleanup_seek_the_enemy(struct Thing *thing);
short creature_combat_flee(struct Thing *thing);
short creature_door_combat(struct Thing *thing);
short creature_in_combat(struct Thing *thing);
short creature_object_combat(struct Thing *thing);
void creature_in_combat_wait(struct Thing *thing);
void creature_in_ranged_combat(struct Thing *thing);
void creature_in_melee_combat(struct Thing *thing);
void combat_object_state_melee_combat(struct Thing *thing);
void combat_object_state_ranged_combat(struct Thing *thing);
void combat_door_state_melee_combat(struct Thing *thing);
void combat_door_state_ranged_combat(struct Thing *thing);

short creature_attack_rooms(struct Thing *thing);
short creature_damage_walls(struct Thing *thing);
short creature_attempt_to_damage_walls(struct Thing *thing);
CrAttackType creature_can_have_combat_with_creature(struct Thing *fighter1, struct Thing *fighter2, long a2, long a4, long a5);
TbBool creature_too_scared_for_combat(struct Thing *thing, struct Thing *enemy);
TbBool creature_is_being_attacked_by_enemy_player(struct Thing *fightng);
TbBool creature_is_being_attacked_by_enemy_creature_not_digger(struct Thing *fightng);
TbBool creature_has_creature_in_combat(const struct Thing *thing, const struct Thing *enmtng);
TbBool set_creature_combat_state(struct Thing *fighter, struct Thing *enemy, CrAttackType attack_type);
TbBool battle_with_creature_of_player(PlayerNumber plyr_idx, BattleIndex battle_id);

long project_creature_attack_target_damage(const struct Thing *firing, const struct Thing *target);

long process_creature_self_spell_casting(struct Thing *thing);
CrInstance get_best_quick_range_instance_to_use(const struct Thing *thing);

TbBool creature_will_do_combat(const struct Thing *thing);
TbBool creature_look_for_combat(struct Thing *creatng);
TbBool creature_look_for_enemy_heart_combat(struct Thing *thing);
TbBool creature_look_for_enemy_door_combat(struct Thing *thing);

struct Thing *check_for_door_to_fight(struct Thing *thing);
CrAttackType check_for_possible_combat_with_attacker_within_distance(struct Thing *figtng, struct Thing **outenmtng, long maxdist, unsigned long *outscore);
CrAttackType check_for_possible_combat_with_enemy_creature_within_distance(struct Thing *fightng, struct Thing **outenmtng, long maxdist);
TbResult creature_retreat_from_combat(struct Thing *figtng, struct Thing *enmtng, CrtrStateId continue_state, long a4);
TbBool creature_can_see_combat_path(const struct Thing *creatng, const struct Thing *enmtng, MapCoordDelta dist);
long get_combat_distance(const struct Thing *thing, const struct Thing *enemy);
TbBool set_creature_in_combat_to_the_death(struct Thing *fighter, struct Thing *enemy, CrAttackType attack_type);
CrAttackType find_fellow_creature_to_fight_in_room(struct Thing *fighter, struct Room *room,long crmodel, struct Thing **enemytng);
long remove_all_traces_of_combat(struct Thing *thing);
long get_combat_score(const struct Thing *thing, const struct Thing *enmtng, CrAttackType attack_type, long a4);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
