/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states.c
 *     Creature states structure and function definitions.
 * @par Purpose:
 *     Defines elements of game.conf.crtr_conf.states[] array, containing valid creature states.
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
#include "pre_inc.h"
#include "creature_states.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_planar.h"
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
#include "config_keeperfx.h"
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
#include "map_data.h"
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
#include "magic_powers.h"
#include "sounds.h"
#include "game_legacy.h"
#include "sprites.h"
#include "lua_cfg_funcs.h"

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
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CREATURE_GUI_STATES_COUNT 3
/* Please note that functions returning 'short' are not ment to return true/false only! */
/******************************************************************************/
/******************************************************************************/
short already_at_call_to_arms(struct Thing *creatng);
short arrive_at_alarm(struct Thing *creatng);
short arrive_at_call_to_arms(struct Thing *creatng);
short cleanup_hold_audience(struct Thing *creatng);
short creature_being_dropped(struct Thing *creatng);
short creature_cannot_find_anything_to_do(struct Thing *creatng);
short creature_casting_preparation(struct Thing *creatng);
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
short creature_kill_diggers(struct Thing* creatng);
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
CrCheckRet move_check_kill_diggers(struct Thing* creatng);
CrCheckRet move_check_near_dungeon_heart(struct Thing *creatng);
CrCheckRet move_check_on_head_for_room(struct Thing *creatng);
CrCheckRet move_check_persuade(struct Thing *creatng);
CrCheckRet move_check_wait_at_door_for_wage(struct Thing *creatng);
short cleanup_timebomb(struct Thing *creatng);

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
short cleanup_creature_leaves_or_dies(struct Thing* creatng);
short state_cleanup_unconscious(struct Thing *creatng);
short state_cleanup_wait_at_door(struct Thing* creatng);
short creature_search_for_spell_to_steal_in_room(struct Thing *creatng);
short creature_pick_up_spell_to_steal(struct Thing *creatng);
short creature_timebomb(struct Thing *creatng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

const struct NamedCommand process_func_commands[] = {
    {"NULL",                                         0},
    {"imp_doing_nothing",                            1},
    {"imp_arrives_at_dig_or_mine",                   2},
    {"imp_digs_mines",                               3},
    {"creature_casting_preparation",                 4},
    {"imp_drops_gold",                               5},
    {"imp_last_did_job",                             6},
    {"imp_arrives_at_improve_dungeon",               7},
    {"imp_improves_dungeon",                         8},
    {"creature_picks_up_trap_object",                9},
    {"creature_arms_trap",                          10},
    {"creature_picks_up_crate_for_workshop",        11},
    {"move_to_position",                            12},
    {"creature_drops_crate_in_workshop",            13},
    {"creature_doing_nothing",                      14},
    {"creature_to_garden",                          15},
    {"creature_arrived_at_garden",                  16},
    {"creature_wants_a_home",                       17},
    {"creature_choose_room_for_lair_site",          18},
    {"creature_at_new_lair",                        19},
    {"person_sulk_head_for_lair",                   20},
    {"person_sulk_at_lair",                         21},
    {"creature_going_home_to_sleep",                22},
    {"creature_sleep",                              23},
    {"tunnelling",                                  24},
    {"at_research_room",                            25},
    {"researching",                                 26},
    {"at_training_room",                            27},
    {"training",                                    28},
    {"good_doing_nothing",                          29},
    {"good_returns_to_start",                       30},
    {"good_back_at_start",                          31},
    {"good_drops_gold",                             32},
    {"arrive_at_call_to_arms",                      33},
    {"creature_arrived_at_prison",                  34},
    {"creature_in_prison",                          35},
    {"at_torture_room",                             36},
    {"torturing",                                   37},
    {"at_workshop_room",                            38},
    {"manufacturing",                               39},
    {"at_scavenger_room",                           40},
    {"scavengering",                                41},
    {"creature_dormant",                            42},
    {"creature_in_combat",                          43},
    {"creature_leaving_dungeon",                    44},
    {"creature_leaves",                             45},
    {"creature_in_hold_audience",                   46},
    {"patrol_here",                                 47},
    {"patrolling",                                  48},
    {"creature_kill_creatures",                     49},
    {"creature_kill_diggers",                       50},
    {"person_sulking",                              51},
    {"at_barrack_room",                             52},
    {"barracking",                                  53},
    {"creature_slap_cowers",                        54},
    {"creature_unconscious",                        55},
    {"creature_pick_up_unconscious_body",           56},
    {"imp_toking",                                  57},
    {"imp_picks_up_gold_pile",                      58},
    {"move_backwards_to_position",                  59},
    {"creature_drop_body_in_prison",                60},
    {"imp_arrives_at_convert_dungeon",              61},
    {"imp_converts_dungeon",                        62},
    {"creature_wants_salary",                       63},
    {"creature_take_salary",                        64},
    {"tunneller_doing_nothing",                     65},
    {"creature_object_combat",                      66},
    {"creature_change_lair",                        67},
    {"imp_birth",                                   68},
    {"at_temple",                                   69},
    {"praying_in_temple",                           70},
    {"creature_follow_leader",                      71},
    {"creature_door_combat",                        72},
    {"creature_combat_flee",                        73},
    {"creature_sacrifice",                          74},
    {"at_lair_to_sleep",                            75},
    {"creature_exempt",                             76},
    {"creature_being_dropped",                      77},
    {"creature_being_sacrificed",                   78},
    {"creature_scavenged_disappear",                79},
    {"creature_scavenged_reappear",                 80},
    {"creature_being_summoned",                     81},
    {"creature_hero_entering",                      82},
    {"imp_arrives_at_reinforce",                    83},
    {"imp_reinforces",                              84},
    {"arrive_at_alarm",                             85},
    {"creature_picks_up_spell_object",              86},
    {"creature_drops_spell_object_in_library",      87},
    {"creature_picks_up_corpse",                    88},
    {"creature_drops_corpse_in_graveyard",          89},
    {"at_guard_post_room",                          90},
    {"guarding",                                    91},
    {"creature_eat",                                92},
    {"creature_evacuate_room",                      93},
    {"creature_wait_at_treasure_room_door",         94},
    {"at_kinky_torture_room",                       95},
    {"kinky_torturing",                             96},
    {"mad_killing_psycho",                          97},
    {"creature_search_for_gold_to_steal_in_room",   98},
    {"creature_vandalise_rooms",                    99},
    {"creature_steal_gold",                        100},
    {"seek_the_enemy",                             101},
    {"already_at_call_to_arms",                    102},
    {"creature_damage_walls",                      103},
    {"creature_attempt_to_damage_walls",           104},
    {"creature_persuade",                          105},
    {"creature_change_to_chicken",                 106},
    {"creature_change_from_chicken",               107},
    {"creature_cannot_find_anything_to_do",        108},
    {"creature_piss",                              109},
    {"creature_roar",                              110},
    {"creature_at_changed_lair",                   111},
    {"creature_be_happy",                          112},
    {"good_leave_through_exit_door",               113},
    {"good_wait_in_exit_door",                     114},
    {"good_attack_room",                           115},
    {"good_arrived_at_attack_room",                116},
    {"creature_pretend_chicken_setup_move",        117},
    {"creature_pretend_chicken_move",              118},
    {"creature_attack_rooms",                      119},
    {"creature_freeze_prisoners",                  120},
    {"creature_explore_dungeon",                   121},
    {"creature_eating_at_garden",                  122},
    {"creature_leaves_or_dies",                    123},
    {"creature_moan",                              124},
    {"creature_set_work_room_based_on_position",   125},
    {"creature_being_scavenged",                   126},
    {"creature_escaping_death",                    127},
    {"creature_present_to_dungeon_heart",          128},
    {"creature_search_for_spell_to_steal_in_room", 129},
    {"creature_pick_up_spell_to_steal",            130},
    {"creature_going_to_safety_for_toking",        131},
    {"creature_timebomb",                          132},
    {"good_arrived_at_combat",                     133},
    {"good_arrived_at_attack_dungeon_heart",       134},
    {"creature_drop_unconscious_in_lair",          135},
    {"creature_save_unconscious_creature",         136},
    {NULL,                                           0},
};

const CreatureStateFunc1 process_func_list[] = {
    NULL,
    imp_doing_nothing,
    imp_arrives_at_dig_or_mine,
    imp_digs_mines,
    creature_casting_preparation,
    imp_drops_gold,
    imp_last_did_job,
    imp_arrives_at_improve_dungeon,
    imp_improves_dungeon,
    creature_picks_up_trap_object,
    creature_arms_trap,
    creature_picks_up_crate_for_workshop,
    move_to_position,
    creature_drops_crate_in_workshop,
    creature_doing_nothing,
    creature_to_garden,
    creature_arrived_at_garden,
    creature_wants_a_home,
    creature_choose_room_for_lair_site,
    creature_at_new_lair,
    person_sulk_head_for_lair,
    person_sulk_at_lair,
    creature_going_home_to_sleep,
    creature_sleep,
    tunnelling,
    at_research_room,
    researching,
    at_training_room,
    training,
    good_doing_nothing,
    good_returns_to_start,
    good_back_at_start,
    good_drops_gold,
    arrive_at_call_to_arms,
    creature_arrived_at_prison,
    creature_in_prison,
    at_torture_room,
    torturing,
    at_workshop_room,
    manufacturing,
    at_scavenger_room,
    scavengering,
    creature_dormant,
    creature_in_combat,
    creature_leaving_dungeon,
    creature_leaves,
    creature_in_hold_audience,
    patrol_here,
    patrolling,
    creature_kill_creatures,
    creature_kill_diggers,
    person_sulking,
    at_barrack_room,
    barracking,
    creature_slap_cowers,
    creature_unconscious,
    creature_pick_up_unconscious_body,
    imp_toking,
    imp_picks_up_gold_pile,
    move_backwards_to_position,
    creature_drop_body_in_prison,
    imp_arrives_at_convert_dungeon,
    imp_converts_dungeon,
    creature_wants_salary,
    creature_take_salary,
    tunneller_doing_nothing,
    creature_object_combat,
    creature_change_lair,
    imp_birth,
    at_temple,
    praying_in_temple,
    creature_follow_leader,
    creature_door_combat,
    creature_combat_flee,
    creature_sacrifice,
    at_lair_to_sleep,
    creature_exempt,
    creature_being_dropped,
    creature_being_sacrificed,
    creature_scavenged_disappear,
    creature_scavenged_reappear,
    creature_being_summoned,
    creature_hero_entering,
    imp_arrives_at_reinforce,
    imp_reinforces,
    arrive_at_alarm,
    creature_picks_up_spell_object,
    creature_drops_spell_object_in_library,
    creature_picks_up_corpse,
    creature_drops_corpse_in_graveyard,
    at_guard_post_room,
    guarding,
    creature_eat,
    creature_evacuate_room,
    creature_wait_at_treasure_room_door,
    at_kinky_torture_room,
    kinky_torturing,
    mad_killing_psycho,
    creature_search_for_gold_to_steal_in_room,
    creature_vandalise_rooms,
    creature_steal_gold,
    seek_the_enemy,
    already_at_call_to_arms,
    creature_damage_walls,
    creature_attempt_to_damage_walls,
    creature_persuade,
    creature_change_to_chicken,
    creature_change_from_chicken,
    creature_cannot_find_anything_to_do,
    creature_piss,
    creature_roar,
    creature_at_changed_lair,
    creature_be_happy,
    good_leave_through_exit_door,
    good_wait_in_exit_door,
    good_attack_room,
    good_arrived_at_attack_room,
    creature_pretend_chicken_setup_move,
    creature_pretend_chicken_move,
    creature_attack_rooms,
    creature_freeze_prisoners,
    creature_explore_dungeon,
    creature_eating_at_garden,
    creature_leaves_or_dies,
    creature_moan,
    creature_set_work_room_based_on_position,
    creature_being_scavenged,
    creature_escaping_death,
    creature_present_to_dungeon_heart,
    creature_search_for_spell_to_steal_in_room,
    creature_pick_up_spell_to_steal,
    creature_going_to_safety_for_toking,
    creature_timebomb,
    good_arrived_at_combat,
    good_arrived_at_attack_dungeon_heart,
    creature_drop_unconscious_in_lair,
    creature_save_unconscious_creature,
};

const struct NamedCommand cleanup_func_commands[] = {
    {"none",                                0},
    {"state_cleanup_dragging_object",       1},
    {"state_cleanup_in_room",               2},
    {"cleanup_sleep",                       3},
    {"state_cleanup_unable_to_fight",       4},
    {"cleanup_prison",                      5},
    {"cleanup_torturing",                   6},
    {"cleanup_combat",                      7},
    {"cleanup_hold_audience",               8},
    {"state_cleanup_unconscious",           9},
    {"state_cleanup_dragging_body",        10},
    {"cleanup_object_combat",              11},
    {"state_cleanup_in_temple",            12},
    {"cleanup_door_combat",                13},
    {"cleanup_sacrifice",                  14},
    {"state_cleanup_wait_at_door",         15},
    {"cleanup_seek_the_enemy",             16},
    {"cleanup_creature_leaves_or_dies",    17},
    {"cleanup_timebomb",                   18},
    {NULL,                                  0},
};

const CreatureStateFunc1 cleanup_func_list[] = {
    NULL,
    state_cleanup_dragging_object,
    state_cleanup_in_room,
    cleanup_sleep,
    state_cleanup_unable_to_fight,
    cleanup_prison,
    cleanup_torturing,
    cleanup_combat,
    cleanup_hold_audience,
    state_cleanup_unconscious,
    state_cleanup_dragging_body,
    cleanup_object_combat,
    state_cleanup_in_temple,
    cleanup_door_combat,
    cleanup_sacrifice,
    state_cleanup_wait_at_door,
    cleanup_seek_the_enemy,
    cleanup_creature_leaves_or_dies,
    cleanup_timebomb,
};

const struct NamedCommand move_from_slab_func_commands[] = {
    {"none",                                         0},
    {"new_slab_tunneller_check_for_breaches",        1},
    {NULL,                                           0},
};

const CreatureStateFunc2 move_from_slab_func_list[] = {
    NULL,
    new_slab_tunneller_check_for_breaches
};

const struct NamedCommand move_check_func_commands[] = {
    {"none",                               0},
    {"move_check_on_head_for_room",        1},
    {"process_research_function",          2},
    {"process_prison_function",            3},
    {"process_torture_function",           4},
    {"process_scavenge_function",          5},
    {"move_check_near_dungeon_heart",      6},
    {"move_check_kill_creatures",          7},
    {"move_check_kill_diggers",            8},
    {"move_check_wait_at_door_for_wage",   9},
    {"process_temple_function",           10},
    {"process_kinky_function",            11},
    {"move_check_attack_any_door",        12},
    {"move_check_can_damage_wall",        13},
    {"move_check_persuade",               14},
};

const CreatureStateCheck move_check_func_list[] = {
    NULL,
    move_check_on_head_for_room,
    process_research_function,
    process_prison_function,
    process_torture_function,
    process_scavenge_function,
    move_check_near_dungeon_heart,
    move_check_kill_creatures,
    move_check_kill_diggers,
    move_check_wait_at_door_for_wage,
    process_temple_function,
    process_kinky_function,
    move_check_attack_any_door,
    move_check_can_damage_wall,
    move_check_persuade,
};

/** GUI States of creatures - from "Creatures" Tab in UI.
 * There are three states:
 * - 0: Idle.
 * - 1: Working.
 * - 2: Fighting.
 */
long const state_type_to_gui_state[STATE_TYPES_COUNT] = {
    0, 1, 0, 0, 0, 2, 0, 0, 1, 0, 0, 2, 2, 1, 1,
};

/******************************************************************************/
struct CreatureStateConfig *get_thing_active_state_info(struct Thing *thing)
{
  if (thing->active_state >= CREATURE_STATES_COUNT)
    return &game.conf.crtr_conf.states[0];
  return &game.conf.crtr_conf.states[thing->active_state];
}

struct CreatureStateConfig *get_thing_continue_state_info(struct Thing *thing)
{
    if (thing->continue_state >= CREATURE_STATES_COUNT)
        return &game.conf.crtr_conf.states[0];
    return &game.conf.crtr_conf.states[thing->continue_state];
}

struct CreatureStateConfig *get_thing_state_info_num(CrtrStateId state_id)
{
    if (state_id >= CREATURE_STATES_COUNT)
        return &game.conf.crtr_conf.states[0];
    return &game.conf.crtr_conf.states[state_id];
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

struct CreatureStateConfig *get_creature_state_with_task_completion(struct Thing *thing)
{
    struct CreatureStateConfig* stati = get_thing_active_state_info(thing);
    if (stati->state_type == CrStTyp_Move)
        stati = get_thing_continue_state_info(thing);
    return stati;
}

TbBool state_info_invalid(struct CreatureStateConfig *stati)
{
  if (stati <= &game.conf.crtr_conf.states[0])
    return true;
  return false;
}

TbBool creature_model_bleeds(unsigned long crmodel)
{
    struct CreatureModelConfig* crconf = creature_stats_get(crmodel);
    if (censorship_enabled())
    {
        // If censorship is on, only evil creatures can have blood
        if (!crconf->bleeds)
            return false;
        return ((crconf->model_flags & CMF_IsEvil) != 0);
  }
  return crconf->bleeds;
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
  unsigned long state = thing->active_state;
  if ( (state > 0) && (state < CREATURE_STATES_COUNT) )
  {
      state_type = game.conf.crtr_conf.states[state].state_type;
  } else
  {
      state_type = game.conf.crtr_conf.states[0].state_type;
      WARNLOG("%s: The %s index %d active state %lu (%s) is out of range",
        func_name,thing_model_name(thing),(int)thing->index,state,creature_state_code_name(state));
  }
  if (state_type == CrStTyp_Move)
  {
      state = thing->continue_state;
      if ( (state > 0) && (state < CREATURE_STATES_COUNT) )
      {
          state_type = game.conf.crtr_conf.states[state].state_type;
      } else
      {
          state_type = game.conf.crtr_conf.states[0].state_type;
          // Show message with text name of active state - it's good as the state was checked before
          WARNLOG("%s: The %s index %d owner %d continue state %lu (%s) is out of range; active state %u (%s)",func_name,
              thing_model_name(thing),(int)thing->index,(int)thing->owner,state,creature_state_code_name(state),thing->active_state,creature_state_code_name(thing->active_state));
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
        if (thing_is_invalid(thing))
            return false;
        CrtrStateId i = thing->active_state;
        if ((i == CrSt_MoveToPosition) || (i == CrSt_MoveBackwardsToPosition))
            i = thing->continue_state;
        if (i == CrSt_CreatureUnconscious)
            return true;
        return false;
}

TbBool creature_can_be_set_unconscious(const struct Thing *creatng, const struct Thing *killertng, CrDeathFlags flags)
{
    if (flag_is_set(flags, CrDed_NoUnconscious))
    {
        return false;
    }
    if (THING_RANDOM(creatng, 100) >= game.conf.rules[creatng->owner].creature.stun_without_prison_chance)
    {
        if (!player_has_room_of_role(killertng->owner, RoRoF_Prison))
        {
            return false;
        }
        if (!player_creature_tends_to(killertng->owner, CrTend_Imprison))
        {
            return false;
        }
    }
    if (flag_is_set(get_creature_model_flags(creatng), CMF_IsEvil) && (THING_RANDOM(creatng, 100) >= game.conf.rules[creatng->owner].creature.stun_enemy_chance_evil))
    {
        return false;
    }
    if (!flag_is_set(get_creature_model_flags(creatng), CMF_IsEvil) && (THING_RANDOM(creatng, 100) >= game.conf.rules[creatng->owner].creature.stun_enemy_chance_good))
    {
        return false;
    }
    if (flag_is_set(get_creature_model_flags(creatng), CMF_NoImprisonment))
    {
        return false;
    }
    return true;
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

TbBool creature_is_leaving_and_cannot_be_stopped(const struct Thing *thing)
{
    CrtrStateId i = get_creature_state_besides_interruptions(thing);
    if (i == CrSt_LeavesBecauseOwnerLost)
        return true;
    return false;
}

TbBool creature_is_being_sacrificed(const struct Thing* thing)
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
    if (i == CrSt_CreatureBeingSummoned)
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
    if (game.conf.crtr_conf.states[i].state_type == CrStTyp_AngerJob)
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
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    return cctrl->called_to_arms;
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
        if (room_is_invalid(room) && !creature_is_being_dropped(thing))
        {
            //If the creature is not inside a room, or moving
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            room = get_room_at_pos(&cctrl->moveto_pos);
            if (room_is_invalid(room))
            {
                // This must mean we're being dropped outside of room, or sold/destroyed the room so not kept in custody - freed
                return false;
            }
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
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            room = get_room_at_pos(&cctrl->moveto_pos);
            if (room_is_invalid(room))
            {
                // This must mean we're being dropped outside of room, or sold/destroyed the room so not kept in custody - freed
                return false;
            }
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
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            room = get_room_at_pos(&cctrl->moveto_pos);
            if (room_is_invalid(room))
            {
                // This must mean we're being dropped outside of room, or sold/destroyed the room so not kept in custody - freed
                return -1;
            }
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
    if (game.conf.crtr_conf.states[crstate].state_type == 0)
        return true;
    return false;
}

TbBool restore_creature_flight_flag(struct Thing *creatng)
{
    struct CreatureModelConfig *crconf = creature_stats_get_from_thing(creatng);
    // Creature can fly either by spell or naturally.
    if ((crconf->flying) || creature_under_spell_effect(creatng, CSAfF_Flying))
    {
        // Even flying creature is grounded while frozen.
        if (!creature_under_spell_effect(creatng, CSAfF_Freeze))
        {
            set_flag(creatng->movement_flags, TMvF_Flying);
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
    if (cctrl->alarm_over_turn < (unsigned long)game.play_gameturn)
    {
        set_start_state(creatng);
        return 1;
    }
    if (THING_RANDOM(creatng, 4) == 0)
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
        output_message(SMsg_EnemyDestroyRooms, MESSAGE_DURATION_FIGHT);
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
    struct Thing* cmbttng = check_for_door_to_fight(creatng);
    if (thing_exists(cmbttng))
    {
        set_creature_door_combat(creatng, cmbttng);
        return 2;
    }
    cmbttng = check_for_object_to_fight(creatng);
    if (thing_exists(cmbttng))
    {
        set_creature_object_combat(creatng, cmbttng);
        return 2;
    }
    if (!attempt_to_destroy_enemy_room(creatng, dungeon->cta_stl_x, dungeon->cta_stl_y))
    {
      if (THING_RANDOM(creatng, 7) == 0)
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
                    if (!thing_in_wall_at_with_radius(thing, pos, block_radius))
                    {
                        return true;
                    }
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
 * Fills given array with information about game.small_around_slab[] elements which are accessible to move to.
 * @param avail The array of size SMALL_AROUND_SLAB_LENGTH
 * @param thing The thing which is to be moved to adjacent tile
 * @param room The room inside which we want to move
 * @return
 */
TbBool fill_moveable_small_around_slabs_array_in_room(TbBool *avail, const struct Thing *thing, const struct Room *room)
{
    long slab_base = get_slab_number(subtile_slab(thing->mappos.x.stl.num), subtile_slab(thing->mappos.y.stl.num));
    // Fill the avail[] array
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        long slab_num = slab_base + game.small_around_slab[n];
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
    struct Coord3d locpos;
    if (creature_find_safe_position_to_move_within_slab(&locpos, thing, slb_x, slb_y, start_stl))
    {
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
    MapSlabCoord slb_x = subtile_slab(thing->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab(thing->mappos.y.stl.num);
    long slab_base = get_slab_number(slb_x, slb_y);

    int start_stl = THING_RANDOM(thing, AROUND_MAP_LENGTH);
    long m = THING_RANDOM(thing, SMALL_AROUND_SLAB_LENGTH);
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        long slab_num = slab_base + game.small_around_slab[m];
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
        slb_x = subtile_slab(thing->mappos.x.stl.num);
        slb_y = subtile_slab(thing->mappos.y.stl.num);
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
        pos->x.val = subtile_coord_center(game.map_subtiles_x/2);
        pos->y.val = subtile_coord_center(game.map_subtiles_y/2);
        pos->z.val = subtile_coord(1,0);
        return false;
    }
    long slab_base = get_slab_number(subtile_slab(thing->mappos.x.stl.num), subtile_slab(thing->mappos.y.stl.num));
    long start_stl = THING_RANDOM(thing, STL_PER_SLB * STL_PER_SLB);
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
                long long tmp = thing->move_angle_xy + DEGREES_90;
                tmp = ((((tmp>>32) ^ (((tmp>>32) ^ tmp) - (tmp>>32))) & ANGLE_MASK) - (tmp>>32));
                arnd = 2 * ((((tmp>>16) & 0x3FF) + tmp) / DEGREES_180);
            } else {
                arnd = avail[1] ? 0 : 3;
            }
        } else
        if (avail[1]) // can go to sibling slab (1,0)
        {
            if (avail[3]) {
                arnd = 2 * ((thing->move_angle_xy & DEGREES_180) / DEGREES_180) + 1;
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
                long slab_num = slab_base + game.small_around_slab[arnd];
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
    struct SlabMap* slb;// = get_slabmap_for_subtile(pos->x.stl.num, pos->y.stl.num);
    struct Room* room;
    SubtlCodedCoords stl_num;
    long m = THING_RANDOM(thing, AROUND_MAP_LENGTH);
    for (long n = 0; n < AROUND_MAP_LENGTH; n++)
    {
        SubtlCodedCoords accepted_stl_num = 0;
        stl_num = get_subtile_number(pos->x.stl.num,pos->y.stl.num);
        // Skip the position equal to current position
        if (game.around_map[m] == 0)
        {
            m = (m + 1) % AROUND_MAP_LENGTH;
            continue;
        }
        // Move radially from of the current position; stop if a room tile
        // of incorrect kind or owner is encoured
        for (long dist = 0; dist < 8; dist++)
        {
            stl_num += game.around_map[m];
            struct Map* mapblk = get_map_block_at_pos(stl_num);
            if ( ((mapblk->flags & SlbAtFlg_IsRoom) != 0) && ((mapblk->flags & SlbAtFlg_Blocking) != 0) )
                break;
            MapSubtlCoord stl_x = stl_num_decode_x(stl_num);
            MapSubtlCoord stl_y = stl_num_decode_y(stl_num);
            slb = get_slabmap_for_subtile(stl_x, stl_y);
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
        {
            return accepted_stl_num;
        }
        m = (m + 1) % AROUND_MAP_LENGTH;
    }
    // No position found - at least try to move within the same slab
    slb = get_slabmap_for_subtile(pos->x.stl.num, pos->y.stl.num);
    room = room_get(slb->room_index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (person_get_somewhere_adjacent_in_room(thing, room, &cctrl->moveto_pos))
    {
        return get_subtile_number(cctrl->moveto_pos.x.stl.num, cctrl->moveto_pos.y.stl.num);
    }
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
    cctrl->seek_enemy.enemy_idx = 0;
    cctrl->seek_enemy.enemy_creation_turn = 0;
    return 1;
}

short cleanup_timebomb(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->max_speed = calculate_correct_creature_maxspeed(creatng);
    cctrl->timebomb_target_id = 0;
    return 0;
}

short creature_being_dropped(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(17,"Starting for %s index %ld",thing_model_name(creatng),(long)creatng->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->creature_control_flags |= CCFlg_NoCompControl;
    // Cannot teleport for a few turns after being dropped
    delay_teleport(creatng);
    cctrl->dropped_turn = game.play_gameturn;
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
    CreatureJob new_job;
    struct Room* room = room_get(slb->room_index);
    if ((!room_is_invalid(room)) && (room->owner != creatng->holding_player))
    {
        new_job = Job_NULL;
    }
    else
    {
        new_job = get_job_for_subtile(creatng, stl_x, stl_y, JoKF_AssignHumanDrop);
    }
    // Most tasks are disabled while creature is a chicken
    if (!creature_under_spell_effect(creatng, CSAfF_Chicken))
    {
        // For creatures with trembling and not changed to chickens, tremble the camera
        if ((get_creature_model_flags(creatng) & CMF_Trembling) != 0)
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
                cctrl->creature_control_flags |= CCFlg_NoCompControl;
            } else {
                cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
            }
        } else
        {
            cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
        }
        // Reveal any nearby terrain
        check_map_explored(creatng, stl_x, stl_y);
        // Creatures dropped far from group are removed from it
        struct Thing* leadtng = get_group_leader(creatng);
        if (thing_is_creature(leadtng))
        {
            if (leadtng->index != creatng->index)
            {
                if (!thing_is_picked_up(leadtng))
                {
                    if (get_chessboard_distance(&creatng->mappos, &leadtng->mappos) > subtile_coord(9,0)) {
                        SYNCDBG(3,"Removing %s index %d owned by player %d from group",
                            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
                        remove_creature_from_group(creatng);
                    }
                }
            }
        }
    }
    // TODO CREATURE_JOBS it would be great if these jobs were also returned by get_job_for_subtile()
    if (!creature_under_spell_effect(creatng, CSAfF_Chicken))
    {
        // Special tasks for diggers
        if (thing_is_creature_digger(creatng))
        {
            if ((slabmap_owner(slb) == creatng->owner) || (slabmap_owner(slb) == game.neutral_player_num))
            {
                if (check_out_available_spdigger_drop_tasks(creatng))
                {
                    SYNCDBG(3, "The %s index %d owner %d found digger job at (%d,%d)",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,(int)stl_x,(int)stl_y);
                    cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
                    delay_heal_sleep(creatng);
                    return 2;
                }
                else
                {
                    cctrl->healing_sleep_check_turn = game.play_gameturn;
                }
            }
        }
        if (creature_under_spell_effect(creatng, CSAfF_Fear))
        {
            external_set_thing_state(creatng, CrSt_CreatureCombatFlee);
            return 2;
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
            if (creature_look_for_enemy_object_combat(creatng)) {
                SYNCDBG(3, "The %s index %d owner %d found enemy combat at (%d,%d)", thing_model_name(creatng), (int)creatng->index, (int)creatng->owner, (int)stl_x, (int)stl_y);
                return 2;
            }
        }
        if (new_job != Job_NULL)
        {
            // Make sure computer control flag is set accordingly to job, now do it straight and without exclusions
            if ((get_flags_for_job(new_job) & JoKF_NoSelfControl) != 0) {
                cctrl->creature_control_flags |= CCFlg_NoCompControl;
            } else {
                cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
            }
        } else
        {
            cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
        }
    }
    if (new_job == Job_NULL)
    {
        SYNCDBG(3,"No job found at (%d,%d) for %s index %d owner %d",(int)stl_x,(int)stl_y,thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        // Job_NULL is already assigned here, and default state is already initialized
        cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
        return 2;
    }
    SYNCDBG(3,"Job %s to be assigned to %s index %d owner %d",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    // Check if specific conditions are met for this job to be assigned
    if (!creature_can_do_job_near_position(creatng, stl_x, stl_y, new_job, JobChk_SetStateOnFail|JobChk_PlayMsgOnFail))
    {
        SYNCDBG(16,"Cannot assign job %s to %s (owner %d)",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->owner);
        cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
        return 2;
    }
    // Now try sending the creature to do job it should do at this position
    if (!send_creature_to_job_near_position(creatng, stl_x, stl_y, new_job))
    {
        SYNCDBG(13,"Cannot assign %s to %s index %d owner %d; could not send to room",creature_job_code_name(new_job),thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
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
    if ((game.play_gameturn - cctrl->countdown) >= 128)
    {
        set_start_state(creatng);
        return 0;
    }
    if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
        creatng->continue_state = CrSt_CreatureCannotFindAnythingToDo;
    return 1;
}

/**
 * @brief process_state function for CreatureCastingPreparation.
 *
 * @param creatng The creature being updated.
 * @return short What we've done to the creature. Enum of CreatureStateReturns.
 */
short creature_casting_preparation(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    TRACE_THING(creatng);
    SYNCDBG(11, "Process %s(%d), bkp act.st: %s, bkp con.st: %s, instance: %s",
        thing_model_name(creatng), creatng->index,
        creature_state_code_name(cctrl->active_state_bkp), creature_state_code_name(cctrl->continue_state_bkp),
        creature_instance_code_name(cctrl->instance_id));

    if (cctrl->instance_id != CrInst_NULL)
    {
        // If the spell has not be casted, keep this state.
        struct Thing* target = thing_get(cctrl->targtng_idx);
        if (target != INVALID_THING)
        {
            // Try to keep facing to the target.
            creature_turn_to_face(creatng, &target->mappos);
        }
        return CrStRet_Unchanged;
    }
    internal_set_thing_state(creatng, cctrl->active_state_bkp);
    creatng->continue_state = cctrl->continue_state_bkp;
    return CrStRet_Modified;
}

void set_creature_size_stuff(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (creature_under_spell_effect(creatng, CSAfF_Chicken))
    {
        creatng->sprite_size = game.conf.crtr_conf.sprite_size;
    }
    else
    {
        creatng->sprite_size = game.conf.crtr_conf.sprite_size + (game.conf.crtr_conf.sprite_size * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100;
    }
}

short creature_change_from_chicken(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    creature_set_speed(creatng, 0);
    if (cctrl->countdown > 0)
    {
        cctrl->countdown--;
    }
    if (cctrl->countdown > 0)
    { // Changing under way - gradually modify size of the creature.
        set_flag(creatng->rendering_flags, TRF_Invisible);
        set_flag(creatng->size_change, TSC_ChangeSize);
        struct Thing *efftng = create_effect_element(&creatng->mappos, TngEffElm_Chicken, creatng->owner);
        if (!thing_is_invalid(efftng))
        {
            long n = (10 - cctrl->countdown) * (game.conf.crtr_conf.sprite_size + (game.conf.crtr_conf.sprite_size * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100) / 10;
            unsigned long k = get_creature_anim(creatng, 0);
            set_thing_draw(efftng, k, 256, n, -1, 0, ODC_Default);
            clear_flag(efftng->rendering_flags, TRF_Transpar_Flags);
            set_flag(efftng->rendering_flags, TRF_Transpar_8);
        }
        return 0;
    }
    else
    {
        clear_flag(creatng->rendering_flags, TRF_Invisible);
        clear_flag(cctrl->stateblock_flags, CCSpl_ChickenRel);
        clear_flag(cctrl->spell_flags, CSAfF_Chicken);
        set_creature_size_stuff(creatng);
        set_start_state(creatng);
        return 1;
    }
}

short creature_change_to_chicken(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    creature_set_speed(creatng, 0);
    if (cctrl->countdown > 0)
    {
        cctrl->countdown--;
    }
    if (cctrl->countdown > 0)
    {
        set_flag(creatng->size_change, TSC_ChangeSize);
        set_flag(creatng->rendering_flags, TRF_Invisible);
        struct Thing *efftng = create_effect_element(&creatng->mappos, TngEffElm_Chicken, creatng->owner);
        if (!thing_is_invalid(efftng))
        {
            unsigned long k = convert_td_iso(819);
            set_thing_draw(efftng, k, 0, 1200 * cctrl->countdown / 10 + game.conf.crtr_conf.sprite_size, -1, 0, ODC_Default);
            clear_flag(efftng->rendering_flags, TRF_Transpar_Flags);
            set_flag(efftng->rendering_flags, TRF_Transpar_8);
        }
        return 0;
    }
    // Apparently, this function bypasses the immunity, as no check was made against CMF_NeverChickens.
    set_flag(cctrl->spell_flags, CSAfF_Chicken);
    clear_flag(creatng->rendering_flags, TRF_Invisible);
    set_creature_size_stuff(creatng);
    creatng->active_state = CrSt_CreaturePretendChickenSetupMove;
    creatng->continue_state = CrSt_Unused;
    cctrl->stopped_for_hand_turns = 0;
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
    if ((!room_role_matches(room->kind,get_room_role_for_job(Job_TAKE_SLEEP))) || (room->owner != creatng->owner)) {
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
    if (creature_under_spell_effect(creatng, CSAfF_MadKilling)) {
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
            struct Room* room = find_nearest_room_of_role_for_thing_with_spare_capacity(creatng, creatng->owner, get_room_role_for_job(Job_TAKE_SLEEP), NavRtF_Default, required_cap);
            if (!room_is_invalid(room))
            {
                internal_set_thing_state(creatng, CrSt_CreatureWantsAHome);
                SYNCDBG(8,"The %s index %d goes make lair",thing_model_name(creatng),creatng->index);
                return 1;
            }
            update_cannot_find_room_of_role_wth_spare_capacity_event(creatng->owner, creatng, get_room_role_for_job(Job_TAKE_SLEEP));
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
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    if ((crconf->job_primary != Job_NULL) && (game.play_gameturn - cctrl->job_primary_check_turn > 128))
    {
        if (attempt_job_preference(creatng, crconf->job_primary)) {
            SYNCDBG(8,"The %s index %d will do primary job with state %s",thing_model_name(creatng),
                (int)creatng->index,creature_state_code_name(get_creature_state_besides_interruptions(creatng)));
            return 1;
        }
        cctrl->job_primary_check_turn = game.play_gameturn;
    }
    long n = THING_RANDOM(creatng, 3);
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
    cctrl->countdown = game.play_gameturn;
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
    struct SlabConfigStats* slabst = get_slab_stats(slb);
    if ( ((slabst->block_flags & SlbAtFlg_IsRoom) != 0) || ((slabst->block_flags & SlbAtFlg_Blocking) == 0) )
        return true;
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    struct Thing* doortng = get_door_for_position(stl_x, stl_y);
    if (!thing_is_invalid(doortng))
    {
      if (door_will_open_for_thing(doortng,thing))
          return true;
    }
    return false;
}

TbBool creature_choose_random_destination_on_valid_adjacent_slab(struct Thing *thing)
{
    SYNCDBG(17,"Starting for %s index %ld",thing_model_name(thing),(long)thing->index);
    MapSlabCoord slb_x = subtile_slab(thing->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab(thing->mappos.y.stl.num);
    long slab_base = get_slab_number(slb_x, slb_y);

    MapSubtlCoord start_stl = THING_RANDOM(thing, 9);
    long m = THING_RANDOM(thing, SMALL_AROUND_SLAB_LENGTH);
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        long slab_num = slab_base + game.small_around_slab[m];
        slb_x = slb_num_decode_x(slab_num);
        slb_y = slb_num_decode_y(slab_num);
        if (slab_is_valid_for_creature_choose_move(thing, slb_x, slb_y))
        {
            struct Coord3d locpos;
            if (creature_find_safe_position_to_move_within_slab(&locpos, thing, slb_x, slb_y, start_stl))
            {
                if (setup_person_move_to_coord(thing, &locpos, NavRtF_Default))
                {
                    SYNCDBG(8,"Moving thing %s index %d from (%d,%d) to (%d,%d)", thing_model_name(thing),
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
        slb_x = subtile_slab(thing->mappos.x.stl.num);
        slb_y = subtile_slab(thing->mappos.y.stl.num);
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

static long get_best_position_outside_room(struct Thing *creatng, struct Coord3d *pos, struct Room *room)
{
    const struct SlabMap * const current_slb = get_slabmap_for_subtile(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    const int current_slb_kind = current_slb->kind;
    SlabCodedCoords room_slab = room->slabs_list;
    const PlayerNumber current_owner = slabmap_owner(current_slb);

    // pick random slab in room slab list
    const unsigned int room_slb_idx = THING_RANDOM(creatng, room->slabs_count);
    for (int i = 0; i < room_slb_idx; ++i) {
        room_slab = get_slabmap_direct(room_slab)->next_in_room;
    }

    // for each room slab, find a nearby slab that's outside the current room
    for (int j = 0; j < room->slabs_count; ++j)
    {
        for (int i = 0; i < AROUND_SLAB_EIGHT_LENGTH; i++)
        {
            const SlabCodedCoords ar_slb_no = game.around_slab_eight[i] + room_slab;
            const struct SlabMap * const around_slb = get_slabmap_direct(ar_slb_no);
            const PlayerNumber ar_slb_owner = slabmap_owner(around_slb);
            if (is_slab_type_walkable(around_slb->kind) && (around_slb->kind != current_slb_kind || current_owner != ar_slb_owner))
            {
                struct Coord3d target;
                target.x.val = slab_subtile_center(slb_num_decode_x(ar_slb_no));
                target.y.val = slab_subtile_center(slb_num_decode_y(ar_slb_no));
                target.z.val = get_thing_height_at(creatng, pos);
                if (creature_can_navigate_to_with_storage(creatng, &target, 0)) {
                    pos->x.val = target.x.val;
                    pos->y.val = target.y.val;
                    pos->z.val = target.z.val;
                    return 0;
                }
            }
        }
        // wrap around
        if (j + room_slb_idx == room->slabs_count - 1) {
            room_slab = room->slabs_list;
        } else {
            room_slab = get_slabmap_direct(room_slab)->next_in_room;
        }
    }
    return -1;
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
    if (cctrl->evacuate.room_idx != room->index)
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
    pos.x.val = subtile_coord_center(game.map_subtiles_x / 2);
    pos.y.val = subtile_coord_center(game.map_subtiles_y/2);
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
        // Nothing to explore found - do some random movement
        if (THING_RANDOM(creatng, 12) != 0)
        {
            if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
            {
                creatng->continue_state = CrSt_CreatureExploreDungeon;
                return CrCkRet_Continue;
            }
        }
        else
            if (get_random_position_in_dungeon_for_creature(creatng->owner, CrWaS_WithinDungeon, creatng, &pos))
            {
                if (setup_person_move_to_coord(creatng, &pos, NavRtF_Default))
                {
                    creatng->continue_state = CrSt_CreatureExploreDungeon;
                }
                return CrCkRet_Continue;
            }
        SYNCDBG(3, "The %s owned by player %d can't navigate from subtile (%d,%d) to explore",
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
    if (creature_under_spell_effect(creatng, CSAfF_MadKilling))
    {
        SYNCLOG("The %s index %d owned by player %d can no longer be in group - became mad",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        remove_creature_from_group(creatng);
        set_start_state(creatng);
        return 1;
    }
    if (player_keeping_creature_in_custody(creatng) != player_keeping_creature_in_custody(leadtng))
    {
        remove_creature_from_group(creatng);
        return 0;
    }
    struct Coord3d follwr_pos;
    if (!get_free_position_behind_leader(leadtng, &follwr_pos))
    {
        if (thing_is_picked_up(leadtng))
        {
            SYNCDBG(3,"The %s index %d owned by player %d can no longer follow %s - leader is picked up",
                thing_model_name(creatng), (int)creatng->index, (int)creatng->owner, thing_model_name(leadtng));
            remove_creature_from_group(creatng);
            return 0;
        }
        else
        {
            SYNCLOG("The %s index %d owned by player %d can no longer follow %s - no place amongst followers",
                thing_model_name(creatng), (int)creatng->index, (int)creatng->owner, thing_model_name(leadtng));
            set_start_state(creatng);
            return 1;
        }
    }
    int fails_amount = cctrl->follow_leader_fails;
    if (fails_amount > 12) //When set too low, group might disband before a white wall is breached
    {
        SYNCDBG(3,"Removing %s index %d owned by player %d from group due to fails to follow",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
        remove_creature_from_group(creatng);
        return 0;
    }
    if ((fails_amount > 0) && (cctrl->following_leader_since + 16 > game.play_gameturn))
    {
        return 0;
    }
    cctrl->following_leader_since = game.play_gameturn;
    MapCoordDelta distance_to_follower_pos = get_chessboard_distance(&creatng->mappos, &follwr_pos);
    TbBool cannot_reach_leader = creature_cannot_move_directly_to(creatng, &leadtng->mappos);
    int speed = get_creature_speed(leadtng);
    int follower_speed = get_creature_speed(creatng);
    // If we're too far from the designated position, do a speed run
    if (distance_to_follower_pos > subtile_coord(12,0))
    {
        speed = max((2 * speed),(5 * follower_speed / 4));
        if (speed >= MAX_VELOCITY)
            speed = MAX_VELOCITY;
        if (creature_move_to(creatng, &follwr_pos, speed, 0, 0) == -1)
        {
            if (cannot_reach_leader) // only count fails when we're not able to get to the leader, instead of getting a position in the trail it cannot reach.
            {
                cctrl->follow_leader_fails++;
            }
          return 0;
        }
    } else
    // If we're far from the designated position, move considerably faster
    if (distance_to_follower_pos > subtile_coord(6,0))
    {
        if (speed > 4) {
            speed = 5 * speed / 4;
        } else {
            speed = speed + 1;
        }
        speed = max(follower_speed, speed);
        if (speed >= MAX_VELOCITY)
            speed = MAX_VELOCITY;
        if (creature_move_to(creatng, &follwr_pos, speed, 0, 0) == -1)
        {
            if (cannot_reach_leader)
            {
                cctrl->follow_leader_fails++;
            }
          return 0;
        }
    } else
    // If we're close, continue moving at normal speed
    if (distance_to_follower_pos <= subtile_coord(2,0))
    {
        if (distance_to_follower_pos <= 0)
        {
            creature_turn_to_face(creatng, &leadtng->mappos);
        } else
        if (creature_move_to(creatng, &follwr_pos, speed, 0, 0) == -1)
        {
            if (cannot_reach_leader)
            {
                cctrl->follow_leader_fails++;
            }
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
            if (cannot_reach_leader)
            {
                cctrl->follow_leader_fails++;
            }
            return 0;
        }
    }
    cctrl->follow_leader_fails = 0;
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
        set_creature_instance(creatng, CrInst_CELEBRATE_SHORT, 0, 0);
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
    long crtr_idx = THING_RANDOM(creatng, dungeon->num_active_creatrs);
    struct Thing* thing = get_player_list_nth_creature_of_model(dungeon->creatr_list_start, CREATURE_ANY, crtr_idx);
    if (thing_is_invalid(thing)) {
        set_start_state(creatng);
        return 0;
    }
    if (setup_person_move_to_coord(creatng, &thing->mappos, NavRtF_Default)) {
        creatng->continue_state = CrSt_CreatureKillCreatures;
    }
    return 1;
}

short creature_kill_diggers(struct Thing* creatng)
{
    TRACE_THING(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    if (dungeon->num_active_diggers <= 1) {
        set_start_state(creatng);
        return 0;
    }
    long crtr_idx = THING_RANDOM(creatng, dungeon->num_active_diggers);
    struct Thing* thing = get_player_list_nth_creature_of_model(dungeon->digger_list_start, CREATURE_ANY, crtr_idx);
    if (thing_is_invalid(thing)) {
        set_start_state(creatng);
        return 0;
    }
    if (setup_person_move_to_coord(creatng, &thing->mappos, NavRtF_Default)) {
        creatng->continue_state = CrSt_CreatureKillDiggers;
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
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
        apply_anger_to_all_players_creatures_excluding(creatng->owner, crconf->annoy_others_leaving, AngR_Other, creatng);
    }
    kill_creature(creatng, INVALID_THING, -1, CrDed_NoEffects | CrDed_NotReallyDying);
    return CrStRet_Deleted;
}

short setup_creature_leaves_or_dies(struct Thing *creatng)
{
    TRACE_THING(creatng);
    // Try heading for nearest entrance
    struct Room* room = find_nearest_room_of_role_for_thing(creatng, creatng->owner, RoRoF_CrPoolLeave, NavRtF_Default);
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
    cctrl->creature_control_flags |= CCFlg_NoCompControl;
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


short cleanup_creature_leaves_or_dies(struct Thing* creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
    return 1;
}

short creature_leaving_dungeon(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct Room* room = find_nearest_room_of_role_for_thing(creatng, creatng->owner, RoRoF_CrPoolLeave, NavRtF_Default);
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
        output_message(SMsg_CreatureLeaving, MESSAGE_DURATION_CRTR_MOOD);
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
                && !creature_is_being_unconscious(thing) && !creature_is_dying(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
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
    struct Room* room = find_nearest_room_of_role_for_thing(creatng, creatng->owner, RoRoF_CrPoolLeave, NavRtF_Default);
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
        output_message(SMsg_CreatureLeaving, MESSAGE_DURATION_CRTR_MOOD);
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
    if ((cctrl->persuade.persuade_count > 0) && (dungeon->num_active_creatrs > 1))
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->dragtng_idx == 0) {
        return false;
    }
    const struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
    if (!thing_exists(dragtng))
    {
        ERRORLOG("The %s is dragging non-existing thing with ID %d",thing_model_name(creatng), cctrl->dragtng_idx);
        cctrl->dragtng_idx = 0;
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
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    long selected = THING_RANDOM(thing, room->slabs_count);
    unsigned long n = 0;
    long i = room->slabs_list;
    // Get the selected index
    while (i != 0)
    {
        struct PlayerInfo *player = get_player(room->owner);
        if (player->roomspace.is_active)
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
            WARNLOG("Number of slabs in %s (%lu) is smaller than count (%u)",room_code_name(room->kind), n, room->slabs_count);
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
        MapSubtlCoord start_stl = THING_RANDOM(thing, AROUND_TILES_COUNT);
        for (nround = 0; nround < AROUND_TILES_COUNT; nround++)
        {
            MapSubtlCoord x = start_stl % 3 + stl_x;
            MapSubtlCoord y = start_stl / 3 + stl_y;
            if ((roomst->storage_height < 0) || (get_floor_filled_subtiles_at(x, y) == roomst->storage_height))
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
            start_stl = (start_stl + 1) % AROUND_TILES_COUNT;
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

                if ((roomst->storage_height < 0) || (get_floor_filled_subtiles_at(astl_x, astl_y) == roomst->storage_height))
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
    create_effect(&creatng->mappos, imp_spangle_effects[get_player_color_idx(creatng->owner)], creatng->owner);
    thing_play_sample(creatng, 76, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    if ( !external_set_thing_state(creatng, CrSt_CreatureDoingNothing) )
      set_start_state(creatng);
    return 1;
}

short creature_pretend_chicken_move(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (flag_is_set(cctrl->stateblock_flags, CCSpl_ChickenRel))
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
    struct Room *room;
    struct Coord3d random_pos;

    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);

    if (flag_is_set(cctrl->stateblock_flags, CCSpl_ChickenRel))
    {
        return 1;
    }

    long offsetted_gameturn = game.play_gameturn + creatng->index;

    if ( (offsetted_gameturn % 16) == 0 )
    {
        room = get_room_thing_is_on(creatng);

        if (room_is_invalid(room) || !room_role_matches(room->kind,RoRoF_FoodStorage) || room->owner != creatng->owner )
        {
            room = find_random_room_of_role_for_thing(creatng, creatng->owner, RoRoF_FoodStorage, 0);
        }

        if ( !room_is_invalid(room) )
        {
            if ( find_random_valid_position_for_thing_in_room(creatng, room, &random_pos) )
            {
                setup_person_move_close_to_position(creatng,random_pos.x.stl.num,random_pos.y.stl.num, 0);
                internal_set_thing_state(creatng, CrSt_CreaturePretendChickenMove);
                return 1;
            }
        }
        else if ( get_random_position_in_dungeon_for_creature(creatng->owner, CrWaS_WithinDungeon, creatng, &random_pos) )
        {
            setup_person_move_close_to_position(creatng,random_pos.x.stl.num,random_pos.y.stl.num, 0);
            internal_set_thing_state(creatng, CrSt_CreaturePretendChickenMove);
        }
    }
    return 1;
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
    if (room_is_invalid(room) || !room_role_matches(room->kind,RoRoF_PowersStorage))
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
    return 1;
}

TbBool init_creature_state(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(17,"Starting for %s index %ld",thing_model_name(creatng),(long)creatng->index);
    long stl_x = creatng->mappos.x.stl.num;
    long stl_y = creatng->mappos.y.stl.num;
    // Set creature to default state, in case giving it job will fail
    set_start_state(creatng);
    // Check job which we can do after dropping at these coordinates
    if (is_neutral_thing(creatng))
    {
        if ((game.conf.rules[creatng->owner].game.classic_bugs_flags & ClscBug_PassiveNeutrals))
        {
            SYNCDBG(3,"Trying to assign initial job at (%ld,%ld) for neutral %s index %d owner %d",stl_x,stl_y,thing_model_name(creatng),creatng->index,creatng->owner);
            return false;
        }
        SYNCDBG(3,"Assigning initial job at (%ld,%ld) for neutral %s index %d owner %d",stl_x,stl_y,thing_model_name(creatng),creatng->index,creatng->owner);
    }
    CreatureJob new_job = get_job_for_subtile(creatng, stl_x, stl_y, JoKF_AssignCeatureInit);
    if (new_job == Job_NULL)
    {
        SYNCDBG(3,"No job found at (%ld,%ld) for %s index %d owner %d",stl_x,stl_y,thing_model_name(creatng),creatng->index,creatng->owner);
        return false;
    }
    // Check if specific conditions are met for this job to be assigned
    if (!creature_can_do_job_near_position(creatng, stl_x, stl_y, new_job, JobChk_None))
    {
        SYNCDBG(3,"Cannot assign %s at (%ld,%ld) to %s index %d owner %d; checked and got refusal",creature_job_code_name(new_job),stl_x,stl_y,thing_model_name(creatng),creatng->index,creatng->owner);
        return false;
    }
    // Now try sending the creature to do job it should do at this position
    if (!send_creature_to_job_near_position(creatng, stl_x, stl_y, new_job))
    {
        WARNDBG(3,"Cannot assign %s at (%ld,%ld) to %s index %d owner %d; could not send to job",creature_job_code_name(new_job),stl_x,stl_y,thing_model_name(creatng),creatng->index,creatng->owner);
        return false;
    }
    SYNCDBG(3,"Job %s at (%ld,%ld) assigned to %s index %d owner %d",creature_job_code_name(new_job),stl_x,stl_y,thing_model_name(creatng),creatng->index,creatng->owner);
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
    struct CreatureStateConfig* active_stati = &game.conf.crtr_conf.states[active_state];
    struct CreatureStateConfig* continue_stati = &game.conf.crtr_conf.states[continue_state];
    if ((active_stati->cleanup_state != 0) || ((continue_state != CrSt_Unused) && (continue_stati->cleanup_state != 0)))
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
    if (cctrl->cowers_from_slap_turns > 0) {
        cctrl->cowers_from_slap_turns--;
    }
    if (cctrl->cowers_from_slap_turns > 0) {
        return 0;
    }
    restore_backup_state(creatng, cctrl->active_state_bkp, cctrl->continue_state_bkp);
    return 1;
}

short creature_steal_gold(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
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
    long max_amount = crconf->gold_hold - creatng->creature.gold_carried;
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
    if ( !thing_exists(picktng) || ((picktng->state_flags & TF1_IsDragged1) != 0)
      || (get_chessboard_distance(&creatng->mappos, &picktng->mappos) >= 512))
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
      ((room->used_capacity == 0) && (dungeon->offmap_money_owned <= 0)))
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
    } else
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        if (cctrl->paydays_owed > 0)
        {
            cctrl->paydays_owed--;
        }
        if ((dungeon->total_money_owned >= salary) && (cctrl->paydays_advanced < 0)) //If the creature has missed payday, and there is money again, time to pay up.
        {
            cctrl->paydays_owed++;
            cctrl->paydays_advanced++;
        }
    }
    set_start_state(creatng);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    struct Thing* efftng = create_price_effect(&creatng->mappos, creatng->owner, received);
    if (!(game.conf.rules[creatng->owner].game.classic_bugs_flags & ClscBug_FullyHappyWithGold))
    {
        anger_apply_anger_to_creature_all_types(creatng, crconf->annoy_got_wage);
    }
    thing_play_sample(efftng, 32, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
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
    set_creature_size_stuff(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (game.conf.rules[creatng->owner].game.classic_bugs_flags & ClscBug_ResurrectRemoved)
    {
        // If the classic bug is enabled, fainted units are also added to resurrect creature.
        update_dead_creatures_list_for_owner(creatng);
    }
    creatng->active_state = CrSt_CreatureUnconscious;
    cctrl->creature_control_flags |= CCFlg_PreventDamage;
    cctrl->creature_control_flags |= CCFlg_NoCompControl;
    cctrl->conscious_back_turns = game.conf.rules[creatng->owner].creature.game_turns_unconscious;
}

void make_creature_conscious_without_changing_state(struct Thing *creatng)
{
    TRACE_THING(creatng);
    SYNCDBG(18,"Starting");
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->creature_control_flags &= ~CCFlg_PreventDamage;
    cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
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
        set_creature_instance(creatng, CrInst_ATTACK_ROOM_SLAB, 0, 0);
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
    MapSubtlCoord base_stl_x = creatng->mappos.x.stl.num;
    MapSubtlCoord base_stl_y = creatng->mappos.y.stl.num;
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing *doortng;
    if (cctrl->blocking_door_id > 0)
    {
        struct Room *room = room_get(cctrl->target_room_id);
        if (!room_is_invalid(room))
        {
            // if there's another route to the current target room, take it
            struct Coord3d roompos;
            roompos.x.val = subtile_coord_center(room->central_stl_x);
            roompos.y.val = subtile_coord_center(room->central_stl_y);
            roompos.z.val = get_floor_height_at(&roompos);
            if (creature_can_navigate_to(creatng, &roompos, NavRtF_Default))
            {
                cctrl->blocking_door_id = 0;
                internal_set_thing_state(creatng, creatng->continue_state);
                return 1;
            }
            else
            {
                // look for another Treasure Room
                unsigned char navtype;
                room = get_room_for_thing_salary(creatng, &navtype);
                if (!room_is_invalid(room))
                {
                    if (room->index != cctrl->target_room_id)
                    {
                        cctrl->target_room_id = room->index;
                        cctrl->collided_door_subtile = 0;
                        internal_set_thing_state(creatng, creatng->continue_state);
                        return CrCkRet_Continue;
                    }
                }
            }
        }
        doortng = thing_get(cctrl->blocking_door_id);
    }
    else
    {
        doortng = INVALID_THING;
    }
    if (!thing_is_deployed_door(doortng) || ((game.play_gameturn - doortng->creation_turn) <= 3) || !doortng->door.is_locked)
    {
        internal_set_thing_state(creatng, CrSt_CreatureWantsSalary);
        return 1;
    }
    anger_apply_anger_to_creature(creatng, crconf->annoy_queue, AngR_NotPaid, 1);
    EventIndex evidx = event_create_event_or_update_nearby_existing_event(creatng->mappos.x.val, creatng->mappos.y.val, EvKind_WorkRoomUnreachable, creatng->owner, RoK_TREASURE);
    if (evidx > 0)
    {
        output_room_message(creatng->owner, RoK_TREASURE, OMsg_RoomNoRoute);
    }
    if (is_creature_other_than_given_waiting_at_closed_door_on_subtile(base_stl_x, base_stl_y, creatng))
    {
        int i = 0;
        int n = THING_RANDOM(creatng, SMALL_AROUND_SLAB_LENGTH);
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
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    if (crconf->lair_size <= 0) {
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

struct Room* get_room_for_thing_salary(struct Thing* creatng, unsigned char *navtype)
{
    RoomKind job_rrole = get_room_role_for_job(Job_TAKE_SALARY);
    if (!player_has_room_of_role(creatng->owner, job_rrole))
    {
        return INVALID_ROOM;
    }

    struct Room* room = find_nearest_room_of_role_for_thing_with_used_capacity(creatng, creatng->owner, job_rrole, NavRtF_Default, 1);
    if (!room_is_invalid(room))
    {
        return room;
    }
    else
    {
        room = find_nearest_room_of_role_for_thing_with_used_capacity(creatng, creatng->owner, job_rrole, NavRtF_NoOwner, 1);
    }
    if (room_is_invalid(room))
    {
        struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
        long salary = calculate_correct_creature_pay(creatng);
        if (dungeon->offmap_money_owned >= salary)
        {

            room = find_nearest_room_of_role_for_thing(creatng, creatng->owner, job_rrole, NavRtF_Default);
            if (room_is_invalid(room))
            {
                // There seem to be a correct room, but we can't reach it
                output_room_message(creatng->owner, job_rrole, OMsg_RoomNoRoute);
            }
        }
    }
    else
    {
        *navtype = NavRtF_NoOwner;
    }
    return room;
}

short creature_wants_salary(struct Thing *creatng)
{
    SYNCDBG(8,"Starting for %s index %d owner %d", thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
    TRACE_THING(creatng);
    struct Coord3d pos;
    unsigned char navtype;
    struct Room* room = get_room_for_thing_salary(creatng,&navtype);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (room_is_invalid(room))
    {
        if (cctrl->paydays_owed > 0)
        {
            cctrl->paydays_owed--;
            if (cctrl->paydays_advanced > SCHAR_MIN)
            {
                cctrl->paydays_advanced--;
            }
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
            anger_apply_anger_to_creature(creatng, crconf->annoy_no_salary, AngR_NotPaid, 1);
        }
        SYNCDBG(5, "No player %d %s with used capacity found to pay %s", (int)creatng->owner, room_role_code_name(get_room_role_for_job(Job_TAKE_SALARY)), thing_model_name(creatng));
        set_start_state(creatng);
        return 1;
    }
    else
    {
        if (navtype == NavRtF_NoOwner)
        {
            if (find_random_valid_position_for_thing_in_room(creatng, room, &pos))
            {
                if (setup_person_move_to_coord(creatng, &pos, NavRtF_NoOwner))
                {
                    creatng->continue_state = CrSt_CreatureTakeSalary;
                    cctrl->target_room_id = room->index;
                }
            }
        }
        else
        {
            if (creature_setup_random_move_for_job_in_room(creatng, room, Job_TAKE_SALARY, NavRtF_Default))
            {
                creatng->continue_state = CrSt_CreatureTakeSalary;
                cctrl->target_room_id = room->index;
            }
        }
    }
    return 1;
}

long setup_head_for_empty_treasure_space(struct Thing *thing, struct Room *room)
{
    SlabCodedCoords start_slbnum = room->slabs_list;

    // Find a random slab to start out with
    long n = THING_RANDOM(thing, room->slabs_count);
    for (unsigned long k = n; k > 0; k--)
    {
        if (start_slbnum == 0)
        {
            break;
        }
        start_slbnum = get_next_slab_number_in_room(start_slbnum);
    }
    if (start_slbnum == 0) {
        ERRORLOG("Taking random slab (%ld/%u) in %s index %u failed - internal inconsistency.", n, room->slabs_count, room_code_name(room->kind), room->index);
        start_slbnum = room->slabs_list;
    }

    SlabCodedCoords slbnum = start_slbnum;
    MapSlabCoord slb_x = slb_num_decode_x(slbnum);
    MapSlabCoord slb_y = slb_num_decode_y(slbnum);
    struct Thing* gldtng = find_gold_hoarde_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));

    // If the random slab has enough space to drop all gold, go there to drop it
    long wealth_size_holds = game.conf.rules[room->owner].game.gold_per_hoard / get_wealth_size_types_count();
    GoldAmount max_hoard_size_in_room = wealth_size_holds * room->total_capacity / room->slabs_count;
    if ((max_hoard_size_in_room - gldtng->valuable.gold_stored) >= thing->creature.gold_carried)
    {
        if (setup_person_move_to_position(thing, slab_subtile_center(slb_x), slab_subtile_center(slb_y), NavRtF_Default))
        {
            return 1;
        }
    }

    // If not, find a slab with the lowest amount of gold
    GoldAmount min_gold_amount = max_hoard_size_in_room;
    SlabCodedCoords slbmin = start_slbnum;
    for (long i = room->slabs_count; i > 0; i--)
    {
        slb_x = slb_num_decode_x(slbnum);
        slb_y = slb_num_decode_y(slbnum);
        gldtng = find_gold_hoarde_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        GoldAmount gold_amount = gldtng->valuable.gold_stored;
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

    // Send imp to slab with lowest amount on it
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
    thing->rendering_flags |= TRF_Invisible;
    thing->state_flags |= TF1_InCtrldLimbo;
}

void remove_thing_from_creature_controlled_limbo(struct Thing *thing)
{
    thing->state_flags &= ~TF1_InCtrldLimbo;
    thing->rendering_flags &= ~TRF_Invisible;
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
    if (!thing_exists(doortng)) {
        SYNCDBG(8,"Door collided with %s not found",thing_model_name(creatng));
        return 0;
    }
    set_creature_door_combat(creatng, doortng);
    return 1;
}

static TbBool is_good_spot_to_stand_to_damage_wall(int plyr_idx, MapSubtlCoord x, MapSubtlCoord y)
{
    const int slab_x = subtile_slab(x);
    const int slab_y = subtile_slab(y);
    struct SlabMap* slb = get_slabmap_block(slab_x, slab_y);

    return (slabmap_owner(slb) == plyr_idx &&
            slab_is_safe_land(plyr_idx, slab_x, slab_y) &&
            !slab_is_liquid(slab_x, slab_y));
}

void instruct_creature_to_damage_wall(struct Thing *creatng, MapSubtlCoord wall_x, MapSubtlCoord wall_y)
{
    const MapSubtlDelta delta_x = wall_x - creatng->mappos.x.stl.num;
    const MapSubtlDelta delta_y = wall_y - creatng->mappos.y.stl.num;
    int start_idx = 0;

    if ( abs(delta_y) >= abs(delta_x) )
    {
        if ( delta_y <= 0 )
            start_idx = 2;
        else
            start_idx = 0;
    }
    else
    {
        if ( delta_x <= 0 )
            start_idx = 1;
        else
            start_idx = 3;
    }

    for (int i = 0; i < SMALL_AROUND_LENGTH; ++i)
    {
        const int around_idx = (start_idx + i) % SMALL_AROUND_LENGTH;
        const MapSubtlCoord stand_x = wall_x + (2 * small_around[around_idx].delta_x);
        const MapSubtlCoord stand_y = wall_y + (2 * small_around[around_idx].delta_y);
        if ( is_good_spot_to_stand_to_damage_wall(creatng->owner, stand_x, stand_y) )
        {
            struct Coord3d pos;
            pos.x.val = subtile_coord_center(stand_x);
            pos.y.val = subtile_coord_center(stand_y);
            pos.z.val = get_thing_height_at(creatng, &pos);
            if (creature_can_navigate_to_with_storage(creatng, &pos, 0)) {
                if ( setup_person_move_to_position(creatng, stand_x, stand_y, 0) )
                {
                    creatng->continue_state = CrSt_CreatureDamageWalls;
                    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
                    cctrl->damage_wall_coords = get_subtile_number(wall_x, wall_y);
                }
            }
        }
    }
}

CrCheckRet move_check_can_damage_wall(struct Thing *creatng)
{
    for (int i = 0; i < SMALL_AROUND_LENGTH; i++)
    {
        const MapSubtlCoord wall_x = creatng->mappos.x.stl.num + (small_around[i].delta_x * STL_PER_SLB);
        const MapSubtlCoord wall_y = creatng->mappos.y.stl.num + (small_around[i].delta_y * STL_PER_SLB);

        struct SlabMap* slb = get_slabmap_for_subtile(wall_x, wall_y);
        PlayerNumber slab_owner = slabmap_owner(slb);
        struct Map* mapblk = get_map_block_at(wall_x, wall_y);
        struct SlabConfigStats* slabst = get_slab_stats(slb);

        if ( (mapblk->flags & SlbAtFlg_Blocking) != 0
            && slab_owner == creatng->owner
            && slabst->category == SlbAtCtg_FortifiedWall )
        {
            instruct_creature_to_damage_wall(creatng, wall_x, wall_y);
            return 1;
        }
    }
    return 0;
}

CrAttackType creature_can_have_combat_with_creature_on_slab(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y, struct Thing ** enemytng, TbBool exclude_diggers)
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
                    if (!exclude_diggers || !flag_is_set(get_creature_model_flags(thing), CMF_IsSpecDigger))
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
    CrAttackType attack_type = creature_can_have_combat_with_creature_on_slab(creatng, slb_x, slb_y, &enemytng, true);
    if (attack_type > AttckT_Unset) {
        set_creature_in_combat_to_the_death(creatng, enemytng, attack_type);
        return CrCkRet_Continue;
    }
    return CrCkRet_Available;
}

CrCheckRet move_check_kill_diggers(struct Thing* creatng)
{
    MapSlabCoord slb_x = coord_slab(creatng->mappos.x.val);
    MapSlabCoord slb_y = coord_slab(creatng->mappos.y.val);
    struct Thing* enemytng;
    CrAttackType attack_type = creature_can_have_combat_with_creature_on_slab(creatng, slb_x, slb_y, &enemytng, false);
    if (thing_is_creature_digger(enemytng))
    {
        if (attack_type > AttckT_Unset)
        {
            set_creature_in_combat_to_the_death(creatng, enemytng, attack_type);
            return CrCkRet_Continue;
        }
    }
    return CrCkRet_Available;
}

CrCheckRet move_check_near_dungeon_heart(struct Thing *creatng)
{
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
    struct Room *room_thing_is_on;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);

    if ( cctrl->target_room_id == 0 )
    {
        return 0;
    }
    room_thing_is_on = get_room_thing_is_on(creatng);
    if ( room_is_invalid(room_thing_is_on) || room_thing_is_on->index != cctrl->target_room_id )
    {
        return 0;
    }
    internal_set_thing_state(creatng, creatng->continue_state);
    return 1;
}

CrCheckRet move_check_persuade(struct Thing *creatng)
{
    struct Thing *group_leader;
    TbBool creature_is_leader;
    struct Thing *i;
    struct Thing *i_leader;

    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->persuade.persuade_count)
    {
        group_leader = get_group_leader(creatng);
        if (group_leader == creatng)
        {
            creature_is_leader = true;
        }
        else
        {
            creature_is_leader = false;
            if (group_leader)
                disband_creatures_group(creatng);
        }

        MapSubtlCoord base_stl_x = creatng->mappos.x.stl.pos - creatng->mappos.x.stl.pos % 3;
        MapSubtlCoord base_stl_y = creatng->mappos.y.stl.pos - creatng->mappos.y.stl.pos % 3;

        for (MapSubtlDelta stl_offset_x = 0; stl_offset_x < STL_PER_SLB; stl_offset_x++)
        {
            for (MapSubtlDelta stl_offset_y = 0; stl_offset_y < STL_PER_SLB; stl_offset_y++)
            {
                struct Map* mapblk = get_map_block_at(base_stl_x + stl_offset_x, base_stl_y + stl_offset_y);
                for ( i = thing_get(get_mapwho_thing_index(mapblk));
                      !thing_is_invalid(i);
                      i = thing_get(i->next_on_mapblk) )
                {
                    if (i->owner != creatng->owner || !thing_is_creature(i) || i == creatng || creature_is_for_dungeon_diggers_list(i))
                        continue;
                    i_leader = get_group_leader(i);
                    if (i_leader)
                    {
                        if (i_leader == creatng)
                            continue;
                        remove_creature_from_group(i);
                    }
                    if ((creature_is_leader && add_creature_to_group(i, creatng)) || add_creature_to_group_as_leader(creatng, i))
                    {
                        cctrl->persuade.persuade_count--;
                        if (cctrl->persuade.persuade_count == 0)
                        {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

CrCheckRet move_check_wait_at_door_for_wage(struct Thing *creatng)
{
  struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
  struct Thing *doortng;
  struct Room *room;
  if (cctrl->collided_door_subtile != 0)
  {
      room = room_get(cctrl->target_room_id);
      if (!room_is_invalid(room))
      {
          // if there's another route to the current target room, take it
          struct Coord3d roompos;
          roompos.x.val = subtile_coord_center(room->central_stl_x);
          roompos.y.val = subtile_coord_center(room->central_stl_y);
          roompos.z.val = get_floor_height_at(&roompos);
          if (creature_can_navigate_to(creatng, &roompos, NavRtF_Default))
          {
              cctrl->collided_door_subtile = 0;
              internal_set_thing_state(creatng, creatng->continue_state);
              return CrCkRet_Continue;
          }
          else
          {
              // look for another Treasure Room
              unsigned char navtype;
              room = get_room_for_thing_salary(creatng, &navtype);
              if (!room_is_invalid(room))
              {
                  if (room->index != cctrl->target_room_id)
                  {
                      cctrl->target_room_id = room->index;
                      cctrl->collided_door_subtile = 0;
                      internal_set_thing_state(creatng, creatng->continue_state);
                      return CrCkRet_Continue;
                  }
              }
          }
      }
      doortng = get_door_for_position(stl_num_decode_x(cctrl->collided_door_subtile), stl_num_decode_y(cctrl->collided_door_subtile));
      if (thing_exists(doortng))
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
    struct Map* mapblk;
    struct Column* col;

    // NB: the code assumes PLAYERS_COUNT = DUNGEONS_COUNT
    for (int i = 0; i < PLAYERS_COUNT; ++i)
    {
        struct PlayerInfo* player = get_player(i);
        struct Dungeon* dgn = get_dungeon(i);
        if (!player_exists(player) || (player->is_active != 1) || players_are_mutual_allies(i,creatng->owner))
            continue;

        if (!dgn->dnheart_idx)
            continue;

        // Player dungeon already broken into
        if (flag_is_set(cctrl->party.player_broken_into_flags, to_flag(i)))
            continue;

        if (!subtile_revealed(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, i))
            continue;

        //If there is a ceiling, the tunneler is invisible too. For tunnels.
        mapblk = get_map_block_at(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
        col = get_map_column(mapblk);
        if ((col->bitfields & CLF_CEILING_MASK) != 0)
            continue;

        if (!creature_can_navigate_to(creatng, &game.things.lookup[dgn->dnheart_idx]->mappos, NavRtF_Default))
            continue;

        set_flag(cctrl->party.player_broken_into_flags, to_flag(i));
        ++dgn->times_broken_into;
        event_create_event_or_update_nearby_existing_event(creatng->mappos.x.val, creatng->mappos.y.val, EvKind_Breach, i, 0);
        if (is_my_player_number(i))
        {
            output_message(SMsg_WallsBreach, 0);
        }
    }
    return 0;
}

TbBool go_to_random_area_near_xy(struct Thing *creatng, MapSubtlCoord bstl_x, MapSubtlCoord bstl_y)
{
    for (int i = 0; i < 5; i++)
    {
        MapSubtlCoord stl_x = bstl_x + THING_RANDOM(creatng, 5) - 2;
        MapSubtlCoord stl_y = bstl_y + THING_RANDOM(creatng, 5) - 2;
        if (setup_person_move_to_position(creatng, stl_x, stl_y, 0)) {
            return true;
        }
    }
    return false;
}

short patrol_here(struct Thing *creatng)
{
    MapSubtlCoord bstl_y = creatng->mappos.y.stl.num;
    MapSubtlCoord bstl_x = creatng->mappos.x.stl.num;
    if (!go_to_random_area_near_xy(creatng, bstl_x, bstl_y))
    {
        set_start_state(creatng);
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->patrol.countdown = 10;
    cctrl->patrol.pos = cctrl->moveto_pos;
    creatng->continue_state = CrSt_Patrolling;
    return 1;
}

short patrolling(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->patrol.countdown <= 0) {
        set_start_state(creatng);
        return 0;
    }
    cctrl->patrol.countdown--;
    // Try random positions near the patrolling point
    MapSubtlCoord stl_x = cctrl->patrol.pos.x.stl.num;
    MapSubtlCoord stl_y = cctrl->patrol.pos.y.stl.num;
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
    if (!room_still_valid_as_type_for_thing(room, RoRoF_LairStorage, creatng))
    {
        WARNLOG("Room %s index %d is not valid %s for %s owned by player %d to work in",
            room_code_name(room->kind),(int)room->index,room_role_code_name(RoRoF_LairStorage),
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
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    anger_apply_anger_to_creature_all_types(creatng, crconf->annoy_sulking);
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
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    if ((crconf->lair_size <= 0) || creature_affected_by_slap(creatng) || player_uses_power_obey(creatng->owner)) {
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
            play_creature_sound(creatng, CrSnd_Sad, 2, 0);
        }
        if (cctrl->turns_at_job - 250 >= 0) {
          cctrl->turns_at_job = 0;
        } else
        if (cctrl->instance_id == CrInst_NULL) {
            set_creature_instance(creatng, CrInst_MOAN, 0, 0);
        }
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    anger_apply_anger_to_creature_all_types(creatng, crconf->annoy_sulking);
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
TbBool room_initially_valid_as_type_for_thing(const struct Room *room, RoomRole rrole, const struct Thing *thing)
{
    if (!room_exists(room)) {
        return false;
    }
    if (!room_role_matches(room->kind,rrole)) {
        return false;
    }
    return ((room->owner == thing->owner) || enemies_may_work_in_room(room->kind));
}

/**
 * Returns if the room is a valid place for a thing for thing which is already working in that room.
 * Used to check if creatures are working in correct rooms.
 * @param room The work room to be checked.
 * @param rrole Room role required for work.
 * @param thing The thing which is working in the room.
 * @return True if the room can still be used, false otherwise.
 */
TbBool room_still_valid_as_type_for_thing(const struct Room *room, RoomRole rrole, const struct Thing *thing)
{
    if (!room_exists(room)) {
        return false;
    }
    if (!room_role_matches(room->kind,rrole)) {
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
    RoomRole rrole = get_room_role_for_job(jobpref);
    if (!room_exists(room))
    {
        SYNCLOG("%s: The %s(%d) owned by player %d can no longer work in %s because former work room doesn't exist",
            func_name,thing_model_name(thing),thing->index,(int)thing->owner,room_role_code_name(rrole));
        // Note that if given room doesn't exist, it do not mean this
        return true;
    }
    if (!room_still_valid_as_type_for_thing(room, rrole, thing))
    {
        WARNLOG("%s: Room %s index %d is not valid %s for %s owned by player %d to work in",
            func_name,room_code_name(room->kind),(int)room->index,room_role_code_name(rrole),
            thing_model_name(thing),(int)thing->owner);
        return true;
    }
    if (!creature_is_working_in_room(thing, room))
    {
        // This is not an error, because room index is often changed, ie. when room is expanded or its slab sold
        SYNCDBG(2,"%s: Room %s index %d is not the %s which %s owned by player %d selected to work in",
            func_name,room_code_name(room->kind),(int)room->index,room_role_code_name(rrole),
            thing_model_name(thing),(int)thing->owner);
        return true;
    }
    return false;
}

void create_effect_around_thing(struct Thing *thing, long eff_kind)
{
    int tng_radius = (thing->clipbox_size_xy >> 1);
    MapCoord coord_x_beg = (MapCoord)thing->mappos.x.val - tng_radius;
    if (coord_x_beg < 0)
        coord_x_beg = 0;
    MapCoord coord_x_end = (MapCoord)thing->mappos.x.val + tng_radius;
    if (coord_x_end >= subtile_coord(game.map_subtiles_x+1, 0) - 1)
        coord_x_end = subtile_coord(game.map_subtiles_x+1, 0) - 1;
    MapCoord coord_y_beg = (MapCoord)thing->mappos.y.val - tng_radius;
    if (coord_y_beg < 0)
        coord_y_beg = 0;
    MapCoord coord_y_end = (MapCoord)thing->mappos.y.val + tng_radius;
    if (coord_y_end >= subtile_coord(game.map_subtiles_y+1, 0) - 1)
        coord_y_end = subtile_coord(game.map_subtiles_y+1, 0) - 1;
    MapCoord coord_z_beg = (MapCoord)thing->mappos.z.val;
    if (coord_z_beg < 0)
        coord_z_beg = 0;
    MapCoord coord_z_end = (MapCoord)thing->mappos.z.val + thing->clipbox_size_z;
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
                if (eff_kind != 0)
                {
                    create_used_effect_or_element(&pos, eff_kind, thing->owner, thing->index);
                }
                pos.z.val += COORD_PER_STL;
            }
            pos.y.val += COORD_PER_STL;
        }
        pos.x.val += COORD_PER_STL;
    }
}

void remove_health_from_thing_and_display_health(struct Thing *thing, HitPoints delta)
{
    if ((thing->health >= 0) && (delta > 0))
    {
        thing->creature.health_bar_turns = 8;
        thing->health -= delta;
    }
}

TbBool process_creature_hunger(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    if ((crconf->hunger_rate == 0) || creature_under_spell_effect(thing, CSAfF_Freeze) || is_neutral_thing(thing))
        return false;
    SYNCDBG(19,"Hungering %s index %d",thing_model_name(thing), (int)thing->index);
    cctrl->hunger_level++;
    if (!hunger_is_creature_hungry(thing))
        return false;
    // HungerHealthLoss is disabled if set to 0 on rules.cfg.
    if (game.conf.rules[thing->owner].health.turns_per_hunger_health_loss > 0)
    {
        // Make sure every creature loses health on different turn.
        if (((game.play_gameturn + thing->index) % game.conf.rules[thing->owner].health.turns_per_hunger_health_loss) == 0) {
            SYNCDBG(9,"The %s index %d lost %d health due to hunger",thing_model_name(thing), (int)thing->index, (int)game.conf.rules[thing->owner].health.hunger_health_loss);
            remove_health_from_thing_and_display_health(thing, game.conf.rules[thing->owner].health.hunger_health_loss);
            return true;
        }
    }
    return false;
}

/**
 * Check if thing is an enemy trap and can be destroyed by the creature.
 * @param fightng The creature that might destroy the trap.
 * @param enmtng The trap to be checked if it could be destroyed.
 */
TbBool trap_is_valid_combat_target_for_creature(const struct Thing* fightng, const struct Thing* enmtng)
{
    if(!players_are_enemies(fightng->owner,enmtng->owner))
        return false;
    if (thing_is_destructible_trap(enmtng) < 0)
        return false;
    if (thing_is_destructible_trap(enmtng) == 1)
        return true;
    return creature_has_disarming_weapon(fightng);
}

TbBool creature_is_hostile_towards(const struct Thing *fightng, const struct Thing *enmtng)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(fightng);
    for (int i = 0; i < CREATURE_TYPES_MAX; i++)
    {
        if ((crconf->hostile_towards[i] == enmtng->model) || (crconf->hostile_towards[i] == CREATURE_ANY))
        {
            return true;
        }
    }
    return false;
}

/* Determines if two creatures will fight each other even when allied.
 * @param fightng The first creature to check.
 * @param enmtng The second creature to check.
 * @returns 'true' if either creature is hostile towards the other.
 * @returns 'false' if both creatures are not hostile towards each other, or if either met any of the following conditions:
 * - Either creature is influenced by 'Call to Arms'.
 * - Either creature is member of a group/party. */
TbBool creature_is_hostile_to_creature(const struct Thing *fightng, const struct Thing *enmtng)
{
    // Creatures cannot be hostile towards allies if influenced by CTA.
    if (creature_affected_by_call_to_arms(fightng) || creature_affected_by_call_to_arms(enmtng))
    {
        return false;
    }
    // Creatures cannot be hostile towards allies if they are part of a group.
    if (creature_is_group_member(fightng) || creature_is_group_member(enmtng))
    {
        return false;
    }
    // Lastly, check if neither creature has hostility set towards the other.
    if (!creature_is_hostile_towards(fightng, enmtng) && !creature_is_hostile_towards(enmtng, fightng))
    {
        return false;
    }
    return true;
}

/**
 * Checks if creatures can attack each other.
 * Note that this function does not include full check from players_are_enemies(), so both should be used when applicable.
 * @param fightng
 * @param enmtng
 * @return
 * @see players_are_enemies()
 */
TbBool creature_will_attack_creature(const struct Thing *fightng, const struct Thing *enmtng)
{
    if (creature_is_leaving_and_cannot_be_stopped(fightng) || creature_is_leaving_and_cannot_be_stopped(enmtng))
    {
        return false;
    }
    if (creature_is_being_unconscious(fightng) || creature_is_being_unconscious(enmtng))
    {
        return false;
    }
    if (thing_is_picked_up(fightng) || thing_is_picked_up(enmtng))
    {
        return false;
    }
    struct CreatureControl* fighctrl = creature_control_get_from_thing(fightng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    if (players_creatures_tolerate_each_other(fightng->owner, enmtng->owner) && !creature_is_hostile_to_creature(fightng, enmtng))
    {
        if ((!creature_under_spell_effect(fightng, CSAfF_MadKilling))
        && (!creature_under_spell_effect(enmtng, CSAfF_MadKilling)))
        {
            if (fighctrl->combat_flags == 0)
            {
                return false;
            }
            struct Thing* tmptng = thing_get(fighctrl->combat.battle_enemy_idx);
            TRACE_THING(tmptng);
            if (tmptng->index != enmtng->index)
            {
                return false;
            }
        }
    }
    // No self fight.
    if (enmtng->index == fightng->index)
    {
        return false;
    }
    // No fight when creature in custody.
    if (creature_is_kept_in_custody_by_player(fightng, enmtng->owner)
    || creature_is_kept_in_custody_by_player(enmtng, fightng->owner))
    {
        return false;
    }
    // No fight while dropping.
    if (creature_is_being_dropped(fightng) || creature_is_being_dropped(enmtng))
    {
        return false;
    }
    // Final check - if creature is in control and can see the enemy - fight.
    if ((creature_control_exists(enmctrl)) && ((enmctrl->creature_control_flags & CCFlg_NoCompControl) == 0))
    {
        if (!creature_is_invisible(enmtng) || creature_can_see_invisible(fightng))
        {
            return true;
        }
    }
    return false;
}

/**
 * Checks if creatures can attack each other.
 * This variant loosens conditions if first creature is fighting till death, besides that it is identical to creature_will_attack_creature().
 * Note that this function does not include full check from players_are_enemies(), so both should be used when applicable.
 * @param fightng
 * @param enmtng
 * @return
 * @see players_are_enemies()
 * @see creature_will_attack_creature()
 */
TbBool creature_will_attack_creature_incl_til_death(const struct Thing *fightng, const struct Thing *enmtng)
{
    if (creature_is_being_unconscious(fightng) || creature_is_being_unconscious(enmtng))
    {
        return false;
    }
    if (thing_is_picked_up(fightng) || thing_is_picked_up(enmtng))
    {
        return false;
    }
    if (creature_is_leaving_and_cannot_be_stopped(fightng) || creature_is_leaving_and_cannot_be_stopped(enmtng))
    {
        return false;
    }
    struct CreatureControl* fighctrl = creature_control_get_from_thing(fightng);
    struct CreatureControl* enmctrl = creature_control_get_from_thing(enmtng);
    if (players_creatures_tolerate_each_other(fightng->owner, enmtng->owner) && !creature_is_hostile_to_creature(fightng, enmtng))
    {
        if ((fighctrl->fight_til_death == 0) // This differs in creature_will_attack_creature()
        && (!creature_under_spell_effect(fightng, CSAfF_MadKilling))
        && (!creature_under_spell_effect(enmtng, CSAfF_MadKilling)))
        {
            struct Thing* tmptng = thing_get(fighctrl->combat.battle_enemy_idx);
            TRACE_THING(tmptng);
            if ((fighctrl->combat_flags == 0) || (tmptng->index != enmtng->index))
            {
                return false;
            }
        }
    }
    // No self fight.
    if (enmtng->index == fightng->index)
    {
        return false;
    }
    // No fight when creature in custody.
    if (creature_is_kept_in_custody_by_player(fightng, enmtng->owner)
    || creature_is_kept_in_custody_by_player(enmtng, fightng->owner))
    {
        return false;
    }
    // No fight while dropping.
    if (creature_is_being_dropped(fightng) || creature_is_being_dropped(enmtng))
    {
        return false;
    }
    // Final check - if creature is in control and can see the enemy - fight.
    if ((creature_control_exists(enmctrl)) && ((enmctrl->creature_control_flags & CCFlg_NoCompControl) == 0))
    {
        if (!creature_is_invisible(enmtng) || creature_can_see_invisible(fightng))
        {
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
    if (cctrl->seek_enemy.enemy_idx != 0)
    {
        enemytng = thing_get(cctrl->seek_enemy.enemy_idx);
        TRACE_THING(enemytng);
        if (((enemytng->alloc_flags & TAlF_Exists) == 0) || (cctrl->seek_enemy.enemy_creation_turn != enemytng->creation_turn))
        {
          enemytng = INVALID_THING;
          cctrl->seek_enemy.enemy_creation_turn = 0;
          cctrl->seek_enemy.enemy_idx = 0;
        }
    } else
    {
        enemytng = INVALID_THING;
    }
    if (game.play_gameturn - cctrl->seek_enemy.turn_looked_for_enemy > 64)
    {
        cctrl->seek_enemy.turn_looked_for_enemy = game.play_gameturn;
        enemytng = find_nearest_enemy_creature(thing);
    }
    if (thing_is_invalid(enemytng))
    {
        cctrl->seek_enemy.enemy_idx = 0;
        return NULL;
    }
    cctrl->seek_enemy.enemy_idx = enemytng->index;
    cctrl->seek_enemy.enemy_creation_turn = enemytng->creation_turn;
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
          long irnd = THING_RANDOM(thing, wandr->points_count);
          MapSubtlCoord stl_x = wandr->points[irnd].stl_x;
          MapSubtlCoord stl_y = wandr->points[irnd].stl_y;
          MapSubtlCoord dist = chessboard_distance(stl_x, stl_y, prevpos->x.stl.num, prevpos->y.stl.num);
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
    if (thing_is_creature(thing))
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
        return subtile_coord(crconf->hearing,0) >= dist;
    }
    else if (thing_is_mature_food(thing))
    {
        return (dist <= 2560); // 10 subtiles
    }
    else
    {
        return false;
    }
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
        return INT32_MAX;

    if (path.waypoints_num <= 0)
        return distance;

    struct Coord3d pos1 = creatng->mappos;
    for (int i = 0; i < path.waypoints_num; ++i)
    {
        struct Coord3d pos2;
        pos2.x.val = path.waypoints[i].x;
        pos2.y.val = path.waypoints[i].y;
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
    struct CreatureSound* crsound;
    if (!thing_is_invalid(enemytng))
    {
        MapCoordDelta dist = get_chessboard_distance(&enemytng->mappos, &creatng->mappos);
        if (creature_can_hear_within_distance(creatng, dist))
        {
            if (cctrl->instance_id == CrInst_NULL)
            {
                if ((dist < 2304) && (game.play_gameturn-cctrl->countdown < 20))
                {
                    crsound = get_creature_sound(creatng, CrSnd_Fight);
                    thing_play_sample(creatng, crsound->index + SOUND_RANDOM(crsound->count), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
                    set_creature_instance(creatng, CrInst_CELEBRATE_SHORT, 0, 0);
                    return 1;
                }
                if (THING_RANDOM(creatng, 4) != 0)
                {
                    if (setup_person_move_close_to_position(creatng, enemytng->mappos.x.stl.num, enemytng->mappos.y.stl.num, NavRtF_Default))
                    {
                        creatng->continue_state = CrSt_SeekTheEnemy;
                        cctrl->countdown = game.play_gameturn;
                        return 1;
                    }
                }
                if (creature_choose_random_destination_on_valid_adjacent_slab(creatng))
                {
                    creatng->continue_state = CrSt_SeekTheEnemy;
                    cctrl->countdown = game.play_gameturn;
                }
            }
            return 1;
        }
        if (THING_RANDOM(creatng, 64) == 0)
        {
            if (setup_person_move_close_to_position(creatng, enemytng->mappos.x.stl.num, enemytng->mappos.y.stl.num, NavRtF_Default))
            {
              creatng->continue_state = CrSt_SeekTheEnemy;
            }
        }
    }
    // No enemy found - do some random movement
    struct Coord3d pos;
    if (THING_RANDOM(creatng, 12) != 0)
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
    cctrl->creature_control_flags &= ~CCFlg_NoCompControl;
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
    if (creature_under_spell_effect(thing, CSAfF_Speed))
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
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
    long i = crconf->to_level[cctrl->exp_level] << 8;
    if (cctrl->exp_points < i)
    {
        return false;
    }
    cctrl->exp_points -= i;
    struct Dungeon *dungeon = get_dungeon(thing->owner);
    if (cctrl->exp_level < dungeon->creature_max_level[thing->model])
    {
        if ((cctrl->exp_level < CREATURE_MAX_LEVEL - 1) || (crconf->grow_up != 0))
        {
            cctrl->exp_level_up = true;
        }
    }
    return true;
}
/******************************************************************************/
TbBool internal_set_thing_state(struct Thing *thing, CrtrStateId nState)
{
    thing->active_state = nState;
    thing->continue_state = CrSt_Unused;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->stopped_for_hand_turns = 0;
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
    thing->continue_state = CrSt_Unused;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("%s: The %s index %d has invalid control",func_name,thing_model_name(thing),(int)thing->index);
        return false;
    }
    cctrl->target_room_id = 0;
    cctrl->stopped_for_hand_turns = 0;
    if ((cctrl->creature_control_flags & CCFlg_IsInRoomList) != 0)
    {
        WARNLOG("%s: The %s stays in room list even after cleanup",func_name,thing_model_name(thing));
        remove_creature_from_work_room(thing);
    }
    return true;
}

TbBool cleanup_current_thing_state(struct Thing *creatng)
{
    struct CreatureStateConfig* stati = get_creature_state_with_task_completion(creatng);
    if (stati->cleanup_state > 0)
    {
        cleanup_func_list[stati->cleanup_state](creatng);
    }
    else if (stati->cleanup_state < 0)
    {
        luafunc_crstate_func(stati->cleanup_state, creatng);
    }
    else
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
    if (creature_is_group_member(creatng))
    {
        remove_creature_from_group(creatng);
    }
    remove_events_thing_is_attached_to(creatng);
    // Use the correct function to clear them properly. Terminating the spells also removes the attached effects.
    if (creature_under_spell_effect(creatng, CSAfF_Armour))
    {
        clean_spell_effect(creatng, CSAfF_Armour);
    }
    if (creature_under_spell_effect(creatng, CSAfF_Disease))
    {
        clean_spell_effect(creatng, CSAfF_Disease);
    }
    delete_familiars_attached_to_creature(creatng);
    state_cleanup_dragging_body(creatng);
    state_cleanup_dragging_object(creatng);
    if (flag_is_set((creature_control_get_from_thing(creatng))->creature_state_flags, TF2_SummonedCreature))
    {
        remove_creature_from_summon_list(get_dungeon(creatng->owner), creatng->index);
    }
    return true;
}

TbBool can_change_from_state_to(const struct Thing *thing, CrtrStateId curr_state, CrtrStateId next_state)
{
    struct CreatureStateConfig* curr_stati = get_thing_state_info_num(curr_state);
    if (curr_stati->state_type == CrStTyp_Move)
      curr_stati = get_thing_state_info_num(thing->continue_state);
    struct CreatureStateConfig* next_stati = get_thing_state_info_num(next_state);
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        if ( (next_stati->state_type != CrStTyp_Idle) )
        {
            return false;
        }
    }
    if ((curr_state == CrSt_Timebomb) || (curr_state == CrSt_CreatureObjectSnipe) || (curr_state == CrSt_GoodArrivedAtSabotageRoom)) //todo this should come from configs
    {
        if (next_stati->state_type == CrStTyp_FightDoor)
        {
            return true;
        }
    }
    if ((curr_stati->transition) && (!next_stati->override_transition))
        return false;
    if ((curr_stati->captive) && (!next_stati->override_captive))
        return false;
    switch (curr_stati->state_type)
    {
    case CrStTyp_OwnNeeds:
        return (next_stati->override_own_needs);
    case CrStTyp_Sleep:
        return (next_stati->override_sleep);
    case CrStTyp_Feed:
        return (next_stati->override_feed);
    case CrStTyp_FightCrtr:
        return (next_stati->override_fight_crtr);
    case CrStTyp_GetsSalary:
        return (next_stati->override_gets_salary);
    case CrStTyp_Escape:
        return (next_stati->override_escape);
    case CrStTyp_Unconscious:
        return (next_stati->override_unconscious);
    case CrStTyp_AngerJob:
        return (next_stati->override_anger_job);
    case CrStTyp_FightDoor:
        return (next_stati->override_fight_door);
    case CrStTyp_FightObj:
        return (next_stati->override_fight_object);
    case CrStTyp_Called2Arms:
        return (next_stati->override_call2arms);
    case CrStTyp_Follow:
        return (next_stati->override_follow);
    default:
        return true;
    }
    return false;
}

short set_start_state_f(struct Thing *thing,const char *func_name)
{
    long i;
    struct CreatureModelConfig* crconf;
    SYNCDBG(8,"%s: Starting for %s index %d, owner %d, last state %s, stacked %s",func_name,thing_model_name(thing),
        (int)thing->index,(int)thing->owner,creature_state_code_name(thing->active_state),creature_state_code_name(thing->continue_state));
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_ManualControl);
        return thing->active_state;
    }
    if (creature_under_spell_effect(thing, CSAfF_Chicken))
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_CreaturePretendChickenSetupMove);
        return thing->active_state;
    }
    if (creature_under_spell_effect(thing, CSAfF_Timebomb))
    {
        cleanup_current_thing_state(thing);
        initialise_thing_state(thing, CrSt_Timebomb);
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
        crconf = creature_stats_get_from_thing(thing);
        i = crconf->good_start_state;
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
        // TODO: Correctly deal with possession of creatures not owned by the player
        if (!creature_is_for_dungeon_diggers_list(thing))
        {
            cleanup_current_thing_state(thing);
            initialise_thing_state(thing, CrSt_LeavesBecauseOwnerLost);
            return thing->active_state;
        }
    }
    crconf = creature_stats_get_from_thing(thing);
    i = crconf->evil_start_state;
    cleanup_current_thing_state(thing);
    initialise_thing_state(thing, i);
    return thing->active_state;
}

TbBool external_set_thing_state_f(struct Thing *thing, CrtrStateId state, const char *func_name)
{
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((creature_affected_by_slap(thing) || creature_is_called_to_arms(thing)) && (cctrl->dropped_turn != game.play_gameturn))
        return false;
    return can_change_from_state_to(thing, thing->active_state, state);
}

long creature_free_for_toking(struct Thing* creatng)
{
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
    return 1;
}

/**
 * If creature health is very low, go back to lair immediately for healing.
 * @param thing
 * @param crconf
 */
long process_creature_needs_to_heal_critical(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (get_creature_health_permil(creatng) >= game.conf.rules[creatng->owner].creature.critical_health_permil) {
        return 0;
    }
    if (game.play_gameturn >= cctrl->healing_sleep_check_turn)
    {
        if (!creature_can_do_healing_sleep(creatng))
        {
            // Creature needs healing but cannot heal in lair - try toking
            if (!creature_free_for_toking(creatng))
            {
                return 0;
            }
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
            if (crconf->toking_recovery <= 0)
            {
                return 0;
            }
            if (external_set_thing_state(creatng, CrSt_CreatureGoingToSafetyForToking)) {
                creatng->continue_state = CrSt_ImpDoingNothing;
                return 1;
            }
        }
        if (creature_is_doing_lair_activity(creatng)) {
            return 1;
        }
        if (!creature_free_for_sleep(creatng, CrSt_CreatureGoingHomeToSleep)) {
            return 0;
        }
        if ((game.play_gameturn - cctrl->healing_sleep_check_turn) > 128)
        {
            if ((cctrl->lair_room_id != 0) || !room_is_invalid(get_best_new_lair_for_creature(creatng)))
            {
                SYNCDBG(4, "Healing critical for %s", thing_model_name(creatng));
                if (external_set_thing_state(creatng, CrSt_CreatureGoingHomeToSleep))
                {
                    return 1;
                }
            }
            else
            {
                struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
                anger_apply_anger_to_creature(creatng, crconf->annoy_no_lair, AngR_NoLair, 1);
            }
            cctrl->healing_sleep_check_turn = game.play_gameturn;
        }
    }
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

long process_creature_needs_a_wage(struct Thing *creatng, const struct CreatureModelConfig *crconf)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((crconf->pay == 0) || (cctrl->paydays_owed == 0)) {
      return 0;
    }
    if (creature_is_taking_salary_activity(creatng)) {
        return 1;
    }
    if (!can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureWantsSalary)) {
        return 0;
    }
    cctrl->collided_door_subtile = 0;
    struct Room* room = find_nearest_room_of_role_for_thing_with_used_capacity(creatng, creatng->owner, RoRoF_GoldStorage, NavRtF_Default, 1);
    if (!room_is_invalid(room))
    {
        if (external_set_thing_state(creatng, CrSt_CreatureWantsSalary))
        {
            anger_apply_anger_to_creature(creatng, crconf->annoy_got_wage, AngR_NotPaid, 1);
            return 1;
        }
        return 0;
    }
    room = find_any_navigable_room_for_thing_closer_than(creatng, creatng->owner, RoRoF_GoldStorage, NavRtF_Default, game.map_subtiles_x / 2 + game.map_subtiles_y / 2);
    if (room_is_invalid(room))
    {
        //if we can't find an unlocked room, try a locked room, to wait in front of the door
        room = find_nearest_room_of_role_for_thing_with_used_capacity(creatng, creatng->owner, RoRoF_GoldStorage, NavRtF_NoOwner, 1);
    }
    if (!room_is_invalid(room))
    {
        cleanup_current_thing_state(creatng);
        if (creature_setup_head_for_treasure_room_door(creatng, room))
        {
            return 1;
        }
        ERRORLOG("State lost, could not get to treasure room door after cleaning up old state");
        set_start_state(creatng);
        return 0;
    }
    struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
    room = find_nearest_room_of_role_for_thing(creatng, creatng->owner, RoRoF_GoldStorage, NavRtF_NoOwner);
    if ((dungeon->total_money_owned >= calculate_correct_creature_pay(creatng)) && !room_is_invalid(room))
    {
        cleanup_current_thing_state(creatng);
        if (creature_setup_head_for_treasure_room_door(creatng, room))
        {
            return 1;
        }
        ERRORLOG("State lost, could not get to treasure room door after cleaning up old state");
        set_start_state(creatng);
        return 0;
    }
    cctrl->paydays_owed--;
    if (cctrl->paydays_advanced > SCHAR_MIN)
    {
        cctrl->paydays_advanced--;
    }
    anger_apply_anger_to_creature(creatng, crconf->annoy_no_salary, AngR_NotPaid, 1);
    return 0;
}

char creature_free_for_lunchtime(struct Thing *creatng)
{
    return !creature_affected_by_slap(creatng)
    && !creature_is_called_to_arms(creatng)
    && !creature_under_spell_effect(creatng, CSAfF_Chicken)
    && can_change_from_state_to(creatng, creatng->active_state, CrSt_CreatureToGarden);
}

long process_creature_needs_to_eat(struct Thing *creatng, const struct CreatureModelConfig *crconf)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    RoomKind rkind;
    if ((crconf->hunger_rate == 0) || (cctrl->hunger_level <= (long)crconf->hunger_rate)) {
        return 0;
    }
    if (creature_is_doing_garden_activity(creatng)) {
        return 1;
    }
    if (!creature_free_for_lunchtime(creatng)) {
      return 0;
    }
    if (crconf->hunger_fill <= cctrl->hunger_loss)
    {
        cctrl->garden_eat_check_turn = game.play_gameturn;
        cctrl->hunger_loss -= crconf->hunger_fill;
        return 0;
    }

    if (!player_has_room_of_role(creatng->owner, RoRoF_FoodStorage))
    {
        rkind = find_first_roomkind_with_role(RoRoF_FoodStorage);
        output_room_message(creatng->owner, rkind, OMsg_RoomNeeded);
        anger_apply_anger_to_creature(creatng, crconf->annoy_no_hatchery, AngR_Hungry, 1);
        return 0;
    }
    if (game.play_gameturn - cctrl->garden_eat_check_turn <= 128) {
        anger_apply_anger_to_creature(creatng, crconf->annoy_no_hatchery, AngR_Hungry, 1);
        return 0;
    }
    struct Room* nroom = find_nearest_room_of_role_for_thing_with_used_capacity(creatng, creatng->owner, RoRoF_FoodStorage, NavRtF_Default, 1);
    if (room_is_invalid(nroom))
    {
        cctrl->garden_eat_check_turn = game.play_gameturn;
        // No food in nearest room, try to find another room
        nroom = find_random_room_of_role_for_thing(creatng, creatng->owner, RoRoF_FoodStorage, 0);
        if (room_is_invalid(nroom))
        {
            // There seem to be a correct room, but we can't reach it
            rkind = find_first_roomkind_with_role(RoRoF_FoodStorage);
            output_room_message(creatng->owner, rkind, OMsg_RoomNoRoute);
        } else
        {
            // The room is reachable, so it probably has just no food
            output_room_message(creatng->owner, nroom->kind, OMsg_RoomTooSmall);
        }
    }
    if (room_is_invalid(nroom)) {
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
        anger_apply_anger_to_creature(creatng, crconf->annoy_no_hatchery, AngR_Hungry, 1);
        return 0;
    }
    if (!external_set_thing_state(creatng, CrSt_CreatureToGarden)) {
        event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreatrHungry, creatng->owner, 0);
        anger_apply_anger_to_creature(creatng, crconf->annoy_no_hatchery, AngR_Hungry, 1);
        return 0;
    }
    short hunger_loss = cctrl->hunger_loss;
    short hunger_fill = crconf->hunger_fill;
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

long anger_process_creature_anger(struct Thing *creatng, const struct CreatureModelConfig *crconf)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    // Creatures with no annoyance level will never get angry
    if (crconf->annoy_level == 0) {
        return 0;
    }
    if (!creature_free_for_anger_job(creatng)) {
        return 0;
    }
    if (!anger_is_creature_angry(creatng)) {
        // If the creature is mad killing, don't allow it not to be angry
        if (creature_under_spell_effect(creatng, CSAfF_MadKilling))
        {
            // Mad creature's mind is tortured, so apply torture anger
            anger_apply_anger_to_creature(creatng, crconf->annoy_in_torture, AngR_Other, 1);
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
            struct Room* room = find_nearest_room_of_role_for_thing(creatng, creatng->owner, RoRoF_GoldStorage, NavRtF_Default);
            if (cctrl->paydays_advanced < 0)
            {
                if ((dungeon->total_money_owned >= dungeon->creatures_total_backpay) && !room_is_invalid(room))
                {
                    cctrl->paydays_advanced++;
                    if (cctrl->paydays_owed < SCHAR_MAX)
                    {
                        cctrl->paydays_owed++; // if there's enough money to pay, go to treasure room now, instead of complaining
                    }
                }
                else
                {
                    output_message(SMsg_CreatrAngryNotPaid, MESSAGE_DURATION_CRTR_MOOD);
                }
            }
            if (cctrl->paydays_advanced >= 0)
            {
                output_message(SMsg_CreatrAngryAnyReason, MESSAGE_DURATION_CRTR_MOOD);
            }
            else
            {
                output_message(SMsg_CreatrAngryNotPaid, MESSAGE_DURATION_CRTR_MOOD);
            }
            break;
        case AngR_Hungry:
            output_message(SMsg_CreatrAngryNoFood, MESSAGE_DURATION_CRTR_MOOD);
            break;
        case AngR_NoLair:
            if (cctrl->lairtng_idx != 0)
            {
                output_message(SMsg_CreatrAngryAnyReason, MESSAGE_DURATION_CRTR_MOOD);
            }
            else
            {
                output_message(SMsg_CreatrAngryNoLair, MESSAGE_DURATION_CRTR_MOOD);
            }
            break;
        case AngR_Other:
            output_message(SMsg_CreatrAngryAnyReason, MESSAGE_DURATION_CRTR_MOOD);
            break;
        default:
            output_message(SMsg_CreatrAngryAnyReason, MESSAGE_DURATION_CRTR_MOOD);
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
    if ((crconf->annoy_in_temple < 0) && (game.play_gameturn - cctrl->temple_pray_check_turn > 128))
    {
        cctrl->temple_pray_check_turn = game.play_gameturn;
        if (creature_is_doing_temple_pray_activity(creatng))
            return 1;
        if (creature_can_do_job_for_player(creatng, creatng->owner, Job_TEMPLE_PRAY, JobChk_PlayMsgOnFail)
            && can_change_from_state_to(creatng, creatng->active_state, CrSt_AtTemple))
        {
            cleanup_current_thing_state(creatng);
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

long process_creature_needs_to_heal(struct Thing *creatng, const struct CreatureModelConfig *crconf)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (game.play_gameturn >= cctrl->healing_sleep_check_turn)
    {
        if (!creature_requires_healing(creatng)) {
            return 0;
        }
        if (!creature_can_do_healing_sleep(creatng)) {
            if (creature_free_for_toking(creatng))
            {
                if (crconf->toking_recovery <= 0)
                {
                    return 0;
                }
                if (external_set_thing_state(creatng, CrSt_CreatureGoingToSafetyForToking))
                {
                    creatng->continue_state = CrSt_ImpDoingNothing;
                    return 1;
                }
            }
            return 0;
        }
        if (creature_is_doing_lair_activity(creatng)) {
            return 1;
        }
        if (!creature_free_for_sleep(creatng, CrSt_CreatureGoingHomeToSleep)) {
            return 0;
        }
        if ((game.play_gameturn - cctrl->healing_sleep_check_turn) > 128)
        {
            if ((cctrl->lair_room_id != 0) || !room_is_invalid(get_best_new_lair_for_creature(creatng)))
            {
                if (external_set_thing_state(creatng, CrSt_CreatureGoingHomeToSleep))
                {
                    return 1;
                }
            }
            else
            {
                anger_apply_anger_to_creature(creatng, crconf->annoy_no_lair, AngR_NoLair, 1);
            }
            cctrl->healing_sleep_check_turn = game.play_gameturn;
        }
    }
    return 0;
}

long process_training_need(struct Thing *thing, const struct CreatureModelConfig *crconf)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((crconf->annoy_untrained == 0) || !creature_can_be_trained(thing)) {
        return 0;
    }
    if (creature_is_training(thing)) {
        return 0;
    }
    cctrl->annoy_untrained_turn++;
    if (cctrl->annoy_untrained_turn >= crconf->annoy_untrained_time)
    {
      anger_apply_anger_to_creature(thing, crconf->annoy_untrained, AngR_Other, 1);
      cctrl->annoy_untrained_turn = 0;
    }
    return 0;
}

long process_piss_need(struct Thing *thing, const struct CreatureModelConfig *crconf)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->corpse_to_piss_on == 0) {
        return 0;
    }
    struct Thing* pisstng = thing_get(cctrl->corpse_to_piss_on);
    if ((pisstng->owner == thing->owner) || !crconf->piss_on_dead) {
        return 0;
    }
    if (game.play_gameturn - cctrl->last_piss_turn <= 200) {
        return 0;
    }
    if (!external_set_thing_state(thing, CrSt_CreaturePiss))
    {
        return 0;
    }
    cctrl->countdown = 50;
    cctrl->last_piss_turn = game.play_gameturn;
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
    if (creature_is_leaving_and_cannot_be_stopped(thing))
    {
        return;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    // Now process the needs
    process_creature_hunger(thing);
    if (process_creature_needs_to_heal_critical(thing)) {
        SYNCDBG(17,"The %s index %ld has a critical need to heal",thing_model_name(thing),(long)thing->index);
    } else
    if (creature_affected_by_call_to_arms(thing)) {
        SYNCDBG(17,"The %s index %ld is called to arms, most needs suspended",thing_model_name(thing),(long)thing->index);
    } else
    if (process_creature_needs_a_wage(thing, crconf)) {
        SYNCDBG(17,"The %s index %ld has a need to get its wage",thing_model_name(thing),(long)thing->index);
    } else
    if (process_creature_needs_to_eat(thing, crconf)) {
        SYNCDBG(17,"The %s index %ld has a need to eat",thing_model_name(thing),(long)thing->index);
    } else
    if (anger_process_creature_anger(thing, crconf)) {
        SYNCDBG(17,"The %s index %ld has a need to cool its anger",thing_model_name(thing),(long)thing->index);
    } else
    if (process_creature_needs_to_heal(thing, crconf)) {
        SYNCDBG(17,"The %s index %ld has a need to heal",thing_model_name(thing),(long)thing->index);
    }
    process_training_need(thing, crconf);
    process_piss_need(thing, crconf);
}

TbBool setup_move_off_lava(struct Thing* thing)
{
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
        const struct SlabConfigStats* slabst;
        slabst = get_slab_stats(slb);
        if (!slabst->is_safe_land)
            continue;
        // Check all subtiles of the slab in random order
        long k;
        long n;
        n = THING_RANDOM(thing, AROUND_TILES_COUNT);
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
    MapSlabCoord bx = 0;
    MapSlabCoord by = 0;
    MapSubtlCoord cx = 0;
    MapSubtlCoord cy = 0;
    struct Thing* tng;
    struct MapOffset* sstep;
    struct Map* blk;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Thing* spawngate = INVALID_THING;
    TbBool valid_flee_pos = true;
    if (is_hero_thing(thing)) //heroes flee to their original spawn gate
    {
        spawngate = find_object_of_genre_on_mapwho(OCtg_HeroGate, cctrl->flee_pos.x.stl.num, cctrl->flee_pos.y.stl.num);
    }
    if (thing_is_invalid(spawngate)) //Non heroes or heroes not from gate find a new flee position now
    {
        if (!setup_combat_flee_position(thing))
        {
            valid_flee_pos = false;
        }
    }
    if (valid_flee_pos) // If a flee position is found, go there.
    {
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

    //If there is no flee position found, or failed to setup move to flee position, try to find a random close spot without cave-in.
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
        long j = THING_RANDOM(thing, AROUND_TILES_COUNT);
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
    return false;
}

short creature_timebomb(struct Thing *creatng)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return CrStRet_Unchanged;
    }
    if ((creatng->alloc_flags & TAlF_IsControlled) == 0)
    {
        struct Thing* trgtng = get_timebomb_target(creatng);
        if (!thing_is_invalid(trgtng))
        {
            cctrl->timebomb_target_id = trgtng->index;
            cctrl->moveto_pos.x.val = trgtng->mappos.x.val;
            cctrl->moveto_pos.y.val = trgtng->mappos.y.val;
            cctrl->moveto_pos.z.val = trgtng->mappos.z.val;
            cctrl->move_flags = NavRtF_Default;
            struct Thing *enmtng = thing_get(cctrl->combat.battle_enemy_idx);
            if (!thing_is_deployed_door(enmtng))
            {
                creature_move_to(creatng, &cctrl->moveto_pos, cctrl->max_speed, NavRtF_Default, false);
            }
        }
        else
        {
            cctrl->timebomb_target_id = 0;
            creature_choose_random_destination_on_valid_adjacent_slab(creatng);
        }
        creatng->continue_state = CrSt_Timebomb;
        return CrStRet_Modified;
    }
    return CrStRet_Unchanged;
}

/******************************************************************************/
