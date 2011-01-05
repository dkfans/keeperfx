/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states.c
 *     Creature states structure and function definitions.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     23 Sep 2009 - 22 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states.h"
#include "globals.h"

#include "bflib_math.h"
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
#include "room_workshop.h"
#include "tasks_list.h"
#include "map_events.h"
#include "power_hand.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "thing_traps.h"
#include "sounds.h"

#include "creature_states_gardn.h"
#include "creature_states_hero.h"
#include "creature_states_lair.h"
#include "creature_states_mood.h"
#include "creature_states_prisn.h"
#include "creature_states_rsrch.h"
#include "creature_states_scavn.h"
#include "creature_states_spdig.h"
#include "creature_states_tortr.h"
#include "creature_states_train.h"
#include "creature_states_wrshp.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CREATURE_GUI_STATES_COUNT 3
/* Please note that functions returning 'short' are not ment to return true/false only! */
/******************************************************************************/
DLLIMPORT short _DK_already_at_call_to_arms(struct Thing *thing);
DLLIMPORT short _DK_arrive_at_alarm(struct Thing *thing);
DLLIMPORT short _DK_arrive_at_call_to_arms(struct Thing *thing);
DLLIMPORT short _DK_at_barrack_room(struct Thing *thing);
DLLIMPORT short _DK_at_guard_post_room(struct Thing *thing);
DLLIMPORT short _DK_at_research_room(struct Thing *thing);
DLLIMPORT short _DK_at_scavenger_room(struct Thing *thing);
DLLIMPORT short _DK_at_temple(struct Thing *thing);
DLLIMPORT short _DK_barracking(struct Thing *thing);
DLLIMPORT short _DK_cleanup_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_door_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_hold_audience(struct Thing *thing);
DLLIMPORT short _DK_cleanup_object_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_sacrifice(struct Thing *thing);
DLLIMPORT short _DK_cleanup_seek_the_enemy(struct Thing *thing);
DLLIMPORT short _DK_creature_arms_trap(struct Thing *thing);
DLLIMPORT short _DK_creature_arrived_at_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_attack_rooms(struct Thing *thing);
DLLIMPORT short _DK_creature_attempt_to_damage_walls(struct Thing *thing);
DLLIMPORT short _DK_creature_be_happy(struct Thing *thing);
DLLIMPORT short _DK_creature_being_dropped(struct Thing *thing);
DLLIMPORT short _DK_creature_being_sacrificed(struct Thing *thing);
DLLIMPORT short _DK_creature_being_scavenged(struct Thing *thing);
DLLIMPORT short _DK_creature_being_summoned(struct Thing *thing);
DLLIMPORT short _DK_creature_cannot_find_anything_to_do(struct Thing *thing);
DLLIMPORT short _DK_creature_change_from_chicken(struct Thing *thing);
DLLIMPORT short _DK_creature_change_to_chicken(struct Thing *thing);
DLLIMPORT short _DK_creature_combat_flee(struct Thing *thing);
DLLIMPORT short _DK_creature_damage_walls(struct Thing *thing);
DLLIMPORT short _DK_creature_doing_nothing(struct Thing *thing);
DLLIMPORT short _DK_creature_door_combat(struct Thing *thing);
DLLIMPORT short _DK_creature_dormant(struct Thing *thing);
DLLIMPORT short _DK_creature_drops_corpse_in_graveyard(struct Thing *thing);
DLLIMPORT short _DK_creature_drops_crate_in_workshop(struct Thing *thing);
DLLIMPORT short _DK_creature_drops_spell_object_in_library(struct Thing *thing);
DLLIMPORT short _DK_creature_eat(struct Thing *thing);
DLLIMPORT short _DK_creature_eating_at_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_escaping_death(struct Thing *thing);
DLLIMPORT short _DK_creature_evacuate_room(struct Thing *thing);
DLLIMPORT short _DK_creature_explore_dungeon(struct Thing *thing);
DLLIMPORT short _DK_creature_fired(struct Thing *thing);
DLLIMPORT short _DK_creature_follow_leader(struct Thing *thing);
DLLIMPORT short _DK_creature_hero_entering(struct Thing *thing);
DLLIMPORT short _DK_creature_in_combat(struct Thing *thing);
DLLIMPORT short _DK_creature_in_hold_audience(struct Thing *thing);
DLLIMPORT short _DK_creature_kill_creatures(struct Thing *thing);
DLLIMPORT short _DK_creature_leaves(struct Thing *thing);
DLLIMPORT short _DK_creature_leaves_or_dies(struct Thing *thing);
DLLIMPORT short _DK_creature_leaving_dungeon(struct Thing *thing);
DLLIMPORT short _DK_creature_moan(struct Thing *thing);
DLLIMPORT short _DK_creature_object_combat(struct Thing *thing);
DLLIMPORT short _DK_creature_persuade(struct Thing *thing);
DLLIMPORT short _DK_creature_pick_up_unconscious_body(struct Thing *thing);
DLLIMPORT short _DK_creature_picks_up_corpse(struct Thing *thing);
DLLIMPORT short _DK_creature_picks_up_spell_object(struct Thing *thing);
DLLIMPORT short _DK_creature_picks_up_trap_for_workshop(struct Thing *thing);
DLLIMPORT short _DK_creature_picks_up_trap_object(struct Thing *thing);
DLLIMPORT short _DK_creature_piss(struct Thing *thing);
DLLIMPORT short _DK_creature_present_to_dungeon_heart(struct Thing *thing);
DLLIMPORT short _DK_creature_pretend_chicken_move(struct Thing *thing);
DLLIMPORT short _DK_creature_pretend_chicken_setup_move(struct Thing *thing);
DLLIMPORT short _DK_creature_roar(struct Thing *thing);
DLLIMPORT short _DK_creature_sacrifice(struct Thing *thing);
DLLIMPORT short _DK_creature_scavenged_disappear(struct Thing *thing);
DLLIMPORT short _DK_creature_scavenged_reappear(struct Thing *thing);
DLLIMPORT short _DK_creature_search_for_gold_to_steal_in_room(struct Thing *thing);
DLLIMPORT short _DK_creature_set_work_room_based_on_position(struct Thing *thing);
DLLIMPORT short _DK_creature_slap_cowers(struct Thing *thing);
DLLIMPORT short _DK_creature_steal_gold(struct Thing *thing);
DLLIMPORT short _DK_creature_take_salary(struct Thing *thing);
DLLIMPORT short _DK_creature_to_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_unconscious(struct Thing *thing);
DLLIMPORT short _DK_creature_vandalise_rooms(struct Thing *thing);
DLLIMPORT short _DK_creature_wait_at_treasure_room_door(struct Thing *thing);
DLLIMPORT short _DK_creature_wants_a_home(struct Thing *thing);
DLLIMPORT short _DK_creature_wants_salary(struct Thing *thing);
DLLIMPORT short _DK_guarding(struct Thing *thing);
DLLIMPORT short _DK_mad_killing_psycho(struct Thing *thing);
DLLIMPORT short _DK_move_backwards_to_position(struct Thing *thing);
DLLIMPORT long _DK_move_check_attack_any_door(struct Thing *thing);
DLLIMPORT long _DK_move_check_can_damage_wall(struct Thing *thing);
DLLIMPORT long _DK_move_check_kill_creatures(struct Thing *thing);
DLLIMPORT long _DK_move_check_near_dungeon_heart(struct Thing *thing);
DLLIMPORT long _DK_move_check_on_head_for_room(struct Thing *thing);
DLLIMPORT long _DK_move_check_persuade(struct Thing *thing);
DLLIMPORT long _DK_move_check_wait_at_door_for_wage(struct Thing *thing);
DLLIMPORT short _DK_move_to_position(struct Thing *thing);
DLLIMPORT char _DK_new_slab_tunneller_check_for_breaches(struct Thing *thing);
DLLIMPORT short _DK_patrol_here(struct Thing *thing);
DLLIMPORT short _DK_patrolling(struct Thing *thing);
DLLIMPORT short _DK_person_sulk_at_lair(struct Thing *thing);
DLLIMPORT short _DK_person_sulk_head_for_lair(struct Thing *thing);
DLLIMPORT short _DK_person_sulking(struct Thing *thing);
DLLIMPORT short _DK_praying_in_temple(struct Thing *thing);
DLLIMPORT long _DK_process_research_function(struct Thing *thing);
DLLIMPORT long _DK_process_scavenge_function(struct Thing *thing);
DLLIMPORT long _DK_process_temple_function(struct Thing *thing);
DLLIMPORT short _DK_researching(struct Thing *thing);
DLLIMPORT short _DK_scavengering(struct Thing *thing);
DLLIMPORT short _DK_seek_the_enemy(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_dragging_body(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_dragging_object(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_in_room(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_in_temple(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_unable_to_fight(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_unconscious(struct Thing *thing);
DLLIMPORT short _DK_tunneller_doing_nothing(struct Thing *thing);
DLLIMPORT short _DK_tunnelling(struct Thing *thing);
DLLIMPORT long _DK_setup_random_head_for_room(struct Thing *thing, struct Room *room, unsigned char a3);
DLLIMPORT long _DK_process_sacrifice_award(struct Coord3d *pos, long model, long plyr_idx);
DLLIMPORT long _DK_make_all_players_creatures_angry(long plyr_idx);
DLLIMPORT long _DK_force_complete_current_manufacturing(long plyr_idx);
DLLIMPORT void _DK_apply_spell_effect_to_players_creatures(long a1, long a2, long a3);
DLLIMPORT void _DK_kill_all_players_chickens(long plyr_idx);
DLLIMPORT void _DK_force_complete_current_research(long plyr_idx);
DLLIMPORT void _DK_anger_set_creature_anger(struct Thing *thing, long a1, long a2);
DLLIMPORT void _DK_create_effect_around_thing(struct Thing *thing, long eff_kind);
DLLIMPORT void _DK_remove_health_from_thing_and_display_health(struct Thing *thing, long delta);
DLLIMPORT long _DK_slab_by_players_land(unsigned char plyr_idx, unsigned char slb_x, unsigned char slb_y);
DLLIMPORT struct Room *_DK_find_nearest_room_for_thing_excluding_two_types(struct Thing *thing, char owner, char a3, char a4, unsigned char a5);
DLLIMPORT unsigned char _DK_initialise_thing_state(struct Thing *thing, long a2);
DLLIMPORT long _DK_cleanup_current_thing_state(struct Thing *thing);
DLLIMPORT unsigned char _DK_find_random_valid_position_for_thing_in_room_avoiding_object(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT void _DK_creature_in_combat_wait(struct Thing *thing);
DLLIMPORT void _DK_creature_in_ranged_combat(struct Thing *thing);
DLLIMPORT void _DK_creature_in_melee_combat(struct Thing *thing);
DLLIMPORT long _DK_creature_has_other_attackers(struct Thing *thing, long a2);
DLLIMPORT long _DK_creature_is_most_suitable_for_combat(struct Thing *thing, struct Thing *enmtng);
DLLIMPORT long _DK_check_for_valid_combat(struct Thing *thing, struct Thing *enmtng);
DLLIMPORT long _DK_combat_type_is_choice_of_creature(struct Thing *thing, long cmbtyp);
DLLIMPORT long _DK_get_best_ranged_offensive_weapon(struct Thing *thing, long a2);
DLLIMPORT long _DK_ranged_combat_move(struct Thing *thing, struct Thing *enmtng, long a3, long a4);
DLLIMPORT long _DK_get_best_melee_offensive_weapon(struct Thing *thing, long a2);
DLLIMPORT long _DK_melee_combat_move(struct Thing *thing, struct Thing *enmtng, long a3, long a4);
DLLIMPORT long _DK_setup_head_for_empty_treasure_space(struct Thing *thing, struct Room *room);
DLLIMPORT short _DK_creature_choose_random_destination_on_valid_adjacent_slab(struct Thing *thing);
DLLIMPORT long _DK_person_get_somewhere_adjacent_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT long _DK_creature_can_have_combat_with_creature(struct Thing *fighter1, struct Thing *fighter2, long a2, long a4, long a5);
DLLIMPORT void _DK_set_creature_combat_state(struct Thing *fighter1, struct Thing *fighter2, long a3);
/******************************************************************************/
short already_at_call_to_arms(struct Thing *thing);
short arrive_at_alarm(struct Thing *thing);
short arrive_at_call_to_arms(struct Thing *thing);
short at_barrack_room(struct Thing *thing);
short at_guard_post_room(struct Thing *thing);
short at_research_room(struct Thing *thing);
short at_scavenger_room(struct Thing *thing);
short at_temple(struct Thing *thing);
short barracking(struct Thing *thing);
short cleanup_combat(struct Thing *thing);
short cleanup_door_combat(struct Thing *thing);
short cleanup_hold_audience(struct Thing *thing);
short cleanup_object_combat(struct Thing *thing);
short cleanup_sacrifice(struct Thing *thing);
short cleanup_seek_the_enemy(struct Thing *thing);
short creature_arms_trap(struct Thing *thing);
short creature_arrived_at_garden(struct Thing *thing);
short creature_attack_rooms(struct Thing *thing);
short creature_attempt_to_damage_walls(struct Thing *thing);
short creature_be_happy(struct Thing *thing);
short creature_being_dropped(struct Thing *thing);
short creature_being_sacrificed(struct Thing *thing);
short creature_being_scavenged(struct Thing *thing);
short creature_being_summoned(struct Thing *thing);
short creature_cannot_find_anything_to_do(struct Thing *thing);
short creature_change_from_chicken(struct Thing *thing);
short creature_change_to_chicken(struct Thing *thing);
short creature_combat_flee(struct Thing *thing);
short creature_damage_walls(struct Thing *thing);
short creature_doing_nothing(struct Thing *thing);
short creature_door_combat(struct Thing *thing);
short creature_dormant(struct Thing *thing);
short creature_drops_corpse_in_graveyard(struct Thing *thing);
short creature_drops_crate_in_workshop(struct Thing *thing);
short creature_drops_spell_object_in_library(struct Thing *thing);
short creature_eat(struct Thing *thing);
short creature_eating_at_garden(struct Thing *thing);
short creature_escaping_death(struct Thing *thing);
short creature_evacuate_room(struct Thing *thing);
short creature_explore_dungeon(struct Thing *thing);
short creature_fired(struct Thing *thing);
short creature_follow_leader(struct Thing *thing);
short creature_hero_entering(struct Thing *thing);
short creature_in_combat(struct Thing *thing);
short creature_in_hold_audience(struct Thing *thing);
short creature_kill_creatures(struct Thing *thing);
short creature_leaves(struct Thing *thing);
short creature_leaves_or_dies(struct Thing *thing);
short creature_leaving_dungeon(struct Thing *thing);
short creature_moan(struct Thing *thing);
short creature_object_combat(struct Thing *thing);
short creature_persuade(struct Thing *thing);
short creature_pick_up_unconscious_body(struct Thing *thing);
short creature_picks_up_corpse(struct Thing *thing);
short creature_picks_up_spell_object(struct Thing *thing);
short creature_picks_up_trap_for_workshop(struct Thing *thing);
short creature_picks_up_trap_object(struct Thing *thing);
short creature_piss(struct Thing *thing);
short creature_present_to_dungeon_heart(struct Thing *thing);
short creature_pretend_chicken_move(struct Thing *thing);
short creature_pretend_chicken_setup_move(struct Thing *thing);
short creature_roar(struct Thing *thing);
short creature_sacrifice(struct Thing *thing);
short creature_scavenged_disappear(struct Thing *thing);
short creature_scavenged_reappear(struct Thing *thing);
short creature_search_for_gold_to_steal_in_room(struct Thing *thing);
short creature_set_work_room_based_on_position(struct Thing *thing);
short creature_slap_cowers(struct Thing *thing);
short creature_steal_gold(struct Thing *thing);
short creature_take_salary(struct Thing *thing);
short creature_to_garden(struct Thing *thing);
short creature_unconscious(struct Thing *thing);
short creature_vandalise_rooms(struct Thing *thing);
short creature_wait_at_treasure_room_door(struct Thing *thing);
short creature_wants_a_home(struct Thing *thing);
short creature_wants_salary(struct Thing *thing);
short guarding(struct Thing *thing);
short mad_killing_psycho(struct Thing *thing);
short move_backwards_to_position(struct Thing *thing);
long move_check_attack_any_door(struct Thing *thing);
long move_check_can_damage_wall(struct Thing *thing);
long move_check_kill_creatures(struct Thing *thing);
long move_check_near_dungeon_heart(struct Thing *thing);
long move_check_on_head_for_room(struct Thing *thing);
long move_check_persuade(struct Thing *thing);
long move_check_wait_at_door_for_wage(struct Thing *thing);
short move_to_position(struct Thing *thing);
char new_slab_tunneller_check_for_breaches(struct Thing *thing);
short patrol_here(struct Thing *thing);
short patrolling(struct Thing *thing);
short person_sulk_at_lair(struct Thing *thing);
short person_sulk_head_for_lair(struct Thing *thing);
short person_sulking(struct Thing *thing);
short praying_in_temple(struct Thing *thing);
long process_research_function(struct Thing *thing);
long process_scavenge_function(struct Thing *thing);
long process_temple_function(struct Thing *thing);
short researching(struct Thing *thing);
short scavengering(struct Thing *thing);
short seek_the_enemy(struct Thing *thing);
short state_cleanup_dragging_body(struct Thing *thing);
short state_cleanup_dragging_object(struct Thing *thing);
short state_cleanup_in_room(struct Thing *thing);
short state_cleanup_in_temple(struct Thing *thing);
short state_cleanup_unable_to_fight(struct Thing *thing);
short state_cleanup_unconscious(struct Thing *thing);
short tunneller_doing_nothing(struct Thing *thing);
short tunnelling(struct Thing *thing);
short creature_search_for_spell_to_steal_in_room(struct Thing *thing);
short creature_steal_spell(struct Thing *thing);
void creature_in_combat_wait(struct Thing *thing);
void creature_in_ranged_combat(struct Thing *thing);
void creature_in_melee_combat(struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct StateInfo states[] = {
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {imp_doing_nothing, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {imp_arrives_at_dig_or_mine, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {imp_arrives_at_dig_or_mine, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {imp_digs_mines, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {imp_digs_mines, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 0, 0, 0, 0, 1},
  {imp_drops_gold, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {imp_last_did_job, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
  {imp_arrives_at_improve_dungeon, NULL, NULL,  NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {imp_improves_dungeon, NULL, NULL, NULL, // [10]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {creature_picks_up_trap_object, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {creature_arms_trap, state_cleanup_dragging_object, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {creature_picks_up_trap_for_workshop, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {move_to_position, NULL, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 0, 0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0,  0, 1, 0, 0, 0, 0, 1},
  {creature_drops_crate_in_workshop, state_cleanup_dragging_object, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  0, 0, 1},
  {creature_doing_nothing, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 1},
  {creature_to_garden, NULL, NULL, NULL,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 4, 0, 0, 1, 0, 59, 1, 0,  1},
  {creature_arrived_at_garden, state_cleanup_in_room, NULL, move_check_on_head_for_room,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 4, 0, 0, 2, 0, 59, 1, 0,  1},
  {creature_wants_a_home, NULL, NULL, NULL, // [20]
    0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 1, 0, 58, 1, 0,  1},
  {creature_choose_room_for_lair_site, NULL, NULL, NULL,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0,  1, 0, 58, 1, 0, 1},
  {creature_at_new_lair, NULL, NULL, NULL,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 1, 0, 58, 1, 0,  1},
  {person_sulk_head_for_lair, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0,   55, 1, 0, 1},
  {person_sulk_at_lair, NULL, NULL, NULL,
    0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 55, 1, 0,  1},
  {creature_going_home_to_sleep, NULL, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 0, 0, 1, 0,  54, 1, 0, 1},
  {creature_sleep, cleanup_sleep, NULL, NULL,
    0, 1, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 0, 0, 2, 0, 54, 1, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,  0, 1, 0, 0, 0, 0, 1},
  {tunnelling, NULL, new_slab_tunneller_check_for_breaches, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1},
  {at_research_room, NULL, NULL, move_check_on_head_for_room, // [30]
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 56, 1, 0,   1},
  {researching, state_cleanup_in_room, NULL, process_research_function,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 56, 1, 0,   1},
  {at_training_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 57, 1, 0,   1},
  {training, state_cleanup_in_room, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 57, 1, 0,   1},
  {good_doing_nothing, NULL, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
  {good_returns_to_start, NULL, NULL, NULL,
    0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  1},
  {good_back_at_start, NULL, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
  {good_drops_gold, NULL, NULL, NULL,
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0,  1, 1, 0, 0, 0, 0, 0},
  {arrive_at_call_to_arms, NULL, NULL, NULL,
    1,  0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 1, 0, 62,   1, 0, 0},
  {creature_arrived_at_prison, state_cleanup_unable_to_fight, NULL, move_check_on_head_for_room, // [40]
    1, 0,   1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 1, 0, 0, 0, 66, 1, 0,  0},
  {creature_in_prison, cleanup_prison, NULL, process_prison_function,
    1, 0, 1,   1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 1, 0, 0, 0, 66, 1, 0, 0},
  {at_torture_room, state_cleanup_unable_to_fight, NULL, move_check_on_head_for_room,
    1, 0,   1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 1, 0, 0, 0, 65, 1, 0,  0},
  {torturing, cleanup_torturing, NULL, process_torture_function,
    1, 0, 1,   1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 1, 0, 0, 0, 65, 1, 0, 0},
  {at_workshop_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 67, 1, 0,   1},
  {manufacturing, state_cleanup_in_room, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 67, 1, 0,   1},
  {at_scavenger_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 69, 1, 0,   1},
  {scavengering, state_cleanup_in_room, NULL, process_scavenge_function,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 69, 1, 0,   1},
  {creature_dormant, NULL, NULL, move_check_near_dungeon_heart,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0,  0, 1},
  {creature_in_combat, cleanup_combat, NULL, NULL,
    1, 1, 1, 0,   1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 5, 0, 0, 1, 1, 51, 1, 0, 0},
  {creature_leaving_dungeon, NULL, NULL, NULL, // [50]
    0,  1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 1, 1, 1, 0, 61,   1, 0, 1},
  {creature_leaves, NULL, NULL, NULL,
    0, 1, 1, 0,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 1, 1, 1, 0, 61, 1, 0, 1},
  {creature_in_hold_audience, cleanup_hold_audience, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
  {patrol_here, NULL, NULL, NULL,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
  {patrolling, NULL, NULL, NULL,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 0},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1},
  {creature_kill_creatures, NULL, NULL, move_check_kill_creatures,
    0, 1, 1,  0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 1, 0, 61, 1, 0,   1},
  {NULL, NULL, NULL, NULL, // [60]
    1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1},
  {person_sulking, NULL, NULL, NULL,
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 55, 1, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {at_barrack_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 63, 1, 0,   1},
  {barracking, state_cleanup_in_room, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 63, 1, 0,   1},
  {creature_slap_cowers, NULL, NULL, NULL,
    1,   1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0,  0, 1},
  {creature_unconscious, state_cleanup_unconscious, NULL, NULL,
    1,  1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 9, 0, 0, 0, 1, 0, 0,  0, 1},
  {creature_pick_up_unconscious_body, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,  0, 0, 0, 0, 1},
  {imp_toking, NULL, NULL, NULL,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {imp_picks_up_gold_pile, NULL, NULL, NULL, // [70]
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0,  0, 1},
  {move_backwards_to_position, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 0, 0,  0, 0, 1},
  {creature_drop_body_in_prison, state_cleanup_dragging_body, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0,  0, 1},
  {imp_arrives_at_convert_dungeon, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_converts_dungeon, NULL, NULL, NULL,
    0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  1},
  {creature_wants_salary, NULL, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 7, 0, 0, 1, 0, 52,   1, 0, 1},
  {creature_take_salary, NULL, NULL, move_check_wait_at_door_for_wage,
    0,  1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 7, 0, 0, 1, 0, 52,   1, 0, 1},
  {tunneller_doing_nothing, NULL, NULL, NULL,
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,  0, 1},
  {creature_object_combat, cleanup_object_combat, NULL, NULL,
    1, 1,   1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 12, 0, 0, 1, 0, 51, 1, 0,  0},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1},
  {creature_change_lair, NULL, NULL, move_check_on_head_for_room, // [80]
    0, 0,   1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 1, 0, 58, 1, 0,  1},
  {imp_birth, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {at_temple, NULL, NULL, move_check_on_head_for_room,
    0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 1, 0, 68, 1, 0,   1},
  {praying_in_temple, state_cleanup_in_temple, NULL, process_temple_function,
    0, 1, 0, 0,  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 2, 0, 68, 1, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 1},
  {creature_follow_leader, NULL, NULL, NULL,
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0,  0, 0},
  {creature_door_combat, cleanup_door_combat, NULL, NULL,
    1, 1, 0,  1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 11, 0, 0, 1, 0, 51, 1, 0, 0},
  {creature_combat_flee, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 53,   1, 0, 0},
  {creature_sacrifice, NULL, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
  {at_lair_to_sleep, NULL, NULL, NULL,
    0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 0, 0, 2, 0, 54, 1, 0,   1},
  {creature_fired, NULL, NULL, NULL, // [90]
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 61, 1, 0, 1},
  {creature_being_dropped, state_cleanup_unable_to_fight, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1},
  {creature_being_sacrificed, cleanup_sacrifice, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {creature_scavenged_disappear, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_scavenged_reappear, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0},
  {creature_being_summoned, cleanup_sacrifice, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {creature_hero_entering, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,  0, 1},
  {imp_arrives_at_reinforce, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,  0, 1},
  {imp_reinforces, NULL, NULL, NULL,
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {arrive_at_alarm, NULL, NULL, NULL,
    1, 1, 1, 0,  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 62, 1, 0, 0},
  {creature_picks_up_spell_object, NULL, NULL, NULL, // [100]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_drops_spell_object_in_library, state_cleanup_dragging_object, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,  0, 0, 1},
  {creature_picks_up_corpse, NULL, NULL, NULL,
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 0, 0, 0,  0, 1},
  {creature_drops_corpse_in_graveyard, state_cleanup_dragging_object, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,  0, 0, 1},
  {at_guard_post_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 50, 1, 0, 0},
  {guarding, state_cleanup_in_room, NULL, NULL,
    0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2, 0, 50, 1, 0, 0},
  {creature_eat, NULL, NULL, NULL,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 59, 1, 0, 1},
  {creature_evacuate_room, NULL, NULL, NULL,
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,  0, 1},
  {creature_wait_at_treasure_room_door,  NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0,  1, 0, 0, 0, 0, 1},
  {at_kinky_torture_room, NULL, NULL, move_check_on_head_for_room,
    1, 1,   1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  1},
  {kinky_torturing, cleanup_torturing, NULL, process_kinky_function, /// [110]
    1, 1, 1, 1,  1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {mad_killing_psycho, NULL, NULL, NULL,
    0, 1, 0,  0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 55, 1, 0,   1},
  {creature_search_for_gold_to_steal_in_room, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 55,   1, 0, 1},
  {creature_vandalise_rooms, NULL, NULL, move_check_attack_any_door,
    0, 1,   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 55, 1, 0,  1},
  {creature_steal_gold, NULL, NULL, move_check_attack_any_door,
    0, 1,   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 55, 1, 0,  1},
  {seek_the_enemy, cleanup_seek_the_enemy, NULL, NULL,
    0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  1},
  {already_at_call_to_arms, NULL, NULL, NULL,
    1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 1, 0,   62, 1, 0, 0},
  {creature_damage_walls, NULL, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0,  0, 1},
  {creature_attempt_to_damage_walls, NULL, NULL, move_check_can_damage_wall,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 55,   1, 0, 1},
  {creature_persuade, NULL, NULL, move_check_persuade,
    0, 1, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 1, 0, 55, 1, 0, 1},
  {creature_change_to_chicken, NULL, NULL, NULL, // [120]
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0,  0, 0, 0},
  {creature_change_from_chicken, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0,  0, 0, 0, 0},
  {NULL, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
  {creature_cannot_find_anything_to_do, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,  1, 0, 0, 0, 0, 1},
  {creature_piss, NULL, NULL, NULL,
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
  {creature_roar, NULL, NULL, NULL,
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 1, 0, 55, 1, 0, 1},
  {creature_at_changed_lair, NULL, NULL, NULL,
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0,  0, 1},
  {creature_be_happy, NULL, NULL, NULL,
    0, 0,   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  1},
  {good_leave_through_exit_door, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,  0, 0, 0, 1},
  {good_wait_in_exit_door, NULL, NULL, NULL,
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,  0, 1},
  {good_attack_room, NULL, NULL, NULL, // [130]
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1},
  {creature_search_for_gold_to_steal_in_room, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 1,   1, 0, 0, 0, 0, 0, 1},
  {good_attack_room, NULL, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1},
  {creature_pretend_chicken_setup_move, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1,  0, 0, 0, 0, 0, 0},
  {creature_pretend_chicken_move, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0,   0, 0, 0, 0},
  {creature_attack_rooms, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 51, 1, 0, 0},
  {creature_freeze_prisonors, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  0, 0, 1},
  {creature_explore_dungeon, NULL, NULL, NULL,
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 1},
  {creature_eating_at_garden, NULL, NULL, NULL,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 4, 0, 0, 2, 0,   59, 1, 0, 1},
  {creature_leaves_or_dies, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 2, 1, 1, 1, 0,   61, 1, 0, 0},
  {creature_moan, NULL, NULL, NULL, // [140]
    1, 1, 1, 0,  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 1},
  {creature_set_work_room_based_on_position, NULL, NULL, NULL,
    1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,  0, 0, 0, 0, 0, 1},
  {creature_being_scavenged, NULL, NULL, NULL,
    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 69, 1, 0, 0},
  {creature_escaping_death, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1,  8, 1, 0, 0,  0, 0, 1, 0, 0},
  {creature_present_to_dungeon_heart, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
  {creature_search_for_spell_to_steal_in_room, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 55, 1, 0, 1},
  {creature_steal_spell, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 55, 1, 0, 1},
  // Some redundant NULLs
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

const CombatState combat_state[] = {
    NULL,
    creature_in_combat_wait,
    creature_in_ranged_combat,
    creature_in_melee_combat,
};

/** GUI States of creatures - from "Creatures" Tab in UI.
 * There are three states:
 * - 0: Idle.
 * - 1: Working.
 * - 2: Fighting.
 */
long const state_type_to_gui_state[] = {
    0, 1, 0, 0, 0, 2, 0, 0, 1, 0, 0, 2, 2, 1, 1, 0,
};

/******************************************************************************/
struct StateInfo *get_thing_active_state_info(struct Thing *thing)
{
  if (thing->active_state >= CREATURE_STATES_COUNT)
    return &states[0];
  return &states[thing->active_state];
}

struct StateInfo *get_thing_continue_state_info(struct Thing *thing)
{
  if (thing->continue_state >= CREATURE_STATES_COUNT)
    return &states[0];
  return &states[thing->continue_state];
}

struct StateInfo *get_thing_state_info_num(long state_id)
{
  if ((state_id < 0) || (state_id >= CREATURE_STATES_COUNT))
    return &states[0];
  return &states[state_id];
}

long get_creature_real_state(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    return i;
}

struct StateInfo *get_creature_state_with_task_completion(struct Thing *thing)
{
  struct StateInfo *stati;
  stati = get_thing_active_state_info(thing);
  if (stati->state_type == 6)
      stati = get_thing_continue_state_info(thing);
  return stati;
}

TbBool state_info_invalid(struct StateInfo *stati)
{
  if (stati <= &states[0])
    return true;
  return false;
}

TbBool creature_model_bleeds(unsigned long crmodel)
{
  struct CreatureStats *crstat;
  struct CreatureModelConfig *crconf;
  crstat = creature_stats_get(crmodel);
  if ( censorship_enabled() )
  {
      // If censorship is on, only evil creatures can have blood
      if (!crstat->bleeds)
          return false;
      crconf = &crtr_conf.model[crmodel];
      return ((crconf->model_flags & MF_IsEvil) != 0);
  }
  return crstat->bleeds;
}
/******************************************************************************/
/** Returns type of given creature state.
 *
 * @param thing The source thing.
 * @return Type of the creature state.
 */
long get_creature_state_type(const struct Thing *thing)
{
  long state_type;
  long state;
  state = thing->active_state;
  if ( (state > 0) && (state < sizeof(states)/sizeof(states[0])) )
  {
      state_type = states[state].state_type;
  } else
  {
      state_type = states[0].state_type;
      WARNLOG("Creature active state %ld is out of range.",state);
  }
  if (state_type == 6)
  {
      state = thing->continue_state;
      if ( (state > 0) && (state < sizeof(states)/sizeof(states[0])) )
      {
          state_type = states[state].state_type;
      } else
      {
          state_type = states[0].state_type;
          WARNLOG("Creature continue state %ld is out of range.",state);
      }
  }
  return state_type;
}

/** Returns GUI Job of given creature.
 *  The GUI Job is a simplified version of creature state which
 *  only takes 3 values: 0-idle, 1-working, 2-fighting.
 *
 * @param thing The source thing.
 * @return GUI state, in range 0..2.
 */
long get_creature_gui_job(const struct Thing *thing)
{
    long state_type;
    state_type = get_creature_state_type(thing);
    if ( (state_type >= 0) && (state_type < sizeof(state_type_to_gui_state)/sizeof(state_type_to_gui_state[0])) )
    {
        return state_type_to_gui_state[state_type];
    } else
    {
        WARNLOG("The %s has invalid state type(%ld)!",thing_model_name(thing),state_type);
        erstat_inc(ESE_BadCreatrState);
        return state_type_to_gui_state[0];
    }
}

TbBool creature_is_doing_lair_activity(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_CreatureSleep)
        return true;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_CreatureGoingHomeToSleep) || (i == CrSt_AtLairToSleep)
      || (i == CrSt_CreatureChooseRoomForLairSite) || (i == CrSt_CreatureAtNewLair) || (i == CrSt_CreatureWantsAHome)
      || (i == CrSt_CreatureChangeLair) || (i == CrSt_CreatureAtChangedLair))
        return true;
    return false;
}

TbBool creature_is_being_dropped(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if (i == CrSt_CreatureBeingDropped)
        return true;
    return false;
}

TbBool creature_is_being_tortured(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_Torturing) || (i == CrSt_AtTortureRoom))
        return true;
    return false;
}

TbBool creature_is_being_sacrificed(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_CreatureSacrifice) || (i == CrSt_CreatureBeingSacrificed))
        return true;
    return false;
}

TbBool creature_is_kept_in_prison(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_CreatureInPrison) || (i == CrSt_CreatureArrivedAtPrison))
        return true;
    return false;
}

TbBool creature_is_being_summoned(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_CreatureBeingSummoned))
        return true;
    return false;
}

TbBool creature_is_training(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_Training) || (i == CrSt_AtTrainingRoom))
        return true;
    return false;
}

TbBool creature_is_doing_dungeon_improvements(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if (states[i].state_type == 10)
        return true;
    return false;
}

TbBool creature_is_doing_garden_activity(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_CreatureEat)
        return true;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_CreatureToGarden) || (i == CrSt_CreatureArrivedAtGarden))
        return true;
    return false;
}

TbBool creature_is_scavengering(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_Scavengering) || (i == CrSt_AtScavengerRoom))
        return true;
    return false;
}

TbBool creature_is_escaping_death(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_CreatureEscapingDeath))
        return true;
    return false;
}

TbBool creature_is_taking_salary_activity(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_CreatureWantsSalary)
        return true;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if (i == CrSt_CreatureTakeSalary)
        return true;
    return false;
}

TbBool creature_is_doing_temple_activity(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if ((i == CrSt_AtTemple) || (i == CrSt_PrayingInTemple))
        return true;
    return false;
}

TbBool creature_state_is_unset(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    if (states[i].state_type == 0)
        return true;
    return false;
}

short already_at_call_to_arms(struct Thing *thing)
{
    //return _DK_already_at_call_to_arms(thing);
    internal_set_thing_state(thing, CrSt_ArriveAtCallToArms);
    return 1;
}

short arrive_at_alarm(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_arrive_at_alarm(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->field_2FA < (unsigned long)game.play_gameturn)
    {
        set_start_state(thing);
        return 1;
    }
    if (ACTION_RANDOM(4) == 0)
    {
        if ( setup_person_move_close_to_position(thing, cctrl->field_2F8, cctrl->field_2F9, 0) )
        {
            thing->continue_state = CrSt_ArriveAtAlarm;
            return 1;
        }
    }
    if ( creature_choose_random_destination_on_valid_adjacent_slab(thing) )
    {
        thing->continue_state = CrSt_ArriveAtAlarm;
        return 1;
    }
    return 1;
}

struct Thing *check_for_door_to_fight(struct Thing *thing)
{
    struct Thing *doortng;
    long m,n;
    m = ACTION_RANDOM(4);
    for (n=0; n < 4; n++)
    {
        long slb_x,slb_y;
        slb_x = map_to_slab[thing->mappos.x.stl.num] + (long)small_around[m].delta_x;
        slb_y = map_to_slab[thing->mappos.y.stl.num] + (long)small_around[m].delta_y;
        doortng = get_door_for_position(3*slb_x+1, 3*slb_y+1);
        if (!thing_is_invalid(doortng))
        {
          if (thing->owner != doortng->owner)
              return doortng;
        }
        m = (m+1) % 4;
    }
    return NULL;
}

long setup_head_for_room(struct Thing *thing, struct Room *room, unsigned char a3)
{
    struct Coord3d pos;
    if ( !find_first_valid_position_for_thing_in_room(thing, room, &pos) )
        return false;
    return setup_person_move_to_position(thing, pos.x.stl.num, pos.y.stl.num, a3);
}

TbBool attempt_to_destroy_enemy_room(struct Thing *thing, unsigned char stl_x, unsigned char stl_y)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    struct Coord3d pos;
    room = subtile_room_get(stl_x, stl_y);
    if (room_is_invalid(room))
        return false;
    if (thing->owner == room->owner)
        return false;
    if ((room->kind == RoK_DUNGHEART) || (room->kind == RoK_ENTRANCE) || (room->kind == RoK_BRIDGE))
        return false;
    if ( !find_first_valid_position_for_thing_in_room(thing, room, &pos) )
        return false;
    if ( !creature_can_navigate_to_with_storage(thing, &pos, 1) )
        return false;

    if ( !setup_head_for_room(thing, room, 1) )
    {
        ERRORLOG("Cannot do this 'destroy room' stuff");
        return false;
    }
    event_create_event_or_update_nearby_existing_event(subtile_coord_center(room->stl_x),
        subtile_coord_center(room->stl_y), 19, room->owner, 0);
    if (is_my_player_number(room->owner))
        output_message(15, 400, 1);
    thing->continue_state = CrSt_CreatureAttackRooms;
    cctrl = creature_control_get_from_thing(thing);
    if (!creature_control_invalid(cctrl))
        cctrl->field_80 = room->index;
    return true;
}

short arrive_at_call_to_arms(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct Thing *doortng;
    SYNCDBG(18,"Starting");
    //return _DK_arrive_at_call_to_arms(thing);
    dungeon = get_dungeon(thing->owner);
    if (dungeon->field_884 == 0)
    {
        set_start_state(thing);
        return 1;
    }
    doortng = check_for_door_to_fight(thing);
    if (!thing_is_invalid(doortng))
    {
        set_creature_door_combat(thing, doortng);
        return 2;
    }
    if ( !attempt_to_destroy_enemy_room(thing, dungeon->field_881, dungeon->field_882) )
    {
      if (ACTION_RANDOM(7) == 0)
      {
          if ( setup_person_move_close_to_position(thing, dungeon->field_881, dungeon->field_882, 0) )
          {
              thing->continue_state = CrSt_AlreadyAtCallToArms;
              return 1;
          }
      }
      if ( creature_choose_random_destination_on_valid_adjacent_slab(thing) )
      {
        thing->continue_state = CrSt_AlreadyAtCallToArms;
        return 1;
      }
    }
    return 1;
}

short at_barrack_room(struct Thing *thing)
{
    struct Room *room;
    struct CreatureControl *cctrl;
    //return _DK_at_barrack_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_BARRACKS) || (room->owner != thing->owner))
    {
        WARNLOG("Room of kind %d and owner %d is invalid for %s",(int)room->kind,(int)room->owner,thing_model_name(thing));
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_Barracking);
    return 1;
}

long person_get_somewhere_adjacent_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_person_get_somewhere_adjacent_in_room(thing, room, pos);
}

short at_guard_post_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_at_guard_post_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_GUARDPOST) || (room->owner != thing->owner))
    {
        WARNLOG("Room of kind %d and owner %d is invalid for %s",(int)room->kind,(int)room->owner,thing_model_name(thing));
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_Guarding);
    if ( !person_get_somewhere_adjacent_in_room(thing, room, &cctrl->moveto_pos) )
    {
        cctrl->moveto_pos.x.val = thing->mappos.x.val;
        cctrl->moveto_pos.y.val = thing->mappos.y.val;
        cctrl->moveto_pos.z.val = thing->mappos.z.val;
    }
    return 1;
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
      i = cctrl->thing_idx;
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

short at_research_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_research_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    if (thing->owner == game.neutral_player_num)
    {
        set_start_state(thing);
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if ((crstat->research_value <= 0) || (dungeon->field_F78 < 0))
    {
        if ((thing->owner != game.neutral_player_num) && (dungeon->field_F78 < 0))
        {
            if (is_my_player_number(dungeon->field_E9F))
                output_message(46, 500, 1);
        }
        set_start_state(thing);
        return 0;
    }
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_LIBRARY) || (room->owner != thing->owner))
    {
        WARNLOG("Room of kind %d and owner %d is invalid for %s",(int)room->kind,(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room))
    {
        set_start_state(thing);
        return 0;
    }
    if ( !setup_random_head_for_room(thing, room, 0) )
    {
        ERRORLOG("The %s can not move in research room", thing_model_name(thing));
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    thing->continue_state = CrSt_Researching;
    cctrl->field_82 = 0;
    cctrl->byte_9A = 3;
    return 1;
}

short at_scavenger_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_scavenger_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_SCAVENGER) || (room->owner != thing->owner))
    {
        WARNLOG("Room of kind %d and owner %d is invalid for %s",(int)room->kind,(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (crstat->scavenger_cost >= dungeon->field_AF9)
    {
        if (is_my_player_number(thing->owner))
            output_message(88, 500, 1);
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_Scavengering);
    cctrl->field_82 = 0;
    return 1;
}

short at_temple(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_temple(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_TEMPLE) || (room->owner != thing->owner))
    {
        WARNLOG("Room of kind %d and owner %d is invalid for %s",(int)room->kind,(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    dungeon = get_dungeon(thing->owner);
    if ( !add_creature_to_work_room(thing, room) )
    {
        if (is_my_player_number(thing->owner))
            output_message(31, 0, 1);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_PrayingInTemple);
    dungeon->creatures_praying[thing->model]++;
    cctrl->field_82 = 0;
    return 1;
}

SubtlCodedCoords find_position_around_in_room(struct Coord3d *pos, long owner, long rkind)
{
    SubtlCodedCoords stl_num;
    SubtlCodedCoords accepted_stl_num;
    long m,n;
    long dist;
    m = ACTION_RANDOM(AROUND_MAP_LENGTH);
    for (n = 0; n < AROUND_MAP_LENGTH; n++)
    {
        accepted_stl_num = 0;
        stl_num = get_subtile_number(pos->x.stl.num,pos->y.stl.num);
        // Skip the position equal to current position
        if (around_map[m] == 0)
        {
            m = (m + 1) % AROUND_MAP_LENGTH;
            continue;
        }
        // Move radially from of the current position; stop if a room tile
        // of incorrect kind or owner is encoured
        for (dist = 0; dist < 8; dist++)
        {
            struct Room *room;
            struct Map *mapblk;
            struct SlabMap *slb;
            MapSubtlCoord stl_x,stl_y;
            stl_num += around_map[m];
            mapblk = get_map_block_at_pos(stl_num);
            if ( ((mapblk->flags & 0x02) != 0) && ((mapblk->flags & 0x10) != 0) )
                break;
            stl_x = stl_num_decode_x(stl_num);
            stl_y = stl_num_decode_y(stl_num);
            slb = get_slabmap_block(map_to_slab[stl_x],map_to_slab[stl_y]);
            if (slabmap_owner(slb) != owner)
                break;
            room = room_get(slb->room_index);
            if (room->kind != rkind)
                break;
            if (dist > 1)
            {
                // Accept, but don't just break the loop. Wait for larger distance.
                accepted_stl_num = stl_num;
            }
        }
        if (accepted_stl_num > 0)
            return accepted_stl_num;
          m = (m + 1) % AROUND_MAP_LENGTH;
    }
    return 0;
}

short barracking(struct Thing *thing)
{
  return _DK_barracking(thing);
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

short cleanup_hold_audience(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_cleanup_hold_audience(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
    return 0;
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

short cleanup_sacrifice(struct Thing *thing)
{
  return _DK_cleanup_sacrifice(thing);
}

short cleanup_seek_the_enemy(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_cleanup_seek_the_enemy(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->word_9A = 0;
    cctrl->long_9C = 0;
    return 1;
}

short creature_arms_trap(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *traptng;
    struct Thing *postng;
    struct Thing *cratetng;
    //return _DK_creature_arms_trap(thing);
    cctrl = creature_control_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    cratetng = thing_get(cctrl->field_6E);
    traptng = thing_get(cctrl->field_70);
    if ( !thing_exists(cratetng) || !thing_exists(traptng) )
    {
        set_start_state(thing);
        return 0;
    }
    postng = get_trap_at_subtile_of_model_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, traptng->model, thing->owner);
    // Note that this means there can be only one trap of given kind at a subtile.
    // Otherwise it won't be possible to rearm it, as the condition below will fail.
    if ( (postng != traptng) || (traptng->byte_13 > 0) )
    {
        ERRORLOG("The %s has moved or been already rearmed",thing_model_name(traptng));
        set_start_state(thing);
        return 0;
    }
    traptng->byte_13 = game.traps_config[traptng->model].shots;
    traptng->field_4F ^= (traptng->field_4F ^ (trap_stats[traptng->model].field_12 << 4)) & 0x30;
    dungeon->lvstats.traps_armed++;
    creature_drop_dragged_object(thing, cratetng);
    delete_thing_structure(cratetng, 0);
    set_start_state(thing);
    return 1;
}

short creature_arrived_at_garden(struct Thing *thing)
{
  return _DK_creature_arrived_at_garden(thing);
}

short creature_attack_rooms(struct Thing *thing)
{
  return _DK_creature_attack_rooms(thing);
}

short creature_attempt_to_damage_walls(struct Thing *thing)
{
  return _DK_creature_attempt_to_damage_walls(thing);
}

short creature_be_happy(struct Thing *thing)
{
  return _DK_creature_be_happy(thing);
}

short creature_being_dropped(struct Thing *thing)
{
  return _DK_creature_being_dropped(thing);
}

void anger_set_creature_anger(struct Thing *thing, long annoy_lv, long reason)
{
  SYNCDBG(8,"Setting to %d",(int)annoy_lv);
  _DK_anger_set_creature_anger(thing, annoy_lv, reason);
}

TbBool anger_is_creature_livid(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    return ((cctrl->field_66 & 0x02) != 0);
}

TbBool anger_is_creature_angry(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    return ((cctrl->field_66 & 0x01) != 0);
}

TbBool anger_make_creature_angry(struct Thing *thing, long reason)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  if ((crstat->annoy_level <= 0) || ((cctrl->field_66 & 0x01) != 0))
    return false;
  anger_set_creature_anger(thing, crstat->annoy_level, reason);
  return true;
}

TbBool make_all_players_creatures_angry(long plyr_idx)
{
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  unsigned long k;
  int i;
  SYNCDBG(8,"Starting");
  //return _DK_make_all_players_creatures_angry(plyr_idx);
  dungeon = get_players_num_dungeon(plyr_idx);
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
    i = cctrl->thing_idx;
    // Thing list loop body
    anger_make_creature_angry(thing, 4);
    // Thing list loop body ends
    k++;
    if (k > CREATURES_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping creatures list");
      break;
    }
  }
  SYNCDBG(19,"Finished");
  return true;
}

long force_complete_current_manufacturing(long plyr_idx)
{
  return _DK_force_complete_current_manufacturing(plyr_idx);
}

void apply_spell_effect_to_players_creatures(long plyr_idx, long spl_idx, long overchrg)
{
  _DK_apply_spell_effect_to_players_creatures(plyr_idx, spl_idx, overchrg);
}

void kill_all_players_chickens(long plyr_idx)
{
  _DK_kill_all_players_chickens(plyr_idx);
}

void force_complete_current_research(long plyr_idx)
{
  _DK_force_complete_current_research(plyr_idx);
}

TbBool summon_creature(long model, struct Coord3d *pos, long owner, long explevel)
{
  struct CreatureControl *cctrl;
  struct Thing *thing;
  SYNCDBG(4,"Creating model %ld for player %ld",model,owner);
  thing = create_creature(pos, model, owner);
  if (thing_is_invalid(thing))
  {
    ERRORLOG("Could not create creature");
    return false;
  }
  init_creature_level(thing, explevel);
  internal_set_thing_state(thing, 95);
  thing->field_25 |= 0x04;
  cctrl = creature_control_get_from_thing(thing);
  cctrl->word_9C = 48;
  return true;
}

long create_sacrifice_unique_award(struct Coord3d *pos, long plyr_idx, long sacfunc, long explevel)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(plyr_idx);
  switch (sacfunc)
  {
  case UnqF_MkAllAngry:
      make_all_players_creatures_angry(plyr_idx);
      return SacR_Punished;
  case UnqF_ComplResrch:
      force_complete_current_research(plyr_idx);
      return SacR_Awarded;
  case UnqF_ComplManufc:
      force_complete_current_manufacturing(plyr_idx);
      return SacR_Awarded;
  case UnqF_KillChickns:
      kill_all_players_chickens(plyr_idx);
      return SacR_Punished;
  case UnqF_CheaperImp:
      // No processing needed - just don't clear the amount of sacrificed imps.
      return SacR_Pleased;
  default:
      ERRORLOG("Unsupported unique secrifice award!");
      return SacR_AngryWarn;
  }
}

long creature_sacrifice_average_explevel(struct Dungeon *dungeon, struct SacrificeRecipe *sac)
{
  long num;
  long exp;
  long i;
  long model;
  num = 0;
  exp = 0;
  for (i=0; i < MAX_SACRIFICE_VICTIMS; i++)
  {
    model = sac->victims[i];
    // Do not count the same model twice
    if (i > 0)
    {
      if (model == sac->victims[i-1])
        break;
    }
    num += dungeon->creature_sacrifice[model];
    exp += dungeon->creature_sacrifice_exp[model];
  }
  if (num < 1) num = 1;
  exp = (exp/num);
  if (exp < 0) return 0;
  return exp;
}

void creature_sacrifice_reset(struct Dungeon *dungeon, struct SacrificeRecipe *sac)
{
  long i;
  long model;
  // Some models may be set more than once; dut we don't really care...
  for (i=0; i < MAX_SACRIFICE_VICTIMS; i++)
  {
    model = sac->victims[i];
    dungeon->creature_sacrifice[model] = 0;
    dungeon->creature_sacrifice_exp[model] = 0;
  }
}

long sacrifice_victim_model_count(struct SacrificeRecipe *sac, long model)
{
  long i;
  long k;
  k = 0;
  for (i=0; i < MAX_SACRIFICE_VICTIMS; i++)
  {
    if (sac->victims[i] == model) k++;
  }
  return k;
}

TbBool sacrifice_victim_conditions_met(struct Dungeon *dungeon, struct SacrificeRecipe *sac)
{
  long i,required;
  long model;
  // Some models may be checked more than once; dut we don't really care...
  for (i=0; i < MAX_SACRIFICE_VICTIMS; i++)
  {
    model = sac->victims[i];
    if (model < 1) continue;
    required = sacrifice_victim_model_count(sac, model);
    SYNCDBG(6,"Model %d exists %d times",(int)model,(int)required);
    if (dungeon->creature_sacrifice[model] < required)
      return false;
  }
  return true;
}

long process_sacrifice_award(struct Coord3d *pos, long model, long plyr_idx)
{
  struct SacrificeRecipe *sac;
  struct Dungeon *dungeon;
  long explevel;
  long ret;
  //return _DK_process_sacrifice_award(pos, model, plyr_idx);
  dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
  {
    ERRORLOG("Player %d cannot sacrifice creatures.",plyr_idx);
    return 0;
  }
  ret = SacR_DontCare;
  sac = &gameadd.sacrifice_recipes[0];
  do {
    // Check if the just sacrificed creature is in the sacrifice
    if (sacrifice_victim_model_count(sac,model) > 0)
    {
      // Set the return value in case of partial sacrifice recipe
      if (ret != SacR_Pleased)
      {
        switch (sac->action)
        {
        case SacA_MkGoodHero:
        case SacA_NegSpellAll:
        case SacA_NegUniqFunc:
          ret = SacR_AngryWarn;
          break;
        default:
          ret = SacR_Pleased;
          break;
        }
      }
      SYNCDBG(8,"Creature %d used in sacrifice %d",(int)model,(int)(sac-&gameadd.sacrifice_recipes[0]));
      // Check if the complete sacrifice condition is met
      if (sacrifice_victim_conditions_met(dungeon, sac))
      {
        SYNCDBG(6,"Sacrifice recipe %d condition met, action %d for player %d",(int)(sac-&gameadd.sacrifice_recipes[0]),(int)sac->action,(int)plyr_idx);
        explevel = creature_sacrifice_average_explevel(dungeon, sac);
        switch (sac->action)
        {
        case SacA_MkCreature:
            if (explevel >= CREATURE_MAX_LEVEL) explevel = CREATURE_MAX_LEVEL-1;
            if ( summon_creature(sac->param, pos, plyr_idx, explevel) )
              dungeon->lvstats.creatures_from_sacrifice++;
            ret = SacR_Awarded;
            break;
        case SacA_MkGoodHero:
            if (explevel >= CREATURE_MAX_LEVEL) explevel = CREATURE_MAX_LEVEL-1;
            if ( summon_creature(sac->param, pos, 4, explevel) )
              dungeon->lvstats.creatures_from_sacrifice++;
            ret = SacR_Punished;
            break;
        case SacA_NegSpellAll:
            if (explevel > SPELL_MAX_LEVEL) explevel = SPELL_MAX_LEVEL;
            apply_spell_effect_to_players_creatures(plyr_idx, sac->param, explevel);
            ret = SacR_Punished;
            break;
        case SacA_PosSpellAll:
            if (explevel > SPELL_MAX_LEVEL) explevel = SPELL_MAX_LEVEL;
            apply_spell_effect_to_players_creatures(plyr_idx, sac->param, explevel);
            ret = SacR_Awarded;
            break;
        case SacA_NegUniqFunc:
        case SacA_PosUniqFunc:
            ret = create_sacrifice_unique_award(pos, plyr_idx, sac->param, explevel);
            break;
        default:
            ERRORLOG("Unsupported sacrifice action %d!",(int)sac->action);
            ret = SacR_Pleased;
            break;
        }
        if ((ret != SacR_Pleased) && (ret != SacR_AngryWarn))
          creature_sacrifice_reset(dungeon, sac);
        return ret;
      }
    }
    sac++;
  } while (sac->action != SacA_None);
  return ret;
}

short creature_being_sacrificed(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct SlabMap *slb;
  struct Coord3d pos;
  long owner,model,award;
  SYNCDBG(6,"Starting");
  //return _DK_creature_being_sacrificed(thing);

  cctrl = creature_control_get_from_thing(thing);
  cctrl->word_9A--;
  if (cctrl->word_9A > 0)
  {
    award = creature_turn_to_face_angle(thing, thing->field_52 + 256);
    thing->field_25 &= 0xDFu;
    return 0;
  }
  slb = get_slabmap_for_subtile(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
  owner = slabmap_owner(slb);
  add_creature_to_sacrifice_list(owner, thing->model, cctrl->explevel);
  pos.x.val = thing->mappos.x.val;
  pos.y.val = thing->mappos.y.val;
  pos.z.val = thing->mappos.z.val;
  model = thing->model;
  kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
  award = process_sacrifice_award(&pos, model, owner);
  if (is_my_player_number(owner))
  {
    switch (award)
    {
    case SacR_AngryWarn:
        output_message(68, 0, 1);
        break;
    case SacR_DontCare:
        output_message(67, 0, 1);
        break;
    case SacR_Pleased:
        output_message(65, 0, 1);
        break;
    case SacR_Awarded:
        output_message(66, 0, 1);
        break;
    case SacR_Punished:
        output_message(69, 0, 1);
        break;
    default:
        ERRORLOG("Invalid sacrifice return");
        break;
    }
  }
  return -1;
}

short creature_being_scavenged(struct Thing *thing)
{
  return _DK_creature_being_scavenged(thing);
}

short creature_being_summoned(struct Thing *thing)
{
  return _DK_creature_being_summoned(thing);
}

short creature_cannot_find_anything_to_do(struct Thing *thing)
{
  return _DK_creature_cannot_find_anything_to_do(thing);
}

short creature_change_from_chicken(struct Thing *thing)
{
  return _DK_creature_change_from_chicken(thing);
}

short creature_change_to_chicken(struct Thing *thing)
{
  return _DK_creature_change_to_chicken(thing);
}

short creature_combat_flee(struct Thing *thing)
{
  return _DK_creature_combat_flee(thing);
}

short creature_damage_walls(struct Thing *thing)
{
  return _DK_creature_damage_walls(thing);
}

short creature_doing_nothing(struct Thing *thing)
{
  return _DK_creature_doing_nothing(thing);
}

short creature_door_combat(struct Thing *thing)
{
  return _DK_creature_door_combat(thing);
}

TbBool creature_choose_random_destination_on_valid_adjacent_slab(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    struct Thing *doortng;
    long base_x,base_y;
    long stl_x,stl_y;
    long start_stl;
    long slab_num,slab_base;
    long i,k,m,n;
    TbBool do_move;
    long x,y;
    SYNCDBG(17,"Starting for thing %d",(long)thing->index);
    //return _DK_creature_choose_random_destination_on_valid_adjacent_slab(thing);

    stl_x = thing->mappos.x.stl.num;
    stl_y = thing->mappos.y.stl.num;

    slab_base = get_slab_number(map_to_slab[stl_x], map_to_slab[stl_y]);

    start_stl = ACTION_RANDOM(9);
    m = ACTION_RANDOM(SMALL_AROUND_SLAB_LENGTH);
    for (n=0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        slab_num = slab_base + small_around_slab[m];
        slb = get_slabmap_direct(slab_num);
        slbattr = get_slab_attrs(slb);
        do_move = false;
        if ( ((slbattr->field_6 & 0x02) != 0) || ((slbattr->field_6 & 0x10) == 0) )
            do_move = true;
        base_x = 3 * slb_num_decode_x(slab_num);
        base_y = 3 * slb_num_decode_y(slab_num);
        if (!do_move)
        {
          doortng = get_door_for_position(base_x, base_y);
          if (!thing_is_invalid(doortng))
          {
            if ((doortng->owner == thing->owner) && (!doortng->byte_18))
                do_move = true;
          }
        }
        if (do_move)
        {
            k = start_stl;
            for (i=0; i < 9; i++)
            {
              x = base_x + (k%3);
              y = base_y + (k/3);
              if ((x != stl_x) || (y != stl_y))
              {
                  crstat = creature_stats_get_from_thing(thing);
                  if ((crstat->hurt_by_lava <= 0) || !map_pos_is_lava(stl_x,stl_y))
                  {
                      if (setup_person_move_to_position(thing, x, y, 0))
                      {
                          SYNCDBG(8,"Moving thing %d from (%d,%d) to (%d,%d)",(int)thing->index,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num,(int)x,(int)y);
                          return true;
                      }
                  }
              }
              k = (k+1) % 9;
            }
            if (slb->kind != SlbT_LAVA)
            {
                SYNCDBG(8,"Found non lava around");
                return false;
            }
        }
        m = (m+1) % SMALL_AROUND_SLAB_LENGTH;
    }
    base_x = 3 * map_to_slab[thing->mappos.x.stl.num];
    base_y = 3 * map_to_slab[thing->mappos.y.stl.num];
    k = start_stl;
    for (i=0; i < 9; i++)
    {
        x = base_x + (k%3);
        y = base_y + (k/3);
        if ((x != stl_x) || (y != stl_y))
        {
          if (setup_person_move_to_position(thing, x, y, 0))
          {
              SYNCDBG(8,"Moving thing %d from (%d,%d) to (%d,%d)",(int)thing->index,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num,(int)x,(int)y);
              return true;
          }
        }
        k = (k+1) % 9;
    }
    SYNCDBG(8,"Moving thing %d from (%d,%d) failed",(int)thing->index,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
    return false;
}

short creature_dormant(struct Thing *thing)
{
    //return _DK_creature_dormant(thing);
    if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
    {
      thing->continue_state = CrSt_CreatureDormant;
      return 1;
    }
    return 0;
}

short creature_drops_corpse_in_graveyard(struct Thing *thing)
{
  return _DK_creature_drops_corpse_in_graveyard(thing);
}

void creature_drop_dragged_object(struct Thing *crtng, struct Thing *dragtng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(crtng);
    if (cctrl->field_6E == 0)
    {
        ERRORLOG("The %s isn't dragging anything",thing_model_name(crtng));
    } else
    if (dragtng->index != cctrl->field_6E)
    {
        ERRORLOG("The %s isn't dragging %s",thing_model_name(crtng),thing_model_name(dragtng));
    }
    cctrl->field_6E = 0;
    dragtng->field_0 &= ~0x80;
    dragtng->field_1 &= ~0x01;
    move_thing_in_map(dragtng, &crtng->mappos);
    if (dragtng->field_62 != 0)
        light_turn_light_on(dragtng->field_62);
}

short creature_drops_crate_in_workshop(struct Thing *thing)
{
    struct Thing *cratetng;
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_creature_drops_crate_in_workshop(thing);
    cctrl = creature_control_get_from_thing(thing);
    cratetng = thing_get(cctrl->field_6E);
    // Check if chate is ok
    if ( !thing_exists(cratetng) )
    {
        set_start_state(thing);
        return 0;
    }
    // Check if we're on correct room
    room = get_room_thing_is_on(thing);
    if ( room_is_invalid(room) )
    {
        set_start_state(thing);
        return 0;
    }

    if ( (room->kind != RoK_WORKSHOP) || (room->owner != thing->owner)
        || (room->used_capacity >= room->total_capacity) )
    {
      set_start_state(thing);
      return 0;
    }
    creature_drop_dragged_object(thing, cratetng);
    cratetng->owner = thing->owner;
    add_workshop_object_to_workshop(room);
    add_workshop_item(room->owner, get_workshop_object_class_for_thing(cratetng),
        box_thing_to_door_or_trap(cratetng));
    set_start_state(thing);
    return 1;
}

short creature_drops_spell_object_in_library(struct Thing *thing)
{
  return _DK_creature_drops_spell_object_in_library(thing);
}

short creature_eat(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  //return _DK_creature_eat(thing);
  cctrl = creature_control_get_from_thing(thing);
  if (cctrl->field_D2 != 36)
    internal_set_thing_state(thing, thing->continue_state);
  return 1;
}

short creature_eating_at_garden(struct Thing *thing)
{
  return _DK_creature_eating_at_garden(thing);
}

short creature_escaping_death(struct Thing *thing)
{
  return _DK_creature_escaping_death(thing);
}

short creature_evacuate_room(struct Thing *thing)
{
  return _DK_creature_evacuate_room(thing);
}

short creature_explore_dungeon(struct Thing *thing)
{
  return _DK_creature_explore_dungeon(thing);
}

short creature_fired(struct Thing *thing)
{
  return _DK_creature_fired(thing);
}

short creature_follow_leader(struct Thing *thing)
{
  return _DK_creature_follow_leader(thing);
}

short creature_hero_entering(struct Thing *thing)
{
  return _DK_creature_hero_entering(thing);
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

short creature_in_hold_audience(struct Thing *thing)
{
  return _DK_creature_in_hold_audience(thing);
}

short creature_kill_creatures(struct Thing *thing)
{
  return _DK_creature_kill_creatures(thing);
}

short creature_leaves(struct Thing *thing)
{
  return _DK_creature_leaves(thing);
}

short creature_leaves_or_dies(struct Thing *thing)
{
  return _DK_creature_leaves_or_dies(thing);
}

short creature_leaving_dungeon(struct Thing *thing)
{
  return _DK_creature_leaving_dungeon(thing);
}

short creature_moan(struct Thing *thing)
{
  return _DK_creature_moan(thing);
}

short creature_object_combat(struct Thing *thing)
{
  return _DK_creature_object_combat(thing);
}

short creature_persuade(struct Thing *thing)
{
  return _DK_creature_persuade(thing);
}

short creature_pick_up_unconscious_body(struct Thing *thing)
{
  return _DK_creature_pick_up_unconscious_body(thing);
}

short creature_picks_up_corpse(struct Thing *thing)
{
  return _DK_creature_picks_up_corpse(thing);
}

void creature_drag_object(struct Thing *thing, struct Thing *dragtng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_6E = dragtng->index;
    dragtng->field_0 |= 0x80;
    dragtng->field_1 |= 0x01;
    dragtng->owner = game.neutral_player_num;
    if (dragtng->field_62 != 0)
    {
      light_turn_light_off(dragtng->field_62);
    }
}

void remove_spell_from_player(long spl_idx, long plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon->magic_level[spl_idx] < 1)
    {
        ERRORLOG("Cannot remove a spell from player as he doesn't have it!");
        return;
    }
    dungeon->magic_level[spl_idx]--;
    switch ( spl_idx )
    {
    case 3:
        if (dungeon->field_888)
            dungeon->field_888 = 0;
        break;
    case 5:
        if (dungeon->field_5D8)
            turn_off_sight_of_evil(plyr_idx);
        break;
    case 6:
        if (dungeon->field_884)
            turn_off_call_to_arms(plyr_idx);
        break;
    }
}

unsigned char find_random_valid_position_for_thing_in_room_avoiding_object(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_find_random_valid_position_for_thing_in_room_avoiding_object(thing, room, pos);
}

TbBool remove_spell_from_library(struct Room *room, struct Thing *spelltng, long new_owner)
{
    if (room->kind != RoK_LIBRARY)
    {
        SYNCDBG(4,"Spell was placed in a room of kind %d instead of library",(int)room->kind);
        return false;
    }
    if (spelltng->owner != room->owner)
    {
        SYNCDBG(4,"Spell owned by player %d was placed in a room owned by player %d",(int)spelltng->owner,(int)room->owner);
        return false;
    }
    if ((room->long_17 <= 0) || (room->used_capacity <= 0))
    {
        ERRORLOG("Trying to remove spell from a room with no spell");
        return false;
    }
    room->long_17--;
    room->used_capacity--;
    remove_spell_from_player(object_to_magic[spelltng->model], room->owner);
    if (is_my_player_number(room->owner))
    {
        output_message(50, 0, 1);
    } else
    if (is_my_player_number(new_owner))
    {
        output_message(47, 0, 1);
    }
    return true;
}

short creature_picks_up_spell_object(struct Thing *thing)
{
    struct Room *enmroom, *ownroom;
    struct CreatureControl *cctrl;
    struct Thing *spelltng;
    struct Coord3d pos;
    //return _DK_creature_picks_up_spell_object(thing);
    cctrl = creature_control_get_from_thing(thing);
    spelltng = thing_get(cctrl->field_72);
    if ( thing_is_invalid(spelltng) || ((spelltng->field_1 & 0x01) != 0)
      || (get_2d_box_distance(&thing->mappos, &spelltng->mappos) >= 512))
    {
      set_start_state(thing);
      return 0;
    }
    enmroom = subtile_room_get(spelltng->mappos.x.stl.num,spelltng->mappos.y.stl.num);
    ownroom = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, 3, 0, 1);
    if ( room_is_invalid(ownroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(thing, ownroom, &pos) )
    {
        WARNLOG("Player %d can't pick spell - doesn't have proper library to store it",(int)thing->owner);
        set_start_state(thing);
        return 0;
    }
    // Check if we're stealing the spell from a library
    if (!room_is_invalid(enmroom))
    {
        remove_spell_from_library(enmroom, spelltng, thing->owner);
    }
    creature_drag_object(thing, spelltng);
    if (!setup_person_move_to_position(thing, pos.x.stl.num, pos.y.stl.num, 0))
    {
        SYNCDBG(8,"Cannot move to (%d,%d)",(int)pos.x.stl.num, (int)pos.y.stl.num);
    }
    thing->continue_state = CrSt_CreatureDropsSpellObjectInLibrary;
    return 1;
}

short creature_picks_up_trap_for_workshop(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *cratetng;
    struct Room *dstroom;
    struct Coord3d pos;
    //return _DK_creature_picks_up_trap_for_workshop(thing);
    // Get the crate thing
    cctrl = creature_control_get_from_thing(thing);
    cratetng = thing_get(cctrl->field_72);
    // Check if everything is right
    if ( thing_is_invalid(cratetng) || ((cratetng->field_1 & 0x01) != 0)
      || (get_2d_box_distance(&thing->mappos, &cratetng->mappos) >= 512) )
    {
        set_start_state(thing);
        return 0;
    }
    // Find room to drag the crate to
    dstroom = find_nearest_room_for_thing_with_spare_item_capacity(thing, thing->owner, 8, 0);
    if ( room_is_invalid(dstroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(thing, dstroom, &pos) )
    {
        set_start_state(thing);
        return 0;
    }
    // Initialize dragging
    if ( !setup_person_move_backwards_to_position(thing, pos.x.stl.num, pos.y.stl.num, 0) )
    {
        set_start_state(thing);
        return 0;
    }
    creature_drag_object(thing, cratetng);
    thing->continue_state = CrSt_CreatureDropsCrateInWorkshop;
    return 1;
}

short creature_picks_up_trap_object(struct Thing *thing)
{
  return _DK_creature_picks_up_trap_object(thing);
}

short creature_piss(struct Thing *thing)
{
  return _DK_creature_piss(thing);
}

short creature_present_to_dungeon_heart(struct Thing *thing)
{
  return _DK_creature_present_to_dungeon_heart(thing);
}

short creature_pretend_chicken_move(struct Thing *thing)
{
  return _DK_creature_pretend_chicken_move(thing);
}

short creature_pretend_chicken_setup_move(struct Thing *thing)
{
  return _DK_creature_pretend_chicken_setup_move(thing);
}

short creature_roar(struct Thing *thing)
{
  return _DK_creature_roar(thing);
}

short creature_sacrifice(struct Thing *thing)
{
  return _DK_creature_sacrifice(thing);
}

short creature_scavenged_disappear(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Room *room;
    struct Coord3d pos;
    long stl_x, stl_y;
    long i;
    //return _DK_creature_scavenged_disappear(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->byte_9A--;
    if (cctrl->byte_9A > 0)
    {
      if ((cctrl->byte_9A == 7) && (cctrl->byte_9B < PLAYERS_COUNT))
      {
        create_effect(&thing->mappos, get_scavenge_effect_element(cctrl->byte_9B), thing->owner);
      }
      return 0;
    }
    // We don't really have to convert coordinates into numbers and back to XY.
    i = get_subtile_number(cctrl->byte_9D, cctrl->byte_9E);
    stl_x = stl_num_decode_x(i);
    stl_y = stl_num_decode_y(i);
    room = subtile_room_get(stl_x, stl_y);
    if (room_is_invalid(room) || (room->kind != RoK_SCAVENGER))
    {
      ERRORLOG("Scavenger room disappeared.");
      kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
      return -1;
    }
    if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
      move_thing_in_map(thing, &pos);
      anger_set_creature_anger_all_types(thing, 0);
      dungeon = get_dungeon(cctrl->byte_9B);
      dungeon->field_98B++;
      if (is_my_player_number(thing->owner))
        output_message(62, 0, 1);
      cctrl->byte_9C = thing->owner;
      change_creature_owner(thing, cctrl->byte_9B);
      internal_set_thing_state(thing, 94);
      return 0;
    } else
    {
      ERRORLOG("No valid position inside scavenger room.");
      kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
      return -1;
    }
}

short creature_scavenged_reappear(struct Thing *thing)
{
  return _DK_creature_scavenged_reappear(thing);
}

struct Thing *find_gold_hoarde_in_room_for_creature(struct Thing *thing, struct Room *room)
{
    struct Thing *gldtng,*tmptng;
    long selected;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    gldtng = NULL;
    if (room->slabs_count <= 0)
    {
        WARNLOG("Room with no slabs detected!");
        return NULL;
    }
    selected = ACTION_RANDOM(room->slabs_count);
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        // Per room tile code
        tmptng = find_gold_hoarde_at(3*slb_x+1, 3*slb_y+1);
        if (!thing_is_invalid(tmptng))
        {
            if (creature_can_navigate_to_with_storage(thing, &tmptng->mappos, 0))
            {
                gldtng = tmptng;
                if (selected > 0)
                {
                    selected--;
                } else
                {
                    break;
                }
            }
        }
        // Per room tile code ends
        i = get_next_slab_number_in_room(i);
        k++;
        if (k > room->slabs_count)
        {
          ERRORLOG("Room slabs list length exceeded when sweeping");
          break;
        }
    }
    return gldtng;
}

/**
 * State handler function for stealing gold.
 * Should be invoked when a thief arrives at enemy treasure room.
 * Searches for specific gold hoard to steal from and enters next
 * phase of stealing (CrSt_CreatureStealGold).
 * @param thing The creature who is stealing gold.
 * @return True on success, false if finding gold to steal failed.
 */
short creature_search_for_gold_to_steal_in_room(struct Thing *thing)
{
    struct SlabMap *slb;
    struct Room *room;
    struct Thing *gldtng;
    //return _DK_creature_search_for_gold_to_steal_in_room(thing);
    slb = get_slabmap_for_subtile(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
    room = room_get(slb->room_index);
    if (room_is_invalid(room) || (room->kind != RoK_TREASURE))
    {
        WARNLOG("Cannot steal gold - not on treasure room at (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    gldtng = find_gold_hoarde_in_room_for_creature(thing, room);
    if (thing_is_invalid(gldtng))
    {
        WARNLOG("Cannot steal gold - no gold hoard found in treasure room");
        set_start_state(thing);
        return 0;
    }
    if (!setup_person_move_to_position(thing, gldtng->mappos.x.stl.num, gldtng->mappos.y.stl.num, 0))
    {
        SYNCDBG(8,"Cannot move to gold at (%d,%d)",(int)gldtng->mappos.x.stl.num, (int)gldtng->mappos.y.stl.num);
    }
    thing->continue_state = CrSt_CreatureStealGold;
    return 1;
}

short creature_search_for_spell_to_steal_in_room(struct Thing *thing)
{
    struct SlabMap *slb;
    struct Room *room;
    struct Thing *spltng;
    slb = get_slabmap_for_subtile(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
    room = room_get(slb->room_index);
    if (room_is_invalid(room) || (room->kind != RoK_LIBRARY))
    {
        WARNLOG("Cannot steal spell - not on library at (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    //TODO STEAL_SPELLS make correct spell finding function
    spltng = NULL;//find_spell_in_room_for_creature(thing, room);
    if (thing_is_invalid(spltng))
    {
        WARNLOG("Cannot steal spell - no spellbook found in library");
        set_start_state(thing);
        return 0;
    }
    if (!setup_person_move_to_position(thing, spltng->mappos.x.stl.num, spltng->mappos.y.stl.num, 0))
    {
        SYNCDBG(8,"Cannot move to spell at (%d,%d)",(int)spltng->mappos.x.stl.num, (int)spltng->mappos.y.stl.num);
    }
    thing->continue_state = CrSt_CreatureStealSpell;
    return 1;
}

short creature_set_work_room_based_on_position(struct Thing *thing)
{
    //return _DK_creature_set_work_room_based_on_position(thing);
    return 1;
}

short creature_slap_cowers(struct Thing *thing)
{
  return _DK_creature_slap_cowers(thing);
}

short creature_steal_gold(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct Room *room;
    struct Thing *hrdtng;
    long amount;
    //return _DK_creature_steal_gold(thing);
    crstat = creature_stats_get_from_thing(thing);
    room = subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (room_is_invalid(room) || (room->kind != RoK_TREASURE))
    {
        WARNLOG("Cannot steal gold - not on treasure room at (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    hrdtng = find_gold_hoarde_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (thing_is_invalid(hrdtng))
    {
        WARNLOG("Cannot steal gold - no gold hoard at (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    if (crstat->gold_hold-thing->long_13 > 0)
    {
        // Success! we are able to steal some gold!
        amount = remove_gold_from_hoarde(hrdtng, room, crstat->gold_hold-thing->long_13);
        thing->long_13 += amount;
        create_price_effect(&thing->mappos, thing->owner, amount);
        SYNCDBG(6,"Stolen %ld gold from hoarde at (%d,%d)",amount,(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
    }
    set_start_state(thing);
    return 0;
}

short creature_steal_spell(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct Room *room;
    //return _DK_creature_steal_gold(thing);
    crstat = creature_stats_get_from_thing(thing);
    room = subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (room_is_invalid(room) || (room->kind != RoK_LIBRARY))
    {
        WARNLOG("Cannot steal spell - not on library at (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    //TODO STEAL_SPELLS write the spell stealing code
    SYNCLOG("Stealing spells not implemented - reset");
    set_start_state(thing);
    return 0;
}

short creature_take_salary(struct Thing *thing)
{
  return _DK_creature_take_salary(thing);
}

short creature_to_garden(struct Thing *thing)
{
  return _DK_creature_to_garden(thing);
}

short creature_unconscious(struct Thing *thing)
{
  return _DK_creature_unconscious(thing);
}

short creature_vandalise_rooms(struct Thing *thing)
{
  return _DK_creature_vandalise_rooms(thing);
}

short creature_wait_at_treasure_room_door(struct Thing *thing)
{
  return _DK_creature_wait_at_treasure_room_door(thing);
}

short creature_wants_a_home(struct Thing *thing)
{
  return _DK_creature_wants_a_home(thing);
}

short creature_wants_salary(struct Thing *thing)
{
  return _DK_creature_wants_salary(thing);
}

long setup_head_for_empty_treasure_space(struct Thing *thing, struct Room *room)
{
    return _DK_setup_head_for_empty_treasure_space(thing, room);
}

long setup_random_head_for_room(struct Thing *thing, struct Room *room, unsigned char a3)
{
  return _DK_setup_random_head_for_room(thing, room, a3);
}

struct Room *find_nearest_room_for_thing_excluding_two_types(struct Thing *thing, char owner, char a3, char a4, unsigned char a5)
{
    return _DK_find_nearest_room_for_thing_excluding_two_types(thing, owner, a3, a4, a5);
}

void place_thing_in_creature_controlled_limbo(struct Thing *thing)
{
    remove_thing_from_mapwho(thing);
    thing->field_4F |= 0x01;
    thing->field_1 |= 0x02;
}

void remove_thing_from_creature_controlled_limbo(struct Thing *thing)
{
    thing->field_1 &= 0xFD;
    thing->field_4F &= 0xFE;
    place_thing_in_mapwho(thing);
}

short guarding(struct Thing *thing)
{
  return _DK_guarding(thing);
}

short mad_killing_psycho(struct Thing *thing)
{
  return _DK_mad_killing_psycho(thing);
}

short move_backwards_to_position(struct Thing *thing)
{
  return _DK_move_backwards_to_position(thing);
}

long move_check_attack_any_door(struct Thing *thing)
{
  return _DK_move_check_attack_any_door(thing);
}

long move_check_can_damage_wall(struct Thing *thing)
{
  return _DK_move_check_can_damage_wall(thing);
}

long move_check_kill_creatures(struct Thing *thing)
{
  return _DK_move_check_kill_creatures(thing);
}

long move_check_near_dungeon_heart(struct Thing *thing)
{
  return _DK_move_check_near_dungeon_heart(thing);
}

long move_check_on_head_for_room(struct Thing *thing)
{
  return _DK_move_check_on_head_for_room(thing);
}

long move_check_persuade(struct Thing *thing)
{
  return _DK_move_check_persuade(thing);
}

long move_check_wait_at_door_for_wage(struct Thing *thing)
{
  return _DK_move_check_wait_at_door_for_wage(thing);
}

char new_slab_tunneller_check_for_breaches(struct Thing *thing)
{
  return _DK_new_slab_tunneller_check_for_breaches(thing);
}

short patrol_here(struct Thing *thing)
{
  return _DK_patrol_here(thing);
}

short patrolling(struct Thing *thing)
{
  return _DK_patrolling(thing);
}

short person_sulk_at_lair(struct Thing *thing)
{
  return _DK_person_sulk_at_lair(thing);
}

short person_sulk_head_for_lair(struct Thing *thing)
{
  return _DK_person_sulk_head_for_lair(thing);
}

short person_sulking(struct Thing *thing)
{
  return _DK_person_sulking(thing);
}

short praying_in_temple(struct Thing *thing)
{
  return _DK_praying_in_temple(thing);
}

long room_still_valid_as_type_for_thing(struct Room *room, long rkind, struct Thing *thing)
{
  return ((room->field_0 & 0x01) != 0) && (room->kind == rkind);
}

void create_effect_around_thing(struct Thing *thing, long eff_kind)
{
  _DK_create_effect_around_thing(thing, eff_kind);
}

void remove_health_from_thing_and_display_health(struct Thing *thing, long delta)
{
  _DK_remove_health_from_thing_and_display_health(thing, delta);
}

long slab_by_players_land(long plyr_idx, long slb_x, long slb_y)
{
  return _DK_slab_by_players_land(plyr_idx, slb_x, slb_y);
}

TbBool process_creature_hunger(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  if ( (crstat->hunger_rate == 0) || ((cctrl->field_AB & 0x02) != 0) )
    return false;
  cctrl->field_39++;
  if (cctrl->field_39 <= crstat->hunger_rate)
    return false;
  if ((game.play_gameturn % game.turns_per_hunger_health_loss) == 0)
    remove_health_from_thing_and_display_health(thing, game.hunger_health_loss);
  return true;
}

long process_research_function(struct Thing *thing)
{
  return _DK_process_research_function(thing);
}

long process_scavenge_function(struct Thing *thing)
{
  return _DK_process_scavenge_function(thing);
}

long process_temple_function(struct Thing *thing)
{
  return _DK_process_temple_function(thing);
}

short researching(struct Thing *thing)
{
  return _DK_researching(thing);
}

short scavengering(struct Thing *thing)
{
  return _DK_scavengering(thing);
}

TbBool creature_will_attack_creature(const struct Thing *tng1, const struct Thing *tng2)
{
    struct CreatureControl *cctrl1;
    struct CreatureControl *cctrl2;
    struct PlayerInfo *player1;
    struct PlayerInfo *player2;
    struct CreatureStats *crstat1;
    struct Thing *tmptng;

    cctrl1 = creature_control_get_from_thing(tng1);
    cctrl2 = creature_control_get_from_thing(tng2);
    player1 = get_player(tng1->owner);
    player2 = get_player(tng2->owner);

    if ((tng2->owner != tng1->owner) && (tng2->owner != game.neutral_player_num))
    {
       if ((tng1->owner == game.neutral_player_num) || (tng2->owner == game.neutral_player_num)
        || (!player_allied_with(player1, tng2->owner)))
          return true;
       if ((tng2->owner == game.neutral_player_num) || (tng1->owner == game.neutral_player_num)
        || (!player_allied_with(player2, tng1->owner)))
          return true;
    }

    tmptng = thing_get(cctrl1->word_A2);
    if  ( (cctrl1->field_AD & 0x10) || (cctrl2->field_AD & 0x10)
        || ((cctrl1->field_3) && (tmptng == tng2)) )
    {
        if (tng2 != tng1)
        {
            if ((creature_control_exists(cctrl2)) && ((cctrl2->flgfield_1 & CCFlg_NoCompControl) == 0)
            && ((tng2->field_0 & 0x10) == 0) && ((tng2->field_1 & 0x02) == 0))
            {
                crstat1 = creature_stats_get_from_thing(tng1);
                if ((cctrl2->spell_flags & 0x20) == 0)
                    return true;
                if (cctrl2->field_AF > 0)
                    return true;
                if (crstat1->can_see_invisible)
                    return true;
                if ((cctrl1->spell_flags & 0x40) != 0)
                    return true;
            }
        }
    }
    return false;
}

struct Thing *thing_update_enemy_to_fight_with(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *enemytng;
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->word_9A != 0)
    {
        enemytng = thing_get(cctrl->word_9A);
        if (((enemytng->field_0 & 0x01) == 0) || (cctrl->long_9C != enemytng->field_9))
        {
          enemytng = NULL;
          cctrl->long_9C = 0;
          cctrl->word_9A = 0;
        }
    } else
    {
        enemytng = NULL;
    }
    if (game.play_gameturn - cctrl->long_A0 > 64)
    {
        cctrl->long_A0 = game.play_gameturn;
        enemytng = find_nearest_enemy_creature(thing);
    }
    if (thing_is_invalid(enemytng))
    {
        cctrl->word_9A = 0;
        return NULL;
    }
    cctrl->word_9A = enemytng->index;
    cctrl->long_9C = enemytng->field_9;
    return enemytng;
}

TbBool wander_point_get_random_pos(struct Wander *wandr, struct Coord3d *pos)
{
  long irnd;
  if ( wandr->field_0 )
  {
    irnd = ACTION_RANDOM(wandr->field_0);
    pos->x.val = wandr->field_18[2*irnd] << 8;
    pos->y.val = wandr->field_18[2*irnd + 1] << 8;
    return true;
  }
  return false;
}
TbBool get_random_position_in_dungeon_for_creature(long plyr_idx, unsigned char a2, struct Thing *thing, struct Coord3d *pos)
{
  struct PlayerInfo *player;
  if (plyr_idx == game.neutral_player_num)
  {
    ERRORLOG("Attempt to get random position in neutral dungeon");
    return false;
  }
  player = get_player(plyr_idx);
  if (player_invalid(player))
  {
    ERRORLOG("Attempt to get random position in invalid dungeon");
    return false;
  }
  if ( a2 )
  {
    if (!wander_point_get_random_pos(&player->wandr1, pos))
      return false;
  } else
  {
    if (!wander_point_get_random_pos(&player->wandr2, pos))
      return false;
  }
  return true;
}

TbBool creature_can_hear_within_distance(struct Thing *thing, long dist)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return (crstat->hearing) >= (dist/256);
}

/**
 * Enemy seeking function for creatures and heroes.
 * Used for performing SEEK_THE_ENEMY job.
 * @param thing The creature to seek the enemy for.
 * @return
 */
short seek_the_enemy(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *enemytng;
    struct Coord3d pos;
    long dist;
    //return _DK_seek_the_enemy(thing);
    cctrl = creature_control_get_from_thing(thing);
    enemytng = thing_update_enemy_to_fight_with(thing);
    if (!thing_is_invalid(enemytng))
    {
        dist = get_2d_box_distance(&enemytng->mappos, &thing->mappos);
        if (creature_can_hear_within_distance(thing, dist))
        {
            if (cctrl->field_D2 == 0)
            {
              if ((dist < 2304) && (game.play_gameturn-cctrl->field_282 < 20))
              {
                set_creature_instance(thing, CrSt_GoodDoingNothing, 1, 0, 0);
                thing_play_sample(thing, 168+UNSYNC_RANDOM(3), 100, 0, 3, 0, 2, 256);
                return 1;
              }
              if (ACTION_RANDOM(4) != 0)
              {
                  if (setup_person_move_close_to_position(thing, enemytng->mappos.x.stl.num, enemytng->mappos.y.stl.num, 0) )
                  {
                    thing->continue_state = CrSt_SeekTheEnemy;
                    cctrl->field_282 = game.play_gameturn;
                    return 1;
                  }
              }
              if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
              {
                  thing->continue_state = CrSt_SeekTheEnemy;
                  cctrl->field_282 = game.play_gameturn;
              }
            }
            return 1;
        }
        if (ACTION_RANDOM(64) == 0)
        {
            if (setup_person_move_close_to_position(thing, enemytng->mappos.x.stl.num, enemytng->mappos.y.stl.num, 0))
            {
              thing->continue_state = CrSt_SeekTheEnemy;
            }
        }
    }
    // No enemy found - do some random movement
    if (ACTION_RANDOM(12) != 0)
    {
        if ( creature_choose_random_destination_on_valid_adjacent_slab(thing) )
        {
            thing->continue_state = CrSt_SeekTheEnemy;
            return 1;
        }
    } else
    if (get_random_position_in_dungeon_for_creature(thing->owner, 1, thing, &pos))
    {
        if ( setup_person_move_to_position(thing, pos.x.val >> 8, pos.y.val >> 8, 0) )
        {
            thing->continue_state = CrSt_SeekTheEnemy;
        }
        return 1;
    }
    set_start_state(thing);
    return 1;
}

short state_cleanup_dragging_body(struct Thing *thing)
{
  return _DK_state_cleanup_dragging_body(thing);
}

short state_cleanup_dragging_object(struct Thing *thing)
{
  return _DK_state_cleanup_dragging_object(thing);
}

short state_cleanup_in_room(struct Thing *thing)
{
  return _DK_state_cleanup_in_room(thing);
}

short state_cleanup_in_temple(struct Thing *thing)
{
  return _DK_state_cleanup_in_temple(thing);
}

short state_cleanup_unable_to_fight(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_state_cleanup_unable_to_fight(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
    return 1;
}

short state_cleanup_unconscious(struct Thing *thing)
{
  return _DK_state_cleanup_unconscious(thing);
}

long process_work_speed_on_work_value(struct Thing *thing, long base_val)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    long val;
    cctrl = creature_control_get_from_thing(thing);
    val = base_val;
    if ((cctrl->spell_flags & CSF_Speed) != 0)
        val = 2 * val;
    if (cctrl->field_21)
        val = 4 * val / 3;
    if (thing->owner != game.neutral_player_num)
    {
        dungeon = get_dungeon(thing->owner);
        if (dungeon->tortured_creatures[thing->model] > 0)
            val = 4 * val / 3;
        if (dungeon->field_888)
            val = 6 * val / 5;
    }
    return val;
}

TbBool check_experience_upgrade(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    long i;
    dungeon = get_dungeon(thing->owner);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    i = crstat->to_level[cctrl->explevel] << 8;
    if (cctrl->field_24 < i)
        return false;
    cctrl->field_24 -= i;
    if (cctrl->explevel < dungeon->creature_max_level[thing->model])
    {
      if ((cctrl->explevel < 9) || (crstat->grow_up != 0))
        cctrl->field_AD |= 0x40;
    }
    return true;
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

short tunneller_doing_nothing(struct Thing *thing)
{
  return _DK_tunneller_doing_nothing(thing);
}

short tunnelling(struct Thing *thing)
{
  return _DK_tunnelling(thing);
}
/******************************************************************************/
TbBool internal_set_thing_state(struct Thing *thing, long nState)
{
  struct CreatureControl *cctrl;
  thing->active_state = nState;
  set_flag_byte(&thing->field_1, 0x10, false);
  thing->continue_state = CrSt_Unused;
  cctrl = creature_control_get_from_thing(thing);
  cctrl->field_302 = 0;
  clear_creature_instance(thing);
  return true;
}

TbBool initialise_thing_state(struct Thing *thing, long nState)
{
    struct CreatureControl *cctrl;
    //return _DK_initialise_thing_state(thing, nState);
    cleanup_current_thing_state(thing);
    thing->continue_state = CrSt_Unused;
    thing->active_state = nState;
    set_flag_byte(&thing->field_1, 0x10, false);
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Creature no %d has invalid control",(int)thing->index);
        return false;
    }
    cctrl->field_80 = 0;
    cctrl->field_302 = 0;
    if ((cctrl->flgfield_1 & CCFlg_IsInRoomList) != 0)
    {
        WARNLOG("The %s stays in room list even after cleanup",thing_model_name(thing));
        remove_creature_from_work_room(thing);
    }
    return true;
}

TbBool cleanup_current_thing_state(struct Thing *thing)
{
    struct StateInfo *stati;
    stati = get_creature_state_with_task_completion(thing);
    if (stati->ofsfield_4 != NULL)
    {
        stati->ofsfield_4(thing);
        set_flag_byte(&thing->field_1, 0x10, true);
    } else
    {
        clear_creature_instance(thing);
    }
    return true;
}

TbBool can_change_from_state_to(struct Thing *thing, long curr_state, long next_state)
{
    struct StateInfo *next_stati;
    struct StateInfo *curr_stati;
    curr_stati = get_thing_state_info_num(curr_state);
    if (curr_stati->state_type == 6)
      curr_stati = get_thing_state_info_num(thing->continue_state);
    next_stati = get_thing_state_info_num(next_state);
    if ((curr_stati->field_20) && (!next_stati->field_16))
        return false;
    if ((curr_stati->field_1F) && (!next_stati->field_15))
        return false;
    switch (curr_stati->state_type)
    {
    case 2:
        if ( next_stati->field_11 )
            return true;
        break;
    case 3:
        if ( next_stati->field_12 )
            return true;
        break;
    case 4:
        if ( next_stati->field_10 )
            return true;
        break;
    case 5:
        if ( next_stati->field_13 )
            return true;
        break;
    case 7:
        if ( next_stati->field_14 )
            return true;
        break;
    case 8:
        if ( next_stati->field_17 )
            return true;
        break;
    case 9:
        if ( next_stati->field_18 )
            return true;
        break;
    case 10:
        if ( next_stati->field_19 )
            return true;
        break;
    case 11:
        if ( next_stati->field_1B )
            return true;
        break;
    case 12:
        if ( next_stati->field_1A )
            return true;
        break;
    case 13:
        if ( next_stati->field_1C )
            return true;
        break;
    case 14:
        if ( next_stati->field_1D )
            return true;
        break;
    default:
        return true;
    }
    return false;
}

short set_start_state(struct Thing *thing)
{
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    long i;
    SYNCDBG(8,"Starting for %s, owner %d",thing_model_name(thing),(int)thing->owner);
//    return _DK_set_start_state(thing);
    if ((thing->field_0 & 0x20) != 0)
    {
      cleanup_current_thing_state(thing);
      initialise_thing_state(thing, CrSt_ManualControl);
      return thing->active_state;
    }
    if (thing->owner == game.neutral_player_num)
    {
      cleanup_current_thing_state(thing);
      initialise_thing_state(thing, CrSt_CreatureDormant);
      return thing->active_state;
    }
    if (thing->owner == game.hero_player_num)
    {
        i = creatures[thing->model%CREATURE_TYPES_COUNT].numfield_2;
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, i);
        return thing->active_state;
    }
    player = get_player(thing->owner);
    if (player->victory_state == 2)
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_CreatureLeavesOrDies);
        return thing->active_state;
    }
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->field_AD & 0x02) != 0)
    {
      cleanup_current_thing_state(thing);
      initialise_thing_state(thing, CrSt_CreaturePretendChickenSetupMove);
      return thing->active_state;
    }
    initialise_thing_state(thing, creatures[thing->model%CREATURE_TYPES_COUNT].start_state);
    return thing->active_state;
}
/******************************************************************************/
