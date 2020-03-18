/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_soundmsgs.c
 *     Allows to play sound messages during the game.
 * @par Purpose:
 *     Functions to manage queue of speeches to be played.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Jun 2010 - 11 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_soundmsgs.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_sound.h"
#include "bflib_math.h"

#include "config_terrain.h"
#include "game_merge.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct MessageQueueEntry _DK_message_queue[MESSAGE_QUEUE_COUNT];
#define message_queue _DK_message_queue
DLLIMPORT unsigned long _DK_message_playing;
#define message_playing _DK_message_playing
DLLIMPORT struct SMessage _DK_messages[126];
/******************************************************************************/
enum SpeechPhraseIndex {
    SpchIdx_Invalid = 0,
    SpchIdx_CreatureAngryAnyReson,
    SpchIdx_CreatureAngryNoLair,
    SpchIdx_CreatureAngryNotPayed,
    SpchIdx_CreatureAngryNoFood,
    SpchIdx_CreatureDestroyingRooms,
    SpchIdx_CreatureLeaving,
    SpchIdx_WallsBreach,
    SpchIdx_HeartUnderAttack,
    SpchIdx_BattleDefeat,
    SpchIdx_BattleVictory, // 10
    SpchIdx_BattleDeath,
    SpchIdx_BattleWon,
    SpchIdx_CreatureDefending,
    SpchIdx_CreatureAttacking,
    SpchIdx_EnemyDestroyingRooms,
    SpchIdx_EnemyClaimingGround,
    SpchIdx_EnemyRoomTakenOver,
    SpchIdx_NewRoomTakenOver,
    SpchIdx_LordOfTheLandComming,
    SpchIdx_FingthingFriends, // 20
    SpchIdx_BattleOver,
    SpchIdx_GardenTooSmall,
    SpchIdx_LairTooSmall,
    SpchIdx_TreasuryTooSmall,
    SpchIdx_LibraryTooSmall,
    SpchIdx_PrisonTooSmall,
    SpchIdx_TortureTooSmall,
    SpchIdx_TrainingTooSmall,
    SpchIdx_WorkshopTooSmall,
    SpchIdx_ScavengeTooSmall, // 30
    SpchIdx_TempleTooSmall,
    SpchIdx_GraveyardTooSmall,
    SpchIdx_BarracksTooSmall,
    SpchIdx_NoRouteToGarden,
    SpchIdx_NoRouteToTreasury,
    SpchIdx_NoRouteToLair,
    SpchIdx_EntranceClaimed,
    SpchIdx_EntranceLost,
    SpchIdx_RoomTreasureNeeded,
    SpchIdx_RoomLairNeeded, // 40
    SpchIdx_RoomGardenNeeded,
    SpchIdx_ResearchedRoom,
    SpchIdx_ResearchedSpell,
    SpchIdx_ManufacturedDoor,
    SpchIdx_ManufacturedTrap,
    SpchIdx_NoMoreReseach,
    SpchIdx_SpellbookTaken,
    SpchIdx_TrapTaken,
    SpchIdx_DoorTaken,
    SpchIdx_SpellbookStolen, // 50
    SpchIdx_TrapStolen,
    SpchIdx_DoorStolen,
    SpchIdx_TortureInformation,
    SpchIdx_TortureConverted,
    SpchIdx_PrisonerMadeSkeleton,
    SpchIdx_TortureMadeGhost,
    SpchIdx_PrisonersEscaping,
    SpchIdx_GraveyardMadeVampire,
    SpchIdx_CreaturesFreedFromPrison,
    SpchIdx_PrisonersStarving, // 60
    SpchIdx_CreatureScanvenged,
    SpchIdx_MinionScanvenged,
    SpchIdx_CreatureJoinedEnemy,
    SpchIdx_CreatureRevealedInfo,
    SpchIdx_SacrificeGood,
    SpchIdx_SacrificeReward,
    SpchIdx_SacrificeNeutral,
    SpchIdx_SacrificeBad,
    SpchIdx_SacrificePunish,
    SpchIdx_SacrificeWishing, // 70
    SpchIdx_DiscoveredSpecial,
    SpchIdx_DiscoveredSpell,
    SpchIdx_DiscoveredDoor,
    SpchIdx_DiscoveredTrap,
    SpchIdx_CreaturesJoinedYou,
    SpchIdx_DugIntoNewArea,
    SpchIdx_SpecialRevealMap,
    SpchIdx_SpecialResurrect,
    SpchIdx_SpecialTransfer,
    SpchIdx_CommonAcknowledge, // 80
    SpchIdx_SpecialHeroStolen,
    SpchIdx_SpecialCreaturesDoubled,
    SpchIdx_SpecialIncreasedLevel,
    SpchIdx_SpecialWallsFortified,
    SpchIdx_SpecialHiddenWorld,
    SpchIdx_GoldLow,
    SpchIdx_GoldNotEnough,
    SpchIdx_NoGoldToScavenge,
    SpchIdx_NoGoldToTrain,
    SpchIdx_Payday, // 90
    SpchIdx_FullOfPies,
    SpchIdx_SurrealHappen,
    SpchIdx_StrangeAccent,
    SpchIdx_PantsTooTight,
    SpchIdx_CraveChocolate,
    SpchIdx_SmellAgain,
    SpchIdx_Hello,
    SpchIdx_Glaagh,
    SpchIdx_Achew,
    SpchIdx_Chgreche, // 100
    SpchIdx_ImpJobsLimit,
    SpchIdx_GameLoaded,
    SpchIdx_GameSaved,
    SpchIdx_DefeatedKeeper,
    SpchIdx_LevelFailed, // 105
    SpchIdx_LevelWon,
    SpchIdx_SenceAvatar,
    SpchIdx_AvatarBodyVanished,
    SpchIdx_GameFinalVictory,
    SpchIdx_KeeperHarassment1,
    SpchIdx_KeeperHarassment2,
    SpchIdx_KeeperHarassment3,
    SpchIdx_KeeperHarassment4,
    SpchIdx_KeeperHarassment5,
    SpchIdx_KeeperHarassment6,
    SpchIdx_KeeperHarassment7,
    SpchIdx_KeeperHarassment8,
    SpchIdx_HeroHarassment1,
    SpchIdx_HeroHarassment2,
    SpchIdx_HeroHarassment3,
    SpchIdx_HeroHarassment4,
    SpchIdx_HeroHarassment5,
    SpchIdx_HeroHarassment6,
    SpchIdx_HeroHarassment7,
    SpchIdx_HeroHarassment8,
};

/** Array used for converting speech phrase index into sample data. */
Phrase phrases[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8, 9,  10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
   64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
   96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,
};

struct SMessage messages[] = {
  {SpchIdx_Invalid, 0, 0}, // [0] SMsg_None
  {SpchIdx_CreatureAngryAnyReson, 1, 0},
  {SpchIdx_CreatureAngryNoLair, 1, 0},
  {SpchIdx_CreatureAngryNotPayed, 1, 0},
  {SpchIdx_CreatureAngryNoFood, 1, 0},
  {SpchIdx_CreatureDestroyingRooms, 1, 0}, // [5]
  {SpchIdx_CreatureLeaving, 1, 0},
  {SpchIdx_WallsBreach, 1, 0},
  {SpchIdx_HeartUnderAttack, 1, 0},
  {SpchIdx_BattleDefeat, 1, 0},
  {SpchIdx_BattleVictory, 1, 0}, // [10] SMsg_BattleVictory
  {SpchIdx_BattleDeath, 1, 0},
  {SpchIdx_BattleWon, 1, 0},
  {SpchIdx_CreatureDefending, 1, 0},
  {SpchIdx_CreatureAttacking, 1, 0},
  {SpchIdx_EnemyDestroyingRooms, 1, 0}, // [15]
  {SpchIdx_EnemyClaimingGround, 1, 0},
  {SpchIdx_EnemyRoomTakenOver, 1, 0},
  {SpchIdx_NewRoomTakenOver, 1, 0},
  {SpchIdx_LordOfTheLandComming, 1, 0},
  {SpchIdx_FingthingFriends, 1, 0}, // [20] SMsg_FingthingFriends
  {SpchIdx_BattleOver, 1, 0},
  {SpchIdx_GardenTooSmall, 1, 0},
  {SpchIdx_LairTooSmall, 1, 0},
  {SpchIdx_TreasuryTooSmall, 1, 0},
  {SpchIdx_LibraryTooSmall, 1, 0},
  {SpchIdx_PrisonTooSmall, 1, 0},
  {SpchIdx_TortureTooSmall, 1, 0},
  {SpchIdx_TrainingTooSmall, 1, 0},
  {SpchIdx_WorkshopTooSmall, 1, 0},
  {SpchIdx_ScavengeTooSmall, 1, 0}, // [30] SMsg_ScavengeTooSmall
  {SpchIdx_TempleTooSmall, 1, 0},
  {SpchIdx_GraveyardTooSmall, 1, 0},
  {SpchIdx_BarracksTooSmall, 1, 0},
  {SpchIdx_NoRouteToGarden, 1, 0},
  {SpchIdx_NoRouteToTreasury, 1, 0},
  {SpchIdx_NoRouteToLair, 1, 0},
  {SpchIdx_EntranceClaimed, 1, 0},
  {SpchIdx_EntranceLost, 1, 0},
  {SpchIdx_RoomTreasureNeeded, 1, 0},
  {SpchIdx_RoomLairNeeded, 1, 0}, // [40] SMsg_RoomLairNeeded
  {SpchIdx_RoomGardenNeeded, 1, 0},
  {SpchIdx_ResearchedRoom, 1, 0},
  {SpchIdx_ResearchedSpell, 1, 0},
  {SpchIdx_ManufacturedDoor, 1, 0},
  {SpchIdx_ManufacturedTrap, 1, 0},
  {SpchIdx_NoMoreReseach, 1, 0},
  {SpchIdx_SpellbookTaken, 1, 0},
  {SpchIdx_TrapTaken, 1, 0},
  {SpchIdx_DoorTaken, 1, 0},
  {SpchIdx_SpellbookStolen, 1, 0}, // [50] SMsg_SpellbookStolen
  {SpchIdx_TrapStolen, 1, 0},
  {SpchIdx_DoorStolen, 1, 0},
  {SpchIdx_TortureInformation, 1, 0},
  {SpchIdx_TortureConverted, 1, 0},
  {SpchIdx_PrisonerMadeSkeleton, 1, 0},
  {SpchIdx_TortureMadeGhost, 1, 0},
  {SpchIdx_PrisonersEscaping, 1, 0},
  {SpchIdx_GraveyardMadeVampire, 1, 0},
  {SpchIdx_CreaturesFreedFromPrison, 1, 0},
  {SpchIdx_PrisonersStarving, 1, 0}, // [60] SMsg_PrisonersStarving
  {SpchIdx_CreatureScanvenged, 1, 0},
  {SpchIdx_MinionScanvenged, 1, 0},
  {SpchIdx_CreatureJoinedEnemy, 1, 0},
  {SpchIdx_CreatureRevealedInfo, 1, 0},
  {SpchIdx_SacrificeGood, 1, 0},
  {SpchIdx_SacrificeReward, 1, 0},
  {SpchIdx_SacrificeNeutral, 1, 0},
  {SpchIdx_SacrificeBad, 1, 0},
  {SpchIdx_SacrificePunish, 1, 0},
  {SpchIdx_SacrificeWishing, 1, 0}, // [70] SMsg_SacrificeWishing
  {SpchIdx_DiscoveredSpecial, 1, 0},
  {SpchIdx_DiscoveredSpell, 1, 0},
  {SpchIdx_DiscoveredDoor, 1, 0},
  {SpchIdx_DiscoveredTrap, 1, 0},
  {SpchIdx_CreaturesJoinedYou, 1, 0},
  {SpchIdx_DugIntoNewArea, 1, 0},
  {SpchIdx_SpecialRevealMap, 1, 0},
  {SpchIdx_SpecialResurrect, 1, 0},
  {SpchIdx_SpecialTransfer, 1, 0},
  {SpchIdx_CommonAcknowledge, 1, 0}, // [80]
  {SpchIdx_SpecialHeroStolen, 1, 0},
  {SpchIdx_SpecialCreaturesDoubled, 1, 0},
  {SpchIdx_SpecialIncreasedLevel, 1, 0},
  {SpchIdx_SpecialWallsFortified, 1, 0},
  {SpchIdx_SpecialHiddenWorld, 1, 0},
  {SpchIdx_GoldLow, 1, 0},
  {SpchIdx_GoldNotEnough, 1, 0},
  {SpchIdx_NoGoldToScavenge, 1, 0},
  {SpchIdx_NoGoldToTrain, 1, 0},
  {SpchIdx_Payday, 1, 0}, // [90]
  {SpchIdx_FullOfPies, 1, 0},
  {SpchIdx_SurrealHappen, 1, 0},
  {SpchIdx_StrangeAccent, 1, 0},
  {SpchIdx_PantsTooTight, 1, 0},
  {SpchIdx_CraveChocolate, 1, 0},
  {SpchIdx_SmellAgain, 1, 0},
  {SpchIdx_Hello, 1, 0},
  {SpchIdx_Glaagh, 1, 0},
  {SpchIdx_Achew, 1, 0},
  {SpchIdx_Chgreche, 1, 0}, // [100]
  {SpchIdx_ImpJobsLimit, 1, 0},
  {SpchIdx_GameLoaded, 1, 0},
  {SpchIdx_GameSaved, 1, 0},
  {SpchIdx_DefeatedKeeper, 1, 0},
  {SpchIdx_LevelFailed, 1, 0},
  {SpchIdx_LevelWon, 1, 0},
  {SpchIdx_SenceAvatar, 1, 0},
  {SpchIdx_AvatarBodyVanished, 1, 0},
  {SpchIdx_GameFinalVictory, 1, 0},
  {SpchIdx_KeeperHarassment1, 1, 0}, // [110]
  {SpchIdx_KeeperHarassment2, 1, 0},
  {SpchIdx_KeeperHarassment3, 1, 0},
  {SpchIdx_KeeperHarassment4, 1, 0},
  {SpchIdx_KeeperHarassment5, 1, 0},
  {SpchIdx_KeeperHarassment6, 1, 0},
  {SpchIdx_KeeperHarassment7, 1, 0},
  {SpchIdx_KeeperHarassment8, 1, 0},
  {SpchIdx_HeroHarassment1, 1, 0},
  {SpchIdx_HeroHarassment2, 1, 0},
  {SpchIdx_HeroHarassment3, 1, 0}, // [120]
  {SpchIdx_HeroHarassment4, 1, 0},
  {SpchIdx_HeroHarassment5, 1, 0},
  {SpchIdx_HeroHarassment6, 1, 0},
  {SpchIdx_HeroHarassment7, 1, 0},
  {SpchIdx_HeroHarassment8, 1, 0},
};

/******************************************************************************/
/**
 * Plays an in-game message.
 *
 * @param msg_idx Message index
 * @param delay Delay between the message can be repeated.
 * @param queue True will allow the message to queue,
 *     false will play it immediately or never.
 * @return Gives true if the message was prepared to play or queued.
 *     If the message couldn't be played nor queued, returns false.
 */
TbBool output_message(long msg_idx, long delay, TbBool queue)
{
    SYNCDBG(5,"Message %ld, delay %ld, queue %s",msg_idx, delay, queue?"on":"off");
    struct SMessage* smsg = &messages[msg_idx];
    if (!message_can_be_played(msg_idx))
    {
        SYNCDBG(8,"Delay to turn %ld didn't passed, skipping",(long)smsg->end_time);
        return false;
    }
    if (!speech_sample_playing())
    {
        long i = get_phrase_sample(get_phrase_for_message(msg_idx));
        if (i == 0)
        {
            SYNCDBG(8, "No phrase %d sample, skipping", (int)msg_idx);
            return false;
      }
      if (play_speech_sample(i))
      {
          message_playing = msg_idx;
          smsg->end_time = (long)game.play_gameturn + delay;
          SYNCDBG(8,"Playing prepared");
          return true;
      }
    }
    if ( (msg_idx == message_playing) || (message_already_in_queue(msg_idx)) )
    {
        SYNCDBG(8,"Message %ld is already in queue",msg_idx);
        return false;
    }
    if (queue)
    {
      if (add_message_to_queue(msg_idx, delay))
      {
          SYNCDBG(8,"Playing queued");
          return true;
      }
    }
    WARNDBG(8,"Playing message %ld failed",msg_idx);
    return false;
}

TbBool remove_message_from_queue(long queue_idx)
{
    SYNCDBG(7,"Starting");
    long i = queue_idx;
    struct MessageQueueEntry* mqentry = &message_queue[i];
    // If there's nothing to remove, don't bother trying
    if (mqentry->state != 1)
    {
        return false;
    }
    for (i++; i < MESSAGE_QUEUE_COUNT; i++)
    {
        struct MessageQueueEntry* mqprev = mqentry;
        mqentry = &message_queue[i];
        mqprev->state = mqentry->state;
        mqprev->msg_idx = mqentry->msg_idx;
        mqprev->delay = mqentry->delay;
    }
    // Now clear the highest entry
    {
        mqentry->state = 0;
        mqentry->msg_idx = 0;
        mqentry->delay = 0;
    }
    return false;
}

void process_messages(void)
{
    SYNCDBG(17,"Starting");
    // If already playing, just wait for next time
    if (!speech_sample_playing())
    {
        SYNCDBG(17,"play on");
        message_playing = 0;
        // If no messages are in queue, don't play anything
        if (message_queue_empty())
        {
            SYNCDBG(19,"Finished");
            return;
        }
        // Otherwise remove next message from queue and try to play it
        unsigned long msg_idx = message_queue[0].msg_idx;
        long delay = message_queue[0].delay;
        remove_message_from_queue(0);
        output_message(msg_idx, delay, true);
    }
    SYNCDBG(19,"Finished");
}

TbBool message_can_be_played(long msg_idx)
{
    if ( (msg_idx < 0) || (msg_idx >= sizeof(messages)/sizeof(messages[0])) )
        return false;
    return ((long)game.play_gameturn >= messages[msg_idx].end_time);
}

TbBool message_already_in_queue(long msg_idx)
{
    for (long i = 0; i < MESSAGE_QUEUE_COUNT; i++)
    {
        struct MessageQueueEntry* mqentry = &message_queue[i];
        if ((mqentry->state == 1) && (msg_idx == mqentry->msg_idx))
            return true;
  }
  return false;
}

TbBool message_queue_empty(void)
{
    return (!message_queue[0].state);
}

TbBool add_message_to_queue(long msg_idx, long delay)
{
    for (long i = 0; i < MESSAGE_QUEUE_COUNT; i++)
    {
        struct MessageQueueEntry* mqentry = &message_queue[i];
        if (mqentry->state == 0)
        {
          mqentry->state = 1;
          mqentry->msg_idx = msg_idx;
          mqentry->delay = delay;
          return true;
        }
    }
    return false;
}

/** Returns a random speech phrase for given message */
long get_phrase_for_message(long msg_idx)
{
    struct SMessage* smsg = &messages[msg_idx];
    if (smsg->count <= 0)
        return -1;
    long i = UNSYNC_RANDOM(smsg->count);
    return smsg->start_idx + i;
}

/** Returns speech sample for given phrase index.
 *
 * @param phr_idx
 * @return The speech sample number; on error, trturns 0;
 */
long get_phrase_sample(long phr_idx)
{
    if ((phr_idx < 0) || (phr_idx >= sizeof(phrases)/sizeof(phrases[0])))
        phr_idx = 0;
    return phrases[phr_idx];
}

void clear_messages(void)
{
    int i;
    for (i=0; i < MESSAGE_QUEUE_COUNT; i++)
    {
        LbMemorySet(&message_queue[i], 0, sizeof(struct MessageQueueEntry));
    }
    // Set end turn to 0 for all messages
    for (i=0; i < sizeof(messages)/sizeof(messages[0]); i++)
    {
        messages[i].end_time = 0;
    }
    // Remove when won't be needed anymore
    for (i=0; i < sizeof(_DK_messages)/sizeof(_DK_messages[0]); i++)
    {
        _DK_messages[i].end_time = 0;
    }
}

void init_messages_turns(long delay)
{
    // Set end turn for all messages
    for (int i = 0; i < sizeof(messages) / sizeof(messages[0]); i++)
    {
        struct SMessage* smsg = &messages[i];
        smsg->end_time = game.play_gameturn + delay;
    }
}

TbBool output_message_room_related_from_computer_or_player_action(PlayerNumber plyr_idx, RoomKind rkind, OutputMessageKind msg_kind)
{
    if (!is_my_player_number(plyr_idx)) {
        return false;
    }
    const struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    long delay;
    long msg_idx;
    switch (msg_kind)
    {
    case OMsg_RoomNeeded:
        msg_idx = roomst->msg_needed;
        delay = MESSAGE_DELAY_ROOM_NEED;
        break;
    case OMsg_RoomTooSmall:
        msg_idx = roomst->msg_too_small;
        delay = MESSAGE_DELAY_ROOM_SMALL;
        break;
    case OMsg_RoomFull:
        msg_idx = roomst->msg_too_small;
        delay = MESSAGE_DELAY_WORSHOP_FULL;
        break;
    case OMsg_RoomNoRoute:
        msg_idx = roomst->msg_no_route;
        delay = MESSAGE_DELAY_ROOM_NEED;
        break;
    default:
        msg_idx = 0;
        delay = 0;
        break;
    }
    if (msg_idx < 1) {
        return false;
    }
    return output_message(msg_idx, delay, true);
}

void init_messages(void)
{
    clear_messages();
    // Set end turn
    init_messages_turns(0);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
