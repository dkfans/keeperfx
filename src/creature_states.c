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
#include "creature_graphics.h"
#include "creature_jobs.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "config_effects.h"
#include "config_crtrstates.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_corpses.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_workshop.h"
#include "room_library.h"
#include "room_lair.h"
#include "room_util.h"
#include "room_list.h"
#include "tasks_list.h"
#include "map_events.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "power_hand.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "engine_arrays.h"
#include "player_utils.h"
#include "player_instances.h"
#include "player_computer.h"
#include "thing_traps.h"
#include "magic.h"
#include "sounds.h"
#include "game_legacy.h"

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
#include "creature_states_barck.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CREATURE_GUI_STATES_COUNT 3
/* Please note that functions returning 'short' are not ment to return true/false only! */
/******************************************************************************/
DLLIMPORT short _DK_creature_pretend_chicken_setup_move(struct Thing *creatng);
DLLIMPORT long _DK_move_check_can_damage_wall(struct Thing *creatng);
DLLIMPORT long _DK_move_check_on_head_for_room(struct Thing *creatng);
DLLIMPORT long _DK_move_check_persuade(struct Thing *creatng);
DLLIMPORT long _DK_get_best_position_outside_room(struct Thing *creatng, struct Coord3d *pos, struct Room *room);
/******************************************************************************/
short already_at_call_to_arms(struct Thing *creatng);
short arrive_at_alarm(struct Thing *creatng);
short arrive_at_call_to_arms(struct Thing *creatng);
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
short creature_exempt(struct Thing *creatng);
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
short creature_search_for_gold_to_steal_in_room(struct Thing *creatng);
short creature_set_work_room_based_on_position(struct Thing *creatng);
short creature_slap_cowers(struct Thing *creatng);
short creature_steal_gold(struct Thing *creatng);
short creature_take_salary(struct Thing *creatng);
short creature_unconscious(struct Thing *creatng);
short creature_vandalise_rooms(struct Thing *creatng);
short creature_wait_at_treasure_room_door(struct Thing *creatng);
short creature_wants_a_home(struct Thing *creatng);
short creature_wants_salary(struct Thing *creatng);
short move_backwards_to_position(struct Thing *creatng);
CrCheckRet move_check_attack_any_door(struct Thing *creatng);
CrCheckRet move_check_can_damage_wall(struct Thing *creatng);
CrCheckRet move_check_kill_creatures(struct Thing *creatng);
CrCheckRet move_check_near_dungeon_heart(struct Thing *creatng);
CrCheckRet move_check_on_head_for_room(struct Thing *creatng);
CrCheckRet move_check_persuade(struct Thing *creatng);
CrCheckRet move_check_wait_at_door_for_wage(struct Thing *creatng);

short move_to_position(struct Thing *creatng);
char new_slab_tunneller_check_for_breaches(struct Thing *creatng);
short patrol_here(struct Thing *creatng);
short patrolling(struct Thing *creatng);
short person_sulk_at_lair(struct Thing *creatng);
short person_sulk_head_for_lair(struct Thing *creatng);
short person_sulking(struct Thing *creatng);
short seek_the_enemy(struct Thing *creatng);
short state_cleanup_dragging_body(struct Thing *creatng);
short state_cleanup_dragging_object(struct Thing *creatng);
short state_cleanup_in_room(struct Thing *creatng);
short state_cleanup_unable_to_fight(struct Thing *creatng);
short state_cleanup_unconscious(struct Thing *creatng);
short state_cleanup_wait_at_door(struct Thing* creatng);
short creature_search_for_spell_to_steal_in_room(struct Thing *creatng);
short creature_pick_up_spell_to_steal(struct Thing *creatng);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
//process_state, cleanup_state, move_from_slab, move_check, 
//override_feed, override_own_needs, override_sleep, override_fight_crtr, override_gets_salary, override_prev_fld1F, override_prev_fld20, override_escape, override_unconscious, override_anger_job, override_fight_object, override_fight_door, override_call2arms, override_follow,
    //state_type, field_1F, field_20, field_21, field_23, sprite_idx, field_26, field_27, react_to_cta
struct StateInfo states[] = {
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 0},
  {imp_doing_nothing, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_arrives_at_dig_or_mine, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_arrives_at_dig_or_mine, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_digs_mines, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_digs_mines, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Move, 0, 0, 1, 0,  0, 0, 0, 1},
  {imp_drops_gold, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_last_did_job, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {imp_arrives_at_improve_dungeon, NULL, NULL,  NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_improves_dungeon, NULL, NULL, NULL, // [10]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_picks_up_trap_object, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_arms_trap, state_cleanup_dragging_object, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_picks_up_crate_for_workshop, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {move_to_position, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Move, 0, 0, 1, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Move, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_drops_crate_in_workshop, state_cleanup_dragging_object, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_doing_nothing, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_to_garden, NULL, NULL, NULL,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Feed, 0, 0, 1, 0, 59, 1, 0, 1},
  {creature_arrived_at_garden, state_cleanup_in_room, NULL, move_check_on_head_for_room,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Feed, 0, 0, 2, 0, 59, 1, 0, 1},
  {creature_wants_a_home, NULL, NULL, NULL, // [20]
    0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0, 58, 1, 0, 1},
  {creature_choose_room_for_lair_site, NULL, NULL, NULL,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0, 58, 1, 0, 1},
  {creature_at_new_lair, NULL, NULL, NULL,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0, 58, 1, 0, 1},
  {person_sulk_head_for_lair, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0, 55, 1, 0, 1},
  {person_sulk_at_lair, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0, 55, 1, 0, 1},
  {creature_going_home_to_sleep, NULL, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Sleep, 0, 0, 1, 0, 54, 1, 0, 1},
  {creature_sleep, cleanup_sleep, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Sleep, 0, 0, 2, 0, 54, 1, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {tunnelling, NULL, new_slab_tunneller_check_for_breaches, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {at_research_room, NULL, NULL, move_check_on_head_for_room, // [30]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0, 56, 1, 0, 1},
  {researching, state_cleanup_in_room, NULL, process_research_function,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 2, 0, 56, 1, 0, 1},
  {at_training_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0, 57, 1, 0, 1},
  {training, state_cleanup_in_room, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 2, 0, 57, 1, 0, 1},
  {good_doing_nothing, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {good_returns_to_start, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {good_back_at_start, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {good_drops_gold, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Work, 0, 1, 1, 0,  0, 0, 0, 0},
  {arrive_at_call_to_arms, NULL, NULL, NULL,
    1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_Called2Arms, 0, 0, 1, 0, 62, 1, 0, 0},
  {creature_arrived_at_prison, state_cleanup_unable_to_fight, NULL, move_check_on_head_for_room, // [40]
    1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1,  CrStTyp_OwnNeeds, 1, 0, 0, 0, 66, 1, 0, 0},
  {creature_in_prison, cleanup_prison, NULL, process_prison_function,
    1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1,  CrStTyp_OwnNeeds, 1, 0, 0, 0, 66, 1, 0, 0},
  {at_torture_room, state_cleanup_unable_to_fight, NULL, move_check_on_head_for_room,
    1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1,  CrStTyp_OwnNeeds, 1, 0, 0, 0, 65, 1, 0, 0},
  {torturing, cleanup_torturing, NULL, process_torture_function,
    1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1,  CrStTyp_OwnNeeds, 1, 0, 0, 0, 65, 1, 0, 0},
  {at_workshop_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0, 67, 1, 0, 1},
  {manufacturing, state_cleanup_in_room, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 2, 0, 67, 1, 0, 1},
  {at_scavenger_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0, 69, 1, 0, 1},
  {scavengering, state_cleanup_in_room, NULL, process_scavenge_function,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 2, 0, 69, 1, 0, 1},
  {creature_dormant, NULL, NULL, move_check_near_dungeon_heart,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_in_combat, cleanup_combat, NULL, NULL,
    1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1,  CrStTyp_FightCrtr, 0, 0, 1, 1, 51, 1, 0, 0},
  {creature_leaving_dungeon, NULL, NULL, NULL, // [50]
    0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 1, 1, 1, 0, 61, 1, 0, 1},
  {creature_leaves, NULL, NULL, NULL,
    0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 1, 1, 1, 0, 61, 1, 0, 1},
  {creature_in_hold_audience, cleanup_hold_audience, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {patrol_here, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {patrolling, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 0},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_kill_creatures, NULL, NULL, move_check_kill_creatures,
    0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 1, 0, 61, 1, 0, 1},
  {NULL, NULL, NULL, NULL, // [60]
    1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {person_sulking, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0, 55, 1, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {at_barrack_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0, 63, 1, 0, 1},
  {barracking, state_cleanup_in_room, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0, 63, 1, 0, 1},
  {creature_slap_cowers, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Work, 0, 1, 0, 0,  0, 0, 0, 1},
  {creature_unconscious, state_cleanup_unconscious, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Unconscious, 0, 0, 0, 1,  0, 0, 0, 1},
  {creature_pick_up_unconscious_body, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {imp_toking, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_picks_up_gold_pile, NULL, NULL, NULL, // [70]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {move_backwards_to_position, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Move, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_drop_body_in_prison, state_cleanup_dragging_body, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {imp_arrives_at_convert_dungeon, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_converts_dungeon, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_wants_salary, NULL, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_GetsSalary, 0, 0, 1, 0, 52, 1, 0, 1},
  {creature_take_salary, NULL, NULL, move_check_wait_at_door_for_wage,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_GetsSalary, 0, 0, 1, 0, 52, 1, 0, 1},
  {tunneller_doing_nothing, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_object_combat, cleanup_object_combat, NULL, NULL,
    1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, CrStTyp_FightObj, 0, 0, 1, 0, 51, 1, 0, 0},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_change_lair, NULL, NULL, move_check_on_head_for_room, // [80]
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0, 58, 1, 0, 1},
  {imp_birth, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {at_temple, NULL, NULL, move_check_on_head_for_room,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0, 68, 1, 0, 1},
  {praying_in_temple, state_cleanup_in_temple, NULL, process_temple_function,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 2, 0, 68, 1, 0, 1},
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_follow_leader, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_Follow, 0, 0, 0, 0,  0, 0, 0, 0},
  {creature_door_combat, cleanup_door_combat, NULL, NULL,
    1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, CrStTyp_FightDoor, 0, 0, 1, 0, 51, 1, 0, 0},
  {creature_combat_flee, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1,  CrStTyp_Idle, 0, 0, 0, 0, 53, 1, 0, 0},
  {creature_sacrifice, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 1, 1, 0, 0,  0, 0, 0, 0},
  {at_lair_to_sleep, NULL, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Sleep, 0, 0, 2, 0, 54, 1, 0, 1},
  {creature_exempt, NULL, NULL, NULL, // [90]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Escape, 0, 0, 0, 0, 61, 1, 0, 1},
  {creature_being_dropped, state_cleanup_unable_to_fight, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 1},
  {creature_being_sacrificed, cleanup_sacrifice, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_scavenged_disappear, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_scavenged_reappear, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_being_summoned, cleanup_sacrifice, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_hero_entering, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 1},
  {imp_arrives_at_reinforce, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {imp_reinforces, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 0, 0,  0, 0, 0, 1},
  {arrive_at_alarm, NULL, NULL, NULL,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0, 62, 1, 0, 0},
  {creature_picks_up_spell_object, NULL, NULL, NULL, // [100]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Move, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_drops_spell_object_in_library, state_cleanup_dragging_object, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_picks_up_corpse, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_drops_corpse_in_graveyard, state_cleanup_dragging_object, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0,  0, 0, 0, 1},
  {at_guard_post_room, NULL, NULL, move_check_on_head_for_room,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 1, 0, 50, 1, 0, 0},
  {guarding, state_cleanup_in_room, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Work, 0, 0, 2, 0, 50, 1, 0, 0},
  {creature_eat, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 1, 1, 1, 0, 59, 1, 0, 1},
  {creature_evacuate_room, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_wait_at_treasure_room_door,  state_cleanup_wait_at_door, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_GetsSalary, 0, 0,  1, 0, 0, 0, 0, 1},
  {at_kinky_torture_room, NULL, NULL, move_check_on_head_for_room,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {kinky_torturing, cleanup_torturing, NULL, process_kinky_function, /// [110]
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {mad_killing_psycho, NULL, NULL, NULL,
    0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 55, 1, 0, 1},
  {creature_search_for_gold_to_steal_in_room, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 55, 1, 0, 1},
  {creature_vandalise_rooms, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 55, 1, 0, 1},
  {creature_steal_gold, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 55, 1, 0, 1},
  {seek_the_enemy, cleanup_seek_the_enemy, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {already_at_call_to_arms, NULL, NULL, NULL,
    1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_Called2Arms, 0, 0, 1, 0, 62, 1, 0, 0},
  {creature_damage_walls, NULL, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_attempt_to_damage_walls, NULL, NULL, move_check_can_damage_wall,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 55, 1, 0, 1},
  {creature_persuade, NULL, NULL, move_check_persuade,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 1, 0, 55, 1, 0, 1},
  {creature_change_to_chicken, NULL, NULL, NULL, // [120]
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_change_from_chicken, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 0},
  {NULL, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Work, 1, 1, 0, 0,  0, 0, 0, 1},
  {creature_cannot_find_anything_to_do, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_piss, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_roar, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 1, 0, 55, 1, 0, 1},
  {creature_at_changed_lair, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_be_happy, NULL, NULL, NULL,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {good_leave_through_exit_door, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 1, 1, 1, 0,  0, 0, 0, 1},
  {good_wait_in_exit_door, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 1},
  {good_attack_room, NULL, NULL, NULL, // [130]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 1},
  {creature_search_for_gold_to_steal_in_room, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 1, 1, 0, 0,  0, 0, 0, 1},
  {good_attack_room, NULL, NULL, NULL,
    0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 1},
  {creature_pretend_chicken_setup_move, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_pretend_chicken_move, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Idle, 1, 1, 0, 0,  0, 0, 0, 0},
  {creature_attack_rooms, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 51, 1, 0, 0},
  {creature_freeze_prisonors, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_explore_dungeon, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_eating_at_garden, NULL, NULL, NULL,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Feed, 0, 0, 2, 0, 59, 1, 0, 1},
  {creature_leaves_or_dies, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1,  CrStTyp_OwnNeeds, 1, 1, 1, 0, 61, 1, 0, 0},
  {creature_moan, NULL, NULL, NULL, // [140]
    1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 1, 0,  0, 0, 0, 1},
  {creature_set_work_room_based_on_position, NULL, NULL, NULL,
    1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  CrStTyp_Work, 1, 1,  0, 0, 0, 0, 0, 1},
  {creature_being_scavenged, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_OwnNeeds, 0, 0, 0, 0, 69, 1, 0, 0},
  {creature_escaping_death, NULL, NULL, NULL,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1,  CrStTyp_Escape, 1, 0, 0, 0,  0, 1, 0, 0},
  {creature_present_to_dungeon_heart, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 1},
  {creature_search_for_spell_to_steal_in_room, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 55, 1, 0, 1},
  {creature_pick_up_spell_to_steal, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 55, 1, 0, 1},
  {good_arrived_at_attack_room, NULL, NULL, move_check_attack_any_door,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CrStTyp_AngerJob, 0, 0, 0, 0, 55, 1, 0, 1},
  {creature_going_to_safety_for_toking, NULL, NULL, NULL,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  CrStTyp_Sleep, 0, 0, 1, 0, 54, 1, 0, 1},
  // Some redundant NULLs
  {NULL, NULL, NULL, NULL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  CrStTyp_Idle, 0, 0, 0, 0,  0, 0, 0, 0},
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
    if (state_id >= CREATURE_STATES_COUNT)
        return &states[0];
    return &states[state_id];
}

CrtrStateId get_creature_state_besides_interruptions(const struct Thing *thing)
{
    CrtrStateId i = thing->active_state;
    if ((i == CrSt_MoveToPosition) || (i == CrSt_MoveBackwardsToPosition))
        i = thing->continue_state;
    if (i == CrSt_CreatureSlapCowers)
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->active_state_bkp;
        if ((i == CrSt_MoveToPosition) || (i == CrSt_MoveBackwardsToPosition))
            i = cctrl->continue_state_bkp;
    }
    return i;
}

CrtrStateId get_creature_state_besides_move(const struct Thing *thing)
{
    long i = thing->active_state;
    if (i == CrSt_MoveToPosition)
        i = thing->continue_state;
    return i;
}

CrtrStateId get_creature_state_besides_drag(const struct Thing *thing)
{
    long i = thing->active_state;
    if (i == CrSt_MoveBackwardsToPosition)
        i = thing->continue_state;
    return i;
}

struct StateInfo *get_creature_state_with_task_completion(struct Thing *thing)
{
    struct StateInfo* stati = get_thing_active_state_info(thing);
    if (stati->state_type == CrStTyp_Move)
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
    struct CreatureStats* crstat = creature_stats_get(crmodel);
    if (censorship_enabled())
    {
        // If censorship is on, only evil creatures can have blood
        if (!crstat->bleeds)
            return false;
        struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[crmodel];
        return ((crconf->model_flags & CMF_IsEvil) != 0);
  }
  return crstat->bleeds;
}
/******************************************************************************/
/** Returns type of given creature state.
 *
 * @param thing The source thing.
 * @return Type of the creature state.
 */
long get_creature_state_type_f(const struct Thing *thing, const char *func_name)
{
  long state_type;
  long state = thing->active_state;
  if ( (state > 0) && (state < sizeof(states)/sizeof(states[0])) )
  {
      state_type = states[state].state_type;
  } else
  {
      state_type = states[0].state_type;
      WARNLOG("%s: The %s index %d active state %d is out of range",func_name,thing_model_name(thing),(int)thing->index,(int)state);
  }
  if (state_type == CrStTyp_Move)
  {
      state = thing->continue_state;
      if ( (state > 0) && (state < sizeof(states)/sizeof(states[0])) )
      {
          state_type = states[state].state_type;
      } else
      {
          state_type = states[0].state_type;
          // Show message with text name of active state - it's good as the state was checked before
          WARNLOG("%s: The %s index %d owner %d continue state %d is out of range; active state %s",func_name,
              thing_model_name(thing),(int)thing->index,(int)thing->owner,(int)state,creature_state_code_name(thing->active_state));
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
    long state_type = get_creature_state_type(thing);
    if ( (state_type >= 0) && (state_type < sizeof(state_type_to_gui_state)/sizeof(state_type_to_gui_state[0])) )
    {
        return state_type_to_gui_state[state_type];
    } else
    {
        WARNLOG("The %s index %d has invalid state type(%d)!",thing_model_name(thing),(int)thing->index,(int)state_type);
        erstat_inc(ESE_BadCreatrState);
        return state_type_to_gui_state[0];
    }
}

TbBool creature_is_dying(const struct Thing *thing)
{
    return (thing->health < 0);
}
TbBool creature_is_being_dropped(const struct Thing *thing)
{
    CrtrStateId i = thing->active_state;
    if ((i == CrSt_MoveToPosition) || (i == CrSt_MoveBackwardsToPosition))
        i = thing->continue_state;
    if (i == CrSt_CreatureBeingDropped)
        return true;
    return false;
}

TbBool creature_is_being_unconscious(const struct Thing *thing)
{
    CrtrStateId i = thing->active_state;
    if ((i == CrSt_MoveToPosition) || (i == CrSt_MoveBackwardsToPosition))
        i = thing->continue_state;
    if (i == CrSt_CreatureUnconscious)
        return true;
    return false;
}

TbBool creature_is_celebrating(const struct Thing *thing)
{
    CrtrStateId i = thing->active_state;
    if ((i == CrSt_MoveToPosition) || (i == CrSt_MoveBackwardsToPosition))
        i = thing->continue_state;
    if (i == CrSt_CreatureBeHappy)
        return true;
    return false;
}

TbBool creature_is_being_tortured(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_Torturing) || (i == CrSt_AtTortureRoom))
        return true;
    return false;
}

TbBool creature_is_being_sacrificed(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_CreatureSacrifice) || (i == CrSt_CreatureBeingSacrificed))
        return true;
    return false;
}

TbBool creature_is_kept_in_prison(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_CreatureInPrison) || (i == CrSt_CreatureArrivedAtPrison))
        return true;
    return false;
}

TbBool creature_is_being_summoned(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_CreatureBeingSummoned))
        return true;
    return false;
}

TbBool creature_is_training(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_Training) || (i == CrSt_AtTrainingRoom))
        return true;
    return false;
}

TbBool creature_is_doing_anger_job(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if (states[i].state_type == CrStTyp_AngerJob)
        return true;
    return false;
}

TbBool creature_is_doing_garden_activity(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_CreatureEat) || (i == CrSt_CreatureEatingAtGarden) || (i == CrSt_CreatureToGarden) || (i == CrSt_CreatureArrivedAtGarden))
        return true;
    return false;
}

TbBool creature_is_scavengering(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_Scavengering) || (i == CrSt_AtScavengerRoom))
        return true;
    return false;
}

TbBool creature_is_being_scavenged(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if (i == CrSt_CreatureBeingScavenged)
        return true;
    return false;
}

TbBool creature_is_at_alarm(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if (i == CrSt_ArriveAtAlarm)
        return true;
    return false;
}

TbBool creature_is_escaping_death(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if (i == CrSt_CreatureEscapingDeath)
        return true;
    return false;
}

TbBool creature_is_fleeing_combat(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if (i == CrSt_CreatureCombatFlee)
    {
        return true;
    }
    return false;
}

/**
 * Returns if creature is in general in CTA following state.
 * Following CTA may be interrupted by fights and other events; if this returns true,
 *  the creature should get back to following CTA when other activity (like battle) is finished.
 * @param thing
 * @return
 */
TbBool creature_affected_by_call_to_arms(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    return ((cctrl->spell_flags & CSAfF_CalledToArms) != 0);
}

/**
 * Returns if creature is currently directly in CTA following state.
 * @param thing
 * @return
 */
TbBool creature_is_called_to_arms(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if ((i == CrSt_AlreadyAtCallToArms) || (i == CrSt_ArriveAtCallToArms))
        return true;
    return false;
}

TbBool creature_is_taking_salary_activity(const struct Thing *thing)
{
    CrtrStateId crstate = get_creature_state_besides_move(thing);
    if ((crstate == CrSt_CreatureWantsSalary) || (crstate == CrSt_CreatureTakeSalary))
        return true;
    return false;
}

TbBool creature_is_arming_trap(const struct Thing *thing)
{
    CrtrStateId crstate = get_creature_state_besides_move(thing);
    if (crstate == CrSt_CreaturePicksUpTrapObject) {
        return true;
    }
    crstate = get_creature_state_besides_drag(thing);
    if (crstate == CrSt_CreatureArmsTrap) {
        return true;
    }
    return false;
}

TbBool creature_is_kept_in_custody(const struct Thing *thing)
{
    return (creature_is_kept_in_prison(thing) ||
            creature_is_being_tortured(thing) ||
            creature_is_being_sacrificed(thing) ||
            creature_is_being_dropped(thing));
}

/**
 * Returns if a creature is kept in custody by a player other than its owner.
 * There is a limited amount of states the creature can be in while kept in custody.
 * This function should check all of them, as not meeting any mean that the creature has been freed.
 * @param thing The creature to be checked.
 * @return
 */
TbBool creature_is_kept_in_custody_by_enemy(const struct Thing *thing)
{
    if (thing_is_picked_up_by_enemy(thing)) {
        // The enemy keep us in hand - the fact is clear
        return true;
    }
    if (creature_is_kept_in_prison(thing) ||
        creature_is_being_tortured(thing) ||
        creature_is_being_sacrificed(thing) ||
        creature_is_being_dropped(thing))
    {
        struct Room* room = get_room_thing_is_on(thing);
        if (room_is_invalid(room)) {
            // This must mean we're being dropped outside of room, or sold/destroyed the room
            // so not kept in custody - freed
            return false;
        }
        if (thing->owner != room->owner) {
            // We're in a room, and it's not our own - must be enemy
            return true;
        }
    }
    return false;
}

TbBool creature_is_kept_in_custody_by_enemy_or_dying(const struct Thing *thing)
{
    return creature_is_kept_in_custody_by_enemy(thing) || creature_is_dying(thing);
}

TbBool creature_is_kept_in_custody_by_player(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (thing_is_picked_up_by_player(thing, plyr_idx)) {
        // The enemy keep us in hand - the fact is clear
        return true;
    }
    if (creature_is_kept_in_prison(thing) ||
        creature_is_being_tortured(thing) ||
        creature_is_being_sacrificed(thing) ||
        creature_is_being_dropped(thing))
    {
        struct Room* room = get_room_thing_is_on(thing);
        if (room_is_invalid(room)) {
            // This must mean we're being dropped outside of room, or sold/destroyed the room
            // so not kept in custody - freed
            return false;
        }
        if (room->owner == plyr_idx) {
            // We're in a room, and it's the player we asked for
            return true;
        }
    }
    return false;
}

short player_keeping_creature_in_custody(const struct Thing* thing)
{
    for (int plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (thing_is_picked_up_by_player(thing, plyr_idx))
        {
            if (thing->owner != plyr_idx)
            {
                // The enemy keeps it in hand - the fact is clear
                return plyr_idx;
            }
            else
            {
                return -1;
            }
        }
    }
    if (creature_is_kept_in_prison(thing) ||
        creature_is_being_tortured(thing) ||
        creature_is_being_sacrificed(thing) ||
        creature_is_being_dropped(thing))
    {
        struct Room* room = get_room_thing_is_on(thing);
        if (room_is_invalid(room)) {
            // This must mean we're being dropped outside of room, or sold/destroyed the room
            // so not kept in custody - freed
            return -1;
        }
        if (room->owner != thing->owner) {
            // We're in a room, and it's not the unit owner
            return room->owner;
        }
    }
    return -1;
}


TbBool creature_state_is_unset(const struct Thing *thing)
{
    CrtrStateId crstate = get_creature_state_besides_move(thing);
    if (states[crstate].state_type == 0)
        return true;
    return false;
}

TbBool restore_creature_flight_flag(struct Thing *creatng)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    // Creature can fly either by spell or natural ability
    if ((crstat->flying) || creature_affected_by_spell(creatng, SplK_Fly))
    {
        // Even flying creature is grounded while frozen
        if (!creature_affected_by_spell(creatng, SplK_Freeze)) {
            creatng->movement_flags |= TMvF_Flying;
            return true;
        }
    }
    return false;
}

short already_at_call_to_arms(struct Thing *creatng)
{
    TRACE_THING(creatng);
    internal_set_thing_state(creatng, CrSt_ArriveAtCallToArms);
    return 1;
}

short arrive_at_alarm(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->field_2FA < (unsigned long)game.play_gameturn)
    {
        set_start_state(creatng);
        return 1;
    }
    if (CREATURE_RANDOM(creatng, 4) == 0)
    {
        if (setup_person_move_close_to_position(creatng, cctrl->alarm_stl_x, cctrl->alarm_stl_y, NavRtF_Default))
        {
            creatng->continue_state = CrSt_ArriveAtAlarm;
            return 1;
        }
    }
    if ( creature_choose_random_destination_on_valid_adjacent_slab(creatng) )
    {
        creatng->continue_state = CrSt_ArriveAtAlarm;
        return 1;
    }
    return 1;
}

long setup_head_for_room(struct Thing *creatng, struct Room *room, unsigned char flags)
{
    TRACE_THING(creatng);
    struct Coord3d pos;
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos))
        return false;
    return setup_person_move_to_coord(creatng, &pos, flags);
}

TbBool attempt_to_destroy_enemy_room(struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Coord3d pos;
    struct Room* room = subtile_room_get(stl_x, stl_y);
    if (room_is_invalid(room))
        return false;
    if (players_creatures_tolerate_each_other(thing->owner, room->owner))
        return false;
    if (room_cannot_vandalise(room->kind))
        return false;
    if (!find_first_valid_position_for_thing_anywhere_in_room(thing, room, &pos))
        return false;
    if (!creature_can_navigate_to_with_storage(thing, &pos, NavRtF_NoOwner))
        return false;

    if (!setup_head_for_room(thing, room, NavRtF_NoOwner))
    {
        ERRORLOG("The %s cannot destroy %s because it can't reach it",thing_model_name(thing),room_code_name(room->kind));
        return false;
    }
    event_create_event_or_update_nearby_existing_event(
        subtile_coord_center(room->central_stl_x), subtile_coord_center(room->central_stl_y),
        EvKind_RoomUnderAttack, room->owner, 0);
    if (is_my_player_number(room->owner))
        output_message(SMsg_EnemyDestroyRooms, MESSAGE_DELAY_FIGHT, true);
    thing->continue_state = CrSt_CreatureAttackRooms;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (!creature_control_invalid(cctrl))
        cctrl->target_room_id = room->index;
    return true;
}

short arrive_at_call_to_arms(struct Thing *creatng)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    if (!player_uses_power_call_to_arms(creatng->owner))
    {
        set_start_state(creatng);
        return 1;
    }
    struct Thing* doortng = check_for_door_to_fight(creatng);
    if (!thing_is_invalid(doortng))
    {
        set_creature_door_combat(creatng, doortng);
        return 2;
    }
    if (!attempt_to_destroy_enemy_room(creatng, dungeon->cta_stl_x, dungeon->cta_stl_y))
    {
      if (CREATURE_RANDOM(creatng, 7) == 0)
      {
          if (setup_person_move_close_to_position(creatng, dungeon->cta_stl_x, dungeon->cta_stl_y, NavRtF_Default))
          {
              creatng->continue_state = CrSt_AlreadyAtCallToArms;
              return 1;
          }
      }
      if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
      {
          creatng->continue_state = CrSt_AlreadyAtCallToArms;
          return 1;
      }
    }
    return 1;
}

/**
 * Finds a safe position for creature on one of subtiles of given slab.
 * @param pos The returned position.
 * @param thing The creature to be moved.
 * @param slb_x Given slab to move onto, X coord.
 * @param slb_y Given slab to move onto, Y coord.
 * @param start_stl Starting element when testing subtiles, used for enhanced randomness.
 * @return True if the safe position was found; false otherwise.
 */
TbBool creature_find_safe_position_to_move_within_slab(struct Coord3d *pos, const struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y, MapSubtlCoord start_stl)
{
    SYNCDBG(7,"Finding at (%d,%d)",(int)slb_x,(int)slb_y);
    MapSubtlCoord stl_x = thing->mappos.x.stl.num;
    MapSubtlCoord stl_y = thing->mappos.y.stl.num;
    MapSubtlCoord base_x = slab_subtile(slb_x, 0);
    MapSubtlCoord base_y = slab_subtile(slb_y, 0);
    long m = start_stl;
    for (long i = 0; i < STL_PER_SLB * STL_PER_SLB; i++)
    {
        MapSubtlCoord x = base_x + (m % STL_PER_SLB);
        MapSubtlCoord y = base_y + (m / STL_PER_SLB);
        if ((x != stl_x) || (y != stl_y))
        {
            struct Map* mapblk = get_map_block_at(x, y);
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
            {
                if (!terrain_toxic_for_creature_at_position(thing, x, y))
                {
                    int block_radius = subtile_coord(thing_nav_block_sizexy(thing), 0) / 2;
                    pos->x.val = subtile_coord_center(x);
                    pos->y.val = subtile_coord_center(y);
                    pos->z.val = get_thing_height_at_with_radius(thing, pos, block_radius);
                    return true;
                }
            }
        }
        m = (m+1) % (STL_PER_SLB*STL_PER_SLB);
    }
    return false;
}

/**
 * Finds any position for creature on one of subtiles of given slab.
 * To be used when finding correct, safe position fails.
 * @param pos The returned position.
 * @param thing The creature to be moved.
 * @param slb_x Given slab to move onto, X coord.
 * @param slb_y Given slab to move onto, Y coord.
 * @param start_stl Starting element when testing subtiles, used for enhanced randomness.
 * @return True if any position was found; false otherwise.
 */
TbBool creature_find_any_position_to_move_within_slab(struct Coord3d *pos, const struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y, MapSubtlCoord start_stl)
{
    MapSubtlCoord stl_x = thing->mappos.x.stl.num;
    MapSubtlCoord stl_y = thing->mappos.y.stl.num;
    MapSubtlCoord base_x = slab_subtile(slb_x, 0);
    MapSubtlCoord base_y = slab_subtile(slb_y, 0);
    long m = start_stl;
    for (long i = 0; i < STL_PER_SLB * STL_PER_SLB; i++)
    {
        MapSubtlCoord x = base_x + (m % STL_PER_SLB);
        MapSubtlCoord y = base_y + (m / STL_PER_SLB);
        if ((x != stl_x) || (y != stl_y))
        {
            pos->x.val = subtile_coord_center(x);
            pos->y.val = subtile_coord_center(y);
            pos->z.val = get_thing_height_at(thing, pos);
            return true;
        }
        m = (m+1) % (STL_PER_SLB*STL_PER_SLB);
    }
    return false;
}

struct Room *get_room_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Room* room = INVALID_ROOM;
    if (subtile_is_room(stl_x, stl_y)) {
        room = subtile_room_get(stl_x, stl_y);
    }
    return room;
}

/**
 * Fills given array with information about small_around_slab[] elements which are accessible to move to.
 * @param avail The array of size SMALL_AROUND_SLAB_LENGTH
 * @param thing The thing which is to be moved to adjacent tile
 * @param room The room inside which we want to move
 * @return
 */
TbBool fill_moveable_small_around_slabs_array_in_room(TbBool *avail, const struct Thing *thing, const struct Room *room)
{
    long slab_base = get_slab_number(subtile_slab_fast(thing->mappos.x.stl.num), subtile_slab_fast(thing->mappos.y.stl.num));
    // Fill the avail[] array
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        long slab_num = slab_base + small_around_slab[n];
        MapSlabCoord slb_x = slb_num_decode_x(slab_num);
        MapSlabCoord slb_y = slb_num_decode_y(slab_num);
        MapSubtlCoord stl_x = slab_subtile_center(slb_x);
        MapSubtlCoord stl_y = slab_subtile_center(slb_y);
        // Per slab code
        struct Room* aroom = get_room_xy(stl_x, stl_y);
        avail[n] = !room_is_invalid(aroom) && (aroom->index == room->index) && !subtile_has_sacrificial_on_top(stl_x, stl_y);
        // Per slab code ends
    }
    return true;
}

TbBool set_position_at_slab_for_thing(struct Coord3d *pos, const struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y, long start_stl)
{
    long block_radius = subtile_coord(thing_nav_block_sizexy(thing), 0) / 2;
    struct Coord3d locpos;
    if (creature_find_safe_position_to_move_within_slab(&locpos, thing, slb_x, slb_y, start_stl))
    {
        if (thing_in_wall_at_with_radius(thing, &locpos, block_radius))
        {
            SYNCDBG(8,"The %s index %d can't fit to safe position (%d,%d)", thing_model_name(thing),
                (int)thing->index, (int)locpos.x.stl.num, (int)locpos.y.stl.num);
            return false;
        }
        pos->x.val = locpos.x.val;
        pos->y.val = locpos.y.val;
        pos->z.val = locpos.z.val;
        return true;
    }
    return false;
}

/**
 * Finds a safe, adjacent position in room for a creature.
 * Makes sure the specific room currently occupied is not left.
 * @param thing The creature to be moved.
 * @param room The room inside which the creature should stay.
 * @param pos The adjacent position returned.
 * @return True if a position was found, false if cannot move.
 * @see find_position_around_in_room()
 */
TbBool person_get_somewhere_adjacent_in_room_f(struct Thing *thing, const struct Room *room, struct Coord3d *pos, const char *func_name)
{
    SYNCDBG(17,"%s: Starting for %s index %d",func_name,thing_model_name(thing),(int)thing->index);
    MapSlabCoord slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    long slab_base = get_slab_number(slb_x, slb_y);

    int start_stl = CREATURE_RANDOM(thing, AROUND_MAP_LENGTH);
    long m = CREATURE_RANDOM(thing, SMALL_AROUND_SLAB_LENGTH);
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        long slab_num = slab_base + small_around_slab[m];
        slb_x = slb_num_decode_x(slab_num);
        slb_y = slb_num_decode_y(slab_num);
        struct Room* aroom = get_room_xy(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        // Per slab code
        if (room_exists(aroom) && (aroom->index == room->index))
        {
            if (set_position_at_slab_for_thing(pos, thing, slb_x, slb_y, start_stl))
            {
                SYNCDBG(8,"%s: Possible to move %s index %d from (%d,%d) to (%d,%d)", func_name, thing_model_name(thing),
                    (int)thing->index, (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num,
                    (int)pos->x.stl.num, (int)pos->y.stl.num);
                return true;
            }
        }
        // Per slab code ends
        m = (m + 1) % SMALL_AROUND_SLAB_LENGTH;
    }
    // Cannot find a good position - but at least move within the same slab we're on
    {
        slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
        slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
        {
            if (set_position_at_slab_for_thing(pos, thing, slb_x, slb_y, start_stl))
            {
                SYNCDBG(8,"%s: Short move %s index %d from (%d,%d) to (%d,%d)", func_name, thing_model_name(thing),
                    (int)thing->index, (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num,
                    (int)pos->x.stl.num, (int)pos->y.stl.num);
                return true;
            }
        }
    }
    return false;
}

/**
 * Finds a safe adjacent position in room for a thing to move to.
 * Makes sure the specific room currently occupied is not left.
 * Prefers selecting the position on room border in a way which would allow the creature to move around the border.
 * Originally named person_get_somewhere_adjacent_in_temple().
 * @param thing The thing which is to be moved.
 * @param room Room within which the new position should be.
 * @param pos The target position pointer.
 */
TbBool person_get_somewhere_adjacent_in_room_around_borders_f(struct Thing *thing, const struct Room *room, struct Coord3d *pos, const char *func_name)
{
    if (!room_exists(room))
    {
        ERRORLOG("Tried to find position in non-existing room");
        pos->x.val = subtile_coord_center(map_subtiles_x/2);
        pos->y.val = subtile_coord_center(map_subtiles_y/2);
        pos->z.val = subtile_coord(1,0);
        return false;
    }
    long slab_base = get_slab_number(subtile_slab_fast(thing->mappos.x.stl.num), subtile_slab_fast(thing->mappos.y.stl.num));
    long start_stl = CREATURE_RANDOM(thing, STL_PER_SLB * STL_PER_SLB);
    // If the room is too small - don't try selecting adjacent slab
    if (room->slabs_count > 1)
    {
        // Fill the avail[] array
        TbBool avail[SMALL_AROUND_SLAB_LENGTH];
        fill_moveable_small_around_slabs_array_in_room(avail, thing, room);
        // Use the array to get first index
        int arnd;
        if (avail[0]) // can go to sibling slab (0,-1)
        {
            if (avail[2]) { // can go to sibling slab (0,1)
                long long tmp = thing->move_angle_xy + LbFPMath_PI / 2;
                tmp = ((((tmp>>32) ^ (((tmp>>32) ^ tmp) - (tmp>>32))) & 0x7FF) - (tmp>>32));
                arnd = 2 * ((((tmp>>16) & 0x3FF) + tmp) / LbFPMath_PI);
            } else {
                arnd = avail[1] ? 0 : 3;
            }
        } else
        if (avail[1]) // can go to sibling slab (1,0)
        {
            if (avail[3]) {
                arnd = 2 * ((thing->move_angle_xy & 0x400) / LbFPMath_PI) + 1;
            } else {
                arnd = 1;
            }
        } else
        {
            arnd = 2;
        }
        for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
        {
            if (avail[arnd])
            {
                long slab_num = slab_base + small_around_slab[arnd];
                MapSlabCoord slb_x = slb_num_decode_x(slab_num);
                MapSlabCoord slb_y = slb_num_decode_y(slab_num);
                if (set_position_at_slab_for_thing(pos, thing, slb_x, slb_y, start_stl))
                {
                    SYNCDBG(8,"Possible to move %s index %d from (%d,%d) to (%d,%d)", thing_model_name(thing),
                        (int)thing->index, (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num,
                        (int)pos->x.stl.num, (int)pos->y.stl.num);
                    return true;
                }
            }
            arnd = (arnd + 1) % SMALL_AROUND_SLAB_LENGTH;
        }
    }
    // No position found - at least try to move within the same slab
    {
        MapSlabCoord slb_x = slb_num_decode_x(slab_base);
        MapSlabCoord slb_y = slb_num_decode_y(slab_base);
        if (set_position_at_slab_for_thing(pos, thing, slb_x, slb_y, start_stl))
        {
            SYNCDBG(8,"Short move %s index %d from (%d,%d) to (%d,%d)", thing_model_name(thing),
                (int)thing->index, (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num,
                (int)pos->x.stl.num, (int)pos->y.stl.num);
            return true;
        }
    }
    return false;
}

/**
 * Finds a safe, adjacent position in room for a creature.
 * Allows moving to adjacent room of the same kind and owner.
 *
 * @param pos Position of the creature to be moved.
 * @param owner Room owner to keep.
 * @param rkind Kind of the room inside which the creature should stay.
 * @return Coded subtiles of the new position, or 0 on failure.
 * @see person_get_somewhere_adjacent_in_room()
 */
SubtlCodedCoords find_position_around_in_room(const struct Coord3d *pos, PlayerNumber owner, RoomKind rkind, struct Thing* thing)
{
    SubtlCodedCoords stl_num;
    long m = CREATURE_RANDOM(thing, AROUND_MAP_LENGTH);
    for (long n = 0; n < AROUND_MAP_LENGTH; n++)
    {
        SubtlCodedCoords accepted_stl_num = 0;
        stl_num = get_subtile_number(pos->x.stl.num,pos->y.stl.num);
        // Skip the position equal to current position
        if (around_map[m] == 0)
        {
            m = (m + 1) % AROUND_MAP_LENGTH;
            continue;
        }
        // Move radially from of the current position; stop if a room tile
        // of incorrect kind or owner is encoured
        for (long dist = 0; dist < 8; dist++)
        {
            stl_num += around_map[m];
            struct Map* mapblk = get_map_block_at_pos(stl_num);
            if ( ((mapblk->flags & SlbAtFlg_IsRoom) != 0) && ((mapblk->flags & SlbAtFlg_Blocking) != 0) )
                break;
            MapSubtlCoord stl_x = stl_num_decode_x(stl_num);
            MapSubtlCoord stl_y = stl_num_decode_y(stl_num);
            struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
            if (slabmap_owner(slb) != owner)
                break;
            struct Room* room = room_get(slb->room_index);
            if (room->kind != rkind)
                break;
            if (dist > 1)
            {
                // Accept, but don't just break the loop. Wait for larger distance.
                accepted_stl_num = stl_num;
            }
        }
        if (accepted_stl_num > 0)
        {
            return accepted_stl_num;
        }
        m = (m + 1) % AROUND_MAP_LENGTH;
    }
    // No position found - at least try to move within the same slab
    do {
        stl_num = get_subtile_number(pos->x.stl.num,pos->y.stl.num);
        struct Map* mapblk = get_map_block_at_pos(stl_num);
        if ( ((mapblk->flags & SlbAtFlg_IsRoom) != 0) && ((mapblk->flags & SlbAtFlg_Blocking) != 0) )
            break;
        MapSubtlCoord stl_x = stl_num_decode_x(stl_num);
        MapSubtlCoord stl_y = stl_num_decode_y(stl_num);
        struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
        if (slabmap_owner(slb) != owner)
            break;
        struct Room* room = room_get(slb->room_index);
        if (room->kind != rkind)
            break;
        return stl_num;
    } while (0);
    // Failed completely - cannot move
    return 0;
}

short cleanup_hold_audience(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->max_speed = calculate_correct_creature_maxspeed(creatng);
    return 0;
}

short cleanup_seek_the_enemy(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->word_9A = 0;
    cctrl->long_9C = 0;
    return 1;
}

void set_flee_delay(struct Thing* creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->wait_to_turn = game.play_gameturn + FIGHT_FEAR_DELAY;
}

short creature_being_dropped(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(17,"Starting for %s index %d",thing_model_name(creatng),(long)creatng->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->flgfield_1 |= CCFlg_NoCompControl;
    // Cannot teleport for a few turns after being dropped
    delay_teleport(creatng);
    set_flee_delay(creatng);
    MapSubtlCoord stl_x = creatng->mappos.x.stl.num;
    MapSubtlCoord stl_y = creatng->mappos.y.stl.num;
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    // If dropping still in progress, do nothing
    if ( !(thing_touching_floor(creatng) || (((creatng->movement_flags & TMvF_Flying) != 0) && thing_touching_flight_altitude(creatng))))
    {
        // Note that the creature should have no self control while dropping - after all, it was in hand moments ago
        SYNCDBG(17,"The %s index %d owner %d dropped at (%d,%d) isn't touching ground yet",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,(int)stl_x,(int)stl_y);
        return 1;
    }
    set_creature_assigned_job(creatng, Job_NULL);
    // If the creature has flight ability, return it to flying state
    restore_creature_flight_flag(creatng);
    // Set creature to default state, in case giving it job will fail
    set_start_state(creatng);
    // Check job which we can do after dropping at these coordinates
    CreatureJob new_job = get_job_for_subtile(creatng, stl_x, stl_y, JoKF_AssignHumanDrop);
    // Most tasks are disabled while creature is a chicken
    if (!creature_affected_by_spell(creatng, SplK_Chicken))
    {
        // For creatures with trembling fat and not changed to chickens, tremble the camera
        if ((get_creature_model_flags(creatng) & CMF_TremblingFat) != 0)
        {
            struct Dungeon* dungeon = get_dungeon(creatng->owner);
            if (!dungeon_invalid(dungeon)) {
                dungeon->camera_deviate_jump = 96;
            }
        }
        // Make sure computer control flag is set (almost) accordingly to job, so that we won't start a fight when in captivity
        // Unless we are dropping our own creature, in that case we may want to ignore captivity and start a fight
        if (new_job != Job_NULL)
        {
            if (((get_flags_for_job(new_job) & JoKF_NoSelfControl) != 0) && (slabmap_owner(slb) != creatng->owner)) {
                cctrl->flgfield_1 |= CCFlg_NoCompControl;
            } else {
                cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
            }
        } else
        {
            cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
        }
        // Reveal any nearby terrain
        check_map_explored(creatng, stl_x, stl_y);
        // Creatures dropped far from group are removed from it
        struct Thing* leadtng = get_group_leader(creatng);
        if (!thing_is_invalid(leadtng))
        {
            if (leadtng->index != creatng->index)
            {
                if (!thing_is_picked_up(leadtng))
                {
                    if (get_2d_box_distance(&creatng->mappos, &leadtng->mappos) > subtile_coord(9,0)) {
                        SYNCDBG(3,"Removing %s index %d owned by player %d from group",
                            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
                        remove_creature_from_group(creatng);
                    }
                }
            }
        }
    }
    // TODO CREATURE_JOBS it would be great if these jobs were also returned by get_job_for_subtile()
    if (!creature_affected_by_spell(creatng, SplK_Chicken))
    {
        // Special tasks for diggers
        if ((get_creature_model_flags(creatng) & CMF_IsSpecDigger) != 0)
        {
            if ((slabmap_owner(slb) == creatng->owner) || (slabmap_owner(slb) == game.neutral_player_num))
            {
                if (check_out_available_spdigger_drop_tasks(creatng))
                {
                    SYNCDBG(3,"No digger job for %s index %d owner %d at (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,(int)stl_x,(int)stl_y);
                    cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
                    return 2;
                }
            }
        }
        // Do combat, if we can
        if (creature_will_do_combat(creatng))
        {
            if (creature_look_for_combat(creatng)) {
                SYNCDBG(3,"The %s index %d owner %d found creature combat at (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,(int)stl_x,(int)stl_y);
                return 2;
            }
            if (creature_look_for_enemy_heart_combat(creatng)) {
                SYNCDBG(3,"The %s index %d owner %d found heart combat at (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,(int)stl_x,(int)stl_y);
                return 2;
            }
            if (creature_look_for_enemy_door_combat(creatng)) {
                SYNCDBG(3,"The %s index %d owner %d found enemy combat at (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,(int)stl_x,(int)stl_y);
                return 2;
            }
        }
        if (new_job != Job_NULL)
        {
            // Make sure computer control flag is set accordingly to job, now do it straight and without exclusions
            if ((get_flags_for_job(new_job) & JoKF_NoSelfControl) != 0) {
                cctrl->flgfield_1 |= CCFlg_NoCompControl;
            } else {
                cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
            }
        } else
        {
            cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
        }
    }
    if (new_job == Job_NULL)
    {
        SYNCDBG(3,"No job found at (%d,%d) for %s index %d owner %d",(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        // Job_NULL is already assigned here, and default state is already initialized
        cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
        return 2;
    }
    SYNCDBG(3,"Job %s to be assigned to %s index %d owner %d",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    // Check if specific conditions are met for this job to be assigned
    if (!creature_can_do_job_near_position(creatng, stl_x, stl_y, new_job, JobChk_SetStateOnFail|JobChk_PlayMsgOnFail))
    {
        SYNCDBG(16,"Cannot assign job %s to %s (owner %d)",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->owner);
        cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
        return 2;
    }
    // Now try sending the creature to do job it should do at this position
    if (!send_creature_to_job_near_position(creatng, stl_x, stl_y, new_job))
    {
        SYNCDBG(13,"Cannot assign %s to %s index %d owner %d; could not send to room",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
        return 2;
    }
    // If applicable, set the job as assigned job for the creature
    if ((get_flags_for_job(new_job) & JoKF_AssignOneTime) == 0) {
        set_creature_assigned_job(creatng, new_job);
    } else {
        // One-time jobs are not assigned to the creature, they are just initialized to be performed once
        //set_creature_assigned_job(creatng, Job_NULL); -- already assigned
    }
    return 2;
}

short creature_cannot_find_anything_to_do(struct Thing *creatng)
{
    TRACE_THING(creatng);
	struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
	if ((game.play_gameturn - cctrl->countdown_282) >= 128)
	{
		set_start_state(creatng);
		return 0;
	}
	if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
		creatng->continue_state = 123;
	return 1;
}

void set_creature_size_stuff(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (creature_affected_by_spell(creatng, SplK_Chicken))
    {
      creatng->sprite_size = gameadd.crtr_conf.sprite_size;
    } else
    {
      creatng->sprite_size = gameadd.crtr_conf.sprite_size + (gameadd.crtr_conf.sprite_size * gameadd.crtr_conf.exp.size_increase_on_exp * cctrl->explevel) / 100;
    }
}

short creature_change_from_chicken(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creature_set_speed(creatng, 0);
    if (cctrl->countdown_282 > 0)
        cctrl->countdown_282--;
    if (cctrl->countdown_282 > 0)
    { // Changing under way - gradually modify size of the creature
        creatng->field_4F |= TF4F_Unknown01;
        creatng->field_50 |= 0x01;
        struct Thing* efftng = create_effect_element(&creatng->mappos, TngEffElm_Chicken, creatng->owner);
        if (!thing_is_invalid(efftng))
        {
            long n = (10 - cctrl->countdown_282) * (gameadd.crtr_conf.sprite_size + (gameadd.crtr_conf.sprite_size * gameadd.crtr_conf.exp.size_increase_on_exp * cctrl->explevel) / 100) / 10;
            unsigned long k = get_creature_anim(creatng, 0);
            set_thing_draw(efftng, k, 256, n, -1, 0, 2);
            efftng->field_4F &= ~TF4F_Transpar_Flags;
            efftng->field_4F |= TF4F_Transpar_8;
        }
        return 0;
    } else
    {
        creatng->field_4F &= ~TF4F_Unknown01;
        cctrl->stateblock_flags &= ~CCSpl_ChickenRel;
        cctrl->spell_flags &= ~CSAfF_Chicken;
        set_creature_size_stuff(creatng);
        set_start_state(creatng);
        return 1;
    }
}

short creature_change_to_chicken(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    creature_set_speed(creatng, 0);
    if (cctrl->countdown_282 > 0)
        cctrl->countdown_282--;
    if (cctrl->countdown_282 > 0)
    {
      creatng->field_50 |= 0x01;
      creatng->field_4F |= TF4F_Unknown01;
      struct Thing* efftng = create_effect_element(&creatng->mappos, TngEffElm_Chicken, creatng->owner);
      if (!thing_is_invalid(efftng))
      {
          unsigned long k = convert_td_iso(819);
          set_thing_draw(efftng, k, 0, 1200 * cctrl->countdown_282 / 10 + gameadd.crtr_conf.sprite_size, -1, 0, 2);
          efftng->field_4F &= ~TF4F_Transpar_Flags;
          efftng->field_4F |= TF4F_Transpar_8;
      }
      return 0;
    }
    cctrl->spell_flags |= CSAfF_Chicken;
    creatng->field_4F &= ~TF4F_Unknown01;
    set_creature_size_stuff(creatng);
    creatng->state_flags &= ~TF1_Unkn10;
    creatng->active_state = CrSt_CreaturePretendChickenSetupMove;
    creatng->continue_state = CrSt_Unused;
    cctrl->field_302 = 0;
    clear_creature_instance(creatng);
    return 1;
}

TbBool creature_try_going_to_lazy_sleep(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Room* room = get_creature_lair_room(creatng);
    if (room_is_invalid(room)) {
        return false;
    }
    if ((room->kind != get_room_for_job(Job_TAKE_SLEEP)) || (room->owner != creatng->owner)) {
        ERRORLOG("The %s index %d has lair in invalid room",thing_model_name(creatng),(int)creatng->index);
        return false;
    }
    if (game.play_gameturn - cctrl->tasks_check_turn <= 128) {
        return false;
    }
    cctrl->tasks_check_turn = game.play_gameturn;
    if (!creature_setup_random_move_for_job_in_room(creatng, room, Job_TAKE_SLEEP, NavRtF_Default)) {
        return false;
    }
    creatng->continue_state = CrSt_CreatureDoingNothing;
    return true;
}

short creature_try_going_to_healing_sleep(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (!creature_can_do_healing_sleep(creatng)) {
        return false;
    }
    if (game.play_gameturn - cctrl->healing_sleep_check_turn <= 200) {
        return false;
    }
    cctrl->healing_sleep_check_turn = game.play_gameturn;
    if (!creature_free_for_sleep(creatng, CrSt_CreatureGoingHomeToSleep)) {
        return false;
    }
    if (!creature_has_lair_room(creatng) && room_is_invalid(get_best_new_lair_for_creature(creatng))) {
        return false;
    }
    if (external_set_thing_state(creatng, CrSt_CreatureGoingHomeToSleep)) {
        return true;
    }
    return false;
}

short creature_doing_nothing(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    // Wait for up to 2 turns (note that this is unsigned substraction)
    if (game.play_gameturn - cctrl->idle.start_gameturn <= 1) {
        return 1;
    }
    if ((cctrl->spell_flags & CSAfF_MadKilling) != 0) {
        SYNCDBG(8,"The %s index %d goes mad killing",thing_model_name(creatng),creatng->index);
        internal_set_thing_state(creatng, CrSt_MadKillingPsycho);
        return 1;
    }
    if (!creature_has_lair_room(creatng) && creature_can_do_healing_sleep(creatng) && creature_free_for_sleep(creatng, CrSt_CreatureWantsAHome))
    {
        if ((game.play_gameturn - cctrl->tasks_check_turn > 128))
        {
            int required_cap = get_required_room_capacity_for_object(RoRoF_LairStorage, 0, creatng->model);
            cctrl->tasks_check_turn = game.play_gameturn;
            struct Room* room = find_nearest_room_for_thing_with_spare_capacity(creatng, creatng->owner, get_room_for_job(Job_TAKE_SLEEP), NavRtF_Default, required_cap);
            if (!room_is_invalid(room))
            {
                internal_set_thing_state(creatng, CrSt_CreatureWantsAHome);
                SYNCDBG(8,"The %s index %d goes make lair",thing_model_name(creatng),creatng->index);
                return 1;
            }
            update_cannot_find_room_wth_spare_capacity_event(creatng->owner, creatng, get_room_for_job(Job_TAKE_SLEEP));
        }
    }
    if (creature_affected_by_call_to_arms(creatng))
    {
        struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
        struct Coord3d cta_pos;
        cta_pos.x.val = subtile_coord_center(dungeon->cta_stl_x);
        cta_pos.y.val = subtile_coord_center(dungeon->cta_stl_y);
        cta_pos.z.val = get_floor_height_at(&cta_pos);
        if (update_creature_influenced_by_call_to_arms_at_pos(creatng, &cta_pos))
        {
            SYNCDBG(8,"The %s index %d is called to arms",thing_model_name(creatng),creatng->index);
            return 1;
        }
    }
    if ((cctrl->job_assigned != Job_NULL) && (game.play_gameturn - cctrl->job_assigned_check_turn > 128))
    {
        if (attempt_job_preference(creatng, cctrl->job_assigned)) {
            SYNCDBG(8,"The %s index %d will do assigned job with state %s",thing_model_name(creatng),
                (int)creatng->index,creature_state_code_name(get_creature_state_besides_interruptions(creatng)));
            return 1;
        }
        cctrl->job_assigned_check_turn = game.play_gameturn;
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if ((crstat->job_primary != Job_NULL) && (game.play_gameturn - cctrl->job_primary_check_turn > 128))
    {
        if (attempt_job_preference(creatng, crstat->job_primary)) {
            SYNCDBG(8,"The %s index %d will do primary job with state %s",thing_model_name(creatng),
                (int)creatng->index,creature_state_code_name(get_creature_state_besides_interruptions(creatng)));
            return 1;
        }
        cctrl->job_primary_check_turn = game.play_gameturn;
    }
    long n = CREATURE_RANDOM(creatng, 3);
    for (long i = 0; i < 3; i++)
    {
        switch (n)
        {
        case 0:
            if (creature_try_going_to_lazy_sleep(creatng)) {
                SYNCDBG(8,"The %s index %d will do lazy sleep",thing_model_name(creatng),(int)creatng->index);
                return 1;
            }
            break;
        case 1:
            if (creature_try_going_to_healing_sleep(creatng)) {
                SYNCDBG(8,"The %s index %d will do healing sleep",thing_model_name(creatng),(int)creatng->index);
                return 1;
            }
            break;
        case 2:
            if (creature_try_doing_secondary_job(creatng)) {
                SYNCDBG(8,"The %s index %d will do secondary job with state %s",thing_model_name(creatng),
                    (int)creatng->index,creature_state_code_name(get_creature_state_besides_interruptions(creatng)));
                return 1;
            }
            break;
        default:
            break;
        }
        n = (n + 1) % 3;
    }
    if (game.play_gameturn - cctrl->wander_around_check_turn > 128)
    {
        cctrl->wander_around_check_turn = game.play_gameturn;
        struct Coord3d pos;
        if (get_random_position_in_dungeon_for_creature(creatng->owner, CrWaS_WithinDungeon, creatng, &pos))
        {
            if (setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
            {
                creatng->continue_state = CrSt_CreatureDoingNothing;
                return 1;
            }
        }
    }
    internal_set_thing_state(creatng, CrSt_CreatureCannotFindAnythingToDo);
    cctrl->countdown_282 = game.play_gameturn;
    return 0;
}

/**
 * Returns if given slab meets the requirements for a creature to move on it for its own will.
 * @param thing The creature which is moving.
 * @param slb_x The destination slab, X coord.
 * @param slb_y The destination slab, Y coord.
 * @return True if the creature is willing to move on that slab, false otherwise.
 */
TbBool slab_is_valid_for_creature_choose_move(const struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    struct SlabAttr* slbattr = get_slab_attrs(slb);
    if ( ((slbattr->block_flags & SlbAtFlg_IsRoom) != 0) || ((slbattr->block_flags & SlbAtFlg_Blocking) == 0) )
        return true;
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    struct Thing* doortng = get_door_for_position(stl_x, stl_y);
    if (!thing_is_invalid(doortng))
    {
      if ((doortng->owner == thing->owner) && (!doortng->door.is_locked))
          return true;
    }
    return false;
}

TbBool creature_choose_random_destination_on_valid_adjacent_slab(struct Thing *thing)
{
    SYNCDBG(17,"Starting for %s index %d",thing_model_name(thing),(long)thing->index);
    MapSlabCoord slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
    long slab_base = get_slab_number(slb_x, slb_y);

    MapSubtlCoord start_stl = CREATURE_RANDOM(thing, 9);
    long m = CREATURE_RANDOM(thing, SMALL_AROUND_SLAB_LENGTH);
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        long slab_num = slab_base + small_around_slab[m];
        slb_x = slb_num_decode_x(slab_num);
        slb_y = slb_num_decode_y(slab_num);
        if (slab_is_valid_for_creature_choose_move(thing, slb_x, slb_y))
        {
            struct Coord3d locpos;
            if (creature_find_safe_position_to_move_within_slab(&locpos, thing, slb_x, slb_y, start_stl))
            {
                if (setup_person_move_to_coord(thing, &locpos, NavRtF_Default))
                {
                    SYNCDBG(8,"Moving thing %s from (%d,%d) to (%d,%d)", thing_model_name(thing),
                        (int)thing->index, (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num,
                        (int)locpos.x.stl.num, (int)locpos.y.stl.num);
                    return true;
                }
            }
        }
        m = (m+1) % SMALL_AROUND_SLAB_LENGTH;
    }
    // Cannot find a good position - but at least move within the same slab we're on
    {
        slb_x = subtile_slab_fast(thing->mappos.x.stl.num);
        slb_y = subtile_slab_fast(thing->mappos.y.stl.num);
        struct Coord3d locpos;
        if (creature_find_any_position_to_move_within_slab(&locpos, thing, slb_x, slb_y, start_stl))
        {
            if (setup_person_move_to_coord(thing, &locpos, NavRtF_Default))
            {
                SYNCDBG(8,"Short moving %s index %d from (%d,%d) to (%d,%d)", thing_model_name(thing),
                    (int)thing->index, (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num,
                    (int)locpos.x.stl.num, (int)locpos.y.stl.num);
                return true;
            }
        }
    }
    SYNCDBG(8,"Moving %s index %d from (%d,%d) failed",thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
    return false;
}

short creature_dormant(struct Thing *creatng)
{
    TRACE_THING(creatng);
    if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
    {
      creatng->continue_state = CrSt_CreatureDormant;
      return 1;
    }
    return 0;
}

short creature_escaping_death(struct Thing *creatng)
{
    TRACE_THING(creatng);
    set_start_state(creatng);
    return 0;
}

long get_best_position_outside_room(struct Thing *creatng, struct Coord3d *pos, struct Room *room)
{
    return _DK_get_best_position_outside_room(creatng, pos, room);
}

short creature_evacuate_room(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    if (!thing_is_on_any_room_tile(creatng))
    {
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    struct Room* room = get_room_thing_is_on(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->word_9A != room->index)
    {
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    long ret = get_best_position_outside_room(creatng, &pos, room);
    set_creature_assigned_job(creatng, Job_NULL);
    if (ret != 1)
    {
        if (ret == -1)
        {
            set_start_state(creatng);
            return CrCkRet_Continue;
        }
        return CrCkRet_Continue;
    }
    if (!setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
    {
        ERRORLOG("Cannot nav outside room");
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    creatng->continue_state = CrSt_CreatureEvacuateRoom;
    return CrCkRet_Continue;
}

short creature_explore_dungeon(struct Thing *creatng)
{
    TbBool ret;
    TRACE_THING(creatng);
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(map_subtiles_x / 2);
    pos.y.val = subtile_coord_center(map_subtiles_y/2);
    pos.z.val = subtile_coord(1,0);
    {
        ret = get_random_position_in_dungeon_for_creature(creatng->owner, CrWaS_OutsideDungeon, creatng, &pos);
        if (ret) {
            ret = setup_person_move_to_coord(creatng, &pos, NavRtF_Default);
        }
    }
    if (!ret) {
        ret = get_random_position_in_dungeon_for_creature(creatng->owner, CrWaS_WithinDungeon, creatng, &pos);
        if (ret) {
            ret = setup_person_move_to_coord(creatng, &pos, NavRtF_Default);
        }
    }
    if (!ret)
    {
        SYNCLOG("The %s owned by player %d can't navigate to subtile (%d,%d) for exploring",
            thing_model_name(creatng),(int)creatng->owner, (int)pos.x.stl.num, (int)pos.y.stl.num);
        set_start_state(creatng);
        return CrCkRet_Available;
    }
    creatng->continue_state = CrSt_CreatureDoingNothing;
    return CrCkRet_Continue;
}

/**
 * Makes the creature leave the dungeon.
 * Originally named creature_fired().
 * @param creatng
 */
short creature_exempt(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Room* room = get_room_thing_is_on(creatng);
    if (!room_exists(room) || !room_role_matches(room->kind, RoRoF_CrPoolLeave))
    {
        set_start_state(creatng);
        return CrCkRet_Continue;
    }
    play_creature_sound_and_create_sound_thing(creatng, CrSnd_Sad, 2);
    kill_creature(creatng, INVALID_THING, -1, CrDed_NoEffects|CrDed_NotReallyDying);
    return CrCkRet_Deleted;
}

short creature_follow_leader(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Thing* leadtng = get_group_leader(creatng);
    if (!thing_is_creature(leadtng))
    {
        SYNCLOG("The %s index %d owned by player %d can no longer follow leader - it's invalid",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        set_start_state(creatng);
        return 1;
    }
    if (leadtng->index == creatng->index)
    {
        SYNCLOG("The %s index %d owned by player %d became party leader",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        set_start_state(creatng);
        return 1;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->spell_flags & CSAfF_MadKilling) != 0)
    {
        SYNCLOG("The %s index %d owned by player %d can no longer be in group - became mad",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        remove_creature_from_group(creatng);
        set_start_state(creatng);
        return 1;
    }
    struct Coord3d follwr_pos;
    if (!get_free_position_behind_leader(leadtng, &follwr_pos))
    {
        SYNCLOG("The %s index %d owned by player %d can no longer follow %s - no place amongst followers",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,thing_model_name(leadtng));
        set_start_state(creatng);
        return 1;
    }
    int fails_amount = cctrl->field_307;
    if (fails_amount > 8)
    {
        SYNCDBG(3,"Removing %s index %d owned by player %d from group due to fails to follow",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        remove_creature_from_group(creatng);
        return 0;
    }
    if ((fails_amount > 0) && (cctrl->field_303 + 16 > game.play_gameturn))
    {
        return 0;
    }
    cctrl->field_303 = game.play_gameturn;
    MapCoord dist = get_2d_box_distance(&creatng->mappos, &follwr_pos);
    int speed = get_creature_speed(leadtng);
    // If we're too far from the designated position, do a speed run
    if (dist > subtile_coord(12,0))
    {
        speed = 2 * speed;
        if (speed >= MAX_VELOCITY)
            speed = MAX_VELOCITY;
        if (creature_move_to(creatng, &follwr_pos, speed, 0, 0) == -1)
        {
          cctrl->field_307++;
          return 0;
        }
    } else
    // If we're far from the designated position, move considerably faster
    if (dist > subtile_coord(6,0))
    {
        if (speed > 4) {
            speed = 5 * speed / 4;
        } else {
            speed = speed + 1;
        }
        if (speed >= MAX_VELOCITY)
            speed = MAX_VELOCITY;
        if (creature_move_to(creatng, &follwr_pos, speed, 0, 0) == -1)
        {
          cctrl->field_307++;
          return 0;
        }
    } else
    // If we're close, continue moving at normal speed
    if (dist <= subtile_coord(2,0))
    {
        if (dist <= 0)
        {
            creature_turn_to_face(creatng, &leadtng->mappos);
        } else
        if (creature_move_to(creatng, &follwr_pos, speed, 0, 0) == -1)
        {
            cctrl->field_307++;
            return 0;
        }
    } else
    // If we're in between, move just a bit faster than leader
    {
        if (speed > 8) {
            speed = 9 * speed / 8;
        } else {
            speed = speed + 1;
        }
        if (speed >= MAX_VELOCITY)
            speed = MAX_VELOCITY;
        if (creature_move_to(creatng, &follwr_pos, speed, 0, 0) == -1)
        {
            cctrl->field_307++;
            return 0;
        }
    }
    cctrl->field_307 = 0;
    return 0;
}

short creature_in_hold_audience(struct Thing *creatng)
{
    TRACE_THING(creatng);
    int speed = get_creature_speed(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->turns_at_job == -1) && (cctrl->instance_id == CrInst_NULL))
    {
        struct Room* room = get_room_thing_is_on(creatng);
        if (room_is_invalid(room)) {
            return 1;
        }
        struct Coord3d pos;
        if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos))
        {
            return 1;
        }
        setup_person_move_to_coord(creatng, &pos, NavRtF_Default);
        creatng->continue_state = CrSt_CreatureInHoldAudience;
        cctrl->turns_at_job = 0;
        return 1;
    }
    long ret = creature_move_to(creatng, &cctrl->moveto_pos, speed, cctrl->move_flags, 0);
    if (ret != 1)
    {
        if (ret == -1)
        {
            ERRORLOG("Hope we never get here...");
            set_start_state(creatng);
            return 0;
        }
        return 1;
    }
    if (cctrl->turns_at_job != 0)
    {
        if ((cctrl->turns_at_job != 1) || (cctrl->instance_id != CrInst_NULL))
            return 1;
        cctrl->turns_at_job = -1;
    } else
    {
        cctrl->turns_at_job = 1;
        set_creature_instance(creatng, CrInst_CELEBRATE_SHORT, 1, 0, 0);
    }
    return 1;
}

short creature_kill_creatures(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    if (dungeon->num_active_creatrs <= 1) {
        set_start_state(creatng);
        return 0;
    }
    long crtr_idx = CREATURE_RANDOM(creatng, dungeon->num_active_creatrs);
    struct Thing* thing = get_player_list_nth_creature_of_model(dungeon->creatr_list_start, 0, crtr_idx);
    if (thing_is_invalid(thing)) {
        set_start_state(creatng);
        return 0;
    }
    if (setup_person_move_to_coord(creatng, &thing->mappos, NavRtF_Default)) {
        creatng->continue_state = CrSt_CreatureKillCreatures;
    }
    return 1;
}

short creature_leaves(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Room* room = get_room_thing_is_on(creatng);
    if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_CrPoolLeave) || (room->owner != creatng->owner))
    {
        internal_set_thing_state(creatng, CrSt_CreatureLeavingDungeon);
        return 1;
    }
    if (!is_neutral_thing(creatng))
    {
        struct Dungeon* dungeon = get_dungeon(creatng->owner);
        dungeon->total_creatures_left++;
        struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
        apply_anger_to_all_players_creatures_excluding(creatng->owner, crstat->annoy_others_leaving, AngR_Other, creatng);
    }
    kill_creature(creatng, INVALID_THING, -1, CrDed_NoEffects||CrDed_NotReallyDying);
    return CrStRet_Deleted;
}

short setup_creature_leaves_or_dies(struct Thing *creatng)
{
    TRACE_THING(creatng);
    // Try heading for nearest entrance
    struct Room* room = find_nearest_room_for_thing(creatng, creatng->owner, RoK_ENTRANCE, NavRtF_Default);
    if (room_is_invalid(room))
    {
        kill_creature(creatng, INVALID_THING, -1, CrDed_Default);
        return -1;
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, room, Job_EXEMPT, NavRtF_Default))
    {
        kill_creature(creatng, INVALID_THING, -1, CrDed_Default);
        return -1;
    }
    creatng->continue_state = CrSt_LeavesBecauseOwnerLost;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->flgfield_1 |= CCFlg_NoCompControl;
    return 1;
}

short creature_leaves_or_dies(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(9,"Starting for %s index %d",thing_model_name(creatng),(int)creatng->index);
    // If we're on an entrance, then just leave the dungeon
    struct Room* room = get_room_thing_is_on(creatng);
    if (!room_is_invalid(room) && room_role_matches(room->kind, RoRoF_CrPoolLeave))
    {
        kill_creature(creatng, INVALID_THING, -1, CrDed_NoEffects);
        return -1;
    }
    // Otherwise, try heading for nearest entrance
    return setup_creature_leaves_or_dies(creatng);
}

short creature_leaving_dungeon(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Room* room = find_nearest_room_for_thing(creatng, creatng->owner, RoK_ENTRANCE, NavRtF_Default);
    if (room_is_invalid(room))
    {
        set_start_state(creatng);
        return 0;
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, room, Job_EXEMPT, NavRtF_Default))
    {
        set_start_state(creatng);
        return 0;
    }
    if (is_my_player_number(creatng->owner))
        output_message(SMsg_CreatureLeaving, MESSAGE_DELAY_CRTR_MOOD, 1);
    creatng->continue_state = CrSt_CreatureLeaves;
    return 1;
}

struct Thing *find_random_creature_for_persuade(PlayerNumber plyr_idx, struct Coord3d *pos)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    int n = PLAYER_RANDOM(plyr_idx, dungeon->num_active_creatrs);
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
        // Per thing code starts
        
        if ((n <= 0) )
        {
            if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody(thing)
                && !creature_is_being_unconscious(thing) && !creature_is_dying(thing))
            {
                return thing;
            }
        }
        n--;
        // Per thing code ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    return INVALID_THING;
}

TbBool make_creature_leave_dungeon(struct Thing *creatng)
{
    struct Room* room = find_nearest_room_for_thing(creatng, creatng->owner, RoK_ENTRANCE, NavRtF_Default);
    if (room_is_invalid(room)) {
        set_start_state(creatng);
        return false;
    }
    if (!internal_set_thing_state(creatng, CrSt_CreatureLeaves)) {
        set_start_state(creatng);
        return false;
    }
    if (!creature_setup_random_move_for_job_in_room(creatng, room, Job_EXEMPT, NavRtF_Default)) {
        set_start_state(creatng);
        return false;
    }
    if (is_my_player_number(creatng->owner))
        output_message(SMsg_CreatureLeaving, MESSAGE_DELAY_CRTR_MOOD, 1);
    creatng->continue_state = CrSt_CreatureLeaves;
    return true;
}

long make_all_creature_in_group_leave_dungeon(struct Thing *leadtng)
{
    if (!creature_is_group_member(leadtng)) {
        set_start_state(leadtng);
        return 0;
    }
    if (perform_action_on_all_creatures_in_group(leadtng, make_creature_leave_dungeon)) {
        // If there were problems while performing actions, make sure that it's performed for the leader
        return make_creature_leave_dungeon(leadtng);
    }
    return 1;
}

short creature_persuade(struct Thing *creatng)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
    if ((cctrl->byte_9A > 0) && (dungeon->num_active_creatrs > 1))
    {
        struct Thing* perstng = find_random_creature_for_persuade(creatng->owner, &creatng->mappos);
        if (setup_person_move_to_coord(creatng, &perstng->mappos, NavRtF_Default)) {
            creatng->continue_state = CrSt_CreaturePersuade;
        }
        return 1;
    }
    make_all_creature_in_group_leave_dungeon(creatng);
    return 0;
}

/**
 * Makes creature drag the specified object.
 * @param creatng The creature which is to start dragging thing.
 * @param dragtng The target object to be dragged.
 */
void creature_drag_object(struct Thing *creatng, struct Thing *dragtng)
{
    TRACE_THING(creatng);
    TRACE_THING(dragtng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->dragtng_idx = dragtng->index;
    dragtng->alloc_flags |= TAlF_IsDragged;
    dragtng->state_flags |= TF1_IsDragged1;
    dragtng->owner = game.neutral_player_num;
    if (dragtng->light_id != 0) {
      light_turn_light_off(dragtng->light_id);
    }
}

void creature_drop_dragged_object(struct Thing *creatng, struct Thing *dragtng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->dragtng_idx == 0)
    {
        ERRORLOG("The %s isn't dragging anything",thing_model_name(creatng));
    } else
    if (dragtng->index != cctrl->dragtng_idx)
    {
        ERRORLOG("The %s isn't dragging %s",thing_model_name(creatng),thing_model_name(dragtng));
    }
    cctrl->dragtng_idx = 0;
    struct CreatureControl* dragctrl = creature_control_get_from_thing(dragtng);
    dragctrl->dragtng_idx = 0;
    dragtng->alloc_flags &= ~TAlF_IsDragged;
    dragtng->state_flags &= ~TF1_IsDragged1;
    move_thing_in_map(dragtng, &creatng->mappos);
    if (dragtng->light_id != 0) {
        light_turn_light_on(dragtng->light_id);
    }
}

TbBool creature_is_dragging_something(const struct Thing *creatng)
{
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->dragtng_idx == 0) {
        return false;
    }
    const struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
    if (!thing_exists(dragtng)) {
        ERRORLOG("The %s is dragging non-existing thing",thing_model_name(creatng));
        return false;
    }
    return true;
}

TbBool creature_is_dragging_spellbook(const struct Thing *creatng)
{
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->dragtng_idx == 0) {
        return false;
    }
    const struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
    if (!thing_exists(dragtng)) {
        ERRORLOG("The %s is dragging non-existing thing",thing_model_name(creatng));
        return false;
    }
    if (!thing_is_spellbook(dragtng)) {
        return false;
    }
    return true;
}

TbBool find_random_valid_position_for_thing_in_room_avoiding_object(struct Thing *thing, const struct Room *room, struct Coord3d *pos)
{
    int nav_sizexy = subtile_coord(thing_nav_block_sizexy(thing), 0);
    if (room_is_invalid(room) || (room->slabs_count <= 0)) {
        ERRORLOG("Invalid room or number of slabs is zero");
        return false;
    }
    long selected = CREATURE_RANDOM(thing, room->slabs_count);
    unsigned long n = 0;
    long i = room->slabs_list;
    // Get the selected index
    while (i != 0)
    {
        struct DungeonAdd *dungeonadd = get_dungeonadd(room->owner);
        if (dungeonadd->roomspace.is_active)
        {
            MapSlabCoord slb_x = slb_num_decode_x(i);
            MapSlabCoord slb_y = slb_num_decode_y(i);
            for (long j = 0; j < SMALL_AROUND_LENGTH; j++)
            {
                MapSlabCoord aslb_x = slb_x + small_around[j].delta_x;
                MapSlabCoord aslb_y = slb_y + small_around[j].delta_y;
                struct Room* nroom = slab_room_get(aslb_x, aslb_y);
                if (!room_is_invalid(nroom))
                {
                    if ( (nroom->kind == room->kind) && (nroom->owner == room->owner) )
                    {
                        if (nroom->index != room->index)
                        {
                            ERRORLOG("Tried to find free position in %s %d but ended up looking in %s %d instead", room_code_name(room->kind),room->index,room_code_name(nroom->kind), nroom->index);
                            return false;
                        }
                    }
                }
            }
        }
        // Per room tile code
        if (n >= selected)
        {
            break;
        }
        // Per room tile code ends
        i = get_next_slab_number_in_room(i);
        n++;
    }
    if (i == 0) 
    {
        if (n < room->slabs_count)
        {
            WARNLOG("Number of slabs in %s (%d) is smaller than count (%d)",room_code_name(room->kind), n, room->slabs_count);
        }
        n = 0;
        i = room->slabs_list;
    }
    // Sweep rooms starting on that index
    unsigned long k = 0;
    long nround;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    while (i != 0)
    {
        stl_x = slab_subtile(slb_num_decode_x(i), 0);
        stl_y = slab_subtile(slb_num_decode_y(i), 0);
        // Per room tile code
        MapSubtlCoord start_stl = CREATURE_RANDOM(thing, AROUND_TILES_COUNT);
        for (nround = 0; nround < AROUND_TILES_COUNT; nround++)
        {
            MapSubtlCoord x = start_stl % 3 + stl_x;
            MapSubtlCoord y = start_stl / 3 + stl_y;
            if (get_floor_filled_subtiles_at(x, y) == 1)
            {
                struct Thing* objtng = find_base_thing_on_mapwho(TCls_Object, 0, x, y);
                if (thing_is_invalid(objtng))
                {
                    pos->x.val = subtile_coord_center(x);
                    pos->y.val = subtile_coord_center(y);
                    pos->z.val = get_thing_height_at_with_radius(thing, pos, nav_sizexy);
                    if (!thing_in_wall_at_with_radius(thing, pos, nav_sizexy)) {
                        return true;
                    }
                }
            }
            start_stl = (start_stl + 1) % 9;
        }
        // Per room tile code ends
        if (n+1 >= room->slabs_count)
        {
            n = 0;
            i = room->slabs_list;
        } else {
            n++;
            i = get_next_slab_number_in_room(i);
        }
        k++;
        if (k > room->slabs_count) {
            ERRORLOG("Infinite loop detected when sweeping room slabs list");
            break;
        }
    }
    if (room->used_capacity <= room->total_capacity)
    {
        SYNCLOG("Could not find valid random point in %s %d for %s. Attempting thorough check.",room_code_name(room->kind),room->index,thing_model_name(thing));
        k = 0;
        for (i = room->slabs_list; (i != 0); i = get_next_slab_number_in_room(i))
        {
            stl_x = slab_subtile_center(slb_num_decode_x(i));
            stl_y = slab_subtile_center(slb_num_decode_y(i));
            for (nround = 0; nround < (AROUND_TILES_COUNT - 1); nround++)
            {
                MapSubtlCoord astl_x = stl_x + around[nround].delta_x;
                MapSubtlCoord astl_y = stl_y + around[nround].delta_y;
                if (get_floor_filled_subtiles_at(astl_x, astl_y) == 1)
                {
                    struct Thing* objtng = find_base_thing_on_mapwho(TCls_Object, 0, astl_x, astl_y);
                    if (thing_is_invalid(objtng))
                    {
                        pos->x.val = subtile_coord_center(astl_x);
                        pos->y.val = subtile_coord_center(astl_y);
                        pos->z.val = get_thing_height_at_with_radius(thing, pos, nav_sizexy);
                        if (!thing_in_wall_at_with_radius(thing, pos, nav_sizexy)) {
                            SYNCLOG("Thorough check succeeded where random check failed.");
                            return true;
                        }
                    }
                }
            }
            k++;
            if (k > room->slabs_count)
            {
                ERRORLOG("Infinite loop detected when sweeping room slabs list");
                break;
            }
        }
    }
    SYNCLOG("Could not find any valid point in %s %d for %s",room_code_name(room->kind),room->index,thing_model_name(thing));
    return false;
}

short creature_present_to_dungeon_heart(struct Thing *creatng)
{
    TRACE_THING(creatng);
    create_effect(&creatng->mappos, imp_spangle_effects[creatng->owner], creatng->owner);
    thing_play_sample(creatng, 76, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    if ( !external_set_thing_state(creatng, CrSt_CreatureDoingNothing) )
      set_start_state(creatng);
    return 1;
}

short creature_pretend_chicken_move(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->stateblock_flags & CCSpl_ChickenRel) != 0)
    {
        return 1;
    }
    long speed = get_creature_speed(creatng);
    long move_ret = creature_move_to(creatng, &cctrl->moveto_pos, speed, cctrl->move_flags, 0);
    if (move_ret == 1)
    {
        internal_set_thing_state(creatng, CrSt_CreaturePretendChickenSetupMove);
    } else
    if (move_ret == -1)
    {
        internal_set_thing_state(creatng, CrSt_CreaturePretendChickenSetupMove);
    }
    return 1;
}

short creature_pretend_chicken_setup_move(struct Thing *creatng)
{
  return _DK_creature_pretend_chicken_setup_move(creatng);
}

/**
 * Gives a gold hoard in a room which can be accessed by given creature.
 * @param creatng The creature which should be able to access the object.
 * @param room Room in which the object lies.
 * @return The goald hoard thing, or invalid thing if not found.
 */
struct Thing *find_gold_hoarde_in_room_for_creature(struct Thing *creatng, struct Room *room)
{
    return find_object_in_room_for_creature_matching_bool_filter(creatng, room, thing_is_gold_hoard);
}

/**
 * Gives a spellbook in a room which can be accessed by given creature.
 * @param creatng The creature which should be able to access the spellbook.
 * @param room Room in which the spellbook lies.
 * @return The spellbook thing, or invalid thing if not found.
 */
struct Thing *find_spell_in_room_for_creature(struct Thing *creatng, struct Room *room)
{
    return find_object_in_room_for_creature_matching_bool_filter(creatng, room, thing_is_spellbook);
}

/**
 * State handler function for stealing gold.
 * Should be invoked when a thief arrives at enemy treasure room.
 * Searches for specific gold hoard to steal from and enters next
 * phase of stealing (CrSt_CreatureStealGold).
 * @param thing The creature who is stealing gold.
 * @return True on success, false if finding gold to steal failed.
 */
short creature_search_for_gold_to_steal_in_room(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Room* room = subtile_room_get(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_GoldStorage))
    {
        WARNLOG("Cannot steal gold - not on treasure room at (%d,%d)",(int)creatng->mappos.x.stl.num, (int)creatng->mappos.y.stl.num);
        set_start_state(creatng);
        return 0;
    }
    struct Thing* gldtng = find_gold_hoarde_in_room_for_creature(creatng, room);
    if (thing_is_invalid(gldtng))
    {
        WARNLOG("Cannot steal gold - no gold hoard found in treasure room");
        set_start_state(creatng);
        return 0;
    }
    if (!setup_person_move_to_coord(creatng, &gldtng->mappos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot move to gold at (%d,%d)",(int)gldtng->mappos.x.stl.num, (int)gldtng->mappos.y.stl.num);
    }
    creatng->continue_state = CrSt_CreatureStealGold;
    return 1;
}

short creature_search_for_spell_to_steal_in_room(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Room* room = subtile_room_get(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    if (room_is_invalid(room) || (room->kind != RoK_LIBRARY))
    {
        WARNLOG("Cannot steal spell - not on library at (%d,%d)",
            (int)creatng->mappos.x.stl.num, (int)creatng->mappos.y.stl.num);
        set_start_state(creatng);
        return 0;
    }
    struct Thing* spltng = find_spell_in_room_for_creature(creatng, room);
    if (thing_is_invalid(spltng))
    {
        WARNLOG("Cannot steal spell - no spellbook found in library");
        set_start_state(creatng);
        return 0;
    }
    cctrl->pickup_object_id = spltng->index;
    if (!setup_person_move_to_coord(creatng, &spltng->mappos, NavRtF_Default))
    {
        SYNCDBG(8,"Cannot move to spell at (%d,%d)",(int)spltng->mappos.x.stl.num, (int)spltng->mappos.y.stl.num);
    }
    creatng->continue_state = CrSt_CreatureStealSpell;
    return 1;
}

short creature_set_work_room_based_on_position(struct Thing *creatng)
{
    //return _DK_creature_set_work_room_based_on_position(thing);
    return 1;
}

TbBool init_creature_state(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(17,"Starting for %s index %d",thing_model_name(creatng),(long)creatng->index);
    long stl_x = creatng->mappos.x.stl.num;
    long stl_y = creatng->mappos.y.stl.num;
    // Set creature to default state, in case giving it job will fail
    set_start_state(creatng);
    // Check job which we can do after dropping at these coordinates
    if (is_neutral_thing(creatng))
    {
        SYNCDBG(3,"Not assigning initial job at (%d,%d) for neutral %s index %d owner %d",(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    CreatureJob new_job = get_job_for_subtile(creatng, stl_x, stl_y, JoKF_AssignCeatureInit);
    if (new_job == Job_NULL)
    {
        SYNCDBG(3,"No job found at (%d,%d) for %s index %d owner %d",(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // Check if specific conditions are met for this job to be assigned
    if (!creature_can_do_job_near_position(creatng, stl_x, stl_y, new_job, JobChk_None))
    {
        SYNCDBG(3,"Cannot assign %s at (%d,%d) to %s index %d owner %d; checked and got refusal",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    // Now try sending the creature to do job it should do at this position
    if (!send_creature_to_job_near_position(creatng, stl_x, stl_y, new_job))
    {
        WARNDBG(3,"Cannot assign %s at (%d,%d) to %s index %d owner %d; could not send to job",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        return false;
    }
    SYNCDBG(3,"Job %s at (%d,%d) assigned to %s index %d owner %d",creature_job_code_name(new_job),(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    // If applicable, set the job as assigned job for the creature
    if ((get_flags_for_job(new_job) & JoKF_AssignOneTime) == 0) {
        set_creature_assigned_job(creatng, new_job);
    } else {
        // One-time jobs are not assigned to the creature, they are just initialized to be performed once
        //set_creature_assigned_job(creatng, Job_NULL); -- already assigned
    }
    return true;
}

TbBool restore_backup_state(struct Thing *creatng, CrtrStateId active_state, CrtrStateId continue_state)
{
    struct StateInfo* active_stati = &states[active_state];
    struct StateInfo* continue_stati = &states[continue_state];
    if ((active_stati->cleanup_state != NULL) || ((continue_state != CrSt_Unused) && (continue_stati->cleanup_state != NULL)))
    {
        if ((active_state == CrSt_CreatureInPrison) || (active_state == CrSt_Torturing)
          || (continue_state == CrSt_CreatureInPrison) || (continue_state == CrSt_Torturing))
        {
            init_creature_state(creatng);
        } else
        {
            set_start_state(creatng);
        }
        return true;
    } else
    {
        internal_set_thing_state(creatng, active_state);
        creatng->continue_state = continue_state;
        return true;
    }
}

short creature_slap_cowers(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    TRACE_THING(creatng);
    if (cctrl->field_27F > 0) {
        cctrl->field_27F--;
    }
    if (cctrl->field_27F > 0) {
        return 0;
    }
    restore_backup_state(creatng, cctrl->active_state_bkp, cctrl->continue_state_bkp);
    cctrl->field_35 = 0;
    return 1;
}

short creature_steal_gold(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    struct Room* room = get_room_thing_is_on(creatng);
    if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_GoldStorage))
    {
        WARNLOG("Cannot steal gold - not on treasure room at (%d,%d)",
            (int)creatng->mappos.x.stl.num, (int)creatng->mappos.y.stl.num);
        set_start_state(creatng);
        return 0;
    }
    struct Thing* hrdtng = find_gold_hoard_at(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    if (thing_is_invalid(hrdtng))
    {
        WARNLOG("Cannot steal gold - no gold hoard at (%d,%d)",
            (int)creatng->mappos.x.stl.num, (int)creatng->mappos.y.stl.num);
        set_start_state(creatng);
        return 0;
    }
    long max_amount = crstat->gold_hold - creatng->creature.gold_carried;
    if (max_amount <= 0)
    {
        set_start_state(creatng);
        return 0;
    }
    // Success! we are able to steal some gold!
    long amount = remove_gold_from_hoarde(hrdtng, room, max_amount);
    creatng->creature.gold_carried += amount;
    create_price_effect(&creatng->mappos, creatng->owner, amount);
    SYNCDBG(6,"Stolen %d gold from hoard at (%d,%d)",(int)amount, (int)creatng->mappos.x.stl.num, (int)creatng->mappos.y.stl.num);
    set_start_state(creatng);
    return 0;
}

/**
 * Steals spell or special form the library.
 * @param creatng
 */
short creature_pick_up_spell_to_steal(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(18,"Starting");
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* picktng = thing_get(cctrl->pickup_object_id);
    TRACE_THING(picktng);
    if ( thing_is_invalid(picktng) || ((picktng->state_flags & TF1_IsDragged1) != 0)
      || (get_2d_box_distance(&creatng->mappos, &picktng->mappos) >= 512))
    {
        set_start_state(creatng);
        return 0;
    }
    struct Room* room = get_room_thing_is_on(picktng);
    // Check if we're stealing the spell from a library
    if (!room_is_invalid(room))
    {
        remove_spell_from_library(room, picktng, creatng->owner);
    }
    // Create event to inform player about the spell or special (need to be done before pickup due to ownership changes)
    update_library_object_pickup_event(creatng, picktng);
    creature_drag_object(creatng, picktng);
    if (!good_setup_wander_to_exit(creatng)) {
        set_start_state(creatng);
        return 0;
    }
    return 1;
}

short creature_take_salary(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(18,"Starting");
    if (!thing_is_on_own_room_tile(creatng))
    {
        internal_set_thing_state(creatng, CrSt_CreatureWantsSalary);
        return 1;
    }
    struct Room* room = get_room_thing_is_on(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    if (room_is_invalid(room) || !room_role_matches(room->kind, RoRoF_GoldStorage) ||
      ((room->used_capacity <= 0) && (dungeon->offmap_money_owned <= 0)))
    {
        internal_set_thing_state(creatng, CrSt_CreatureWantsSalary);
        return 1;
    }
    GoldAmount salary = calculate_correct_creature_pay(creatng);
    GoldAmount received = take_money_from_dungeon(creatng->owner, salary, 0);
    if (received < 1) {
        ERRORLOG("The %s index %d has used capacity %d but no gold for %s salary",room_code_name(room->kind),
            (int)room->index,(int)room->used_capacity,thing_model_name(creatng));
        internal_set_thing_state(creatng, CrSt_CreatureWantsSalary);
        return 1;
    }
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        if (cctrl->paydays_owed > 0)
            cctrl->paydays_owed--;
    }
    set_start_state(creatng);
    {
        struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
        struct Thing* efftng = create_price_effect(&creatng->mappos, creatng->owner, salary);
        if (!(gameadd.classic_bugs_flags & ClscBug_FullyHappyWithGold))
        {
            anger_apply_anger_to_creature_all_types(creatng, crstat->annoy_got_wage);
        }
        thing_play_sample(efftng, 32, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    dungeon->lvstats.salary_cost += salary;
    return 1;
}

void stop_creature_being_dragged_by(struct Thing *dragtng, struct Thing *creatng)
{
    TRACE_THING(creatng);
    TRACE_THING(dragtng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (!creature_control_invalid(cctrl)) {
        if (cctrl->dragtng_idx <= 0) {
            WARNLOG("The %s is not dragging something",thing_model_name(creatng));
        }
        cctrl->dragtng_idx = 0;
    } else {
        ERRORLOG("The %s has no valid control structure",thing_model_name(creatng));
    }
    struct CreatureControl* dragctrl = creature_control_get_from_thing(dragtng);
    if (dragctrl->dragtng_idx <= 0) {
        WARNLOG("The %s is not dragged by something",thing_model_name(dragtng));
    }
    dragtng->state_flags &= ~TF1_IsDragged1;
    dragctrl->dragtng_idx = 0;
}

void make_creature_unconscious(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(18,"Starting");
    clear_creature_instance(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (gameadd.classic_bugs_flags & ClscBug_ResurrectRemoved)
    {
        // If the classic bug is enabled, fainted units are also added to resurrect creature.
        update_dead_creatures_list_for_owner(creatng);
    }
    creatng->active_state = CrSt_CreatureUnconscious;
    cctrl->flgfield_1 |= CCFlg_PreventDamage;
    cctrl->flgfield_1 |= CCFlg_NoCompControl;
    cctrl->conscious_back_turns = gameadd.game_turns_unconscious;
}

void make_creature_conscious_without_changing_state(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(18,"Starting");
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->flgfield_1 &= ~CCFlg_PreventDamage;
    cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
    cctrl->conscious_back_turns = 0;
    if ((creatng->state_flags & TF1_IsDragged1) != 0)
    {
        struct Thing* sectng = thing_get(cctrl->dragtng_idx);
        TRACE_THING(sectng);
        stop_creature_being_dragged_by(creatng, sectng);
    }
}

void make_creature_conscious(struct Thing *creatng)
{
    make_creature_conscious_without_changing_state(creatng);
    set_start_state(creatng);
}

short creature_unconscious(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(18,"Starting");
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->conscious_back_turns > 0)
        cctrl->conscious_back_turns--;
    if (cctrl->conscious_back_turns > 0)
    {
        return 0;
    }
    make_creature_conscious(creatng);
    return 1;
}

short creature_vandalise_rooms(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(18,"Starting");
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->target_room_id = 0;
    struct Room* room = get_room_thing_is_on(creatng);
    if (room_is_invalid(room))
    {
      set_start_state(creatng);
      return 0;
    }
    if (room_cannot_vandalise(room->kind))
    {
        return 1;
    }
    if (cctrl->instance_id == CrInst_NULL)
    {
        set_creature_instance(creatng, CrInst_ATTACK_ROOM_SLAB, 1, 0, 0);
    }
    return 1;
}

TbBool is_creature_other_than_given_waiting_at_closed_door_on_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, const struct Thing *besidetng)
{
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
        // Per-thing code start
        if ((thing->index != besidetng->index) && thing_is_creature(thing))
        {
            CrtrStateId crstate = get_creature_state_besides_interruptions(thing);
            if (crstate == CrSt_CreatureWaitAtTreasureRoomDoor) {
                return true;
            }
        }
        // Per-thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return false;
}

short creature_wait_at_treasure_room_door(struct Thing *creatng)
{
    //return _DK_creature_wait_at_treasure_room_door(creatng);
    MapSubtlCoord base_stl_x = creatng->mappos.x.stl.num;
    MapSubtlCoord base_stl_y = creatng->mappos.y.stl.num;
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    anger_apply_anger_to_creature(creatng, crstat->annoy_queue, AngR_NotPaid, 1);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing *doortng;
    if (cctrl->blocking_door_id > 0) {
        doortng = thing_get(cctrl->blocking_door_id);
    } else {
        doortng = INVALID_THING;
    }
    if (!thing_is_deployed_door(doortng) || ((game.play_gameturn - doortng->creation_turn) <= 3) || !doortng->door.is_locked)
    {
        internal_set_thing_state(creatng, CrSt_CreatureWantsSalary);
        return 1;
    }
    EventIndex evidx = event_create_event_or_update_nearby_existing_event(creatng->mappos.x.val, creatng->mappos.y.val, EvKind_WorkRoomUnreachable, creatng->owner, RoK_TREASURE);
    if (evidx > 0) 
    {
        output_message_room_related_from_computer_or_player_action(creatng->owner, RoK_TREASURE, OMsg_RoomNoRoute);
    }
    if (is_creature_other_than_given_waiting_at_closed_door_on_subtile(base_stl_x, base_stl_y, creatng))
    {
        int i = 0;
        int n = CREATURE_RANDOM(creatng, SMALL_AROUND_SLAB_LENGTH);
        for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
        {
            MapSubtlCoord stl_x = base_stl_x + small_around[n].delta_x;
            MapSubtlCoord stl_y = base_stl_y + small_around[n].delta_y;
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if (((mapblk->flags & SlbAtFlg_Blocking) == 0) && !terrain_toxic_for_creature_at_position(creatng, stl_x, stl_y))
            {
                if (setup_person_move_to_position(creatng, stl_x, stl_y, 0) ) {
                    creatng->continue_state = CrSt_CreatureWaitAtTreasureRoomDoor;
                    return 1;
                }
            }
            n = (n+1) % SMALL_AROUND_SLAB_LENGTH;
        }
        ERRORLOG("Could not find position to wait");
    }
    creature_turn_to_face(creatng, &doortng->mappos);
    return 0;
}

short creature_wants_a_home(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if (crstat->lair_size <= 0) {
        set_start_state(creatng);
        return 0;
    }
    if ((cctrl->lair_room_id <= 0) || (cctrl->lairtng_idx <= 0)) {
        internal_set_thing_state(creatng, CrSt_CreatureChooseRoomForLairSite);
        return 0;
    }
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    if (thing_exists(lairtng))
    {
        if (setup_person_move_to_coord(creatng, &lairtng->mappos, NavRtF_Default) == 1)
        {
            creatng->continue_state = CrSt_CreatureDoingNothing;
            return 1;
        }
    }
    internal_set_thing_state(creatng, CrSt_CreatureChooseRoomForLairSite);
    return 0;
}

short creature_wants_salary(struct Thing *creatng)
{
    SYNCDBG(8,"Starting for %s index %d owner %d", thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Room* room = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, RoK_TREASURE, NavRtF_Default, 1);
    if (!room_is_invalid(room))
    {
        if (creature_setup_random_move_for_job_in_room(creatng, room, Job_TAKE_SALARY, NavRtF_Default))
        {
            creatng->continue_state = CrSt_CreatureTakeSalary;
            cctrl->target_room_id = room->index;
            return 1;
        }
    } else {
        room = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, RoK_TREASURE, NavRtF_NoOwner, 1);
    }
    if (room_is_invalid(room))
    {
        SYNCDBG(5,"No player %d %s with used capacity found to pay %s",(int)creatng->owner,room_code_name(RoK_TREASURE),thing_model_name(creatng));
        if (cctrl->paydays_owed > 0)
        {
            cctrl->paydays_owed--;
            struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
            anger_apply_anger_to_creature(creatng, crstat->annoy_no_salary, AngR_NotPaid, 1);
        }
        set_start_state(creatng);
        return 1;
    }
    struct Coord3d pos;
    if (find_random_valid_position_for_thing_in_room(creatng, room, &pos))
    {
        if (setup_person_move_to_coord(creatng, &pos, NavRtF_NoOwner))
        {
            creatng->continue_state = CrSt_CreatureTakeSalary;
            cctrl->target_room_id = room->index;
        }
    }
    return 1;
}

long setup_head_for_empty_treasure_space(struct Thing *thing, struct Room *room)
{
    SlabCodedCoords start_slbnum = room->slabs_list;
   
    //Find a random slab to start out with
    long n = CREATURE_RANDOM(thing, room->slabs_count);
    for (unsigned long k = n; k > 0; k--)
    {
        if (start_slbnum == 0)
        {
            break;
        }
        start_slbnum = get_next_slab_number_in_room(start_slbnum);
    }
    if (start_slbnum == 0) {
        ERRORLOG("Taking random slab (%d/%u) in %s index %u failed - internal inconsistency.", n, room->slabs_count, room_code_name(room->kind), room->index);
        start_slbnum = room->slabs_list;
    }
    
    SlabCodedCoords slbnum = start_slbnum;
    MapSlabCoord slb_x = slb_num_decode_x(slbnum);
    MapSlabCoord slb_y = slb_num_decode_y(slbnum);
    struct Thing* gldtng = find_gold_hoarde_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));

    // If the random slab has enough space to drop all gold, go there to drop it
    long wealth_size_holds = gameadd.gold_per_hoard / get_wealth_size_types_count();
    GoldAmount max_hoard_size_in_room = wealth_size_holds * room->total_capacity / room->slabs_count;
    if((max_hoard_size_in_room - gldtng->valuable.gold_stored) >= thing->creature.gold_carried)
    {
        if (setup_person_move_to_position(thing, slab_subtile_center(slb_x), slab_subtile_center(slb_y), NavRtF_Default))
        {
            return 1;
        }
    }

    //If not, find a slab with the lowest amount of gold
    GoldAmount gold_amount = gldtng->valuable.gold_stored;
    GoldAmount min_gold_amount = gldtng->valuable.gold_stored;
    SlabCodedCoords slbmin = start_slbnum;
    for (long i = room->slabs_count; i > 0; i--)
    { 
        slb_x = slb_num_decode_x(slbnum);
        slb_y = slb_num_decode_y(slbnum);
        gldtng = find_gold_hoarde_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        gold_amount = gldtng->valuable.gold_stored;
        if (gold_amount <= 0) //Any empty slab will do
        {
            slbmin = slbnum;
            break;
        }
        if (gold_amount <= min_gold_amount)
        { 
            min_gold_amount = gold_amount;
            slbmin = slbnum;
        }
        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0) 
        {
            slbnum = room->slabs_list;
        }
        
    }
    
    //Send imp to slab with lowest amount on it
    slb_x = slb_num_decode_x(slbmin);
    slb_y = slb_num_decode_y(slbmin);
    if (setup_person_move_to_position(thing, slab_subtile_center(slb_x), slab_subtile_center(slb_y), NavRtF_Default))
    {
        return 1;
    }
    return 0;
}

void place_thing_in_creature_controlled_limbo(struct Thing *thing)
{
    remove_thing_from_mapwho(thing);
    thing->field_4F |= TF4F_Unknown01;
    thing->state_flags |= TF1_InCtrldLimbo;
}

void remove_thing_from_creature_controlled_limbo(struct Thing *thing)
{
    thing->state_flags &= ~TF1_InCtrldLimbo;
    thing->field_4F &= ~TF4F_Unknown01;
    place_thing_in_mapwho(thing);
}

short move_backwards_to_position(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    long speed = get_creature_speed(creatng);
    SYNCDBG(18,"Starting to move %s index %d into (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)cctrl->moveto_pos.x.stl.num,(int)cctrl->moveto_pos.y.stl.num);
    long move_result = creature_move_to(creatng, &cctrl->moveto_pos, speed, cctrl->move_flags, 1);
    if (move_result == 1)
    {
        internal_set_thing_state(creatng, creatng->continue_state);
        creatng->continue_state = CrSt_Unused;
        return 1;
    }
    if (move_result == -1)
    {
        ERRORLOG("Bad place (%d,%d) to move %s backwards to.",
            (int)cctrl->moveto_pos.x.val,(int)cctrl->moveto_pos.y.val,thing_model_name(creatng));
        set_start_state(creatng);
        creatng->continue_state = CrSt_Unused;
        return 0;
    }
    return 0;
}

CrCheckRet move_check_attack_any_door(struct Thing *creatng)
{
    SYNCDBG(18,"Starting for %s",thing_model_name(creatng));
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->collided_door_subtile == 0) {
        SYNCDBG(18,"No door collision with %s",thing_model_name(creatng));
        return 0;
    }
    MapSubtlCoord stl_x = stl_num_decode_x(cctrl->collided_door_subtile);
    MapSubtlCoord stl_y = stl_num_decode_y(cctrl->collided_door_subtile);
    SYNCDBG(8,"Door at (%d,%d) collided with %s",(int)stl_x,(int)stl_y,thing_model_name(creatng));
    struct Thing* doortng = get_door_for_position(stl_x, stl_y);
    if (thing_is_invalid(doortng)) {
        SYNCDBG(8,"Door collided with %s not found",thing_model_name(creatng));
        return 0;
    }
    set_creature_door_combat(creatng, doortng);
    return 1;
}

CrCheckRet move_check_can_damage_wall(struct Thing *creatng)
{
  return _DK_move_check_can_damage_wall(creatng);
}

CrAttackType creature_can_have_combat_with_creature_on_slab(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y, struct Thing ** enemytng)
{
    MapSubtlCoord endstl_x = slab_subtile(slb_x + 1, 0);
    MapSubtlCoord endstl_y = slab_subtile(slb_y + 1, 0);
    for (MapSubtlCoord stl_y = slab_subtile(slb_y, 0); stl_y < endstl_y; stl_y++)
    {
        for (MapSubtlCoord stl_x = slab_subtile(slb_x, 0); stl_x < endstl_x; stl_x++)
        {
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
                if ( thing_is_creature(thing) && (thing != creatng) )
                {
                    if ((get_creature_model_flags(thing) & CMF_IsSpecDigger) == 0)
                    {
                        long dist = get_combat_distance(creatng, thing);
                        CrAttackType attack_type = creature_can_have_combat_with_creature(creatng, thing, dist, 0, 0);
                        if (attack_type > AttckT_Unset) {
                            (*enemytng) = thing;
                            return attack_type;
                        }
                    }
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
        }
    }
    (*enemytng) = INVALID_THING;
    return 0;
}

CrCheckRet move_check_kill_creatures(struct Thing *creatng)
{
    MapSlabCoord slb_x = coord_slab(creatng->mappos.x.val);
    MapSlabCoord slb_y = coord_slab(creatng->mappos.y.val);
    struct Thing* enemytng;
    CrAttackType attack_type = creature_can_have_combat_with_creature_on_slab(creatng, slb_x, slb_y, &enemytng);
    if (attack_type > AttckT_Unset) {
        set_creature_in_combat_to_the_death(creatng, enemytng, attack_type);
        return CrCkRet_Continue;
    }
    return CrCkRet_Available;
}

CrCheckRet move_check_near_dungeon_heart(struct Thing *creatng)
{
    //return _DK_move_check_near_dungeon_heart(creatng);
    if (is_neutral_thing(creatng))
    {
        if (change_creature_owner_if_near_dungeon_heart(creatng)) {
            return 1;
        }
    }
    return 0;
}

CrCheckRet move_check_on_head_for_room(struct Thing *creatng)
{
  return _DK_move_check_on_head_for_room(creatng);
}

CrCheckRet move_check_persuade(struct Thing *creatng)
{
  return _DK_move_check_persuade(creatng);
}

CrCheckRet move_check_wait_at_door_for_wage(struct Thing *creatng)
{
  // return _DK_move_check_wait_at_door_for_wage(creatng);
  struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
  struct Thing *doortng;
  struct Room *room;
  if (cctrl->collided_door_subtile != 0)
  {
    doortng = get_door_for_position(stl_num_decode_x(cctrl->collided_door_subtile), stl_num_decode_y(cctrl->collided_door_subtile));
    if (!thing_is_invalid(doortng))
    {
      internal_set_thing_state(creatng, CrSt_CreatureWaitAtTreasureRoomDoor);
      cctrl->blocking_door_id = doortng->index;
      cctrl->collided_door_subtile = 0;
      return CrCkRet_Continue;
    }
  }
  else if ( cctrl->target_room_id != 0 )
  {
    room = get_room_thing_is_on(creatng);
    if (!room_is_invalid(room))
    {
      if ( room->index == cctrl->target_room_id )
      {
        internal_set_thing_state(creatng, creatng->continue_state);
        return CrCkRet_Continue;
      }
    }
  }
  return CrCkRet_Available;
}

char new_slab_tunneller_check_for_breaches(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);

    // NB: the code assumes PLAYERS_COUNT = DUNGEONS_COUNT
    for (int i = 0; i < PLAYERS_COUNT; ++i)
    {
        struct PlayerInfo* player = get_player(i);
        struct Dungeon* dgn = get_dungeon(i);
        if (!player_exists(player) || (player->is_active != 1))
            continue;

        if (!dgn->dnheart_idx)
            continue;

        if (cctrl->byte_8A & (1 << i))
            continue;

        if (!creature_can_navigate_to(
                creatng,
                &game.things.lookup[dgn->dnheart_idx]->mappos,
                0))
            continue;
        if (!((game.map[creatng->mappos.x.stl.num + (creatng->mappos.y.stl.num << 8)].data >> 28) & (1 << i)))
            continue;

        cctrl->byte_8A |= 1 << i;
        ++dgn->times_broken_into;
        event_create_event_or_update_nearby_existing_event(creatng->mappos.x.val, creatng->mappos.y.val, EvKind_Breach, i, 0);
        if (is_my_player_number(i))
        {
            output_message(SMsg_WallsBreach, 0, 1);
        }
    }
    return 0;
}

TbBool go_to_random_area_near_xy(struct Thing *creatng, MapSubtlCoord bstl_x, MapSubtlCoord bstl_y)
{
    for (int i = 0; i < 5; i++)
    {
        MapSubtlCoord stl_x = bstl_x + CREATURE_RANDOM(creatng, 5) - 2;
        MapSubtlCoord stl_y = bstl_y + CREATURE_RANDOM(creatng, 5) - 2;
        if (setup_person_move_to_position(creatng, stl_x, stl_y, 0)) {
            return true;
        }
    }
    return false;
}

short patrol_here(struct Thing *creatng)
{
    //return _DK_patrol_here(creatng);
    MapSubtlCoord bstl_y = creatng->mappos.y.stl.num;
    MapSubtlCoord bstl_x = creatng->mappos.x.stl.num;
    if (!go_to_random_area_near_xy(creatng, bstl_x, bstl_y))
    {
        set_start_state(creatng);
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->patrol.word_89 = 10;
    cctrl->patrol.word_8B = cctrl->moveto_pos.x.stl.num;
    cctrl->patrol.word_8D = cctrl->moveto_pos.y.stl.num;
    creatng->continue_state = CrSt_Patrolling;
    return 1;
}

short patrolling(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->patrol.word_89 <= 0) {
        set_start_state(creatng);
        return 0;
    }
    cctrl->patrol.word_89--;
    // Try random positions near the patrolling point
    MapSubtlCoord stl_x = cctrl->patrol.word_8B;
    MapSubtlCoord stl_y = cctrl->patrol.word_8D;
    if (go_to_random_area_near_xy(creatng, stl_x, stl_y))
    {
        creatng->continue_state = CrSt_Patrolling;
        return 1;
    }
    // Try the exact patrolling point
    if (setup_person_move_to_position(creatng, stl_x, stl_y, 0))
    {
        creatng->continue_state = CrSt_Patrolling;
        return 1;
    }
    // Cannot patrol any longer - reset
    set_start_state(creatng);
    return 0;
}

short person_sulk_at_lair(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    if (creature_affected_by_slap(creatng) || player_uses_power_obey(creatng->owner) || !thing_exists(lairtng)) {
        set_start_state(creatng);
        return 0;
    }
    MapSubtlDelta dx = abs(creatng->mappos.x.stl.num - (MapSubtlDelta)lairtng->mappos.x.stl.num);
    MapSubtlDelta dy = abs(creatng->mappos.y.stl.num - (MapSubtlDelta)lairtng->mappos.y.stl.num);
    if ((dx >= 1) || (dy >= 1)) {
        set_start_state(creatng);
        return 0;
    }

    struct Room* room = get_room_thing_is_on(creatng);
    // Usually we use creature_job_in_room_no_longer_possible() for checking rooms
    // but sulking in lair is a special case, we can't compare room id as it's not working in room
    if (!room_still_valid_as_type_for_thing(room, RoK_LAIR, creatng))
    {
        WARNLOG("Room %s index %d is not valid %s for %s owned by player %d to work in",
            room_code_name(room->kind),(int)room->index,room_code_name(RoK_LAIR),
            thing_model_name(creatng),(int)creatng->owner);
        set_start_state(creatng);
        return 0;
    }
    if (!anger_is_creature_angry(creatng)) {
        set_start_state(creatng);
        return 0;
    }
    process_lair_enemy(creatng, room);
    internal_set_thing_state(creatng, CrSt_PersonSulking);
    cctrl->turns_at_job = 0;
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    anger_apply_anger_to_creature_all_types(creatng, crstat->annoy_sulking);
    return 1;
}

short person_sulk_head_for_lair(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    if (thing_exists(lairtng))
    {
        if (setup_person_move_to_coord(creatng, &lairtng->mappos, NavRtF_Default) == 1) {
            creatng->continue_state = CrSt_PersonSulkAtLair;
            return 1;
        }
    }
    // For some reason we can't go to lair; either leave dungeon o reset.
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    if ((crstat->lair_size <= 0) || creature_affected_by_slap(creatng) || player_uses_power_obey(creatng->owner)) {
        set_start_state(creatng);
        return 0;
    }
    internal_set_thing_state(creatng, CrSt_CreatureLeavingDungeon);
    return 0;
}

short person_sulking(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    if (creature_affected_by_slap(creatng) || player_uses_power_obey(creatng->owner) || !thing_exists(lairtng)) {
        set_start_state(creatng);
        return 0;
    }
    MapSubtlDelta dx = abs(creatng->mappos.x.stl.num - (MapSubtlDelta)lairtng->mappos.x.stl.num);
    MapSubtlDelta dy = abs(creatng->mappos.y.stl.num - (MapSubtlDelta)lairtng->mappos.y.stl.num);
    if ((dx >= 1) || (dy >= 1)) {
        set_start_state(creatng);
        return 0;
    }
    struct Room* room = get_room_thing_is_on(creatng);
    if (creature_job_in_room_no_longer_possible(room, Job_TAKE_SLEEP, creatng)) {
        set_start_state(creatng);
        return 0;
    }
    if (!anger_is_creature_angry(creatng)) {
        set_start_state(creatng);
        return 0;
    }
    process_lair_enemy(creatng, room);
    cctrl->turns_at_job++;
    if (cctrl->turns_at_job - 200 > 0)
    {
        if ((cctrl->turns_at_job % 32) == 0) {
            play_creature_sound(creatng, 4, 2, 0);
        }
        if (cctrl->turns_at_job - 250 >= 0) {
          cctrl->turns_at_job = 0;
        } else
        if (cctrl->instance_id == CrInst_NULL) {
            set_creature_instance(creatng, CrInst_MOAN, 1, 0, 0);
        }
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    anger_apply_anger_to_creature_all_types(creatng, crstat->annoy_sulking);
    return 1;
}

/**
 * Returns if the room is a valid place for a thing which hasn't yet started working in it.
 * Used to check if creatures can start working in specific rooms.
 * @param room The work room to be checked.
 * @param rkind Room kind required for work.
 * @param thing The thing which seeks for work room.
 * @return True if the room can be used, false otherwise.
 */
TbBool room_initially_valid_as_type_for_thing(const struct Room *room, RoomKind rkind, const struct Thing *thing)
{
    if (!room_exists(room)) {
        return false;
    }
    if (room->kind != rkind) {
        return false;
    }
    return ((room->owner == thing->owner) || enemies_may_work_in_room(room->kind));
}

/**
 * Returns if the room is a valid place for a thing for thing which is already working in that room.
 * Used to check if creatures are working in correct rooms.
 * @param room The work room to be checked.
 * @param rkind Room kind required for work.
 * @param thing The thing which is working in the room.
 * @return True if the room can still be used, false otherwise.
 */
TbBool room_still_valid_as_type_for_thing(const struct Room *room, RoomKind rkind, const struct Thing *thing)
{
    if (!room_exists(room)) {
        return false;
    }
    if (room->kind != rkind) {
        return false;
    }
    return ((room->owner == thing->owner) || enemies_may_work_in_room(room->kind));
}

/**
 * Returns if it's no longer possible for a creature to work in given room.
 * Used to check if creatures are able to continue working in the rooms they're working.
 * @param room The work room to be checked, usually the one creature stands on.
 * @param rkind Room kind required for work.
 * @param thing The thing which is working in the room.
 * @return True if the room can still be used, false otherwise.
 */
TbBool creature_job_in_room_no_longer_possible_f(const struct Room *room, CreatureJob jobpref, const struct Thing *thing, const char *func_name)
{
    RoomKind rkind = get_room_for_job(jobpref);
    if (!room_exists(room))
    {
        SYNCLOG("%s: The %s owned by player %d can no longer work in %s because former work room doesn't exist",
            func_name,thing_model_name(thing),(int)thing->owner,room_code_name(rkind));
        // Note that if given room doesn't exist, it do not mean this
        return true;
    }
    if (!room_still_valid_as_type_for_thing(room, rkind, thing))
    {
        WARNLOG("%s: Room %s index %d is not valid %s for %s owned by player %d to work in",
            func_name,room_code_name(room->kind),(int)room->index,room_code_name(rkind),
            thing_model_name(thing),(int)thing->owner);
        return true;
    }
    if (!creature_is_working_in_room(thing, room))
    {
        // This is not an error, because room index is often changed, ie. when room is expanded or its slab sold
        SYNCDBG(2,"%s: Room %s index %d is not the %s which %s owned by player %d selected to work in",
            func_name,room_code_name(room->kind),(int)room->index,room_code_name(rkind),
            thing_model_name(thing),(int)thing->owner);
        return true;
    }
    return false;
}

void create_effect_around_thing(struct Thing *thing, long eff_kind)
{
    //_DK_create_effect_around_thing(thing, eff_kind);
    int tng_radius = (thing->clipbox_size_xy >> 1);
    MapCoord coord_x_beg = (MapCoord)thing->mappos.x.val - tng_radius;
    if (coord_x_beg < 0)
        coord_x_beg = 0;
    MapCoord coord_x_end = (MapCoord)thing->mappos.x.val + tng_radius;
    if (coord_x_end >= subtile_coord(map_subtiles_x+1, 0) - 1)
        coord_x_end = subtile_coord(map_subtiles_x+1, 0) - 1;
    MapCoord coord_y_beg = (MapCoord)thing->mappos.y.val - tng_radius;
    if (coord_y_beg < 0)
        coord_y_beg = 0;
    MapCoord coord_y_end = (MapCoord)thing->mappos.y.val + tng_radius;
    if (coord_y_end >= subtile_coord(map_subtiles_y+1, 0) - 1)
        coord_y_end = subtile_coord(map_subtiles_y+1, 0) - 1;
    MapCoord coord_z_beg = (MapCoord)thing->mappos.z.val;
    if (coord_z_beg < 0)
        coord_z_beg = 0;
    MapCoord coord_z_end = (MapCoord)thing->mappos.z.val + thing->clipbox_size_yz;
    if (coord_z_end >= subtile_coord(map_subtiles_z, 0) - 1)
        coord_z_end = subtile_coord(map_subtiles_z, 0) - 1;
    struct Coord3d pos;
    pos.x.val = coord_x_beg;
    while (pos.x.val <= coord_x_end)
    {
        pos.y.val = coord_y_beg;
        while (pos.y.val <= coord_y_end)
        {
            pos.z.val = coord_z_beg;
            while (pos.z.val <= coord_z_end)
            {
                create_effect(&pos, eff_kind, thing->owner);
                pos.z.val += COORD_PER_STL;
            }
            pos.y.val += COORD_PER_STL;
        }
        pos.x.val += COORD_PER_STL;
    }
}

void remove_health_from_thing_and_display_health(struct Thing *thing, long delta)
{
    if ((thing->health >= 0) && (delta > 0))
    {
        thing->creature.health_bar_turns = 8;
        thing->health -= delta;
    }
}

/**
 * Returns if given slab has an adjacent slab owned by given player.
 * @param plyr_idx
 * @param slb_x
 * @param slb_y
 * @return
 */
TbBool slab_by_players_land(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        long aslb_x = slb_x + (long)small_around[n].delta_x;
        long aslb_y = slb_y + (long)small_around[n].delta_y;
        struct SlabMap* slb = get_slabmap_block(aslb_x, aslb_y);
        if (slabmap_owner(slb) == plyr_idx)
        {
            if (slab_is_safe_land(plyr_idx, aslb_x, aslb_y) && !slab_is_liquid(aslb_x, aslb_y)) {
                return true;
            }
        }
    }
    return false;
}

TbBool process_creature_hunger(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    if ( (crstat->hunger_rate == 0) || creature_affected_by_spell(thing, SplK_Freeze) )
        return false;
    SYNCDBG(19,"Hungering %s index %d",thing_model_name(thing), (int)thing->index);
    cctrl->hunger_level++;
    if (!hunger_is_creature_hungry(thing))
        return false;
    // Make sure every creature loses health on different turn
    if (((game.play_gameturn + thing->index) % game.turns_per_hunger_health_loss) == 0) {
        SYNCDBG(9,"The %s index %d lost %d health due to hunger",thing_model_name(thing), (int)thing->index, (int)game.hunger_health_loss);
        remove_health_from_thing_and_display_health(thing, game.hunger_health_loss);
        return true;
    }
    return false;
}

/**
 * Checks if creatures can attack each other.
 * Note that this function does not include full check from players_are_enemies(),
 *  so both should be used when applicable.
 * @param tng1
 * @param tng2
 * @return
 * @see players_are_enemies()
 */
TbBool creature_will_attack_creature(const struct Thing *fightng, const struct Thing *enmtng)
{
    if (creature_is_being_unconscious(fightng) || creature_is_being_unconscious(enmtng)) {
        return false;
    }
    if (thing_is_picked_up(fightng) || thing_is_picked_up(enmtng)) {
        return false;
    }
    struct CreatureControl* fighctrl = creature_control_get_from_thing(fightng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    if (players_creatures_tolerate_each_other(fightng->owner, enmtng->owner))
    {
        if  (((fighctrl->spell_flags & CSAfF_MadKilling) == 0)
         && ((enmctrl->spell_flags  & CSAfF_MadKilling) == 0)) {
            if (fighctrl->combat_flags == 0) {
                return false;
            }
            struct Thing* tmptng = thing_get(fighctrl->combat.battle_enemy_idx);
            TRACE_THING(tmptng);
            if (tmptng->index != enmtng->index) {
                return false;
            }
        }
        // No self fight
        if (enmtng->index == fightng->index) {
            return false;
        }
    }
    // No fight when creature in custody
    if (creature_is_kept_in_custody_by_player(fightng, enmtng->owner)
     || creature_is_kept_in_custody_by_player(enmtng, fightng->owner)) {
        return false;
    }
    // No fight while dropping
    if (creature_is_being_dropped(fightng) || creature_is_being_dropped(enmtng)) {
        return false;
    }
    // Final check - if creature is in control and can see the enemy - fight
    if ((creature_control_exists(enmctrl)) && ((enmctrl->flgfield_1 & CCFlg_NoCompControl) == 0))
    {
        if (!creature_is_invisible(enmtng) || creature_can_see_invisible(fightng)) {
            return true;
        }
    }
    return false;
}

/**
 * Checks if creatures can attack each other.
 * This variant loosens conditions if first creature is fighting till death, besides
 * that it is identical to creature_will_attack_creature().
 * Note that this function does not include full check from players_are_enemies(),
 *  so both should be used when applicable.
 * @param fightng
 * @param enmtng
 * @return
 * @see players_are_enemies()
 * @see creature_will_attack_creature()
 */
TbBool creature_will_attack_creature_incl_til_death(const struct Thing *fightng, const struct Thing *enmtng)
{
    if (creature_is_being_unconscious(fightng) || creature_is_being_unconscious(enmtng)) {
        return false;
    }
    if (thing_is_picked_up(fightng) || thing_is_picked_up(enmtng)) {
        return false;
    }
    struct CreatureControl* fighctrl = creature_control_get_from_thing(fightng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);

    if (players_creatures_tolerate_each_other(fightng->owner, enmtng->owner))
    {
        if  ((fighctrl->fight_til_death == 0) // This differs in creature_will_attack_creature()
         && ((fighctrl->spell_flags & CSAfF_MadKilling) == 0)
         && ((enmctrl->spell_flags  & CSAfF_MadKilling) == 0)) {
            struct Thing* tmptng = thing_get(fighctrl->combat.battle_enemy_idx);
            TRACE_THING(tmptng);
            if ((fighctrl->combat_flags == 0) || (tmptng->index != enmtng->index)) {
                return false;
            }
        }
        // No self fight
        if (enmtng->index == fightng->index) {
            return false;
        }
    }
    // No fight when creature in custody
    if (creature_is_kept_in_custody_by_player(fightng, enmtng->owner)
     || creature_is_kept_in_custody_by_player(enmtng, fightng->owner)) {
        return false;
    }
    // No fight while dropping
    if (creature_is_being_dropped(fightng) || creature_is_being_dropped(enmtng)) {
        return false;
    }
    // Final check - if creature is in control and can see the enemy - fight
    if ((creature_control_exists(enmctrl)) && ((enmctrl->flgfield_1 & CCFlg_NoCompControl) == 0))
    {
        if (!creature_is_invisible(enmtng) || creature_can_see_invisible(fightng)) {
            return true;
        }
    }
    return false;
}

/**
 * Returns if state of given thing can can never be blocked by stateblock_flags.
 * This only happens for some utility states which should have state callback executed
 *  even if the creature is unresponsive due to spells.
 * @param thing
 * @return
 */
TbBool creature_state_cannot_be_blocked(const struct Thing *thing)
{
    CrtrStateId i = thing->active_state;
    if ((i == CrSt_MoveToPosition) || (i == CrSt_MoveBackwardsToPosition))
        i = thing->continue_state;
    if ((i == CrSt_CreatureBeingDropped) || (i == CrSt_CreatureHeroEntering) || (i == CrSt_ImpBirth))
        return true;
    if ((i == CrSt_CreatureChangeFromChicken) || (i == CrSt_CreatureChangeToChicken))
        return true;
    return false;
}

struct Thing *thing_update_enemy_to_fight_with(struct Thing *thing)
{
    struct Thing *enemytng;
    SYNCDBG(9,"Starting");
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->word_9A != 0)
    {
        enemytng = thing_get(cctrl->word_9A);
        TRACE_THING(enemytng);
        if (((enemytng->alloc_flags & TAlF_Exists) == 0) || (cctrl->long_9C != enemytng->creation_turn))
        {
          enemytng = INVALID_THING;
          cctrl->long_9C = 0;
          cctrl->word_9A = 0;
        }
    } else
    {
        enemytng = INVALID_THING;
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
    cctrl->long_9C = enemytng->creation_turn;
    return enemytng;
}

static TbBool wander_point_get_random_pos(const struct Wander *wandr, const struct Coord3d *prevpos, struct Coord3d *pos,
        struct Thing* thing)
{
  SYNCDBG(12,"Selecting out of %d points",(int)wandr->points_count);
  MapSubtlCoord selected_dist = 0;
  if (wandr->points_count > 0)
  {
      // Select a position based on 3 tries
      for (long i = 0; i < 3; i++)
      {
          long irnd = CREATURE_RANDOM(thing, wandr->points_count);
          MapSubtlCoord stl_x = wandr->points[irnd].stl_x;
          MapSubtlCoord stl_y = wandr->points[irnd].stl_y;
          MapSubtlCoord dist = get_2d_box_distance_xy(stl_x, stl_y, prevpos->x.stl.num, prevpos->y.stl.num);
          // Move at least 2 slabs, and prefer distance around 7 slabs
          // If previously selected selected_dist is too low, allow any place
          if (((dist > 6) && (abs(dist-21) < abs(selected_dist-21))) || (selected_dist <= 6))
          {
              pos->x.val = subtile_coord_center(stl_x);
              pos->y.val = subtile_coord_center(stl_y);
              pos->z.val = subtile_coord(1,0);
              selected_dist = dist;
          }
      }
      return true;
  }
  return false;
}

TbBool get_random_position_in_dungeon_for_creature(PlayerNumber plyr_idx, unsigned char wandr_slot, struct Thing *thing, struct Coord3d *pos)
{
    if (plyr_idx == game.neutral_player_num)
    {
        ERRORLOG("Attempt to get random position in neutral dungeon");
        return false;
    }
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_invalid(player))
    {
        ERRORLOG("Attempt to get random position in invalid dungeon %d",(int)plyr_idx);
        return false;
    }
    if (wandr_slot == CrWaS_WithinDungeon)
    {
        if (!wander_point_get_random_pos(&player->wandr_within, &thing->mappos, pos, thing)) {
            SYNCDBG(12,"Cannot get position from wander slot %d",(int)wandr_slot);
            return false;
        }
    } else
    { // means (wandr_slot == CrWaS_OutsideDungeon)
        if (!wander_point_get_random_pos(&player->wandr_outside, &thing->mappos, pos, thing)) {
            SYNCDBG(12,"Cannot get position from wander slot %d",(int)wandr_slot);
            return false;
        }
    }
    return true;
}

TbBool creature_can_hear_within_distance(const struct Thing *thing, long dist)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    return subtile_coord(crstat->hearing,0) >= dist;
}

long get_thing_navigation_distance(struct Thing* creatng, struct Coord3d* pos, unsigned char resetOwnerPlayerNavigating)
{
    if (pos->x.val == creatng->mappos.x.val && pos->y.val == creatng->mappos.y.val)
        return 0;

    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(creatng);
    if (resetOwnerPlayerNavigating)
        owner_player_navigating = -1;
    else
        owner_player_navigating = creatng->owner;
    long nav_sizexy = thing_nav_block_sizexy(creatng);
    if (nav_sizexy > 0)
        --nav_sizexy;
    struct Path path = {0};
    path_init8_wide_f(
        &path,
        creatng->mappos.x.val,
        creatng->mappos.y.val,
        pos->x.val,
        pos->y.val,
	    -2, nav_sizexy, __func__);
    nav_thing_can_travel_over_lava = 0;

    int distance = 0;
    if (!path.waypoints_num)
        return LONG_MAX;

    if (path.waypoints_num <= 0)
        return distance;

    struct Coord3d pos1 = creatng->mappos;
    for (int i = 0; i < path.waypoints_num; ++i)
    {
        struct Coord3d pos2;
        pos2.x.val = (uint16_t)path.waypoints[i].x;
        pos2.y.val = (uint16_t)path.waypoints[i].y;
        distance += get_2d_distance(&pos1, (struct Coord3d*)&pos2);
        pos1 = pos2;
    }
    return distance;
}

/**
 * Enemy seeking function for creatures and heroes.
 * Used for performing SEEK_THE_ENEMY job.
 * @param thing The creature to seek the enemy for.
 * @return
 */
short seek_the_enemy(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* enemytng = thing_update_enemy_to_fight_with(creatng);
    if (!thing_is_invalid(enemytng))
    {
        long dist = get_2d_box_distance(&enemytng->mappos, &creatng->mappos);
        if (creature_can_hear_within_distance(creatng, dist))
        {
            if (cctrl->instance_id == CrInst_NULL)
            {
              if ((dist < 2304) && (game.play_gameturn-cctrl->countdown_282 < 20))
              {
                set_creature_instance(creatng, CrInst_CELEBRATE_SHORT, 1, 0, 0);
                thing_play_sample(creatng, 168+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
                return 1;
              }
              if (CREATURE_RANDOM(creatng, 4) != 0)
              {
                  if (setup_person_move_close_to_position(creatng, enemytng->mappos.x.stl.num, enemytng->mappos.y.stl.num, NavRtF_Default))
                  {
                    creatng->continue_state = CrSt_SeekTheEnemy;
                    cctrl->countdown_282 = game.play_gameturn;
                    return 1;
                  }
              }
              if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
              {
                  creatng->continue_state = CrSt_SeekTheEnemy;
                  cctrl->countdown_282 = game.play_gameturn;
              }
            }
            return 1;
        }
        if (CREATURE_RANDOM(creatng, 64) == 0)
        {
            if (setup_person_move_close_to_position(creatng, enemytng->mappos.x.stl.num, enemytng->mappos.y.stl.num, NavRtF_Default))
            {
              creatng->continue_state = CrSt_SeekTheEnemy;
            }
        }
    }
    // No enemy found - do some random movement
    struct Coord3d pos;
    if (CREATURE_RANDOM(creatng, 12) != 0)
    {
        if ( creature_choose_random_destination_on_valid_adjacent_slab(creatng) )
        {
            creatng->continue_state = CrSt_SeekTheEnemy;
            return 1;
        }
    } else
    if (get_random_position_in_dungeon_for_creature(creatng->owner, CrWaS_WithinDungeon, creatng, &pos))
    {
        if (setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
        {
            creatng->continue_state = CrSt_SeekTheEnemy;
        }
        return 1;
    }
    set_start_state(creatng);
    return 1;
}

short state_cleanup_wait_at_door(struct Thing* creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->blocking_door_id = 0;
    cctrl->collided_door_subtile = 0;
    return 1;
}

short state_cleanup_dragging_body(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->dragtng_idx > 0)
    {
        struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
        if (dragtng->class_id == TCls_Creature)
        {
            stop_creature_being_dragged_by(dragtng, creatng);
        } else 
        if (dragtng->class_id == TCls_DeadCreature)
        {
            creature_drop_dragged_object(creatng, dragtng);
            dragtng->owner = game.neutral_player_num;
        }
    }
    return 1;
}

short state_cleanup_dragging_object(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->dragtng_idx > 0)
    {
        struct Thing* objctng = thing_get(cctrl->dragtng_idx);
        creature_drop_dragged_object(creatng, objctng);
        objctng->owner = game.neutral_player_num;
    }
    return 1;
}

short state_cleanup_in_room(struct Thing *creatng)
{
    TRACE_THING(creatng);
    remove_creature_from_work_room(creatng);
    return 1;
}

short state_cleanup_unable_to_fight(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
    return 1;
}

short state_cleanup_unconscious(struct Thing *creatng)
{
    TRACE_THING(creatng);
    make_creature_conscious_without_changing_state(creatng);
    return 1;
}

long process_work_speed_on_work_value(const struct Thing *thing, long base_val)
{
    long val = base_val;
    if (creature_affected_by_spell(thing, SplK_Speed))
        val = 2 * val;
    if (creature_affected_by_slap(thing))
        val = 4 * val / 3;
    if (!is_neutral_thing(thing))
    {
        struct Dungeon* dungeon = get_dungeon(thing->owner);
        if (dungeon->tortured_creatures[thing->model] > 0)
            val = 4 * val / 3;
        if (player_uses_power_obey(thing->owner))
            val = 6 * val / 5;
    }
    SYNCDBG(19,"Work value %d changed to %d for %s index %d",(int)base_val, (int)val, thing_model_name(thing), (int)thing->index);
    return val;
}

TbBool check_experience_upgrade(struct Thing *thing)
{
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    long i = crstat->to_level[cctrl->explevel] << 8;
    if (cctrl->exp_points < i)
        return false;
    cctrl->exp_points -= i;
    if (cctrl->explevel < dungeon->creature_max_level[thing->model])
    {
      if ((cctrl->explevel < CREATURE_MAX_LEVEL-1) || (crstat->grow_up != 0))
        cctrl->spell_flags |= CSAfF_ExpLevelUp;
    }
    return true;
}
/******************************************************************************/
TbBool internal_set_thing_state(struct Thing *thing, CrtrStateId nState)
{
    thing->active_state = nState;
    thing->state_flags &= ~TF1_Unkn10;
    thing->continue_state = CrSt_Unused;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->field_302 = 0;
    clear_creature_instance(thing);
    return true;
}

TbBool initialise_thing_state_f(struct Thing *thing, CrtrStateId nState, const char *func_name)
{
    TRACE_THING(thing);
    SYNCDBG(9,"%s: State change %s to %s for %s index %d",func_name,creature_state_code_name(thing->active_state),
        creature_state_code_name(nState), thing_model_name(thing),(int)thing->index);
    cleanup_current_thing_state(thing);
    thing->active_state = nState;
    thing->state_flags &= ~TF1_Unkn10;
    thing->continue_state = CrSt_Unused;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("%s: The %s index %d has invalid control",func_name,thing_model_name(thing),(int)thing->index);
        return false;
    }
    cctrl->target_room_id = 0;
    cctrl->field_302 = 0;
    if ((cctrl->flgfield_1 & CCFlg_IsInRoomList) != 0)
    {
        WARNLOG("%s: The %s stays in room list even after cleanup",func_name,thing_model_name(thing));
        remove_creature_from_work_room(thing);
    }
    return true;
}

TbBool cleanup_current_thing_state(struct Thing *creatng)
{
    struct StateInfo* stati = get_creature_state_with_task_completion(creatng);
    CreatureStateFunc1 cleanup_cb = stati->cleanup_state;
    if (cleanup_cb != NULL)
    {
        cleanup_cb(creatng);
        creatng->state_flags |= TF1_Unkn10;
    } else
    {
        clear_creature_instance(creatng);
    }
    return true;
}

TbBool cleanup_creature_state_and_interactions(struct Thing *creatng)
{
    cleanup_current_thing_state(creatng);
    set_creature_assigned_job(creatng, Job_NULL);
    remove_all_traces_of_combat(creatng);
    if (creature_is_group_member(creatng)) {
        remove_creature_from_group(creatng);
    }
    remove_events_thing_is_attached_to(creatng);
    delete_effects_attached_to_creature(creatng);
    state_cleanup_dragging_body(creatng);
    state_cleanup_dragging_object(creatng);
    return true;
}

TbBool can_change_from_state_to(const struct Thing *thing, CrtrStateId curr_state, CrtrStateId next_state)
{
    struct StateInfo* curr_stati = get_thing_state_info_num(curr_state);
    if (curr_stati->state_type == CrStTyp_Move)
      curr_stati = get_thing_state_info_num(thing->continue_state);
    struct StateInfo* next_stati = get_thing_state_info_num(next_state);
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        if ( (next_stati->state_type != CrStTyp_Idle) )
        {
            return false;
        }
    }
    if ((curr_stati->field_20) && (!next_stati->override_prev_fld20))
        return false;
    if ((curr_stati->field_1F) && (!next_stati->override_prev_fld1F))
        return false;
    switch (curr_stati->state_type)
    {
    case CrStTyp_OwnNeeds:
        if (next_stati->override_own_needs)
            return true;
        break;
    case CrStTyp_Sleep:
        if (next_stati->override_sleep)
            return true;
        break;
    case CrStTyp_Feed:
        if (next_stati->override_feed)
            return true;
        break;
    case CrStTyp_FightCrtr:
        if (next_stati->override_fight_crtr)
            return true;
        break;
    case CrStTyp_GetsSalary:
        if (next_stati->override_gets_salary)
            return true;
        break;
    case CrStTyp_Escape:
        if (next_stati->override_escape)
            return true;
        break;
    case CrStTyp_Unconscious:
        if (next_stati->override_unconscious)
            return true;
        break;
    case CrStTyp_AngerJob:
        if (next_stati->override_anger_job)
            return true;
        break;
    case CrStTyp_FightDoor:
        if (next_stati->override_fight_door)
            return true;
        break;
    case CrStTyp_FightObj:
        if (next_stati->override_fight_object)
            return true;
        break;
    case CrStTyp_Called2Arms:
        if (next_stati->override_call2arms)
            return true;
        break;
    case CrStTyp_Follow:
        if (next_stati->override_follow)
            return true;
        break;
    default:
        return true;
    }
    return false;
}

short set_start_state_f(struct Thing *thing,const char *func_name)
{
    long i;
    SYNCDBG(8,"%s: Starting for %s index %d, owner %d, last state %s, stacked %s",func_name,thing_model_name(thing),
        (int)thing->index,(int)thing->owner,creature_state_code_name(thing->active_state),creature_state_code_name(thing->continue_state));
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_ManualControl);
        return thing->active_state;
    }
    if (creature_affected_by_spell(thing, SplK_Chicken))
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_CreaturePretendChickenSetupMove);
        return thing->active_state;
    }
    if (is_neutral_thing(thing))
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_CreatureDormant);
        return thing->active_state;
    }
    if (is_hero_thing(thing))
    {
        i = creatures[thing->model%CREATURE_TYPES_COUNT].good_start_state;
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, i);
        return thing->active_state;
    }
    struct PlayerInfo* player = get_player(thing->owner);
    // For creatures which do not have corresponding player
    if (!player_exists(player))
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_CreatureDormant);
        return thing->active_state;
    }
    if (player->victory_state == VicS_LostLevel)
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_LeavesBecauseOwnerLost);
        return thing->active_state;
    }
    i = creatures[thing->model%CREATURE_TYPES_COUNT].evil_start_state;
    cleanup_current_thing_state(thing);
    initialise_thing_state(thing, i);
    return thing->active_state;
}

TbBool external_set_thing_state_f(struct Thing *thing, CrtrStateId state, const char *func_name)
{
    EVM_CREATURE_EVENT_WITH_TARGET("ext_state", thing->owner, thing, state);
    if (!can_change_from_state_to(thing, thing->active_state, state))
    {
        WARNDBG(4,"%s: State change %s to %s for %s not allowed",func_name,creature_state_code_name(thing->active_state),
            creature_state_code_name(state), thing_model_name(thing));
        return false;
    }
    initialise_thing_state(thing, state);
    return true;
}

TbBool creature_free_for_sleep(const struct Thing *thing,  CrtrStateId state)
{
    if (creature_affected_by_slap(thing) || creature_is_called_to_arms(thing))
        return false;
    return can_change_from_state_to(thing, thing->active_state, state);
}

/**
 * If creature health is very low, go back to lair immediately for healing.
 * @param thing
 * @param crstat
 */
long process_creature_needs_to_heal_critical(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (get_creature_health_permil(creatng) >= gameadd.critical_health_permil) {
        return 0;
    }
    if (!creature_can_do_healing_sleep(creatng))
    {
        // Creature needs healing but cannot heal in lair - try toking
        struct SlabMap* slb = get_slabmap_thing_is_on(creatng);
        if (slabmap_owner(slb) != creatng->owner) {
            return 0;
        }
        if (!creature_free_for_sleep(creatng, CrSt_CreatureGoingToSafetyForToking)) {
            return 0;
        }
        if (creature_is_doing_toking(creatng)) {
            return 0;
        }
        if (external_set_thing_state(creatng, CrSt_CreatureGoingToSafetyForToking)) {
            creatng->continue_state = CrSt_ImpDoingNothing;
            cctrl->countdown_282 = 200;
            return 1;
        }
    }
    if (creature_is_doing_lair_activity(creatng)) {
        return 1;
    }
    if (!creature_free_for_sleep(creatng, CrSt_CreatureGoingHomeToSleep)) {
        return 0;
    }
    if ( (game.play_gameturn - cctrl->healing_sleep_check_turn > 128) &&
      ((cctrl->lair_room_id != 0) || !room_is_invalid(get_best_new_lair_for_creature(creatng))) )
    {
        SYNCDBG(4,"Healing critical for %s",thing_model_name(creatng));
        if (external_set_thing_state(creatng, CrSt_CreatureGoingHomeToSleep)) {
            return 1;
        }
    } else
    {
        struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
        anger_apply_anger_to_creature(creatng, crstat->annoy_no_lair, AngR_NoLair, 1);
    }
    cctrl->healing_sleep_check_turn = game.play_gameturn;
    return 0;
}

long creature_setup_head_for_treasure_room_door(struct Thing *creatng, struct Room *room)
{
    struct Coord3d pos;
    if (find_random_valid_position_for_thing_in_room(creatng, room, &pos))
    {
        if (setup_person_move_to_position(creatng, pos.x.stl.num, pos.y.stl.num, NavRtF_NoOwner))
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
            creatng->continue_state = CrSt_CreatureTakeSalary;
            cctrl->target_room_id = room->index;
            return 1;
        }
    }
    return 0;
}

long process_creature_needs_a_wage(struct Thing *thing, const struct CreatureStats *crstat)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((crstat->pay == 0) || (cctrl->paydays_owed == 0)) {
      return 0;
    }
    if (creature_is_taking_salary_activity(thing)) {
        return 1;
    }
    if (!can_change_from_state_to(thing, thing->active_state, CrSt_CreatureWantsSalary)) {
        return 0;
    }
    struct Room* room = find_nearest_room_for_thing_with_used_capacity(thing, thing->owner, RoK_TREASURE, NavRtF_Default, 1);
    if (!room_is_invalid(room))
    {
        if (external_set_thing_state(thing, CrSt_CreatureWantsSalary))
        {
            anger_apply_anger_to_creature(thing, crstat->annoy_got_wage, AngR_NotPaid, 1);
            return 1;
        }
        return 0;
    }
    room = find_any_navigable_room_for_thing_closer_than(thing, thing->owner, RoK_TREASURE, NavRtF_Default, map_subtiles_x / 2 + map_subtiles_y / 2);
    if (room_is_invalid(room))
    {
        //if we can't find an unlocked room, try a locked room, to wait in front of the door
        room = find_nearest_room_for_thing_with_used_capacity(thing, thing->owner, RoK_TREASURE, NavRtF_NoOwner, 1);
    }
    if (!room_is_invalid(room))
    {
        cleanup_current_thing_state(thing);
        if (creature_setup_head_for_treasure_room_door(thing, room))
        {
            return 1;
        }
        ERRORLOG("Shit, could not get to treasure room door and I've cleaned up my old state");
        set_start_state(thing);
        return 0;
    }
    struct Dungeon* dungeon = get_players_num_dungeon(thing->owner);
    room = find_nearest_room_for_thing(thing, thing->owner, RoK_TREASURE, NavRtF_Default);
    if ((dungeon->total_money_owned >= calculate_correct_creature_pay(thing)) && !room_is_invalid(room))
    {
        cleanup_current_thing_state(thing);
        if (creature_setup_random_move_for_job_in_room(thing, room, Job_TAKE_SALARY, NavRtF_Default))
        {
            thing->continue_state = CrSt_CreatureTakeSalary;
            cctrl->target_room_id = room->index;
            return 1;
        }
        ERRORLOG("Shit, could not get to treasure room and I've cleaned up my old state");
        set_start_state(thing);
        return 0;
    }
    cctrl->paydays_owed--;
    anger_apply_anger_to_creature(thing, crstat->annoy_no_salary, AngR_NotPaid, 1);
    return 0;
}

char creature_free_for_lunchtime(struct Thing *creatng)
{
    return !creature_affected_by_slap(creatng)
        && !creature_is_called_to_arms(creatng)
        && !creature_affected_by_spell(creatng, SplK_Chicken)
        && can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureToGarden);
}

long process_creature_needs_to_eat(struct Thing *creatng, const struct CreatureStats *crstat)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((crstat->hunger_rate == 0) || (cctrl->hunger_level <= (long)crstat->hunger_rate)) {
        return 0;
    }
    if (creature_is_doing_garden_activity(creatng)) {
        return 1;
    }
    if (!creature_free_for_lunchtime(creatng)) {
      return 0;
    }
    if (crstat->hunger_fill <= cctrl->hunger_loss)
    {
        cctrl->garden_eat_check_turn = game.play_gameturn;
        cctrl->hunger_loss -= crstat->hunger_fill;
        return 0;
    }

    if (!player_has_room_of_role(creatng->owner, RoRoF_FoodStorage))
    {
        output_message_room_related_from_computer_or_player_action(creatng->owner, RoK_GARDEN, OMsg_RoomNeeded);
        anger_apply_anger_to_creature(creatng, crstat->annoy_no_hatchery, AngR_Hungry, 1);
        return 0;
    }
    if (game.play_gameturn - cctrl->garden_eat_check_turn <= 128) {
        anger_apply_anger_to_creature(creatng, crstat->annoy_no_hatchery, AngR_Hungry, 1);
        return 0;
    }
    struct Room* nroom = find_nearest_room_for_thing_with_used_capacity(creatng, creatng->owner, RoK_GARDEN, NavRtF_Default, 1);
    if (room_is_invalid(nroom))
    {
        cctrl->garden_eat_check_turn = game.play_gameturn;
        // No food in nearest room, try to find another room
        nroom = find_random_room_for_thing(creatng, creatng->owner, RoK_GARDEN, 0);
        if (room_is_invalid(nroom))
        {
            // There seem to be a correct room, but we can't reach it
            output_message_room_related_from_computer_or_player_action(creatng->owner, RoK_GARDEN, OMsg_RoomNoRoute);
        } else
        {
            // The room is reachable, so it probably has just no food
            output_message_room_related_from_computer_or_player_action(creatng->owner, RoK_GARDEN, OMsg_RoomTooSmall);
        }
    }
    if (room_is_invalid(nroom)) {
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
        anger_apply_anger_to_creature(creatng, crstat->annoy_no_hatchery, AngR_Hungry, 1);
        return 0;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureToGarden)) {
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
        anger_apply_anger_to_creature(creatng, crstat->annoy_no_hatchery, AngR_Hungry, 1);
        return 0;
    }
    short hunger_loss = cctrl->hunger_loss;
    short hunger_fill = crstat->hunger_fill;
    if (hunger_loss != 0)
    {
        short hunger = hunger_fill - hunger_loss;
        cctrl->hunger_amount = hunger;
        if (hunger <= 0)
        {
          ERRORLOG("This shouldn't happen");
          cctrl->hunger_amount = 1;
        }
        cctrl->hunger_loss = 0;
    } else
    {
        cctrl->hunger_amount = hunger_fill;
    }
    return 1;
}

long anger_process_creature_anger(struct Thing *creatng, const struct CreatureStats *crstat)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    // Creatures with no annoyance level will never get angry
    if (crstat->annoy_level == 0) {
        return 0;
    }
    if (!creature_free_for_anger_job(creatng)) {
        return 0;
    }
    if (!anger_is_creature_angry(creatng)) {
        // If the creature is mad killing, don't allow it not to be angry
        if ((cctrl->spell_flags & CSAfF_MadKilling) != 0) {
            // Mad creature's mind is tortured, so apply torture anger
            anger_apply_anger_to_creature(creatng, crstat->annoy_in_torture, AngR_Other, 1);
        }
        return 0;
    }
    if (is_my_player_number(creatng->owner))
    {
        struct Dungeon* dungeon;
        AnnoyMotive anger_motive = anger_get_creature_anger_type(creatng);
        switch (anger_motive)
        {
        case AngR_NotPaid:
            dungeon = get_players_num_dungeon(creatng->owner);
            struct Room *room = find_nearest_room_for_thing(creatng, creatng->owner, RoK_TREASURE, NavRtF_Default);
            if ((dungeon->total_money_owned >= calculate_correct_creature_pay(creatng)) && !room_is_invalid(room))
            {
                if (cctrl->paydays_owed <= 0)
                {
                    cctrl->paydays_owed++;
                }
                else
                {
                    output_message(SMsg_CreatrAngryAnyReason, MESSAGE_DELAY_CRTR_MOOD, 1);
                }
            }
            else
            {
                output_message(SMsg_CreatrAngryNotPaid, MESSAGE_DELAY_CRTR_MOOD, 1);
            }
            break;
        case AngR_Hungry:
            output_message(SMsg_CreatrAngryNoFood, MESSAGE_DELAY_CRTR_MOOD, 1);
            break;
        case AngR_NoLair:
            if (cctrl->lairtng_idx != 0)
            {
                output_message(SMsg_CreatrAngryAnyReason, MESSAGE_DELAY_CRTR_MOOD, 1);
            }
            else
            {
                output_message(SMsg_CreatrAngryNoLair, MESSAGE_DELAY_CRTR_MOOD, 1);
            }
            break;
        case AngR_Other:
            output_message(SMsg_CreatrAngryAnyReason, MESSAGE_DELAY_CRTR_MOOD, 1);
            break;
        default:
            output_message(SMsg_CreatrAngryAnyReason, MESSAGE_DELAY_CRTR_MOOD, 1);
            ERRORLOG("The %s owned by player %d is angry but has no motive (%d).",thing_model_name(creatng),(int)creatng->owner,(int)anger_motive);
            break;
        }
    }
    if (creature_is_doing_anger_job(creatng)) {
        return 0;
    }
    if (anger_is_creature_livid(creatng) && (((game.play_gameturn + creatng->index) & 0x3F) == 0))
    {
        if (creature_find_and_perform_anger_job(creatng))
            return 1;
    }
    if ((crstat->annoy_in_temple < 0) && (game.play_gameturn - cctrl->temple_pray_check_turn > 128))
    {
        cctrl->temple_pray_check_turn = game.play_gameturn;
        if (creature_is_doing_temple_pray_activity(creatng))
            return 1;
        if (creature_can_do_job_for_player(creatng, creatng->owner, Job_TEMPLE_PRAY, JobChk_PlayMsgOnFail)
            && can_change_from_state_to(creatng, creatng->active_state, CrSt_AtTemple))
        {
            if (send_creature_to_job_for_player(creatng, creatng->owner, Job_TEMPLE_PRAY)) {
                return 1;
            }
            ERRORLOG("Tried sending %s owner %d to job %s, could not do this",thing_model_name(creatng),(int)creatng->owner,creature_job_code_name(Job_TEMPLE_PRAY));
        }
    }
    if (creature_has_lair_room(creatng) && creature_can_do_healing_sleep(creatng))
    {
      if (game.play_gameturn - cctrl->sulking_sleep_check_turn > 128)
      {
          cctrl->sulking_sleep_check_turn = game.play_gameturn;
          // If creature has lair, try to go to it
          if (anger_get_creature_anger_type(creatng) == AngR_NoLair)
          {
              if (external_set_thing_state(creatng, CrSt_CreatureGoingHomeToSleep))
                return 1;
          } else
          {
              if (external_set_thing_state(creatng, CrSt_PersonSulkHeadForLair))
                return 1;
          }
      }
    }
    return 0;
}

long process_creature_needs_to_heal(struct Thing *creatng, const struct CreatureStats *crstat)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (!creature_can_do_healing_sleep(creatng) || !creature_requires_healing(creatng)) {
        return 0;
    }
    if (creature_is_doing_lair_activity(creatng)) {
        return 1;
    }
    if (!creature_free_for_sleep(creatng, CrSt_CreatureGoingHomeToSleep)) {
        return 0;
    }
    if (((game.play_gameturn - cctrl->healing_sleep_check_turn) > 128)
      && ((cctrl->lair_room_id != 0) || !room_is_invalid(get_best_new_lair_for_creature(creatng))))
    {
        if (external_set_thing_state(creatng, CrSt_CreatureGoingHomeToSleep)) {
            return 1;
        }
    } else
    {
      anger_apply_anger_to_creature(creatng, crstat->annoy_no_lair, AngR_NoLair, 1);
    }
    cctrl->healing_sleep_check_turn = game.play_gameturn;
    return 0;
}

long process_training_need(struct Thing *thing, const struct CreatureStats *crstat)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((crstat->annoy_untrained == 0) || !creature_can_be_trained(thing)) {
        return 0;
    }
    if (creature_is_training(thing)) {
        return 0;
    }
    cctrl->annoy_untrained_turn++;
    if (cctrl->annoy_untrained_turn >= crstat->annoy_untrained_time)
    {
      anger_apply_anger_to_creature(thing, crstat->annoy_untrained, AngR_Other, 1);
      cctrl->annoy_untrained_turn = 0;
    }
    return 0;
}

long process_piss_need(struct Thing *thing, const struct CreatureStats *crstat)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->field_B9 == 0) {
        return 0;
    }
    struct Thing* pisstng = thing_get(cctrl->field_B9);
    if ((pisstng->owner == thing->owner) || !crstat->piss_on_dead) {
        return 0;
    }
    if (game.play_gameturn - cctrl->field_B2 <= 200) {
        return 0;
    }
    if (!external_set_thing_state(thing, CrSt_CreaturePiss))
    {
        return 0;
    }
    cctrl->countdown_282 = 50;
    cctrl->field_B2 = game.play_gameturn;
    return 1;
}

void process_person_moods_and_needs(struct Thing *thing)
{
    if (is_hero_thing(thing) || is_neutral_thing(thing)) {
        // Heroes and neutral creatures have no special needs
        return;
    }
    if (creature_is_kept_in_custody(thing)) {
        // And creatures being tortured, imprisoned, etc.
        return;
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    // Now process the needs
    process_creature_hunger(thing);
    if (process_creature_needs_to_heal_critical(thing)) {
        SYNCDBG(17,"The %s index %d has a critical need to heal",thing_model_name(thing),(long)thing->index);
    } else
    if (creature_affected_by_call_to_arms(thing)) {
        SYNCDBG(17,"The %s index %d is called to arms, most needs suspended",thing_model_name(thing),(long)thing->index);
    } else
    if (process_creature_needs_a_wage(thing, crstat)) {
        SYNCDBG(17,"The %s index %d has a need to get its wage",thing_model_name(thing),(long)thing->index);
    } else
    if (process_creature_needs_to_eat(thing, crstat)) {
        SYNCDBG(17,"The %s index %d has a need to eat",thing_model_name(thing),(long)thing->index);
    } else
    if (anger_process_creature_anger(thing, crstat)) {
        SYNCDBG(17,"The %s index %d has a need to cool its anger",thing_model_name(thing),(long)thing->index);
    } else
    if (process_creature_needs_to_heal(thing, crstat)) {
        SYNCDBG(17,"The %s index %d has a need to heal",thing_model_name(thing),(long)thing->index);
    }
    process_training_need(thing, crstat);
    process_piss_need(thing, crstat);
}

TbBool setup_move_off_lava(struct Thing* thing)
{
    //return _DK_setup_move_off_lava(thing);
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab(thing->mappos.x.stl.num);
    slb_y = subtile_slab(thing->mappos.y.stl.num);
    long i;
    for (i = 0; i < 32; i++)
    {
        struct MapOffset* sstep;
        MapSubtlCoord cx;
        MapSubtlCoord cy;
        sstep = &spiral_step[i];
        cx = slab_subtile_center(slb_x + sstep->h);
        cy = slab_subtile_center(slb_y + sstep->v);
        struct SlabMap* slb;
        slb = get_slabmap_for_subtile(cx, cy);
        if (slabmap_block_invalid(slb))
            continue;
        const struct SlabAttr* slbattr;
        slbattr = get_slab_attrs(slb);
        if (!slbattr->is_safe_land)
            continue;
        // Check all subtiles of the slab in random order
        long k;
        long n;
        n = CREATURE_RANDOM(thing, AROUND_TILES_COUNT);
        for (k = 0; k < AROUND_TILES_COUNT; k++, n = (n + 1) % AROUND_TILES_COUNT)
        {
            struct Map* mapblk;
            long stl_x;
            long stl_y;
            stl_x = cx + around[k].delta_x;
            stl_y = cy + around[k].delta_y;
            mapblk = get_map_block_at(stl_x, stl_y);
            if (!map_block_invalid(mapblk))
            {
                if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
                {
                    if (setup_person_move_to_position(thing, stl_x, stl_y, 0)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

//todo CAVE_IN_NEAR_FLEE_POSITION into config file
#define CAVE_IN_NEAR_FLEE_POSITION 200
TbBool setup_move_out_of_cave_in(struct Thing* thing)
{
    // return _DK_setup_move_out_of_cave_in(thing);
    MapSlabCoord bx = 0;
    MapSlabCoord by = 0;
    MapSubtlCoord cx = 0;
    MapSubtlCoord cy = 0;
    struct Thing* tng;
    struct MapOffset* sstep;
    struct Map* blk;
    if (setup_combat_flee_position(thing))
    {
        struct CreatureControl* cctrl;
        cctrl = creature_control_get_from_thing(thing);
        long dist = LbDiagonalLength(abs(thing->mappos.x.val - cctrl->flee_pos.x.val), abs(thing->mappos.y.val - cctrl->flee_pos.y.val));
        // If you're too close to the flee position, no point in going there to escape cave in damage.
        if (dist <= CAVE_IN_NEAR_FLEE_POSITION)
        {
            // Heroes that are near to a hero gate, should escape through it if they can.
            if (is_hero_thing(thing))
            {
                if (good_leave_through_exit_door(thing))
                {
                    return true;
                }
                if (creature_choose_random_destination_on_valid_adjacent_slab(thing))
                {
                    return true;
                }
            }
            // Creatures (or weird heroes) at their flee position should find a random space in the dungeon.
            struct Coord3d pos;
            if (get_random_position_in_dungeon_for_creature(thing->owner, CrWaS_WithinDungeon, thing, &pos))
            {
                if (setup_person_move_to_coord(thing, &pos, 0))
                {
                    return true;
                }
            }
            return false;
        }
        else
        {
            if (setup_person_move_to_coord(thing, &cctrl->flee_pos, 0))
            {
                return true;
            }
        }
    }
    else
    {
        MapSlabCoord slb_x = subtile_slab(thing->mappos.x.stl.num);
        MapSlabCoord slb_y = subtile_slab(thing->mappos.y.stl.num);
        for (signed int i = 0; i < 32; i++)
        {
            sstep = &spiral_step[i];
            bx = sstep->h + slb_x;
            by = sstep->v + slb_y;
            struct SlabMap* slb;
            slb = get_slabmap_block(bx, by);
            if (slabmap_block_invalid(slb))
            {
                continue;
            }
            blk = get_map_block_at(slab_subtile(bx, 0), slab_subtile(by, 0));
            long n = get_mapwho_thing_index(blk);
            while (n != 0)
            {
                tng = thing_get(n);
                TRACE_THING(tng);
                if (tng->class_id == TCls_EffectElem && tng->model == 46)
                {
                    break;
                }
                n = tng->next_on_mapblk;
                if (thing_is_invalid(tng))
                {
                    bx = sstep->h + slb_x;
                    break;
                }
            }
            bx = sstep->h + slb_x;
            cx = slab_subtile_center(bx);
            cy = slab_subtile_center(by);
            long j = CREATURE_RANDOM(thing, AROUND_TILES_COUNT);
            for (long k = 0; k < AROUND_TILES_COUNT; k++, j = (j + 1) % AROUND_TILES_COUNT)
            {
                MapSubtlCoord stl_x = cx + around[j].delta_x;
                MapSubtlCoord stl_y = cy + around[j].delta_y;
                struct Map* mapblk = get_map_block_at(stl_x, stl_y);
                if (!map_block_invalid(mapblk))
                {
                    if (subtile_is_blocking_wall_or_lava(stl_x, stl_y, thing->owner) == 0)
                    {
                        if (setup_person_move_to_position(thing, stl_x, stl_y, 0))
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

/******************************************************************************/
