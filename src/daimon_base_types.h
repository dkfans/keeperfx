/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_base_types.h
 *     The integer types Daimon's core and its engine-facing headers use for map places, things, models, powers and
 *     turns, at fixed widths.
 * @par Purpose:
 *     The engine's own typedefs for these (MapSlabCoord, ThingIndex, GameTurn, ...) have other widths in upstream
 *     KeeperFX, some unsigned there: a core that used them would compute differently when ported. These are the
 *     fork's widths, the same in every tree; the seam converts at the engine boundary
 *     (docs/plans/ai-plans/v2/16-daimon-port.md; scripts/check_daimon_modules.py keeps the engine names out of the
 *     core).
 */
/******************************************************************************/
#ifndef DK_DAIMON_BASE_TYPES_H
#define DK_DAIMON_BASE_TYPES_H

#include <stdint.h>

/** A slab coordinate (MapSlabCoord's role). */
typedef int64_t DaiSlabCoord;
/** A subtile coordinate (MapSubtlCoord's role). */
typedef int64_t DaiSubtlCoord;
/** A thing's index (ThingIndex's role). */
typedef int64_t DaiThingIndex;
/** A creature, object, trap or door model (ThingModel's role). */
typedef int64_t DaiThingModel;
/** A keeper power's kind (PowerKind's role). */
typedef int64_t DaiPowerKind;
/** A game turn (GameTurn's role). */
typedef uint64_t DaiTurn;

/** Subtiles per slab and map units per subtile (the engine's STL_PER_SLB and COORD_PER_STL, the same in every tree):
 *  enum constants, so C files see them too. */
enum { DAI_STL_PER_SLB = 3, DAI_COORD_PER_STL = 256 };

#endif
