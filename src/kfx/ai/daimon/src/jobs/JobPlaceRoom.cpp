// place_room, grow_room, rebuild_room (04 section 6, Construction): a room on the rectangle the layout planner chose
// -- its earth marked first and its entrance last, then, once imps have claimed every slab, build_room with the gold
// for it claimed. A growth is the same on a strip beside the room (the new slabs join it); a rebuild the same where a
// lost room was. A refusal (PARTLY_UNBUILDABLE, CANNOT_BUILD_HERE) fails the job: Construction plans again. A slab
// left undug or unclaimed long while the imps work elsewhere gets an imp fetched to it, as dig_route's (dai_build_basic
// after the refactor-renderer merge: a lair's strip marked at turn 4886 was first dug at 6375, the lair full meanwhile).
// Phase 1's build_treasury, generalised.
#include "pre_inc.h"
#include "JobKinds.h"

#include "JobPath.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiGeom.h"

#include <cinttypes>
#include "post_inc.h"

namespace {
constexpr DaiTurn WAIT_TURNS = 20;
/** A room build fetches only as a last resort: a slab of it left waiting this long since its last progress (three of
 *  dig_route's windows: a fetch takes the imp and the hand from other work -- dai_island_wade's route to the island,
 *  waded by imps left free, was not opened before turn 11,600 with room fetches after 300 turns, at 3,654 without),
 *  and at most this many. */
constexpr DaiTurn ROOM_DROP_AFTER = 3 * JOB_DROP_AFTER;
constexpr int64_t MAX_DROPS = 4;

struct AreaState {
    int64_t dig = 0, dig_unmarked = 0;  /**< still to dig, and of those not marked */
    int64_t ready = 0;                  /**< the keeper's claimed floor: the room may go there */
    int64_t built = 0;                  /**< the room already */
};

AreaState look(const DaiTurnCtx &c, const DaiPlaceRoomParams &p)
{
    AreaState st;
    const RoomKind kind = c.know.RoomKindOf(p.room);
    for (DaiSlabCoord y = p.area.y0; y <= p.area.y1; y++) {
        for (DaiSlabCoord x = p.area.x0; x <= p.area.x1; x++) {
            const DaiSlabInfo s = c.know.SlabAt(x, y);
            if (JobDiggableBy(s, c.player)) {
                st.dig++;
                if (!s.marked) st.dig_unmarked++;
            } else if ((s.owner == c.player) && (s.terrain == DaiTerrain::Room) && (kind != 0) && (s.room == kind)) {
                st.built++;
            } else if ((s.owner == c.player) && (s.terrain == DaiTerrain::Floor)) {
                st.ready++;
            }
        }
    }
    return st;
}

const DaiPlaceRoomParams *params(const DaiKnowledge &k, const DaiJob &j)
{
    const DaiPlaceRoomParams *p = std::get_if<DaiPlaceRoomParams>(&j.p);
    if ((p == nullptr) || (p->room >= DaiRoom::Count)) return nullptr;
    const DaiRect &a = p->area;
    const bool ok = (a.x0 >= 0) && (a.y0 >= 0) && (a.x1 >= a.x0) && (a.y1 >= a.y0) && (a.x1 < k.Width()) && (a.y1 < k.Height())
        && (JobRectSlabs(a) <= EXTSEAT_MAX_BUILD_SLABS);
    return ok ? p : nullptr;
}

/** The entrance still to mark: earth (or the keeper's wall, fortified meanwhile) not marked yet. */
bool entrance_to_mark(const DaiTurnCtx &c, const DaiPlaceRoomParams &p)
{
    const DaiSlabXY &e = p.entrance;
    if ((e.x < 0) || (e.y < 0) || (e.x >= c.know.Width()) || (e.y >= c.know.Height())) return false;
    const DaiSlabInfo s = c.know.SlabAt(e.x, e.y);
    return JobDiggableBy(s, c.player) && !s.marked;
}

/** A slab of the area (or its entrance) the imps have yet to take: marked and undug, or open floor not yet claimed. */
bool waiting_slab(const DaiTurnCtx &c, const DaiPlaceRoomParams &p, DaiSlabXY *out)
{
    auto waiting = [&](DaiSlabCoord x, DaiSlabCoord y) {
        const DaiSlabInfo s = c.know.SlabAt(x, y);
        return (JobDiggableBy(s, c.player) && s.marked) || ((s.terrain == DaiTerrain::Floor) && (s.owner < 0));
    };
    for (DaiSlabCoord y = p.area.y0; y <= p.area.y1; y++)
        for (DaiSlabCoord x = p.area.x0; x <= p.area.x1; x++)
            if (waiting(x, y)) { *out = DaiSlabXY{ x, y }; return true; }
    const DaiSlabXY &e = p.entrance;
    if ((e.x >= 0) && (e.y >= 0) && (e.x < c.know.Width()) && (e.y < c.know.Height()) && waiting(e.x, e.y)) { *out = e; return true; }
    return false;
}

/** The fetch (Tick): a drop under way ends once its gesture went (or it stood JOB_DROP_TURNS); a slab left waiting
 *  ROOM_DROP_AFTER turns since the job's last progress and fetch gets the next. True while one is under way (the next
 *  gesture is the drop). */
bool fetch(const DaiTurnCtx &c, DaiJob &j, DaiPlaceRoomParams &p, const char *kind)
{
    if (p.drop_gestures >= 0) {
        const bool dropped = ((int64_t)j.gestures > p.drop_gestures);
        if (dropped || (c.now >= p.drop_turn + JOB_DROP_TURNS)) {
            DaiLog::Info(c.player, "j%u %s fetch %" PRId64 " %s", j.id, kind, p.drops, dropped ? "dropped" : "not dropped");
            p.drop_gestures = -1;
        }
    }
    const DaiTurn quiet = std::max<DaiTurn>(std::max<DaiTurn>(j.last_progress_turn, j.created), p.drop_turn);
    DaiSlabXY a, site;
    if ((p.drop_gestures < 0) && (p.drops < MAX_DROPS) && (c.now >= quiet + ROOM_DROP_AFTER) && waiting_slab(c, p, &a)
        && JobDropSite(c.know, c.player, nullptr, a, &site)) {
        if (const DaiImpLook *imp = JobDropImp(c.know, site)) {
            p.drop_imp = imp->ref;
            p.drop_at = site;
            p.drop_turn = c.now;
            p.drop_gestures = (int64_t)j.gestures;
            p.drops++;
            DaiLog::Info(c.player, "j%u %s fetch %" PRId64 ": imp %d to (%d,%d) for (%d,%d)", j.id, kind, p.drops, (int)imp->ref.idx,
                (int)site.x, (int)site.y, (int)a.x, (int)a.y);
        }
    }
    if (p.drop_gestures < 0) return false;
    j.needs.hand = true;
    j.needs.thing = p.drop_imp;
    return true;
}

class JobPlaceRoom : public IDaiJobKind {
public:
    explicit JobPlaceRoom(const char *name) : m_name(name) {}
    const char *GetName() const override { return m_name; }
    DaiTurn MinStallWindow() const override { return DAI_DIG_STALL_WINDOW; }
    DaiDomain GetDomain() const override { return DaiDomain::Construction; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiPlaceRoomParams *p = params(c.know, j);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_AREA");
            return DaiJobState::Failed;
        }
        j.needs = DaiJobNeeds();
        if (fetch(c, j, *std::get_if<DaiPlaceRoomParams>(&j.p), m_name)) return j.state;
        const AreaState st = look(c, *p);
        const int64_t slabs = JobRectSlabs(p->area);
        if ((st.dig_unmarked > 0) || entrance_to_mark(c, *p)) return j.state;     // marks to place
        if (st.ready + st.built < slabs) {                                          // the claim wait
            j.retry_after = c.now + WAIT_TURNS;
            return j.state;
        }
        j.needs.gold = st.ready * c.know.Build(p->room).price;
        return j.state;
    }

    int64_t Watch(const DaiTurnCtx &c, const DaiJob &j, DaiSlabXY *out, int64_t max) const override
    {
        // The rectangle while imps claim it and the room goes up.
        const DaiPlaceRoomParams *p = params(c.know, j);
        if (p == nullptr) return 0;
        int64_t n = 0;
        for (DaiSlabCoord y = p->area.y0; (y <= p->area.y1) && (n < max); y++)
            for (DaiSlabCoord x = p->area.x0; (x <= p->area.x1) && (n < max); x++) out[n++] = DaiSlabXY{ x, y };
        return n;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiPlaceRoomParams *p = params(c.know, j);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_AREA");
            return DaiResult::BadInput;
        }
        if (!c.know.Build(p->room).available) {
            snprintf(why, why_len, "NOT_AVAILABLE");
            return DaiResult::Fail;
        }
        if (p->drop_gestures >= 0) {
            memset(verb, 0, sizeof(*verb));
            verb->kind = ESV_PickUpAndDrop;
            verb->thing_count = 1;
            verb->thing_ids[0] = p->drop_imp.idx;
            verb->has_pos = true;
            verb->stl_x = DaiSlabSubtileCenter(p->drop_at.x);
            verb->stl_y = DaiSlabSubtileCenter(p->drop_at.y);
            return DaiResult::Ok;
        }
        const AreaState st = look(c, *p);
        if (st.dig_unmarked > 0) {
            JobMarkDigVerb(verb, p->area);
            return DaiResult::Ok;
        }
        if (entrance_to_mark(c, *p)) {
            JobMarkDigVerb(verb, DaiRect{ p->entrance.x, p->entrance.y, p->entrance.x, p->entrance.y });
            return DaiResult::Ok;
        }
        if (st.ready == 0) return (st.built == JobRectSlabs(p->area)) ? DaiResult::Done : DaiResult::Wait;
        if (st.ready + st.built < JobRectSlabs(p->area)) return DaiResult::Wait;
        JobBuildRoomVerb(verb, p->room, p->area);
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiPlaceRoomParams *p = params(c.know, j);
        if (p == nullptr) return 0;
        const AreaState st = look(c, *p);
        return st.ready + 2 * st.built;     // dug and claimed, then built
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiPlaceRoomParams *p = params(c.know, j);
        return (p != nullptr) && (look(c, *p).built == JobRectSlabs(p->area));
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        // A refused build: Construction plans again. A refused fetch only ends the fetch (Tick, once the gesture went).
        const DaiPlaceRoomParams *p = std::get_if<DaiPlaceRoomParams>(&j.p);
        const bool drop = (p != nullptr) && (p->drop_gestures >= 0) && ((int64_t)j.gestures == p->drop_gestures + 1);
        return (r.failed && !drop) ? DaiJobState::Failed : DaiJobState::Ready;
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiPlaceRoomParams *p = std::get_if<DaiPlaceRoomParams>(&j.p);
        const DaiPlaceRoomParams v = (p != nullptr) ? *p : DaiPlaceRoomParams();
        w.I64((int64_t)v.room);
        w.I64(v.area.x0); w.I64(v.area.y0); w.I64(v.area.x1); w.I64(v.area.y1);
        w.I64(v.entrance.x); w.I64(v.entrance.y);
        w.U32(v.record);
        w.Ref(v.drop_imp);
        w.I64(v.drop_at.x);
        w.I64(v.drop_at.y);
        w.U64(v.drop_turn);
        w.I64(v.drop_gestures);
        w.I64(v.drops);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiPlaceRoomParams p;
        p.room = (DaiRoom)r.Range(0, (int64_t)DaiRoom::Count - 1);
        p.area.x0 = (DaiSlabCoord)r.Range(0, 255); p.area.y0 = (DaiSlabCoord)r.Range(0, 255);
        p.area.x1 = (DaiSlabCoord)r.Range(0, 255); p.area.y1 = (DaiSlabCoord)r.Range(0, 255);
        p.entrance.x = (DaiSlabCoord)r.Range(-1, 255); p.entrance.y = (DaiSlabCoord)r.Range(-1, 255);
        p.record = r.U32();
        p.drop_imp = r.Ref();
        p.drop_at.x = (DaiSlabCoord)r.Range(0, 255);
        p.drop_at.y = (DaiSlabCoord)r.Range(0, 255);
        p.drop_turn = (DaiTurn)r.U64();
        p.drop_gestures = r.Range(-1, INT64_MAX);
        p.drops = r.Range(0, MAX_DROPS);
        j.p = p;
        return r.Ok();
    }
private:
    const char *m_name;
};

const JobPlaceRoom s_place_room("place_room"), s_grow_room("grow_room"), s_rebuild_room("rebuild_room");

// place_room_door (04 section 6): a door of the keeper's on a room's entrance, when it has one in stock. The game
// allows it on the keeper's claimed floor between two walls; refused, the job fails and Construction waits.
class JobPlaceRoomDoor : public IDaiJobKind {
public:
    const char *GetName() const override { return "place_room_door"; }
    DaiDomain GetDomain() const override { return DaiDomain::Construction; }

    DaiJobState Tick(const DaiTurnCtx &c, DaiJob &j, char *why, size_t why_len) const override
    {
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        const DaiStockLook &st = c.know.Stock();
        if ((p == nullptr) || (st.Slot(true, p->door_kind) < 0)) {
            snprintf(why, why_len, "BAD_DOOR");
            return DaiJobState::Failed;
        }
        if ((st.Placeable(true, p->door_kind) <= 0) && !Achieved(c, j)) {    // its own door may have been the last
            snprintf(why, why_len, "NO_DOOR");
            return DaiJobState::Failed;
        }
        return j.state;
    }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_DOOR");
            return DaiResult::BadInput;
        }
        if (Achieved(c, j)) return DaiResult::Done;
        memset(verb, 0, sizeof(*verb));
        verb->kind = ESV_PlaceDoor;
        snprintf(verb->name, sizeof(verb->name), "%s", c.know.Stock().Name(true, p->door_kind));
        verb->has_pos = true;
        verb->stl_x = DaiSlabSubtileCenter(p->at.x);
        verb->stl_y = DaiSlabSubtileCenter(p->at.y);
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { return Achieved(c, j) ? 1 : 0; }

    int64_t Watch(const DaiTurnCtx &c, const DaiJob &j, DaiSlabXY *out, int64_t max) const override
    {
        // The door's slab, read each turn: the door seen at once, not sent again (the game would refuse it).
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
        const DaiSlabInfo s = c.know.SlabAt(p->at.x, p->at.y);
        return (s.terrain == DaiTerrain::Door) && (s.owner == c.player);
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Ready;
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
        p.at.x = (DaiSlabCoord)r.Range(0, 255);
        p.at.y = (DaiSlabCoord)r.Range(0, 255);
        p.door_kind = r.Range(0, DaiStockLook::MAX_KINDS - 1);
        j.p = p;
        return r.Ok();
    }
};

// sell_room (04 section 6, 8): the keeper's room slabs in a rectangle sold, only when a manager asks Construction to
// (Economy's emergency rule, P2.7). Never loses a spell (the controller refuses then).
class JobSellRoom : public IDaiJobKind {
public:
    const char *GetName() const override { return "sell_room"; }
    DaiDomain GetDomain() const override { return DaiDomain::Construction; }

    DaiResult NextVerb(const DaiTurnCtx &c, const DaiJob &j, KeeperVerb *verb, char *why, size_t why_len) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if (p == nullptr) {
            snprintf(why, why_len, "BAD_AREA");
            return DaiResult::BadInput;
        }
        if (Achieved(c, j)) return DaiResult::Done;
        JobMarkDigVerb(verb, p->area);
        verb->kind = ESV_Sell;
        return DaiResult::Ok;
    }

    int64_t Progress(const DaiTurnCtx &c, const DaiJob &j) const override { return Achieved(c, j) ? 1 : 0; }

    int64_t Watch(const DaiTurnCtx &c, const DaiJob &j, DaiSlabXY *out, int64_t max) const override
    {
        // The strip, read each turn: sold, the job ends before it sells again (a refusal Economy would take as final).
        (void)c;
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if (p == nullptr) return 0;
        int64_t n = 0;
        for (DaiSlabCoord y = p->area.y0; (y <= p->area.y1) && (n < max); y++)
            for (DaiSlabCoord x = p->area.x0; (x <= p->area.x1) && (n < max); x++) out[n++] = DaiSlabXY{ x, y };
        return n;
    }

    bool Achieved(const DaiTurnCtx &c, const DaiJob &j) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        if (p == nullptr) return false;
        for (DaiSlabCoord y = p->area.y0; y <= p->area.y1; y++) {
            for (DaiSlabCoord x = p->area.x0; x <= p->area.x1; x++) {
                const DaiSlabInfo s = c.know.SlabAt(x, y);
                if ((s.terrain == DaiTerrain::Room) && (s.owner == c.player)) return false;
            }
        }
        return true;
    }

    DaiJobState OnResult(const DaiJob &j, const DaiActResult &r) const override
    {
        (void)j;
        return r.failed ? DaiJobState::Failed : DaiJobState::Ready;
    }

    void SaveParams(const DaiJob &j, DaiBlobWriter &w) const override
    {
        const DaiAreaParams *p = std::get_if<DaiAreaParams>(&j.p);
        const DaiRect a = (p != nullptr) ? p->area : DaiRect();
        w.I64(a.x0); w.I64(a.y0); w.I64(a.x1); w.I64(a.y1);
    }

    bool LoadParams(DaiJob &j, DaiBlobReader &r) const override
    {
        DaiAreaParams p;
        p.area.x0 = (DaiSlabCoord)r.Range(0, 255); p.area.y0 = (DaiSlabCoord)r.Range(0, 255);
        p.area.x1 = (DaiSlabCoord)r.Range(0, 255); p.area.y1 = (DaiSlabCoord)r.Range(0, 255);
        j.p = p;
        return r.Ok();
    }
};

const JobPlaceRoomDoor s_place_room_door;
const JobSellRoom s_sell_room;
}

const IDaiJobKind &JobPlaceRoomKind() { return s_place_room; }
const IDaiJobKind &JobGrowRoomKind() { return s_grow_room; }
const IDaiJobKind &JobRebuildRoomKind() { return s_rebuild_room; }
const IDaiJobKind &JobPlaceRoomDoorKind() { return s_place_room_door; }
const IDaiJobKind &JobSellRoomKind() { return s_sell_room; }
