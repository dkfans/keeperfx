/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file desync_analysis.h
 *     Header file for desync_analysis.c.
 * @par Purpose:
 *     Desync analysis history tracking for network game debugging.
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
#ifndef DK_DESYNC_ANALYSIS_H
#define DK_DESYNC_ANALYSIS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
void initialize_desync_analysis(void);
void clear_desync_analysis(void);
void store_turn_checksums(void);
void pack_desync_history_for_resync(void);
void compare_desync_history_from_host(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
