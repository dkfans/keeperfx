/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_saves.c
 *     Saved games maintain functions.
 * @par Purpose:
 *     For opening, writing, listing saved games.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 Jan 2009 - 25 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_saves.h"
#include "ariadne_update.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_campaigns.h"
#include "config_settings.h"
#include "dungeon_stats.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_compp.h"
#include "sound_manager.h"
#include "custom_sprites.h"
#include "front_simple.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "front_landview.h"
#include "front_highscore.h"
#include "front_lvlstats.h"
#include "lens_api.h"
#include "local_camera.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "frontmenu_ingame_map.h"
#include "gui_boxmenu.h"
#include "net_exchange_gameplay.h"
#include "packets.h"
#include "player_data.h"
#include "roomspace.h"
#include "keeperfx.hpp"
#include "api.h"
#include "lvl_filesdk1.h"
#include "lua_base.h"
#include "lua_triggers.h"
#include "moonphase.h"
#include "light_data.h"
#include "map_ceiling.h"
#include "map_columns.h"
#include "map_data.h"
#include "thing_creature.h"
#include "post_inc.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
TbBool is_campaign_progress_level(const struct GameCampaign *campgn, LevelNumber lvnum)
{
    return (campaign_singleplayer_level_index(campgn, lvnum) >= 0) || (campaign_bonus_level_index(campgn, lvnum) >= 0)
        || (campaign_extra_level_index(campgn, lvnum) >= 0);
}

static TbBool is_level_completed(const struct IntralevelData *ilvl, LevelNumber lvnum)
{
    if (lvnum < 1)
        return false;
    for (int i = 0; i < LEVELS_COMPLETED_COUNT; i++)
    {
        if (ilvl->levels_completed[i] == (uint32_t)lvnum + 1)
            return true;
    }
    return false;
}

void mark_level_completed(struct IntralevelData *ilvl, LevelNumber lvnum)
{
    if ((lvnum < 1) || is_level_completed(ilvl, lvnum))
        return;
    for (int i = 0; i < LEVELS_COMPLETED_COUNT; i++)
    {
        if (ilvl->levels_completed[i] == 0)
        {
            ilvl->levels_completed[i] = (uint32_t)lvnum + 1;
            return;
        }
    }
}

static void fill_missing_levels_completed(struct IntralevelData *ilvl, const struct GameCampaign *campgn, LevelNumber ref_lvnum)
{
    for (int i = 0; i < LEVELS_COMPLETED_COUNT; i++)
    {
        if (ilvl->levels_completed[i] != 0)
            return;
    }
    int limit;
    if (ref_lvnum == SINGLEPLAYER_FINISHED)
    {
        limit = CAMPAIGN_LEVELS_COUNT;
    } else
    {
        limit = campaign_singleplayer_level_index(campgn, ref_lvnum);
        if (limit < 0)
            limit = campaign_bonus_level_index(campgn, ref_lvnum);
    }
    for (int i = 0; i < limit; i++)
        mark_level_completed(ilvl, campgn->single_levels[i]);
}

uint32_t encode_continue_level(LevelNumber lvnum)
{
    if (lvnum == SINGLEPLAYER_FINISHED)
        return UINT32_MAX;
    if (lvnum < 0)
        return 0;
    return (uint32_t)lvnum + 1;
}

static LevelNumber stored_continue_level(const struct IntralevelData *ilvl)
{
    if (ilvl->continue_level == 0)
        return LEVELNUMBER_ERROR;
    if (ilvl->continue_level == UINT32_MAX)
        return SINGLEPLAYER_FINISHED;
    return (LevelNumber)(ilvl->continue_level - 1);
}

static LevelNumber resolve_continue_level(const struct IntralevelData *ilvl, const struct GameCampaign *campgn)
{
    LevelNumber lvnum = stored_continue_level(ilvl);
    if ((lvnum == SINGLEPLAYER_FINISHED) || (lvnum == SINGLEPLAYER_NOTSTARTED) || (campaign_singleplayer_level_index(campgn, lvnum) >= 0))
        return lvnum;
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        if ((campgn->single_levels[i] > 0) && !is_level_completed(ilvl, campgn->single_levels[i]))
            return campgn->single_levels[i];
    }
    return SINGLEPLAYER_FINISHED;
}

LevelNumber resolve_campaign_progress(struct IntralevelData *ilvl, const struct GameCampaign *campgn)
{
    fill_missing_levels_completed(ilvl, campgn, stored_continue_level(ilvl));
    LevelNumber lvnum = resolve_continue_level(ilvl, campgn);
    ilvl->continue_level = encode_continue_level(lvnum);
    return lvnum;
}

short campaign_progress_percent(const struct GameCampaign *campgn, const struct IntralevelData *ilvl, LevelNumber continue_lvnum)
{
    int total = 0;
    int done = 0;
    int secret = 0;
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        if (campgn->single_levels[i] <= 0)
            continue;
        total++;
        if (is_level_completed(ilvl, campgn->single_levels[i]))
            done++;
    }
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        LevelNumber lvnum = campgn->bonus_levels[i];
        if ((campaign_bonus_level_index(campgn, lvnum) == i) && (campaign_singleplayer_level_index(campgn, lvnum) < 0)
          && is_level_completed(ilvl, lvnum))
            secret++;
    }
    for (int i = 0; i < EXTRA_LEVELS_COUNT; i++)
    {
        LevelNumber lvnum = campgn->extra_levels[i];
        if ((campaign_extra_level_index(campgn, lvnum) == i) && (campaign_singleplayer_level_index(campgn, lvnum) < 0)
          && (campaign_bonus_level_index(campgn, lvnum) < 0) && is_level_completed(ilvl, lvnum))
            secret++;
    }
    TbBool finished = (continue_lvnum == SINGLEPLAYER_FINISHED) || (done == total);
    int percent;
    if (finished)
        percent = 100;
    else if ((done == 0) || (total <= 0))
        percent = 0;
    else
        percent = (99 * done + total - 1) / total;
    percent += secret;
    if (!finished && (percent > 99))
        percent = 99;
    return percent;
}

TbBool add_transfered_creature(PlayerNumber plyr_idx, ThingModel model, CrtrExpLevel exp_level, char *name)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
    {
        ERRORDBG(11, "Can't transfer creature; player %d has no dungeon.", (int)plyr_idx);
        return false;
    }

    short i = dungeon->creatures_transferred; //makes sure it fits 255 units

    intralvl.transferred_creatures[plyr_idx][i].model = model;
    intralvl.transferred_creatures[plyr_idx][i].exp_level = exp_level;
    strcpy(intralvl.transferred_creatures[plyr_idx][i].creature_name, name);
    return true;
}

void clear_transfered_creatures(void)
{
    for (int p = 0; p < PLAYERS_COUNT; p++)
    {
        for (int i = 0; i < TRANSFER_CREATURE_STORAGE_COUNT; i++)
        {
            intralvl.transferred_creatures[p][i].model = 0;
            intralvl.transferred_creatures[p][i].exp_level = 0;
        }
    }
}

LevelNumber move_campaign_to_next_level(void)
{
    LevelNumber curr_lvnum = get_continue_level_number();
    LevelNumber lvnum = next_singleplayer_level(curr_lvnum, false);
    SYNCDBG(15,"Campaign move %d to %d",curr_lvnum,lvnum);
    {
        struct PlayerInfo* player = get_my_player();
        player->display_flags &= ~PlaF6_PlyrHasQuit;
    }
    if (lvnum != LEVELNUMBER_ERROR)
    {
        curr_lvnum = set_continue_level_number(lvnum);
        SYNCDBG(8,"Continue level moved to %d.",curr_lvnum);
        return curr_lvnum;
    } else
    {
        curr_lvnum = set_continue_level_number(SINGLEPLAYER_NOTSTARTED);
        SYNCDBG(8,"Continue level moved to NOTSTARTED.");
        return curr_lvnum;
    }
}

LevelNumber move_campaign_to_prev_level(void)
{
    LevelNumber curr_lvnum = get_continue_level_number();
    LevelNumber lvnum = prev_singleplayer_level(curr_lvnum);
    SYNCDBG(15,"Campaign move %d to %d",curr_lvnum,lvnum);
    if (lvnum != LEVELNUMBER_ERROR)
    {
        curr_lvnum = set_continue_level_number(lvnum);
        SYNCDBG(8,"Continue level moved to %d.",curr_lvnum);
        return curr_lvnum;
    } else
    {
        curr_lvnum = set_continue_level_number(SINGLEPLAYER_FINISHED);
        SYNCDBG(8,"Continue level moved to FINISHED.");
        return curr_lvnum;
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
