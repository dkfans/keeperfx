#include "pre_inc.h"
#include "daimon/DaiFightRules.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <utility>
#include <vector>

#include "daimon/DaiLog.h"
#include "post_inc.h"

namespace {
constexpr int64_t FINAL_FLOOR = -1000000000;
constexpr int32_t FINAL_MAX_SPEND = 1000000000;

/** One rule in the making: a cast of `power` on the target set, the conditions added one by one. */
struct RuleMaker {
    KeeperAssistRule r;
    RuleMaker(DaiPowerKind power, KeeperAssistTarget target, int64_t level)
    {
        keeper_assist_default_rule(&r, KAA_Cast, power, target);
        r.level = (uint8_t)level;
    }
    RuleMaker &When(KeeperAssistFact fact, KeeperAssistOp op, int64_t value)
    {
        if (r.cond_count < KEEPER_ASSIST_CONDS) r.cond[r.cond_count++] = KeeperAssistCond{ (uint8_t)fact, (uint8_t)op, (int32_t)value };
        return *this;
    }
    RuleMaker &Pick(KeeperAssistPick p) { r.pick = (uint8_t)p; return *this; }
    RuleMaker &Cooldown(int64_t turns) { r.cooldown = (uint16_t)turns; return *this; }
    RuleMaker &TargetCooldown(int64_t turns) { r.target_cooldown = (uint16_t)turns; return *this; }
};
}

DaiFightRules::DaiFightRules()
{
    keeper_assist_default_globals(&m_globals);
    refresh_view();
}

void DaiFightRules::SetBank(DaiFightBank b)
{
    m_bank = b;
    refresh_view();
}

void DaiFightRules::refresh_view()
{
    m_view.rules = m_rules[(size_t)m_bank].data();
    m_view.nrules = m_count[(size_t)m_bank];
    m_view.globals = &m_globals;
    m_view.bank = (int64_t)m_bank;
}

void DaiFightRules::UseHuman(const DaiHumanFightRules &h)
{
    m_source = DaiFightRulesSource::Human;
    m_globals = h.globals;
    m_globals.enabled = 1;
    for (size_t b = 0; b < (size_t)DaiFightBank::Count; b++) {
        m_rules[b] = {};
        int64_t n = 0;
        for (const KeeperAssistRule &r : h.rules) {
            if (!r.used || ((r.action != KAA_Cast) && (r.action != KAA_PowerOff))) continue;
            m_rules[b][(size_t)n++] = r;
        }
        m_count[b] = n;
    }
    refresh_view();
}

void DaiFightRules::UsePersonality(const DaiPersonality &p, const IDaiPlanWorld &world, PlayerNumber player, bool final, int64_t spare,
                                   const DaiContent *content)
{
    m_source = DaiFightRulesSource::Personality;
    keeper_assist_default_globals(&m_globals);
    m_globals.enabled = 1;
    m_globals.reserve = final ? 0 : (int32_t)p.tactics_cast_reserve;
    // With spare gold the pay day's spend rises to cast_spare_share % of it (plan 13: the gold banked fights).
    const int64_t spend = std::max<int64_t>(p.tactics_cast_max_spend, std::max<int64_t>(spare, 0) * p.tactics_cast_spare_share / 100);
    m_globals.max_spend = final ? FINAL_MAX_SPEND : (int32_t)std::min<int64_t>(spend, INT32_MAX);
    // In priority order: a creature saved first, then the strongest enemy hindered, then finished, then help.
    struct Want { std::array<KeeperAssistRule, KEEPER_ASSIST_RULES> r{}; int64_t n = 0; };
    std::array<Want, (size_t)DaiFightBank::Count> want{};
    auto add = [&](DaiFightBank b, int64_t on, DaiPower power, const RuleMaker &m) {
        if (!on) return;
        const DaiPowerKind k = world.PowerOf(power);
        if (k <= 0) return;
        Want &into = want[(size_t)b];
        if (into.n >= (int64_t)into.r.size()) return;
        into.r[(size_t)into.n] = m.r;
        into.r[(size_t)into.n].power = (uint16_t)k;
        into.n++;
    };
    // The last fight keeps nothing back: every gold piece may go on its spells (a floor no gold falls under).
    const int64_t floor = final ? FINAL_FLOOR : p.tactics_cast_reserve, level = p.tactics_spell_level;
    const int64_t disease_floor = final ? FINAL_FLOOR : p.tactics_disease_reserve;
    const RuleMaker heal = RuleMaker(0, KAT_AllyInFight, p.tactics_heal_level)
        .When(KAF_HealthPct, KAO_Lt, p.tactics_heal_below).When(KAF_GoldAfterWages, KAO_Gt, floor).Pick(KAP_LowestHealth);
    const RuleMaker lightning = RuleMaker(0, KAT_EnemyInFight, level)
        .When(KAF_HealthPct, KAO_Lt, p.tactics_finish_below).When(KAF_AlliesNear, KAO_Eq, 0)
        .When(KAF_GoldAfterWages, KAO_Gt, floor).Pick(KAP_Strongest).Cooldown(40);
    const RuleMaker speed = RuleMaker(0, KAT_AllyInFight, level)
        .When(KAF_ForceRatio, KAO_Gt, p.tactics_hinder_ratio).When(KAF_GoldAfterWages, KAO_Gt, floor).Pick(KAP_Strongest);
    const RuleMaker protect = RuleMaker(0, KAT_AllyInFight, level)
        .When(KAF_ForceRatio, KAO_Gt, p.tactics_hinder_ratio).When(KAF_GoldAfterWages, KAO_Gt, floor).Pick(KAP_Strongest);
    // P9: an imp caught in a fight hidden, so the enemy loses it (the built-in computer's use).
    const RuleMaker conceal = RuleMaker(0, KAT_AllyInFight, level)
        .When(KAF_IsDigger, KAO_Eq, 1).When(KAF_HealthPct, KAO_Lt, p.tactics_conceal_imps_below).When(KAF_GoldAfterWages, KAO_Gt, floor)
        .Pick(KAP_LowestHealth);
    const std::array<std::pair<int64_t, DaiPower>, 3> helps = { { { p.tactics_cast_rebound, DaiPower::Rebound },
        { p.tactics_cast_flight, DaiPower::Flight }, { p.tactics_cast_vision, DaiPower::Vision } } };
    // Defensive: the hindering powers on the keeper's own ground (freeze, chicken and disease work only there).
    add(DaiFightBank::Defensive, p.tactics_cast_heal, DaiPower::Heal, heal);
    for (DaiPower hinder : { DaiPower::Freeze, DaiPower::Chicken, DaiPower::Slow }) {
        const int64_t on = (hinder == DaiPower::Freeze) ? p.tactics_cast_freeze
                         : (hinder == DaiPower::Chicken) ? p.tactics_cast_chicken : p.tactics_cast_slow;
        add(DaiFightBank::Defensive, on, hinder, RuleMaker(0, KAT_EnemyInFight, level)
            .When(KAF_ForceRatio, KAO_Gt, p.tactics_hinder_ratio).When(KAF_OnOwnGround, KAO_Eq, 1)
            .When(KAF_GoldAfterWages, KAO_Gt, floor).Pick(KAP_Strongest).Cooldown(60).TargetCooldown(200));
    }
    add(DaiFightBank::Defensive, p.tactics_cast_disease, DaiPower::Disease, RuleMaker(0, KAT_EnemyInFight, level)
        .When(KAF_ForceRatio, KAO_Gt, p.tactics_hinder_ratio).When(KAF_OnOwnGround, KAO_Eq, 1)
        .When(KAF_BattleSize, KAO_Ge, p.tactics_disease_battle).When(KAF_GoldAfterWages, KAO_Gt, disease_floor)
        .Pick(KAP_Strongest).Cooldown(200).TargetCooldown(600));
    add(DaiFightBank::Defensive, p.tactics_cast_lightning, DaiPower::Lightning, lightning);
    add(DaiFightBank::Defensive, p.tactics_cast_speed, DaiPower::Speed, speed);
    add(DaiFightBank::Defensive, p.tactics_cast_protect, DaiPower::Protect, protect);
    for (const auto &h : helps) add(DaiFightBank::Defensive, h.first, h.second, protect);
    add(DaiFightBank::Defensive, p.tactics_cast_conceal, DaiPower::Conceal, conceal);
    // Offensive (07 section 4.3): on another keeper's ground only slow hinders (magic.cfg: ALL_GROUND); heal first, as
    // at home, so its slot keeps its run state when the bank changes.
    add(DaiFightBank::Offensive, p.tactics_cast_heal, DaiPower::Heal, heal);
    add(DaiFightBank::Offensive, p.tactics_cast_slow, DaiPower::Slow, RuleMaker(0, KAT_EnemyInFight, level)
        .When(KAF_ForceRatio, KAO_Gt, p.tactics_hinder_ratio).When(KAF_GoldAfterWages, KAO_Gt, floor)
        .Pick(KAP_Strongest).Cooldown(60).TargetCooldown(200));
    add(DaiFightBank::Offensive, p.tactics_cast_lightning, DaiPower::Lightning, lightning);
    add(DaiFightBank::Offensive, p.tactics_cast_speed, DaiPower::Speed, speed);
    add(DaiFightBank::Offensive, p.tactics_cast_protect, DaiPower::Protect, protect);
    for (const auto &h : helps) add(DaiFightBank::Offensive, h.first, h.second, protect);
    add(DaiFightBank::Offensive, p.tactics_cast_conceal, DaiPower::Conceal, conceal);
    // A map pack's own powers (Phase 7): each by its role, as KeeperFX's power of that role, after those; the best
    // worth for its price first.
    if (content != nullptr) {
        auto addk = [&](DaiFightBank b, int64_t on, DaiPowerKind k, const RuleMaker &m) {
            if (!on || (k <= 0)) return;
            Want &into = want[(size_t)b];
            if (into.n >= (int64_t)into.r.size()) return;
            into.r[(size_t)into.n] = m.r;
            into.r[(size_t)into.n].power = (uint16_t)k;
            into.n++;
        };
        std::vector<const DaiContentEntry *> own;
        for (const DaiContentEntry &e : content->Entries(DaiContentClass::Power)) {
            if (e.base) continue;
            if ((e.role == DaiContentRole::Strike) || (e.role == DaiContentRole::Disable) || (e.role == DaiContentRole::Boost)
                || (e.role == DaiContentRole::Heal)) own.push_back(&e);
        }
        std::stable_sort(own.begin(), own.end(), [](const DaiContentEntry *a, const DaiContentEntry *b) {
            return a->worth * std::max<int64_t>(b->cost, 1) > b->worth * std::max<int64_t>(a->cost, 1);
        });
        const int64_t any_disable = p.tactics_cast_freeze || p.tactics_cast_chicken || p.tactics_cast_slow;
        const int64_t any_boost = p.tactics_cast_speed || p.tactics_cast_protect;
        for (const DaiContentEntry *e : own) {
            const DaiPowerKind k = (DaiPowerKind)e->kind;
            switch (e->role) {
            case DaiContentRole::Strike:
                addk(DaiFightBank::Defensive, p.tactics_cast_lightning, k, lightning);
                addk(DaiFightBank::Offensive, p.tactics_cast_lightning, k, lightning);
                break;
            case DaiContentRole::Disable:
                addk(DaiFightBank::Defensive, any_disable, k, RuleMaker(0, KAT_EnemyInFight, level)
                    .When(KAF_ForceRatio, KAO_Gt, p.tactics_hinder_ratio).When(KAF_OnOwnGround, KAO_Eq, 1)
                    .When(KAF_GoldAfterWages, KAO_Gt, floor).Pick(KAP_Strongest).Cooldown(60).TargetCooldown(200));
                addk(DaiFightBank::Offensive, any_disable, k, RuleMaker(0, KAT_EnemyInFight, level)
                    .When(KAF_ForceRatio, KAO_Gt, p.tactics_hinder_ratio).When(KAF_GoldAfterWages, KAO_Gt, floor)
                    .Pick(KAP_Strongest).Cooldown(60).TargetCooldown(200));
                break;
            case DaiContentRole::Boost:
                addk(DaiFightBank::Defensive, any_boost, k, protect);
                addk(DaiFightBank::Offensive, any_boost, k, protect);
                break;
            case DaiContentRole::Heal:
                addk(DaiFightBank::Defensive, p.tactics_cast_heal, k, heal);
                addk(DaiFightBank::Offensive, p.tactics_cast_heal, k, heal);
                break;
            default: break;
            }
        }
    }
    for (size_t b = 0; b < (size_t)DaiFightBank::Count; b++) {
        int64_t n = 0;
        m_rules[b] = {};
        for (int64_t i = 0; (i < want[b].n) && (n < KEEPER_ASSIST_RULES); i++) {
            const char *why = keeper_assist_rule_validate(&want[b].r[(size_t)i]);
            if (why != nullptr) {
                DaiLog::Warn(player, "fight rules: power %d left out (%s)", (int)want[b].r[(size_t)i].power, why);
                continue;
            }
            m_rules[b][(size_t)n++] = want[b].r[(size_t)i];
        }
        m_count[b] = n;
    }
    refresh_view();
}
