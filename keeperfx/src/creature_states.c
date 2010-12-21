/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_control.c
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
#include "tasks_list.h"
#include "spworker_stack.h"
#include "map_events.h"
#include "power_hand.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "sounds.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CREATURE_GUI_STATES_COUNT 3
/******************************************************************************/
DLLIMPORT short _DK_already_at_call_to_arms(struct Thing *thing);
DLLIMPORT short _DK_arrive_at_alarm(struct Thing *thing);
DLLIMPORT short _DK_arrive_at_call_to_arms(struct Thing *thing);
DLLIMPORT short _DK_at_barrack_room(struct Thing *thing);
DLLIMPORT short _DK_at_guard_post_room(struct Thing *thing);
DLLIMPORT short _DK_at_kinky_torture_room(struct Thing *thing);
DLLIMPORT short _DK_at_lair_to_sleep(struct Thing *thing);
DLLIMPORT short _DK_at_research_room(struct Thing *thing);
DLLIMPORT short _DK_at_scavenger_room(struct Thing *thing);
DLLIMPORT short _DK_at_temple(struct Thing *thing);
DLLIMPORT short _DK_at_torture_room(struct Thing *thing);
DLLIMPORT short _DK_at_training_room(struct Thing *thing);
DLLIMPORT short _DK_at_workshop_room(struct Thing *thing);
DLLIMPORT short _DK_barracking(struct Thing *thing);
DLLIMPORT short _DK_cleanup_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_door_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_hold_audience(struct Thing *thing);
DLLIMPORT short _DK_cleanup_object_combat(struct Thing *thing);
DLLIMPORT short _DK_cleanup_prison(struct Thing *thing);
DLLIMPORT short _DK_cleanup_sacrifice(struct Thing *thing);
DLLIMPORT short _DK_cleanup_seek_the_enemy(struct Thing *thing);
DLLIMPORT short _DK_cleanup_sleep(struct Thing *thing);
DLLIMPORT short _DK_cleanup_torturing(struct Thing *thing);
DLLIMPORT short _DK_creature_arms_trap(struct Thing *thing);
DLLIMPORT short _DK_creature_arrived_at_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_arrived_at_prison(struct Thing *thing);
DLLIMPORT short _DK_creature_at_changed_lair(struct Thing *thing);
DLLIMPORT short _DK_creature_at_new_lair(struct Thing *thing);
DLLIMPORT short _DK_creature_attack_rooms(struct Thing *thing);
DLLIMPORT short _DK_creature_attempt_to_damage_walls(struct Thing *thing);
DLLIMPORT short _DK_creature_be_happy(struct Thing *thing);
DLLIMPORT short _DK_creature_being_dropped(struct Thing *thing);
DLLIMPORT short _DK_creature_being_sacrificed(struct Thing *thing);
DLLIMPORT short _DK_creature_being_scavenged(struct Thing *thing);
DLLIMPORT short _DK_creature_being_summoned(struct Thing *thing);
DLLIMPORT short _DK_creature_cannot_find_anything_to_do(struct Thing *thing);
DLLIMPORT short _DK_creature_change_from_chicken(struct Thing *thing);
DLLIMPORT short _DK_creature_change_lair(struct Thing *thing);
DLLIMPORT short _DK_creature_change_to_chicken(struct Thing *thing);
DLLIMPORT short _DK_creature_choose_room_for_lair_site(struct Thing *thing);
DLLIMPORT short _DK_creature_combat_flee(struct Thing *thing);
DLLIMPORT short _DK_creature_damage_walls(struct Thing *thing);
DLLIMPORT short _DK_creature_doing_nothing(struct Thing *thing);
DLLIMPORT short _DK_creature_door_combat(struct Thing *thing);
DLLIMPORT short _DK_creature_dormant(struct Thing *thing);
DLLIMPORT short _DK_creature_drop_body_in_prison(struct Thing *thing);
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
DLLIMPORT short _DK_creature_freeze_prisonors(struct Thing *thing);
DLLIMPORT short _DK_creature_going_home_to_sleep(struct Thing *thing);
DLLIMPORT short _DK_creature_hero_entering(struct Thing *thing);
DLLIMPORT short _DK_creature_in_combat(struct Thing *thing);
DLLIMPORT short _DK_creature_in_hold_audience(struct Thing *thing);
DLLIMPORT short _DK_creature_in_prison(struct Thing *thing);
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
DLLIMPORT short _DK_creature_sleep(struct Thing *thing);
DLLIMPORT short _DK_creature_steal_gold(struct Thing *thing);
DLLIMPORT short _DK_creature_take_salary(struct Thing *thing);
DLLIMPORT short _DK_creature_to_garden(struct Thing *thing);
DLLIMPORT short _DK_creature_unconscious(struct Thing *thing);
DLLIMPORT short _DK_creature_vandalise_rooms(struct Thing *thing);
DLLIMPORT short _DK_creature_wait_at_treasure_room_door(struct Thing *thing);
DLLIMPORT short _DK_creature_wants_a_home(struct Thing *thing);
DLLIMPORT short _DK_creature_wants_salary(struct Thing *thing);
DLLIMPORT short _DK_good_attack_room(struct Thing *thing);
DLLIMPORT short _DK_good_back_at_start(struct Thing *thing);
DLLIMPORT short _DK_good_doing_nothing(struct Thing *thing);
DLLIMPORT short _DK_good_drops_gold(struct Thing *thing);
DLLIMPORT short _DK_good_leave_through_exit_door(struct Thing *thing);
DLLIMPORT short _DK_good_returns_to_start(struct Thing *thing);
DLLIMPORT short _DK_good_wait_in_exit_door(struct Thing *thing);
DLLIMPORT short _DK_guarding(struct Thing *thing);
DLLIMPORT short _DK_imp_arrives_at_convert_dungeon(struct Thing *thing);
DLLIMPORT short _DK_imp_arrives_at_dig_or_mine(struct Thing *thing);
DLLIMPORT short _DK_imp_arrives_at_improve_dungeon(struct Thing *thing);
DLLIMPORT short _DK_imp_arrives_at_reinforce(struct Thing *thing);
DLLIMPORT short _DK_imp_birth(struct Thing *thing);
DLLIMPORT short _DK_imp_converts_dungeon(struct Thing *thing);
DLLIMPORT short _DK_imp_digs_mines(struct Thing *thing);
DLLIMPORT short _DK_imp_doing_nothing(struct Thing *thing);
DLLIMPORT short _DK_imp_drops_gold(struct Thing *thing);
DLLIMPORT short _DK_imp_improves_dungeon(struct Thing *thing);
DLLIMPORT short _DK_imp_last_did_job(struct Thing *thing);
DLLIMPORT short _DK_imp_picks_up_gold_pile(struct Thing *thing);
DLLIMPORT short _DK_imp_reinforces(struct Thing *thing);
DLLIMPORT short _DK_imp_toking(struct Thing *thing);
DLLIMPORT short _DK_kinky_torturing(struct Thing *thing);
DLLIMPORT short _DK_mad_killing_psycho(struct Thing *thing);
DLLIMPORT short _DK_manufacturing(struct Thing *thing);
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
DLLIMPORT long _DK_process_kinky_function(struct Thing *thing);
DLLIMPORT long _DK_process_prison_function(struct Thing *thing);
DLLIMPORT long _DK_process_research_function(struct Thing *thing);
DLLIMPORT long _DK_process_scavenge_function(struct Thing *thing);
DLLIMPORT long _DK_process_temple_function(struct Thing *thing);
DLLIMPORT long _DK_process_torture_function(struct Thing *thing);
DLLIMPORT short _DK_researching(struct Thing *thing);
DLLIMPORT short _DK_scavengering(struct Thing *thing);
DLLIMPORT short _DK_seek_the_enemy(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_dragging_body(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_dragging_object(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_in_room(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_in_temple(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_unable_to_fight(struct Thing *thing);
DLLIMPORT short _DK_state_cleanup_unconscious(struct Thing *thing);
DLLIMPORT short _DK_torturing(struct Thing *thing);
DLLIMPORT short _DK_training(struct Thing *thing);
DLLIMPORT short _DK_tunneller_doing_nothing(struct Thing *thing);
DLLIMPORT short _DK_tunnelling(struct Thing *thing);
DLLIMPORT long _DK_process_torture_visuals(struct Thing *thing, struct Room *room, long a3);
DLLIMPORT long _DK_creature_can_be_trained(struct Thing *thing);
DLLIMPORT long _DK_player_can_afford_to_train_creature(struct Thing *thing);
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
DLLIMPORT long _DK_process_prison_food(struct Thing *thing, struct Room *room);
DLLIMPORT long _DK_slab_by_players_land(unsigned char plyr_idx, unsigned char slb_x, unsigned char slb_y);
DLLIMPORT long _DK_setup_prison_move(struct Thing *thing, struct Room *room);
DLLIMPORT struct Room *_DK_find_nearest_room_for_thing_excluding_two_types(struct Thing *thing, char owner, char a3, char a4, unsigned char a5);
DLLIMPORT long _DK_good_setup_loot_treasure_room(struct Thing *thing, long dngn_id);
DLLIMPORT unsigned char _DK_initialise_thing_state(struct Thing *thing, long a2);
DLLIMPORT unsigned char _DK_remove_creature_from_work_room(struct Thing *thing);
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
DLLIMPORT void _DK_process_creature_in_training_room(struct Thing *thing, struct Room *room);
DLLIMPORT void _DK_setup_move_to_new_training_position(struct Thing *thing, struct Room *room, unsigned long a3);
DLLIMPORT long _DK_reveal_players_map_to_player(struct Thing *thing, long a2);
/******************************************************************************/
short already_at_call_to_arms(struct Thing *thing);
short arrive_at_alarm(struct Thing *thing);
short arrive_at_call_to_arms(struct Thing *thing);
short at_barrack_room(struct Thing *thing);
short at_guard_post_room(struct Thing *thing);
short at_kinky_torture_room(struct Thing *thing);
short at_lair_to_sleep(struct Thing *thing);
short at_research_room(struct Thing *thing);
short at_scavenger_room(struct Thing *thing);
short at_temple(struct Thing *thing);
short at_torture_room(struct Thing *thing);
short at_training_room(struct Thing *thing);
short at_workshop_room(struct Thing *thing);
short barracking(struct Thing *thing);
short cleanup_combat(struct Thing *thing);
short cleanup_door_combat(struct Thing *thing);
short cleanup_hold_audience(struct Thing *thing);
short cleanup_object_combat(struct Thing *thing);
short cleanup_prison(struct Thing *thing);
short cleanup_sacrifice(struct Thing *thing);
short cleanup_seek_the_enemy(struct Thing *thing);
short cleanup_sleep(struct Thing *thing);
short cleanup_torturing(struct Thing *thing);
short creature_arms_trap(struct Thing *thing);
short creature_arrived_at_garden(struct Thing *thing);
short creature_arrived_at_prison(struct Thing *thing);
short creature_at_changed_lair(struct Thing *thing);
short creature_at_new_lair(struct Thing *thing);
short creature_attack_rooms(struct Thing *thing);
short creature_attempt_to_damage_walls(struct Thing *thing);
short creature_be_happy(struct Thing *thing);
short creature_being_dropped(struct Thing *thing);
short creature_being_sacrificed(struct Thing *thing);
short creature_being_scavenged(struct Thing *thing);
short creature_being_summoned(struct Thing *thing);
short creature_cannot_find_anything_to_do(struct Thing *thing);
short creature_change_from_chicken(struct Thing *thing);
short creature_change_lair(struct Thing *thing);
short creature_change_to_chicken(struct Thing *thing);
short creature_choose_room_for_lair_site(struct Thing *thing);
short creature_combat_flee(struct Thing *thing);
short creature_damage_walls(struct Thing *thing);
short creature_doing_nothing(struct Thing *thing);
short creature_door_combat(struct Thing *thing);
short creature_dormant(struct Thing *thing);
short creature_drop_body_in_prison(struct Thing *thing);
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
short creature_freeze_prisonors(struct Thing *thing);
short creature_going_home_to_sleep(struct Thing *thing);
short creature_hero_entering(struct Thing *thing);
short creature_in_combat(struct Thing *thing);
short creature_in_hold_audience(struct Thing *thing);
short creature_in_prison(struct Thing *thing);
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
short creature_sleep(struct Thing *thing);
short creature_steal_gold(struct Thing *thing);
short creature_take_salary(struct Thing *thing);
short creature_to_garden(struct Thing *thing);
short creature_unconscious(struct Thing *thing);
short creature_vandalise_rooms(struct Thing *thing);
short creature_wait_at_treasure_room_door(struct Thing *thing);
short creature_wants_a_home(struct Thing *thing);
short creature_wants_salary(struct Thing *thing);
short good_attack_room(struct Thing *thing);
short good_back_at_start(struct Thing *thing);
short good_doing_nothing(struct Thing *thing);
short good_drops_gold(struct Thing *thing);
short good_leave_through_exit_door(struct Thing *thing);
short good_returns_to_start(struct Thing *thing);
short good_wait_in_exit_door(struct Thing *thing);
short guarding(struct Thing *thing);
short imp_arrives_at_convert_dungeon(struct Thing *thing);
short imp_arrives_at_dig_or_mine(struct Thing *thing);
short imp_arrives_at_improve_dungeon(struct Thing *thing);
short imp_arrives_at_reinforce(struct Thing *thing);
short imp_birth(struct Thing *thing);
short imp_converts_dungeon(struct Thing *thing);
short imp_digs_mines(struct Thing *thing);
short imp_doing_nothing(struct Thing *thing);
short imp_drops_gold(struct Thing *thing);
short imp_improves_dungeon(struct Thing *thing);
short imp_last_did_job(struct Thing *thing);
short imp_picks_up_gold_pile(struct Thing *thing);
short imp_reinforces(struct Thing *thing);
short imp_toking(struct Thing *thing);
short kinky_torturing(struct Thing *thing);
short mad_killing_psycho(struct Thing *thing);
short manufacturing(struct Thing *thing);
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
long process_kinky_function(struct Thing *thing);
long process_prison_function(struct Thing *thing);
long process_research_function(struct Thing *thing);
long process_scavenge_function(struct Thing *thing);
long process_temple_function(struct Thing *thing);
long process_torture_function(struct Thing *thing);
short researching(struct Thing *thing);
short scavengering(struct Thing *thing);
short seek_the_enemy(struct Thing *thing);
short state_cleanup_dragging_body(struct Thing *thing);
short state_cleanup_dragging_object(struct Thing *thing);
short state_cleanup_in_room(struct Thing *thing);
short state_cleanup_in_temple(struct Thing *thing);
short state_cleanup_unable_to_fight(struct Thing *thing);
short state_cleanup_unconscious(struct Thing *thing);
short torturing(struct Thing *thing);
short training(struct Thing *thing);
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
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_attack_rooms, NULL, NULL, move_check_attack_any_door,
    0, 1,   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 51, 1, 0,  0},
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
struct StateInfo *get_thing_state7_info(struct Thing *thing)
{
  if (thing->field_7 >= CREATURE_STATES_COUNT)
    return &states[0];
  return &states[thing->field_7];
}

struct StateInfo *get_thing_state8_info(struct Thing *thing)
{
  if (thing->field_8 >= CREATURE_STATES_COUNT)
    return &states[0];
  return &states[thing->field_8];
}

struct StateInfo *get_thing_state_info_num(long state_id)
{
  if ((state_id < 0) || (state_id >= CREATURE_STATES_COUNT))
    return &states[0];
  return &states[state_id];
}

struct StateInfo *get_creature_state_with_task_completion(struct Thing *thing)
{
  struct StateInfo *stati;
  stati = get_thing_state7_info(thing);
  if (stati->state_type == 6)
      stati = get_thing_state8_info(thing);
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
  state = thing->field_7;
  if ( (state > 0) && (state < sizeof(states)/sizeof(states[0])) )
  {
      state_type = states[state].state_type;
  } else
  {
      state_type = states[0].state_type;
      WARNLOG("Creature state[0]=%ld is out of range.",state);
  }
  if (state_type == 6)
  {
    state = thing->field_8;
    if ( (state > 0) && (state < sizeof(states)/sizeof(states[0])) )
    {
        state_type = states[state].state_type;
    } else
    {
        state_type = states[0].state_type;
        WARNLOG("Creature state[1]=%ld is out of range.",state);
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
      WARNLOG("Creature of breed %d has invalid state type(%ld)!",(int)thing->model,state_type);
      erstat_inc(ESE_BadCreatrState);
      return state_type_to_gui_state[0];
    }
}

TbBool creature_is_doing_lair_activity(const struct Thing *thing)
{
    long i;
    i = thing->field_7;
    if (i == CrSt_CreatureSleep)
        return true;
    if (i == CrSt_MoveToPosition)
        i = thing->field_8;
    if ((i == CrSt_CreatureGoingHomeToSleep) || (i == CrSt_AtLairToSleep)
      || (i == CrSt_CreatureChooseRoomForLairSite) || (i == CrSt_CreatureAtNewLair) || (i == CrSt_CreatureWantsAHome)
      || (i == CrSt_CreatureChangeLair) || (i == CrSt_CreatureAtChangedLair))
        return true;
    return false;
}

TbBool creature_is_being_dropped(const struct Thing *thing)
{
    long i;
    i = thing->field_7;
    if (i == CrSt_MoveToPosition)
        i = thing->field_8;
    if (i == CrSt_CreatureBeingDropped)
        return true;
    return false;
}

TbBool creature_is_doing_dungeon_improvements(const struct Thing *thing)
{
    long i;
    i = thing->field_7;
    if (i == CrSt_MoveToPosition)
        i = thing->field_8;
    if (states[i].state_type == CrSt_ImpImprovesDungeon)
        return true;
    return false;
}

TbBool creature_is_doing_garden_activity(const struct Thing *thing)
{
    long i;
    i = thing->field_7;
    if (i == CrSt_CreatureEat)
        return true;
    if (i == CrSt_MoveToPosition)
        i = thing->field_8;
    if ((i == CrSt_CreatureToGarden) || (i == CrSt_CreatureArrivedAtGarden))
        return true;
    return false;
}

TbBool creature_is_taking_salary_activity(const struct Thing *thing)
{
    long i;
    i = thing->field_7;
    if (i == CrSt_CreatureWantsSalary)
        return true;
    if (i == CrSt_MoveToPosition)
        i = thing->field_8;
    if (i == CrSt_CreatureTakeSalary)
        return true;
    return false;
}

TbBool creature_is_doing_temple_activity(const struct Thing *thing)
{
    long i;
    i = thing->field_7;
    if (i == CrSt_MoveToPosition)
        i = thing->field_8;
    if ((i == CrSt_AtTemple) || (i == CrSt_PrayingInTemple))
        return true;
    return false;
}

TbBool creature_state_is_unset(const struct Thing *thing)
{
    long i;
    i = thing->field_7;
    if (i == CrSt_MoveToPosition)
        i = thing->field_8;
    if (states[i].state_type == 0)
        return true;
    return false;
}

short already_at_call_to_arms(struct Thing *thing)
{
  return _DK_already_at_call_to_arms(thing);
}

short arrive_at_alarm(struct Thing *thing)
{
  return _DK_arrive_at_alarm(thing);
}

short arrive_at_call_to_arms(struct Thing *thing)
{
  return _DK_arrive_at_call_to_arms(thing);
}

short at_barrack_room(struct Thing *thing)
{
  return _DK_at_barrack_room(thing);
}

short at_guard_post_room(struct Thing *thing)
{
  return _DK_at_guard_post_room(thing);
}

short at_kinky_torture_room(struct Thing *thing)
{
  return _DK_at_kinky_torture_room(thing);
}

short at_lair_to_sleep(struct Thing *thing)
{
  return _DK_at_lair_to_sleep(thing);
}

short at_research_room(struct Thing *thing)
{
  return _DK_at_research_room(thing);
}

short at_scavenger_room(struct Thing *thing)
{
  return _DK_at_scavenger_room(thing);
}

short at_temple(struct Thing *thing)
{
  return _DK_at_temple(thing);
}

short at_torture_room(struct Thing *thing)
{
  return _DK_at_torture_room(thing);
}

short at_training_room(struct Thing *thing)
{
  return _DK_at_training_room(thing);
}

short at_workshop_room(struct Thing *thing)
{
  return _DK_at_workshop_room(thing);
}

short barracking(struct Thing *thing)
{
  return _DK_barracking(thing);
}

short cleanup_combat(struct Thing *thing)
{
  return _DK_cleanup_combat(thing);
}

short cleanup_door_combat(struct Thing *thing)
{
  return _DK_cleanup_door_combat(thing);
}

short cleanup_hold_audience(struct Thing *thing)
{
  return _DK_cleanup_hold_audience(thing);
}

short cleanup_object_combat(struct Thing *thing)
{
  return _DK_cleanup_object_combat(thing);
}

short cleanup_prison(struct Thing *thing)
{
  return _DK_cleanup_prison(thing);
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

short cleanup_sleep(struct Thing *thing)
{
  return _DK_cleanup_sleep(thing);
}

short cleanup_torturing(struct Thing *thing)
{
  return _DK_cleanup_torturing(thing);
}

short creature_arms_trap(struct Thing *thing)
{
  return _DK_creature_arms_trap(thing);
}

short creature_arrived_at_garden(struct Thing *thing)
{
  return _DK_creature_arrived_at_garden(thing);
}

short creature_arrived_at_prison(struct Thing *thing)
{
  return _DK_creature_arrived_at_prison(thing);
}

short creature_at_changed_lair(struct Thing *thing)
{
  return _DK_creature_at_changed_lair(thing);
}

short creature_at_new_lair(struct Thing *thing)
{
  return _DK_creature_at_new_lair(thing);
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

short creature_change_lair(struct Thing *thing)
{
  return _DK_creature_change_lair(thing);
}

short creature_change_to_chicken(struct Thing *thing)
{
  return _DK_creature_change_to_chicken(thing);
}

short creature_choose_room_for_lair_site(struct Thing *thing)
{
  return _DK_creature_choose_room_for_lair_site(thing);
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
      thing->field_8 = CrSt_CreatureDormant;
      return 1;
    }
    return 0;
}

short creature_drop_body_in_prison(struct Thing *thing)
{
  return _DK_creature_drop_body_in_prison(thing);
}

short creature_drops_corpse_in_graveyard(struct Thing *thing)
{
  return _DK_creature_drops_corpse_in_graveyard(thing);
}

short creature_drops_crate_in_workshop(struct Thing *thing)
{
  return _DK_creature_drops_crate_in_workshop(thing);
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
    internal_set_thing_state(thing, thing->field_8);
  return true;
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

short creature_freeze_prisonors(struct Thing *thing)
{
  return _DK_creature_freeze_prisonors(thing);
}

short creature_going_home_to_sleep(struct Thing *thing)
{
  return _DK_creature_going_home_to_sleep(thing);
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

long get_combat_distance(struct Thing *thing, struct Thing *enemy)
{
    long dist,avgc;
    dist = get_2d_box_distance(&thing->mappos, &enemy->mappos);
    avgc = (enemy->field_56 + thing->field_56) / 2;
    if (dist < avgc)
        return 0;
    return dist - avgc;
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

long setup_prison_move(struct Thing *thing, struct Room *room)
{
  return _DK_setup_prison_move(thing, room);
}

long process_prison_visuals(struct Thing *thing, struct Room *room)
{
  struct CreatureControl *cctrl;
  cctrl = creature_control_get_from_thing(thing);
  if (cctrl->field_D2 != 0)
    return Lb_OK;
  if (game.play_gameturn-cctrl->field_82 > 200)
  {
    if (game.play_gameturn-cctrl->field_82 < 250)
    {
      set_creature_instance(thing, 44, 1, 0, 0);
      if (game.play_gameturn-cctrl->long_9A > 32)
      {
        play_creature_sound(thing, CrSnd_PrisonMoan, 2, 0);
        cctrl->long_9A = game.play_gameturn;
      }
      return Lb_SUCCESS;
    }
    cctrl->field_82 = game.play_gameturn;
  }
  if ( setup_prison_move(thing, room) )
  {
    thing->field_8 = CrSt_CreatureInPrison;
    return Lb_SUCCESS;
  }
  return Lb_OK;
}

short creature_in_prison(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct Room *room;
  TbResult ret;
  //return _DK_creature_in_prison(thing);
  cctrl = creature_control_get_from_thing(thing);
  room = get_room_thing_is_on(thing);
  if (room_is_invalid(room))
  {
    set_start_state(thing);
    return Lb_OK;
  }
  if ( (room->kind != RoK_PRISON) || (cctrl->work_room_id != room->index) )
  {
    set_start_state(thing);
    return Lb_OK;
  }
  if (room->total_capacity < room->workers_in)
  {
    if (is_my_player_number(room->owner))
      output_message(26, 0, 1);
    set_start_state(thing);
    return Lb_OK;
  }
  ret = process_prison_function(thing);
  if (ret == Lb_OK)
    process_prison_visuals(thing,room);
  return ret;
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
    if ((room->long_17 <= 0) || (room->workers_in <= 0))
    {
        ERRORLOG("Trying to remove spell from a room with no spell");
        return false;
    }
    room->long_17--;
    room->workers_in--;
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
    if (((spelltng->field_1 & 0x01) != 0) || thing_is_invalid(thing)
      || (get_2d_box_distance(&thing->mappos, &spelltng->mappos) >= 512))
    {
      set_start_state(thing);
      return 0;
    }
    enmroom = subtile_room_get(spelltng->mappos.x.stl.num,spelltng->mappos.y.stl.num);
    ownroom = find_nearest_room_for_thing_with_spare_capacity(thing, thing->owner, 3, 0, 1);
    if (room_is_invalid(ownroom) || !find_random_valid_position_for_thing_in_room_avoiding_object(thing, ownroom, &pos))
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
    thing->field_8 = CrSt_CreatureDropsSpellObjectInLibrary;
    return 1;
}

short creature_picks_up_trap_for_workshop(struct Thing *thing)
{
  return _DK_creature_picks_up_trap_for_workshop(thing);
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
    thing->field_8 = CrSt_CreatureStealGold;
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
    thing->field_8 = CrSt_CreatureStealSpell;
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

short creature_sleep(struct Thing *thing)
{
  return _DK_creature_sleep(thing);
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

/**
 * Return index of a dungeon which the hero may attack.
 * @todo HERO_AI Shouldn't we support allies with heroes?
 *
 * @param thing The hero searching for target.
 * @return Player index, or -1 if no dungeon to attack found.
 */
long good_find_enemy_dungeon(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  long i;
  SYNCDBG(18,"Starting");
  cctrl = creature_control_get_from_thing(thing);
  if ((cctrl->byte_8C != 0) || (cctrl->byte_8B != 0))
  {
    cctrl->byte_8C = 0;
    cctrl->byte_8B = 0;
    for (i = 0; i < PLAYERS_COUNT; i++)
    {
      if ( creature_can_get_to_dungeon(thing, i) )
      {
          SYNCDBG(18,"Returning enemy player %ld",i);
          return i;
      }
    }
  }
  SYNCDBG(18,"No enemy found");
  return -1;
}

TbBool good_setup_wander_to_exit(struct Thing *thing)
{
    struct Thing *gatetng;
    SYNCDBG(7,"Starting");
    gatetng = find_hero_door_hero_can_navigate_to(thing);
    if (thing_is_invalid(gatetng))
    {
        SYNCLOG("Can't find any exit gate for hero of breed %d.",(int)thing->model);
        return false;
    }
    if (!setup_person_move_to_position(thing, gatetng->mappos.x.stl.num, gatetng->mappos.y.stl.num, 0))
    {
        WARNLOG("Hero of breed %d can't move to exit gate at (%d,%d).",(int)thing->model,(int)gatetng->mappos.x.stl.num, (int)gatetng->mappos.y.stl.num);
        return false;
    }
    thing->field_8 = CrSt_GoodLeaveThroughExitDoor;
    return true;
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

TbBool good_setup_attack_rooms(struct Thing *thing, long dngn_id)
{
    struct Room *room;
    struct CreatureControl *cctrl;
    struct Coord3d pos;
    room = find_nearest_room_for_thing_excluding_two_types(thing, dngn_id, 7, 1, 1);
    if (room_is_invalid(room))
    {
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(thing, room, &pos)
      || !creature_can_navigate_to_with_storage(thing, &pos, 1) )
    {
        return false;
    }
    if (!setup_random_head_for_room(thing, room, 1))
    {
        ERRORLOG("setup random head for room failed");
        return false;
    }
    event_create_event_or_update_nearby_existing_event(
        get_subtile_center_pos(room->stl_x), get_subtile_center_pos(room->stl_y),
        19, room->owner, 0);
    if (is_my_player_number(room->owner))
      output_message(15, 400, 1);
    cctrl = creature_control_get_from_thing(thing);
    thing->field_8 = CrSt_GoodAttackRoom1;
    cctrl->field_80 = room->index;
    return true;
}

TbBool good_setup_loot_treasure_room(struct Thing *thing, long dngn_id)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    //return _DK_good_setup_loot_treasure_room(thing, dngn_id);
    room = find_random_room_creature_can_navigate_to(thing, dngn_id, RoK_TREASURE, 0);
    if (room_is_invalid(room))
    {
        SYNCDBG(6,"No accessible player %ld treasure room found",dngn_id);
        return false;
    }
    if (!setup_person_move_to_position(thing, room->stl_x, room->stl_y, 0))
    {
        WARNLOG("Cannot setup move to player %ld treasure room",dngn_id);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->field_8 = CrSt_CreatureSearchForGoldToStealInRoom2;
    cctrl->field_80 = room->index;
    return true;
}

TbBool good_setup_loot_research_room(struct Thing *thing, long dngn_id)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    room = find_random_room_creature_can_navigate_to(thing, dngn_id, RoK_LIBRARY, 0);
    if (room_is_invalid(room))
    {
        SYNCDBG(6,"No accessible player %ld library found",dngn_id);
        return false;
    }
    if (!setup_person_move_to_position(thing, room->stl_x, room->stl_y, 0))
    {
        WARNLOG("Cannot setup move to player %ld library",dngn_id);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->field_8 = CrSt_CreatureSearchForSpellToStealInRoom;
    cctrl->field_80 = room->index;
    return true;
}

TbBool good_setup_wander_to_creature(struct Thing *wanderer, long dngn_id)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *thing;
    long navigable_targets,target_match;
    unsigned long k;
    long i;
    SYNCDBG(7,"Starting");
    cctrl = creature_control_get_from_thing(wanderer);
    dungeon = get_dungeon(dngn_id);
    navigable_targets = 0;
    // Get the amount of possible targets
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
      if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
      {
          if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
            navigable_targets++;
      }
      // Thing list loop body ends
      k++;
      if (k > CREATURES_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
    }
    // Select random target
    if (navigable_targets < 1)
    {
        SYNCDBG(4,"No player %d creatures found to wander to",(int)dngn_id);
        return false;
    }
    target_match = ACTION_RANDOM(navigable_targets);
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
      if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
      {
          if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
          {
              if (target_match > 0)
              {
                  target_match--;
              } else
              if ( setup_person_move_to_position(wanderer, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0) )
              {
                  thing->field_8 = CrSt_GoodDoingNothing;
                  return true;
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
    WARNLOG("Internal - couldn't wander to player %d creature",(int)dngn_id);
    return false;
}

TbBool good_setup_wander_to_imp(struct Thing *wanderer, long dngn_id)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *thing;
    long navigable_targets,target_match;
    unsigned long k;
    long i;
    SYNCDBG(7,"Starting");
    cctrl = creature_control_get_from_thing(wanderer);
    dungeon = get_dungeon(dngn_id);
    navigable_targets = 0;
    // Get the amount of possible targets
    k = 0;
    i = dungeon->worker_list_start;
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
      if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
      {
          if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
            navigable_targets++;
      }
      // Thing list loop body ends
      k++;
      if (k > CREATURES_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
    }
    // Select random target
    if (navigable_targets < 1)
    {
        SYNCDBG(4,"No player %d creatures found to wander to",(int)dngn_id);
        return false;
    }
    target_match = ACTION_RANDOM(navigable_targets);
    k = 0;
    i = dungeon->worker_list_start;
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
      if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
      {
          if ( creature_can_navigate_to(wanderer, &thing->mappos, 0) )
          {
              if (target_match > 0)
              {
                  target_match--;
              } else
              if ( setup_person_move_to_position(wanderer, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 0) )
              {
                  thing->field_8 = CrSt_GoodDoingNothing;
                  return true;
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
    WARNLOG("Internal - couldn't wander to player %d creature",(int)dngn_id);
    return false;
}

short good_attack_room(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        return false;
    }
    return _DK_good_attack_room(thing);
}

short good_back_at_start(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        return false;
    }
    return _DK_good_back_at_start(thing);
}

TbBool good_setup_wander_to_dungeon_heart(struct Thing *thing, long dngn_idx)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *heartng;
    dungeon = get_dungeon(dngn_idx);
    if (dungeon_invalid(dungeon) || (thing->owner == dngn_idx))
    {
        ERRORLOG("Creature breed %d tried to wander to invalid player (%d) heart", (int)thing->model, (int)dngn_idx);
        return false;
    }
    player = get_player(dngn_idx);
    if ((!player_exists(player)) || (dungeon->dnheart_idx < 1))
    {
        WARNLOG("Creature breed %d tried to wander to inactive player (%d) heart", (int)thing->model, (int)dngn_idx);
        return false;
    }
    heartng = thing_get(dungeon->dnheart_idx);
    set_creature_object_combat(thing, heartng);
    return true;
}

short good_doing_nothing(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct PlayerInfo *player;
    long nturns;
    long i;
    //return _DK_good_doing_nothing(thing);
    SYNCDBG(18,"Starting");
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        return false;
    }
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return false;
    }
    nturns = game.play_gameturn - cctrl->long_9A;
    if (nturns <= 1)
      return true;
    if (cctrl->field_5 > (long)game.play_gameturn)
    {
      if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
        thing->field_8 = CrSt_GoodDoingNothing;
      return true;
    }
    i = cctrl->sbyte_89;
    if (i != -1)
    {
      player = get_player(i);
      if (player_invalid(player))
      {
          ERRORLOG("Invalid target player in thing no %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
          cctrl->sbyte_89 = -1;
          return false;
      }
      if (player->victory_state != 2)
      {
        nturns = game.play_gameturn - cctrl->long_91;
        if (nturns <= 400)
        {
          if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
          {
            thing->field_8 = CrSt_GoodDoingNothing;
            return false;
          }
        } else
        {
          if (!creature_can_get_to_dungeon(thing,i))
          {
            cctrl->sbyte_89 = -1;
          }
        }
      } else
      {
        cctrl->sbyte_89 = -1;
      }
    }
    i = cctrl->sbyte_89;
    if (i == -1)
    {
      nturns = game.play_gameturn - cctrl->long_91;
      if (nturns > 400)
      {
        cctrl->long_91 = game.play_gameturn;
        cctrl->byte_8C = 1;
      }
      nturns = game.play_gameturn - cctrl->long_8D;
      if (nturns > 64)
      {
        cctrl->long_8D = game.play_gameturn;
        cctrl->sbyte_89 = good_find_enemy_dungeon(thing);
      }
      i = cctrl->sbyte_89;
      if (i == -1)
      {
        SYNCDBG(4,"No enemy dungeon to perform task");
        if ( creature_choose_random_destination_on_valid_adjacent_slab(thing) )
        {
          thing->field_8 = CrSt_GoodDoingNothing;
          return true;
        }
        cctrl->field_5 = game.play_gameturn + 16;
      }
      return true;
    }
    SYNCDBG(8,"Performing task %d",(int)cctrl->field_4);
    switch (cctrl->field_4)
    {
    case 1: // ATTACK_ROOMS
        if (good_setup_attack_rooms(thing, i))
        {
            return true;
        }
        WARNLOG("Can't attack player %d rooms, switching to attack heart", (int)i);
        cctrl->field_4 = 3;
        return false;
    case 3: // ATTACK_DUNGEON_HEART
        if (good_setup_wander_to_dungeon_heart(thing, i))
        {
            return true;
        }
        ERRORLOG("Cannot wander to player %d heart", (int)i);
        return false;
    case 4: // STEAL_GOLD
        crstat = creature_stats_get_from_thing(thing);
        if (thing->long_13 < crstat->gold_hold)
        {
            if (good_setup_loot_treasure_room(thing, i))
                return true;
            WARNLOG("Can't loot player %d treasury, switching to attack heart", (int)i);
            cctrl->field_4 = 3;
        } else
        {
            if (good_setup_wander_to_exit(thing))
                return true;
            WARNLOG("Can't wander to exit after looting player %d treasury, switching to attack heart", (int)i);
            cctrl->field_4 = 3;
        }
        return false;
    case 5: // STEAL_SPELLS
        //TODO STEAL_SPELLS write a correct code for stealing spells, then enable this
        if (true)//!thing->holds_a_spell)
        {
            if (good_setup_loot_research_room(thing, i))
                return true;
            WARNLOG("Can't loot player %d spells, switching to attack heart", (int)i);
            cctrl->field_4 = 3;
        } else
        {
            if (good_setup_wander_to_exit(thing))
                return true;
            WARNLOG("Can't wander to exit after looting player %d spells, switching to attack heart", (int)i);
            cctrl->field_4 = 3;
        }
        return false;
    case 2: // ATTACK_ENEMIES
    case 0:
    default:
        if (ACTION_RANDOM(2) == 1)
        {
          if (good_setup_wander_to_creature(thing, cctrl->sbyte_89))
          {
              SYNCDBG(17,"Finished - wandering to creature");
              return true;
          }
        }
        if (good_setup_wander_to_imp(thing, cctrl->sbyte_89))
        {
            SYNCDBG(17,"Finished - wandering to worker");
            return true;
        }
        WARNLOG("Can't attack player %d creature, switching to attack heart", (int)cctrl->sbyte_89);
        cctrl->field_4 = 3;
        return false;
    }
}

short good_drops_gold(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    return _DK_good_drops_gold(thing);
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

short good_leave_through_exit_door(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *tmptng;
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    //return _DK_good_leave_through_exit_door(thing);
    tmptng = find_base_thing_on_mapwho(1, 49, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (thing_is_invalid(tmptng))
    {
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->long_13 = 0;
    cctrl->field_282 = game.hero_door_wait_time;
    cctrl->byte_8A = tmptng->field_9;
    place_thing_in_creature_controlled_limbo(thing);
    internal_set_thing_state(thing, CrSt_GoodWaitInExitDoor);
    return 1;
}

short good_returns_to_start(struct Thing *thing)
{
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    return _DK_good_returns_to_start(thing);
}

short good_wait_in_exit_door(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *tmptng;
    // Debug code to find incorrect states
    if (thing->owner != hero_player_number)
    {
        ERRORLOG("Non hero thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return false;
    }
    //return _DK_good_wait_in_exit_door(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->field_282 <= 0)
        return 0;
    cctrl->field_282--;
    if (cctrl->field_282 == 0)
    {
        tmptng = find_base_thing_on_mapwho(1, 49, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        if (!thing_is_invalid(tmptng))
        {
            if (cctrl->byte_8A == tmptng->field_9)
            {
              remove_thing_from_creature_controlled_limbo(thing);
              set_start_state(thing);
              return 1;
            }
        }
        thing->long_13 = 0;
        tmptng = thing_get(cctrl->field_6E);
        if (!thing_is_invalid(tmptng))
        {
            delete_thing_structure(tmptng, 0);
        }
        kill_creature(thing, 0, -1, 1, 0, 0);
    }
    return 0;
}

short guarding(struct Thing *thing)
{
  return _DK_guarding(thing);
}

short imp_arrives_at_convert_dungeon(struct Thing *thing)
{
    //return _DK_imp_arrives_at_convert_dungeon(thing);
    if (check_place_to_convert_excluding(thing,
           map_to_slab[thing->mappos.x.stl.num],
           map_to_slab[thing->mappos.y.stl.num]) )
    {
      internal_set_thing_state(thing, 74);
    } else
    {
      internal_set_thing_state(thing, 8);
    }
    return 1;
}

short imp_arrives_at_dig_or_mine(struct Thing *thing)
{
  return _DK_imp_arrives_at_dig_or_mine(thing);
}

short imp_arrives_at_improve_dungeon(struct Thing *thing)
{
  //return _DK_imp_arrives_at_improve_dungeon(thing);
  if ( check_place_to_pretty_excluding(thing,
          map_to_slab[thing->mappos.x.stl.num],
          map_to_slab[thing->mappos.y.stl.num]) )
  {
    internal_set_thing_state(thing, 10);
  } else
  {
    internal_set_thing_state(thing, 8);
  }
  return 1;
}

short imp_arrives_at_reinforce(struct Thing *thing)
{
  return _DK_imp_arrives_at_reinforce(thing);
}

short imp_birth(struct Thing *thing)
{
  return _DK_imp_birth(thing);
}

short imp_converts_dungeon(struct Thing *thing)
{
  return _DK_imp_converts_dungeon(thing);
}

TbBool too_much_gold_lies_around_thing(struct Thing *thing)
{
  return gold_pile_with_maximum_at_xy(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

short imp_digs_mines(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct MapTask* mtask;
    struct SlabMap *slb;
    struct Coord3d pos;
    MapSubtlCoord stl_x,stl_y;
    long delta_x,delta_y;
    SYNCDBG(19,"Starting");
    // return _DK_imp_digs_mines(thing);
    cctrl = creature_control_get_from_thing(thing);
    mtask = get_task_list_entry(thing->owner, cctrl->word_91);
    stl_x = stl_num_decode_x(cctrl->word_8F);
    stl_y = stl_num_decode_y(cctrl->word_8F);
    slb = get_slabmap_for_subtile(stl_x, stl_y);

    // Check if we've arrived at the destination
    delta_x = abs(thing->mappos.x.stl.num - cctrl->moveto_pos.x.stl.num);
    delta_y = abs(thing->mappos.y.stl.num - cctrl->moveto_pos.y.stl.num);
    if ((mtask->field_1 != cctrl->word_8F) || (delta_x >= 1) || (delta_y >= 1))
    {
      clear_creature_instance(thing);
      internal_set_thing_state(thing, 8);
      return 1;
    }
    // If gems are marked for digging, but there is too much gold laying around, then don't dig
    if ((slb->kind == SlbT_GEMS) && too_much_gold_lies_around_thing(thing))
    {
      clear_creature_instance(thing);
      internal_set_thing_state(thing, 8);
      return 1;
    }
    // Turn to the correct direction to do the task
    pos.x.stl.num = stl_x;
    pos.y.stl.num = stl_y;
    pos.x.stl.pos = 128;
    pos.y.stl.pos = 128;
    if (creature_turn_to_face(thing, &pos))
    {
      return 1;
    }

    if (mtask->field_0 == 0)
    {
        clear_creature_instance(thing);
        internal_set_thing_state(thing, 8);
        return 1;
    }

    if (cctrl->field_D2 == 0)
    {
        set_creature_instance(thing, 30, 0, 0, 0);
    }

    if (mtask->field_0 == 2)
    {
        crstat = creature_stats_get_from_thing(thing);
        // If the creature holds more gold than its able
        if (thing->long_13 > crstat->gold_hold)
        {
          if (game.play_gameturn - cctrl->field_2C7 > 128)
          {
            if (check_out_imp_has_money_for_treasure_room(thing))
              return 1;
            cctrl->field_2C7 = game.play_gameturn;
          }

          drop_gold_pile(thing->long_13 - crstat->gold_hold, &thing->mappos);
          thing->long_13 = crstat->gold_hold;
        }
    }
    return 1;
}

short imp_doing_nothing(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    SYNCDBG(19,"Starting");
    //return _DK_imp_doing_nothing(thing);
    if (!thing_is_creature_special_digger(thing))
    {
        ERRORLOG("Non digger thing %ld, %s, owner %ld - reset",(long)thing->index,thing_model_name(thing),(long)thing->owner);
        set_start_state(thing);
        erstat_inc(ESE_BadCreatrState);
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (game.play_gameturn-cctrl->long_9A <= 1)
        return 1;
    if (check_out_imp_last_did(thing))
        return 1;
    if (check_out_available_imp_tasks(thing))
        return 1;
    if (check_out_imp_tokes(thing))
        return 1;
    if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
    {
        thing->field_8 = CrSt_ImpDoingNothing;
        return 1;
    }
    dungeon->lvstats.promises_broken++;
    return 1;
}

short imp_drops_gold(struct Thing *thing)
{
  return _DK_imp_drops_gold(thing);
}

short imp_improves_dungeon(struct Thing *thing)
{
  return _DK_imp_improves_dungeon(thing);
}

short imp_last_did_job(struct Thing *thing)
{
    //return _DK_imp_last_did_job(thing);
    if (check_out_imp_last_did(thing))
    {
        return true;
    } else
    {
        set_start_state(thing);
        return false;
    }
}

short imp_picks_up_gold_pile(struct Thing *thing)
{
  return _DK_imp_picks_up_gold_pile(thing);
}

short imp_reinforces(struct Thing *thing)
{
  return _DK_imp_reinforces(thing);
}

short imp_toking(struct Thing *thing)
{
  return _DK_imp_toking(thing);
}

long process_torture_visuals(struct Thing *thing, struct Room *room, long a3)
{
  return _DK_process_torture_visuals(thing, room, a3);
}

short kinky_torturing(struct Thing *thing)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  struct Room *room;
  //return _DK_kinky_torturing(thing);
  crstat = creature_stats_get_from_thing(thing);
  cctrl = creature_control_get_from_thing(thing);
  room = get_room_thing_is_on(thing);
  if (!room_is_invalid(room))
    if ((room->kind == RoK_TORTURE) && (cctrl->work_room_id == room->index))
      if (game.play_gameturn-cctrl->field_82 <= crstat->torture_time)
      {
        process_kinky_function(thing);
        process_torture_visuals(thing, room, 110);
        return true;
      }
  set_start_state(thing);
  return false;
}

short mad_killing_psycho(struct Thing *thing)
{
  return _DK_mad_killing_psycho(thing);
}

short manufacturing(struct Thing *thing)
{
  return _DK_manufacturing(thing);
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

long process_kinky_function(struct Thing *thing)
{
  struct CreatureStats *crstat;
  //return _DK_process_kinky_function(thing);
  crstat = creature_stats_get_from_thing(thing);
  anger_apply_anger_to_creature(thing, crstat->annoy_in_torture, 4, 1);
  return 0;
}

long room_still_valid_as_type_for_thing(struct Room *room, long rkind, struct Thing *thing)
{
  return ((room->field_0 & 0x01) != 0) && (room->kind == rkind);
}

void create_effect_around_thing(struct Thing *thing, long eff_kind)
{
  _DK_create_effect_around_thing(thing, eff_kind);
}

void prison_convert_creature_to_skeleton(struct Room *room, struct Thing *thing)
{
  struct Dungeon *dungeon;
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  struct Thing *crthing;
  long crmodel;
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  crmodel = get_room_create_creature_model(room->kind); // That normally returns skeleton breed
  crthing = create_creature(&thing->mappos, crmodel, room->owner);
  if (thing_is_invalid(crthing))
  {
      ERRORLOG("Couldn't create creature model %ld in prison", crmodel);
      return;
  }
  init_creature_level(crthing, cctrl->explevel);
  set_start_state(crthing);
  if (creature_model_bleeds(thing->model))
    create_effect_around_thing(thing, 10);
  kill_creature(thing, INVALID_THING, -1, 1, 0, 0);
  dungeon = get_dungeon(room->owner);
  if (!dungeon_invalid(dungeon))
      dungeon->lvstats.skeletons_raised++;
}

TbBool process_prisoner_skelification(struct Thing *thing, struct Room *room)
{
  struct CreatureStats *crstat;
  crstat = creature_stats_get_from_thing(thing);
  if ( (thing->health >= 0) || (!crstat->humanoid_creature) )
    return false;
  if (ACTION_RANDOM(101) > game.prison_skeleton_chance)
    return false;
  if (is_my_player_number(thing->owner))
    output_message(55, 0, 1);
  prison_convert_creature_to_skeleton(room,thing);
  return true;
}

void remove_health_from_thing_and_display_health(struct Thing *thing, long delta)
{
  _DK_remove_health_from_thing_and_display_health(thing, delta);
}

long slab_by_players_land(long plyr_idx, long slb_x, long slb_y)
{
  return _DK_slab_by_players_land(plyr_idx, slb_x, slb_y);
}

TbBool jailbreak_possible(struct Room *room, long plyr_idx)
{
  unsigned long i;
  unsigned long k;
  struct SlabMap *slb;
  if ( (room->owner == plyr_idx) || (!room->slabs_list) )
    return false;
  k = 0;
  i = room->slabs_list;
  while (i > 0)
  {
    slb = get_slabmap_direct(i);
    if (slabmap_block_invalid(slb))
    {
      ERRORLOG("Jump to invalid room slab detected");
      break;
    }
    if (slab_by_players_land(plyr_idx, slb_num_decode_x(i), slb_num_decode_y(i)))
      return true;
    i = get_next_slab_number_in_room(i);
    k++;
    if (k > map_tiles_x*map_tiles_y)
    {
      ERRORLOG("Infinite loop detected when sweeping room slabs");
      break;
    }
  }
  return false;
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

long process_prison_food(struct Thing *thing, struct Room *room)
{
  return _DK_process_prison_food(thing, room);
}

long process_prison_function(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct Room *room;
  //return _DK_process_prison_function(thing);
  cctrl = creature_control_get_from_thing(thing);
  room = room_get(cctrl->work_room_id);
  if ( !room_still_valid_as_type_for_thing(room, 4, thing) )
  {
    set_start_state(thing);
    return 1;
  }
  process_creature_hunger(thing);
  if ( process_prisoner_skelification(thing,room) )
    return -1;
  if ((cctrl->field_D2 == 0) && process_prison_food(thing, room) )
    return 1;
  if ((game.play_gameturn & 0x3F) != 0)
    return 0;
  if (!jailbreak_possible(room, thing->owner))
    return 0;
  if ( is_my_player_number(room->owner) )
    output_message(57, 0, 1);
  set_start_state(thing);
  return 1;
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

void convert_creature_to_ghost(struct Room *room, struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *newthing;
    int crmodel;
    crmodel = get_room_create_creature_model(room->kind);
    newthing = create_creature(&thing->mappos, crmodel, room->owner);
    if (thing_is_invalid(newthing))
    {
        ERRORLOG("Couldn't create creature model %d in torture room",crmodel);
        return;
    }
    cctrl = creature_control_get_from_thing(thing);
    init_creature_level(newthing, cctrl->explevel);
    if (creature_model_bleeds(thing->model))
      create_effect_around_thing(newthing, 10);
    set_start_state(newthing);
    kill_creature(thing, INVALID_THING, -1, 1, 1, 0);
    dungeon = get_dungeon(room->owner);
    if (!dungeon_invalid(dungeon))
        dungeon->lvstats.ghosts_raised++;
    if (is_my_player_number(room->owner))
        output_message(56, 0, 1);
}

void convert_tortured_creature_owner(struct Thing *thing, long new_owner)
{
    struct Dungeon *dungeon;
    if (is_my_player_number(new_owner))
    {
        output_message(54, 0, 1);
    } else
    if (is_my_player_number(thing->owner))
    {
        output_message(63, 0, 1);
    }
    change_creature_owner(thing, new_owner);
    anger_set_creature_anger_all_types(thing, 0);
    dungeon = get_dungeon(new_owner);
    if (!dungeon_invalid(dungeon))
        dungeon->lvstats.creatures_converted++;
}

long reveal_players_map_to_player(struct Thing *thing, long a2)
{
    return _DK_reveal_players_map_to_player(thing, a2);
}

long process_torture_function(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Room *room;
    long i;
    //return _DK_process_torture_function(thing);
    crstat = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);
    room = room_get(cctrl->work_room_id);
    if ( !room_still_valid_as_type_for_thing(room,5,thing) )
    {
        set_start_state(thing);
        return 1;
    }
    anger_apply_anger_to_creature(thing, crstat->annoy_in_torture, 4, 1);
    if ((long)game.play_gameturn >= cctrl->field_82 + game.turns_per_torture_health_loss)
    {
        i = compute_creature_max_health(game.torture_health_loss,cctrl->explevel);
        remove_health_from_thing_and_display_health(thing, i);
        cctrl->field_82 = (long)game.play_gameturn;
    }
    if ((thing->health < 0) && (game.ghost_convert_chance > 0))
    {
        if (ACTION_RANDOM(game.ghost_convert_chance) == 0)
        {
            convert_creature_to_ghost(room, thing);
            return -1;
        }
    }
    if (room->owner == thing->owner)
        return 0;

    i = ((long)game.play_gameturn - cctrl->long_9A) * room->efficiency >> 8;

    if ((cctrl->spell_flags & 0x02) != 0)
      i = (4 * i) / 3;
    if (cctrl->field_21 != 0)
      i = (5 * i) / 4;
    if ((i < crstat->torture_time) || (cctrl->word_A6 == 0))
        return 0;
    i = (long)game.play_gameturn - crstat->torture_time - cctrl->long_9A;
    if (ACTION_RANDOM(100) >= i/64 + 1)
        return 0;
    if (ACTION_RANDOM(3) == 0)
    {
        convert_tortured_creature_owner(thing, room->owner);
        return 1;
    }
    cctrl->long_9A = game.play_gameturn - crstat->torture_time / 2;
    reveal_players_map_to_player(thing, room->owner);
    return 0;
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
        || (cctrl1->field_3) && (tmptng == tng2))
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
                    thing->field_8 = CrSt_SeekTheEnemy;
                    cctrl->field_282 = game.play_gameturn;
                    return 1;
                  }
              }
              if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
              {
                  thing->field_8 = CrSt_SeekTheEnemy;
                  cctrl->field_282 = game.play_gameturn;
              }
            }
            return 1;
        }
        if (ACTION_RANDOM(64) == 0)
        {
            if (setup_person_move_close_to_position(thing, enemytng->mappos.x.stl.num, enemytng->mappos.y.stl.num, 0))
            {
              thing->field_8 = CrSt_SeekTheEnemy;
            }
        }
    }
    // No enemy found - do some random movement
    if (ACTION_RANDOM(12) != 0)
    {
        if ( creature_choose_random_destination_on_valid_adjacent_slab(thing) )
        {
            thing->field_8 = CrSt_SeekTheEnemy;
            return 1;
        }
    } else
    if (get_random_position_in_dungeon_for_creature(thing->owner, 1, thing, &pos))
    {
        if ( setup_person_move_to_position(thing, pos.x.val >> 8, pos.y.val >> 8, 0) )
        {
            thing->field_8 = CrSt_SeekTheEnemy;
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

short torturing(struct Thing *thing)
{
  return _DK_torturing(thing);
}

/** Returns if the creature meets conditions to be trained.
 *
 * @param thing The creature thing to be tested.
 * @return
 */
TbBool creature_can_be_trained(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    //return _DK_creature_can_be_trained(thing);
    dungeon = get_dungeon(thing->owner);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    // Creatures without training value can't be trained
    if (crstat->training_value == 0)
        return false;
    // Creatures which reached players max level can't be trained
    if (cctrl->explevel >= dungeon->creature_max_level[thing->model])
        return false;
    // Creatures which reached absolute max level and have no grow up creature
    if ((cctrl->explevel >= 9) && (crstat->grow_up == 0))
        return false;
    return true;
}

TbBool player_can_afford_to_train_creature(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct CreatureStats *crstat;
    //return _DK_player_can_afford_to_train_creature(thing);
    dungeon = get_dungeon(thing->owner);
    crstat = creature_stats_get_from_thing(thing);
    return (dungeon->field_AF9 >= crstat->training_cost);
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
        if (dungeon->field_1420[thing->model])
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
    if (dungeon->creature_max_level[thing->model] > cctrl->explevel)
    {
      if ((cctrl->explevel < 9) || (crstat->grow_up != 0))
        cctrl->field_AD |= 0x40;
    }
    return true;
}

void setup_training_move(struct Thing *thing, SubtlCodedCoords stl_num)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->moveto_pos.x.stl.num = stl_num_decode_x(stl_num);
    cctrl->moveto_pos.x.stl.pos = 128;
    cctrl->moveto_pos.y.stl.num = stl_num_decode_y(stl_num);
    cctrl->moveto_pos.y.stl.pos = 128;
    cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
    if (thing_in_wall_at(thing, &cctrl->moveto_pos))
    {
        ERRORLOG("Illegal setup to (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
        set_start_state(thing);
    }
}

struct Thing *get_creature_in_training_room_which_could_accept_partner(struct Room *room, struct Thing *partnertng)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i;
    i = room->creatures_list;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
          ERRORLOG("Jump to invalid creature %ld detected",i);
        break;
      }
      i = cctrl->next_in_room;
      // Per creature code
      if (thing != partnertng)
      {
        if (cctrl->word_9F == 0)
        {
          return thing;
        }
      }
      // Per creature code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
    }
    return INVALID_THING;
}

void setup_move_to_new_training_position(struct Thing *thing, struct Room *room, unsigned long a3)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Thing *prtng;
    struct CreatureControl *prctrl;
    struct Coord3d pos;
    SYNCDBG(8,"Starting");
    //_DK_setup_move_to_new_training_position(thing, room, a3);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    if ( a3 )
      cctrl->byte_9E = 50;
    if ((crstat->partner_training > 0) && (ACTION_RANDOM(100) < crstat->partner_training))
    {
        prtng = get_creature_in_training_room_which_could_accept_partner(room, thing);
        if (!thing_is_invalid(prtng))
        {
            prctrl = creature_control_get_from_thing(thing);
            prctrl->byte_9A = 6;
            prctrl->byte_9B = 75;
            prctrl->word_9F = thing->index;
            prctrl->long_A1 = thing->field_9;
            cctrl->byte_9A = 6;
            cctrl->byte_9B = 75;
            cctrl->word_9F = prtng->index;
            cctrl->long_A1 = prtng->field_9;
            return;
      }
    }
    cctrl->byte_9A = 1;
    if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
      cctrl->moveto_pos.x.stl.num = pos.x.stl.num;
      cctrl->moveto_pos.x.stl.pos = 128;
      cctrl->moveto_pos.y.stl.num = pos.y.stl.num;
      cctrl->moveto_pos.y.stl.pos = 128;
      cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
      if (thing_in_wall_at(thing, &cctrl->moveto_pos))
      {
          ERRORLOG("Illegal setup to wall at (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
          set_start_state(thing);
      }
    }
    if (cctrl->field_D2 == 0)
    {
      set_creature_instance(thing, 1, 1, 0, 0);
    }
    SYNCDBG(8,"End");
}

void setup_training_search_for_post(struct Thing *thing)
{
    struct Room *room;
    struct Thing *traintng;
    long start_slab;
    long slb_x,slb_y;
    long i,k;
    room = subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    // Let's start from a random slab
    slb_x = -1;
    slb_y = -1;
    start_slab = ACTION_RANDOM(room->slabs_count);
    k = start_slab;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        if (k <= 0)
            break;
        k--;
    }
    // Got random starting slab, now sweep room slabs from it
    traintng = INVALID_THING;
    k = room->slabs_count;
    i = get_slab_number(slb_x,slb_y);
    while (k > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        if (i == 0)
          i = room->slabs_list;
        // Per room tile code - find a training post
        traintng = get_object_at_subtile_of_model_and_owned_by(3*slb_x+1, 3*slb_y+1, 31, thing->owner);
        if (!thing_is_invalid(traintng))
            break;
        // Per room tile code ends
        k--;
    }
    // Got trainer (or not...), now do the correct action
    if (thing_is_invalid(traintng))
    {
        SYNCDBG(6,"Room no longer have training post, moving somewhere else.");
        setup_move_to_new_training_position(thing, room, 1);
    } else
    {
        i = get_subtile_number(traintng->mappos.x.stl.num,traintng->mappos.y.stl.num);
        setup_training_move(thing, i);
    }
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
      thing1->field_8 = a3;
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
      thing1->field_8 = a3;
      return 1;
    }
    return 1;
}

void process_creature_in_training_room(struct Thing *thing, struct Room *room)
{
    static const struct Around corners[] = {
        {1, 2},
        {0, 1},
        {1, 0},
        {2, 1},
    };
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Thing *traintng;
    struct Thing *crtng;
    struct CreatureControl *cctrl2;
    struct Coord3d pos;
    long speed,dist;
    long i;
    cctrl = creature_control_get_from_thing(thing);
    SYNCDBG(18,"Starting mode %d",(int)cctrl->byte_9A);
    //_DK_process_creature_in_training_room(thing, room); return;
    cctrl->field_4A = 0;
    switch (cctrl->byte_9A)
    {
    case 1:
        if (cctrl->field_D2 != 0)
            break;
        if (cctrl->byte_9E <= 1)
        {
            setup_training_search_for_post(thing);
            cctrl->byte_9E = 50;
            break;
        }
        cctrl->byte_9E--;
        speed = get_creature_speed(thing);
        i = creature_move_to(thing, &cctrl->moveto_pos, speed, 0, 0);
        if (i == 1)
        {
            // Find a training post
            traintng = get_object_at_subtile_of_model_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, 31, thing->owner);
            if (thing_is_invalid(traintng))
            {
                setup_move_to_new_training_position(thing, room, 0);
                break;
            }
            cctrl->byte_9A = 2;
            cctrl->byte_9E = 50;
        } else
        if (i == -1)
        {
            ERRORLOG("Cannot get where we're going in the training room.");
            set_start_state(thing);
        }
        break;
    case 2:
        for (i=0; i < 4; i++)
        {
            long slb_x,slb_y;
            long stl_x,stl_y;
            struct SlabMap *slb;
            slb_x = map_to_slab[thing->mappos.x.stl.num] + (long)small_around[i].delta_x;
            slb_y = map_to_slab[thing->mappos.y.stl.num] + (long)small_around[i].delta_y;
            slb = get_slabmap_block(slb_x,slb_y);
            if ((slb->kind != SlbT_TRAINING) || (slabmap_owner(slb) != thing->owner))
                continue;
              stl_x = 3*slb_x + (long)corners[i].delta_x;
              stl_y = 3*slb_y + (long)corners[i].delta_y;
              traintng = INVALID_THING;
              crtng = get_creature_of_model_training_at_subtile_and_owned_by(stl_x, stl_y, -1, thing->owner);
              if (thing_is_invalid(crtng))
              {
                  traintng = get_object_at_subtile_of_model_and_owned_by(stl_x, stl_y, 31, thing->owner);
              }
              if (!thing_is_invalid(crtng) || !thing_is_invalid(traintng))
              {
                  cctrl->byte_9C = 3 * map_to_slab[thing->mappos.x.stl.num] + 1;
                  cctrl->byte_9D = 3 * map_to_slab[thing->mappos.y.stl.num] + 1;
                  cctrl->moveto_pos.x.stl.num = stl_x;
                  cctrl->moveto_pos.y.stl.num = stl_y;
                  cctrl->moveto_pos.x.stl.pos = 128;
                  cctrl->moveto_pos.y.stl.pos = 128;
                  cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
                  if (thing_in_wall_at(thing, &cctrl->moveto_pos))
                  {
                    ERRORLOG("Illegal setup to (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
                    set_start_state(thing);
                  }
                  cctrl->byte_9A = 3;
                  break;
              }
        }
        if (cctrl->byte_9A == 2)
          setup_move_to_new_training_position(thing, room, 1);
        break;
    case 3:
        speed = get_creature_speed(thing);
        i = creature_move_to(thing, &cctrl->moveto_pos, speed, 0, 0);
        if (i == 1)
        {
            crtng = get_creature_of_model_training_at_subtile_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, -1, thing->owner);
            if (!thing_is_invalid(crtng))
            {
                setup_move_to_new_training_position(thing, room, 1);
                break;
            }
            cctrl->byte_9A = 4;
        } else
        if ( i == -1 )
        {
            ERRORLOG("Cannot get where we're going in the training room.");
            set_start_state(thing);
        }
        break;
    case 4:
        pos.x.val = ((long)cctrl->byte_9C << 8) + 128;
        pos.y.val = ((long)cctrl->byte_9D << 8) + 128;
        if (creature_turn_to_face(thing, &pos) < 56)
        {
          cctrl->byte_9A = 5;
          cctrl->byte_9B = 75;
        }
        break;
    case 6:
        if (cctrl->word_9F == 0)
        {
            setup_move_to_new_training_position(thing, room, 0);
            return;
        }
        crtng = thing_get(cctrl->word_9F);
        if ((crtng->field_9 != cctrl->long_A1) || ((crtng->field_0 & 0x01) == 0))
        {
            setup_move_to_new_training_position(thing, room, 0);
            return;
        }
        cctrl2 = creature_control_get_from_thing(crtng);
        if (cctrl2->word_9F != thing->index)
        {
            cctrl->word_9F = 0;
            setup_move_to_new_training_position(thing, room, 0);
            break;
        }
        if (subtile_room_get(crtng->mappos.x.stl.num, crtng->mappos.y.stl.num) != room)
        {
            cctrl->word_9F = 0;
            cctrl2->word_9F = 0;
            setup_move_to_new_training_position(thing, room, 0);
            break;
        }
        crstat = creature_stats_get_from_thing(thing);
        dist = get_combat_distance(thing, crtng);
        if (dist > 284)
        {
            if (creature_move_to(thing, &crtng->mappos, get_creature_speed(thing), 0, 0) == -1)
            {
              ERRORLOG("cannot navigate to training partner");
              setup_move_to_new_training_position(thing, room, 0);
              cctrl->word_9F = 0;
            }
        } else
        if (dist >= 156)
        {
            if (creature_turn_to_face(thing, &crtng->mappos) < 56)
            {
              cctrl->byte_9B--;
              if (cctrl->byte_9B > 0)
              {
                if ((cctrl->field_D2 == 0) && ((cctrl->byte_9B % 8) == 0))
                {
                    set_creature_instance(thing, 1, 1, 0, 0);
                }
              } else
              {
                if (cctrl->field_D2 == 0)
                {
                    setup_move_to_new_training_position(thing, room, 0);
                    cctrl->word_9F = 0;
                } else
                {
                    cctrl->byte_9B = 1;
                }
                cctrl->field_24 += (room->efficiency * crstat->training_value);
              }
            }
        } else
        {
            creature_retreat_from_combat(thing, crtng, 33, 0);
        }
        break;
    case 5:
    default:
        cctrl->byte_9B--;
        if (cctrl->byte_9B > 0)
        {
          if ((cctrl->field_D2 == 0) && ((cctrl->byte_9B % 8) == 0))
          {
              set_creature_instance(thing, 1, 1, 0, 0);
          }
        } else
        {
          if (cctrl->field_D2 == 0)
          {
              setup_move_to_new_training_position(thing, room, 1);
          } else
          {
              cctrl->byte_9B = 1;
          }
        }
        break;
    }
    SYNCDBG(18,"End");
}

short training(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    TbBool finish_training;
    struct Room *room;
    long i;
    SYNCDBG(18,"Starting");
    //return _DK_training(thing);
    dungeon = get_dungeon(thing->owner);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);

    // Check if we should finish training
    finish_training = false;
    if (!creature_can_be_trained(thing))
    {
        finish_training = true;
    }
    if (!player_can_afford_to_train_creature(thing))
    {
        if (is_my_player_number(thing->owner))
            output_message(89, 500, 1);
        finish_training = true;
    }
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room) || (room->kind != RoK_TRAINING)
     || (cctrl->work_room_id != room->index) || (room->owner != thing->owner))
    {
        finish_training = true;
    }
    if (finish_training)
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    // Pay for the training
    cctrl->field_82++;
    if (cctrl->field_82 >= game.train_cost_frequency)
    {
        cctrl->field_82 -= game.train_cost_frequency;
        if (take_money_from_dungeon(thing->owner, crstat->training_cost, 1) < 0)
            ERRORLOG("Cannot take money i am supposed to be able to afford from dungeon");
        create_price_effect(&thing->mappos, thing->owner, crstat->training_cost);
    }
    if (cctrl->field_D2 || !check_experience_upgrade(thing))
    {
        i = process_work_speed_on_work_value(thing,
            (long)room->efficiency * (long)crstat->training_value);
        cctrl->field_24 += i;
        dungeon->field_1179 += i;
        process_creature_in_training_room(thing, room);
    } else
    {
      if (external_set_thing_state(thing, 127))
        cctrl->field_282 = 50;
      dungeon->lvstats.creatures_trained++;
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
  thing->field_7 = nState;
  set_flag_byte(&thing->field_1, 0x10, false);
  thing->field_8 = CrSt_Unused;
  cctrl = creature_control_get_from_thing(thing);
  cctrl->field_302 = 0;
  clear_creature_instance(thing);
  return true;
}

TbBool remove_creature_from_work_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    struct Thing *sectng;
    struct CreatureControl *secctrl;
    //return _DK_remove_creature_from_work_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->work_room_id == 0) || ((cctrl->flgfield_1 & CCFlg_IsInRoomList) == 0))
        return false;
    room = room_get(cctrl->work_room_id);
    if (!room_is_invalid(room))
    {
        if (room->workers_in > 0)
        {
          room->workers_in--;
        } else
        {
          WARNLOG("Attempt to remove a creature from room type %d with no used space", (int)room->kind);
        }
    } else
    {
        WARNLOG("Creature had invalid room index %d",(int)cctrl->work_room_id);
        erstat_inc(ESE_BadCreatrState);
    }
    if (cctrl->prev_in_room > 0)
    {
        sectng = thing_get(cctrl->prev_in_room);
        secctrl = creature_control_get_from_thing(sectng);
        if (!creature_control_invalid(secctrl))
        {
            secctrl->next_in_room = cctrl->next_in_room;
        }
    } else
    {
        if (!room_is_invalid(room))
        {
            room->creatures_list = cctrl->next_in_room;
        }
    }
    if (cctrl->next_in_room > 0)
    {
        sectng = thing_get(cctrl->next_in_room);
        secctrl = creature_control_get_from_thing(sectng);
        if (!creature_control_invalid(secctrl))
        {
            secctrl->prev_in_room = cctrl->prev_in_room;
        }
    }
    cctrl->last_work_room_id = cctrl->work_room_id;
    cctrl->work_room_id = 0;
    cctrl->flgfield_1 &= ~CCFlg_IsInRoomList;
    cctrl->next_in_room = 0;
    cctrl->prev_in_room = 0;
    return true;
}

TbBool initialise_thing_state(struct Thing *thing, long nState)
{
    struct CreatureControl *cctrl;
    //return _DK_initialise_thing_state(thing, nState);
    cleanup_current_thing_state(thing);
    thing->field_8 = CrSt_Unused;
    thing->field_7 = nState;
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
    //return _DK_can_change_from_state_to(thing, curr_state, next_state);
    curr_stati = get_thing_state_info_num(curr_state);
    if (curr_stati->state_type == 6)
      curr_stati = get_thing_state_info_num(thing->field_8);
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
      initialise_thing_state(thing, 122);
      return thing->field_7;
    }
    if (thing->owner == game.neutral_player_num)
    {
      cleanup_current_thing_state(thing);
      initialise_thing_state(thing, 48);
      return thing->field_7;
    }
    if (thing->owner == game.hero_player_num)
    {
      i = creatures[thing->model%CREATURE_TYPES_COUNT].numfield_2;
      cleanup_current_thing_state(thing);
      initialise_thing_state(thing, i);
      return thing->field_7;
    }
    player = get_player(thing->owner);
    if (player->victory_state == 2)
    {
      cleanup_current_thing_state(thing);
      initialise_thing_state(thing, 139);
      return thing->field_7;
    }
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->field_AD & 0x02) != 0)
    {
      cleanup_current_thing_state(thing);
      initialise_thing_state(thing, 133);
      return thing->field_7;
    }
    initialise_thing_state(thing, creatures[thing->model%CREATURE_TYPES_COUNT].numfield_0);
    return thing->field_7;
}
/******************************************************************************/
