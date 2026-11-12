#include "pre_inc.h"
#include "daimon/DaiForecast.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

#include "daimon/DaiGeom.h"
#include "post_inc.h"

namespace {
/** Gold an income gives over `turns`. */
int64_t income_over(DaiFix per_kturn, int64_t turns)
{
    return (per_kturn <= 0) || (turns <= 0) ? 0 : (per_kturn * turns) / (1000 * DAI_FIX_ONE);
}

/** The rival hearts the keeper has seen (slabs), and how many rival keepers there are. */
std::vector<DaiSlabXY> rival_hearts(const DaiKnowledge &k, int64_t *rivals)
{
    std::vector<DaiSlabXY> hearts;
    for (PlayerNumber r = 0; r < (PlayerNumber)DAI_MAX_PLAYERS; r++) {
        if (!k.RivalKeeper(r)) continue;
        if (rivals != nullptr) (*rivals)++;
        const DaiRivalSighting &s = k.Sighting(r);
        if (s.heart_seen) hearts.push_back(DaiSlabXY{ DaiSubtileSlab(s.heart_x), DaiSubtileSlab(s.heart_y) });
    }
    return hearts;
}

DaiVeinSide side_of(const DaiKnowledge &k, const DaiPersonality &p, const DaiVein &v, const std::vector<DaiSlabXY> &hearts,
                    const DaiGoldSides *sides)
{
    if ((sides != nullptr) && sides->Split()) {
        if (const DaiGoldSides::Vein *t = sides->Find(v.id)) {
            if (t->share <= 0) return DaiVeinSide::Theirs;          // a rival's, or no heart reaches it
            return (t->sharers > 1) ? DaiVeinSide::Contested : DaiVeinSide::Own;
        }
    }
    const DaiOwnLook &own = k.Own();
    if (!own.has_heart || hearts.empty()) return DaiVeinSide::Unknown;
    const int64_t mine = DaiChess(v.cx, v.cy, DaiSubtileSlab(own.heart_x), DaiSubtileSlab(own.heart_y));
    int64_t theirs = INT64_MAX;
    for (const DaiSlabXY &h : hearts) theirs = std::min(theirs, DaiChess(v.cx, v.cy, h.x, h.y));
    if (std::abs(mine - theirs) * 100 <= p.economy_gold_contest_share * std::max(mine, theirs)) return DaiVeinSide::Contested;
    return (mine < theirs) ? DaiVeinSide::Own : DaiVeinSide::Theirs;
}

/** The gold outlook (P9, plan 13 G1): the map's gold from the veins, its side's share, a gem face held. */
void outlook(const DaiKnowledge &k, const DaiPersonality &p, DaiForecastOut *f, const DaiGoldSides *sides)
{
    const PlayerNumber me = k.Player();
    int64_t rivals = 0;
    const std::vector<DaiSlabXY> hearts = rival_hearts(k, &rivals);
    const bool table = (sides != nullptr) && sides->Split();
    const bool split = (k.Own().has_heart && !hearts.empty()) || table;
    k.ForEachVein([&](const DaiVein &v) {
        if ((v.gem_slabs > 0) && !f->gem_held) {
            for (DaiSlabCoord y = v.box.y0; (y <= v.box.y1) && !f->gem_held; y++)
                for (DaiSlabCoord x = v.box.x0; (x <= v.box.x1) && !f->gem_held; x++) {
                    if (k.SlabAt(x, y).terrain != DaiTerrain::Gem) continue;
                    for (const DaiSlabXY &d : { DaiSlabXY{ 0, -1 }, DaiSlabXY{ 1, 0 }, DaiSlabXY{ 0, 1 }, DaiSlabXY{ -1, 0 } }) {
                        const DaiSlabCoord nx = x + d.x, ny = y + d.y;
                        if ((nx < 0) || (ny < 0) || (nx >= k.Width()) || (ny >= k.Height())) continue;
                        const DaiSlabInfo n = k.SlabAt(nx, ny);
                        if ((n.owner == me) && ((n.terrain == DaiTerrain::Floor) || (n.terrain == DaiTerrain::Room))) f->gem_held = true;
                    }
                }
        }
        if (v.gold_left <= 0) return;
        f->map_gold += v.gold_left;
        // By the routes: its share (a third when three keepers are about as near); else the straight line's half.
        const DaiGoldSides::Vein *t = table ? sides->Find(v.id) : nullptr;
        if (t != nullptr) {
            f->side_gold += v.gold_left * t->share / DAI_FIX_ONE;
            return;
        }
        if (hearts.empty()) return;
        const DaiVeinSide side = side_of(k, p, v, hearts, nullptr);
        if (side == DaiVeinSide::Contested) f->side_gold += v.gold_left / 2;
        else if (side == DaiVeinSide::Own) f->side_gold += v.gold_left;
    });
    // Rival hearts not seen yet (levels 60, 61, 79: most seats saw none, or one of two, in 30,000 turns): their keepers
    // hold a share of the map too, out of what the table gave the keeper -- its share as if they were (seen + 1) of
    // (rivals + 1) keepers (plan 13 section 31: on 60 a seat that had seen one of two counted 62,000 its own, mined 29,000).
    if (table && (sides->HeartsSeen() < rivals))
        f->side_gold = f->side_gold * (sides->HeartsSeen() + 1) / (rivals + 1);
    if (!split) f->side_gold = f->map_gold / (rivals + 1);
    // A mined-out vein leaves the table: empty after some mining, the map's gold is gone, still known (P9: map 124's
    // last vein gone read as "not known", the income endless again). A full table may have left some out.
    f->gold_known = (k.VeinCount() < DAI_MAX_VEINS) && ((k.VeinCount() > 0) || (k.Own().gold_mined > 0));
}
}

DaiVeinSide DaiForecast::VeinSide(const DaiKnowledge &k, const DaiPersonality &p, const DaiVein &v, const DaiGoldSides *sides)
{
    return side_of(k, p, v, rival_hearts(k, nullptr), sides);
}

void DaiIncomeEma::Save(DaiBlobWriter &w) const
{
    w.I64(income_per_kturn);
    w.U64(last);
    w.I64(last_mined);
    w.I64(samples);
}

bool DaiIncomeEma::Load(DaiBlobReader &r)
{
    income_per_kturn = r.Range(0, INT64_MAX / 4);
    last = (DaiTurn)r.U64();
    last_mined = r.Range(0, INT64_MAX / 4);
    samples = r.Range(0, INT64_MAX);
    if (!r.Ok()) *this = DaiIncomeEma();
    return r.Ok();
}

int64_t DaiForecast::TurnsToPayday(int64_t progress, int64_t gap, int64_t speed)
{
    const int64_t per_turn = speed / 100;
    if (per_turn <= 0) return DAI_NEVER;
    if (gap <= progress) return 0;
    return (gap - progress + per_turn - 1) / per_turn;
}

void DaiForecast::Compute(const DaiKnowledge &k, const DaiLedger &l, const DaiPersonality &p, const DaimonGlobalConfig &cfg,
                          const DaiIncomeEma &ema, DaiTurn now, DaiForecastOut *out, const DaiGoldSides *sides)
{
    const DaiOwnLook &own = k.Own();
    DaiForecastOut f;
    f.now = now;
    f.gold_now = own.gold;
    const int64_t room = std::max<int64_t>(own.treasury_capacity - own.treasury_used, 0);
    f.gold_pending = std::min(std::max<int64_t>(own.loose_gold + own.gold_carried, 0), room);
    f.wages_next = std::max<int64_t>(own.wage_bill, 0);
    f.turns_to_payday = TurnsToPayday(own.payday_progress, own.payday_gap, own.payday_speed);
    f.committed = l.GoldClaimed();
    f.income_per_kturn = ema.income_per_kturn;
    f.sides = sides;
    outlook(k, p, &f, sides);
    // Training and scavenging are paid as they go (04 section 8): what they cost until a turn is gold not there then.
    for (const DaiCreatureLook &c : k.Creatures()) f.upkeep_per_kturn += c.upkeep_per_kturn;
    if (own.obey_active) f.upkeep_per_kturn += own.obey_upkeep_per_kturn;     // Must Obey, paid as it runs
    const DaiFix upkeep = FixFromInt(f.upkeep_per_kturn);
    const bool payday_comes = (f.turns_to_payday != DAI_NEVER);
    if (payday_comes && (f.turns_to_payday <= cfg.economy_reserve_window))
        f.reserve = f.wages_next + FixMul(f.wages_next, p.economy_reserve_margin) + income_over(upkeep, f.turns_to_payday);
    // Armageddon (P9, whoever cast it): from the cast on the gold is the last fight's spells', nothing else's.
    if (own.armageddon) f.reserve = std::max<int64_t>(f.reserve, f.gold_now - f.committed);
    f.spendable = f.gold_now - f.committed - f.reserve;
    f.bank_target = f.wages_next * p.economy_bank_paydays;
    f.spare = f.gold_now - f.committed - std::max(f.reserve, f.bank_target);
    const int64_t free_gold = f.gold_now - f.committed;
    // The income until a turn: its average, but no more than the side's gold left (P9) unless a gem is held.
    const bool capped = f.gold_known && !f.gem_held;
    auto income_until = [&](int64_t turns) {
        const int64_t g = income_over(f.income_per_kturn, turns);
        return capped ? std::min(g, f.side_gold) : g;
    };
    if (capped && (f.side_gold <= 0)) f.horizon_turns = 0;           // its side's gold gone: the income has ended
    else if (capped && (f.income_per_kturn > 0))
        f.horizon_turns = (f.side_gold * 1000 * DAI_FIX_ONE) / f.income_per_kturn;
    f.short_next = payday_comes
        && (free_gold + income_until(f.turns_to_payday) - income_over(upkeep, f.turns_to_payday) < f.wages_next);
    // Pay day n (from 1) comes turns_to_payday + (n - 1) * interval turns from now; it is covered when the free gold
    // and the income until then pay all n wage bills.
    if (!payday_comes || (f.wages_next == 0)) {
        f.runway_paydays = DAI_RUNWAY_MAX;
    } else {
        const int64_t interval = std::max<int64_t>(TurnsToPayday(0, own.payday_gap, own.payday_speed), 1);
        int64_t n = 0;
        while ((n < DAI_RUNWAY_MAX)
            && (free_gold + income_until(f.turns_to_payday + n * interval)
                - income_over(upkeep, f.turns_to_payday + n * interval) >= (n + 1) * f.wages_next))
            n++;
        f.runway_paydays = n;
    }
    *out = f;
}

void DaiForecast::UpdateEma(DaiIncomeEma &ema, const DaiKnowledge &k, DaiTurn now)
{
    const int64_t mined = std::max<int64_t>(k.Own().gold_mined, 0);
    if ((ema.samples == 0) || (mined < ema.last_mined) || (now < ema.last)) {
        ema = DaiIncomeEma();
        ema.last = now;
        ema.last_mined = mined;
        ema.samples = 1;
        return;
    }
    const int64_t dt = (int64_t)(now - ema.last);
    if (dt < DAI_INCOME_SAMPLE) return;
    const DaiFix rate = ((mined - ema.last_mined) * 1000 * DAI_FIX_ONE) / dt;
    if (ema.samples == 1) {
        ema.income_per_kturn = rate;
    } else {
        const DaiFix alpha = (dt * DAI_FIX_ONE) / (dt + DAI_INCOME_TAU);
        ema.income_per_kturn += FixMul(rate - ema.income_per_kturn, alpha);
    }
    ema.last = now;
    ema.last_mined = mined;
    ema.samples++;
}
