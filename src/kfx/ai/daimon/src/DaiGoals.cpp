#include "pre_inc.h"
#include "daimon/DaiGoals.h"

#include <cstdio>

#include "post_inc.h"

uint32_t DaiGoals::Set(const DaiGoal &g, DaiTurn now)
{
    DaiGoal *found = nullptr;
    m_goals.ForEach([&](DaiGoal &o) { if ((found == nullptr) && (o.kind == g.kind) && (o.p.key == g.p.key)) found = &o; });
    if (found == nullptr) {
        found = m_goals.Add();
        if (found == nullptr) return 0;
        found->status = DaiGoalStatus::Open;
        found->status_turn = now;
        found->set_turn = now;
    }
    // A level script's goal (Phase 7) outranks the director's, an agent's directive (P8.4) both: a lower source's Set of
    // the same goal leaves it as the higher one set it.
    auto rank = [](DaiGoalSource s) { return (s == DaiGoalSource::Directive) ? 2 : (s == DaiGoalSource::Script) ? 1 : 0; };
    if (rank(found->source) > rank(g.source)) return found->id;
    found->kind = g.kind;
    found->weight = g.weight;
    found->deadline = g.deadline;
    found->source = g.source;
    found->p = g.p;
    return found->id;
}

const DaiGoal *DaiGoals::Find(DaiGoalKind k, int64_t key) const
{
    const DaiGoal *found = nullptr;
    m_goals.ForEach([&](const DaiGoal &o) { if ((found == nullptr) && (o.kind == k) && (o.p.key == key)) found = &o; });
    return found;
}

void DaiGoals::NoteStatus(uint32_t id, DaiGoalStatus s, const char *why, DaiTurn now)
{
    DaiGoal *g = m_goals.Find(id);
    if (g == nullptr) return;
    if (g->status != s) g->status_turn = now;
    g->status = s;
    snprintf(g->why, sizeof(g->why), "%s", (why != nullptr) ? why : "");
}

const char *DaiGoalKindName(DaiGoalKind k)
{
    static const char *const names[] = { "keep_imps", "keep_gold", "mine", "expand_to", "build", "train", "budget",
                                         "defend", "attack", "forbid", "custody", "research", "manufacture" };
    return (k < DaiGoalKind::Count) ? names[(size_t)k] : "?";
}

const char *DaiGoalStatusName(DaiGoalStatus s)
{
    switch (s) {
    case DaiGoalStatus::Open: return "open";
    case DaiGoalStatus::Progress: return "in progress";
    case DaiGoalStatus::Met: return "met";
    case DaiGoalStatus::Blocked: return "blocked";
    }
    return "?";
}
