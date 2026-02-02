/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_checksums.h
 *     Header file for net_checksums.c.
 * @par Purpose:
 *     Network checksum computation and desync analysis for multiplayer games.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     03 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_CHECKSUMS_H
#define DK_NET_CHECKSUMS_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_coroutine.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct PlayerInfo;
struct Thing;

void update_turn_checksums(void);
void pack_desync_history_for_resync(void);
void compare_desync_history_from_host(void);
TbBigChecksum get_thing_checksum(const struct Thing *thing);
short checksums_different(void);
CoroutineLoopState perform_checksum_verification(CoroutineLoop *con);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
