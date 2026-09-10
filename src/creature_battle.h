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
#define BATTLES_COUNT          192
#define MESSAGE_BATTLERS_COUNT   8
/** Amount of battles the battle panel can show at the same time. */
#define VISIBLE_BATTLES_COUNT    3
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct PlayerInfo;

struct CreatureBattle {
  unsigned long fighters_num;
  unsigned short first_creatr;
  unsigned short last_creatr;
  /** Turn on which the battle started, ie. on which its first fighter was added. */
  GameTurn start_turn;
};

#pragma pack()
/******************************************************************************/
#define INVALID_CRTR_BATTLE (&game.battles[0])
/******************************************************************************/
/** Battles shown in the battle panel, newest first, and the creatures listed on each of its
 * rows. This is interface state of the local client alone, no part of the game state; it is
 * derived from game.battles[] by maintain_my_battle_list(), which runs every frame and builds
 * it again whenever battle_lists_changed() has been called or the alliances differ. */
extern BattleIndex visible_battles[VISIBLE_BATTLES_COUNT];
extern unsigned short friendly_battler_list[VISIBLE_BATTLES_COUNT*MESSAGE_BATTLERS_COUNT];
extern unsigned short enemy_battler_list[VISIBLE_BATTLES_COUNT*MESSAGE_BATTLERS_COUNT];
/******************************************************************************/

struct CreatureBattle *creature_battle_get(BattleIndex battle_id);
struct CreatureBattle *creature_battle_get_from_thing(const struct Thing *thing);
TbBool creature_battle_invalid(const struct CreatureBattle *battle);
TbBool creature_battle_exists(BattleIndex battle_idx);

BattleIndex find_first_battle_of_mine(PlayerNumber plyr_idx);
unsigned long count_active_battles(PlayerNumber plyr_idx);

TbBool has_melee_combat_attackers(struct Thing *victim);
TbBool can_add_melee_combat_attacker(struct Thing *victim);
TbBool has_ranged_combat_attackers(const struct Thing *victim);
TbBool can_add_ranged_combat_attacker(const struct Thing *victim);

TbBool setup_combat_flee_position(struct Thing *thing);
long get_flee_position(struct Thing *creatng, struct Coord3d *pos);
void set_creature_in_combat(struct Thing *fightng, struct Thing *enmtng, CrAttackType attack_type);
long get_combat_state_for_combat(struct Thing *fightng, struct Thing *enmtng, CrAttackType attack_pref);

TbBool active_battle_exists(void);
void battle_lists_changed(void);
void maintain_my_battle_list(void);
void reset_visible_battles(void);
TbBool step_battles_forward(PlayerNumber plyr_idx);
TbBool step_battles_backward(PlayerNumber plyr_idx);
TbBool cycle_to_next_battle(PlayerNumber plyr_idx);
TbBool battle_panel_can_scroll(void);
long battle_move_player_towards_battle(struct PlayerInfo *player, BattleIndex battle_id);
void battle_initialise(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
