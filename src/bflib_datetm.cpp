/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_datetm.cpp
 *     Date and time related routines.
 * @par Purpose:
 *     Gets system date and time, makes delay, converts date/time formats.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_datetm.h"

#include <chrono>
#include "bflib_basics.h"
#include "globals.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LARGE_DELAY_TIME 20
/******************************************************************************/
struct TbTime global_time;
struct TbDate global_date;
TbClockMSec (* LbTimerClock)(void);
/******************************************************************************/
#define TimePoint std::chrono::high_resolution_clock::time_point
#define TimeNow std::chrono::high_resolution_clock::now()
TimePoint initialized_time_point;
struct FrametimeMeasurements frametime_measurements;
TimePoint delta_time_previous_timepoint;
int debug_display_frametime = 0;
/******************************************************************************/
void initial_time_point()
{
  initialized_time_point = TimeNow;
  gameadd.process_turn_time = 1.0; // Begin initial turn as soon as possible (like original game)
}

float get_delta_time()
{
    // Allow frame skip to work correctly when delta time is enabled
    if ( (game.frame_skip != 0) && ((game.play_gameturn % game.frame_skip) != 0)) {
        return 1.0;
    }
    long double frame_time_in_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(TimeNow - delta_time_previous_timepoint).count();
    delta_time_previous_timepoint = TimeNow;
    float calculated_delta_time = (frame_time_in_nanoseconds/1000000000.0) * game.num_fps;
    if (calculated_delta_time > 1.0) { // Fix for when initially loading the map, frametime takes too long. Possibly other circumstances too.
        calculated_delta_time = 1.0;
    }
    return calculated_delta_time;
}

void frametime_set_all_measurements_to_be_displayed()
{
    // Display the frametime of the previous frame only, not the current frametime. Drawing "frametime_current" is a bad idea because frametimes are displayed on screen half-way through the rest of the measurements.
    
    bool once_per_half_second = false;
    if (debug_display_frametime == 2)
    {
        // Once per half-second set the display text to highest frametime of the past half-second
        frametime_measurements.max_timer += gameadd.delta_time;
        if (frametime_measurements.max_timer > (game.num_fps/2)) {
            frametime_measurements.max_timer = 0;
            once_per_half_second = true;
        }
    }

    for (int i = 0; i < TOTAL_FRAMETIME_KINDS; i++)
    {
        switch (debug_display_frametime)
        {
            case 1: // Frametime (show constantly)
                frametime_measurements.frametime_display[i] = frametime_measurements.frametime_current[i];
                break;
            case 2: // Frametime max (shown once per half-second)
                // Always get highest frametime
                if (frametime_measurements.frametime_current[i] > frametime_measurements.frametime_get_max[i]) {
                    frametime_measurements.frametime_get_max[i] = frametime_measurements.frametime_current[i];
                }
                // Display once per half-second
                if (once_per_half_second == true)
                {
                    frametime_measurements.frametime_display[i] = frametime_measurements.frametime_get_max[i];
                    frametime_measurements.frametime_get_max[i] = 0;
                }
                break;
        }
    }
}

void frametime_start_measurement(int frametime_kind)
{
    long double current_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(TimeNow - initialized_time_point).count();
    long double current_milliseconds = current_nanoseconds/1000000.0;
    frametime_measurements.starting_measurement[frametime_kind] = float(current_milliseconds);
}

void frametime_end_measurement(int frametime_kind)
{
    long double current_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(TimeNow - initialized_time_point).count();
    long double current_milliseconds = current_nanoseconds/1000000.0;
    float result = float(current_milliseconds) - frametime_measurements.starting_measurement[frametime_kind];
    frametime_measurements.frametime_current[frametime_kind] = result;
    
    if (frametime_kind == Frametime_FullFrame) {
        // Done last at end of frame
        frametime_set_all_measurements_to_be_displayed();
    }
}
/******************************************************************************/
/**
 * Returns the number of milliseconds elapsed since the program was launched.
 * A version for (CLOCKS_PER_SEC == 1000).
 */
TbClockMSec LbTimerClock_1000(void)
{
  return clock();
}

/**
 * Returns the number of milliseconds elapsed since the program was launched.
 * A version for (CLOCKS_PER_SEC == 1024).
 */
TbClockMSec LbTimerClock_1024(void)
{
    clock_t cclk = clock();
    return cclk - (cclk >> 6) - (cclk >> 7);
}

/**
 * Returns the number of milliseconds elapsed since the program was launched.
 * Version for any CLOCKS_PER_SEC, but unsafe.
 */
TbClockMSec LbTimerClock_any(void)
{
  long long clk = 500 * clock();
  return (clk / CLOCKS_PER_SEC) << 1;
}

/** Fills structure with current time.
 *
 * @param curr_time The structure to be filled.
 * @return
 */
TbResult LbTime(struct TbTime *curr_time)
{
  time_t dtime;
  time(&dtime);
  return LbDateTimeDecode(&dtime,NULL,curr_time);
}

/** Returns current calendar time in seconds.
 *
 * @return System time in seconds.
 */
TbTimeSec LbTimeSec(void)
{
  time_t dtime;
  time(&dtime);
  return dtime;
}

//Fills structure with current date
TbResult LbDate(struct TbDate *curr_date)
{
  time_t dtime;
  time(&dtime);
  return LbDateTimeDecode(&dtime,curr_date,NULL);
}

//Fills structures with current date and time
TbResult LbDateTime(struct TbDate *curr_date, struct TbTime *curr_time)
{
  time_t dtime;
  time(&dtime);
  return LbDateTimeDecode(&dtime,curr_date,curr_time);
}

TbResult LbDateTimeDecode(const time_t *datetime,struct TbDate *curr_date,struct TbTime *curr_time)
{
  struct tm *ltime=localtime(datetime);
  if (curr_date!=NULL)
  {
    curr_date->Day=ltime->tm_mday;
    curr_date->Month=ltime->tm_mon+1;
    curr_date->Year=1900+ltime->tm_year;
    curr_date->DayOfWeek=ltime->tm_wday;
  }
  if (curr_time!=NULL)
  {
    curr_time->Hour=ltime->tm_hour;
    curr_time->Minute=ltime->tm_min;
    curr_time->Second=ltime->tm_sec;
    curr_time->HSecond=0;
  }
  return Lb_SUCCESS;
}

inline void LbDoMultitasking(void)
{
#if defined(_WIN32)
    Sleep(LARGE_DELAY_TIME>>1); // This switches to other tasks
#endif
}

TbBool __fastcall LbSleepFor(TbClockMSec delay)
{
    TbClockMSec currclk = LbTimerClock();
    TbClockMSec endclk = currclk + delay;
    while ((currclk + LARGE_DELAY_TIME) < endclk)
    {
        LbDoMultitasking();
        currclk = LbTimerClock();
  }
  while (currclk < endclk)
    currclk=LbTimerClock();
  return true;
}

TbBool __fastcall LbSleepUntil(TbClockMSec endtime)
{
    TbClockMSec currclk = LbTimerClock();
    while ((currclk + LARGE_DELAY_TIME) < endtime)
    {
        LbDoMultitasking();
        currclk = LbTimerClock();
  }
  while (currclk < endtime)
    currclk=LbTimerClock();
  return true;
}

TbResult LbTimerInit(void)
{
  switch (CLOCKS_PER_SEC)
  {
  case 1000:
    LbTimerClock = LbTimerClock_1000;
    break;
  case 1024:
    LbTimerClock = LbTimerClock_1024;
    break;
  default:
    LbTimerClock = LbTimerClock_any;
    WARNMSG("Timer uses unsafe clock multiplication!");
    break;
  }
  return Lb_SUCCESS;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
