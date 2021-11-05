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
#include "bflib_planar.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_battle.h"
#include "creature_instances.h"
#include "creature_senses.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_shots.h"
#include "thing_navigate.h"
#include "creature_states_lair.h"
#include "power_hand.h"
#include "room_data.h"
#include "room_jobs.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "engine_redraw.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_creature_attempt_to_damage_walls(struct Thing *creatng);
DLLIMPORT short _DK_creature_damage_walls(struct Thing *creatng);
/******************************************************************************/
TbBool combat_has_line_of_sight(const struct Thing *creatng, const struct Thing *enmtng, MapCoordDelta enmdist);
/******************************************************************************/
const CombatState combat_state[] = {
    NULL,
    creature_in_combat_wait,
    creature_in_ranged_combat,
    creature_in_melee_combat,
};

const CombatState combat_object_state[] = {
    NULL,
    combat_object_state_melee_combat,
    combat_object_state_ranged_combat,
};

const CombatState combat_door_state[] = {
    NULL,
    combat_door_state_melee_combat,
    combat_door_state_ranged_combat,
};

struct CombatWeapon offensive_weapon[] = {
    {CrInst_FREEZE,                 156, LONG_MAX},
    {CrInst_FEAR,                   156, LONG_MAX},
    {CrInst_CAST_SPELL_DISEASE,     156, LONG_MAX},
    {CrInst_CAST_SPELL_CHICKEN,     156, LONG_MAX},
    {CrInst_CAST_SPELL_TIME_BOMB,   768, LONG_MAX},
    {CrInst_LIZARD,                1000, LONG_MAX},
    {CrInst_FIRE_BOMB,              768, LONG_MAX},
    {CrInst_LIGHTNING,              768, LONG_MAX},
    {CrInst_HAILSTORM,              156, LONG_MAX},
    {CrInst_POISON_CLOUD,           156, LONG_MAX},
    {CrInst_DRAIN,                  156, LONG_MAX},
    {CrInst_SLOW,                   156, LONG_MAX},
    {CrInst_NAVIGATING_MISSILE,     156, LONG_MAX},
    {CrInst_MISSILE,                156, LONG_MAX},
    {CrInst_FIREBALL,               156, LONG_MAX},
    {CrInst_FIRE_ARROW,             156, LONG_MAX},
    {CrInst_WIND,                     0, LONG_MAX},
    {CrInst_WORD_OF_POWER,            0, 284},
    {CrInst_FART,                     0, 284},
    {CrInst_FLAME_BREATH,           156, 284},
    {CrInst_SWING_WEAPON_SWORD,       0, 284},
    {CrInst_SWING_WEAPON_FIST,        0, 284},
    {CrInst_NULL,                     0,   0},
};

const signed char pos_calcs[][2] = {
    {1, 0}, {1, 1}, {1, 1}, {0, 1},
    {0, 1},{-1, 1},{-1, 1},{-1, 0},
   {-1, 0},{-1,-1},{-1,-1}, {0,-1},
    {0,-1}, {1,-1}, {1,-1}, {1, 0},
};

#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool creature_is_being_attacked_by_enemy_player(struct Thing *fightng)
{
    long oppn_idx;
    TRACE_THING(fightng);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    // Check any enemy creature is in melee opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        struct Thing* enmtng = thing_get(figctrl->opponents_melee[oppn_idx]);
        if (!thing_is_invalid(enmtng))
        {
            if (players_are_enemies(fightng->owner,enmtng->owner)) {
                return true;
            }
        }
    }
    // Check any enemy creature is in ranged opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        struct Thing* enmtng = thing_get(figctrl->opponents_ranged[oppn_idx]);
        if (!thing_is_invalid(enmtng))
        {
            if (players_are_enemies(fightng->owner,enmtng->owner)) {
                return true;
            }
        }
    }
    return false;
}

TbBool creature_is_being_attacked_by_enemy_creature_not_digger(struct Thing *fightng)
{
    long oppn_idx;
    TRACE_THING(fightng);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    // Check any enemy creature is in melee opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        struct Thing* enmtng = thing_get(figctrl->opponents_melee[oppn_idx]);
        if (!thing_is_invalid(enmtng) && !thing_is_creature_special_digger(enmtng))
        {
            if (players_are_enemies(fightng->owner,enmtng->owner)) {
                return true;
            }
        }
    }
    // Check any enemy creature is in ranged opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        struct Thing* enmtng = thing_get(figctrl->opponents_ranged[oppn_idx]);
        if (!thing_is_invalid(enmtng) && !thing_is_creature_special_digger(enmtng))
        {
            if (players_are_enemies(fightng->owner,enmtng->owner)) {
                return true;
            }
        }
    }
    return false;
}

TbBool creature_can_see_combat_path(const struct Thing *creatng, const struct Thing *enmtng, MapCoordDelta dist)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if (dist > subtile_coord(crstat->visual_range,0)) {
        return false;
    }
    if (!jonty_creature_can_see_thing_including_lava_check(creatng, enmtng)) {
        return false;
    }
    return true;
}

TbBool creature_will_do_combat(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // Creature turned to chicken is defenseless
    if (creature_affected_by_spell(thing, SplK_Chicken))
        return false;
    // Neutral creatures won't fight
    if (is_neutral_thing(thing))
        return false;
    if ((cctrl->flgfield_1 & CCFlg_NoCompControl) != 0)
        return false;
    // Frozen creature cannot attack
    if (creature_affected_by_spell(thing, SplK_Freeze))
        return false;
    return can_change_from_state_to(thing, thing->active_state, CrSt_CreatureInCombat);
}

long get_combat_distance(const struct Thing *thing, const struct Thing *enmtng)
{
    long dist = get_2d_box_distance(&thing->mappos, &enmtng->mappos);
    long avgc = ((long)enmtng->clipbox_size_xy + (long)thing->clipbox_size_xy) / 2;
    if (dist < avgc)
        return 0;
    return dist - avgc;
}

/**
 * Returns if a creature has attackers of different kind than given creature model.
 * @param fightng The thing which enemies are to be checked.
 * @param enmodel Enemy creature model which should be ignored.
 * @return
 */
TbBool creature_has_other_attackers(const struct Thing *fightng, ThingModel enmodel)
{
    long oppn_idx;
    TRACE_THING(fightng);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    // Check any enemy creature is in melee opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        struct Thing* enmtng = thing_get(figctrl->opponents_melee[oppn_idx]);
        if (!thing_is_invalid(enmtng))
        {
            if (enmtng->model != enmodel) {
                return true;
            }
        }
    }
    // Check any enemy creature is in ranged opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        struct Thing* enmtng = thing_get(figctrl->opponents_ranged[oppn_idx]);
        if (!thing_is_invalid(enmtng))
        {
            if (enmtng->model != enmodel) {
                return true;
            }
        }
    }
    return false;
}

TbBool creature_is_actually_scared(const struct Thing *creatng, const struct Thing *enmtng)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    struct CreatureStats* enmstat = creature_stats_get_from_thing(enmtng);
    // Neutral creatures are not easily scared, as they shouldn't have enemies
    if (is_neutral_thing(creatng))
        return false;
    // Creature with fear 101 are scared of everything other that their own model
    if (crstat->fear_wounded >= 101)
    {
        if (enmtng->model != creatng->model)
            return true;
        if (creature_has_other_attackers(creatng, creatng->model))
            return true;
        // But if faced only creatures of same model, they will fight with no fear
        return false;
    }
    // Creatures are scared if their health drops lower than
    // fear_wounded percent of base health
    long fear;
    if (player_creature_tends_to(creatng->owner,CrTend_Flee)) {
        // In flee mode, use full fear value
        fear = crstat->fear_wounded * 10;
    } else {
        fear = 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    HitPoints crmaxhealth = cctrl->max_health;
    HitPoints enmaxhealth = enmctrl->max_health;
    if (enmaxhealth > 15000)
    {
        enmaxhealth = 15000;
    }
    if (creatng->health < (fear * (long long)crmaxhealth) / 1000)
    {
        SYNCDBG(8,"The %s index %d is scared due to low health (%ld/%ld)",thing_model_name(creatng),(int)creatng->index,(long)creatng->health,crmaxhealth);
        return true;
    }
    // Units dropped will fight stronger units for a bit
    if ((cctrl->wait_to_turn > (long)game.play_gameturn))
    {
        return false;
    }
    // If the enemy is way stronger, a creature may be scared anyway
    fear = crstat->fear_stronger;
    long long enmstrength = LbSqrL(project_melee_damage(enmtng)) * (enmstat->fearsome_factor) / 100 * ((long long)enmaxhealth + (long long)enmtng->health) / 2;
    long long ownstrength = LbSqrL(project_melee_damage(creatng)) * (crstat->fearsome_factor) / 100 * ((long long)crmaxhealth + (long long)creatng->health) / 2;
    if (enmstrength >= (fear * ownstrength) / 100)
    {
        // check if there are allied creatures nearby enemy; assume that such creatures are multiplying strength of the creature we're checking
        long support_count = count_creatures_near_and_owned_by_or_allied_with(enmtng->mappos.x.val, enmtng->mappos.y.val, 12, creatng->owner);
        if (support_count <= 3) // Never flee when in groups of 4 or bigger
        {
            ownstrength *= support_count;
            if (enmstrength >= (fear * ownstrength) / 100)
            {
                SYNCDBG(8,"The %s index %d is scared due to enemy %s strength (%d vs. %d)",thing_model_name(creatng),(int)creatng->index,thing_model_name(enmtng),(int)ownstrength,(int)enmstrength);
                return true;
            }
        }
    }
    return false;
}

long creature_can_move_to_combat(struct Thing *fightng, struct Thing *enmtng)
{
    long result = get_thing_navigation_distance(fightng, &enmtng->mappos, 0);
    if (result == -1 || result == LONG_MAX || result >= subtile_coord(21, 0))
    {
        return -1;
  }
  return result;
}

/**
 * Gives combat type a creature can have with another creature based on its senses.
 *
 * @param fightng
 * @param enmtng
 * @param dist
 * @param move_on_ground
 * @param set_if_seen
 * @return Gives AttckT_Unset if creature does not sense the opponent,
 *     AttckT_Ranged if creature is in sight but cannot be approached,
 *     AttckT_Melee if creature is discovered and can be reached by walk.
 */
CrAttackType creature_can_have_combat_with_creature(struct Thing *fightng, struct Thing *enmtng, long dist, long move_on_ground, long set_if_seen)
{
    SYNCDBG(19,"Starting for %s index %d vs %s index %d",thing_model_name(fightng),(int)fightng->index,thing_model_name(enmtng),(int)enmtng->index);
    TRACE_THING(fightng);
    TRACE_THING(enmtng);
    TbBool can_see = false;
    if (creature_can_hear_within_distance(fightng, dist))
    {
        // We can have a melee combat if we hear an enemy and we can move to it
        if (move_on_ground)
        {
            if (creature_can_move_to_combat(fightng, enmtng) >= 0) {
                return AttckT_Melee;
            }
        } else
        {
            if (slab_wall_hug_route(fightng, &enmtng->mappos, 8) > 0) {
                return AttckT_Melee;
            }
        }
        // If we cannot move to the enemy, then ranged attack is the only option, and we need line of sight
        if (!creature_has_ranged_weapon(fightng)) {
            // Checking line of sight is expensive - in case we don't have ranged weapon, skip it
            return AttckT_Unset;
        }
        can_see = creature_can_see_combat_path(fightng, enmtng, dist);
        if (can_see <= 0) {
            return AttckT_Unset;
        }
    }
    else
    {
        can_see = creature_can_see_combat_path(fightng, enmtng, dist);
        if (!can_see) {
          return AttckT_Unset;
        }
        // If we can see it, assume that we can reach it
        //TODO COMBAT is it acceptable to assume we can do melee combat here? Why no seen_enemy update?
        return AttckT_Melee;
    }
    if (set_if_seen)
    {
        struct CreatureControl* fcctrl = creature_control_get_from_thing(fightng);
        fcctrl->combat.seen_enemy_los = can_see;
        fcctrl->combat.seen_enemy_idx = enmtng->index;
        fcctrl->combat.seen_enemy_turn = game.play_gameturn;
    }
    return AttckT_Ranged;
}

void remove_thing_from_battle_list(struct Thing *thing)
{
    SYNCDBG(9,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (!thing_is_creature(thing) || creature_control_invalid(cctrl)) {
      ERRORLOG("Creature should have been already removed due to death");
    }
    struct CreatureBattle* battle = creature_battle_get(cctrl->battle_id);
    // Change next index in prev creature
    unsigned short partner_id = cctrl->battle_prev_creatr;
    if (cctrl->battle_next_creatr > 0)
    {
        struct Thing* attctng = thing_get(cctrl->battle_next_creatr);
        TRACE_THING(attctng);
        struct CreatureControl* attcctrl = creature_control_get_from_thing(attctng);
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
        struct Thing* attctng = thing_get(cctrl->battle_prev_creatr);
        TRACE_THING(attctng);
        struct CreatureControl* attcctrl = creature_control_get_from_thing(attctng);
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
        ERRORLOG("Removing %s index %d from battle, but counter is 0",thing_model_name(thing),(int)thing->index);
    }
    SYNCDBG(19,"Finished");
}

void insert_thing_in_battle_list(struct Thing *thing, BattleIndex battle_id)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    cctrl->battle_next_creatr = battle->last_creatr;
    cctrl->battle_prev_creatr = 0;
    cctrl->battle_id = battle_id;
    if (battle->first_creatr <= 0) {
        battle->first_creatr = thing->index;
    }
    if (battle->last_creatr > 0) {
        struct Thing* enmtng = thing_get(battle->last_creatr);
        struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
        enmctrl->battle_prev_creatr = thing->index;
    }
    battle->last_creatr = thing->index;
    battle->fighters_num++;
}

long count_creatures_really_in_combat(BattleIndex battle_id)
{
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle))
        return 0;
    long count = 0;
    unsigned long k = 0;
    int i = battle->first_creatr;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->battle_prev_creatr;
        // Per thing code starts
        if (cctrl->combat_flags != 0) {
          count++;
        }
        // Per thing code ends
        k++;
        if (k > CREATURES_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    return count;
}

void cleanup_battle(BattleIndex battle_id)
{
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle))
        return;
    long count = count_creatures_really_in_combat(battle_id);
    if (count == 0)
    {
        // If no creature is really fighting, dissolve the battle
        unsigned long k = 0;
        while (battle->first_creatr)
        {
            struct Thing* thing = thing_get(battle->first_creatr);
            TRACE_THING(thing);
            remove_thing_from_battle_list(thing);
            k++;
            if (k > CREATURES_COUNT)
            {
              ERRORLOG("Infinite loop detected when sweeping creatures list");
              break;
            }
        }
        SYNCDBG(7,"Removed %d wanderers from battle %d",(int)k,(int)battle_id);
    } else
    {
        SYNCDBG(7,"There are still %d participants in battle %d",(int)count,(int)battle_id);
    }
}

void update_battle_events(BattleIndex battle_id)
{
    unsigned short owner_flags = 0;
    MapCoord map_x = -1;
    MapCoord map_y = -1;
    unsigned long k = 0;
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    int i = battle->first_creatr;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->battle_prev_creatr;
        // Per thing code starts
        owner_flags |= (1 << thing->owner);
        map_x = thing->mappos.x.val;
        map_y = thing->mappos.y.val;
        // Per thing code ends
        k++;
        if (k > CREATURES_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        if ((i == game.hero_player_num) || (i == game.neutral_player_num))
            continue;
        if ((1 << i) & owner_flags) {
            if ((1 << i) == owner_flags) {
                event_create_event_or_update_old_event(map_x, map_y, EvKind_FriendlyFight, i, 0);
            } else {
                event_create_event_or_update_old_event(map_x, map_y, EvKind_EnemyFight, i, 0);
            }
        }
    }
}

TbBool battle_with_creature_of_player(PlayerNumber plyr_idx, BattleIndex battle_id)
{
    unsigned long k = 0;
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    long i = battle->first_creatr;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->battle_prev_creatr;
        // Per thing code starts
        if (thing->owner == plyr_idx)
            return true;
        // Per thing code ends
        k++;
        if (k > CREATURES_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    return false;
}

/**
 * Checks if any of two things fights as enemy in given battle.
 * @param battle The battle to be checked.
 * @param tng1 First thing to be checked.
 * @param tng2 Second thing to be checked.
 * @return Gives true if any of things is fighting in given battle.
 */
TbBool battle_any_of_things_in_specific_battle(const struct CreatureBattle *battle, const struct Thing *tng1, const struct Thing *tng2)
{
    unsigned long k = 0;
    long i = battle->first_creatr;
    while (i != 0)
    {
        struct Thing* batltng = thing_get(i);
        TRACE_THING(batltng);
        struct CreatureControl* cctrl = creature_control_get_from_thing(batltng);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Invalid control of thing in battle, index %d.",(int)i);
            break;
        }
        i = cctrl->battle_prev_creatr;
        // Per battle creature code
        if (cctrl->combat_flags != 0)
        {
            struct Thing* attcktng = thing_get(cctrl->combat.battle_enemy_idx);
            TRACE_THING(attcktng);
            if (!thing_is_invalid(attcktng) && ((attcktng->index == tng1->index) || (attcktng->index == tng2->index)))
            {
                return true;
            }
        }
        // Per battle creature code ends
        k++;
        if (k >= CREATURES_COUNT) {
            ERRORLOG("Infinite loop in battle add");
            break;
        }
    }
    return false;
}

unsigned short find_battle_for_thing(const struct Thing *fighter, const struct Thing *enmtng)
{
    TRACE_THING(fighter);
    TRACE_THING(enmtng);
    unsigned short battle_id = 0;
    for (long i = 1; i < BATTLES_COUNT; i++) // Originally was 32, but I'm pretty sure there's 48 battles
    {
        struct CreatureBattle* battle = creature_battle_get(i);
        // If the battle exists, check who is fighting
        if (battle->fighters_num != 0)
        {
            if (battle_any_of_things_in_specific_battle(battle, fighter, enmtng)) {
                battle_id = i;
                break;
            }
        } else
        // If the battle is empty, remember its index - we might want first empty battle
        {
            if (battle_id <= 0)
              battle_id = i;
        }
    }
    if (battle_id <= 0) {
        ERRORLOG("No free battle structures");
    }
    return battle_id;
}

TbBool battle_add(struct Thing *fighter, struct Thing *enmtng)
{
    unsigned short battle_id;
    if (thing_is_invalid(fighter) || thing_is_invalid(enmtng))
    {
        ERRORLOG("Attempt to create battle with invalid creature!");
        return false;
    }
    SYNCDBG(9,"Starting for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enmtng),(int)enmtng->index);
    TRACE_THING(fighter);
    TRACE_THING(enmtng);
    { // Remove fighter from previous battle
        struct CreatureControl* figctrl = creature_control_get_from_thing(fighter);
        battle_id = figctrl->battle_id;
        if (battle_id > 0) {
            remove_thing_from_battle_list(fighter);
            cleanup_battle(battle_id);
        }
        if (figctrl->battle_id > 0) {
            ERRORLOG("Removing %s index %d from battle doesn't seem to have effect",thing_model_name(fighter),(int)fighter->index);
            return false;
        }
    }
    battle_id = 0;
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    if (creature_control_invalid(enmctrl)) {
        ERRORLOG("Invalid enemy %s index %d control",thing_model_name(enmtng),(int)enmtng->index);
        return false;
    }
    // Find a new battle to fight in, or use the one enemy is already in
    if (enmctrl->battle_id > 0)
    {
        battle_id = enmctrl->battle_id;
    } else
    {
        battle_id = find_battle_for_thing(fighter, enmtng);
    }
    if (battle_id <= 0) {
        ERRORLOG("No free battle structures");
        return false;
    }
    // Add both fighter and enemy to the new battle
    insert_thing_in_battle_list(fighter, battle_id);
    if (enmctrl->battle_id <= 0) {
        insert_thing_in_battle_list(enmtng, battle_id);
    }
    update_battle_events(battle_id);
    cleanup_battle(battle_id);
    SYNCDBG(12,"Finished");
    return true;
}

TbBool battle_remove(struct Thing *fightng)
{
    SYNCDBG(9,"Starting for %s index %d",thing_model_name(fightng),(int)fightng->index);
    TRACE_THING(fightng);
    {
        struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
        long battle_id = figctrl->battle_id;
        if (battle_id > 0) {
            remove_thing_from_battle_list(fightng);
            cleanup_battle(battle_id);
        } else {
            ERRORLOG("Attempt to remove %s index %d from battle when he isn't in one",thing_model_name(fightng),(int)fightng->index);
        }
        if (figctrl->battle_id > 0) {
            ERRORLOG("Removing %s index %d from battle doesn't seem to have effect",thing_model_name(fightng),(int)fightng->index);
            return false;
        }
    }
    event_update_on_battle_removal();
    return true;
}

TbBool add_ranged_combat_attacker(struct Thing *enmtng, unsigned short fighter_idx)
{
    long oppn_idx;
    TRACE_THING(enmtng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    // Check if the fighter is already in opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (enmctrl->opponents_ranged[oppn_idx] == fighter_idx) {
            WARNLOG("Fighter %s index %d already in opponents list",thing_model_name(enmtng),(int)enmtng->index);
            return true;
        }
    }
    // Find empty opponent slot
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (enmctrl->opponents_ranged[oppn_idx] == 0)
            break;
    }
    SYNCDBG(7,"Adding to %s index %d attacker %d index %d",thing_model_name(enmtng),(int)enmtng->index,(int)oppn_idx,(int)fighter_idx);
    if (oppn_idx >= COMBAT_RANGED_OPPONENTS_LIMIT)
        return false;
    // Add the opponent
    enmctrl->opponents_ranged_count++;
    enmctrl->opponents_ranged[oppn_idx] = fighter_idx;
    return true;
}

TbBool remove_ranged_combat_attacker(struct Thing *enmtng, unsigned short fighter_idx)
{
    long oppn_idx;
    TRACE_THING(enmtng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    for (oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (enmctrl->opponents_ranged[oppn_idx] == fighter_idx)
            break;
    }
    SYNCDBG(7,"Removing from %s index %d attacker %d index %d",thing_model_name(enmtng),(int)enmtng->index,(int)oppn_idx,(int)fighter_idx);
    if (oppn_idx >= COMBAT_RANGED_OPPONENTS_LIMIT)
        return false;
    enmctrl->opponents_ranged_count--;
    enmctrl->opponents_ranged[oppn_idx] = 0;
    return true;
}

TbBool add_melee_combat_attacker(struct Thing *enmtng, unsigned short fighter_idx)
{
    long oppn_idx;
    TRACE_THING(enmtng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    // Check if the fighter is already in opponents list
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (enmctrl->opponents_melee[oppn_idx] == fighter_idx) {
            WARNLOG("Fighter %s index %d already in opponents list",thing_model_name(enmtng),(int)enmtng->index);
            return true;
        }
    }
    // Find empty opponent slot
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (enmctrl->opponents_melee[oppn_idx] == 0)
            break;
    }
    if (oppn_idx >= COMBAT_MELEE_OPPONENTS_LIMIT)
        return false;
    // Add the opponent
    enmctrl->opponents_melee_count++;
    enmctrl->opponents_melee[oppn_idx] = fighter_idx;
    return true;
}

TbBool remove_melee_combat_attacker(struct Thing *enmtng, unsigned short fighter_idx)
{
    long oppn_idx;
    TRACE_THING(enmtng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    for (oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        if (enmctrl->opponents_melee[oppn_idx] == fighter_idx)
            break;
    }
    if (oppn_idx >= COMBAT_MELEE_OPPONENTS_LIMIT)
        return false;
    enmctrl->opponents_melee_count--;
    enmctrl->opponents_melee[oppn_idx] = 0;
    return true;
}

TbBool remove_melee_attacker(struct Thing *fightng, struct Thing *enmtng)
{
    TRACE_THING(fightng);
    TRACE_THING(enmtng);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    {
        struct Dungeon* dungeon = get_players_num_dungeon(fightng->owner);
        if ( !dungeon_invalid(dungeon) && (dungeon->fights_num > 0) ) {
            dungeon->fights_num--;
        } else {
            WARNLOG("Fight count incorrect while removing attacker %s index %d",thing_model_name(fightng),(int)fightng->index);
        }
    }
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    if (creature_control_invalid(enmctrl)) {
        ERRORLOG("Invalid enemy %s index %d control",thing_model_name(enmtng),(int)enmtng->index);
        return false;
    }
    if (has_melee_combat_attackers(enmtng))
    {
        if (!remove_melee_combat_attacker(enmtng, fightng->index)) {
            ERRORLOG("Cannot remove attacker - not in %s index %d opponents",thing_model_name(enmtng),(int)enmtng->index);
        }
    } else {
        WARNLOG("Cannot remove attacker - the %s index %d has no opponents",thing_model_name(enmtng),(int)enmtng->index);
    }
    figctrl->combat_flags &= ~CmbtF_Melee;
    figctrl->combat.battle_enemy_idx = 0;
    figctrl->combat.battle_enemy_crtn = 0;
    figctrl->fight_til_death = 0;
    delay_teleport(fightng);

    battle_remove(fightng);
    return true;
}

/**
 * Removes given fighter from combat with given enemy.
 * @param fighter
 * @param enmtng
 * @return
 */
TbBool remove_ranged_attacker(struct Thing *fightng, struct Thing *enmtng)
{
    TRACE_THING(fightng);
    TRACE_THING(enmtng);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    {
        struct Dungeon* dungeon = get_players_num_dungeon(fightng->owner);
        if (!dungeon_invalid(dungeon) && (dungeon->fights_num > 0)) {
            dungeon->fights_num--;
        } else {
            WARNLOG("Fight count incorrect while removing attacker %s index %d",thing_model_name(fightng),(int)fightng->index);
        }
    }
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    if (creature_control_invalid(enmctrl)) {
        ERRORLOG("Invalid enemy %s index %d control",thing_model_name(enmtng),(int)enmtng->index);
        return false;
    }
    if (has_ranged_combat_attackers(enmtng))
    {
        if (!remove_ranged_combat_attacker(enmtng, fightng->index)) {
            ERRORLOG("Cannot remove attacker - not in %s index %d opponents",thing_model_name(enmtng),(int)enmtng->index);
        }
    } else {
        WARNLOG("Cannot remove attacker - the %s index %d has no opponents",thing_model_name(enmtng),(int)enmtng->index);
    }
    figctrl->combat_flags &= ~CmbtF_Ranged;
    figctrl->combat.battle_enemy_idx = 0;
    figctrl->combat.battle_enemy_crtn = 0;
    figctrl->fight_til_death = 0;
    delay_teleport(fightng);

    battle_remove(fightng);
    return true;
}

long remove_all_melee_combat_attackers(struct Thing *victmtng)
{
    TRACE_THING(victmtng);
    long num = 0;
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victmtng);
    for (long oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        long fighter_idx = vicctrl->opponents_melee[oppn_idx];
        if (fighter_idx > 0) {
            struct Thing* fightng = thing_get(fighter_idx);
            TRACE_THING(fightng);
            if (remove_melee_attacker(fightng, victmtng)) {
                num++;
            }
        }
    }
    if (vicctrl->opponents_melee_count != 0) {
        ERRORLOG("Removed all opponents, but count is still %d",(int)vicctrl->opponents_melee_count);
        vicctrl->opponents_melee_count = 0;
    }
    SYNCDBG(8,"Removed %d attackers of %s index %d owner %d",(int)num,thing_model_name(victmtng),(int)victmtng->index,(int)victmtng->owner);
    return num;
}

long remove_all_ranged_combat_attackers(struct Thing *victmtng)
{
    TRACE_THING(victmtng);
    long num = 0;
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victmtng);
    for (long oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        long fighter_idx = vicctrl->opponents_ranged[oppn_idx];
        if (fighter_idx > 0) {
            struct Thing* fightng = thing_get(fighter_idx);
            TRACE_THING(fightng);
            if (remove_ranged_attacker(fightng, victmtng)) {
                num++;
            }
        }
    }
    if (vicctrl->opponents_ranged_count != 0) {
        ERRORLOG("Removed all opponents, but count is still %d",(int)vicctrl->opponents_ranged_count);
        vicctrl->opponents_ranged_count = 0;
    }
    SYNCDBG(8,"Removed %d attackers of %s index %d owner %d",(int)num,thing_model_name(victmtng),(int)victmtng->index,(int)victmtng->owner);
    return num;
}

long add_ranged_attacker(struct Thing *fighter, struct Thing *enemy)
{
    SYNCDBG(18,"Starting for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enemy),(int)enemy->index);
    TRACE_THING(fighter);
    TRACE_THING(enemy);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fighter);
    if (figctrl->combat_flags != 0)
    {
        if ((figctrl->combat_flags & CmbtF_Ranged) != 0) {
            SYNCDBG(8,"The %s index %d in ranged combat already - no action",thing_model_name(fighter),(int)fighter->index);
            return false;
        }
        SYNCDBG(8,"The %s index %d in combat already - adding ranged",thing_model_name(fighter),(int)fighter->index);
        return false; // We're not going to add anything
    }
    if (!can_add_ranged_combat_attacker(enemy)) {
        SYNCDBG(7,"Cannot add a ranged attacker to %s index %d - opponents limit reached",thing_model_name(fighter),(int)fighter->index);
        return false;
    }
    figctrl->combat_flags |= CmbtF_Ranged;
    figctrl->combat.battle_enemy_idx = enemy->index;
    figctrl->combat.battle_enemy_crtn = enemy->creation_turn;
    if (!add_ranged_combat_attacker(enemy, fighter->index)) {
        ERRORLOG("Cannot add a ranged attacker, but there was free space - internal error");
        figctrl->combat_flags &= ~CmbtF_Ranged;
        figctrl->combat.battle_enemy_idx = 0;
        figctrl->combat.battle_enemy_crtn = 0;
        figctrl->fight_til_death = 0;
        return false;
    }
    if (!battle_add(fighter, enemy)) {
        remove_ranged_combat_attacker(enemy, fighter->index);
        figctrl->combat_flags &= ~CmbtF_Ranged;
        figctrl->combat.battle_enemy_idx = 0;
        figctrl->combat.battle_enemy_crtn = 0;
        figctrl->fight_til_death = 0;
        return false;
    }
    return true;
}

long add_melee_attacker(struct Thing *fighter, struct Thing *enemy)
{
    SYNCDBG(18,"Starting for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enemy),(int)enemy->index);
    TRACE_THING(fighter);
    TRACE_THING(enemy);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fighter);
    if (figctrl->combat_flags != 0)
    {
        if ((figctrl->combat_flags & CmbtF_Melee) != 0) {
            SYNCDBG(8,"The %s index %d in melee combat already - no action",thing_model_name(fighter),(int)fighter->index);
            return false;
        }
        SYNCDBG(8,"The %s index %d in combat already - adding melee",thing_model_name(fighter),(int)fighter->index);
        return false; // We're not going to add anything
    }
    if (!can_add_melee_combat_attacker(enemy)) {
        SYNCDBG(7,"Cannot add a melee attacker to %s index %d - opponents limit reached",thing_model_name(fighter),(int)fighter->index);
        return false;
    }
    figctrl->combat_flags |= CmbtF_Melee;
    figctrl->combat.battle_enemy_idx = enemy->index;
    figctrl->combat.battle_enemy_crtn = enemy->creation_turn;
    if (!add_melee_combat_attacker(enemy, fighter->index)) {
        ERRORLOG("Cannot add a melee attacker, but %s index %d had free slot - internal error",thing_model_name(fighter),(int)fighter->index);
        figctrl->combat_flags &= ~CmbtF_Melee;
        figctrl->combat.battle_enemy_idx = 0;
        figctrl->combat.battle_enemy_crtn = 0;
        figctrl->fight_til_death = 0;
        return false;
    }
    if (!battle_add(fighter, enemy)) {
        remove_melee_combat_attacker(enemy, fighter->index);
        figctrl->combat_flags &= ~CmbtF_Melee;
        figctrl->combat.battle_enemy_idx = 0;
        figctrl->combat.battle_enemy_crtn = 0;
        figctrl->fight_til_death = 0;
        return false;
    }
    return true;
}

TbBool add_waiting_attacker(struct Thing *fighter, struct Thing *enemy)
{
    SYNCDBG(18,"Starting for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enemy),(int)enemy->index);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fighter);
    if (figctrl->combat_flags) {
        SYNCDBG(7,"The %s index %d in combat already - waiting",thing_model_name(fighter),(int)fighter->index);
    }
    figctrl->combat_flags |= CmbtF_Waiting;
    figctrl->combat.battle_enemy_idx = enemy->index;
    figctrl->combat.battle_enemy_crtn = enemy->creation_turn;
    if (!battle_add(fighter, enemy)) {
        //TODO COMBAT write the function to remove the waiting attacker (might be dummy)
        //remove_waiting_combat_attacker(enemy, fighter->index);
        figctrl->combat_flags &= ~CmbtF_Waiting;
        figctrl->combat.battle_enemy_idx = 0;
        figctrl->combat.battle_enemy_crtn = 0;
        figctrl->fight_til_death = 0;
        return false;
    }
    return true;
}

TbBool set_creature_combat_state(struct Thing *fighter, struct Thing *enemy, CrAttackType attack_type)
{
    SYNCDBG(18,"Starting for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enemy),(int)enemy->index);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fighter);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enemy);
    {
        struct Dungeon* dungeon = get_players_num_dungeon(fighter->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->fights_num++;
        }
    }
    figctrl->combat.attack_type = attack_type;
    // If creatures weren't at combat before, then play a speech
    if ((enmctrl->combat_flags & (CmbtF_Melee|CmbtF_Ranged)) == 0)
    {
      if (is_my_player_number(fighter->owner))
      {
          if (is_my_player_number(enemy->owner)) {
              output_message(SMsg_FingthingFriends, MESSAGE_DELAY_FIGHT, 1);
          } else {
              output_message(SMsg_CreatureAttacking, MESSAGE_DELAY_FIGHT, 1);
          }
      } else
      {
          if (is_my_player_number(enemy->owner)) {
            output_message(SMsg_CreatureDefending, MESSAGE_DELAY_FIGHT, 1);
          }
      }
    }
    // If we're supposed to enter ranged combat
    if (attack_type == AttckT_Ranged)
    {
        if ( add_ranged_attacker(fighter, enemy) )
        {
            play_creature_sound(fighter, CrSnd_Fight, 3, 0);
            figctrl->combat.state_id = CmbtSt_Ranged;
            return true;
        } else
        if ( add_waiting_attacker(fighter, enemy) )
        {
            figctrl->combat.state_id = CmbtSt_Waiting;
            return true;
        }
        return false;
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(fighter);
    if ((crstat->attack_preference == AttckT_Ranged) && creature_has_ranged_weapon(fighter))
    {
        if (add_ranged_attacker(fighter, enemy))
        {
            play_creature_sound(fighter, CrSnd_Fight, 3, 0);
            figctrl->combat.state_id = CmbtSt_Ranged;
            return true;
        }
    }
    if (add_melee_attacker(fighter, enemy))
    {
        play_creature_sound(fighter, CrSnd_Fight, 3, 0);
        figctrl->combat.state_id = CmbtSt_Melee;
        return true;
    }
    if (creature_has_ranged_weapon(fighter) && add_ranged_attacker(fighter, enemy))
    {
        play_creature_sound(fighter, CrSnd_Fight, 3, 0);
        figctrl->combat.state_id = CmbtSt_Ranged;
        return true;
    } else
    if (add_waiting_attacker(fighter, enemy))
    {
        figctrl->combat.state_id = CmbtSt_Waiting;
        return true;
    }
    return false;
}

TbBool set_creature_in_combat_to_the_death(struct Thing *fighter, struct Thing *enemy, CrAttackType attack_type)
{
    SYNCDBG(8,"Starting for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enemy),(int)enemy->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(fighter);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Invalid creature control");
        return false;
    }
    if (cctrl->combat_flags != 0) {
        WARNLOG("The %s index %d in combat already - adding till death",thing_model_name(fighter),(int)fighter->index);
        return false; // We're not going to add anything
    }
    if (!external_set_thing_state(fighter, CrSt_CreatureInCombat)) {
        return false;
    }
    if (!set_creature_combat_state(fighter, enemy, attack_type))
    {
        WARNLOG("Couldn't setup combat state for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enemy),(int)enemy->index);
        set_start_state(fighter);
        return false;
    }
    cctrl->field_AA = 0;
    cctrl->fight_til_death = 1;
    return true;
}

CrAttackType find_fellow_creature_to_fight_in_room(struct Thing *fightng, struct Room *room,long crmodel, struct Thing **enemytng)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(fightng->owner);
    unsigned long k = 0;
    int i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (thing_is_creature(thing) && (thing->model == crmodel) && (cctrl->combat_flags == 0))
        {
            if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody(thing)
             && !creature_is_being_unconscious(thing) && !creature_is_dying(thing))
            {
                if ((thing->index != fightng->index) && (get_room_thing_is_on(thing)->index == room->index))
                {
                    long dist = get_combat_distance(fightng, thing);
                    CrAttackType attack_type = creature_can_have_combat_with_creature(fightng, thing, dist, 0, 0);
                    if (attack_type > AttckT_Unset)
                    {
                        *enemytng = thing;
                        return attack_type;
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
    return AttckT_Unset;
}

short cleanup_combat(struct Thing *creatng)
{
    SYNCDBG(8,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    remove_all_traces_of_combat(creatng);
    return 0;
}

short cleanup_door_combat(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->combat_flags &= ~CmbtF_DoorFight;
    cctrl->combat.battle_enemy_idx = 0;
    return 1;
}

short cleanup_object_combat(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->combat_flags &= ~CmbtF_ObjctFight;
    cctrl->combat.battle_enemy_idx = 0;
    return 1;
}

CrAttackType check_for_possible_combat_within_distance(struct Thing *creatng, struct Thing **fightng, long dist)
{
    SYNCDBG(9,"Starting");
    unsigned long outscore = 0;
    struct Thing* enmtng;
    CrAttackType attack_type = check_for_possible_combat_with_attacker_within_distance(creatng, &enmtng, dist, &outscore);
    if (attack_type <= AttckT_Unset)
    {
        attack_type = check_for_possible_combat_with_enemy_creature_within_distance(creatng, &enmtng, dist);
    }
    if (attack_type <= AttckT_Unset) {
        return AttckT_Unset;
    }
    *fightng = enmtng;
    return attack_type;
}

short creature_combat_flee(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    GameTurnDelta turns_in_flee = game.play_gameturn - (GameTurnDelta)cctrl->flee_start_turn;
    if (get_2d_box_distance(&creatng->mappos, &cctrl->flee_pos) >= 1536)
    {
        SYNCDBG(8,"Starting distant flee for %s index %d",thing_model_name(creatng),(int)creatng->index);
        if (has_melee_combat_attackers(creatng) || has_ranged_combat_attackers(creatng)
          || creature_requires_healing(creatng))
        {
            if (creature_move_to(creatng, &cctrl->flee_pos, cctrl->max_speed, 0, 0) == -1)
            {
                cctrl->flee_pos.x.val = creatng->mappos.x.val;
                cctrl->flee_pos.y.val = creatng->mappos.y.val;
                cctrl->flee_pos.z.val = creatng->mappos.z.val;
            }
            cctrl->flee_start_turn = game.play_gameturn;
        } else
        if (turns_in_flee <= game.game_turns_in_flee)
        {
            GameTurnDelta escape_turns = (game.game_turns_in_flee >> 2);
            if (escape_turns <= 50)
                escape_turns = 50;
            if (turns_in_flee <= escape_turns)
            {
                if (creature_move_to(creatng, &cctrl->flee_pos, cctrl->max_speed, 0, 0) == -1) {
                    cctrl->flee_pos.x.val = creatng->mappos.x.val;
                    cctrl->flee_pos.y.val = creatng->mappos.y.val;
                    cctrl->flee_pos.z.val = creatng->mappos.z.val;
                }
            } else
            {
                if (creature_choose_random_destination_on_valid_adjacent_slab(creatng)) {
                    creatng->continue_state = CrSt_CreatureCombatFlee;
                }
            }
        } else
        {
            set_start_state(creatng);
        }
    } else
    {
        SYNCDBG(8,"Starting near flee for %s index %d",thing_model_name(creatng),(int)creatng->index);
        if (turns_in_flee > 8)
        {
            struct Thing *fightng;
            CrAttackType attack_type = check_for_possible_combat_within_distance(creatng, &fightng, 2304);
            if (attack_type > AttckT_Unset)
            {
                set_creature_in_combat_to_the_death(creatng, fightng, attack_type);
                return 1;
            }
        }
        if (turns_in_flee <= game.game_turns_in_flee)
        {
            if (creature_choose_random_destination_on_valid_adjacent_slab(creatng)) {
                creatng->continue_state = CrSt_CreatureCombatFlee;
            }
        }
        else
        {
            set_start_state(creatng);
        }
    }
    return 1;
}

TbBool combat_enemy_exists(struct Thing *thing, struct Thing *enmtng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ( (!thing_exists(enmtng)) || (cctrl->combat.battle_enemy_crtn != enmtng->creation_turn) )
    {
        SYNCDBG(8,"Enemy creature doesn't exist");
        return false;
    }
    struct CreatureControl* enmcctrl = creature_control_get_from_thing(enmtng);
    if (creature_control_invalid(enmcctrl) && (enmtng->class_id != TCls_Object) && (enmtng->class_id != TCls_Door))
    {
        ERRORLOG("No control structure - C%d M%d GT%ld CA%d", (int)enmtng->class_id,
            (int)enmtng->model, (long)game.play_gameturn, (int)thing->creation_turn);
        return false;
    }
    return true;
}

short creature_door_combat(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* doortng = thing_get(cctrl->combat.battle_enemy_idx);
    if ((cctrl->combat_flags & CmbtF_DoorFight) == 0)
    {
        ERRORLOG("The %s index %d is not in door combat but should be", thing_model_name(creatng), (int)creatng->index);
        set_start_state(creatng);
        return 0;
    }
    if (!combat_enemy_exists(creatng, doortng) || (doortng->active_state == DorSt_Open))
    {
        check_map_explored(creatng, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
        set_start_state(creatng);
        return 0;
    }
    if (players_are_mutual_allies(creatng->owner, doortng->owner))
    {
        set_start_state(creatng);
        return 0;
    }
    CombatState combat_func;
    if (cctrl->combat.state_id < sizeof(combat_door_state)/sizeof(combat_door_state[0]))
        combat_func = combat_door_state[cctrl->combat.state_id];
    else
        combat_func = NULL;
    if (combat_func != NULL)
    {
        combat_func(creatng);
        return 1;
    }
    ERRORLOG("Invalid fight door state");
    set_start_state(creatng);
    return 0;
}

TbBool creature_has_creature_in_combat(const struct Thing *thing, const struct Thing *enmtng)
{
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    if ( (enmctrl->combat_flags != 0) && (enmctrl->combat.battle_enemy_idx > 0) ) {
        return (enmctrl->combat.battle_enemy_idx == thing->index);
    }
    return false;
}

long get_combat_score(const struct Thing *thing, const struct Thing *enmtng, CrAttackType attack_type, long a4)
{
    //return _DK_get_combat_score(thing, enmtng, attack_type, a4);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);

    long score_extra;
    long score_base;

    if (crstat->attack_preference == AttckT_Ranged)
    {
        if ((attack_type == AttckT_Ranged) || creature_has_ranged_weapon(thing))
        {
            score_extra = 258;
            score_base = 258 * (4 - enmctrl->opponents_ranged_count) + score_extra;
        } else
        if (attack_type != AttckT_Ranged)
        {
            score_extra = 1;
            score_base = 258 * (4 - enmctrl->opponents_melee_count) + score_extra + 128;
        } else
        {
            score_extra = 1;
            score_base = 258 * (4 - enmctrl->opponents_ranged_count) + score_extra;
        }
    } else
    {
        if (attack_type == AttckT_Ranged)
        {
            score_extra = 1;
            score_base = 258 * (4 - enmctrl->opponents_ranged_count) + score_extra;
        } else
        if (attack_type != AttckT_Ranged)
        {
            score_extra = 258;
            score_base = 258 * (4 - enmctrl->opponents_melee_count) + score_extra + 128;
        } else
        {
            score_extra = 258;
            score_base = 258 * (4 - enmctrl->opponents_ranged_count) + score_extra;
        }
    }
    if (a4 >= 5376)
        a4 = 5376;
    return score_base + ((5376 - a4) << 8) / 5376;
}

CrAttackType check_for_possible_melee_combat_with_attacker_within_distance(struct Thing *fightng, struct Thing **outenmtng, long maxdist, unsigned long *outscore)
{
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    CrAttackType best = AttckT_Unset;
    // Check scores of melee opponents
    for (long oppn_idx = 0; oppn_idx < COMBAT_MELEE_OPPONENTS_LIMIT; oppn_idx++)
    {
        long thing_idx = figctrl->opponents_melee[oppn_idx];
        struct Thing* thing;
        if (thing_idx > 0)
            thing = thing_get(thing_idx);
        else
            thing = INVALID_THING;
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            continue;
        // When counting distance, take size of creatures into account
        long distance = get_combat_distance(fightng, thing);
        if (distance >= maxdist) {
            continue;
        }
        CrAttackType attack_type = creature_can_have_combat_with_creature(fightng, thing, distance, 1, 0);
        if (attack_type > AttckT_Unset)
        {
            unsigned long score = get_combat_score(fightng, thing, attack_type, distance);
            if (*outscore < score)
            {
                *outscore = score;
                *outenmtng = thing;
                best = attack_type;
            }
        }
    }
    return best;
}

CrAttackType check_for_possible_ranged_combat_with_attacker_within_distance(struct Thing *fightng, struct Thing **outenmtng, long maxdist, unsigned long *outscore)
{
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    CrAttackType best = AttckT_Unset;
    // Check scores of ranged opponents
    for (long oppn_idx = 0; oppn_idx < COMBAT_RANGED_OPPONENTS_LIMIT; oppn_idx++)
    {
        long thing_idx = figctrl->opponents_ranged[oppn_idx];
        struct Thing* thing;
        if (thing_idx > 0)
            thing = thing_get(thing_idx);
        else
            thing = INVALID_THING;
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            continue;
        // When counting distance, take size of creatures into account
        long distance = get_combat_distance(fightng, thing);
        if (distance >= maxdist) {
            continue;
        }
        CrAttackType attack_type = creature_can_have_combat_with_creature(fightng, thing, distance, 1, 0);
        if (attack_type > AttckT_Unset)
        {
            unsigned long score = get_combat_score(fightng, thing, attack_type, distance);
            if (*outscore < score)
            {
                *outscore = score;
                *outenmtng = thing;
                best = attack_type;
            }
        }
    }
    return best;
}

CrAttackType check_for_possible_combat_with_enemy_creature_within_distance(struct Thing *fightng, struct Thing **outenmtng, long maxdist)
{
    struct Thing* thing = get_highest_score_enemy_creature_within_distance_possible_to_attack_by(fightng, maxdist);
    if (!thing_is_invalid(thing))
    {
        SYNCDBG(9,"Best enemy for %s index %d is %s index %d",thing_model_name(fightng),(int)fightng->index,thing_model_name(thing),(int)thing->index);
        // When counting distance, take size of creatures into account
        long distance = get_combat_distance(fightng, thing);
        CrAttackType attack_type = creature_can_have_combat_with_creature(fightng, thing, distance, 1, 0);
        if (attack_type > AttckT_Unset) {
            *outenmtng = thing;
            return attack_type;
        } else {
            ERRORLOG("The %s index %d cannot fight with %s index %d returned as fight partner",thing_model_name(fightng),(int)fightng->index,thing_model_name(thing),(int)thing->index);
        }
    }
    return AttckT_Unset;
}

CrAttackType check_for_possible_combat_with_attacker_within_distance(struct Thing *figtng, struct Thing **outenmtng, long maxdist, unsigned long *outscore)
{
    unsigned long max_score;
    struct Thing *enmtng;
    long best = AttckT_Unset;
    // Do the same code two times - for melee and ranged opponents
    if (has_melee_combat_attackers(figtng))
    {
        max_score = 0;
        best = check_for_possible_melee_combat_with_attacker_within_distance(figtng, &enmtng, maxdist, &max_score);
        if (max_score > 0)
        {
            *outenmtng = enmtng;
            *outscore = max_score;
            return best;
        }
    }
    if (has_ranged_combat_attackers(figtng))
    {
        max_score = 0;
        best = check_for_possible_ranged_combat_with_attacker_within_distance(figtng, &enmtng, maxdist, &max_score);
        if (max_score > 0)
        {
            *outenmtng = enmtng;
            *outscore = max_score;
            return best;
        }
    }
    return AttckT_Unset;
}

CrAttackType check_for_possible_combat_with_attacker(struct Thing *figtng, struct Thing **outenmtng, unsigned long *outscore)
{
    return check_for_possible_combat_with_attacker_within_distance(figtng, outenmtng, LONG_MAX, outscore);
}

long creature_is_most_suitable_for_combat(struct Thing *thing, struct Thing *enmtng)
{
    unsigned long curr_score;
    // If we're already fighting with that enemy
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ( creature_has_creature_in_combat(thing, enmtng) )
    {
        struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
        // And it's a melee fight, or it's not melee but all opponents are non-melee
        if ( ((enmctrl->combat_flags & CmbtF_Melee) != 0) || (cctrl->opponents_melee_count == 0) ) {
            return true;
        }
    }
    { // Compute the score we can get from current battle
        long distance = get_combat_distance(thing, enmtng);
        curr_score = get_combat_score(thing, enmtng, cctrl->combat.attack_type, distance);
    }
    // Compute highest possible score from other battles
    unsigned long other_score = curr_score;
    struct Thing* other_enmtng = INVALID_THING;
    check_for_possible_combat_with_attacker(thing, &other_enmtng, &other_score);
    SYNCDBG(9,"Current fight score is %lu, fight with %s index %d might give %lu",curr_score,thing_model_name(other_enmtng),(int)other_enmtng->index,other_score);
    // If the benefit of changing fight is low, then inform that this is most suitable fight
    return (enmtng->index == other_enmtng->index) || (other_score <= curr_score + 258);
}

CrAttackType check_for_valid_combat(struct Thing *fightng, struct Thing *enmtng)
{
    SYNCDBG(19,"Starting for %s index %d vs %s index %d",thing_model_name(fightng),(int)fightng->index,thing_model_name(enmtng),(int)enmtng->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(fightng);
    CrAttackType attack_type = cctrl->combat.attack_type;
    if (!creature_will_attack_creature_incl_til_death(fightng, enmtng)) {
        return AttckT_Unset;
    }
    if (((game.play_gameturn + fightng->index) & 7) == 0) {
        long dist = get_combat_distance(fightng, enmtng);
        attack_type = creature_can_have_combat_with_creature(fightng, enmtng, dist, 1, 1);
    }
    return attack_type;
}

TbBool combat_type_is_choice_of_creature(const struct Thing *thing, CrAttackType attack_type)
{
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    //return _DK_combat_type_is_choice_of_creature(thing, attack_type);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (attack_type <= AttckT_Unset) {
        return false;
    }
    if (attack_type == AttckT_Ranged)
    {
        if (cctrl->combat.attack_type == AttckT_Ranged)
            return true;
        return false;
    }
    // so (attack_type == AttckT_Melee)
    if (cctrl->combat.attack_type != AttckT_Ranged) {
        return true;
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    if (crstat->attack_preference != AttckT_Ranged)
        return false;
    return creature_has_ranged_weapon(thing);
}

long guard_post_combat_move(struct Thing *thing, long cntn_crstate)
{
    //return _DK_guard_post_combat_move(thing, a2);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_is_invalid(room) && (room->kind == get_room_for_job(Job_GUARD)) && (cctrl->last_work_room_id == room->index)) {
        return 0;
    }
    if (cctrl->last_work_room_id <= 0)
    {
        ERRORLOG("Cannot get to %s",room_code_name(get_room_for_job(Job_GUARD)));
        cctrl->job_assigned = 0;
        return 0;
    }
    room = room_get(cctrl->last_work_room_id);
    if (!room_still_valid_as_type_for_thing(room, get_room_for_job(Job_GUARD), thing))
    {
        cctrl->job_assigned = 0;
        return 0;
    }
    if (get_distance_to_room(&thing->mappos, room) <= subtile_coord(27,0))
    {
        return 0;
    }
    if (!creature_setup_random_move_for_job_in_room(thing, room, Job_GUARD, NavRtF_Default))
    {
        cctrl->job_assigned = 0;
        return 0;
    }
    thing->continue_state = cntn_crstate;
    return 1;
}

TbBool thing_in_field_of_view(struct Thing *thing, struct Thing *checktng)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    long angle = get_angle_xy_to(&thing->mappos, &checktng->mappos);
    long angdiff = get_angle_difference(thing->move_angle_xy, angle);
    return (angdiff < crstat->field_of_view);
}

long ranged_combat_move(struct Thing *thing, struct Thing *enmtng, MapCoordDelta enmdist, CrtrStateId nstat)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->instance_id != CrInst_NULL)
    {
        creature_turn_to_face(thing, &enmtng->mappos);
        return false;
    }
    if (cctrl->job_assigned == Job_GUARD)
    {
        if (guard_post_combat_move(thing, nstat)) {
            return false;
        }
    }
    if (!combat_has_line_of_sight(thing, enmtng, enmdist))
    {
        if (creature_move_to(thing, &enmtng->mappos, cctrl->max_speed, 0, 0) == -1) {
            set_start_state(thing);
        }
        return false;
    }
    if (enmdist < subtile_coord(3,0)) {
        creature_retreat_from_combat(thing, enmtng, nstat, 1);
    } else
    if (enmdist > compute_creature_attack_range(subtile_coord(8,0), 0, cctrl->explevel)) {
        creature_move_to(thing, &enmtng->mappos, cctrl->max_speed, 0, 0);
    }
    return thing_in_field_of_view(thing, enmtng);
}

#define INSTANCE_RET_IF_AVAIL(thing, inst_id) \
    if (creature_instance_is_available(thing, inst_id) \
      && creature_instance_has_reset(thing, inst_id)) { \
        return inst_id; \
    }
#define INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, inst_id) \
    if (creature_instance_is_available(thing, inst_id)) { \
        return -inst_id; \
    }

/**
 * Gives attack type optimized for self preservation.
 * @param thing The creature for which the instance is selected.
 */
CrInstance get_best_self_preservation_instance_to_use(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->spell_flags & CSAfF_PoisonCloud) != 0)
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_WIND);
    }
    if (!creature_affected_by_spell(thing, SplK_Invisibility))
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_INVISIBILITY);
    }
    if (creature_requires_healing(thing))
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_HEAL);
    }
    if (!creature_affected_by_spell(thing, SplK_Armour))
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_ARMOUR);
    }
    if (!creature_affected_by_spell(thing, SplK_Speed))
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_SPEED);
    }
    if (!creature_affected_by_spell(thing, SplK_Rebound))
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_REBOUND);
    }
    if (!creature_affected_by_spell(thing, SplK_Fly))
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_FLY);
    }
    return CrInst_NULL;
}

CrInstance get_self_spell_casting(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (!creature_affected_by_spell(thing, SplK_Sight))
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_SIGHT);
    }
    if (creature_requires_healing(thing))
    {
        INSTANCE_RET_IF_AVAIL(thing, CrInst_HEAL);
    }
    long state_type = get_creature_state_type(thing);
    if (!creature_is_kept_in_custody(thing))
    {
        // casting wind when under influence of gas
        if ((cctrl->spell_flags & CSAfF_PoisonCloud) != 0)
        {
            INSTANCE_RET_IF_AVAIL(thing, CrInst_WIND);
        }
        if (!creature_affected_by_spell(thing, SplK_Speed) && (state_type != CrStTyp_Idle))
        {
            INSTANCE_RET_IF_AVAIL(thing, CrInst_SPEED);
        }
        if (!creature_affected_by_spell(thing, SplK_Fly) && ((state_type != CrStTyp_Idle) || terrain_toxic_for_creature_at_position(thing, coord_subtile(thing->mappos.x.val), coord_subtile(thing->mappos.y.val))))
        {
            INSTANCE_RET_IF_AVAIL(thing, CrInst_FLY);
        }
        //TODO CREATURE_AI allow using invisibility when creature is being attacked or escaping
        if (!creature_affected_by_spell(thing, SplK_Invisibility) && (state_type != CrStTyp_Idle))
        {
            INSTANCE_RET_IF_AVAIL(thing, CrInst_INVISIBILITY);
        }
    }
    return CrInst_NULL;
}

CrInstance get_best_quick_range_instance_to_use(const struct Thing *thing)
{
    INSTANCE_RET_IF_AVAIL(thing, CrInst_FIREBALL);
    INSTANCE_RET_IF_AVAIL(thing, CrInst_FIRE_ARROW);
    INSTANCE_RET_IF_AVAIL(thing, CrInst_MISSILE);
    INSTANCE_RET_IF_AVAIL(thing, CrInst_NAVIGATING_MISSILE);
    INSTANCE_RET_IF_AVAIL(thing, CrInst_LIGHTNING);
    INSTANCE_RET_IF_AVAIL(thing, CrInst_HAILSTORM);
    INSTANCE_RET_IF_AVAIL(thing, CrInst_GRENADE);
    INSTANCE_RET_IF_AVAIL(thing, CrInst_POISON_CLOUD);
    INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, CrInst_FIREBALL);
    INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, CrInst_FIRE_ARROW);
    INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, CrInst_MISSILE);
    INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, CrInst_NAVIGATING_MISSILE);
    INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, CrInst_LIGHTNING);
    INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, CrInst_HAILSTORM);
    INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, CrInst_GRENADE);
    INSTANCE_RET_NEG_IF_AVAIL_ONLY(thing, CrInst_POISON_CLOUD);
    return CrInst_NULL;
}

#undef INSTANCE_RET_IF_AVAIL
#undef INSTANCE_RET_NEG_IF_AVAIL_ONLY

/**
 * Gives combat weapon instance from given array which matches given distance.
 * @param thing The creature for which the instance is selected.
 * @param cweapons Pointer to the first element of 0-terminated array of weapons.
 * @param dist The distance which needs to be matched.
 * @param atktype The required properties of the attack
 * @return
 */
CrInstance get_best_combat_weapon_instance_to_use(const struct Thing *thing, const struct CombatWeapon * cweapons, long dist, int atktype)
{
    CrInstance inst_id = CrInst_NULL;
    struct InstanceInfo* inst_inf;
    for (const struct CombatWeapon* cweapon = cweapons; cweapon->inst_id != CrInst_NULL; cweapon++)
    {
        inst_inf = creature_instance_info_get(cweapon->inst_id);
        if (creature_instance_is_available(thing, cweapon->inst_id))
        {
            if ( ( ((inst_inf->flags & (InstPF_RangedAttack | InstPF_RangedDebuff | InstPF_MeleeAttack)) && (atktype & InstPF_RangedAttack)) ||
                   ((inst_inf->flags & (InstPF_MeleeAttack | InstPF_RangedDebuff))  && (atktype & InstPF_MeleeAttack)) ) &&
                 (!(inst_inf->flags & InstPF_Dangerous)   || !(atktype & InstPF_Dangerous)) &&
                 ((inst_inf->flags & InstPF_Destructive)  >=  (atktype & InstPF_Destructive)) )
            {
                if (creature_instance_has_reset(thing, cweapon->inst_id))
                {
                    if ((cweapon->range_min <= dist) && (cweapon->range_max >= dist)) {
                        return cweapon->inst_id;
                    }
                }
                if (inst_id == CrInst_NULL) {
                    inst_id = -(cweapon->inst_id);
                }
            }
        }
    }
    return inst_id;
}

CrInstance get_best_ranged_offensive_weapon(const struct Thing *thing, long dist)
{
    CrInstance inst_id = get_best_self_preservation_instance_to_use(thing);
    if (inst_id == CrInst_NULL)
    {
        int atktyp = InstPF_RangedAttack;
        inst_id = get_best_combat_weapon_instance_to_use(thing, offensive_weapon, dist, atktyp);
    }
    return inst_id;
}

CrInstance get_best_melee_offensive_weapon(const struct Thing *thing, long dist)
{
    CrInstance inst_id = get_best_self_preservation_instance_to_use(thing);
    if (inst_id == CrInst_NULL)
    {
        int atktyp = InstPF_MeleeAttack;
        inst_id = get_best_combat_weapon_instance_to_use(thing, offensive_weapon, dist, atktyp);
    }
    return inst_id;
}

long get_best_melee_object_offensive_weapon(const struct Thing *thing, long dist)
{
    int atktyp = (InstPF_MeleeAttack | InstPF_Destructive | InstPF_Dangerous);
    CrInstance inst_id = get_best_combat_weapon_instance_to_use(thing, offensive_weapon, dist, atktyp);
    return inst_id;
}

long get_best_ranged_object_offensive_weapon(const struct Thing *thing, long dist)
{
    int atktyp = (InstPF_RangedAttack | InstPF_Destructive | InstPF_Dangerous);
    CrInstance inst_id = get_best_combat_weapon_instance_to_use(thing, offensive_weapon, dist,atktyp);
    return inst_id;
}

TbBool combat_has_line_of_sight(const struct Thing *creatng, const struct Thing *enmtng, MapCoordDelta enmdist)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->combat.seen_enemy_turn != game.play_gameturn) || (cctrl->combat.seen_enemy_idx != enmtng->index))
    {
      cctrl->combat.seen_enemy_turn = game.play_gameturn;
      cctrl->combat.seen_enemy_idx = enmtng->index;
      cctrl->combat.seen_enemy_los = creature_can_see_combat_path(creatng, enmtng, enmdist);
    }
    return cctrl->combat.seen_enemy_los;
}

long collide_filter_thing_is_in_my_fight(const struct Thing *firstng, const struct Thing *coldtng, long a3, long a4)
{
    if (!thing_is_creature(firstng)) {
        return false;
    }
    struct CreatureControl* firsctrl = creature_control_get_from_thing(firstng);
    struct CreatureControl* coldctrl = creature_control_get_from_thing(coldtng);
    return (firsctrl->combat_flags != 0) && (firsctrl->field_AA) && (coldctrl->combat_flags == firsctrl->combat_flags) && (firstng->index != coldtng->index);
}

struct Thing *get_thing_collided_with_at_satisfying_filter_in_square_of_for_subtile(struct Thing *shotng, struct Coord3d *pos,
    long square_size, Thing_Collide_Func filter, long filter_par1, long filter_par2, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing* creatng = INVALID_THING;
    if (shotng->parent_idx > 0) {
        creatng = thing_get(shotng->parent_idx);
    }
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if ((thing->index != shotng->index) && filter(thing, creatng, filter_par1, filter_par2) && thing_on_thing_at(shotng, pos, thing)) {
            return thing;
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return INVALID_THING;
}

struct Thing *get_thing_collided_with_at_satisfying_filter_in_square_of(struct Thing *shotng, struct Coord3d *pos, long square_size, Thing_Collide_Func filter, long filter_par1, long filter_par2)
{
    MapSubtlCoord stl_x_beg = coord_subtile(pos->x.val - square_size / 2);
    if (stl_x_beg <= 0)
        stl_x_beg = 0;
    MapSubtlCoord stl_x_end = coord_subtile(pos->x.val + square_size / 2);
    if (stl_x_end >= map_subtiles_x)
        stl_x_end = map_subtiles_x;
    MapSubtlCoord stl_y_end = coord_subtile(pos->y.val + square_size / 2);
    if (stl_y_end >= map_subtiles_y)
        stl_y_end = map_subtiles_y;
    MapSubtlCoord stl_y_beg = coord_subtile(pos->y.val - square_size / 2);
    if (stl_y_beg <= 0)
        stl_y_beg = 0;
    for (MapSubtlCoord stl_y = stl_y_beg; stl_y <= stl_y_end; stl_y++)
    {
        for (MapSubtlCoord stl_x = stl_x_beg; stl_x <= stl_x_end; stl_x++)
        {
            struct Thing* thing = get_thing_collided_with_at_satisfying_filter_in_square_of_for_subtile(shotng, pos, square_size, filter, filter_par1, filter_par2, stl_x, stl_y);
            if (!thing_is_invalid(thing))
                return thing;
        }
    }
    return 0;
}

TbBool creature_fighting_is_occupying_my_position(struct Thing *thing, struct Coord3d *pos)
{
    struct Thing* coldtng = get_thing_collided_with_at_satisfying_filter_in_square_of(thing, pos, 768, collide_filter_thing_is_in_my_fight, 0, 0);
    return thing_is_invalid(coldtng);
}

#define POSITION_FIND_TRIES 18
long creature_move_to_a_space_around_enemy(struct Thing *creatng, struct Thing *enmtng, long enm_distance, CrtrStateId ncrstate)
{
    long req_distance = enm_distance + (creatng->clipbox_size_xy + enmtng->clipbox_size_xy) / 2;
    // This will be out new position
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    int i;
    for (i = 0; i < POSITION_FIND_TRIES; i++)
    {
        long angle_final = get_angle_xy_to(&enmtng->mappos, &pos);
        long angle_dt = angle_final;
        do
        {
            int calc_idx = angle_dt / (LbFPMath_PI / 8);
            pos.x.val += 128 * pos_calcs[calc_idx][0];
            pos.y.val += 128 * pos_calcs[calc_idx][1];
            pos.z.val = 0;

            if (pos.x.val < enmtng->mappos.x.val - req_distance)
            {
                pos.x.val = enmtng->mappos.x.val - req_distance;
            } else
            if (pos.x.val > enmtng->mappos.x.val + req_distance)
            {
                pos.x.val = req_distance + enmtng->mappos.x.val;
            }

            if (pos.y.val < enmtng->mappos.y.val - req_distance)
            {
                pos.y.val = enmtng->mappos.y.val - req_distance;
            } else
            if (pos.y.val > enmtng->mappos.y.val + req_distance)
            {
                pos.y.val = req_distance + enmtng->mappos.y.val;
            }

            angle_dt = get_angle_xy_to(&enmtng->mappos, &pos);
        }
        while (get_angle_difference(angle_final, angle_dt) < LbFPMath_PI/8);
        // Update Z coord
        pos.z.val = get_thing_height_at(creatng, &pos);
        // Check if we can accept that position
        if (!thing_in_wall_at(creatng, &pos) && !terrain_toxic_for_creature_at_position(creatng, pos.x.stl.num, pos.y.stl.num))
          break;
    }
    if (i == POSITION_FIND_TRIES)
    {
        ERRORLOG("The %s index %d has stuck finding a melee pos vs %s index %d - tries count %d", thing_model_name(creatng),(int)creatng->index,thing_model_name(enmtng),(int)enmtng->index,POSITION_FIND_TRIES);
        return 0;
    }
    if (!setup_person_move_to_coord(creatng, &pos, 0)) {
        return 0;
    }
    creatng->continue_state = ncrstate;
    return 1;
}
#undef POSITION_FIND_TRIES

long old_combat_move(struct Thing *thing, struct Thing *enmtng, long enm_distance, CrtrStateId ncrstate)
{
    //return _DK_old_combat_move(thing, enmtng, enm_distance, ncrstate);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->combat_flags & CmbtF_DoorFight) != 0)
    {
        creature_turn_to_face(thing, &enmtng->mappos);
        return 0;
    }
    if (creature_fighting_is_occupying_my_position(thing, &thing->mappos))
    {
        cctrl->field_AA = 1;
        creature_turn_to_face(thing, &enmtng->mappos);
        return 0;
    }
    cctrl->field_AA = 0;
    return creature_move_to_a_space_around_enemy(thing, enmtng, enm_distance, ncrstate);
}

long melee_combat_move(struct Thing *thing, struct Thing *enmtng, long enmdist, CrtrStateId nstat)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->instance_id != CrInst_NULL)
    {
        creature_turn_to_face(thing, &enmtng->mappos);
        return false;
    }
    if (cctrl->job_assigned == Job_GUARD)
    {
        if (guard_post_combat_move(thing, nstat)) {
            return false;
        }
    }
    if (enmdist < 156)
    {
        creature_retreat_from_combat(thing, enmtng, nstat, 0);
        cctrl->field_AA = 0;
        return thing_in_field_of_view(thing, enmtng);
    }
    if (enmdist <= 284)
    {
        if (old_combat_move(thing, enmtng, 284, nstat)) {
            return false;
        }
        return thing_in_field_of_view(thing, enmtng);
    }
    cctrl->field_AA = 0;
    if (thing_in_field_of_view(thing, enmtng))
    {
        if ((cctrl->combat_flags & (CmbtF_DoorFight|CmbtF_ObjctFight)) == 0)
        {
            if (combat_has_line_of_sight(thing, enmtng, enmdist))
            {
                CrInstance inst_id = get_best_ranged_offensive_weapon(thing, enmdist);
                if (inst_id > CrInst_NULL)
                {
                    set_creature_instance(thing, inst_id, 1, enmtng->index, 0);
                    return false;
                }
            }
        }
    }
    // Move to enemy
    if (creature_move_to(thing, &enmtng->mappos, cctrl->max_speed, 0, 0) == -1)
    {
        // If cannot move to enemy, and not waiting for ranged weapon cooldown, then retreat from him
        if (!creature_has_ranged_weapon(thing))
        {
            CrInstance inst_id = get_best_self_preservation_instance_to_use(thing);
            if (inst_id > CrInst_NULL)
            {
                set_creature_instance(thing, inst_id, 1, 0, 0);
            } else
            if (creature_retreat_from_combat(thing, enmtng, nstat, 0) == Lb_FAIL)
            {
                // If cannot move at all, reset
                set_start_state(thing);
            }
        }
    }
    return false;
}

TbBool creature_scared(struct Thing *thing, struct Thing *enemy)
{
    if (thing_is_invalid(enemy))
    {
        ERRORLOG("Thing %d enemy is invalid",(int)thing->index);
        return false;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->fight_til_death)
    {
        return false;
    }
    return creature_is_actually_scared(thing, enemy);
}

TbBool creature_in_flee_zone(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Creature index %d has invalid control",(int)thing->index);
        return false;
    }
    unsigned long dist = get_2d_box_distance(&thing->mappos, &cctrl->flee_pos);
    return (dist < gameadd.flee_zone_radius);
}

TbBool creature_too_scared_for_combat(struct Thing *thing, struct Thing *enmtng)
{
    //get_combat_distance(thing, enemy);
    if (!creature_scared(thing, enmtng))
    {
        return false;
    }
    if (creature_in_flee_zone(thing))
    {
        return false;
    }
    return true;
}

TbBool remove_waiting_attacker(struct Thing *fightng)
{
    TRACE_THING(fightng);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    {
        struct Dungeon* dungeon = get_players_num_dungeon(fightng->owner);
        if (!dungeon_invalid(dungeon) && (dungeon->fights_num > 0)) {
            dungeon->fights_num--;
        } else {
            WARNLOG("Fight count incorrect while removing attacker %s index %d",thing_model_name(fightng),(int)fightng->index);
        }
    }
    figctrl->combat_flags &= ~CmbtF_Waiting;
    figctrl->combat.battle_enemy_idx = 0;
    figctrl->fight_til_death = 0;
    figctrl->combat.battle_enemy_crtn = 0;
    delay_teleport(fightng);

    battle_remove(fightng);
    return true;
}

void remove_attacker(struct Thing *figtng, struct Thing *enmtng)
{
    struct CreatureControl* figctrl = creature_control_get_from_thing(figtng);
    if ((figctrl->combat_flags & CmbtF_Melee) != 0)
    {
        remove_melee_attacker(figtng, enmtng);
    } else
    if ((figctrl->combat_flags & CmbtF_Ranged) != 0)
    {
        remove_ranged_attacker(figtng, enmtng);
    } else
    if ((figctrl->combat_flags & CmbtF_Waiting) != 0)
    {
        remove_waiting_attacker(figtng);
    }
}

void cleanup_battle_leftovers(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->battle_id > 0) {
        battle_remove(creatng);
    }
}

long remove_all_traces_of_combat(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(8,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    // Remove creature as attacker
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->combat_flags != 0) && (cctrl->combat.battle_enemy_idx > 0))
    {
        struct Thing* enmtng = thing_get(cctrl->combat.battle_enemy_idx);
        TRACE_THING(enmtng);
        remove_attacker(creatng, enmtng);
    }
    // Remove creature as victim of enemy attack
    remove_all_ranged_combat_attackers(creatng);
    remove_all_melee_combat_attackers(creatng);
    // Cleanup battle
    cleanup_battle_leftovers(creatng);
    return 1;
}

TbBool change_current_combat(struct Thing *fighter, struct Thing *enemy, CrAttackType attack_type)
{
    SYNCDBG(18,"Starting for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enemy),(int)enemy->index);
    TRACE_THING(fighter);
    TRACE_THING(enemy);
    struct CreatureControl* figctrl = creature_control_get_from_thing(fighter);
    if (creature_control_invalid(figctrl)) {
        ERRORLOG("Invalid fighter creature control");
        return false;
    }
    struct Thing* oldenemy = thing_get(figctrl->combat.battle_enemy_idx);
    TRACE_THING(oldenemy);
    remove_attacker(fighter, oldenemy);
    if ( !set_creature_combat_state(fighter, enemy, attack_type) ) {
        WARNLOG("Couldn't setup combat state for %s index %d and %s index %d",thing_model_name(fighter),(int)fighter->index,thing_model_name(enemy),(int)enemy->index);
        set_start_state(fighter);
        return false;
    }
    return true;
}

long creature_has_spare_slot_for_combat(struct Thing *fighter, struct Thing *enemy, CrAttackType attack_type)
{
    //return _DK_creature_has_spare_slot_for_combat(fighter, enemy, attack_type);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enemy);
    if (attack_type == AttckT_Ranged)
    {
        if (enmctrl->opponents_ranged_count < COMBAT_RANGED_OPPONENTS_LIMIT)
            return true;
        return false;
    }
    // Melee combat was requested; but we may still check for ranged attacker, if creature prefers it
    struct CreatureStats* figstat = creature_stats_get_from_thing(fighter);
    if (figstat->attack_preference == AttckT_Ranged)
    {
        if (creature_has_ranged_weapon(fighter))
        {
            if (enmctrl->opponents_ranged_count < COMBAT_RANGED_OPPONENTS_LIMIT)
                return true;
        }
    }
    if (enmctrl->opponents_melee_count < COMBAT_MELEE_OPPONENTS_LIMIT)
        return true;
    return false;
}

long change_creature_with_existing_attacker(struct Thing *fighter, struct Thing *enemy, CrAttackType attack_type)
{
    //return _DK_change_creature_with_existing_attacker(fighter, enemy, attack_type);
    int i;
    struct CreatureControl* cctrl = creature_control_get_from_thing(enemy);
    MapCoordDelta dist = get_2d_box_distance(&fighter->mappos, &enemy->mappos) - (enemy->clipbox_size_xy + fighter->clipbox_size_xy) / 2;
    struct Thing* best_fightng = fighter;
    long best_score = get_combat_score(fighter, enemy, attack_type, dist);
    long prev_score = best_score;
    struct Thing *creatng;
    long score;
    if (attack_type == AttckT_Ranged)
    {
      if (cctrl->opponents_ranged_count <= 0) {
          ERRORLOG("No ranged attackers - serious");
      }
      for (i = 0; i < COMBAT_RANGED_OPPONENTS_LIMIT; i++)
      {
          if (cctrl->opponents_ranged[i] > 0)
          {
              creatng = thing_get(cctrl->opponents_ranged[i]);
              struct CreatureControl* crctrl = creature_control_get_from_thing(creatng);
              dist = get_2d_box_distance(&creatng->mappos, &enemy->mappos) - (enemy->clipbox_size_xy + creatng->clipbox_size_xy) / 2;
              score = get_combat_score(creatng, enemy, crctrl->combat.attack_type, dist);
              if (creature_is_actually_scared(creatng, enemy)) {
                  score -= 512;
              }
              if (score < best_score) {
                  best_score = score;
                  best_fightng = creatng;
              }
          }
      }
    } else
    {
        if (cctrl->opponents_melee_count <= 0) {
            ERRORLOG("No melee attackers - serious");
        }
        for (i = 0; i < COMBAT_MELEE_OPPONENTS_LIMIT; i++)
        {
            if (cctrl->opponents_melee[i] > 0)
            {
                creatng = thing_get(cctrl->opponents_melee[i]);
                struct CreatureControl* csctrl = creature_control_get_from_thing(creatng);
                dist = get_2d_box_distance(&creatng->mappos, &enemy->mappos) - (enemy->clipbox_size_xy + creatng->clipbox_size_xy) / 2;
                score = get_combat_score(creatng, enemy, csctrl->combat.attack_type, dist);
                if (creature_is_actually_scared(creatng, enemy)) {
                    score -= 512;
                }
                if (score < best_score) {
                    best_score = score;
                    best_fightng = creatng;
                }
            }
        }
    }
    if (best_score != prev_score)
    {
        set_start_state(best_fightng);
        struct CreatureControl* figctrl = creature_control_get_from_thing(fighter);
        struct Thing* prevenmy = thing_get(figctrl->combat.battle_enemy_idx);
        if ((figctrl->combat_flags & CmbtF_Melee) != 0)
        {
          remove_melee_attacker(fighter, prevenmy);
        } else
        if (( figctrl->combat_flags & CmbtF_Ranged) != 0)
        {
          remove_ranged_attacker(fighter, prevenmy);
        } else
        if ((figctrl->combat_flags & CmbtF_Waiting) != 0)
        {
            struct Dungeon* dungeon = get_players_num_dungeon(fighter->owner);
            if (!dungeon_invalid(dungeon) && (dungeon->fights_num > 0)) {
                dungeon->fights_num--;
            } else {
                ERRORLOG("Fight count incorrect");
            }
            figctrl->combat_flags &= ~0x04;
            figctrl->combat.battle_enemy_idx = 0;
            figctrl->fight_til_death = 0;
            figctrl->combat.battle_enemy_crtn = 0;
            battle_remove(fighter);
        }
        set_creature_combat_state(fighter, enemy, attack_type);
    }
    return 0;
}

CrAttackType check_for_possible_combat(struct Thing *creatng, struct Thing **fightng)
{
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    TRACE_THING(creatng);
    unsigned long outscore = 0;
    // Check for combat with attacker - someone who already participates in a fight
    struct Thing* enmtng;
    CrAttackType attack_type = check_for_possible_combat_with_attacker_within_distance(creatng, &enmtng, LONG_MAX, &outscore);
    if (attack_type <= AttckT_Unset)
    {
        // Look for a new fight - with creature we're not fighting yet
        attack_type = check_for_possible_combat_with_enemy_creature_within_distance(creatng, &enmtng, LONG_MAX);
    }
    if (attack_type <= AttckT_Unset) {
        return AttckT_Unset;
    }
    *fightng = enmtng;
    SYNCDBG(19,"The %s index %d can fight %s index %d",thing_model_name(creatng),(int)creatng->index,thing_model_name(enmtng),(int)enmtng->index);
    return attack_type;
}

/**
 * Switches fight partner to the one suggested by creature_is_most_suitable_for_combat().
 * @param thing The creature to be switched.
 * @return True on success.
 * @see creature_is_most_suitable_for_combat()
 */
TbBool creature_change_to_most_suitable_combat(struct Thing *figtng)
{
    //set_start_state(thing); return true; -- this is how originally such situation was handled
    // Compute highest possible score from battles
    unsigned long other_score = 0;
    struct Thing* enmtng = INVALID_THING;
    CrAttackType attack_type = check_for_possible_combat_with_attacker(figtng, &enmtng, &other_score);
    if (thing_is_invalid(enmtng))
        return false;
    struct CreatureControl* figctrl = creature_control_get_from_thing(figtng);
    if (figctrl->combat.battle_enemy_idx == enmtng->index)
        return false;
    if (!change_current_combat(figtng, enmtng, attack_type))
        return false;
    return true;
}

long check_for_better_combat(struct Thing *figtng)
{
    SYNCDBG(9,"Starting for %s index %d",thing_model_name(figtng),(int)figtng->index);
    struct CreatureControl* figctrl = creature_control_get_from_thing(figtng);
    // Allow the switch only once per certain amount of turns
    if (((game.play_gameturn + figtng->index) % BATTLE_CHECK_INTERVAL) != 0)
        return 0;
    struct Thing* enmtng = INVALID_THING;
    CrAttackType attack_type = check_for_possible_combat(figtng, &enmtng);
    if (attack_type <= AttckT_Unset)
        return 1;
    // If we're here, that means there is a better combat
    //TODO CREATURE_AI The condition here seems strange; we need to figure out what's its purpose
    //if ( (figctrl->battle_enemy_idx != enmtng->index) || !combat_type_is_choice_of_creature(figtng, attack_type) )
    if ( (figctrl->combat.battle_enemy_idx == enmtng->index) && combat_type_is_choice_of_creature(figtng, attack_type) )
    {
        // we want to fight with the same enemy, but to use combat type preferred by creature
        // this is so good that we won't even do any additional tests - let's just do it!
        if (!change_current_combat(figtng, enmtng, attack_type))
            return 0;
        return 1;
    }
    if (figctrl->combat.state_id != CmbtSt_Waiting) {
        // it's not a waiting but real fight - don't change anything
        // (note that this condition makes battles very stable - creatures are unlikely to change enemies)
        return 0;
    }
    // Check if there's place for new combat, add or replace a slot
    if (creature_has_spare_slot_for_combat(figtng, enmtng, attack_type))
    {
        if (!change_current_combat(figtng, enmtng, attack_type))
            return 0;
    } else
    {
        if (!change_creature_with_existing_attacker(figtng, enmtng, attack_type))
            return 0;
    }
    return 1;
}

long waiting_combat_move(struct Thing *figtng, struct Thing *enmtng, long enmdist, CrtrStateId retreat_crstate)
{
    struct CreatureControl* figctrl = creature_control_get_from_thing(figtng);
    if (figctrl->instance_id != CrInst_NULL)
    {
        creature_turn_to_face(figtng, &enmtng->mappos);
        return 0;
    }
    if (figctrl->job_assigned == Job_GUARD)
    {
        if (guard_post_combat_move(figtng, retreat_crstate)) {
            return 0;
        }
    }
    if (enmdist < 768) {
        creature_retreat_from_combat(figtng, enmtng, retreat_crstate, 1);
        return 0;
    }
    if (enmdist > 2048) {
        creature_move_to(figtng, &enmtng->mappos, figctrl->max_speed, 0, 0);
        return 0;
    }
    if (creature_turn_to_face(figtng, &enmtng->mappos) >= LbFPMath_PI/12) {
        return 0;
    }
    // If the creature has ranged combat, let it fight
    if (creature_has_ranged_weapon(figtng))
    {
        if (combat_has_line_of_sight(figtng, enmtng, enmdist))
        {
            CrInstance weapon = get_best_ranged_offensive_weapon(figtng, enmdist);
            if (weapon > CrInst_NULL) {
                set_creature_instance(figtng, weapon, 1, enmtng->index, 0);
                return 0;
            }
        } else
        {
            creature_move_to(figtng, &enmtng->mappos, figctrl->max_speed, 0, 0);
        }
    }
    // Randomly jump waiting for combat
    if (thing_touching_floor(figtng))
    {
        if (CREATURE_RANDOM(figtng, 6) == 0)
        {
            figtng->veloc_push_add.z.val += CREATURE_RANDOM(figtng, 80) + 40;
            figtng->state_flags |= TF1_PushAdd;
        }
    }
    return 1;
}

void creature_in_combat_wait(struct Thing *creatng)
{
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    if (check_for_better_combat(creatng)) {
        SYNCDBG(19,"Switching to better combat");
        return;
    }
    // Check to attack dungeon heart once every 8 turns
    if (((game.play_gameturn+creatng->index) & 7) == 0)
    {
        if (creature_look_for_enemy_heart_combat(creatng)) {
            SYNCDBG(19,"Switching to heart combat");
            return;
        }
    }
    // Check if we're best combat partner for the enemy
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* enmtng = thing_get(cctrl->combat.battle_enemy_idx);
    TRACE_THING(enmtng);
    if (!creature_is_most_suitable_for_combat(creatng, enmtng))
    {
        SYNCDBG(9,"The %s index %d is not most suitable for combat with %s index %d",
            thing_model_name(creatng),(int)creatng->index,thing_model_name(enmtng),(int)enmtng->index);
        creature_change_to_most_suitable_combat(creatng);
        return;
    }
    long cmbtyp = check_for_valid_combat(creatng, enmtng);
    if ( !combat_type_is_choice_of_creature(creatng, cmbtyp) ) {
        SYNCDBG(9,"Current combat type is not choice of %s index %d",thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return;
    }
    long dist = get_combat_distance(creatng, enmtng);
    waiting_combat_move(creatng, enmtng, dist, CrSt_CreatureInCombat);
    SYNCDBG(19,"Done, continuing combat");
}

void creature_in_ranged_combat(struct Thing *creatng)
{
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* enmtng = thing_get(cctrl->combat.battle_enemy_idx);
    TRACE_THING(enmtng);
    if (!creature_is_most_suitable_for_combat(creatng, enmtng))
    {
        SYNCDBG(9,"The %s index %d is not most suitable for combat with %s index %d",thing_model_name(creatng),(int)creatng->index,thing_model_name(enmtng),(int)enmtng->index);
        creature_change_to_most_suitable_combat(creatng);
        return;
    }
    long cmbtyp = check_for_valid_combat(creatng, enmtng);
    if (!combat_type_is_choice_of_creature(creatng, cmbtyp))
    {
        SYNCDBG(9,"Current combat type is not choice of %s index %d",thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return;
    }
    long dist = get_combat_distance(creatng, enmtng);
    CrInstance weapon = get_best_ranged_offensive_weapon(creatng, dist);
    if (weapon == 0)
    {
        set_start_state(creatng);
        SYNCDBG(9,"The %s index %d cannot choose ranged offensive weapon",thing_model_name(creatng),(int)creatng->index);
        return;
    }
    if (!ranged_combat_move(creatng, enmtng, dist, CrSt_CreatureInCombat))
    {
        SYNCDBG(9,"The %s index %d is moving and cannot attack in this turn",thing_model_name(creatng),(int)creatng->index);
        return;
    }
    if (weapon > 0)
    {
        set_creature_instance(creatng, weapon, 1, enmtng->index, 0);
    }
}

void creature_in_melee_combat(struct Thing *creatng)
{
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* enmtng = thing_get(cctrl->combat.battle_enemy_idx);
    TRACE_THING(enmtng);
    if (!creature_is_most_suitable_for_combat(creatng, enmtng))
    {
        SYNCDBG(9,"The %s index %d is not most suitable for combat with %s index %d",thing_model_name(creatng),(int)creatng->index,thing_model_name(enmtng),(int)enmtng->index);
        creature_change_to_most_suitable_combat(creatng);
        return;
    }
    CrAttackType attack_type = check_for_valid_combat(creatng, enmtng);
    if (!combat_type_is_choice_of_creature(creatng, attack_type))
    {
        SYNCDBG(9,"Current combat type %d is not choice of %s index %d",(int)attack_type,thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return;
    }
    long dist = get_combat_distance(creatng, enmtng);
    CrInstance weapon = get_best_melee_offensive_weapon(creatng, dist);
    if (weapon == CrInst_NULL)
    {
        SYNCDBG(9,"The %s index %d cannot choose melee offensive weapon",thing_model_name(creatng),(int)creatng->index);
        set_start_state(creatng);
        return;
    }
    if (!melee_combat_move(creatng, enmtng, dist, CrSt_CreatureInCombat))
    {
        SYNCDBG(9,"The %s index %d is moving and cannot attack in this turn",thing_model_name(creatng),(int)creatng->index);
        return;
    }
    if (weapon > CrInst_NULL)
    {
        set_creature_instance(creatng, weapon, 1, enmtng->index, 0);
    }
}

short creature_in_combat(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    SYNCDBG(9,"Starting for %s index %d, combat state %d",thing_model_name(creatng),(int)creatng->index,(int)cctrl->combat.state_id);
    TRACE_THING(creatng);
    struct Thing* enmtng = thing_get(cctrl->combat.battle_enemy_idx);
    TRACE_THING(enmtng);
    if (!combat_enemy_exists(creatng, enmtng))
    {
        set_start_state(creatng);
        return 0;
    }
    if (creature_too_scared_for_combat(creatng, enmtng))
    {
        if (!external_set_thing_state(creatng, CrSt_CreatureCombatFlee)) {
            ERRORLOG("Cannot get %s index %d into flee",thing_model_name(creatng),(int)creatng->index);
            return 0;
        }
        cctrl->flee_start_turn = game.play_gameturn;
        return 0;
    }
    CombatState combat_func;
    if (cctrl->combat.state_id < sizeof(combat_state)/sizeof(combat_state[0]))
        combat_func = combat_state[cctrl->combat.state_id];
    else
        combat_func = NULL;
    if (combat_func != NULL)
    {
        combat_func(creatng);
        return 1;
    }
    ERRORLOG("No valid fight state %d in %s index %d",(int)cctrl->combat.state_id,thing_model_name(creatng),(int)creatng->index);
    set_start_state(creatng);
    return 0;
}

void combat_object_state_melee_combat(struct Thing *creatng)
{
    //_DK_combat_object_state_melee_combat(creatng); return;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* objtng = thing_get(cctrl->combat.battle_enemy_idx);
    long dist = get_combat_distance(creatng, objtng);
    CrInstance inst_id = get_best_melee_object_offensive_weapon(creatng, dist);
    if (inst_id == CrInst_NULL)
    {
        ERRORLOG("The %s index %d has no melee instance in fight", thing_model_name(creatng), (int)creatng->index);
        set_start_state(creatng);
    }
    if (melee_combat_move(creatng, objtng, dist, CrSt_CreatureObjectCombat))
    {
        if (inst_id > CrInst_NULL) {
            set_creature_instance(creatng, inst_id, 1, objtng->index, 0);
        }
    }
}

void combat_object_state_ranged_combat(struct Thing *creatng)
{
    //_DK_combat_object_state_ranged_combat(thing); return;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* objtng = thing_get(cctrl->combat.battle_enemy_idx);
    long dist = get_combat_distance(creatng, objtng);
    CrInstance inst_id = get_best_ranged_object_offensive_weapon(creatng, dist);
    if (inst_id == CrInst_NULL)
    {
        WARNLOG("The %s index %d has no ranged instance in fight", thing_model_name(creatng), (int)creatng->index);
    }
    if (ranged_combat_move(creatng, objtng, dist, CrSt_CreatureObjectCombat))
    {
        if (inst_id > CrInst_NULL) {
            set_creature_instance(creatng, inst_id, 0, objtng->index, 0);
        }
    }
}

void combat_door_state_melee_combat(struct Thing *creatng)
{
    //_DK_combat_door_state_melee_combat(creatng); return;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* objtng = thing_get(cctrl->combat.battle_enemy_idx);
    long dist = get_combat_distance(creatng, objtng);
    CrInstance inst_id = get_best_melee_object_offensive_weapon(creatng, dist);
    if (inst_id == CrInst_NULL)
    {
        ERRORLOG("The %s index %d has no melee instance in fight", thing_model_name(creatng), (int)creatng->index);
        set_start_state(creatng);
    }
    if (melee_combat_move(creatng, objtng, dist, CrSt_CreatureDoorCombat))
    {
        if (inst_id > CrInst_NULL) {
            set_creature_instance(creatng, inst_id, 1, objtng->index, 0);
        }
    }
}

void combat_door_state_ranged_combat(struct Thing *creatng)
{
    //_DK_combat_door_state_ranged_combat(creatng); return;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* objtng = thing_get(cctrl->combat.battle_enemy_idx);
    long dist = get_combat_distance(creatng, objtng);
    CrInstance inst_id = get_best_ranged_object_offensive_weapon(creatng, dist);
    if (inst_id == CrInst_NULL)
    {
        WARNLOG("The %s index %d has no ranged instance in fight", thing_model_name(creatng), (int)creatng->index);
    }
    if (ranged_combat_move(creatng, objtng, dist, CrSt_CreatureDoorCombat))
    {
        if (inst_id > CrInst_NULL) {
            set_creature_instance(creatng, inst_id, 0, objtng->index, 0);
        }
    }
}

short creature_object_combat(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* objctng = thing_get(cctrl->combat.battle_enemy_idx);
    if ((cctrl->combat_flags & CmbtF_ObjctFight) == 0)
    {
        ERRORLOG("The %s index %d is not in object combat but should be", thing_model_name(creatng), (int)creatng->index);
        set_start_state(creatng);
        return 0;
    }
    if (!combat_enemy_exists(creatng, objctng) || (objctng->active_state == ObSt_BeingDestroyed))
    {
        set_start_state(creatng);
        return 0;
    }
    if (!players_are_enemies(creatng->owner, objctng->owner))
    {
        set_start_state(creatng);
        return 0;
    }
    CombatState combat_func;
    if (cctrl->combat.state_id < sizeof(combat_object_state)/sizeof(combat_object_state[0]))
        combat_func = combat_object_state[cctrl->combat.state_id];
    else
        combat_func = NULL;
    if (combat_func != NULL)
    {
        combat_func(creatng);
        return 1;
    }
    ERRORLOG("The %s index %d has invalid fight object state", thing_model_name(creatng), (int)creatng->index);
    set_start_state(creatng);
    return 0;
}

TbBool creature_look_for_combat(struct Thing *creatng)
{
    SYNCDBG(9,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* enmtng;
    CrAttackType attack_type = check_for_possible_combat(creatng, &enmtng);
    if (attack_type <= AttckT_Unset)
    {
        if ( (cctrl->opponents_melee_count == 0) && (cctrl->opponents_ranged_count == 0) ) {
            return false;
        }
        CrInstance inst_id = get_best_self_preservation_instance_to_use(creatng);
        if (inst_id > CrInst_NULL)
        {
            set_creature_instance(creatng, inst_id, 0, 0, 0);
            return false;
        } else
        if (!external_set_thing_state(creatng, CrSt_CreatureCombatFlee)) {
            return false;
        }
        setup_combat_flee_position(creatng);
        cctrl->flee_start_turn = game.play_gameturn;
        return true;
    }

    if (cctrl->combat_flags != 0)
    {
        if (get_combat_state_for_combat(creatng, enmtng, attack_type) == CmbtSt_Waiting) {
          return false;
        }
    }

    // If high fear creature is invisible and not in combat, then don't let it start one
    if (creature_is_invisible(creatng))
    {
        if ( (cctrl->opponents_melee_count == 0) && (cctrl->opponents_ranged_count == 0) ) {
            struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
            if (crstat->fear_wounded >= 101)
                return false;
        }
    }

    // If not too scared for combat, then do the combat
    if ((!creature_too_scared_for_combat(creatng, enmtng)) || (cctrl->wait_to_turn > (long)game.play_gameturn) )
    {
        set_creature_in_combat(creatng, enmtng, attack_type);
        return true;
    }

    // If any creature is scared, invisible and not in combat, then don't let it start one
    if (creature_is_invisible(creatng))
    {
        if ( (cctrl->opponents_melee_count == 0) && (cctrl->opponents_ranged_count == 0) ) {
            return false;
        }
    }
    // Setup fleeing from combat
    if ( !external_set_thing_state(creatng, CrSt_CreatureCombatFlee) ) {
        ERRORLOG("The %s index %d is scared but cannot flee",thing_model_name(creatng),(int)creatng->index);
        return false;
    }
    setup_combat_flee_position(creatng);
    cctrl->flee_start_turn = game.play_gameturn;
    return true;
}

TbBool creature_look_for_enemy_heart_combat(struct Thing *thing)
{
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    TRACE_THING(thing);
    if ((get_creature_model_flags(thing) & CMF_NoEnmHeartAttack) != 0) {
        return false;
    }
    struct Thing *heartng;
    // If already fighting dungeon heart, skip the rest
    if (get_creature_state_besides_interruptions(thing) == CrSt_CreatureObjectCombat) {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        heartng = thing_get(cctrl->combat.battle_enemy_idx);
        if (thing_is_dungeon_heart(heartng)) {
            return false;
        }
    }
    heartng = get_enemy_soul_container_creature_can_see(thing);
    if (thing_is_invalid(heartng) || !(creature_can_navigate_to(thing,&heartng->mappos, NavRtF_Default)))
    {
        return false;
    }
    TRACE_THING(heartng);
    set_creature_object_combat(thing, heartng);
    return true;
}

struct Thing *check_for_door_to_fight(struct Thing *thing)
{
    long m = CREATURE_RANDOM(thing, SMALL_AROUND_SLAB_LENGTH);
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        MapSlabCoord slb_x = subtile_slab_fast(thing->mappos.x.stl.num) + (long)small_around[m].delta_x;
        MapSlabCoord slb_y = subtile_slab_fast(thing->mappos.y.stl.num) + (long)small_around[m].delta_y;
        struct Thing* doortng = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (!thing_is_invalid(doortng))
        {
          if (thing->owner != doortng->owner)
              return doortng;
        }
        m = (m+1) % SMALL_AROUND_SLAB_LENGTH;
    }
    return INVALID_THING;
}

TbBool creature_look_for_enemy_door_combat(struct Thing *thing)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    // Creatures which can pass doors shouldn't pick a fight with them
    if (crstat->can_go_locked_doors) {
        return false;
    }
    struct Thing* doortng = check_for_door_to_fight(thing);
    if (thing_is_invalid(doortng)) {
        return false;
    }
    set_creature_door_combat(thing, doortng);
    return true;
}

TbResult creature_retreat_from_combat(struct Thing *figtng, struct Thing *enmtng, CrtrStateId continue_state, long a4)
{
    struct Coord3d pos;
    long i;
    TRACE_THING(figtng);
    TRACE_THING(enmtng);

    struct CreatureControl* figctrl = creature_control_get_from_thing(figtng);
    MapCoordDelta dist_x = enmtng->mappos.x.val - (MapCoordDelta)figtng->mappos.x.val;
    MapCoordDelta dist_y = enmtng->mappos.y.val - (MapCoordDelta)figtng->mappos.y.val;

    if (a4 && ((figctrl->combat_flags & (CmbtF_ObjctFight|CmbtF_DoorFight)) == 0))
    {
        pos.x.val = figtng->mappos.x.val - dist_x;
        pos.y.val = figtng->mappos.y.val - dist_y;
        pos.z.val = get_thing_height_at(figtng, &pos);
        if (creature_move_to(figtng, &pos, get_creature_speed(figtng), 0, 1) != -1)
        {
           return Lb_SUCCESS;
        }
    }
    // First try
    pos.x.val = figtng->mappos.x.val;
    pos.y.val = figtng->mappos.y.val;
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
    pos.z.val = get_thing_height_at(figtng, &pos);

    if (setup_person_move_backwards_to_coord(figtng, &pos, NavRtF_Default))
    {
      figtng->continue_state = continue_state;
      return Lb_SUCCESS;
    }
    // Second try
    pos.x.val = figtng->mappos.x.val;
    pos.y.val = figtng->mappos.y.val;
    if (CREATURE_RANDOM(figtng, 2) == 0)
        i = 1;
    else
        i = -1;
    if (abs(dist_y) >= abs(dist_x))
      pos.x.val += 768 * i;
    else
      pos.y.val += 768 * i;
    pos.z.val = get_thing_height_at(figtng, &pos);
    if (setup_person_move_backwards_to_coord(figtng, &pos, NavRtF_Default))
    {
        figtng->continue_state = continue_state;
        return Lb_SUCCESS;
    }
    return Lb_FAIL;
}

short creature_attack_rooms(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->target_room_id = 0;
    // Destroy the room tile we're on
    if (thing_is_on_any_room_tile(creatng))
    {
        if (cctrl->instance_id == CrInst_NULL) {
            set_creature_instance(creatng, CrInst_ATTACK_ROOM_SLAB, 1, 0, 0);
        }
        return 1;
    }
    // If we're not (or no longer) on room tile, find adjacent one
    int n = CREATURE_RANDOM(creatng, SMALL_AROUND_LENGTH);
    for (int i = 0; i < SMALL_AROUND_LENGTH; i++)
    {
        MapSubtlCoord stl_x = creatng->mappos.x.stl.num + STL_PER_SLB * (int)small_around[n].delta_x;
        MapSubtlCoord stl_y = creatng->mappos.y.stl.num + STL_PER_SLB * (int)small_around[n].delta_y;
        if (attempt_to_destroy_enemy_room(creatng, stl_x, stl_y)) {
            return 1;
        }
        n = (n + 1) % SMALL_AROUND_LENGTH;
    }
    set_start_state(creatng);
    return 0;
}

short creature_attempt_to_damage_walls(struct Thing *creatng)
{
    TRACE_THING(creatng);
    return _DK_creature_attempt_to_damage_walls(creatng);
}

short creature_damage_walls(struct Thing *creatng)
{
    TRACE_THING(creatng);
    return _DK_creature_damage_walls(creatng);
}

/**
 * Projects damage made by a creature attack on given target.
 * Gives a best estimate of the damage, but shouldn't be used to actually inflict it.
 * @param firing The creature which will be shooting.
 * @param target The target creature.
 */
long project_creature_attack_target_damage(const struct Thing *firing, const struct Thing *target)
{
    // Determine most likely shot of the firing creature
    CrInstance inst_id;
    long dist = get_combat_distance(firing, target);
    struct CreatureStats* crstat = creature_stats_get_from_thing(firing);
    if (crstat->attack_preference == AttckT_Ranged) {
        inst_id = get_best_combat_weapon_instance_to_use(firing, offensive_weapon, dist,2);
        if (inst_id == CrInst_NULL) {
            inst_id = get_best_combat_weapon_instance_to_use(firing, offensive_weapon, dist,4);
        }
    } else {
        inst_id = get_best_combat_weapon_instance_to_use(firing, offensive_weapon, dist,4);
        if (inst_id == CrInst_NULL) {
            inst_id = get_best_combat_weapon_instance_to_use(firing, offensive_weapon, dist,2);
        }
    }
    if (inst_id == CrInst_NULL) {
        // It seem the creatures cannot currently attack each other
        return CrInst_NULL;
    }
    // Get shot model from instance
    ThingModel shot_model;
    {
        struct InstanceInfo* inst_inf = creature_instance_info_get(inst_id);
        //TODO CREATURES Instance doesn't necessarily contain shot model, that depends on callback
        // Do a check to make sure the instance fires a shot
        shot_model = inst_inf->func_params[0];
    }
    long damage = project_creature_shot_damage(firing, shot_model);
    // Adjust the damage with target creature defense
    struct CreatureControl* cctrl = creature_control_get_from_thing(firing);
    long dexterity = compute_creature_max_dexterity(crstat->dexterity, cctrl->explevel);
    damage = project_damage_of_melee_shot(dexterity, damage, target);
    return damage;
}

long process_creature_self_spell_casting(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (((creatng->alloc_flags & TAlF_IsControlled) != 0)
      || (cctrl->conscious_back_turns != 0)
      || ((cctrl->stateblock_flags & CCSpl_Freeze) != 0)) {
        return 0;
    }
    if (cctrl->instance_id != CrInst_NULL) {
        return 0;
    }
    long inst_idx = get_self_spell_casting(creatng);
    if (inst_idx <= 0) {
        return 0;
    }
    set_creature_instance(creatng, inst_idx, 1, creatng->index, 0);
    return 1;
}
/******************************************************************************/
