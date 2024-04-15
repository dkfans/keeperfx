/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_msgs.h
 *     Header file for gui_msgs.c.
 * @par Purpose:
 *     Game GUI Messages functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     14 May 2010 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_MSGS_H
#define DK_GUI_MSGS_H

#include "globals.h"
#include "bflib_basics.h"

#define GUI_MESSAGES_COUNT      7
#define GUI_MESSAGES_DELAY      400

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
enum MessageTypes {
    MsgType_Player = 0,
    MsgType_Creature,
    MsgType_CreatureSpell,
    MsgType_Room,
    MsgType_KeeperSpell,
    MsgType_Query, //5
    MsgType_Blank,
    MsgType_CreatureInstance,
};
/******************************************************************************/
#pragma pack(1)

struct GuiMessage_OLD { // sizeof = 0x45 (69)
    char text[64];
PlayerNumber plyr_idx;
unsigned long creation_turn;
};

struct GuiMessage {
    char text[64];
PlayerNumber plyr_idx;
unsigned long creation_turn;
PlayerNumber target_idx;
char type;
};

#pragma pack()
/******************************************************************************/
void message_update(void);
void message_draw(void);
void zero_messages(void);
void message_add(char type, PlayerNumber plyr_idx, const char *text);
void message_add_fmt(char type, PlayerNumber plyr_idx, const char *fmt_str, ...);
void show_game_time_taken(unsigned long fps, unsigned long turns);
void show_real_time_taken(void);
void clear_messages_from_player(char type, PlayerNumber plyr_idx);
void delete_message(unsigned char msg_idx);
void targeted_message_add(char type, PlayerNumber plyr_idx, PlayerNumber target_idx, unsigned long timeout, const char *fmt_str, ...);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
/******************************************************************************/
