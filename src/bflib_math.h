/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_math.h
 *     Header file for bflib_math.c.
 * @par Purpose:
 *     Math routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     24 Jan 2009 - 08 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_MATH_H
#define BFLIB_MATH_H

#include "bflib_basics.h"

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define LbFPMath_PI 1024
#define LbFPMath_AngleMask 0x7FF
/** Amount of fractional bits in resulting values of trigonometric operations. */
#define LbFPMath_TrigmBits 16

enum MathOperator {
    MOp_UNDEFINED                      =  0,
    MOp_EQUAL                          =  1,
    MOp_NOT_EQUAL                      =  2,
    MOp_SMALLER                        =  3,
    MOp_GREATER                        =  4,
    MOp_SMALLER_EQ                     =  5,
    MOp_GREATER_EQ                     =  6,
    MOp_LOGIC_AND                      =  7,
    MOp_LOGIC_OR                       =  8,
    MOp_LOGIC_XOR                      =  9,
    MOp_BITWS_AND                      = 10,
    MOp_BITWS_OR                       = 11,
    MOp_BITWS_XOR                      = 12,
    MOp_SUM                            = 13,
    MOp_SUBTRACT                       = 14,
    MOp_MULTIPLY                       = 15,
    MOp_DIVIDE                         = 16,
    MOp_MODULO                         = 17,
};

struct Proportion { // sizeof = 8
    long field_0;
    long distance_ratio;
};

//extern struct Proportion proportions[513];
/******************************************************************************/
#define LB_RANDOM(range,seed) LbRandomSeries(range, seed, __func__, __LINE__, "lb")
#define FIXED_POLAR_TO_X(orient,distance) ((distance * LbSinL(orient)) >> LbFPMath_TrigmBits)
#define FIXED_POLAR_TO_Y(orient,distance) ((distance * LbCosL(orient)) >> LbFPMath_TrigmBits)

/******************************************************************************/

long LbSinL(long x);
long LbCosL(long x);
long LbSqrL(long x);
long LbArcTanL(long arg);
long LbArcTanAngle(long x,long n);
long LbMathOperation(unsigned char opkind, long val1, long val2);
unsigned long LbRandomSeries(unsigned long range, unsigned long *seed, const char *func_name, unsigned long place, const char *tag);
TbBool LbNumberSignsSame(long num_a, long num_b);
char LbCompareMultiplications(long mul1a, long mul1b, long mul2a, long mul2b);
long LbDiagonalLength(long a, long b);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
