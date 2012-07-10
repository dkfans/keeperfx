/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_combt.c
 *     Creature state machine functions related to combat.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_combt.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_battle.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_cleanup_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_door_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_object_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_seek_the_enemy(struct Thing *thing);
DLLIMPORT short _DK_creature_attack_rooms(struct Thing *thing);
DLLIMPORT short _DK_creature_attempt_to_damage_walls(struct Thing *thing);
DLLIMPORT short _DK_creature_combat_flee(struct Thing *thing);
DLLIMPORT short _DK_creature_damage_walls(struct Thing *thing);
DLLIMPORT short _DK_creature_door_combat(struct Thing *thing);
DLLIMPORT short _DK_creature_in_combat(struct Thing *thing);
DLLIMPORT short _DK_creature_object_combat(struct Thing *thing);
DLLIMPORT void _DK_creature_in_combat_wait(struct Thing *thing);
DLLIMPORT void _DK_creature_in_ranged_combat(struct Thing *thing);
DLLIMPORT void _DK_creature_in_melee_combat(struct Thing *thing);
DLLIMPORT long _DK_creature_is_most_suitable_for_combat(struct Thing *thing, struct Thing *enmtng);
DLLIMPORT long _DK_check_for_valid_combat(struct Thing *thing, struct Thing *enmtng);
DLLIMPORT long _DK_combat_type_is_choice_of_creature(struct Thing *thing, long cmbtyp);
DLLIMPORT long _DK_ranged_combat_move(struct Thing *thing, struct Thing *enmtng, long dist, long a4);
DLLIMPORT long _DK_melee_combat_move(struct Thing *thing, struct Thing *enmtng, long dist, long a4);
DLLIMPORT long _DK_creature_can_have_combat_with_creature(const struct Thing *fighter1, const struct Thing *fighter2, long enemy, long a4, long a5);
DLLIMPORT void _DK_set_creature_combat_state(struct Thing *fighter1, struct Thing *fighter2, long dist);
DLLIMPORT long _DK_creature_has_other_attackers(struct Thing *thing, long enemy);
DLLIMPORT long _DK_get_best_ranged_offensive_weapon(struct Thing *thing, long enemy);
DLLIMPORT long _DK_get_best_melee_offensive_weapon(struct Thing *thing, long enemy);
DLLIMPORT long _DK_jonty_creature_can_see_thing_including_lava_check(struct Thing * creature, struct Thing * thing);
DLLIMPORT long _DK_add_ranged_attacker(struct Thing *fighter, struct Thing *victim);
DLLIMPORT long _DK_add_melee_attacker(struct Thing *fighter, struct Thing *victim);
DLLIMPORT long _DK_creature_has_ranged_weapon(struct Thing *thing);
DLLIMPORT void _DK_battle_add(struct Thing *fighter, struct Thing *victim);
DLLIMPORT long _DK_event_create_event_or_update_old_event(long a1, long a2, unsigned char a3, unsigned char a4, long a5);
DLLIMPORT void _DK_remove_thing_from_battle_list(struct Thing *thing);
DLLIMPORT void _DK_insert_thing_in_battle_list(struct Thing *thing, unsigned short a2);
DLLIMPORT void _DK_cleanup_battle(unsigned short a1);
DLLIMPORT long _DK_check_for_better_combat(struct Thing *thing);
DLLIMPORT long _DK_check_for_possible_combat(struct Thing *crtng, struct Thing **battltng);
DLLIMPORT long _DK_creature_look_for_combat(struct Thing *thing);
DLLIMPORT long _DK_waiting_combat_move(struct Thing *fighter, struct Thing *enemy, long a1, long a2);
DLLIMPORT void _DK_remove_melee_attacker(struct Thing *fighter, struct Thing *victim);
DLLIMPORT void _DK_remove_ranged_attacker(struct Thing *fighter, struct Thing *victim);
DLLIMPORT void _DK_remove_waiting_attacker(struct Thing *fighter);
DLLIMPORT void _DK_battle_remove(struct Thing *fighter);
DLLIMPORT long _DK_creature_has_spare_slot_for_combat(struct Thing *fighter, struct Thing *victim, long combat_kind);
DLLIMPORT long _DK_change_creature_with_existing_attacker(struct Thing *fighter, struct Thing *victim, long combat_kind);
DLLIMPORT void _DK_cleanup_battle_leftovers(struct Thing *thing);
DLLIMPORT long _DK_remove_all_traces_of_combat(struct Thing *thing);
/******************************************************************************/
const CombatState combat_state[] = {
    NULL,
    creature_in_combat_wait,
    creature_in_ranged_combat,
    creature_in_melee_combat,
};


#ifdef __cplusplus
}
#endif
/******************************************************************************/

long jonty_creature_can_see_thing_including_lava_check(struct Thing * creature, struct Thing * thing)
{
    return _DK_jonty_creature_can_see_thing_including_lava_check(creature, thing);
}

long creature_can_see_combat_path(struct Thing * creature, struct Thing * enemy, long dist)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creature);
    if ((crstat->visual_range << 8) >= dist) {
        return jonty_creature_can_see_thing_including_lava_check(creature, enemy);
    }

    return 0;
}

long get_combat_distance(const struct Thing *thing, const struct Thing *enemy)
{
    long dist,avgc;
    dist = get_2d_box_distance(&thing->mappos, &enemy->mappos);
    avgc = (enemy->sizexy + thing->sizexy) / 2;
    if (dist < avgc)
        return 0;
    return dist - avgc;
}

long creature_has_other_attackers(struct Thing *thing, long a2)
{
    return _DK_creature_has_other_attackers(thing, a2);
}

TbBool creature_is_actually_scared(struct Thing *thing, struct Thing *enemy)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    long maxhealth;
    crstat = creature_stats_get_from_thing(thing);
    // Creature with fear 255 are scared of everything other that their own model
    if (crstat->fear == 255)
    {
        if (enemy->model != thing->model)
          return true;
        if (creature_has_other_attackers(thing, thing->model))
          return true;
        return false;
    }
    // With "Flee" tendency on, then creatures are scared if their health
    // drops lower than  fear/256 percentage of base health
    if (player_creature_tends_to(thing->owner,CrTend_Flee))
    {
        cctrl = creature_control_get_from_thing(thing);
        maxhealth = compute_creature_max_health(crstat->health,cctrl->explevel);
        if ((crstat->fear * maxhealth) / 256 >= thing->health)
        {
            if (thing->owner != game.neutral_player_num)
            {
                SYNCDBG(8,"Creature is scared due to tendencies");
                return true;
            }
        }
    }
    return false;
}

long creature_can_have_combat_with_creature(const struct Thing *fighter1, const struct Thing *fighter2, long a2, long a4, long a5)
{
    return _DK_creature_can_have_combat_with_creature(fighter1, fighter2, a2, a4, a5);
}

long event_create_event_or_update_old_event(long a1, long a2, unsigned char a3, unsigned char a4, long a5)
{
    return _DK_event_create_event_or_update_old_event(a1, a2, a3, a4, a5);
}

void remove_thing_from_battle_list(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    unsigned short partner_id;
    struct CreatureBattle *battle;
    SYNCDBG(9,"Starting for %s",thing_model_name(thing));
    //_DK_remove_thing_from_battle_list(thing); return;
    cctrl = creature_control_get_from_thing(thing);
    if ( !thing_is_creature(thing) || creature_control_invalid(cctrl) ) {
      ERRORLOG("Creature should have been already removed due to death");
    }
    battle = creature_battle_get(cctrl->battle_id);
    // Change next index in prev creature
    partner_id = cctrl->battle_prev_creatr;
    if (cctrl->battle_next_creatr > 0)
    {
        struct Thing *attctng;
        struct CreatureControl *attcctrl;
        attctng = thing_get(cctrl->battle_next_creatr);
        attcctrl = creature_control_get_from_thing(attctng);
        if ( creature_control_invalid(attcctrl) ) {
            WARNLOG("Invalid next in battle, %s index %d",thing_model_name(attctng),(int)cctrl->battle_next_creatr);
            // Truncate the list of creatures in battle
            battle->first_creatr = partner_id;
        } else {
            attcctrl->battle_prev_creatr = partner_id;
        }
    } else
    {
        battle->first_creatr = partner_id;
    }
    // Change prev index in next creature
    partner_id = cctrl->battle_next_creatr;
    if (cctrl->battle_prev_creatr > 0) {
        struct Thing *attctng;
        struct CreatureControl *attcctrl;
        attctng = thing_get(cctrl->battle_prev_creatr);
        attcctrl = creature_control_get_from_thing(attctng);
        if ( creature_control_invalid(attcctrl) ) {
            WARNLOG("Invalid previous in battle, %s index %d",thing_model_name(attctng),(int)cctrl->battle_prev_creatr);
            // Truncate the list of creatures in battle
            battle->last_creatr = partner_id;
        } else {
            attcctrl->battle_next_creatr = partner_id;
        }
    } else {
        battle->last_creatr = partner_id;
    }
    cctrl->battle_id = 0;
    cctrl->battle_prev_creatr = 0;
    cctrl->battle_next_creatr = 0;
    // Make sure we're not starting to use invalid battle
    if (creature_battle_invalid(battle))
    {
        ERRORLOG("The %s index %d was in invalid battle",thing_model_name(thing),(int)thing->index);
        battle->fighters_num = 0;
        battle->first_creatr = 0;
        battle->last_creatr = 0;
        return;
    }
    if (battle->fighters_num > 0) {
        battle->fighters_num--;
    } else {
        ERRORLOG("Removing creature from battle, but counter is 0");
    }
    SYNCDBG(19,"Finished");
}

void insert_thing_in_battle_list(struct Thing *thing, unsigned short a2)
{
    _DK_insert_thing_in_battle_list(thing, a2);
}

void cleanup_battle(unsigned short a1)
{
    _DK_cleanup_battle(a1);
}

void update_battle_events(unsigned short battle_id)
{
    struct CreatureBattle *battle;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned short owner_flags;
    MapSubtlCoord pos_x,pos_y;
    unsigned long k;
    int i;
    owner_flags = 0;
    k = 0;
    battle = creature_battle_get(battle_id);
    i = battle->first_creatr;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        cctrl = creature_control_get_from_thing(thing);
        i = cctrl->battle_prev_creatr;
        // Per thing code starts
        owner_flags |= (1 << thing->owner);
        pos_x = thing->mappos.x.stl.pos;
        pos_y = thing->mappos.y.stl.pos;
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        if (i == hero_player_number)
            continue;
        if ( (1 << i) & owner_flags ) {
            event_create_event_or_update_old_event(pos_x, pos_y, 2, i, 0);
        }
    }
}

long battle_any_of_things_in_specific_battle(struct CreatureBattle *battle, struct Thing *tng1, struct Thing *tng2)
{
    struct CreatureControl *cctrl;
    struct Thing *batltng;
    struct Thing *attcktng;
    long i;
    unsigned long k;
    k = 0;
    i = battle->first_creatr;
    while (i != 0)
    {
        batltng = thing_get(i);
        cctrl = creature_control_get_from_thing(batltng);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Invalid control of thing in battle, index %d.",(int)i);
            break;
        }
        i = cctrl->battle_prev_creatr;
        // Per battle creature code
        if ( cctrl->combat_flags )
        {
            attcktng = thing_get(cctrl->battle_victim_idx);
            if ( !thing_is_invalid(attcktng) )
            {
                if ( (attcktng == tng1) || (attcktng == tng2) )
                {
                    if (cctrl->battle_id >= 0) {
                        return cctrl->battle_id;
                    }
                }
            }
        }
        // Per battle creature code ends
        k++;
        if ( k >= 200 ) {
            ERRORLOG("Infinite loop in battle add");
            break;
        }
    }
    return 0;
}

unsigned short find_battle_for_thing(struct Thing *fighter, struct Thing *victim)
{
    struct CreatureBattle *battle;
    unsigned short battle_id;
    long i,n;
    battle_id = 0;
    for (i = 1; i < BATTLES_COUNT; i++) // Originally was 32, but I'm pretty sure there's 48 battles
    {
        battle = creature_battle_get(i);
        if (battle->fighters_num != 0)
        {
            n = battle_any_of_things_in_specific_battle(battle, fighter, victim);
            if (n > 0) {
                battle_id = i;
                break;
            }
        }
        if (battle_id <= 0)
          battle_id = i;
    }
    if (battle_id <= 0) {
        ERRORLOG("No free battle structures");
    }
    return battle_id;
}

TbBool battle_add(struct Thing *fighter, struct Thing *victim)
{
    unsigned short battle_id;
    SYNCDBG(9,"Starting");
    //TODO BATTLE Check if the rewritten code didn't cause wrong battles (ie. leaving creature after battle is over)
    //_DK_battle_add(fighter, victim); return;
    if (thing_is_invalid(fighter) || thing_is_invalid(victim))
    {
        ERRORLOG("Attempt to create battle with invalid creature!");
        return false;
    }
    { // Remove fighter from previous battle
        struct CreatureControl *figctrl;
        figctrl = creature_control_get_from_thing(fighter);
        if (figctrl->battle_id > 0)
        {
            remove_thing_from_battle_list(fighter);
        }
    }
    battle_id = 0;
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    if (creature_control_invalid(vicctrl)) {
        ERRORLOG("Invalid victim %s index %d control",thing_model_name(victim),(int)victim->index);
        return false;
    }
    // Find a new battle to fight in, or use the one victim is already in
    if (vicctrl->battle_id > 0)
    {
        battle_id = vicctrl->battle_id;
    } else
    {
        battle_id = find_battle_for_thing(fighter, victim);
    }
    if (battle_id <= 0) {
        ERRORLOG("No free battle structures");
        return false;
    }
    // Add both fighter and victim to the new battle
    insert_thing_in_battle_list(fighter, battle_id);
    if (vicctrl->battle_id <= 0)
      insert_thing_in_battle_list(victim, battle_id);
    update_battle_events(battle_id);
    cleanup_battle(battle_id);
    SYNCDBG(12,"Finished");
    return true;
}

void battle_remove(struct Thing *fighter)
{
    SYNCDBG(9,"Starting");
    _DK_battle_remove(fighter);
}

TbBool has_ranged_combat_attackers(struct Thing *victim)
{
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_ranged_count > 0);
}

TbBool can_add_ranged_combat_attacker(struct Thing *victim)
{
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_ranged_count < COMBAT_RANGED_OPPONENTS_LIMIT);
}

TbBool add_ranged_combat_attacker(struct Thing *victim, unsigned short fighter_idx)
{
    struct CreatureControl *vicctrl;
    long oppn_idx;
    vicctrl = creature_control_get_from_thing(victim);
    // Check if the fighter is already in opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (vicctrl->opponents_ranged[oppn_idx] == fighter_idx) {
            WARNLOG("Fighter %s index %d already in opponents list",thing_model_name(victim),(int)victim->index);
            return true;
        }
    }
    // Find empty opponent slot
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (vicctrl->opponents_ranged[oppn_idx] == 0)
            break;
    }
    if (oppn_idx >= COMBAT_RANGED_OPPONENTS_LIMIT)
        return false;
    // Add the opponent
    vicctrl->opponents_ranged_count++;
    vicctrl->opponents_ranged[oppn_idx] = fighter_idx;
    return true;
}

TbBool remove_ranged_combat_attacker(struct Thing *victim, unsigned short fighter_idx)
{
    struct CreatureControl *vicctrl;
    long oppn_idx;
    vicctrl = creature_control_get_from_thing(victim);
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (vicctrl->opponents_ranged[oppn_idx] == fighter_idx)
            break;
    }
    if (oppn_idx >= COMBAT_RANGED_OPPONENTS_LIMIT)
        return false;
    vicctrl->opponents_ranged_count--;
    vicctrl->opponents_ranged[oppn_idx] = 0;
    return true;
}

TbBool remove_ranged_attacker(struct Thing *fighter, struct Thing *victim)
{
    struct CreatureControl *figctrl;
    //_DK_remove_ranged_attacker(fighter,victim); return;
    figctrl = creature_control_get_from_thing(fighter);
    {
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(fighter->owner);
        if ( !dungeon_invalid(dungeon) && (dungeon->fights_num > 0) ) {
            dungeon->fights_num--;
        } else {
            WARNLOG("Fight count incorrect while removing attacker %s index %d",thing_model_name(fighter),(int)fighter->index);
        }
    }
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    if (creature_control_invalid(vicctrl)) {
        ERRORLOG("Invalid victim %s index %d control",thing_model_name(victim),(int)victim->index);
        return false;
    }
    if (has_ranged_combat_attackers(victim))
    {
        if (!remove_ranged_combat_attacker(victim, fighter->index)) {
            ERRORLOG("Cannot remove attacker - not in %s index %d opponents",thing_model_name(victim),(int)victim->index);
        }
    } else {
        WARNLOG("Cannot remove attacker - the %s index %d has no opponents",thing_model_name(victim),(int)victim->index);
    }
    figctrl->combat_flags &= ~CmbtF_Ranged;
    figctrl->battle_victim_idx = 0;
    figctrl->long_9E = 0;
    figctrl->fight_til_death = 0;

    battle_remove(fighter);
    return true;
}

long remove_all_ranged_combat_attackers(struct Thing *victim)
{
    struct CreatureControl *vicctrl;
    struct Thing *fighter;
    long oppn_idx,num;
    long fighter_idx;
    num = 0;
    vicctrl = creature_control_get_from_thing(victim);
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        fighter_idx = vicctrl->opponents_ranged[oppn_idx];
        if (fighter_idx > 0) {
            fighter = thing_get(fighter_idx);
            if (remove_ranged_attacker(fighter, victim))
                num++;
        }
    }
    vicctrl->opponents_ranged_count = 0;
    return num;
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

TbBool add_melee_combat_attacker(struct Thing *victim, unsigned short fighter_idx)
{
    struct CreatureControl *vicctrl;
    long oppn_idx;
    vicctrl = creature_control_get_from_thing(victim);
    // Check if the fighter is already in opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (vicctrl->opponents_melee[oppn_idx] == fighter_idx) {
            WARNLOG("Fighter %s index %d already in opponents list",thing_model_name(victim),(int)victim->index);
            return true;
        }
    }
    // Find empty opponent slot
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (vicctrl->opponents_melee[oppn_idx] == 0)
            break;
    }
    if (oppn_idx >= COMBAT_MELEE_OPPONENTS_LIMIT)
        return false;
    // Add the opponent
    vicctrl->opponents_melee_count++;
    vicctrl->opponents_melee[oppn_idx] = fighter_idx;
    return true;
}

TbBool remove_melee_combat_attacker(struct Thing *victim, unsigned short fighter_idx)
{
    struct CreatureControl *vicctrl;
    long oppn_idx;
    vicctrl = creature_control_get_from_thing(victim);
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (vicctrl->opponents_melee[oppn_idx] == fighter_idx)
            break;
    }
    if (oppn_idx >= COMBAT_MELEE_OPPONENTS_LIMIT)
        return false;
    vicctrl->opponents_melee_count--;
    vicctrl->opponents_melee[oppn_idx] = 0;
    return true;
}

TbBool remove_melee_attacker(struct Thing *fighter, struct Thing *victim)
{
    struct CreatureControl *figctrl;
    //_DK_remove_melee_attacker(fighter,victim); return;
    figctrl = creature_control_get_from_thing(fighter);
    {
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(fighter->owner);
        if ( !dungeon_invalid(dungeon) && (dungeon->fights_num > 0) ) {
            dungeon->fights_num--;
        } else {
            WARNLOG("Fight count incorrect while removing attacker %s index %d",thing_model_name(fighter),(int)fighter->index);
        }
    }
    struct CreatureControl *vicctrl;
    vicctrl = creature_control_get_from_thing(victim);
    if (creature_control_invalid(vicctrl)) {
        ERRORLOG("Invalid victim %s index %d control",thing_model_name(victim),(int)victim->index);
        return false;
    }
    if (has_melee_combat_attackers(victim))
    {
        if (!remove_melee_combat_attacker(victim, fighter->index)) {
            ERRORLOG("Cannot remove attacker - not in %s index %d opponents",thing_model_name(victim),(int)victim->index);
        }
    } else {
        WARNLOG("Cannot remove attacker - the %s index %d has no opponents",thing_model_name(victim),(int)victim->index);
    }
    figctrl->combat_flags &= ~CmbtF_Melee;
    figctrl->battle_victim_idx = 0;
    figctrl->long_9E = 0;
    figctrl->fight_til_death = 0;

    battle_remove(fighter);
    return true;
}

long remove_all_melee_combat_attackers(struct Thing *victim)
{
    struct CreatureControl *vicctrl;
    struct Thing *fighter;
    long oppn_idx,num;
    long fighter_idx;
    num = 0;
    vicctrl = creature_control_get_from_thing(victim);
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        fighter_idx = vicctrl->opponents_melee[oppn_idx];
        if (fighter_idx > 0) {
            fighter = thing_get(fighter_idx);
            if (remove_melee_attacker(fighter, victim))
                num++;
        }
    }
    vicctrl->opponents_melee_count = 0;
    return num;
}

long add_ranged_attacker(struct Thing *fighter, struct Thing *victim)
{
    struct CreatureControl *figctrl;
    SYNCDBG(8,"Starting for %s and %s",thing_model_name(fighter),thing_model_name(victim));
    //return _DK_add_ranged_attacker(fighter, victim);
    figctrl = creature_control_get_from_thing(fighter);
    if (figctrl->combat_flags)
    {
        if ((figctrl->combat_flags & CmbtF_Ranged) != 0) {
            SYNCDBG(8,"The %s in ranged combat already - no action",thing_model_name(fighter));
            return false;
        }
        SYNCDBG(8,"The %s index %d in combat already - adding ranged",thing_model_name(fighter),(int)fighter->index);
        return true; // We're not going to add anything
    }
    if (!can_add_ranged_combat_attacker(victim)) {
        SYNCLOG("Cannot Add A Ranged Attacker - opponents limit reached");
        return false;
    }
    figctrl->combat_flags |= CmbtF_Ranged;
    figctrl->battle_victim_idx = victim->index;
    figctrl->long_9E = victim->field_9;
    if (!add_ranged_combat_attacker(victim, fighter->index)) {
        ERRORLOG("Cannot add a ranged attacker, but there was free space - internal error");
        return false;
    }
    if (!battle_add(fighter, victim)) {
        remove_ranged_combat_attacker(victim, fighter->index);
        return false;
    }
    return true;
}

long add_melee_attacker(struct Thing *fighter, struct Thing *victim)
{
    struct CreatureControl *figctrl;
    SYNCDBG(8,"Starting for %s and %s",thing_model_name(fighter),thing_model_name(victim));
    figctrl = creature_control_get_from_thing(fighter);
    if (figctrl->combat_flags)
    {
        if ((figctrl->combat_flags & CmbtF_Melee) != 0) {
            SYNCDBG(8,"The %s in melee combat already - no action",thing_model_name(fighter));
            return false;
        }
        SYNCDBG(8,"The %s index %d in combat already - adding melee",thing_model_name(fighter),(int)fighter->index);
        return true; // We're not going to add anything
    }
    if (!can_add_ranged_combat_attacker(victim)) {
        SYNCLOG("Cannot Add A Melee Attacker - opponents limit reached");
        return false;
    }
    figctrl->combat_flags |= CmbtF_Melee;
    figctrl->battle_victim_idx = victim->index;
    figctrl->long_9E = victim->field_9;
    if (!add_melee_combat_attacker(victim, fighter->index)) {
        ERRORLOG("Cannot add a melee attacker, but there was free space - internal error");
        return false;
    }
    if (!battle_add(fighter, victim)) {
        remove_melee_combat_attacker(victim, fighter->index);
        return false;
    }
    return true;
}

TbBool add_waiting_attacker(struct Thing *fighter, struct Thing *victim)
{
    struct CreatureControl *figctrl;
    SYNCDBG(8,"Starting for %s and %s",thing_model_name(fighter),thing_model_name(victim));
    figctrl = creature_control_get_from_thing(fighter);
    if (figctrl->combat_flags) {
        SYNCLOG("The %s index %d in combat already - waiting",thing_model_name(fighter),(int)fighter->index);
    }
    figctrl->combat_flags |= 0x04;
    figctrl->battle_victim_idx = victim->index;
    figctrl->long_9E = victim->field_9;
    if (!battle_add(fighter, victim)) {
        return false;
    }
    return true;
}

long creature_has_ranged_weapon(struct Thing *thing)
{
    return _DK_creature_has_ranged_weapon(thing);
}

void set_creature_combat_state(struct Thing *fighter1, struct Thing *fighter2, long a3)
{
    struct CreatureControl *fig1ctrl;
    struct CreatureControl *fig2ctrl;
    struct CreatureStats *crstat;
    SYNCDBG(8,"Starting for %s and %s",thing_model_name(fighter1),thing_model_name(fighter2));
    //_DK_set_creature_combat_state(fighter1, fighter2, a3); return;
    fig1ctrl = creature_control_get_from_thing(fighter1);
    fig2ctrl = creature_control_get_from_thing(fighter2);
    {
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(fighter1->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->fights_num++;
        }
    }
    fig1ctrl->byte_A7 = a3;
    if ((fig2ctrl->combat_flags & (CmbtF_Melee|CmbtF_Ranged)) == 0)
    {
      if (is_my_player_number(fighter1->owner))
      {
          if (is_my_player_number(fighter2->owner)) {
              output_message(20, 400, 1);
          } else {
              output_message(14, 400, 1);
          }
      } else
      {
          if (is_my_player_number(fighter2->owner)) {
            output_message(13, 400, 1);
          }
      }
    }
    if (a3 == 2)
    {
      if ( add_ranged_attacker(fighter1, fighter2) )
      {
          play_creature_sound(fighter1, 11, 3, 0);
          fig1ctrl->combat_state_id = 2;
      } else
      {
          add_waiting_attacker(fighter1, fighter2);
          fig1ctrl->combat_state_id = 1;
      }
      return;
    }
    crstat = creature_stats_get_from_thing(fighter1);
    if ( (crstat->attack_preference == 2) && creature_has_ranged_weapon(fighter1) )
    {
        if ( add_ranged_attacker(fighter1, fighter2) )
        {
            play_creature_sound(fighter1, 11, 3, 0);
            fig1ctrl->combat_state_id = 2;
            return;
        }
    }
    if ( add_melee_attacker(fighter1, fighter2) )
    {
        play_creature_sound(fighter1, 11, 3, 0);
        fig1ctrl->combat_state_id = 3;
    }
    if ( creature_has_ranged_weapon(fighter1) && add_ranged_attacker(fighter1, fighter2) )
    {
        play_creature_sound(fighter1, 11, 3, 0);
        fig1ctrl->combat_state_id = 2;
    } else
    {
        add_waiting_attacker(fighter1, fighter2);
        fig1ctrl->combat_state_id = 1;
    }
}

long set_creature_in_combat_to_the_death(struct Thing *fighter1, struct Thing *fighter2, long a3)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(fighter1);
    if (cctrl->combat_flags != 0)
    {
        WARNLOG("The %s index %d in combat already - adding till death",thing_model_name(fighter1),(int)fighter1->index);
    }
    if (external_set_thing_state(fighter1, CrSt_CreatureInCombat))
    {
        set_creature_combat_state(fighter1, fighter2, a3);
        cctrl->field_AA = 0;
        cctrl->fight_til_death = 1;
        return true;
    }
    return false;
}

long find_fellow_creature_to_fight_in_room(struct Thing *fighter, struct Room *room,long crmodel, struct Thing **enemytng)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    long dist,combat_factor;
    unsigned long k;
    int i;
    SYNCDBG(8,"Starting");
    dungeon = get_players_num_dungeon(fighter->owner);
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature detected");
        break;
      }
      i = cctrl->players_next_creature_idx;
      // Thing list loop body
      if ( (thing->model == crmodel) && (cctrl->combat_flags == 0) )
      {
          if ( ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & TF1_Unkn02) == 0) )
          {
              if ((thing != fighter) && (get_room_thing_is_on(thing) == room))
              {
                  dist = get_combat_distance(fighter, thing);
                  combat_factor = creature_can_have_combat_with_creature(fighter, thing, dist, 0, 0);
                  if (combat_factor > 0)
                  {
                      *enemytng = thing;
                      return combat_factor;
                  }
              }
          }
      }
      // Thing list loop body ends
      k++;
      if (k > CREATURES_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
    }
    SYNCDBG(19,"Finished");
    *enemytng = INVALID_THING;
    return 0;
}

short cleanup_combat(struct Thing *thing)
{
    //return _DK_cleanup_combat(thing);
    remove_all_traces_of_combat(thing);
    return 0;
}

short cleanup_door_combat(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_cleanup_door_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->combat_flags &= ~0x10;
    cctrl->battle_victim_idx = 0;
    return 1;

}

short cleanup_object_combat(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_cleanup_object_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->combat_flags &= ~0x08;
    cctrl->battle_victim_idx = 0;
    return 1;
}

short creature_combat_flee(struct Thing *thing)
{
  return _DK_creature_combat_flee(thing);
}

short creature_door_combat(struct Thing *thing)
{
  return _DK_creature_door_combat(thing);
}

long combat_enemy_exists(struct Thing *thing, struct Thing *enemy)
{
    struct CreatureControl *cctrl;
    struct CreatureControl *enmcctrl;
    cctrl = creature_control_get_from_thing(thing);
    if ( (!thing_exists(enemy)) || (cctrl->long_9E != enemy->field_9) )
    {
        SYNCDBG(8,"Enemy creature doesn't exist");
        return 0;
    }
    enmcctrl = creature_control_get_from_thing(enemy);
    if (creature_control_invalid(enmcctrl) && (enemy->class_id != TCls_Object) && (enemy->class_id != TCls_Door))
    {
        ERRORLOG("No control structure - C%d M%d GT%ld CA%d", (int)enemy->class_id,
            (int)enemy->model, (long)game.play_gameturn, (int)thing->field_9);
        return 0;
    }
    return 1;
}

long creature_is_most_suitable_for_combat(struct Thing *thing, struct Thing *enmtng)
{
    return _DK_creature_is_most_suitable_for_combat(thing, enmtng);
}

long check_for_valid_combat(struct Thing *thing, struct Thing *enmtng)
{
    return _DK_check_for_valid_combat(thing, enmtng);
}

long combat_type_is_choice_of_creature(struct Thing *thing, long cmbtyp)
{
    return _DK_combat_type_is_choice_of_creature(thing, cmbtyp);
}

long get_best_ranged_offensive_weapon(struct Thing *thing, long a2)
{
    return _DK_get_best_ranged_offensive_weapon(thing, a2);
}

long ranged_combat_move(struct Thing *thing, struct Thing *enmtng, long a3, long a4)
{
    return _DK_ranged_combat_move(thing, enmtng, a3, a4);
}

long get_best_melee_offensive_weapon(struct Thing *thing, long a2)
{
    return _DK_get_best_melee_offensive_weapon(thing, a2);
}

long melee_combat_move(struct Thing *thing, struct Thing *enmtng, long a3, long a4)
{
    return _DK_melee_combat_move(thing, enmtng, a3, a4);
}

TbBool creature_scared(struct Thing *thing, struct Thing *enemy)
{
    struct CreatureControl *cctrl;
    if (thing_is_invalid(enemy))
    {
        ERRORLOG("Thing %d enemy is invalid",(int)thing->index);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->fight_til_death)
    {
        return false;
    }
    return creature_is_actually_scared(thing, enemy);
}

TbBool creature_in_flee_zone(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    unsigned long dist;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Creature index %d has invalid control",(int)thing->index);
        return false;
    }
    dist = get_2d_box_distance(&thing->mappos, &cctrl->pos_288);
    return (dist < 1536);
    //!!!!!!!! return (dist < gameadd.flee_zone_radius);
}

TbBool creature_too_scared_for_combat(struct Thing *thing, struct Thing *enemy)
{
    //get_combat_distance(thing, enemy);
    if (!creature_scared(thing, enemy))
    {
        return false;
    }
    if (creature_in_flee_zone(thing))
    {
        return false;
    }
    return true;
}

void remove_waiting_attacker(struct Thing *fighter)
{
    _DK_remove_waiting_attacker(fighter);
}

void remove_attacker(struct Thing *fighter, struct Thing *victim)
{
    struct CreatureControl *figctrl;
    figctrl = creature_control_get_from_thing(fighter);
    if ( (figctrl->combat_flags & CmbtF_Melee) != 0 )
    {
        remove_melee_attacker(fighter, victim);
    } else
    if ( (figctrl->combat_flags & CmbtF_Ranged) != 0 )
    {
        remove_ranged_attacker(fighter, victim);
    } else
    if ( (figctrl->combat_flags & CmbtF_Waiting) != 0 )
    {
        remove_waiting_attacker(fighter);
    }
}

void cleanup_battle_leftovers(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //_DK_cleanup_battle_leftovers(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->battle_id > 0)
        battle_remove(thing);
}

long remove_all_traces_of_combat(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *victim;
    //return _DK_remove_all_traces_of_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    if ( (cctrl->combat_flags != 0) && (cctrl->battle_victim_idx > 0) )
    {
        victim = thing_get(cctrl->battle_victim_idx);
        remove_attacker(thing, victim);
    }
    remove_all_ranged_combat_attackers(thing);
    remove_all_melee_combat_attackers(thing);
    cleanup_battle_leftovers(thing);
    return 1;
}

void change_current_combat(struct Thing *fighter, struct Thing *victim, long possible_combat)
{
    struct CreatureControl *figctrl;
    struct Thing *oldvictm;
    figctrl = creature_control_get_from_thing(fighter);
    oldvictm = thing_get(figctrl->battle_victim_idx);
    remove_attacker(fighter, oldvictm);
    set_creature_combat_state(fighter, victim, possible_combat);
}

long creature_has_spare_slot_for_combat(struct Thing *fighter, struct Thing *victim, long combat_kind)
{
    return _DK_creature_has_spare_slot_for_combat(fighter, victim, combat_kind);
}

long change_creature_with_existing_attacker(struct Thing *fighter, struct Thing *victim, long combat_kind)
{
    return _DK_change_creature_with_existing_attacker(fighter, victim, combat_kind);
}

long check_for_possible_combat(struct Thing *crtng, struct Thing **battltng)
{
    return _DK_check_for_possible_combat(crtng, battltng);
}

long check_for_better_combat(struct Thing *fighter)
{
    struct CreatureControl *figctrl;
    struct Thing *victim;
    long combat_kind;
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(fighter),(int)fighter->index);
    //return _DK_check_for_better_combat(fighter);
    figctrl = creature_control_get_from_thing(fighter);
    // Allow the switch only once per 8 turns
    if ((game.play_gameturn + fighter->index) & 7)
        return 0;
    combat_kind = check_for_possible_combat(fighter, &victim);
    if (combat_kind == 0)
        return 1;
    if ( (figctrl->battle_victim_idx != victim->index) || !combat_type_is_choice_of_creature(fighter, combat_kind) )
    {
        change_current_combat(fighter, victim, combat_kind);
    } else
    {
        if (figctrl->combat_state_id != 1)
            return 0;
        if ( !creature_has_spare_slot_for_combat(fighter, victim, combat_kind) )
        {
            if ( !change_creature_with_existing_attacker(fighter, victim, combat_kind) )
                return 0;
            return 1;
        }
        change_current_combat(fighter, victim, combat_kind);
    }
    return 1;
}

long waiting_combat_move(struct Thing *fighter, struct Thing *enemy, long a1, long a2)
{
    return _DK_waiting_combat_move(fighter, enemy, a1, a2);
}

void creature_in_combat_wait(struct Thing *thing)
{
    struct Thing *enemy;
    struct CreatureControl *cctrl;
    long dist;
    SYNCDBG(19,"Starting for %s",thing_model_name(thing));
    //_DK_creature_in_combat_wait(thing);
    if ( check_for_better_combat(thing) ) {
        return;
    }
    // For creatures which are not special diggers, check to attack dungeon heart once every 8 turns
    if ( (((game.play_gameturn+thing->index) & 7) == 0) && ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0) )
    {
        struct Thing *heartng;
        heartng = get_enemy_dungeon_heart_creature_can_see(thing);
        if (!thing_is_invalid(heartng))
        {
            if ( set_creature_object_combat(thing, heartng) ) {
                return;
            }
        }
    }
    // Check if we're best combat partner for the enemy
    long combat_valid;
    cctrl = creature_control_get_from_thing(thing);
    enemy = thing_get(cctrl->battle_victim_idx);
    if ( !creature_is_most_suitable_for_combat(thing, enemy) ) {
        set_start_state(thing);
        return;
    }
    combat_valid = check_for_valid_combat(thing, enemy);
    if ( !combat_type_is_choice_of_creature(thing, combat_valid) ) {
        set_start_state(thing);
        return;
    }
    dist = get_combat_distance(thing, enemy);
    waiting_combat_move(thing, enemy, dist, 49);
}

void creature_in_ranged_combat(struct Thing *thing)
{
    SYNCDBG(19,"Starting for %s",thing_model_name(thing));
    struct CreatureControl *cctrl;
    struct Thing *enmtng;
    long dist, cmbtyp, weapon;
    //_DK_creature_in_ranged_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    enmtng = thing_get(cctrl->battle_victim_idx);
    if (!creature_is_most_suitable_for_combat(thing, enmtng))
    {
        set_start_state(thing);
        return;
    }
    cmbtyp = check_for_valid_combat(thing, enmtng);
    if (!combat_type_is_choice_of_creature(thing, cmbtyp))
    {
        set_start_state(thing);
        return;
    }
    dist = get_combat_distance(thing, enmtng);
    weapon = get_best_ranged_offensive_weapon(thing, dist);
    if (weapon == 0)
    {
        set_start_state(thing);
        return;
    }
    if (!ranged_combat_move(thing, enmtng, dist, 49))
    {
        return;
    }
    if (weapon > 0)
    {
        set_creature_instance(thing, weapon, 1, enmtng->index, 0);
    }
}

void creature_in_melee_combat(struct Thing *thing)
{
    SYNCDBG(19,"Starting for %s",thing_model_name(thing));
    struct CreatureControl *cctrl;
    struct Thing *enmtng;
    long dist, cmbtyp, weapon;
    //_DK_creature_in_melee_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    enmtng = thing_get(cctrl->battle_victim_idx);
    if (!creature_is_most_suitable_for_combat(thing, enmtng))
    {
        set_start_state(thing);
        return;
    }
    cmbtyp = check_for_valid_combat(thing, enmtng);
    if (!combat_type_is_choice_of_creature(thing, cmbtyp))
    {
        set_start_state(thing);
        return;
    }
    dist = get_combat_distance(thing, enmtng);
    weapon = get_best_melee_offensive_weapon(thing, dist);
    if (weapon == 0)
    {
        set_start_state(thing);
        return;
    }
    if (!melee_combat_move(thing, enmtng, dist, 49))
    {
        return;
    }
    if (weapon > 0)
    {
        set_creature_instance(thing, weapon, 1, enmtng->index, 0);
    }
}

short creature_in_combat(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *enmtng;
    CombatState combat_func;
    SYNCDBG(9,"Starting for %s",thing_model_name(thing));
    //return _DK_creature_in_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    enmtng = thing_get(cctrl->battle_victim_idx);
    if (!combat_enemy_exists(thing, enmtng))
    {
        set_start_state(thing);
        return 0;
    }
    if (creature_too_scared_for_combat(thing, enmtng))
    {
        if (!external_set_thing_state(thing, CrSt_CreatureCombatFlee))
        {
            ERRORLOG("Cannot get %s index %d into flee",thing_model_name(thing),(int)thing->index);
            return 0;
        }
        cctrl->field_28E = game.play_gameturn;
        return 0;
    }
    if (cctrl->combat_state_id < sizeof(combat_state)/sizeof(combat_state[0]))
        combat_func = combat_state[cctrl->combat_state_id];
    else
        combat_func = NULL;
    if (combat_func != NULL)
    {
        combat_func(thing);
        return 1;
    }
    ERRORLOG("No valid fight state %d in %s index %d",(int)cctrl->combat_state_id,thing_model_name(thing),(int)thing->index);
    set_start_state(thing);
    return 0;
}

short creature_object_combat(struct Thing *thing)
{
    return _DK_creature_object_combat(thing);
}

long creature_look_for_combat(struct Thing *thing)
{
    struct Thing *enmtng;
    struct CreatureControl *cctrl;
    long combat_kind;
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    //return _DK_creature_look_for_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    combat_kind = check_for_possible_combat(thing, &enmtng);
    if (combat_kind <= 0)
    {
        if ( (cctrl->opponents_melee_count == 0) && (cctrl->opponents_ranged_count == 0) ) {
            return 0;
        }
        if ( !external_set_thing_state(thing, CrSt_CreatureCombatFlee) ) {
            return 0;
        }
        setup_combat_flee_position(thing);
        cctrl->field_28E = game.play_gameturn;
        return 1;
    }

    if (cctrl->combat_flags != 0)
    {
        if (get_combat_state_for_combat(thing, enmtng, combat_kind) == 1) {
          return 0;
        }
    }


    if ( !creature_too_scared_for_combat(thing, enmtng) )
    {
        set_creature_in_combat(thing, enmtng, combat_kind);
        return 1;
    }

    if ( ((cctrl->spell_flags & 0x20) != 0) && (cctrl->field_AF <= 0) )
    {
      if ( (cctrl->opponents_melee_count == 0) && (cctrl->opponents_ranged_count == 0) ) {
          return 0;
      }
    }
    if ( !external_set_thing_state(thing, CrSt_CreatureCombatFlee) ) {
        return 0;
    }
    setup_combat_flee_position(thing);
    cctrl->field_28E = game.play_gameturn;
    return 1;
}

long creature_retreat_from_combat(struct Thing *thing1, struct Thing *thing2, long a3, long a4)
{
    struct CreatureControl *cctrl1;
    struct Coord3d pos;
    long dist_x,dist_y;
    long i;

    cctrl1 = creature_control_get_from_thing(thing1);
    dist_x = thing2->mappos.x.val - thing1->mappos.x.val;
    dist_y = thing2->mappos.y.val - thing1->mappos.y.val;
    if (a4 && ((cctrl1->combat_flags & 0x18) == 0))
    {
        pos.x.val = thing1->mappos.x.val - dist_x;
        pos.y.val = thing1->mappos.y.val - dist_y;
        pos.z.val = get_thing_height_at(thing1, &pos);
        if (creature_move_to(thing1, &pos, get_creature_speed(thing1), 0, 1) != -1)
        {
           return 1;
        }
    }
    // First try
    pos.x.val = thing1->mappos.x.val;
    pos.y.val = thing1->mappos.y.val;
    if (abs(dist_y) >= abs(dist_x))
    {
      if (dist_y <= 0)
        pos.y.val += 256;
      else
        pos.y.val -= 256;
    } else
    {
      if (dist_x <= 0)
        pos.x.val += 256;
      else
        pos.x.val -= 256;
    }
    pos.z.val = get_thing_height_at(thing1, &pos);

    if (setup_person_move_backwards_to_coord(thing1, &pos, 0))
    {
      thing1->continue_state = a3;
      return 1;
    }
    // Second try
    pos.x.val = thing1->mappos.x.val;
    pos.y.val = thing1->mappos.y.val;
    if (ACTION_RANDOM(2) == 0)
        i = 1;
    else
        i = -1;
    if (abs(dist_y) >= abs(dist_x))
      pos.x.val += 768 * i;
    else
      pos.y.val += 768 * i;
    pos.z.val = get_thing_height_at(thing1, &pos);
    if (setup_person_move_backwards_to_coord(thing1, &pos, 0))
    {
      thing1->continue_state = a3;
      return 1;
    }
    return 1;
}

short creature_attack_rooms(struct Thing *thing)
{
    return _DK_creature_attack_rooms(thing);
}

short creature_attempt_to_damage_walls(struct Thing *thing)
{
    return _DK_creature_attempt_to_damage_walls(thing);
}

short creature_damage_walls(struct Thing *thing)
{
    return _DK_creature_damage_walls(thing);
}

/******************************************************************************/
