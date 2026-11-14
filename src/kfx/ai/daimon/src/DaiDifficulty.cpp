#include "pre_inc.h"
#include "daimon/DaiDifficulty.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>

#include "config_daimon.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiLog.h"
#include "bflib_basics.h"
#include "post_inc.h"

namespace {
const char *const kPaceNames[] = { "relaxed", "human", "unlimited", nullptr };

const DaiDifficulty::Field kFields[] = {
    { "interval_scale", &DaiDifficulty::interval_scale, DAI_FIX_ONE / 4, 8 * DAI_FIX_ONE, "DaiKeeper", DAI_FIX_ONE },
    { "budget_scale",   &DaiDifficulty::budget_scale,   DAI_FIX_ONE / 8, 8 * DAI_FIX_ONE, "DaiKeeper", DAI_FIX_ONE },
    { "noise",          &DaiDifficulty::noise,          0, DAI_FIX_ONE,                   "DaiArbiter", DAI_FIX_ONE },
    { "care_delay",     &DaiDifficulty::care_delay,     0, 100000,                        "DaiCreatures" },
    { "pace",           &DaiDifficulty::pace,           0, 2,                             "DaiArbiter", 1, kPaceNames },
};

/** The built-in levels (05 section 11): easy slower, poorer plans, a fifth of its choices second-best, needs seen
 *  to 600 turns late; hard quicker with larger budgets. */
struct Builtin { const char *id; int64_t interval_scale, budget_scale, noise, care_delay, pace; };
const Builtin kBuiltin[] = {        // pace: easy relaxed, normal human (Q1), hard unlimited
    { "easy",   2 * DAI_FIX_ONE, DAI_FIX_ONE / 2, 205, 600, 0 },
    { "normal", DAI_FIX_ONE,     DAI_FIX_ONE,       0,   0, 1 },
    { "hard",   768,             1536,              0,   0, 2 },
};

struct Cheat { const char *name; uint64_t flag; const char *shown; };
const Cheat kCheats[] = {
    { "sees_all", DAI_CHEAT_SEES_ALL, "sees all" },
    { "rich_start", DAI_CHEAT_RICH_START, "rich start" },
    { "stronger_creatures", DAI_CHEAT_STRONGER, "stronger creatures" },
    { "knows_pool", DAI_CHEAT_KNOWS_POOL, "knows the pool" },
};

const VALUE *difficulty_table(const DaimonConfig &cfg, const char *id)
{
    if (!cfg.loaded) return nullptr;
    const VALUE *all = value_dict_get(&cfg.root, "difficulty");
    const VALUE *d = ((all != nullptr) && (value_type(all) == VALUE_DICT)) ? value_dict_get(all, id) : nullptr;
    return ((d != nullptr) && (value_type(d) == VALUE_DICT)) ? d : nullptr;
}

int64_t scaled(int64_t v, int64_t f) { return std::max<int64_t>(v * f / DAI_FIX_ONE, 1); }
}

std::span<const DaiDifficulty::Field> DaiDifficulty::Fields()
{
    return std::span<const Field>(kFields);
}

bool DaiDifficulty::Resolve(const DaimonConfig &cfg, const char *name, DaiDifficulty *out, std::string *error)
{
    std::string text = ((name == nullptr) || (name[0] == '\0')) ? "normal" : name;
    for (char &ch : text) ch = (char)std::tolower((unsigned char)ch);   // DAIMON_PLAYER's argument comes upper-cased
    // The level, then its cheats after '+'.
    std::string id = text.substr(0, text.find('+'));
    if (id.empty()) id = "normal";
    uint64_t cheats = 0;
    for (size_t at = text.find('+'); at != std::string::npos;) {
        const size_t next = text.find('+', at + 1);
        const std::string c = text.substr(at + 1, (next == std::string::npos) ? std::string::npos : next - at - 1);
        bool known = false;
        for (const Cheat &k : kCheats) if (c == k.name) { cheats |= k.flag; known = true; }
        if (!known) {
            if (error) *error = "no cheat '" + c + "'";
            return false;
        }
        at = next;
    }
    DaiDifficulty d;
    d.m_id = id;
    d.m_cheats = cheats;
    bool known = false;
    for (const Builtin &b : kBuiltin) {
        if (id != b.id) continue;
        d.interval_scale = b.interval_scale;
        d.budget_scale = b.budget_scale;
        d.noise = b.noise;
        d.care_delay = b.care_delay;
        d.pace = b.pace;
        known = true;
    }
    const VALUE *t = difficulty_table(cfg, id.c_str());
    if (!known && (t == nullptr)) {
        if (error) *error = "no difficulty '" + id + "'";
        return false;
    }
    for (const Field &f : kFields) {
        const VALUE *v = (t != nullptr) ? value_dict_get(t, f.key) : nullptr;
        if (v == nullptr) continue;
        int64_t x;
        if (f.names != nullptr) {
            int64_t found = -1;
            if (value_type(v) == VALUE_STRING)
                for (int64_t i = 0; (found < 0) && (f.names[i] != nullptr); i++) if (strcmp(f.names[i], value_string(v)) == 0) found = i;
            if (found < 0) {
                DaiLog::Warn(-1, "difficulty %s: %s is not one of its names; using the default", id.c_str(), f.key);
                continue;
            }
            x = found;
        } else if ((f.scale != 1) && value_is_compatible(v, VALUE_DOUBLE)) {
            const double dv = value_double(v) * (double)f.scale;
            x = (dv >= 9.0e18) ? INT64_MAX : (dv <= -9.0e18) ? INT64_MIN : (int64_t)llround(dv);
        } else if (value_is_compatible(v, VALUE_INT64)) {
            x = value_int64(v) * ((f.scale != 1) ? f.scale : 1);
        } else {
            DaiLog::Warn(-1, "difficulty %s: %s is not a number; using the default", id.c_str(), f.key);
            continue;
        }
        if ((x < f.min) || (x > f.max)) {
            DaiLog::Warn(-1, "difficulty %s: %s is outside its range; clamped", id.c_str(), f.key);
            x = std::clamp(x, f.min, f.max);
        }
        d.*(f.member) = x;
    }
    *out = d;
    return true;
}

std::vector<DaiPersonality::KeyIssue> DaiDifficulty::CheckKeys(const DaimonConfig &cfg)
{
    std::vector<DaiPersonality::KeyIssue> out;
    if (!cfg.loaded) return out;
    const VALUE *all = value_dict_get(&cfg.root, "difficulty");
    if ((all == nullptr) || (value_type(all) != VALUE_DICT)) return out;
    const VALUE *ids[64];
    const size_t n = value_dict_keys_sorted(all, ids, std::min<size_t>(value_dict_size(all), 64));
    for (size_t i = 0; i < n; i++) {
        const char *id = value_string(ids[i]);
        const VALUE *t = value_dict_get(all, id);
        if (value_type(t) != VALUE_DICT) {
            out.push_back({ false, std::string("difficulty.") + id });
            continue;
        }
        const VALUE *keys[64];
        const size_t m = value_dict_keys_sorted(t, keys, std::min<size_t>(value_dict_size(t), 64));
        for (size_t k = 0; k < m; k++) {
            const char *key = value_string(keys[k]);
            const Field *field = nullptr;
            for (const Field &f : kFields) if (strcmp(f.key, key) == 0) field = &f;
            if (field == nullptr) {
                out.push_back({ false, std::string("difficulty.") + id + "." + key });
                continue;
            }
            const VALUE *v = value_dict_get(t, key);
            if ((field->names == nullptr) || (value_type(v) != VALUE_STRING)) continue;
            bool named = false;
            for (int64_t j = 0; field->names[j] != nullptr; j++) named |= (strcmp(field->names[j], value_string(v)) == 0);
            if (!named) out.push_back({ true, std::string("difficulty.") + id + "." + key + "=" + value_string(v) });
        }
    }
    return out;
}

void DaiDifficulty::Apply(DaimonGlobalConfig *cfg, DaiBudget *budget) const
{
    cfg->director_interval = scaled(cfg->director_interval, interval_scale);
    cfg->programme_interval = scaled(cfg->programme_interval, interval_scale);
    cfg->excavation_rescore_interval = scaled(cfg->excavation_rescore_interval, interval_scale);
    if (cfg->creatures_moves_per_kturn > 0)     // 0 stays off
        cfg->creatures_moves_per_kturn = std::max<int64_t>(cfg->creatures_moves_per_kturn * DAI_FIX_ONE / interval_scale, 1);
    budget->reach_visits = scaled(budget->reach_visits, budget_scale);
    budget->search_nodes = scaled(budget->search_nodes, budget_scale);
    budget->layout_candidates = scaled(budget->layout_candidates, budget_scale);
}

int64_t DaiDifficulty::PaceCap(const DaimonGlobalConfig &cfg) const
{
    return (pace == 0) ? cfg.pace_relaxed : (pace == 2) ? cfg.pace_unlimited : cfg.pace_human;
}

std::string DaiDifficulty::Spec() const
{
    std::string out = m_id;
    for (const Cheat &k : kCheats) if ((m_cheats & k.flag) != 0) out += std::string("+") + k.name;
    return out;
}

std::string DaiDifficulty::CheatsText() const
{
    std::string out;
    for (const Cheat &k : kCheats) {
        if ((m_cheats & k.flag) == 0) continue;
        if (!out.empty()) out += ", ";
        out += k.shown;
    }
    return out;
}
