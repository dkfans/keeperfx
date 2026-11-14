/** @file DaiAbilities.h
 *  What a creature kind's abilities are worth at each level (P9): each a damage-equivalent of a use (dealt, or kept
 *  from it: a disable, a buff, a heal), the uses fitted into the creature's 100 turns by what a turn of casting is
 *  worth (a use holds it for the instance's time, and comes again after its reset), melee in the same 100 turns;
 *  travel and sight a worth of their own, outside the fight. The reader fills the specs from the config
 *  (DaiWorldReaderSim); this is the pure half, in whole numbers (the simulation's determinism).
 */
#ifndef DAIMON_DAIABILITIES_H
#define DAIMON_DAIABILITIES_H

#include "daimon_base_types.h"
#include <cstdint>

#include "globals.h"
#include "daimon/DaiFix.h"

constexpr int64_t DAI_ABILITY_LEVELS = 10;
constexpr int64_t DAI_ABILITY_MAX = 16;

/** One ability its kind learns. */
struct DaiAbilitySpec {
    int64_t level = 1;          /**< learnt at (1-based) */
    int64_t time = 10;          /**< turns a use holds the creature */
    int64_t cycle = 10;         /**< turns from one use to the next at most (time and reset) */
    bool flat = false;          /**< travel or sight: worth[] is 100 turns' worth, not a use's */
    bool shot = false;          /**< fires a shot (melee swings too): what a door takes */
    bool magical = false;       /**< its shot is magical: a magic door takes it in full, other hits an eighth */
    int64_t worth[DAI_ABILITY_LEVELS] = {};     /**< a use at each level (0-based), damage-equivalent */
};

/** How hard its kind is to lose: reborn at its lair (a level lower) when killed from level `level` (the config's
 *  Rebirth; 0 never), its output worth `share` (DaiFix) more from then on (P9: a vampire). */
struct DaiRebirth {
    int64_t level = 0;
    DaiFix share = 0;
};

/** Its output in 100 turns at `level` (1-based), with the abilities (and the rebirth) learnt by level `upto`. */
int64_t DaiAbilityOutput(const DaiAbilitySpec *a, int64_t n, int64_t level, int64_t upto, const DaiRebirth &rebirth = DaiRebirth());
/** By level (0-based): the output, and the share of it (DaiFix) the abilities (and the rebirth) of the next two levels
 *  add. */
void DaiAbilityTable(const DaiAbilitySpec *a, int64_t n, int64_t output[DAI_ABILITY_LEVELS], DaiFix gain[DAI_ABILITY_LEVELS],
                     const DaiRebirth &rebirth = DaiRebirth());

/** Of its shots' worth in 100 turns at `level` (the uses Output schedules), the magical share (DaiFix; 0 with no shot):
 *  what a magic door takes in full (Phase 7, doors by effective health). */
DaiFix DaiAbilityMagicShare(const DaiAbilitySpec *a, int64_t n, int64_t level);
/** The game's (DaiAbilitiesSim.cpp): a kind's abilities from its config, at most max; how many. */
int64_t DaiAbilitySpecsOfKind(DaiThingModel m, DaiAbilitySpec *out, int64_t max);
/** The game's: a trap kind's worth in 100 turns while triggered (a one-shot trap: its shot), the same scale. */
int64_t DaiTrapRating(DaiThingModel trap);

#endif
