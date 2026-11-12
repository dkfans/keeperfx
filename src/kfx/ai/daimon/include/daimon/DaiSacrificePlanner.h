/** @file DaiSacrificePlanner.h
 *  The temple's recipe planner (08 section 7.2): pure. It emulates the engine's rule for a creature dropped on the
 *  keeper's pool (process_sacrifice_award) -- its kind's offered count goes up by one and its level into the kind's
 *  level sum; the first recipe in the table's order that has the kind among its victims and is now complete fires, at
 *  the victims' summed levels over their count (the engine's average), and only that recipe's victims' counts reset (not
 *  cheaper or costlier imps': theirs keep counting). The plan: for each recipe worth something, the drops still
 *  missing, each of the cheapest candidate of its kind, in every order -- a sequence fires nothing until its last drop,
 *  and that drop fires the recipe. Its value is the outcome's worth at that level less the victims' costs; the best
 *  positive one's first drop is the next sacrifice. A punishment (a hero, a curse, a bad unique function, a custom
 *  punishment) is never worth it and never fired on the way. */
#ifndef DAIMON_DAISACRIFICEPLANNER_H
#define DAIMON_DAISACRIFICEPLANNER_H

#include "daimon_base_types.h"
#include <array>
#include <cstdint>
#include <functional>
#include <span>

#include "daimon/DaiLimits.h"
#include "daimon/IDaiWorldReader.h"

/** A creature that may go on the pool: a prisoner, one of the keeper's own, or an imp; what giving it up costs. */
struct DaiSacrificeCandidate {
    DaiThingRef ref;
    DaiThingModel model = 0;
    int64_t level = 1;          /**< 1-based */
    int64_t cost = 0;           /**< force score */
};

/** What each outcome is worth (force score): a summoned creature by its kind's score at its level; the boons; a blessing
 *  on all its creatures, per creature that gets it. */
struct DaiSacrificeWorth {
    std::function<int64_t(DaiThingModel kind, int64_t exp_level)> creature;
    int64_t research = 0, manufacture = 0, all_happy = 0, cheaper_imps = 0;
    int64_t bless_per_creature = 0, creatures = 0;
};

/** The offered counts and their level sums, by kind (the custody look's, as the engine keeps them). */
struct DaiSacrificeCounts {
    std::array<int64_t, DAI_MAX_KINDS> n{}, levels{};
};

struct DaiSacrificeChoice {
    bool any = false;
    DaiSacrificeCandidate victim;   /**< the next drop */
    int64_t recipe = -1;            /**< the recipe the sequence completes (its place in the list given; its look's `index` is
                                     *   the game's slot, which sacrifice facts and the logs use) */
    int64_t value = 0;              /**< the outcome's worth less the victims' costs */
    int64_t drops = 0;              /**< drops left, this one included */
};

class DaiSacrificePlanner {
public:
    static DaiSacrificeChoice Plan(std::span<const DaiRecipeLook> recipes, const DaiSacrificeCounts &counts,
                                   std::span<const DaiSacrificeCandidate> candidates, const DaiSacrificeWorth &worth);
    /** One drop of `kind` at `level` (1-based): the recipe it fires (-1: none) and that recipe's summoned level
     *  (0-based, *exp_level); `counts` as the engine leaves them. */
    static int64_t Drop(std::span<const DaiRecipeLook> recipes, DaiSacrificeCounts &counts, DaiThingModel kind, int64_t level, int64_t *exp_level);
    /** A recipe's outcome at that level; INT64_MIN for a punishment. */
    static int64_t Worth(const DaiRecipeLook &r, int64_t exp_level, const DaiSacrificeWorth &worth);
};

#endif
