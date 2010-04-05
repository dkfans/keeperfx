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

#define CREATURE_STATES_COUNT 145

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;

enum CreatureStates {
    CrSt_Unused = 0,
    CrSt_ImpDoingNothing,
    CrSt_ImpArrivesAtDigOrMine1,
    CrSt_ImpArrivesAtDigOrMine2,
    CrSt_ImpDigsMines1,
    CrSt_ImpDigsMines2,
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
    CrSt_Null38,
    CrSt_ArriveAtCallToArms,
    CrSt_CreatureArrivedAtPrison,//[40]
    CrSt_CreatureInPrison,
    CrSt_AtTortureRoom,
    CrSt_Torturing,
    CrSt_AtWorkshopRoom,
    CrSt_Manufacturing,
    CrSt_AtScavengerRoom,
    CrSt_Scavengering,
    CrSt_CreatureDormant,
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
    CrSt_Null,//[60]
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
    CrSt_CreatureHeroEntering,
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
    CrSt_Null122,
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
    CrSt_CreatureLeavesOrDies,
    CrSt_CreatureMoan,//[140]
    CrSt_CreatureSetWorkRoomBasedOnPosition,
    CrSt_CreatureBeingScavenged,
    CrSt_CreatureEscapingDeath,
    CrSt_CreaturePresentToDungeonHeart,
};

struct StateInfo { // sizeof = 41
  short (*ofsfield_0)(struct Thing *);
  short (*ofsfield_4)(struct Thing *);
  char (*ofsfield_8)(struct Thing *);
  long (*ofsfield_C)(struct Thing *);
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

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT struct StateInfo _DK_states[];
//#define states _DK_states
extern struct StateInfo states[];
DLLIMPORT long _DK_r_stackpos;
#define r_stackpos _DK_r_stackpos
DLLIMPORT struct ImpStack _DK_reinforce_stack[];
#define reinforce_stack _DK_reinforce_stack
/******************************************************************************/
long check_out_imp_last_did(struct Thing *thing);
/******************************************************************************/
TbBool creature_model_bleeds(unsigned long model);
TbBool can_change_from_state_to(struct Thing *thing, long curr_state, long next_state);
TbBool internal_set_thing_state(struct Thing *thing, long nState);
TbBool initialise_thing_state(struct Thing *thing, long nState);
TbBool cleanup_current_thing_state(struct Thing *thing);
struct StateInfo *get_thing_state7_info(struct Thing *thing);
struct StateInfo *get_thing_state8_info(struct Thing *thing);
struct StateInfo *get_thing_state_info_num(long state_id);
struct StateInfo *get_creature_state_with_task_completion(struct Thing *thing);
TbBool state_info_invalid(struct StateInfo *stati);
void create_effect_around_thing(struct Thing *thing, long eff_kind);
long get_creature_state_type(const struct Thing *thing);
long get_creature_gui_job(const struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
