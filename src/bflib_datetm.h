/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 12 Feb 2008

// Purpose:
//    Header file for bflib_datetm.c.

// Comment:
//   Just a header file - #defines, typedefs, function prototypes etc.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#ifndef BFLIB_DATETM_H
#define BFLIB_DATETM_H

#include <time.h>
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern TbTime global_time;
extern TbDate global_date;
/******************************************************************************/
short __fastcall LbSleepFor(unsigned long delay);
short __fastcall LbSleepUntil(unsigned long endtime);
unsigned long LbTimerClock(void);
int __fastcall LbTime(struct TbTime *curr_time);
int __fastcall LbDate(struct TbDate *curr_date);
int __fastcall LbDateTime(struct TbDate *curr_date, struct TbTime *curr_time);
int __fastcall LbDateTimeDecode(const time_t *datetime,struct TbDate *curr_date, struct TbTime *curr_time);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
