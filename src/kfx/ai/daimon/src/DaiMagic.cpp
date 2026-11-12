#include "pre_inc.h"
#include "daimon/DaiMagic.h"
#include "daimon/DaiGeom.h"

#include <algorithm>
#include <cinttypes>

#include "daimon/DaiLog.h"
#include "post_inc.h"

namespace {
/** A requested cast: after the managers' own work, before Tactics' fights (the arbiter's step utility). */
constexpr DaiFix CAST_UTILITY = 4 * DAI_FIX_ONE;
}

DaiPower DaiMagic::PowerFor(DaiService s)
{
    switch (s) {
    case DaiService::SightPeek: return DaiPower::Sight;
    case DaiService::DestroyWalls: return DaiPower::DestroyWalls;
    default: return DaiPower::Count;
    }
}

void DaiMagic::obey(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiPersonality &p = c.personality;
    const DaiKnowledge &k = c.know;
    const DaiOwnLook &own = k.Own();
    if (own.obey_active && (m_obey_since == 0)) m_obey_since = c.now;
    if (!own.obey_active && (m_obey_since != 0)) {         // off: its switch, or the gold ran out
        m_obey_turns += (int64_t)(c.now - m_obey_since);
        m_obey_since = 0;
    }
    if (DaiJob *j = w.FindJob(DAI_JOB_MAGIC_CAST, KEY_OBEY)) if (!DaiJobEnded(j->state)) return;
    const DaiPowerLook &power = k.Power(DaiPower::Obey);
    if (!own.obey_active && ((p.magic_obey == 0) || !power.available)) return;
    int64_t fighters = 0, workers = 0;
    const uint64_t work_roles = DAI_ROLE_RESEARCH | DAI_ROLE_MANUFACTURE | DaiWorkRole(DaiWork::Train);
    for (const DaiCreatureLook &cr : k.Creatures()) {
        if (k.Kind(cr.model).digger || (cr.held_by >= 0)) continue;
        if ((cr.battle != 0) || (cr.activity == DaiActivity::Fight) || (cr.activity == DaiActivity::FightDoor)) {
            fighters++;
        } else if (((cr.activity == DaiActivity::Work) || (cr.activity == DaiActivity::DeepWork)) && (cr.in_room != 0)) {
            for (const DaiRoomLook &r : k.Rooms()) {
                if ((r.kind != cr.in_room) || ((r.roles & work_roles) == 0)) continue;
                workers++;
                break;
            }
        }
    }
    const DaiForecastOut &f = c.forecast;
    const int64_t after_wages = f.gold_now - f.committed - f.wages_next;
    // Armageddon's last fight (P9): on for it, whatever the gold, and never off (the game ends it when the gold does).
    if (own.armageddon) {
        if (own.obey_active || !power.available || (p.magic_obey == 0) || (power.price > f.gold_now - f.committed)) return;
        DaiCastParams cp;
        cp.power = DaiPower::Obey;
        cp.price = power.price;
        if (DaiJob *j = w.AddJob(DAI_JOB_MAGIC_CAST, KEY_OBEY, CAST_UTILITY)) {
            j->p = cp;
            m_obey_switched = c.now;
            DaiLog::Info(c.player, "must obey on (ARMAGEDDON)");
        }
        return;
    }
    const bool fight = (p.magic_obey >= 1) && (fighters >= p.magic_obey_fighters);
    const bool work = (p.magic_obey >= 2) && (workers >= p.magic_obey_workers);
    const bool held = (m_obey_switched != 0) && (c.now < m_obey_switched + (DaiTurn)p.magic_obey_hold);
    DaiCastParams cp;
    cp.power = DaiPower::Obey;
    const char *why = nullptr;
    if (own.obey_active) {
        why = (p.magic_obey == 0) ? "NEVER" : (after_wages < p.magic_obey_floor) ? "FLOOR" : (!fight && !work && !held) ? "NO_NEED" : nullptr;
        if (why == nullptr) return;
        cp.off = true;
    } else {
        if ((!fight && !work) || held || (after_wages < p.magic_obey_reserve)) return;
        why = fight ? "FIGHT" : "WORK";
        cp.price = power.price;
    }
    DaiJob *j = w.AddJob(DAI_JOB_MAGIC_CAST, KEY_OBEY, CAST_UTILITY);
    if (j == nullptr) return;
    j->p = cp;
    m_obey_switched = c.now;
    DaiLog::Info(c.player, "must obey %s (%s): %" PRId64 " fighting, %" PRId64 " at work, %" PRId64 " gold after wages",
        cp.off ? "off" : "on", why, fighters, workers, after_wages);
}

bool DaiMagic::SpecialWanted(const DaiKnowledge &k, const DaiPersonality &p, DaiSpecial kind, DaiSpecialParams *sp)
{
    switch (kind) {
    case DaiSpecial::RevealMap: case DaiSpecial::StealHero: case DaiSpecial::Multiply: case DaiSpecial::IncreaseLevel:
    case DaiSpecial::MakeSafe: case DaiSpecial::GetGold:
        return true;
    case DaiSpecial::HealAll: {
        int64_t hurt = 0;
        for (const DaiCreatureLook &cr : k.Creatures()) hurt += (cr.health_pct < p.magic_heal_all_below) ? 1 : 0;
        return hurt >= p.magic_heal_all_hurt;
    }
    case DaiSpecial::Resurrect: {
        int64_t best = -1;
        for (const DaiDeadLook &d : k.Dead()) {
            // Not an imp (the box waits for a fighter): with the rival's imps dying often, they were most of those back.
            if ((d.model < 1) || (d.model > k.KindCount()) || !k.Kind(d.model).known || k.Kind(d.model).digger) continue;
            const int64_t lv = std::clamp<int64_t>(d.level - 1, 0, DaiKindLook::LEVELS - 1);
            const int64_t score = k.Kind(d.model).score[lv];
            if (score <= best) continue;
            best = score;
            sp->model = d.model;
            sp->level = d.level;
            snprintf(sp->code, sizeof(sp->code), "%s", k.Kind(d.model).code);
        }
        return best >= 0;
    }
    default:    // transfer, hidden world (a campaign's choices), a level script's own, the harmful ones
        return false;
    }
}

void DaiMagic::specials(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if ((c.personality.magic_specials == 0) || c.know.Specials().empty() || (c.now < m_special_retry)) return;
    if (DaiJob *j = w.FindJob(DAI_JOB_USE_SPECIAL, KEY_SPECIAL)) if (!DaiJobEnded(j->state)) return;
    for (const DaiSpecialLook &b : c.know.Specials()) {
        DaiSpecialParams sp;
        sp.box = b.ref;
        sp.kind = b.kind;
        if (!SpecialWanted(c.know, c.personality, b.kind, &sp)) continue;
        DaiJob *j = w.AddJob(DAI_JOB_USE_SPECIAL, KEY_SPECIAL, CAST_UTILITY);
        if (j == nullptr) return;
        j->p = sp;
        DaiLog::Info(c.player, "special box %d (kind %d) at (%d,%d) used%s%s", (int)b.ref.idx, (int)b.kind, (int)b.x, (int)b.y,
            (b.kind == DaiSpecial::Resurrect) ? ": resurrect " : "", (b.kind == DaiSpecial::Resurrect) ? sp.code : "");
        return;
    }
}

int64_t DaiMagic::RivalsForce(const DaiKnowledge &k, const DaiRivals &rivals, PlayerNumber me, DaiTurn now)
{
    // Each rival's known force (the query panel's count at the levels seen, or the levels seen) and the growth guessed.
    int64_t theirs = 0, n = 0;
    for (PlayerNumber r = 0; r < DAI_MAX_PLAYERS; r++) {
        if ((r == me) || !k.RivalKeeper(r) || k.Alliance(r).ours) continue;
        theirs += k.KnownForce(r, now) + rivals.Rival(r).growth;
        n++;
    }
    return (n > 0) ? theirs : -1;
}

bool DaiMagic::ArmageddonReady(const DaiKnowledge &k, const DaiPersonality &p, const DaiForecastOut &f, int64_t rivals_force,
                               const char **why)
{
    const DaiOwnLook &own = k.Own();
    const DaiPowerLook &power = k.Power(DaiPower::Armageddon);
    *why = (p.magic_armageddon == 0) ? "NEVER" : !power.available ? "NOT_AVAILABLE" : own.armageddon ? "UNDER_WAY"
         : !own.has_heart ? "NO_HEART" : nullptr;
    if (*why != nullptr) return false;
    if ((own.heart_max_health > 0) && (own.heart_health * 100 < p.magic_armageddon_heart * own.heart_max_health)) { *why = "HEART"; return false; }
    // An end-game move: the wages after it do not count, the last fight's spells do (P9).
    if (f.gold_now - f.committed < power.price + p.magic_armageddon_reserve) { *why = "GOLD"; return false; }
    // Only with gold plentiful (owner, plan 13): the price at most armageddon_gold_share % of the gold it can still have,
    // its bank and its side's gold left -- else training, Destroy Walls and an assault do more with it. A gem: no limit.
    if (f.gold_known && !f.gem_held && (power.price * 100 > p.magic_armageddon_gold_share * (f.gold_now + f.side_gold))) {
        *why = "GOLD_SCARCE";
        return false;
    }
    // Its levels against every rival keeper's estimated (RivalsForce; -1: no rival).
    if (rivals_force < 0) { *why = "NO_RIVAL"; return false; }
    int64_t mine = 0;
    for (const DaiCreatureLook &cr : k.Creatures()) if (!k.Kind(cr.model).digger) mine += cr.level;
    if (mine * 100 < p.magic_armageddon_superiority * rivals_force) { *why = "NOT_STRONG_ENOUGH"; return false; }
    // Traps by the heart, where the last fight is.
    int64_t traps = 0;
    for (const DaiTrapLook &t : k.Traps()) {
        const int64_t dx = std::abs((int64_t)t.x - own.heart_x), dy = std::abs((int64_t)t.y - own.heart_y);
        traps += (std::max(dx, dy) <= ARMAGEDDON_TRAP_REACH * DAI_STL_PER_SLB) ? 1 : 0;
    }
    if (traps < p.magic_armageddon_traps) { *why = "TRAPS"; return false; }
    return true;
}

void DaiMagic::armageddon(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    if ((c.personality.magic_armageddon == 0) || (c.now < m_armageddon_next)) return;
    m_armageddon_next = c.now + ARMAGEDDON_LOOK;
    if (DaiJob *j = w.FindJob(DAI_JOB_MAGIC_CAST, KEY_ARMAGEDDON)) if (!DaiJobEnded(j->state)) return;
    const char *why = nullptr;
    const int64_t theirs = RivalsForce(c.know, c.rivals, c.player, c.now);
    const bool ready = ArmageddonReady(c.know, c.personality, c.forecast, theirs, &why);
    if ((why != nullptr) && (why != m_armageddon_why)) {
        int64_t levels = 0;
        for (const DaiCreatureLook &cr : c.know.Creatures()) if (!c.know.Kind(cr.model).digger) levels += cr.level;
        DaiLog::Info(c.player, "armageddon not cast (%s): %" PRId64 " gold, %" PRId64 " levels against %" PRId64, why,
            c.forecast.gold_now, levels, theirs);
    }
    m_armageddon_why = why;
    if (!ready) return;
    DaiCastParams cp;
    cp.power = DaiPower::Armageddon;
    cp.price = c.know.Power(DaiPower::Armageddon).price;
    DaiJob *j = w.AddJob(DAI_JOB_MAGIC_CAST, KEY_ARMAGEDDON, CAST_UTILITY);
    if (j == nullptr) return;
    j->p = cp;
    m_armageddon_cast = c.now;
    DaiLog::Info(c.player, "armageddon cast, %" PRId64 " gold", cp.price);
}

/** Speed Up on a creature at work (P9, plan 13; the built-in computer's SplSpdu): it trains, researches and manufactures
 *  twice as fast while it lasts. Training first, then research while a room is left to research, then manufacture; one
 *  not sped up already, the lowest level first; one cast at a time, at level 1 (the cheapest a turn of it), while the
 *  spare gold covers work_speed_paydays pay days' wages and no room is being placed or grown (the rooms' gold first:
 *  dai_build_basic's lair stayed full 1,079 turns when the casts took the gold a workshop and the lair's growth were
 *  waiting for). */
void DaiMagic::work_speed(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    const DaiKnowledge &k = c.know;
    const DaiPersonality &p = c.personality;
    if ((p.magic_work_speed == 0) || k.Own().armageddon || (c.forecast.SparePaydays() < p.magic_work_speed_paydays)) return;
    if (const DaiJob *j = w.FindJob(DAI_JOB_MAGIC_CAST, KEY_WORK_SPEED)) if (!DaiJobEnded(j->state)) return;
    bool building = false;
    c.jobs.ForEach([&](const DaiJob &j) {
        building = building || (((j.kind == DAI_JOB_PLACE_ROOM) || (j.kind == DAI_JOB_GROW_ROOM)) && !DaiJobEnded(j.state));
    });
    if (building) return;
    const DaiPowerLook &pw = k.Power(DaiPower::Speed);
    if (!pw.available || (pw.price > c.forecast.spendable)) return;
    const DaiResearchLook &rs = k.Research();
    bool rooms_left = false;
    for (int64_t i = 0; (i < rs.count) && (i < DaiResearchLook::MAX); i++) rooms_left = rooms_left || (rs.items[i].kind == DaiResearchKind::Room);
    const DaiCreatureLook *target = nullptr;
    for (uint64_t role : { DAI_ROLE_TRAIN, rooms_left ? DAI_ROLE_RESEARCH : (uint64_t)0, DAI_ROLE_MANUFACTURE }) {
        if (role == 0) continue;
        for (const DaiCreatureLook &cr : k.Creatures()) {
            if (cr.sped || (cr.activity != DaiActivity::Work) || (cr.in_room == 0) || k.Kind(cr.model).digger) continue;
            bool there = false;
            there = there || k.InRoomWith(cr, role);
            if (there && ((target == nullptr) || (cr.level < target->level))) target = &cr;
        }
        if (target != nullptr) break;
    }
    if (target == nullptr) return;
    DaiJob *j = w.AddJob(DAI_JOB_MAGIC_CAST, KEY_WORK_SPEED, FixFromInt(1));
    if (j == nullptr) return;
    DaiCastParams cp;
    cp.power = DaiPower::Speed;
    cp.level = 0;
    cp.target = target->ref;
    cp.price = pw.price;
    j->p = cp;
}

void DaiMagic::Plan(const DaiTurnCtx &c, DaiManagerWrites &w)
{
    armageddon(c, w);
    obey(c, w);
    specials(c, w);
    work_speed(c, w);
    // A request withdrawn (its asker gave up): its cast is dropped, and with it the gold claimed.
    std::vector<uint64_t> gone;
    c.jobs.ForEach([&](const DaiJob &j) {
        if ((j.kind != DAI_JOB_MAGIC_CAST) || DaiJobEnded(j.state) || (j.request_id == 0)) return;
        bool there = false;
        c.requests.ForEach([&](const DaiRequest &r) { there = there || (r.id == j.request_id); });
        if (!there) gone.push_back(j.key);
    });
    for (uint64_t key : gone) if (DaiJob *j = w.FindJob(DAI_JOB_MAGIC_CAST, key)) w.Cancel(*j, "WITHDRAWN");
    std::vector<DaiRequest> asked;
    c.requests.ForEachService(DaiDomain::Magic, [&](const DaiRequest &r) { if (!r.served) asked.push_back(r); });
    for (const DaiRequest &r : asked) {
        const DaiPower power = PowerFor(r.service);
        const DaiSlabCoord x = r.at.x, y = r.at.y;
        if ((power >= DaiPower::Count) || (x < 0) || (y < 0) || (x >= c.know.Width()) || (y >= c.know.Height())) {
            w.MarkRefused(r.id);
            continue;
        }
        const DaiPowerKind kind = c.world.PowerOf(power);
        const int64_t level = std::clamp<int64_t>(r.amount, 0, 8);
        const DaiSubtlCoord sx = DaiSlabSubtileCenter(x), sy = DaiSlabSubtileCenter(y);
        // As a human would find it: the power known and castable at that spot (Destroy Walls: on a revealed wall). The
        // spot's answer comes with the next gather: until then the request waits.
        const DaiAnswer castable = ((kind == 0) || !c.know.Power(power).available) ? DaiAnswer::No : c.world.PowerCastable(c.player, kind, sx, sy);
        if (castable == DaiAnswer::Unknown) continue;
        if (castable == DaiAnswer::No) {
            DaiLog::Info(c.player, "%s at (%d,%d) refused: %s", DaiPowerCode(power), (int)x, (int)y,
                (kind == 0) ? "no such power" : !c.know.Power(power).available ? "not available" : "not castable there");
            w.MarkRefused(r.id);
            continue;
        }
        DaiJob *j = w.AddJob(DAI_JOB_MAGIC_CAST, KEY_REQUEST + r.id, CAST_UTILITY);
        if (j == nullptr) return;           // the pool is full: next turn
        DaiCastParams p;
        p.power = power;
        p.level = level;
        p.x = sx;
        p.y = sy;
        p.price = c.world.PowerPrice(c.player, kind, level);
        j->p = p;
        j->request_id = r.id;
        w.MarkServed(r.id);
        DaiLog::Info(c.player, "%s at (%d,%d), level %" PRId64 ", %" PRId64 " gold, for %s", DaiPowerCode(power), (int)x, (int)y, level,
            p.price, DaiDomainName(r.from));
    }
}

void DaiMagic::OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w)
{
    if ((j.kind == DAI_JOB_USE_SPECIAL) && (j.state != DaiJobState::Done)) {
        m_special_retry = c.now + SPECIAL_RETRY;
        DaiLog::Info(c.player, "special box use ended %s (%s)", DaiJobStateName(j.state), j.why);
    }
    if ((j.kind == DAI_JOB_MAGIC_CAST) && (j.state == DaiJobState::Done)) {
        const DaiCastParams *p = std::get_if<DaiCastParams>(&j.p);
        if ((p != nullptr) && (p->power == DaiPower::DestroyWalls)) {
            m_walls_cast++;
            m_walls_gold += p->price;
        }
    }
    if ((j.kind != DAI_JOB_MAGIC_CAST) || (j.state == DaiJobState::Done) || (j.request_id == 0)) return;
    w.MarkRefused(j.request_id);
}
