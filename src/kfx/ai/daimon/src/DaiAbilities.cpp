#include "pre_inc.h"
#include "daimon/DaiAbilities.h"

#include <algorithm>

#include "post_inc.h"

namespace {
/** The uses of 100 turns at level index lv with abilities learnt by `upto`, best worth a turn of casting first: each
 *  ability's worth over those uses (in the out array), and the flat worth. */
int64_t schedule(const DaiAbilitySpec *a, int64_t n, int64_t lv, int64_t upto, int64_t *each)
{
    int64_t order[DAI_ABILITY_MAX];
    int64_t m = 0, flat = 0;
    for (int64_t i = 0; (i < n) && (i < DAI_ABILITY_MAX); i++) {
        each[i] = 0;
        if ((a[i].level > upto) || (a[i].worth[lv] <= 0)) continue;
        if (a[i].flat) {
            flat += a[i].worth[lv];
            continue;
        }
        order[m++] = i;
    }
    std::stable_sort(order, order + m, [a, lv](int64_t x, int64_t y) {
        return a[x].worth[lv] * std::max<int64_t>(a[y].time, 1) > a[y].worth[lv] * std::max<int64_t>(a[x].time, 1);
    });
    int64_t time_left = 100 * 1000;
    for (int64_t k = 0; (k < m) && (time_left > 0); k++) {
        const DaiAbilitySpec &s = a[order[k]];
        const int64_t time = std::max<int64_t>(s.time, 1);
        const int64_t uses = std::min<int64_t>(100 * 1000 / std::max<int64_t>(s.cycle, time), time_left / time);
        each[order[k]] = uses * s.worth[lv] / 1000;
        time_left -= uses * time;
    }
    return flat;
}
}

DaiFix DaiAbilityMagicShare(const DaiAbilitySpec *a, int64_t n, int64_t level)
{
    const int64_t lv = std::clamp<int64_t>(level, 1, DAI_ABILITY_LEVELS) - 1;
    int64_t each[DAI_ABILITY_MAX] = {};
    schedule(a, n, lv, level, each);
    int64_t shots = 0, magic = 0;
    for (int64_t i = 0; (i < n) && (i < DAI_ABILITY_MAX); i++) {
        if (!a[i].shot) continue;
        shots += each[i];
        if (a[i].magical) magic += each[i];
    }
    return (shots > 0) ? FixDiv(FixFromInt(magic), FixFromInt(shots)) : 0;
}

int64_t DaiAbilityOutput(const DaiAbilitySpec *a, int64_t n, int64_t level, int64_t upto, const DaiRebirth &rebirth)
{
    const int64_t lv = std::clamp<int64_t>(level, 1, DAI_ABILITY_LEVELS) - 1;
    // The uses, best worth a turn of casting first (the earlier listed on a tie), each at most once a cycle, in the
    // 100 turns there are (in thousandths of a use and of a turn).
    int64_t each[DAI_ABILITY_MAX] = {};
    const int64_t flat = schedule(a, n, lv, upto, each);
    int64_t total = 0;
    for (int64_t i = 0; (i < n) && (i < DAI_ABILITY_MAX); i++) total += each[i];
    total += flat;
    if ((rebirth.level > 0) && (rebirth.level <= upto)) total += FixToInt(FixMul(FixFromInt(total), rebirth.share));
    return total;
}

void DaiAbilityTable(const DaiAbilitySpec *a, int64_t n, int64_t output[DAI_ABILITY_LEVELS], DaiFix gain[DAI_ABILITY_LEVELS],
                     const DaiRebirth &rebirth)
{
    for (int64_t lv = 1; lv <= DAI_ABILITY_LEVELS; lv++) output[lv - 1] = DaiAbilityOutput(a, n, lv, lv, rebirth);
    for (int64_t lv = 1; lv <= DAI_ABILITY_LEVELS; lv++) {
        // At two levels on: with what those levels bring, against without.
        const int64_t on = std::min<int64_t>(lv + 2, DAI_ABILITY_LEVELS);
        const int64_t added = output[on - 1] - DaiAbilityOutput(a, n, on, lv, rebirth);
        gain[lv - 1] = (added > 0) ? FixDiv(FixFromInt(added), FixFromInt(std::max<int64_t>(output[lv - 1], 1))) : 0;
    }
}
