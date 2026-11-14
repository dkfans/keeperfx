/** @file DaiForecast.h
 *  The economy forecast (02 section 4.12, 03 section 6): from what the keeper knows of its own gold, its claims and
 *  its income, how much it may spend now and whether the next pay day is covered. Pure: its only history is the
 *  income moving average, which Economy owns and saves (Phase 1, P1.5) and updates once a turn with UpdateEma. No
 *  floating point, no randomness. Readers: Economy, Workforce, every gold claim (it must fit in `spendable`). */
#ifndef DAIMON_DAIFORECAST_H
#define DAIMON_DAIFORECAST_H

#include "daimon_base_types.h"
#include <algorithm>
#include <cstdint>

#include "globals.h"
#include "config_daimon.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiGoldSides.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiLedger.h"
#include "daimon/DaiPersonality.h"

/** Turns between income samples, and the moving average's time constant: a sample `dt` turns long moves the
 *  average dt / (dt + DAI_INCOME_TAU) of the way to the sample's rate. */
constexpr int64_t DAI_INCOME_SAMPLE = 100;
constexpr int64_t DAI_INCOME_TAU = 2000;
/** "Never" for turns_to_payday (pay day does not come: its speed is below 100), and the most pay days runway counts. */
constexpr int64_t DAI_NEVER = -1;
constexpr int64_t DAI_RUNWAY_MAX = 99;

/** The income moving average: gold mined a 1000 turns (sales join in Phase 2). */
struct DaiIncomeEma {
    DaiFix income_per_kturn = 0;
    DaiTurn last = 0;          /**< the turn of the last sample */
    int64_t last_mined = 0;     /**< gold_mined then */
    int64_t samples = 0;        /**< 0: not started; 1: started, no rate yet */
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
};

struct DaiForecastOut {
    DaiTurn now = 0;               /**< the turn it was worked out */
    int64_t gold_now = 0;           /**< spendable gold: off-map and the treasuries' (total_money_owned) */
    int64_t gold_pending = 0;       /**< loose and carried gold, as much as the treasuries have room for */
    int64_t wages_next = 0;         /**< the wage bill at the next pay day */
    int64_t turns_to_payday = 0;    /**< DAI_NEVER if pay day does not come */
    int64_t committed = 0;          /**< the ledger's active gold claims */
    int64_t reserve = 0;            /**< wages and margin, kept back once pay day is within the reserve window */
    int64_t spendable = 0;          /**< gold_now - committed - reserve (may be below 0: nothing may be claimed) */
    int64_t runway_paydays = 0;     /**< future pay days the gold and income cover, at most DAI_RUNWAY_MAX */
    DaiFix income_per_kturn = 0;
    int64_t upkeep_per_kturn = 0;   /**< training and scavenging, what the creatures doing it cost a 1000 turns (Phase 2),
                                     *   and Must Obey while it is on (P9) */
    bool short_next = false;        /**< the next pay day's wages are not covered by gold and income until then */
    // The gold outlook (P9, plan 13): the gold the keeper knows of is what a human sees in that seat -- every gold, dense
    // gold and gem slab from the start (the engine reveals them to every keeper), gone to shroud once mined out of
    // sight -- so its veins are the map's gold. The income lasts while its side's gold does, or for ever with a gem.
    int64_t map_gold = 0;           /**< the gold left in every vein it knows */
    int64_t side_gold = 0;          /**< its share: by the routes from the hearts (DaiGoldSides) -- veins its heart's route
                                     *   reaches first, a share of those others reach within gold_contest_share %; a vein
                                     *   the table does not have yet by the straight line; with no rival heart seen,
                                     *   map_gold over the keepers */
    const DaiGoldSides *sides = nullptr;    /**< the side table it was worked out with (nullptr: the straight line) */
    bool gem_held = false;          /**< a gem face beside its own ground: an income without end */
    bool gold_known = false;        /**< the vein table holds the map's gold (not full; empty after mining: none left):
                                     *   else the income is not capped */
    int64_t horizon_turns = DAI_NEVER;  /**< turns the income lasts at its average, side_gold mined (0: its side's gold
                                         *   gone; DAI_NEVER: a gem, no income, or the gold not known) */
    int64_t bank_target = 0;        /**< gold to keep in hand: bank_paydays pay days' wages */
    int64_t spare = 0;              /**< gold_now - committed - the larger of reserve and bank_target: gold to put to work */
    /** The spare gold in pay days' wages (at least 1,000 a pay day): what "gold in hand" thresholds read (plan 13). */
    int64_t SparePaydays() const { return spare / std::max<int64_t>(wages_next, 1000); }
    /** The income ends within `turns` (no gem held, the gold known). */
    bool HorizonWithin(int64_t turns) const { return (horizon_turns != DAI_NEVER) && (horizon_turns < turns); }
};

/** Whose a vein's gold is (plan 13): the keeper's alone, shared with a rival about as near (within gold_contest_share
 *  %), a rival's -- by the routes from the hearts (DaiGoldSides) where the table has the vein, else by the straight
 *  line; Unknown without its heart or a rival heart seen. */
enum class DaiVeinSide : uint8_t { Unknown, Own, Contested, Theirs };

class DaiForecast {
public:
    static void Compute(const DaiKnowledge &k, const DaiLedger &l, const DaiPersonality &p, const DaimonGlobalConfig &cfg,
                        const DaiIncomeEma &ema, DaiTurn now, DaiForecastOut *out, const DaiGoldSides *sides = nullptr);
    /** Takes a sample when DAI_INCOME_SAMPLE turns have passed since the last; a fall in gold_mined (a new level)
     *  starts the average over. */
    static void UpdateEma(DaiIncomeEma &ema, const DaiKnowledge &k, DaiTurn now);
    /** Turns until pay day from its progress, gap and speed (the game adds speed / 100 a turn); DAI_NEVER if it does
     *  not come. */
    static int64_t TurnsToPayday(int64_t progress, int64_t gap, int64_t speed);
    static DaiVeinSide VeinSide(const DaiKnowledge &k, const DaiPersonality &p, const DaiVein &v, const DaiGoldSides *sides = nullptr);
};

#endif
