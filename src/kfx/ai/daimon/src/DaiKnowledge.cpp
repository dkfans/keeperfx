#include "pre_inc.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiGeom.h"

#include <algorithm>

#include "post_inc.h"

namespace {
// The packed slab word: terrain:5 owner:4 (owner + 1, 0 none) room:8 revealed:1 marked:1 dirty:1 (unused):1 seen:11.
// Reach is the reach map's, double-buffered apart from the words.
constexpr uint32_t TERRAIN_SHIFT = 0, OWNER_SHIFT = 5, ROOM_SHIFT = 9, REVEALED_BIT = 1u << 17, MARKED_BIT = 1u << 18,
                   DIRTY_BIT = 1u << 19, SEEN_SHIFT = 21;
constexpr uint32_t TERRAIN_MASK = 0x1F << TERRAIN_SHIFT, OWNER_MASK = 0xF << OWNER_SHIFT;

uint32_t pack(const DaiSlabLook &l, DaiTurn now)
{
    const uint32_t owner = ((l.owner >= 0) && (l.owner < 15)) ? (uint32_t)(l.owner + 1) : 0;
    return ((uint32_t)l.terrain << TERRAIN_SHIFT) | (owner << OWNER_SHIFT) | (((uint32_t)l.room & 0xFF) << ROOM_SHIFT)
        | (l.revealed ? REVEALED_BIT : 0) | (l.marked ? MARKED_BIT : 0) | ((uint32_t)((now / 64) & 0x7FF) << SEEN_SHIFT);
}

DaiTerrain terrain_of(uint32_t wd) { return (DaiTerrain)((wd & TERRAIN_MASK) >> TERRAIN_SHIFT); }

bool valuable(DaiTerrain t) { return (t == DaiTerrain::Gold) || (t == DaiTerrain::DenseGold) || (t == DaiTerrain::Gem); }
}

void DaiKnowledge::Init(DaiSlabCoord w, DaiSlabCoord h)
{
    m_w = (w > 0) ? w : 0;
    m_h = (h > 0) ? h : 0;
    m_slab.assign((size_t)m_w * (size_t)m_h, 0);
    m_human_kept.clear();
    m_scan_x = m_scan_y = 0;
    m_dirty.clear();
    m_rivals.fill(DaiRivalSighting());
    m_cycles = 0;
    m_gold.assign(m_slab.size(), 0);
    m_slab_vein.assign(m_slab.size(), 0);
    m_marks.clear();
    m_veins = DaiPool<DaiVein, DAI_MAX_VEINS>();
    m_vein_edits.clear();
    m_veins_stale = false;
    m_vein_rebuilds = 0;
    m_reach.Init(m_w, m_h);
    m_room_records.Clear();
    m_enemy_defences.clear();
    m_rival_rooms.clear();
    m_enemy_defences_turn = m_rival_rooms_turn = 0;
    m_change_seq += m_change_log.size() + 1;    // a reader from before cannot follow the log across a new map
}

void DaiKnowledge::MarkDirty(const DaiRect &r)
{
    for (DaiSlabCoord y = std::max<DaiSlabCoord>(r.y0, 0); y <= std::min<DaiSlabCoord>(r.y1, m_h - 1); y++) {
        for (DaiSlabCoord x = std::max<DaiSlabCoord>(r.x0, 0); x <= std::min<DaiSlabCoord>(r.x1, m_w - 1); x++) {
            uint32_t &wd = word(x, y);
            if ((wd & DIRTY_BIT) != 0) continue;
            wd |= DIRTY_BIT;
            m_dirty.push_back(DaiSlabXY{ x, y });
        }
    }
}

void DaiKnowledge::read_slab(IDaiWorldReader &world, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, DaiTurn now)
{
    DaiSlabLook l;
    world.Slab(p, x, y, &l);
    const int64_t at = (int64_t)y * m_w + x;
    uint32_t &wd = word(x, y);
    const uint32_t old = wd;
    const int32_t gold = (int32_t)std::clamp<int64_t>(l.gold_left, 0, INT32_MAX);
    const int32_t old_gold = m_gold[(size_t)at];
    const DaiReachMap::Walk old_walk = walk(at, p);
    wd = pack(l, now);
    m_gold[(size_t)at] = gold;
    if ((old & (TERRAIN_MASK | OWNER_MASK | REVEALED_BIT)) != (wd & (TERRAIN_MASK | OWNER_MASK | REVEALED_BIT)))
        m_change_log[(size_t)(m_change_seq++ % m_change_log.size())] = (int32_t)at;
    // A guard post or water come or gone changes the steps between slabs (RaisedStep) as well.
    const bool raised_was = post_word(old) || (terrain_of(old) == DaiTerrain::Water);
    const bool raised_is = post_word(wd) || (terrain_of(wd) == DaiTerrain::Water);
    if ((walk(at, p) != old_walk) || (raised_was != raised_is)) m_reach.NoteChange();
    const bool was = ((old & REVEALED_BIT) != 0) && valuable(terrain_of(old)), is = l.revealed && valuable(l.terrain);
    if ((was != is) || (is && (terrain_of(old) != l.terrain))) {
        m_veins_stale = true;
    } else if (is && (gold != old_gold) && (m_slab_vein[(size_t)at] != 0)) {
        DaiVein *v = m_veins.Find(m_veins.SlotAt(m_slab_vein[(size_t)at] - 1).id);
        if (v != nullptr) v->gold_left += gold - old_gold;
    }
}

/** The keeper's marks are its own state: exact every turn. A slab whose mark went was dug (or the game cleared it):
 *  re-read now, so a dig shows the turn after, not when the scan comes round. */
void DaiKnowledge::read_marks(IDaiWorldReader &world, PlayerNumber p)
{
    if ((m_w <= 0) || (m_h <= 0)) return;
    static constexpr int64_t MAX_MARKS = 512;
    DaiSlabXY now[MAX_MARKS];
    const int64_t n = world.OwnMarks(p, now, MAX_MARKS);
    std::vector<int32_t> marks;
    marks.reserve((size_t)n);
    for (int64_t i = 0; i < n; i++) {
        if ((now[i].x < 0) || (now[i].y < 0) || (now[i].x >= m_w) || (now[i].y >= m_h)) continue;
        marks.push_back((int32_t)(now[i].y * m_w + now[i].x));
    }
    std::sort(marks.begin(), marks.end());
    marks.erase(std::unique(marks.begin(), marks.end()), marks.end());
    for (int32_t at : m_marks) {
        if (std::binary_search(marks.begin(), marks.end(), at)) continue;
        m_slab[(size_t)at] &= ~MARKED_BIT;
        MarkDirty(DaiRect{ (DaiSlabCoord)(at % m_w), (DaiSlabCoord)(at / m_w), (DaiSlabCoord)(at % m_w), (DaiSlabCoord)(at / m_w) });
    }
    for (int32_t at : marks) m_slab[(size_t)at] |= MARKED_BIT;
    m_marks.swap(marks);
}

DaiReachMap::Walk DaiKnowledge::walk(int64_t at, PlayerNumber p) const
{
    const uint32_t wd = m_slab[(size_t)at];
    if ((wd & REVEALED_BIT) == 0) return DaiReachMap::NotWalkable;
    const uint32_t owner = (wd & OWNER_MASK) >> OWNER_SHIFT;
    const bool own = (owner != 0) && ((PlayerNumber)(owner - 1) == p);
    // As the seat's reach check (slab_walkable_for): not blocking, not lava, not abyss, not another keeper's door.
    switch (terrain_of(wd)) {
    case DaiTerrain::Floor: case DaiTerrain::Room: case DaiTerrain::Water: break;
    case DaiTerrain::Door: if (!own) return DaiReachMap::NotWalkable; break;
    default: return DaiReachMap::NotWalkable;
    }
    return own ? DaiReachMap::OwnWalkable : DaiReachMap::Walkable;
}

bool DaiKnowledge::post_word(uint32_t wd) const
{
    const RoomKind post = m_room_kind[(size_t)DaiRoom::GuardPost];
    return (post != 0) && (terrain_of(wd) == DaiTerrain::Room) && (((wd >> ROOM_SHIFT) & 0xFF) == ((uint32_t)post & 0xFF));
}

bool DaiKnowledge::raised_step(int64_t from, int64_t to) const
{
    return (terrain_of(m_slab[(size_t)from]) == DaiTerrain::Water) && post_word(m_slab[(size_t)to]);
}

bool DaiKnowledge::RaisedStep(DaiSlabCoord fx, DaiSlabCoord fy, DaiSlabCoord tx, DaiSlabCoord ty) const
{
    if ((fx < 0) || (fy < 0) || (fx >= m_w) || (fy >= m_h) || (tx < 0) || (ty < 0) || (tx >= m_w) || (ty >= m_h)) return false;
    return raised_step((int64_t)fy * m_w + fx, (int64_t)ty * m_w + tx);
}

void DaiKnowledge::note_things(IDaiWorldReader &world, PlayerNumber p, const DaiRect &rows, DaiTurn now)
{
    DaiThingLook things[256];
    const int64_t n = world.VisibleThings(p, rows, things, 256);
    for (int64_t i = 0; i < n; i++) {
        const DaiThingLook &t = things[i];
        if ((t.owner < 0) || (t.owner >= DAI_MAX_PLAYERS) || (t.owner == p)) continue;
        DaiRivalSighting &r = m_rivals[t.owner];
        r.last_seen = now;
        if (t.heart) {
            r.heart_seen = true;
            r.heart_x = t.x;
            r.heart_y = t.y;
        } else if (!t.digger) {
            // Its fighters only, as the query panel counts them (P9: its imps, often seen mining by the keeper's land,
            // made the levels KnownForce puts on the panel's count tiny, a rival of 40 creatures counted as none).
            r.creatures_this_cycle++;
            r.force_this_cycle += t.level;
        }
    }
}

void DaiKnowledge::end_cycle(DaiTurn now)
{
    for (DaiRivalSighting &r : m_rivals) {
        r.creatures_last_cycle = r.creatures_this_cycle;
        r.force_last_cycle = r.force_this_cycle;
        r.force_turn = now;
        r.creatures_this_cycle = 0;
        r.force_this_cycle = 0;
    }
    m_cycles++;
}

void DaiKnowledge::Update(IDaiWorldReader &world, PlayerNumber p, const KeeperFact *facts, int64_t nfacts, DaiTurn now, DaiBudget &budget)
{
    Read(world, p, facts, nfacts, now, budget);
    Derive(p, budget);
}

void DaiKnowledge::Read(IDaiWorldReader &world, PlayerNumber p, const KeeperFact *facts, int64_t nfacts, DaiTurn now, DaiBudget &budget)
{
    DaiSlabCoord w = 0, h = 0;
    world.MapSize(&w, &h);
    if ((w != m_w) || (h != m_h) || (p != m_player)) Init(w, h);
    m_player = p;
    m_built_turn = now;
    // Own state: exact, every turn.
    world.Own(p, &m_own);
    m_nrooms = world.OwnRooms(p, m_rooms.data(), (int64_t)m_rooms.size());
    match_rooms(world, p, now);
    read_creatures(world, p);
    m_nkinds = std::min<int64_t>(world.KindCount(), DAI_MAX_KINDS - 1);
    for (int64_t m = 0; m < DAI_MAX_KINDS; m++) {
        if ((m >= 1) && (m <= m_nkinds)) world.Kind(p, (DaiThingModel)m, &m_kinds[(size_t)m]);
        else m_kinds[(size_t)m] = DaiKindLook();
    }
    world.Research(p, &m_research);
    for (PlayerNumber r = 0; r < DAI_MAX_PLAYERS; r++) {
        m_rival_keeper[(size_t)r] = world.RivalKeeper(p, r);
        world.Alliance(p, r, &m_alliance[(size_t)r]);
    }
    m_nfood = world.OwnFood(p, m_food.data(), (int64_t)m_food.size());
    // Phase 4's fight reads: whole, every update (a few hundred things at most).
    m_nbattles = world.Battles(p, m_battles.data(), (int64_t)m_battles.size());
    m_ntraps = world.OwnTraps(p, m_traps.data(), (int64_t)m_traps.size());
    m_ndoors = world.OwnDoors(p, m_doors.data(), (int64_t)m_doors.size());
    {
        DaiThingLook seen[DAI_MAX_ENEMIES];
        const int64_t n = world.VisibleThings(p, DaiRect{ 0, 0, (DaiSlabCoord)(w - 1), (DaiSlabCoord)(h - 1) }, seen, DAI_MAX_ENEMIES);
        m_nenemies = 0;
        for (int64_t i = 0; i < n; i++) {
            if (!seen[i].heart) {
                m_enemies[(size_t)m_nenemies++] = seen[i];
            } else if ((seen[i].owner >= 0) && (seen[i].owner < DAI_MAX_PLAYERS)) {
                // A heart in view now (on revealed ground, or in its Sight of Evil: Phase 5) is a sighting at once.
                DaiRivalSighting &r = m_rivals[(size_t)seen[i].owner];
                r.heart_seen = true;
                r.heart_x = seen[i].x;
                r.heart_y = seen[i].y;
                r.last_seen = now;
            }
        }
    }
    world.SightOfEvil(p, &m_sight);
    read_rivals(world, p, now);
    if ((m_hero_gates_turn == 0) || (now >= m_hero_gates_turn + DAI_HERO_GATE_READ)) {
        m_nhero_gates = world.HeroGates(p, m_hero_gates.data(), (int64_t)m_hero_gates.size());
        m_hero_gates_turn = (now > 0) ? now : 1;
    }
    m_nbooks = world.OwnBookSlabs(p, m_books.data(), (int64_t)m_books.size());
    if (!m_content.Built()) {
        std::vector<DaiPowerSpec> powers;
        std::vector<DaiTrapSpec> traps;
        std::vector<DaiDoorSpec> doors;
        world.ContentSpecs(&powers, &traps, &doors);
        m_content.Build(powers, traps, doors, DaiContent::ReadOverrides());
        m_content.Log(p);
    }
    world.Stock(p, &m_stock);
    world.LiveExclusion(p, &m_live);
    world.HumanFightRules(p, &m_human_rules);
    world.Custody(p, &m_custody);
    m_recipes.resize((size_t)DAI_MAX_RECIPES);
    m_recipes.resize((size_t)world.Recipes(m_recipes.data(), DAI_MAX_RECIPES));
    summarise();
    for (size_t d = 0; d < m_powers.size(); d++) {
        const DaiPowerKind k = world.PowerOf((DaiPower)d);
        m_powers[d].available = (k != 0) && world.PowerAvailable(p, k);
        m_powers[d].price = (k != 0) ? world.PowerPrice(p, k, 0) : 0;
    }
    m_own_powers.clear();
    for (const DaiContentEntry &e : m_content.Entries(DaiContentClass::Power)) {
        if (e.base || (e.role == DaiContentRole::Ignore)) continue;
        DaiPowerLook look;
        look.available = world.PowerAvailable(p, (DaiPowerKind)e.kind);
        look.price = world.PowerPrice(p, (DaiPowerKind)e.kind, 0);
        m_own_powers.emplace_back(e.kind, look);
    }
    m_nimps = world.OwnImps(p, m_imps.data(), (int64_t)m_imps.size());
    m_nspecials = world.SpecialBoxes(p, m_specials.data(), (int64_t)m_specials.size());
    m_ndead = world.DeadCreatures(p, m_dead.data(), (int64_t)m_dead.size());
    read_marks(world, p);
    for (size_t d = 0; d < m_build.size(); d++) {
        const RoomKind k = world.RoomOf((DaiRoom)d);
        m_room_kind[d] = k;
        m_build[d].available = (k != 0) && world.RoomAvailable(p, k);
        m_build[d].price = (k != 0) ? world.RoomPrice(p, k) : 0;
    }
    m_bridge = world.BridgeRoles();
    // Slabs a fact names (a loss: that slab and the ring around it) before the scan.
    for (int64_t i = 0; i < nfacts; i++) {
        if (facts[i].kind != KF_Loss) continue;
        const DaiSlabCoord sx = DaiSubtileSlab(facts[i].x), sy = DaiSubtileSlab(facts[i].y);
        MarkDirty(DaiRect{ (DaiSlabCoord)(sx - 1), (DaiSlabCoord)(sy - 1), (DaiSlabCoord)(sx + 1), (DaiSlabCoord)(sy + 1) });
    }
    m_last_fact_reads = 0;
    for (const DaiSlabXY &d : m_dirty) {
        read_slab(world, p, d.x, d.y, now);          // clears the dirty bit with the rest of the word
        m_last_fact_reads++;
    }
    m_dirty.clear();
    // The rolling scan, row by row from where it stopped.
    m_last_scanned = 0;
    if ((m_w <= 0) || (m_h <= 0)) return;
    DaiSlabCoord row0 = m_scan_y;
    while (budget.slab_scan > 0) {
        read_slab(world, p, m_scan_x, m_scan_y, now);
        budget.slab_scan--;
        m_last_scanned++;
        if (++m_scan_x < m_w) continue;
        // A row finished: what the keeper sees on it.
        note_things(world, p, DaiRect{ 0, row0, (DaiSlabCoord)(m_w - 1), m_scan_y }, now);
        m_scan_x = 0;
        if (++m_scan_y >= m_h) {
            m_scan_y = 0;
            end_cycle(now);
        }
        row0 = m_scan_y;
    }
}

void DaiKnowledge::Derive(PlayerNumber p, DaiBudget &budget)
{
    if ((m_w <= 0) || (m_h <= 0)) return;
    if (m_veins_stale) rebuild_veins();
    m_reach.Step([this, p](int64_t at) { return walk(at, p); }, [this](int64_t from, int64_t to) { return raised_step(from, to); },
                 budget.reach_visits);
}

bool DaiKnowledge::in_view(IDaiWorldReader &world, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y) const
{
    if ((x < 0) || (y < 0) || (x >= m_w) || (y >= m_h)) return false;
    return SlabAt(x, y).revealed || (m_sight.active && world.SightShows(p, DaiSlabSubtileCenter(x), DaiSlabSubtileCenter(y)));
}

/** Other keepers' traps, doors and rooms in view now, each as last seen; those out of view kept as remembered, those
 *  whose slab is in view again without them forgotten. */
void DaiKnowledge::read_rivals(IDaiWorldReader &world, PlayerNumber p, DaiTurn now)
{
    if ((m_enemy_defences_turn == 0) || (now >= m_enemy_defences_turn + DAI_ENEMY_DEFENCE_READ)) {
        m_enemy_defences_turn = (now > 0) ? now : 1;
        std::vector<DaiEnemyDefenceLook> seen((size_t)DAI_MAX_ENEMY_DEFENCES);
        seen.resize((size_t)world.EnemyDefences(p, seen.data(), DAI_MAX_ENEMY_DEFENCES));
        std::vector<DaiEnemyDefence> next;
        for (const DaiEnemyDefenceLook &l : seen) next.push_back(DaiEnemyDefence{ l, now });
        for (const DaiEnemyDefence &old : m_enemy_defences) {
            const DaiSlabCoord sx = DaiSubtileSlab(old.look.x), sy = DaiSubtileSlab(old.look.y);
            bool again = false;
            for (const DaiEnemyDefenceLook &l : seen) again = again || ((DaiSubtileSlab(l.x) == sx) && (DaiSubtileSlab(l.y) == sy));
            if (again || in_view(world, p, sx, sy) || ((int64_t)next.size() >= DAI_MAX_ENEMY_DEFENCES)) continue;
            next.push_back(old);
        }
        m_enemy_defences = std::move(next);
    }
    if ((m_rival_rooms_turn == 0) || (now >= m_rival_rooms_turn + DAI_RIVAL_ROOM_READ)) {
        m_rival_rooms_turn = (now > 0) ? now : 1;
        std::vector<DaiRivalRoomLook> seen((size_t)DAI_MAX_RIVAL_ROOMS);
        seen.resize((size_t)world.RivalRooms(p, seen.data(), DAI_MAX_RIVAL_ROOMS));
        std::vector<DaiRivalRoom> next;
        for (const DaiRivalRoomLook &l : seen) next.push_back(DaiRivalRoom{ l, now });
        for (const DaiRivalRoom &old : m_rival_rooms) {
            bool again = false;
            for (const DaiRivalRoomLook &l : seen) again = again || (l.index == old.look.index);
            if (again || in_view(world, p, old.look.x, old.look.y) || ((int64_t)next.size() >= DAI_MAX_RIVAL_ROOMS)) continue;
            next.push_back(old);
        }
        m_rival_rooms = std::move(next);
    }
}

const DaiEnemyDefence *DaiKnowledge::EnemyDefenceAt(DaiSlabCoord x, DaiSlabCoord y) const
{
    for (const DaiEnemyDefence &d : m_enemy_defences)
        if ((DaiSubtileSlab(d.look.x) == x) && (DaiSubtileSlab(d.look.y) == y)) return &d;
    return nullptr;
}

void DaiKnowledge::read_creatures(IDaiWorldReader &world, PlayerNumber p)
{
    m_ncreatures = 0;
    while (m_ncreatures < DAI_MAX_OWN_CREATURES) {
        const int64_t n = world.OwnCreatures(p, m_creatures.data() + m_ncreatures, DAI_MAX_OWN_CREATURES - m_ncreatures, m_ncreatures);
        if (n <= 0) break;
        m_ncreatures += n;
    }
}

const DaiDoorLook *DaiKnowledge::DoorAt(DaiSlabCoord x, DaiSlabCoord y) const
{
    for (const DaiDoorLook &d : Doors()) {
        if ((DaiSubtileSlab(d.x) == x) && (DaiSubtileSlab(d.y) == y)) return &d;
    }
    return nullptr;
}

const DaiRoomLook *DaiKnowledge::RoomByIndex(int64_t index) const
{
    if (index <= 0) return nullptr;
    for (int64_t i = 0; i < m_nrooms; i++) if (m_rooms[(size_t)i].index == index) return &m_rooms[(size_t)i];
    return nullptr;
}

const DaiRoomRecord *DaiKnowledge::RoomRecordOf(int64_t index) const
{
    const DaiRoomRecord *found = nullptr;
    if (index > 0) m_room_records.ForEach([&](const DaiRoomRecord &r) { if ((found == nullptr) && (r.index == index)) found = &r; });
    return found;
}

bool DaiKnowledge::BookAt(DaiSlabCoord x, DaiSlabCoord y) const
{
    for (int64_t i = 0; i < m_nbooks; i++) if ((m_books[(size_t)i].x == x) && (m_books[(size_t)i].y == y)) return true;
    return false;
}

const DaiKindLook &DaiKnowledge::Kind(DaiThingModel m) const
{
    static const DaiKindLook none;
    return ((m >= 1) && (m <= m_nkinds)) ? m_kinds[(size_t)m] : none;
}

/** Each record finds its room by the slab inside it; a room merged into another's goes with it; a room whose slab was
 *  sold keeps its record if a room of its kind still overlaps where it was; any room left is new. */
void DaiKnowledge::match_rooms(IDaiWorldReader &world, PlayerNumber p, DaiTurn now)
{
    std::array<bool, 256> taken{};
    auto position = [this](int64_t index) {
        for (int64_t i = 0; i < m_nrooms; i++) if (m_rooms[(size_t)i].index == index) return i;
        return (int64_t)-1;
    };
    auto bind = [&](DaiRoomRecord &r, int64_t at) {
        const DaiRoomLook &rm = m_rooms[(size_t)at];
        taken[(size_t)at] = true;
        r.index = rm.index;
        r.slabs = rm.slabs;
        r.box = rm.box;
        r.lost = 0;
    };
    std::vector<uint32_t> merged, unbound;
    m_room_records.ForEach([&](DaiRoomRecord &r) {
        const int64_t at = position(world.OwnRoomAt(p, r.ax, r.ay));
        if ((at >= 0) && (m_rooms[(size_t)at].kind == r.kind)) {
            if (taken[(size_t)at]) merged.push_back(r.id);
            else bind(r, at);
        } else {
            unbound.push_back(r.id);
        }
    });
    for (uint32_t id : merged) m_room_records.Remove(id);
    for (uint32_t id : unbound) {
        DaiRoomRecord *r = m_room_records.Find(id);
        bool found = false;
        for (int64_t i = 0; (i < m_nrooms) && (r->index != 0); i++) {
            const DaiRoomLook &rm = m_rooms[(size_t)i];
            if (taken[(size_t)i] || (rm.kind != r->kind) || (rm.box.x1 < r->box.x0) || (rm.box.x0 > r->box.x1)
                || (rm.box.y1 < r->box.y0) || (rm.box.y0 > r->box.y1)) continue;
            r->ax = rm.ax;
            r->ay = rm.ay;
            bind(*r, i);
            found = true;
            break;
        }
        if (!found) {
            if (r->lost == 0) r->lost = now;
            r->index = 0;
        }
    }
    for (int64_t i = 0; i < m_nrooms; i++) {
        if (taken[(size_t)i]) continue;
        DaiRoomRecord *r = m_room_records.Add();
        if (r == nullptr) {
            // Full: the record lost longest ago makes way.
            uint32_t oldest = 0;
            DaiTurn when = 0;
            m_room_records.ForEach([&](const DaiRoomRecord &o) {
                if ((o.lost != 0) && ((oldest == 0) || (o.lost < when))) { oldest = o.id; when = o.lost; }
            });
            if (oldest == 0) break;
            m_room_records.Remove(oldest);
            r = m_room_records.Add();
        }
        const DaiRoomLook &rm = m_rooms[(size_t)i];
        r->kind = rm.kind;
        r->ax = rm.ax;
        r->ay = rm.ay;
        r->since = now;
        bind(*r, i);
    }
}

void DaiKnowledge::summarise()
{
    DaiCreatureSummary &s = m_summary;
    s = DaiCreatureSummary();
    for (int64_t i = 0; i < m_ncreatures; i++) {
        const DaiCreatureLook &c = m_creatures[(size_t)i];
        const DaiKindLook &k = Kind(c.model);
        s.creatures++;
        if ((c.model >= 0) && (c.model < DAI_MAX_KINDS)) s.by_kind[(size_t)c.model]++;
        if (c.anger != DaiAnger::Calm) s.angry[(size_t)c.anger_reason]++;
        if (c.anger == DaiAnger::Livid) s.livid++;
        if (c.homeless) s.lair_used += k.lair_size;     // the game's free lair space counts it taken (calculate_free_lair_space)
        s.doing[(size_t)c.activity]++;
        if (c.health_pct * 256 < k.heal_requirement * 100) s.wounded++;
        s.wages += k.pay;
    }
    for (int64_t i = 0; i < m_nrooms; i++) {
        const DaiRoomLook &rm = m_rooms[(size_t)i];
        if ((rm.roles & DAI_ROLE_LAIR) != 0) { s.lair_capacity += rm.capacity; s.lair_used += rm.used; }
        if ((rm.roles & DAI_ROLE_FOOD) != 0) { s.food_capacity += rm.capacity; s.food += rm.used; }
    }
}

std::vector<DaiVeinEdit> DaiKnowledge::TakeVeinEdits()
{
    std::vector<DaiVeinEdit> out;
    for (uint32_t id : m_vein_edits) {
        const DaiVein *v = m_veins.Find(id);
        if (v == nullptr) continue;
        DaiVeinEdit e;
        e.id = id;
        e.status = v->status;
        e.ex = v->ex;
        e.ey = v->ey;
        e.route_cost = v->route_cost;
        e.retry_after = v->retry_after;
        out.push_back(e);
    }
    m_vein_edits.clear();
    return out;
}

void DaiKnowledge::ApplyVeinEdit(const DaiVeinEdit &e)
{
    DaiVein *v = m_veins.Find(e.id);
    if (v == nullptr) return;
    v->status = e.status;
    v->ex = e.ex;
    v->ey = e.ey;
    v->route_cost = e.route_cost;
    v->retry_after = e.retry_after;
}

void DaiKnowledge::rebuild_veins()
{
    m_veins_stale = false;
    m_vein_rebuilds++;
    const int64_t n = (int64_t)m_w * m_h;
    auto is_valuable = [this](int64_t at) {
        const uint32_t wd = m_slab[(size_t)at];
        return ((wd & REVEALED_BIT) != 0) && valuable(terrain_of(wd));
    };
    // The connected parts, found in raster order of their first slab; each notes the old vein it overlaps most.
    struct Part {
        int32_t gold = 0, dense = 0, gem = 0;
        int64_t gold_left = 0;
        DaiSlabCoord cx = 0, cy = 0;
        DaiRect box;
        int32_t old_slot = -1, overlap = 0;     /**< the old vein (pool slot) it shares most slabs with */
        int32_t slot = -1;                      /**< its vein now; -1 none */
    };
    std::vector<int32_t> part_of((size_t)n, -1);
    std::vector<Part> parts;
    std::vector<int32_t> queue;
    for (int64_t s0 = 0; s0 < n; s0++) {
        if ((part_of[(size_t)s0] >= 0) || !is_valuable(s0)) continue;
        const int32_t pi = (int32_t)parts.size();
        Part pt;
        std::array<int32_t, DAI_MAX_VEINS> overlap{};
        int64_t sx = 0, sy = 0;
        queue.assign(1, (int32_t)s0);
        part_of[(size_t)s0] = pi;
        for (size_t qh = 0; qh < queue.size(); qh++) {
            const int32_t at = queue[qh];
            const DaiSlabCoord x = at % m_w, y = at / m_w;
            switch (terrain_of(m_slab[(size_t)at])) {
            case DaiTerrain::Gold: pt.gold++; pt.gold_left += m_gold[(size_t)at]; break;
            case DaiTerrain::DenseGold: pt.dense++; pt.gold_left += m_gold[(size_t)at]; break;
            default: pt.gem++; break;
            }
            if (m_slab_vein[(size_t)at] != 0) overlap[m_slab_vein[(size_t)at] - 1]++;
            if (qh == 0) pt.box = DaiRect{ x, y, x, y };
            pt.box.x0 = std::min(pt.box.x0, x); pt.box.y0 = std::min(pt.box.y0, y);
            pt.box.x1 = std::max(pt.box.x1, x); pt.box.y1 = std::max(pt.box.y1, y);
            sx += x;
            sy += y;
            const int32_t w = (int32_t)m_w;
            const int32_t next[4] = { (y > 0) ? at - w : -1, (x + 1 < m_w) ? at + 1 : -1, (y + 1 < m_h) ? at + w : -1,
                                      (x > 0) ? at - 1 : -1 };
            for (int32_t nb : next) {
                if ((nb < 0) || (part_of[(size_t)nb] >= 0) || !is_valuable(nb)) continue;
                part_of[(size_t)nb] = pi;
                queue.push_back(nb);
            }
        }
        // The slab nearest the centroid, in whole numbers (scaled by the count): the first found wins a tie.
        const int64_t cnt = (int64_t)queue.size();
        int64_t best = INT64_MAX;
        for (int32_t at : queue) {
            const int64_t dx = (int64_t)(at % m_w) * cnt - sx, dy = (int64_t)(at / m_w) * cnt - sy;
            const int64_t d = dx * dx + dy * dy;
            if (d < best) { best = d; pt.cx = (DaiSlabCoord)(at % m_w); pt.cy = (DaiSlabCoord)(at / m_w); }
        }
        for (int32_t i = 0; i < (int32_t)DAI_MAX_VEINS; i++) {
            if (overlap[(size_t)i] > pt.overlap) { pt.overlap = overlap[(size_t)i]; pt.old_slot = i; }
        }
        parts.push_back(pt);
    }
    // A vein keeps its id in the part that overlaps it most (a split: the bigger share; the earlier part on a tie).
    std::array<int32_t, DAI_MAX_VEINS> keeper;
    keeper.fill(-1);
    for (int32_t pi = 0; pi < (int32_t)parts.size(); pi++) {
        const Part &pt = parts[(size_t)pi];
        if (pt.old_slot < 0) continue;
        int32_t &k = keeper[(size_t)pt.old_slot];
        if ((k < 0) || (pt.overlap > parts[(size_t)k].overlap)) k = pi;
    }
    auto write = [](DaiVein &v, const Part &pt) {
        v.gold_slabs = pt.gold;
        v.dense_slabs = pt.dense;
        v.gem_slabs = pt.gem;
        v.gold_left = pt.gold_left;
        v.cx = pt.cx;
        v.cy = pt.cy;
        v.box = pt.box;
    };
    auto slot_of = [this](uint32_t id) {
        for (int32_t i = 0; i < (int32_t)DAI_MAX_VEINS; i++) if (m_veins.SlotAt(i).id == id) return i;
        return (int32_t)-1;
    };
    for (int32_t i = 0; i < (int32_t)DAI_MAX_VEINS; i++) {
        const uint32_t id = m_veins.SlotAt(i).id;
        if (id == 0) continue;
        if (keeper[(size_t)i] < 0) {
            m_veins.Remove(id);                 // mined out or no longer known
            continue;
        }
        write(*m_veins.Find(id), parts[(size_t)keeper[(size_t)i]]);
        parts[(size_t)keeper[(size_t)i]].slot = i;
    }
    // New veins, biggest first; a full table gives up its smallest untouched vein to a bigger one, never one in use.
    std::vector<int32_t> fresh;
    for (int32_t pi = 0; pi < (int32_t)parts.size(); pi++) if (parts[(size_t)pi].slot < 0) fresh.push_back(pi);
    auto size_of = [](const Part &pt) { return (int64_t)pt.gold + pt.dense + pt.gem; };
    std::stable_sort(fresh.begin(), fresh.end(),
        [&](int32_t a, int32_t b) { return size_of(parts[(size_t)a]) > size_of(parts[(size_t)b]); });
    for (int32_t pi : fresh) {
        Part &pt = parts[(size_t)pi];
        DaiVein *v = m_veins.Add();
        if (v == nullptr) {
            int32_t victim = -1;
            for (int32_t i = 0; i < (int32_t)DAI_MAX_VEINS; i++) {
                const DaiVein &c = m_veins.SlotAt(i);
                if ((c.status != DaiVeinStatus::Untouched) || (c.Slabs() >= size_of(pt))) continue;
                if ((victim < 0) || (c.Slabs() < m_veins.SlotAt(victim).Slabs())) victim = i;
            }
            if (victim < 0) continue;           // no room: the part stays out of the table
            for (Part &o : parts) if (o.slot == victim) o.slot = -1;
            m_veins.Remove(m_veins.SlotAt(victim).id);
            v = m_veins.Add();
        }
        write(*v, pt);
        pt.slot = slot_of(v->id);
    }
    for (int64_t at = 0; at < n; at++) {
        const int32_t pi = part_of[(size_t)at];
        m_slab_vein[(size_t)at] = ((pi >= 0) && (parts[(size_t)pi].slot >= 0)) ? (uint8_t)(parts[(size_t)pi].slot + 1) : 0;
    }
}

DaiSlabInfo DaiKnowledge::SlabAt(DaiSlabCoord x, DaiSlabCoord y) const
{
    DaiSlabInfo s;
    if ((x < 0) || (y < 0) || (x >= m_w) || (y >= m_h)) return s;
    const uint32_t wd = word(x, y);
    s.terrain = (DaiTerrain)((wd >> TERRAIN_SHIFT) & 0x1F);
    const uint32_t owner = (wd >> OWNER_SHIFT) & 0xF;
    s.owner = (owner == 0) ? -1 : (PlayerNumber)(owner - 1);
    s.room = (RoomKind)((wd >> ROOM_SHIFT) & 0xFF);
    s.revealed = (wd & REVEALED_BIT) != 0;
    s.marked = (wd & MARKED_BIT) != 0;
    s.dirty = (wd & DIRTY_BIT) != 0;
    s.reach = m_reach.Reached(x, y);
    s.seen = (uint16_t)((wd >> SEEN_SHIFT) & 0x7FF);
    return s;
}

void DaiKnowledge::SetHumanKept(const std::vector<DaiSlabXY> &kept)
{
    m_human_kept.clear();
    if (kept.empty() || (m_w <= 0) || (m_h <= 0)) return;
    m_human_kept.assign((size_t)m_w * (size_t)m_h, 0);
    for (const DaiSlabXY &s : kept)
        if ((s.x >= 0) && (s.y >= 0) && (s.x < m_w) && (s.y < m_h)) m_human_kept[(size_t)s.y * (size_t)m_w + (size_t)s.x] = 1;
}

bool DaiKnowledge::IsDiggable(DaiSlabCoord x, DaiSlabCoord y) const
{
    switch (SlabAt(x, y).terrain) {
    case DaiTerrain::Dirt: case DaiTerrain::Gold: case DaiTerrain::DenseGold: case DaiTerrain::Gem: return true;
    default: return false;
    }
}

int64_t DaiKnowledge::RoomCluster(DaiSlabCoord x, DaiSlabCoord y, DaiSlabXY *out, int64_t max) const
{
    const DaiSlabInfo first = SlabAt(x, y);
    if ((first.terrain != DaiTerrain::Room) || (max <= 0)) return 0;
    std::vector<uint8_t> seen((size_t)m_w * (size_t)m_h, 0);
    int64_t n = 0;
    out[n++] = DaiSlabXY{ x, y };
    seen[(size_t)y * m_w + x] = 1;
    for (int64_t head = 0; (head < n) && (n < max); head++) {
        const DaiSlabXY s = out[head];
        const DaiSlabXY next[4] = { { s.x, s.y - 1 }, { s.x + 1, s.y }, { s.x, s.y + 1 }, { s.x - 1, s.y } };
        for (const DaiSlabXY &nb : next) {
            if ((nb.x < 0) || (nb.y < 0) || (nb.x >= m_w) || (nb.y >= m_h) || seen[(size_t)nb.y * m_w + nb.x] || (n >= max)) continue;
            const DaiSlabInfo o = SlabAt(nb.x, nb.y);
            if ((o.terrain != DaiTerrain::Room) || (o.room != first.room) || (o.owner != first.owner)) continue;
            seen[(size_t)nb.y * m_w + nb.x] = 1;
            out[n++] = nb;
        }
    }
    return n;
}

int64_t DaiKnowledge::GoldLeft(DaiSlabCoord x, DaiSlabCoord y) const
{
    if ((x < 0) || (y < 0) || (x >= m_w) || (y >= m_h)) return 0;
    return m_gold[(size_t)y * (size_t)m_w + (size_t)x];
}

const DaiVein *DaiKnowledge::VeinAt(DaiSlabCoord x, DaiSlabCoord y) const
{
    if ((x < 0) || (y < 0) || (x >= m_w) || (y >= m_h)) return nullptr;
    const uint8_t s = m_slab_vein[(size_t)y * (size_t)m_w + (size_t)x];
    if (s == 0) return nullptr;
    const DaiVein &v = m_veins.SlotAt(s - 1);
    return (v.id != 0) ? &v : nullptr;
}

const DaiRivalSighting &DaiKnowledge::Sighting(PlayerNumber r) const
{
    static const DaiRivalSighting none;
    return ((r >= 0) && (r < DAI_MAX_PLAYERS)) ? m_rivals[r] : none;
}

int64_t DaiKnowledge::KnownForce(PlayerNumber r, DaiTurn now) const
{
    const DaiRivalSighting &s = Sighting(r);
    int64_t seen = 0;
    if (s.force_turn != 0) {
        const int64_t age = (now > s.force_turn) ? (int64_t)(now - s.force_turn) : 0;
        const int64_t halvings = age / DAI_FORCE_HALF_LIFE;
        seen = (halvings >= 62) ? 0 : (s.force_last_cycle >> halvings);
    }
    const int64_t count = Alliance(r).creatures;
    if (count <= 0) return seen;
    int64_t levels = s.force_last_cycle, n = s.creatures_last_cycle;
    if (n <= 0) {
        levels = 0;
        for (const DaiCreatureLook &cr : Creatures()) {
            if (Kind(cr.model).digger) continue;
            levels += cr.level;
            n++;
        }
    }
    const int64_t by_count = (n > 0) ? (count * levels + n - 1) / n : count;
    return std::max(seen, by_count);
}

bool DaiKnowledge::NearHome(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, int64_t radius) const
{
    const DaiOwnLook &home = Own();
    if (home.has_heart && (std::max(std::abs(DaiSubtileSlab(home.heart_x) - x), std::abs(DaiSubtileSlab(home.heart_y) - y)) <= radius)) return true;
    const RoomKind bridge = RoomKindOf(DaiRoom::Bridge);
    for (DaiSlabCoord sy = (DaiSlabCoord)std::max<int64_t>(y - radius, 0); sy <= std::min<int64_t>(y + radius, Height() - 1); sy++)
        for (DaiSlabCoord sx = (DaiSlabCoord)std::max<int64_t>(x - radius, 0); sx <= std::min<int64_t>(x + radius, Width() - 1); sx++) {
            const DaiSlabInfo s = SlabAt(sx, sy);
            if ((s.terrain == DaiTerrain::Room) && (s.owner == p) && ((bridge == 0) || (s.room != bridge))) return true;
        }
    return false;
}

/** The largest map a saved knowledge may have (slabs a side): above the game's own maximum. */
constexpr int64_t DAI_KNOW_MAX_SIDE = 256;

void DaiKnowledge::Save(DaiBlobWriter &w) const
{
    w.I64(m_w);
    w.I64(m_h);
    w.I64(m_scan_x);
    w.I64(m_scan_y);
    for (uint32_t wd : m_slab) w.U32(wd);
    w.I64((int64_t)m_dirty.size());
    for (const DaiSlabXY &d : m_dirty) { w.I64(d.x); w.I64(d.y); }
    for (const DaiRivalSighting &r : m_rivals) {
        w.U64(r.last_seen);
        w.Bool(r.heart_seen);
        w.I64(r.heart_x);
        w.I64(r.heart_y);
        w.I64(r.creatures_last_cycle);
        w.I64(r.force_last_cycle);
        w.U64(r.force_turn);
        w.I64(r.creatures_this_cycle);
        w.I64(r.force_this_cycle);
    }
    w.I64(m_cycles);
    w.U64(m_built_turn);
    w.I64(m_own.gold); w.I64(m_own.imps); w.I64(m_own.creatures); w.I64(m_own.wage_bill); w.I64(m_own.turns_to_payday);
    w.Bool(m_own.has_heart); w.I64(m_own.heart_x); w.I64(m_own.heart_y); w.I64(m_own.heart_health);
    w.I64((int64_t)m_powers.size());
    for (const DaiPowerLook &p : m_powers) { w.Bool(p.available); w.I64(p.price); }
    w.I64(m_nrooms);
    for (int64_t i = 0; i < m_nrooms; i++) {
        const DaiRoomLook &rm = m_rooms[(size_t)i];
        w.I64(rm.index); w.I64(rm.kind); w.I64(rm.slabs); w.I64(rm.capacity); w.I64(rm.used); w.I64(rm.cx); w.I64(rm.cy);
        w.U64(rm.roles); w.I64(rm.efficiency); w.I64(rm.health); w.I64(rm.max_health); w.I64(rm.stored); w.I64(rm.occupants);
        w.I64(rm.open_sides); w.I64(rm.ax); w.I64(rm.ay); w.I64(rm.box.x0); w.I64(rm.box.y0); w.I64(rm.box.x1); w.I64(rm.box.y1);
    }
    w.I64(m_own.payday_progress); w.I64(m_own.payday_gap); w.I64(m_own.payday_speed); w.I64(m_own.offmap_gold);
    w.I64(m_own.treasury_capacity); w.I64(m_own.treasury_used); w.I64(m_own.loose_gold); w.I64(m_own.gold_carried);
    w.I64(m_own.gold_mined);
    w.I64((int64_t)m_build.size());
    for (const DaiBuildLook &b : m_build) { w.Bool(b.available); w.I64(b.price); }
    w.Bool(m_bridge.water); w.Bool(m_bridge.lava); w.Bool(m_bridge.abyss);
    w.I64(m_player);
    for (int32_t g : m_gold) w.U32((uint32_t)g);
    w.U32(m_veins.NextId());
    for (int64_t i = 0; i < DAI_MAX_VEINS; i++) {
        const DaiVein &v = m_veins.SlotAt(i);
        w.U32(v.id);
        if (v.id == 0) continue;
        w.I64((int64_t)v.status);
        w.I64(v.gold_slabs); w.I64(v.dense_slabs); w.I64(v.gem_slabs); w.I64(v.gold_left);
        w.I64(v.cx); w.I64(v.cy); w.I64(v.ex); w.I64(v.ey); w.I64(v.route_cost); w.U64(v.retry_after);
        w.I64(v.box.x0); w.I64(v.box.y0); w.I64(v.box.x1); w.I64(v.box.y1);
    }
    w.Bytes(m_slab_vein.data(), m_slab_vein.size());
    w.Bool(m_veins_stale);
    w.I64(m_vein_rebuilds);
    w.I64((int64_t)m_marks.size());
    for (int32_t at : m_marks) w.U32((uint32_t)at);
    w.U64(m_change_seq);
    for (int32_t at : m_change_log) w.U32((uint32_t)at);
    m_reach.Save(w);
    w.U32(m_room_records.NextId());
    for (int64_t i = 0; i < DAI_MAX_ROOM_RECORDS; i++) {
        const DaiRoomRecord &rr = m_room_records.SlotAt(i);
        w.U32(rr.id);
        if (rr.id == 0) continue;
        w.I64(rr.kind); w.I64(rr.ax); w.I64(rr.ay); w.I64(rr.index); w.I64(rr.slabs);
        w.I64(rr.box.x0); w.I64(rr.box.y0); w.I64(rr.box.x1); w.I64(rr.box.y1); w.U64(rr.since); w.U64(rr.lost);
    }
    // Phase 5 (KNOW v6): other keepers' defences and rooms as last seen; a trap's spent (v7, P5.13).
    w.I64((int64_t)m_enemy_defences.size());
    for (const DaiEnemyDefence &d : m_enemy_defences) {
        const DaiEnemyDefenceLook &l = d.look;
        w.Ref(l.ref); w.I64(l.owner); w.Bool(l.door); w.I64(l.model); w.I64(l.x); w.I64(l.y);
        w.I64(l.health); w.Bool(l.locked); w.Bool(l.spent); w.U64(d.seen);
    }
    w.I64((int64_t)m_rival_rooms.size());
    for (const DaiRivalRoom &rm : m_rival_rooms) {
        const DaiRivalRoomLook &l = rm.look;
        w.I64(l.index); w.I64(l.owner); w.I64(l.kind); w.I64(l.slabs_seen);
        w.I64(l.box.x0); w.I64(l.box.y0); w.I64(l.box.x1); w.I64(l.box.y1); w.I64(l.x); w.I64(l.y); w.U64(rm.seen);
    }
}

bool DaiKnowledge::Load(DaiBlobReader &r)
{
    const DaiSlabCoord w = (DaiSlabCoord)r.Range(0, DAI_KNOW_MAX_SIDE);
    const DaiSlabCoord h = (DaiSlabCoord)r.Range(0, DAI_KNOW_MAX_SIDE);
    if (!r.Ok()) return false;
    Init(w, h);
    m_scan_x = (DaiSlabCoord)r.Range(0, (w > 0) ? w - 1 : 0);
    m_scan_y = (DaiSlabCoord)r.Range(0, (h > 0) ? h - 1 : 0);
    for (uint32_t &wd : m_slab) {
        wd = r.U32();
        // Every packed field must decode to something the readers can index: a terrain class, a player or nobody.
        if ((((wd >> TERRAIN_SHIFT) & 0x1F) >= (uint32_t)DaiTerrain::Count) || (((wd >> OWNER_SHIFT) & 0xF) > (uint32_t)DAI_MAX_PLAYERS))
            r.Fail("bad slab word");
    }
    const int64_t ndirty = r.Count((int64_t)w * h);
    for (int64_t i = 0; (i < ndirty) && r.Ok(); i++) {
        DaiSlabXY d;
        d.x = (DaiSlabCoord)r.Range(0, w - 1);
        d.y = (DaiSlabCoord)r.Range(0, h - 1);
        m_dirty.push_back(d);
    }
    for (DaiRivalSighting &s : m_rivals) {
        s.last_seen = (DaiTurn)r.U64();
        s.heart_seen = r.Bool();
        s.heart_x = (DaiSubtlCoord)r.I64();
        s.heart_y = (DaiSubtlCoord)r.I64();
        s.creatures_last_cycle = r.I64();
        s.force_last_cycle = r.I64();
        s.force_turn = (DaiTurn)r.U64();
        s.creatures_this_cycle = r.I64();
        s.force_this_cycle = r.I64();
    }
    m_cycles = r.Range(0, INT64_MAX);
    m_built_turn = (DaiTurn)r.U64();
    m_own = DaiOwnLook();
    m_own.gold = r.I64(); m_own.imps = r.Range(0, INT32_MAX); m_own.creatures = r.Range(0, INT32_MAX);
    m_own.wage_bill = r.I64(); m_own.turns_to_payday = r.I64();
    m_own.has_heart = r.Bool(); m_own.heart_x = (DaiSubtlCoord)r.I64(); m_own.heart_y = (DaiSubtlCoord)r.I64();
    m_own.heart_health = r.I64();
    m_powers.fill(DaiPowerLook());
    const int64_t npowers = r.Count((int64_t)m_powers.size());
    for (int64_t i = 0; (i < npowers) && r.Ok(); i++) {
        m_powers[(size_t)i].available = r.Bool();
        m_powers[(size_t)i].price = r.Range(0, INT64_MAX);
    }
    m_nrooms = r.Count((int64_t)m_rooms.size());
    for (int64_t i = 0; (i < m_nrooms) && r.Ok(); i++) {
        DaiRoomLook &rm = m_rooms[(size_t)i];
        rm.index = r.I64(); rm.kind = (RoomKind)r.Range(0, 255); rm.slabs = r.I64(); rm.capacity = r.I64(); rm.used = r.I64();
        rm.cx = (DaiSlabCoord)r.I64(); rm.cy = (DaiSlabCoord)r.I64();
        rm.roles = r.U64(); rm.efficiency = r.Range(0, 256); rm.health = r.I64(); rm.max_health = r.I64();
        rm.stored = r.I64(); rm.occupants = r.Range(0, INT32_MAX); rm.open_sides = r.Range(0, INT32_MAX);
        rm.ax = (DaiSlabCoord)r.Range(0, DAI_KNOW_MAX_SIDE); rm.ay = (DaiSlabCoord)r.Range(0, DAI_KNOW_MAX_SIDE);
        rm.box.x0 = (DaiSlabCoord)r.Range(0, DAI_KNOW_MAX_SIDE); rm.box.y0 = (DaiSlabCoord)r.Range(0, DAI_KNOW_MAX_SIDE);
        rm.box.x1 = (DaiSlabCoord)r.Range(0, DAI_KNOW_MAX_SIDE); rm.box.y1 = (DaiSlabCoord)r.Range(0, DAI_KNOW_MAX_SIDE);
    }
    m_own.payday_progress = r.I64(); m_own.payday_gap = r.I64(); m_own.payday_speed = r.I64(); m_own.offmap_gold = r.I64();
    m_own.treasury_capacity = r.I64(); m_own.treasury_used = r.I64(); m_own.loose_gold = r.I64();
    m_own.gold_carried = r.I64(); m_own.gold_mined = r.I64();
    m_build.fill(DaiBuildLook());
    const int64_t nbuild = r.Count((int64_t)m_build.size());
    for (int64_t i = 0; (i < nbuild) && r.Ok(); i++) {
        m_build[(size_t)i].available = r.Bool();
        m_build[(size_t)i].price = r.Range(0, INT64_MAX);
    }
    m_bridge.water = r.Bool(); m_bridge.lava = r.Bool(); m_bridge.abyss = r.Bool();
    m_player = (PlayerNumber)r.Range(-1, DAI_MAX_PLAYERS - 1);
    for (int32_t &g : m_gold) {
        const uint32_t v = r.U32();
        if (v > (uint32_t)INT32_MAX) r.Fail("slab gold");
        g = (int32_t)v;
    }
    // The veins back into their slots (the slab index names a slot), each checked against what the readers assume.
    const uint32_t next_id = r.U32();
    if (next_id == 0) r.Fail("vein id");
    m_veins.SetNextId(next_id);
    for (int64_t i = 0; (i < DAI_MAX_VEINS) && r.Ok(); i++) {
        DaiVein v;
        v.id = r.U32();
        if (v.id == 0) continue;
        v.status = (DaiVeinStatus)r.Range(0, (int64_t)DaiVeinStatus::Count - 1);
        v.gold_slabs = (int32_t)r.Range(0, (int64_t)w * h);
        v.dense_slabs = (int32_t)r.Range(0, (int64_t)w * h);
        v.gem_slabs = (int32_t)r.Range(0, (int64_t)w * h);
        v.gold_left = r.Range(0, INT64_MAX);
        v.cx = (DaiSlabCoord)r.Range(0, (w > 0) ? w - 1 : 0); v.cy = (DaiSlabCoord)r.Range(0, (h > 0) ? h - 1 : 0);
        v.ex = (DaiSlabCoord)r.Range(0, (w > 0) ? w - 1 : 0); v.ey = (DaiSlabCoord)r.Range(0, (h > 0) ? h - 1 : 0);
        v.route_cost = (int32_t)r.Range(-1, INT32_MAX);
        v.retry_after = (DaiTurn)r.U64();
        v.box.x0 = (DaiSlabCoord)r.Range(0, (w > 0) ? w - 1 : 0); v.box.y0 = (DaiSlabCoord)r.Range(0, (h > 0) ? h - 1 : 0);
        v.box.x1 = (DaiSlabCoord)r.Range(v.box.x0, (w > 0) ? w - 1 : 0); v.box.y1 = (DaiSlabCoord)r.Range(v.box.y0, (h > 0) ? h - 1 : 0);
        if (r.Ok() && !m_veins.RestoreAt(i, v)) r.Fail("vein slot");
    }
    r.Bytes(m_slab_vein.data(), m_slab_vein.size());
    for (uint8_t sv : m_slab_vein) {
        if ((sv != 0) && ((sv > DAI_MAX_VEINS) || (m_veins.SlotAt(sv - 1).id == 0))) r.Fail("slab vein");
    }
    m_veins_stale = r.Bool();
    m_vein_rebuilds = r.Range(0, INT64_MAX);
    const int64_t nmarks = r.Count((int64_t)w * h);
    for (int64_t i = 0; (i < nmarks) && r.Ok(); i++) {
        const uint32_t at = r.U32();
        if (at >= (uint64_t)w * (uint64_t)h) r.Fail("mark");
        else m_marks.push_back((int32_t)at);
    }
    m_change_seq = r.U64();
    for (int32_t &at : m_change_log) {
        const uint32_t v = r.U32();
        if ((v >= (uint64_t)w * (uint64_t)h) && (v != 0)) r.Fail("change log");
        at = (int32_t)v;
    }
    if (r.Ok() && (!m_reach.Load(r) || (m_reach.Width() != w) || (m_reach.Height() != h))) r.Fail("reach map");
    m_room_records.Clear();
    m_room_records.SetNextId(r.U32());
    for (int64_t i = 0; (i < DAI_MAX_ROOM_RECORDS) && r.Ok(); i++) {
        DaiRoomRecord rr;
        rr.id = r.U32();
        if (rr.id == 0) continue;
        rr.kind = (RoomKind)r.Range(1, 255);
        rr.ax = (DaiSlabCoord)r.Range(0, (w > 0) ? w - 1 : 0); rr.ay = (DaiSlabCoord)r.Range(0, (h > 0) ? h - 1 : 0);
        rr.index = r.Range(0, INT32_MAX); rr.slabs = r.Range(0, (int64_t)w * h);
        rr.box.x0 = (DaiSlabCoord)r.Range(0, (w > 0) ? w - 1 : 0); rr.box.y0 = (DaiSlabCoord)r.Range(0, (h > 0) ? h - 1 : 0);
        rr.box.x1 = (DaiSlabCoord)r.Range(rr.box.x0, (w > 0) ? w - 1 : 0); rr.box.y1 = (DaiSlabCoord)r.Range(rr.box.y0, (h > 0) ? h - 1 : 0);
        rr.since = (DaiTurn)r.U64(); rr.lost = (DaiTurn)r.U64();
        if (r.Ok() && !m_room_records.RestoreAt(i, rr)) r.Fail("room record slot");
    }
    const int64_t mx = (w > 0) ? (int64_t)w * 3 - 1 : 0, my = (h > 0) ? (int64_t)h * 3 - 1 : 0;
    const int64_t ndef = r.Count(DAI_MAX_ENEMY_DEFENCES);
    for (int64_t i = 0; (i < ndef) && r.Ok(); i++) {
        DaiEnemyDefence d;
        DaiEnemyDefenceLook &l = d.look;
        l.ref = r.Ref();
        l.owner = (PlayerNumber)r.Range(0, 255); l.door = r.Bool(); l.model = (DaiThingModel)r.Range(0, 255);
        l.x = (DaiSubtlCoord)r.Range(0, mx); l.y = (DaiSubtlCoord)r.Range(0, my);
        l.health = r.I64(); l.locked = r.Bool(); l.spent = r.Bool(); d.seen = (DaiTurn)r.U64();
        m_enemy_defences.push_back(d);
    }
    const int64_t nrooms = r.Count(DAI_MAX_RIVAL_ROOMS);
    for (int64_t i = 0; (i < nrooms) && r.Ok(); i++) {
        DaiRivalRoom rm;
        DaiRivalRoomLook &l = rm.look;
        l.index = r.Range(0, INT32_MAX); l.owner = (PlayerNumber)r.Range(0, 255); l.kind = (RoomKind)r.Range(0, 255);
        l.slabs_seen = r.Range(0, (int64_t)w * h);
        l.box.x0 = (DaiSlabCoord)r.Range(0, (w > 0) ? w - 1 : 0); l.box.y0 = (DaiSlabCoord)r.Range(0, (h > 0) ? h - 1 : 0);
        l.box.x1 = (DaiSlabCoord)r.Range(l.box.x0, (w > 0) ? w - 1 : 0); l.box.y1 = (DaiSlabCoord)r.Range(l.box.y0, (h > 0) ? h - 1 : 0);
        l.x = (DaiSlabCoord)r.Range(0, (w > 0) ? w - 1 : 0); l.y = (DaiSlabCoord)r.Range(0, (h > 0) ? h - 1 : 0);
        rm.seen = (DaiTurn)r.U64();
        m_rival_rooms.push_back(rm);
    }
    if (!r.Ok()) {
        Init(0, 0);
        m_own = DaiOwnLook();
        m_nrooms = 0;
        m_player = -1;
    }
    return r.Ok();
}
