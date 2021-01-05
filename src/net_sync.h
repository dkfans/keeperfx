/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_sync.h
 *     Header file for net_sync.c.
 * @par Purpose:
 *     Network game synchronization for Dungeon Keeper.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 30 Aug 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NETSYNC_H
#define DK_NETSYNC_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_coroutine.h"

#include "creature_control.h"
#include "player_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

struct ChecksumStorage
{
    unsigned long checksum_creatures[CREATURES_COUNT];
};

struct ChecksumContext
{
    int sent;
    struct PacketEx* base;
    unsigned int checked_players; // bitmask of each player
    unsigned int answers_mask;

    TbBool different;
    TbChecksum checksum;
};
/******************************************************************************/
/**
 * This is a polling function
 * When desync detected after synced state first time first_resync is set to true 
 * returns true when resync is complete
 */
TbBool resync_game(TbBool first_resync);
/*
    Validate checksum of all players
*/
TbBool perform_checksum_verification(CoroutineLoop *context);

void resync_reset_storage();

const char *get_desync_info();

void net_force_sync(unsigned long expected_turn);
TbBool net_sync_process_force_packet(unsigned long turn, int plyr_idx, unsigned char kind, void *data, short size);

TbBool check_resync_turn();

typedef void (*EmptyLambda)();
EmptyLambda net_resync_needed_f(const char *fn_name);
#define net_resync_needed net_resync_needed_f(__func__)
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
