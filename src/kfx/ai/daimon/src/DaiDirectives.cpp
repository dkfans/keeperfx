#include "pre_inc.h"
#include "daimon/DaiDirectives.h"

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <string>

#include "daimon/DaiDirector.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiRivals.h"
#include "post_inc.h"

namespace {
std::string lower(const char *s)
{
    std::string out = (s != nullptr) ? s : "";
    for (char &c : out) c = (char)std::tolower((unsigned char)c);
    return out;
}

bool same_goal(const DaiDirective &o, const DaiGoal &g)
{
    return (g.kind == o.goal_kind) && (g.p.key == o.goal_key) && (g.source == DaiGoalSource::Directive);
}

DaiGoal goal_of(const DaiDirective &d)
{
    DaiGoal g;
    g.kind = d.goal_kind;
    g.p = d.params;
    g.weight = d.weight;
    g.source = DaiGoalSource::Directive;
    return g;
}
}

DaiFix DaiDirectives::Weight(int64_t priority)
{
    const int64_t p = (priority <= 0) ? 2 : std::min<int64_t>(priority, 4);
    return (DaiFix)(DAI_FIX_ONE * p);
}

bool DaiDirectives::Live(const DaiDirective &d, DaiTurn now)
{
    return !d.expired && ((d.expires == 0) || (d.expires > now));
}

const char *DaiDirectives::Check(const DaiDirectiveMap &m, const DaiDirectiveIn &d, DaiGoal *g, DaiDirectiveEffect *effect)
{
    *g = DaiGoal();
    *effect = DaiDirectiveEffect::None;
    if ((d.id == nullptr) || (d.id[0] == '\0')) return "MISSING_ID";
    g->source = DaiGoalSource::Directive;
    g->weight = Weight(d.priority);
    const std::string kind = lower(d.kind);
    if (kind == "build") {
        DaiRoom r = DaiRoom::Count;
        for (size_t i = 0; i < (size_t)DaiRoom::Count; i++) if ((d.room_kind > 0) && (m.rooms[i] == d.room_kind)) r = (DaiRoom)i;
        if (r == DaiRoom::Count) return "BAD_ROOM";
        if ((d.count < 0) || (d.count > 400)) return "BAD_COUNT";
        g->kind = DaiGoalKind::Build;
        g->p.key = (int64_t)r;                  // the director's own room goal, taken over
        g->p.count = (d.count > 0) ? d.count : 9;
    } else if (kind == "expand_to") {
        if ((d.x < 0) || (d.y < 0) || (d.x >= m.width) || (d.y >= m.height)) return "BAD_POSITION";
        g->kind = DaiGoalKind::ExpandTo;
        g->p.count = DAI_EXPAND_POINT;
        g->p.at = DaiRect{ (DaiSlabCoord)d.x, (DaiSlabCoord)d.y, (DaiSlabCoord)d.x, (DaiSlabCoord)d.y };
        g->p.key = d.y * std::max<int64_t>(m.width, 1) + d.x;
    } else if (kind == "keep") {
        if ((d.imps > 0) == (d.paydays > 0)) return "BAD_COUNT";     // one of the two
        if ((d.imps > 200) || (d.paydays > 20)) return "BAD_COUNT";
        g->kind = (d.imps > 0) ? DaiGoalKind::KeepImps : DaiGoalKind::KeepGold;
        g->p.count = (d.imps > 0) ? d.imps : d.paydays;
    } else if (kind == "defend") {
        g->kind = DaiGoalKind::Defend;
        g->p.key = DAI_DEFEND_HOME;
    } else if (kind == "attack") {
        if ((d.rival < 0) || (d.rival >= DAI_MAX_PLAYERS) || ((PlayerNumber)d.rival == m.self)) return "BAD_RIVAL";
        const std::string obj = lower(d.objective);
        g->kind = DaiGoalKind::Attack;
        g->p.key = d.rival + 1;
        // The goal's aim (14 R4): the target kind the rival scoring prefers for this rival, over the personality's.
        if (obj.empty() || (obj == "any")) g->p.aim = -1;
        else if (obj == "heart") g->p.aim = (int64_t)DaiTargetKind::Heart;
        else if (obj == "portal") g->p.aim = (int64_t)DaiTargetKind::Portal;
        else if (obj == "rooms") g->p.aim = (int64_t)DaiTargetKind::Room;
        else return "BAD_OBJECTIVE";
    } else if (kind == "forbid") {
        if (lower(d.action) != "dismiss") return "BAD_ACTION";
        g->kind = DaiGoalKind::Forbid;
        g->p.key = DAI_FORBID_DISMISS;
    } else if (kind == "perimeter") {
        const std::string a = lower(d.action);
        if ((a != "open") && (a != "close")) return "BAD_ACTION";
        *effect = (a == "open") ? DaiDirectiveEffect::PerimeterOpen : DaiDirectiveEffect::PerimeterClose;
        g->kind = DaiGoalKind::Count;
    } else if ((kind == "budget") || (kind == "bias") || (kind == "custody") || (kind == "staff") || (kind == "ally")) {
        return "NOT_BUILT_YET";                 // the policy directives (10 section 16, P8.4b)
    } else {
        return "UNKNOWN_KIND";
    }
    return nullptr;
}

void DaiDirectives::drop_goal(const DaiDirective &o, DaiGoals &goals)
{
    if (o.goal_kind == DaiGoalKind::Count) return;
    std::vector<uint32_t> ids;
    goals.ForEach([&](const DaiGoal &g) { if (same_goal(o, g)) ids.push_back(g.id); });
    for (uint32_t id : ids) goals.Drop(id);
}

const char *DaiDirectives::Set(std::vector<DaiDirective> &list, const DaiDirectiveMap &m, const DaiDirectiveIn *d, int64_t n,
                               bool replace, bool dry_run, DaiTurn now, DaiGoals &goals, int64_t *bad,
                               std::vector<DaiDirectiveEffect> *effects)
{
    if (bad != nullptr) *bad = -1;
    if ((n < 0) || (n > (int64_t)MAX)) return "TOO_MANY";
    // Every one checked first; and the list it would make not over MAX.
    size_t added = 0;
    for (int64_t i = 0; i < n; i++) {
        DaiGoal g;
        DaiDirectiveEffect e;
        const char *err = Check(m, d[i], &g, &e);
        if (err == nullptr) {
            const bool known = !replace && std::any_of(list.begin(), list.end(), [&](const DaiDirective &o) { return strcmp(o.id, d[i].id) == 0; });
            bool repeat = false;
            for (int64_t j = 0; j < i; j++) repeat = repeat || (strcmp(d[j].id, d[i].id) == 0);
            if (!known && !repeat) added++;
            if ((replace ? 0 : list.size()) + added > MAX) err = "TOO_MANY";
        }
        if (err != nullptr) {
            if (bad != nullptr) *bad = i;
            return err;
        }
    }
    if (dry_run) return nullptr;
    if (replace) {
        for (const DaiDirective &o : list) drop_goal(o, goals);
        list.clear();
    }
    for (int64_t i = 0; i < n; i++) {
        DaiGoal g;
        DaiDirectiveEffect e;
        Check(m, d[i], &g, &e);
        if ((effects != nullptr) && (e != DaiDirectiveEffect::None)) effects->push_back(e);
        DaiDirective *at = nullptr;
        for (DaiDirective &o : list) if (strcmp(o.id, d[i].id) == 0) at = &o;
        if (at != nullptr) drop_goal(*at, goals);              // updated: its old goal goes
        else {
            list.emplace_back();
            at = &list.back();
        }
        snprintf(at->id, sizeof(at->id), "%s", d[i].id);
        snprintf(at->kind, sizeof(at->kind), "%s", d[i].kind);
        at->goal_kind = g.kind;
        at->goal_key = g.p.key;
        at->params = g.p;
        at->weight = g.weight;
        at->arrived = now;
        at->expires = (d[i].expires_turn > 0) ? (DaiTurn)d[i].expires_turn : 0;
        at->expired = false;
        if (g.kind != DaiGoalKind::Count) goals.Set(g, now);
        // Not in the replay (directives are not packets, 10 section 5): each arrival logged with its content.
        DaiLog::Info(m.self, "directive %s: %s room %" PRId64 " count %" PRId64 " at (%" PRId64 ",%" PRId64 ") rival %" PRId64
            " imps %" PRId64 " paydays %" PRId64 " %s%s priority %" PRId64 ", until %" PRId64, d[i].id, d[i].kind, d[i].room_kind,
            d[i].count, d[i].x, d[i].y, d[i].rival, d[i].imps, d[i].paydays, d[i].objective, d[i].action, d[i].priority,
            d[i].expires_turn);
    }
    return nullptr;
}

bool DaiDirectives::Due(const std::vector<DaiDirective> &list, DaiTurn now)
{
    return std::any_of(list.begin(), list.end(), [&](const DaiDirective &o) { return !o.expired && (o.expires != 0) && (o.expires <= now); });
}

void DaiDirectives::Expire(std::vector<DaiDirective> &list, PlayerNumber self, DaiTurn now, DaiGoals &goals)
{
    for (DaiDirective &o : list) {
        if (o.expired || (o.expires == 0) || (o.expires > now)) continue;
        o.expired = true;
        drop_goal(o, goals);
        DaiLog::Info(self, "directive %s: expired at turn %" PRIu64, o.id, (uint64_t)now);
    }
}

void DaiDirectives::Restore(const std::vector<DaiDirective> &list, DaiTurn now, DaiGoals &goals)
{
    for (const DaiDirective &d : list) {
        if (!Live(d, now) || (d.goal_kind == DaiGoalKind::Count)) continue;     // a lapsed one stays lapsed (14 R7)
        goals.Set(goal_of(d), now);
    }
}

std::vector<DaiDirectiveState> DaiDirectives::Statuses(const std::vector<DaiDirective> &list, const DaiGoals &goals)
{
    std::vector<DaiDirectiveState> out;
    out.reserve(list.size());
    for (const DaiDirective &o : list) {
        DaiDirectiveState s;
        s.d = &o;
        s.status = "done";
        if (o.expired) s.status = "expired";
        else if (o.goal_kind != DaiGoalKind::Count) {
            goals.ForEach([&](const DaiGoal &g) {
                if (!same_goal(o, g)) return;
                s.status = (g.status == DaiGoalStatus::Met) ? "done" : (g.status == DaiGoalStatus::Blocked) ? "blocked" : "active";
                s.why = g.why;
            });
        }
        out.push_back(s);
    }
    return out;
}

void DaiDirectives::Save(const std::vector<DaiDirective> &list, DaiBlobWriter &w)
{
    w.I64((int64_t)list.size());
    for (const DaiDirective &d : list) {
        w.Str(d.id);
        w.Str(d.kind);
        w.I64((int64_t)d.goal_kind);
        w.I64(d.goal_key);
        w.I64(d.params.count);
        w.I64(d.params.at.x0); w.I64(d.params.at.y0); w.I64(d.params.at.x1); w.I64(d.params.at.y1);
        w.I64(d.params.aim);
        w.I64(d.weight);
        w.U64(d.arrived);
        w.U64(d.expires);
        w.Bool(d.expired);
    }
}

bool DaiDirectives::Load(std::vector<DaiDirective> *list, DaiBlobReader &r)
{
    list->clear();
    const int64_t n = r.Count((int64_t)MAX);
    for (int64_t i = 0; (i < n) && r.Ok(); i++) {
        DaiDirective d;
        snprintf(d.id, sizeof(d.id), "%s", r.Str().c_str());
        snprintf(d.kind, sizeof(d.kind), "%s", r.Str().c_str());
        d.goal_kind = (DaiGoalKind)r.Range(0, (int64_t)DaiGoalKind::Count);
        d.goal_key = r.I64();
        d.params.key = d.goal_key;
        d.params.count = r.I64();
        d.params.at.x0 = (DaiSlabCoord)r.Range(-1, 4096); d.params.at.y0 = (DaiSlabCoord)r.Range(-1, 4096);
        d.params.at.x1 = (DaiSlabCoord)r.Range(-1, 4096); d.params.at.y1 = (DaiSlabCoord)r.Range(-1, 4096);
        d.params.aim = r.Range(-1, 8);
        d.weight = (DaiFix)r.Range(0, 64 * DAI_FIX_ONE);
        d.arrived = (DaiTurn)r.U64();
        d.expires = (DaiTurn)r.U64();
        d.expired = r.Bool();
        list->push_back(d);
    }
    return r.Ok();
}
