/** @file DaiGeom.h
 *  The slab grid's small geometry, one copy (14 K1): the Chebyshev (king's move) distance and the four neighbours in
 *  N, E, S, W order. A site whose ties are broken by another order keeps its own table (DaiDefence's E, W, S, N). */
#ifndef DAIMON_DAIGEOM_H
#define DAIMON_DAIGEOM_H

#include <cstdint>

#include "daimon/DaiTypes.h"

/** The king's-move distance between two slabs. */
inline int64_t DaiChess(DaiSlabCoord ax, DaiSlabCoord ay, DaiSlabCoord bx, DaiSlabCoord by)
{
    const int64_t dx = (ax > bx) ? (int64_t)ax - bx : (int64_t)bx - ax, dy = (ay > by) ? (int64_t)ay - by : (int64_t)by - ay;
    return (dx > dy) ? dx : dy;
}

/** The four neighbours, N, E, S, W: as offsets, as pairs and as slab steps. */
constexpr int DAI_N4_DX[4] = { 0, 1, 0, -1 };
constexpr int DAI_N4_DY[4] = { -1, 0, 1, 0 };
constexpr int8_t DAI_N4_PAIRS[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
constexpr DaiSlabXY DAI_N4_STEPS[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };

#endif
