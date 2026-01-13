/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_resync.h
 *     Header file for net_resync.cpp.
 * @par Purpose:
 *     Network resynchronization for Dungeon Keeper multiplayer.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     31 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_RESYNC_H
#define DK_NET_RESYNC_H

#include "bflib_basics.h"
#include "bflib_coroutine.h"

#ifdef __cplusplus
extern "C" {
#endif

TbBool LbNetwork_Resync(void *data_buffer, size_t buffer_length);
void LbNetwork_TimesyncBarrier(void);
void animate_resync_progress_bar(int current_phase, int total_phases);
void resync_game(void);

#ifdef __cplusplus
}
#endif

#endif
