/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_datetm.c
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
#include "bflib_datetm.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "bflib_basics.h"

#if defined(WIN32)
//instead of #include <windows.h>
#include <windef.h>
#include <winbase.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LARGE_DELAY_TIME 20
/******************************************************************************/
TbTime global_time;
TbDate global_date;
TbClockMSec (* LbTimerClock)(void);
/******************************************************************************/
/*
 * Returns the number of miliseconds elapsed since the program was launched.
 * A version for (CLOCKS_PER_SEC == 1000).
 */
TbClockMSec LbTimerClock_1000(void)
{
  // original DK uses win32 function timeGetTime();
  return clock();
}

/*
 * Returns the number of miliseconds elapsed since the program was launched.
 * A version for (CLOCKS_PER_SEC == 1024).
 */
TbClockMSec LbTimerClock_1024(void)
{
  clock_t cclk;
  cclk = clock();
  return cclk - (cclk>>6) - (cclk>>7);
}

/*
 * Returns the number of miliseconds elapsed since the program was launched.
 * Version for any CLOCKS_PER_SEC, but unsafe.
 */
TbClockMSec LbTimerClock_any(void)
{
  return (500 * clock() / CLOCKS_PER_SEC) << 1;
}

//Fills structure with current time
int LbTime(TbTime *curr_time)
{
  time_t dtime;
  time(&dtime);
  return LbDateTimeDecode(&dtime,NULL,curr_time);
}

//Returns current time in seconds
TbTimeSec LbTimeSec(void)
{
  time_t dtime;
  time(&dtime);
  return dtime;
}

//Fills structure with current date
int LbDate(TbDate *curr_date)
{
  time_t dtime;
  time(&dtime);
  return LbDateTimeDecode(&dtime,curr_date,NULL);
}

//Fills structures with current date and time
int LbDateTime(TbDate *curr_date, TbTime *curr_time)
{
  time_t dtime;
  time(&dtime);
  return LbDateTimeDecode(&dtime,curr_date,curr_time);
}

int LbDateTimeDecode(const time_t *datetime,TbDate *curr_date, TbTime *curr_time)
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
  return 1;
}

void inline LbDoMultitasking(void)
{
#if defined(WIN32)
    Sleep(LARGE_DELAY_TIME>>1); // This switches to other tasks
#endif
}

short __fastcall LbSleepFor(TbClockMSec delay)
{
  register TbClockMSec currclk;
  register TbClockMSec endclk;
  currclk=LbTimerClock();
  endclk=currclk+delay;
  while ((currclk+LARGE_DELAY_TIME) < endclk)
  {
    LbDoMultitasking();
    currclk=LbTimerClock();
  }
  while (currclk < endclk)
    currclk=LbTimerClock();
  return 1;
}

short __fastcall LbSleepUntil(TbClockMSec endtime)
{
  register TbClockMSec currclk;
  currclk=LbTimerClock();
  while ((currclk+LARGE_DELAY_TIME) < endtime)
  {
    LbDoMultitasking();
    currclk=LbTimerClock();
  }
  while (currclk < endtime)
    currclk=LbTimerClock();
  return 1;
}

short LbTimerInit(void)
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
    LbWarnLog("Timer uses unsafe clock multiplication!\n");
    break;
  }
  return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
