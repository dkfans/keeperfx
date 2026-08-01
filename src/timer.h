/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file timer.h
 *     Header file for timer.c.
 *     Note that this file is a C header, while its code is CPP.
 * @par Purpose:
 *     Timer functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

void update_time(void);
extern TbClockMSec timerstarttime;
struct TimerTime {
        unsigned char Hours;
        unsigned char Minutes;
        unsigned char Seconds;
        unsigned short MSeconds;
};
extern struct TimerTime Timer;
extern TbBool TimerGame;
extern TbBool TimerNoReset;
extern TbBool TimerFreeze;
struct GameTime {
    unsigned char Seconds;
    unsigned char Minutes;
    unsigned char Hours;
};

struct GameTime get_game_time(unsigned long turns, unsigned long fps);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // TIMER_H
