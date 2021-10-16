/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_soundmsgs.h
 *     Header file for gui_soundmsgs.c.
 * @par Purpose:
 *     Allows to play sound messages during the game.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     29 Jun 2010 - 11 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_SOUNDMSGS_H
#define DK_GUI_SOUNDMSGS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#define MESSAGE_QUEUE_COUNT     4

#define MESSAGE_DELAY_ROOM_NEED     500
#define MESSAGE_DELAY_ROOM_SMALL    500
#define MESSAGE_DELAY_WORSHOP_FULL 1750
#define MESSAGE_DELAY_TREASURY      500
#define MESSAGE_DELAY_FIGHT         400
#define MESSAGE_DELAY_BATTLE         40
#define MESSAGE_DELAY_CRTR_MOOD     500
#define MESSAGE_DELAY_SPECIAL       100
#define MESSAGE_DELAY_LORD          100
#define MESSAGE_DELAY_CRTR_JOINED   500
#define MESSAGE_DELAY_STARVING      500

enum TbSpeechMessages {
        SMsg_None = 0,
        SMsg_CreatrAngryAnyReason,
        SMsg_CreatrAngryNoLair,
        SMsg_CreatrAngryNotPaid,
        SMsg_CreatrAngryNoFood,
        SMsg_CreatrDestroyRooms,
        SMsg_CreatureLeaving,
        SMsg_WallsBreach,
        SMsg_HeartUnderAttack,
        SMsg_BattleDefeat,
        SMsg_BattleVictory, // 10
        SMsg_BattleDeath,
        SMsg_BattleWon,
        SMsg_CreatureDefending,
        SMsg_CreatureAttacking,
        SMsg_EnemyDestroyRooms,
        SMsg_EnemyClaimGround,
        SMsg_EnemyRoomTakeOver,
        SMsg_NewRoomTakenOver,
        SMsg_LordOfLandComming,
        SMsg_FingthingFriends, // 20
        SMsg_BattleOver,
        SMsg_GardenTooSmall,
        SMsg_LairTooSmall,
        SMsg_TreasuryTooSmall,    //'You need a bigger treasure room'
        SMsg_LibraryTooSmall,
        SMsg_PrisonTooSmall,
        SMsg_TortureTooSmall,
        SMsg_TrainingTooSmall,
        SMsg_WorkshopTooSmall,
        SMsg_ScavengeTooSmall, // 30
        SMsg_TempleTooSmall,
        SMsg_GraveyardTooSmall,
        SMsg_BarracksTooSmall,
        SMsg_NoRouteToGarden,
        SMsg_NoRouteToTreasury,    //'Some of your minions are unable to reach the treasure room'
        SMsg_NoRouteToLair,
        SMsg_EntranceClaimed,
        SMsg_EntranceLost,
        SMsg_RoomTreasrNeeded,    //'You must build a treasure room, to store gold'
        SMsg_RoomLairNeeded, // 40
        SMsg_RoomGardenNeeded,
        SMsg_ResearchedRoom,
        SMsg_ResearchedSpell,
        SMsg_ManufacturedDoor,
        SMsg_ManufacturedTrap,
        SMsg_NoMoreReseach,
        SMsg_SpellbookTaken,
        SMsg_TrapTaken,
        SMsg_DoorTaken,
        SMsg_SpellbookStolen, // 50
        SMsg_TrapStolen,
        SMsg_DoorStolen,
        SMsg_TortureInformation,
        SMsg_TortureConverted,
        SMsg_PrisonMadeSkeleton,
        SMsg_TortureMadeGhost,
        SMsg_PrisonersEscaping,
        SMsg_GraveyardMadeVampire,
        SMsg_CreatrFreedPrison,
        SMsg_PrisonersStarving, // 60
        SMsg_CreatureScanvenged,
        SMsg_MinionScanvenged,
        SMsg_CreatureJoinedEnemy,
        SMsg_CreatureRevealInfo,
        SMsg_SacrificeGood,
        SMsg_SacrificeReward,
        SMsg_SacrificeNeutral,
        SMsg_SacrificeBad,
        SMsg_SacrificePunish,
        SMsg_SacrificeWishing, // 70
        SMsg_DiscoveredSpecial,
        SMsg_DiscoveredSpell,
        SMsg_DiscoveredDoor,
        SMsg_DiscoveredTrap,
        SMsg_CreaturesJoinedYou,
        SMsg_DugIntoNewArea,
        SMsg_SpecRevealMap,
        SMsg_SpecResurrect,
        SMsg_SpecTransfer,
        SMsg_CommonAcknowledge, // 80
        SMsg_SpecHeroStolen,
        SMsg_SpecCreatrDoubled,
        SMsg_SpecIncLevel,
        SMsg_SpecWallsFortify,
        SMsg_SpecHiddenWorld,
        SMsg_GoldLow,
        SMsg_GoldNotEnough,     //'You do not have enough gold'
        SMsg_NoGoldToScavenge,
        SMsg_NoGoldToTrain,
        SMsg_Payday, // 90
        SMsg_FullOfPies,
        SMsg_SurrealHappen,
        SMsg_StrangeAccent,
        SMsg_PantsTooTight,    //'Your pants are definitely too tight'
        SMsg_CraveChocolate,
        SMsg_SmellAgain,
        SMsg_Hello,
        SMsg_Glaagh,
        SMsg_Achew,
        SMsg_Chgreche,        // 100
        SMsg_WorkerJobsLimit, //'You cannot give your Imps any more jobs'
        SMsg_GameLoaded,      //'Game loaded'
        SMsg_GameSaved,
        SMsg_DefeatedKeeper,
        SMsg_LevelFailed,     //'You have been defeated'
        SMsg_LevelWon,        //'Your have conquered this realm'
        SMsg_SenceAvatar,     //'I sense the presence of the Avatar'
        SMsg_AvatarBodyVanish,
        SMsg_GameFinalVictory,
/*
        SMsg_TreasrRoomTooSmall =   24,
        SMsg_TreasrUnreachable  =   35,
        SMsg_TreasureRoomNeeded =   39,
        SMsg_NotEnoughGold      =   87,
        SMsg_PantsTooTight      =   94,
        SMsg_DefeatedOnRealm    =  105,
        SMsg_NoMoreWorkerJobs   =  101,
        SMsg_GameLoaded         =  102,
        SMsg_ConqueredRealm     =  106,*/
};
#define SMsg_FunnyMessages      SMsg_FullOfPies  // Starts a list of 10 funny quotes
#define SMsg_EnemyHarassments  110  // Starts a list of harassments
#define SMsg_EnemyLordQuote    118  // Starts a list of lord of the land quotes

// Define empty messages, which may be used later
#define SMsg_NoRouteToPrison SMsg_None
#define SMsg_NoRouteToGraveyard SMsg_None

typedef unsigned long Phrase;

enum OutputMessageKinds {
    OMsg_None = 0,
    OMsg_RoomNeeded,
    OMsg_RoomTooSmall,
    OMsg_RoomNoRoute,
    OMsg_RoomFull,
};

typedef unsigned int OutputMessageKind;

struct SMessage {
      long start_idx;
      long count;
      long end_time;
};

struct MessageQueueEntry { // sizeof = 9
     unsigned char state;
     unsigned long msg_idx;
     long delay;
};

#pragma pack()
/******************************************************************************/
TbBool output_message(long msg_idx, long delay, TbBool queue);
TbBool message_already_in_queue(long msg_idx);
TbBool add_message_to_queue(long msg_idx, long delay);
TbBool message_queue_empty(void);
long get_phrase_for_message(long msg_idx);
long get_phrase_sample(long phr_idx);
TbBool message_can_be_played(long msg_idx);
void clear_messages(void);
void init_messages_turns(long delay);
void init_messages(void);
void process_messages(void);
TbBool output_message_room_related_from_computer_or_player_action(PlayerNumber plyr_idx, RoomKind rkind, OutputMessageKind msg_kind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
