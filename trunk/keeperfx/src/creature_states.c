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
#include "creature_instances.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "config_crtrstates.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_workshop.h"
#include "room_library.h"
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
#include "creature_states_combt.h"
#include "creature_states_guard.h"
#include "creature_states_pray.h"
#include "creature_states_tresr.h"
#include "creature_states_combt.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CREATURE_GUI_STATES_COUNT 3
/* Please note that functions returning 'short' are not ment to return true/false only! */
/******************************************************************************/
DLLIMPORT short _DK_already_at_call_to_arms(struct Thing *creatng);
DLLIMPORT short _DK_arrive_at_alarm(struct Thing *creatng);
DLLIMPORT short _DK_arrive_at_call_to_arms(struct Thing *creatng);
DLLIMPORT short _DK_at_barrack_room(struct Thing *creatng);
DLLIMPORT short _DK_barracking(struct Thing *creatng);
DLLIMPORT short _DK_cleanup_hold_audience(struct Thing *creatng);
DLLIMPORT short _DK_creature_being_dropped(struct Thing *creatng);
DLLIMPORT short _DK_creature_cannot_find_anything_to_do(struct Thing *creatng);
DLLIMPORT short _DK_creature_change_from_chicken(struct Thing *creatng);
DLLIMPORT short _DK_creature_change_to_chicken(struct Thing *creatng);
DLLIMPORT short _DK_creature_doing_nothing(struct Thing *creatng);
DLLIMPORT short _DK_creature_dormant(struct Thing *creatng);
DLLIMPORT short _DK_creature_escaping_death(struct Thing *creatng);
DLLIMPORT short _DK_creature_evacuate_room(struct Thing *creatng);
DLLIMPORT short _DK_creature_explore_dungeon(struct Thing *creatng);
DLLIMPORT short _DK_creature_fired(struct Thing *creatng);
DLLIMPORT short _DK_creature_follow_leader(struct Thing *creatng);
DLLIMPORT short _DK_creature_in_hold_audience(struct Thing *creatng);
DLLIMPORT short _DK_creature_kill_creatures(struct Thing *creatng);
DLLIMPORT short _DK_creature_leaves(struct Thing *creatng);
DLLIMPORT short _DK_creature_leaves_or_dies(struct Thing *creatng);
DLLIMPORT short _DK_creature_leaving_dungeon(struct Thing *creatng);
DLLIMPORT short _DK_creature_persuade(struct Thing *creatng);
DLLIMPORT short _DK_creature_present_to_dungeon_heart(struct Thing *creatng);
DLLIMPORT short _DK_creature_pretend_chicken_move(struct Thing *creatng);
DLLIMPORT short _DK_creature_pretend_chicken_setup_move(struct Thing *creatng);
DLLIMPORT short _DK_creature_search_for_gold_to_steal_in_room(struct Thing *creatng);
DLLIMPORT short _DK_creature_set_work_room_based_on_position(struct Thing *creatng);
DLLIMPORT short _DK_creature_slap_cowers(struct Thing *creatng);
DLLIMPORT short _DK_creature_steal_gold(struct Thing *creatng);
DLLIMPORT short _DK_creature_take_salary(struct Thing *creatng);
DLLIMPORT short _DK_creature_unconscious(struct Thing *creatng);
DLLIMPORT short _DK_creature_vandalise_rooms(struct Thing *creatng);
DLLIMPORT short _DK_creature_wait_at_treasure_room_door(struct Thing *creatng);
DLLIMPORT short _DK_creature_wants_a_home(struct Thing *creatng);
DLLIMPORT short _DK_creature_wants_salary(struct Thing *creatng);
DLLIMPORT short _DK_move_backwards_to_position(struct Thing *creatng);
DLLIMPORT long _DK_move_check_attack_any_door(struct Thing *creatng);
DLLIMPORT long _DK_move_check_can_damage_wall(struct Thing *creatng);
DLLIMPORT long _DK_move_check_kill_creatures(struct Thing *creatng);
DLLIMPORT long _DK_move_check_near_dungeon_heart(struct Thing *creatng);
DLLIMPORT long _DK_move_check_on_head_for_room(struct Thing *creatng);
DLLIMPORT long _DK_move_check_persuade(struct Thing *creatng);
DLLIMPORT long _DK_move_check_wait_at_door_for_wage(struct Thing *creatng);
DLLIMPORT short _DK_move_to_position(struct Thing *creatng);
DLLIMPORT char _DK_new_slab_tunneller_check_for_breaches(struct Thing *creatng);
DLLIMPORT short _DK_patrol_here(struct Thing *creatng);
DLLIMPORT short _DK_patrolling(struct Thing *creatng);
DLLIMPORT short _DK_person_sulk_at_lair(struct Thing *creatng);
DLLIMPORT short _DK_person_sulk_head_for_lair(struct Thing *creatng);
DLLIMPORT short _DK_person_sulking(struct Thing *creatng);
DLLIMPORT short _DK_seek_the_enemy(struct Thing *creatng);
DLLIMPORT short _DK_state_cleanup_dragging_body(struct Thing *creatng);
DLLIMPORT short _DK_state_cleanup_dragging_object(struct Thing *creatng);
DLLIMPORT short _DK_state_cleanup_in_room(struct Thing *creatng);
DLLIMPORT short _DK_state_cleanup_unable_to_fight(struct Thing *creatng);
DLLIMPORT short _DK_state_cleanup_unconscious(struct Thing *creatng);
DLLIMPORT short _DK_tunneller_doing_nothing(struct Thing *creatng);
DLLIMPORT short _DK_tunnelling(struct Thing *creatng);
DLLIMPORT long _DK_setup_random_head_for_room(struct Thing *creatng, struct Room *room, unsigned char a3);
DLLIMPORT void _DK_anger_set_creature_anger(struct Thing *creatng, long a1, long a2);
DLLIMPORT void _DK_create_effect_around_thing(struct Thing *creatng, long eff_kind);
DLLIMPORT void _DK_remove_health_from_thing_and_display_health(struct Thing *creatng, long delta);
DLLIMPORT long _DK_slab_by_players_land(unsigned char plyr_idx, unsigned char slb_x, unsigned char slb_y);
DLLIMPORT struct Room *_DK_find_nearest_room_for_thing_excluding_two_types(struct Thing *creatng, char owner, char a3, char a4, unsigned char a5);
DLLIMPORT unsigned char _DK_initialise_thing_state(struct Thing *creatng, long a2);
DLLIMPORT long _DK_cleanup_current_thing_state(struct Thing *creatng);
DLLIMPORT unsigned char _DK_find_random_valid_position_for_thing_in_room_avoiding_object(struct Thing *creatng, struct Room *room, struct Coord3d *pos);
DLLIMPORT long _DK_setup_head_for_empty_treasure_space(struct Thing *creatng, struct Room *room);
DLLIMPORT short _DK_creature_choose_random_destination_on_valid_adjacent_slab(struct Thing *creatng);
DLLIMPORT long _DK_person_get_somewhere_adjacent_in_room(struct Thing *creatng, struct Room *room, struct Coord3d *pos);
DLLIMPORT unsigned char _DK_external_set_thing_state(struct Thing *thing, long state);
/******************************************************************************/
short already_at_call_to_arms(struct Thing *creatng);
short arrive_at_alarm(struct Thing *thing);
short arrive_at_call_to_arms(struct Thing *thing);
short at_barrack_room(struct Thing *thing);
short barracking(struct Thing *creatng);
short cleanup_hold_audience(struct Thing *creatng);
short creature_being_dropped(struct Thing *creatng);
short creature_cannot_find_anything_to_do(struct Thing *creatng);
short creature_change_from_chicken(struct Thing *creatng);
short creature_change_to_chicken(struct Thing *creatng);
short creature_doing_nothing(struct Thing *creatng);
short creature_dormant(struct Thing *creatng);
short creature_escaping_death(struct Thing *creatng);
short creature_evacuate_room(struct Thing *creatng);
short creature_explore_dungeon(struct Thing *creatng);
short creature_fired(struct Thing *creatng);
short creature_follow_leader(struct Thing *creatng);
short creature_in_hold_audience(struct Thing *creatng);
short creature_kill_creatures(struct Thing *creatng);
short creature_leaves(struct Thing *creatng);
short creature_leaves_or_dies(struct Thing *creatng);
short creature_leaving_dungeon(struct Thing *creatng);
short creature_persuade(struct Thing *creatng);
short creature_present_to_dungeon_heart(struct Thing *creatng);
short creature_pretend_chicken_move(struct Thing *creatng);
short creature_pretend_chicken_setup_move(struct Thing *creatng);
short creature_search_for_gold_to_steal_in_room(struct Thing *thing);
short creature_set_work_room_based_on_position(struct Thing *creatng);
short creature_slap_cowers(struct Thing *creatng);
short creature_steal_gold(struct Thing *thing);
short creature_take_salary(struct Thing *creatng);
short creature_unconscious(struct Thing *creatng);
short creature_vandalise_rooms(struct Thing *creatng);
short creature_wait_at_treasure_room_door(struct Thing *creatng);
short creature_wants_a_home(struct Thing *creatng);
short creature_wants_salary(struct Thing *creatng);
short move_backwards_to_position(struct Thing *thing);
long move_check_attack_any_door(struct Thing *creatng);
long move_check_can_damage_wall(struct Thing *creatng);
long move_check_kill_creatures(struct Thing *creatng);
long move_check_near_dungeon_heart(struct Thing *creatng);
long move_check_on_head_for_room(struct Thing *creatng);
long move_check_persuade(struct Thing *creatng);
long move_check_wait_at_door_for_wage(struct Thing *creatng);
short move_to_position(struct Thing *creatng);
char new_slab_tunneller_check_for_breaches(struct Thing *creatng);
short patrol_here(struct Thing *creatng);
short patrolling(struct Thing *creatng);
short person_sulk_at_lair(struct Thing *creatng);
short person_sulk_head_for_lair(struct Thing *creatng);
short person_sulking(struct Thing *creatng);
short seek_the_enemy(struct Thing *thing);
short state_cleanup_dragging_body(struct Thing *creatng);
short state_cleanup_dragging_object(struct Thing *creatng);
short state_cleanup_in_room(struct Thing *creatng);
short state_cleanup_unable_to_fight(struct Thing *creatng);
short state_cleanup_unconscious(struct Thing *creatng);
short tunneller_doing_nothing(struct Thing *creatng);
short tunnelling(struct Thing *creatng);
short creature_search_for_spell_to_steal_in_room(struct Thing *thing);
short creature_pick_up_spell_to_steal(struct Thing *thing);

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
  {creature_pick_up_spell_to_steal, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 55, 1, 0, 1},
  // Some redundant NULLs
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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

struct StateInfo *get_thing_state_info_num(CrtrStateId state_id)
{
    if ((state_id < 0) || (state_id >= CREATURE_STATES_COUNT))
        return &states[0];
    return &states[state_id];
}

long get_creature_state_besides_move(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    return i;
}

long get_creature_state_besides_drag(const struct Thing *thing)
{
    long i;
    i = thing->active_state;
    if (i == CrSt_MoveBackwardsToPosition)
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

short already_at_call_to_arms(struct Thing *creatng)
{
    //return _DK_already_at_call_to_arms(thing);
    internal_set_thing_state(creatng, CrSt_ArriveAtCallToArms);
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
    event_create_event_or_update_nearby_existing_event(subtile_coord_center(room->central_stl_x),
        subtile_coord_center(room->central_stl_y), 19, room->owner, 0);
    if (is_my_player_number(room->owner))
        output_message(SMsg_EnemyDestroyRooms, 400, true);
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
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
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
            if ( ((mapblk->flags & MapFlg_IsRoom) != 0) && ((mapblk->flags & MapFlg_Unkn10) != 0) )
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

short barracking(struct Thing *creatng)
{
  return _DK_barracking(creatng);
}

short cleanup_hold_audience(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    //return _DK_cleanup_hold_audience(thing);
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->max_speed = calculate_correct_creature_maxspeed(creatng);
    return 0;
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

short creature_being_dropped(struct Thing *creatng)
{
  return _DK_creature_being_dropped(creatng);
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

short creature_cannot_find_anything_to_do(struct Thing *creatng)
{
  return _DK_creature_cannot_find_anything_to_do(creatng);
}

short creature_change_from_chicken(struct Thing *creatng)
{
  return _DK_creature_change_from_chicken(creatng);
}

short creature_change_to_chicken(struct Thing *creatng)
{
  return _DK_creature_change_to_chicken(creatng);
}

short creature_doing_nothing(struct Thing *creatng)
{
  return _DK_creature_doing_nothing(creatng);
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
    dragtng->alloc_flags &= ~TAlF_IsDragged;
    dragtng->field_1 &= ~0x01;
    move_thing_in_map(dragtng, &crtng->mappos);
    if (dragtng->light_id != 0) {
        light_turn_light_on(dragtng->light_id);
    }
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

short creature_dormant(struct Thing *creatng)
{
    //return _DK_creature_dormant(thing);
    if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
    {
      creatng->continue_state = CrSt_CreatureDormant;
      return 1;
    }
    return 0;
}

short creature_escaping_death(struct Thing *creatng)
{
  return _DK_creature_escaping_death(creatng);
}

short creature_evacuate_room(struct Thing *creatng)
{
  return _DK_creature_evacuate_room(creatng);
}

short creature_explore_dungeon(struct Thing *creatng)
{
  return _DK_creature_explore_dungeon(creatng);
}

short creature_fired(struct Thing *creatng)
{
  return _DK_creature_fired(creatng);
}

short creature_follow_leader(struct Thing *creatng)
{
  return _DK_creature_follow_leader(creatng);
}

short creature_in_hold_audience(struct Thing *creatng)
{
  return _DK_creature_in_hold_audience(creatng);
}

short creature_kill_creatures(struct Thing *creatng)
{
  return _DK_creature_kill_creatures(creatng);
}

short creature_leaves(struct Thing *creatng)
{
  return _DK_creature_leaves(creatng);
}

short creature_leaves_or_dies(struct Thing *creatng)
{
  return _DK_creature_leaves_or_dies(creatng);
}

short creature_leaving_dungeon(struct Thing *creatng)
{
  return _DK_creature_leaving_dungeon(creatng);
}

short creature_persuade(struct Thing *creatng)
{
  return _DK_creature_persuade(creatng);
}

void creature_drag_object(struct Thing *thing, struct Thing *dragtng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_6E = dragtng->index;
    dragtng->alloc_flags |= TAlF_IsDragged;
    dragtng->field_1 |= 0x01;
    dragtng->owner = game.neutral_player_num;
    if (dragtng->light_id != 0) {
      light_turn_light_off(dragtng->light_id);
    }
}

unsigned char find_random_valid_position_for_thing_in_room_avoiding_object(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_find_random_valid_position_for_thing_in_room_avoiding_object(thing, room, pos);
}

short creature_present_to_dungeon_heart(struct Thing *creatng)
{
  return _DK_creature_present_to_dungeon_heart(creatng);
}

short creature_pretend_chicken_move(struct Thing *creatng)
{
  return _DK_creature_pretend_chicken_move(creatng);
}

short creature_pretend_chicken_setup_move(struct Thing *creatng)
{
  return _DK_creature_pretend_chicken_setup_move(creatng);
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
        tmptng = find_gold_hoard_at(3*slb_x+1, 3*slb_y+1);
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
    struct CreatureControl *cctrl;
    struct SlabMap *slb;
    struct Room *room;
    struct Thing *spltng;
    cctrl = creature_control_get_from_thing(thing);
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
    cctrl->pickup_object_id = spltng->index;
    if (!setup_person_move_to_position(thing, spltng->mappos.x.stl.num, spltng->mappos.y.stl.num, 0))
    {
        SYNCDBG(8,"Cannot move to spell at (%d,%d)",(int)spltng->mappos.x.stl.num, (int)spltng->mappos.y.stl.num);
    }
    thing->continue_state = CrSt_CreatureStealSpell;
    return 1;
}

short creature_set_work_room_based_on_position(struct Thing *creatng)
{
    //return _DK_creature_set_work_room_based_on_position(thing);
    return 1;
}

short creature_slap_cowers(struct Thing *creatng)
{
  return _DK_creature_slap_cowers(creatng);
}

short creature_steal_gold(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct Room *room;
    struct Thing *hrdtng;
    long max_amount,amount;
    //return _DK_creature_steal_gold(thing);
    crstat = creature_stats_get_from_thing(thing);
    room = subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (room_is_invalid(room) || (room->kind != RoK_TREASURE))
    {
        WARNLOG("Cannot steal gold - not on treasure room at (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    hrdtng = find_gold_hoard_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (thing_is_invalid(hrdtng))
    {
        WARNLOG("Cannot steal gold - no gold hoard at (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
        set_start_state(thing);
        return 0;
    }
    max_amount = crstat->gold_hold - thing->creature.gold_carried;
    if (max_amount <= 0)
    {
        set_start_state(thing);
        return 0;
    }
    // Success! we are able to steal some gold!
    amount = remove_gold_from_hoarde(hrdtng, room, max_amount);
    thing->creature.gold_carried += amount;
    create_price_effect(&thing->mappos, thing->owner, amount);
    SYNCDBG(6,"Stolen %ld gold from hoarde at (%d,%d)",amount,(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
    set_start_state(thing);
    return 0;
}

short creature_pick_up_spell_to_steal(struct Thing *thing)
{
    struct Room *room;
    struct CreatureControl *cctrl;
    struct Thing *spelltng;
    struct Coord3d pos;
    cctrl = creature_control_get_from_thing(thing);
    spelltng = thing_get(cctrl->pickup_object_id);
    if ( thing_is_invalid(spelltng) || ((spelltng->field_1 & TF1_Unkn01) != 0)
      || (get_2d_box_distance(&thing->mappos, &spelltng->mappos) >= 512))
    {
        set_start_state(thing);
        return 0;
    }
    room = subtile_room_get(spelltng->mappos.x.stl.num,spelltng->mappos.y.stl.num);
    // Check if we're stealing the spell from a library
    if (!room_is_invalid(room))
    {
        remove_spell_from_library(room, spelltng, thing->owner);
    }
    pos.x.val = 0;
    pos.y.val = 0;
    //TODO STEAL_SPELLS write the spell stealing code
    SYNCLOG("Stealing spells not implemented - reset");
    set_start_state(thing);
    return 0;
/*
    creature_drag_object(thing, spelltng);
    if (!setup_person_move_to_position(thing, pos.x.stl.num, pos.y.stl.num, 0))
    {
        SYNCDBG(8,"Cannot move to (%d,%d)",(int)pos.x.stl.num, (int)pos.y.stl.num);
    }
    thing->continue_state = CrSt_GoodReturnsToStart;
    return 1;
*/
}

short creature_take_salary(struct Thing *creatng)
{
  return _DK_creature_take_salary(creatng);
}

short creature_unconscious(struct Thing *creatng)
{
  return _DK_creature_unconscious(creatng);
}

short creature_vandalise_rooms(struct Thing *creatng)
{
  return _DK_creature_vandalise_rooms(creatng);
}

short creature_wait_at_treasure_room_door(struct Thing *creatng)
{
  return _DK_creature_wait_at_treasure_room_door(creatng);
}

short creature_wants_a_home(struct Thing *creatng)
{
  return _DK_creature_wants_a_home(creatng);
}

short creature_wants_salary(struct Thing *creatng)
{
  return _DK_creature_wants_salary(creatng);
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
    thing->field_1 &= ~0x02;
    thing->field_4F &= ~0x01;
    place_thing_in_mapwho(thing);
}

short move_backwards_to_position(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long i,speed;
    //return _DK_move_backwards_to_position(thing);
    cctrl = creature_control_get_from_thing(thing);
    speed = get_creature_speed(thing);
    i = creature_move_to_using_gates(thing, &cctrl->moveto_pos, speed, -2, cctrl->field_88, 1);
    if (i == 1)
    {
        internal_set_thing_state(thing, thing->continue_state);
        thing->continue_state = 0;
        return 1;
    }
    if (i == -1)
    {
        ERRORLOG("Bad place (%d,%d) to move %s backwards to.",(int)cctrl->moveto_pos.x.val,(int)cctrl->moveto_pos.y.val,thing_model_name(thing));
        set_start_state(thing);
        thing->continue_state = 0;
        return 0;
    }
    return 0;
}

long move_check_attack_any_door(struct Thing *creatng)
{
  return _DK_move_check_attack_any_door(creatng);
}

long move_check_can_damage_wall(struct Thing *creatng)
{
  return _DK_move_check_can_damage_wall(creatng);
}

long creature_can_have_combat_with_creature_on_slab(const struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y, struct Thing ** enemytng)
{
    struct Map *map;
    long endstl_x,endstl_y;
    long stl_x,stl_y;
    long dist;
    endstl_x = 3*slb_x+3;
    endstl_y = 3*slb_y+3;
    for (stl_y = 3*slb_y; stl_y < endstl_y; stl_y++)
    {
        for (stl_x = 3*slb_x; stl_x < endstl_x; stl_x++)
        {
            struct Thing *thing;
            long can_combat;
            long i;
            unsigned long k;
            map = get_map_block_at(stl_x,stl_y);
            k = 0;
            i = get_mapwho_thing_index(map);
            while (i != 0)
            {
                thing = thing_get(i);
                if (thing_is_invalid(thing))
                {
                    ERRORLOG("Jump to invalid thing detected");
                    break;
                }
                i = thing->field_2;
                // Per thing code start
                if ( thing_is_creature(thing) && (thing != creatng) )
                {
                    if ((get_creature_model_flags(thing) & MF_IsSpecDigger) == 0)
                    {
                        dist = get_combat_distance(creatng, thing);
                        can_combat = creature_can_have_combat_with_creature(creatng, thing, dist, 0, 0);
                        if (can_combat > 0) {
                            (*enemytng) = thing;
                            return can_combat;
                        }
                    }
                }
                // Per thing code end
                k++;
                if (k > THINGS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping things list");
                    break;
                }
            }
        }
    }
    (*enemytng) = INVALID_THING;
    return 0;
}

long move_check_kill_creatures(struct Thing *creatng)
{
    struct Thing * enemytng;
    MapSlabCoord slb_x,slb_y;
    long can_combat;
    //return _DK_move_check_kill_creatures(thing);
    slb_x = creatng->mappos.x.stl.num/3;
    slb_y = creatng->mappos.y.stl.num/3;
    can_combat = creature_can_have_combat_with_creature_on_slab(creatng, slb_x, slb_y, &enemytng);
    if (can_combat > 0) {
        set_creature_in_combat_to_the_death(creatng, enemytng, can_combat);
        return 1;
    }
    return 0;
}

long move_check_near_dungeon_heart(struct Thing *creatng)
{
  return _DK_move_check_near_dungeon_heart(creatng);
}

long move_check_on_head_for_room(struct Thing *creatng)
{
  return _DK_move_check_on_head_for_room(creatng);
}

long move_check_persuade(struct Thing *creatng)
{
  return _DK_move_check_persuade(creatng);
}

long move_check_wait_at_door_for_wage(struct Thing *creatng)
{
  return _DK_move_check_wait_at_door_for_wage(creatng);
}

char new_slab_tunneller_check_for_breaches(struct Thing *creatng)
{
  return _DK_new_slab_tunneller_check_for_breaches(creatng);
}

short patrol_here(struct Thing *creatng)
{
  return _DK_patrol_here(creatng);
}

short patrolling(struct Thing *creatng)
{
  return _DK_patrolling(creatng);
}

short person_sulk_at_lair(struct Thing *creatng)
{
  return _DK_person_sulk_at_lair(creatng);
}

short person_sulk_head_for_lair(struct Thing *creatng)
{
  return _DK_person_sulk_head_for_lair(creatng);
}

short person_sulking(struct Thing *creatng)
{
  return _DK_person_sulking(creatng);
}

long room_still_valid_as_type_for_thing(struct Room *room, RoomKind rkind, struct Thing *thing)
{
    if (!room_exists(room))
        return false;
    return (room->kind == rkind);
}

void create_effect_around_thing(struct Thing *thing, long eff_kind)
{
  _DK_create_effect_around_thing(thing, eff_kind);
}

void remove_health_from_thing_and_display_health(struct Thing *thing, long delta)
{
  _DK_remove_health_from_thing_and_display_health(thing, delta);
}

long slab_by_players_land(long plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
  return _DK_slab_by_players_land(plyr_idx, slb_x, slb_y);
}

TbBool process_creature_hunger(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  if ( (crstat->hunger_rate == 0) || ((cctrl->affected_by_spells & CCSpl_Freeze) != 0) )
    return false;
  cctrl->field_39++;
  if (cctrl->field_39 <= crstat->hunger_rate)
    return false;
  if ((game.play_gameturn % game.turns_per_hunger_health_loss) == 0)
    remove_health_from_thing_and_display_health(thing, game.hunger_health_loss);
  return true;
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

    tmptng = thing_get(cctrl1->battle_enemy_idx);
    if  ( (cctrl1->field_AD & 0x10) || (cctrl2->field_AD & 0x10)
        || ((cctrl1->combat_flags) && (tmptng == tng2)) )
    {
        if (tng2 != tng1)
        {
            if ((creature_control_exists(cctrl2)) && ((cctrl2->flgfield_1 & CCFlg_NoCompControl) == 0)
            && ((tng2->alloc_flags & TAlF_IsInLimbo) == 0) && ((tng2->field_1 & TF1_Unkn02) == 0))
            {
                crstat1 = creature_stats_get_from_thing(tng1);
                if ((cctrl2->spell_flags & CSAfF_Invisibility) == 0)
                    return true;
                if (cctrl2->field_AF > 0)
                    return true;
                if (crstat1->can_see_invisible)
                    return true;
                if ((cctrl1->spell_flags & CSAfF_Sight) != 0)
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
        if (((enemytng->alloc_flags & TAlF_Exists) == 0) || (cctrl->long_9C != enemytng->field_9))
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
            if (cctrl->instance_id == CrInst_NULL)
            {
              if ((dist < 2304) && (game.play_gameturn-cctrl->field_282 < 20))
              {
                set_creature_instance(thing, CrInst_CELEBRATE_SHORT, 1, 0, 0);
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

short state_cleanup_dragging_body(struct Thing *creatng)
{
  return _DK_state_cleanup_dragging_body(creatng);
}

short state_cleanup_dragging_object(struct Thing *creatng)
{
  return _DK_state_cleanup_dragging_object(creatng);
}

short state_cleanup_in_room(struct Thing *creatng)
{
  return _DK_state_cleanup_in_room(creatng);
}

short state_cleanup_unable_to_fight(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    //return _DK_state_cleanup_unable_to_fight(thing);
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
    return 1;
}

short state_cleanup_unconscious(struct Thing *creatng)
{
  return _DK_state_cleanup_unconscious(creatng);
}

long process_work_speed_on_work_value(struct Thing *thing, long base_val)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    long val;
    cctrl = creature_control_get_from_thing(thing);
    val = base_val;
    if ((cctrl->spell_flags & CSAfF_Speed) != 0)
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
    if (cctrl->exp_points < i)
        return false;
    cctrl->exp_points -= i;
    if (cctrl->explevel < dungeon->creature_max_level[thing->model])
    {
      if ((cctrl->explevel < CREATURE_MAX_LEVEL-1) || (crstat->grow_up != 0))
        cctrl->field_AD |= 0x40;
    }
    return true;
}

short tunneller_doing_nothing(struct Thing *creatng)
{
  return _DK_tunneller_doing_nothing(creatng);
}

short tunnelling(struct Thing *creatng)
{
  return _DK_tunnelling(creatng);
}
/******************************************************************************/
TbBool internal_set_thing_state(struct Thing *thing, CrtrStateId nState)
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

TbBool initialise_thing_state(struct Thing *thing, CrtrStateId nState)
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
        ERRORLOG("The %s index %d has invalid control",thing_model_name(thing),(int)thing->index);
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
    if (stati->cleanup_state != NULL)
    {
        stati->cleanup_state(thing);
        set_flag_byte(&thing->field_1, 0x10, true);
    } else
    {
        clear_creature_instance(thing);
    }
    return true;
}

TbBool can_change_from_state_to(struct Thing *thing, CrtrStateId curr_state, CrtrStateId next_state)
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

short set_start_state_f(struct Thing *thing,const char *func_name)
{
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    long i;
    SYNCDBG(8,"%s: Starting for %s index %d, owner %d, last state %s",func_name,thing_model_name(thing),(int)thing->index,(int)thing->owner,creature_state_code_name(thing->active_state));
//    return _DK_set_start_state(thing);
    if ((thing->alloc_flags & 0x20) != 0)
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
        i = creatures[thing->model%CREATURE_TYPES_COUNT].good_start_state;
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, i);
        return thing->active_state;
    }
    player = get_player(thing->owner);
    if (player->victory_state == VicS_LostLevel)
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_LeavesBecauseOwnerLost);
        return thing->active_state;
    }
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->field_AD & 0x02) != 0)
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_CreaturePretendChickenSetupMove);
        return thing->active_state;
    }
    i = creatures[thing->model%CREATURE_TYPES_COUNT].evil_start_state;
    initialise_thing_state(thing, i);
    return thing->active_state;
}

TbBool external_set_thing_state(struct Thing *thing, CrtrStateId state)
{
    struct CreatureControl *cctrl;
    struct StateInfo *stati;
    CreatureStateFunc1 callback;
    //return _DK_external_set_thing_state(thing, state);
    if ( !can_change_from_state_to(thing, thing->active_state, state) )
    {
        ERRORDBG(4,"State change %s to %s for %s not allowed",creature_state_code_name(thing->active_state), creature_state_code_name(state), thing_model_name(thing));
        return false;
    }
    SYNCDBG(9,"State change %s to %s for %s index %d",creature_state_code_name(thing->active_state), creature_state_code_name(state), thing_model_name(thing),(int)thing->index);
    stati = get_thing_active_state_info(thing);
    if (stati->state_type == CrStTyp_Value6)
        stati = get_thing_continue_state_info(thing);
    callback = stati->cleanup_state;
    if (callback != NULL) {
        callback(thing);
        thing->field_1 |= 0x10;
    } else {
        clear_creature_instance(thing);
    }
    thing->active_state = state;
    thing->field_1 &= ~0x10;
    thing->continue_state = 0;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    cctrl->field_302 = 0;
    if ((cctrl->flgfield_1 & 0x20) != 0)
    {
        ERRORLOG("External change state %s to %s, but %s in room list even after cleanup",creature_state_code_name(thing->active_state), creature_state_code_name(state), thing_model_name(thing));
        remove_creature_from_work_room(thing);
    }
    return true;
}

/******************************************************************************/
