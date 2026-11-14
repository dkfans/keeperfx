#include "pre_inc.h"
#include "daimon/DaiFortify.h"

#include <algorithm>
#include <cstring>
#include <iterator>

#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
bool open_ground(DaiTerrain t)
{
    return (t == DaiTerrain::Floor) || (t == DaiTerrain::Room) || (t == DaiTerrain::Door) || (t == DaiTerrain::Water);
}

int64_t kind_named(const DaiStockLook &st, bool door, const char *name, bool in_stock)
{
    int64_t found = 0;
    st.ForEachKind(door, [&](int64_t s, int64_t k) {
        if ((found != 0) || (strcmp(door ? st.door_names[s] : st.trap_names[s], name) != 0)) return;
        const bool placeable = (door ? st.doors_placeable[s] : st.traps_placeable[s]) > 0;
        const bool makeable = door ? st.doors_buildable[s] : st.traps_buildable[s];
        if (placeable || (!in_stock && makeable)) found = k;
    });
    return found;
}

constexpr const DaiSlabXY (&SIDES)[4] = DAI_N4_STEPS;

bool lava_trap_at(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y)
{
    const DaiStockLook &st = k.Stock();
    for (const DaiTrapLook &tr : k.Traps()) {
        if ((DaiSubtileSlab(tr.x) != x) || (DaiSubtileSlab(tr.y) != y)) continue;
        if (strcmp(st.Name(false, tr.model), "LAVA") == 0) return true;
    }
    return false;
}

/** Placeable now, or makeable now when not `in_stock`. */
bool have_kind(const DaiStockLook &st, bool door, int64_t kind, bool in_stock)
{
    const int64_t s = st.Slot(door, kind);
    if (s < 0) return false;
    if ((door ? st.doors_placeable[s] : st.traps_placeable[s]) > 0) return true;
    const bool makeable = door ? (st.doors_buildable[s] && (st.door_levels[s] <= st.manufacture_level))
                               : (st.traps_buildable[s] && (st.trap_levels[s] <= st.manufacture_level));
    return !in_stock && makeable;
}

/** A map pack's own kind (not KeeperFX's) of a role, the best-worth one had now; 0 none. `skip_gas`: no gas trap. */
int64_t custom_kind(const DaiContent *content, const DaiStockLook &st, DaiContentClass c, DaiContentRole role, bool in_stock,
                    bool skip_gas, int64_t *worth)
{
    int64_t best = 0;
    *worth = 0;
    if (content == nullptr) return 0;
    for (const DaiContentEntry *e : content->OfRole(c, role)) {
        if (e->base || (skip_gas && e->gas)) continue;
        if (!have_kind(st, c == DaiContentClass::Door, e->kind, in_stock)) continue;
        // A trap's worth is the stock's rating (as the named kinds'), unless daimon.toml set one; a blocker's its health.
        const int64_t s = st.Slot(false, e->kind);
        const bool rated = (c == DaiContentClass::Trap) && (role != DaiContentRole::Blocker) && !e->overridden && (s >= 0);
        const int64_t w = rated ? st.trap_ratings[s] : e->worth;
        if ((best == 0) || (w > *worth)) { best = e->kind; *worth = w; }
    }
    return best;
}

/** At the heart: KeeperFX's kinds as before; a pack's own only if it is a `pressure` kind. */
bool heart_role_ok(const DaiContent *content, int64_t kind)
{
    const DaiContentEntry *e = (content != nullptr) ? content->Find(DaiContentClass::Trap, kind) : nullptr;
    return (e == nullptr) || e->base || (e->role == DaiContentRole::Pressure);
}

/** Closed on two opposite sides, or on three. */
bool one_wide(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y)
{
    const bool n = DaiFortify::Closed(k, x, y - 1), s = DaiFortify::Closed(k, x, y + 1);
    const bool e = DaiFortify::Closed(k, x + 1, y), w = DaiFortify::Closed(k, x - 1, y);
    return (n && s) || (e && w) || (((int)n + (int)s + (int)e + (int)w) >= 3);
}

/** A share (DaiFix 0..1) of `max`, a half taking all of it: 0.5 is the plan's default share. */
int64_t share(int64_t max, DaiFix s)
{
    return std::min<int64_t>(max, FixToInt(FixMul(FixFromInt(2 * max), s)));
}
}

bool DaiFortify::Closed(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y)
{
    if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height())) return true;
    const DaiTerrain t = k.SlabAt(x, y).terrain;
    return !open_ground(t) && (t != DaiTerrain::Unknown) && (t != DaiTerrain::Lava) && (t != DaiTerrain::Abyss);
}

bool DaiFortify::FreeClaimed(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y)
{
    if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height())) return false;
    const DaiSlabInfo s = k.SlabAt(x, y);
    if ((s.terrain != DaiTerrain::Floor) || (s.owner != p)) return false;
    for (const DaiTrapLook &tr : k.Traps()) if ((DaiSubtileSlab(tr.x) == x) && (DaiSubtileSlab(tr.y) == y)) return false;
    return k.DoorAt(x, y) == nullptr;
}

bool DaiFortify::DoorSite(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y)
{
    return FreeClaimed(k, p, x, y) && one_wide(k, x, y);
}

bool DaiFortify::MoatSite(const DaiKnowledge &k, const DaiThreatModel &t, PlayerNumber p, DaiSlabXY post, DaiSlabXY *lava,
                          int64_t *weight)
{
    if (!one_wide(k, post.x, post.y)) return false;
    int64_t best = -1;
    for (const DaiThreatRoute &rt : t.Routes()) {
        if (rt.weight <= best) continue;
        for (size_t i = 1; i < rt.path.size(); i++) {
            const DaiSlabXY a = rt.path[i - 1], b = rt.path[i];
            if ((b.x != post.x) || (b.y != post.y)) continue;
            const int sx = (int)(b.x - a.x), sy = (int)(b.y - a.y);
            const DaiSlabInfo s = k.SlabAt(a.x, a.y);
            const bool ground = (s.terrain == DaiTerrain::Lava) || FreeClaimed(k, p, a.x, a.y) || ((s.owner == p) && lava_trap_at(k, a.x, a.y));
            if (ground && (t.TrafficAt(a.x, a.y) == 0) && Closed(k, a.x - sy, a.y + sx) && Closed(k, a.x + sy, a.y - sx)) {
                *lava = a;
                best = rt.weight;
            }
            break;
        }
    }
    if ((best >= 0) && (weight != nullptr)) *weight = best;
    return best >= 0;
}

bool DaiFortify::BoulderSite(const DaiKnowledge &k, const DaiThreatModel &t, PlayerNumber p, const std::vector<DaiSlabXY> &path,
                             DaiSlabXY *at)
{
    int64_t run = 0;
    int dx = 0, dy = 0;
    for (size_t i = 1; i < path.size(); i++) {
        const DaiSlabXY a = path[i - 1], b = path[i];
        const int sx = (int)(b.x - a.x), sy = (int)(b.y - a.y);
        // One wide across the way, the keeper's free floor, off its creatures' ways.
        const bool ok = FreeClaimed(k, p, b.x, b.y) && (t.TrafficAt(b.x, b.y) == 0)
                     && Closed(k, b.x - sy, b.y + sx) && Closed(k, b.x + sy, b.y - sx);
        if (!ok) { run = 0; continue; }
        if ((run > 0) && ((sx != dx) || (sy != dy))) run = 0;   // a turn: the boulder would stop there
        dx = sx;
        dy = sy;
        run++;
        if (run >= DAI_BOULDER_RUN) {
            *at = b;
            return true;
        }
    }
    return false;
}

bool DaiFortify::LandingSite(const DaiKnowledge &k, PlayerNumber p, const std::vector<DaiSlabXY> &path, DaiSlabXY *at)
{
    for (size_t i = 1; i < path.size(); i++) {
        const DaiSlabXY a = path[i - 1], b = path[i];
        if (k.SlabAt(b.x, b.y).owner != p) continue;
        if (k.SlabAt(a.x, a.y).terrain != DaiTerrain::Water) continue;
        if (!FreeClaimed(k, p, b.x, b.y)) return false;
        *at = b;
        return true;
    }
    return false;
}

int64_t DaiFortify::LandingsStanding(const DaiKnowledge &k)
{
    int64_t n = 0;
    for (const DaiRoomLook &r : k.Rooms()) {
        if (((r.roles & DAI_ROLE_GUARD) == 0) || (r.slabs != 1)) continue;
        bool water = false;
        for (const DaiSlabXY d : SIDES) water = water || (k.SlabAt((DaiSlabCoord)(r.ax + d.x), (DaiSlabCoord)(r.ay + d.y)).terrain == DaiTerrain::Water);
        n += water ? 1 : 0;
    }
    return n;
}

int64_t DaiFortify::MoatsStanding(const DaiKnowledge &k)
{
    int64_t n = 0;
    for (const DaiRoomLook &r : k.Rooms()) {
        if (((r.roles & DAI_ROLE_GUARD) == 0) || (r.slabs != 1)) continue;
        bool water = false, lava = false;
        for (const DaiSlabXY d : SIDES) {
            const DaiSlabCoord x = (DaiSlabCoord)(r.ax + d.x), y = (DaiSlabCoord)(r.ay + d.y);
            const DaiTerrain t = k.SlabAt(x, y).terrain;
            water = water || (t == DaiTerrain::Water);
            lava = lava || (t == DaiTerrain::Lava) || lava_trap_at(k, x, y);
        }
        n += (lava && !water) ? 1 : 0;
    }
    return n;
}

DaiFortWho DaiFortify::Who(const DaiKnowledge &k)
{
    DaiFortWho who;
    int64_t all = 0, gas = 0, levels = 0;
    DaiFix magic = 0;
    for (const DaiThingLook &e : k.Enemies()) {
        if (e.heart || e.digger) continue;
        const DaiKindLook &kind = k.Kind(e.model);
        const int64_t lv = std::max<int64_t>(e.level, 1);
        // Doors face heroes as much as keepers: every enemy counts toward the magical share.
        levels += lv;
        magic += kind.magic_share * lv;
        if (!k.RivalKeeper(e.owner)) continue;
        all += lv;
        if (kind.immune_gas) gas += lv;
        who.boulder_immune = who.boulder_immune || kind.immune_boulder;
    }
    who.gas_immune = (all > 0) && (2 * gas >= all);
    if (levels > 0) who.magic_share = (DaiFix)(magic / levels);
    return who;
}

int64_t DaiFortify::TrapKind(const DaiStockLook &st, DaiFortRole role, bool traffic, bool in_stock, const DaiFortWho &who,
                             const DaiContent *content)
{
    switch (role) {
    case DaiFortRole::Alarm: return kind_named(st, false, "ALARM", in_stock);
    case DaiFortRole::Boulder: return who.boulder_immune ? 0 : kind_named(st, false, "BOULDER", in_stock);
    case DaiFortRole::LavaMoat: return kind_named(st, false, "LAVA", in_stock);
    case DaiFortRole::Pressure: case DaiFortRole::Imps: {
        // Word of Power and lightning spare the keeper's own creatures; gas does not, so only off their ways (and an
        // imp trap stands where its own imps work too); nor against a force gas does not hurt.
        const bool own_way = traffic || (role == DaiFortRole::Imps);
        int64_t kind = 0;
        for (const char *name : { "WORD_OF_POWER", "LIGHTNING", "POISON_GAS" }) {
            if ((own_way || who.gas_immune) && (strcmp(name, "POISON_GAS") == 0)) continue;
            kind = kind_named(st, false, name, in_stock);
            if (kind != 0) break;
        }
        // A pack's own pressure trap that outrates it (a sentry), gas only where gas may go.
        int64_t worth = 0;
        const int64_t custom = custom_kind(content, st, DaiContentClass::Trap, DaiContentRole::Pressure, in_stock,
                                           own_way || who.gas_immune, &worth);
        const int64_t s = st.Slot(false, kind);
        if ((custom != 0) && ((kind == 0) || (worth > ((s >= 0) ? st.trap_ratings[s] : 0)))) return custom;
        return kind;
    }
    case DaiFortRole::Heart: {
        // The best-rated trap it has or can make now (DaiTrapRating; its workshop's level, P9: a Word of Power wanted
        // from a level-2 workshop was never made) but a boulder (it rolls off), an alarm or lava; gas only off its own
        // creatures' ways and against a force it hurts.
        int64_t best = 0, best_rating = 0;
        st.ForEachKind(false, [&](int64_t s, int64_t k) {
            const char *name = st.trap_names[s];
            if ((strcmp(name, "BOULDER") == 0) || (strcmp(name, "ALARM") == 0) || (strcmp(name, "LAVA") == 0)) return;
            if ((traffic || who.gas_immune) && (strcmp(name, "POISON_GAS") == 0)) return;
            if (!heart_role_ok(content, k)) return;
            const bool makeable = st.traps_buildable[s] && (st.trap_levels[s] <= st.manufacture_level);
            if ((st.traps_placeable[s] <= 0) && (in_stock || !makeable)) return;
            if (st.trap_ratings[s] > best_rating) { best_rating = st.trap_ratings[s]; best = k; }
        });
        return best;
    }
    case DaiFortRole::Barricade: {
        // A pack's blocker, the one with the most health (its worth) had now; KeeperFX has none of its own.
        int64_t worth = 0;
        return custom_kind(content, st, DaiContentClass::Trap, DaiContentRole::Blocker, in_stock, false, &worth);
    }
    default: return 0;
    }
}

int64_t DaiFortify::DoorWorth(const DaiContentEntry &e, const DaiFortWho &who, int64_t spare_gold)
{
    const DaiFix m = (who.magic_share < 0) ? DAI_DOOR_MAGIC_UNSEEN : std::min<DaiFix>(who.magic_share, DAI_FIX_ONE);
    int64_t worth = e.health;
    if (e.resist_non_magic) worth = e.health * (8 * (DAI_FIX_ONE - m) + m) / DAI_FIX_ONE;
    if (e.midas) worth += e.health * std::max<int64_t>(spare_gold, 0);
    return worth;
}

bool DaiFortify::SecretSite(const DaiKnowledge &k, DaiSlabXY at)
{
    const int64_t r = DAI_SECRET_SEEN_REACH;
    for (DaiSlabCoord y = (DaiSlabCoord)std::max<int64_t>(at.y - r, 0); y <= std::min<int64_t>(at.y + r, k.Height() - 1); y++)
        for (DaiSlabCoord x = (DaiSlabCoord)std::max<int64_t>(at.x - r, 0); x <= std::min<int64_t>(at.x + r, k.Width() - 1); x++) {
            const DaiSlabInfo s = k.SlabAt(x, y);
            if ((s.owner >= 0) && k.RivalKeeper(s.owner)) return false;
        }
    for (const DaiThingLook &e : k.Enemies())
        if (DaiChess(DaiSubtileSlab(e.x), DaiSubtileSlab(e.y), at.x, at.y) <= r) return false;
    return true;
}

int64_t DaiFortify::DoorKind(const DaiStockLook &st, bool in_stock, bool secret, const DaiContent *content,
                             const DaiFortWho &who, int64_t spare_gold)
{
    if ((content != nullptr) && !content->Entries(DaiContentClass::Door).empty()) {
        // By effective health, every door kind the level has; a secret one first where the enemy has not seen.
        int64_t best = 0, best_worth = -1, best_secret = 0, secret_worth = -1;
        for (const DaiContentEntry &e : content->Entries(DaiContentClass::Door)) {
            if ((e.role == DaiContentRole::Ignore) || !have_kind(st, true, e.kind, in_stock)) continue;
            const int64_t w = DoorWorth(e, who, spare_gold);
            if (w > best_worth) { best = e.kind; best_worth = w; }
            if (e.secret && (w > secret_worth)) { best_secret = e.kind; secret_worth = w; }
        }
        return (secret && (best_secret != 0)) ? best_secret : best;
    }
    if (secret) {
        const int64_t kind = kind_named(st, true, "SECRET", in_stock);
        if (kind != 0) return kind;
    }
    int64_t kind = 0;
    for (const char *name : { "STEEL", "MAGIC", "BRACED", "WOOD" }) {
        kind = kind_named(st, true, name, in_stock);
        if (kind != 0) break;
    }
    return kind;
}

std::vector<DaiFortPlacement> DaiFortify::Plan(const DaiKnowledge &k, const DaiThreatModel &t, PlayerNumber p,
                                               const DaimonGlobalConfig &cfg, const DaiPersonality &per, uint32_t contested,
                                               const std::vector<DaiFortImpSite> &imps)
{
    std::vector<DaiFortPlacement> doors, traps;
    std::vector<uint8_t> taken((size_t)k.Width() * k.Height(), 0);
    auto take = [&](const DaiSlabXY &s) { taken[(size_t)s.y * k.Width() + s.x] = 1; };
    auto is_taken = [&](const DaiSlabXY &s) { return taken[(size_t)s.y * k.Width() + s.x] != 0; };
    auto add = [&](std::vector<DaiFortPlacement> &v, DaiSlabXY at, DaiFortRole role, int64_t score, bool taking = true) {
        DaiFortPlacement f;
        f.at = at;
        f.role = role;
        f.traffic = t.TrafficAt(at.x, at.y) > 0;
        f.score = score;
        v.push_back(f);
        if (taking) take(at);
    };
    auto in_doors = [&](const DaiSlabXY &at) -> DaiFortPlacement * {
        for (DaiFortPlacement &f : doors) if ((f.at.x == at.x) && (f.at.y == at.y)) return &f;
        return nullptr;
    };
    int64_t doors_standing = LandingsStanding(k) + MoatsStanding(k);
    for (const DaiChokepoint &cp : t.Chokepoints()) doors_standing += (cp.own && (k.DoorAt(cp.x, cp.y) != nullptr)) ? 1 : 0;
    const int64_t more = (contested != 0) ? 3 : 2;      // contest: the caps half as much again
    const int64_t door_cap = std::max<int64_t>(0, share(cfg.defence_max_doors * more / 2, per.defence_door_preference) - doors_standing);
    const int64_t trap_cap = std::max<int64_t>(0, share(cfg.defence_max_traps * more / 2, per.defence_trap_usage) - (int64_t)k.Traps().size());
    // First what only one slab will do: an alarm where each route first sets foot on the keeper's ground, a boulder at
    // the inner end of its straight run, a guard post where it lands out of water. Then doors on the busiest one-wide
    // chokepoints (some made moats), then pressure traps on the rest.
    const bool posts = k.Build(DaiRoom::GuardPost).available;
    auto own_crossing = [&](const DaiSlabXY &s) {     // its own creatures wade onto the slab: a post would stop them
        if (t.TrafficAt(s.x, s.y) == 0) return false;
        for (const DaiSlabXY d : SIDES) {
            const DaiSlabCoord x = (DaiSlabCoord)(s.x + d.x), y = (DaiSlabCoord)(s.y + d.y);
            if ((k.SlabAt(x, y).terrain == DaiTerrain::Water) && (t.TrafficAt(x, y) > 0)) return true;
        }
        return false;
    };
    // The perimeter's gates (plan 13 section 32): a door on each, before anything else and outside the cap -- the
    // perimeter is closed only with them.
    std::vector<DaiFortPlacement> gate_doors;
    for (const DaiSlabXY &g : k.Gates())
        if (!is_taken(g) && DoorSite(k, p, g.x, g.y)) add(gate_doors, g, DaiFortRole::Door, DAI_GATE_DOOR_SCORE);
    for (const DaiThreatRoute &rt : t.Routes()) {
        const bool rival = (rt.source == DaiThreatSource::Rival) && (rt.rival >= 0) && (rt.rival < 32) && ((contested >> rt.rival) & 1u);
        const int64_t weight = rival ? 2 * rt.weight : rt.weight;
        DaiSlabXY land;
        const bool lands = posts && LandingSite(k, p, rt.path, &land) && !own_crossing(land);
        for (const DaiSlabXY &s : rt.path) {
            if (k.SlabAt(s.x, s.y).owner != p) continue;
            if (lands && (s.x == land.x) && (s.y == land.y)) continue;     // the post there: the alarm on the next slab
            if (!is_taken(s) && FreeClaimed(k, p, s.x, s.y)) add(traps, s, DaiFortRole::Alarm, weight);
            break;
        }
        DaiSlabXY at;
        if ((per.defence_boulder_use != 0) && BoulderSite(k, t, p, rt.path, &at) && !is_taken(at)) add(traps, at, DaiFortRole::Boulder, weight);
        if (!lands || is_taken(land)) continue;
        if (DaiFortPlacement *f = in_doors(land)) f->score += weight;  // routes landing on one slab: weights together
        else add(doors, land, DaiFortRole::Landing, weight, false);
    }
    // Moats (P5.14): a one-slab post of its own on a route gets a lava trap on the route's slab outward of it, while the
    // trap cap has room (a lava already there needs none); later a kept door's place is made such a post. The trap
    // waits for its post: the post is what leaves whoever falls into the lava only the way back.
    std::vector<DaiFortPlacement> moats;
    const bool lava_kind = TrapKind(k.Stock(), DaiFortRole::LavaMoat, false, false) != 0;
    auto moat = [&](DaiSlabXY post, int64_t score, bool held) {
        DaiSlabXY o;
        if (!MoatSite(k, t, p, post, &o) || is_taken(o)) return false;
        const bool needs = FreeClaimed(k, p, o.x, o.y);
        if (needs && (!lava_kind || ((int64_t)moats.size() >= trap_cap))) return false;
        take(o);
        if (needs) {
            add(moats, o, DaiFortRole::LavaMoat, score);
            moats.back().held = held;
        }
        return true;
    };
    for (const DaiRoomLook &r : k.Rooms()) {
        if (((r.roles & DAI_ROLE_GUARD) == 0) || (r.slabs != 1)) continue;
        DaiSlabXY o;
        int64_t weight = 0;
        if (MoatSite(k, t, p, DaiSlabXY{ r.ax, r.ay }, &o, &weight)) moat(DaiSlabXY{ r.ax, r.ay }, weight, false);
    }
    for (const DaiChokepoint &cp : t.Chokepoints()) {   // busiest first
        const DaiSlabXY at{ cp.x, cp.y };
        if (cp.own && !is_taken(at) && (in_doors(at) == nullptr) && DoorSite(k, p, cp.x, cp.y)) add(doors, at, DaiFortRole::Door, cp.score, false);
    }
    // Landings and doors ranked together under the door cap; only those kept take their slab.
    auto by_score = [](const DaiFortPlacement &a, const DaiFortPlacement &b) { return a.score > b.score; };
    std::stable_sort(doors.begin(), doors.end(), by_score);
    if ((int64_t)doors.size() > door_cap) doors.resize((size_t)door_cap);
    for (const DaiFortPlacement &f : doors) take(f.at);
    if (posts) for (DaiFortPlacement &f : doors) if ((f.role == DaiFortRole::Door) && moat(f.at, f.score, true)) f.role = DaiFortRole::MoatPost;
    doors.insert(doors.begin(), gate_doors.begin(), gate_doors.end());
    for (const DaiChokepoint &cp : t.Chokepoints()) {
        const DaiSlabXY at{ cp.x, cp.y };
        if (!cp.own || is_taken(at) || !FreeClaimed(k, p, cp.x, cp.y)) continue;
        bool near = false;
        for (const DaiFortPlacement &f : traps) near = near || (DaiChess(f.at.x, f.at.y, cp.x, cp.y) < 2);
        for (const DaiTrapLook &tr : k.Traps()) near = near || (DaiChess(DaiSubtileSlab(tr.x), DaiSubtileSlab(tr.y), cp.x, cp.y) < 2);
        if (!near) add(traps, at, DaiFortRole::Pressure, cp.score);
    }
    // Imp traps (P5.13): where rival imps keep converting its ground, by the imps seen there.
    for (const DaiFortImpSite &s : imps) {
        if (is_taken(s.at) || !FreeClaimed(k, p, s.at.x, s.at.y)) continue;
        bool near = false;
        for (const DaiFortPlacement &f : traps) near = near || (DaiChess(f.at.x, f.at.y, s.at.x, s.at.y) < 2);
        for (const DaiTrapLook &tr : k.Traps()) near = near || (DaiChess(DaiSubtileSlab(tr.x), DaiSubtileSlab(tr.y), s.at.x, s.at.y) < 2);
        if (!near) add(traps, s.at, DaiFortRole::Imps, s.seen * DAI_IMP_TRAP_WEIGHT);
    }
    // Barricades (Phase 7): a pack's blocker on the route's slab just outward of a door kept or standing, or of a
    // chokepoint's trap planned or standing (just inward when that is not its floor) -- the attackers stop to break
    // it at the door, or in the trap's reach.
    // Under the trap cap, after the traps that hurt (half their site's score).
    if (TrapKind(k.Stock(), DaiFortRole::Barricade, false, false, DaiFortWho(), &k.Content()) != 0) {
        std::vector<std::pair<DaiSlabXY, int64_t>> held;
        for (const DaiFortPlacement &f : doors) held.emplace_back(f.at, f.score);
        for (const DaiFortPlacement &f : traps) if (f.role == DaiFortRole::Pressure) held.emplace_back(f.at, f.score);
        for (const DaiChokepoint &cp : t.Chokepoints()) {
            if (!cp.own) continue;
            bool standing = (k.DoorAt(cp.x, cp.y) != nullptr);
            for (const DaiTrapLook &tr : k.Traps()) standing = standing || ((DaiSubtileSlab(tr.x) == cp.x) && (DaiSubtileSlab(tr.y) == cp.y));
            if (standing) held.emplace_back(DaiSlabXY{ cp.x, cp.y }, cp.score);
        }
        std::vector<DaiFortPlacement> barricades;
        for (const auto &[site, score] : held) {
            for (const DaiThreatRoute &rt : t.Routes()) {
                size_t i = 0;
                while ((i < rt.path.size()) && ((rt.path[i].x != site.x) || (rt.path[i].y != site.y))) i++;
                if (i >= rt.path.size()) continue;
                // The path runs from the route's source: outward first; inward when the outer slab is not its floor
                // (the attackers broke the door, or crossed the trap, and stall again).
                bool placed = false;
                for (const size_t j : { i - 1, i + 1 }) {
                    if ((i == 0) && (j == i - 1)) continue;
                    if (j >= rt.path.size()) continue;
                    const DaiSlabXY o = rt.path[j];
                    if (is_taken(o) || !FreeClaimed(k, p, o.x, o.y)) continue;
                    bool near = false;
                    for (const DaiFortPlacement &b : barricades) near = near || (DaiChess(b.at.x, b.at.y, o.x, o.y) < 2);
                    if (near) continue;
                    add(barricades, o, DaiFortRole::Barricade, score / 2);
                    placed = true;
                    break;
                }
                if (placed) break;
            }
        }
        traps.insert(traps.end(), barricades.begin(), barricades.end());
    }
    // Heart traps (P9): the last ditch, nearest the heart first, apart from the cap; those standing there count.
    std::vector<DaiFortPlacement> heart;
    const DaiOwnLook &own = k.Own();
    if ((per.defence_heart_traps > 0) && own.has_heart) {
        const DaiSlabCoord hx = DaiSubtileSlab(own.heart_x), hy = DaiSubtileSlab(own.heart_y);
        int64_t standing = 0;
        for (const DaiTrapLook &tr : k.Traps())
            standing += (DaiChess(DaiSubtileSlab(tr.x), DaiSubtileSlab(tr.y), hx, hy) <= DAI_HEART_TRAP_REACH) ? 1 : 0;
        for (int64_t d = 1; (d <= DAI_HEART_TRAP_REACH) && (standing + (int64_t)heart.size() < per.defence_heart_traps); d++) {
            for (DaiSlabCoord y = hy - d; y <= hy + d; y++) {
                for (DaiSlabCoord x = hx - d; x <= hx + d; x++) {
                    if ((DaiChess(x, y, hx, hy) != d) || (standing + (int64_t)heart.size() >= per.defence_heart_traps)) continue;
                    const DaiSlabXY at{ x, y };
                    if (!FreeClaimed(k, p, x, y) || is_taken(at)) continue;      // on the map first
                    bool near = false;
                    for (const DaiFortPlacement &f : heart) near = near || (DaiChess(f.at.x, f.at.y, x, y) < 2);
                    if (!near) add(heart, at, DaiFortRole::Heart, 0);
                }
            }
        }
    }
    std::stable_sort(traps.begin(), traps.end(), by_score);
    const int64_t room = trap_cap - (int64_t)moats.size();
    if ((int64_t)traps.size() > room) traps.resize((size_t)room);
    traps.insert(traps.end(), moats.begin(), moats.end());
    traps.insert(traps.end(), heart.begin(), heart.end());
    std::stable_sort(traps.begin(), traps.end(), by_score);
    std::vector<DaiFortPlacement> out;
    std::merge(doors.begin(), doors.end(), traps.begin(), traps.end(), std::back_inserter(out), by_score);
    return out;
}
