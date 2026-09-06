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
/******************************************************************************/

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
