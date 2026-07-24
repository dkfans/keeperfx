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
/** Amount of fractional bits in resulting values of trigonometric operations. */
#define LbFPMath_TrigmBits 16

#define CEILING_POS(X) ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X))
#define CEILING_NEG(X) ((X-(int)(X)) < 0 ? (int)(X-1) : (int)(X))
#define CEILING(X) ( ((X) > 0) ? CEILING_POS(X) : CEILING_NEG(X) )

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
    int32_t base_value;
    int32_t distance_ratio;
};

//extern struct Proportion proportions[513];
/******************************************************************************/
#define LB_RANDOM(range,seed) LbRandomSeries(range, seed, __func__, __LINE__)

/******************************************************************************/

int32_t LbSinL(int32_t x);
int32_t LbCosL(int32_t x);
int32_t LbSqrL(int32_t x);
int32_t LbArcTanAngle(int32_t x,int32_t y);
int32_t LbMathOperation(unsigned char opkind, int32_t first_operand, int32_t second_operand);
uint32_t LbRandomSeries(uint32_t range, uint32_t *seed, const char *func_name, uint32_t place);
TbBool LbNumberSignsSame(int32_t num_a, int32_t num_b);
char LbCompareMultiplications(int32_t mul1a, int32_t mul1b, int32_t mul2a, int32_t mul2b);
int32_t LbDiagonalLength(int32_t a, int32_t b);
float LbLerp(float low, float high, float interval);
float LbFmodf(float x, float y);
float lerp_angle(float from, float to, float weight);
double fastPow(double a, double b);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
