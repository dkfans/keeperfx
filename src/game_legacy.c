/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_legacy.c
 *     Module which contains the legacy Game structure.
 * @par Purpose:
 *     Allows easy saving and loading of game data.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     21 Oct 2009 - 23 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_legacy.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_datetm.h"
#include "post_inc.h"

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Game game;
struct Configs *conf_baseline = NULL;

void take_conf_baseline(void)
{
    if (conf_baseline == NULL)
        conf_baseline = (struct Configs *)malloc(sizeof(struct Configs));
    if (conf_baseline != NULL)
        memcpy(conf_baseline, &game.conf, sizeof(struct Configs));
}
unsigned char local_system_flags;

GameTurn get_gameturn()
{
    return game.play_gameturn;
}

void take_game_timestamp(void)
{
    if (!get_local_timezone(&game.timestamp_tz, NULL))
        game.timestamp_tz = 0;
    const TbTimeSec now = LbTimeSec();
    game.timestamp = (now == (TbTimeSec)-1) ? 0 : (int64_t)now;
    game.timestamp_turn = get_gameturn();
}

TbBool network_is_active(void)
{
    return flag_is_set(local_system_flags, GSF_NetworkActive);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/******************************************************************************/
