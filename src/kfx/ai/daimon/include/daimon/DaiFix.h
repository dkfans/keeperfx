/** @file DaiFix.h
 *  Fixed point for utilities, weights and rates: int64_t x1024, rounding toward zero (02 section 7). Decisions use
 *  no floating point (principle 6). */
#ifndef DAIMON_DAIFIX_H
#define DAIMON_DAIFIX_H

#include <cstdint>

typedef int64_t DaiFix;
constexpr DaiFix DAI_FIX_ONE = 1024;

constexpr DaiFix FixFromInt(int64_t v) { return v * DAI_FIX_ONE; }
constexpr int64_t FixToInt(DaiFix v) { return v / DAI_FIX_ONE; }
constexpr DaiFix FixMul(DaiFix a, DaiFix b) { return (a * b) / DAI_FIX_ONE; }
constexpr DaiFix FixDiv(DaiFix a, DaiFix b) { return (b == 0) ? 0 : (a * DAI_FIX_ONE) / b; }

#endif
