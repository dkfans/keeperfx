/** @file IDaiPlanWorld.h
 *  What a planning turn may ask the game itself (12 section 13.7), everything else coming from its knowledge: the
 *  powers' table -- each power's kind, whether it is available, its price at each level, an area power's reach -- as the
 *  gather read it; and two live checks, whether a power can be cast at a spot and whether a spell would take on a
 *  creature. A planning turn cannot ask the game a live question (it may run on a worker thread), so a check is a
 *  question: answered from the gather when that gather answered it, Unknown when not -- the next gather answers it. */
#ifndef DAIMON_IDAIPLANWORLD_H
#define DAIMON_IDAIPLANWORLD_H

#include <cstdint>

#include "globals.h"
#include "daimon/DaiTypes.h"

enum class DaiAnswer : uint8_t { Unknown, No, Yes };

class IDaiPlanWorld {
public:
    virtual ~IDaiPlanWorld() = default;
    virtual DaiPowerKind PowerOf(DaiPower d) const = 0;
    virtual bool PowerAvailable(PlayerNumber p, DaiPowerKind k) const = 0;
    virtual int64_t PowerPrice(PlayerNumber p, DaiPowerKind k, int64_t level) const = 0;
    /** An area power's reach at that level (map coordinates, 0: not an area power) and whether it hits the keeper's own. */
    virtual int64_t PowerAreaRange(DaiPowerKind k, int64_t level, bool *own_too) const = 0;
    /** The power castable at a subtile, as a human's cursor would find it. */
    virtual DaiAnswer PowerCastable(PlayerNumber p, DaiPowerKind k, DaiSubtlCoord x, DaiSubtlCoord y) const = 0;
    /** The spell would take on that creature (it is there, the keeper sees it, it has not got it, is not immune). */
    virtual DaiAnswer PowerTakes(PlayerNumber p, DaiPowerKind k, const DaiThingRef &target) const = 0;
};

#endif
