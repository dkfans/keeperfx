/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_datetm.h
 *     Header file for bflib_datetm.cpp.
 * @par Purpose:
 *     Gets system date and time, makes delay, converts date/time formats.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_DATETM_H
#define BFLIB_DATETM_H

#include <time.h>
#include "bflib_basics.h"
#include "keeperfx.hpp"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern struct TbTime global_time;
extern struct TbDate global_date;
extern TbClockMSec (* LbTimerClock)(void);
/******************************************************************************/
void LbDoMultitasking(void);
TbBool LbSleepFor(TbClockMSec delay);
TbBool LbSleepUntil(TbClockMSec endtime);
TbResult LbTime(struct TbTime *curr_time);
TbTimeSec LbTimeSec(void);
TbResult LbDate(struct TbDate *curr_date);
TbResult LbDateTime(struct TbDate *curr_date, struct TbTime *curr_time);
TbResult LbDateTimeDecode(const time_t *datetime,struct TbDate *curr_date, struct TbTime *curr_time);
TbResult LbTimerInit(void);
double LbMoonPhase(void);
TbClockMSec LbTimerClock_1000(void);
/******************************************************************************/

#define TOTAL_FRAMETIME_KINDS 4
enum FrametimeKinds {
    Frametime_FullFrame = 0,
    Frametime_Logic = 1,
    Frametime_Draw = 2,
    Frametime_Sleep = 3,
};
struct FrametimeMeasurements {
    float starting_measurement[TOTAL_FRAMETIME_KINDS];
    float frametime_current[TOTAL_FRAMETIME_KINDS];
    float frametime_get_max[TOTAL_FRAMETIME_KINDS];
    float frametime_display[TOTAL_FRAMETIME_KINDS];
    float max_timer;
};

extern int debug_display_frametime;
extern void initial_time_point();
extern void frametime_start_measurement(int frametime_kind);
extern void frametime_end_measurement(int frametime_kind);
extern float get_delta_time();

extern struct FrametimeMeasurements frametime_measurements;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
