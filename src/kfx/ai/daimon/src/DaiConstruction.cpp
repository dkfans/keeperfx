#include "pre_inc.h"
#include "daimon/DaiConstruction.h"
#include "daimon/DaiGeom.h"

#include <algorithm>
#include <cinttypes>

#include "daimon/DaiLog.h"
#include "daimon/DaiStockPlanner.h"
#include "post_inc.h"

namespace {
/** A kind's rooms' capacity and use, for "full past grow_at": a room that needs to grow. A lair's use also counts the
 *  lair sizes of the creatures with no lair yet, as the game's free lair space does (P2.10: a lair full in the game
 *  looked 83 % used to Daimon). A hatchery's is the other way round: chickens fill one that breeds more than its
 *  creatures eat, so it is "full" when nearly empty -- below 1 - grow_at of its room, with creatures to feed (P2.10,
 *  the owner: grown when full of chickens, it grew for ever, 173 slabs by turn 30,000). */
void room_use(const DaiKnowledge &k, RoomKind kind, int64_t *capacity, int64_t *used, int64_t grow_at)
{
    *capacity = 0;
    *used = 0;
    bool lair = false, hatchery = false;
    for (const DaiRoomLook &rm : k.Rooms()) {
        if ((kind == 0) || (rm.kind != kind)) continue;
        *capacity += rm.capacity;
        *used += rm.used;
        lair = lair || ((rm.roles & DAI_ROLE_LAIR) != 0);
        hatchery = hatchery || ((rm.roles & DAI_ROLE_FOOD_SPAWN) != 0);
    }
    if (hatchery) {
        // The chickens short of the room: as much use as the room is empty, so few chickens read as full.
        const bool eaters = k.CreatureSummary().creatures > 0;
        const bool starving = eaters && (*used * 1024 < *capacity * (1024 - grow_at));
        *used = starving ? *capacity : 0;
        return;
    }
    if (!lair) return;
    for (const DaiCreatureLook &c : k.Creatures()) if (c.homeless) *used += k.Kind(c.model).lair_size;
}

constexpr DaiTurn RETRY_TURNS = 250;           /**< after a room's site was not found or its build failed */
constexpr DaiTurn GROW_RETRY_TURNS = 2000;     /**< after growing a room failed: new rooms of the kind meanwhile */
constexpr DaiTurn DOOR_RETRY_TURNS = 1000;     /**< after a door was refused (its walls not fortified yet) */
constexpr int64_t MAX_ENTRANCES = 64;
constexpr int64_t MAX_SITE_SLABS = 81;          /**< the game's largest room drag, 9x9 */
/** Job keys: a room's build (1 + its DaiRoom), its link, a door (by slab), a sale (by request), a room on one slab asked
 *  for (by slab). */
constexpr uint64_t KEY_LINK = 1000, KEY_DOOR = 0x10000, KEY_SELL = 0x20000, KEY_AT = 0x30000;

bool own_solid(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y)
{
    if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height())) return false;
    const DaiSlabInfo s = k.SlabAt(x, y);
    return (s.owner == p) && ((s.terrain == DaiTerrain::Floor) || (s.terrain == DaiTerrain::Room) || (s.terrain == DaiTerrain::Door));
}

/** On or beside the keeper's own solid land: imps can dig and claim it from there. */
bool by_own_land(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y)
{
    return own_solid(k, p, x, y) || own_solid(k, p, x, y - 1) || own_solid(k, p, x + 1, y) || own_solid(k, p, x, y + 1)
        || own_solid(k, p, x - 1, y);
}

bool is_build_kind(DaiJobKindId k)
{
    return (k == DAI_JOB_PLACE_ROOM) || (k == DAI_JOB_GROW_ROOM) || (k == DAI_JOB_REBUILD_ROOM);
}

/** The keeper's largest standing room of a kind (its record), or nullptr. */
const DaiRoomRecord *largest(const DaiKnowledge &k, RoomKind kind)
{
    const DaiRoomRecord *best = nullptr;
    k.ForEachRoomRecord([&](const DaiRoomRecord &r) {
        if ((r.kind == kind) && (r.index != 0) && ((best == nullptr) || (r.slabs > best->slabs))) best = &r;
    });
    return best;
}

/** The room of a kind lost most recently, or nullptr. */
const DaiRoomRecord *lost(const DaiKnowledge &k, RoomKind kind)
{
    const DaiRoomRecord *best = nullptr;
    k.ForEachRoomRecord([&](const DaiRoomRecord &r) {
        if ((r.kind == kind) && (r.index == 0) && ((best == nullptr) || (r.lost > best->lost))) best = &r;
    });
    return best;
}

DaiSlabXY middle(const DaiRect &r)
{
    return DaiSlabXY{ (DaiSlabCoord)((r.x0 + r.x1) / 2), (DaiSlabCoord)((r.y0 + r.y1) / 2) };
}

/** The room on an entrance's area still stands as the keeper's. */
bool stands(const DaiTurnCtx &c, const DaiConstruction::Entrance &e)
{
    const RoomKind kind = c.know.RoomKindOf(e.room);
    const DaiSlabInfo s = c.know.SlabAt(e.area.x0, e.area.y0);
    return (kind != 0) && (s.terrain == DaiTerrain::Room) && (s.owner == c.player) && (s.room == kind);
}
}

void DaiConstruction::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    serve(c, w);
    doors(c, w);
    rooms(c, w);
}

/** The slabs to sell of a room: up to `amount` in a row along one edge of its largest, all of them its slabs; for a
 *  library none holding a spellbook (selling one would move the book, or lose the spell when no other library has room
 *  -- the controller refuses that too). The longest such run, bottom edge first; false if there is none. */
bool DaiConstruction::sell_strip(const DaiTurnCtx &c, DaiRoom r, int64_t amount, DaiRect *out)
{
    const DaiKnowledge &k = c.know;
    const RoomKind kind = k.RoomKindOf(r);
    const DaiRoomRecord *rec = largest(k, kind);
    if (rec == nullptr) return false;
    const bool books = (DaiRoomRole(r) & DAI_ROLE_RESEARCH) != 0;
    auto ok = [&](DaiSlabCoord x, DaiSlabCoord y) {
        const DaiSlabInfo s = k.SlabAt(x, y);
        return (s.terrain == DaiTerrain::Room) && (s.owner == c.player) && (s.room == kind) && !(books && k.BookAt(x, y));
    };
    const DaiRect &b = rec->box;
    struct Edge { DaiSlabCoord x, y, dx, dy, len; };
    const Edge edges[4] = { { b.x0, b.y1, 1, 0, (DaiSlabCoord)(b.x1 - b.x0 + 1) }, { b.x0, b.y0, 1, 0, (DaiSlabCoord)(b.x1 - b.x0 + 1) },
                            { b.x1, b.y0, 0, 1, (DaiSlabCoord)(b.y1 - b.y0 + 1) }, { b.x0, b.y0, 0, 1, (DaiSlabCoord)(b.y1 - b.y0 + 1) } };
    int64_t best = 0;
    for (const Edge &e : edges) {
        int64_t run = 0;
        for (DaiSlabCoord i = 0; i < e.len; i++) {
            const DaiSlabCoord x = (DaiSlabCoord)(e.x + e.dx * i), y = (DaiSlabCoord)(e.y + e.dy * i);
            run = ok(x, y) ? run + 1 : 0;
            const int64_t take = std::min<int64_t>(run, std::max<int64_t>(amount, 1));
            if (take > best) {
                best = take;
                const DaiSlabCoord sx = (DaiSlabCoord)(x - e.dx * (take - 1)), sy = (DaiSlabCoord)(y - e.dy * (take - 1));
                *out = DaiRect{ sx, sy, x, y };
            }
        }
    }
    return best > 0;
}

/** Service requests addressed to Construction: sell slabs of a room, a strip off the edge of its largest; a one-slab
 *  room on a slab of its own floor (nothing to dig or site: served at once). */
void DaiConstruction::serve(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    std::vector<DaiRequest> todo;
    c.requests.ForEachService(DaiDomain::Construction, [&](const DaiRequest &r) { if (!r.served) todo.push_back(r); });
    for (const DaiRequest &r : todo) {
        if (r.service == DaiService::PlaceRoom) continue;      // when no other build is under way (rooms())
        if (r.service == DaiService::PlaceRoomAt) {
            place_at(c, w, r);
            continue;
        }
        if ((r.service != DaiService::SellRoom) || (r.key < 0) || (r.key >= (int64_t)DaiRoom::Count)) {
            w.MarkServed(r.id);
            continue;
        }
        DaiRect strip;
        if (!sell_strip(c, (DaiRoom)r.key, r.amount, &strip)) {
            w.MarkRefused(r.id);
            continue;
        }
        DaiJob *j = w.AddJob(DAI_JOB_SELL_ROOM, KEY_SELL + r.id, FixFromInt(2));
        if (j == nullptr) return;
        j->p = DaiAreaParams{ strip };
        j->request_id = r.id;
        w.MarkServed(r.id);
    }
}

void DaiConstruction::place_at(const DaiTurnCtx &c, DaiManagerWrites &w, const DaiRequest &r)
{
    const DaiKnowledge &k = c.know;
    const DaiSlabXY at = r.at;
    const bool known = (r.key >= 0) && (r.key < (int64_t)DaiRoom::Count) && k.Build((DaiRoom)r.key).available;
    const DaiSlabInfo s = k.SlabAt(at.x, at.y);
    if (!known || (at.x < 0) || (at.y < 0) || (at.x >= k.Width()) || (at.y >= k.Height()) || (s.owner != c.player)
     || (s.terrain != DaiTerrain::Floor)) {
        w.MarkRefused(r.id);
        return;
    }
    const uint64_t key = KEY_AT + (uint64_t)at.y * 256 + (uint64_t)at.x;
    if (w.FindJob(DAI_JOB_PLACE_ROOM, key) == nullptr) {
        DaiJob *j = w.AddJob(DAI_JOB_PLACE_ROOM, key, FixFromInt(3));
        if (j == nullptr) return;
        DaiPlaceRoomParams p;
        p.room = (DaiRoom)r.key;
        p.area = DaiRect{ at.x, at.y, at.x, at.y };
        j->p = p;
        j->request_id = r.id;
        DaiLog::Info(c.player, "%s asked on (%d,%d) (%s)", DaiRoomCode(p.room), (int)at.x, (int)at.y, r.why);
    }
    w.MarkServed(r.id);
}

/** A door on each entrance of the rooms it built, when the keeper has one; workshop workers for the doors it lacks. */
void DaiConstruction::doors(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    int64_t open = 0;
    for (const Entrance &e : m_entrances) if (!e.door && stands(c, e)) open++;
    const DaiStockWant want{ true, DaiStockPlanner::DoorToMake(k), open };
    const DaiStockPlan plan = DaiStockPlanner::Plan(k, std::span<const DaiStockWant>(&want, 1));
    const DaiRequest *asked = c.requests.Find(DaiDomain::Construction, DaiGoalKind::Manufacture, 0);
    if (plan.workers > 0) {
        if ((asked == nullptr) || (asked->count_min != plan.workers)) {
            DaiRequest r;
            r.goal = DaiGoalKind::Manufacture;
            r.count_min = plan.workers;
            snprintf(r.why, sizeof(r.why), "DOORS");
            w.PostRequest(r);
        }
    } else if (asked != nullptr) {
        w.WithdrawRequest(asked->id);
    }
    const int64_t kind = DaiStockPlanner::DoorInStock(k);
    if (kind == 0) return;
    bool busy = false;
    c.jobs.ForEach([&](const DaiJob &j) { busy = busy || ((j.kind == DAI_JOB_PLACE_ROOM_DOOR) && !DaiJobEnded(j.state)); });
    if (busy) return;
    for (Entrance &e : m_entrances) {
        if (e.door || (e.retry_after > c.now) || !stands(c, e)) continue;
        // The game puts a door on the keeper's claimed floor between two walls: across the way into the room.
        const bool across_x = (e.at.y < e.area.y0) || (e.at.y > e.area.y1);
        const DaiSlabCoord ax = across_x ? (DaiSlabCoord)(e.at.x - 1) : e.at.x, ay = across_x ? e.at.y : (DaiSlabCoord)(e.at.y - 1);
        const DaiSlabCoord bx = across_x ? (DaiSlabCoord)(e.at.x + 1) : e.at.x, by = across_x ? e.at.y : (DaiSlabCoord)(e.at.y + 1);
        const DaiSlabInfo at = k.SlabAt(e.at.x, e.at.y), a = k.SlabAt(ax, ay), b = k.SlabAt(bx, by);
        if ((at.terrain == DaiTerrain::Door) && (at.owner == c.player)) {
            e.door = true;
            continue;
        }
        const bool walls = (a.terrain == DaiTerrain::Wall) && (a.owner == c.player) && (b.terrain == DaiTerrain::Wall) && (b.owner == c.player);
        if ((at.terrain != DaiTerrain::Floor) || (at.owner != c.player) || !walls) continue;
        DaiJob *j = w.AddJob(DAI_JOB_PLACE_ROOM_DOOR, KEY_DOOR + (uint64_t)e.at.y * 256 + (uint64_t)e.at.x, FixFromInt(1));
        if (j != nullptr) j->p = DaiDoorParams{ e.at, kind };
        return;
    }
}

void DaiConstruction::rooms(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    if (m_searching) {
        step_search(c, w);
        if (m_searching) return;
    }
    int64_t building = 0;
    c.jobs.ForEach([&](const DaiJob &j) {
        if ((j.domain == DaiDomain::Construction) && !DaiJobEnded(j.state) && (is_build_kind(j.kind) || ((j.kind == DAI_JOB_DIG_ROUTE) && (j.key >= KEY_LINK) && (j.key < KEY_DOOR))))
            building++;
    });
    if (m_has_site) {
        // A site found earlier: good for [layout] cache_turns while nothing in it changed.
        if ((c.now > m_site_turn + (DaiTurn)c.cfg.layout_cache_turns) || !m_search.StillFeasible(k)) m_has_site = false;
        else if (building < c.cfg.construction_room_build_concurrency) start_site(c, w);
        return;
    }
    if (building >= c.cfg.construction_room_build_concurrency) return;
    // A room another manager asked for near a spot comes first.
    const DaiRequest *place = nullptr;
    c.requests.ForEachService(DaiDomain::Construction, [&](const DaiRequest &r) {
        if ((place == nullptr) && !r.served && (r.service == DaiService::PlaceRoom)) place = &r;
    });
    if (place != nullptr) {
        const uint32_t id = place->id;
        if ((place->key >= 0) && (place->key < (int64_t)DaiRoom::Count) && k.Build((DaiRoom)place->key).available) {
            start_search(c, (DaiRoom)place->key, place->amount, 0, &place->at);
            m_budget = place->budget;
            w.MarkServed(id);
            step_search(c, w);
            return;
        }
        w.MarkServed(id);
    }
    m_budget = 0;
    // The most wanted Build goal not met: a room already full past grow_at first (its creatures are waiting: a full
    // lair turns new ones away, P2.10), then by weight, then by room.
    auto room_full = [&](const DaiGoal *g) {
        if ((g->p.key < 0) || (g->p.key >= (int64_t)DaiRoom::Count)) return false;
        int64_t capacity = 0, used = 0;
        room_use(k, k.RoomKindOf((DaiRoom)g->p.key), &capacity, &used, c.cfg.layout_grow_at);
        return (capacity > 0) && (used * 1024 >= capacity * c.cfg.layout_grow_at);
    };
    std::vector<const DaiGoal *> goals;
    c.goals.ForEach([&](const DaiGoal &g) { if (g.kind == DaiGoalKind::Build) goals.push_back(&g); });
    std::stable_sort(goals.begin(), goals.end(), [&](const DaiGoal *a, const DaiGoal *b) {
        const bool fa = room_full(a), fb = room_full(b);
        if (fa != fb) return fa;
        return (a->weight != b->weight) ? (a->weight > b->weight) : (a->p.key < b->p.key);
    });
    // With more than one room going up at once, a room already being built is not started again.
    std::array<bool, (size_t)DaiRoom::Count> under_way{};
    c.jobs.ForEach([&](const DaiJob &j) {
        if ((j.domain != DaiDomain::Construction) || DaiJobEnded(j.state) || !is_build_kind(j.kind)) return;
        if (const DaiPlaceRoomParams *pp = std::get_if<DaiPlaceRoomParams>(&j.p))
            if (pp->room < DaiRoom::Count) under_way[(size_t)pp->room] = true;
    });
    if (m_site_room < DaiRoom::Count) under_way[(size_t)m_site_room] = under_way[(size_t)m_site_room] || m_has_site;
    // A level script's DAIMON_BUILD first (Phase 7): its room on its rectangle -- met once a room of that kind covers
    // a slab of it.
    for (const DaiGoal *g : goals) {
        if ((g->p.key < DAI_SCRIPT_BUILD_KEY) || (g->p.at.x0 < 0) || (g->status == DaiGoalStatus::Met)) continue;
        const DaiRoom r = (DaiRoom)DaiGoalBuildRoom(g->p.key);
        if (r >= DaiRoom::Count) continue;
        const RoomKind kind = k.RoomKindOf(r);
        bool built = false;
        k.ForEachRoomRecord([&](const DaiRoomRecord &rec) {
            if ((rec.kind != kind) || (rec.index == 0)) return;
            built = built || ((rec.box.x0 <= g->p.at.x1) && (rec.box.x1 >= g->p.at.x0) && (rec.box.y0 <= g->p.at.y1) && (rec.box.y1 >= g->p.at.y0));
        });
        if (built) {
            w.NoteGoal(g->id, DaiGoalStatus::Met, "");
            continue;
        }
        if ((kind == 0) || !k.Build(r).available) {
            w.NoteGoal(g->id, DaiGoalStatus::Blocked, "NOT_AVAILABLE");
            continue;
        }
        if (under_way[(size_t)r] || (m_retry[(size_t)r] > c.now)) continue;
        w.NoteGoal(g->id, DaiGoalStatus::Progress, "PLANNING");
        start_search(c, r, (int64_t)(g->p.at.x1 - g->p.at.x0 + 1) * (g->p.at.y1 - g->p.at.y0 + 1), 0, nullptr, &g->p.at);
        step_search(c, w);
        return;
    }
    for (const DaiGoal *g : goals) {
        if ((g->p.key < 0) || (g->p.key >= (int64_t)DaiRoom::Count)) continue;
        const DaiRoom r = (DaiRoom)g->p.key;
        if (under_way[(size_t)r]) continue;
        const RoomKind kind = k.RoomKindOf(r);
        int64_t have = 0, capacity = 0, used = 0;
        k.ForEachRoomRecord([&](const DaiRoomRecord &rec) { if ((rec.kind == kind) && (rec.index != 0)) have += rec.slabs; });
        room_use(k, kind, &capacity, &used, c.cfg.layout_grow_at);
        const bool full = (capacity > 0) && (used * 1024 >= capacity * c.cfg.layout_grow_at);
        if ((kind == 0) || ((have >= g->p.count) && !full)) {
            if (g->status != DaiGoalStatus::Met) w.NoteGoal(g->id, DaiGoalStatus::Met, "");
            continue;
        }
        if (!k.Build(r).available) {
            w.NoteGoal(g->id, DaiGoalStatus::Blocked, "NOT_AVAILABLE");
            continue;
        }
        if (m_retry[(size_t)r] > c.now) {
            w.NoteGoal(g->id, DaiGoalStatus::Blocked, "RETRY");
            continue;
        }
        int64_t need = std::max<int64_t>(g->p.count - have, full ? std::max<int64_t>(have / 4, 3) : 0);
        // A new room at most the personality's starting size, grown toward its goal later: a big one finds no site
        // near the dungeon (P2.10: a 37-slab workshop, for the level 4 items, was never built in 30,000 turns).
        if (have == 0) need = std::min<int64_t>(need, std::max<int64_t>(c.personality.ProgrammeSlabs(r), 1));
        w.NoteGoal(g->id, DaiGoalStatus::Progress, "PLANNING");
        start_search(c, r, need, have);
        step_search(c, w);
        return;
    }
}

void DaiConstruction::start_search(const DaiTurnCtx &c, DaiRoom r, int64_t need, int64_t have, const DaiSlabXY *at,
                                   const DaiRect *exact)
{
    const DaiKnowledge &k = c.know;
    const RoomKind kind = k.RoomKindOf(r);
    DaiLayoutQuery q;
    q.kind = kind;
    q.door = (DaiStockPlanner::DoorToMake(k) != 0) || (DaiStockPlanner::DoorInStock(k) != 0);
    m_mode = Mode::Place;
    m_record = 0;
    const DaiRoomRecord *grow = ((have > 0) && (exact == nullptr)) ? largest(k, kind) : nullptr;
    const DaiRoomRecord *gone = lost(k, kind);
    if ((grow != nullptr) && (c.now >= m_grow_failed[(size_t)r] + GROW_RETRY_TURNS || (m_grow_failed[(size_t)r] == 0))) {
        m_mode = Mode::Grow;
        m_record = grow->id;
        q.grow_record = grow->id;
        q.slabs_want = std::max<int64_t>(need, 1);
        const DaiSlabXY mid = middle(grow->box);
        q.ax = mid.x;
        q.ay = mid.y;
    } else {
        q.slabs_want = std::clamp<int64_t>(std::max(need, c.personality.ProgrammeSlabs(r)), 1, MAX_SITE_SLABS);
        q.ax = DaiSubtileSlab(k.Own().heart_x);
        q.ay = DaiSubtileSlab(k.Own().heart_y);
        const DaiRoom anchor = c.personality.ProgrammeAnchor(r);
        if (anchor != DaiRoom::Count) {
            if (const DaiRoomRecord *a = largest(k, k.RoomKindOf(anchor))) {
                const DaiSlabXY mid = middle(a->box);
                q.ax = mid.x;
                q.ay = mid.y;
                q.anchor_kind = a->kind;
            }
        }
        if (exact != nullptr) {
            // A level script's DAIMON_BUILD: that rectangle and no other.
            q.exact = true;
            q.exact_rect = *exact;
            q.slabs_want = (int64_t)(exact->x1 - exact->x0 + 1) * (exact->y1 - exact->y0 + 1);
            q.ax = (DaiSlabCoord)((exact->x0 + exact->x1) / 2);
            q.ay = (DaiSlabCoord)((exact->y0 + exact->y1) / 2);
            q.anchor_kind = 0;
        } else if (at != nullptr) {
            q.ax = at->x;
            q.ay = at->y;
            q.anchor_kind = 0;
        } else if (gone != nullptr) {
            m_mode = Mode::Rebuild;
            m_record = gone->id;
            const DaiSlabXY mid = middle(gone->box);
            q.ax = mid.x;
            q.ay = mid.y;
        }
    }
    // Rooms being built: not on them, and -- one of another kind -- not within the rim of them either, as of a standing
    // room (two rooms planned at once came out touching, P2.10).
    std::vector<DaiRect> blocked;
    c.jobs.ForEach([&](const DaiJob &j) {
        const DaiPlaceRoomParams *p = std::get_if<DaiPlaceRoomParams>(&j.p);
        if ((p == nullptr) || DaiJobEnded(j.state)) return;
        DaiRect b = p->area;
        if (p->room != r) {
            const DaiSlabCoord rim = (DaiSlabCoord)c.cfg.layout_rim;
            b = DaiRect{ (DaiSlabCoord)(b.x0 - rim), (DaiSlabCoord)(b.y0 - rim), (DaiSlabCoord)(b.x1 + rim), (DaiSlabCoord)(b.y1 + rim) };
        }
        blocked.push_back(b);
    });
    m_search.Start(k, c.player, q, DaiLayoutWeights::From(c.personality), DaiDigCosts::ClaimRoute(c.cfg, c.personality, k),
                   c.cfg.layout_radius, c.cfg.layout_rim, blocked);
    m_searching = true;
    m_search_room = r;
}

void DaiConstruction::step_search(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    (void)w;
    const DaiResult r = m_search.Step(c.know, c.budget);
    if (r == DaiResult::Wait) return;
    m_searching = false;
    const size_t i = (size_t)m_search_room;
    if (r == DaiResult::Done) {
        if ((m_budget > 0) && (m_search.Best().dig_turns > m_budget)) {
            // Asked for with a budget (a forward treasury): this site costs more than it would save.
            DaiLog::Info(c.player, "%s near the spot asked for: the best site costs %d turns, over %" PRId64, DaiRoomCode(m_search_room),
                (int)m_search.Best().dig_turns, m_budget);
            m_budget = 0;
            return;
        }
        m_has_site = true;
        m_site = m_search.Best();
        m_site_turn = c.now;
        m_site_room = m_search_room;
        m_site_mode = m_mode;
        m_site_record = m_record;
        return;
    }
    if (m_mode == Mode::Grow) {
        m_grow_failed[i] = c.now;               // no strip fits: a room of the kind elsewhere next time
    } else {
        m_retry[i] = c.now + RETRY_TURNS;
        DaiLog::Debug(c.player, "no site for %s", DaiRoomCode(m_search_room));
    }
}

/** The site's jobs: a claim route to its entrance first when the entrance is not beside the keeper's land, then the
 *  room itself. */
void DaiConstruction::start_site(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiRoom r = m_site_room;
    const DaiSlabXY ent{ m_site.ent_x, m_site.ent_y };
    if ((m_site_mode != Mode::Grow) && (ent.x >= 0) && !by_own_land(k, c.player, ent.x, ent.y)) {
        if (w.FindJob(DAI_JOB_DIG_ROUTE, KEY_LINK + (uint64_t)r) != nullptr) return;
        DaiJob *j = w.AddJob(DAI_JOB_DIG_ROUTE, KEY_LINK + (uint64_t)r, FixFromInt(4));
        if (j == nullptr) return;
        DaiDigRouteParams p;
        p.claim = true;
        p.goals.push_back(ent);
        j->p = p;
        DaiLog::Info(c.player, "%s site (%d,%d)-(%d,%d) not by its land: linking its entrance (%d,%d)", DaiRoomCode(r),
            (int)m_site.r.x0, (int)m_site.r.y0, (int)m_site.r.x1, (int)m_site.r.y1, (int)ent.x, (int)ent.y);
        return;
    }
    const DaiJobKindId kind = (m_site_mode == Mode::Grow) ? DAI_JOB_GROW_ROOM : (m_site_mode == Mode::Rebuild) ? DAI_JOB_REBUILD_ROOM
                                                                                                                 : DAI_JOB_PLACE_ROOM;
    DaiJob *j = w.AddJob(kind, 1 + (uint64_t)r, FixFromInt((r == DaiRoom::Treasury) ? 4 : 3));
    if (j == nullptr) return;
    DaiPlaceRoomParams p;
    p.room = r;
    p.area = m_site.r;
    p.entrance = ent;
    p.record = m_site_record;
    j->p = p;
    m_has_site = false;
}

void DaiConstruction::OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w)
{
    if ((j.kind == DAI_JOB_DIG_ROUTE) && (j.key >= KEY_LINK) && (j.key < KEY_LINK + (uint64_t)DaiRoom::Count)) {
        // Linked: the room goes up next (the site waits); not linked: a pause, and another site then.
        if (j.state != DaiJobState::Done) {
            m_has_site = false;
            m_retry[(size_t)(j.key - KEY_LINK)] = c.now + RETRY_TURNS;
            if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
        }
        return;
    }
    if (is_build_kind(j.kind) && (j.key >= KEY_AT)) {    // a room asked for on a slab: the asker asks again or not
        if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
        return;
    }
    if (is_build_kind(j.kind)) {
        const DaiPlaceRoomParams *p = std::get_if<DaiPlaceRoomParams>(&j.p);
        if (p == nullptr) return;
        const size_t i = (size_t)p->room;
        if (j.state == DaiJobState::Done) {
            if ((p->entrance.x >= 0) && ((int64_t)m_entrances.size() < MAX_ENTRANCES)) {
                Entrance e;
                e.room = p->room;
                e.area = p->area;
                e.at = p->entrance;
                m_entrances.push_back(e);
            }
            return;
        }
        if (j.kind == DAI_JOB_GROW_ROOM) m_grow_failed[i] = c.now;
        else m_retry[i] = c.now + RETRY_TURNS;
        if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
        return;
    }
    if ((j.kind == DAI_JOB_SELL_ROOM) && (j.state != DaiJobState::Done)) {
        if (j.request_id != 0) w.MarkRefused(j.request_id);    // refused (WOULD_LOSE_SPELLS...): Economy asks for another room
        if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
        return;
    }
    if (j.kind == DAI_JOB_PLACE_ROOM_DOOR) {
        const DaiDoorParams *p = std::get_if<DaiDoorParams>(&j.p);
        if (p == nullptr) return;
        for (Entrance &e : m_entrances) {
            if ((e.at.x != p->at.x) || (e.at.y != p->at.y)) continue;
            if (j.state == DaiJobState::Done) e.door = true;
            else e.retry_after = c.now + DOOR_RETRY_TURNS;
        }
        if (j.state == DaiJobState::Blocked) w.Cancel(j, j.why);
    }
}

void DaiConstruction::Reset()
{
    m_search = DaiLayoutSearch();
    m_searching = false;
    m_search_room = DaiRoom::Count;
    m_mode = Mode::Place;
    m_record = 0;
    m_has_site = false;
    m_site = DaiSite();
    m_site_turn = 0;
    m_site_room = DaiRoom::Count;
    m_site_mode = Mode::Place;
    m_site_record = 0;
    m_budget = 0;
    m_retry.fill(0);
    m_grow_failed.fill(0);
    m_entrances.clear();
}

void DaiConstruction::Save(DaiBlobWriter &w) const
{
    w.Bool(m_searching);
    w.Bool(m_has_site);
    if (m_searching || m_has_site) m_search.Save(w);     // a waiting site is re-checked against it (StillFeasible)
    w.I64((int64_t)m_search_room);
    w.I64((int64_t)m_mode);
    w.U32(m_record);
    w.Bool(m_site.found);
    w.I64(m_site.r.x0); w.I64(m_site.r.y0); w.I64(m_site.r.x1); w.I64(m_site.r.y1);
    w.I64(m_site.ent_x); w.I64(m_site.ent_y); w.I64(m_site.score); w.I64(m_site.eff_pred); w.I64(m_site.dig_turns);
    w.U64(m_site_turn);
    w.I64((int64_t)m_site_room);
    w.I64((int64_t)m_site_mode);
    w.U32(m_site_record);
    w.I64(m_budget);
    w.I64((int64_t)m_retry.size());
    for (size_t i = 0; i < m_retry.size(); i++) { w.U64(m_retry[i]); w.U64(m_grow_failed[i]); }
    w.I64((int64_t)m_entrances.size());
    for (const Entrance &e : m_entrances) {
        w.I64((int64_t)e.room);
        w.I64(e.area.x0); w.I64(e.area.y0); w.I64(e.area.x1); w.I64(e.area.y1);
        w.I64(e.at.x); w.I64(e.at.y);
        w.Bool(e.door);
        w.U64(e.retry_after);
    }
}

bool DaiConstruction::Load(DaiBlobReader &r)
{
    Reset();
    auto coord = [&r]() { return (DaiSlabCoord)r.Range(-300, 300); };
    m_searching = r.Bool();
    m_has_site = r.Bool();
    if ((m_searching || m_has_site) && !m_search.Load(r)) r.Fail("construction search");
    m_search_room = (DaiRoom)r.Range(0, (int64_t)DaiRoom::Count);
    m_mode = (Mode)r.Range(0, (int64_t)Mode::Rebuild);
    m_record = r.U32();
    m_site.found = r.Bool();
    m_site.r.x0 = coord(); m_site.r.y0 = coord(); m_site.r.x1 = coord(); m_site.r.y1 = coord();
    m_site.ent_x = coord(); m_site.ent_y = coord();
    m_site.score = r.I64();
    m_site.eff_pred = (int32_t)r.Range(0, 256);
    m_site.dig_turns = (int32_t)r.Range(0, INT32_MAX);
    m_site_turn = (DaiTurn)r.U64();
    m_site_room = (DaiRoom)r.Range(0, (int64_t)DaiRoom::Count);
    m_site_mode = (Mode)r.Range(0, (int64_t)Mode::Rebuild);
    m_site_record = r.U32();
    m_budget = r.Range(0, INT64_MAX);
    const int64_t n = r.Count((int64_t)m_retry.size());
    for (int64_t i = 0; (i < n) && r.Ok(); i++) { m_retry[(size_t)i] = (DaiTurn)r.U64(); m_grow_failed[(size_t)i] = (DaiTurn)r.U64(); }
    const int64_t ne = r.Count(MAX_ENTRANCES);
    for (int64_t i = 0; (i < ne) && r.Ok(); i++) {
        Entrance e;
        e.room = (DaiRoom)r.Range(0, (int64_t)DaiRoom::Count - 1);
        e.area.x0 = coord(); e.area.y0 = coord(); e.area.x1 = coord(); e.area.y1 = coord();
        e.at.x = coord(); e.at.y = coord();
        e.door = r.Bool();
        e.retry_after = (DaiTurn)r.U64();
        m_entrances.push_back(e);
    }
    if (!r.Ok()) Reset();
    return r.Ok();
}
