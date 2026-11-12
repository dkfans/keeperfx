/** @file DaiStockPlanner.h
 *  The stock planner (02 section 4.13b, 04 section 9), used by Construction: what the keeper wants made in its
 *  workshop against what it has. Today the workshop builds whatever item it has fewest of and the keeper cannot
 *  choose (`DaiWorkshopMode::Auto`), so the one lever is how much work goes in: the planner turns the shortfall into a
 *  number of workshop workers, which Construction asks of the director as the Manufacture goal's count. Workshop
 *  requests and purchases (a planned engine feature) plug in here when they exist. */
#ifndef DAIMON_DAISTOCKPLANNER_H
#define DAIMON_DAISTOCKPLANNER_H

#include <cstdint>
#include <span>

#include "daimon/DaiKnowledge.h"

/** An item wanted in stock: a door or trap kind (the game's, 1-based) and how many. Phase 2: a door a room entrance. */
struct DaiStockWant {
    bool door = true;
    int64_t kind = 0;
    int64_t count = 0;
};

struct DaiStockPlan {
    int64_t short_items = 0;    /**< wanted and not in stock, of kinds the workshop can make */
    int64_t workers = 0;        /**< workshop workers wanted (Auto mode) */
};

/** Workers a missing item asks for, and the most the planner asks for at once. */
constexpr int64_t DAI_STOCK_WORKERS_PER_ITEM = 2, DAI_STOCK_MAX_WORKERS = 6;

class DaiStockPlanner {
public:
    /** Auto mode: the shortfall of `wants` against what the keeper may place now, and the workers for it (none while
     *  the keeper has no workshop to work in). */
    static DaiStockPlan Plan(const DaiKnowledge &k, std::span<const DaiStockWant> wants);
    /** The first door kind the workshop can make (0: none). */
    static int64_t DoorToMake(const DaiKnowledge &k);
    /** The first door kind the keeper has one of to place (0: none). */
    static int64_t DoorInStock(const DaiKnowledge &k);
};

#endif
