/** @file DaiPlanWorld.h
 *  The keeper's IDaiPlanWorld (12 section 13.7): filled by each gather on the main thread -- the powers' table read
 *  whole, the questions the last planning turn asked answered -- and read by the planning turn, which notes every
 *  question it asks for the next gather (a question asked every turn is answered every turn, a turn late). */
#ifndef DAIMON_DAIPLANWORLD_H
#define DAIMON_DAIPLANWORLD_H

#include "daimon_base_types.h"
#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <span>
#include <vector>

#include "daimon/IDaiPlanWorld.h"
#include "daimon/IDaiWorldReader.h"

/** Price and reach are read for levels 0 .. DAI_POWER_LEVELS - 1 (the overcharge levels). */
constexpr int64_t DAI_POWER_LEVELS = 9;

/** A live check a planning turn asked. */
struct DaiQuestion {
    enum class Kind : uint8_t { Castable, Takes };
    Kind kind = Kind::Castable;
    DaiPowerKind power = 0;
    DaiSubtlCoord x = 0, y = 0;     /**< Castable */
    DaiThingRef target;             /**< Takes */
    bool operator<(const DaiQuestion &o) const;
};

class DaiPlanWorld : public IDaiPlanWorld {
public:
    /** The gather: the powers' table, and the answers to `asked`. */
    void Gather(IDaiWorldReader &world, PlayerNumber p, std::span<const DaiQuestion> asked);
    /** Every question asked since the last call, once each, in order; the list starts again. */
    std::vector<DaiQuestion> TakeAsked();

    DaiPowerKind PowerOf(DaiPower d) const override;
    bool PowerAvailable(PlayerNumber p, DaiPowerKind k) const override;
    int64_t PowerPrice(PlayerNumber p, DaiPowerKind k, int64_t level) const override;
    int64_t PowerAreaRange(DaiPowerKind k, int64_t level, bool *own_too) const override;
    DaiAnswer PowerCastable(PlayerNumber p, DaiPowerKind k, DaiSubtlCoord x, DaiSubtlCoord y) const override;
    DaiAnswer PowerTakes(PlayerNumber p, DaiPowerKind k, const DaiThingRef &target) const override;
private:
    DaiAnswer ask(const DaiQuestion &q) const;
    bool known(DaiPowerKind k) const { return (k > 0) && (k < (DaiPowerKind)m_available.size()); }

    std::array<DaiPowerKind, (size_t)DaiPower::Count> m_kind{};
    std::vector<bool> m_available;          /**< by kind */
    std::vector<int64_t> m_price, m_range;  /**< by kind * DAI_POWER_LEVELS + level */
    std::vector<bool> m_own_too;            /**< by kind */
    std::map<DaiQuestion, bool> m_answers;
    mutable std::set<DaiQuestion> m_asked_set;
    mutable std::vector<DaiQuestion> m_asked;
};

#endif
