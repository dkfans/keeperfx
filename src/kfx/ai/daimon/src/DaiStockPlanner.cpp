#include "pre_inc.h"
#include "daimon/DaiStockPlanner.h"

#include <algorithm>

#include "post_inc.h"

DaiStockPlan DaiStockPlanner::Plan(const DaiKnowledge &k, std::span<const DaiStockWant> wants)
{
    DaiStockPlan plan;
    const DaiStockLook &st = k.Stock();
    for (const DaiStockWant &w : wants) {
        if (st.Slot(w.door, w.kind) < 0) continue;
        if (!st.Buildable(w.door, w.kind)) continue;
        const int64_t have = st.Placeable(w.door, w.kind);
        plan.short_items += std::max<int64_t>(w.count - have, 0);
    }
    bool workshop = false;
    workshop = workshop || k.HasRoomRole(DAI_ROLE_MANUFACTURE);
    if (workshop) plan.workers = std::min(plan.short_items * DAI_STOCK_WORKERS_PER_ITEM, DAI_STOCK_MAX_WORKERS);
    return plan;
}

int64_t DaiStockPlanner::DoorToMake(const DaiKnowledge &k)
{
    const DaiStockLook &st = k.Stock();
    int64_t kind = 0;
    st.ForEachKind(true, [&](int64_t s, int64_t m) { if ((kind == 0) && st.doors_buildable[s]) kind = m; });
    return kind;
}

int64_t DaiStockPlanner::DoorInStock(const DaiKnowledge &k)
{
    const DaiStockLook &st = k.Stock();
    int64_t kind = 0;
    st.ForEachKind(true, [&](int64_t s, int64_t m) { if ((kind == 0) && (st.doors_placeable[s] > 0)) kind = m; });
    return kind;
}
