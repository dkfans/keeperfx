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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Room;

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
    CrSt_CreaturePicksUpTrapForWorkshop,
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
    CrSt_CreatureFired,//[90]
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
    CrSt_CreatureFreezePrisonors,
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
    CrStTyp_Value0 = 0,
    CrStTyp_Value1,
    CrStTyp_Value2,
    CrStTyp_Value3,
    CrStTyp_Value4,
    CrStTyp_Value5,
    CrStTyp_Value6,
};

/** Defines return values of creature state functions. */
enum CreatureStateReturns {
    CrStRet_Deleted       = -1, /**< Returned if the creature being updated no longer exists. */
    CrStRet_Unchanged     =  0, /**< Returned if no change was made to the creature data. */
    CrStRet_Modified      =  1, /**< Returned if the creature was updated and possibly some variables have changed inside, including state. */
    CrStRet_ResetOk       =  2, /**< Returned if the creature state has been reset because task was completed. */
    CrStRet_ResetFail     =  3, /**< Returned if the creature state has been reset, task was either abandoned or couldn't be completed. */
};

typedef short (*CreatureStateFunc1)(struct Thing *);
typedef char (*CreatureStateFunc2)(struct Thing *);
typedef long (*CreatureStateFunc3)(struct Thing *);

struct StateInfo { // sizeof = 41
    CreatureStateFunc1 ofsfield_0;
    CreatureStateFunc1 cleanup_state;
    CreatureStateFunc2 ofsfield_8;
    CreatureStateFunc3 ofsfield_C;
  unsigned char field_10;
  unsigned char field_11;
  unsigned char field_12;
  unsigned char field_13;
  unsigned char field_14;
  unsigned char field_15;
  unsigned char field_16;
  unsigned char field_17;
  unsigned char field_18;
  unsigned char field_19;
  unsigned char field_1A;
  unsigned char field_1B;
  unsigned char field_1C;
  unsigned char field_1D;
  unsigned char state_type;
  unsigned char field_1F;
  unsigned char field_20;
  unsigned short field_21;
  unsigned char field_23;
  unsigned short field_24;
  unsigned char field_26;
  unsigned char field_27;
  unsigned char field_28;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT struct StateInfo _DK_states[];
//#define states _DK_states
extern struct StateInfo states[];
DLLIMPORT long _DK_r_stackpos;
#define r_stackpos _DK_r_stackpos
DLLIMPORT struct DiggerStack _DK_reinforce_stack[];
#define reinforce_stack _DK_reinforce_stack
/******************************************************************************/
TbBool creature_model_bleeds(unsigned long crmodel);
TbBool can_change_from_state_to(const struct Thing *thing, CrtrStateId curr_state, CrtrStateId next_state);
TbBool internal_set_thing_state(struct Thing *thing, CrtrStateId nState);
TbBool external_set_thing_state(struct Thing *thing, CrtrStateId state);
TbBool initialise_thing_state(struct Thing *thing, CrtrStateId nState);
TbBool cleanup_current_thing_state(struct Thing *thing);
struct StateInfo *get_thing_active_state_info(struct Thing *thing);
struct StateInfo *get_thing_continue_state_info(struct Thing *thing);
struct StateInfo *get_thing_state_info_num(CrtrStateId state_id);
struct StateInfo *get_creature_state_with_task_completion(struct Thing *thing);
TbBool state_info_invalid(struct StateInfo *stati);
void create_effect_around_thing(struct Thing *thing, long eff_kind);
long get_creature_state_besides_move(const struct Thing *thing);
long get_creature_state_besides_drag(const struct Thing *thing);
long get_creature_state_type(const struct Thing *thing);
long get_creature_gui_job(const struct Thing *thing);
short set_start_state_f(struct Thing *thing,const char *func_name);
#define set_start_state(thing) set_start_state_f(thing,__func__)

long setup_random_head_for_room(struct Thing *thing, struct Room *room, unsigned char a3);
long setup_head_for_empty_treasure_space(struct Thing *thing, struct Room *room);

TbBool check_experience_upgrade(struct Thing *thing);
void creature_drop_dragged_object(struct Thing *crtng, struct Thing *dragtng);
void creature_drag_object(struct Thing *thing, struct Thing *dragtng);
long process_work_speed_on_work_value(struct Thing *thing, long base_val);
unsigned char find_random_valid_position_for_thing_in_room_avoiding_object(struct Thing *thing, struct Room *room, struct Coord3d *pos);
SubtlCodedCoords find_position_around_in_room(const struct Coord3d *pos, long owner, long rkind);
void remove_health_from_thing_and_display_health(struct Thing *thing, long delta);
long slab_by_players_land(long plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool process_creature_hunger(struct Thing *thing);
TbBool room_initially_valid_as_type_for_thing(const struct Room *room, RoomKind rkind, const struct Thing *thing);
TbBool room_still_valid_as_type_for_thing(const struct Room *room, RoomKind rkind, const struct Thing *thing);
TbBool creature_choose_random_destination_on_valid_adjacent_slab(struct Thing *thing);
struct Room *find_nearest_room_for_thing_excluding_two_types(struct Thing *thing, char owner, char a3, char a4, unsigned char a5);
void place_thing_in_creature_controlled_limbo(struct Thing *thing);
void remove_thing_from_creature_controlled_limbo(struct Thing *thing);
TbBool anger_make_creature_angry(struct Thing *thing, long reason);
TbBool person_get_somewhere_adjacent_in_room(const struct Thing *thing, const struct Room *room, struct Coord3d *pos);
TbBool get_random_position_in_dungeon_for_creature(long plyr_idx, unsigned char a2, struct Thing *thing, struct Coord3d *pos);
/******************************************************************************/
TbBool creature_is_doing_lair_activity(const struct Thing *thing);
TbBool creature_is_being_dropped(const struct Thing *thing);
TbBool creature_is_being_tortured(const struct Thing *thing);
TbBool creature_is_being_sacrificed(const struct Thing *thing);
TbBool creature_is_kept_in_prison(const struct Thing *thing);
TbBool creature_is_being_summoned(const struct Thing *thing);
TbBool creature_is_sleeping(const struct Thing *thing);
TbBool creature_is_doing_dungeon_improvements(const struct Thing *thing);
TbBool creature_is_doing_garden_activity(const struct Thing *thing);
TbBool creature_is_taking_salary_activity(const struct Thing *thing);
TbBool creature_is_doing_temple_activity(const struct Thing *thing);
TbBool creature_is_training(const struct Thing *thing);
TbBool creature_is_scavengering(const struct Thing *thing);
TbBool creature_is_escaping_death(const struct Thing *thing);
TbBool creature_is_kept_in_custody(const struct Thing *thing);
TbBool creature_state_is_unset(const struct Thing *thing);
TbBool remove_creature_from_work_room(struct Thing *thing);
TbBool creature_will_attack_creature(const struct Thing *tng1, const struct Thing *tng2);
TbBool anger_is_creature_livid(const struct Thing *thing);
TbBool anger_is_creature_angry(const struct Thing *thing);
void anger_set_creature_anger(struct Thing *thing, long annoy_lv, long reason);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
