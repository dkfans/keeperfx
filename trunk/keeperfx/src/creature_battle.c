/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_battle.c
 *     Creature battle structure and utility functions.
 * @par Purpose:
 *     Defines CreatureBattle struct and various battle-related routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Jul 2011 - 05 Sep 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_battle.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_memory.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "creature_states_combt.h"
#include "creature_instances.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "player_instances.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned char _DK_find_first_battle_of_mine(unsigned char idx);
DLLIMPORT unsigned char _DK_active_battle_exists(unsigned char plyr_idx);
DLLIMPORT unsigned char _DK_step_battles_forward(unsigned char plyr_idx);
DLLIMPORT long _DK_battle_move_player_towards_battle(struct PlayerInfo *player, long var);
DLLIMPORT void _DK_maintain_my_battle_list(void);
DLLIMPORT void _DK_battle_initialise(void);
/******************************************************************************/
DLLIMPORT extern unsigned short _DK_friendly_battler_list[3*MESSAGE_BATTLERS_COUNT];
#define friendly_battler_list _DK_friendly_battler_list
DLLIMPORT extern unsigned short _DK_enemy_battler_list[3*MESSAGE_BATTLERS_COUNT];
#define enemy_battler_list _DK_enemy_battler_list
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Returns CreatureBattle of given index.
 */
struct CreatureBattle *creature_battle_get(BattleIndex battle_idx)
{
    if ((battle_idx < 1) || (battle_idx >= BATTLES_COUNT))
        return INVALID_CRTR_BATTLE;
    return &game.battles[battle_idx];
}

/**
 * Returns CreatureBattle assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureBattle *creature_battle_get_from_thing(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    if (!thing_is_creature(thing)) {
        return INVALID_CRTR_BATTLE;
    }
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        return INVALID_CRTR_BATTLE;
    }
    if ((cctrl->battle_id < 1) || (cctrl->battle_id >= BATTLES_COUNT)) {
      return INVALID_CRTR_BATTLE;
    }
    return &game.battles[cctrl->battle_id];
}

/**
 * Returns if given CreatureBattle pointer is incorrect.
 */
TbBool creature_battle_invalid(const struct CreatureBattle *battle)
{
  return (battle <= &game.battles[0]) || (battle == NULL);
}

/**
 * Returns if CreatureBattle of given index is an existing (ongoing) battle.
 */
TbBool creature_battle_exists(BattleIndex battle_idx)
{
    struct CreatureBattle *battle;
    battle = creature_battle_get(battle_idx);
    if (creature_battle_invalid(battle))
        return false;
    return (battle->fighters_num > 0);
}

TbBool has_melee_combat_attackers(struct Thing *victim)
{
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_melee_count > 0);
}

TbBool can_add_melee_combat_attacker(struct Thing *victim)
{
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_melee_count < COMBAT_MELEE_OPPONENTS_LIMIT);
}

TbBool has_ranged_combat_attackers(const struct Thing *victim)
{
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_ranged_count > 0);
}

BattleIndex find_first_battle_of_mine(PlayerNumber plyr_idx)
{
    struct CreatureBattle *battle;
    long i;
    //return _DK_find_first_battle_of_mine(plyr_idx);
    for (i = 1; i < BATTLES_COUNT; i++)
    {
        battle = creature_battle_get(i);
        if (battle->fighters_num != 0)
        {
            if (battle_with_creature_of_player(plyr_idx, i))
               return i;
        }
    }
    return 0;
}

TbBool can_add_ranged_combat_attacker(const struct Thing *victim)
{
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_ranged_count < COMBAT_RANGED_OPPONENTS_LIMIT);
}

long get_flee_position(struct Thing *thing, struct Coord3d *pos)
{
    struct Dungeon *dungeon;
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct Thing *lairtng;
    struct Thing *heartng;
    struct Thing *gatetng;
    //return _DK_get_flee_position(thing, pos);

    cctrl = creature_control_get_from_thing(thing);
    // Heroes should flee to their gate
    if (thing->owner == game.hero_player_num)
    {
        gatetng = find_hero_door_hero_can_navigate_to(thing);
        if ( !thing_is_invalid(gatetng) )
        {
            pos->x.val = gatetng->mappos.x.val;
            pos->y.val = gatetng->mappos.y.val;
            pos->z.val = gatetng->mappos.z.val;
            return 1;
        }
    } else
    // Neutral creatures don't have flee place
    if (is_neutral_thing(thing))
    {
        if ( (pos->x.val != 0) || (pos->y.val != 0) )
        {
            return 1;
        }
        return 0;
    }
    // Same with creatures without dungeon - try using last place
    dungeon = get_dungeon(thing->owner);
    if ( dungeon_invalid(dungeon) )
    {
        if ( (pos->x.val != 0) || (pos->y.val != 0) )
        {
            return 1;
        }
        return 0;
    }
    // Other creatures can flee to heart or their lair
    if (cctrl->lairtng_idx > 0)
    {
        lairtng = thing_get(cctrl->lairtng_idx);
        TRACE_THING(lairtng);
        pos->x.val = lairtng->mappos.x.val;
        pos->y.val = lairtng->mappos.y.val;
        pos->z.val = lairtng->mappos.z.val;
    } else
    if (dungeon->dnheart_idx > 0)
    {
        heartng = thing_get(dungeon->dnheart_idx);
        TRACE_THING(heartng);
        pos->x.val = heartng->mappos.x.val;
        pos->y.val = heartng->mappos.y.val;
        pos->z.val = heartng->mappos.z.val;
    } else
    {
        player = get_player(thing->owner);
        if ( ((player->field_0 & 0x01) != 0) && (player->field_2C == 1) && (player->victory_state != VicS_LostLevel) )
        {
            ERRORLOG("The %s has no dungeon heart or lair to flee to",thing_model_name(thing));
            return 0;
        }
        pos->x.val = thing->mappos.x.val;
        pos->y.val = thing->mappos.y.val;
        pos->z.val = thing->mappos.z.val;
    }
    return 1;
}

void setup_combat_flee_position(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Invalid creature control");
        return;
    }
    if ( !get_flee_position(thing, &cctrl->flee_pos) )
    {
        ERRORLOG("Couldn't get a flee position for %s index %d",thing_model_name(thing),(int)thing->index);
        cctrl->flee_pos.x.val = thing->mappos.x.val;
        cctrl->flee_pos.y.val = thing->mappos.y.val;
        cctrl->flee_pos.z.val = thing->mappos.z.val;
    }
}

long get_combat_state_for_combat(struct Thing *fighter, struct Thing *enemy, long possible_combat)
{
    struct CreatureStats *crstat;
    if (possible_combat == 2)
    {
        if (can_add_ranged_combat_attacker(enemy)) {
            return 2;
        }
        return 1;
    }
    crstat = creature_stats_get_from_thing(fighter);
    if (crstat->attack_preference == PrefAttck_Ranged)
    {
        if ( creature_has_ranged_weapon(fighter) && can_add_ranged_combat_attacker(enemy) ) {
            return 2;
        }
    }
    if (can_add_melee_combat_attacker(enemy)) {
        return 3;
    }
    if ( !creature_has_ranged_weapon(fighter) ) {
        return 1;
    }
    if (can_add_ranged_combat_attacker(enemy)) {
        return 2;
    }
    return 1;
}

void set_creature_in_combat(struct Thing *fighter, struct Thing *enemy, long combat_kind)
{
    struct CreatureControl *cctrl;
    SYNCDBG(8,"Starting for %s and %s",thing_model_name(fighter),thing_model_name(enemy));
    cctrl = creature_control_get_from_thing(fighter);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Invalid creature control");
        return;
    }
    if ( (cctrl->combat_flags != 0) && ((cctrl->combat_flags & (CmbtF_Unknown08|CmbtF_Unknown10)) == 0) )
    {
        CrtrStateId crstate = get_creature_state_besides_move(fighter);
        ERRORLOG("Creature in combat already - state %s", creature_state_code_name(crstate));
        return;
    }
    if ( !external_set_thing_state(fighter, CrSt_CreatureInCombat) ) {
        ERRORLOG("Failed to change state");
        return;
    }
    cctrl->field_AA = 0;
    cctrl->fight_til_death = 0;
    if ( !set_creature_combat_state(fighter, enemy, combat_kind) ) {
        WARNLOG("Couldn't setup combat state for %s and %s",thing_model_name(fighter),thing_model_name(enemy));
        set_start_state(fighter);
        return;
    }
    setup_combat_flee_position(fighter);
}

unsigned char active_battle_exists(unsigned char plyr_idx)
{
    //return _DK_active_battle_exists(a1);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->visible_battles[0] != 0);
}

unsigned char step_battles_forward(unsigned char a1)
{
    return _DK_step_battles_forward(a1);
}

long battle_move_player_towards_battle(struct PlayerInfo *player, BattleIndex battle_id)
{
    struct CreatureBattle *battle;
    struct Thing *thing;
    //return _DK_battle_move_player_towards_battle(player, var);
    battle = creature_battle_get(battle_id);
    thing = thing_get(battle->first_creatr);
    TRACE_THING(thing);
    if (!thing_exists(thing))
    {
        ERRORLOG("Jump to invalid thing detected");
        player->zoom_to_pos_x = subtile_coord_center(map_subtiles_x/2);
        player->zoom_to_pos_y = subtile_coord_center(map_subtiles_y/2);
        return 0;
    }
    player->zoom_to_pos_x = thing->mappos.x.val;
    player->zoom_to_pos_y = thing->mappos.y.val;
    set_player_instance(player, 16, 0);
    return 1;
}

void battle_initialise(void)
{
    int battle_idx;
    //_DK_battle_initialise();
    for (battle_idx = 0; battle_idx < BATTLES_COUNT; battle_idx++) {
        LbMemorySet(&game.battles[battle_idx], 0, sizeof(struct CreatureBattle));
    }
}

BattleIndex find_next_battle_of_mine(PlayerNumber plyr_idx, BattleIndex prev_idx)
{
    BattleIndex next_idx;
    for (next_idx = prev_idx+1; next_idx < BATTLES_COUNT; next_idx++)
    {
        if (creature_battle_exists(next_idx) && battle_with_creature_of_player(plyr_idx, next_idx)) {
            return next_idx;
        }
    }
    return find_first_battle_of_mine(plyr_idx);
}

TbBool battle_in_list(PlayerNumber plyr_idx, BattleIndex battle_idx)
{
    struct Dungeon *dungeon;
    long i;
    dungeon = get_players_num_dungeon(plyr_idx);
    for (i=0; i < 3; i++)
    {
        if (battle_idx > 0)
        {
            if (dungeon->visible_battles[i] == battle_idx)
                return true;
        }
    }
    return false;
}

BattleIndex find_next_battle_of_mine_excluding_current_list(PlayerNumber plyr_idx, BattleIndex prev_idx)
{
    BattleIndex first_idx;
    BattleIndex battle_idx;
    battle_idx = find_next_battle_of_mine(plyr_idx, prev_idx);
    first_idx = battle_idx;
    while (battle_in_list(plyr_idx, battle_idx))
    {
        battle_idx = find_next_battle_of_mine(plyr_idx, battle_idx);
        if (battle_idx == first_idx)
            return 0;
    }
    return battle_idx;
}

TbBool clear_battlers(unsigned short *friendly_battlers, unsigned short *enemy_battlers)
{
    long i;
    for (i=0; i < MESSAGE_BATTLERS_COUNT; i++)
    {
        friendly_battlers[i] = 0;
        enemy_battlers[i] = 0;
    }
    return true;
}

long setup_player_battlers(struct PlayerInfo *player, struct CreatureBattle *battle, unsigned short *friendly_battlers, unsigned short *enemy_battlers)
{
    long i;
    unsigned long k;
    short friendly_pos, enemy_pos;
    friendly_pos = 0;
    enemy_pos = 0;
    i = battle->first_creatr;
    k = 0;
    while (i > 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Invalid thing index %d in friend/enemy battle-list",(int)i);
            break;
        }
        TRACE_THING(thing);
        if (!thing_is_creature(thing)) {
            ERRORLOG("Dead thing index %d in friend/enemy battle-list",(int)i);
            break;
        }
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        i = cctrl->battle_prev_creatr;
        // Per creature code
        long n;
        if (players_are_mutual_allies(player->id_number, thing->owner))
        {
            n = friendly_pos;
            if (n < MESSAGE_BATTLERS_COUNT) {
                friendly_pos++;
                friendly_battlers[n] = thing->index;
            }
        } else
        {
            n = enemy_pos;
            if (n < MESSAGE_BATTLERS_COUNT) {
                enemy_pos++;
                enemy_battlers[n] = thing->index;
            }
        }
        // Per creature code ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping battle creatures list");
            break;
        }
    }
    return friendly_pos+enemy_pos;
}

long setup_my_battlers(unsigned char battle_idx, unsigned short *friendly_battlers, unsigned short *enemy_battlers)
{
    // Clear the battlers
    clear_battlers(friendly_battlers, enemy_battlers);
    // And fill them with new data
    struct PlayerInfo *player;
    player = get_my_player();
    struct CreatureBattle *battle;
    battle = creature_battle_get(battle_idx);
    if (creature_battle_invalid(battle)) {
        ERRORLOG("Invalid battle %d",(int)battle_idx);
        return 0;
    }
    return setup_player_battlers(player, battle, friendly_battlers, enemy_battlers);
}

void maintain_my_battle_list(void)
{
    //_DK_maintain_my_battle_list();

    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    BattleIndex battle_idx;
    long i,n;
    // Find battle index
    player = get_my_player();
    dungeon = get_players_dungeon(player);
    battle_idx = 0;
    for (i=0; i < 3; i++)
    {
        struct CreatureBattle *battle;
        battle = creature_battle_get(dungeon->visible_battles[i]);
        if (battle->fighters_num > 0) {
            battle_idx = dungeon->visible_battles[i];
        } else {
            dungeon->visible_battles[i] = 0;
        }
    }
    // Move array items down to make sure empty slots are at end
    for (i=0; i < 2; i++)
    {
      if (dungeon->visible_battles[i] <= 0)
      {
          // Got empty spot - fill it with first non-empty item
          for (n=i+1; n < 3; n++)
          {
              if (dungeon->visible_battles[n] > 0)
              {
                  dungeon->visible_battles[i] = dungeon->visible_battles[n];
                  dungeon->visible_battles[n] = 0;
                  break;
              }
          }
      }
    }
    // Find battles to fill empty slots
    for (i=0; i < 3; i++)
    {
      if (dungeon->visible_battles[i] <= 0)
      {
          battle_idx = find_next_battle_of_mine_excluding_current_list(player->id_number, battle_idx);
          if (battle_idx > 0) {
              dungeon->visible_battles[i] = battle_idx;
          }
      }
    }
    for (i=0; i < 3; i++)
    {
        battle_idx = dungeon->visible_battles[i];
        if (battle_idx > 0) {
            setup_my_battlers(dungeon->visible_battles[i], &friendly_battler_list[MESSAGE_BATTLERS_COUNT*i], &enemy_battler_list[MESSAGE_BATTLERS_COUNT*i]);
        }
    }
}
/******************************************************************************/
