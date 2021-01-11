/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_datetm.h
 *     Header file for bflib_datetm.c.
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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern struct TbTime global_time;
extern struct TbDate global_date;
extern TbClockMSec (* LbTimerClock)(void);
/******************************************************************************/
void LbDoMultitasking(void);
TbBool __fastcall LbSleepFor(TbClockMSec delay);
TbBool __fastcall LbSleepUntil(TbClockMSec endtime);
TbResult LbTime(struct TbTime *curr_time);
TbTimeSec LbTimeSec(void);
TbResult LbDate(struct TbDate *curr_date);
TbResult LbDateTime(struct TbDate *curr_date, struct TbTime *curr_time);
TbResult LbDateTimeDecode(const time_t *datetime,struct TbDate *curr_date, struct TbTime *curr_time);
TbResult LbTimerInit(void);
double LbMoonPhase(void);
TbClockMSec LbTimerClock_1000(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
