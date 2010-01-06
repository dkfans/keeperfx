/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games.
// PhaseOfMoon namespace for Dungeon Keeper remake.
/******************************************************************************/
/** @file bflib_pom.hpp
 *     Header file for bflib_pom.cpp.
 * @par Purpose:
 *     Moon phase calculator.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     30 Jul 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_POM_H
#define BFLIB_POM_H
/******************************************************************************/
namespace PhaseOfMoon {
  double Calculate(void);
  void FlMoon(const long n, const short nph, long &jd, double &frac);
  long JulDay(const short mm, const short id, const int iyyy);
  long JulToday(double &daypart,long &moon_periods_n);
  void pom_error(const char *msg);
  const double JULIAN_DAYS_PER_YEAR=365.25;       // days per year in Julian calendar
  const double JULIAN_DAYS_PER_AVGMONTH=30.6001;  // average days per month in Julian calendar
  const double SYNODIC_MONTH_IN_DAYS=29.53058868; // synodic month (new Moon to new Moon) in days
  const double PI=3.141592653589793238;
}
/******************************************************************************/
#endif
