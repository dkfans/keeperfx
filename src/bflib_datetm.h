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
void LbSleepExtInit();
TbBool LbSleepUntilExt(long double tick_ns_end);
TbBool LbSleepDelayExt(long double tick_ns_delay);
TbResult LbTime(struct TbTime *curr_time);
TbTimeSec LbTimeSec(void);
uint64_t LbSystemClockMilliseconds(void);
TbResult LbDate(struct TbDate *curr_date);
TbResult LbDateTime(struct TbDate *curr_date, struct TbTime *curr_time);
TbResult LbDateTimeDecode(const time_t *datetime,struct TbDate *curr_date, struct TbTime *curr_time);
TbResult LbTimerInit(void);
/******************************************************************************/



// TriggerTime: Measure the number of triggers within a specified time, such as frame rate

// Assuming that the number of triggers per second does not exceed 1000
#define MAX_TRIGGER_TIME_CNT 2000
struct TriggerTimeMeasurement {
    float trigger_time[MAX_TRIGGER_TIME_CNT];
    int trigger_cnt;
};

void trigger_time_measurement_capture(struct TriggerTimeMeasurement *trigger);
int get_trigger_time_measurement_fps(struct TriggerTimeMeasurement *trigger);


// Framerate: Measure the number of triggers within a specified time, such as frame rate
#define TOTAL_FRAMERATE_KINDS 3
enum FramerateKinds {
    Framerate_FullFrame = 0,
    Framerate_Draw = 1,
    Framerate_Logic = 2,
};

// Frametime: Measure the duration of each frame
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
    float frametime_display[TOTAL_FRAMETIME_KINDS];
    float frametime_get_max[TOTAL_FRAMETIME_KINDS];
    float frametime_get_min[TOTAL_FRAMETIME_KINDS];
    float max_timer;

    struct TriggerTimeMeasurement framerate_measurement[TOTAL_FRAMERATE_KINDS];
    int framerate_display[TOTAL_FRAMERATE_KINDS];
    int framerate_max[TOTAL_FRAMERATE_KINDS];
    int framerate_min[TOTAL_FRAMERATE_KINDS];
};

extern int debug_display_frametime;
extern void initial_time_point();
extern long double get_time_tick_ns();
extern void frametime_start_measurement(int frametime_kind);
extern void frametime_end_measurement(int frametime_kind);
extern void framerate_measurement_capture(int framerate_kind);
extern float get_delta_time();

extern struct FrametimeMeasurements frametime_measurements;

int get_current_slowdown_percentage(void);

extern int slowdown_current;
extern int slowdown_average;
extern int slowdown_max;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
