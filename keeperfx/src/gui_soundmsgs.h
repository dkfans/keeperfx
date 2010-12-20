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
#ifdef __cplusplus
#pragma pack(1)
#endif

#define MESSAGE_QUEUE_COUNT     4

enum TbSpeechMessages {
        SMsg_None               =    0,
        SMsg_TreasrRoomTooSmall =   24,  //'You need a bigger treasure room'
        SMsg_TreasrUnreachable  =   35,  //'Some of your minions are unable to reach the treasure room'
        SMsg_TreasureRoomNeeded =   39,  //'You must build a treasure room, to store gold'
        SMsg_NotEnoughGold      =   87,  //'You do not have enough gold'
        SMsg_PantsTooTight      =   94,  //'Your pants are definitely too tight'
        SMsg_DefeatedOnRealm    =  105,  //'You have been defeated'
        SMsg_NoMoreWorkerJobs   =  101,  //'You cannot give your Imps any more jobs'
        SMsg_GameLoaded         =  102,  //'Game loaded'
        SMsg_ConqueredRealm     =  106,  //'Your have conquered this realm'
};
#define SMsg_FunnyMessages      91  // Starts a list of 10 funny quotes
#define SMsg_EnemyHarassments  110  // Starts a list of harassments

typedef unsigned long Phrase;

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

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
TbBool output_message(long msg_idx, long delay, TbBool queue);
TbBool message_already_in_queue(long msg_idx);
TbBool add_message_to_queue(long msg_idx, long delay);
long get_phrase_for_message(long msg_idx);
long get_phrase_sample(long phr_idx);
TbBool message_can_be_played(long msg_idx);
void clear_messages(void);
void init_messages_turns(long delay);
void process_messages(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
