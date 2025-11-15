/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file redundant_packets.h
 *     Header file for redundant_packets.c.
 * @par Purpose:
 *     Redundant packet bundling for network game packet loss prevention.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     11 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_REDUNDANT_PACKETS_H
#define DK_REDUNDANT_PACKETS_H

#include "globals.h"
#include "bflib_basics.h"
#include "packets.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#define REDUNDANT_PACKET_COUNT 3

struct BundledPacket {
    unsigned char valid_count;
    struct Packet packets[REDUNDANT_PACKET_COUNT];
};

#pragma pack()
/******************************************************************************/
void initialize_redundant_packets(void);
void clear_redundant_packets(void);
void store_sent_packet(PlayerNumber player, const struct Packet* packet);
size_t bundle_packets(PlayerNumber player, const struct Packet* current_packet, char* out_buffer);
void unbundle_packets(const char* bundled_buffer, PlayerNumber source_player);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
