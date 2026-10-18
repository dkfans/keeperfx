/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_saves.h
 *     Header file for game_saves.c.
 * @par Purpose:
 *     The campaign rules that go with saved games: which levels are completed, where a campaign continues,
 *     and the creatures carried to the next level. Writing and reading the files is the SaveManager's.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 Jan 2009 - 25 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GAMESAVE_H
#define DK_GAMESAVE_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct GameCampaign;
struct IntralevelData;
/******************************************************************************/
TbBool add_transfered_creature(PlayerNumber plyr_idx, ThingModel model, CrtrExpLevel exp_level, char *name);
void clear_transfered_creatures(void);
/******************************************************************************/
LevelNumber move_campaign_to_next_level(void);
LevelNumber move_campaign_to_prev_level(void);
/******************************************************************************/
TbBool is_campaign_progress_level(const struct GameCampaign *campgn, LevelNumber lvnum);
void mark_level_completed(struct IntralevelData *ilvl, LevelNumber lvnum);
uint32_t encode_continue_level(LevelNumber lvnum);
LevelNumber resolve_campaign_progress(struct IntralevelData *ilvl, const struct GameCampaign *campgn);
short campaign_progress_percent(const struct GameCampaign *campgn, const struct IntralevelData *ilvl, LevelNumber continue_lvnum);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
