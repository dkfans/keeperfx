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
 * @date     11 Mar 2010 - 09 Oct 2010
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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;

#pragma pack()
/******************************************************************************/
void resync_game(void);
CoroutineLoopState perform_checksum_verification(CoroutineLoop *con);
void compute_multiplayer_checksum_sync(void);
TbBigChecksum compute_players_checksum(void);
void player_packet_checksum_add(PlayerNumber plyr_idx, TbBigChecksum sum, const char *area_name);
short checksums_different(void);
TbBigChecksum get_thing_checksum(const struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
