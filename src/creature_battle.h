/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_battle.h
 *     Header file for creature_battle.c.
 * @par Purpose:
 *     Creature battle structure and utility functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     23 Jul 2011 - 05 Sep 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRBATTLE_H
#define DK_CRTRBATTLE_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Max amount of battles supported on any map. */
#define BATTLES_COUNT          48
#define MESSAGE_BATTLERS_COUNT  8
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct PlayerInfo;

struct CreatureBattle { // sizeof = 17
  unsigned long fighters_num;
  unsigned char field_4[9];
  unsigned short first_creatr;
  unsigned short last_creatr;
};

#pragma pack()
/******************************************************************************/
#define INVALID_CRTR_BATTLE (&game.battles[0])
/******************************************************************************/
DLLIMPORT extern unsigned short _DK_friendly_battler_list[3*MESSAGE_BATTLERS_COUNT];
#define friendly_battler_list _DK_friendly_battler_list
DLLIMPORT extern unsigned short _DK_enemy_battler_list[3*MESSAGE_BATTLERS_COUNT];
#define enemy_battler_list _DK_enemy_battler_list
/******************************************************************************/

struct CreatureBattle *creature_battle_get(BattleIndex battle_id);
struct CreatureBattle *creature_battle_get_from_thing(const struct Thing *thing);
TbBool creature_battle_invalid(const struct CreatureBattle *battle);
TbBool creature_battle_exists(BattleIndex battle_idx);

BattleIndex find_first_battle_of_mine(PlayerNumber plyr_idx);
BattleIndex find_last_battle_of_mine(PlayerNumber plyr_idx);
BattleIndex find_next_battle_of_mine(PlayerNumber plyr_idx, BattleIndex prev_idx);
BattleIndex find_previous_battle_of_mine(PlayerNumber plyr_idx, BattleIndex next_idx);
BattleIndex find_next_battle_of_mine_excluding_current_list(PlayerNumber plyr_idx, BattleIndex prev_idx);
BattleIndex find_previous_battle_of_mine_excluding_current_list(PlayerNumber plyr_idx, BattleIndex next_idx);

TbBool has_melee_combat_attackers(struct Thing *victim);
TbBool can_add_melee_combat_attacker(struct Thing *victim);
TbBool has_ranged_combat_attackers(const struct Thing *victim);
TbBool can_add_ranged_combat_attacker(const struct Thing *victim);

TbBool setup_combat_flee_position(struct Thing *thing);
long get_flee_position(struct Thing *creatng, struct Coord3d *pos);
void set_creature_in_combat(struct Thing *fightng, struct Thing *enmtng, CrAttackType attack_type);
long get_combat_state_for_combat(struct Thing *fightng, struct Thing *enmtng, CrAttackType attack_pref);

TbBool active_battle_exists(PlayerNumber plyr_idx);
void maintain_my_battle_list(void);
TbBool step_battles_forward(PlayerNumber plyr_idx);
long battle_move_player_towards_battle(struct PlayerInfo *player, BattleIndex battle_id);
void battle_initialise(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
