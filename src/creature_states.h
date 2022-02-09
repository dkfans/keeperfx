/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states.h
 *     Header file for creature_states.c.
 * @par Purpose:
 *     Creature states structure and function definitions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     23 Sep 2009 - 11 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRSTATE_H
#define DK_CRTRSTATE_H

#include "bflib_basics.h"
#include "globals.h"
#include "dungeon_data.h"

/** Count of creature states, originally 147. */
#define CREATURE_STATES_COUNT CrSt_ListEnd

#define FIGHT_FEAR_DELAY 160

#ifdef __cplusplus
extern "C" {
#endif

enum CreatureStates {
    CrSt_Unused = 0,
    CrSt_ImpDoingNothing,
    CrSt_ImpArrivesAtDigDirt,
    CrSt_ImpArrivesAtMineGold,
    CrSt_ImpDigsDirt,
    CrSt_ImpMinesGold,
    CrSt_Null6,
    CrSt_ImpDropsGold,
    CrSt_ImpLastDidJob,
    CrSt_ImpArrivesAtImproveDungeon,
    CrSt_ImpImprovesDungeon,//[10]
    CrSt_CreaturePicksUpTrapObject,
    CrSt_CreatureArmsTrap,
    CrSt_CreaturePicksUpCrateForWorkshop,
    CrSt_MoveToPosition,
    CrSt_Null15,
    CrSt_CreatureDropsCrateInWorkshop,
    CrSt_CreatureDoingNothing,
    CrSt_CreatureToGarden,
    CrSt_CreatureArrivedAtGarden,
    CrSt_CreatureWantsAHome,//[20]
    CrSt_CreatureChooseRoomForLairSite,
    CrSt_CreatureAtNewLair,
    CrSt_PersonSulkHeadForLair,
    CrSt_PersonSulkAtLair,
    CrSt_CreatureGoingHomeToSleep,
    CrSt_CreatureSleep,
    CrSt_Null27,
    CrSt_Tunnelling,
    CrSt_Null29,
    CrSt_AtResearchRoom,//[30]
    CrSt_Researching,
    CrSt_AtTrainingRoom,
    CrSt_Training,
    CrSt_GoodDoingNothing,
    CrSt_GoodReturnsToStart,
    CrSt_GoodBackAtStart,
    CrSt_GoodDropsGold,
    CrSt_InPowerHand,
    CrSt_ArriveAtCallToArms,
    CrSt_CreatureArrivedAtPrison,//[40]
    CrSt_CreatureInPrison,
    CrSt_AtTortureRoom,
    CrSt_Torturing,
    CrSt_AtWorkshopRoom,
    CrSt_Manufacturing,
    CrSt_AtScavengerRoom,
    CrSt_Scavengering,
    CrSt_CreatureDormant, // For neutral creatures, moving around without purpose
    CrSt_CreatureInCombat,
    CrSt_CreatureLeavingDungeon,//[50]
    CrSt_CreatureLeaves,
    CrSt_CreatureInHoldAudience,
    CrSt_PatrolHere,
    CrSt_Patrolling,
    CrSt_Null55,
    CrSt_Null56,
    CrSt_Null57,
    CrSt_Null58,
    CrSt_CreatureKillCreatures,
    CrSt_Null60,//[60]
    CrSt_PersonSulking,
    CrSt_Null62,
    CrSt_Null63,
    CrSt_AtBarrackRoom,
    CrSt_Barracking,
    CrSt_CreatureSlapCowers,
    CrSt_CreatureUnconscious,
    CrSt_CreaturePickUpUnconsciousBody,
    CrSt_ImpToking,
    CrSt_ImpPicksUpGoldPile,//[70]
    CrSt_MoveBackwardsToPosition,
    CrSt_CreatureDropBodyInPrison,
    CrSt_ImpArrivesAtConvertDungeon,
    CrSt_ImpConvertsDungeon,
    CrSt_CreatureWantsSalary,
    CrSt_CreatureTakeSalary,
    CrSt_TunnellerDoingNothing,
    CrSt_CreatureObjectCombat,
    CrSt_Null79,
    CrSt_CreatureChangeLair,//[80]
    CrSt_ImpBirth,
    CrSt_AtTemple,
    CrSt_PrayingInTemple,
    CrSt_Null84,
    CrSt_CreatureFollowLeader,
    CrSt_CreatureDoorCombat,
    CrSt_CreatureCombatFlee,
    CrSt_CreatureSacrifice,
    CrSt_AtLairToSleep,
    CrSt_CreatureExempt,//[90]
    CrSt_CreatureBeingDropped,
    CrSt_CreatureBeingSacrificed,
    CrSt_CreatureScavengedDisappear,
    CrSt_CreatureScavengedReappear,
    CrSt_CreatureBeingSummoned,
    CrSt_CreatureHeroEntering, // State used for hero entering from ceiling
    CrSt_ImpArrivesAtReinforce,
    CrSt_ImpReinforces,
    CrSt_ArriveAtAlarm,
    CrSt_CreaturePicksUpSpellObject,//[100]
    CrSt_CreatureDropsSpellObjectInLibrary,
    CrSt_CreaturePicksUpCorpse,
    CrSt_CreatureDropsCorpseInGraveyard,
    CrSt_AtGuardPostRoom,
    CrSt_Guarding,
    CrSt_CreatureEat,
    CrSt_CreatureEvacuateRoom,
    CrSt_CreatureWaitAtTreasureRoomDoor,
    CrSt_AtKinkyTortureRoom,
    CrSt_KinkyTorturing,//[110]
    CrSt_MadKillingPsycho,
    CrSt_CreatureSearchForGoldToStealInRoom1,
    CrSt_CreatureVandaliseRooms,
    CrSt_CreatureStealGold,
    CrSt_SeekTheEnemy,
    CrSt_AlreadyAtCallToArms,
    CrSt_CreatureDamageWalls,
    CrSt_CreatureAttemptToDamageWalls,
    CrSt_CreaturePersuade,
    CrSt_CreatureChangeToChicken,//[120]
    CrSt_CreatureChangeFromChicken,
    CrSt_ManualControl,
    CrSt_CreatureCannotFindAnythingToDo,
    CrSt_CreaturePiss,
    CrSt_CreatureRoar,
    CrSt_CreatureAtChangedLair,
    CrSt_CreatureBeHappy,
    CrSt_GoodLeaveThroughExitDoor,
    CrSt_GoodWaitInExitDoor,
    CrSt_GoodAttackRoom1,//[130]
    CrSt_CreatureSearchForGoldToStealInRoom2,
    CrSt_GoodAttackRoom2,
    CrSt_CreaturePretendChickenSetupMove,
    CrSt_CreaturePretendChickenMove,
    CrSt_CreatureAttackRooms,
    CrSt_CreatureFreezePrisoners,
    CrSt_CreatureExploreDungeon,
    CrSt_CreatureEatingAtGarden,
    CrSt_LeavesBecauseOwnerLost,
    CrSt_CreatureMoan,//[140]
    CrSt_CreatureSetWorkRoomBasedOnPosition,
    CrSt_CreatureBeingScavenged,
    CrSt_CreatureEscapingDeath,
    CrSt_CreaturePresentToDungeonHeart,
    CrSt_CreatureSearchForSpellToStealInRoom,
    CrSt_CreatureStealSpell,
    CrSt_GoodArrivedAtAttackRoom,
    CrSt_CreatureGoingToSafetyForToking,
    CrSt_ListEnd,
};

enum CreatureTrainingModes {
    CrTrMd_Unused = 0,
    CrTrMd_SearchForTrainPost,
    CrTrMd_SelectPositionNearTrainPost,
    CrTrMd_MoveToTrainPost,
    CrTrMd_TurnToTrainPost,
    CrTrMd_DoTrainWithTrainPost,
    CrTrMd_PartnerTraining,
};

enum CreatureStateTypes {
    CrStTyp_Idle = 0,
    CrStTyp_Work,
    CrStTyp_OwnNeeds,
    CrStTyp_Sleep,
    CrStTyp_Feed,
    CrStTyp_FightCrtr,
    CrStTyp_Move,
    CrStTyp_GetsSalary,
    CrStTyp_Escape,
    CrStTyp_Unconscious,
    CrStTyp_AngerJob,
    CrStTyp_FightDoor,
    CrStTyp_FightObj,
    CrStTyp_Called2Arms,
    CrStTyp_Follow,
    CrStTyp_ListEnd,
};

/** Defines return values of creature state functions. */
enum CreatureStateReturns {
    CrStRet_Deleted       = -1, /**< Returned if the creature being updated no longer exists. */
    CrStRet_Unchanged     =  0, /**< Returned if no change was made to the creature data. */
    CrStRet_Modified      =  1, /**< Returned if the creature was updated and possibly some variables have changed inside, including state. */
    CrStRet_ResetOk       =  2, /**< Returned if the creature state has been reset because task was completed. */
    CrStRet_ResetFail     =  3, /**< Returned if the creature state has been reset, task was either abandoned or couldn't be completed. */
};

/** Defines return values of creature state check functions. */
enum CreatureCheckReturns {
    CrCkRet_Deleted       = -1, /**< Returned if the creature being updated no longer exists. */
    CrCkRet_Available     =  0, /**< Returned if the creature is available for additional processing, even reset. */
    CrCkRet_Continue      =  1, /**< Returned if the action being performed on the creature shall continue, creature shouldn't be processed. */
};

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;

typedef short (*CreatureStateFunc1)(struct Thing *);
typedef char (*CreatureStateFunc2)(struct Thing *);
typedef CrCheckRet (*CreatureStateCheck)(struct Thing *);

struct StateInfo { // sizeof = 41
    CreatureStateFunc1 process_state;
    CreatureStateFunc1 cleanup_state;
    CreatureStateFunc2 move_from_slab;
    CreatureStateCheck move_check; /**< Check function to be used when the creature is in moving sub-state during that state. */
    unsigned char override_feed;
    unsigned char override_own_needs;
    unsigned char override_sleep;
    unsigned char override_fight_crtr;
    unsigned char override_gets_salary;
    unsigned char override_prev_fld1F;
    unsigned char override_prev_fld20;
    unsigned char override_escape;
    unsigned char override_unconscious;
    unsigned char override_anger_job;
    unsigned char override_fight_object;
    unsigned char override_fight_door;
    unsigned char override_call2arms;
    unsigned char override_follow;
    unsigned char state_type;
  unsigned char field_1F;
  unsigned char field_20;
  unsigned short field_21;
  unsigned char field_23;
    unsigned short sprite_idx;
  unsigned char field_26;
  unsigned char field_27;
  unsigned char react_to_cta;
};

/******************************************************************************/
DLLIMPORT struct StateInfo _DK_states[];
//#define states _DK_states
extern struct StateInfo states[];
DLLIMPORT long _DK_r_stackpos;
#define r_stackpos _DK_r_stackpos
DLLIMPORT struct DiggerStack _DK_reinforce_stack[DIGGER_TASK_MAX_COUNT];
#define reinforce_stack _DK_reinforce_stack

#pragma pack()
/******************************************************************************/

extern long const state_type_to_gui_state[];
/******************************************************************************/
CrtrStateId get_creature_state_besides_move(const struct Thing *thing);
CrtrStateId get_creature_state_besides_drag(const struct Thing *thing);
CrtrStateId get_creature_state_besides_interruptions(const struct Thing *thing);
long get_creature_state_type_f(const struct Thing *thing, const char *func_name);
#define get_creature_state_type(thing) get_creature_state_type_f(thing,__func__)

struct StateInfo *get_thing_active_state_info(struct Thing *thing);
struct StateInfo *get_thing_continue_state_info(struct Thing *thing);
struct StateInfo *get_thing_state_info_num(CrtrStateId state_id);
struct StateInfo *get_creature_state_with_task_completion(struct Thing *thing);

struct TunnelDistance{
    unsigned int creatid;
    unsigned long olddist;
    unsigned long newdist;
};

TbBool state_info_invalid(struct StateInfo *stati);
TbBool can_change_from_state_to(const struct Thing *thing, CrtrStateId curr_state, CrtrStateId next_state);
TbBool internal_set_thing_state(struct Thing *thing, CrtrStateId nState);
TbBool external_set_thing_state_f(struct Thing *thing, CrtrStateId state, const char *func_name);
#define external_set_thing_state(thing,state) external_set_thing_state_f(thing,state,__func__)
TbBool init_creature_state(struct Thing *creatng);
TbBool initialise_thing_state_f(struct Thing *thing, CrtrStateId nState, const char *func_name);
#define initialise_thing_state(thing, nState) initialise_thing_state_f(thing, nState,__func__)
TbBool cleanup_current_thing_state(struct Thing *thing);
TbBool cleanup_creature_state_and_interactions(struct Thing *thing);
short state_cleanup_in_room(struct Thing *creatng);
short set_start_state_f(struct Thing *thing,const char *func_name);
#define set_start_state(thing) set_start_state_f(thing,__func__)
short patrol_here(struct Thing* creatng);
short patrolling(struct Thing* creatng);
/******************************************************************************/
TbBool creature_model_bleeds(unsigned long crmodel);
TbBool creature_can_hear_within_distance(const struct Thing *thing, long dist);
long get_thing_navigation_distance(struct Thing *creatng, struct Coord3d *pos , unsigned char a3);
void create_effect_around_thing(struct Thing *thing, long eff_kind);
long get_creature_gui_job(const struct Thing *thing);
long setup_head_for_empty_treasure_space(struct Thing *thing, struct Room *room);
long process_creature_needs_to_heal_critical(struct Thing *creatng);
short setup_creature_leaves_or_dies(struct Thing *creatng);

void creature_drop_dragged_object(struct Thing *crtng, struct Thing *dragtng);
void creature_drag_object(struct Thing *creatng, struct Thing *dragtng);
TbBool creature_is_dragging_something(const struct Thing *creatng);
TbBool creature_is_dragging_spellbook(const struct Thing *creatng);

void make_creature_conscious(struct Thing *creatng);
void make_creature_unconscious(struct Thing *creatng);
void make_creature_conscious_without_changing_state(struct Thing *creatng);

TbBool check_experience_upgrade(struct Thing *thing);
void set_creature_size_stuff(struct Thing *creatng);
long process_work_speed_on_work_value(const struct Thing *thing, long base_val);
TbBool find_random_valid_position_for_thing_in_room_avoiding_object(struct Thing *thing, const struct Room *room, struct Coord3d *pos);
SubtlCodedCoords find_position_around_in_room(const struct Coord3d *pos, PlayerNumber owner, RoomKind rkind, struct Thing *thing);
void remove_health_from_thing_and_display_health(struct Thing *thing, long delta);
TbBool slab_by_players_land(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);

TbBool process_creature_hunger(struct Thing *thing);
void process_person_moods_and_needs(struct Thing *thing);
TbBool restore_creature_flight_flag(struct Thing *creatng);
TbBool attempt_to_destroy_enemy_room(struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

TbBool room_initially_valid_as_type_for_thing(const struct Room *room, RoomKind rkind, const struct Thing *thing);
TbBool room_still_valid_as_type_for_thing(const struct Room *room, RoomKind rkind, const struct Thing *thing);
TbBool creature_job_in_room_no_longer_possible_f(const struct Room *room, CreatureJob jobpref, const struct Thing *thing, const char *func_name);
#define creature_job_in_room_no_longer_possible(room, jobpref, thing) creature_job_in_room_no_longer_possible_f(room, jobpref, thing, __func__)
TbBool creature_free_for_sleep(const struct Thing *thing,  CrtrStateId state);

// Finding a nearby position to move during a job
TbBool creature_choose_random_destination_on_valid_adjacent_slab(struct Thing *thing);
TbBool person_get_somewhere_adjacent_in_room_f(struct Thing *thing, const struct Room *room, struct Coord3d *pos, const char *func_name);
#define person_get_somewhere_adjacent_in_room(thing, room, pos) person_get_somewhere_adjacent_in_room_f(thing, room, pos, __func__)
TbBool person_get_somewhere_adjacent_in_room_around_borders_f(struct Thing *thing, const struct Room *room, struct Coord3d *pos, const char *func_name);
#define person_get_somewhere_adjacent_in_room_around_borders(thing, room, pos) person_get_somewhere_adjacent_in_room_around_borders_f(thing, room, pos, __func__)

void place_thing_in_creature_controlled_limbo(struct Thing *thing);
void remove_thing_from_creature_controlled_limbo(struct Thing *thing);
TbBool get_random_position_in_dungeon_for_creature(PlayerNumber plyr_idx, unsigned char wandr_select, struct Thing *thing, struct Coord3d *pos);
/******************************************************************************/
TbBool creature_is_dying(const struct Thing *thing);
TbBool creature_is_being_dropped(const struct Thing *thing);
TbBool creature_is_being_unconscious(const struct Thing *thing);
TbBool creature_is_celebrating(const struct Thing *thing);
TbBool creature_is_being_tortured(const struct Thing *thing);
TbBool creature_is_being_sacrificed(const struct Thing *thing);
TbBool creature_is_kept_in_prison(const struct Thing *thing);
TbBool creature_is_being_summoned(const struct Thing *thing);
TbBool creature_is_doing_anger_job(const struct Thing *thing);
TbBool creature_is_doing_garden_activity(const struct Thing *thing);
TbBool creature_is_taking_salary_activity(const struct Thing *thing);
TbBool creature_is_doing_temple_pray_activity(const struct Thing *thing);
TbBool creature_is_training(const struct Thing *thing);
TbBool creature_is_being_scavenged(const struct Thing *thing);
TbBool creature_is_scavengering(const struct Thing *thing);
TbBool creature_is_at_alarm(const struct Thing *thing);
TbBool creature_is_escaping_death(const struct Thing *thing);
TbBool creature_is_fleeing_combat(const struct Thing *thing);
TbBool creature_is_called_to_arms(const struct Thing *thing);
TbBool creature_affected_by_call_to_arms(const struct Thing *thing);
TbBool creature_is_kept_in_custody(const struct Thing *thing);
TbBool creature_is_kept_in_custody_by_enemy(const struct Thing *thing);
TbBool creature_is_kept_in_custody_by_player(const struct Thing *thing, PlayerNumber plyr_idx);
short player_keeping_creature_in_custody(const struct Thing* thing);
TbBool creature_state_is_unset(const struct Thing *thing);
TbBool creature_will_attack_creature(const struct Thing *tng1, const struct Thing *tng2);
TbBool creature_will_attack_creature_incl_til_death(const struct Thing *tng1, const struct Thing *tng2);
// Compound checks for specific cases
TbBool creature_is_kept_in_custody_by_enemy_or_dying(const struct Thing *thing);

TbBool creature_state_cannot_be_blocked(const struct Thing *thing);

TbBool setup_move_off_lava(struct Thing* thing);
TbBool setup_move_out_of_cave_in(struct Thing* thing);

struct Room* get_room_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
