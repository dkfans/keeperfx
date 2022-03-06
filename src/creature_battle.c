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

/******************************************************************************/
/**
 * Returns CreatureBattle of given index.
 */
struct CreatureBattle *creature_battle_get(BattleIndex battle_id)
{
    if ((battle_id < 1) || (battle_id >= BATTLES_COUNT))
        return INVALID_CRTR_BATTLE;
    return &game.battles[battle_id];
}

/**
 * Returns CreatureBattle assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureBattle *creature_battle_get_from_thing(const struct Thing *thing)
{
    if (!thing_is_creature(thing)) {
        return INVALID_CRTR_BATTLE;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
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
TbBool creature_battle_exists(BattleIndex battle_id)
{
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle))
        return false;
    return (battle->fighters_num > 0);
}

TbBool has_melee_combat_attackers(struct Thing *victim)
{
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_melee_count > 0);
}

TbBool can_add_melee_combat_attacker(struct Thing *victim)
{
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_melee_count < COMBAT_MELEE_OPPONENTS_LIMIT);
}

TbBool has_ranged_combat_attackers(const struct Thing *victim)
{
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_ranged_count > 0);
}

BattleIndex find_first_battle_of_mine(PlayerNumber plyr_idx)
{
    for (long i = 1; i < BATTLES_COUNT; i++)
    {
        struct CreatureBattle* battle = creature_battle_get(i);
        if (battle->fighters_num != 0)
        {
            if (battle_with_creature_of_player(plyr_idx, i))
               return i;
        }
    }
    return 0;
}

BattleIndex find_last_battle_of_mine(PlayerNumber plyr_idx)
{
    for (long i = BATTLES_COUNT; i > 0; i--)
    {
        struct CreatureBattle* battle = creature_battle_get(i);
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
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_ranged_count < COMBAT_RANGED_OPPONENTS_LIMIT);
}

long get_flee_position(struct Thing *creatng, struct Coord3d *pos)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    // Heroes should flee to their gate
    if (is_hero_thing(creatng))
    {
        struct Thing* gatetng = find_hero_door_hero_can_navigate_to(creatng);
        if ( !thing_is_invalid(gatetng) )
        {
            pos->x.val = gatetng->mappos.x.val;
            pos->y.val = gatetng->mappos.y.val;
            pos->z.val = gatetng->mappos.z.val;
            return 1;
        }
    }
    else
    // Neutral creatures don't have flee place
    if (is_neutral_thing(creatng))
    {
        if ( (pos->x.val != 0) || (pos->y.val != 0) )
        {
            return 1;
        }
        return 0;
    }
    // Same with creatures without dungeon - try using last place
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    if (dungeon_invalid(dungeon))
    {
        if ( (pos->x.val != 0) || (pos->y.val != 0) )
        {
            return 1;
        }
        return 0;
    }
    // Other creatures can flee to heart or their lair
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    if ( (!thing_is_invalid(lairtng)) && (creature_can_navigate_to_with_storage(creatng, &lairtng->mappos, NavRtF_Default)) )
    {
        TRACE_THING(lairtng);
        pos->x.val = lairtng->mappos.x.val;
        pos->y.val = lairtng->mappos.y.val;
        pos->z.val = lairtng->mappos.z.val;
    }
    else
    if (creature_can_get_to_dungeon(creatng, creatng->owner))
    {
        struct Thing* heartng = get_player_soul_container(creatng->owner);
        TRACE_THING(heartng);
        pos->x.val = heartng->mappos.x.val;
        pos->y.val = heartng->mappos.y.val;
        pos->z.val = heartng->mappos.z.val;
    } else
    {
        struct PlayerInfo* player = get_player(creatng->owner);
        if ( ((player->allocflags & PlaF_Allocated) != 0) && (player->is_active == 1) && (player->victory_state != VicS_LostLevel) )
        {
            ERRORLOG("The %s index %d has no dungeon heart or lair to flee to",thing_model_name(creatng),(int)creatng->index);
            return 0;
        }
        pos->x.stl.pos = creatng->mappos.x.stl.pos;
        pos->y.stl.pos = creatng->mappos.y.stl.pos;
        pos->z.stl.pos = creatng->mappos.z.stl.pos;
    }
    return 1;
}

TbBool setup_combat_flee_position(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Invalid creature control");
        return false;
    }
    if (!get_flee_position(thing, &cctrl->flee_pos))
    {
        ERRORLOG("Couldn't get a flee position for %s index %d",thing_model_name(thing),(int)thing->index);
        cctrl->flee_pos.x.stl.pos = thing->mappos.x.stl.pos;
        cctrl->flee_pos.y.stl.pos = thing->mappos.y.stl.pos;
        cctrl->flee_pos.z.stl.pos = thing->mappos.z.stl.pos;
        return false;
    }
    return true;
}

long get_combat_state_for_combat(struct Thing *fightng, struct Thing *enmtng, CrAttackType attack_pref)
{
    if (attack_pref == AttckT_Ranged)
    {
        if (can_add_ranged_combat_attacker(enmtng)) {
            return CmbtSt_Ranged;
        }
        return CmbtSt_Waiting;
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(fightng);
    if (crstat->attack_preference == AttckT_Ranged)
    {
        if (creature_has_ranged_weapon(fightng) && can_add_ranged_combat_attacker(enmtng)) {
            return CmbtSt_Ranged;
        }
    }
    if (can_add_melee_combat_attacker(enmtng)) {
        return CmbtSt_Melee;
    }
    if ( !creature_has_ranged_weapon(fightng) ) {
        return CmbtSt_Waiting;
    }
    if (can_add_ranged_combat_attacker(enmtng)) {
        return CmbtSt_Ranged;
    }
    return CmbtSt_Waiting;
}

void set_creature_in_combat(struct Thing *fightng, struct Thing *enmtng, CrAttackType attack_type)
{
    SYNCDBG(8,"Starting for %s index %d and %s index %d",thing_model_name(fightng),(int)fightng->index,thing_model_name(enmtng),(int)enmtng->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(fightng);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Invalid creature control");
        return;
    }
    if ( (cctrl->combat_flags != 0) && ((cctrl->combat_flags & (CmbtF_ObjctFight|CmbtF_DoorFight)) == 0) )
    {
        CrtrStateId crstate = get_creature_state_besides_move(fightng);
        ERRORLOG("Creature in combat already - state %s", creature_state_code_name(crstate));
        return;
    }
    if ( !external_set_thing_state(fightng, CrSt_CreatureInCombat) ) {
        ERRORLOG("Failed to enter combat state for %s index %d",thing_model_name(fightng),(int)fightng->index);
        return;
    }
    cctrl->field_AA = 0;
    cctrl->fight_til_death = 0;
    if ( !set_creature_combat_state(fightng, enmtng, attack_type) ) {
        WARNLOG("Couldn't setup combat state for %s index %d and %s index %d",thing_model_name(fightng),(int)fightng->index,thing_model_name(enmtng),(int)enmtng->index);
        set_start_state(fightng);
        return;
    }
    setup_combat_flee_position(fightng);
}

TbBool active_battle_exists(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->visible_battles[0] != 0);
}

TbBool step_battles_forward(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(8,"Cannot do; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    BattleIndex i = dungeon->visible_battles[2];
    if (i > 0) {
        i = find_next_battle_of_mine_excluding_current_list(plyr_idx, i);
    }
    if (i > 0) {
        dungeon->visible_battles[2] = i;
        i = find_previous_battle_of_mine_excluding_current_list(plyr_idx, i);
        dungeon->visible_battles[1] = i;
        i = find_previous_battle_of_mine_excluding_current_list(plyr_idx, i);
        dungeon->visible_battles[0] = i;
    }
    return active_battle_exists(plyr_idx);
}

long battle_move_player_towards_battle(struct PlayerInfo *player, BattleIndex battle_idx)
{
    struct CreatureBattle* battle = creature_battle_get(battle_idx);
    struct Thing* thing = thing_get(battle->first_creatr);
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
    set_player_instance(player, PI_ZoomToPos, 0);
    return 1;
}

void battle_initialise(void)
{
    for (int battle_idx = 0; battle_idx < BATTLES_COUNT; battle_idx++)
    {
        LbMemorySet(&game.battles[battle_idx], 0, sizeof(struct CreatureBattle));
    }
}

BattleIndex find_next_battle_of_mine(PlayerNumber plyr_idx, BattleIndex prev_idx)
{
    for (BattleIndex next_idx = prev_idx + 1; next_idx < BATTLES_COUNT; next_idx++)
    {
        if (creature_battle_exists(next_idx) && battle_with_creature_of_player(plyr_idx, next_idx)) {
            return next_idx;
        }
    }
    return find_first_battle_of_mine(plyr_idx);
}

BattleIndex find_previous_battle_of_mine(PlayerNumber plyr_idx, BattleIndex next_idx)
{
    for (BattleIndex prev_idx = next_idx - 1; prev_idx > 0; prev_idx--)
    {
        if (creature_battle_exists(prev_idx) && battle_with_creature_of_player(plyr_idx, prev_idx)) {
            return prev_idx;
        }
    }
    return find_last_battle_of_mine(plyr_idx);
}

TbBool battle_in_list(PlayerNumber plyr_idx, BattleIndex battle_id)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    for (long i = 0; i < 3; i++)
    {
        if (battle_id > 0)
        {
            if (dungeon->visible_battles[i] == battle_id)
                return true;
        }
    }
    return false;
}

BattleIndex find_next_battle_of_mine_excluding_current_list(PlayerNumber plyr_idx, BattleIndex prev_idx)
{
    BattleIndex battle_idx = find_next_battle_of_mine(plyr_idx, prev_idx);
    BattleIndex first_idx = battle_idx;
    while (battle_in_list(plyr_idx, battle_idx))
    {
        battle_idx = find_next_battle_of_mine(plyr_idx, battle_idx);
        if (battle_idx == first_idx)
            return 0;
    }
    return battle_idx;
}

BattleIndex find_previous_battle_of_mine_excluding_current_list(PlayerNumber plyr_idx, BattleIndex next_idx)
{
    BattleIndex battle_id = find_previous_battle_of_mine(plyr_idx, next_idx);
    BattleIndex last_idx = battle_id;
    while (battle_in_list(plyr_idx, battle_id))
    {
        battle_id = find_previous_battle_of_mine(plyr_idx, battle_id);
        if (battle_id == last_idx)
            return 0;
    }
    return battle_id;
}

TbBool clear_battlers(unsigned short *friendly_battlers, unsigned short *enemy_battlers)
{
    for (long i = 0; i < MESSAGE_BATTLERS_COUNT; i++)
    {
        friendly_battlers[i] = 0;
        enemy_battlers[i] = 0;
    }
    return true;
}

long setup_player_battlers(struct PlayerInfo *player, struct CreatureBattle *battle, unsigned short *friendly_battlers, unsigned short *enemy_battlers)
{
    short friendly_pos = 0;
    short enemy_pos = 0;
    long i = battle->first_creatr;
    unsigned long k = 0;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Invalid thing index %d in friend/enemy battle-list",(int)i);
            break;
        }
        TRACE_THING(thing);
        if (!thing_is_creature(thing)) {
            ERRORLOG("Dead thing index %d in friend/enemy battle-list",(int)i);
            break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
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
    struct PlayerInfo* player = get_my_player();
    struct CreatureBattle* battle = creature_battle_get(battle_idx);
    if (creature_battle_invalid(battle)) {
        ERRORLOG("Invalid battle %d",(int)battle_idx);
        return 0;
    }
    return setup_player_battlers(player, battle, friendly_battlers, enemy_battlers);
}

void maintain_my_battle_list(void)
{
    long i;
    // Find battle index
    struct PlayerInfo* player = get_my_player();
    struct Dungeon* dungeon = get_players_dungeon(player);
    BattleIndex battle_id = 0;
    for (i=0; i < 3; i++)
    {
        struct CreatureBattle* battle = creature_battle_get(dungeon->visible_battles[i]);
        if (battle->fighters_num > 0) {
            battle_id = dungeon->visible_battles[i];
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
          for (long n = i + 1; n < 3; n++)
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
          battle_id = find_next_battle_of_mine_excluding_current_list(player->id_number, battle_id);
          if (battle_id > 0) {
              dungeon->visible_battles[i] = battle_id;
          }
      }
    }
    for (i=0; i < 3; i++)
    {
        battle_id = dungeon->visible_battles[i];
        if (battle_id > 0) {
            setup_my_battlers(dungeon->visible_battles[i], &friendly_battler_list[MESSAGE_BATTLERS_COUNT*i], &enemy_battler_list[MESSAGE_BATTLERS_COUNT*i]);
        }
    }
}
/******************************************************************************/
