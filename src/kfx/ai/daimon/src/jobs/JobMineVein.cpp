// mine_vein (03 section 8.2): marks a vein's slabs from the faces the imps can reach outward (a breadth-first walk
// over the vein from its reachable faces, the faces nearest the route's entry first), keeping at most mine_marks
// marks open on it. Gems are marked once and left marked: they are never used up, and imps keep working a marked gem
// on their own. Done when the vein's gold is gone and its gems are marked (a job kept for gems alone would hold one
// of Excavation's max_veins for ever).
#include "pre_inc.h"
#include "JobKinds.h"

#include <algorithm>
#include <vector>

#include "JobPath.h"
#include "post_inc.h"

namespace {
constexpr DaiTurn WAIT_TURNS = 20;

/** The vein's slabs in mining order and how many of them are marked. */
std::vector<DaiSlabXY> mining_order(const DaiTurnCtx &c, uint32_t vein, const DaiSlabXY &entry, int64_t *marked)
{
    const DaiKnowledge &k = c.know;
    *marked = 0;
    const DaiVein *self = k.Vein(vein);
    if (self == nullptr) return {};
    // Everything here is inside the vein's box: the walk's marks are box-sized.
    const DaiRect b = self->box;
    const DaiSlabCoord bw = b.x1 - b.x0 + 1;
    std::vector<uint8_t> seen((size_t)bw * (size_t)(b.y1 - b.y0 + 1), 0);
    auto at = [&](const DaiSlabXY &s) -> uint8_t & { return seen[(size_t)(s.y - b.y0) * (size_t)bw + (size_t)(s.x - b.x0)]; };
    auto in_vein = [&](const DaiSlabXY &s) {
        if ((s.x < b.x0) || (s.y < b.y0) || (s.x > b.x1) || (s.y > b.y1)) return false;
        const DaiVein *v = k.VeinAt(s.x, s.y);
        return (v != nullptr) && (v->id == vein);
    };
    std::vector<DaiSlabXY> faces;
    for (DaiSlabCoord y = b.y0; y <= b.y1; y++) {
        for (DaiSlabCoord x = b.x0; x <= b.x1; x++) {
            if (!in_vein(DaiSlabXY{ x, y })) continue;
            if (k.SlabAt(x, y).marked) (*marked)++;
            if (k.Reach().NextToReached(x, y)) faces.push_back(DaiSlabXY{ x, y });
        }
    }
    auto dist = [&](const DaiSlabXY &s) { return std::abs(s.x - entry.x) + std::abs(s.y - entry.y); };
    std::stable_sort(faces.begin(), faces.end(), [&](const DaiSlabXY &p, const DaiSlabXY &q) { return dist(p) < dist(q); });
    std::vector<DaiSlabXY> order;
    for (const DaiSlabXY &f : faces) { at(f) = 1; order.push_back(f); }
    for (size_t head = 0; head < order.size(); head++) {
        const DaiSlabXY s = order[head];
        const DaiSlabXY next[4] = { { s.x, s.y - 1 }, { s.x + 1, s.y }, { s.x, s.y + 1 }, { s.x - 1, s.y } };
        for (const DaiSlabXY &n : next) {
            if (!in_vein(n) || at(n)) continue;
            at(n) = 1;
            order.push_back(n);
        }
    }
    return order;
}

/** The next mark: the first unmarked slab in mining order, run along its row through unmarked vein slabs next in
 *  the order, at most `room` slabs. None within reach of a hot hazard (P5.13: the marks lured the imps to it). */
int64_t next_marks(const DaiTurnCtx &c, const DaiMineVeinParams &p, DaiRect *rect)
{
    int64_t marked = 0;
    const std::vector<DaiSlabXY> order = mining_order(c, p.vein, p.entry, &marked);
    const int64_t room = c.cfg.excavation_mine_marks - marked;
    if (room <= 0) return 0;
    auto open = [&](const DaiSlabXY &s) {
        return !c.know.SlabAt(s.x, s.y).marked && (c.threat.HotNear(s.x, s.y, c.cfg.defence_hazard_reach) == nullptr);
    };
    for (size_t i = 0; i < order.size(); i++) {
        if (!open(order[i])) continue;
        return JobRun(std::span<const DaiSlabXY>(order), (int64_t)i, room, [&](int64_t k) { return open(order[(size_t)k]); }, rect);
    }
    return 0;
}

class JobMineVein : public IDaiJobKind {
public:
    const char *GetName() const override { return "mine_vein"; }
    DaiTurn MinStallWindow() const override { return DAI_DIG_STALL_WINDOW; }
    DaiDomain GetDomain() const override { return DaiDomain::Excavation; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        DaiMineVeinParams *p = std::get_if<DaiMineVeinParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiJobState::Failed;
        }
        const DaiVein *v = c.know.Vein(p->vein);
        if (v != nullptr) {
            if (p->gold_start == 0) p->gold_start = v->gold_left;
            p->mined = std::max(p->mined, p->gold_start - v->gold_left);
        }
        DaiRect r;
        if ((v != nullptr) && (JobMarksFull(c) || (next_marks(c, *p, &r) == 0))) j.retry_after = c.now + WAIT_TURNS;
        else if (v != nullptr) JobMarkRepeated(p->marks, r, j, c.now);
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiMineVeinParams *p = std::get_if<DaiMineVeinParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        if (c.know.Vein(p->vein) == nullptr) return DaiResult::Done;
        DaiRect r;
        if (JobMarksFull(c) || (next_marks(c, *p, &r) == 0)) return DaiResult::Wait;   // enough marks open, or no face yet
        JobMarkDigVerb(verb, r);
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        // Gold out of the vein; with gems in it, the keeper's gold mined too -- a gem hit pays without using the gem
        // up, so the vein's gold does not move while the imps work its gems.
        const DaiMineVeinParams *p = std::get_if<DaiMineVeinParams>(&j.p);
        if (p == nullptr) return 0;
        const DaiVein *v = c.know.Vein(p->vein);
        return p->mined + (((v != nullptr) && (v->gem_slabs > 0)) ? c.know.Own().gold_mined : 0);
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiMineVeinParams *p = std::get_if<DaiMineVeinParams>(&j.p);
        if (p == nullptr) return false;
        const DaiVein *v = c.know.Vein(p->vein);
        if (v == nullptr) return true;
        if ((v->gold_slabs != 0) || (v->dense_slabs != 0)) return false;
        for (DaiSlabCoord y = v->box.y0; y <= v->box.y1; y++) {
            for (DaiSlabCoord x = v->box.x0; x <= v->box.x1; x++) {
                const DaiVein *at = c.know.VeinAt(x, y);
                if ((at != nullptr) && (at->id == v->id) && !c.know.SlabAt(x, y).marked) return false;
            }
        }
        return true;
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j; (void)r;
        return DaiJobState::Ready;
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiMineVeinParams *p = std::get_if<DaiMineVeinParams>(&j.p);
        const DaiMineVeinParams q = (p != nullptr) ? *p : DaiMineVeinParams();
        w.U32(q.vein);
        w.I64(q.entry.x);
        w.I64(q.entry.y);
        w.I64(q.gold_start);
        w.I64(q.mined);
        JobSaveMarks(w, q.marks);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiMineVeinParams p;
        p.vein = r.U32();
        p.entry.x = (DaiSlabCoord)r.Range(0, 255);
        p.entry.y = (DaiSlabCoord)r.Range(0, 255);
        p.gold_start = r.Range(0, INT64_MAX);
        p.mined = r.Range(0, INT64_MAX);
        JobLoadMarks(r, &p.marks);
        j.p = p;
        return r.Ok();
    }
};

const JobMineVein s_mine_vein;
}

const IDaiJobKind &JobMineVeinKind()
{
    return s_mine_vein;
}
