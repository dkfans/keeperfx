// Defence's jobs (06 section 4.2): place_trap (a trap of the keeper's stock on a slab; done once an imp has brought its
// crate and it is armed -- a long stall window, for an imp free to carry it), place_door (a door on a chokepoint, done
// when it stands), set_door_lock (a door locked or unlocked, Q6). The controller checks each with the engine's rules.
#include "pre_inc.h"
#include "daimon/DaiGeom.h"
#include "JobKinds.h"

#include <cstring>

#include "post_inc.h"

namespace {
const DaiTrapLook *trap_at(const DaiKnowledge &k, DaiSlabXY at)
{
    for (const DaiTrapLook &t : k.Traps()) if ((DaiSubtileSlab(t.x) == at.x) && (DaiSubtileSlab(t.y) == at.y)) return &t;
    return nullptr;
}

class JobPlaceDefence : public IDaiJobKind {
public:
    explicit JobPlaceDefence(bool door) : m_door(door) {}
    const char *GetName() const override { return m_door ? "place_door" : "place_trap"; }
    DaiDomain GetDomain() const override { return DaiDomain::Defence; }
    /** A trap waits for an imp with nothing better to do to carry its crate. */
    DaiTurn MinStallWindow() const override { return m_door ? 0 : 3000; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        const DaiStockLook &st = c.know.Stock();
        if ((p == nullptr) || (st.Slot(m_door, p->door_kind) < 0)) {
            snprintf(why, why_len, "BAD_KIND");
            return DaiJobState::Failed;
        }
        const bool placed = m_door ? (c.know.DoorAt(p->at.x, p->at.y) != nullptr) : (trap_at(c.know, p->at) != nullptr);
        const int64_t have = st.Placeable(m_door, p->door_kind);
        if (!placed && (have <= 0)) {
            snprintf(why, why_len, "NONE_IN_STOCK");
            return DaiJobState::Failed;
        }
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_KIND");
            return DaiResult::BadInput;
        }
        // Placed and waiting for its crate: nothing more to do by hand.
        if (m_door ? (c.know.DoorAt(p->at.x, p->at.y) != nullptr) : (trap_at(c.know, p->at) != nullptr)) return DaiResult::Wait;
        memset(verb, 0, sizeof(*verb));
        verb->kind = m_door ? ESV_PlaceDoor : ESV_PlaceTrap;
        const DaiStockLook &st = c.know.Stock();
        snprintf(verb->name, sizeof(verb->name), "%s", st.Name(m_door, p->door_kind));
        verb->has_pos = true;
        verb->stl_x = DaiSlabSubtileCenter(p->at.x);
        verb->stl_y = DaiSlabSubtileCenter(p->at.y);
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        if (p == nullptr) return 0;
        if (m_door) return (c.know.DoorAt(p->at.x, p->at.y) != nullptr) ? 1 : 0;
        const DaiTrapLook *t = trap_at(c.know, p->at);
        return (t == nullptr) ? 0 : (t->armed ? 2 : 1);
    }

    int64_t Watch(const DaiTurnCtx &c, const DaiJob &j, DaiSlabXY *out, int64_t max) const override
    {
        (void)c;
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        if ((p == nullptr) || (max < 1)) return 0;
        out[0] = p->at;
        return 1;
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        if (p == nullptr) return false;
        if (m_door) return c.know.DoorAt(p->at.x, p->at.y) != nullptr;
        const DaiTrapLook *t = trap_at(c.know, p->at);
        return (t != nullptr) && t->armed;
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Running;   // done when it stands (a trap: when armed)
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        const DaiDoorParams v = (p != nullptr) ? *p : DaiDoorParams();
        w.I64(v.at.x); w.I64(v.at.y); w.I64(v.door_kind);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiDoorParams p;
        p.at.x = (DaiSlabCoord)r.Range(0, 4096);
        p.at.y = (DaiSlabCoord)r.Range(0, 4096);
        p.door_kind = r.Range(0, 255);
        j.p = p;
        return r.Ok();
    }
private:
    bool m_door;
};

class JobSetDoorLock : public IDaiJobKind {
public:
    const char *GetName() const override { return "set_door_lock"; }
    DaiDomain GetDomain() const override { return DaiDomain::Defence; }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiLockParams *p = std::get_if<DaiLockParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_PARAMS");
            return DaiResult::BadInput;
        }
        const DaiDoorLook *d = c.know.DoorAt(p->at.x, p->at.y);
        if (d == nullptr) {
            snprintf(why, why_len, "GONE");
            return DaiResult::Fail;
        }
        if (d->locked == p->lock) return DaiResult::Done;
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_SetDoorLock;
        verb->has_pos = true;
        verb->stl_x = d->x;
        verb->stl_y = d->y;
        verb->has_enabled = true;
        verb->enabled = p->lock;
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { return Achieved(c, j) ? 1 : 0; }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiLockParams *p = std::get_if<DaiLockParams>(&j.p);
        const DaiDoorLook *d = (p != nullptr) ? c.know.DoorAt(p->at.x, p->at.y) : nullptr;
        return (d != nullptr) && (d->locked == p->lock);
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiLockParams *p = std::get_if<DaiLockParams>(&j.p);
        const DaiLockParams v = (p != nullptr) ? *p : DaiLockParams();
        w.I64(v.at.x); w.I64(v.at.y); w.Bool(v.lock);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiLockParams p;
        p.at.x = (DaiSlabCoord)r.Range(0, 4096);
        p.at.y = (DaiSlabCoord)r.Range(0, 4096);
        p.lock = r.Bool();
        j.p = p;
        return r.Ok();
    }
};
}

const IDaiJobKind &JobPlaceTrapKind() { static const JobPlaceDefence k(false); return k; }
const IDaiJobKind &JobPlaceDoorKind() { static const JobPlaceDefence k(true); return k; }
const IDaiJobKind &JobSetDoorLockKind() { static const JobSetDoorLock k; return k; }
