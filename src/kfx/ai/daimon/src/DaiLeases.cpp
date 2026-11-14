#include "pre_inc.h"
#include "daimon/DaiLeases.h"

#include <algorithm>

#include "post_inc.h"

void DaiLeases::Note(const KeeperTouch &t, DaiTurn now)
{
    Family f = Family::Count;
    switch (t.kind) {
    case KTch_DigTag: case KTch_DigUntag: f = Family::Dig; break;
    case KTch_RoomBuilt: case KTch_RoomSold: f = Family::Room; break;
    case KTch_TrapPlaced: case KTch_TrapSold: f = Family::Trap; break;
    case KTch_DoorPlaced: case KTch_DoorSold: f = Family::Door; break;
    case KTch_Creature:
        if (t.thing <= 0) return;
        if ((m_creatures.count((DaiThingIndex)t.thing) == 0) && ((int64_t)m_creatures.size() >= MAX_CREATURES)) {
            // Full: the oldest lease goes.
            auto oldest = std::min_element(m_creatures.begin(), m_creatures.end(),
                                           [](const auto &a, const auto &b) { return a.second.turn < b.second.turn; });
            m_creatures.erase(oldest);
        }
        m_creatures[(DaiThingIndex)t.thing] = CreatureLease{ (DaiTurn)t.thing_created, now };
        return;
    default: return;
    }
    const int64_t k = key(f, (DaiSlabCoord)t.x, (DaiSlabCoord)t.y);
    if ((m_slabs.count(k) == 0) && ((int64_t)m_slabs.size() >= MAX_SLABS)) {
        auto oldest = std::min_element(m_slabs.begin(), m_slabs.end(),
                                       [](const auto &a, const auto &b) { return a.second.turn < b.second.turn; });
        m_slabs.erase(oldest);
    }
    m_slabs[k] = SlabDecision{ t.kind, now };     // the newest decision on the slab stands
}

bool DaiLeases::CreatureLeased(const DaiThingRef &t, DaiTurn now, int64_t lease_turns) const
{
    auto it = m_creatures.find(t.idx);
    if (it == m_creatures.end()) return false;
    if ((t.creation_turn != 0) && (it->second.created != 0) && (t.creation_turn != it->second.created)) return false;
    return now < it->second.turn + (DaiTurn)std::max<int64_t>(lease_turns, 0);
}

uint8_t DaiLeases::Decision(Family f, DaiSlabCoord x, DaiSlabCoord y) const
{
    auto it = m_slabs.find(key(f, x, y));
    return (it != m_slabs.end()) ? it->second.kind : (uint8_t)KTch_None;
}

void DaiLeases::Expire(DaiTurn now, int64_t lease_turns)
{
    for (auto it = m_creatures.begin(); it != m_creatures.end();) {
        if (now >= it->second.turn + (DaiTurn)std::max<int64_t>(lease_turns, 0)) it = m_creatures.erase(it);
        else ++it;
    }
}

void DaiLeases::Save(DaiBlobWriter &w) const
{
    w.I64((int64_t)m_slabs.size());
    for (const auto &s : m_slabs) { w.I64(s.first); w.I64(s.second.kind); w.U64(s.second.turn); }
    w.I64((int64_t)m_creatures.size());
    for (const auto &c : m_creatures) { w.I64(c.first); w.U64(c.second.created); w.U64(c.second.turn); }
}

bool DaiLeases::Load(DaiBlobReader &r)
{
    Clear();
    const int64_t ns = r.Count(MAX_SLABS);
    for (int64_t i = 0; (i < ns) && r.Ok(); i++) {
        const int64_t k = r.I64();
        SlabDecision d;
        d.kind = (uint8_t)r.Range(KTch_None, KTch_Count - 1);
        d.turn = (DaiTurn)r.U64();
        m_slabs[k] = d;
    }
    const int64_t nc = r.Count(MAX_CREATURES);
    for (int64_t i = 0; (i < nc) && r.Ok(); i++) {
        const DaiThingIndex t = (DaiThingIndex)r.Range(0, 1 << 20);
        CreatureLease l;
        l.created = (DaiTurn)r.U64();
        l.turn = (DaiTurn)r.U64();
        m_creatures[t] = l;
    }
    if (!r.Ok()) Clear();
    return r.Ok();
}

std::vector<DaiSlabXY> DaiLeases::Untagged() const
{
    std::vector<DaiSlabXY> out;
    for (const auto &[k, d] : m_slabs) {
        if (((k >> 40) != (int64_t)Family::Dig) || (d.kind != KTch_DigUntag)) continue;
        out.push_back(DaiSlabXY{ (DaiSlabCoord)((k >> 20) & 0xFFFFF), (DaiSlabCoord)(k & 0xFFFFF) });
    }
    return out;
}
