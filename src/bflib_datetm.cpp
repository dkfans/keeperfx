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
#include <chrono>
#include <time.h>
#include "globals.h"
#include "bflib_datetm.h"
#include "bflib_basics.h"
#include "game_legacy.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LARGE_DELAY_TIME 20
long double sleep_precision_ns = 20000000; // 20ms
/******************************************************************************/
struct TbTime global_time;
struct TbDate global_date;
TbClockMSec (* LbTimerClock)(void);
int slowdown_current = 0;
int slowdown_average = 0;
int slowdown_max = 0;
/******************************************************************************/
#define TimePoint std::chrono::high_resolution_clock::time_point
#define TimeNow std::chrono::high_resolution_clock::now()
#define TimeTickNs std::chrono::duration_cast<std::chrono::nanoseconds>(TimeNow - initialized_time_point).count()


TimePoint initialized_time_point;
struct FrametimeMeasurements frametime_measurements;
TimePoint delta_time_previous_timepoint;
int debug_display_frametime = 0;
/******************************************************************************/
void initial_time_point()
{
  initialized_time_point = TimeNow;
  game.process_turn_time = 1.0; // Begin initial turn as soon as possible (like original game)
}

long double get_time_tick_ns()
{
  return TimeTickNs;
}

void trigger_time_measurement_capture(struct TriggerTimeMeasurement *trigger)
{
  long double current_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(TimeNow - initialized_time_point).count();
  long double current_milliseconds = current_nanoseconds/1000000.0;
  if (trigger->trigger_cnt >= MAX_TRIGGER_TIME_CNT)
  {
    int keep_cnt = MAX_TRIGGER_TIME_CNT/2;
    memmove(trigger->trigger_time, trigger->trigger_time+trigger->trigger_cnt-keep_cnt, sizeof(trigger->trigger_time[0])*keep_cnt);
    trigger->trigger_cnt=keep_cnt;
  }
  trigger->trigger_time[trigger->trigger_cnt] = (float)current_milliseconds;
  trigger->trigger_cnt++;
}

int get_trigger_time_measurement_fps(struct TriggerTimeMeasurement *trigger)
{
  int cnt = 0;
  if (trigger->trigger_cnt > 0)
  {
    const float measurement_duration = 1000;
    const float last_time = trigger->trigger_time[trigger->trigger_cnt-1];
    cnt++;
    for (int i=trigger->trigger_cnt-2; i>=0; i--)
    {
      if (last_time - trigger->trigger_time[i] >= measurement_duration)
        break;
      cnt++;
    }
  }
  return cnt;
}


float get_delta_time()
{
    // Allow frame skip to work correctly when delta time is enabled
    if ( (game.frame_skip != 0) && ((game.play_gameturn % game.frame_skip) != 0)) {
        return 1.0;
    }
    long double frame_time_in_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(TimeNow - delta_time_previous_timepoint).count();
    delta_time_previous_timepoint = TimeNow;
    float calculated_delta_time = (frame_time_in_nanoseconds/1000000000.0) * game_num_fps;
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
        frametime_measurements.max_timer += game.delta_time;
        if (frametime_measurements.max_timer > (game_num_fps/2)) {
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
            case 2: // Frametime min/max (shown once per half-second)
                frametime_measurements.frametime_display[i] = frametime_measurements.frametime_current[i];
                 // Display once per half-second
                if (once_per_half_second == true)
                {
                    frametime_measurements.frametime_get_min[i] = 99999;
                    frametime_measurements.frametime_get_max[i] = 0;
                }
                if (frametime_measurements.frametime_get_min[i] > frametime_measurements.frametime_display[i]) {
                    frametime_measurements.frametime_get_min[i] = frametime_measurements.frametime_display[i];
                }
                if (frametime_measurements.frametime_get_max[i] < frametime_measurements.frametime_display[i]) {
                    frametime_measurements.frametime_get_max[i] = frametime_measurements.frametime_display[i];
                }
                break;
        }
    }

    for (int i = 0; i < TOTAL_FRAMERATE_KINDS; i++)
    {
        switch (debug_display_frametime)
        {
            case 1: // Framerate (show constantly)
            case 2: // Framerate min/max (shown once per half-second)
              {
                int cur_fps = get_trigger_time_measurement_fps(frametime_measurements.framerate_measurement+i);
                frametime_measurements.framerate_display[i] = cur_fps;
                if (debug_display_frametime == 2) {
                  // Display once per half-second
                  if (once_per_half_second == true) {
                      frametime_measurements.framerate_min[i] = 99999;
                      frametime_measurements.framerate_max[i] = 0;
                  }
                  if (frametime_measurements.framerate_min[i] > cur_fps) {
                      frametime_measurements.framerate_min[i] = cur_fps;
                  }
                  if (frametime_measurements.framerate_max[i] < cur_fps) {
                      frametime_measurements.framerate_max[i] = cur_fps;
                  }
                }
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

void framerate_measurement_capture(int framerate_kind)
{
  if (framerate_kind < 0 || framerate_kind >= TOTAL_FRAMERATE_KINDS)
    return;
  trigger_time_measurement_capture(frametime_measurements.framerate_measurement + framerate_kind);
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
  clock_t cclk = clock();
  if (CLOCKS_PER_SEC > 1000) {
    return cclk / (CLOCKS_PER_SEC / 1000);
  } else if (CLOCKS_PER_SEC > 100) {
    return (cclk / (CLOCKS_PER_SEC / 100)) * 10;
  } else if (CLOCKS_PER_SEC > 10) {
    return (cclk / (CLOCKS_PER_SEC / 10)) * 100;
  } else {
    return (cclk / CLOCKS_PER_SEC) * 1000;
  }
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

extern "C" uint64_t LbSystemClockMilliseconds(void)
{
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  return static_cast<uint64_t>(millis);
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
  if (curr_date) {
    curr_date->Day = 0;
    curr_date->Month = 0;
    curr_date->Year = 0;
    curr_date->DayOfWeek = 0;
  }
  if (curr_time) {
    curr_time->Hour = 0;
    curr_time->Minute = 0;
    curr_time->Second = 0;
    curr_time->HSecond = 0;
  }
  struct tm *ltime = localtime(datetime);
  if (ltime == NULL)
  {
      return Lb_FAIL;
  }
  if (curr_date != NULL)
  {
    curr_date->Day = ltime->tm_mday;
    curr_date->Month = ltime->tm_mon+1;
    curr_date->Year = 1900+ltime->tm_year;
    curr_date->DayOfWeek = ltime->tm_wday;
  }
  if (curr_time != NULL)
  {
    curr_time->Hour = ltime->tm_hour;
    curr_time->Minute = ltime->tm_min;
    curr_time->Second = ltime->tm_sec;
    curr_time->HSecond = 0;
  }
  return Lb_SUCCESS;
}

inline void LbDoMultitasking(void)
{
#if defined(_WIN32)
    Sleep(LARGE_DELAY_TIME>>1); // This switches to other tasks
#endif
}

TbBool LbSleepFor(TbClockMSec delay)
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

TbBool LbSleepUntil(TbClockMSec endtime)
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

// Ext(Extend): High Precision Sleep Version
void LbSleepExtInit()
{
  // Test and obtain accurate precision values
  long double tick_ns_begin = TimeTickNs;
  long double tick_ns_end = 0;
  const long double tick_ns_max_test = 100000000; // 100m
  const int max_test_cnt = 30; // for 1ms precision, need 30ms
  int cur_cnt_test = 0;
  for (int i=0; i<max_test_cnt; i++)
  {
    SDL_Delay(1);
    tick_ns_end = TimeTickNs;
    cur_cnt_test++;
    if (tick_ns_end - tick_ns_begin > tick_ns_max_test)
       break;
  }
  sleep_precision_ns = (tick_ns_end - tick_ns_begin)/cur_cnt_test;
}

/* @comment for precision
 *   Windows Platform, this function, the precision is more affected by std::chrono than SDL_Delay.
 *   After deeper testing, it should be a compiler issue, at least MinGW has precision issue, std::chrono can only reach 1ms level.
 *   For more information, please refer to
 *   https://stackoverflow.com/questions/67584437/stdchrono-nanosecond-timer-works-on-msvc-but-not-gcc
 *   https://github.com/msys2/MINGW-packages/issues/5086
 */
TbBool LbSleepUntilExt(long double tick_ns_end)
{
  while(1)
  {
    long double tick_ns_cur = TimeTickNs;
    if (tick_ns_cur >= tick_ns_end)
      break;
    long double tick_ns_delay = tick_ns_end - tick_ns_cur;
    if (tick_ns_delay > sleep_precision_ns) {
      int ms_delay = (int)(tick_ns_delay/1000000);
      SDL_Delay(ms_delay);
    }
  }
  return true;
}

TbBool LbSleepDelayExt(long double tick_ns_delay)
{
    long double tick_ns_cur = TimeTickNs;
    long double tick_ns_end = tick_ns_cur + tick_ns_delay;
    return LbSleepUntilExt(tick_ns_end);
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

int get_current_slowdown_percentage() {
    static TbClockMSec last_frame_timestamp = 0;
    static int slowdown_history[50] = {0};
    static int history_index = 0;
    TbClockMSec current_timestamp = LbTimerClock();
    TbClockMSec frame_time_ms = 0;
    int slowdown_pct = 0;
    if (last_frame_timestamp != 0) {
        frame_time_ms = current_timestamp - last_frame_timestamp;
        int expected_frame_time = 1000 / game_num_fps;
        if (frame_time_ms > expected_frame_time) {
            slowdown_pct = ((frame_time_ms - expected_frame_time) * 100) / expected_frame_time;
        }
    }
    last_frame_timestamp = current_timestamp;
    slowdown_current = slowdown_pct;
    slowdown_history[history_index] = slowdown_pct;
    history_index = (history_index + 1) % 50;
    int sum = 0;
    int max = 0;
    int i;
    for (i = 0; i < 50; i++) {
        sum += slowdown_history[i];
        if (slowdown_history[i] > max) {
            max = slowdown_history[i];
        }
    }
    slowdown_average = sum / 50;
    slowdown_max = max;
    return slowdown_pct;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
