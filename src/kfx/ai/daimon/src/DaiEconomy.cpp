#include "pre_inc.h"
#include "daimon/DaiEconomy.h"

#include <algorithm>
#include <cinttypes>
#include <vector>

#include "daimon/DaiJob.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
/** A treasury counts as full when this share (x1024) of it holds gold. */
constexpr int64_t FULL_SHARE = 870;
/** Veins and dig routes a forward treasury was asked for, remembered (the oldest forgotten past this). */
constexpr size_t FORWARD_ROUTES = 16;

bool remembered(const std::vector<uint32_t> &ids, uint32_t id)
{
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

void remember(std::vector<uint32_t> &ids, uint32_t id)
{
    if (ids.size() >= FORWARD_ROUTES) ids.erase(ids.begin());
    ids.push_back(id);
}

DaiRequest weight(DaiGoalKind g, DaiFix delta, const char *why)
{
    DaiRequest r;
    r.goal = g;
    r.weight_delta = delta;
    snprintf(r.why, sizeof(r.why), "%s", why);
    return r;
}
}

void DaiEconomy::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    payday(c);
    pressure(c, w);
    treasury_size(c, w);
    emergency(c, w);
    forward_treasury(c, w);
    fortify(c, w);
}

namespace {
/** What a room's efficiency is worth (P9): the work rooms most (research, training, making), then the stores and the
 *  lair and hatchery; a portal, a bridge, a guard post, the heart nothing (not built for their efficiency). */
int64_t fortify_weight(uint64_t roles)
{
    if ((roles & (DAI_ROLE_RESEARCH | DAI_ROLE_MANUFACTURE | DaiWorkRole(DaiWork::Train))) != 0) return 3;
    if ((roles & (DAI_ROLE_GOLD | DAI_ROLE_LAIR | DAI_ROLE_FOOD | DAI_ROLE_SCAVENGE | DAI_ROLE_PRAY)) != 0) return 2;
    if ((roles & (DAI_ROLE_HEART | DAI_ROLE_PORTAL | DAI_ROLE_GUARD | DAI_ROLE_PASS_WATER | DAI_ROLE_PASS_LAVA)) != 0) return 0;
    return 1;
}
}

/** An imp dropped on a room's edge beside its earth, to fortify it (P9, the owner's idea): a fortified wall scores a
 *  room side 2 to earth's 1. The room and the spot worth most -- its weight, how far below full efficiency, the earth
 *  sides of the room within two slabs of the spot (the walls the imp goes on to) -- one drop each fortify_interval
 *  turns, with the imps the work wants (KeepImps met, imps_min to spare) and no hazard hot. */
void DaiEconomy::fortify(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiPersonality &p = c.personality;
    if ((p.economy_fortify == 0) || (c.now < m_fortify_next)) return;
    if (DaiJob *j = w.FindJob(DAI_JOB_FORTIFY_ROOM, 0)) if (!DaiJobEnded(j->state)) return;
    m_fortify_next = c.now + (DaiTurn)p.economy_fortify_interval;
    const DaiKnowledge &k = c.know;
    if (c.threat.AnyHot()) return;
    int64_t imps = 0;
    for (const DaiImpLook &i : k.OwnImps()) imps += i.tunneller ? 0 : 1;
    const DaiGoal *keep = c.goals.Find(DaiGoalKind::KeepImps);
    if ((imps <= p.workforce_imps_min) || ((keep != nullptr) && (k.Own().imps < keep->p.count))) return;
    auto earth = [&](DaiSlabCoord x, DaiSlabCoord y) {
        if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height())) return false;
        const DaiSlabInfo s = k.SlabAt(x, y);
        return (s.terrain == DaiTerrain::Dirt) && s.revealed && !s.marked;
    };
    const DaiSlabXY (&kSides)[4] = DAI_N4_STEPS;
    int64_t best = 0;
    DaiFortifyParams fp;
    for (const DaiRoomLook &r : k.Rooms()) {
        const int64_t weight = fortify_weight(r.roles);
        if ((weight == 0) || (r.efficiency * 100 >= p.economy_fortify_below * 256)) continue;
        const DaiSlabCoord cx = (DaiSlabCoord)r.cx, cy = (DaiSlabCoord)r.cy;
        int64_t reach = 3;
        while (reach * reach < r.slabs) reach++;
        for (DaiSlabCoord y = cy - reach; y <= cy + reach; y++) {
            for (DaiSlabCoord x = cx - reach; x <= cx + reach; x++) {
                if ((x < 0) || (y < 0) || (x >= k.Width()) || (y >= k.Height())) continue;
                const DaiSlabInfo s = k.SlabAt(x, y);
                if ((s.owner != c.player) || (s.terrain != DaiTerrain::Room) || (s.room != r.kind)) continue;
                for (const DaiSlabXY &d : kSides) {
                    if (!earth(x + d.x, y + d.y)) continue;
                    // The room's earth sides within two slabs of it: those the imp goes on to.
                    int64_t near = 0;
                    for (DaiSlabCoord yy = y - 2; yy <= y + 2; yy++)
                        for (DaiSlabCoord xx = x - 2; xx <= x + 2; xx++) {
                            if ((xx < 0) || (yy < 0) || (xx >= k.Width()) || (yy >= k.Height())) continue;
                            const DaiSlabInfo n = k.SlabAt(xx, yy);
                            if ((n.owner != c.player) || (n.terrain != DaiTerrain::Room) || (n.room != r.kind)) continue;
                            for (const DaiSlabXY &e : kSides) near += earth(xx + e.x, yy + e.y) ? 1 : 0;
                        }
                    const int64_t score = weight * near * (256 - r.efficiency);
                    if (score <= best) continue;
                    best = score;
                    fp.stl_x = (DaiSubtlCoord)(DaiSlabSubtileCenter(x) + d.x);
                    fp.stl_y = (DaiSubtlCoord)(DaiSlabSubtileCenter(y) + d.y);
                    fp.wall_x = x + d.x;
                    fp.wall_y = y + d.y;
                }
            }
        }
    }
    if (best == 0) return;
    // The nearest imp free of work and gold (one working, or laden, only if none is).
    const DaiImpLook *imp = nullptr;
    int64_t imp_d = 0;
    for (const DaiImpLook &i : k.OwnImps()) {
        if (i.tunneller || i.training || i.sacrificing || !i.pickable) continue;
        const int64_t d = ((i.gold > 0) ? 2000 : 0) + (i.working ? 1000 : 0)
                        + std::max<int64_t>(std::abs(DaiSubtileSlab(i.x) - fp.wall_x), std::abs(DaiSubtileSlab(i.y) - fp.wall_y));
        if ((imp == nullptr) || (d < imp_d)) { imp = &i; imp_d = d; }
    }
    if (imp == nullptr) return;
    fp.imp = imp->ref;
    DaiJob *j = w.AddJob(DAI_JOB_FORTIFY_ROOM, 0, FixFromInt(1));
    if (j == nullptr) return;
    j->p = fp;
    DaiLog::Info(c.player, "fortify: imp %d dropped by (%d,%d), worth %" PRId64, (int)imp->ref.idx, (int)fp.wall_x, (int)fp.wall_y, best);
}

/** Library slabs the keeper could lose and still keep every spellbook. A library holds slabs / 2 x efficiency / 256
 *  books and researchers together (terrain.cfg: slabs_div2_wth_effcncy); a book on a sold slab is moved elsewhere in
 *  the library, else (the only copy) to another library, else it is destroyed and its spell must be researched again
 *  (kill_room_contents_at_subtile); and once the library has shrunk, books beyond its capacity are moved out or lost
 *  the same way (room_storage_recount). So a sale may only take slabs the books do not need, with two to spare: the
 *  room's efficiency falls as it shrinks. */
int64_t DaiEconomy::library_spare(const DaiKnowledge &k)
{
    int64_t slabs = 0, books = 0, slab_eff = 0;
    for (const DaiRoomLook &r : k.Rooms()) {
        if ((r.roles & DAI_ROLE_SPELLBOOKS) == 0) continue;
        slabs += r.slabs;
        books += r.stored;
        slab_eff += r.slabs * r.efficiency;
    }
    if (slabs == 0) return 0;
    const int64_t eff = std::max<int64_t>(slab_eff / slabs, 1);
    const int64_t needed = (books * 2 * 256 + eff - 1) / eff + 2;
    return std::max<int64_t>(slabs - needed, 0);
}

/** Short at the next pay day even with the pressure on: sell (04 section 8). The shortfall in slabs, at half the
 *  room's price a slab (what selling gives back), of the first room in this order that may go: one the programme
 *  does not want at all, then the slabs beyond a Build goal -- the workshop's and the training room's, the library's
 *  last and only those its spellbooks can spare (library_spare; Construction sells library slabs without a book). Never the treasury, lairs, hatcheries, the portal
 *  or bridges. A room Construction could not sell from is passed over until the keeper is out of the emergency. */
void DaiEconomy::emergency(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    static const DaiRoom unused_first[] = { DaiRoom::Prison, DaiRoom::Torture, DaiRoom::Temple, DaiRoom::Graveyard, DaiRoom::Scavenger,
                                            DaiRoom::Barracks, DaiRoom::GuardPost, DaiRoom::Training, DaiRoom::Workshop };
    static const DaiRoom extra_then[] = { DaiRoom::Workshop, DaiRoom::Training, DaiRoom::Library };
    const DaiKnowledge &k = c.know;
    const DaiForecastOut &f = c.forecast;
    const DaiRequest *asked = nullptr;
    c.requests.ForEachService(DaiDomain::Construction, [&](const DaiRequest &r) {
        if ((r.from == DaiDomain::Economy) && (r.service == DaiService::SellRoom)) asked = &r;
    });
    // An imp below the floor counts as a shortfall too: without imps no gold comes in to pay anything.
    const DaiPowerLook &imp = k.Power(DaiPower::CreateImp);
    const int64_t imp_short = ((k.Own().imps < c.cfg.defence_hazard_imps_floor) && imp.available && k.Own().has_heart)
        ? imp.price - (f.gold_now - f.committed) : 0;
    if (!c.personality.economy_sell_rooms_in_emergency || (!f.short_next && (imp_short <= 0))) {
        if (asked != nullptr) w.WithdrawRequest(asked->id);
        m_sell_refused = 0;
        return;
    }
    if ((asked != nullptr) && asked->refused) {
        m_sell_refused |= 1u << asked->key;
        w.WithdrawRequest(asked->id);
        asked = nullptr;
    }
    const int64_t shortfall = std::max(f.short_next ? f.wages_next - (f.gold_now - f.committed) : 0, imp_short);
    if (shortfall <= 0) return;
    auto slabs = [&](DaiRoom r) {
        const RoomKind kind = k.RoomKindOf(r);
        int64_t have = 0;
        for (const DaiRoomLook &rm : k.Rooms()) if ((kind != 0) && (rm.kind == kind)) have += rm.slabs;
        return have;
    };
    auto ask = [&](DaiRoom r, int64_t spare) {
        const int64_t back = std::max<int64_t>(k.Build(r).price / 2, 1);
        DaiRequest q;
        q.kind = DaiRequestKind::Service;
        q.to = DaiDomain::Construction;
        q.service = DaiService::SellRoom;
        q.key = (int64_t)r;
        q.amount = std::min(spare, (shortfall + back - 1) / back);
        snprintf(q.why, sizeof(q.why), (imp_short > 0) ? "NO_IMPS" : "SHORT_NEXT");
        if ((asked != nullptr) && (asked->key != q.key)) w.WithdrawRequest(asked->id);
        if ((asked == nullptr) || (asked->key != q.key) || (asked->amount != q.amount)) w.PostRequest(q);
    };
    // A room still wanted is kept for the wages, not when the imps are gone: they come first.
    for (DaiRoom r : unused_first) {
        if ((m_sell_refused & (1u << (int)r)) || ((imp_short <= 0) && (c.goals.Find(DaiGoalKind::Build, (int64_t)r) != nullptr))) continue;
        const int64_t have = slabs(r);
        if (have > 0) { ask(r, have); return; }
    }
    for (DaiRoom r : extra_then) {
        if (m_sell_refused & (1u << (int)r)) continue;
        const DaiGoal *g = c.goals.Find(DaiGoalKind::Build, (int64_t)r);
        int64_t spare = slabs(r) - ((g != nullptr) ? g->p.count : 0);
        if (r == DaiRoom::Library) spare = std::min(spare, library_spare(k));
        if (spare > 0) { ask(r, spare); return; }
    }
    if (asked != nullptr) w.WithdrawRequest(asked->id);     // nothing more may go
}

/** A vein being mined whose gold has far to go: more than forward_treasury_turns of hauling left to the nearest
 *  treasury. Construction is asked for a treasury of the programme's smallest size by the vein's entry, worth at most
 *  the hauling it saves in digging (a site across water carries its bridge; 03 section 7.5). Asked once a vein. A dig
 *  route's gold counts too: its imps carry every load of a slab they dig through home before the next (level 52: a
 *  breach along a gold vein across the lake, a slab every thousand turns); the treasury by the slab before the first
 *  gold left on its way, once a route. Economy's treasury request is one (a new ask moves its site, and Construction
 *  looks for one again): while the last ask is younger than the limit none is made (two far veins, each remembered
 *  alone, moved it between them every look -- thousands of asks a game, a site started for each). */
void DaiEconomy::forward_treasury(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const int64_t limit = c.personality.economy_forward_treasury_turns;
    const int64_t hold = std::max<int64_t>(k.Own().imp_gold_hold, 1);
    if (limit <= 0) return;
    std::vector<DaiSlabXY> treasuries;
    for (const DaiRoomLook &r : k.Rooms()) if ((r.roles & DAI_ROLE_GOLD) != 0) treasuries.push_back(DaiSlabXY{ r.cx, r.cy });
    if (treasuries.empty()) return;
    const auto haul_of = [&](int64_t gold, DaiSlabCoord x, DaiSlabCoord y) {
        int64_t d = INT64_MAX;
        for (const DaiSlabXY &t : treasuries) d = std::min<int64_t>(d, std::abs(t.x - x) + std::abs(t.y - y));
        return (gold + hold - 1) / hold * 2 * d * c.cfg.dig_floor;
    };
    bool recent = false;
    c.requests.ForEach([&](const DaiRequest &r) {
        recent = recent || ((r.from == DaiDomain::Economy) && (r.kind == DaiRequestKind::Service) && (r.service == DaiService::PlaceRoom)
                            && (c.now < r.posted + (DaiTurn)limit));
    });
    if (recent) return;
    const DaiVein *far = nullptr;
    int64_t far_haul = 0;
    k.ForEachVein([&](const DaiVein &v) {
        if ((v.status != DaiVeinStatus::Mining) || remembered(m_forward_veins, v.id) || (v.route_cost < 0)) return;
        const int64_t haul = haul_of(v.gold_left, v.ex, v.ey);
        if ((haul > limit) && ((far == nullptr) || (haul > far_haul))) { far = &v; far_haul = haul; }
    });
    uint32_t route = 0;
    DaiSlabXY route_at{ -1, -1 };
    int64_t route_haul = 0;
    c.jobs.ForEach([&](const DaiJob &j) {
        const DaiDigRouteParams *p = std::get_if<DaiDigRouteParams>(&j.p);
        if ((p == nullptr) || DaiJobEnded(j.state) || (p->path.size() < 2)) return;
        if (remembered(m_forward_routes, j.id)) return;
        int64_t gold = 0;
        size_t first = 0;
        for (size_t i = 1; i < p->path.size(); i++) {
            const DaiSlabXY &s = p->path[i];
            const DaiTerrain t = k.SlabAt(s.x, s.y).terrain;
            if ((t != DaiTerrain::Gold) && (t != DaiTerrain::DenseGold)) continue;
            gold += k.GoldLeft(s.x, s.y);
            if (first == 0) first = i;
        }
        if (first == 0) return;
        const DaiSlabXY at = p->path[first - 1];
        const int64_t haul = haul_of(gold, at.x, at.y);
        if ((haul > limit) && (haul > route_haul)) { route = j.id; route_at = at; route_haul = haul; }
    });
    if ((far == nullptr) && (route == 0)) return;
    const bool by_route = (route != 0) && ((far == nullptr) || (route_haul > far_haul));
    DaiRequest q;
    q.kind = DaiRequestKind::Service;
    q.to = DaiDomain::Construction;
    q.service = DaiService::PlaceRoom;
    q.key = (int64_t)DaiRoom::Treasury;
    q.amount = c.personality.ProgrammeSlabs(DaiRoom::Treasury);
    q.at = by_route ? route_at : DaiSlabXY{ far->ex, far->ey };
    q.budget = by_route ? route_haul : far_haul;
    snprintf(q.why, sizeof(q.why), "FORWARD");
    if (w.PostRequest(q) == 0) return;
    if (by_route) {
        remember(m_forward_routes, route);
        DaiLog::Info(c.player, "a treasury asked for by dig route j%u at (%d,%d): %" PRId64 " turns of hauling its gold", route,
            (int)route_at.x, (int)route_at.y, route_haul);
    } else {
        remember(m_forward_veins, far->id);
        DaiLog::Info(c.player, "a treasury asked for by vein %u at (%d,%d): %" PRId64 " turns of hauling left", far->id,
            (int)far->ex, (int)far->ey, far_haul);
    }
}

/** The treasury's size in the room programme (04 section 4, 02 section 12 C8): slabs enough to hold the gold it has
 *  and the gold on its way below FULL_SHARE, at what its treasuries hold a slab now; asked of the director as a floor
 *  on the treasury's Build goal. Until the keeper has a treasury the personality's size stands. */
void DaiEconomy::treasury_size(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiOwnLook &own = k.Own();
    const DaiRequest *asked = c.requests.Find(DaiDomain::Economy, DaiGoalKind::Build, (int64_t)DaiRoom::Treasury);
    int64_t slabs = 0;
    slabs += k.SlabsWithRole(DAI_ROLE_GOLD);
    if ((slabs == 0) || (own.treasury_capacity <= 0)) {
        if (asked != nullptr) w.WithdrawRequest(asked->id);
        return;
    }
    const int64_t per_slab = std::max<int64_t>(own.treasury_capacity / slabs, 1);
    const int64_t gold = own.treasury_used + std::max<int64_t>(own.loose_gold + own.gold_carried, 0);
    const int64_t room_needed = gold * 1024 / FULL_SHARE;
    const int64_t want = std::max((room_needed + per_slab - 1) / per_slab, c.personality.ProgrammeSlabs(DaiRoom::Treasury));
    if ((asked != nullptr) && (asked->count_min == want)) return;
    DaiRequest r = weight(DaiGoalKind::Build, 0, "TREASURY_SIZE");
    r.key = (int64_t)DaiRoom::Treasury;
    r.count_min = want;
    w.PostRequest(r);
}

/** A pay day passed (the turns to it jumped up): was the gold short of the wages just before? */
void DaiEconomy::payday(const DaiTurnCtx &c)
{
    const DaiForecastOut &f = c.forecast;
    if ((m_last_ttp >= 0) && (m_last_ttp <= 1) && (f.turns_to_payday > m_last_ttp) && (m_gold_before < m_wages_before)) {
        m_missed++;
        DaiLog::Info(c.player, "payday short by %" PRId64 " (%" PRId64 " missed)", m_wages_before - m_gold_before, m_missed);
    }
    m_last_ttp = f.turns_to_payday;
    m_gold_before = f.gold_now;
    m_wages_before = f.wages_next;
}

/** Short next pay day: Mine at the most and more KeepGold; enough runway again: back off, and a little less Mine. */
void DaiEconomy::pressure(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiForecastOut &f = c.forecast;
    const DaiGoal *keep_gold = c.goals.Find(DaiGoalKind::KeepGold);
    if (f.short_next) {
        w.PostRequest(weight(DaiGoalKind::Mine, DAI_GOAL_WEIGHT_MAX, "SHORT_NEXT"));
        w.PostRequest(weight(DaiGoalKind::KeepGold, DAI_FIX_ONE, "SHORT_NEXT"));
        m_pressing = true;
    } else if (f.runway_paydays >= c.personality.economy_comfortable_paydays) {
        if (m_pressing) {
            if (const DaiRequest *r = c.requests.Find(DaiDomain::Economy, DaiGoalKind::KeepGold, 0)) w.WithdrawRequest(r->id);
            m_pressing = false;
        }
        w.PostRequest(weight(DaiGoalKind::Mine, -(DAI_FIX_ONE / 4), "COMFORTABLE"));
    }
    if (keep_gold != nullptr) {
        const DaiGoalStatus s = f.short_next ? DaiGoalStatus::Blocked
            : (f.runway_paydays >= keep_gold->p.count) ? DaiGoalStatus::Met : DaiGoalStatus::Progress;
        if (s != keep_gold->status) w.NoteGoal(keep_gold->id, s, f.short_next ? "SHORT_NEXT" : "");
    }
}

void DaiEconomy::Reset()
{
    m_last_ttp = -1;
    m_gold_before = m_wages_before = 0;
    m_pressing = false;
    m_forward_veins.clear();
    m_forward_routes.clear();
    m_sell_refused = 0;
    m_fortify_next = 0;
}

void DaiEconomy::Save(DaiBlobWriter &w) const
{
    w.I64(m_last_ttp);
    w.I64(m_gold_before);
    w.I64(m_wages_before);
    w.I64(m_missed);
    w.Bool(m_pressing);
    w.I64((int64_t)m_forward_veins.size());
    for (uint32_t id : m_forward_veins) w.U32(id);
    w.U32(m_sell_refused);
    w.I64((int64_t)m_forward_routes.size());
    for (uint32_t id : m_forward_routes) w.U32(id);
    w.U64(m_fortify_next);
}

bool DaiEconomy::Load(DaiBlobReader &r)
{
    m_last_ttp = r.Range(-1, INT64_MAX);
    m_gold_before = r.I64();
    m_wages_before = r.I64();
    m_missed = r.Range(0, INT64_MAX);
    m_pressing = r.Bool();
    m_forward_veins.clear();
    const int64_t nv = r.Count(FORWARD_ROUTES);
    for (int64_t i = 0; (i < nv) && r.Ok(); i++) m_forward_veins.push_back(r.U32());
    m_sell_refused = r.U32();
    m_forward_routes.clear();
    const int64_t nr = r.Count(FORWARD_ROUTES);
    for (int64_t i = 0; (i < nr) && r.Ok(); i++) m_forward_routes.push_back(r.U32());
    m_fortify_next = (DaiTurn)r.U64();
    return r.Ok();
}
