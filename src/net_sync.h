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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)


#pragma pack()
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
void perform_checksum_verification(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
