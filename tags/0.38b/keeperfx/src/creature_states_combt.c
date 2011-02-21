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
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"

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
DLLIMPORT long _DK_creature_can_have_combat_with_creature(struct Thing *fighter1, struct Thing *fighter2, long enemy, long a4, long a5);
DLLIMPORT void _DK_set_creature_combat_state(struct Thing *fighter1, struct Thing *fighter2, long dist);
DLLIMPORT long _DK_creature_has_other_attackers(struct Thing *thing, long enemy);
DLLIMPORT long _DK_get_best_ranged_offensive_weapon(struct Thing *thing, long enemy);
DLLIMPORT long _DK_get_best_melee_offensive_weapon(struct Thing *thing, long enemy);
DLLIMPORT long _DK_jonty_creature_can_see_thing_including_lava_check(struct Thing * creature, struct Thing * thing);
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
    if (game.creature_stats[creature->model].visual_range << 8 >= dist) {
        return jonty_creature_can_see_thing_including_lava_check(creature, enemy);
    }

    return 0;
}

long get_combat_distance(struct Thing *thing, struct Thing *enemy)
{
    long dist,avgc;
    dist = get_2d_box_distance(&thing->mappos, &enemy->mappos);
    avgc = (enemy->field_56 + thing->field_56) / 2;
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

long creature_can_have_combat_with_creature(struct Thing *fighter1, struct Thing *fighter2, long a2, long a4, long a5)
{
    return _DK_creature_can_have_combat_with_creature(fighter1, fighter2, a2, a4, a5);
}

void set_creature_combat_state(struct Thing *fighter1, struct Thing *fighter2, long a3)
{
    _DK_set_creature_combat_state(fighter1, fighter2, a3);
}

long set_creature_in_combat_to_the_death(struct Thing *fighter1, struct Thing *fighter2, long a3)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(fighter1);
    if (cctrl->field_3 != 0)
    {
        ERRORLOG("Creature in combat already - adding till death");
        return false;
    }
    if (external_set_thing_state(fighter1, CrSt_CreatureInCombat))
    {
        set_creature_combat_state(fighter1, fighter2, a3);
        cctrl->field_AA = 0;
        cctrl->field_A9 = 1;
    }
    return true;
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
      i = cctrl->next_players_creature_idx;
      // Thing list loop body
      if ( (thing->model == crmodel) && (cctrl->field_3 == 0) )
      {
          if ( ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0) )
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
    cctrl->field_3 &= ~0x10;
    cctrl->word_A2 = 0;
    return 1;

}

short cleanup_object_combat(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_cleanup_object_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_3 &= ~0x08;
    cctrl->word_A2 = 0;
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
    if (((enemy->field_0 & 0x01) == 0) || (cctrl->long_9E != enemy->field_9))
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
    if (cctrl->field_A9)
    {
        return false;
    }
    return creature_is_actually_scared(thing, enemy);
}

TbBool creature_in_flee_zone(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long dist;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Creature no %d has invalid control",(int)thing->index);
        return false;
    }
    dist = get_2d_box_distance(&thing->mappos, &cctrl->pos_288);
    //TODO CREATURE_AI put flee_zone_radius into config file
    return (dist < 1536);
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

void creature_in_combat_wait(struct Thing *thing)
{
    _DK_creature_in_combat_wait(thing);
}

void creature_in_ranged_combat(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *enmtng;
    long dist, cmbtyp, weapon;
    //_DK_creature_in_ranged_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    enmtng = thing_get(cctrl->word_A2);
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
    struct CreatureControl *cctrl;
    struct Thing *enmtng;
    long dist, cmbtyp, weapon;
    //_DK_creature_in_melee_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    enmtng = thing_get(cctrl->word_A2);
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
    //return _DK_creature_in_combat(thing);
    cctrl = creature_control_get_from_thing(thing);
    enmtng = thing_get(cctrl->word_A2);
    if (!combat_enemy_exists(thing, enmtng))
    {
      set_start_state(thing);
      return 0;
    }
    if (creature_too_scared_for_combat(thing, enmtng))
    {
        if (!external_set_thing_state(thing, CrSt_CreatureCombatFlee))
        {
            ERRORLOG("Cannot get thing no %d, %s, into flee",(int)thing->index,thing_model_name(thing));
            return 0;
        }
        cctrl->field_28E = game.play_gameturn;
        return 0;
    }
    if (cctrl->byte_A6 < sizeof(combat_state)/sizeof(combat_state[0]))
        combat_func = combat_state[cctrl->byte_A6];
    else
        combat_func = NULL;
    if (combat_func != NULL)
    {
        combat_func(thing);
        return 1;
    }
    ERRORLOG("No valid fight state %d in thing no %d",(int)cctrl->byte_A6,(int)thing->index);
    set_start_state(thing);
    return 0;
}

short creature_object_combat(struct Thing *thing)
{
    return _DK_creature_object_combat(thing);
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
    if (a4 && ((cctrl1->field_3 & 0x18) == 0))
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
