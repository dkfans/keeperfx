/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 12 Feb 2008

// Purpose:
//    Date and time related routines.

// Comment:
//   Gets system date and time, makes delay, converts date/time formats.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#include "bflib_datetm.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "bflib_basics.h"

#if defined(WIN32)
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
TbTime global_time;
TbDate global_date;
/******************************************************************************/
//Returns the number of miliseconds elapsed since the program was launched.
unsigned long LbTimerClock(void)
{
  // original DK uses win32 function timeGetTime();
  return 1000 * clock() / CLOCKS_PER_SEC;
}

//Fills structure with current time
int LbTime(TbTime *curr_time)
{
  time_t dtime;
  time(&dtime);
  return LbDateTimeDecode(&dtime,NULL,curr_time);
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
    Sleep(5); // This switches to other tasks
#endif
}

short __fastcall LbSleepFor(unsigned long delay)
{
  register clock_t endclk = CLOCKS_PER_SEC*delay / 1000;
  endclk+=clock();
  register clock_t currclk;
  currclk=clock();
  if (currclk+(CLOCKS_PER_SEC>>4)<endclk)
  {
    LbDoMultitasking();
    currclk=clock();
  }
  while (currclk<endclk)
  {
    currclk=clock();
    if (currclk==-1)
      return 0;
  }
  return 1;
}

short __fastcall LbSleepUntil(unsigned long endtime)
{
  register clock_t endclk = CLOCKS_PER_SEC*endtime / 1000;
  register clock_t currclk;
  currclk=clock();
  if (currclk+(CLOCKS_PER_SEC>>4)<endclk)
  {
    LbDoMultitasking();
    currclk=clock();
  }
  while (currclk<endclk)
  {
    currclk=clock();
    if (currclk==-1)
      return 0;
  }
  return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
