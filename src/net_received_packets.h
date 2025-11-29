/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file received_packets.h
 *     Header file for received_packets.c.
 * @par Purpose:
 *     Received packets list tracking for network game synchronization.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     24 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_RECEIVED_PACKETS_H
#define DK_RECEIVED_PACKETS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Packet;

#pragma pack()
/******************************************************************************/
void initialize_packet_tracking(void);
void clear_packet_tracking(void);
void store_received_packets(void);
void store_received_packet(GameTurn turn, PlayerNumber player, const struct Packet* packet);
const struct Packet* get_received_packets_for_turn(GameTurn turn);
const struct Packet* get_received_packet_for_player(GameTurn turn, PlayerNumber player);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
