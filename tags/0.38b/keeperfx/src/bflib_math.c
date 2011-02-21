/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_math.c
 *     Math routines.
 * @par Purpose:
 *     Fast math routines, mostly fixed point.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     24 Jan 2009 - 08 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_math.h"

#include "bflib_basics.h"

/******************************************************************************/
unsigned short const lbSqrTable[] = {
   0x0001, 0x0002, 0x0002, 0x0004, 0x0005, 0x0008, 0x000B, 0x0010,
   0x0016, 0x0020, 0x002D, 0x0040, 0x005A, 0x0080, 0x00B5, 0x0100,
   0x016A, 0x0200, 0x02D4, 0x0400, 0x05A8, 0x0800, 0x0B50, 0x1000,
   0x16A0, 0x2000, 0x2D41, 0x4000, 0x5A82, 0x8000, 0xB504, 0xFFFF,};

unsigned short const lbArcTanFactors[] = {
     0,  1,  2,  3,  5,  6,  7,  8, 10, 11, 12, 13, 15, 16, 17, 19,
    20, 21, 22, 24, 25, 26, 27, 29, 30, 31, 32, 34, 35, 36, 38, 39,
    40, 41, 43, 44, 45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 57, 59,
    60, 61, 62, 64, 65, 66, 67, 68, 70, 71, 72, 73, 75, 76, 77, 78,
    79, 81, 82, 83, 84, 85, 87, 88, 89, 90, 91, 92, 94, 95, 96, 97,
    98, 99,101,102,103,104,105,106,107,109,110,111,112,113,114,115,
   116,118,119,120,121,122,123,124,125,126,127,129,130,131,132,133,
   134,135,136,137,138,139,140,141,142,143,144,145,147,148,149,150,
   151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,
   167,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,
   182,182,183,184,185,186,187,188,189,190,191,191,192,193,194,195,
   196,197,198,198,199,200,201,202,203,203,204,205,206,207,208,208,
   209,210,211,212,212,213,214,215,216,216,217,218,219,220,220,221,
   222,223,223,224,225,226,226,227,228,229,229,230,231,232,232,233,
   234,235,235,236,237,237,238,239,239,240,241,242,242,243,244,244,
   245,246,246,247,248,248,249,250,250,251,252,252,253,254,254,255,
   256 };
/******************************************************************************/
DLLIMPORT int _DK_lbCosTable[2048];
#define lbCosTable _DK_lbCosTable
DLLIMPORT int _DK_lbSinTable[2048];
#define lbSinTable _DK_lbSinTable
/******************************************************************************/
long LbSinL(long x)
{
    // "& 0x7FF" is faster than "% ANGLE_TRIGL_PERIOD"
    return lbSinTable[(unsigned long)x & 0x7FF];
}

long LbCosL(long x)
{
    // "& 0x7FF" is faster than "% ANGLE_TRIGL_PERIOD"
    return lbCosTable[(unsigned long)x & 0x7FF];
}

long LbArcTan(long x,long n)
{
    long ux,un;
    long result;
    if ((x == 0) && (n == 0))
        return 0;
    if (x < 0)
    {
        ux = -x;
        if (n < 0)
        {
            un = -n;
            if (ux < un) {
              result = 2048 - lbArcTanFactors[(ux << 8)/un];
            } else {
              result = lbArcTanFactors[(un << 8)/ux] + 1536;
            }
        } else
        {
            un = n;
            if (ux < un) {
              result = lbArcTanFactors[(ux << 8)/un] + 1024;
            } else {
              result = 1536 - lbArcTanFactors[(un << 8)/ux];
            }
        }
    } else
    {
        ux = x;
        if (n < 0)
        {
            un = -n;
            if (ux < un) {
              result = lbArcTanFactors[(ux << 8)/un];
            } else {
              result = 512 - lbArcTanFactors[(un << 8)/ux];
            }
        } else
        {
            un = n;
            if (ux < un) {
              result = 1024 - lbArcTanFactors[(ux << 8)/un];
            } else {
              result = lbArcTanFactors[(un << 8)/ux] + 512;
            }
        }
    }
    return result;
}

long LbSqrL(long x)
{
  long y;
  if (x <= 0)
    return 0;
  //
  asm ("bsrl     %1, %%eax;\n"
       "movl %%eax, %0;\n"
       :"=r"(y)  // output
       :"r"(x)   // input
       :"%eax"   // clobbered register
       );
  y = lbSqrTable[y];
  while ((x/y) < y)
    y = ((x/y) + y) >> 1;
  return y;
}

long LbMathOperation(unsigned char opkind, long val1, long val2)
{
  switch (opkind)
  {
    case MOp_EQUAL:
      return val1 == val2;
    case MOp_NOT_EQUAL:
      return val1 != val2;
    case MOp_SMALLER:
      return val1 < val2;
    case MOp_GREATER:
      return val1 > val2;
    case MOp_SMALLER_EQ:
      return val1 <= val2;
    case MOp_GREATER_EQ:
      return val1 >= val2;
    case MOp_LOGIC_AND:
      return val1 && val2;
    case MOp_LOGIC_OR:
      return val1 || val2;
    case MOp_LOGIC_XOR:
      return (val1!=0) ^ (val2!=0);
    case MOp_BITWS_AND:
      return val1 & val2;
    case MOp_BITWS_OR:
      return val1 | val2;
    case MOp_BITWS_XOR:
      return val1 ^ val2;
    case MOp_SUM:
      return val1 + val2;
    case MOp_SUBTRACT:
      return val1 - val2;
    case MOp_MULTIPLY:
      return val1 * val2;
    case MOp_DIVIDE:
      return val1 / val2;
    case MOp_MODULO:
      return val1 % val2;
    default:
      return val1;
  }
}

unsigned long LbRandomSeries(unsigned long range, unsigned long *seed, const char *func_name, unsigned long place)
{
  if (range == 0)
    return 0;
  unsigned long i;
  i = 9377 * (*seed) + 9439;
  *seed = _lrotr(i, 13);
  i = (*seed) % range;
//  SYNCMSG("%s: at %d, random val %d", func_name, place, i);
  return i;
}

TbBool LbNumberSignsSame(long num_a, long num_b)
{
    int sign_a,sign_b;
    if (num_a >= 0)
        sign_a = (num_a != 0);
    else
        sign_a = -1;
    if (num_b >= 0)
        sign_b = (num_b != 0);
    else
        sign_b = -1;
    return (sign_a == sign_b);
}

char LbCompareMultiplications(long mul1a, long mul1b, long mul2a, long mul2b)
{
  long long mul1,mul2;
  mul1 = (long long)mul1a * (long long)mul1b;
  mul2 = (long long)mul2a * (long long)mul2b;
  if (mul1 > mul2)
      return 1;
  if (mul1 < mul2)
      return -1;
  return 0;
}

/**
 *  Optimized function for computing proportion.
 * @param a
 * @param b
 * @return
 */
long LbProportion(long a, long b)
{
    long long tmpval;
    if (a > b) {
        tmpval = a * proportions[(b << 8)/a + 256].field_4;
    } else
    {
        if (b == 0) {
            return 0; // Just to avoid dividing by 0
        }
        tmpval = b * proportions[(a << 8)/b + 256].field_4;
    }
    return (tmpval >> 13);
}

/******************************************************************************/
