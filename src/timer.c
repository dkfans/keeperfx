/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file timer.c
 *     Timer support functions.
 * @par Purpose:
 *     Definitions and functions to maintain timers.
 * @par Comment:
 *     None.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "timer.h"

#include <stdbool.h>

#include "bflib_datetm.h"
#include "globals.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
TbClockMSec timerstarttime = 0;
struct TimerTime Timer;
TbBool TimerGame = false;
TbBool TimerNoReset = false;
TbBool TimerFreeze = false;
static TbClockMSec level_load_times[LevelLoadTime_Count];
static TbClockMSec level_load_total_start;
static TbClockMSec level_load_phase_start;
static enum LevelLoadTimeKind level_load_phase;
static TbBool level_load_time_active;
/******************************************************************************/

void level_load_time_phase(enum LevelLoadTimeKind kind)
{
    TbClockMSec now = LbTimerClock();
    if (kind == LevelLoadTime_EngineStartup && !level_load_time_active) {
        memset(level_load_times, 0, sizeof(level_load_times));
        level_load_total_start = now;
        level_load_phase_start = now;
        level_load_phase = LevelLoadTime_EngineStartup;
        level_load_time_active = true;
        return;
    }
    if (!level_load_time_active)
        return;
    level_load_times[level_load_phase] += now - level_load_phase_start;
    if (kind == LevelLoadTime_Total) {
        level_load_times[LevelLoadTime_Total] = now - level_load_total_start;
        JUSTLOG("Level load timing: Engine startup: %d ms, Custom sprites: %d ms, Config files: %d ms, Level data: %d ms, Navigation: %d ms, Game setup: %d ms, Total: %d ms", level_load_times[LevelLoadTime_EngineStartup], level_load_times[LevelLoadTime_Sprites], level_load_times[LevelLoadTime_Configs], level_load_times[LevelLoadTime_Data], level_load_times[LevelLoadTime_Navigation], level_load_times[LevelLoadTime_GameSetup], level_load_times[LevelLoadTime_Total]);
        level_load_time_active = false;
        return;
    }
    level_load_phase = kind;
    level_load_phase_start = now;
}

void update_time(void)
{
    unsigned long time = ((unsigned long)LbTimerClock()) - timerstarttime;
    Timer.MSeconds = time % 1000;
    time /= 1000;
    Timer.Seconds = time % 60;
    time /= 60;
    Timer.Minutes = time % 60;
    Timer.Hours = time / 60;
}

struct GameTime get_game_time(unsigned long turns, unsigned long fps)
{
    struct GameTime GameT;
    unsigned long time = turns / fps;
    GameT.Seconds = time % 60;
    time /= 60;
    GameT.Minutes = time % 60;
    GameT.Hours = time / 60;
    return GameT;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
