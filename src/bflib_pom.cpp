/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games.
// PhaseOfMoon namespace for Dungeon Keeper remake.
/******************************************************************************/
/** @file bflib_pom.cpp
 *     Moon phase calculator.
 * @par Purpose:
 *     Calculates current phase of moon in a way similar to Dungeon Keeper.
 * @par Comment:
 *     The algorithm is little perfected compared to one originally used in DK.
 *     It computes phase of moon until year 2199 with accuracy to 2 hours.
 * @author   Tomasz Lis
 * @date     30 Jul 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <cmath>
#include <time.h>

#include "bflib_pom.hpp"
#include "bflib_datetm.h"

using namespace std;

/**
 * Returns current phase of the moon, as a number ranged -1.0 to 1.0.
 *
 */
double PhaseOfMoon::Calculate(void)
{
    int nph = 0; // 0 means full moon

    long n;

    double frac_now = 0.0;
    // Compute the current julian date and moon period index
    long jul_now = JulToday(frac_now, n);
    // And make sure we're before the current date with the period
    n -= 8;

    long jul_prev = 0;
    double frac_prev = 0.0;

    long jul_next = 0;
    double frac_next = 0.0;
    PhaseOfMoon::FlMoon(n, nph, jul_next, frac_next);
    unsigned long iter_num = 0;
    short found_phase = 0;
    do
    {
        iter_num++;
        if (iter_num > 16)
        {
            pom_error("POM_Calculate: Too many iterations.");
            return 0.0;
        }
        n++;
        jul_prev = jul_next;
        frac_prev = frac_next;
        PhaseOfMoon::FlMoon(n, nph, jul_next, frac_next);
        if (jul_now <= jul_prev)
        {
            if ((jul_now != jul_prev) || (frac_now < frac_prev))
                continue;
        }
        if (jul_now >= jul_next)
        {
            if ((jul_now != jul_next) || (frac_now >= frac_next))
                continue;
        }
        found_phase = 1;
  }
  while ( !found_phase );

  double phase=frac_now-frac_prev+(double)(jul_now-jul_prev);
  double period=frac_next-frac_prev+(double)(jul_next-jul_prev);
  return (phase/period)*2.0 - 1.0;
}

void PhaseOfMoon::pom_error(const char *msg)
{
//    error("pom.cpp", 54, msg);
}

/**
 * Computes exact time of n-th moon phase. The phase type is given in nph
 * (0=full moon). Returns julian day and exact time in jd and frac parameters.
 *
 * Algorithm originally taken from old Fortran function, perfected by Tomasz Lis
 *
 * @param n Phase number.
 * @param nph Phase type, 0...3; 0-full moon, 2-new moon, 1,3 - quadres.
 * @param jd Julian day result.
 * @param frac Day fraction (time) result.
 */
void PhaseOfMoon::FlMoon(const long n, const short nph, long &jd, double &frac)
{
  const double RAD=PI/180.0;

  double c = n + nph / 4.0;
  double t = c / 1236.85; // time in Julian centuries from 1900 January 0.5
  double t2 = t * t;      // square for frequent use
  double t3 = t2 * t;     // cube for frequent use
  // Sun's mean anomaly
  double as = 359.2242 + 29.10535608 * c - 0.0000333 * t2 - 0.00000347 * t3;
  // Moon's mean anomaly
  double am = 306.0253 + 385.81691806 * c + 0.010730 * t2 + 0.00001236 * t3;
  jd=2415020+28*n+7*nph;
  // Moon's argument of latitude
  double fml = 21.2964 + 390.67050646 * c - 0.0016528 * t2 - 0.00000239 * t3;
  // Mean time of phase
  double xtra = 0.75933 + 1.53058868 * c + ((1.178e-4) - (1.55e-7) * t) * t2;
  if ((nph==0) || (nph==2))
  {
    // Corrections for New and Full Moon
    xtra += (0.1734-3.93e-4*t)*sin(RAD*as) + 0.0021*sin(2.0*RAD*as)
         - 0.4068*sin(RAD*am) + 0.0161*sin(2*RAD*am)
         - 0.0004*sin(3*RAD*am) + 0.0104*sin(2*RAD*fml)
         - 0.0051*sin(RAD*(as+am)) - 0.0074 * sin(RAD*(as-am))
         + 0.0004*sin(RAD*(2*fml+as)) - 0.0004*sin(RAD*(2*fml-as))
         - 0.0006 * sin(RAD*(2*fml+am)) + 0.0010*sin(RAD*(2*fml-am))
         + 0.0005 * sin(RAD*(as+2*am));

  } else
  if ((nph==1) || (nph==3))
  {
    xtra += (0.1721-4.0e-4*t)*sin(RAD*as) + 0.0021*sin(2*RAD*as)
         - 0.6280*sin(RAD*am) + 0.0089*sin(2*RAD*am)
         - 0.0004*sin(3*RAD*am) + 0.0079*sin(2*RAD*fml)
         - 0.0119*sin(RAD*(as+am)) - 0.0047*sin(RAD*(as-am))
         + 0.0003*sin(RAD*(2*fml+as)) - 0.0004*sin(RAD*(2*fml-as))
         - 0.0006*sin(RAD*(2*fml+am)) + 0.0021*sin(RAD*(2*fml-am))
         + 0.0003*sin(RAD*(as+2*am)) + 0.0004*sin(RAD*(as-2*am))
         - 0.0003*sin(RAD*(2*as+am));
  } else
    pom_error("FlMoon: nph is not recognized.");
  int i = int(xtra >= 0.0 ? floor(xtra) : ceil(xtra - 1.0));
  jd += i;
  frac=xtra-i;
}

/**
 * Converts given Gregorian date into Julian day.
 */
long PhaseOfMoon::JulDay(const short mm, const short id, const int iyyy)
{
  const int IGREG=15+31*(10+12*1582);
  int jy=iyyy;
  int jm;

  if (jy == 0) pom_error("JulDay: there is no year zero.");
  if (jy < 0) ++jy;
  if (mm > 2)
  {
    jm=mm+1;
  } else
  {
    --jy;
    jm=mm+13;
  }
  long jul = long(floor(JULIAN_DAYS_PER_YEAR * jy) + floor(JULIAN_DAYS_PER_AVGMONTH * jm) + id + 1720995);
  if (id+31*(mm+12*iyyy) >= IGREG)
  {
      int ja = int(0.01 * jy);
      jul += 2 - ja + int(0.25 * ja);
  }
  return jul;
}

/**
 * Returns current day in Julian calendar, and part of the day in fval parameter.
 */
long PhaseOfMoon::JulToday(double &daypart,long &moon_periods_n)
{
  //moon periods per Julian year, computed assuming
  // constant orbit orientation with respect to the stars
  const double MOON_CNSTORBIT_YEAR_CYCLES=JULIAN_DAYS_PER_YEAR/SYNODIC_MONTH_IN_DAYS;
  time_t rawtime;
  time(&rawtime);
  tm* ptm = gmtime(&rawtime);
  long jul=JulDay(ptm->tm_mon,ptm->tm_mday,ptm->tm_year+1900);
  long tdjul = jul;
  // Approximate number of full moons since january 1900
  moon_periods_n=int(MOON_CNSTORBIT_YEAR_CYCLES*(ptm->tm_year+((ptm->tm_mon-0.5)/12.0)));
  long jul2;
  double frac2;
  PhaseOfMoon::FlMoon(moon_periods_n,0,jul2,frac2);
  // Now correct the approximation
  moon_periods_n += int((jul-jul2)/SYNODIC_MONTH_IN_DAYS + (jul >= jul2 ? 0.5 : -0.5));
  // Now part of the day
  if (ptm->tm_hour >= 12)
  {
    daypart = 3600*(ptm->tm_hour-12) + 60*(ptm->tm_min) + (ptm->tm_sec);
  } else
  {
    tdjul = jul - 1;
    daypart = 3600*(ptm->tm_hour+12) + 60*(ptm->tm_min) + (ptm->tm_sec);
  }
  daypart /= (3600.0*24.0);
  return tdjul;
}

/**
 * Library exported routine for calculating Phase Of Moon.
 * Defined in Ansi-C header "bflib_datetm.h".
 */
double LbMoonPhase(void)
{
  return PhaseOfMoon::Calculate();
}
/******************************************************************************/
