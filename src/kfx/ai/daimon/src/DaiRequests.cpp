#include "pre_inc.h"
#include "daimon/DaiRequests.h"

#include <algorithm>
#include <cstring>

#include "post_inc.h"

uint32_t DaiRequests::Post(DaiDomain from, const DaiRequest &r, DaiTurn now)
{
    if ((from >= DaiDomain::Count) || (r.kind >= DaiRequestKind::Count)) return 0;
    const bool service = (r.kind == DaiRequestKind::Service);
    if (service ? ((r.to >= DaiDomain::Count) || (r.service == DaiService::None) || (r.service >= DaiService::Count))
                : (r.goal >= DaiGoalKind::Count))
        return 0;
    DaiRequest *q = nullptr;
    m_requests.ForEach([&](DaiRequest &o) {
        if ((q != nullptr) || (o.from != from) || (o.kind != r.kind) || (o.key != r.key)) return;
        if (service ? ((o.to == r.to) && (o.service == r.service)) : (o.goal == r.goal)) q = &o;
    });
    if (q == nullptr) {
        q = m_requests.Add();
        if (q == nullptr) return 0;
        q->posted = now;
    }
    const uint32_t id = q->id;
    const DaiTurn posted = q->posted;
    const bool same = service && (q->amount == r.amount) && (q->at.x == r.at.x) && (q->at.y == r.at.y);
    const bool served = q->served && same, refused = q->refused && same;     // the same ask stays served (or refused)
    *q = r;
    q->id = id;
    q->served = served;
    q->refused = refused;
    q->from = from;
    q->posted = posted;
    q->weight_delta = std::clamp<DaiFix>(q->weight_delta, -DAI_GOAL_WEIGHT_MAX, DAI_GOAL_WEIGHT_MAX);
    q->count_min = std::clamp<int64_t>(q->count_min, 0, 1000000);
    q->why[sizeof(q->why) - 1] = '\0';
    return id;
}

void DaiRequests::Withdraw(DaiDomain from, uint32_t id)
{
    const DaiRequest *q = m_requests.Find(id);
    if ((q != nullptr) && (q->from == from)) m_requests.Remove(id);
}

const DaiRequest *DaiRequests::Find(DaiDomain from, DaiGoalKind goal, int64_t key) const
{
    const DaiRequest *found = nullptr;
    m_requests.ForEach([&](const DaiRequest &o) {
        if ((found == nullptr) && (o.from == from) && (o.goal == goal) && (o.key == key)) found = &o;
    });
    return found;
}

void DaiRequests::Expire(DaiTurn now)
{
    uint32_t gone[DAI_MAX_REQUESTS];
    int64_t n = 0;
    m_requests.ForEach([&](const DaiRequest &o) { if ((o.expires != 0) && (o.expires <= now)) gone[n++] = o.id; });
    for (int64_t i = 0; i < n; i++) m_requests.Remove(gone[i]);
}

DaiFix DaiRequests::WeightDelta(DaiGoalKind goal, int64_t key) const
{
    DaiFix sum = 0;
    m_requests.ForEach([&](const DaiRequest &o) {
        if ((o.kind == DaiRequestKind::GoalWeight) && (o.goal == goal) && (o.key == key)) sum += o.weight_delta;
    });
    return sum;
}

DaiFix DaiRequests::Folded(DaiGoalKind goal, int64_t key, DaiFix base) const
{
    return std::clamp<DaiFix>(base + WeightDelta(goal, key), 0, DAI_GOAL_WEIGHT_MAX);
}

void DaiRequests::MarkServed(DaiDomain server, uint32_t id)
{
    DaiRequest *q = m_requests.Find(id);
    if ((q != nullptr) && (q->kind == DaiRequestKind::Service) && (q->to == server)) q->served = true;
}

void DaiRequests::MarkRefused(DaiDomain server, uint32_t id)
{
    DaiRequest *q = m_requests.Find(id);
    if ((q != nullptr) && (q->kind == DaiRequestKind::Service) && (q->to == server)) q->served = q->refused = true;
}

int64_t DaiRequests::CountFloor(DaiGoalKind goal, int64_t key) const
{
    int64_t floor = 0;
    m_requests.ForEach([&](const DaiRequest &o) {
        if ((o.kind == DaiRequestKind::GoalWeight) && (o.goal == goal) && (o.key == key)) floor = std::max(floor, o.count_min);
    });
    return floor;
}

void DaiRequests::Save(DaiBlobWriter &w) const
{
    w.U32(m_requests.NextId());
    for (int64_t i = 0; i < m_requests.Capacity(); i++) {
        const DaiRequest &o = m_requests.SlotAt(i);
        w.U32(o.id);
        if (o.id == 0) continue;
        w.I64((int64_t)o.from);
        w.I64((int64_t)o.kind);
        w.I64((int64_t)o.goal);
        w.I64(o.key);
        w.I64(o.weight_delta);
        w.I64(o.count_min);
        w.I64((int64_t)o.to);
        w.I64((int64_t)o.service);
        w.I64(o.amount);
        w.Bool(o.served);
        w.Bool(o.refused);
        w.I64(o.at.x);
        w.I64(o.at.y);
        w.I64(o.budget);
        w.U64(o.posted);
        w.U64(o.expires);
        w.Str(o.why);
    }
}

bool DaiRequests::Load(DaiBlobReader &r)
{
    m_requests = DaiPool<DaiRequest, DAI_MAX_REQUESTS>();
    const uint32_t next = r.U32();
    if (next == 0) r.Fail("request id");
    m_requests.SetNextId(next);
    for (int64_t i = 0; (i < m_requests.Capacity()) && r.Ok(); i++) {
        DaiRequest o;
        o.id = r.U32();
        if (o.id == 0) continue;
        o.from = (DaiDomain)r.Range(0, (int64_t)DaiDomain::Count - 1);
        o.kind = (DaiRequestKind)r.Range(0, (int64_t)DaiRequestKind::Count - 1);
        o.goal = (DaiGoalKind)r.Range(0, (int64_t)DaiGoalKind::Count);
        o.key = r.I64();
        o.weight_delta = r.Range(-DAI_GOAL_WEIGHT_MAX, DAI_GOAL_WEIGHT_MAX);
        o.count_min = r.Range(0, 1000000);
        o.to = (DaiDomain)r.Range(0, (int64_t)DaiDomain::Count);
        o.service = (DaiService)r.Range(0, (int64_t)DaiService::Count - 1);
        o.amount = r.Range(0, 1000000);
        o.served = r.Bool();
        o.refused = r.Bool();
        o.at.x = (DaiSlabCoord)r.Range(-1, 1023);
        o.at.y = (DaiSlabCoord)r.Range(-1, 1023);
        o.budget = r.Range(0, INT64_MAX);
        o.posted = (DaiTurn)r.U64();
        o.expires = (DaiTurn)r.U64();
        const std::string why = r.Str();
        snprintf(o.why, sizeof(o.why), "%s", why.c_str());
        if (r.Ok() && !m_requests.RestoreAt(i, o)) r.Fail("request slot or id");
    }
    if (!r.Ok()) m_requests = DaiPool<DaiRequest, DAI_MAX_REQUESTS>();
    return r.Ok();
}

const char *DaiRequestKindName(DaiRequestKind k)
{
    return (k == DaiRequestKind::GoalWeight) ? "goal_weight" : (k == DaiRequestKind::Service) ? "service" : "?";
}

const char *DaiServiceName(DaiService s)
{
    switch (s) {
    case DaiService::SellRoom: return "sell_room";
    case DaiService::PlaceRoom: return "place_room";
    case DaiService::Dig: return "dig";
    case DaiService::SightPeek: return "sight_peek";
    case DaiService::DestroyWalls: return "destroy_walls";
    case DaiService::Creep: return "creep";
    case DaiService::Reinforce: return "reinforce";
    case DaiService::PlaceRoomAt: return "place_room_at";
    default: return "?";
    }
}
