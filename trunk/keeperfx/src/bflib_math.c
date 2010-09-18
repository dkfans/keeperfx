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
unsigned short lbSqrTable[] =
  {0x0001, 0x0002, 0x0002, 0x0004, 0x0005, 0x0008, 0x000B, 0x0010,
   0x0016, 0x0020, 0x002D, 0x0040, 0x005A, 0x0080, 0x00B5, 0x0100,
   0x016A, 0x0200, 0x02D4, 0x0400, 0x05A8, 0x0800, 0x0B50, 0x1000,
   0x16A0, 0x2000, 0x2D41, 0x4000, 0x5A82, 0x8000, 0xB504, 0xFFFF,};
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

/******************************************************************************/
