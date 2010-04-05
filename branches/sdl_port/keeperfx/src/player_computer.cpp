/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_computer.c
 *     Computer player definitions and activities.
 * @par Purpose:
 *     Defines a computer player control variables and events/checks/processes
 *      functions.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_computer.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_memory.h"

#include "config.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_compplayer_file[]="keepcompp.cfg";

const struct NamedCommand compp_common_commands[] = {
  {"COMPUTERASSISTS", 1},
  {"PROCESSESCOUNT",  2},
  {"CHECKSCOUNT",     3},
  {"EVENTSCOUNT",     4},
  {"COMPUTERSCOUNT",  5},
  {NULL,              0},
  };

const struct NamedCommand compp_process_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PARAMS",          4},
  {"MNEMONIC",        5},
  {NULL,              0},
  };

const struct NamedCommand compp_check_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PARAMS",          4},
  {"MNEMONIC",        5},
  {NULL,              0},
  };

const struct NamedCommand compp_event_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PROCESS",         4},
  {"PARAMS",          5},
  {"MNEMONIC",        6},
  {NULL,              0},
  };

const struct NamedCommand compp_computer_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"PROCESSES",       3},
  {"CHECKS",          4},
  {"EVENTS",          5},
  {NULL,              0},
  };

ComputerType computer_assist_types[] = { 6, 7, 8, 9 };
unsigned short computer_types[] = { 201, 201, 201, 201, 201, 201, 729, 730, 731, 732 };

/******************************************************************************/
DLLIMPORT void _DK_setup_computer_players2(void);
DLLIMPORT void _DK_process_computer_player2(unsigned long plridx);
DLLIMPORT void _DK_setup_a_computer_player(unsigned long a1, long a2);
DLLIMPORT struct ComputerTask *_DK_computer_setup_build_room(struct Computer2 *comp, unsigned short a2, long a3, long a4, long a5);
DLLIMPORT void _DK_process_computer_players2(void);
DLLIMPORT long _DK_set_next_process(struct Computer2 *comp);
DLLIMPORT void _DK_computer_check_events(struct Computer2 *comp);
DLLIMPORT long _DK_process_checks(struct Computer2 *comp);
DLLIMPORT long _DK_process_tasks(struct Computer2 *comp);
DLLIMPORT long _DK_get_computer_money_less_cost(struct Computer2 *comp);

DLLIMPORT long _DK_computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_build_all_rooms _DK_computer_check_build_all_rooms
DLLIMPORT long _DK_computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_any_room_continue _DK_computer_setup_any_room_continue
DLLIMPORT long _DK_computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_any_room _DK_computer_check_any_room
DLLIMPORT long _DK_computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_any_room _DK_computer_setup_any_room
DLLIMPORT long _DK_computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_dig_to_entrance _DK_computer_check_dig_to_entrance
DLLIMPORT long _DK_computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_dig_to_entrance _DK_computer_setup_dig_to_entrance
DLLIMPORT long _DK_computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
#define computer_check_dig_to_gold _DK_computer_check_dig_to_gold
DLLIMPORT long _DK_computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_dig_to_gold _DK_computer_setup_dig_to_gold
DLLIMPORT long _DK_computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_sight_of_evil _DK_computer_check_sight_of_evil
DLLIMPORT long _DK_computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_sight_of_evil _DK_computer_setup_sight_of_evil
DLLIMPORT long _DK_computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_process_sight_of_evil _DK_computer_process_sight_of_evil
DLLIMPORT long _DK_computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_attack1 _DK_computer_check_attack1
DLLIMPORT long _DK_computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_attack1 _DK_computer_setup_attack1
DLLIMPORT long _DK_computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_completed_attack1 _DK_computer_completed_attack1
DLLIMPORT long _DK_computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_safe_attack _DK_computer_check_safe_attack
DLLIMPORT long _DK_computer_process_task(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_process_task _DK_computer_process_task
DLLIMPORT long _DK_computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_completed_build_a_room _DK_computer_completed_build_a_room
DLLIMPORT long _DK_computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_paused_task _DK_computer_paused_task
DLLIMPORT long _DK_computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_completed_task _DK_computer_completed_task

DLLIMPORT long _DK_computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_count_creatures_for_defend_pickup(struct Computer2 *comp);
DLLIMPORT long _DK_computer_find_non_solid_block(struct Computer2 *comp, struct Coord3d *pos);
DLLIMPORT long _DK_computer_able_to_use_magic(struct Computer2 *comp, long a2, long a3, long a4);
DLLIMPORT long _DK_check_call_to_arms(struct Computer2 *comp);

DLLIMPORT long _DK_computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
DLLIMPORT long _DK_computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
DLLIMPORT long _DK_computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
//TODO: we may also make "computer_event_breach" from beta
//#define computer_event_battle _DK_computer_event_battle
//#define computer_event_find_link _DK_computer_event_find_link
//#define computer_event_battle_test _DK_computer_event_battle_test
//#define computer_event_check_fighters _DK_computer_event_check_fighters
//#define computer_event_attack_magic_foe _DK_computer_event_attack_magic_foe
//#define computer_event_check_rooms_full _DK_computer_event_check_rooms_full
//#define computer_event_check_imps_in_danger _DK_computer_event_check_imps_in_danger
//#define computer_event_check_payday _DK_computer_event_check_payday

DLLIMPORT extern struct ComputerProcess _DK_BuildAllRooms3x3;
#define BuildAllRooms3x3 _DK_BuildAllRooms3x3
DLLIMPORT extern struct ComputerProcess _DK_BuildAllRooms4x4;
#define BuildAllRooms4x4 _DK_BuildAllRooms4x4
DLLIMPORT extern struct ComputerProcess _DK_BuildPrisonRoom;
#define BuildPrisonRoom _DK_BuildPrisonRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTortureRoom;
#define BuildTortureRoom _DK_BuildTortureRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildScavengerRoom;
#define BuildScavengerRoom _DK_BuildScavengerRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTempleRoom;
#define BuildTempleRoom _DK_BuildTempleRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildGraveyardRoom;
#define BuildGraveyardRoom _DK_BuildGraveyardRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildBarrackRoom;
#define BuildBarrackRoom _DK_BuildBarrackRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTreasureRoom;
#define BuildTreasureRoom _DK_BuildTreasureRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildResearchRoom;
#define BuildResearchRoom _DK_BuildResearchRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildHatcheryRoom;
#define BuildHatcheryRoom _DK_BuildHatcheryRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildLairRoom;
#define BuildLairRoom _DK_BuildLairRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTrainingRoom;
#define BuildTrainingRoom _DK_BuildTrainingRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildWorkshopRoom;
#define BuildWorkshopRoom _DK_BuildWorkshopRoom
DLLIMPORT extern struct ComputerProcess _DK_DigToEntrance;
#define DigToEntrance _DK_DigToEntrance
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldForMoney;
#define DigToGoldForMoney _DK_DigToGoldForMoney
DLLIMPORT extern struct ComputerProcess _DK_BuildTreasureRoom4x4;
#define BuildTreasureRoom4x4 _DK_BuildTreasureRoom4x4
DLLIMPORT extern struct ComputerProcess _DK_BuildLairRoom4x4;
#define BuildLairRoom4x4 _DK_BuildLairRoom4x4
DLLIMPORT extern struct ComputerProcess _DK_DigToCloseGoldForMoney;
#define DigToCloseGoldForMoney _DK_DigToCloseGoldForMoney
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldGreedy;
#define DigToGoldGreedy _DK_DigToGoldGreedy
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldGreedy2;
#define DigToGoldGreedy2 _DK_DigToGoldGreedy2
DLLIMPORT extern struct ComputerProcess _DK_ComputerSightOfEvil;
#define ComputerSightOfEvil _DK_ComputerSightOfEvil
DLLIMPORT extern struct ComputerProcess _DK_ComputerSightOfEvilScare;
#define ComputerSightOfEvilScare _DK_ComputerSightOfEvilScare
DLLIMPORT extern struct ComputerProcess _DK_ComputerAttack1;
#define ComputerAttack1 _DK_ComputerAttack1
DLLIMPORT extern struct ComputerProcess _DK_ComputerSafeAttack;
#define ComputerSafeAttack _DK_ComputerSafeAttack

/******************************************************************************/
long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
//TODO: this function address is compared; rewrite the comparison!
//long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process);
long computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_process_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process);

/*TODO!
struct ComputerProcess BuildAllRooms3x3 = {
  "BUILD ALL ROOM 3x3", 0, 3, 3, 0, -1, computer_check_build_all_rooms,
  computer_setup_any_room_continue, computer_process_task,
  computer_completed_build_a_room, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildAllRooms4x4 = {
  "BUILD ALL ROOM 4x4", 0, 4, 4, 0, -1, computer_check_build_all_rooms,
  computer_setup_any_room_continue, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildPrisonRoom = {
  "BUILD A PRISON ROOM", 0, 3, 4, 4, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTortureRoom = {
  "BUILD A TORTURE ROOM", 0, 3, 4, 5, 4, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildScavengerRoom = {
  "BUILD A SCAVENGER ROOM", 0, 3, 3, 9, 4, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTempleRoom = {
  "BUILD A TEMPLE ROOM", 0, 3, 3, 10, 8, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildGraveyardRoom = {
  "BUILD A GRAVEYARD ROOM", 0, 4, 5, 11, 5, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildBarrackRoom = {
  "BUILD A BARRACK ROOM", 0, 3, 4, 12, 6, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTreasureRoom = {
  "BUILD A TREASURE ROOM", 10, 5, 5, 2, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildResearchRoom = {
  "BUILD A RESEARCH ROOM", 0, 5, 5, 3, 2, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildHatcheryRoom = {
  "BUILD A HATCHERY", 0, 6, 5, 13, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildLairRoom = {
 "BUILD A LAIR ROOM", 0, 5, 5, 14, 7, computer_check_any_room,
 computer_setup_any_room, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTrainingRoom = {
  "BUILD A TRAINING ROOM", 0, 4, 5, 6, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildWorkshopRoom = {
 "BUILD A WORKSHOP ROOM", 0, 6, 6, 8, 3, computer_check_any_room,
 computer_setup_any_room, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToEntrance = {
 "DIG TO AN ENTRANCE", 0, 1700, 0, 0, 0, computer_check_dig_to_entrance,
 computer_setup_dig_to_entrance, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldForMoney = {
 "DIG TO GOLD", 0, 10999, 150, 7, 0, computer_check_dig_to_gold,
 computer_setup_dig_to_gold, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTreasureRoom4x4 = {
  "BUILD A TREASURE ROOM 4x4", 10, 4, 4, 2, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildLairRoom4x4 = {
  "BUILD A LAIR ROOM 4x4", 0, 4, 4, 14, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToCloseGoldForMoney = {
 "DIG TO CLOSE GOLD", 0, 30999, 500, 5, 71, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldGreedy = {
 "DIG TO GREEDY GOLD", 0, 40999, 400, 7, 900, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldGreedy2 = {
  "DIG TO GREEDY GOLD2", 0, 40999, 50, 7, 900, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSightOfEvil = {
  "SIGHT OF EVIL", 0, 8, 64, 1500, 0, computer_check_sight_of_evil,
  computer_setup_sight_of_evil, computer_process_sight_of_evil,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSightOfEvilScare = {
  "SIGHT OF EVIL SCARE", 0, 8, 10, 5000, 0, computer_check_sight_of_evil,
  computer_setup_sight_of_evil, computer_process_sight_of_evil,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerAttack1 = {
  "ATTACK PLAN 1", 0, 55, 6, 80, 0, computer_check_attack1,
  computer_setup_attack1, computer_process_task,
  computer_completed_attack1, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSafeAttack = {
  "ATTACK SAFE ATTACK", 0, 25, 4, 80, 0, computer_check_safe_attack,
  computer_setup_attack1, computer_process_task,
  computer_completed_attack1, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};
*/

struct ValidRooms valid_rooms_to_build[] = {
  {RoK_TREASURE,  &BuildTreasureRoom},
  {RoK_LAIR,      &BuildLairRoom},
  {RoK_GARDEN,    &BuildHatcheryRoom},
  {RoK_LIBRARY,   &BuildResearchRoom},
  {RoK_TRAINING,  &BuildTrainingRoom},
  {RoK_WORKSHOP,  &BuildWorkshopRoom},
  {RoK_SCAVENGER, &BuildScavengerRoom},
  {RoK_PRISON,    &BuildPrisonRoom},
  {RoK_TEMPLE,    &BuildTempleRoom},
  {RoK_TORTURE,   &BuildTortureRoom},
  {RoK_GRAVEYARD, &BuildGraveyardRoom},
  {RoK_BARRACKS,  &BuildBarrackRoom},
  {-1,            NULL},
};

char const event_pay_day_text[] = "EVENT PAY DAY";
char const event_save_imps_text[] = "EVENT SAVE IMPS";
char const event_check_room_text[] = "EVENT CHECK ROOMS FULL";
char const event_magic_foe_text[] = "EVENT MAGIC FOE";
char const event_check_fighters_text[] = "EVENT CHECK FIGHTERS";
char const event_fight_test_text[] = "EVENT FIGHT TEST";
char const event_fight_text[] = "EVENT FIGHT";
char const event_living_space_full_text[] = "EVENT LIVING SPACE FULL";
char const event_treasure_room_full_text[] = "EVENT TREASURE ROOM FULL";
char const event_heart_under_attack_text[] = "EVENT HEART UNDER ATTACK";
char const event_room_attack_text[] = "EVENT ROOM ATTACK";
char const event_dungeon_breach_text[] = "EVENT DUNGEON BREACH";

char const check_money_text[] = "CHECK MONEY";
char const check_expand_room_text[] = "CHECK EXPAND ROOM";
char const check_avail_trap_text[] = "CHECK AVAILIABLE TRAP";
char const check_neutral_places_text[] = "CHECK FOR NEUTRAL PLACES";
char const check_avail_door_text[] = "CHECK AVAILIABLE DOOR";
char const check_enemy_entrances_text[] = "CHECK FOR ENEMY ENTRANCES";
char const check_for_slap_imp_text[] = "CHECK FOR SLAP IMP";
char const check_for_speed_up_text[] = "CHECK FOR SPEED UP";
char const check_for_quick_attack_text[] = "CHECK FOR QUICK ATTACK";
char const check_to_pretty_text[] = "CHECK TO PRETTY";
char const check_enough_imps_text[] = "CHECK FOR ENOUGH IMPS";
char const move_creature_to_train_text[] = "MOVE CREATURE TO TRAINING";
char const move_creature_to_best_text[] = "MOVE CREATURE TO BEST ROOM";
char const computer_check_hates_text[] = "COMPUTER CHECK HATES";

long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check);

long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);

struct ComputerProcessMnemonic computer_process_config_list[] = {
  {"Unused", NULL,},
  {"", &BuildAllRooms3x3,},
  {"", &BuildAllRooms4x4,},
  {"", &BuildPrisonRoom,},
  {"", &BuildTortureRoom,},
  {"", &BuildScavengerRoom,},
  {"", &BuildTempleRoom,},
  {"", &BuildGraveyardRoom,},
  {"", &BuildBarrackRoom,},
  {"", &BuildTreasureRoom,},
  {"", &BuildResearchRoom,},
  {"", &BuildHatcheryRoom,},
  {"", &BuildLairRoom,},
  {"", &BuildTrainingRoom,},
  {"", &BuildWorkshopRoom,},
  {"", &DigToEntrance,},
  {"", &DigToGoldForMoney,},
  {"", &BuildTreasureRoom4x4,},
  {"", &BuildLairRoom4x4,},
  {"", &DigToCloseGoldForMoney,},
  {"", &DigToGoldGreedy,},
  {"", &DigToGoldGreedy2,},
  {"", &ComputerSightOfEvil,},
  {"", &ComputerSightOfEvilScare,},
  {"", &ComputerAttack1,},
  {"", &ComputerSafeAttack,},
};

const struct NamedCommand computer_process_func_type[] = {
  {"check_build_all_rooms",   1,},
  {"setup_any_room_continue", 2,},
  {"check_any_room",          3,},
  {"setup_any_room",          4,},
  {"check_dig_to_entrance",   5,},
  {"setup_dig_to_entrance",   6,},
  {"check_dig_to_gold",       7,},
  {"setup_dig_to_gold",       8,},
  {"check_sight_of_evil",     9,},
  {"setup_sight_of_evil",    10,},
  {"process_sight_of_evil",  11,},
  {"check_attack1",          12,},
  {"setup_attack1",          13,},
  {"completed_attack1",      14,},
  {"check_safe_attack",      15,},
  {"process_task",           16,},
  {"completed_build_a_room", 17,},
  {"paused_task",            18,},
  {"completed_task",         19,},
  {"none",                   20,},
  {NULL,                      0,},
};

Comp_Process_Func computer_process_func_list[] = {
  NULL,
  computer_check_build_all_rooms,
  computer_setup_any_room_continue,
  computer_check_any_room,
  computer_setup_any_room,
  computer_check_dig_to_entrance,
  computer_setup_dig_to_entrance,
  computer_check_dig_to_gold,
  computer_setup_dig_to_gold,
  computer_check_sight_of_evil,
  computer_setup_sight_of_evil,
  computer_process_sight_of_evil,
  computer_check_attack1,
  computer_setup_attack1,
  computer_completed_attack1,
  computer_check_safe_attack,
  computer_process_task,
  computer_completed_build_a_room,
  computer_paused_task,
  computer_completed_task,
  NULL,
  NULL,
};

ComputerName computer_check_names[COMPUTER_CHECKS_TYPES_COUNT];
struct ComputerCheck computer_checks[COMPUTER_CHECKS_TYPES_COUNT];
struct ComputerCheckMnemonic computer_check_config_list[COMPUTER_CHECKS_TYPES_COUNT];

const struct NamedCommand computer_check_func_type[] = {
  {"checks_hates",            1,},
  {"check_move_to_best_room", 2,},
  {"check_move_to_room",      3,},
  {"check_no_imps",           4,},
  {"check_for_pretty",        5,},
  {"check_for_quick_attack",  6,},
  {"check_for_accelerate",    7,},
  {"check_slap_imps",         8,},
  {"check_enemy_entrances",   9,},
  {"check_for_place_door",   10,},
  {"check_neutral_places",   11,},
  {"check_for_place_trap",   12,},
  {"check_for_expand_room",  13,},
  {"check_for_money",        14,},
  {"none",                   15,},
  {NULL,                      0,},
};

Comp_Check_Func computer_check_func_list[] = {
  NULL,
  computer_checks_hates,
  computer_check_move_creatures_to_best_room,
  computer_check_move_creatures_to_room,
  computer_check_no_imps,
  computer_check_for_pretty,
  computer_check_for_quick_attack,
  computer_check_for_accelerate,
  computer_check_slap_imps,
  computer_check_enemy_entrances,
  computer_check_for_place_door,
  computer_check_neutral_places,
  computer_check_for_place_trap,
  computer_check_for_expand_room,
  computer_check_for_money,
  NULL,
  NULL,
};

ComputerName computer_event_names[COMPUTER_EVENTS_TYPES_COUNT];
struct ComputerEvent computer_events[COMPUTER_EVENTS_TYPES_COUNT];
struct ComputerEventMnemonic computer_event_config_list[COMPUTER_EVENTS_TYPES_COUNT];

const struct NamedCommand computer_event_test_func_type[] = {
  {"event_battle_test",       1,},
  {"event_check_fighters",    2,},
  {"event_attack_magic_foe",  3,},
  {"event_check_rooms_full",  4,},
  {"event_check_imps_danger", 5,},
  {"none",                    6,},
  {NULL,                      0,},
};

Comp_EvntTest_Func computer_event_test_func_list[] = {
  NULL,
  computer_event_battle_test,
  computer_event_check_fighters,
  computer_event_attack_magic_foe,
  computer_event_check_rooms_full,
  computer_event_check_imps_in_danger,
  NULL,
  NULL,
};

const struct NamedCommand computer_event_func_type[] = {
  {"event_battle",            1,},
  {"event_find_link",         2,},
  {"event_check_payday",      3,},
  {"none",                    4,},
  {NULL,                      0,},
};

Comp_Event_Func computer_event_func_list[] = {
  NULL,
  computer_event_battle,
  computer_event_find_link,
  computer_event_check_payday,
  NULL,
  NULL,
};

ComputerName ComputerProcessListsNames[COMPUTER_PROCESS_LISTS_COUNT];
struct ComputerProcessTypes ComputerProcessLists[COMPUTER_PROCESS_LISTS_COUNT];
/******************************************************************************/

int get_computer_process_config_list_index_prc(struct ComputerProcess *process)
{
  int i;
  const int arr_size = sizeof(computer_process_config_list)/sizeof(computer_process_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    if (computer_process_config_list[i].process == process)
      return i;
  }
  return 0;
}

int get_computer_process_config_list_index_mnem(const char *mnemonic)
{
  int i;
  const int arr_size = sizeof(computer_process_config_list)/sizeof(computer_process_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    if (stricmp(computer_process_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

int get_computer_check_config_list_index_mnem(const char *mnemonic)
{
  int i;
  const int arr_size = sizeof(computer_check_config_list)/sizeof(computer_check_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    if (stricmp(computer_check_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

int get_computer_event_config_list_index_mnem(const char *mnemonic)
{
  int i;
  const int arr_size = sizeof(computer_event_config_list)/sizeof(computer_event_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    if (strcasecmp(computer_event_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

TbBool computer_type_clear_processes(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_PROCESSES_COUNT; i++)
  {
    cpt->processes[i] = NULL;
  }
  return true;
}

int computer_type_add_process(struct ComputerProcessTypes *cpt, struct ComputerProcess *process)
{
  int i;
  for (i=0; i<COMPUTER_PROCESSES_COUNT; i++)
  {
      if (cpt->processes[i] == NULL)
      {
        cpt->processes[i] = process;
        return i;
      }
  }
  return -1;
}

short computer_type_clear_checks(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_CHECKS_COUNT; i++)
  {
    LbMemorySet(&cpt->checks[i], 0, sizeof(struct ComputerCheck));
  }
  return true;
}

int computer_type_add_check(struct ComputerProcessTypes *cpt, struct ComputerCheck *check)
{
  int i;
  for (i=0; i<COMPUTER_CHECKS_COUNT; i++)
  {
      if (cpt->checks[i].name == NULL)
      {
        LbMemoryCopy(&cpt->checks[i], check, sizeof(struct ComputerCheck));
        return i;
      }
  }
  return -1;
}

short computer_type_clear_events(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_EVENTS_COUNT; i++)
  {
    LbMemorySet(&cpt->events[i], 0, sizeof(struct ComputerEvent));
  }
  return true;
}

int computer_type_add_event(struct ComputerProcessTypes *cpt, struct ComputerEvent *event)
{
  int i;
  for (i=0; i<COMPUTER_EVENTS_COUNT; i++)
  {
      if (cpt->events[i].name == NULL)
      {
        LbMemoryCopy(&cpt->events[i], event, sizeof(struct ComputerEvent));
        return i;
      }
  }
  return -1;
}

short init_computer_process_lists(void)
{
  struct ComputerProcessTypes *cpt;
  int i;
  for (i=0; i<COMPUTER_PROCESS_LISTS_COUNT; i++)
  {
    cpt = &ComputerProcessLists[i];
    LbMemorySet(cpt, 0, sizeof(struct ComputerProcessTypes));
    LbMemorySet(ComputerProcessListsNames[i], 0, LINEMSG_SIZE);
  }
  for (i=0; i<COMPUTER_PROCESS_LISTS_COUNT-1; i++)
  {
    cpt = &ComputerProcessLists[i];
    cpt->name = ComputerProcessListsNames[i];
    computer_type_clear_processes(cpt);
    computer_type_clear_checks(cpt);
  }
  return true;
}

short parse_computer_player_common_blocks(char *buf,long len)
{
  long pos;
  int k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[32];
  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
    return 0;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // COMPUTERASSISTS
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 2: // PROCESSESCOUNT
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 3: // CHECKSCOUNT
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 4: // EVENTSCOUNT
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 5: // COMPUTERSCOUNT
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return 1;
}

short parse_computer_player_process_blocks(char *buf,long len)
{
  struct ComputerProcess *process;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variable
  char block_buf[32];
  char word_buf[32];
  const int arr_size = sizeof(computer_process_config_list)/sizeof(computer_process_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"process%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    process = computer_process_config_list[i].process;
    process->parent = NULL;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_process_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          //For now, let's leave default names.
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_4 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_8 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_C = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_10 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_14 = k;
            n++;
          }
          if (n < 5)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
            //TODO: hack to make it work, for now (don't set check_dig_to_gold)
            if (k != 7)
              process->func_check = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_setup = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_task = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_complete = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_pause = computer_process_func_list[k];
              n++;
          }
          if (n < 5)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PARAMS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_30 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_34 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_38 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_3C = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_40 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_44 = k;
            n++;
          }
          if (n < 6)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 5: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_process_config_list[i].name,sizeof(computer_process_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short parse_computer_player_check_blocks(char *buf,long len)
{
  struct ComputerCheck *check;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[32];
  char word_buf[32];
  // Initialize the checks array
  const int arr_size = sizeof(computer_check_config_list)/sizeof(computer_check_config_list[0]);
  for (i=0; i < arr_size; i++)
  {
    check = &computer_checks[i];
    computer_check_config_list[i].name[0] = '\0';
    computer_check_config_list[i].check = check;
    check->name = computer_check_names[i];
    LbMemorySet(computer_check_names[i], 0, LINEMSG_SIZE);
  }
  strcpy(computer_check_names[0],"INCORRECT CHECK");
  // Load the file
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"check%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    check = computer_check_config_list[i].check;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_check_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,check->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->flags = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->turns_interval = k;
            n++;
          }
          if (n < 2)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
          k = recognize_conf_parameter(buf,&pos,len,computer_check_func_type);
          if (k > 0)
          {
              check->func = computer_check_func_list[k];
              n++;
          }
          if (n < 1)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PARAMS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param1 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param2 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param3 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->turns_last = k;
            n++;
          }
          if (n < 4)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 5: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_check_config_list[i].name,sizeof(computer_check_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short parse_computer_player_event_blocks(char *buf,long len)
{
  struct ComputerEvent *event;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[32];
  char word_buf[32];
  // Initialize the events array
  const int arr_size = sizeof(computer_event_config_list)/sizeof(computer_event_config_list[0]);
  for (i=0; i < arr_size; i++)
  {
    event = &computer_events[i];
    computer_event_config_list[i].name[0] = '\0';
    computer_event_config_list[i].event = event;
    event->name = computer_event_names[i];
    LbMemorySet(computer_event_names[i], 0, LINEMSG_SIZE);
  }
  strcpy(computer_event_names[0],"INCORRECT EVENT");
  // Load the file
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"event%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    event = computer_event_config_list[i].event;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_event_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,event->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            event->field_4 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            event->field_8 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            event->field_14 = k;
            n++;
          }
          if (n < 3)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
          k = recognize_conf_parameter(buf,&pos,len,computer_event_func_type);
          if (k > 0)
          {
              event->func_event = computer_event_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_event_test_func_type);
          if (k > 0)
          {
              event->func_test = computer_event_test_func_list[k];
              n++;
          }
          if (n < 2)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PROCESS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_process_config_list_index_mnem(word_buf);
            if (k > 0)
            {
              event->process = computer_process_config_list[k].process;
            } else
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","PROCESS",word_buf);
            }
          }
          break;
      case 5: // PARAMS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            event->param1 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            event->param2 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            event->param3 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            event->param4 = k;
            n++;
          }
          if (n < 4)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 6: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_event_config_list[i].name,sizeof(computer_event_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short write_computer_player_check_to_log(struct ComputerCheck *check)
{
  JUSTMSG("[checkXX]");
  JUSTMSG("Name = %s",check->name);
  JUSTMSG("Mnemonic = %s","XX");
  JUSTMSG("Values = %d %d",check->flags,check->turns_interval);
  JUSTMSG("Functions = %x",check->func);
  JUSTMSG("Params = %d %d %d %d",check->param1,check->param2,check->param3,check->turns_last);
  return true;
}

short write_computer_player_event_to_log(struct ComputerEvent *event)
{
  JUSTMSG("[eventXX]");
  JUSTMSG("Name = %s",event->name);
  JUSTMSG("Mnemonic = %s","XX");
  JUSTMSG("Values = %d %d %d",event->field_4,event->field_8,event->field_14);
  JUSTMSG("Functions = %x %x",event->func_event,event->func_test);
  JUSTMSG("Params = %d %d %d %d",event->param1,event->param2,event->param3,event->param4);
  return true;
}

short parse_computer_player_computer_blocks(char *buf,long len)
{
  struct ComputerProcessTypes *cpt;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variable
  char block_buf[32];
  char word_buf[32];
  const int arr_size = sizeof(ComputerProcessLists)/sizeof(ComputerProcessLists[0])-1;
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"computer%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    cpt = &ComputerProcessLists[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_computer_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,cpt->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_4 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_8 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_C = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_10 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_14 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_18 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_1C = k;
            n++;
          }
          if (n < 7)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // PROCESSES
          computer_type_clear_processes(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_process_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","PROCESSES",word_buf);
              continue;
            }
            n = computer_type_add_process(cpt, computer_process_config_list[k].process);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","PROCESSES",word_buf);
          }
          break;
      case 4: // CHECKS
          computer_type_clear_checks(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_check_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","CHECKS",word_buf);
              continue;
            }
            n = computer_type_add_check(cpt, computer_check_config_list[k].check);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","CHECKS",word_buf);
          }
          break;
      case 5: // EVENTS
          computer_type_clear_events(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_event_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","EVENTS",word_buf);
              continue;
            }
            n = computer_type_add_event(cpt, computer_event_config_list[k].event);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","EVENTS",word_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short load_computer_player_config(void)
{
  const char *fname;
  char *buf;
  long len;
  init_computer_process_lists();
  // Load the config file
  fname = prepare_file_path(FGrp_FxData,keeper_compplayer_file);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    ERRORLOG("Computer Player file \"%s\" doesn't exist or is too small.",keeper_compplayer_file);
    return false;
  }
  if (len > 65536)
  {
    ERRORLOG("Computer Player file \"%s\" is too large.",keeper_compplayer_file);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  if (len>0)
  {
    parse_computer_player_common_blocks(buf,len);
    parse_computer_player_process_blocks(buf,len);
    parse_computer_player_check_blocks(buf,len);
    parse_computer_player_event_blocks(buf,len);
    parse_computer_player_computer_blocks(buf,len);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  // Hack to synchronize local structure with the one inside DLL.
  // Remove when it's not needed anymore.
  LbMemoryCopy(_DK_ComputerProcessLists,ComputerProcessLists,13*sizeof(struct ComputerProcessTypes));
  return 1;
}

void shut_down_process(struct Computer2 *comp, struct ComputerProcess *process)
{
  Comp_Process_Func callback;
  if (process != NULL)
  {
    set_flag_dword(&process->field_44, 0x0008, true);
    set_flag_dword(&process->field_44, 0x0020, false);
    process->field_34 = game.play_gameturn;
    callback = process->func_complete;
    if (callback != NULL)
      callback(comp, process);
  }
}

void reset_process(struct Computer2 *comp, struct ComputerProcess *process)
{
  if (process != NULL)
  {
    process->field_3C = 0;
    process->field_38 = 0;
    set_flag_dword(&process->field_44, 0x0020, false);
    process->field_34 = game.play_gameturn;
  }
}

long get_computer_money_less_cost(struct Computer2 *comp)
{
  return _DK_get_computer_money_less_cost(comp);
}

struct ComputerTask *computer_setup_build_room(struct Computer2 *comp, unsigned short rkind, long a3, long a4, long a5)
{
  return _DK_computer_setup_build_room(comp, rkind, a3, a4, a5);
}

long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process)
{
  //return _DK_computer_setup_any_room(comp, process);
  struct ComputerTask *task;
  long i;
  task = computer_setup_build_room(comp, process->field_10, process->field_8, process->field_C, process->field_14);
  if (task != NULL)
  {
    set_flag_dword(&process->field_44, 0x0020, true);
    i = (long)((char *)process - (char *)&comp->processes[0]) / sizeof(struct ComputerProcess);
    if ((i < 0) || (i > COMPUTER_PROCESSES_COUNT))
    {
      ERRORLOG("Process \"%s\" is outside of Computer Player.",process->name);
      i = COMPUTER_PROCESSES_COUNT;
    }
    task->field_8C = i;
    shut_down_process(comp, process);
    return 2;
  }
  if (process->field_8 > process->field_C)
  {
    if (process->field_8 <= 2)
      return 0;
    process->field_8--;
  } else
  {
    if (process->field_C <= 2)
      return 0;
    process->field_C--;
  }
  reset_process(comp, process);
  return 2;
}

long computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_setup_dig_to_entrance(comp, process);
}

long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_setup_dig_to_gold(comp, process);
}

long computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_setup_any_room_continue(comp, process);
}

long computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_setup_sight_of_evil(comp, process);
}

long computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_setup_attack1(comp, process);
}

long computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_build_all_rooms(comp, process);
}

long computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_any_room(comp, process);
}

long computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_dig_to_entrance(comp, process);
}

/*TODO enable when computer_check_dig_to_gold offset will not be used in DLL
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_dig_to_gold(comp, process);
}
*/

long computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_sight_of_evil(comp, process);
}

long computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_attack1(comp, process);
}

long computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_safe_attack(comp, process);
}

long computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_process_sight_of_evil(comp, process);
}

long computer_process_task(struct Computer2 *comp, struct ComputerProcess *process)
{
  return 0;
}

long computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process)
{
  comp->field_0 = 2;
  return 0;
}

long computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process)
{
  process->field_34 = game.play_gameturn;
  comp->field_0 = 2;
  return 0;
}

long computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_completed_attack1(comp, process);
}

long computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process)
{
  process->field_44 &= 0xFFFFFFF7u;
  comp->field_0 = 2;
  return 0;
}

long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_checks_hates(comp, check);
}

long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_move_creatures_to_best_room(comp, check);
}

long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_move_creatures_to_room(comp, check);
}

long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_no_imps(comp, check);
}

long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_for_pretty(comp, check);
}

long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_for_quick_attack(comp, check);
}

long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_for_accelerate(comp, check);
}

long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_slap_imps(comp, check);
}

long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_enemy_entrances(comp, check);
}

long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_for_place_door(comp, check);
}

long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_neutral_places(comp, check);
}

long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_for_place_trap(comp, check);
}

long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check)
{
  return _DK_computer_check_for_expand_room(comp, check);
}

long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check)
{
    long money;
    struct ComputerProcess *cproc;
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    struct Room *room;
    long new_tasks,crstate,ret;
    unsigned long k;
    long i;
    //return _DK_computer_check_for_money(comp, check);
    ret = 4;
    money = get_computer_money_less_cost(comp);
    if ((check->param3 > money) || (check->param2 > money))
    {
      for (i=0; i <= COMPUTER_PROCESSES_COUNT; i++)
      {
          cproc = &comp->processes[i];
          if ((cproc->field_44 & 0x02) != 0)
              break;
          if (cproc->func_check == computer_check_dig_to_gold)
          {
            cproc->field_4++;
            if (game.play_gameturn - cproc->field_3C > 20)
              cproc->field_3C = 0;
          }
      }
    }

    dungeon = comp->field_24;
    if (dungeon->field_14B8 > dungeon->field_AF9)
    {
      if (dungeon->room_kind[8] != 0)
      {
        if (get_task_in_progress(comp, CTT_SellTrapsAndDoors) == NULL)
        {
          if (create_task_sell_traps_and_doors(comp, 3*dungeon->field_14B8/2))
          {
            ret = 1;
          }
        }
      }
    }
    if (3*dungeon->field_14B8/2 <= dungeon->field_AF9)
      return ret;

    room = room_get(dungeon->room_kind[14]);
    if (room_is_invalid(room))
      return ret;
    new_tasks = 0;
    // Sweep through creatures list
    i = dungeon->creatr_list_start;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      if (thing_is_invalid(thing))
      {
        ERRORLOG("Jump to invalid thing detected");
        break;
      }
      cctrl = creature_control_get_from_thing(thing);
      i = cctrl->thing_idx;
      // Per creature code
      if (thing->field_7 == 14)
        crstate = thing->field_8;
      else
        crstate = thing->field_7;
      if (crstate == 33)
      {
        if (!create_task_move_creature_to_pos(comp, thing, room->stl_x, room->stl_y))
          return ret;
        new_tasks++;
        ret = 1;
        if (new_tasks >= 3)
          return ret;
      }
      // Per creature code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    if (new_tasks >= 3)
      return ret;
    // Sweep through workers list
    i = dungeon->worker_list_start;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      if (thing_is_invalid(thing))
      {
        ERRORLOG("Jump to invalid thing detected");
        break;
      }
      cctrl = creature_control_get_from_thing(thing);
      i = cctrl->thing_idx;
      // Per creature code
      if (thing->field_7 == 14)
        crstate = thing->field_8;
      else
        crstate = thing->field_7;
      if (crstate == 33)
      {
        if (!create_task_move_creature_to_pos(comp, thing, room->stl_x, room->stl_y))
          return ret;
        new_tasks++;
        ret = 1;
        if (new_tasks >= 3)
          return ret;
      }
      // Per creature code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    return ret;
}

long count_creatures_for_defend_pickup(struct Computer2 *comp)
{
    return _DK_count_creatures_for_defend_pickup(comp);
}

long computer_find_non_solid_block(struct Computer2 *comp, struct Coord3d *pos)
{
    return _DK_computer_find_non_solid_block(comp, pos);
}

long computer_able_to_use_magic(struct Computer2 *comp, long a2, long a3, long a4)
{
    return _DK_computer_able_to_use_magic(comp, a2, a3, a4);
}

long check_call_to_arms(struct Computer2 *comp)
{
    return _DK_check_call_to_arms(comp);
}

long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
    long creatrs_def, creatrs_num;
    struct Coord3d pos;
    //return _DK_computer_event_battle(comp, cevent, event);
    pos.x.stl.num = event->mappos_x;
    pos.x.stl.pos = 0;
    pos.y.stl.num = event->mappos_y;
    pos.y.stl.pos = 0;
    pos.z.val = 0;
    if ((pos.x.val <= 0) || (pos.y.val <= 0))
        return false;
    creatrs_def = count_creatures_for_defend_pickup(comp);
    creatrs_num = creatrs_def * cevent->param1 / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0))
        creatrs_num = 1;
    if (creatrs_num <= 0)
        return false;
    if (!computer_find_non_solid_block(comp, &pos))
        return false;
    if (!get_task_in_progress(comp, CTT_MoveCreaturesToDefend) || ((cevent->param2 & 0x02) != 0))
    {
        return create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->param2);
    } else
    if (computer_able_to_use_magic(comp, 6, 1, 1) == 1)
    {
        if (!get_task_in_progress(comp, CTT_MagicCallToArms) || ((cevent->param2 & 0x02) != 0))
        {
            if ( check_call_to_arms(comp) )
            {
                return create_task_magic_call_to_arms(comp, &pos, creatrs_num);
            }
        }
    }
    return false;
}

long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
  return _DK_computer_event_find_link(comp, cevent, event);
}

long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_battle_test(comp, cevent);
}

long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_check_fighters(comp, cevent);
}

long computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_attack_magic_foe(comp, cevent);
}

long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_check_rooms_full(comp, cevent);
}

long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_check_imps_in_danger(comp, cevent);
}

long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
  return _DK_computer_event_check_payday(comp, cevent, event);
}

void setup_a_computer_player(unsigned short plyridx, long comp_model)
{
  struct ComputerProcessTypes *cproctype;
  struct ComputerProcess *process;
  struct ComputerProcess *newproc;
  struct ComputerCheck *check;
  struct ComputerCheck *newchk;
  struct ComputerEvent *event;
  struct ComputerEvent *newevnt;
  struct Comp2_UnkStr1 *unkptr;
  struct Computer2 *comp;
  long i;
  //_DK_setup_a_computer_player(plyridx, comp_model); return;
  comp = &game.computer[plyridx];
  LbMemorySet(comp, 0, sizeof(struct Computer2));
  cproctype = &ComputerProcessLists[comp_model];
  comp->field_24 = get_players_num_dungeon(plyridx);
  comp->model = comp_model;
  comp->field_18 = cproctype->field_C;
  comp->field_14 = cproctype->field_8;
  comp->field_30 = cproctype->field_10;
  comp->field_2C = cproctype->field_14;
  comp->field_20 = cproctype->field_18;
  comp->field_C = 1;
  comp->field_0 = 2;

  for (i=0; i < 5; i++)
  {
    unkptr = &comp->unkarr_A10[i];
    if (i == plyridx)
      unkptr->field_6 = 0x80000000;
    else
      unkptr->field_6 = 0;
  }
  comp->field_1C = cproctype->field_4;

  for (i=0; i < COMPUTER_PROCESSES_COUNT; i++)
  {
    process = cproctype->processes[i];
    newproc = &comp->processes[i];
    if ((process == NULL) || (process->name == NULL))
    {
      newproc->name = NULL;
      break;
    }
    // Modifying original ComputerProcessTypes structure - I don't like it!
    if (process->func_setup == computer_setup_any_room)
    {
      if (process->field_14 >= 0)
        process->field_14 = get_room_look_through(process->field_14);
    }
    LbMemoryCopy(newproc, process, sizeof(struct ComputerProcess));
    newproc->parent = process;
  }
  newproc = &comp->processes[i];
  newproc->field_44 |= 0x02;

  for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
  {
    check = &cproctype->checks[i];
    newchk = &comp->checks[i];
    if ((check == NULL) || (check->name == NULL))
    {
      newchk->name = NULL;
      break;
    }
    LbMemoryCopy(newchk, check, sizeof(struct ComputerCheck));
  }
  // Note that we don't have special, empty check at end of array
  // The check with 0x02 flag identifies end of active checks
  // (the check with 0x02 flag is invalid - only previous checks are in use)
  //newchk = &comp->checks[i];
  newchk->flags |= 0x02;

  for (i=0; i < COMPUTER_EVENTS_COUNT; i++)
  {
    event = &cproctype->events[i];
    newevnt = &comp->events[i];
    if ((event == NULL) || (event->name == NULL))
    {
      newevnt->name = NULL;
      break;
    }
    LbMemoryCopy(newevnt, event, sizeof(struct ComputerEvent));
  }
}

long set_next_process(struct Computer2 *comp)
{
  return _DK_set_next_process(comp);
}

void computer_check_events(struct Computer2 *comp)
{
  _DK_computer_check_events(comp);
}

TbBool process_checks(struct Computer2 *comp)
{
    struct ComputerCheck *ccheck;
    long delta;
    long i;
    //return _DK_process_checks(comp);
    for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        ccheck = &comp->checks[i];
        if (comp->field_10 <= 0)
            break;
        if ((ccheck->flags & 0x02) != 0)
            break;
        if ((ccheck->flags & 0x01) == 0)
        {
            delta = (game.play_gameturn - ccheck->turns_last);
            if (delta > ccheck->turns_interval)
            {
              ccheck->func(comp, ccheck);
              ccheck->turns_last = game.play_gameturn;
            }
        }
    }
    return true;
}

TbBool process_processes_and_task(struct Computer2 *comp)
{
  struct ComputerProcess *process;
  Comp_Process_Func callback;
  int i;
  for (i=comp->field_10; i > 0; i--)
  {
    if (comp->field_10 <= 0)
      return false;
    if ((game.play_gameturn % comp->field_18) == 0)
      process_tasks(comp);
    switch (comp->field_0)
    {
    case 1:
        comp->field_8--;
        if (comp->field_8 <= 0)
        {
          comp->field_8 = comp->field_4;
          set_next_process(comp);
        }
        break;
    case 2:
        set_next_process(comp);
        break;
    case 3:
        if (comp->field_14C4 > 0)
        {
          process = &comp->processes[comp->field_14C4];
          callback = process->func_task;
          if (callback != NULL)
            callback(comp,process);
        } else
        {
          ERRORLOG("No Process for a computer player");
          comp->field_0 = 1;
        }
        break;
    }
  }
  return true;
}

void process_computer_player2(unsigned long plyr_idx)
{
  struct Computer2 *comp;
  SYNCDBG(7,"Starting for player %lu",plyr_idx);
  //_DK_process_computer_player2(plyr_idx);
  comp = &game.computer[plyr_idx%PLAYERS_COUNT];
  if ((comp->field_14 != 0) && (comp->field_2C <= game.play_gameturn))
    comp->field_10 = 1;
  else
    comp->field_10 = 0;
  if (comp->field_10 <= 0)
    return;
  computer_check_events(comp);
  process_checks(comp);
  process_processes_and_task(comp);
  if ((comp->field_10 < 0) || (comp->field_10 > 1))
    ERRORLOG("Computer performed more than one task");
}

struct ComputerProcess *computer_player_find_process_by_func_setup(long plyr_idx,Comp_Process_Func func_setup)
{
  struct ComputerProcess *process;
  struct Computer2 *comp;
  comp = &(game.computer[plyr_idx]);
  process = &comp->processes[0];
  while ((process->field_44 & 0x02) == 0)
  {
    if (process->func_setup == func_setup)
    {
        return process;
    }
    process++;
  }
  return NULL;
}

TbBool computer_player_demands_gold_check(long plyr_idx)
{
  struct ComputerProcess *dig_process;
  dig_process = computer_player_find_process_by_func_setup(plyr_idx,computer_setup_dig_to_gold);
  // If this computer player has no gold digging process
  if (dig_process == NULL)
    return false;
  if ((dig_process->field_44 & 0x04) == 0)
    return false;
  // If the computer player needs to dig for gold
  if (gameadd.turn_last_checked_for_gold+5000 < game.play_gameturn)
  {
    set_flag_dword(&dig_process->field_44, 0x04, false);
    return true;
  }
  return false;
}

void process_computer_players2(void)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  TbBool needs_gold_check;
  int i;
  //_DK_process_computer_players2();
  needs_gold_check = false;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if ((player->field_0 & 0x01) == 0)
      continue;
    dungeon = get_players_dungeon(player);
    if (((player->field_0 & 0x40) != 0) || ((dungeon->computer_enabled & 0x01) != 0))
    {
      if (player->field_2C == 1)
      {
        process_computer_player2(i);
        if (computer_player_demands_gold_check(i))
        {
          needs_gold_check = true;
        }
      }
    }
  }
  if (needs_gold_check)
  {
    SYNCDBG(0,"Computer players demand gold check.");
    gameadd.turn_last_checked_for_gold = game.play_gameturn;
    check_map_for_gold();
  } else
  if (gameadd.turn_last_checked_for_gold > game.play_gameturn)
  {
    gameadd.turn_last_checked_for_gold = 0;
  }
}

void setup_computer_players2(void)
{
  struct PlayerInfo *player;
  int i;
  gameadd.turn_last_checked_for_gold = game.play_gameturn;
  check_map_for_gold();
  for (i=0; i < COMPUTER_TASKS_COUNT; i++)
  {
    LbMemorySet(&game.computer_task[i], 0, sizeof(struct ComputerTask));
  }
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if ((player->field_0 & 0x01) != 0)
    {
      if (player->field_2C == 1)
      {
        setup_a_computer_player(i, 7);
      }
    }
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
