#include "pre_inc.h"
#include "daimon/DaiSacrificePlanner.h"

#include <algorithm>
#include <climits>
#include <vector>

#include "post_inc.h"

namespace {
int64_t victims_of(const DaiRecipeLook &r, DaiThingModel kind)
{
    int64_t k = 0;
    for (DaiThingModel v : r.victims) k += (v == kind) ? 1 : 0;
    return k;
}

bool complete(const DaiRecipeLook &r, const DaiSacrificeCounts &c)
{
    for (DaiThingModel v : r.victims) {
        if (v < 1) continue;
        if ((v >= DAI_MAX_KINDS) || (c.n[(size_t)v] < victims_of(r, v))) return false;
    }
    return true;
}

/** The engine's creature_sacrifice_average_exp_level: the victims' kinds in order, stopping at a kind repeated straight
 *  after itself; their level sums over their counts. */
int64_t average_level(const DaiRecipeLook &r, const DaiSacrificeCounts &c)
{
    int64_t num = 0, sum = 0;
    for (int64_t i = 0; i < DaiRecipeLook::MAX_VICTIMS; i++) {
        const DaiThingModel m = r.victims[i];
        if ((i > 0) && (m == r.victims[i - 1])) break;
        if ((m < 0) || (m >= DAI_MAX_KINDS)) continue;
        num += c.n[(size_t)m];
        sum += c.levels[(size_t)m];
    }
    if (num < 1) num = 1;
    return std::max<int64_t>(0, std::min<int64_t>(sum / num, DaiKindLook::LEVELS - 1));
}

bool resets(const DaiRecipeLook &r)
{
    // Cheaper (and costlier) imps leave the counts as they are (the engine's Pleased and AngryWarn results).
    return !((r.action == DaiRecipeAction::Boon) && (r.boon == DaiBoon::CheaperImps));
}
}

int64_t DaiSacrificePlanner::Worth(const DaiRecipeLook &r, int64_t exp_level, const DaiSacrificeWorth &w)
{
    switch (r.action) {
    case DaiRecipeAction::MakeCreature: return w.creature ? w.creature(r.creature, exp_level) : 0;
    case DaiRecipeAction::BlessAll: return w.bless_per_creature * w.creatures;
    case DaiRecipeAction::Boon:
        switch (r.boon) {
        case DaiBoon::CompleteResearch: return w.research;
        case DaiBoon::CompleteManufacture: return w.manufacture;
        case DaiBoon::CheaperImps: return w.cheaper_imps;
        case DaiBoon::AllHappy: return w.all_happy;
        default: return 0;
        }
    case DaiRecipeAction::CustomReward: return 0;   // a level script's own reward: unknown, never pursued
    case DaiRecipeAction::None: return 0;
    default: return INT64_MIN;                      // a hero, a curse, a bad unique function, a custom punishment
    }
}

int64_t DaiSacrificePlanner::Drop(std::span<const DaiRecipeLook> recipes, DaiSacrificeCounts &c, DaiThingModel kind, int64_t level, int64_t *exp_level)
{
    if ((kind < 1) || (kind >= DAI_MAX_KINDS)) return -1;
    c.n[(size_t)kind]++;
    c.levels[(size_t)kind] += level;
    for (size_t i = 0; i < recipes.size(); i++) {
        const DaiRecipeLook &r = recipes[i];
        if ((victims_of(r, kind) == 0) || !complete(r, c)) continue;
        if (exp_level != nullptr) *exp_level = average_level(r, c);
        if (resets(r)) {
            for (DaiThingModel v : r.victims) {
                if ((v < 0) || (v >= DAI_MAX_KINDS)) continue;
                c.n[(size_t)v] = 0;
                c.levels[(size_t)v] = 0;
            }
        }
        return (int64_t)i;
    }
    return -1;
}

DaiSacrificeChoice DaiSacrificePlanner::Plan(std::span<const DaiRecipeLook> recipes, const DaiSacrificeCounts &counts,
                                             std::span<const DaiSacrificeCandidate> candidates, const DaiSacrificeWorth &worth)
{
    DaiSacrificeChoice best;
    for (size_t ri = 0; ri < recipes.size(); ri++) {
        const DaiRecipeLook &r = recipes[ri];
        if (Worth(r, 0, worth) == INT64_MIN) continue;
        // The kinds still missing, each once per victim short (complete already: one more of any of its kinds).
        std::vector<DaiThingModel> need;
        for (DaiThingModel v : r.victims) {
            if ((v < 1) || (v >= DAI_MAX_KINDS) || (std::find(need.begin(), need.end(), v) != need.end())) continue;
            for (int64_t k = counts.n[(size_t)v]; k < victims_of(r, v); k++) need.push_back(v);
        }
        if (need.empty() && (r.victims[0] > 0)) need.push_back(r.victims[0]);
        if (need.empty()) continue;
        // The cheapest candidates of each kind, one per drop.
        std::vector<const DaiSacrificeCandidate *> pick;
        std::vector<bool> used(candidates.size(), false);
        for (DaiThingModel v : need) {
            const DaiSacrificeCandidate *b = nullptr;
            size_t bi = 0;
            for (size_t i = 0; i < candidates.size(); i++) {
                if (used[i] || (candidates[i].model != v)) continue;
                if ((b == nullptr) || (candidates[i].cost < b->cost)) { b = &candidates[i]; bi = i; }
            }
            if (b == nullptr) break;
            used[bi] = true;
            pick.push_back(b);
        }
        if (pick.size() != need.size()) continue;
        int64_t cost = 0;
        for (const DaiSacrificeCandidate *p : pick) cost += p->cost;
        // Every order of the drops: nothing fires before the last, which fires this recipe.
        std::vector<size_t> order(pick.size());
        for (size_t i = 0; i < order.size(); i++) order[i] = i;
        std::sort(order.begin(), order.end(), [&](size_t a, size_t b) { return pick[a]->model < pick[b]->model; });
        do {
            DaiSacrificeCounts c = counts;
            bool ok = true;
            int64_t lv = 0;
            for (size_t s = 0; ok && (s < order.size()); s++) {
                const DaiSacrificeCandidate &d = *pick[order[s]];
                const int64_t fired = Drop(recipes, c, d.model, d.level, &lv);
                ok = (s + 1 < order.size()) ? (fired < 0) : (fired == (int64_t)ri);
            }
            if (!ok) continue;
            const int64_t value = Worth(r, lv, worth) - cost;
            if ((value > 0) && (!best.any || (value > best.value))) {
                best.any = true;
                best.victim = *pick[order[0]];
                best.recipe = (int64_t)ri;
                best.value = value;
                best.drops = (int64_t)order.size();
            }
        } while (std::next_permutation(order.begin(), order.end(), [&](size_t a, size_t b) { return pick[a]->model < pick[b]->model; }));
    }
    return best;
}
